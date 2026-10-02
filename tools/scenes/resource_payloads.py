"""Stored payload layouts for scene resources other than the battle container.

The readers follow FIELD's typed resource consumers. They do not execute item
scripts, generate equipment or load names from another game file. Offsets are
absolute IMG positions; bytes with no established layout remain explicit.
"""

from __future__ import annotations

import struct

from tools.overlays import saved_game, text_table
from tools.scenes.strings import read_text_table, text_preview, uncovered


TEXT_IDS = frozenset((8, 9, 12, 0x100, 0x101, 0x103, 0x104))
RESOURCE_NAMES = {
    1: 'battle', 3: 'guest_templates', 4: 'equipment_generation', 5: 'item_templates',
    6: 'triggers', 7: 'companion_templates', 8: 'item_names', 9: 'coordinate_labels',
    10: 'shop_lists', 11: 'effect_picks', 12: 'text', 13: 'weapon_templates',
    14: 'armor_templates', 15: 'instrument_generation', 16: 'effect_thresholds',
    17: 'item_values', 18: 'region_effects', 19: 'nibble_table',
    0x100: 'golem_names', 0x101: 'object_text', 0x102: 'object_values',
    0x103: 'item_entry_text', 0x104: 'item_column_text',
}
CODE_REFERENCES = {
    1: ('field_records.h', 'FieldActionDescriptor'),
    3: ('field_active_record_ops.c', 'FieldGuestTemplateTable'),
    4: ('field_records.h', 'FieldItemTable'),
    5: ('field_resource_table_ops.c', 'FieldItemTemplateTable'),
    6: ('field_records.h', 'FieldTriggerTable'),
    7: ('field_saved_slot_ops.c', 'FieldCompanionTemplateTable'),
    8: ('field_generated_record_ops.c', 'FieldGeneratedItemNameTable'),
    9: ('field_action_modifiers.c', 'field_select_coordinate_labels'),
    10: ('field_script_ops.c', 'field_script_op_37'),
    11: ('field_record_effect_ops.c', 'FieldSlotEffectTable'),
    12: ('field_script_ops.c', 'FieldTextResource'),
    13: ('field_active_record_ops.c', 'FieldItemTemplateTable'),
    14: ('field_active_record_ops.c', 'FieldItemTemplateTable'),
    15: ('field_records.h', 'FieldItemGridTable'),
    16: ('field_record_effect_ops.c', 'FieldEffectThresholdTable'),
    17: ('field_record_table_ops.c', 'FieldItemValueTables'),
    18: ('field_record_effect_ops.c', 'FieldRegionEffectTable'),
    19: ('field_script_commands.c', 'FieldNibbleTable'),
    0x100: ('field_group_derived_stats.c', 'GolemNameText'),
    0x101: ('field_menu_ops.c', 'FIELD_OBJECT_TEXT_RESOURCE'),
    0x102: ('field_menu_ops.c', 'field_menu_lookup_object_value'),
    0x103: ('field_menu_ops.c', 'FIELD_ITEM_ENTRY_TEXT_RESOURCE'),
    0x104: ('field_menu_ops.c', 'FIELD_ITEM_COLUMN_TEXT_RESOURCE'),
}


def action_descriptor(data: bytes, offset: int) -> dict:
    """Decode the eight-byte battle descriptor, including its handler union."""
    if len(data) != 8:
        raise ValueError('action descriptor must be eight bytes')
    info, params = struct.unpack('<II', data)
    handler = info >> 24
    fields = {'attack_stat': params & 15, 'defense_stat': (params >> 4) & 15}
    if handler in (0, 1, 2, 3, 6):
        fields['element_mask'] = (params >> 8) & 255
    if handler in (0, 4):
        fields.update(chance=(params >> 16) & 15, effect=(params >> 20) & 15, duration=params >> 24)
    if handler == 4:
        fields.update(match=(params >> 8) & 15, doubled=(params >> 12) & 15)
    elif handler in (1, 3, 6):
        fields.update(base=(params >> 16) & 255, spread=params >> 24)
    elif handler == 2:
        fields.update(attacker_scale=(params >> 16) & 15, attacker_stat=(params >> 20) & 15,
                      target_scale=(params >> 24) & 15, target_stat=params >> 28)
    elif handler == 5:
        fields.update(operation=(params >> 8) & 255, value=params >> 16)
    return {'offset': offset, 'info': info, 'params': params, 'kind': info & 15,
            'side_rule': (info >> 4) & 3, 'defense_slot': (info >> 6) & 3,
            'status_intensity_shift': (info >> 8) & 7, 'unknown_info_bits': (info >> 11) & 31,
            'power': (info >> 16) & 255, 'handler': handler,
            'parameter_fields': fields, 'bytes': data.hex(' ')}


