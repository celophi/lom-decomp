#!/usr/bin/env python3
"""
Run objdiff diff on every unit in objdiff.json and write JSON results to
build/diffs/, mirroring the unit name as a path (e.g. "main/cdrom" ->
build/diffs/main/cdrom.json).

Usage:
    python3 tools/objdiff/run_diffs.py [--cli <path>]
"""

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent.parent.parent
DEFAULT_CLI = PROJECT_ROOT / "tools" / "objdiff" / "objdiff-cli-linux-x86_64"
CONFIG_PATH = PROJECT_ROOT / "objdiff.json"
OUTPUT_DIR = PROJECT_ROOT / "build" / "diffs"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cli", default=str(DEFAULT_CLI), help="Path to objdiff-cli binary")
    args = parser.parse_args()

    cli = Path(args.cli)
    if not cli.exists():
        print(f"error: objdiff-cli not found at {cli}", file=sys.stderr)
        sys.exit(1)
    cli.chmod(cli.stat().st_mode | 0o111)

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
        out = OUTPUT_DIR / f"{name}.json"
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

    print(f"Done: {ok} ok, {fail} failed. Results in {OUTPUT_DIR}/")


if __name__ == "__main__":
    main()
