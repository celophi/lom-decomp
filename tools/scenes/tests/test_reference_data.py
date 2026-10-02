"""Shared names retain their bytes and offsets; drop odds follow the runtime roll."""

from collections import Counter
from contextlib import redirect_stdout, redirect_stderr
import io
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch

import yaml

from tools.compressor.compressor import compress
from tools.scenes.drop_rates import SCENARIOS, choice_rates, profiles, slot_weights
from tools.scenes.field_scene import extract, reconstruct
from tools.scenes.reference_data import ReferenceData, decode_name, load_reference, resolve_item
from tools.scenes.scene_format import SceneHeader
from tools.scenes.scene_report import format_report, main, read_scene_report
from tools.scenes.tests.test_scene_report import report_scene


def reference_bytes():
    # Duplicate names deliberately share their pointer. Entry 255 still exists.
    table = struct.pack('<256H', *([512] * 256)) + b'Plat\x16um\0'
    return b'\0' + table + bytes((3, 4, 5, 6, 7, 7, 7, 7))


def reference():
    data = reference_bytes()
    return ReferenceData.parse(data, source='fixture/FIELD.BIN', version='us',
                               names_start=1, names_end=len(data) - 8, drops_start=len(data) - 8)


class ReferenceTests(unittest.TestCase):
    def test_dictionary_names_keep_shared_offsets_and_raw_bytes(self):
        names = reference()
        self.assertEqual(names.item(0x96)['name'], 'Platinum')
        self.assertEqual(names.item(0x96)['name_bytes'], b'Plat\x16um'.hex(' '))
        self.assertEqual(names.item(0)['name_offset'], 513)
        self.assertEqual(names.item(0x96)['name_offset'], 513)
        self.assertEqual(names.item(0x96)['pointer_offset'], 1 + 0x96 * 2)
        self.assertEqual(names.item(0x96)['file'], 'fixture/FIELD.BIN')

    def test_unknown_codes_and_japanese_are_not_guessed_as_english(self):
        self.assertEqual(decode_name(b'A\x19\0B', 'us'), 'A{19 00}B')
        self.assertEqual(decode_name(b'\x16', 'jp'), '{16}')
        self.assertEqual(decode_name(b'A\x1f\0B', 'us'), 'A forB')
        data = reference_bytes().replace(b'\x16', b'\x01')
        parsed = ReferenceData.parse(data, source='fixture', version='us', names_start=1,
                                     names_end=len(data) - 8, drops_start=len(data) - 8)
        self.assertEqual(parsed.item(0)['status'], 'encoded')
        self.assertEqual(parsed.item(0)['name'], 'Plat{01}um')

    def test_missing_reference_and_invalid_ids_are_explicit(self):
        self.assertEqual(resolve_item(0x96, None)['status'], 'unresolved')
        self.assertEqual(reference().item(254)['status'], 'resolved')
        for item_id in (-1, 255, 256, 65535):
            self.assertEqual(reference().item(item_id)['status'], 'invalid')

    def test_damaged_tables_fail_before_names_are_used(self):
        original = reference_bytes()
        for changed in (original[:10], original[:1] + b'\x00\x01' + original[3:],
                        original[:3] + b'\xff\xff' + original[5:]):
            with self.subTest(size=len(changed)), self.assertRaises(ValueError):
                ReferenceData.parse(changed, source='fixture', version='us', names_start=1,
                                    names_end=len(changed) - 8, drops_start=len(changed) - 8)

    def test_original_compressed_file_uses_configured_region_and_offsets(self):
        data = reference_bytes()
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            config = root / 'config/us'
            (config / 'overlays').mkdir(parents=True)
            (config / 'symbols').mkdir()
            (config / 'overlays/FIELD.BIN.yaml').write_text(yaml.safe_dump({
                'segments': [{'start': 1, 'vram': 0x80000000}, [len(data)]]}))
            (config / 'symbols/field_symbol_addrs.txt').write_text(
                f'g_field_item_name_table = 0x80000000;\n'
                f'g_field_drop_handlers = 0x{0x80000000 + len(data) - 9:X};\n'
                f'g_field_drop_slots_by_level = 0x{0x80000000 + len(data) - 9:X};\n')
            source = root / 'disc/us/BIN/FIELD.BIN'
            source.parent.mkdir(parents=True)
            source.write_bytes(data[:1] + compress(data[1:]))
            with patch('tools.scenes.reference_data.REPO_ROOT', root):
                names = load_reference('us')
                self.assertEqual(names.item(0x96)['name'], 'Platinum')
                self.assertEqual(names.item(0x96)['name_offset'], 513)
                self.assertEqual(names.item(0x96)['offset_space'], 'decompressed_field')
                self.assertIn('decompressed_sha256', names.source)
                source.write_bytes(b'\0\xf1')
                with self.assertRaisesRegex(ValueError, 'invalid compressed'):
                    load_reference('us')

    def test_named_report_preserves_reconstruction_and_matches_cli(self):
        names = reference()
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / 'test.IMG'
            source.write_bytes(report_scene())
            output = root / 'export'
            extract(source, output, names)
            self.assertEqual(reconstruct(output), source.read_bytes())
            parsed = yaml.safe_load((output / 'objects.yaml').read_text())
            chest = parsed['objects'][1]['chest']
            self.assertEqual(chest['item']['name'], 'Platinum')
            self.assertEqual(int.from_bytes(source.read_bytes()[chest['item_id_offset']:][:2], 'little'), 0x96)
            drop = parsed['monster_templates'][0]['drops'][2]
            self.assertEqual(source.read_bytes()[drop['offset']:][:2], bytes((2, 0x96)))
            self.assertEqual(drop['item']['name'], 'Platinum')
            self.assertIn('Platinum (0x0096)', (output / 'objects.txt').read_text())
            with patch('tools.scenes.reference_data.load_reference', return_value=names):
                for kind, suffix in (('yaml', 'yaml'), ('text', 'txt')):
                    stream = io.StringIO()
                    with redirect_stdout(stream):
                        self.assertEqual(main([str(source), '--version', 'us', '--format', kind]), 0)
                    self.assertEqual(stream.getvalue(), (output / f'objects.{suffix}').read_text())
            with redirect_stderr(io.StringIO()):
                self.assertEqual(main([str(source), '--field-bin', 'missing']), 1)

    def test_reports_combine_repeated_choices_but_keep_slot_evidence(self):
        scene = bytearray(report_scene())
        header = SceneHeader.parse(scene)
        initial = read_scene_report(scene, header, reference())
        start = initial['monster_templates'][0]['drops'][2]['offset']
        scene[start + 2:start + 4] = bytes((2, 0x96))
        result = read_scene_report(scene, header, reference())
        choices = result['monster_templates'][0]['drop_choices']
        choice = next(item for item in choices if item['slots'] == [2, 3])
        self.assertEqual(choice['rates']['normal'][0]['percent'], '25%')
        self.assertEqual(choice['rates']['normal'][0]['level_max'], 15)
        self.assertEqual(choice['rates']['no_common'][0]['percent'], '100%')
        self.assertIn('Slots 2, 3: Inventory item Platinum', format_report(result))


