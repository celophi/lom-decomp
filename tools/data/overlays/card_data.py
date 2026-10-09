"""Text, party icons and byte maps shared by the memory-card overlays.

Each overlay still decides where its parts begin and what they mean. These
readers and writers handle the formats CARDA and ADDHERO have in common.
"""

from __future__ import annotations

import codecs
import re
from dataclasses import dataclass
from pathlib import Path

from tools.data.overlays import icon_set, png, text_table
from tools.data.overlays.resources import (
    Blob, Part, dump_yaml, hex_address, write_part,
)


# Shared C values checked by tests/test_addhero_sources.py.
CARD_MENU_HEADER = Path(__file__).resolve().parents[3] / "include/common/card_menu.h"
CHART_ROW_BYTES = 33  # GLYPH_CHART_ROW_BYTES, include/common/glyph_cache.h
CHART_COLUMNS = 16  # GLYPH_CHART_COLUMNS, include/common/glyph_cache.h
CHART_ROWS_PER_PAGE = 16  # GLYPH_CHART_PAGE_BYTES is 16 rows, include/common/glyph_cache.h
CHART_FIRST_CODE = 0x20  # GLYPH_TEXT_FIRST_PRINTABLE, include/common/glyph_cache.h
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
# Inputs and decoded parts


@dataclass(frozen=True)
class CardSymbols:
    """The addresses both card overlays use, plus their complete symbol map."""

    messages: int
    locations: int
    icon_offsets: int
    card_steps: int
    chart: int
    chart_pages: int
    decimal_glyphs: int
    hex_glyphs: int
    named: dict[str, int]

    def with_prefix(self, prefix: str | tuple[str, ...]) -> dict[str, int]:
        """Symbols whose names start with @p prefix."""
        return {
            name: address for name, address in self.named.items() if name.startswith(prefix)
        }


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
    note: str


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
# Reading


def card_menu_step_names(exchange: bool = False) -> dict[int, str]:
    """Opcode names of the card step tables from card_menu.h.

    Every card menu uses the CARD_MENU_STEP_* opcodes; ADDHERO and NIKI add the
    CARD_MENU_EXCHANGE_STEP_* ones.
    """
    text = CARD_MENU_HEADER.read_text(encoding="ascii")
    prefixes = ["CARD_MENU_STEP_"] + (["CARD_MENU_EXCHANGE_STEP_"] if exchange else [])
    names: dict[int, str] = {}
    for prefix in prefixes:
        names.update({int(value): name for name, value in re.findall(rf"\b({prefix}\w+)\s*=\s*(\d+)", text)})
    return names


def read_text_list(blob: Blob, chart: Chart, table: int, prefix: str) -> tuple[TextList, int]:
    """Read a text table and return it with its end offset.

    An entry gets a symbol name when a symbol with @p prefix points at its slot
    in the offset table.
    """
    start = blob.offset(table)
    parsed = text_table.parse(blob.data, start, text_table.TWO_BYTE_CODES[blob.version])
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


def read_card_titles(blob: Blob, start: int, end: int, note: str) -> list[Part]:
    """The Shift-JIS card title templates between blob offsets @p start and @p end.

    Zero bytes between them are padding. Every range becomes a Part, but only
    the first carries the content, because all titles go into one file.
    """
    ranges = []
    position = start
    while position < end:
        if blob.data[position] == 0:
            position += 1
            continue
        terminator = blob.data.find(b"\x00", position, end)
        if terminator < 0:
            raise ValueError(f"card title at offset 0x{position:X} has no terminator")
        ranges.append((position, terminator + 1))
        position = terminator + 1
    if len(ranges) != len(CARD_TITLE_NOTES):
        raise ValueError(
            f"expected {len(CARD_TITLE_NOTES)} card titles before the location table, "
            f"found {len(ranges)}"
        )
    titles = tuple(
        CardTitle(blob.address + first, blob.data[first : last - 1].decode("shift_jis"), title_note)
        for (first, last), title_note in zip(ranges, CARD_TITLE_NOTES)
    )
    parts = []
    for index, (first, last) in enumerate(ranges):
        content = CardTitles(titles, note) if index == 0 else None
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
    """The variables start at the first symbol after the hex digit glyphs."""
    blob_end = blob.address + len(blob.data)
    later = [
        address
        for address in blob.symbols.named.values()
        if blob.symbols.hex_glyphs < address < blob_end
    ]
    return blob.offset(min(later))


# ---------------------------------------------------------------------------
# The character chart


def _braces(error: UnicodeDecodeError) -> tuple[str, int]:
    """Decoding error handler: write undecodable bytes as {XX}."""
    bad = error.object[error.start : error.end]
    return "".join(f"{{{byte:02X}}}" for byte in bad), error.end


def sjis_pairs(raw: bytes) -> tuple[str, ...]:
    """Shift-JIS characters, two bytes each, up to a zero pair."""
    glyphs = []
    for index in range(0, len(raw) - 1, 2):
        pair = raw[index : index + 2]
        if pair == b"\x00\x00":
            break
        glyphs.append(pair.decode("shift_jis", errors="replace"))
    return tuple(glyphs)


codecs.register_error("braces", _braces)


class Chart:
    """The text-to-Shift-JIS chart, read the way expand_text_glyph_codes reads it.

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
            if code in text_table.TWO_BYTE_CODES[self.blob.version] and index + 1 < len(raw):
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
# Writing


def icon_group(index: int) -> str:
    """What kind of character a save-screen party icon id shows."""
    if index < ICON_HERO_COUNT:
        return "hero"
    if index >= ICON_GOLEM_BASE:
        return "golem"
    if index >= ICON_PET_BASE:
        return "pet"
    return "character"


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
    dump_yaml(path, {"titles": titles}, content.note)


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
