#!/usr/bin/env python3
"""Export CLOAD's text and tables as files people can read.

CLOAD already keeps its data in one blob. Its icons come from a separate CD
resource; the blob holds messages, locations, card steps and character charts.
The build links those bytes unchanged. This tool writes readable YAML copies.

``CloadSymbols`` finds addresses in the version's symbol file. ``read_blob``
walks the data in order, using the same format readers as ADDHERO and CARDA.
The byte map also covers padding and the zero-filled runtime buffers.

Example:

    python3 -m tools.data.overlays.cload --version us assets/exports/us/overlays/cload
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import shutil
import sys
import tempfile

from tools.data.overlays import card_data, splat_config, symbols, text_table
from tools.data.overlays.card_data import (
    CHART_LEADS,
    CHART_PAGE_BYTES,
    CardSteps,
    Chart,
    StepSequence,
    read_digit_glyphs,
    read_text_list,
)
from tools.data.overlays.resources import (
    Blob, Part, byte_map_entry, cover_gaps, dump_yaml, hex_address, write_part,
)

REPO_ROOT = Path(__file__).resolve().parents[3]
OVERLAY_CONFIG = "overlays/CLOAD.BIN.yaml"
SYMBOL_FILE = "symbols/cload_symbol_addrs.txt"


# ---------------------------------------------------------------------------
# Inputs


@dataclass(frozen=True)
class CloadSymbols:
    """The addresses CLOAD's readers need, named in SYMBOL_NAMES below."""

    messages: int
    locations: int
    card_steps: int
    chart: int
    double_byte_chart: int
    decimal_glyphs: int
    hex_glyphs: int
    variables: int
    named: dict[str, int]

    @classmethod
    def load(cls, path: Path) -> CloadSymbols:
        named = symbols.load(path)
        missing = [name for name in SYMBOL_NAMES.values() if name not in named]
        if missing:
            raise ValueError(
                f"{path} has no {', '.join(missing)}. If a symbol was renamed, "
                "update SYMBOL_NAMES in tools/data/overlays/cload.py."
            )
        addresses = {key: named[name] for key, name in SYMBOL_NAMES.items()}
        return cls(**addresses, named=named)

    @property
    def chart_pages(self) -> int:
        """The decoder adds the lead byte times a page to this base.

        The named double-byte chart starts at lead 0x19. Counting back gives
        the same base as CLOAD_GLYPH_CHART_PAGE_BASE in cload_glyph.c.
        """
        return self.double_byte_chart - CHART_LEADS.start * CHART_PAGE_BYTES

    def with_prefix(self, prefix: str | tuple[str, ...]) -> dict[str, int]:
        return {
            name: address for name, address in self.named.items() if name.startswith(prefix)
        }


SYMBOL_NAMES = {
    "messages": "g_cload_text_check_memory_card",
    "locations": "g_card_menu_location_names",
    "card_steps": "g_cload_steps_initial_scan",
    "chart": "g_glyph_single_byte_chart",
    "double_byte_chart": "g_cload_double_byte_char_table",
    "decimal_glyphs": "g_glyph_decimal_digits",
    "hex_glyphs": "g_glyph_hex_digits",
    "variables": "g_card_menu_element_pool",
}
MESSAGE_SYMBOL_PREFIX = ("g_cload_text_", "g_card_menu_text_")
CARD_STEP_SYMBOL_PREFIX = "g_cload_steps_"


@dataclass(frozen=True)
class Inputs:
    """Where one version's CLOAD blob and symbols live."""

    version: str
    config: Path
    assets: Path

    @property
    def symbol_file(self) -> Path:
        return self.config / SYMBOL_FILE

    @property
    def overlay_config(self) -> Path:
        return self.config / OVERLAY_CONFIG


def card_step_names() -> dict[int, str]:
    """Step opcode names from the C headers, so names stay in one place."""
    return card_data.card_menu_step_names()


# ---------------------------------------------------------------------------
# Reading the blob, in address order


def read_blob(blob: Blob[CloadSymbols]) -> list[Part]:
    """Read each resource, then account for the bytes between them."""
    chart = Chart(blob)
    parts = [
        read_messages(blob, chart),
        read_locations(blob, chart),
        read_card_steps(blob),
        read_chart(chart),
        read_digit_glyphs(blob),
        read_variables(blob),
    ]
    return cover_gaps(blob, parts)


def read_messages(blob: Blob[CloadSymbols], chart: Chart) -> Part:
    """The shared card-screen message table carried by the load screen."""
    table = blob.symbols.messages
    content, end = read_text_list(blob, chart, table, MESSAGE_SYMBOL_PREFIX)
    if end > blob.offset(blob.symbols.locations):
        raise ValueError("message table extends into the location table")
    return Part("messages", blob.offset(table), end, "text/messages.yaml", content)


