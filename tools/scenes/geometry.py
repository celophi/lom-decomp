"""Decode the scene geometry section's actor animation resources.

The loader uses consecutive u32 boundaries for each actor description. Within
an actor resource, field_advance_actor_animation_frame reads animation offsets
and a frame table; field_render_effect_frame8/16 read sprites and placements.
This is a stored-format listing, not a scene or animation renderer.
"""

from __future__ import annotations

import struct

from tools.scenes.scene_format import SceneHeader
from tools.scenes.section_cli import run
from tools.scenes.strings import uncovered


PLACEMENTS = {0: 'quad', 1: 'contact', 2: 'shadow', 3: 'quad',
              4: 'attachment', 5: 'sound', 6: 'attachment'}


def signed(value: int, bits: int) -> int:
    return value - (1 << bits) if value & (1 << (bits - 1)) else value


def read_frame(data: bytes, start: int, offset: int, wide: bool) -> tuple[dict, int]:
    """Read variable-sized records; placement operands remain byte-exact."""
    if start >= len(data):
        raise ValueError('frame offset outside actor geometry')
    count = data[start]
    cursor = start + 1
    records = []
    for index in range(count):
        if cursor + 8 > len(data):
            raise ValueError('truncated frame record')
        flags = data[cursor + 7]
        placement = bool(flags & 0x20)
        size = (17 if placement else 11) if wide else 9
        raw = data[cursor:cursor + size]
        if len(raw) != size:
            raise ValueError('truncated frame record')
        record = {'index': index, 'offset': offset + cursor, 'size': size,
                  'kind': 'placement' if placement else 'sprite', 'flags': flags, 'bytes': raw.hex(' ')}
        if placement:
            record.update(command=flags & 15, command_name=PLACEMENTS.get(flags & 15, 'unknown'),
                          operand_bytes=(raw[:7] + raw[8:]).hex(' '))
        else:
            x = raw[0] | (raw[9] << 8) if wide else raw[0]
            y = raw[1] | (raw[10] << 8) if wide else raw[1]
            record.update(x=signed(x, 16 if wide else 8), y=signed(y, 16 if wide else 8),
                          u=raw[2], v=raw[3], width=raw[4], height=raw[5],
                          clut_column=raw[6], tpage_column=flags & 3,
                          mirror_u=bool(flags & 0x40), mirror_v=bool(flags & 0x80),
                          tilt=raw[8])
        records.append(record)
        cursor += size
    return {'offset': offset + start, 'record_count': count, 'records': records}, cursor


def read_animation_resource(data: bytes, offset: int) -> dict:
    if len(data) < 6:
        raise ValueError('truncated actor geometry header')
    unknown, frame_table, count, mode = struct.unpack_from('<HHBB', data)
    table_end = 6 + count * 2
    if table_end > len(data) or not table_end <= frame_table <= len(data) - 2:
        raise ValueError('invalid actor animation directory or frame table')
    pointers = struct.unpack_from(f'<{count}H', data, 6)
    frame_count = struct.unpack_from('<H', data, frame_table)[0]
    frames_end = frame_table + 2 + frame_count * 2
    if frames_end > len(data):
        raise ValueError('frame directory exceeds actor geometry')
    frame_offsets = struct.unpack_from(f'<{frame_count}H', data, frame_table + 2)
    spans = [(0, table_end), (frame_table, frames_end)]
    animations = []
    for index, pointer in enumerate(pointers):
        start, stride = pointer & 0x7FFF, 4 if pointer & 0x8000 else 2
        if start < frames_end or start >= len(data):
            raise ValueError('animation offset outside sequence payload')
        end = start + 1 + data[start] * stride
        if end > len(data):
            raise ValueError('animation frames exceed actor geometry')
        entries = []
        for frame in range(data[start]):
            position = start + 1 + frame * stride
            raw = data[position:position + stride]
            entry = {'index': frame, 'offset': offset + position, 'frame': raw[0], 'duration': raw[1]}
            if raw[0] < frame_count:
                entry['frame_offset'] = offset + frame_offsets[raw[0]]
            else:
                entry['unresolved'] = 'Frame index outside frame directory.'
            if stride == 4:
                entry.update(height=raw[2], motion=raw[3])
            entries.append(entry)
        animations.append({'index': index, 'offset': offset + start, 'pointer_word': pointer,
                           'entry_size': stride, 'frame_count': data[start], 'frames': entries})
        spans.append((start, end))
    frames = []
    decoded = {}
    for index, start in enumerate(frame_offsets):
        if start < frames_end:
            raise ValueError('frame offset overlaps actor geometry directory')
        if start not in decoded:
            frame, end = read_frame(data, start, offset, bool(mode & 1))
            decoded[start] = frame
            spans.append((start, end))
        frames.append({'index': index, **decoded[start]})
    return {'offset': offset, 'size': len(data), 'unknown_00': unknown,
            'frame_table_offset': offset + frame_table, 'animation_count': count,
            'mode': mode, 'wide_coordinates': bool(mode & 1),
            'default_motion': (mode >> 1) & 0x3F, 'mode_high_bit': bool(mode & 0x80),
            'animations': animations, 'frames': frames, 'undecoded': uncovered(data, spans, offset)}


