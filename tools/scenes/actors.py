"""Decode scene actor descriptions, image references and action slots.

The format follows FieldSceneActorDescription in field_scene_transition.c and
FieldActionSlot in field_actor_tables.h. Action parameters retain their stored
values: their meaning depends on the command, not just the actor description.
"""

from __future__ import annotations

import struct

from tools.scenes.scene_format import SceneHeader
from tools.scenes.section_cli import run


ACTOR_HEADER = struct.Struct("<BB6BHHI")
ACTION_SLOT = struct.Struct("<4H")


def read_action(data: bytes, offset: int) -> dict:
    """Decode one action slot without assuming its parameter is a scene script."""
    command, flags, animation, parameter = ACTION_SLOT.unpack(data)
    return {
        "offset": f"0x{offset:X}",
        "command": command,
        "is_technique": bool(command & 0x8000),
        "flags": f"0x{flags:04X}",
        "target_filter": flags & 0xFF,
        "target_group": (flags >> 8) & 3,
        "instrument": bool(flags & 0x400),
        "unknown_flag_bits": flags >> 11,
        "animation": animation,
        "parameter": parameter,
        "bytes": data.hex(" "),
    }


def read_actors(data: bytes, header: SceneHeader) -> dict:
    """Read the actor section and resolve its image indices within the scene."""
    section = data[header.actors:header.geometry]
    if len(section) < 8:
        raise ValueError("truncated actors header")
    volume, palette, map_word, count = struct.unpack_from("<4H", section)
    table_end = 8 + count * 4
    if table_end > len(section):
        raise ValueError("actor directory exceeds its section")
    offsets = struct.unpack_from(f"<{count}I", section, 8)
    if any(offset < table_end or offset % 4 or offset >= len(section) for offset in offsets):
        raise ValueError("actor description offset outside section or unaligned")

    images = data[header.images:header.portraits]
    image_offsets = ()
    if images:
        table_size = struct.unpack_from("<I", images)[0]
        if table_size < 4 or table_size % 4 or table_size > len(images):
            raise ValueError("invalid image directory size")
        image_offsets = struct.unpack_from(f"<{table_size // 4}I", images)
        if any(offset < table_size or offset >= len(images) for offset in image_offsets):
            raise ValueError("image offset outside section")
    texture_indices = {offset: index for index, offset in enumerate(sorted(set(image_offsets)))}

    starts = sorted(set(offsets))
    ends = dict(zip(starts, starts[1:] + [len(section)]))
    actors = []
    for index, start in enumerate(offsets):
        raw = section[start:ends[start]]
        if len(raw) < ACTOR_HEADER.size:
            raise ValueError(f"truncated actor description {index}")
        unknown, flags, *fields = ACTOR_HEADER.unpack_from(raw)
        image_slots = fields[:6]
        actor_palette, animation_flags, action_count = fields[6:]
        actions_end = ACTOR_HEADER.size + action_count * ACTION_SLOT.size
        if actions_end > len(raw):
            raise ValueError(f"action table exceeds actor description {index}")
        references = []
        for image_index in image_slots:
            if image_index == 0xFF:
                break
            reference = {"image_index": image_index}
            if image_index < len(image_offsets):
                image_offset = image_offsets[image_index]
                reference.update(offset=f"0x{header.images + image_offset:X}",
                                 file=f"textures/{texture_indices[image_offset]:03d}.tim")
            else:
                reference["unresolved"] = "image index outside scene directory"
            references.append(reference)
        actions = []
        for action_index in range(action_count):
            position = ACTOR_HEADER.size + action_index * ACTION_SLOT.size
            action = read_action(raw[position:position + ACTION_SLOT.size], header.actors + start + position)
            actions.append({"index": action_index, **action})
        actors.append({
            "index": index,
            "resource_index": index + 3,
            "offset": f"0x{header.actors + start:X}",
            "unknown_00": unknown,
            "flags": f"0x{flags:02X}",
            "has_actions_flag": bool(flags & 0x80),
            "image_slots": image_slots,
            "images": references,
            "inherits_previous_images": not references,
            "palette": actor_palette,
            "bound_animation_flags": f"0x{animation_flags:04X}",
            "actions": actions,
            "trailing_bytes": raw[actions_end:].hex(" "),
            "record_bytes": raw.hex(" "),
        })
    return {
        "offset": f"0x{header.actors:X}",
        "size": len(section),
        "song_volume": volume,
        "party_palette_index": palette,
        "map_word": f"0x{map_word:04X}",
        "map_id": map_word & 0x7FFF,
        "parts_timer_mode": map_word >> 15,
        "actor_count": count,
        "relative_offsets": list(offsets),
        "directory_trailing_bytes": section[table_end:starts[0] if starts else len(section)].hex(" "),
        "actors": actors,
    }


def main(argv: list[str] | None = None) -> int:
    """Inspect the actors section of a scene IMG."""
    return run(read_actors, __doc__, argv, kind="actors")


if __name__ == "__main__":
    raise SystemExit(main())