def read_locations(blob: Blob[CloadSymbols], chart: Chart) -> Part:
    """Location names picked by the music track stored in each save."""
    table = blob.symbols.locations
    content, end = read_text_list(blob, chart, table, "g_card_menu_location_names")
    if end > blob.offset(blob.symbols.card_steps):
        raise ValueError("location table extends into the card steps")
    return Part("locations", blob.offset(table), end, "text/locations.yaml", content)


def read_card_steps(blob: Blob[CloadSymbols]) -> Part:
    """Follow each named entry through DONE, keeping shared tails intact."""
    names = card_step_names()
    starts = sorted(
        (address, name)
        for name, address in blob.symbols.with_prefix(CARD_STEP_SYMBOL_PREFIX).items()
    )
    start = blob.offset(blob.symbols.card_steps)
    end = blob.offset(blob.symbols.chart)
    sequences = []
    for address, name in starts:
        position = blob.offset(address)
        if not start <= position < end:
            raise ValueError(f"{name} is outside the card step table")
        done = blob.data.find(b"\x00", position, end)
        if done < 0:
            raise ValueError(f"{name} has no CARD_MENU_STEP_DONE before the character chart")
        raw = blob.data[position : done + 1]
        steps = tuple(names.get(value, f"0x{value:02X}") for value in raw)
        sequences.append(StepSequence(name, steps, raw))
    content = CardSteps(blob.symbols.card_steps, tuple(sequences))
    return Part("card steps", start, end, "tables/card_steps.yaml", content)


def read_chart(chart: Chart) -> Part:
    """The game's character codes and their Shift-JIS characters."""
    return Part(
        "text to Shift-JIS table", chart.start, chart.end,
        "tables/text_conversion.yaml", chart.content(),
    )


def read_variables(blob: Blob[CloadSymbols]) -> Part:
    """Runtime variables and buffers, which are all zero in the original blob."""
    start, end = blob.offset(blob.symbols.variables), len(blob.data)
    if any(blob.data[start:]):
        return Part("variables", start, end, "unknown/variables.bin", blob.data[start:])
    note = "CLOAD's variables and buffers; all zero on the disc, not exported."
    return Part("variables", start, end, note=note)


# ---------------------------------------------------------------------------
# Putting it together


def load_blob(inputs: Inputs) -> Blob[CloadSymbols]:
    """Find the blob through splat and check its size and resource order."""
    names = CloadSymbols.load(inputs.symbol_file)
    files = splat_config.data_files(inputs.overlay_config, inputs.assets)
    source = splat_config.file_containing(files, names.messages, SYMBOL_NAMES["messages"])
    if not source.path.exists():
        raise ValueError(f"{source.path} is missing; run make splat first")
    data = source.path.read_bytes()
    if len(data) != source.end - source.start:
        raise ValueError(f"{source.path} size does not match the splat config; run make splat again")
    previous = source.start - 1
    for key in SYMBOL_NAMES:
        address = getattr(names, key)
        if not source.contains(address):
            raise ValueError(f"{SYMBOL_NAMES[key]} is outside {source.path}; check the splat config")
        if address <= previous:
            raise ValueError(f"{SYMBOL_NAMES[key]} is out of resource order")
        previous = address
    return Blob(data, source.start, source.path.name, inputs.version, names)


def extract(inputs: Inputs, output: Path) -> None:
    """Read everything before writing, then move the finished folder into place."""
    if output.exists():
        raise FileExistsError(f"{output} already exists")
    blob = load_blob(inputs)
    parts = read_blob(blob)
    byte_map = {
        "source": blob.file_name,
        "address": hex_address(blob.address),
        "size": f"0x{len(blob.data):X}",
        "ranges": [byte_map_entry(blob, part) for part in parts],
    }
    output.parent.mkdir(parents=True, exist_ok=True)
    staging = Path(tempfile.mkdtemp(prefix=f".{output.name}-", dir=output.parent))
    try:
        for part in parts:
            if part.content is not None:
                write_part(part.content, staging / part.file)
        dump_yaml(staging / "byte-map.yaml", byte_map)
        staging.rename(output)
    except BaseException:
        shutil.rmtree(staging, ignore_errors=True)
        raise


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("output", type=Path, help="new folder to write")
    parser.add_argument("--version", choices=sorted(text_table.TWO_BYTE_CODES), default="us")
    parser.add_argument("--config", type=Path, help="config folder (default config/<version>)")
    parser.add_argument("--assets", type=Path, help="splat assets (default assets/<version>)")
    args = parser.parse_args()
    inputs = Inputs(
        args.version,
        args.config or REPO_ROOT / "config" / args.version,
        args.assets or REPO_ROOT / "assets" / args.version,
    )
    try:
        extract(inputs, args.output)
    except (OSError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
