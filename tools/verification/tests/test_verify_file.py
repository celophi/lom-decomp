import contextlib
import io
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from tools.verification.verify_file import verify


class VerifyFileTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.original = self.root / 'original.bin'
        self.rebuilt = self.root / 'rebuilt.bin'
        self.manifest = self.root / 'complete.txt'
        self.original.write_bytes(b'original data')
        self.rebuilt.write_bytes(b'original data')

    def check_file(self, **kwargs):
        with contextlib.redirect_stdout(io.StringIO()):
            return verify(self.original, self.rebuilt, **kwargs)

    def test_match_is_recorded_once(self):
        for _ in range(2):
            self.assertTrue(self.check_file(manifest=self.manifest, name='main'))
        self.assertEqual(self.manifest.read_text(), 'main\n')

    def test_mismatch_removes_only_its_previous_success(self):
        self.manifest.write_text('main\ncheckps\n')
        self.rebuilt.write_bytes(b'changed data')
        self.assertFalse(self.check_file(manifest=self.manifest, name='main'))
        self.assertEqual(self.manifest.read_text(), 'checkps\n')

    def test_missing_file_removes_previous_success(self):
        self.manifest.write_text('main\n')
        self.rebuilt.unlink()
        with self.assertRaises(FileNotFoundError):
            self.check_file(manifest=self.manifest, name='main')
        self.assertEqual(self.manifest.read_text(), '')

    def test_raw_match_does_not_create_manifest(self):
        self.assertTrue(self.check_file(raw=True))
        self.assertFalse(self.manifest.exists())

    def test_parallel_checks_keep_every_result(self):
        script = Path(__file__).resolve().parents[1] / 'verify_file.py'
        names = ['main', 'checkps', 'field', 'wmap', 'menu', 'shop']
        processes = [subprocess.Popen(
            [sys.executable, str(script), str(self.original), str(self.rebuilt),
             '--manifest', str(self.manifest), '--name', name],
            stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
        ) for name in names]
        for process in processes:
            _, error = process.communicate(timeout=30)
            self.assertEqual(process.returncode, 0, error.decode())
        self.assertCountEqual(self.manifest.read_text().splitlines(), names)

    def test_cli_reports_mismatch_as_failure(self):
        self.rebuilt.write_bytes(b'wrong')
        script = Path(__file__).resolve().parents[1] / 'verify_file.py'
        result = subprocess.run(
            [sys.executable, str(script), str(self.original), str(self.rebuilt)],
            capture_output=True, text=True,
        )
        self.assertEqual(result.returncode, 1)
        self.assertIn('[FAIL]', result.stdout)


if __name__ == '__main__':
    unittest.main()
