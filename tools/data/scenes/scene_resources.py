"""Read a scene's resource directory and recognized stored payloads.

Offsets in the directory are relative to the records section. Battle table
pointers are relative to their resource; monster pointers are relative to the
monster table. YAML offsets are converted to absolute positions in the IMG.
"""

from __future__ import annotations

import struct
from dataclasses import dataclass
from textwrap import wrap

from tools.data.scenes.presentation import HEX_WIDTHS
from tools.data.scenes.scene_format import SceneHeader
from tools.data.scenes.section_cli import run
from tools.data.scenes.strings import format_strings
from tools.data.scenes.resource_payloads import (
    RESOURCE_NAMES, CODE_REFERENCES, action_descriptor, item_record, read_payload,
)


BATTLE_RESOURCE_ID = 1
MONSTER_HEADER_SIZE = 0x54
MONSTER_ACTION_SIZE = 8
ITEM_SIZE = 0x40
REWARD_SIZE = 4 + ITEM_SIZE
DROP_HANDLERS = ("experience_and_money", "restore", "inventory_item", "template_item")


@dataclass(frozen=True)
class SceneResource:
    """One stored resource, possibly referenced by several directory slots."""

    offset: int
    data: bytes
    slots: tuple[int, ...]

    def document(self, section_offset: int, text_encoding: str = "us") -> dict:
        """Describe the resource and decode recognized stored payloads."""
        offset = section_offset + self.offset
        resource_id = struct.unpack_from("<H", self.data)[0] if self.data else None
        result = {
            "directory_slots": list(self.slots),
            "offset": f"0x{offset:X}",
            "size": len(self.data),
            "resource_id": resource_id,
        }
        result['kind'] = RESOURCE_NAMES.get(resource_id, 'empty' if not self.data else 'unknown')
        if resource_id in CODE_REFERENCES:
            file, symbol = CODE_REFERENCES[resource_id]
            result['code_reference'] = {'file': file, 'symbol': symbol}
        if resource_id == BATTLE_RESOURCE_ID:
            result["battle"] = read_battle_resource(self.data, offset, text_encoding)
        elif self.data:
            try:
                payload = read_payload(self.data, offset, resource_id, text_encoding)
            except ValueError as error:
                result['diagnostics'] = [str(error)]
            else:
                if payload is not None:
                    result['payload'] = payload
        return result


def read_resource_directory(data: bytes) -> tuple[list[int], list[SceneResource]]:
    """Read section-relative offsets, preserving shared and empty entries."""
    if not data:
        return [], []
    if len(data) < 4:
        raise ValueError("truncated scene resource directory")
    table_size = struct.unpack_from("<I", data)[0]
    if table_size < 4 or table_size % 4 or table_size > len(data):
        raise ValueError("invalid scene resource directory size")
    offsets = list(struct.unpack_from(f"<{table_size // 4}I", data))
    if any(offset < table_size or offset > len(data) or offset % 4 for offset in offsets):
        raise ValueError("scene resource offset outside the section or unaligned")

    starts = sorted(set(offsets))
    resources = []
    for start, end in zip(starts, starts[1:] + [len(data)]):
        slots = tuple(index for index, offset in enumerate(offsets) if offset == start)
        resources.append(SceneResource(start, data[start:end], slots))
    return offsets, resources


def ascii_name(raw: bytes) -> str | None:
    """Show plain ASCII names only; callers keep the original encoded bytes."""
    name = raw.split(b"\0", 1)[0]
    if all(32 <= value < 127 for value in name):
        return name.decode("ascii")
    return None


