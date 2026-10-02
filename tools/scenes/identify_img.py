#!/usr/bin/env python3
"""Identify scene IMG files by their contents, without extracting any assets.

Run from the repository root with Python 3.10 or newer. A directory scan checks
files with an .IMG extension, ignoring case; an explicit file can have any name.
Exit status is 0 when inspection completes, 1 for an input or reading error,
and 2 for invalid command-line arguments. Nonmatching files are normal results.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from tools.scenes.field_scene import (
    SceneHeader,
    read_layout,
    read_group_bounds,
    read_portraits,
    read_textures,
)


def scene_format_error(data: bytes) -> str | None:
    """Return the first failed scene check, or None when the format matches.

    These are structural checks, not a validation of script behavior or every
    resource type. Unknown IMG families and damaged scenes both fail detection.
    """
    try:
        header = SceneHeader.parse(data)
        read_layout(data, header)
        read_textures(data, header)
        read_portraits(data, header)
        read_group_bounds(data, header)
    except ValueError as error:
        return str(error)

    return None


def find_img_files(source: Path, recursive: bool) -> list[Path]:
    """List IMG files in a directory, or return the explicitly requested file."""
    if source.is_file():
        return [source]
    if not source.is_dir():
        raise ValueError(f"not a file or directory: {source}")

    paths = source.rglob("*") if recursive else source.iterdir()
    files = sorted(path for path in paths if path.suffix.lower() == ".img" and path.is_file())
    if not files:
        raise ValueError(f"no IMG files found in {source}")
    return files


def main(argv: list[str] | None = None) -> int:
    """Print each file's classification and return the documented exit status."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path, help="file to check, or directory of IMG files")
    parser.add_argument("-r", "--recursive", action="store_true", help="include subdirectories")
    args = parser.parse_args(argv)

    try:
        files = find_img_files(args.source, args.recursive)
    except (OSError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    scenes = 0
    other = 0
    errors = 0
    for path in files:
        try:
            data = path.read_bytes()
        except OSError as error:
            print(f"error: {error}", file=sys.stderr)
            errors += 1
            continue

        reason = scene_format_error(data)
        if reason is None:
            print(f"{path}: scene IMG")
            scenes += 1
        else:
            print(f"{path}: not scene IMG ({reason})")
            other += 1

    if args.source.is_dir():
        print(f"Checked {len(files)} files: {scenes} scene IMG, {other} not scene IMG, {errors} errors")
    if errors:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
