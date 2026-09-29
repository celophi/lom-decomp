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
* ``write_part`` turns each kind of content into its file.

Example:

    python3 -m tools.overlays.addhero --version us assets/exports/us/overlays/addhero
"""

from __future__ import annotations

import argparse
import codecs
from dataclasses import dataclass
from functools import singledispatch
from pathlib import Path
import re
import shutil
import sys
import tempfile
from typing import NamedTuple

import yaml

from tools.overlays import icon_set, png, splat_config, symbols, text_table

REPO_ROOT = Path(__file__).resolve().parents[2]
OVERLAY_CONFIG = "overlays/ADDHERO.BIN.yaml"
SYMBOL_FILE = "symbols/addhero_symbol_addrs.txt"
STEP_HEADER = REPO_ROOT / "src/overlays/addhero/addhero_internal.h"

# Values copied from the C sources. tests/test_addhero_sources.py checks each
# one against its #define, so a change in C shows up as a failing test.
CHART_ROW_BYTES = 33  # GLYPH_CHART_ROW_BYTES, include/glyph_cache.h
CHART_COLUMNS = 16  # GLYPH_CHART_COLUMNS, include/glyph_cache.h
CHART_ROWS_PER_PAGE = 16  # GLYPH_CHART_PAGE_BYTES is 16 rows, include/glyph_cache.h
CHART_FIRST_CODE = 0x20  # GLYPH_TEXT_FIRST_PRINTABLE, include/glyph_cache.h
ICON_HERO_COUNT = 2  # SAVE_ICON_HERO_COUNT, saved_game.h
ICON_PET_BASE = 0x0E  # SAVE_ICON_PET_BASE, saved_game.h
ICON_GOLEM_BASE = 0x4F  # SAVE_ICON_GOLEM_BASE, saved_game.h

CHART_PAGE_BYTES = CHART_ROWS_PER_PAGE * CHART_ROW_BYTES
CHART_LEADS = range(0x19, 0x20)  # lead bytes of the chart's seven two-byte pages

TEXT_NOTES = {
    "us": "# Codes in braces are dictionary word pieces and control bytes, "
    'like {16} for "in".\n',
    "jp": "# Decoded with the game's character chart (tables/text_conversion.yaml); "
    "codes in braces are control bytes.\n",
}
CARD_TITLES_NOTE = (
    "# Shift-JIS memory card title templates; the save screen (CARDA) writes them, "
    "ADDHERO only carries a copy.\n"
)
CARD_TITLE_NOTES = (
    "The title of a finished save. A note sign replaces the second dash when option bit 2 "
    "is set, the same flag that puts + in the file name.",
    "The title a save is written with. Saving puts the real title back as its last step, "
    "so a save still showing this one was interrupted.",
)
CHART_NOTE = (
    "# How the game turns its text codes into Shift-JIS for memory card titles; "
    "each row holds 16 codes.\n"
)
ICONS_NOTE = (
    "Icon ids are the save-screen party icons. At runtime the guest hero and golems are "
    "drawn with palettes the game builds, not the ones stored here."
)


# ---------------------------------------------------------------------------
# Inputs


class AddheroSymbols(NamedTuple):
    """Every address the tool needs, looked up by name in the symbol file.

    SYMBOL_NAMES below gives the symbol for each field. When a symbol is renamed
    in the config, change it there; ``load`` reports any name it can't find.
    """

    messages: int  # message text table (its first entry)
    locations: int  # location name text table
    icon_offsets: int  # icon set offset table; the icon count is the word before it
    card_steps: int  # first card step sequence; the others follow it
    chart: int  # text-to-Shift-JIS chart, one-byte part
    chart_pages: int  # base the chart's two-byte pages are reached from
    decimal_glyphs: int  # full-width digits 0-9
    hex_glyphs: int  # full-width digits 0-F
    overflow_text: int  # text shown for a number above 999999
    file_template: int  # memory card device prefix
    directory_pattern: int  # memory card directory search pattern
    named: dict[str, int]  # the whole symbol file, for per-entry names

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

    def with_prefix(self, prefix: str | tuple[str, ...]) -> dict[str, int]:
        """Symbols whose names start with @p prefix, such as each card step sequence."""
        return {
            name: address for name, address in self.named.items() if name.startswith(prefix)
        }


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


@dataclass(frozen=True)
class Blob:
    """The data blob and where it sits in memory."""

    data: bytes
    address: int  # memory address of data[0]
    file_name: str
    version: str
    symbols: AddheroSymbols

    def offset(self, address: int) -> int:
        """Blob offset of a memory address."""
        return address - self.address

    @property
    def two_byte_codes(self) -> frozenset[int]:
        return text_table.TWO_BYTE_CODES[self.version]


def card_step_names() -> dict[int, str]:
    """AddheroCardStep values from the C header, so the names stay in one place."""
    text = STEP_HEADER.read_text(encoding="ascii")
    pairs = re.findall(r"\b(ADDHERO_STEP_\w+)\s*=\s*(\d+)", text)
    return {int(value): name for name, value in pairs}


# ---------------------------------------------------------------------------
# The parts of the blob, and what each one holds


@dataclass(frozen=True)
class Part:
    """One range of the blob; end is exclusive.

    content is what gets written for the part, or None when the byte map entry
    and its note are all there is. file is where the content goes.
    """

    name: str
    start: int
    end: int
    file: str | None = None
    content: object | None = None
    note: str | None = None


@dataclass(frozen=True)
class TextEntry:
    index: int
    symbol: str | None
    text: str
    raw: bytes


@dataclass(frozen=True)
class TextList:
    """A text table: its address and every entry, decoded."""

    address: int
    entries: tuple[TextEntry, ...]
    note: str


@dataclass(frozen=True)
class CardTitle:
    address: int
    text: str
    note: str


@dataclass(frozen=True)
class CardTitles:
    titles: tuple[CardTitle, ...]


@dataclass(frozen=True)
class Icons:
    address: int
    icons: icon_set.IconSet


@dataclass(frozen=True)
class StepSequence:
    symbol: str
    steps: tuple[str, ...]
    raw: bytes


@dataclass(frozen=True)
class CardSteps:
    address: int
    sequences: tuple[StepSequence, ...]


@dataclass(frozen=True)
class CharacterChart:
    """The text-to-Shift-JIS chart, as grids of 16 codes per row."""

    address: int
    one_byte: dict[str, str]
    two_byte: dict[str, dict[str, str]]
    lines: tuple[str, ...]


@dataclass(frozen=True)
class DigitGlyphs:
    decimal_address: int
    decimal: tuple[str, ...]
    hex_address: int
    hexadecimal: tuple[str, ...]


# ---------------------------------------------------------------------------
# Reading the blob, in address order


def read_blob(blob: Blob) -> list[Part]:
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


def read_text_list(blob: Blob, chart: Chart, table: int, prefix: str) -> tuple[TextList, int]:
    """Read a text table and return it with its end offset.

    An entry gets a symbol name when a symbol with @p prefix points at its slot
    in the offset table.
    """
    start = blob.offset(table)
    parsed = text_table.parse(blob.data, start, blob.two_byte_codes)
    slots_end = table + parsed.entries[0].offset
    names = {
        (address - table) // 2: name
        for name, address in blob.symbols.with_prefix(prefix).items()
        if table <= address < slots_end
    }
    entries = []
    for entry in parsed.entries:
        text = chart.decode(entry.data) if blob.version == "jp" else entry.text
        entries.append(TextEntry(entry.index, names.get(entry.index), text, entry.data))
    return TextList(table, tuple(entries), TEXT_NOTES[blob.version]), start + parsed.size


def read_messages(blob: Blob, chart: Chart) -> Part:
    """The message table: every string on the 2P screens."""
    table = blob.symbols.messages
    content, end = read_text_list(blob, chart, table, MESSAGE_SYMBOL_PREFIX)
    return Part("messages", blob.offset(table), end, "text/messages.yaml", content)


def read_locations(blob: Blob, chart: Chart) -> Part:
    """The location names the save list shows, picked by a save's music track."""
    table = blob.symbols.locations
    content, end = read_text_list(blob, chart, table, LOCATION_SYMBOL_PREFIX)
    return Part("locations", blob.offset(table), end, "text/locations.yaml", content)


