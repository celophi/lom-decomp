"""Object associations must come from stored selectors and actual script references."""

import io
from pathlib import Path
import struct
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout

import yaml

from tools.scenes.field_scene import extract, reconstruct
from tools.scenes.layout import COMMON_CHEST_INITIALIZER
from tools.scenes.scene_report import CHEST_FLAG_SETUP, main, read_scene_report, format_report
from tools.scenes.scene_format import SceneHeader
from tools.scenes.script_links import EventLinks
from tools.scenes.event_scripts import read_event_scripts
from tools.scenes.tests.test_actor_sections import replace_section
from tools.scenes.tests.test_field_scene import sample_scene
from tools.scenes.tests.test_scene_resources import battle_resource


def report_scene():
    scene = bytearray(sample_scene())
    header = SceneHeader.parse(scene)
    actor = header.layout + 4
    chest = actor + 48
    # Actor resource 0, template ID 7: these are deliberately different indices.
    scene[actor] = 1
    scene[actor + 1] = 7
    struct.pack_into('<16H', scene, actor + 16, *([0xFFFF] * 16))
    struct.pack_into('<H', scene, chest + 14, 0x8102)
    struct.pack_into('<H', scene, chest + 16, 0x8001)
    struct.pack_into('<H', scene, chest + 16 + 8 * 2, 0x8001)
    initializer = COMMON_CHEST_INITIALIZER + CHEST_FLAG_SETUP
    give_item = bytes.fromhex('44 30 13 40 e0 44 30 2b 30 e0 00')
    section = struct.pack('<HH', 4, 4 + len(initializer)) + initializer + give_item
    section += bytes(-len(section) % 4)
    return replace_section(bytes(scene), 'event_scripts', section)


def report(scene):
    return read_scene_report(scene, SceneHeader.parse(scene))


