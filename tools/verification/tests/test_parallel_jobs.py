"""Check the job limits chosen by mk/parallel.mk, including recursive Make calls."""

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


REPOSITORY_ROOT = Path(__file__).resolve().parents[3]
FIXTURES_DIRECTORY = Path(__file__).resolve().parent / 'fixtures'


class ParallelJobsTests(unittest.TestCase):
    def setUp(self):
        temporary_directory = tempfile.TemporaryDirectory()
        self.addCleanup(temporary_directory.cleanup)
        self.project_directory = Path(temporary_directory.name)
        self.commands_directory = self.project_directory / 'bin'
        self.commands_directory.mkdir()

        shutil.copyfile(
            REPOSITORY_ROOT / 'mk/parallel.mk',
            self.project_directory / 'parallel.mk',
        )
        shutil.copyfile(
            FIXTURES_DIRECTORY / 'parallel_jobs.mk',
            self.project_directory / 'Makefile',
        )

        # Fixed CPU counts make the expected limit independent of this machine.
        self.write_fake_command('nproc', 'echo 6')
        self.write_fake_command('getconf', 'echo 3')
        self.make_environment = os.environ.copy()
        self.make_environment['PATH'] = f"{self.commands_directory}:{os.environ['PATH']}"

        # Start a new top-level Make even when this suite runs under make test-tools.
        # An inherited jobserver would refer to descriptors closed by subprocess.
        for variable in ('MAKEFLAGS', 'MFLAGS', 'GNUMAKEFLAGS', 'MAKELEVEL'):
            self.make_environment.pop(variable, None)

    def write_fake_command(self, name, shell_command):
        command_path = self.commands_directory / name
        command_path.write_text(f'#!/bin/sh\n{shell_command}\n')
        command_path.chmod(0o755)

    def run_make_and_read_flags(self, *arguments, environment_overrides=None):
        environment = self.make_environment.copy()
        if environment_overrides:
            environment.update(environment_overrides)

        result = subprocess.run(
            ['make', '--no-print-directory', *arguments],
            cwd=self.project_directory,
            env=environment,
            capture_output=True,
            text=True,
            timeout=30,
        )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(result.stderr, '', 'Make should not warn about a broken jobserver')

        # The fixture prints one "parent=..." line and one "child=..." line.
        flags_by_process = {}
        for line in result.stdout.splitlines():
            process_name, separator, flag_text = line.partition('=')
            self.assertEqual(separator, '=', f'Unexpected Make output: {line}')
            flags_by_process[process_name] = flag_text.split()
        self.assertEqual(set(flags_by_process), {'parent', 'child'})
        return flags_by_process

    def assert_job_limit(self, flags_by_process, expected_flag):
        for process_name in ('parent', 'child'):
            self.assertIn(expected_flag, flags_by_process[process_name], process_name)

    def test_default_uses_the_cpu_count_reported_by_nproc(self):
        flags = self.run_make_and_read_flags()

        self.assert_job_limit(flags, '-j6')

    def test_recursive_make_shares_the_parent_jobserver(self):
        flags = self.run_make_and_read_flags()
        parent_jobserver_flags = [
            flag for flag in flags['parent'] if flag.startswith('--jobserver-auth=')
        ]

        self.assertEqual(len(parent_jobserver_flags), 1)
        self.assertIn(parent_jobserver_flags[0], flags['child'])
        self.assertEqual(flags['parent'], flags['child'])

    def test_explicit_single_job_disables_automatic_parallelism(self):
        flags = self.run_make_and_read_flags('-j1')

        self.assert_job_limit(flags, '-j1')

    def test_explicit_job_count_overrides_the_detected_cpu_count(self):
        flags = self.run_make_and_read_flags('-j4')

        self.assert_job_limit(flags, '-j4')

    def test_jobs_option_without_a_count_keeps_unlimited_parallelism(self):
        flags = self.run_make_and_read_flags('-j')

        self.assert_job_limit(flags, '-j')

    def test_long_jobs_option_overrides_the_detected_cpu_count(self):
        flags = self.run_make_and_read_flags('--jobs=2')

        self.assert_job_limit(flags, '-j2')

    def test_job_count_can_be_a_separate_command_line_argument(self):
        flags = self.run_make_and_read_flags('-j', '3')

        self.assert_job_limit(flags, '-j3')

    def test_makeflags_environment_can_set_the_job_count(self):
        flags = self.run_make_and_read_flags(environment_overrides={'MAKEFLAGS': '-j2'})

        self.assert_job_limit(flags, '-j2')

    def test_gnumakeflags_environment_can_set_the_job_count(self):
        flags = self.run_make_and_read_flags(environment_overrides={'GNUMAKEFLAGS': '-j2'})

        self.assert_job_limit(flags, '-j2')

    def test_getconf_supplies_the_cpu_count_when_nproc_fails(self):
        self.write_fake_command('nproc', 'exit 1')

        flags = self.run_make_and_read_flags()

        self.assert_job_limit(flags, '-j3')

    def test_one_job_is_used_when_both_cpu_count_commands_fail(self):
        self.write_fake_command('nproc', 'exit 1')
        self.write_fake_command('getconf', 'exit 1')

        flags = self.run_make_and_read_flags()

        self.assert_job_limit(flags, '-j1')


if __name__ == '__main__':
    unittest.main()