def read_geometry(data: bytes, header: SceneHeader) -> dict:
    section = data[header.geometry:header.images]
    count = struct.unpack_from('<H', data, header.actors + 6)[0]
    size = (count + 1) * 4
    if len(section) < size:
        raise ValueError('truncated geometry boundary directory')
    boundaries = list(struct.unpack_from(f'<{count + 1}I', section))
    # Empty synthetic/placeholder sections have no usable boundary directory.
    if not count and boundaries == [0]:
        return {'offset': header.geometry, 'size': len(section), 'actor_count': 0,
                'relative_offsets': boundaries, 'actors': [], 'undecoded': uncovered(section, [], header.geometry)}
    if boundaries != sorted(boundaries) or boundaries[0] < size or boundaries[-1] > len(section):
        raise ValueError('invalid geometry actor boundaries')
    actors = []
    spans = [(0, size)]
    for index, (start, end) in enumerate(zip(boundaries, boundaries[1:])):
        actor = {'index': index, 'resource_index': index + 3, 'offset': header.geometry + start, 'size': end - start}
        if start < end:
            actor.update(read_animation_resource(section[start:end], header.geometry + start))
        actors.append(actor)
        spans.append((start, end))
    return {'offset': header.geometry, 'size': len(section), 'actor_count': count,
            'relative_offsets': boundaries, 'actors': actors, 'undecoded': uncovered(section, spans, header.geometry)}


def format_geometry(document: dict) -> str:
    lines = ['ACTOR GEOMETRY / ANIMATIONS', f"IMG offset 0x{document['offset']:08X}; {document['size']} bytes",
             'Consecutive section-relative u32 boundaries delimit one resource per actor description.',
             'Animation pointer bit 15 selects four-byte entries (frame, duration, height, motion).',
             'Duration is the stored byte (the game treats zero as one tick); height and motion are stored bytes.',
             'Sprite coordinates are local. Placement operands remain raw; no rendering is performed.', '']
    for actor in document['actors']:
        lines.append(f"ACTOR {actor['index']} / resource {actor['resource_index']} "
                     f"at 0x{actor['offset']:08X} ({actor['size']} bytes)")
        if not actor['size']:
            lines.append('  Empty resource.')
            continue
        lines.append(f"  mode=0x{actor['mode']:02X}; wide coordinates={actor['wide_coordinates']}; "
                     f"default motion={actor['default_motion']}")
        for animation in actor['animations']:
            lines.append(f"  Animation {animation['index']:3d} at 0x{animation['offset']:08X}: "
                         f"{animation['frame_count']} steps, {animation['entry_size']} bytes per step")
            if not animation['frames']:
                continue
            wide_entries = animation['entry_size'] == 4
            lines.append('    Step  Frame  Duration' + ('  Height  Motion' if wide_entries else ''))
            for entry in animation['frames']:
                row = f"    {entry['index']:4d}  {entry['frame']:5d}  {entry['duration']:8d}"
                if wide_entries:
                    row += f"  {entry['height']:6d}  {entry['motion']:6d}"
                if 'unresolved' in entry:
                    row += '  [frame index outside directory]'
                lines.append(row)
        for frame in actor['frames']:
            lines.append(f"  Frame {frame['index']:3d} at 0x{frame['offset']:08X}: {frame['record_count']} records")
            for record in frame['records']:
                if record['kind'] == 'sprite':
                    detail = (f"sprite xy=({record['x']},{record['y']}) uv=({record['u']},{record['v']}) "
                              f"size={record['width']}x{record['height']} clut={record['clut_column']} "
                              f"tpage={record['tpage_column']} mirror=({int(record['mirror_u'])},{int(record['mirror_v'])}) "
                              f"tilt={record['tilt']}")
                else:
                    detail = f"placement {record['command']} ({record['command_name']}), operands={record['operand_bytes']}"
                lines.append(f"    0x{record['offset']:08X} {detail}")
        for span in actor['undecoded']:
            lines.append(f"  Unreferenced bytes at 0x{span['offset']:08X}: {span['bytes']}")
        lines.append('')
    for span in document['undecoded']:
        lines.append(f"Unreferenced section bytes at 0x{span['offset']:08X}: {span['bytes']}")
    return '\n'.join(lines) + '\n'


def main(argv: list[str] | None = None) -> int:
    return run(read_geometry, __doc__, argv, kind='geometry', text_renderer=format_geometry)


if __name__ == '__main__':
    raise SystemExit(main())
