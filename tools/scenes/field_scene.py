#!/usr/bin/env python3
"""Inspect and extract Legend of Mana FIELD scene IMG files (ANA/INFO_*)."""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

from tools.assets.psx_tim import TimFormatError, TimImage


SECTION_NAMES = (
    "layout", "event_scripts", "strings", "actor_scripts", "records",
    "actors", "geometry", "images", "portraits", "group_bounds",
)
HEADER_SIZE = 40
LAYOUT_ENTRY_SIZE = 48
PORTRAIT_SIZE = 1184
FORMAT_NAME = "lom_field_scene"
FORMAT_VERSION = 1

# The common initializer sets a local flag, then reads scripts[4] and scripts[5]
# into owner-relative variables E040 and E050. This recognizes a known script
# prefix; it does not decode arbitrary scripts or prove reachability.
COMMON_CHEST_INITIALIZER = bytes.fromhex(
    "40 84 20 c0 "
    "0c 01 02 ff 10 00 08 40 40 e0 "
    "0c 01 02 ff 10 00 09 40 50 e0"
)


class SceneFormatError(ValueError):
    """A file does not satisfy the supported FIELD scene layout."""


@dataclass(frozen=True)
class SceneSection:
    """One contiguous section with its absolute source-file offset."""

    index: int
    offset: int
    data: bytes

    @property
    def name(self) -> str:
        return SECTION_NAMES[self.index]

    @property
    def filename(self) -> str:
        return f"sections/{self.index:02d}_{self.name}.bin"

    def metadata(self) -> dict[str, object]:
        """Describe the section using absolute source offsets and relative output paths."""
        return {
            "name": self.name,
            "offset": self.offset,
            "size": len(self.data),
            "sha256": hashlib.sha256(self.data).hexdigest(),
            "file": self.filename,
        }


@dataclass(frozen=True)
class SceneTexture:
    """One TIM referenced by the scene's image-offset table."""

    index: int
    offset: int
    data: bytes
    image: TimImage

    @property
    def filename(self) -> str:
        return f"textures/{self.index:03d}.tim"

    def metadata(self) -> dict[str, object]:
        """Describe the TIM and its absolute offset in the scene file."""
        return {
            "index": self.index,
            "offset": self.offset,
            "size": len(self.data),
            "file": self.filename,
            "tim": self.image.metadata(),
        }


def read_counted_records(data: bytes, record_size: int, label: str) -> int:
    """Validate a u32 count followed by exactly that many fixed-size records."""
    if len(data) < 4:
        raise SceneFormatError(f"{label}: missing record count")
    count = struct.unpack_from("<I", data)[0]
    expected = 4 + count * record_size
    if len(data) != expected:
        raise SceneFormatError(
            f"{label}: {count} records require {expected} bytes, got {len(data)}"
        )
    return count


def parse_textures(section: SceneSection) -> tuple[SceneTexture, ...]:
    """Return validated TIMs in image-table order, including repeated references."""
    data = section.data
    if not data:
        return ()
    if len(data) < 4:
        raise SceneFormatError("images: missing offset table")
    table_size = struct.unpack_from("<I", data)[0]
    if table_size < 4 or table_size % 4 or table_size > len(data):
        raise SceneFormatError("images: invalid offset table size")
    offsets = struct.unpack_from(f"<{table_size // 4}I", data)
    if any(offset < table_size or offset >= len(data) for offset in offsets):
        raise SceneFormatError("images: texture offset outside image payload")
    # Repeated offsets may refer to the same image. Preserve every table index.
    boundaries = sorted(set(offsets)) + [len(data)]
    ends = dict(zip(boundaries, boundaries[1:]))
    textures = []
    for index, offset in enumerate(offsets):
        payload = data[offset:ends[offset]]
        try:
            image = TimImage.parse(payload)
        except TimFormatError as error:
            raise SceneFormatError(f"images[{index}]: {error}") from error
        textures.append(SceneTexture(index, section.offset + offset, payload, image))
    return tuple(textures)


