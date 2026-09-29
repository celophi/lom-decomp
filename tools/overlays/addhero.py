#!/usr/bin/env python3
"""Export ADDHERO's data as files people can read.

ADDHERO keeps its text, party icons and small tables in one data blob, and two
short strings in separate rodata files. The build links those bytes unchanged.
This tool reads them and writes YAML and PNG files plus a byte map; the build
never reads anything back from here.

How the code is organised:

* ``AddheroSymbols`` names every symbol the tool relies on. The addresses come
  from the version's symbol file, so a renamed symbol is fixed in one place.
* ``read_blob`` walks the blob in address order, one ``read_*`` function per
  part. Each returns a ``Part``: where the bytes are and what they hold.
* ``cover_gaps`` adds whatever lies between the parts, so the byte map
  accounts for every byte of the blob.
* ``write_part`` turns each kind of content into its file. It lives in
  ``card_data`` with the format readers CARDA uses too.

Example:

    python3 -m tools.overlays.addhero --version us assets/exports/us/overlays/addhero
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import re
import shutil
import sys
import tempfile

from tools.overlays import splat_config, symbols, text_table
from tools.overlays.card_data import (
    Blob,
    CARD_TITLE_NOTES,
    CardSteps,
    CardSymbols,
    CardTitle,
    CardTitles,
    Chart,
    Part,
    StepSequence,
    byte_map_entry,
    cover_gaps,
    dump_yaml,
    hex_address,
    read_digit_glyphs,
    read_icons,
    read_text_list,
    variables_start,
    write_part,
)

REPO_ROOT = Path(__file__).resolve().parents[2]
OVERLAY_CONFIG = "overlays/ADDHERO.BIN.yaml"
SYMBOL_FILE = "symbols/addhero_symbol_addrs.txt"
STEP_HEADER = REPO_ROOT / "src/overlays/addhero/addhero_internal.h"

CARD_TITLES_NOTE = (
    "# Shift-JIS memory card title templates; the save screen (CARDA) writes them, "
    "ADDHERO only carries a copy.\n"
)


# ---------------------------------------------------------------------------
# Inputs


@dataclass(frozen=True)
class AddheroSymbols(CardSymbols):
    """Every address the tool needs, looked up by name in the symbol file.

    SYMBOL_NAMES below gives the symbol for each field. When a symbol is renamed
    in the config, change it there; ``load`` reports any name it can't find.
    """

    overflow_text: int  # text shown for a number above 999999
    file_template: int  # memory card device prefix
    directory_pattern: int  # memory card directory search pattern

    @classmethod
    def load(cls, path: Path) -> AddheroSymbols:
        named = symbols.load(path)
        missing = [name for name in SYMBOL_NAMES.values() if name not in named]
        if missing:
            raise ValueError(
                f"{path} has no {', '.join(missing)}. If a symbol was renamed, "
                "update SYMBOL_NAMES in tools/overlays/addhero.py."
            )
        addresses = {key: named[name] for key, name in SYMBOL_NAMES.items()}
        return cls(**addresses, named=named)


SYMBOL_NAMES = {
    "messages": "g_addhero_text_table",
    "locations": "g_addhero_location_text_table",
    "icon_offsets": "g_addhero_icon_image_table",
    "card_steps": "g_addhero_loadseq_start",
    "chart": "g_glyph_single_byte_chart",
    "chart_pages": "g_glyph_chart_page_base",
    "decimal_glyphs": "g_glyph_decimal_digits",
    "hex_glyphs": "g_glyph_hex_digits",
    "overflow_text": "g_decimal_overflow_text",
    "file_template": "g_addhero_file_template",
    "directory_pattern": "g_addhero_entry_header_template",
}
MESSAGE_SYMBOL_PREFIX = "g_addhero_text_"
LOCATION_SYMBOL_PREFIX = "g_addhero_location_text_table"
# The idle table has the shared name every card overlay uses (include/card_events.h).
CARD_STEP_SYMBOL_PREFIX = ("g_addhero_loadseq_", "g_card_steps_")


@dataclass(frozen=True)
class Inputs:
    """Where one version's ADDHERO data and symbols live."""

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
    """AddheroCardStep values from the C header, so the names stay in one place."""
    text = STEP_HEADER.read_text(encoding="ascii")
    pairs = re.findall(r"\b(ADDHERO_STEP_\w+)\s*=\s*(\d+)", text)
    return {int(value): name for name, value in pairs}


