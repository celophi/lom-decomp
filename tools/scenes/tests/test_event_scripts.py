"""Event operand encodings, control-flow boundaries and export byte preservation."""

import io
from pathlib import Path
import struct
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout

import yaml

from tools.scenes.event_scripts import DecodeError, OperandReader, decode_instruction, describe_variable
from tools.scenes.event_scripts import format_disassembly, main, read_event_scripts
from tools.scenes.field_scene import COMMON_CHEST_INITIALIZER, extract, reconstruct
from tools.scenes.presentation import dump_yaml
from tools.scenes.scene_format import SceneHeader
from tools.scenes.tests.test_actor_sections import replace_section
from tools.scenes.tests.test_identify_img import make_scene


def event_scene(code, entries=(2,)):
    section = struct.pack(f'<{len(entries)}H', *entries) + code
    section += bytes(-len(section) % 4)
    return replace_section(make_scene(), 'event_scripts', section)


def read_scene(scene):
    return read_event_scripts(scene, SceneHeader.parse(scene))


class EventOperandTests(unittest.TestCase):
    def test_basic_types_signed_words_and_owner_values(self):
        reader = OperandReader(bytes.fromhex('40 e0 ff 34 12 ff ff ff ff'), 0)
        self.assertEqual(reader.basic(0), {'encoding': 'variable', 'reference': 0xE040})
        self.assertEqual(reader.basic(1, owner=True)['meaning'], 'owner')
        self.assertEqual(reader.basic(2)['value'], 0x1234)
        self.assertEqual(reader.basic(3), {'encoding': 's32', 'value': -1})
        self.assertEqual(reader.position, 9)
        with self.assertRaisesRegex(DecodeError, 'truncated_operands'):
            reader.basic(2)

    def test_nibble_types_preserve_encoding_and_unassigned_values(self):
        cases = [(0, b'\x7f', 127), (1, b'\x34\x12', 0x1234),
                 (2, b'\xff' * 4, -1), (4, b'\x34\x12', 0x1234),
                 (5, b'\x34\x12', 0x11234), (8, b'', 0), (9, b'', 1), (10, b'', 255)]
        for kind, raw, expected in cases:
            with self.subTest(kind=kind):
                reader = OperandReader(raw, 0)
                self.assertEqual(reader.nibble(kind)['value'], expected)
                self.assertEqual(reader.position, len(raw))
        for kind in (6, 7):
            self.assertIn('unresolved', OperandReader(b'', 0).nibble(kind))
        self.assertEqual(OperandReader(b'\x20\xe0', 0).nibble(3)['reference'], 0xE020)
        with self.assertRaisesRegex(DecodeError, 'unknown_operand_type'):
            OperandReader(b'', 0).nibble(11)

    def test_variable_address_bits_apply_owner_base_only_to_runtime_kinds(self):
        local = describe_variable(0xE045)
        self.assertEqual((local['scope'], local['word_index'], local['bit_shift']), ('runtime', 2, 5))
        self.assertTrue(local['owner_relative'])
        saved = describe_variable(0x8BC0)
        self.assertEqual(saved['scope'], 'saved_game')
        self.assertFalse(saved['owner_relative'])

    def test_basic_descriptor_order_differs_between_handlers(self):
        # Actor/target read high bits for FACE; actor/target/resource read low bits for ANIMATION.
        face = decode_instruction(bytes.fromhex('2d 60 ff 34 12'), 0)
        self.assertEqual(face['operands']['actor']['meaning'], 'owner')
        self.assertEqual(face['operands']['target']['value'], 0x1234)
        animation = decode_instruction(bytes.fromhex('35 39 ff 34 12 ff ff ff ff'), 0)
        self.assertEqual(animation['operands']['actor']['meaning'], 'owner')
        self.assertEqual(animation['operands']['target']['value'], 0x1234)
        self.assertEqual(animation['operands']['resource_index']['value'], -1)
        self.assertEqual(animation['size'], 9)

    def test_pair_and_extended_descriptors_are_low_nibble_first(self):
        instruction = decode_instruction(bytes.fromhex('40 35 40 e0 20 e0'), 0)
        self.assertEqual(instruction['operands']['destination']['reference'], 0xE040)
        self.assertTrue(instruction['operands']['destination']['keep_top_bit'])
        self.assertEqual(instruction['operands']['value']['encoding'], 'variable')
        extended = decode_instruction(bytes.fromhex('85 81 82 2d 81 ff 01 ff 00'), 0)
        self.assertEqual(extended['operands']['scene_id']['value'], 0x812D)
        self.assertEqual(extended['operands']['object_id']['value'], 0)
        self.assertEqual(extended['operands']['audio']['value'], 0xFF01FF)
        self.assertEqual(extended['successors'], [{'kind': 'scene_change'}])
        # The implicit 255 constant is an owner only for the appropriate argument.
        battle = decode_instruction(bytes.fromhex('84 aa aa'), 0)
        self.assertEqual(battle['operands']['actor']['meaning'], 'owner')
        self.assertNotIn('meaning', battle['operands']['sound'])

    def test_chest_initializer_fields_follow_known_record_layout(self):
        first = decode_instruction(COMMON_CHEST_INITIALIZER, 0)
        self.assertEqual(first['operands']['destination']['reference'], 0xC020)
        second = decode_instruction(COMMON_CHEST_INITIALIZER, first['size'])
        self.assertEqual(second['command'], 'read_record_bits')
        self.assertEqual(second['record_field'], {'base': 'actor', 'element_width': 1,
                                                 'element_index': 8, 'bit_shift': 0, 'bit_count': 16})
        self.assertEqual(second['operands']['destination']['reference'], 0xE040)
        self.assertEqual(second['size'], 10)

    def test_copy_owner_variable_preserves_the_skipped_halfword(self):
        instruction = decode_instruction(bytes.fromhex('0b 05 ff 20 e0 aa bb 03 40 e0'), 0)
        self.assertEqual(instruction['size'], 10)
        self.assertEqual(instruction['operands']['skipped_halfword']['value'], 0xBBAA)
        self.assertEqual(instruction['operands']['destination_owner']['value'], 3)
        self.assertEqual(instruction['operands']['destination']['reference'], 0xE040)


