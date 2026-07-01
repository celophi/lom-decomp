"""Inspect back-reference (FC/FD/FE) candidates at a decompressed position.

For the given position, lists every earlier position in the 4KB window whose
4-gram matches and whose match length is >= 4, annotated with:
  - offset/distance from the query position
  - match length
  - which REFERENCE opcode covers the candidate (type + offset within it),
    so insertion-policy hypotheses can be checked against the reference's
    actual pick.

This is the main tool for the one remaining unsolved rule (see PROGRESS.md
"Open problem": backref candidate selection).

Usage:
    python backref_candidates.py GNAME 21985
    python backref_candidates.py GNAME 21985 --ref-off 0x6b3   # mark ref pick
"""
import os
import sys

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(os.path.dirname(TOOLS_DIR))
sys.path.insert(0, os.path.join(REPO_ROOT, 'tools', 'splat_ext'))
sys.path.insert(0, TOOLS_DIR)

from decompress import decompress
from region_diff import decode_opcodes


def match_len(raw, i, p, cap=275):
    n = len(raw)
    dist = i - p
    m = 0
    mx = min(cap, dist)  # no self-referential copies
    while m < mx and i + m < n and raw[p + m] == raw[i + m]:
        m += 1
    return m


def opname(t):
    return 'RAW' if t < 0xF0 else f'F{t & 0xF:X}'


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)
    name = sys.argv[1].upper()
    i = int(sys.argv[2])
    ref_off = None
    if '--ref-off' in sys.argv:
        ref_off = int(sys.argv[sys.argv.index('--ref-off') + 1], 0)

    ref = open(os.path.join(REPO_ROOT, 'disc', 'BIN', f'{name}.BIN'), 'rb').read()
    raw = bytes(decompress(ref))
    rops = decode_opcodes(ref)

    cover = {}
    for p, e, a in rops:
        for k in range(a):
            cover[p + k] = (e[0], k, p, a)

    key = raw[i:i + 4]
    ref_p = i - ref_off - 1 if ref_off is not None else None
    print(f'{name} dec {i}: key={key.hex(" ")}'
          + (f'  ref_pick={ref_p} (offset {ref_off:#x})' if ref_p is not None else ''))
    for p in range(max(0, i - 4096), i):
        if raw[p:p + 4] != key:
            continue
        ml = match_len(raw, i, p)
        if ml < 4:
            continue
        t, k, st, a = cover.get(p, (None, None, None, None))
        mark = '  <== REF PICK' if p == ref_p else ''
        print(f'  p={p:7d} offset={i - p - 1:5d} mlen={ml:3d} '
              f'covered_by={opname(t)}+{k} (op at {st}, adv {a}){mark}')


if __name__ == '__main__':
    main()