def read_card_titles(blob: Blob, start: int) -> list[Part]:
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
        content = CardTitles(titles) if index == 0 else None
        parts.append(Part("card title template", first, last, "text/card_titles.yaml", content))
    return parts


def read_icons(blob: Blob) -> list[Part]:
    """The party icon set, plus the repeated last word that follows it."""
    start = blob.offset(blob.symbols.icon_offsets) - 4  # the icon count
    icons = icon_set.parse(blob.data, start)
    end = start + icons.size
    content = Icons(blob.address + start, icons)
    parts = [Part("icons", start, end, "icons/icons.yaml", content)]
    if blob.data[end : end + 4] == blob.data[end - 4 : end]:
        note = (
            "Repeats the icon set's last word, like the trailing word after some "
            "embedded TIMs."
        )
        parts.append(Part("icon set trailing word", end, end + 4, note=note))
    return parts


def read_card_steps(blob: Blob) -> Part:
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


def read_digit_glyphs(blob: Blob) -> Part:
    """Full-width digits for save numbers: 0-9, then 0-F, each ended by a zero pair."""
    decimal = blob.offset(blob.symbols.decimal_glyphs)
    hexadecimal = blob.offset(blob.symbols.hex_glyphs)
    end = variables_start(blob)
    content = DigitGlyphs(
        blob.symbols.decimal_glyphs,
        sjis_pairs(blob.data[decimal:hexadecimal]),
        blob.symbols.hex_glyphs,
        sjis_pairs(blob.data[hexadecimal:end]),
    )
    return Part("digit glyphs", decimal, end, "tables/digit_glyphs.yaml", content)