class ObjectReportTests(unittest.TestCase):
    def test_chest_settings_roles_and_evidenced_actions(self):
        chest = report(report_scene())['objects'][1]
        self.assertEqual(chest['kind'], 'chest')
        self.assertEqual(chest['chest']['item_id'], 0x96)
        self.assertEqual(chest['chest']['collection_variable_ref'], 0xBC0)
        self.assertTrue(chest['chest']['flag_binding'])
        self.assertEqual(chest['interaction'], {'touch': False, 'action_button': True})
        scripts = {item['slot']: item for item in chest['scripts']}
        self.assertEqual(scripts[15]['entry'], 0)
        self.assertEqual(scripts[0]['entry'], 1)
        self.assertTrue(scripts[0]['enabled'])
        self.assertEqual(scripts[8]['entry'], 1)
        self.assertNotIn(4, scripts)
        self.assertNotIn(5, scripts)
        descriptions = [operation['description'] for operation in chest['possible_operations']]
        self.assertTrue(any('chest item 0x0096' in text for text in descriptions))
        self.assertTrue(any('collection flag 0x0BC0' in text for text in descriptions))
        self.assertEqual(chest['possible_operations'][0]['via_slots'], [0, 8])
        self.assertTrue(all(item['offset'] > 0 for item in chest['possible_operations']))

    def test_template_lookup_uses_id_and_preserves_drop_links(self):
        result = report(report_scene())
        actor = result['objects'][0]
        template = actor['monster_template']
        self.assertEqual(actor['actor_id'], 3)
        self.assertEqual(actor['actor_resource']['resource_index'], 3)
        self.assertEqual(template['template_id'], 7)
        self.assertEqual(template['index'], 0)
        self.assertEqual(template['name'], 'Test monster')
        self.assertEqual(template['file'], 'records/000.yaml')
        drops = result['monster_templates'][0]['drops']
        self.assertIn('0x0096', drops[2]['description'])
        self.assertIn('Test item', drops[3]['description'])
        self.assertEqual(drops[3]['reward_indices'], [0])

    def test_missing_and_multiple_battle_resources_do_not_guess(self):
        scene = bytearray(report_scene())
        header = SceneHeader.parse(scene)
        scene[header.layout + 5] = 99
        link = report(scene)['objects'][0]['monster_template']
        self.assertEqual(link['status'], 'no_match')
        resource = battle_resource()
        section = struct.pack('<II', 8, 8 + len(resource)) + resource + resource
        scene = replace_section(report_scene(), 'records', section)
        link = report(scene)['objects'][0]['monster_template']
        self.assertEqual(link['status'], 'unresolved')
        self.assertNotIn('index', link)

    def test_duplicate_template_ids_use_the_runtime_first_match(self):
        original = battle_resource()
        monster = original[20:112]
        second = bytearray(monster)
        second[:12] = b'Other enemy!'
        table = struct.pack('<III', 2, 12, 12 + len(monster)) + monster + second
        battle = struct.pack('<III', 1, 12, 12 + len(table)) + table + original[112:]
        scene = replace_section(report_scene(), 'records', struct.pack('<I', 4) + battle)
        link = report(scene)['objects'][0]['monster_template']
        self.assertEqual(link['matching_indices'], [0, 1])
        self.assertEqual(link['index'], 0)
        self.assertEqual(link['name'], 'Test monster')

    def test_script_only_layout_does_not_get_a_placed_monster_link(self):
        scene = bytearray(report_scene())
        header = SceneHeader.parse(scene)
        scene[header.layout + 4] = 0x61
        obj = report(scene)['objects'][0]
        self.assertEqual(obj['kind'], 'script_only_actor')
        self.assertNotIn('monster_template', obj)
        self.assertNotIn('actor_resource', obj)

    def test_disabled_initializer_does_not_claim_live_chest_bindings(self):
        scene = bytearray(report_scene())
        header = SceneHeader.parse(scene)
        struct.pack_into('<H', scene, header.layout + 4 + 48 + 14, 0x102)
        chest = report(scene)['objects'][1]
        self.assertFalse(chest['chest']['initializer_enabled'])
        for operation in chest['possible_operations']:
            self.assertNotIn('initialized to chest', operation['description'])
            self.assertNotIn('initializer selects', operation['description'])

    def test_changed_flag_setup_keeps_reference_unresolved(self):
        scene = bytearray(report_scene())
        header = SceneHeader.parse(scene)
        # Change the mask operand, while preserving the recognized initializer prefix.
        position = header.event_scripts + 4 + len(COMMON_CHEST_INITIALIZER) + len(CHEST_FLAG_SETUP) - 5
        scene[position] ^= 1
        chest = report(scene)['objects'][1]
        self.assertEqual(chest['kind'], 'chest')
        self.assertFalse(chest['chest']['flag_binding'])
        self.assertTrue(all('initializer selects' not in op['description'] for op in chest['possible_operations']))

    def test_interaction_without_high_bit_is_a_message_not_event_code(self):
        scene = bytearray(report_scene())
        header = SceneHeader.parse(scene)
        struct.pack_into('<H', scene, header.layout + 4 + 48 + 16, 1)
        chest = report(scene)['objects'][1]
        interaction = next(script for script in chest['scripts'] if script['slot'] == 0)
        self.assertEqual(interaction['kind'], 'message')
        self.assertEqual(interaction['string_index'], 1)
        self.assertNotIn('entry', interaction)
        self.assertTrue(all(0 not in operation['via_slots'] for operation in chest['possible_operations']))

    def test_invalid_entry_is_visible_without_inventing_effects(self):
        scene = bytearray(report_scene())
        header = SceneHeader.parse(scene)
        struct.pack_into('<H', scene, header.layout + 4 + 48 + 16, 0x8123)
        chest = report(scene)['objects'][1]
        interaction = next(script for script in chest['scripts'] if script['slot'] == 0)
        self.assertIn('unresolved', interaction)
        self.assertNotIn('offset', interaction)
        self.assertIn('UNRESOLVED', format_report(report(scene)))

    def test_static_trace_keeps_dynamic_calls_unresolved_and_terminates_loops(self):
        section = struct.pack('<H', 2) + bytes.fromhex('10 20 e0 01 fd ff')
        scene = replace_section(report_scene(), 'event_scripts', section)
        document = read_event_scripts(scene, SceneHeader.parse(scene))
        links = EventLinks(document)
        trace = links.resolve(0)
        self.assertEqual(len(trace['instructions']), 2)
        self.assertTrue(trace['unresolved'])
        self.assertIs(links.resolve(0), trace)

    def test_standalone_matches_integrated_report_and_never_overwrites(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / 'sample.IMG'
            source.write_bytes(report_scene())
            export = root / 'export'
            extract(source, export)
            self.assertEqual(reconstruct(export), source.read_bytes())
            for kind, suffix in (('yaml', 'yaml'), ('text', 'txt')):
                stream = io.StringIO()
                with redirect_stdout(stream):
                    self.assertEqual(main([str(source), '--format', kind]), 0)
                self.assertEqual(stream.getvalue(), (export / f'objects.{suffix}').read_text())
                target = root / f'report.{suffix}'
                arguments = [str(source), '--format', kind, '-o', str(target)]
                self.assertEqual(main(arguments), 0)
                with redirect_stderr(io.StringIO()):
                    self.assertEqual(main(arguments), 1)
                self.assertEqual(target.read_text(), stream.getvalue())
            saved = yaml.safe_load((export / 'objects.yaml').read_text())
            self.assertEqual(saved['objects'][1]['chest']['item_id'], 0x96)


if __name__ == '__main__':
    unittest.main()
