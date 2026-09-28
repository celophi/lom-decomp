#!/usr/bin/env python3
"""Export the resources in ADDHERO's data blob as files people can look at.

Reads the addhero_data databin that splat extracts and the version's ADDHERO
symbol file, and writes the text tables, the party icons (as PNG), the card
step sequences and the digit glyph tables. The build keeps using the databin;
nothing here is read back.

    python3 -m tools.overlays.addhero --version us assets/us/addhero_data.databin.bin \\
        config/us/symbols/addhero_symbol_addrs.txt assets/exports/us/overlays/addhero
"""

from __future__ import annotations

import argparse
import codecs
import re
import sys
from pathlib import Path

import yaml

from tools.overlays import icon_set, png, symbols, text_table

REPO_ROOT = Path(__file__).resolve().parents[2]
STEP_HEADER = REPO_ROOT / "src" / "overlays" / "addhero" / "addhero_internal.h"

# Save-screen icon ids (saved_game.h: SAVE_ICON_HERO_COUNT, _PET_BASE, _GOLEM_BASE).
ICON_HERO_COUNT = 2
ICON_PET_BASE = 0x0E
ICON_GOLEM_BASE = 0x4F


# Comment written at the top of the text exports, per version.
TEXT_NOTES = {
    "us": '# Codes in braces are dictionary word pieces and control bytes, like {16} for "in".\n',
    "jp": "# Decoded with the game's character chart (tables/text_conversion.yaml); codes in braces are control bytes.\n",
}


def _dump(path: Path, document: object, header: str = "") -> None:
    body = yaml.safe_dump(document, sort_keys=False, allow_unicode=True, width=120)
    path.write_text(header + body, encoding="utf-8")


def _step_names() -> dict[int, str]:
    """AddheroCardStep values from the C header, so the names stay in one place."""
    text = STEP_HEADER.read_text(encoding="ascii")
    return {int(value): name for name, value in re.findall(r"\b(ADDHERO_STEP_\w+)\s*=\s*(\d+)", text)}


def _icon_group(index: int) -> str:
    if index < ICON_HERO_COUNT:
        return "hero"
    if index >= ICON_GOLEM_BASE:
        return "golem"
    if index >= ICON_PET_BASE:
        return "pet"
    return "character"