def variables_start(blob: Blob) -> int:
    """ADDHERO's variables start at the first symbol after the hex digit glyphs."""
    blob_end = blob.address + len(blob.data)
    later = [
        address
        for address in blob.symbols.named.values()
        if blob.symbols.hex_glyphs < address < blob_end
    ]
    return blob.offset(min(later))


def read_variables(blob: Blob) -> Part:
    """ADDHERO's own variables and buffers, which are all zero on the disc."""
    start, end = variables_start(blob), len(blob.data)
    if any(blob.data[start:]):
        return Part("variables", start, end, "unknown/variables.bin", blob.data[start:])
    note = "ADDHERO's variables and buffers; all zero on the disc, not exported."
    return Part("variables", start, end, note=note)


def cover_gaps(blob: Blob, parts: list[Part]) -> list[Part]:
    """Add the bytes between parts: zero padding, or unknown data saved as-is."""
    result = []
    cursor = 0
    for part in parts + [Part("end", len(blob.data), len(blob.data))]:
        if part.start < cursor:
            raise ValueError(f"{part.name} overlaps the part before it")
        if cursor < part.start:
            raw = blob.data[cursor : part.start]
            if any(raw):
                file = f"unknown/{blob.address + cursor:08X}.bin"
                result.append(Part("unknown", cursor, part.start, file, raw))
            else:
                result.append(Part("padding", cursor, part.start, note="zero bytes"))
        if part.name != "end":
            result.append(part)
        cursor = part.end
    return result


# ---------------------------------------------------------------------------
# The character chart


def _braces(error: UnicodeDecodeError) -> tuple[str, int]:
    """Decoding error handler: write undecodable bytes as {XX}."""
    bad = error.object[error.start : error.end]
    return "".join(f"{{{byte:02X}}}" for byte in bad), error.end


codecs.register_error("braces", _braces)