class EventControlFlowTests(unittest.TestCase):
    def assert_preserves_section(self, scene, document):
        header = SceneHeader.parse(scene)
        section = scene[header.event_scripts:header.strings]
        rebuilt = bytearray(section[:document.get('directory_size', 0)])
        spans = sorted(document['instructions'] + document['undecoded'], key=lambda item: item['offset'])
        for span in spans:
            self.assertEqual(span['offset'], header.event_scripts + len(rebuilt))
            raw = bytes.fromhex(span.get('bytes', span.get('raw_bytes', '')))
            self.assertEqual(len(raw), span['size'])
            rebuilt.extend(raw)
        self.assertEqual(rebuilt, section)

    def test_calls_decode_beyond_return_and_backward_loops_terminate(self):
        scene = event_scene(bytes.fromhex('02 07 00 00 7f 7f 7f 06 01 ff ff'))
        header = SceneHeader.parse(scene)
        result = read_scene(scene)
        self.assertEqual([item['offset'] - header.event_scripts for item in result['instructions']], [2, 5, 9, 10])
        self.assertEqual(result['instructions'][0]['successors'][0]['target_offset'], header.event_scripts + 9)
        self.assert_preserves_section(scene, result)

    def test_zero_delta_returns_and_conditional_branches_keep_fallthrough(self):
        jump = decode_instruction(bytes.fromhex('01 00 00'), 0, 0x100)
        self.assertEqual(jump['successors'], [{'kind': 'return', 'condition': 'jump'}])
        conditional = decode_instruction(bytes.fromhex('04 00 00'), 0, 0x100)
        self.assertEqual(conditional['successors'][0]['kind'], 'return')
        self.assertEqual(conditional['successors'][1]['target_offset'], 0x103)

    def test_switch_branches_use_each_case_address_as_the_base(self):
        raw = bytes.fromhex('09 01 07 01 08 00 ff 04 00')
        instruction = decode_instruction(raw, 0, 0x100)
        self.assertEqual([case['target_offset'] for case in instruction['cases']], [0x10B, 0x10A])
        self.assertTrue(instruction['cases'][1]['default'])
        self.assertFalse(any(edge['kind'] == 'fallthrough' for edge in instruction['successors']))
        with self.assertRaisesRegex(DecodeError, 'truncated_operands'):
            decode_instruction(raw[:-1], 0)

    def test_indexed_jump_table_remains_raw_without_a_count(self):
        scene = event_scene(bytes.fromhex('0e 20 e0 04 00 06 00'))
        result = read_scene(scene)
        self.assertEqual(len(result['instructions']), 1)
        self.assertIn('unresolved', result['instructions'][0]['successors'][0])
        self.assertTrue(result['undecoded'])
        self.assert_preserves_section(scene, result)

    def test_aliases_raw_slots_empty_entries_and_overlapping_targets(self):
        scene = event_scene(b'\x00' * 8, entries=(8, 8, 2, 16))
        result = read_scene(scene)
        self.assertEqual(result['entries'][0]['target_offset'], result['entries'][1]['target_offset'])
        self.assertIn('unresolved', result['entries'][2])
        self.assertTrue(result['entries'][3]['empty'])
        self.assert_preserves_section(scene, result)
        overlap = read_scene(event_scene(bytes.fromhex('40 88 00'), entries=(4, 5)))
        self.assertEqual(overlap['diagnostics'][0]['reason'], 'target_inside_instruction')
        self.assertIn('unresolved', overlap['entries'][1])
        self.assertIn('TARGET_INSIDE_INSTRUCTION', format_disassembly(overlap))

    def test_unknown_truncated_and_negative_targets_preserve_raw_data(self):
        for raw, reason in ((b'\x90', 'unknown_opcode'), (b'\x01\x00', 'truncated_operands'),
                            (b'\x40\x8b', 'unknown_operand_type_0xB'), (b'\x17', 'rejected_opcode')):
            with self.subTest(reason=reason):
                with self.assertRaisesRegex(DecodeError, reason):
                    decode_instruction(raw, 0)
        scene = event_scene(bytes.fromhex('01 00 80'))
        document = read_scene(scene)
        self.assertEqual(document['diagnostics'][0]['reason'], 'target_outside_code')
        loaded = yaml.safe_load(dump_yaml(document, 'event_scripts'))
        self.assertLess(loaded['instructions'][0]['successors'][0]['target_offset'], 0)
        self.assert_preserves_section(scene, document)

    def test_empty_sections_and_invalid_directory_headers(self):
        scene = replace_section(make_scene(), 'event_scripts', b'')
        self.assertEqual(read_scene(scene)['instructions'], [])
        for raw in (bytes(4), b'\x03\x00\x00\x00', b'\x06\x00\x00\x00'):
            with self.subTest(raw=raw):
                with self.assertRaisesRegex(ValueError, 'directory size'):
                    read_scene(replace_section(make_scene(), 'event_scripts', raw))


