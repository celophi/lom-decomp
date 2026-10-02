"""The overview must describe observed structure and expose decoder limitations."""

from pathlib import Path
import struct
import subprocess
import tempfile
import unittest

import yaml

from tools.scenes.field_scene import extract, reconstruct, split_scene
from tools.scenes.scene_format import SceneHeader
from tools.scenes.structure import describe_scene
from tools.scenes.tests.test_actor_sections import replace_section
from tools.scenes.tests.test_field_scene import sample_scene
from tools.scenes.tests.test_identify_img import make_scene


REPO = Path(__file__).resolve().parents[3]


def overview(data):
    return describe_scene(Path('sample.IMG'), SceneHeader.parse(data), len(data), split_scene(data))


def section(data, name):
    return next(row for row in overview(data)['sections'] if row['name'] == name)


class StructureTests(unittest.TestCase):
    def test_header_fields_match_the_original_bytes_and_loader(self):
        raw = sample_scene()
        document = overview(raw)
        self.assertEqual(document['header']['size'], 40)
        self.assertEqual(document['header']['pointer_base'], 'IMG byte zero')
        self.assertEqual(len(document['header']['fields']), 10)
        for field in document['header']['fields']:
            self.assertEqual(struct.unpack_from('<I', raw, field['offset'])[0], field['section_offset'])
            self.assertEqual(field['type'], 'u32')
            self.assertEqual(field['size'], 4)
        for row in document['sections']:
            reference = row['code_reference']
            source = (REPO / reference['file']).read_text()
            self.assertIn(reference['symbol'], source)
        self.assertEqual(sum(row['size'] for row in document['sections']) + 40, len(raw))

    def test_raw_partial_decoded_and_empty_are_distinguishable(self):
        raw = make_scene(with_texture=False)
        self.assertEqual(section(raw, 'layout')['coverage']['status'], 'decoded')
        self.assertEqual(section(raw, 'strings')['coverage']['status'], 'partial')
        self.assertEqual(section(raw, 'geometry')['coverage']['status'], 'decoded')
        self.assertEqual(section(raw, 'images')['coverage']['status'], 'empty')
        self.assertEqual(section(raw, 'group_bounds')['coverage']['status'], 'partial')
        # The resource's empty end-offset entry must not leak into actor coverage.
        self.assertEqual(section(raw, 'records')['coverage']['unique_resources'], 1)
        self.assertEqual(section(raw, 'actors')['coverage']['directory_entries'], 0)

    def test_actor_script_counts_preserve_aliases_and_unsupported_tails(self):
        # Two entries share one script, and a third points back into the directory.
        raw = replace_section(make_scene(), 'actor_scripts', struct.pack('<HHH', 6, 6, 2) + bytes.fromhex('ff 00'))
        report = section(raw, 'actor_scripts')['coverage']
        self.assertEqual(report['status'], 'partial')
        self.assertEqual(report['undecoded_bytes'], 1)
        self.assertEqual(report['instructions'], 1)
        self.assertEqual(report['scripts'], 1)
        self.assertEqual(report['unresolved_entries'], 1)
        unknown = replace_section(make_scene(), 'actor_scripts', bytes.fromhex('02 00 7f 00'))
        report = section(unknown, 'actor_scripts')['coverage']
        self.assertEqual(report['undecoded_bytes'], 2)
        self.assertEqual(report['unsupported_ranges'], 1)
        self.assertEqual(report['status'], 'partial')

    def test_event_decoder_gaps_are_reported_as_partial(self):
        raw = replace_section(make_scene(), 'event_scripts', bytes.fromhex('02 00 3b 00'))
        report = section(raw, 'event_scripts')['coverage']
        self.assertEqual(report['status'], 'partial')
        self.assertEqual(report['diagnostics'], 1)
        self.assertEqual(report['undecoded_bytes'], 2)

    def test_shared_actor_descriptions_count_raw_tails_once(self):
        header = struct.pack('<4H2I', 0, 0, 0, 2, 16, 16)
        actor = bytes(16) + bytes.fromhex('01 02 03 04')
        raw = replace_section(make_scene(), 'actors', header + actor)
        report = section(raw, 'actors')['coverage']
        self.assertEqual(report['directory_entries'], 2)
        self.assertEqual(report['undecoded_bytes'], 4)
        self.assertEqual(report['status'], 'partial')

    def test_unknown_resource_payloads_remain_raw(self):
        raw = replace_section(make_scene(), 'records', struct.pack('<I', 4) + bytes.fromhex('02 00 11 22 33 44 55 66'))
        report = section(raw, 'records')['coverage']
        self.assertEqual(report['status'], 'partial')
        self.assertEqual(report['raw_resources'], 1)
        self.assertEqual(report['battle_resources'], 0)

    def test_make_extracts_without_a_field_binary_and_keeps_yaml_and_text_consistent(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / 'sample.IMG'
            source.write_bytes(sample_scene())
            output = root / 'exports'
            command = ['make', '-s', 'extract-scene', f'SCENE={source}', f'SCENE_OUTPUT={output}',
                       'VERSION=jp', 'SCENE_REFERENCE_VERSION=', f'SCENE_FIELD_BIN={root / "absent.bin"}']
            result = subprocess.run(command, cwd=REPO, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            exported = output / 'sample'
            self.assertEqual(reconstruct(exported), source.read_bytes())
            self.assertEqual(yaml.safe_load((exported / 'objects.yaml').read_text())['reference_data'],
                             {'status': 'not_selected'})
            document = yaml.safe_load((exported / 'scene.yaml').read_text())
            text = (exported / 'scene.txt').read_text()
            self.assertIn('HEADER LAYOUT', text)
            self.assertIn('READING THE FORMAT', text)
            self.assertNotIn('MODDING', text)
            for row in document['sections']:
                self.assertIn(f"{row['name']} [{row['coverage']['status']}]", text)
                self.assertIn(row['format'], text)
                self.assertIn(row['code_reference']['symbol'], text)
            # Explicit external annotation selection must still validate its source.
            result = subprocess.run(command[:-2] + ['SCENE_REFERENCE_VERSION=jp',
                                    f'SCENE_FIELD_BIN={root / "absent.bin"}', f'SCENE_OUTPUT={root / "failed"}'],
                                    cwd=REPO, capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertFalse((root / 'failed/sample').exists())


if __name__ == '__main__':
    unittest.main()