def sjis_pairs(raw: bytes) -> tuple[str, ...]:
    """Shift-JIS characters, two bytes each, up to a zero pair."""
    glyphs = []
    for index in range(0, len(raw) - 1, 2):
        pair = raw[index : index + 2]
        if pair == b"\x00\x00":
            break
        glyphs.append(pair.decode("shift_jis", errors="replace"))
    return tuple(glyphs)


class Chart:
    """The text-to-Shift-JIS chart, read the way addhero_expand_text_glyph_codes reads it.

    Rows are 33 bytes: 16 two-byte characters and a newline. A one-byte code c
    sits at row (c - 0x20) / 16, column (c - 0x20) % 16. A two-byte code
    (lead, second) sits on page lead at row second >> 4, column second & 0xF,
    with pages counted from the chart_pages symbol.
    """

    def __init__(self, blob: Blob):
        self.blob = blob
        self.start = blob.offset(blob.symbols.chart)
        self.end = blob.offset(blob.symbols.decimal_glyphs)
        self.pages = blob.offset(blob.symbols.chart_pages)

    def one_byte_offset(self, code: int) -> int:
        row, column = divmod(code - CHART_FIRST_CODE, CHART_COLUMNS)
        return self.start + row * CHART_ROW_BYTES + column * 2

    def two_byte_offset(self, lead: int, second: int) -> int:
        row, column = second >> 4, second & 0x0F
        return self.pages + lead * CHART_PAGE_BYTES + row * CHART_ROW_BYTES + column * 2

    def cell(self, offset: int) -> str | None:
        """The character at @p offset.

        Two bytes that aren't one Shift-JIS character come back as {XX YY}; an
        offset outside the chart comes back as None.
        """
        if not (self.start <= offset and offset + 2 <= self.end):
            return None
        raw = self.blob.data[offset : offset + 2]
        if 0x81 <= raw[0] <= 0x9F or 0xE0 <= raw[0] <= 0xFC:
            try:
                return raw.decode("shift_jis")
            except UnicodeDecodeError:
                pass
        return f"{{{raw[0]:02X} {raw[1]:02X}}}"

    def decode(self, raw: bytes) -> str:
        """Decode game text through the chart, as the Japanese release stores it."""
        out = []
        index = 0
        while index < len(raw):
            code = raw[index]
            if code in self.blob.two_byte_codes and index + 1 < len(raw):
                offset = self.two_byte_offset(code, raw[index + 1])
                index += 2
            elif code >= CHART_FIRST_CODE:
                offset = self.one_byte_offset(code)
                index += 1
            else:
                out.append(f"{{{code:02X}}}")
                index += 1
                continue
            out.append(self.cell(offset) or "{??}")
        return "".join(out)

    def row(self, offsets: list[int]) -> str | None:
        """One grid row, or None when any of its cells falls outside the chart."""
        cells = [self.cell(offset) for offset in offsets]
        if any(cell is None for cell in cells):
            return None
        return "".join(cells)

    def content(self) -> CharacterChart:
        one_byte = {}
        for high in range(CHART_FIRST_CODE >> 4, 0x10):
            codes = [(high << 4) + column for column in range(CHART_COLUMNS)]
            row = self.row([self.one_byte_offset(code) for code in codes])
            if row is not None:
                one_byte[f"0x{high:X}_"] = row
        two_byte = {}
        for lead in CHART_LEADS:
            rows = {}
            for high in range(CHART_ROWS_PER_PAGE):
                seconds = [(high << 4) + column for column in range(CHART_COLUMNS)]
                row = self.row([self.two_byte_offset(lead, second) for second in seconds])
                if row is not None:
                    rows[f"0x{high:X}_"] = row
            if rows:
                two_byte[f"0x{lead:02X}"] = rows
        stored = self.blob.data[self.start : self.end].rstrip(b"\x00")
        lines = tuple(line.decode("shift_jis", errors="braces") for line in stored.split(b"\n"))
        return CharacterChart(self.blob.address + self.start, one_byte, two_byte, lines)


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
# Writing