class EventExportTests(unittest.TestCase):
    def test_standalone_and_integrated_outputs_agree_and_refuse_overwrites(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / 'scene.IMG'
            source.write_bytes(event_scene(COMMON_CHEST_INITIALIZER + b'\x00'))
            export = root / 'export'
            extract(source, export)
            self.assertEqual(reconstruct(export), source.read_bytes())
            self.assertEqual((export / 'event_scripts/data.bin').read_bytes(),
                             source.read_bytes()[SceneHeader.parse(source.read_bytes()).event_scripts:
                                                 SceneHeader.parse(source.read_bytes()).strings])
            for format, suffix in (('yaml', '.yaml'), ('text', '.txt')):
                stdout = io.StringIO()
                with redirect_stdout(stdout):
                    self.assertEqual(main([str(source), '--format', format]), 0)
                self.assertEqual(stdout.getvalue(), (export / 'event_scripts' / ('data' + suffix)).read_text())
                target = root / ('standalone' + suffix)
                arguments = [str(source), '--format', format, '-o', str(target)]
                self.assertEqual(main(arguments), 0)
                with redirect_stderr(io.StringIO()):
                    self.assertEqual(main(arguments), 1)
                self.assertEqual(target.read_text(), stdout.getvalue())

    def test_bad_input_does_not_publish_output(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / 'bad.IMG'
            source.write_bytes(b'bad')
            output = root / 'events.yaml'
            with redirect_stderr(io.StringIO()):
                self.assertEqual(main([str(source), '-o', str(output)]), 1)
            self.assertFalse(output.exists())


if __name__ == '__main__':
    unittest.main()