def read_monster(data: bytes, offset: int) -> dict:
    """Decode a FieldActorTemplate and decode its action descriptors."""
    if len(data) < MONSTER_HEADER_SIZE:
        raise ValueError(f"truncated monster template at 0x{offset:X}")
    action_count = struct.unpack_from("<i", data, 0x50)[0]
    actions_end = MONSTER_HEADER_SIZE + action_count * MONSTER_ACTION_SIZE
    if action_count < 0 or actions_end > len(data):
        raise ValueError(f"monster actions exceed template at 0x{offset:X}")

    hp_base, hp_growth, unknown_base, unknown_growth = struct.unpack_from("<4H", data, 0x1C)
    drops = []
    for slot in range(8):
        handler, value = struct.unpack_from("<BB", data, 0x40 + slot * 2)
        drop = {"slot": slot, "handler": handler, "value": value}
        drop["kind"] = DROP_HANDLERS[handler] if handler < len(DROP_HANDLERS) else "no_drop"
        if handler == 0:
            drop.update(experience_pickups=value >> 4, money_pickups=value & 15)
        elif handler == 1:
            drop["half_restore_chance_out_of_256"] = value
        elif handler == 2:
            drop["item_id"] = value
        elif handler == 3:
            drop["reward_key"] = data[0x18] * 16 + value
        drops.append(drop)

    return {
        "offset": f"0x{offset:X}",
        "size": len(data),
        "name_ascii": ascii_name(data[:21]),
        "name_bytes": data[:21].hex(" "),
        "raise_mask": data[0x15],
        "lower_mask": data[0x16],
        "unknown_17": data[0x17],
        "id": data[0x18],
        "unknown_19": data[0x19],
        "race": data[0x1A],
        "counter_reset": data[0x1B],
        "hp_base": hp_base,
        "hp_growth": hp_growth,
        "hp_uses_stat_4_curve": hp_growth == 0xFFFF,
        "unknown_20_base": unknown_base,
        "unknown_20_growth": unknown_growth,
        "equipment_stats": [{"base": data[index], "growth": data[index + 1]} for index in range(0x24, 0x2C, 2)],
        "stats": [{"base": data[index], "growth": data[index + 1]} for index in range(0x2C, 0x3C, 2)],
        "immunity_flags": data[0x3C],
        "weak_elements": data[0x3D],
        "resist_elements": data[0x3E],
        "flags": data[0x3F],
        "drops": drops,
        "action_count": action_count,
        "actions": [{"index": index, **action_descriptor(data[start:start + MONSTER_ACTION_SIZE], offset + start)}
                    for index, start in enumerate(range(MONSTER_HEADER_SIZE, actions_end, MONSTER_ACTION_SIZE))],
        "trailing_bytes": data[actions_end:].hex(" "),
    }


def read_battle_resource(data: bytes, offset: int, text_encoding: str = "us") -> dict:
    """Decode monster templates and keyed 64-byte reward items in resource 1."""
    if len(data) < 12:
        raise ValueError("truncated battle resource header")
    header, templates, rewards = struct.unpack_from("<3I", data)
    if not 12 <= templates <= rewards <= len(data) - 4 or templates % 4 or rewards % 4:
        raise ValueError("invalid battle table offsets")
    count = struct.unpack_from("<I", data, templates)[0]
    table_size = 4 + count * 4
    if table_size > rewards - templates:
        raise ValueError("monster offset table exceeds battle templates")
    offsets = list(struct.unpack_from(f"<{count}I", data, templates + 4))
    if any(start < table_size or start >= rewards - templates or start % 4 for start in offsets):
        raise ValueError("monster offset outside template table or unaligned")
    starts = sorted(set(offsets))
    ends = dict(zip(starts, starts[1:] + [rewards - templates]))
    monsters = []
    for index, start in enumerate(offsets):
        monster = read_monster(data[templates + start:templates + ends[start]], offset + templates + start)
        monsters.append({"index": index, **monster})

    # The first four bytes hold a marker and count. Each entry then has a u32
    # key and a full FieldItemRecord; the C lookup walks entries at stride 0x44.
    marker, count = struct.unpack_from("<HH", data, rewards)
    end = rewards + 4 + count * REWARD_SIZE
    if end > len(data):
        raise ValueError("reward items exceed battle resource")
    items = []
    for index in range(count):
        start = rewards + 4 + index * REWARD_SIZE
        key = struct.unpack_from("<I", data, start)[0]
        item = data[start + 4:start + REWARD_SIZE]
        items.append({
            "index": index,
            "offset": f"0x{offset + start:X}",
            "key": key,
            "name_ascii": ascii_name(item[:20]),
            "item_bytes": item.hex(" "),
            "item": item_record(item, offset + start + 4, text_encoding),
        })
    for monster in monsters:
        for drop in monster["drops"]:
            if drop["handler"] == 3:
                drop["reward_indices"] = [item["index"] for item in items if item["key"] == drop["reward_key"]]

    return {
        "header_word": f"0x{header:08X}",
        "templates_offset": f"0x{offset + templates:X}",
        "rewards_offset": f"0x{offset + rewards:X}",
        "monsters": monsters,
        "reward_marker": f"0x{marker:04X}",
        "rewards": items,
        "trailing_bytes": data[end:].hex(" "),
    }


