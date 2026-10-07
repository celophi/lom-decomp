from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest


class ParallelStagingTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        repo = Path(__file__).resolve().parents[3]
        (self.root / 'mk').mkdir()
        shutil.copyfile(repo / 'mk/staging.mk', self.root / 'mk/staging.mk')
        (self.root / 'mk/version.mk').touch()
        for directory in ('src', 'include', 'asm/us', 'linker/us',
                          'assets/us', 'tools/maspsx', 'bin'):
            (self.root / directory).mkdir(parents=True, exist_ok=True)
        self.script = self.root / 'linker/us/game.ld'
        self.script.write_text('SECTIONS {}\n')
        (self.root / 'assets/us/image.bin').write_bytes(b'asset data')
        # Line-ending conversion is separate from the dependency ordering under test.
        dos2unix = self.root / 'bin/dos2unix'
        dos2unix.write_text('#!/bin/sh\nexit 0\n')
        dos2unix.chmod(0o755)
        self.env = dict(os.environ, PATH=f"{self.root / 'bin'}:{os.environ['PATH']}")
        (self.root / 'Makefile').write_text('''
.DEFAULT_GOAL := all
VERSION := us
STAGING := staging
ASM_DIR := asm/us
LINKER_DIR := linker/us
ASSETS_DIR := assets/us
include mk/staging.mk
.PHONY: all
all: staging/linker/us/game.ld staging/assets/us/image.bin
\tcmp linker/us/game.ld staging/linker/us/game.ld
\tcmp assets/us/image.bin staging/assets/us/image.bin
''')

    def make(self):
        return subprocess.run(['make', '--no-print-directory', '-j8'],
                              cwd=self.root, env=self.env, capture_output=True, text=True)

    def test_fresh_parallel_build_waits_for_staged_inputs(self):
        result = self.make()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(result.stdout.count('Staging complete (us).'), 1)

    def test_missing_host_input_still_fails(self):
        self.script.unlink()
        result = self.make()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('game.ld', result.stderr)


if __name__ == '__main__':
    unittest.main()
