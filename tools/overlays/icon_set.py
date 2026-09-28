"""Read save-screen icon sets: a count, byte offsets, then 16-color 48 x 48 images.

Each image is a 16-entry BGR555 palette followed by 4-bit pixels, low nibble
first. Offsets count from the start of the set (the count word).
"""

from __future__ import annotations

from dataclasses import dataclass

ICON_SIZE = 48
PALETTE_COLORS = 16
PALETTE_BYTES = PALETTE_COLORS * 2
PIXEL_BYTES = ICON_SIZE * ICON_SIZE // 2
IMAGE_BYTES = PALETTE_BYTES + PIXEL_BYTES


@dataclass(frozen=True)
class Icon:
    index: int
    offset: int
    palette: tuple[int, ...]
    pixels: bytes

    def rgba(self) -> list[tuple[int, int, int, int]]:
        """Row-major RGBA pixels; palette value 0 is transparent, as on the GPU."""
        colors = [bgr555_to_rgba(value) for value in self.palette]
        out = []
        for byte in self.pixels:
            out.append(colors[byte & 0x0F])
            out.append(colors[byte >> 4])
        return out


@dataclass(frozen=True)
class IconSet:
    icons: tuple[Icon, ...]
    size: int


def bgr555_to_rgba(value: int) -> tuple[int, int, int, int]:
    def expand(channel: int) -> int:
        return (channel << 3) | (channel >> 2)

    red = expand(value & 0x1F)
    green = expand((value >> 5) & 0x1F)
    blue = expand((value >> 10) & 0x1F)
    return (red, green, blue, 0 if value == 0 else 255)


def parse(data: bytes, start: int) -> IconSet:
    """Parse the icon set whose count word is at @p start in @p data."""
    count = int.from_bytes(data[start : start + 4], "little")
    if count == 0 or start + 4 + count * 4 > len(data):
        raise ValueError(f"icon count {count} does not fit the data")
    icons = []
    end = 4 + count * 4
    for index in range(count):
        offset = int.from_bytes(data[start + 4 + index * 4 : start + 8 + index * 4], "little")
        position = start + offset
        if offset < 4 + count * 4 or position + IMAGE_BYTES > len(data):
            raise ValueError(f"icon {index} has an invalid offset 0x{offset:X}")
        palette = tuple(
            int.from_bytes(data[position + color * 2 : position + color * 2 + 2], "little") for color in range(PALETTE_COLORS)
        )
        pixels = data[position + PALETTE_BYTES : position + IMAGE_BYTES]
        icons.append(Icon(index, offset, palette, pixels))
        end = max(end, offset + IMAGE_BYTES)
    return IconSet(tuple(icons), end)
