"""Stored text controls, animation records, and typed resource payloads."""

import io
from pathlib import Path
import struct
import tempfile
import unittest
from contextlib import redirect_stdout

import yaml

from tools.scenes.field_scene import extract, reconstruct
from tools.scenes.geometry import read_animation_resource, read_geometry, format_geometry, main as geometry_main
from tools.scenes.presentation import dump_yaml
from tools.scenes.resource_payloads import action_descriptor, read_payload
from tools.scenes.scene_format import SceneHeader
from tools.scenes.scene_resources import SceneResource, format_resource, main as resources_main
from tools.scenes.strings import read_text, read_text_table, format_strings, main as strings_main
from tools.scenes.tests.test_actor_sections import replace_section
from tools.scenes.tests.test_field_scene import sample_scene
from tools.scenes.tests.test_identify_img import make_scene


def animation_resource(wide=False):
    """Two aliases of a sequence, one frame containing a sprite and a sound command."""
    stride = 4 if wide else 2
    start = 14
    frame = start + 1 + 2 * stride
    pointer = start | (0x8000 if wide else 0)
    header = struct.pack('<HHBB4H', 1, 10, 2, int(wide), pointer, pointer, 1, frame)
    sequence = bytes((2, 0, 3, 7, 9, 0, 0, 8, 10)) if wide else bytes((2, 0, 3, 0, 5))
    sprite = bytes.fromhex('fe 03 10 20 08 09 02 c2 04')
    if wide:
        sprite += bytes.fromhex('ff 00')
    placement = bytearray(17 if wide else 9)
    placement[7] = 0x25
    return header + sequence + bytes((2,)) + sprite + placement


def decoded_scene():
    strings = bytes.fromhex('04 00 04 00') + b'Hello\x14\x1f\0world\x06'
    strings += bytes(-len(strings) % 4)
    data = replace_section(make_scene(), 'strings', strings)
    data = replace_section(data, 'actors', struct.pack('<4HI', 127, 0, 0, 1, 12) + bytes(16))
    resource = animation_resource(True)
    return replace_section(data, 'geometry', struct.pack('<II', 8, 8 + len(resource)) + resource)


class TextTests(unittest.TestCase):
    def test_dictionary_zero_argument_and_finish_do_not_merge_strings(self):
        raw = b'Hi\x1f\0\x0e\0\x06unused'
        text, tokens, end, terminated = read_text(raw)
        self.assertEqual(text, 'Hi for{macro:0}{finish}')
        self.assertEqual(end, 7)
        self.assertTrue(terminated)
        self.assertEqual(tokens[0]['text'], 'Hi')
        self.assertEqual(bytes.fromhex(' '.join(token['bytes'] for token in tokens)), raw[:end])

    def test_aliases_and_interior_string_pointers_keep_directory_slots(self):
        raw = struct.pack('<3H', 6, 6, 7) + b'AB\0\xaa'
        doc = read_text_table(raw, 0x100)
        self.assertEqual([entry['string_index'] for entry in doc['entries']], [0, 0, 1])
        self.assertEqual([entry['text'] for entry in doc['strings']], ['AB', 'B'])
        self.assertEqual(doc['undecoded'], [{'offset': 0x109, 'size': 1, 'bytes': 'aa'}])
        self.assertIn('[002] IMG 0x00000107\n  B', format_strings(doc))

    def test_japanese_zero_glyph_is_not_an_end_code(self):
        text, tokens, end, terminated = read_text(bytes.fromhex('1d 00 00'), encoding='jp')
        self.assertEqual(text, '{1D 00}')
        self.assertEqual(tokens[0]['bytes'], '1d 00')
        self.assertEqual(end, 3)
        self.assertTrue(terminated)

    def test_prefixed_glyph_run_consumes_both_us_argument_bytes(self):
        text, tokens, end, terminated = read_text(bytes.fromhex('12 00 02 00'))
        self.assertEqual(tokens[0]['arguments'], [0, 2])
        self.assertEqual(end, 4)
        self.assertTrue(terminated)
        self.assertIn('prefixed_glyph_run:0,2', text)

    def test_incomplete_tokens_and_invalid_pointers_remain_explicit(self):
        doc = read_text_table(struct.pack('<2H', 4, 2) + b'\x0e', 0)
        self.assertIn('unresolved', doc['entries'][1])
        self.assertEqual(doc['strings'][0]['tokens'][0]['kind'], 'truncated')
        self.assertFalse(doc['strings'][0]['terminated'])
        self.assertEqual(len(doc['diagnostics']), 1)
        with self.assertRaisesRegex(ValueError, 'directory size'):
            read_text_table(bytes.fromhex('03 00 00 00'), 0)
        with self.assertRaisesRegex(ValueError, 'encoding'):
            read_text(b'A', encoding='other')


