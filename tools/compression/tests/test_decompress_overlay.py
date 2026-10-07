"""Check prepared splat inputs and the Make rules that produce them."""

import hashlib
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

import yaml

from tools.compression.decompress_overlay import decompress_overlay


REPOSITORY_ROOT = Path(__file__).resolve().parents[3]


class OverlayDecompressionTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.config_directory = self.root / 'config'
        self.disc_directory = self.root / 'disc'
        self.build_directory = self.root / 'build'
        (self.config_directory / 'overlays').mkdir(parents=True)
        self.disc_directory.mkdir()
        self.source = self.disc_directory / 'EXAMPLE.BIN'
        self.destination = self.build_directory / 'decompressed/EXAMPLE.BIN'
        self.config_path = self.config_directory / 'overlays/EXAMPLE.BIN.yaml'

        payload = b'overlay data'
        # One literal block followed by the stream terminator; byte 0 is the tag.
        self.compressed = b'\x01' + bytes([len(payload) - 1]) + payload + b'\xff'
        self.expected_output = b'\x01' + payload
        self.source.write_bytes(self.compressed)
        self.config = {
            'compressed_sha1': hashlib.sha1(self.compressed).hexdigest(),
            'sha1': hashlib.sha1(self.expected_output).hexdigest(),
        }
        self.write_config()

    def write_config(self):
        self.config_path.write_text(yaml.safe_dump(self.config))

    def prepare(self):
        return decompress_overlay(self.source, self.destination, self.config_path)

    def test_output_preserves_the_format_byte_and_existing_offsets(self):
        self.prepare()

        self.assertEqual(self.destination.read_bytes(), self.expected_output)
        self.assertEqual(self.destination.read_bytes()[1:], b'overlay data')

    def test_changed_disc_bytes_are_rejected_without_replacing_existing_output(self):
        self.prepare()
        self.source.write_bytes(self.compressed + b'changed')

        with self.assertRaisesRegex(ValueError, 'compressed SHA-1 mismatch'):
            self.prepare()

        self.assertEqual(self.destination.read_bytes(), self.expected_output)

    def test_wrong_decompressed_hash_does_not_create_an_output(self):
        self.config['sha1'] = '0' * 40
        self.write_config()

        with self.assertRaisesRegex(ValueError, 'decompressed SHA-1 mismatch'):
            self.prepare()

        self.assertFalse(self.destination.exists())

    def test_unrecognized_format_is_rejected_even_when_its_hash_matches(self):
        invalid = b'\x02' + self.compressed[1:]
        self.source.write_bytes(invalid)
        self.config['compressed_sha1'] = hashlib.sha1(invalid).hexdigest()
        self.write_config()

        with self.assertRaisesRegex(ValueError, 'compression format 0x01'):
            self.prepare()

        self.assertFalse(self.destination.exists())

    def run_make(self):
        environment = os.environ.copy()
        for variable in ('MAKEFLAGS', 'MFLAGS', 'GNUMAKEFLAGS', 'MAKELEVEL'):
            environment.pop(variable, None)
        return subprocess.run(
            ['make', '--no-print-directory', '-f', 'mk/splat.mk', 'decompress',
             f'CONFIG_DIR={self.config_directory}', f'ROM_BIN_DIR={self.disc_directory}',
             f'BUILD_DIR={self.build_directory}', 'GAME=unused'],
            cwd=REPOSITORY_ROOT, env=environment, capture_output=True, text=True,
            timeout=30,
        )

    def test_make_reuses_an_unchanged_prepared_file(self):
        first = self.run_make()
        self.assertEqual(first.returncode, 0, first.stdout + first.stderr)
        original_timestamp = self.destination.stat().st_mtime_ns

        second = self.run_make()

        self.assertEqual(second.returncode, 0, second.stdout + second.stderr)
        self.assertNotIn('Decompressed ', second.stdout)
        self.assertEqual(self.destination.stat().st_mtime_ns, original_timestamp)
        self.assertEqual(self.destination.read_bytes(), self.expected_output)

    def test_make_rechecks_the_hash_when_the_disc_file_changes(self):
        first = self.run_make()
        self.assertEqual(first.returncode, 0, first.stdout + first.stderr)
        self.source.write_bytes(self.compressed + b'changed')
        timestamp = self.destination.stat().st_mtime_ns + 1
        os.utime(self.source, ns=(timestamp, timestamp))

        changed = self.run_make()

        self.assertNotEqual(changed.returncode, 0)
        self.assertIn('compressed SHA-1 mismatch', changed.stderr)
        self.assertEqual(self.destination.read_bytes(), self.expected_output)


if __name__ == '__main__':
    unittest.main()
