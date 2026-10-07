"""Describe the scene container and report what its parsers actually decode."""

from __future__ import annotations

from pathlib import Path

from tools.data.scenes.scene_format import SCENE_HEADER, SceneHeader


LOADER_FILE = 'src/overlays/field/scene/field_scene_transition.c'
LOADER = {'file': LOADER_FILE, 'symbol': 'field_update_scene'}

# These describe disk layouts, not runtime structures after relocation.
SECTION_FORMATS = {
    'layout': ('u32 count, then count * 48-byte FieldLayoutRecord records',
               'include/overlays/field/field_interaction_start.h', 'FieldLayoutRecord'),
    'event_scripts': ('u16 section-relative offsets, followed by variable-length event instructions',
                      'src/overlays/field/actors/field_actor_key_ops.c', 'field_get_event_script'),
    'strings': ('u16 text-table-relative offsets, followed by encoded text and control tokens',
                'src/overlays/field/ui/field_text_window_api.c', 'FIELD_SCENE_STRING'),
    'actor_scripts': ('u16 section-relative offsets, followed by variable-length actor instructions',
                      'src/overlays/field/actors/field_actor_script_ops.c', 'field_run_actor_script_command'),
    'records': ('u32 section-relative offsets, followed by resources identified by a u16 ID',
                LOADER_FILE, 'field_update_scene'),
    'actors': ('Four u16 header fields, u32 section-relative offsets, 16-byte actor headers and 8-byte action slots',
               LOADER_FILE, 'field_update_scene'),
    'geometry': ('u32 actor resource boundaries; animation/frame directories, timed entries and sprite/placement records',
                 'src/overlays/field/actors/field_actor_runtime.c', 'field_advance_actor_animation_frame'),
    'images': ('u32 section-relative offsets, followed by TIM images', LOADER_FILE, 'field_upload_actor_image'),
    'portraits': ('u32 count, then 1184-byte records: 32 palette bytes and 1152 packed pixel bytes',
                 LOADER_FILE, 'field_update_scene'),
    'group_bounds': ('u32 count, then count * u32 stored words; word meanings remain unclassified',
                     LOADER_FILE, 'field_update_scene'),
}


def byte_count(text: str) -> int:
    return len(bytes.fromhex(text))


def payload_gaps(value) -> int:
    """Count explicit raw tails/gaps, not authoritative copies of decoded bytes."""
    if isinstance(value, dict):
        count = byte_count(value.get('trailing_bytes', ''))
        count += sum(span['size'] for span in value.get('undecoded', []))
        if value.get('in_use') is False and value.get('fields') is None:
            count += value['size']
        return count + sum(payload_gaps(item) for key, item in value.items()
                           if key not in ('trailing_bytes', 'undecoded'))
    if isinstance(value, list):
        return sum(payload_gaps(item) for item in value)
    return 0


