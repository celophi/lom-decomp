#!/usr/bin/env python3
"""
Run objdiff diff on every unit in objdiff.json and write JSON results to
build/<version>/diffs/, mirroring the unit name as a path (e.g. "main/cdrom" ->
build/us/diffs/main/cdrom.json).

Usage:
    python3 tools/objdiff/run_diffs.py [--cli <path>] [--output-dir <dir>]
"""

import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent.parent.parent
DEFAULT_CLI = "objdiff-cli"
CONFIG_PATH = PROJECT_ROOT / "objdiff.json"
DEFAULT_OUTPUT_DIR = PROJECT_ROOT / "build" / "us" / "diffs"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cli", default=DEFAULT_CLI,
                        help="objdiff-cli command or path (default: objdiff-cli on PATH)")
    parser.add_argument("--output-dir", default=str(DEFAULT_OUTPUT_DIR),
                        help="Directory for per-unit JSON diffs (default: build/us/diffs)")
    args = parser.parse_args()
    output_dir = Path(args.output_dir)

    cli = shutil.which(args.cli)
    if cli is None:
        print(f"error: objdiff-cli executable not found: {args.cli} -- "
              "install it with tools/objdiff/install_objdiff.sh or pass --cli <path>",
              file=sys.stderr)
        sys.exit(1)

    if not CONFIG_PATH.exists():
        print(f"error: {CONFIG_PATH} not found -- run 'make objdiff-config' first", file=sys.stderr)
        sys.exit(1)

    with open(CONFIG_PATH) as f:
        config = json.load(f)

    units = config.get("units", [])
    print(f"Diffing {len(units)} units...")

    ok = 0
    fail = 0
    for unit in units:
        name = unit["name"]
        target = unit["target_path"]
        base = unit["base_path"]
        out = output_dir / f"{name}.json"
        out.parent.mkdir(parents=True, exist_ok=True)

        result = subprocess.run(
            [str(cli), "diff", "-1", target, "-2", base, "-o", str(out), "--format", "json"],
            capture_output=True,
        )
        if result.returncode == 0:
            ok += 1
        else:
            fail += 1
            print(f"  FAIL: {name}", file=sys.stderr)
            if result.stderr:
                print(f"        {result.stderr.decode().strip()}", file=sys.stderr)

    print(f"Done: {ok} ok, {fail} failed. Results in {output_dir}/")


if __name__ == "__main__":
    main()
