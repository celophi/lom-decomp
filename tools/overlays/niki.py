#!/usr/bin/env python3
"""Export NIKI's data as files people can read.

NIKI is the diary's save-file screen. It browses both memory cards and loads a
save, or reads a save back, copies the current four trailing records into it
and writes it again. Its data blob holds the card-screen messages, two card
title templates, the location names, the party icons, the card step sequences,
the character chart and the digit glyphs, followed by NIKI's own variables.
The build links that blob unchanged; this tool writes YAML and PNG copies.

``NikiSymbols`` finds every address in the version's symbol file. ``read_blob``
walks the blob in address order with the readers ADDHERO, CARDA and CLOAD share
in ``card_data``. The byte map also covers padding and the zero-filled
variables. JP text decodes through NIKI's own chart.

Example:

    python3 -m tools.overlays.niki --version us assets/exports/us/overlays/niki
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import re
import shutil
import sys
import tempfile

from tools.overlays import card_data, splat_config, symbols, text_table
from tools.overlays.card_data import (
    CardSteps,
    CardSymbols,
    Chart,
    StepSequence,
    read_digit_glyphs,
    read_icons,
    read_text_list,
    variables_start,
)
from tools.overlays.resources import (
    Blob, Part, byte_map_entry, cover_gaps, dump_yaml, hex_address, write_part,
)

REPO_ROOT = Path(__file__).resolve().parents[2]
OVERLAY_CONFIG = "overlays/NIKI.BIN.yaml"
SYMBOL_FILE = "symbols/niki_symbol_addrs.txt"
COMMAND_HEADER = REPO_ROOT / "src/overlays/niki/niki_internal.h"

CARD_TITLES_NOTE = (
    "# Shift-JIS memory card title templates; the save screen (CARDA) writes them. "
    "NIKI keeps the title a save already has, so it only carries a copy.\n"
)
CHART_NOTE = "NIKI includes the conversion helper that reads this chart but never calls it."


# ---------------------------------------------------------------------------
# Inputs


@dataclass(frozen=True)
class NikiSymbols(CardSymbols):
    """Every address the tool needs, named in SYMBOL_NAMES below."""

    @classmethod
    def load(cls, path: Path) -> NikiSymbols:
        named = symbols.load(path)
        missing = [name for name in SYMBOL_NAMES.values() if name not in named]
        if missing:
            raise ValueError(
                f"{path} has no {', '.join(missing)}. If a symbol was renamed, "
                "update SYMBOL_NAMES in tools/overlays/niki.py."
            )
        addresses = {key: named[name] for key, name in SYMBOL_NAMES.items()}
        return cls(**addresses, named=named)


# In blob order, except chart_pages: the decoder's page base lies inside the icon set.
SYMBOL_NAMES = {
    "messages": "g_niki_text_table",
    "locations": "g_niki_location_names",
    "icon_offsets": "g_niki_icon_offsets",
    "card_steps": "g_niki_card_setup_sequence",
    "chart": "g_glyph_single_byte_chart",
    "decimal_glyphs": "g_glyph_decimal_digits",
    "hex_glyphs": "g_glyph_hex_digits",
    "chart_pages": "g_glyph_chart_page_base",
}
BLOB_ORDER = ("messages", "locations", "icon_offsets", "card_steps", "chart", "decimal_glyphs", "hex_glyphs")
MESSAGE_SYMBOL_PREFIX = "g_niki_text_"
LOCATION_SYMBOL_PREFIX = "g_niki_location_names"


@dataclass(frozen=True)
class Inputs:
    """Where one version's NIKI blob and symbols live."""

    version: str
    config: Path
    assets: Path

    @property
    def symbol_file(self) -> Path:
        return self.config / SYMBOL_FILE

    @property
    def overlay_config(self) -> Path:
        return self.config / OVERLAY_CONFIG


@dataclass(frozen=True)
class NikiCardSteps(CardSteps):
    """The step table with its raw bytes and any command no named entry reaches."""

    raw: bytes
    unreached: tuple[dict[str, str], ...]


def card_step_names() -> dict[int, str]:
    """NikiLoadCommand values from the C header, so the names stay in one place."""
    text = COMMAND_HEADER.read_text(encoding="ascii")
    match = re.search(r"typedef enum\s*\{([^}]+)\}\s*NikiLoadCommand;", text)
    if match is None:
        raise ValueError(f"{COMMAND_HEADER} has no NikiLoadCommand")
    return {int(value): name for name, value in re.findall(r"(NIKI_COMMAND_\w+)\s*=\s*(\d+)", match[1])}


# ---------------------------------------------------------------------------
# Reading the blob, in address order


def read_blob(blob: Blob[NikiSymbols]) -> list[Part]:
    """Parse every known part, then fill the gaps so every byte is accounted for.

    The parts, in blob order: the message table, two card title templates, the
    location table, the icon set and its trailing word, the card step table,
    the character chart, the digit glyphs, and NIKI's variables.
    """
    chart = Chart(blob)
    messages = read_messages(blob, chart)
    parts = [messages]
    parts += card_data.read_card_titles(blob, messages.end, blob.offset(blob.symbols.locations), CARD_TITLES_NOTE)
    parts.append(read_locations(blob, chart))
    parts += read_icons(blob)
    parts.append(read_card_steps(blob))
    parts.append(read_chart(chart))
    parts.append(read_digit_glyphs(blob))
    parts.append(read_variables(blob))
    return cover_gaps(blob, sorted(parts, key=lambda part: part.start))


