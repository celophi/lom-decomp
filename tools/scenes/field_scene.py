#!/usr/bin/env python3
"""Extract complete assets and common chest records from ANA/INFO_* scene IMGs.

The header locates the scene sections. Each section parser returns byte ranges;
extraction writes chest records as YAML, copies other assets unchanged, and
saves the gaps as unknown data.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import json
from pathlib import Path
import struct
from typing import NamedTuple

from tools.assets.psx_tim import TimImage


SCENE_HEADER = struct.Struct("<10I")
LAYOUT_RECORD = struct.Struct("<IHBBIHH16H")
PORTRAIT_SIZE = 1184

# The common chest initializer sets local state, then reads layout parameters
# scripts[4] and scripts[5]. Other actors using the chest graphics can differ.
COMMON_CHEST_INITIALIZER = bytes.fromhex(
    "40 84 20 c0 "
    "0c 01 02 ff 10 00 08 40 40 e0 "
    "0c 01 02 ff 10 00 09 40 50 e0"
)


@dataclass(frozen=True)
class SceneSection:
    """A named section's absolute IMG byte range; end is exclusive."""

    name: str
    start: int
    end: int


class SceneHeader(NamedTuple):
    """The 40-byte disk header: ten little-endian u32 offsets from IMG byte zero.

    Each section ends where the next begins. Equal offsets mean an empty section.
    These fields follow FIELD_SCENE_* in field_scene_transition.c.
    """

    layout: int          # 0x00: count and 48-byte actor/action records
    event_scripts: int   # 0x04: u16 offsets followed by event bytecode
    strings: int         # 0x08: scene text
    actor_scripts: int   # 0x0C: actor animation scripts
    records: int         # 0x10: general records
    actors: int          # 0x14: actor descriptions
    geometry: int        # 0x18: scene geometry
    images: int          # 0x1C: u32 offset table followed by TIM files
    portraits: int       # 0x20: count followed by palette-and-pixel records
    group_bounds: int    # 0x24: final section, extending to end of file

    @classmethod
    def parse(cls, data: bytes) -> SceneHeader:
        """Read the header and check that its section boundaries fit the IMG."""
        if len(data) < SCENE_HEADER.size:
            raise ValueError("truncated scene header")
        header = cls(*SCENE_HEADER.unpack_from(data))
        if header.layout != SCENE_HEADER.size:
            raise ValueError("unsupported IMG: expected an ANA/INFO_* scene")
        if any(offset % 4 or offset > len(data) for offset in header):
            raise ValueError("section offsets must be aligned and within the file")
        if tuple(header) != tuple(sorted(header)):
            raise ValueError("section offsets must be in file order")
        return header

    def sections(self, file_size: int) -> list[SceneSection]:
        """Pair each section start with its following boundary."""
        return [
            SceneSection("layout", self.layout, self.event_scripts),
            SceneSection("event_scripts", self.event_scripts, self.strings),
            SceneSection("strings", self.strings, self.actor_scripts),
            SceneSection("actor_scripts", self.actor_scripts, self.records),
            SceneSection("records", self.records, self.actors),
            SceneSection("actors", self.actors, self.geometry),
            SceneSection("geometry", self.geometry, self.images),
            SceneSection("images", self.images, self.portraits),
            SceneSection("portraits", self.portraits, self.group_bounds),
            SceneSection("group_bounds", self.group_bounds, file_size),
        ]