def common_chest_parameters(
    source: int, kind: int, scripts: tuple[int, ...], events: SceneSection
) -> dict[str, object] | None:
    """Return chest parameters for a recognized initializer prefix, otherwise None."""
    if kind not in (0, 7) or (source & 7) != 5:
        return None
    initializer = scripts[15]
    if initializer == 0xFFFF or not (initializer & 0x8000):
        return None
    index = initializer & 0x7FFF
    if index * 2 + 2 > len(events.data):
        return None
    offset = struct.unpack_from("<H", events.data, index * 2)[0]
    # Event-script offsets are section-relative; entry zero is not a count.
    if offset < index * 2 + 2:
        return None
    if not events.data[offset:].startswith(COMMON_CHEST_INITIALIZER):
        return None
    return {
        "recognition": "common_initializer_prefix",
        "item_id": scripts[4],
        "collection_variable": scripts[5] & 0x7FFF,
        "alternate_facing": bool(scripts[5] & 0x8000),
        "initializer_script_id": index,
        "initializer_file_offset": events.offset + offset,
        "graphics_resource": "FIELD shared chest geometry and texture",
    }


def parse_layout(
    section: SceneSection, events: SceneSection
) -> tuple[dict[str, object], ...]:
    """Decode all layout records, including actions that are not visible actors."""
    count = read_counted_records(section.data, LAYOUT_ENTRY_SIZE, "layout")
    entries = []
    for index in range(count):
        offset = 4 + index * LAYOUT_ENTRY_SIZE
        control, variable, minimum, maximum, position, source, enabled = (
            struct.unpack_from("<IHBBIHH", section.data, offset)
        )
        scripts = struct.unpack_from("<16H", section.data, offset + 16)
        kind = (control >> 4) & 15
        chest = common_chest_parameters(source, kind, scripts, events)
        entries.append({
            "index": index,
            "file_offset": section.offset + offset,
            "control_raw": control,
            "control": {
                "trigger_group": control & 15,
                "kind": kind,
                "selector": (control >> 8) & 255,
                "local_variable_count": (control >> 16) & 255,
                "palette": (control >> 24) & 15,
                "group": (control >> 28) & 3,
                "hidden": bool(control & 0x40000000),
                "active": bool(control & 0x80000000),
            },
            "condition": {"variable": variable, "minimum": minimum, "maximum": maximum},
            "position_raw": position,
            "position_bits": {
                "x": position & 0xFFFF,
                "z": (position >> 16) & 0x7FF,
                "unknown_bits_27_29": (position >> 27) & 7,
                "y": position >> 30,
            },
            "source_raw": source,
            "enabled_events": enabled,
            "scripts_and_parameters": list(scripts),
            "chest_graphics_candidate": kind in (0, 7) and (source & 7) == 5,
            "common_chest": chest,
        })
    return tuple(entries)