def item_record(raw: bytes, offset: int, encoding: str = 'us') -> dict:
    """Use the shared saved-game reader while retaining free/empty records too."""
    record = saved_game.item_record(
        raw, lambda data: text_preview(data, encoding),
        text_table.TWO_BYTE_CODES[encoding], include_empty=True,
    )
    return {'offset': offset, 'size': len(raw), 'in_use': raw[0] != 0,
            'fields': record, 'record_bytes': raw.hex(' ')}


def companion_record(raw: bytes, offset: int, encoding: str) -> dict:
    progress = struct.unpack_from('<I', raw, 0x18)[0]
    stats = struct.unpack_from('<8H', raw, 0x28)
    return {'offset': offset, 'name': text_preview(raw[:21], encoding), 'name_bytes': raw[:21].hex(' '),
            'species': raw[21], 'egg_species': raw[22], 'unknown_17': raw[23],
            'level': progress & 255, 'experience': progress >> 8,
            'hp': struct.unpack_from('<H', raw, 0x1C)[0], 'power': struct.unpack_from('<H', raw, 0x1E)[0],
            'equipment_totals': list(struct.unpack_from('<4H', raw, 0x20)),
            'stats': [{'value_times_four': value & 511, 'growth': value >> 9} for value in stats],
            'unknown_38_bytes': raw[0x38:0x3C].hex(' '), 'weapon_id': raw[0x3C], 'armor_ids': list(raw[0x3D:0x40]),
            'padding_40_bytes': raw[0x40:0x42].hex(' '), 'hatch_counter': struct.unpack_from('<H', raw, 0x42)[0],
            'status': struct.unpack_from('<I', raw, 0x44)[0], 'unknown_48': struct.unpack_from('<I', raw, 0x48)[0],
            'stat_growth': [{'rate': value & 15, 'accumulator': value >> 4} for value in raw[0x4C:0x54]],
            'total_growth': [{'rate': value & 15, 'accumulator': value >> 4} for value in raw[0x54:0x58]],
            'extra_growth': struct.unpack_from('<I', raw, 0x58)[0],
            'unique_id': struct.unpack_from('<i', raw, 0x5C)[0]}


def counted_records(data: bytes, size: int) -> tuple[int, int]:
    count = struct.unpack_from('<H', data, 2)[0]
    end = 4 + count * size
    if end > len(data):
        raise ValueError(f'counted records exceed scene resource: count={count}, stride={size}, '
                         f'required={end} bytes, stored={len(data)} bytes')
    return count, end


def read_shop_lists(data: bytes, offset: int) -> dict:
    """Header word then u32 offsets; shared offsets remain separate list slots."""
    if len(data) < 8:
        raise ValueError('truncated shop directory')
    first = struct.unpack_from('<I', data, 4)[0]
    if first < 8 or first % 4 or first > len(data):
        raise ValueError('invalid shop directory size')
    pointers = struct.unpack_from(f'<{first // 4 - 1}I', data, 4)
    lists, spans = [], [(0, first)]
    for index, start in enumerate(pointers):
        if start < first or start % 4 or start + 4 > len(data):
            raise ValueError('shop list offset outside payload')
        count = struct.unpack_from('<I', data, start)[0]
        end = start + 4 + count * 4
        if end > len(data):
            raise ValueError('shop entries exceed resource')
        entries = []
        for item_index, word in enumerate(struct.unpack_from(f'<{count}I', data, start + 4)):
            entries.append({'index': item_index, 'offset': offset + start + 4 + item_index * 4,
                            'word': word, 'item': word & 255, 'generated': bool(word & 256), 'price': word >> 9})
        lists.append({'index': index, 'offset': offset + start, 'count': count, 'entries': entries})
        spans.append((start, end))
    return {'relative_offsets': list(pointers), 'lists': lists, 'undecoded': uncovered(data, spans, offset)}