def format_fields(value, indent: int = 2) -> list[str]:
    """A labeled field outline for less common tables, with bounded hex rows."""
    prefix = ' ' * indent
    if isinstance(value, dict):
        lines = []
        for key, item in value.items():
            label = key.replace('_', ' ')
            if isinstance(item, (dict, list)):
                lines.append(prefix + label + ':')
                lines.extend(format_fields(item, indent + 2))
            elif key.endswith('_bytes') and isinstance(item, str):
                octets = item.split()
                lines.append(prefix + label + ':' + (' (none)' if not octets else ''))
                lines.extend(prefix + '  ' + ' '.join(octets[start:start + 16]) for start in range(0, len(octets), 16))
            else:
                if key in HEX_WIDTHS and isinstance(item, int) and not isinstance(item, bool):
                    item = f'0x{item:0{HEX_WIDTHS[key]}X}'
                lines.append(prefix + label + ': ' + str(item))
        return lines
    if isinstance(value, list):
        if not value:
            return [prefix + '(none)']
        if all(not isinstance(item, (dict, list)) for item in value):
            return [prefix + line for line in wrap(', '.join(str(item) for item in value), width=max(40, 110 - indent))]
        lines = []
        for index, item in enumerate(value):
            lines.append(prefix + f'[{index}]')
            lines.extend(format_fields(item, indent + 2))
        return lines
    return [prefix + str(value)]


def format_resource(document: dict) -> str:
    """Render stored fields with short guides for battle, triggers and text."""
    offset = int(document['offset'], 16)
    lines = [f"RESOURCE: {document['kind']}", f"IMG offset 0x{offset:08X}; {document['size']} bytes; ID={document['resource_id']}",
             'Directory slots: ' + ', '.join(str(slot) for slot in document['directory_slots'])]
    if 'code_reference' in document:
        reference = document['code_reference']
        lines.append(f"Code: {reference['file']} : {reference['symbol']}")
    lines.append('')
    if 'battle' in document:
        battle = document['battle']
        lines.extend(['MONSTER TEMPLATES', 'Base/growth pairs are stored inputs. Drop slots are choices, not equal probabilities.',
                      'Action descriptors describe battle effects; actor animation/action slots use a different format.', ''])
        for monster in battle['monsters']:
            lines.append(f"[{monster['index']}] ID {monster['id']}: {monster['name_ascii'] or '(encoded name; see name bytes)'} at {monster['offset']}")
            lines.extend(format_fields(monster))
            lines.append('')
        lines.extend(['REWARD ITEMS', 'Keys match template_item drops within this resource; item fields retain stored values.'])
        lines.extend(format_fields(battle['rewards']))
        if battle['trailing_bytes']:
            lines.extend(format_fields({'trailing_bytes': battle['trailing_bytes']}))
    elif 'payload' in document:
        payload = document['payload']
        if 'texts' in payload:
            lines.append(format_strings(payload['texts']).rstrip())
        else:
            guides = {
                'triggers': 'Coordinates are stored u16 bounds. Command bit 15 selects an event script; otherwise it selects a monster group.',
                'shop_lists': 'Generated entries select this IMG\'s item templates. Prices are stored before the event script applies its scale.',
                'nibble_table': 'Each cell packs low/high nibble values; columns begin at 0x60.',
                'equipment_generation': 'Script halfwords are offsets relative to this resource. Script bytecode remains in undecoded spans.',
                'instrument_generation': 'Script halfwords are offsets relative to this resource. Script bytecode remains in undecoded spans.',
            }
            if document['kind'] in guides:
                lines.extend([guides[document['kind']], ''])
            lines.extend(format_fields(payload))
    elif document['size']:
        lines.append('Payload retained in the companion binary; no usable layout was decoded.')
    else:
        lines.append('Empty resource entry.')
    for diagnostic in document.get('diagnostics', []):
        lines.append('Diagnostic: ' + diagnostic)
    return '\n'.join(lines) + '\n'


def read_resources(data: bytes, header: SceneHeader, *, text_encoding: str = 'us') -> dict:
    """Inspect the resource section without extracting companion binaries."""
    section = data[header.records:header.actors]
    offsets, resources = read_resource_directory(section)
    return {'offset': header.records, 'size': len(section), 'relative_offsets': offsets,
            'resources': [resource.document(header.records, text_encoding) for resource in resources]}


def format_resources(document: dict) -> str:
    return '\n'.join(format_resource(resource) for resource in document['resources'])


def main(argv: list[str] | None = None) -> int:
    return run(read_resources, __doc__, argv, kind='resources', text_renderer=format_resources, text_encoding=True)


if __name__ == '__main__':
    raise SystemExit(main())
