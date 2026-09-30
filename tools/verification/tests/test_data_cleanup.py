from pathlib import Path
import subprocess
import tempfile
import unittest


class DataCleanupTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        rules = Path(__file__).resolve().parents[3] / 'mk' / 'verification.mk'
        self.makefile = self.root / 'test.mk'
        # The inner build makes representative outputs without a PS1 compiler.
        # It also checks that the previous data object was removed first.
        self.makefile.write_text(f'''
STAGING := staging
BUILD_DIR := build/test
ifeq ($(DATA_AS_C),1)
.PHONY: verify-bins
verify-bins:
\ttest ! -e staging/build/test/data/old.o
\tmkdir -p staging/build/test/data build/test/data staging/datac/build/test
\ttouch staging/build/test/data/new.o build/test/data/new.o staging/datac/build/test/generated.c
\texit $(RESULT)
else
include {rules}
endif
''')
        self.old = self.root / 'staging/build/test/data/old.o'
        self.old.parent.mkdir(parents=True)
        self.old.touch()
        self.code = self.root / 'staging/build/test/src/main.o'
        self.code.parent.mkdir(parents=True)
        self.code.touch()

    def run_check(self, result, *flags):
        return subprocess.run(
            ['make', '--no-print-directory', '-f', str(self.makefile),
             f'MAKE=make --no-print-directory -f {self.makefile}',
             f'RESULT={result}', *flags, 'verify-data-as-c'],
            cwd=self.root, capture_output=True, text=True,
        )

    def assert_cleaned(self):
        self.assertFalse(self.old.exists())
        self.assertFalse((self.old.parent / 'new.o').exists())
        self.assertFalse((self.root / 'build/test/data/new.o').exists())
        self.assertFalse((self.root / 'staging/datac/build/test').exists())
        self.assertTrue(self.code.exists())

    def test_success_cleans_before_and_after(self):
        result = self.run_check(0)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assert_cleaned()

    def test_failure_cleans_and_still_fails(self):
        result = self.run_check(7)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Error 7', result.stderr)
        self.assert_cleaned()

    def test_dry_run_leaves_objects_alone(self):
        result = self.run_check(0, '-n')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertTrue(self.old.exists())
        self.assertTrue(self.code.exists())
        self.assertFalse((self.old.parent / 'new.o').exists())


if __name__ == '__main__':
    unittest.main()
