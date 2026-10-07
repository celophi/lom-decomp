"""Shared UI atlas and glyph previews for GOLEM and GOSUB.

Both use the same eight-byte glyph layout. A glyph's palette comes from its
caller, so the preview shows every stored palette in a four-column grid.
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import struct

from tools.data.formats.psx_tim import TimImage
from tools.data.overlays import icon_set, png
from tools.data.overlays.resources import dump_yaml, hex_address, write_part

GLYPH_RECORD = struct.Struct("<4B2H")
PALETTE_COLUMNS = 4


@dataclass(frozen=True)
class Glyph:
    """These glyphs use halfword dimensions and separate reserved bytes beside U/V."""

    u: int
    reserved_01: int
    v: int
    reserved_03: int
    width: int
    height: int

    def document(self) -> dict[str, int]:
        return {"u": self.u, "v": self.v, "width": self.width, "height": self.height,
                "reserved_01": self.reserved_01, "reserved_03": self.reserved_03}


@dataclass(frozen=True)
class Image:
    address: int
    tim: TimImage
    glyphs: tuple[Glyph, ...]


def glyph_preview(indices: list[int], image_width: int, glyph: Glyph, palettes: list[list[tuple]]) -> list[tuple]:
    """Show the same source rectangle through each palette, in row-major order."""
    sheet_width = glyph.width * PALETTE_COLUMNS
    rows = (len(palettes) + PALETTE_COLUMNS - 1) // PALETTE_COLUMNS
    pixels = [(0, 0, 0, 0)] * (sheet_width * glyph.height * rows)
    for index, colors in enumerate(palettes):
        row, column = divmod(index, PALETTE_COLUMNS)
        for y in range(glyph.height):
            source = (glyph.v + y) * image_width + glyph.u
            target = (row * glyph.height + y) * sheet_width + column * glyph.width
            pixels[target : target + glyph.width] = [colors[value] for value in indices[source : source + glyph.width]]
    return pixels


@write_part.register
def _write_image(content: Image, path: Path) -> None:
    tim = content.tim
    width, height = tim.pixels.width_words * 4, tim.pixels.height
    words = tuple(struct.iter_unpack("<16H", tim.clut.payload))
    palettes = [[icon_set.bgr555_to_rgba(value) for value in palette] for palette in words]
    indices = [nibble for byte in tim.pixels.payload for nibble in (byte & 15, byte >> 4)]
    path.parent.mkdir(parents=True, exist_ok=True)
    (path.parent / "image.tim").write_bytes(tim.to_bytes())
    previews = []
    for index, colors in enumerate(palettes):
        file = f"palette_{index:02X}.png"
        png.write_rgba(path.parent / file, width, height, [colors[value] for value in indices])
        previews.append({"index": index, "file": file, "colors": [f"0x{word:04X}" for word in words[index]]})
    glyphs = []
    (path.parent / "glyphs").mkdir()
    rows = (len(palettes) + PALETTE_COLUMNS - 1) // PALETTE_COLUMNS
    for index, glyph in enumerate(content.glyphs):
        entry = {"index": index, **glyph.document()}
        if glyph.width and glyph.height:
            file = f"glyphs/{index:02d}.png"
            pixels = glyph_preview(indices, width, glyph, palettes)
            png.write_rgba(path.parent / file, glyph.width * PALETTE_COLUMNS, glyph.height * rows, pixels)
            entry["file"] = file
        glyphs.append(entry)
    dump_yaml(path, {
        "address": hex_address(content.address), "file": "image.tim", "width": width, "height": height,
        "stored_layout": tim.metadata(), "palettes": previews, "glyphs": glyphs,
        "glyph_preview_grid": {"columns": PALETTE_COLUMNS, "rows": rows,
                               "order": "Palette indices increase left to right, then top to bottom."},
        "note": "Glyph palettes are chosen at runtime. These previews show every stored palette, "
                "with zero transparent and other colors opaque; runtime tinting and blending are not applied. "
                "The UI uploads the pixels at (320, 0) and the 256 colors in one row at (0, 498). "
                "The original TIM coordinates and palette words are preserved.",
    })