def read_generation(data: bytes, offset: int, instrument: bool) -> dict:
    """Fixed generation tables followed by item-script bytes, kept unexecuted."""
    result = {}
    if instrument:
        end = 4 + 64 + 64 * 8 + 192 * 2
        if len(data) < end:
            raise ValueError('truncated instrument generation tables')
        result['grid'] = [list(data[4 + row * 8:12 + row * 8]) for row in range(8)]
        result['materials'] = []
        for index in range(64):
            start = 68 + index * 8
            instruments = []
            for kind in range(4):
                instruments.append({'packed_cell': data[start + kind * 2],
                                    'power': data[start + kind * 2 + 1]})
            result['materials'].append({'index': index, 'offset': offset + start,
                                        'instruments': instruments})
        result['secondary_scripts'] = list(struct.unpack_from('<192H', data, 580))
    else:
        end = 4 + 32 * 12 + 64 * 20 + 192 * 4 + 160 * 8
        if len(data) < end:
            raise ValueError('truncated equipment generation tables')
        for name, start in [('weapon_types', 4), ('armor_types', 196)]:
            entries = []
            for index in range(16):
                position = start + index * 12
                entries.append({'index': index, 'offset': offset + position,
                                'scripts': list(struct.unpack_from('<2H', data, position)),
                                'weights': list(data[position + 4:position + 8]),
                                'factors': list(data[position + 8:position + 12])})
            result[name] = entries
        result['materials'] = []
        for index in range(64):
            start = 388 + index * 20
            script, divisor = struct.unpack_from('<HH', data, start)
            result['materials'].append({'index': index, 'offset': offset + start, 'script': script, 'divisor': divisor,
                                        'weights': list(data[start + 4:start + 8]),
                                        'multipliers': list(data[start + 8:start + 12]), 'costs': list(data[start + 12:start + 20])})
        result['secondary_items'] = []
        for index in range(192):
            start = 1668 + index * 4
            result['secondary_items'].append({'index': index + 64, 'pool_bonus': data[start],
                                              'unknown_01': data[start + 1],
                                              'script': struct.unpack_from('<H', data, start + 2)[0]})
        result['slot_values'] = [list(struct.unpack_from('<4H', data, 2436 + index * 8)) for index in range(160)]
    result['script_pointer_base'] = offset
    result['undecoded'] = uncovered(data, [(0, end)], offset)
    return result