def hex_address(address: int) -> str:
    return f"0x{address:08X}"


def dump_yaml(path: Path, document: object, header: str = "") -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    body = yaml.safe_dump(document, sort_keys=False, allow_unicode=True, width=120)
    path.write_text(header + body, encoding="utf-8")


def icon_group(index: int) -> str:
    """What kind of character a save-screen party icon id shows."""
    if index < ICON_HERO_COUNT:
        return "hero"
    if index >= ICON_GOLEM_BASE:
        return "golem"
    if index >= ICON_PET_BASE:
        return "pet"
    return "character"


@singledispatch
def write_part(content: object, path: Path) -> None:
    """Write one part's content to @p path. Each content type registers its own writer."""
    raise TypeError(f"no writer for {type(content).__name__}")


@write_part.register
def _write_bytes(content: bytes, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(content)


@write_part.register
def _write_text_list(content: TextList, path: Path) -> None:
    entries = []
    for entry in content.entries:
        item: dict[str, object] = {"index": entry.index}
        if entry.symbol:
            item["symbol"] = entry.symbol
        item["text"] = entry.text
        item["bytes"] = entry.raw.hex(" ")
        entries.append(item)
    document = {"address": hex_address(content.address), "entries": entries}
    dump_yaml(path, document, content.note)


@write_part.register
def _write_card_titles(content: CardTitles, path: Path) -> None:
    titles = [
        {"address": hex_address(title.address), "text": title.text, "note": title.note}
        for title in content.titles
    ]
    dump_yaml(path, {"titles": titles}, CARD_TITLES_NOTE)


@write_part.register
def _write_icons(content: Icons, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    listing = []
    for icon in content.icons.icons:
        file_name = f"icon_{icon.index:02X}.png"
        size = icon_set.ICON_SIZE
        png.write_rgba(path.parent / file_name, size, size, icon.rgba())
        listing.append(
            {
                "icon": f"0x{icon.index:02X}",
                "group": icon_group(icon.index),
                "file": file_name,
                "offset": f"0x{icon.offset:X}",
                "palette": [f"0x{value:04X}" for value in icon.palette],
            }
        )
    document = {"address": hex_address(content.address), "note": ICONS_NOTE, "icons": listing}
    dump_yaml(path, document)


@write_part.register
def _write_card_steps(content: CardSteps, path: Path) -> None:
    sequences = [
        {"symbol": sequence.symbol, "steps": list(sequence.steps), "bytes": sequence.raw.hex(" ")}
        for sequence in content.sequences
    ]
    dump_yaml(path, {"address": hex_address(content.address), "sequences": sequences})


@write_part.register
def _write_chart(content: CharacterChart, path: Path) -> None:
    document = {
        "address": hex_address(content.address),
        "one_byte": content.one_byte,
        "two_byte": content.two_byte,
        "chart_lines": list(content.lines),
    }
    dump_yaml(path, document, CHART_NOTE)


@write_part.register
def _write_digit_glyphs(content: DigitGlyphs, path: Path) -> None:
    document = {
        "decimal": {
            "address": hex_address(content.decimal_address),
            "glyphs": list(content.decimal),
        },
        "hexadecimal": {
            "address": hex_address(content.hex_address),
            "glyphs": list(content.hexadecimal),
        },
    }
    dump_yaml(path, document)


def byte_map_entry(blob: Blob, part: Part) -> dict[str, str]:
    entry = {
        "name": part.name,
        "address": hex_address(blob.address + part.start),
        "offset": f"0x{part.start:X}",
        "size": f"0x{part.end - part.start:X}",
    }
    if part.file:
        entry["file"] = part.file
    if part.note:
        entry["note"] = part.note
    return entry


# ---------------------------------------------------------------------------
# Putting it together


def load_blob(inputs: Inputs) -> tuple[Blob, list[splat_config.DataFile]]:
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
