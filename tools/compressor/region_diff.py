"""Opcode-level diff between a reference BIN and our recompression of it.

Decodes both compressed streams into (decompressed_position, encoding, advance)
opcode lists and reports the divergence regions where they disagree. This is
the primary debugging tool: byte-level diff counts are misleading (one wrong
opcode shifts every following byte), but region count tells you how many
distinct wrong DECISIONS remain, and the dump around each region start shows
the reference's choice vs ours next to the underlying data.

Usage:
    python region_diff.py GNAME              # list divergence regions
    python region_diff.py GNAME --dump 8     # also dump first 8 regions
    python region_diff.py GNAME --at 21985   # dump opcodes/data around one
                                             # decompressed position
"""
import os
import sys

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(os.path.dirname(TOOLS_DIR))
sys.path.insert(0, os.path.join(REPO_ROOT, 'tools', 'splat_ext'))
sys.path.insert(0, TOOLS_DIR)

from decompress import decompress
import compressor


def decode_opcodes(src):
    """Decode a compressed stream into a list of (dec_pos, enc_bytes, advance).

    Lengths must mirror tools/splat_ext/decompress.py exactly. Note F6:
    iterations = param + 3 (NOT +2) - an earlier version of this tooling had
    that wrong, which is why old analyses concluded F6 was never generated.
    """
    ops = []
    sp = 0
    dp = 0
    n = len(src)
    while sp < n:
        op = src[sp]
        start = sp
        if op < 0xF0:
            cnt = op + 1
            sp += 1 + cnt
            adv = cnt
        elif op == 0xF0:
            adv = (src[sp + 1] & 0xF) + 3
            sp += 2
        elif op == 0xF1:
            adv = src[sp + 1] + 4
            sp += 3
        elif op == 0xF2:
            adv = (src[sp + 1] + 2) * 2
            sp += 3
        elif op == 0xF3:
            adv = (src[sp + 1] + 2) * 2
            sp += 4
        elif op == 0xF4:
            adv = (src[sp + 1] + 2) * 3
            sp += 5
        elif op == 0xF5:
            cnt = src[sp + 1] + 4
            adv = cnt * 2
            sp += 3 + cnt
        elif op == 0xF6:
            cnt = src[sp + 1] + 3
            adv = cnt * 3
            sp += 4 + cnt
        elif op == 0xF7:
            cnt = src[sp + 1] + 2
            adv = cnt * 4
            sp += 5 + cnt
        elif op == 0xF8:
            adv = src[sp + 1] + 4
            sp += 3
        elif op == 0xF9:
            adv = src[sp + 1] + 4
            sp += 3
        elif op == 0xFA:
            adv = src[sp + 1] + 5
            sp += 4
        elif op == 0xFB:
            adv = (src[sp + 1] + 3) * 2
            sp += 5
        elif op == 0xFC:
            adv = ((src[sp + 2] >> 4) & 0xF) + 4
            sp += 3
        elif op == 0xFD:
            adv = src[sp + 2] + 0x14
            sp += 3
        elif op == 0xFE:
            adv = (src[sp + 1] & 0xF) + 3
            sp += 2
        elif op == 0xFF:
            break
        ops.append((dp, bytes(src[start:sp]), adv))
        dp += adv
    return ops


def find_regions(rops, oops):
    """Regions of decompressed positions where the opcode streams disagree.

    A position is 'in sync' when both streams start an identical opcode
    there; a region runs from the first out-of-sync position to the next
    sync point.
    """
    rd = {p: e for p, e, a in rops}
    od = {p: e for p, e, a in oops}
    positions = sorted(set(rd) | set(od))
    events = []
    in_div = False
    start = None
    for p in positions:
        same = (p in rd) and (p in od) and rd[p] == od[p]
        if not same and not in_div:
            in_div, start = True, p
        elif same and in_div:
            in_div = False
            events.append((start, p))
    if in_div:
        events.append((start, positions[-1]))
    return events


def dump_around(ops, dec_pos, label, before=4, after=100):
    print(f'--- {label} ---')
    for p, e, a in ops:
        if p + a > dec_pos - before and p < dec_pos + after:
            trunc = ' ...' if len(e) > 20 else ''
            print(f'  dec {p:6d} adv {a:4d}  {e[:20].hex(" ")}{trunc}')


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    name = sys.argv[1].upper()
    dump_n = 0
    at = None
    if '--dump' in sys.argv:
        dump_n = int(sys.argv[sys.argv.index('--dump') + 1])
    if '--at' in sys.argv:
        at = int(sys.argv[sys.argv.index('--at') + 1])

    ref = open(os.path.join(REPO_ROOT, 'disc', 'BIN', f'{name}.BIN'), 'rb').read()
    raw = bytes(decompress(ref))
    ours = compressor.compress(raw)

    rops = decode_opcodes(ref)
    oops = decode_opcodes(ours)

    if at is not None:
        dump_around(rops, at, 'REF')
        dump_around(oops, at, 'OURS')
        lo = max(0, at - 8)
        print('data:', raw[lo:at].hex(' '), '|', raw[at:at + 72].hex(' '))
        return

    events = find_regions(rops, oops)
    print(f'{name}: ref {len(rops)} opcodes, ours {len(oops)}, '
          f'{len(events)} divergence regions')
    rd = {p: e for p, e, a in rops}
    od = {p: e for p, e, a in oops}
    for s, e in events[:60]:
        r = rd.get(s)
        o = od.get(s)
        rs = r[:10].hex(' ') if r else '(mid-opcode)'
        os_ = o[:10].hex(' ') if o else '(mid-opcode)'
        print(f'  dec {s:6d}..{e:6d} len {e - s:6d}  ref={rs:30s} ours={os_}')
    if len(events) > 60:
        print(f'  ... and {len(events) - 60} more')

    for s, e in events[:dump_n]:
        print(f'\n=== region dec {s}..{e} ===')
        dump_around(rops, s, 'REF')
        dump_around(oops, s, 'OURS')
        lo = max(0, s - 8)
        print('data:', raw[lo:s].hex(' '), '|', raw[s:s + 72].hex(' '))


if __name__ == '__main__':
    main()