class GeometryTests(unittest.TestCase):
    def test_narrow_sprites_and_aliased_sequences(self):
        doc = read_animation_resource(animation_resource(), 0x100)
        self.assertEqual(doc['animations'][0]['offset'], doc['animations'][1]['offset'])
        self.assertEqual(doc['animations'][0]['frames'][1]['duration'], 5)
        sprite, placement = doc['frames'][0]['records']
        self.assertEqual((sprite['x'], sprite['y']), (-2, 3))
        self.assertEqual((sprite['u'], sprite['v'], sprite['width'], sprite['height']), (16, 32, 8, 9))
        self.assertTrue(sprite['mirror_u'])
        self.assertTrue(sprite['mirror_v'])
        self.assertEqual(placement['command_name'], 'sound')
        self.assertEqual(doc['undecoded'], [])

    def test_wide_sprites_and_four_byte_timing_entries(self):
        doc = read_animation_resource(animation_resource(True), 0x100)
        self.assertTrue(doc['wide_coordinates'])
        entry = doc['animations'][0]['frames'][0]
        self.assertEqual((entry['height'], entry['motion']), (7, 9))
        sprite, placement = doc['frames'][0]['records']
        self.assertEqual((sprite['size'], placement['size']), (11, 17))
        self.assertEqual(sprite['x'], -2)
        self.assertEqual(len(bytes.fromhex(placement['operand_bytes'])), 16)

    def test_malformed_animation_and_frame_ranges_are_rejected(self):
        original = animation_resource(True)
        for position, value in ((2, 0xFFFF), (6, 1), (12, 0xFFFF)):
            raw = bytearray(original)
            struct.pack_into('<H', raw, position, value)
            with self.subTest(position=position), self.assertRaises(ValueError):
                read_animation_resource(raw, 0)
        with self.assertRaisesRegex(ValueError, 'frame record'):
            read_animation_resource(original[:-1], 0)

    def test_geometry_tracks_actor_slots_and_reports_section_gaps(self):
        scene = decoded_scene()
        doc = read_geometry(scene, SceneHeader.parse(scene))
        self.assertEqual(doc['actor_count'], 1)
        self.assertEqual(doc['actors'][0]['resource_index'], 3)
        self.assertIn('placement 5 (sound)', format_geometry(doc))
        self.assertEqual(doc['undecoded'], [])
        broken = bytearray(scene)
        struct.pack_into('<I', broken, SceneHeader.parse(scene).geometry + 4, len(scene))
        with self.assertRaisesRegex(ValueError, 'boundaries'):
            read_geometry(broken, SceneHeader.parse(broken))