def read_payload(data: bytes, offset: int, resource_id: int, encoding: str = 'us') -> dict | None:
    """Return a recognized payload; unknown IDs keep their entire binary range."""
    if resource_id not in RESOURCE_NAMES or resource_id == 1:
        return None
    if len(data) < 4:
        raise ValueError('truncated scene resource header')
    result = {'header_word': struct.unpack_from('<I', data)[0]}
    if resource_id in TEXT_IDS:
        result['texts'] = read_text_table(data[4:], offset + 4, encoding)
        return result
    if resource_id == 10:
        return {**result, **read_shop_lists(data, offset)}
    if resource_id in (4, 15):
        return {**result, **read_generation(data, offset, resource_id == 15)}
    end = len(data)
    if resource_id in (5, 13, 14):
        if resource_id == 5:
            count, end = counted_records(data, 64)
            result['count'] = count
        else:
            # Weapon/armor consumers index these arrays directly. Their high
            # header word is zero in the original corpus, despite the C count view.
            count = (len(data) - 4) // 64
            end = 4 + count * 64
            result.update(header_high_word=struct.unpack_from('<H', data, 2)[0], record_count=count)
        items = []
        for index in range(count):
            start = 4 + index * 64
            items.append({'index': index, **item_record(data[start:start + 64], offset + start, encoding)})
        result['items'] = items
    elif resource_id == 6:
        count, end = counted_records(data, 12)
        regions = []
        for index in range(count):
            start = 4 + index * 12
            x0, z0, x1, z1, command, unknown = struct.unpack_from('<6H', data, start)
            regions.append({'index': index, 'offset': offset + start, 'min_x': x0, 'min_z': z0,
                            'max_x': x1, 'max_z': z1, 'command': command, 'unknown_0a': unknown,
                            'kind': 'script' if command & 0x8000 else 'battle', 'target': command & 0x7FFF})
        result.update(count=count, regions=regions)
    elif resource_id == 7:
        count, end = counted_records(data, 96)
        companions = []
        for index in range(count):
            start = 4 + index * 96
            companions.append({'index': index, **companion_record(data[start:start + 96], offset + start, encoding)})
        result.update(count=count, companions=companions)
    elif resource_id == 3:
        count, end = counted_records(data, 4 + 4 * saved_game.CHARACTER_SIZE)
        guests = []
        for index in range(count):
            start = 4 + index * (4 + 4 * saved_game.CHARACTER_SIZE)
            banks = []
            for bank in range(4):
                position = start + 4 + bank * saved_game.CHARACTER_SIZE
                fields = saved_game.character_record(data[position:position + saved_game.CHARACTER_SIZE],
                                                     lambda raw: text_preview(raw, encoding), text_table.TWO_BYTE_CODES[encoding])
                banks.append({'index': bank, 'offset': offset + position, **fields})
            guests.append({'index': index, 'offset': offset + start,
                           'id': struct.unpack_from('<i', data, start)[0], 'banks': banks})
        result.update(count=count, guests=guests)
    elif resource_id == 17:
        if len(data) < 204 or (len(data) - 204) % 2:
            raise ValueError('truncated item value tables')
        result.update(type_values=list(struct.unpack_from('<36H', data, 4)),
                      subtype_values=list(struct.unpack_from('<64H', data, 76)),
                      special_values=list(struct.unpack_from(f'<{(len(data) - 204) // 2}H', data, 204)))
    elif resource_id == 0x102:
        if (len(data) - 4) % 4:
            raise ValueError('truncated object value pair')
        result['objects'] = [{'index': index, 'values': list(values)}
                             for index, values in enumerate(struct.iter_unpack('<HH', data[4:]))]
    elif resource_id == 19:
        rows = (len(data) - 4) // 48
        end = 4 + rows * 48
        result['first_column'] = 0x60
        result['rows'] = [{'index': index, 'offset': offset + 4 + index * 48,
                           'cells': list(data[4 + index * 48:52 + index * 48])} for index in range(rows)]
    elif resource_id == 11:
        if len(data) < 132:
            raise ValueError('truncated effect picks')
        result.update(picks=[list(data[4 + index * 16:20 + index * 16]) for index in range(8)], codes=list(data[132:]))
    elif resource_id == 16:
        if len(data) < 340:
            raise ValueError('truncated effect thresholds')
        end = 340
        result.update(thresholds=[{'low': data[4 + index * 2], 'high': data[5 + index * 2]} for index in range(8)],
                      threshold_indexes=[list(data[20 + index * 8:28 + index * 8]) for index in range(40)])
    elif resource_id == 18:
        if len(data) < 804:
            raise ValueError('truncated region effects')
        count = (len(data) - 4) // 20
        end = 4 + count * 20
        result['runtime_effect_count'] = 40
        result['record_count'] = count
        result['effects'] = []
        for index in range(count):
            start = 4 + index * 20
            effects = []
            for slot in range(4):
                effects.append({'kind': data[start + 12 + slot * 2], 'value': data[start + 13 + slot * 2]})
            result['effects'].append({'index': index, 'offset': offset + start,
                                      'stat_deltas': list(data[start:start + 8]),
                                      'total_deltas': list(data[start + 8:start + 12]), 'effects': effects})
    result['trailing_bytes'] = data[end:].hex(' ')
    return result