def read_messages(blob: Blob[NikiSymbols], chart: Chart) -> Part:
    """The card-screen message table; NIKI draws about a third of its entries."""
    table = blob.symbols.messages
    content, end = read_text_list(blob, chart, table, MESSAGE_SYMBOL_PREFIX)
    if end > blob.offset(blob.symbols.locations):
        raise ValueError("message table extends into the location table")
    return Part("messages", blob.offset(table), end, "text/messages.yaml", content)


def read_locations(blob: Blob[NikiSymbols], chart: Chart) -> Part:
    """Location names the details panel shows, picked by a save's music track."""
    table = blob.symbols.locations
    content, end = read_text_list(blob, chart, table, LOCATION_SYMBOL_PREFIX)
    if end > blob.offset(blob.symbols.icon_offsets) - 4:
        raise ValueError("location table extends into the icon set")
    return Part("locations", blob.offset(table), end, "text/locations.yaml", content)


def read_card_steps(blob: Blob[NikiSymbols]) -> Part:
    """Follow every sequence symbol in the table to NIKI_COMMAND_STOP.

    Sequences may share a tail. Nonzero bytes that no sequence reaches are
    listed, since the export would otherwise only keep them in the raw bytes.
    """
    names = card_step_names()
    start = blob.offset(blob.symbols.card_steps)
    end = blob.offset(blob.symbols.chart)
    entries = sorted(
        (address, name)
        for name, address in blob.symbols.named.items()
        if blob.symbols.card_steps <= address < blob.symbols.chart
    )
    sequences = []
    reached = set()
    for address, name in entries:
        position = blob.offset(address)
        stop = blob.data.find(b"\x00", position, end)
        if stop < 0:
            raise ValueError(f"{name} has no NIKI_COMMAND_STOP before the character chart")
        raw = blob.data[position : stop + 1]
        reached.update(range(position, stop + 1))
        sequences.append(StepSequence(name, tuple(names.get(value, f"0x{value:02X}") for value in raw), raw))
    unreached = tuple(
        {"address": hex_address(blob.address + offset), "step": names.get(blob.data[offset], f"0x{blob.data[offset]:02X}")}
        for offset in range(start, end)
        if offset not in reached and blob.data[offset]
    )
    content = NikiCardSteps(blob.symbols.card_steps, tuple(sequences), blob.data[start:end], unreached)
    return Part("card steps", start, end, "tables/card_steps.yaml", content)


def read_chart(chart: Chart) -> Part:
    """The chart that turns game text into Shift-JIS, also used to decode JP text here."""
    file = "tables/text_conversion.yaml"
    return Part("text to Shift-JIS table", chart.start, chart.end, file, chart.content(), CHART_NOTE)


def read_variables(blob: Blob[NikiSymbols]) -> Part:
    """NIKI's variables and buffers, from the dialog state to the glyph cache cursor."""
    start, end = variables_start(blob), len(blob.data)
    if any(blob.data[start:]):
        return Part("variables", start, end, "unknown/variables.bin", blob.data[start:])
    note = "NIKI's variables and buffers; all zero on the disc, not exported."
    return Part("variables", start, end, note=note)


# ---------------------------------------------------------------------------
# Writing


@write_part.register
def _write_niki_card_steps(content: NikiCardSteps, path: Path) -> None:
    sequences = [
        {"symbol": sequence.symbol, "steps": list(sequence.steps), "bytes": sequence.raw.hex(" ")}
        for sequence in content.sequences
    ]
    document: dict[str, object] = {"address": hex_address(content.address), "sequences": sequences}
    if content.unreached:
        document["unreached"] = list(content.unreached)
    document["bytes"] = content.raw.hex(" ")
    dump_yaml(path, document)


# ---------------------------------------------------------------------------
# Putting it together


def load_blob(inputs: Inputs) -> Blob[NikiSymbols]:
    """Find the blob through splat and check its size and resource order."""
    names = NikiSymbols.load(inputs.symbol_file)
    files = splat_config.data_files(inputs.overlay_config, inputs.assets)
    source = splat_config.file_containing(files, names.messages, SYMBOL_NAMES["messages"])
    if not source.path.exists():
        raise ValueError(f"{source.path} is missing; run make splat first")
    data = source.path.read_bytes()
    if len(data) != source.end - source.start:
        raise ValueError(f"{source.path} size does not match the splat config; run make splat again")
    previous = source.start - 1
    for key in BLOB_ORDER:
        address = getattr(names, key)
        if not source.contains(address):
            raise ValueError(f"{SYMBOL_NAMES[key]} is outside {source.path}; check the splat config")
        if address <= previous:
            raise ValueError(f"{SYMBOL_NAMES[key]} is out of resource order")
        previous = address
    return Blob(data, source.start, source.path.name, inputs.version, names)


def extract(inputs: Inputs, output: Path) -> None:
    """Read everything first, then write into a temporary folder and move it into place.

    A failure while reading leaves nothing behind, and a failure while writing
    removes the temporary folder, so @p output is either complete or absent.
    """
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
