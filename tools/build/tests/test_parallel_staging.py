"""Check that parallel builds wait for linker scripts and assets to be staged."""

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


REPOSITORY_ROOT = Path(__file__).resolve().parents[3]
FIXTURES_DIRECTORY = Path(__file__).resolve().parent / 'fixtures'


class ParallelStagingTests(unittest.TestCase):
    def setUp(self):
        temporary_directory = tempfile.TemporaryDirectory()
        self.addCleanup(temporary_directory.cleanup)
        self.project_directory = Path(temporary_directory.name)
        self.staging_directory = self.project_directory / 'staging'

        # staging.mk expects these source directories, even in this tiny project.
        for directory in (
            'src',
            'include',
            'asm/us',
            'linker/us',
            'assets/us',
            'tools/external/maspsx',
            'mk',
            'bin',
        ):
            (self.project_directory / directory).mkdir(parents=True, exist_ok=True)

        shutil.copyfile(
            REPOSITORY_ROOT / 'mk/staging.mk',
            self.project_directory / 'mk/staging.mk',
        )
        shutil.copyfile(
            FIXTURES_DIRECTORY / 'parallel_staging.mk',
            self.project_directory / 'Makefile',
        )
        # The fixture defines its version, but staging.mk also tracks this file.
        (self.project_directory / 'mk/version.mk').touch()

        self.source_linker_script = self.project_directory / 'linker/us/game.ld'
        self.source_linker_script.write_text('SECTIONS {}\n')
        self.source_asset = self.project_directory / 'assets/us/image.bin'
        self.source_asset.write_bytes(b'asset data')

        # Use a no-op dos2unix so these ordering tests need only Make and coreutils.
        dos2unix = self.project_directory / 'bin/dos2unix'
        dos2unix.write_text('#!/bin/sh\nexit 0\n')
        dos2unix.chmod(0o755)
        self.make_environment = os.environ.copy()
        self.make_environment['PATH'] = f"{self.project_directory / 'bin'}:{os.environ['PATH']}"

    def run_parallel_make(self):
        return subprocess.run(
            ['make', '--no-print-directory', '-j8'],
            cwd=self.project_directory,
            env=self.make_environment,
            capture_output=True,
            text=True,
            timeout=30,
        )

    def test_fresh_parallel_build_copies_both_inputs_before_using_them(self):
        self.assertFalse(self.staging_directory.exists())

        result = self.run_parallel_make()

        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(
            (self.staging_directory / 'linker/us/game.ld').read_bytes(),
            self.source_linker_script.read_bytes(),
        )
        self.assertEqual(
            (self.staging_directory / 'assets/us/image.bin').read_bytes(),
            self.source_asset.read_bytes(),
        )
        self.assertEqual(result.stdout.count('Staging complete (us).'), 1)

    def test_missing_source_linker_script_fails_the_build(self):
        self.source_linker_script.unlink()

        result = self.run_parallel_make()

        self.assertNotEqual(result.returncode, 0)
        self.assertIn('game.ld', result.stderr)


if __name__ == '__main__':
    unittest.main()
