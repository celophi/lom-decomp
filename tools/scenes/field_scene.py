#!/usr/bin/env python3
"""Extract scene IMG layouts, actors, scripts, battle resources and images.

Every byte is preserved in the exported files. Undecoded data stays under its
section name, and each export is reconstructed and checked before publication.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import json
from pathlib import Path
import struct
import tempfile

import yaml

from tools.assets.psx_tim import TimImage
from tools.scenes.scene_resources import read_resource_directory, format_resource
from tools.scenes.strings import read_strings, format_strings
from tools.scenes.geometry import read_geometry, format_geometry
from tools.scenes.actors import read_actors
from tools.scenes.layout import COMMON_CHEST_INITIALIZER, LAYOUT_RECORD, FieldLayoutRecord, read_layout_records
from tools.scenes.actor_scripts import format_disassembly as format_actor_scripts, read_actor_scripts
from tools.scenes.event_scripts import format_disassembly as format_event_scripts, read_event_scripts
from tools.scenes.presentation import dump_yaml
from tools.scenes.scene_report import build_report, format_report
from tools.scenes.reference_data import ReferenceData, add_reference_arguments, reference_from_arguments
from tools.scenes.scene_format import SCENE_HEADER, SceneHeader, SceneSection
from tools.scenes.structure import describe_scene, format_structure


PORTRAIT_SIZE = 1184

@dataclass(frozen=True)
class AssetRange:
    """An extraction result, not a disk structure. Each file represents one IMG range.

    offset is absolute in the IMG; size is in bytes. Layouts are stored as YAML
    with their original record bytes. Other ranges are binary, with an optional
    decoded YAML document beside them.
    """

    name: str
    offset: int
    size: int
    filename: str
    layout: FieldLayoutRecord | None = None
    document: dict | None = None


def read_layout(data: bytes, header: SceneHeader) -> list[AssetRange]:
    """Export every layout record, keeping recognized chests at their old paths."""
    records = read_layout_records(data, header)
    event_scripts = data[header.event_scripts:header.strings]
    result = [AssetRange("layout_count", header.layout, 4, "layout/count.bin")]
    for index, record in enumerate(records):
        offset = header.layout + 4 + index * LAYOUT_RECORD.size
        chest = record.is_common_chest(event_scripts)
        folder = "chests" if chest else "layout"
        result.append(AssetRange(
            "chest" if chest else "layout_record", offset, LAYOUT_RECORD.size,
            f"{folder}/{index:03d}.yaml", layout=record,
        ))
    return result


def read_chests(data: bytes, header: SceneHeader) -> list[AssetRange]:
    """Return only recognized chests from the complete layout."""
    return [asset for asset in read_layout(data, header) if asset.name == "chest"]


def read_resources(data: bytes, header: SceneHeader, text_encoding: str = "us") -> list[AssetRange]:
    """Export the resource directory and each unique resource with its metadata."""
    offsets, resources = read_resource_directory(data[header.records:header.actors])
    if not offsets:
        return []
    files = {resource.offset: f"records/{resource.slots[0]:03d}.bin" for resource in resources}
    directory = {"entries": [
        {"index": index, "relative_offset": f"0x{offset:X}", "file": files[offset]}
        for index, offset in enumerate(offsets)
    ]}
    result = [AssetRange("resource_directory", header.records, len(offsets) * 4,
                         "records/directory.bin", document=directory)]
    for resource in resources:
        result.append(AssetRange(
            "resource", header.records + resource.offset, len(resource.data),
            files[resource.offset], document=resource.document(header.records, text_encoding),
        ))
    return result


def read_group_bounds(data: bytes, header: SceneHeader) -> list[AssetRange]:
    """Preserve the final counted word array without guessing its bit fields."""
    bounds = data[header.group_bounds:]
    if len(bounds) < 4:
        raise ValueError("missing group-bounds count")
    count = struct.unpack_from("<I", bounds)[0]
    if len(bounds) != 4 + count * 4:
        raise ValueError("group-bounds count does not match the section size")
    document = {"count": count, "words": [f"0x{word:08X}" for word in struct.unpack_from(f"<{count}I", bounds, 4)]}
    return [AssetRange("group_bounds", header.group_bounds, len(bounds),
                       "group_bounds/data.bin", document=document)]


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
    directory = {"entries": [
        {"index": index, "relative_offset": f"0x{offset:X}",
         "file": f"textures/{boundaries.index(offset):03d}.tim"}
        for index, offset in enumerate(offsets)
    ]}
    textures = [AssetRange("image_directory", header.images, table_size,
                           "textures/directory.bin", document=directory)]
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
    result = [AssetRange("portrait_count", header.portraits, 4, "portraits/count.bin")]
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
                section.name, cursor, asset.offset - cursor, f"{section.name}/{cursor:08X}.bin",
            ))
        result.append(asset)
        cursor = asset.offset + asset.size
    if cursor < section.end:
        result.append(AssetRange(
            section.name, cursor, section.end - cursor, f"{section.name}/{cursor:08X}.bin",
        ))
    return result


def split_scene(data: bytes, text_encoding: str = "us") -> list[AssetRange]:
    """Parse the container and return a complete map of its original bytes."""
    header = SceneHeader.parse(data)
    assets_by_section = {
        "layout": read_layout(data, header),
        "records": read_resources(data, header, text_encoding),
        "strings": [AssetRange("strings", header.strings, header.actor_scripts - header.strings,
                                "strings/data.bin", document=read_strings(data, header, text_encoding=text_encoding))],
        "geometry": [AssetRange("geometry", header.geometry, header.images - header.geometry,
                                 "geometry/data.bin", document=read_geometry(data, header))],
        "group_bounds": read_group_bounds(data, header),
        "images": read_textures(data, header),
        "portraits": read_portraits(data, header),
        "actors": [AssetRange("actors", header.actors, header.geometry - header.actors,
                               "actors/data.bin", document=read_actors(data, header))],
        "event_scripts": [AssetRange("event_scripts", header.event_scripts, header.strings - header.event_scripts,
                                      "event_scripts/data.bin", document=read_event_scripts(data, header))],
        "actor_scripts": [AssetRange("actor_scripts", header.actor_scripts, header.records - header.actor_scripts,
                                      "actor_scripts/data.bin", document=read_actor_scripts(data, header))],
    }
    result = [AssetRange("header", 0, SCENE_HEADER.size, "header.bin")]
    for section in header.sections(len(data)):
        result.extend(cover_section(section, assets_by_section.get(section.name, [])))
    return result


def write_yaml(path: Path, document: dict, kind: str = "") -> None:
    """Write decoded fields in insertion order, using the project's YAML style."""
    path.write_text(dump_yaml(document, kind), encoding="ascii")


