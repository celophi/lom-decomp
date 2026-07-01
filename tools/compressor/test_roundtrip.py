"""Round-trip test: decompress each reference BIN, recompress it with
compressor.py, and compare byte-for-byte against the original.

This is the canonical progress metric for the compressor reverse-engineering
effort (see PROGRESS.md). It needs no build artifacts: the input to the
compressor is reconstructed by decompressing the reference BIN itself, which
is exactly what the build pipeline would feed it (objcopy output + 0x00
prefix byte).

Usage:
    python test_roundtrip.py                # all BINs in disc/BIN/
    python test_roundtrip.py GNAME GOVER    # only these
"""
import glob
import os
import sys

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(os.path.dirname(TOOLS_DIR))
sys.path.insert(0, os.path.join(REPO_ROOT, 'tools', 'splat_ext'))
sys.path.insert(0, TOOLS_DIR)

from decompress import decompress  # reference decompressor (correct, frozen)
import compressor


def main():
    only = set(a.upper() for a in sys.argv[1:]) if len(sys.argv) > 1 else None
    bins = sorted(glob.glob(os.path.join(REPO_ROOT, 'disc', 'BIN', '*.BIN')))
    exact = 0
    tested = 0
    for path in bins:
        name = os.path.basename(path)
        if only and name.split('.')[0] not in only:
            continue
        tested += 1
        ref = open(path, 'rb').read()
        try:
            dec = bytes(decompress(ref))
        except Exception as e:
            print(f'{name:12s} DECOMPRESS ERROR: {e}')
            continue
        ours = compressor.compress(dec)
        if ours == ref:
            exact += 1
            print(f'{name:12s} MATCH ({len(ref)} bytes)')
        else:
            ndiff = sum(1 for a, b in zip(ours, ref) if a != b) + abs(len(ours) - len(ref))
            first = next((k for k in range(min(len(ours), len(ref))) if ours[k] != ref[k]),
                         min(len(ours), len(ref)))
            print(f'{name:12s} diffs={ndiff:7d} first={first:7d} '
                  f'(ref {len(ref)}, ours {len(ours)})')
    print(f'\n{exact}/{tested} files match exactly.')


if __name__ == '__main__':
    main()