def coverage(name: str, size: int, assets: list) -> dict:
    """Summarize structural decoding without claiming unknown fields are understood."""
    if not size:
        return {'status': 'empty', 'details': 'No bytes in this section.'}
    documents = [asset.document for asset in assets if asset.document is not None]
    document = documents[0] if documents else {}
    if name == 'layout':
        return {'status': 'decoded', 'details': 'Count and all stored record fields decoded; unnamed bits stay explicit.',
                'record_count': sum(asset.layout is not None for asset in assets), 'record_size': 48}
    if name == 'event_scripts':
        remaining = sum(span['size'] for span in document['undecoded'])
        unresolved = sum('unresolved' in entry for entry in document['entries'])
        diagnostics = len(document['diagnostics'])
        return {'status': 'partial' if remaining or unresolved or diagnostics else 'decoded',
                'details': 'Directory and statically decoded instructions; execution is not simulated.',
                'directory_entries': len(document['entries']), 'instructions': len(document['instructions']),
                'undecoded_bytes': remaining, 'unresolved_entries': unresolved, 'diagnostics': diagnostics}
    if name == 'actor_scripts':
        scripts = document['scripts']
        # Include bytes not reached through any directory entry, as well as
        # undecoded tails. Distinct scripts are counted once despite aliases.
        decoded = document['directory_size'] + sum(
            sum(byte_count(instruction['bytes']) for instruction in script['instructions']) for script in scripts)
        remaining = size - decoded
        unresolved = sum('unresolved' in entry for entry in document['entries'])
        stopped = sum(script['stop_reason'] not in ('end', 'range_end') for script in scripts)
        return {'status': 'partial' if remaining or unresolved or stopped else 'decoded',
                'details': 'Directory and distinct script ranges; unsupported instructions retain their bytes.',
                'directory_entries': len(document['entries']), 'scripts': len(scripts),
                'instructions': sum(len(script['instructions']) for script in scripts),
                'undecoded_bytes': remaining, 'unresolved_entries': unresolved, 'unsupported_ranges': stopped}
    if name == 'strings':
        remaining = sum(span['size'] for span in document['undecoded'])
        unresolved = sum('unresolved' in entry for entry in document['entries'])
        diagnostics = len(document['diagnostics'])
        return {'status': 'partial' if remaining or unresolved or diagnostics else 'decoded',
                'details': 'Directory, text tokens and control arguments decoded; external glyphs and macros remain symbolic.',
                'directory_entries': len(document['entries']), 'unique_strings': len(document['strings']),
                'undecoded_bytes': remaining, 'unresolved_entries': unresolved, 'diagnostics': diagnostics}
    if name == 'geometry':
        actors = document['actors']
        remaining = sum(span['size'] for span in document['undecoded'])
        remaining += sum(sum(span['size'] for span in actor.get('undecoded', [])) for actor in actors)
        frames = {frame['offset']: frame for actor in actors for frame in actor.get('frames', [])}.values()
        placements = sum(record['kind'] == 'placement' for frame in frames for record in frame['records'])
        unresolved = sum('unresolved' in entry for actor in actors for animation in actor.get('animations', [])
                         for entry in animation['frames'])
        return {'status': 'partial' if remaining or placements or unresolved else 'decoded',
                'details': 'Actor boundaries, animations, frame timing and sprites decoded; placement operands retain raw bytes.',
                'actor_resources': len(actors), 'animations': sum(len(actor.get('animations', [])) for actor in actors),
                'frames': sum(len(actor.get('frames', [])) for actor in actors),
                'placement_records': placements, 'undecoded_bytes': remaining, 'unresolved_frames': unresolved}
    if name == 'records':
        resources = [asset for asset in assets if asset.name == 'resource']
        battle = [asset.document['battle'] for asset in resources if 'battle' in asset.document]
        unknown = [asset for asset in resources if asset.size and not any(key in asset.document for key in ('battle', 'payload'))]
        remaining = sum(payload_gaps(asset.document) for asset in resources)
        # The shared character reader exposes named fields rather than mapping
        # every padding byte of the 0x250-byte saved record.
        guests = sum(asset.document.get('kind') == 'guest_templates' for asset in resources)
        return {'status': 'partial' if unknown or remaining or guests else 'decoded',
                'details': 'Known resource layouts, monster action descriptors and reward item fields decoded; diagnostics and raw tails stay explicit.',
                'directory_entries': len(document.get('entries', [])), 'unique_resources': len(resources),
                'battle_resources': len(battle), 'raw_resources': len(unknown),
                'decoded_payloads': sum('payload' in asset.document for asset in resources),
                'undecoded_bytes': remaining,
                'monster_templates': sum(len(resource['monsters']) for resource in battle)}
    if name == 'actors':
        unique = {actor['offset']: actor for actor in document['actors']}.values()
        remaining = byte_count(document['directory_trailing_bytes']) + sum(byte_count(actor['trailing_bytes']) for actor in unique)
        return {'status': 'partial' if remaining else 'decoded',
                'details': 'Header, directory, image slots and action fields decoded; unnamed values stay explicit.',
                'directory_entries': document['actor_count'], 'undecoded_bytes': remaining}
    if name == 'images':
        return {'status': 'partial', 'details': 'Directory decoded and TIM payloads validated; image fields stay in the TIM files.',
                'directory_entries': len(document['entries']),
                'unique_images': sum(asset.name == 'tim' for asset in assets)}
    if name == 'portraits':
        return {'status': 'partial' if any(asset.name == 'portrait' for asset in assets) else 'decoded',
                'details': 'Count and record boundaries decoded; palette and pixel bytes preserved raw.',
                'record_count': sum(asset.name == 'portrait' for asset in assets), 'record_size': 1184}
    if name == 'group_bounds':
        return {'status': 'partial' if document['count'] else 'decoded',
                'details': 'Count and stored words decoded; word meanings are not established.',
                'word_count': document['count']}
    return {'status': 'raw', 'details': 'Complete section preserved as binary; no internal fields decoded.'}


