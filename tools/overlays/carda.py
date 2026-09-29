#!/usr/bin/env python3
"""Export CARDA's data as files people can read.

The save screen keeps its messages, item names, icons and card step tables in
one data blob. The build links that blob unchanged. This tool writes YAML and
PNG copies under assets/exports, just like the ADDHERO extractor.

``CardaSymbols`` looks up addresses in the version's symbol file. ``read_blob``
walks the data in order, with one reader per part. The formats shared with
ADDHERO use the same readers and writers in ``card_data``.

Example:

    python3 -m tools.overlays.carda --version us assets/exports/us/overlays/carda
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import re
import shutil
import sys
import tempfile

from tools.overlays import icon_set, png, splat_config, symbols, text_table
from tools.overlays.card_data import (
    CARD_TITLE_NOTES,
    Blob,
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
OVERLAY_CONFIG = "overlays/CARDA.BIN.yaml"
SYMBOL_FILE = "symbols/carda_symbol_addrs.txt"
STEP_HEADER = REPO_ROOT / "src/overlays/carda/carda_internal.h"

# CardaSaveIcon in carda_widgets.c: a palette and two 16 x 16, 4-bit frames.
SAVE_ICON_SIZE = 16
SAVE_ICON_FRAMES = 2
SAVE_ICON_FRAME_BYTES = SAVE_ICON_SIZE * SAVE_ICON_SIZE // 2
SAVE_ICON_BYTES = icon_set.PALETTE_BYTES + SAVE_ICON_FRAMES * SAVE_ICON_FRAME_BYTES

CARD_TITLES_NOTE = "# Shift-JIS title templates written into the memory card header.\n"
SAVE_ICONS_NOTE = (
    "The icons shown by the console's memory card browser. CARDA picks one from "
    "the number of placed lands: placed_land_count * 9 / 26, capped at icon 9. "
    "Each icon has two animation frames sharing one palette."
)


# ---------------------------------------------------------------------------
# Inputs


@dataclass(frozen=True)
class CardaSymbols(CardSymbols):
    """Every address the tool needs, named in SYMBOL_NAMES below."""

    items: int
    save_title: int
    bad_title: int
    save_icon_offsets: int
    title_dash: int
    title_colon: int
    overflow_text: int
    save_card_path: int
    card_path: int
    directory_pattern: int

    @classmethod
    def load(cls, path: Path) -> CardaSymbols:
        named = symbols.load(path)
        missing = [name for name in SYMBOL_NAMES.values() if name not in named]
        if missing:
            raise ValueError(
                f"{path} has no {', '.join(missing)}. If a symbol was renamed, "
                "update SYMBOL_NAMES in tools/overlays/carda.py."
            )
        addresses = {key: named[name] for key, name in SYMBOL_NAMES.items()}
        return cls(**addresses, named=named)


SYMBOL_NAMES = {
    "messages": "g_carda_text_checking_card",
    "items": "g_carda_item_names",
    "save_title": "g_carda_save_title_template",
    "bad_title": "g_carda_bad_title_template",
    "save_icon_offsets": "g_carda_save_icon_offsets",
    "locations": "g_carda_location_names",
    "icon_offsets": "g_carda_icon_image_offsets",
    "card_steps": "g_carda_steps_initial_scan",
    "chart": "g_glyph_single_byte_chart",
    "chart_pages": "g_glyph_chart_page_base",
    "decimal_glyphs": "g_glyph_decimal_digits",
    "hex_glyphs": "g_glyph_hex_digits",
    "title_dash": "g_carda_title_dash",
    "title_colon": "g_carda_title_colon",
    "overflow_text": "g_decimal_overflow_text",
    "save_card_path": "g_carda_save_card_path_prefix",
    "card_path": "g_carda_card_path_prefix",
    "directory_pattern": "g_carda_card_search_path",
}
MESSAGE_SYMBOL_PREFIX = "g_carda_text_"
CARD_STEP_SYMBOL_PREFIX = ("g_carda_steps_", "g_card_steps_")


@dataclass(frozen=True)
class Inputs:
    """Where one version's CARDA data and symbols live."""

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
    """CardaCardStep values from the C header, so names stay in one place."""
    text = STEP_HEADER.read_text(encoding="ascii")
    pairs = re.findall(r"\b(CARDA_STEP_\w+)\s*=\s*(\d+)", text)
    return {int(value): name for name, value in pairs}


@dataclass(frozen=True)
class SaveIcon:
    """One memory card icon: both frames and their shared palette."""

    index: int
    offset: int
    palette: tuple[int, ...]
    frames: tuple[bytes, ...]


@dataclass(frozen=True)
class SaveIcons:
    address: int
    icons: tuple[SaveIcon, ...]


# ---------------------------------------------------------------------------
# Reading the blob, in address order


