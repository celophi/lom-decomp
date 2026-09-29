#!/usr/bin/env python3
"""Export GOLEM's logic-grid editor artwork, text and panel tables.

The build links one unchanged data blob. Readers describe its resources as
Parts, and the byte map also records padding and the zero-filled editor state.
JP text uses CLOAD's character chart, as FIELD and GNAME do.

Glyph metrics have no palette field: the caller chooses a palette at runtime.
Each glyph preview therefore shows all stored palettes in a four-column grid.

Example:

    python3 -m tools.overlays.golem --version us assets/exports/us/overlays/golem
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

from tools.assets.psx_tim import parse_embedded_tim
from tools.overlays import cload, splat_config, symbols, text_table
from tools.overlays.glyph_texture import GLYPH_RECORD, Glyph, Image
from tools.overlays.card_data import Chart
from tools.overlays.resources import (
    Blob, Part, byte_map_entry, cover_gaps, dump_yaml, hex_address, write_part,
)

REPO_ROOT = Path(__file__).resolve().parents[2]
OVERLAY_CONFIG = "overlays/GOLEM.BIN.yaml"
SYMBOL_FILE = "symbols/golem_symbol_addrs.txt"
SOURCE = REPO_ROOT / "src/overlays/golem/golem.c"
PANEL_COUNT = 154
GLYPH_COUNT = 82
PANEL_RECORD = struct.Struct("<II6H")


# ---------------------------------------------------------------------------
# Inputs and decoded parts


@dataclass(frozen=True)
class GolemSymbols:
    """Resource boundaries in their stored order."""

    image: int
    text: int
    glyphs: int
    panels: int
    variables: int

    @classmethod
    def load(cls, path: Path) -> GolemSymbols:
        named = symbols.load(path)
        missing = [name for name in SYMBOL_NAMES.values() if name not in named]
        if missing:
            raise ValueError(
                f"{path} has no {', '.join(missing)}. If a symbol was renamed, "
                "update SYMBOL_NAMES in tools/overlays/golem.py."
            )
        return cls(**{key: named[name] for key, name in SYMBOL_NAMES.items()})


SYMBOL_NAMES = {
    "image": "g_golem_ui_image",
    "text": "g_golem_text_archive",
    "glyphs": "g_golem_glyph_metrics",
    "panels": "g_golem_panel_records",
    "variables": "g_golem_render_buffers",
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
class TextArchive:
    address: int
    raw: bytes
    sections: tuple[dict[str, object], ...]


def read_bytes(blob: Blob, address: int, end: int, what: str) -> bytes:
    start, stop = blob.offset(address), blob.offset(end)
    if not 0 <= start <= stop <= len(blob.data):
        raise ValueError(f"{what} is outside {blob.file_name}")
    return blob.data[start:stop]


def panel_behavior_names() -> dict[int, str]:
    """Read the behavior names from their owning C enum."""
    source = SOURCE.read_text(encoding="ascii")
    match = re.search(r"typedef enum\s*\{([^}]+)\}\s*GolemPanelBehavior;", source)
    if match is None:
        raise ValueError(f"{SOURCE} has no GolemPanelBehavior")
    return {int(value): name for name, value in re.findall(r"(GOLEM_PANEL_\w+)\s*=\s*(\d+)", match[1])}


# ---------------------------------------------------------------------------
# Reading the blob


def read_image(blob: Blob[GolemSymbols], glyphs: tuple[Glyph, ...]) -> list[Part]:
    raw = read_bytes(blob, blob.symbols.image, blob.symbols.text, "editor TIM")
    tim = parse_embedded_tim(raw, trailing_duplicate_word=True)
    if tim.pixel_mode != 0 or tim.clut is None or not tim.clut.payload or len(tim.clut.payload) % 32:
        raise ValueError("editor TIM must have 4-bit pixels and complete 16-color palettes")
    for index, glyph in enumerate(glyphs):
        if glyph.u + glyph.width > tim.pixels.width_words * 4 or glyph.v + glyph.height > tim.pixels.height:
            raise ValueError(f"glyph {index} lies outside the editor TIM")
    start = blob.offset(blob.symbols.image)
    end = start + len(tim.to_bytes())
    return [
        Part("editor TIM", start, end, "image/image.yaml", Image(blob.symbols.image, tim, glyphs)),
        Part("TIM trailing word", end, end + 4, note="Repeats the TIM's last word."),
    ]


def read_text(blob: Blob[GolemSymbols], chart: Chart | None) -> Part:
    """The archive has two section-relative offset tables: names and descriptions."""
    address = blob.symbols.text
    raw = read_bytes(blob, address, blob.symbols.glyphs, "text archive")
    if len(raw) < 12:
        raise ValueError("text archive header is truncated")
    count, names, descriptions = struct.unpack_from("<3I", raw)
    if count != 2 or names != 12 or not names < descriptions < len(raw):
        raise ValueError("invalid text archive section offsets")
    if blob.version == "jp" and chart is None:
        raise ValueError("JP GOLEM text needs CLOAD's character chart")
    sections = []
    for name, start, end in (("names", names, descriptions), ("descriptions", descriptions, len(raw))):
        table = text_table.parse(raw[start:end], 0, text_table.TWO_BYTE_CODES[blob.version])
        entries = [{
            "index": entry.index, "offset": f"0x{entry.offset:X}",
            "text": chart.decode(entry.data) if chart else entry.text,
            "bytes": entry.data.hex(" "),
        } for entry in table.entries]
        sections.append({"name": name, "offset": f"0x{start:X}",
                         "address": hex_address(address + start), "entries": entries})
    if len(sections[0]["entries"]) != len(sections[1]["entries"]):
        raise ValueError("logic-block names and descriptions have different entry counts")
    start = blob.offset(address)
    return Part("text archive", start, start + len(raw), "text/archive.yaml", TextArchive(address, raw, tuple(sections)))


def read_glyphs(blob: Blob[GolemSymbols]) -> tuple[Part, tuple[Glyph, ...]]:
    address = blob.symbols.glyphs
    raw = read_bytes(blob, address, blob.symbols.panels, "glyph metrics")
    if len(raw) != GLYPH_COUNT * GLYPH_RECORD.size:
        raise ValueError(f"expected {GLYPH_COUNT} complete glyph records")
    glyphs = tuple(Glyph(*values) for values in GLYPH_RECORD.iter_unpack(raw))
    values = {"record_format": GLYPH_RECORD.format,
              "entries": [{"index": index, **glyph.document()} for index, glyph in enumerate(glyphs)],
              "note": "The caller supplies a palette. Zero-size entries remain in the table but have no PNG."}
    start = blob.offset(address)
    return Part("glyph metrics", start, start + len(raw), "tables/glyph_metrics.yaml", Table(address, values, raw)), glyphs


def read_panels(blob: Blob[GolemSymbols]) -> Part:
    """Decode the packed fields used by golem_draw_panel, keeping the original words."""
    address = blob.symbols.panels
    raw = read_bytes(blob, address, blob.symbols.variables, "panel records")
    if len(raw) != PANEL_COUNT * PANEL_RECORD.size:
        raise ValueError(f"expected {PANEL_COUNT} complete panel records")
    behaviors = panel_behavior_names()
    entries = []
    for index, (attributes, texture, high, x, y, width, height, reserved) in enumerate(PANEL_RECORD.iter_unpack(raw)):
        behavior = (attributes >> 3) & 15
        entries.append({
            "index": index, "attributes": hex_address(attributes), "texture": hex_address(texture),
            "blend_mode": attributes & 3, "semi_transparent": bool(attributes & 4),
            "behavior": behavior, "behavior_name": behaviors.get(behavior, f"0x{behavior:X}"),
            "flash_frames": (attributes >> 7) & 15,
            "u": (attributes >> 11) & 255, "v": (texture >> 3) & 255,
            "palette": (texture >> 11) & 63, "cell_width": (texture >> 17) & 511,
            "cell_height": ((high & 7) << 6) | (texture >> 26),
            "x": x, "y": y, "width": width, "height": height,
            "texture_height_high": high, "reserved": reserved,
        })
    values = {
        "record_format": PANEL_RECORD.format, "entries": entries,
        "note": "Screen rectangles repeat the source cell. The renderer adds 8 to X; "
                "visibility and flash tint depend on editor state. Raw words and reserved fields are retained.",
    }
    start = blob.offset(address)
    return Part("panel records", start, start + len(raw), "tables/panels.yaml", Table(address, values, raw))


def read_variables(blob: Blob[GolemSymbols]) -> Part:
    start = blob.offset(blob.symbols.variables)
    if any(blob.data[start:]):
        return Part("editor state", start, len(blob.data), "unknown/variables.bin", blob.data[start:])
    return Part("editor state", start, len(blob.data), note="Runtime variables and buffers; all zero on disc, not exported.")


def read_blob(blob: Blob[GolemSymbols], chart: Chart | None = None) -> list[Part]:
    glyph_part, glyphs = read_glyphs(blob)
    parts = read_image(blob, glyphs)
    parts.append(read_text(blob, chart))
    parts.append(glyph_part)
    parts.append(read_panels(blob))
    parts.append(read_variables(blob))
    return cover_gaps(blob, parts)


# ---------------------------------------------------------------------------
# Writing


@write_part.register
def _write_table(content: Table, path: Path) -> None:
    dump_yaml(path, {"address": hex_address(content.address), **content.values, "bytes": content.raw.hex(" ")})


@write_part.register
def _write_text(content: TextArchive, path: Path) -> None:
    sections = [{"name": section["name"], "offset": section["offset"], "file": f"{section['name']}.yaml"}
                for section in content.sections]
    dump_yaml(path, {"address": hex_address(content.address), "file": "archive.bin", "section_count": 2, "sections": sections})
    (path.parent / "archive.bin").write_bytes(content.raw)
    for section in content.sections:
        dump_yaml(path.parent / f"{section['name']}.yaml", section)


# ---------------------------------------------------------------------------
# Putting it together


def load_blob(inputs: Inputs) -> Blob[GolemSymbols]:
    names = GolemSymbols.load(inputs.config / SYMBOL_FILE)
    files = splat_config.data_files(inputs.config / OVERLAY_CONFIG, inputs.assets)
    source = splat_config.file_containing(files, names.image, SYMBOL_NAMES["image"])
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


def load_text_chart(inputs: Inputs) -> Chart | None:
    if inputs.version != "jp":
        return None
    return Chart(cload.load_blob(cload.Inputs(inputs.version, inputs.config, inputs.assets)))


def extract(inputs: Inputs, output: Path) -> None:
    """Read everything, write a temporary folder, then move the finished export into place."""
    if output.exists():
        raise FileExistsError(f"{output} already exists")
    blob = load_blob(inputs)
    chart = load_text_chart(inputs)
    parts = read_blob(blob, chart)
    byte_map = {
        "source": blob.file_name, "address": hex_address(blob.address),
        "size": f"0x{len(blob.data):X}", "ranges": [byte_map_entry(blob, part) for part in parts],
    }
    if chart:
        byte_map["text_decoder"] = {
            "source": chart.blob.file_name, "overlay": "CLOAD",
            "address": hex_address(chart.blob.address + chart.start), "size": f"0x{chart.end - chart.start:X}",
            "note": "JP text uses CLOAD's character chart. GOLEM does not contain this conversion table.",
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