def reconstruct(output: Path) -> bytes:
    """Recover original bytes from an export's byte map, checking every range.

    Decoded fields are descriptive. Layouts use record_bytes; other ranges use
    the original binary files, not their optional YAML companions.
    """
    document = yaml.safe_load((output / "byte-map.yaml").read_text(encoding="ascii"))
    result = bytearray()
    for entry in document["sections"]:
        path = output / entry["file"]
        if path.suffix == ".yaml":
            record = yaml.safe_load(path.read_text(encoding="ascii"))
            payload = bytes.fromhex(record["record_bytes"])
        else:
            payload = path.read_bytes()
        if entry["offset"] != len(result) or entry["size"] != len(payload):
            raise ValueError(f"byte map range does not match {entry['file']}")
        result.extend(payload)
    return bytes(result)


def format_overview(source: Path, header: SceneHeader, size: int, assets: list[AssetRange]) -> str:
    """Describe the stored structure, decoding coverage and decoded record paths."""
    document = describe_scene(source, header, size, assets)
    lines = [f"SCENE: {source.name}", f"Size: {size} bytes", ""]
    lines.extend(format_structure(document))
    lines.extend(["", "WHERE TO LOOK",
                  "  objects.txt               Object settings, script links, monster templates and drops",
                  "  chests/*.yaml             Recognized chest items, positions and conditions",
                  "  layout/*.yaml             Other layout records and script/parameter slots",
                  "  records/directory.yaml    Resource directory; follow files to their YAML companions",
                  "  records/*.txt and .yaml    Resource fields, monster templates, actions, drops and rewards",
                  "  actors/data.yaml          Actor descriptions, texture references and action slots",
                  "  actor_scripts/data.txt    Actor movement/action disassembly and directory references",
                  "  event_scripts/data.txt    Event commands, branches, calls and variable references",
                  "  strings/data.txt          Text entries, controls and shared directory references",
                  "  geometry/data.txt         Actor animations, timing, sprite fields and placements",
                  "  textures/directory.yaml   Image slots and exported TIM files",
                  "  byte-map.yaml             Original byte ranges and their exported files",
                  "",
                  "CHESTS"])
    chests = [asset for asset in assets if asset.name == "chest"]
    lines.extend(f"  {asset.filename}  IMG offset 0x{asset.offset:08X}" for asset in chests)
    if not chests:
        lines.append("  No recognized chest records.")
    lines.extend(["", "MONSTER TEMPLATES"])
    monsters = []
    for asset in assets:
        if asset.document is None or "battle" not in asset.document:
            continue
        metadata = Path(asset.filename).with_suffix(".yaml").as_posix()
        for monster in asset.document["battle"]["monsters"]:
            name = monster["name_ascii"] or "(see encoded name_bytes)"
            monsters.append(f"  {metadata}  template {monster['index']}, ID {monster['id']}: {name}")
    lines.extend(monsters or ["  No monster templates in decoded battle resources."])
    lines.extend(["", "READING THE FORMAT",
                  "  Offsets locate bytes in the original IMG; binary values are little-endian.",
                  "  Directory and record indices are zero-based. Shared offsets mean shared data.",
                  "  Decoded fields and text listings are reference exports. Editing them does not rebuild the IMG.",
                  "  Reconstruction reads byte-map files: layout record_bytes and original binary payloads.",
                  "  Monster stats are stored growth inputs. Drop handler/value pairs are stored choices.",
                  "  Split flags, ASCII name previews and object summaries interpret the stored fields.",
                  "  unknown/unresolved fields keep their stored values without a guessed interpretation.",
                  "  Text controls, placement operands and unclassified spans retain their bytes in YAML."])
    return "\n".join(lines) + "\n"


