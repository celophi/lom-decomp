"""Scene detection with synthetic files, including damaged headers and sections."""

import io
import struct
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest.mock import patch

from tools.scenes.field_scene import SceneHeader
from tools.scenes.identify_img import main, scene_format_error


def make_scene(with_texture=True):
    """Build a scene container with one layout record and an optional 16-bit TIM."""
    texture = struct.pack("<IIIIHHHH", 4, 0x10, 2, 16, 0, 0, 1, 2) + bytes(4)
    sections = [
        struct.pack("<I", 1) + bytes(48),  # Layout count and one actor.
        struct.pack("<HBB", 2, 0, 0),  # One event script: RETURN, then padding.
        bytes.fromhex("02 00 00 00"),  # One empty string and padding.
        struct.pack("<HBB", 2, 0xFF, 0),  # One actor script: END, then padding.
        struct.pack("<I", 4),  # Resource directory with one empty entry.
        bytes(8),  # Actor-section header with no actors.
        struct.pack("<I", 4),  # Geometry end boundary; no actors.
        texture if with_texture else b"",
        bytes(4),  # No portraits.
        struct.pack("<II", 1, 0),  # One group-bounds word.
    ]
    offsets = []
    offset = 40
    for section in sections:
        offsets.append(offset)
        offset += len(section)
    return struct.pack("<10I", *offsets) + b"".join(sections)


class SceneDetectionTests(unittest.TestCase):
    def test_accepts_scene_with_or_without_textures(self):
        for with_texture in (True, False):
            with self.subTest(with_texture=with_texture):
                self.assertIsNone(scene_format_error(make_scene(with_texture)))

    def test_rejects_truncated_headers(self):
        for size in (0, 1, 39):
            with self.subTest(size=size):
                self.assertEqual(scene_format_error(bytes(size)), "truncated scene header")

    def test_rejects_invalid_section_offsets(self):
        original = make_scene()
        for offset, value, reason in (
            (0, 44, "layout offset"),
            (4, 89, "aligned"),
            (4, len(original) + 4, "within the file"),
            (8, 40, "file order"),
        ):
            with self.subTest(offset=offset, value=value):
                data = bytearray(original)
                struct.pack_into("<I", data, offset, value)
                self.assertIn(reason, scene_format_error(data))

    def test_rejects_incorrect_section_counts(self):
        original = make_scene()
        header = SceneHeader.parse(original)
        for offset, reason in (
            (header.layout, "layout count"),
            (header.portraits, "portrait count"),
            (header.group_bounds, "group-bounds count"),
        ):
            with self.subTest(offset=offset):
                data = bytearray(original)
                struct.pack_into("<I", data, offset, 0xFFFFFFFF)
                self.assertIn(reason, scene_format_error(data))

    def test_rejects_invalid_texture_offsets_and_payloads(self):
        original = make_scene()
        images = SceneHeader.parse(original).images
        for offset, value, reason in (
            (images, 0, "image-offset table"),
            (images, 0x1000, "image-offset table"),
            (images + 4, 0, "TIM magic"),
            (images + 12, 0x1000, "past file size"),
        ):
            with self.subTest(offset=offset, value=value):
                data = bytearray(original)
                struct.pack_into("<I", data, offset, value)
                self.assertIn(reason, scene_format_error(data))

    def test_rejects_truncated_or_extra_group_bounds(self):
        original = make_scene()
        bounds = SceneHeader.parse(original).group_bounds
        self.assertEqual(scene_format_error(original[:bounds]), "missing group-bounds count")
        self.assertIn("group-bounds count", scene_format_error(original[:-1]))
        self.assertIn("group-bounds count", scene_format_error(original + bytes(4)))


class IdentifyImgCommandTests(unittest.TestCase):
    def run_command(self, *args):
        """Capture the command's normal output, errors, and exit status."""
        output = io.StringIO()
        errors = io.StringIO()
        with redirect_stdout(output), redirect_stderr(errors):
            status = main([str(arg) for arg in args])
        return status, output.getvalue(), errors.getvalue()

    def test_explicit_file_uses_contents_not_name(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "scene.dat"
            data = make_scene()
            path.write_bytes(data)
            status, output, errors = self.run_command(path)
            self.assertEqual(status, 0)
            self.assertEqual(output, f"{path}: scene IMG\n")
            self.assertEqual(errors, "")
            self.assertEqual(path.read_bytes(), data)

    def test_directory_scan_and_recursion(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "scene.img").write_bytes(make_scene())
            (root / "ignored.dat").write_bytes(b"not an IMG")
            nested = root / "nested"
            nested.mkdir()
            (nested / "sprite.IMG").write_bytes(b"not a scene")
            status, output, errors = self.run_command(root)
            self.assertEqual(status, 0)
            self.assertIn("Checked 1 files: 1 scene IMG, 0 not scene IMG, 0 errors", output)
            self.assertEqual(errors, "")

            status, output, errors = self.run_command("--recursive", root)
            self.assertEqual(status, 0)
            self.assertIn("sprite.IMG: not scene IMG (truncated scene header)", output)
            self.assertIn("Checked 2 files: 1 scene IMG, 1 not scene IMG, 0 errors", output)
            self.assertEqual(errors, "")

    def test_missing_input_and_empty_directory_are_errors(self):
        with tempfile.TemporaryDirectory() as directory:
            for path in (Path(directory), Path(directory) / "missing.IMG"):
                with self.subTest(path=path):
                    status, output, errors = self.run_command(path)
                    self.assertEqual(status, 1)
                    self.assertEqual(output, "")
                    self.assertIn("error:", errors)

    def test_read_error_is_not_a_format_classification(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "scene.IMG"
            path.write_bytes(make_scene())
            with patch.object(Path, "read_bytes", side_effect=PermissionError("access denied")):
                status, output, errors = self.run_command(path)
            self.assertEqual(status, 1)
            self.assertEqual(output, "")
            self.assertIn("access denied", errors)


if __name__ == "__main__":
    unittest.main()