def describe_scene(source: Path, header: SceneHeader, size: int, assets: list) -> dict:
    """Build a self-contained format reference alongside the original byte map."""
    section_assets = {
        'layout': {'layout', 'layout_count', 'layout_record', 'chest'},
        'records': {'records', 'resource_directory', 'resource'},
        'images': {'images', 'image_directory', 'tim'},
        'portraits': {'portraits', 'portrait_count', 'portrait'},
    }
    sections = []
    for index, section in enumerate(header.sections(size)):
        names = section_assets.get(section.name, {section.name})
        selected = [asset for asset in assets if asset.name in names and asset.offset >= section.start
                    and asset.offset + asset.size <= section.end]
        layout, file, symbol = SECTION_FORMATS[section.name]
        sections.append({'name': section.name, 'offset': section.start, 'size': section.end - section.start,
                         'header_offset': index * 4, 'format': layout,
                         'coverage': coverage(section.name, section.end - section.start, selected),
                         'code_reference': {'file': file, 'symbol': symbol}})
    return {
        'source': source.name, 'size': size,
        'header': {'offset': 0, 'size': SCENE_HEADER.size, 'byte_order': 'little', 'section_alignment': 4,
                   'pointer_base': 'IMG byte zero', 'code_reference': LOADER,
                   'fields': [{'name': name, 'offset': index * 4, 'type': 'u32', 'size': 4, 'section_offset': value}
                              for index, (name, value) in enumerate(zip(header._fields, header))]},
        'coverage_notes': [
            'Coverage describes exported structure, not whether every field meaning is known.',
            'decoded: stored layout mapped; partial: raw payloads, unclassified words or decoding gaps remain.',
            'raw: preserved binary; empty: section has no bytes. All statuses retain the original bytes.',
        ],
        'value_notes': [
            'Stored fields and raw bytes are the format reference; flags split into bits and ASCII names are interpretations.',
            'objects.yaml summarizes links within this IMG. Conditions and possible script operations are interpretations.',
            'A selected external FIELD reference adds annotations; extraction defaults to this IMG alone.',
            'Decoded YAML is descriptive. Byte recovery uses byte-map files and layout record_bytes.',
        ],
        'sections': sections,
    }


def format_structure(document: dict) -> list[str]:
    """Render the same header, coverage and code references as scene.yaml."""
    header = document['header']
    lines = ['HEADER LAYOUT', '40 bytes: ten little-endian u32 section offsets, relative to IMG byte zero.',
             'Sections start on four-byte boundaries; equal offsets describe an empty section.',
             'Header byte  Field             Stored section offset']
    for field in header['fields']:
        lines.append(f"0x{field['offset']:02X}         {field['name']:<18}0x{field['section_offset']:08X}")
    lines.extend(['', 'SECTION MAP', 'Section           IMG offset    Bytes   Coverage'])
    for section in document['sections']:
        lines.append(f"{section['name']:<18}0x{section['offset']:08X}  {section['size']:7d}   {section['coverage']['status']}")
    lines.extend(['', 'STRUCTURE AND DECODING COVERAGE'])
    for section in document['sections']:
        report = section['coverage']
        lines.append(f"  {section['name']} [{report['status']}]: {section['format']}")
        lines.append('    ' + report['details'])
        counts = [f"{key.replace('_', ' ')}={value}" for key, value in report.items() if isinstance(value, int)]
        if counts:
            lines.append('    ' + '; '.join(counts))
        reference = section['code_reference']
        lines.append(f"    Code: {reference['file']} : {reference['symbol']}")
    lines.extend([''] + document['coverage_notes'])
    return lines
