#!/usr/bin/env python3
"""Compare a rebuilt file with its disc original and record the result."""

import argparse
import fcntl
import hashlib
from pathlib import Path


def update_manifest(path, name, matched):
    path.parent.mkdir(parents=True, exist_ok=True)
    # make -j can finish several overlays at once. Lock the read and write
    # together so one result cannot overwrite another overlay's entry.
    with path.open('a+') as manifest:
        fcntl.flock(manifest, fcntl.LOCK_EX)
        manifest.seek(0)
        entries = manifest.read().splitlines()
        entries = [entry for entry in entries if entry != name]
        if matched:
            entries.append(name)
        manifest.seek(0)
        manifest.truncate()
        manifest.writelines(entry + '\n' for entry in entries)
        manifest.flush()


def verify(original, rebuilt, *, manifest=None, name=None, raw=False):
    matched = False
    label = original.name + (' raw' if raw else '')
    try:
        expected = hashlib.sha1(original.read_bytes()).hexdigest()
        actual = hashlib.sha1(rebuilt.read_bytes()).hexdigest()
        print(f'{label} expected: {expected}')
        print(f'{label} actual:   {actual}')
        matched = expected == actual
    finally:
        # A failed check must also clear an earlier success. Otherwise objdiff
        # keeps reporting the image as complete after its bytes have changed.
        if manifest is not None:
            update_manifest(manifest, name, matched)
    if matched:
        print(f'[OK] {label} matches original ROM')
    else:
        print(f'[FAIL] {label} sha1 mismatch')
    return matched


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('original', type=Path)
    parser.add_argument('rebuilt', type=Path)
    parser.add_argument('--manifest', type=Path)
    parser.add_argument('--name')
    parser.add_argument('--raw', action='store_true')
    args = parser.parse_args()
    if (args.manifest is None) != (args.name is None):
        parser.error('--manifest and --name must be supplied together')
    try:
        matched = verify(args.original, args.rebuilt, manifest=args.manifest,
                         name=args.name, raw=args.raw)
    except OSError as error:
        parser.exit(1, f'[FAIL] {error}\n')
    return 0 if matched else 1


if __name__ == '__main__':
    raise SystemExit(main())