class ResourcePayloadTests(unittest.TestCase):
    def test_handler_specific_battle_action_fields(self):
        descriptor = action_descriptor(struct.pack('<II', 0x045A0712, 0xABCDEF21), 0x800)
        self.assertEqual(descriptor['handler'], 4)
        self.assertEqual(descriptor['power'], 0x5A)
        self.assertEqual(descriptor['parameter_fields'], {
            'attack_stat': 1, 'defense_stat': 2, 'match': 15, 'doubled': 14,
            'chance': 13, 'effect': 12, 'duration': 171,
        })
        unknown = action_descriptor(struct.pack('<II', 0xFF000000, 0xFFFFFFFF), 0)
        self.assertEqual(unknown['params'], 0xFFFFFFFF)
        self.assertEqual(unknown['parameter_fields'], {'attack_stat': 15, 'defense_stat': 15})

    def test_reward_items_and_empty_items_keep_complete_fields(self):
        raw = bytearray(64)
        struct.pack_into('<I', raw, 0x14, 0x002A0C00)
        doc = read_payload(struct.pack('<HH', 5, 1) + raw, 0x100, 5)
        item = doc['items'][0]
        self.assertFalse(item['in_use'])
        self.assertEqual(item['fields']['material'], 42)
        self.assertEqual(bytes.fromhex(item['record_bytes']), raw)
        direct = read_payload(struct.pack('<HH', 13, 0) + raw, 0, 13)
        self.assertEqual(direct['record_count'], 1)
        self.assertEqual(len(direct['items']), 1)

    def test_trigger_commands_distinguish_scripts_and_battle_groups(self):
        data = struct.pack('<HH12H', 6, 2, 1, 2, 3, 4, 0x8012, 0xAAAA, 5, 6, 7, 8, 9, 0)
        doc = read_payload(data, 0x200, 6)
        self.assertEqual([(region['kind'], region['target']) for region in doc['regions']], [('script', 18), ('battle', 9)])
        self.assertEqual(doc['regions'][0]['unknown_0a'], 0xAAAA)

    def test_shared_shop_lists_decode_packed_prices(self):
        word = 7 | 256 | (123 << 9)
        raw = struct.pack('<6I', 0xAAAA000A, 12, 12, 2, word, 0)
        doc = read_payload(raw, 0x100, 10)
        self.assertEqual(doc['relative_offsets'], [12, 12])
        entry = doc['lists'][0]['entries'][0]
        self.assertEqual((entry['item'], entry['generated'], entry['price']), (7, True, 123))
        self.assertEqual(doc['lists'][0]['offset'], doc['lists'][1]['offset'])
        self.assertEqual(doc['undecoded'], [])
        with self.assertRaises(ValueError):
            read_payload(raw[:-1], 0, 10)

    def test_text_resource_offsets_are_relative_to_body_after_header(self):
        raw = struct.pack('<I', 0xBBAA0101) + struct.pack('<H', 2) + b'Hi\0'
        doc = SceneResource(4, raw, (0,)).document(0x100)
        self.assertEqual(doc['payload']['texts']['strings'][0]['offset'], 0x10A)
        self.assertIn('Hi', format_resource(doc))
        self.assertEqual(doc['code_reference']['symbol'], 'FIELD_OBJECT_TEXT_RESOURCE')

    def test_invalid_known_payloads_and_unknown_ids_preserve_raw_range(self):
        doc = SceneResource(4, struct.pack('<HH', 7, 65) + bytes(64 * 96), (0,)).document(0)
        self.assertIn('counted records exceed', doc['diagnostics'][0])
        self.assertNotIn('payload', doc)
        unknown = SceneResource(4, bytes.fromhex('fe 7f 00 00 11 22'), (0, 2)).document(0)
        self.assertEqual(unknown['kind'], 'unknown')
        self.assertNotIn('payload', unknown)
        self.assertIn('companion binary', format_resource(unknown))

    def test_fixed_generation_tables_keep_item_scripts_at_original_offsets(self):
        raw = struct.pack('<I', 15) + bytes(960) + b'\x11\x22\x33\x44'
        doc = read_payload(raw, 0x100, 15)
        self.assertEqual(doc['script_pointer_base'], 0x100)
        self.assertEqual(doc['undecoded'], [{'offset': 0x4C4, 'size': 4, 'bytes': '11 22 33 44'}])
        with self.assertRaisesRegex(ValueError, 'generation'):
            read_payload(bytes.fromhex('04 00 00 00'), 0, 4)


class DecodedExportTests(unittest.TestCase):
    def test_txt_yaml_and_binary_recovery_for_all_new_sections(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / 'sample.IMG'
            scene = decoded_scene()
            source.write_bytes(scene)
            output = root / 'export'
            extract(source, output)
            self.assertEqual(reconstruct(output), scene)
            for name, command in [('strings', strings_main), ('geometry', geometry_main)]:
                doc = yaml.safe_load((output / name / 'data.yaml').read_text())
                with redirect_stdout(io.StringIO()) as text:
                    self.assertEqual(command([str(source)]), 0)
                self.assertEqual(yaml.safe_load(text.getvalue()), doc)
                with redirect_stdout(io.StringIO()) as text:
                    self.assertEqual(command([str(source), '--format', 'text']), 0)
                self.assertEqual(text.getvalue(), (output / name / 'data.txt').read_text())
            overview = yaml.safe_load((output / 'scene.yaml').read_text())
            geometry = next(section for section in overview['sections'] if section['name'] == 'geometry')
            self.assertEqual(geometry['coverage']['placement_records'], 1)
            self.assertEqual(geometry['coverage']['status'], 'partial')

    def test_battle_resource_txt_matches_structured_actions_and_items(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / 'sample.IMG'
            source.write_bytes(sample_scene())
            output = root / 'export'
            extract(source, output)
            doc = yaml.safe_load((output / 'records/000.yaml').read_text())
            self.assertEqual(doc['battle']['monsters'][0]['actions'][0]['info'], 0x03020100)
            self.assertIn('parameter fields:', (output / 'records/000.txt').read_text())
            self.assertIn('item:', (output / 'records/000.txt').read_text())
            with redirect_stdout(io.StringIO()) as text:
                self.assertEqual(resources_main([str(source), '--format', 'text']), 0)
            self.assertEqual(text.getvalue().rstrip(), (output / 'records/000.txt').read_text().rstrip())

    def test_yaml_keeps_integer_fields_and_descriptive_pointer_base_separate(self):
        doc = {'info': 0xFFFFFFFF, 'params': 0, 'pointer_base': 'IMG byte zero',
               'status': 'decoded', 'frame_table_offset': 42}
        self.assertEqual(yaml.safe_load(dump_yaml(doc)), doc)


if __name__ == '__main__':
    unittest.main()