# ---------------------------------------------------------------------------
# Reading the blob, in address order


def read_blob(blob: Blob[AddheroSymbols]) -> list[Part]:
    """Parse every known part, then fill the gaps so every byte is accounted for.

    The parts, in the order they sit in the blob: the message table, two card
    title templates, the location table, the icon set and its trailing word,
    the card step sequences, the character chart, the digit glyphs, and
    ADDHERO's own variables.
    """
    chart = Chart(blob)
    messages = read_messages(blob, chart)
    parts = [messages]
    parts += read_card_titles(blob, messages.end)
    parts.append(read_locations(blob, chart))
    parts += read_icons(blob)
    parts.append(read_card_steps(blob))
    parts.append(read_chart(chart))
    parts.append(read_digit_glyphs(blob))
    parts.append(read_variables(blob))
    return cover_gaps(blob, sorted(parts, key=lambda part: part.start))


def read_messages(blob: Blob[AddheroSymbols], chart: Chart) -> Part:
    """The message table: every string on the 2P screens."""
    table = blob.symbols.messages
    content, end = read_text_list(blob, chart, table, MESSAGE_SYMBOL_PREFIX)
    return Part("messages", blob.offset(table), end, "text/messages.yaml", content)


def read_locations(blob: Blob[AddheroSymbols], chart: Chart) -> Part:
    """The location names the save list shows, picked by a save's music track."""
    table = blob.symbols.locations
    content, end = read_text_list(blob, chart, table, LOCATION_SYMBOL_PREFIX)
    return Part("locations", blob.offset(table), end, "text/locations.yaml", content)


def read_card_titles(blob: Blob[AddheroSymbols], start: int) -> list[Part]:
    """The two Shift-JIS card title templates between the message and location tables."""
    end = blob.offset(blob.symbols.locations)
    ranges = []
    position = start
    while position < end:
        if blob.data[position] == 0:
            position += 1
            continue
        terminator = blob.data.index(b"\x00", position)
        ranges.append((position, terminator + 1))
        position = terminator + 1
    if len(ranges) != len(CARD_TITLE_NOTES):
        raise ValueError(
            f"expected {len(CARD_TITLE_NOTES)} card titles before the location table, "
            f"found {len(ranges)}"
        )
    titles = tuple(
        CardTitle(blob.address + first, blob.data[first : last - 1].decode("shift_jis"), note)
        for (first, last), note in zip(ranges, CARD_TITLE_NOTES)
    )
    # Both ranges go into one file, so only the first part carries the content.
    parts = []
    for index, (first, last) in enumerate(ranges):
        content = CardTitles(titles, CARD_TITLES_NOTE) if index == 0 else None
        parts.append(Part("card title template", first, last, "text/card_titles.yaml", content))
    return parts


def read_card_steps(blob: Blob[AddheroSymbols]) -> Part:
    """The card step sequences, each running to the next; the last runs to the chart."""
    names = card_step_names()
    found = blob.symbols.with_prefix(CARD_STEP_SYMBOL_PREFIX)
    starts = sorted((address, name) for name, address in found.items())
    ends = [address for address, _ in starts[1:]] + [blob.symbols.chart]
    sequences = []
    for (address, name), end in zip(starts, ends):
        raw = blob.data[blob.offset(address) : blob.offset(end)]
        steps = []
        for value in raw:
            steps.append(names.get(value, f"0x{value:02X}"))
            if value == 0:  # ADDHERO_STEP_DONE; any bytes after it are padding
                break
        sequences.append(StepSequence(name, tuple(steps), raw))
    first = starts[0][0]
    content = CardSteps(first, tuple(sequences))
    start, end = blob.offset(first), blob.offset(blob.symbols.chart)
    return Part("card steps", start, end, "tables/card_steps.yaml", content)


