"""Actor description and script decoding, including shared entries and CLI output."""

import io
import struct
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path

import yaml

from tools.scenes.actor_scripts import decode_script, main as scripts_main, read_actor_scripts
from tools.scenes.actors import main as actors_main, read_actors
from tools.scenes.field_scene import extract
from tools.scenes.scene_format import SceneHeader
from tools.scenes.tests.test_identify_img import make_scene


def replace_section(scene, name, data):
    """Replace a synthetic section, moving later sections and their header offsets."""
    header = SceneHeader.parse(scene)
    # Actor fixtures without geometry carry empty ranges for their actor count.
    empty_geometry = scene[header.geometry:header.images] == struct.pack("<I", 4)
    chunks = []
    offsets = []
    cursor = 40
    for section in header.sections(len(scene)):
        chunk = data if section.name == name else scene[section.start:section.end]
        if name == "actors" and section.name == "geometry" and empty_geometry:
            count = struct.unpack_from("<H", data, 6)[0]
            chunk = struct.pack(f"<{count + 1}I", *([4 * (count + 1)] * (count + 1)))
        offsets.append(cursor)
        chunks.append(chunk)
        cursor += len(chunk)
    return struct.pack("<10I", *offsets) + b"".join(chunks)


def actor_scene():
    """Build two actor directory entries sharing one description and texture."""
    actor = struct.pack("<BB6BHHI", 9, 0x80, 0, 0xFF, 4, 5, 6, 7, 2, 0x12, 1)
    actor += struct.pack("<4H", 0x8003, 0xB655, 0xFFFF, 42)
    actors = struct.pack("<4H2I", 127, 3, 0x8005, 2, 16, 16) + actor
    return replace_section(make_scene(), "actors", actors)


class ActorDescriptionTests(unittest.TestCase):
    def test_header_actions_shared_descriptions_and_image_references(self):
        scene = actor_scene()
        header = SceneHeader.parse(scene)
        result = read_actors(scene, header)
        self.assertEqual(result["map_id"], 5)
        self.assertEqual(result["parts_timer_mode"], 1)
        self.assertEqual(result["song_volume"], 127)
        self.assertEqual(result["party_palette_index"], 3)
        first, second = result["actors"]
        self.assertEqual(first["offset"], second["offset"])
        self.assertEqual(first["resource_index"], 3)
        self.assertEqual(second["resource_index"], 4)
        self.assertEqual(first["images"], [{"image_index": 0, "offset": f"0x{header.images + 4:X}",
                                           "file": "textures/000.tim"}])
        self.assertEqual(first["image_slots"], [0, 255, 4, 5, 6, 7])
        action = first["actions"][0]
        self.assertTrue(action["is_technique"])
        self.assertTrue(action["instrument"])
        self.assertEqual(action["target_filter"], 0x55)
        self.assertEqual(action["target_group"], 2)
        self.assertEqual(action["unknown_flag_bits"], 22)
        self.assertEqual(action["parameter"], 42)

    def test_empty_image_list_inherits_previous_resource(self):
        scene = bytearray(actor_scene())
        header = SceneHeader.parse(scene)
        scene[header.actors + 16 + 2] = 0xFF
        result = read_actors(scene, header)
        self.assertTrue(result["actors"][0]["inherits_previous_images"])
        self.assertEqual(result["actors"][0]["images"], [])

    def test_unresolved_image_index_is_retained(self):
        scene = bytearray(actor_scene())
        header = SceneHeader.parse(scene)
        scene[header.actors + 16 + 2] = 200
        reference = read_actors(scene, header)["actors"][0]["images"][0]
        self.assertEqual(reference["image_index"], 200)
        self.assertIn("unresolved", reference)

    def test_rejects_invalid_actor_offsets_and_action_counts(self):
        original = actor_scene()
        header = SceneHeader.parse(original)
        for position, value in ((8, 4), (8, 0xFFFFFFFC), (16 + 12, 999)):
            with self.subTest(position=position):
                scene = bytearray(original)
                struct.pack_into("<I", scene, header.actors + position, value)
                with self.assertRaises(ValueError):
                    read_actors(scene, header)