def read_blob(blob: Blob[CardaSymbols]) -> list[Part]:
    """Read the known parts and account for whatever falls between them."""
    chart = Chart(blob)
    parts = [
        read_messages(blob, chart),
        read_items(blob, chart),
    ]
    parts += read_card_titles(blob)
    parts += read_save_icons(blob)
    parts.append(read_locations(blob, chart))
    parts += read_icons(blob)
    parts.append(read_card_steps(blob))
    parts.append(read_chart(chart))
    parts.append(read_digit_glyphs(blob))
    parts.append(read_variables(blob))
    return cover_gaps(blob, sorted(parts, key=lambda part: part.start))


def read_messages(blob: Blob[CardaSymbols], chart: Chart) -> Part:
    """Messages and labels for saving, loading and Ring Ring Land transfers."""
    table = blob.symbols.messages
    content, end = read_text_list(blob, chart, table, MESSAGE_SYMBOL_PREFIX)
    return Part("messages", blob.offset(table), end, "text/messages.yaml", content)


def read_items(blob: Blob[CardaSymbols], chart: Chart) -> Part:
    """Item names shown when a pet brings something back from Ring Ring Land."""
    table = blob.symbols.items
    content, end = read_text_list(blob, chart, table, "g_carda_item_names")
    return Part("items", blob.offset(table), end, "text/items.yaml", content)


def read_card_titles(blob: Blob[CardaSymbols]) -> list[Part]:
    """The finished and interrupted-save titles, stored as Shift-JIS strings."""
    titles = []
    ranges = []
    for address, note in zip(
        (blob.symbols.save_title, blob.symbols.bad_title), CARD_TITLE_NOTES
    ):
        start = blob.offset(address)
        end = blob.data.index(b"\x00", start)
        titles.append(CardTitle(address, blob.data[start:end].decode("shift_jis"), note))
        ranges.append((start, end + 1))
    parts = []
    for index, (start, end) in enumerate(ranges):
        content = CardTitles(tuple(titles), CARD_TITLES_NOTE) if index == 0 else None
        parts.append(Part("card title template", start, end, "text/card_titles.yaml", content))
    return parts


def read_save_icons(blob: Blob[CardaSymbols]) -> list[Part]:
    """A count and offset table, then the two-frame memory card icons."""
    start = blob.offset(blob.symbols.save_icon_offsets) - 4
    limit = blob.offset(blob.symbols.locations)
    count = int.from_bytes(blob.data[start : start + 4], "little")
    table_size = 4 + count * 4
    if count == 0 or start < 0 or start + table_size > limit:
        raise ValueError(f"save icon count {count} does not fit before the location table")
    icons = []
    end = start + table_size
    for index in range(count):
        slot = start + 4 + index * 4
        offset = int.from_bytes(blob.data[slot : slot + 4], "little")
        position = start + offset
        if offset < table_size or position + SAVE_ICON_BYTES > limit:
            raise ValueError(f"save icon {index} has an invalid offset 0x{offset:X}")
        palette = tuple(
            int.from_bytes(blob.data[position + color * 2 : position + color * 2 + 2], "little")
            for color in range(icon_set.PALETTE_COLORS)
        )
        position += icon_set.PALETTE_BYTES
        frames = []
        for frame in range(SAVE_ICON_FRAMES):
            first = position + frame * SAVE_ICON_FRAME_BYTES
            frames.append(blob.data[first : first + SAVE_ICON_FRAME_BYTES])
        icons.append(SaveIcon(index, offset, palette, tuple(frames)))
        end = max(end, start + offset + SAVE_ICON_BYTES)
    content = SaveIcons(blob.address + start, tuple(icons))
    parts = [Part("save icons", start, end, "save_icons/icons.yaml", content)]
    if blob.data[end : end + 4] == blob.data[end - 4 : end]:
        parts.append(
            Part("save icon set trailing word", end, end + 4,
                 note="Repeats the save icon set's last word.")
        )
    return parts


def read_locations(blob: Blob[CardaSymbols], chart: Chart) -> Part:
    """Location names selected by each save's music track."""
    table = blob.symbols.locations
    content, end = read_text_list(blob, chart, table, "g_carda_location_names")
    return Part("locations", blob.offset(table), end, "text/locations.yaml", content)