class Blob:
    """The databin plus the symbol addresses that locate things inside it."""

    def chart_text(self, raw: bytes) -> str:
        """Decode text through the game's character chart, as the Japanese release stores it."""
        single = self.offset("g_addhero_single_byte_char_table")
        pages = self.offset("g_addhero_char_page_base")
        out = []
        index = 0
        while index < len(raw):
            code = raw[index]
            if code in self.two_byte_codes and index + 1 < len(raw):
                second = raw[index + 1]
                out.append(self.chart_cell(pages + code * CHART_PAGE_BYTES + (second >> 4) * CHART_ROW_BYTES + (second & 0x0F) * 2))
                index += 2
            elif code >= CHART_FIRST_CODE:
                position = code - CHART_FIRST_CODE
                out.append(self.chart_cell(single + (position // 16) * CHART_ROW_BYTES + (position % 16) * 2))
                index += 1
            else:
                out.append(f"{{{code:02X}}}")
                index += 1
        return "".join(out)

    def chart_cell(self, offset: int) -> str:
        raw = self.data[offset : offset + 2]
        try:
            return raw.decode("shift_jis")
        except UnicodeDecodeError:
            return f"{{{raw[0]:02X} {raw[1]:02X}}}"

    def __init__(self, data: bytes, symbol_map: dict[str, int], version: str):
        self.data = data
        self.symbols = symbol_map
        self.version = version
        self.two_byte_codes = text_table.TWO_BYTE_CODES[version]
        self.base = symbol_map["g_addhero_text_table"]
        self.end = self.base + len(data)

    def offset(self, name: str) -> int:
        return self.symbols[name] - self.base

    def next_symbol_after(self, name: str) -> int:
        """Offset of the first symbol above @p name that lies inside the blob."""
        address = self.symbols[name]
        later = [value for value in self.symbols.values() if address < value < self.end]
        return min(later) - self.base


def export_text(blob: Blob, name: str, prefix: str, path: Path) -> tuple[int, int]:
    start = blob.offset(name)
    table = text_table.parse(blob.data, start, blob.two_byte_codes)
    names = {
        (address - blob.symbols[name]) // 2: symbol
        for symbol, address in blob.symbols.items()
        if symbol.startswith(prefix) and blob.symbols[name] <= address < blob.symbols[name] + table.entries[0].offset
    }
    entries = []
    for entry in table.entries:
        item = {"index": entry.index}
        if entry.index in names:
            item["symbol"] = names[entry.index]
        item["text"] = blob.chart_text(entry.data) if blob.version == "jp" else entry.text
        item["bytes"] = entry.data.hex(" ")
        entries.append(item)
    _dump(path, {"address": f"0x{blob.symbols[name]:08X}", "entries": entries}, TEXT_NOTES[blob.version])
    return start, table.size


def export_icons(blob: Blob, output: Path) -> tuple[int, int]:
    start = blob.offset("g_addhero_icon_image_table") - 4
    icons = icon_set.parse(blob.data, start)
    folder = output / "icons"
    folder.mkdir()
    listing = []
    for icon in icons.icons:
        file_name = f"icon_{icon.index:02X}.png"
        png.write_rgba(folder / file_name, icon_set.ICON_SIZE, icon_set.ICON_SIZE, icon.rgba())
        listing.append(
            {
                "icon": f"0x{icon.index:02X}",
                "group": _icon_group(icon.index),
                "file": file_name,
                "offset": f"0x{icon.offset:X}",
                "palette": [f"0x{value:04X}" for value in icon.palette],
            }
        )
    _dump(
        folder / "icons.yaml",
        {
            "address": f"0x{blob.base + start:08X}",
            "note": "Icon ids are the save-screen party icons. At runtime the guest hero and golems are drawn with palettes the game builds, not the ones stored here.",
            "icons": listing,
        },
    )
    return start, icons.size


def export_steps(blob: Blob, path: Path) -> tuple[int, int]:
    names = _step_names()
    sequences = sorted((address, symbol) for symbol, address in blob.symbols.items() if symbol.startswith("g_addhero_loadseq_"))
    start = sequences[0][0] - blob.base
    end = blob.offset("g_addhero_single_byte_char_table")
    documents = []
    for position, (address, symbol) in enumerate(sequences):
        stop = sequences[position + 1][0] - blob.base if position + 1 < len(sequences) else end
        raw = blob.data[address - blob.base : stop]
        steps = []
        for value in raw:
            steps.append(names.get(value, f"0x{value:02X}"))
            if value == 0:
                break
        documents.append({"symbol": symbol, "steps": steps, "bytes": raw.hex(" ")})
    _dump(path, {"address": f"0x{sequences[0][0]:08X}", "sequences": documents})
    return start, end - start


# Layout of the text conversion chart (addhero_glyph.c: ADDHERO_CHAR_TABLE_*).
CHART_ROW_BYTES = 33
CHART_PAGE_BYTES = 16 * CHART_ROW_BYTES
CHART_FIRST_CODE = 0x20
CHART_LEADS = range(0x19, 0x20)

codecs.register_error("braces", lambda error: ("".join(f"{{{byte:02X}}}" for byte in error.object[error.start : error.end]), error.end))


def _sjis(raw: bytes) -> str:
    """Shift-JIS text, with bytes that don't decode written as {XX}."""
    return raw.decode("shift_jis", errors="braces")


def export_conversion(blob: Blob, path: Path, end: int) -> tuple[int, int]:
    """Export the game-text to Shift-JIS chart as the converter reads it."""
    start = blob.offset("g_addhero_single_byte_char_table")
    chart = blob.data[start:end]
    pages = blob.offset("g_addhero_char_page_base")

    def cell(offset: int) -> str | None:
        """One converted code: a Shift-JIS character, or its two bytes when they aren't one."""
        if not (start <= offset and offset + 2 <= end):
            return None
        raw = blob.data[offset : offset + 2]
        if 0x81 <= raw[0] <= 0x9F or 0xE0 <= raw[0] <= 0xFC:
            try:
                return raw.decode("shift_jis")
            except UnicodeDecodeError:
                pass
        return f"{{{raw[0]:02X} {raw[1]:02X}}}"

    one_byte = {}
    for row in range(CHART_FIRST_CODE >> 4, 0x10):
        index = (row << 4) - CHART_FIRST_CODE
        one_byte[f"0x{row:X}_"] = "".join(cell(start + (index // 16) * CHART_ROW_BYTES + (index % 16 + column) * 2) or "" for column in range(16))

    two_byte = {}
    for lead in CHART_LEADS:
        rows = {}
        for row in range(16):
            cells = [cell(pages + lead * CHART_PAGE_BYTES + row * CHART_ROW_BYTES + column * 2) for column in range(16)]
            if all(value is not None for value in cells):
                rows[f"0x{row:X}_"] = "".join(cells)
        if rows:
            two_byte[f"0x{lead:02X}"] = rows

    header = "# How the game turns its text codes into Shift-JIS for memory card titles; each row holds 16 codes.\n"
    _dump(
        path,
        {
            "address": f"0x{blob.base + start:08X}",
            "one_byte": one_byte,
            "two_byte": two_byte,
            "chart_lines": [_sjis(line) for line in chart.rstrip(b"\x00").split(b"\n")],
        },
        header,
    )
    return start, end - start


def _sjis_glyphs(raw: bytes) -> list[str]:
    glyphs = []
    for index in range(0, len(raw) - 1, 2):
        pair = raw[index : index + 2]
        if pair == b"\x00\x00":
            break
        glyphs.append(pair.decode("shift_jis", errors="replace"))
    return glyphs


def export_digits(blob: Blob, path: Path, variables: int) -> tuple[int, int]:
    decimal = blob.offset("g_addhero_decimal_glyphs")
    hexadecimal = blob.offset("g_addhero_hex_glyphs")
    _dump(
        path,
        {
            "decimal": {"address": f"0x{blob.base + decimal:08X}", "glyphs": _sjis_glyphs(blob.data[decimal:hexadecimal])},
            "hexadecimal": {"address": f"0x{blob.base + hexadecimal:08X}", "glyphs": _sjis_glyphs(blob.data[hexadecimal:variables])},
        },
    )
    return decimal, variables - decimal


def export_card_titles(blob: Blob, start: int, end: int, path: Path) -> list[tuple[int, int]]:
    """Export the Shift-JIS strings between the two text tables; returns their ranges."""
    found = []
    position = start
    while position < end:
        if blob.data[position] == 0:
            position += 1
            continue
        terminator = blob.data.index(b"\x00", position)
        found.append((position, terminator + 1 - position))
        position = terminator + 1
    notes = (
        "The title of a finished save. A note sign replaces the second dash when option bit 2 is set, the same flag that puts + in the file name.",
        "The title a save is written with. Saving puts the real title back as its last step, so a save still showing this one was interrupted.",
    )
    titles = []
    for position, (offset, size) in enumerate(found):
        title = {"address": f"0x{blob.base + offset:08X}", "text": blob.data[offset : offset + size - 1].decode("shift_jis", errors="replace")}
        if len(found) == len(notes):
            title["note"] = notes[position]
        titles.append(title)
    _dump(path, {"titles": titles}, "# Shift-JIS memory card title templates; the save screen (CARDA) writes them, ADDHERO only carries a copy.\n")
    return found


def export_card_strings(source: Path, symbol_map: dict[str, int], path: Path) -> list[dict[str, str]]:
    """Export the small data files next to the blob: the number overflow text and the card paths."""
    folder = source.parent
    overflow_file = folder / "addhero_format_overflow_and_file_template.rodatabin.bin"
    pattern_file = folder / "addhero_entry_header_template.rodatabin.bin"
    overflow_data = overflow_file.read_bytes()
    pattern_data = pattern_file.read_bytes()
    overflow_base = symbol_map["g_addhero_decimal_overflow_glyphs"]
    template = symbol_map["g_addhero_file_template"] - overflow_base

    def string_at(data: bytes, offset: int, encoding: str) -> str:
        return data[offset : data.index(b"\x00", offset)].decode(encoding)

    strings = [
        {"symbol": "g_addhero_decimal_overflow_glyphs", "address": f"0x{overflow_base:08X}", "text": string_at(overflow_data, 0, "shift_jis"),
         "note": "Shown instead of a number above 999999."},
        {"symbol": "g_addhero_file_template", "address": f"0x{overflow_base + template:08X}", "text": string_at(overflow_data, template, "ascii"),
         "note": "Memory card device; the slot digit is patched before use."},
        {"symbol": "g_addhero_entry_header_template", "address": f"0x{symbol_map['g_addhero_entry_header_template']:08X}", "text": string_at(pattern_data, 0, "ascii"),
         "note": "Directory search pattern for the whole card."},
    ]
    _dump(path, {"strings": strings})
    return [
        {"file": overflow_file.name, "address": f"0x{overflow_base:08X}", "size": f"0x{len(overflow_data):X}", "exported": "text/fixed_strings.yaml"},
        {"file": pattern_file.name, "address": f"0x{symbol_map['g_addhero_entry_header_template']:08X}", "size": f"0x{len(pattern_data):X}", "exported": "text/fixed_strings.yaml"},
    ]


def extract(source: Path, symbol_file: Path, output: Path, version: str) -> None:
    blob = Blob(source.read_bytes(), symbols.load(symbol_file), version)
    if output.exists():
        raise FileExistsError(f"{output} already exists")
    output.mkdir(parents=True)
    text = output / "text"
    tables = output / "tables"
    unknown = output / "unknown"
    text.mkdir()
    tables.mkdir()

    ranges = []

    def add(name: str, start: int, size: int, **extra: str) -> None:
        ranges.append({"name": name, "address": f"0x{blob.base + start:08X}", "offset": f"0x{start:X}", "size": f"0x{size:X}", **extra})

    start, size = export_text(blob, "g_addhero_text_table", "g_addhero_text_", text / "messages.yaml")
    add("messages", start, size, file="text/messages.yaml")
    messages_end = start + size
    start, size = export_text(blob, "g_addhero_location_text_table", "g_addhero_location_text_table", text / "locations.yaml")
    add("locations", start, size, file="text/locations.yaml")
    for title_start, title_size in export_card_titles(blob, messages_end, start, text / "card_titles.yaml"):
        add("card title template", title_start, title_size, file="text/card_titles.yaml")
    start, size = export_icons(blob, output)
    add("icons", start, size, file="icons/icons.yaml")
    end = start + size
    if blob.data[end : end + 4] == blob.data[end - 4 : end]:
        add("icon set trailing word", end, 4, note="Repeats the icon set's last word, like the trailing word after some embedded TIMs.")
    start, size = export_steps(blob, tables / "card_steps.yaml")
    add("card steps", start, size, file="tables/card_steps.yaml")

    digits = blob.offset("g_addhero_decimal_glyphs")
    start, size = export_conversion(blob, tables / "text_conversion.yaml", digits)
    add("text to Shift-JIS table", start, size, file="tables/text_conversion.yaml",
        note="Used by a conversion helper that ADDHERO never calls.")

    variables = blob.next_symbol_after("g_addhero_hex_glyphs")
    start, size = export_digits(blob, tables / "digit_glyphs.yaml", variables)
    add("digit glyphs", start, size, file="tables/digit_glyphs.yaml")

    tail = blob.data[variables:]
    if any(tail):
        unknown.mkdir(exist_ok=True)
        (unknown / "variables.bin").write_bytes(tail)
        add("variables", variables, len(tail), file="unknown/variables.bin")
    else:
        add("variables", variables, len(tail), note="ADDHERO's variables and buffers; all zero on the disc, not exported.")

    ranges.sort(key=lambda item: int(item["offset"], 16))
    covered = 0
    gaps = []
    for item in ranges:
        item_start = int(item["offset"], 16)
        if item_start > covered:
            gaps.append((covered, item_start - covered))
        covered = max(covered, item_start + int(item["size"], 16))
    for gap_start, gap_size in gaps:
        raw = blob.data[gap_start : gap_start + gap_size]
        if any(raw):
            file_name = f"{blob.base + gap_start:08X}.bin"
            unknown.mkdir(exist_ok=True)
            (unknown / file_name).write_bytes(raw)
            add("unknown", gap_start, gap_size, file=f"unknown/{file_name}")
        else:
            add("padding", gap_start, gap_size, note="zero bytes")
    ranges.sort(key=lambda item: int(item["offset"], 16))
    other_files = export_card_strings(source, blob.symbols, text / "fixed_strings.yaml")
    _dump(
        output / "byte-map.yaml",
        {"source": source.name, "address": f"0x{blob.base:08X}", "size": f"0x{len(blob.data):X}", "ranges": ranges, "other_files": other_files},
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("databin", type=Path, help="addhero_data.databin.bin from the version's assets folder")
    parser.add_argument("symbols", type=Path, help="the version's addhero_symbol_addrs.txt")
    parser.add_argument("output", type=Path, help="new folder to write")
    parser.add_argument("--version", choices=sorted(text_table.TWO_BYTE_CODES), default="us", help="game version the databin comes from")
    args = parser.parse_args()
    try:
        extract(args.databin, args.symbols, args.output, args.version)
    except (OSError, KeyError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