def extract(source: Path, output: Path, reference: ReferenceData | None = None, *, text_encoding: str = "us") -> None:
    """Validate, write and reconstruct one scene before publishing its directory."""
    if output.exists():
        raise FileExistsError(f"Output already exists: {output}")
    data = source.read_bytes()
    header = SceneHeader.parse(data)
    assets = split_scene(data, text_encoding)
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix=f".{output.name}-", dir=output.parent) as temporary:
        staging = Path(temporary) / "scene"
        staging.mkdir()
        renderers = {"actor_scripts": format_actor_scripts, "event_scripts": format_event_scripts,
                     "strings": format_strings, "geometry": format_geometry, "resource": format_resource}
        lines = [f"source: {json.dumps(source.name)}",
                 "# Offsets and sizes are bytes in the original IMG.", "sections:"]
        for asset in assets:
            path = staging / asset.filename
            path.parent.mkdir(parents=True, exist_ok=True)
            payload = data[asset.offset:asset.offset + asset.size]
            if asset.layout is not None:
                path.write_text(asset.layout.layout_yaml(payload, chest=asset.name == "chest"), encoding="ascii")
            else:
                path.write_bytes(payload)
            lines.extend([f"  - name: {asset.name}", f"    offset: 0x{asset.offset:08X}",
                          f"    size: {asset.size}", f"    file: {asset.filename}"])
            if asset.document is not None:
                metadata = path.with_suffix(".yaml")
                write_yaml(metadata, asset.document, asset.name)
                lines.append(f"    metadata: {metadata.relative_to(staging).as_posix()}")
                if asset.name in renderers:
                    listing = path.with_suffix(".txt")
                    renderer = renderers[asset.name]
                    listing.write_text(renderer(asset.document), encoding="ascii")
                    lines.append(f"    listing: {listing.relative_to(staging).as_posix()}")
        (staging / "byte-map.yaml").write_text("\n".join(lines) + "\n", encoding="ascii")
        write_yaml(staging / "scene.yaml", describe_scene(source, header, len(data), assets))
        (staging / "scene.txt").write_text(format_overview(source, header, len(data), assets), encoding="utf-8")
        documents = {asset.name: asset.document for asset in assets if asset.document is not None}
        report = build_report(data, header, documents["event_scripts"], documents["actors"], reference)
        write_yaml(staging / "objects.yaml", report, "scene_report")
        (staging / "objects.txt").write_text(format_report(report), encoding="utf-8")
        if reconstruct(staging) != data:
            raise ValueError(f"export does not reconstruct {source}")
        if output.exists():
            raise FileExistsError(f"Output already exists: {output}")
        staging.rename(output)


def extract_all(ana_directory: Path, output: Path, reference: ReferenceData | None = None, *, text_encoding: str = "us") -> int:
    """Extract INFO_* and MAPINFO scenes, keeping group and scene directories."""
    if not ana_directory.is_dir():
        raise ValueError(f"ANA directory not found: {ana_directory}")
    groups = sorted(path for path in ana_directory.iterdir()
                    if path.is_dir() and (path.name.startswith("INFO_") or path.name == "MAPINFO"))
    scenes = sorted(path for group in groups for path in group.iterdir()
                    if path.is_file() and path.suffix.lower() == ".img")
    if not scenes:
        raise ValueError(f"No INFO_* or MAPINFO scenes found under {ana_directory}")

    # Check every destination before starting, so an existing export stops the batch early.
    for scene in scenes:
        destination = output / scene.parent.name / scene.stem
        if destination.exists():
            raise FileExistsError(f"Output already exists: {destination}")
    for scene in scenes:
        destination = output / scene.parent.name / scene.stem
        try:
            extract(scene, destination, reference, text_encoding=text_encoding)
        except (OSError, ValueError) as error:
            raise ValueError(f"{scene}: {error}") from error
    return len(scenes)


def main() -> None:
    """Extract one scene, or all supported scenes under an ANA directory."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path, help="scene IMG, or ANA directory with --all")
    parser.add_argument("--all", action="store_true", help="extract every INFO_* and MAPINFO scene")
    parser.add_argument("output", type=Path)
    parser.add_argument("--text-encoding", choices=("us", "jp"), default="us",
                        help="text control/glyph dialect; no external files (default: us)")
    add_reference_arguments(parser)
    args = parser.parse_args()
    try:
        reference = reference_from_arguments(args)
        if args.all:
            count = extract_all(args.source, args.output, reference, text_encoding=args.text_encoding)
            print(f"Extracted {count} scenes to {args.output}")
        else:
            extract(args.source, args.output, reference, text_encoding=args.text_encoding)
            print(f"Extracted to {args.output}")
    except (OSError, ValueError) as error:
        parser.exit(1, f"{error}\n")


if __name__ == "__main__":
    main()