def read_card_steps(blob: Blob[CardaSymbols]) -> Part:
    """Follow each named entry point to DONE, including entries inside another sequence.

    For example, write_save_keep_handles starts one byte into write_save.
    Stopping at the next symbol would cut off write_save before it did any work.
    """
    names = card_step_names()
    start = blob.offset(blob.symbols.card_steps)
    end = blob.offset(blob.symbols.chart)
    sequences = []
    found = blob.symbols.with_prefix(CARD_STEP_SYMBOL_PREFIX)
    for name, address in sorted(found.items(), key=lambda item: item[1]):
        position = blob.offset(address)
        if not start <= position < end:
            raise ValueError(f"{name} is outside the card step table")
        stop = blob.data.find(b"\x00", position, end)
        if stop < 0:
            raise ValueError(f"{name} has no CARDA_STEP_DONE before the character chart")
        raw = blob.data[position : stop + 1]
        steps = tuple(names.get(value, f"0x{value:02X}") for value in raw)
        sequences.append(StepSequence(name, steps, raw))
    content = CardSteps(blob.symbols.card_steps, tuple(sequences))
    return Part("card steps", start, end, "tables/card_steps.yaml", content)


def read_chart(chart: Chart) -> Part:
    """The character chart CARDA uses when it writes the hero's name into the title."""
    return Part(
        "text to Shift-JIS table", chart.start, chart.end,
        "tables/text_conversion.yaml", chart.content(),
    )


def read_variables(blob: Blob[CardaSymbols]) -> Part:
    """Runtime state and buffers; zero on the disc, but keep anything unexpected."""
    start, end = variables_start(blob), len(blob.data)
    if any(blob.data[start:]):
        return Part("variables", start, end, "unknown/variables.bin", blob.data[start:])
    note = "CARDA's variables and buffers; all zero on the disc, not exported."
    return Part("variables", start, end, note=note)


def read_fixed_strings(
    files: list[splat_config.DataFile], names: CardaSymbols
) -> tuple[list[dict[str, str]], list[dict[str, str]]]:
    """Title punctuation, number overflow text and paths from the small rodata files."""
    wanted = (
        ("title_dash", "shift_jis", "Separates the save number from the play time."),
        ("title_colon", "shift_jis", "Separates the hours and minutes in the play time."),
        ("save_card_path", "ascii", "Memory card device used by the save screen."),
        ("overflow_text", "shift_jis", "Shown instead of a number above 999999."),
        ("card_path", "ascii", "Memory card device used by the card step machine."),
        ("directory_pattern", "ascii", "Directory search pattern for the whole card."),
    )
    strings = []
    sources = {}
    for key, encoding, note in wanted:
        address = getattr(names, key)
        source = splat_config.file_containing(files, address, SYMBOL_NAMES[key])
        data = source.path.read_bytes()
        start = address - source.start
        text = data[start : data.index(b"\x00", start)].decode(encoding)
        strings.append(
            {"symbol": SYMBOL_NAMES[key], "address": hex_address(address), "text": text, "note": note}
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


@write_part.register
def _write_save_icons(content: SaveIcons, path: Path) -> None:
    """Write each animation frame as a PNG, with the palette and frame order in YAML."""
    path.parent.mkdir(parents=True, exist_ok=True)
    listing = []
    for icon in content.icons:
        frames = []
        for index, pixels in enumerate(icon.frames):
            file_name = f"icon_{icon.index:02X}_{index}.png"
            frame = icon_set.Icon(icon.index, icon.offset, icon.palette, pixels)
            png.write_rgba(path.parent / file_name, SAVE_ICON_SIZE, SAVE_ICON_SIZE, frame.rgba())
            frames.append(file_name)
        listing.append(
            {
                "icon": f"0x{icon.index:02X}",
                "frames": frames,
                "offset": f"0x{icon.offset:X}",
                "palette": [f"0x{value:04X}" for value in icon.palette],
            }
        )
    dump_yaml(path, {"address": hex_address(content.address), "note": SAVE_ICONS_NOTE, "icons": listing})


# ---------------------------------------------------------------------------
# Putting it together


def load_blob(inputs: Inputs) -> tuple[Blob[CardaSymbols], list[splat_config.DataFile]]:
    """Find the data blob through the splat config and check it against its declared size."""
    names = CardaSymbols.load(inputs.symbol_file)
    files = splat_config.data_files(inputs.overlay_config, inputs.assets)
    source = splat_config.file_containing(files, names.messages, SYMBOL_NAMES["messages"])
    if not source.path.exists():
        raise ValueError(f"{source.path} is missing; run make splat first")
    data = source.path.read_bytes()
    if len(data) != source.end - source.start:
        raise ValueError(f"{source.path} size does not match the splat config; run make splat again")
    for key in (
        "messages", "items", "save_title", "bad_title", "save_icon_offsets", "locations",
        "icon_offsets", "card_steps", "chart", "decimal_glyphs", "hex_glyphs",
    ):
        if not source.contains(getattr(names, key)):
            raise ValueError(f"{SYMBOL_NAMES[key]} is outside {source.path}; check the splat config")
    return Blob(data, source.start, source.path.name, inputs.version, names), files


def extract(inputs: Inputs, output: Path) -> None:
    """Read everything before writing; move the finished folder into place at the end."""
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
