#!/usr/bin/env python3
"""Export WSEL's play-area screen: its eight TIMs and the land-map tables.

The build links one unchanged data blob. Readers describe its resources as
Parts, and the byte map also records padding and the zero-filled screen state.

wsel_load_resources uploads TIM n to sprite layer n, and wsel_upload_tim sends
pixels and palette to that layer's VRAM coordinates, not to the TIM's own. Each
image therefore lists its layer's destination next to the unchanged TIM. The
layer names come from the WSEL_SPRITE_* defines in wsel.c.

Example:

    python3 -m tools.data.overlays.wsel --version us assets/exports/us/overlays/wsel
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import re
import shutil
import struct
import sys
import tempfile

from tools.data.formats.psx_tim import PIXEL_MODE_NAMES, TimImage, parse_embedded_tim
from tools.data.overlays import icon_set, png, splat_config, symbols
from tools.data.overlays.resources import (
    Blob, Part, byte_map_entry, cover_gaps, dump_yaml, hex_address, write_part,
)

REPO_ROOT = Path(__file__).resolve().parents[3]
OVERLAY_CONFIG = "overlays/WSEL.BIN.yaml"
SYMBOL_FILE = "symbols/wsel_symbol_addrs.txt"
SOURCE = REPO_ROOT / "src/overlays/wsel/wsel.c"
MAP_CELLS = 19
SUBCELLS = 6
CELL_SIZE = 16
SPRITE_COUNT = 8
HERO_POSE_COUNT = 12
POSE_UNIT = 8
SPRITE_RECORD = struct.Struct("<4B10H")
SPRITE_FIELDS = ("tpage_mode", "blend_mode", "semi_trans", "brightness", "tpage_x", "tpage_y",
                 "clut_x", "clut_y", "u", "v", "width", "height", "x", "y")
POSE_RECORD = struct.Struct("<6B")
POSE_FIELDS = ("u", "v", "width", "height", "x_offset", "y_offset")
BLEND_MODES = {0: "average", 1: "add", 2: "subtract", 3: "add quarter"}
TIM_KEYS = ("land_map", "world_map", "cursor", "world_overlay",
            "hero_default", "hero_alternate", "hero_shadow", "prompt")


# ---------------------------------------------------------------------------
# Inputs and decoded parts


@dataclass(frozen=True)
class WselSymbols:
    """Resource boundaries in their stored order."""

    land_map: int
    world_map: int
    cursor: int
    world_overlay: int
    hero_default: int
    hero_alternate: int
    hero_shadow: int
    prompt: int
    occupied: int
    tile_masks: int
    sprites: int
    poses_default: int
    poses_alternate: int
    variables: int

    @classmethod
    def load(cls, path: Path) -> WselSymbols:
        named = symbols.load(path)
        missing = [name for name in SYMBOL_NAMES.values() if name not in named]
        if missing:
            raise ValueError(
                f"{path} has no {', '.join(missing)}. If a symbol was renamed, "
                "update SYMBOL_NAMES in tools/data/overlays/wsel.py."
            )
        return cls(**{key: named[name] for key, name in SYMBOL_NAMES.items()})


SYMBOL_NAMES = {
    "land_map": "g_wsel_land_map_tim",
    "world_map": "g_wsel_world_map_tim",
    "cursor": "g_wsel_cursor_tim",
    "world_overlay": "g_wsel_world_overlay_tim",
    "hero_default": "g_wsel_hero_tim_default",
    "hero_alternate": "g_wsel_hero_tim_alternate",
    "hero_shadow": "g_wsel_hero_shadow_tim",
    "prompt": "g_wsel_prompt_tim",
    "occupied": "g_wsel_cell_occupied",
    "tile_masks": "g_wsel_cell_edges",
    "sprites": "g_wsel_sprites",
    "poses_default": "g_wsel_hero_poses_default",
    "poses_alternate": "g_wsel_hero_poses_alternate",
    "variables": "g_wsel_render_context",
}


@dataclass(frozen=True)
class Inputs:
    version: str
    config: Path
    assets: Path


@dataclass(frozen=True)
class Table:
    address: int
    values: dict[str, object]
    raw: bytes


@dataclass(frozen=True)
class Image:
    """One original TIM, the sprite layer it is uploaded to, and any pose crops."""

    address: int
    tim: TimImage
    layer: dict[str, object]
    poses: tuple[dict[str, int], ...] = ()


def read_bytes(blob: Blob, address: int, end: int, what: str) -> bytes:
    start, stop = blob.offset(address), blob.offset(end)
    if not 0 <= start <= stop <= len(blob.data):
        raise ValueError(f"{what} is outside {blob.file_name}")
    return blob.data[start:stop]


def table_part(blob: Blob, name: str, address: int, raw: bytes, values: dict) -> Part:
    start = blob.offset(address)
    return Part(name, start, start + len(raw), f"tables/{name}.yaml", Table(address, values, raw))


def sprite_layer_names() -> dict[int, str]:
    """Read the layer names from the WSEL_SPRITE_* block in wsel.c."""
    source = SOURCE.read_text(encoding="ascii")
    match = re.search(r"/\* Sprite layers in g_wsel_sprites[^*]*\*/\n((?:#define WSEL_SPRITE_\w+ \d+\n)+)", source)
    if match is None:
        raise ValueError(f"{SOURCE} has no sprite layer defines")
    return {int(value): name for name, value in re.findall(r"#define (WSEL_SPRITE_\w+) (\d+)", match[1])}


def palette_size(tim: TimImage) -> int:
    return 16 if tim.pixel_mode == 0 else 256


def image_width(tim: TimImage) -> int:
    return tim.pixels.width_words * (4 if tim.pixel_mode == 0 else 2)


def pixel_indices(tim: TimImage) -> list[int]:
    if tim.pixel_mode == 0:
        return [nibble for byte in tim.pixels.payload for nibble in (byte & 15, byte >> 4)]
    return list(tim.pixels.payload)


# ---------------------------------------------------------------------------
# Reading the blob, in address order


def read_sprites(blob: Blob[WselSymbols]) -> tuple[Part, tuple[dict[str, object], ...]]:
    address = blob.symbols.sprites
    raw = read_bytes(blob, address, blob.symbols.poses_default, "sprite layers")
    if len(raw) != SPRITE_COUNT * SPRITE_RECORD.size:
        raise ValueError(f"expected {SPRITE_COUNT} complete sprite layer records")
    names = sprite_layer_names()
    layers = tuple(
        {"index": index, "name": names.get(index, f"layer {index}"), **dict(zip(SPRITE_FIELDS, values)),
         "blend": BLEND_MODES[values[1] & 3]}
        for index, values in enumerate(SPRITE_RECORD.iter_unpack(raw))
    )
    values = {
        "record_format": SPRITE_RECORD.format,
        "entries": [{**layer, "image": f"images/{TIM_KEYS[layer['index']]}/image.yaml"} for layer in layers],
        "note": "tpage_x/tpage_y and clut_x/clut_y are where wsel_upload_tim puts the layer's TIM. "
                "tpage_mode 0 is 4-bit, 1 is 8-bit. brightness 128 is neutral. Layers 4 to 6 are drawn "
                "by wsel_draw_hero, so their width and height stay 0. The screen updates brightness, "
                "semi_trans, blend_mode and x/y of layers 0 and 2 at runtime.",
    }
    return table_part(blob, "sprite_layers", address, raw, values), layers


def read_poses(blob: Blob[WselSymbols]) -> tuple[list[Part], dict[str, tuple[dict[str, int], ...]]]:
    names = blob.symbols
    parts, poses = [], {}
    for key, address, end in (("default", names.poses_default, names.poses_alternate),
                              ("alternate", names.poses_alternate, names.variables)):
        raw = read_bytes(blob, address, end, f"{key} hero poses")
        if len(raw) != HERO_POSE_COUNT * POSE_RECORD.size:
            raise ValueError(f"expected {HERO_POSE_COUNT} complete {key} hero poses")
        entries = tuple({"index": index, **dict(zip(POSE_FIELDS, values))}
                        for index, values in enumerate(POSE_RECORD.iter_unpack(raw)))
        poses[key] = entries
        values = {
            "record_format": POSE_RECORD.format, "units": f"{POSE_UNIT} pixels",
            "entries": [{**entry, "file": f"images/hero_{key}/poses/{entry['index']:02d}.png"}
                        if entry["width"] and entry["height"] else entry for entry in entries],
            "note": "wsel_draw_hero draws pose 0 only. It places the cell at the layer's x + 32 - x_offset * 8, "
                    "y + 40 - y_offset * 8.",
        }
        parts.append(table_part(blob, f"hero_poses_{key}", address, raw, values))
    return parts, poses


def read_images(blob: Blob[WselSymbols], layers: tuple[dict[str, object], ...],
                poses: dict[str, tuple[dict[str, int], ...]]) -> list[Part]:
    addresses = [getattr(blob.symbols, key) for key in TIM_KEYS] + [blob.symbols.occupied]
    parts = []
    for index, key in enumerate(TIM_KEYS):
        address = addresses[index]
        raw = read_bytes(blob, address, addresses[index + 1], f"{key} TIM")
        tim = parse_embedded_tim(raw, trailing_duplicate_word=True)
        if tim.pixel_mode not in (0, 1) or tim.clut is None or not tim.clut.payload \
                or len(tim.clut.payload) % (palette_size(tim) * 2):
            raise ValueError(f"{key} TIM must have 4- or 8-bit pixels and complete palettes")
        layer = layers[index]
        if (layer["tpage_mode"] == 0) != (tim.pixel_mode == 0):
            raise ValueError(f"{key} TIM depth differs from sprite layer {index}")
        crops = poses.get(key.removeprefix("hero_"), ()) if key.startswith("hero_") else ()
        width, height = image_width(tim), tim.pixels.height
        for pose in crops:
            if (pose["u"] + pose["width"]) * POSE_UNIT > width or (pose["v"] + pose["height"]) * POSE_UNIT > height:
                raise ValueError(f"{key} pose {pose['index']} lies outside its TIM")
        start = blob.offset(address)
        end = start + len(tim.to_bytes())
        parts.append(Part(f"{key} TIM", start, end, f"images/{key}/image.yaml", Image(address, tim, layer, crops)))
        parts.append(Part(f"{key} TIM trailing word", end, end + 4, note="Repeats the TIM's last word."))
    return parts


def read_grid(blob: Blob[WselSymbols]) -> list[Part]:
    names = blob.symbols
    cells = MAP_CELLS * MAP_CELLS
    raw = read_bytes(blob, names.occupied, names.tile_masks, "occupied cells")
    if len(raw) < cells:
        raise ValueError(f"expected {cells} occupied-cell flags")
    raw = raw[:cells]
    rows = ["".join("1" if raw[row * MAP_CELLS + column] else "0" for column in range(MAP_CELLS))
            for row in range(MAP_CELLS)]
    parts = [table_part(blob, "cell_occupied", names.occupied, raw, {
        "columns": MAP_CELLS, "rows": rows,
        "note": "One byte per cursor cell, row by row. Nonzero cells are refused with an error sound and "
                "darkened as a whole. The cursor cell is (scroll + cursor - 16) / 16 on each axis.",
    })]
    size = SUBCELLS * SUBCELLS
    raw = read_bytes(blob, names.tile_masks, names.sprites, "cell tile masks")
    if len(raw) != cells * size:
        raise ValueError(f"expected {cells} complete cell tile masks")
    entries = []
    for cell in range(cells):
        mask = raw[cell * size : (cell + 1) * size]
        entries.append({
            "index": cell, "column": cell % MAP_CELLS, "row": cell // MAP_CELLS,
            "tiles": ["".join("1" if mask[row * SUBCELLS + column] else "0" for column in range(SUBCELLS))
                      for row in range(SUBCELLS)],
        })
    parts.append(table_part(blob, "cell_tile_masks", names.tile_masks, raw, {
        "tiles_per_side": SUBCELLS, "tile_size": CELL_SIZE, "entries": entries,
        "note": "One 6 x 6 mask per cursor cell for the 96-pixel square that starts there. While Square is "
                "held on a free cell, tiles stored as 0 are brightened. One more row and column are drawn past "
                "the square: from the last row of the cell below and the last column of the cell to the right.",
    }))
    return parts


def read_variables(blob: Blob[WselSymbols]) -> Part:
    start = blob.offset(blob.symbols.variables)
    if any(blob.data[start:]):
        return Part("screen state", start, len(blob.data), "unknown/variables.bin", blob.data[start:])
    return Part("screen state", start, len(blob.data),
                note="Runtime variables and the music sequence buffer; all zero on disc, not exported.")


def read_blob(blob: Blob[WselSymbols]) -> list[Part]:
    sprite_part, layers = read_sprites(blob)
    pose_parts, poses = read_poses(blob)
    parts = read_images(blob, layers, poses)
    parts += read_grid(blob)
    parts.append(sprite_part)
    parts += pose_parts
    parts.append(read_variables(blob))
    return cover_gaps(blob, parts)


# ---------------------------------------------------------------------------
# Writing


@write_part.register
def _write_table(content: Table, path: Path) -> None:
    dump_yaml(path, {"address": hex_address(content.address), **content.values, "bytes": content.raw.hex(" ")})


@write_part.register
def _write_image(content: Image, path: Path) -> None:
    tim, layer = content.tim, content.layer
    width, height = image_width(tim), tim.pixels.height
    colors_per_palette = palette_size(tim)
    palettes = tuple(struct.iter_unpack(f"<{colors_per_palette}H", tim.clut.payload))
    indices = pixel_indices(tim)
    path.parent.mkdir(parents=True, exist_ok=True)
    (path.parent / "image.tim").write_bytes(tim.to_bytes())
    previews = []
    for index, palette in enumerate(palettes):
        colors = [icon_set.bgr555_to_rgba(value) for value in palette]
        file = f"palette_{index:02X}.png"
        png.write_rgba(path.parent / file, width, height, [colors[value] for value in indices])
        previews.append({"index": index, "file": file})
    poses = []
    if content.poses:
        colors = [icon_set.bgr555_to_rgba(value) for value in palettes[0]]
        (path.parent / "poses").mkdir()
        for pose in content.poses:
            if not pose["width"] or not pose["height"]:
                continue
            left, top = pose["u"] * POSE_UNIT, pose["v"] * POSE_UNIT
            crop_width, crop_height = pose["width"] * POSE_UNIT, pose["height"] * POSE_UNIT
            pixels = [colors[indices[y * width + x]]
                      for y in range(top, top + crop_height) for x in range(left, left + crop_width)]
            file = f"poses/{pose['index']:02d}.png"
            png.write_rgba(path.parent / file, crop_width, crop_height, pixels)
            poses.append({"index": pose["index"], "file": file})
    document = {
        "address": hex_address(content.address), "file": "image.tim",
        "width": width, "height": height, "depth": PIXEL_MODE_NAMES[tim.pixel_mode],
        "stored_layout": tim.metadata(),
        "sprite_layer": {"index": layer["index"], "name": layer["name"]},
        "uploaded_to": {"pixels": [layer["tpage_x"], layer["tpage_y"]], "palette": [layer["clut_x"], layer["clut_y"]]},
        "palettes": previews,
        "note": "Palette value zero is transparent; other colors are shown opaque. The game ignores the TIM's "
                "own VRAM coordinates and uploads to uploaded_to, with the whole palette block as one row.",
    }
    if poses:
        document["poses"] = poses
    dump_yaml(path, document)


# ---------------------------------------------------------------------------
# Putting it together


def load_blob(inputs: Inputs) -> Blob[WselSymbols]:
    names = WselSymbols.load(inputs.config / SYMBOL_FILE)
    files = splat_config.data_files(inputs.config / OVERLAY_CONFIG, inputs.assets)
    source = splat_config.file_containing(files, names.land_map, SYMBOL_NAMES["land_map"])
    if not source.path.exists():
        raise ValueError(f"{source.path} is missing; run make splat first")
    data = source.path.read_bytes()
    if len(data) != source.end - source.start:
        raise ValueError(f"{source.path} size does not match the splat config; run make splat again")
    previous = source.start - 1
    for key in SYMBOL_NAMES:
        address = getattr(names, key)
        if not source.contains(address):
            raise ValueError(f"{SYMBOL_NAMES[key]} is outside {source.path}")
        if address <= previous:
            raise ValueError(f"{SYMBOL_NAMES[key]} is out of resource order")
        previous = address
    return Blob(data, source.start, source.path.name, inputs.version, names)


def extract(inputs: Inputs, output: Path) -> None:
    """Read everything, write a temporary folder, then move the finished export into place."""
    if output.exists():
        raise FileExistsError(f"{output} already exists")
    blob = load_blob(inputs)
    parts = read_blob(blob)
    byte_map = {
        "source": blob.file_name, "address": hex_address(blob.address),
        "size": f"0x{len(blob.data):X}", "ranges": [byte_map_entry(blob, part) for part in parts],
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
