#!/usr/bin/env python3
"""Export TITLE's menu artwork, new-game setup screen and game-state templates.

The build links one unchanged data blob after title_save.c. Readers describe
its resources as Parts, reusing the TIM and table parsers in tools/assets, and
the byte map also records the TIMs' repeated last words and the zero-filled
menu state at the end. Game-state records follow include/saved_game.h. JP
text uses CLOAD's character chart, as FIELD, GNAME and GOLEM do.

Example:

    python3 -m tools.overlays.title --version us assets/exports/us/overlays/title
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import shutil
import sys
import tempfile

from tools.assets.asset_offset_table import AssetOffsetTable
from tools.assets.psx_tim import TimImage, parse_embedded_tim
from tools.assets.save_layout_table import SaveLayoutTable
from tools.assets.tim_upload_table import TimUploadTable
from tools.assets.u8_sequence import U8Sequence
from tools.assets.uv_rect_table import UvRectTable
from tools.overlays import cload, saved_game, splat_config, symbols, text_table, tim_preview
from tools.overlays.card_data import Chart
from tools.overlays.resources import (
    Blob, Part, byte_map_entry, cover_gaps, dump_yaml, hex_address, write_part,
)

REPO_ROOT = Path(__file__).resolve().parents[2]
OVERLAY_CONFIG = "overlays/TITLE.BIN.yaml"
SYMBOL_FILE = "symbols/title_symbol_addrs.txt"
SAVE_SOURCE = REPO_ROOT / "src/overlays/title/title_save.c"
MENU_TIM_NAMES = ("menu_items", "backdrop")
CURSOR_FRAME_COUNT = 4  # render_title_menu_items indexes (frame >> 2) & 3
TEXTURE_COUNT = 11  # upload_save_layout_textures uploads 11 entries
TEXTURE_SYMBOL = "g_save_layout_tim_{:02d}"
WEAPON_SLOT_COUNT = 11  # handle_save_slot_input wraps the cursor at 11
UV_UNIT_PIXELS = 8  # SlotUvRect fields are in 8-pixel units
LAYOUT_ENTRY_COUNT = 27  # SAVE_LAYOUT_ENTRIES
NEW_GAME_SIZE = 0x50BC
COPIED_WORDS = 0xC9A  # SAVED_GAME_TEMPLATE_WORDS
HERO_WORDS = 0x94  # HERO_TEMPLATE_WORDS

# upload_tim(table + offset, x, y, clut_x, clut_y) in init_title_menu_state.
MENU_UPLOADS = {
    "menu_items": {"image": {"x": 320, "y": 0}, "palettes": {"x": 0, "y": 480}},
    "backdrop": {"image": {"x": 320, "y": 256}, "palettes": {"x": 0, "y": 481}},
}


# ---------------------------------------------------------------------------
# Inputs and decoded parts


@dataclass(frozen=True)
class TitleSymbols:
    """Resource boundaries in their stored order; textures are the 11 save-layout TIMs."""

    menu_offsets: int
    menu_items: int
    backdrop: int
    cursor_blink: int
    textures: tuple[int, ...]
    texture_table: int
    panel_uvs: int
    sprite_uvs: int
    layout: int
    weapons: int
    new_game: int
    alternate: int
    hero_default: int
    hero_continue: int
    variables: int

    @classmethod
    def load(cls, path: Path) -> TitleSymbols:
        named = symbols.load(path)
        wanted = list(SYMBOL_NAMES.values()) + [TEXTURE_SYMBOL.format(index) for index in range(TEXTURE_COUNT)]
        missing = [name for name in wanted if name not in named]
        if missing:
            raise ValueError(
                f"{path} has no {', '.join(missing)}. If a symbol was renamed, "
                "update SYMBOL_NAMES or TEXTURE_SYMBOL in tools/overlays/title.py."
            )
        textures = tuple(named[TEXTURE_SYMBOL.format(index)] for index in range(TEXTURE_COUNT))
        return cls(textures=textures, **{key: named[name] for key, name in SYMBOL_NAMES.items()})

    def ordered(self) -> list[tuple[str, int]]:
        """Every boundary with its symbol name, in blob order."""
        result = []
        for key, name in SYMBOL_NAMES.items():
            if key == "texture_table":
                result += [(TEXTURE_SYMBOL.format(index), address) for index, address in enumerate(self.textures)]
            result.append((name, getattr(self, key)))
        return result


SYMBOL_NAMES = {
    "menu_offsets": "g_title_menu_tim_table",
    "menu_items": "g_title_menu_tim_0",
    "backdrop": "g_title_menu_tim_1",
    "cursor_blink": "g_cursor_blink_u_offsets",
    "texture_table": "g_save_layout_tex_table",
    "panel_uvs": "g_save_slot_panel_uv_table",
    "sprite_uvs": "g_save_slot_sprite_uv_table",
    "layout": "g_save_layout_table",
    "weapons": "g_starting_weapon_records",
    "new_game": "g_new_game_template",
    "alternate": "g_field_start_template",
    "hero_default": "g_hero_template_type_0",
    "hero_continue": "g_hero_template_type_1",
    "variables": "g_title_menu_exit_state",
}


@dataclass(frozen=True)
class Inputs:
    version: str
    config: Path
    assets: Path


@dataclass(frozen=True)
class Table:
    address: int
    document: dict[str, object]
    raw: bytes


@dataclass(frozen=True)
class Image:
    """An original TIM, where TITLE uploads it, and anything else worth noting."""

    address: int
    tim: TimImage
    upload: dict[str, object]
    note: str


@dataclass(frozen=True)
class Record:
    """A saved-game record: decoded fields in YAML beside the complete original bytes."""

    address: int
    raw: bytes
    document: dict[str, object]


@dataclass(frozen=True)
class Text:
    """How game text in this version becomes readable."""

    version: str
    chart: Chart | None

    @property
    def two_byte_codes(self) -> frozenset[int]:
        return text_table.TWO_BYTE_CODES[self.version]

    def read(self, raw: bytes) -> str:
        if self.chart is not None:
            return self.chart.decode(raw)
        return text_table.decode(raw, self.two_byte_codes)


def read_bytes(blob: Blob, address: int, end: int, what: str) -> bytes:
    start, stop = blob.offset(address), blob.offset(end)
    if not 0 <= start <= stop <= len(blob.data):
        raise ValueError(f"{what} is outside {blob.file_name}")
    return blob.data[start:stop]


def part(blob: Blob, name: str, address: int, size: int, file: str, content: object) -> Part:
    start = blob.offset(address)
    return Part(name, start, start + size, file, content)


# ---------------------------------------------------------------------------
# Reading the blob, in address order


def read_tim(blob: Blob, name: str, address: int, end: int, file: str,
             trailing_word: bool, upload: dict, note: str) -> list[Part]:
    """One TIM, and its repeated last word when the layout has one."""
    raw = read_bytes(blob, address, end, name)
    tim = parse_embedded_tim(raw, trailing_duplicate_word=trailing_word)
    tim_preview.check(tim, name)
    size = len(tim.to_bytes())
    parts = [part(blob, name, address, size, file, Image(address, tim, upload, note))]
    if trailing_word:
        start = blob.offset(address) + size
        parts.append(Part(f"{name} trailing word", start, start + 4, note="Repeats the TIM's last word."))
    return parts


def read_menu(blob: Blob[TitleSymbols]) -> list[Part]:
    names = blob.symbols
    raw = read_bytes(blob, names.menu_offsets, names.menu_items, "menu TIM offsets")
    table = AssetOffsetTable.parse_binary(raw, MENU_TIM_NAMES)
    starts = [names.menu_offsets + entry.offset for entry in table.entries]
    if starts != [names.menu_items, names.backdrop]:
        raise ValueError("menu TIM offsets don't point at g_title_menu_tim_0 and g_title_menu_tim_1")
    document = {**table.document(), "note": "Offsets count from the table's own address; the count word comes first."}
    parts = [part(blob, "menu TIM offsets", names.menu_offsets, len(raw), "title_menu/offsets.yaml",
                  Table(names.menu_offsets, document, raw))]
    parts += read_tim(blob, "menu item TIM", names.menu_items, names.backdrop, "title_menu/menu_items/image.yaml",
                      False, MENU_UPLOADS["menu_items"],
                      "Each menu row is 16 pixels tall: row 0 is the header, row N the item in slot N - 1, "
                      "row 7 the cursor. Palette 1 draws the header and the selected item, palette 2 the "
                      "other items, palette 0 the cursor. The 256 colors are uploaded as one row.")
    parts += read_tim(blob, "title backdrop TIM", names.backdrop, names.cursor_blink, "title_menu/backdrop/image.yaml",
                      True, MENU_UPLOADS["backdrop"], "16-bit direct color; no palette.")
    raw = read_bytes(blob, names.cursor_blink, names.textures[0], "cursor blink offsets")
    frames = U8Sequence.parse_binary(raw, CURSOR_FRAME_COUNT)
    document = {**frames.document(),
                "note": "Texture U of the cursor for each blink frame; the frame advances every fourth render."}
    parts.append(part(blob, "cursor blink offsets", names.cursor_blink, len(raw), "title_menu/cursor_blink.yaml",
                      Table(names.cursor_blink, document, raw)))
    return parts


def read_textures(blob: Blob[TitleSymbols]) -> list[Part]:
    """The save-layout TIMs, found through the upload table that points at them."""
    names = blob.symbols
    raw = read_bytes(blob, names.texture_table, names.panel_uvs, "save-layout texture table")
    targets = tuple(f"texture_{index:02d}" for index in range(TEXTURE_COUNT))
    table = TimUploadTable.parse_binary(raw, targets)
    sources = [entry.source_address for entry in table.entries]
    if sources != list(names.textures):
        raise ValueError("save-layout texture table doesn't point at the g_save_layout_tim_NN symbols")
    parts = []
    entries = []
    ends = list(names.textures[1:]) + [names.texture_table]
    for index, (entry, address, end) in enumerate(zip(table.entries, names.textures, ends)):
        file = f"save_slot_menu/textures/{index:02d}/image.yaml"
        upload = {"image": {"x": entry.texture_x, "y": entry.texture_y},
                  "palettes": {"x": entry.clut_x, "y": entry.clut_y}}
        parts += read_tim(blob, f"save-layout TIM {index:02d}", address, end, file, True, upload,
                          "upload_save_layout_textures uploads the palettes as one row and the pixels at the "
                          "table's coordinates, not the ones stored in the TIM.")
        entries.append({
            "index": index, "file": f"textures/{index:02d}/image.yaml",
            "texture_vram": upload["image"], "clut_vram": upload["palettes"],
            "source_address": hex_address(entry.source_address),
            "initial_control": f"0x{entry.initial_control:08X}",
        })
    document = {
        "entry_count": len(entries), "entries": entries,
        "note": "The control word is filled at runtime from each TIM: pixel mode in bits 0-2, "
                "width in bits 3-12, height in bits 13-22.",
    }
    parts.append(part(blob, "save-layout texture table", names.texture_table, len(raw),
                      "save_slot_menu/textures.yaml", Table(names.texture_table, document, raw)))
    return parts


def read_uv_tables(blob: Blob[TitleSymbols]) -> list[Part]:
    names = blob.symbols
    parts = []
    for name, address, end, file, side in (
        ("panel UV table", names.panel_uvs, names.sprite_uvs, "panel_uvs.yaml", "right (g_slot_slide_x > 0)"),
        ("sprite UV table", names.sprite_uvs, names.layout, "sprite_uvs.yaml", "left (g_slot_slide_x < 0)"),
    ):
        raw = read_bytes(blob, address, end, name)
        table = UvRectTable.parse_binary(raw, UV_UNIT_PIXELS, WEAPON_SLOT_COUNT + 1)
        document = table.document()
        for index, entry in enumerate(document["entries"]):
            entry["weapon_slot"] = None if index == 0 else index - 1
        document["note"] = (f"Entry 0 is drawn until the stage slides {side}; after that, entry "
                            "weapon slot + 1. Values are already multiplied by unit_pixels.")
        parts.append(part(blob, name, address, len(raw), f"save_slot_menu/{file}", Table(address, document, raw)))
    return parts


def read_layout(blob: Blob[TitleSymbols]) -> Part:
    names = blob.symbols
    raw = read_bytes(blob, names.layout, names.weapons, "save-layout table")
    textures = tuple(f"texture_{index:02d}" for index in range(TEXTURE_COUNT))
    table = SaveLayoutTable.parse_binary(raw, textures, LAYOUT_ENTRY_COUNT)
    document = table.document()
    for index, entry in enumerate(document["entries"]):
        entry["index"] = index
    document["note"] = ("TITLE rewrites entries 0, 2-15, 18 and 19 while the screen runs; these are the "
                        "starting values. Textures name save_slot_menu/textures/NN.")
    return part(blob, "save-layout table", names.layout, len(raw), "save_slot_menu/layout.yaml",
                Table(names.layout, document, raw))


def read_weapons(blob: Blob[TitleSymbols], text: Text) -> Part:
    names = blob.symbols
    raw = read_bytes(blob, names.weapons, names.new_game, "starting weapons")
    if len(raw) != WEAPON_SLOT_COUNT * saved_game.ITEM_SIZE:
        raise ValueError(f"expected {WEAPON_SLOT_COUNT} starting weapon records")
    records = []
    for index in range(WEAPON_SLOT_COUNT):
        record = saved_game.item_record(raw[index * saved_game.ITEM_SIZE : (index + 1) * saved_game.ITEM_SIZE],
                                        text.read, text.two_byte_codes)
        if record is None or record["category"] != "weapon":
            raise ValueError(f"starting weapon {index} is not a weapon record")
        records.append({"slot": index, **record})
    document = {
        "record_size": saved_game.ITEM_SIZE, "records": records,
        "note": "The selected slot's record is copied into the hero's weapon slot, and the technique "
                "masks of every other slot index are cleared.",
    }
    return part(blob, "starting weapons", names.weapons, len(raw), "save_slot_menu/starting_weapons.yaml",
                Table(names.weapons, document, raw))


def read_game_states(blob: Blob[TitleSymbols], text: Text) -> list[Part]:
    names = blob.symbols
    parts = []
    for name, address, end, size, file, note in (
        ("new-game state", names.new_game, names.alternate, NEW_GAME_SIZE, "new_game",
         "load_saved_game_template(0) copies the first copied_size bytes over g_saved_game. "
         "Bytes from layout_size on are outside SavedGameLayout."),
        ("alternate game state", names.alternate, names.hero_default, saved_game.LAYOUT_SIZE, "alternate",
         "load_saved_game_template with a nonzero argument copies copied_size bytes, which runs "
         "into the first bytes of hero_default."),
    ):
        raw = read_bytes(blob, address, end, name)
        if len(raw) != size:
            raise ValueError(f"{name} is 0x{len(raw):X} bytes, expected 0x{size:X}")
        document = {"file": f"{file}.bin", "size": size, "layout_size": saved_game.LAYOUT_SIZE,
                    "copied_size": COPIED_WORDS * 4, "note": note,
                    **saved_game.layout(raw, text.read, text.two_byte_codes)}
        parts.append(part(blob, name, address, size, f"game_state/{file}.yaml", Record(address, raw, document)))
    for name, address, end, file, note in (
        ("default hero record", names.hero_default, names.hero_continue, "hero_default",
         "load_hero_template(0) copies this over characters[0]."),
        ("continue hero record", names.hero_continue, names.variables, "hero_continue",
         "load_hero_template(1) copies this over characters[0] and sets bit 0 of the mode flags word."),
    ):
        raw = read_bytes(blob, address, end, name)
        if len(raw) != HERO_WORDS * 4:
            raise ValueError(f"{name} is 0x{len(raw):X} bytes, expected 0x{HERO_WORDS * 4:X}")
        document = {"file": f"{file}.bin", "size": len(raw), "note": note,
                    **saved_game.character_record(raw, text.read, text.two_byte_codes)}
        parts.append(part(blob, name, address, len(raw), f"game_state/{file}.yaml", Record(address, raw, document)))
    return parts


def read_variables(blob: Blob[TitleSymbols]) -> Part:
    start = blob.offset(blob.symbols.variables)
    if any(blob.data[start:]):
        return Part("menu state", start, len(blob.data), "unknown/variables.bin", blob.data[start:])
    return Part("menu state", start, len(blob.data), note="Runtime menu variables; all zero on disc, not exported.")


def read_blob(blob: Blob[TitleSymbols], text: Text) -> list[Part]:
    parts = read_menu(blob)
    parts += read_textures(blob)
    parts += read_uv_tables(blob)
    parts.append(read_layout(blob))
    parts.append(read_weapons(blob, text))
    parts += read_game_states(blob, text)
    parts.append(read_variables(blob))
    return cover_gaps(blob, sorted(parts, key=lambda item: item.start))


# ---------------------------------------------------------------------------
# Writing


@write_part.register
def _write_table(content: Table, path: Path) -> None:
    dump_yaml(path, {"address": hex_address(content.address), **content.document, "bytes": content.raw.hex(" ")})


@write_part.register
def _write_image(content: Image, path: Path) -> None:
    tim = content.tim
    path.parent.mkdir(parents=True, exist_ok=True)
    (path.parent / "image.tim").write_bytes(tim.to_bytes())
    previews = tim_preview.write_previews(path.parent, tim)
    dump_yaml(path, {
        "address": hex_address(content.address), "file": "image.tim",
        "width": tim_preview.width(tim), "height": tim.pixels.height,
        "pixel_mode": tim.metadata()["pixel_mode_name"], "stored_layout": tim.metadata(),
        "upload": content.upload, "previews": [preview.document() for preview in previews],
        "note": content.note + " Previews show color zero as transparent and every other color opaque.",
    })


@write_part.register
def _write_record(content: Record, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    (path.parent / content.document["file"]).write_bytes(content.raw)
    dump_yaml(path, {"address": hex_address(content.address), **content.document})


# ---------------------------------------------------------------------------
# Putting it together


def load_blob(inputs: Inputs) -> Blob[TitleSymbols]:
    names = TitleSymbols.load(inputs.config / SYMBOL_FILE)
    files = splat_config.data_files(inputs.config / OVERLAY_CONFIG, inputs.assets)
    source = splat_config.file_containing(files, names.menu_offsets, SYMBOL_NAMES["menu_offsets"])
    if not source.path.exists():
        raise ValueError(f"{source.path} is missing; run make splat first")
    data = source.path.read_bytes()
    if len(data) != source.end - source.start:
        raise ValueError(f"{source.path} size does not match the splat config; run make splat again")
    if names.menu_offsets != source.start:
        raise ValueError(f"TITLE data must start at {SYMBOL_NAMES['menu_offsets']}")
    previous = source.start - 1
    for name, address in names.ordered():
        if not source.contains(address):
            raise ValueError(f"{name} is outside {source.path}")
        if address <= previous:
            raise ValueError(f"{name} is out of resource order")
        previous = address
    return Blob(data, source.start, source.path.name, inputs.version, names)


def load_text(inputs: Inputs) -> Text:
    if inputs.version != "jp":
        return Text(inputs.version, None)
    return Text(inputs.version, Chart(cload.load_blob(cload.Inputs(inputs.version, inputs.config, inputs.assets))))


def extract(inputs: Inputs, output: Path) -> None:
    """Read everything, write a temporary folder, then move the finished export into place."""
    if output.exists():
        raise FileExistsError(f"{output} already exists")
    blob = load_blob(inputs)
    text = load_text(inputs)
    parts = read_blob(blob, text)
    byte_map = {
        "source": blob.file_name, "address": hex_address(blob.address),
        "size": f"0x{len(blob.data):X}", "ranges": [byte_map_entry(blob, item) for item in parts],
    }
    if text.chart:
        chart = text.chart
        byte_map["text_decoder"] = {
            "source": chart.blob.file_name, "overlay": "CLOAD",
            "address": hex_address(chart.blob.address + chart.start), "size": f"0x{chart.end - chart.start:X}",
            "note": "JP text uses CLOAD's character chart. TITLE does not contain this conversion table.",
        }
    output.parent.mkdir(parents=True, exist_ok=True)
    staging = Path(tempfile.mkdtemp(prefix=f".{output.name}-", dir=output.parent))
    try:
        for item in parts:
            if item.content is not None:
                write_part(item.content, staging / item.file)
        dump_yaml(staging / "byte-map.yaml", byte_map)
        staging.rename(output)
    except BaseException:
        shutil.rmtree(staging, ignore_errors=True)
        raise


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("output", type=Path, help="new folder to write")
    parser.add_argument("--version", choices=("us", "jp"), default="us")
    parser.add_argument("--config", type=Path, help="config folder (default config/<version>)")
    parser.add_argument("--assets", type=Path, help="splat assets (default assets/<version>)")
    args = parser.parse_args()
    inputs = Inputs(args.version, args.config or REPO_ROOT / "config" / args.version,
                    args.assets or REPO_ROOT / "assets" / args.version)
    try:
        extract(inputs, args.output)
    except (OSError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
