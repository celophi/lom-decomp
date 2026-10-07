from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest


class ParallelJobsTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        repo = Path(__file__).resolve().parents[3]
        shutil.copyfile(repo / 'mk/parallel.mk', self.root / 'parallel.mk')
        (self.root / 'bin').mkdir()
        for name, body in (('nproc', 'echo 6'), ('getconf', 'echo 3')):
            executable = self.root / 'bin' / name
            executable.write_text(f'#!/bin/sh\n{body}\n')
            executable.chmod(0o755)
        self.env = dict(os.environ, PATH=f"{self.root / 'bin'}:{os.environ['PATH']}")
        for name in ('MAKEFLAGS', 'MFLAGS', 'GNUMAKEFLAGS', 'MAKELEVEL'):
            self.env.pop(name, None)
        (self.root / 'Makefile').write_text('''
include parallel.mk
.PHONY: all child
all:
\t@echo PARENT=$(MAKEFLAGS)
\t+@$(MAKE) --no-print-directory child
child:
\t@echo CHILD=$(MAKEFLAGS)
''')

    def flags(self, *arguments, **environment):
        result = subprocess.run(['make', '--no-print-directory', *arguments],
                                cwd=self.root, env=dict(self.env, **environment),
                                capture_output=True, text=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(result.stderr, '')
        parent, child = [line.split('=', 1)[1].split() for line in result.stdout.splitlines()]
        self.assertEqual(parent, child)
        return parent

    def test_default_uses_detected_cores_and_shares_jobserver(self):
        flags = self.flags()
        self.assertIn('-j6', flags)
        self.assertTrue(any(flag.startswith('--jobserver-auth=') for flag in flags))

    def test_explicit_job_settings_override_detection(self):
        for arguments, expected in ((('-j1',), '-j1'), (('-j4',), '-j4'),
                                    (('-j',), '-j'), (('--jobs=2',), '-j2'),
                                    (('-j', '3'), '-j3')):
            with self.subTest(arguments=arguments):
                self.assertIn(expected, self.flags(*arguments))

    def test_environment_job_settings_are_preserved(self):
        for variable in ('MAKEFLAGS', 'GNUMAKEFLAGS'):
            with self.subTest(variable=variable):
                self.assertIn('-j2', self.flags(**{variable: '-j2'}))

    def test_falls_back_to_getconf(self):
        (self.root / 'bin/nproc').write_text('#!/bin/sh\nexit 1\n')
        self.assertIn('-j3', self.flags())

    def test_falls_back_to_one_job(self):
        for name in ('nproc', 'getconf'):
            (self.root / 'bin' / name).write_text('#!/bin/sh\nexit 1\n')
        self.assertIn('-j1', self.flags())


if __name__ == '__main__':
    unittest.main()
