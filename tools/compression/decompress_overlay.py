"""Prepare a disc overlay for splat, preserving its leading format byte."""

import argparse
import hashlib
from pathlib import Path
import sys

import yaml

from .decompress import decompress


def decompress_overlay(source: Path, destination: Path, config_path: Path) -> int:
    config = yaml.safe_load(config_path.read_text())
    compressed = source.read_bytes()
    expected = config['compressed_sha1'].lower()
    actual = hashlib.sha1(compressed).hexdigest()
    if actual != expected:
        raise ValueError(f'{source}: compressed SHA-1 mismatch: expected {expected}, got {actual}')
    if compressed[:1] != b'\x01':
        raise ValueError(f'{source}: expected overlay compression format 0x01')

    # Splat subsegment offsets include byte zero, although the code starts at 1.
    output = compressed[:1] + decompress(compressed[1:])
    expected = config['sha1'].lower()
    actual = hashlib.sha1(output).hexdigest()
    if actual != expected:
        raise ValueError(f'{source}: decompressed SHA-1 mismatch: expected {expected}, got {actual}')

    # A failed hash check must leave any previously prepared file intact.
    destination.parent.mkdir(parents=True, exist_ok=True)
    temporary = destination.with_name(destination.name + '.tmp')
    temporary.write_bytes(output)
    temporary.replace(destination)
    return len(output)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config', required=True, type=Path, help='overlay splat YAML with both expected hashes')
    parser.add_argument('source', type=Path, help='original compressed disc BIN')
    parser.add_argument('destination', type=Path, help='decompressed splat input')
    args = parser.parse_args()
    try:
        size = decompress_overlay(args.source, args.destination, args.config)
    except (OSError, ValueError, KeyError, IndexError, yaml.YAMLError) as error:
        print(f'decompress-overlay: {error}', file=sys.stderr)
        return 1
    print(f'Decompressed {args.source} -> {args.destination} ({size} bytes)')
    return 0


if __name__ == '__main__':
    sys.exit(main())