def read_chart(chart: Chart) -> Part:
    """The chart that turns game text into Shift-JIS for memory card titles."""
    note = "Used by a conversion helper that ADDHERO never calls."
    file = "tables/text_conversion.yaml"
    return Part("text to Shift-JIS table", chart.start, chart.end, file, chart.content(), note)


def read_variables(blob: Blob[AddheroSymbols]) -> Part:
    """ADDHERO's own variables and buffers, which are all zero on the disc."""
    start, end = variables_start(blob), len(blob.data)
    if any(blob.data[start:]):
        return Part("variables", start, end, "unknown/variables.bin", blob.data[start:])
    note = "ADDHERO's variables and buffers; all zero on the disc, not exported."
    return Part("variables", start, end, note=note)


# ---------------------------------------------------------------------------
# The two small rodata files


def read_fixed_strings(
    files: list[splat_config.DataFile], names: AddheroSymbols
) -> tuple[list[dict[str, str]], list[dict[str, str]]]:
    """The number overflow text and the two memory card path templates.

    Returns the strings to write and the byte map entries for the files they
    came from. Each file is found through the splat config by the address of
    the symbol inside it.
    """
    wanted = (
        ("overflow_text", "shift_jis", "Shown instead of a number above 999999."),
        ("file_template", "ascii", "Memory card device; the slot digit is patched before use."),
        ("directory_pattern", "ascii", "Directory search pattern for the whole card."),
    )
    strings = []
    sources: dict[str, splat_config.DataFile] = {}
    for key, encoding, note in wanted:
        address = getattr(names, key)
        source = splat_config.file_containing(files, address, SYMBOL_NAMES[key])
        data = source.path.read_bytes()
        start = address - source.start
        text = data[start : data.index(b"\x00", start)].decode(encoding)
        strings.append(
            {
                "symbol": SYMBOL_NAMES[key],
                "address": hex_address(address),
                "text": text,
                "note": note,
            }
        )
        sources[source.name] = source
    byte_map = [
        {
            "file": source.path.name,
            "address": hex_address(source.start),
            "size": f"0x{source.end - source.start:X}",
            "exported": "text/fixed_strings.yaml",
        }
        for source in sources.values()
    ]
    return strings, byte_map


# ---------------------------------------------------------------------------
# Putting it together


def load_blob(inputs: Inputs) -> tuple[Blob[AddheroSymbols], list[splat_config.DataFile]]:
    """Find the blob through the splat config: the data file holding the message table."""
    names = AddheroSymbols.load(inputs.symbol_file)
    files = splat_config.data_files(inputs.overlay_config, inputs.assets)
    source = splat_config.file_containing(files, names.messages, SYMBOL_NAMES["messages"])
    if not source.path.exists():
        raise ValueError(f"{source.path} is missing; run make splat first")
    blob = Blob(source.path.read_bytes(), source.start, source.path.name, inputs.version, names)
    return blob, files


def extract(inputs: Inputs, output: Path) -> None:
    """Read everything first, then write into a temporary folder and move it into place.

    A failure while reading leaves nothing behind, and a failure while writing
    removes the temporary folder, so @p output is either complete or absent.
    """
    if output.exists():
        raise FileExistsError(f"{output} already exists")
    blob, files = load_blob(inputs)
    parts = read_blob(blob)
    strings, other_files = read_fixed_strings(files, blob.symbols)
    byte_map = {
        "source": blob.file_name,
        "address": hex_address(blob.address),
        "size": f"0x{len(blob.data):X}",
        "ranges": [byte_map_entry(blob, part) for part in parts],
        "other_files": other_files,
    }

    output.parent.mkdir(parents=True, exist_ok=True)
    staging = Path(tempfile.mkdtemp(prefix=f".{output.name}-", dir=output.parent))
    try:
        for part in parts:
            if part.content is not None:
                write_part(part.content, staging / part.file)
        dump_yaml(staging / "text/fixed_strings.yaml", {"strings": strings})
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