class DropRateTests(unittest.TestCase):
    def test_fallback_receives_all_unclaimed_masks(self):
        self.assertEqual(slot_weights(3), [64, 32, 16, 16, 0, 0, 0, 0])
        self.assertEqual(slot_weights(7), [64, 32, 16, 8, 4, 2, 1, 1])
        self.assertEqual(slot_weights(3, 0x08000000), [0, 0, 64, 64, 0, 0, 0, 0])

    def test_weights_match_exhaustive_runtime_roll_including_modified_tables(self):
        for boundary in (0, 1, 3, 4, 5, 6, 7, 8, 255):
            for _, flags in SCENARIOS:
                counts = Counter()
                limit = min(boundary + (2 if flags & 0x04000000 else 0), 7)
                for random in range(32768):
                    mask = random & (0xFFFC if flags & 0x08000000 else 0xFFFF)
                    slot = 0
                    while slot < limit and not mask & 1:
                        slot += 1
                        mask >>= 1
                    counts[slot] += 1
                expected = [counts[slot] // 256 for slot in range(8)]
                with self.subTest(boundary=boundary, flags=flags):
                    self.assertEqual(slot_weights(boundary, flags), expected)
                    self.assertEqual(sum(expected), 128)

    def test_level_bands_and_extra_slot_cap(self):
        cases = profiles((3, 4, 5, 6, 7, 7, 7, 7))
        normal, extra, no_common, both = cases
        self.assertEqual(normal['bands'][0]['level_min'], 1)
        self.assertEqual(normal['bands'][-1]['level_max'], 99)
        self.assertEqual(extra['bands'][0]['last_slot'], 5)
        self.assertEqual(extra['bands'][2]['last_slot'], 7)
        self.assertEqual(both['bands'][0]['weights_out_of_128'], [0, 0, 64, 32, 16, 16, 0, 0])
        self.assertEqual(choice_rates([3, 4, 5, 6, 7], normal), [
            {'level_min': 1, 'level_max': 99, 'weight_out_of_128': 16, 'fraction': '1/8', 'percent': '12.5%'}])


if __name__ == '__main__':
    unittest.main()
