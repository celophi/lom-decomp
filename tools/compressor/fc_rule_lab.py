"""Dataset extractor + rule scorer for the open backref-selection problem.

Extracts every FC opcode in the reference streams where MULTIPLE window
candidates existed (a real choice), then scores candidate-selection rules
against all sites at once. This replaces one-hypothesis-at-a-time testing:
a correct rule must hit 100% here (2,798 sites over the default five files)
before it is worth wiring into compressor.py.

Sites where an FD could compete (match len >= 20 with offset <= 255) are
excluded to keep the decision pure-FC.

Usage:
    python fc_rule_lab.py                     # score built-in rules
    python fc_rule_lab.py GNAME SHOP          # restrict files
    python fc_rule_lab.py --failures oldest   # dump sites a rule misses

Add new rules to RULES; each takes (i, cands) where cands is a list of
(position, capped_match_len) and returns the predicted match position.

Results so far (2026-07-01): 'oldest' = current compressor behavior fits
95.1% overall and 100% of GOVER/MOVIE. Everything else tried is WORSE:
newest 33%, fixed-depth-K-oldest peaks 84% at K=8, ring-buffer-order scans
80%/41%, and a zlib-style prev-ring clobbering simulation is 94.9% at
mask 4095 (= oldest with an off-by-one) degrading with smaller masks.
The ~138 deviant sites (GNAME 89, SHOP 38, CLOAD 11) show variable
effective depth: ref sometimes picks 2nd-oldest, sometimes newest,
sometimes the middle of the savings-tied set. See PROGRESS.md.
"""
import os
import sys

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(os.path.dirname(TOOLS_DIR))
sys.path.insert(0, os.path.join(REPO_ROOT, 'tools', 'splat_ext'))
sys.path.insert(0, TOOLS_DIR)

from decompress import decompress
from region_diff import decode_opcodes

BIN_DIR = os.path.join(REPO_ROOT, 'disc', 'BIN')


def match_len(raw, i, p, cap=275):
    n = len(raw)
    dist = i - p
    m = 0
    mx = min(cap, dist)
    while m < mx and i + m < n and raw[p + m] == raw[i + m]:
        m += 1
    return m


def extract_sites(name):
    """Return (raw, sites); sites = (name, i, p_ref, cnt, cands, in_model)."""
    ref = open(os.path.join(BIN_DIR, f'{name}.BIN'), 'rb').read()
    raw = bytes(decompress(ref))
    rops = decode_opcodes(ref)
    sites = []
    for i, e, a in rops:
        if e[0] != 0xFC:
            continue
        off = e[1] | ((e[2] & 0xF) << 8)
        cnt = ((e[2] >> 4) & 0xF) + 4
        p_ref = i - off - 1
        key = raw[i:i + 4]
        cands = []
        skip = False
        for p in range(max(0, i - 4096), i):
            if raw[p:p + 4] != key:
                continue
            ml = match_len(raw, i, p)
            if ml < 4:
                continue
            if ml >= 20 and i - p - 1 <= 255:
                skip = True  # FD would compete
                break
            cands.append((p, min(ml, 19)))
        if skip or len(cands) < 2:
            continue
        sites.append((name, i, p_ref, cnt, cands,
                      p_ref in [p for p, m in cands]))
    return raw, sites


# ---------------- rules ----------------

def r_oldest(i, cands):
    """Current compressor behavior: max savings, tie -> largest distance."""
    best = max(m for p, m in cands)
    return min(p for p, m in cands if m == best)


def r_newest(i, cands):
    best = max(m for p, m in cands)
    return max(p for p, m in cands if m == best)


def make_depthk_oldest(k):
    def rule(i, cands):
        newest_k = sorted(cands, key=lambda c: -c[0])[:k]
        best = max(m for p, m in newest_k)
        return min(p for p, m in newest_k if m == best)
    rule.__name__ = f'depth{k}_oldest'
    return rule


def make_ring(start_at_wp, strict):
    def rule(i, cands):
        wp = i & 4095
        def order(c):
            b = c[0] & 4095
            return ((b - wp - 1) & 4095) if start_at_wp else b
        best_p, best_m = None, -1
        for p, m in sorted(cands, key=order):
            if (m > best_m) if strict else (m >= best_m):
                best_p, best_m = p, m
        return best_p
    rule.__name__ = f'ring_{"wp" if start_at_wp else "0"}_{"first" if strict else "last"}'
    return rule


RULES = {
    'oldest': r_oldest,
    'newest': r_newest,
    'depth2_oldest': make_depthk_oldest(2),
    'depth4_oldest': make_depthk_oldest(4),
    'depth8_oldest': make_depthk_oldest(8),
    'ring0_first': make_ring(False, True),
    'ring0_last': make_ring(False, False),
    'ring_wp_first': make_ring(True, True),
    'ring_wp_last': make_ring(True, False),
}


def main():
    args = [a for a in sys.argv[1:]]
    dump_rule = None
    if '--failures' in args:
        k = args.index('--failures')
        dump_rule = args[k + 1]
        del args[k:k + 2]
    names = [a.upper() for a in args] or ['GOVER', 'MOVIE', 'GNAME', 'SHOP', 'CLOAD']

    usable = []
    for name in names:
        _, sites = extract_sites(name)
        good = [s for s in sites if s[5]]
        usable.extend(good)
        print(f'{name}: {len(good)} usable multi-candidate FC sites')
    print(f'total: {len(usable)}\n')

    if dump_rule:
        rule = RULES[dump_rule]
        for (name, i, p_ref, cnt, cands, ok) in usable:
            if rule(i, cands) == p_ref:
                continue
            best = max(m for p, m in cands)
            tied = sorted((p for p, m in cands if m == best), reverse=True)
            rank = tied.index(p_ref) + 1 if p_ref in tied else -1
            cl = ' '.join(f'{"*" if p == p_ref else ""}{p}(off{i - p - 1},ml{m})'
                          for p, m in cands)
            print(f'{name} i={i} ref_rank_from_newest={rank}/{len(tied)}: {cl}')
        return

    for rname, rule in RULES.items():
        parts = []
        tot = 0
        for name in names:
            fs = [s for s in usable if s[0] == name]
            if not fs:
                continue
            h = sum(1 for (nm, i, p_ref, cnt, cands, ok) in fs
                    if rule(i, cands) == p_ref)
            tot += h
            parts.append(f'{name} {h}/{len(fs)}')
        print(f'{rname:16s} {tot:5d}/{len(usable)} '
              f'({100.0 * tot / len(usable):5.1f}%)  ' + '  '.join(parts))


if __name__ == '__main__':
    main()