class ActorScriptTests(unittest.TestCase):
    def test_operands_offsets_and_unused_byte_are_decoded(self):
        raw = bytes.fromhex("97 34 12 01 02 03 78 56 04 a9 34 12 07 9f 99 bc ff 00")
        result = decode_script(raw, 0x100)
        texture, action, bound, start, end = result["instructions"]
        self.assertEqual(texture["operands"], {"x": 0x1234, "y": 1, "width": 2, "height": 3,
                                              "destination_x": 0x5678, "destination_y": 4})
        self.assertEqual(action["offset"], "0x109")
        self.assertEqual(action["operands"], {"resource_id": 0x1234, "action_index": 7})
        self.assertEqual(bound["operands"], {"unused": 0x99})
        self.assertEqual(start["bytes"], "bc")
        self.assertEqual(result["stop_reason"], "end")
        self.assertEqual(result["remaining_bytes"], "00")
        decoded = b"".join(bytes.fromhex(item["bytes"]) for item in result["instructions"])
        self.assertEqual(decoded + bytes.fromhex(result["remaining_bytes"]), raw)

    def test_unknown_and_truncated_opcodes_stop_without_resynchronizing(self):
        for raw, reason in ((b"\x00\x7f\xff", "unknown_opcode"), (b"\x00\xa9\x12\xff", "truncated_operands")):
            with self.subTest(reason=reason):
                result = decode_script(raw, 0)
                self.assertEqual(len(result["instructions"]), 1)
                self.assertEqual(result["stop_reason"], reason)
                self.assertEqual(bytes.fromhex(result["remaining_bytes"]), raw[1:])

    def test_shared_empty_unresolved_entries_and_continuation(self):
        # Entry 3 points into the directory, as two shipped scene entries do.
        scripts = struct.pack("<6H", 12, 13, 12, 2, 16, 99) + bytes([0xA3, 0xFF, 0, 0])
        scene = replace_section(make_scene(), "actor_scripts", scripts)
        result = read_actor_scripts(scene, SceneHeader.parse(scene))
        entries = result["entries"]
        self.assertEqual(entries[0]["script"], entries[2]["script"])
        self.assertIn("unresolved", entries[3])
        self.assertTrue(entries[4]["empty"])
        self.assertIn("unresolved", entries[5])
        self.assertEqual(result["scripts"][0]["continues_at_script"], 1)
        self.assertEqual(result["scripts"][1]["stop_reason"], "end")

    def test_invalid_directory_size_is_rejected(self):
        for raw in (bytes(4), struct.pack("<HH", 3, 0), struct.pack("<HH", 6, 0)):
            with self.subTest(raw=raw):
                scene = replace_section(make_scene(), "actor_scripts", raw)
                with self.assertRaises(ValueError):
                    read_actor_scripts(scene, SceneHeader.parse(scene))


class ActorSectionCommandTests(unittest.TestCase):
    def test_standalone_and_integrated_documents_agree(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "scene.IMG"
            source.write_bytes(actor_scene())
            export = root / "export"
            extract(source, export)
            for command, section in ((actors_main, "actors"), (scripts_main, "actor_scripts")):
                with self.subTest(section=section):
                    output = io.StringIO()
                    with redirect_stdout(output):
                        self.assertEqual(command([str(source)]), 0)
                    expected = yaml.safe_load((export / section / "data.yaml").read_text())
                    self.assertEqual(yaml.safe_load(output.getvalue()), expected)
                    target = root / f"{section}.yaml"
                    self.assertEqual(command([str(source), "-o", str(target)]), 0)
                    saved = target.read_bytes()
                    with redirect_stderr(io.StringIO()):
                        self.assertEqual(command([str(source), "-o", str(target)]), 1)
                    self.assertEqual(target.read_bytes(), saved)

    def test_bad_input_does_not_create_output(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "broken.IMG"
            source.write_bytes(b"bad")
            target = root / "output.yaml"
            with redirect_stderr(io.StringIO()):
                self.assertEqual(actors_main([str(source), "-o", str(target)]), 1)
            self.assertFalse(target.exists())


if __name__ == "__main__":
    unittest.main()
