"""
Run the compressor up to a target position, then show what find_best sees
at each pair boundary within the potential F5 run starting there.
Usage: python f5_trace.py CHECKPS 10214
"""
import sys, os
sys.path.insert(0, 'd:/tmp')
from collections import defaultdict, deque
from compressor import count_backref_match, f5_cnt_qualifies, fb_qualifies_at, best_pattern_savings

def compress_to(src, target_pos):
    """Run the compressor, collecting the hash table, stop just before target_pos."""
    src = bytes(src)
    n = len(src)
    hash_table = defaultdict(deque)

    def add_to_hash(pos):
        if pos + 4 <= n:
            hash_table[src[pos:pos+4]].append(pos)

    # Replay compression up to target_pos using the same logic as compressor.py
    # but track what opcodes are emitted (to advance i correctly)
    from compressor import find_best
    i = 0
    raw_buf = bytearray()
    while i < target_pos:
        result = find_best(src, i, n, hash_table)
        if result is not None:
            encoded, advance = result
            advance = min(advance, target_pos - i)
            for j in range(i, i + advance):
                add_to_hash(j)
            i += advance
        else:
            add_to_hash(i)
            i += 1
    return hash_table

def find_best_at(src, pos, hash_table):
    """Return all candidates at pos, sorted by savings."""
    from compressor import find_best
    result = find_best(src, pos, len(src), hash_table)
    return result

def opcode_name(b):
    names = {0xF0:'F0',0xF1:'F1',0xF2:'F2',0xF3:'F3',0xF4:'F4',
             0xF5:'F5',0xF6:'F6',0xF7:'F7',0xF8:'F8',0xF9:'F9',
             0xFA:'FA',0xFB:'FB',0xFC:'FC',0xFD:'FD',0xFE:'FE',0xFF:'FF'}
    return names.get(b, f'{b:02x}')

name = sys.argv[1] if len(sys.argv) > 1 else 'CHECKPS'
f5_start = int(sys.argv[2]) if len(sys.argv) > 2 else 10214

dec_path = f'd:/tmp/decompressed/{name}.BIN.decompressed'
src = open(dec_path,'rb').read()
n = len(src)

print(f"Building hash table up to pos {f5_start}...")
hash_table = compress_to(src, f5_start)
print(f"Hash table has {len(hash_table)} entries")

# Compute F5 params at f5_start
fixed = src[f5_start]
cnt_q = 0
while cnt_q < 259 and f5_start+cnt_q*2+1 < n and src[f5_start+cnt_q*2] == fixed and src[f5_start+cnt_q*2+1] != fixed:
    cnt_q += 1
cnt_max = cnt_q
while cnt_max < 259 and f5_start+cnt_max*2+1 < n and src[f5_start+cnt_max*2] == fixed:
    cnt_max += 1

print(f"\nF5 at dec_pos={f5_start}: fixed=0x{fixed:02x}, cnt_q={cnt_q}, cnt_max={cnt_max}")
print(f"\nScanning pair boundaries (k=4..{min(cnt_max,cnt_q+5)}):")
print(f"{'k':>4} {'pair_pos':>9} {'best_op':>8} {'sav':>5} {'adv':>5}  raw_bytes  notes")

for k in range(4, min(cnt_max, 70) + 1):
    pair_pos = f5_start + k * 2
    if pair_pos >= n:
        break

    # Also add to hash table as if we're emitting F5 pairs up to here
    # (the reference compressor builds hash from emitted data)
    # For now, show what find_best sees at this point
    result = find_best_at(src, pair_pos, hash_table)

    if result is None:
        enc_name = 'RAW'
        sav = 0
        adv = 1
        raw_hex = f'{src[pair_pos]:02x}'
    else:
        enc, adv = result
        enc = bytes(enc)
        sav = adv - len(enc)
        enc_name = opcode_name(enc[0])
        raw_hex = enc[:6].hex(' ')

    # what's the data at pair_pos?
    data_bytes = src[pair_pos:pair_pos+4].hex(' ')
    notes = ''
    if enc[0] if result else 0 in (0xFC, 0xFD, 0xFE):
        notes = '<-- BACKREF'
    elif result and sav > 1:
        notes = '<-- HIGH SAV'
    elif result and sav == 1:
        notes = '<-- sav=1'

    print(f"{k:4d} {pair_pos:9d} {enc_name:>8} {sav:5d} {adv:5d}  [{raw_hex}]  {data_bytes}  {notes}")

    # Stop after finding first interesting case
    if result and sav > 1 and bytes(result[0])[0] in (0xFC, 0xFD, 0xFE):
        print(f"  --> BACKREF at k={k}, this is where reference would stop F5")
        break