@dataclass(frozen=True)
class LayoutRecord:
    """One 48-byte FieldActionRequest, all multi-byte values little-endian.

    The layout section starts with a u32 count, followed by these records.
    See include/field_interaction_start.h. scripts[15] selects the initializer;
    for the common chest script, scripts[4] is the item and scripts[5] holds the
    collection flag, with bit 15 selecting the alternate facing.
    """

    control: int             # 0x00: u32; bits 4-7 select the action kind
    condition_variable: int  # 0x04: u16 saved-variable reference
    condition_minimum: int   # 0x06: u8
    condition_maximum: int   # 0x07: u8
    position: int            # 0x08: u32; X in bits 0-15, Z in bits 16-26
    source: int              # 0x0C: u16; low three bits select actor resources
    enabled_events: int      # 0x0E: u16 event mask
    scripts: tuple[int, ...]  # 0x10: sixteen u16 script references or parameters

    @classmethod
    def parse(cls, data: bytes, offset: int) -> LayoutRecord:
        """Read one record at an absolute IMG offset, after validating its section."""
        control, variable, minimum, maximum, position, source, events, *scripts = (
            LAYOUT_RECORD.unpack_from(data, offset)
        )
        return cls(control, variable, minimum, maximum, position, source, events, tuple(scripts))

    def is_common_chest(self, event_scripts: bytes) -> bool:
        """Require both the chest resource selector and the known initializer."""
        kind = (self.control >> 4) & 0xF
        # Actor and grouped-actor records use resource selector 5 for chest graphics.
        if kind not in (0, 7) or (self.source & 7) != 5:
            return False
        initializer = self.scripts[15]
        if initializer == 0xFFFF or (initializer & 0x8000) == 0:
            return False
        table_offset = (initializer & 0x7FFF) * 2
        if table_offset + 2 > len(event_scripts):
            return False
        script_offset = struct.unpack_from("<H", event_scripts, table_offset)[0]
        # Script offsets are section-relative. Entry zero is not a script count.
        if script_offset < table_offset + 2:
            return False
        return event_scripts[script_offset:].startswith(COMMON_CHEST_INITIALIZER)


    def chest_yaml(self, record_bytes: bytes) -> str:
        """Write chest details and retain the original record for byte recovery."""
        lines = [
            f"x: {self.position & 0xFFFF}",
            f"z: {(self.position >> 16) & 0x7FF}",
            f"item_id: 0x{self.scripts[4]:04X}",
            f"collection_flag: 0x{self.scripts[5] & 0x7FFF:04X}",
            f"alternate_facing: {'true' if self.scripts[5] & 0x8000 else 'false'}",
            "# Original 48 bytes, including fields not shown above.",
            f'record_bytes: "{record_bytes.hex()}"',
        ]
        return "\n".join(lines) + "\n"


@dataclass(frozen=True)
class AssetRange:
    """An extraction result, not a disk structure. Each file represents one IMG range.

    offset is absolute in the IMG; size is in bytes. chest is present only when
    a layout record has been recognized as a common chest and is written as YAML.
    """

    name: str
    offset: int
    size: int
    filename: str
    chest: LayoutRecord | None = None


def read_chests(data: bytes, header: SceneHeader) -> list[AssetRange]:
    """Extract recognized chest records from the counted layout array."""
    layout = data[header.layout:header.event_scripts]
    if len(layout) < 4:
        raise ValueError("missing layout record count")
    count = struct.unpack_from("<I", layout)[0]
    if len(layout) != 4 + count * LAYOUT_RECORD.size:
        raise ValueError("layout count does not match the section size")
    event_scripts = data[header.event_scripts:header.strings]
    chests = []
    for index in range(count):
        offset = header.layout + 4 + index * LAYOUT_RECORD.size
        record = LayoutRecord.parse(data, offset)
        if record.is_common_chest(event_scripts):
            chests.append(AssetRange(
                "chest", offset, LAYOUT_RECORD.size, f"chests/{index:03d}.yaml", record,
            ))
    return chests