@dataclass(frozen=True)
class FieldScene:
    """Validated scene sections and interpreted metadata; source bytes are retained."""

    data: bytes
    sections: tuple[SceneSection, ...]
    layout: tuple[dict[str, object], ...]
    textures: tuple[SceneTexture, ...]
    portrait_count: int
    group_bounds_count: int

    @classmethod
    def parse(cls, data: bytes) -> FieldScene:
        """Parse a ten-section FIELD scene, raising SceneFormatError for invalid input."""
        if len(data) < HEADER_SIZE:
            raise SceneFormatError("truncated scene header (expected ten u32 offsets)")
        offsets = struct.unpack_from("<10I", data)
        if offsets[0] != HEADER_SIZE:
            raise SceneFormatError(
                "unsupported IMG: FIELD scene layout must start at byte 40"
            )
        if any(offset % 4 or offset > len(data) for offset in offsets):
            raise SceneFormatError("section offsets must be aligned and within the file")
        if tuple(sorted(offsets)) != offsets:
            raise SceneFormatError("section offsets must be nondecreasing")
        ends = offsets[1:] + (len(data),)
        sections = tuple(
            SceneSection(index, start, data[start:end])
            for index, (start, end) in enumerate(zip(offsets, ends))
        )
        layout = parse_layout(sections[0], sections[1])
        textures = parse_textures(sections[7])
        portraits = read_counted_records(sections[8].data, PORTRAIT_SIZE, "portraits")
        bounds = read_counted_records(sections[9].data, 4, "group_bounds")
        return cls(data, sections, layout, textures, portraits, bounds)

    def manifest(self, source_name: str) -> dict[str, object]:
        """Build JSON metadata with absolute input offsets and relative output paths."""
        return {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "source": source_name,
            "size": len(self.data),
            "sha256": hashlib.sha256(self.data).hexdigest(),
            "header_file": "header.bin",
            "sections": [section.metadata() for section in self.sections],
            "layout_entries": self.layout,
            "textures": [texture.metadata() for texture in self.textures],
            "portrait_count": self.portrait_count,
            "group_bounds_count": self.group_bounds_count,
            "notes": [
                "Only the ANA/INFO_* FIELD scene layout is supported, not every IMG format.",
                "Script, string, geometry and other undecoded data remain in raw sections.",
                "Common chests are recognized by initializer prefix, not runtime simulation.",
                "Chest artwork is external to this file, in FIELD's shared resources.",
            ],
        }

    def summary(self, source_name: str) -> str:
        lines = [
            f"{source_name}: {len(self.data)} bytes, {len(self.layout)} layout entries, "
            f"{len(self.textures)} TIM textures",
            "",
            "Section           File offset     Size",
        ]
        for section in self.sections:
            lines.append(f"{section.name:17} 0x{section.offset:08X}  {len(section.data):7}")
        lines.extend(["", "Chest graphics candidates:"])
        candidates = [entry for entry in self.layout if entry["chest_graphics_candidate"]]
        if not candidates:
            lines.append("  None")
        for entry in candidates:
            position = entry["position_bits"]
            label = f"  Layout {entry['index']}: x={position['x']}, z={position['z']}"
            chest = entry["common_chest"]
            if chest is None:
                lines.append(label + "; initializer unrecognized (reward not inferred)")
            else:
                lines.append(
                    label + f"; item=0x{chest['item_id']:04X}, "
                    f"collection=0x{chest['collection_variable']:04X}, "
                    f"alternate facing={chest['alternate_facing']}"
                )
        lines.extend(["", "Scripts and strings are raw; chest artwork comes from FIELD."])
        return "\n".join(lines) + "\n"

    def extract(self, output: Path, source_name: str) -> None:
        """Write sections, TIMs and metadata; the output directory must not exist."""
        manifest = json.dumps(self.manifest(source_name), indent=2) + "\n"
        output.mkdir(parents=True, exist_ok=False)
        (output / "sections").mkdir()
        (output / "textures").mkdir()
        (output / "header.bin").write_bytes(self.data[:HEADER_SIZE])
        for section in self.sections:
            (output / section.filename).write_bytes(section.data)
        for texture in self.textures:
            (output / texture.filename).write_bytes(texture.data)
        (output / "manifest.json").write_text(manifest, encoding="ascii")
        (output / "summary.txt").write_text(self.summary(source_name), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    info = commands.add_parser("info", help="show section and chest metadata")
    info.add_argument("source", type=Path)
    info.add_argument("--json", action="store_true", help="print the full manifest")
    extract = commands.add_parser("extract", help="extract into a new directory")
    extract.add_argument("source", type=Path)
    extract.add_argument("output", type=Path)
    validate = commands.add_parser("validate", help="validate one or more scene files")
    validate.add_argument("sources", type=Path, nargs="+")
    args = parser.parse_args()
    try:
        if args.command == "validate":
            for source in args.sources:
                scene = FieldScene.parse(source.read_bytes())
                print(f"{source}: valid ({len(scene.layout)} entries, {len(scene.textures)} textures)")
        else:
            scene = FieldScene.parse(args.source.read_bytes())
            if args.command == "extract":
                scene.extract(args.output, args.source.name)
                print(scene.summary(args.source.name), end="")
                print(f"Extracted to {args.output}")
            elif args.json:
                print(json.dumps(scene.manifest(args.source.name), indent=2))
            else:
                print(scene.summary(args.source.name), end="")
    except (OSError, SceneFormatError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