def read_textures(data: bytes, header: SceneHeader) -> list[AssetRange]:
    """Read the image section's u32 offset table and validate each complete TIM.

    Offsets are relative to this section. The first offset also gives the table
    size; there is no separate count. Repeated references share one TIM asset.
    """
    images = data[header.images:header.portraits]
    if not images:
        return []
    table_size = struct.unpack_from("<I", images)[0]
    if table_size < 4 or table_size % 4 or table_size > len(images):
        raise ValueError("invalid image-offset table size")
    offsets = struct.unpack_from(f"<{table_size // 4}I", images)
    if any(offset < table_size or offset >= len(images) for offset in offsets):
        raise ValueError("TIM offset outside the image section")
    boundaries = sorted(set(offsets)) + [len(images)]
    textures = []
    for index, (start, end) in enumerate(zip(boundaries, boundaries[1:])):
        TimImage.parse(images[start:end])
        textures.append(AssetRange(
            "tim", header.images + start, end - start, f"textures/{index:03d}.tim",
        ))
    return textures


def read_portraits(data: bytes, header: SceneHeader) -> list[AssetRange]:
    """Read a u32 count followed by 1,184-byte portrait records.

    Each record has 16 little-endian BGR555 colors at 0x00 and 48x48 packed 4bpp
    pixels at 0x20. It has no TIM header and is extracted as a whole record.
    """
    portraits = data[header.portraits:header.group_bounds]
    if len(portraits) < 4:
        raise ValueError("missing portrait count")
    count = struct.unpack_from("<I", portraits)[0]
    if len(portraits) != 4 + count * PORTRAIT_SIZE:
        raise ValueError("portrait count does not match the section size")
    result = []
    for index in range(count):
        offset = header.portraits + 4 + index * PORTRAIT_SIZE
        result.append(AssetRange(
            "portrait", offset, PORTRAIT_SIZE, f"portraits/{index:03d}.bin",
        ))
    return result


def cover_section(section: SceneSection, assets: list[AssetRange]) -> list[AssetRange]:
    """Keep known assets in file order and preserve the data between them."""
    result = []
    cursor = section.start
    for asset in assets:
        if asset.offset < cursor or asset.offset + asset.size > section.end:
            raise ValueError(f"invalid asset range in {section.name}")
        if cursor < asset.offset:
            result.append(AssetRange(
                "unknown_data", cursor, asset.offset - cursor, f"unknown/{cursor:08X}.bin",
            ))
        result.append(asset)
        cursor = asset.offset + asset.size
    if cursor < section.end:
        result.append(AssetRange(
            "unknown_data", cursor, section.end - cursor, f"unknown/{cursor:08X}.bin",
        ))
    return result


def split_scene(data: bytes) -> list[AssetRange]:
    """Parse the container and return a complete map of its original bytes."""
    header = SceneHeader.parse(data)
    assets_by_section = {
        "layout": read_chests(data, header),
        "images": read_textures(data, header),
        "portraits": read_portraits(data, header),
    }
    result = [AssetRange("header", 0, SCENE_HEADER.size, "header.bin")]
    for section in header.sections(len(data)):
        result.extend(cover_section(section, assets_by_section.get(section.name, [])))
    return result


def extract(source: Path, output: Path) -> None:
    """Validate the IMG, then write its assets and byte map in a new directory."""
    data = source.read_bytes()
    assets = split_scene(data)
    output.mkdir(parents=True, exist_ok=False)
    lines = [f"source: {json.dumps(source.name)}",
             "# Offsets and sizes are bytes in the original IMG.", "sections:"]
    for asset in assets:
        path = output / asset.filename
        path.parent.mkdir(parents=True, exist_ok=True)
        payload = data[asset.offset:asset.offset + asset.size]
        if asset.chest is not None:
            path.write_text(asset.chest.chest_yaml(payload), encoding="ascii")
        else:
            path.write_bytes(payload)
        lines.extend([f"  - name: {asset.name}", f"    offset: 0x{asset.offset:X}",
                      f"    size: {asset.size}", f"    file: {asset.filename}"])
    (output / "byte-map.yaml").write_text("\n".join(lines) + "\n", encoding="ascii")


def main() -> None:
    """Extract one scene into a new output directory."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    try:
        extract(args.source, args.output)
    except (OSError, ValueError) as error:
        parser.exit(1, f"{error}\n")
    print(f"Extracted to {args.output}")


if __name__ == "__main__":
    main()
