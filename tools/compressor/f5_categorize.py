"""
Categorize every F5 instance in every reference .BIN file.
For each F5 emitted by the reference compressor, record:
  - dec_pos, ref_cnt, fixed byte
  - cnt_q, cnt_max (theoretical max F5 length at that position)
  - what opcode the reference emits immediately AFTER this F5
  - what our find_best() finds at each pair boundary within the run
  - the smallest k >= 4 where any non-F5-extension opcode is found

Then tally by termination cause and look for invariants.
"""
import sys, os
from collections import defaultdict, deque, Counter

sys.path.insert(0, 'd:/tmp')
from compressor import find_best

# ──────────────────────────────────────────────────────────────────
# Decoder: parse reference compressed file into list of opcodes
# ──────────────────────────────────────────────────────────────────
def decode_opcodes(data):
    ops = []; src = data; n = len(src); src_ptr = 0; decomp_pos = 0
    while src_ptr < n:
        op_start = src_ptr; opcode = src[src_ptr]
        if opcode == 0xFF:
            ops.append((op_start, decomp_pos, 0xFF, bytes([0xFF]), 0)); break
        elif opcode == 0xF5:
            cnt = src[src_ptr+1]+4
            ops.append((op_start, decomp_pos, 0xF5, bytes(src[src_ptr:src_ptr+3+cnt]), cnt*2))
            src_ptr += 3+cnt; decomp_pos += cnt*2
        elif opcode == 0xF0:
            cnt = (src[src_ptr+1]&0xF)+3
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+2]), cnt))
            src_ptr += 2; decomp_pos += cnt
        elif opcode in (0xF1, 0xF8, 0xF9):
            cnt = src[src_ptr+1]+4
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+3]), cnt))
            src_ptr += 3; decomp_pos += cnt
        elif opcode == 0xF2:
            cnt = (src[src_ptr+1]+2)*2
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+3]), cnt))
            src_ptr += 3; decomp_pos += cnt
        elif opcode == 0xF3:
            cnt = (src[src_ptr+1]+2)*2
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+4]), cnt))
            src_ptr += 4; decomp_pos += cnt
        elif opcode == 0xF4:
            cnt = (src[src_ptr+1]+2)*3
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+5]), cnt))
            src_ptr += 5; decomp_pos += cnt
        elif opcode == 0xF6:
            c = src[src_ptr+1]+3
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+4+c]), c*3))
            src_ptr += 4+c; decomp_pos += c*3
        elif opcode == 0xF7:
            cnt = src[src_ptr+1]+2
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+5+cnt]), cnt*4))
            src_ptr += 5+cnt; decomp_pos += cnt*4
        elif opcode == 0xFA:
            cnt = src[src_ptr+1]+5
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+4]), cnt))
            src_ptr += 4; decomp_pos += cnt
        elif opcode == 0xFB:
            cnt = (src[src_ptr+1]+3)*2
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+5]), cnt))
            src_ptr += 5; decomp_pos += cnt
        elif opcode == 0xFC:
            cnt = (src[src_ptr+2]>>4)+4
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+3]), cnt))
            src_ptr += 3; decomp_pos += cnt
        elif opcode == 0xFD:
            cnt = src[src_ptr+2]+0x14
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+3]), cnt))
            src_ptr += 3; decomp_pos += cnt
        elif opcode == 0xFE:
            cnt = (src[src_ptr+1]&0xF)+3
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+2]), cnt))
            src_ptr += 2; decomp_pos += cnt
        else:
            cnt = opcode+1
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+1+cnt]), cnt))
            src_ptr += 1+cnt; decomp_pos += cnt
    return ops

def opname(b):
    return {0xF0:'F0',0xF1:'F1',0xF2:'F2',0xF3:'F3',0xF4:'F4',0xF5:'F5',
            0xF6:'F6',0xF7:'F7',0xF8:'F8',0xF9:'F9',0xFA:'FA',0xFB:'FB',
            0xFC:'FC',0xFD:'FD',0xFE:'FE',0xFF:'FF'}.get(b, 'RAW')

def compute_max_f5(dec, pos):
    n = len(dec)
    if pos+3 >= n: return 0, 0
    fixed = dec[pos]; cnt_q = 0
    while cnt_q<259 and pos+cnt_q*2+1<n and dec[pos+cnt_q*2]==fixed and dec[pos+cnt_q*2+1]!=fixed:
        cnt_q += 1
    if cnt_q < 1: return 0, 0
    cnt = cnt_q
    while cnt<259 and pos+cnt*2+1<n and dec[pos+cnt*2]==fixed:
        cnt += 1
    return cnt_q, cnt

# ──────────────────────────────────────────────────────────────────
# Categorize each F5
# ──────────────────────────────────────────────────────────────────
def categorize_file(name):
    ref_path = f'd:/tmp/compressed/{name}.BIN'
    dec_path = f'd:/tmp/decompressed/{name}.BIN.decompressed'
    if not os.path.exists(ref_path) or not os.path.exists(dec_path):
        return []

    ref_data = open(ref_path,'rb').read()
    dec_data = open(dec_path,'rb').read()
    ops = decode_opcodes(ref_data)

    # Walk through reference ops, building a hash table that mimics what the
    # original compressor would have built. For each F5 op, record context.
    n = len(dec_data)
    hash_table = defaultdict(deque)
    def add_to_hash(p):
        if p+4 <= n:
            hash_table[dec_data[p:p+4]].append(p)

    records = []
    for oi, op in enumerate(ops):
        cmp_pos, dec_pos, opcode, raw, adv = op
        if opcode == 0xF5:
            ref_cnt = len(raw) - 3
            fixed = raw[2]
            cnt_q, cnt_max = compute_max_f5(dec_data, dec_pos)
            next_op = ops[oi+1][2] if oi+1 < len(ops) else 0xFF
            next_op_raw = ops[oi+1][3] if oi+1 < len(ops) else b''
            next_dec = ops[oi+1][1] if oi+1 < len(ops) else dec_pos+adv

            # Find first k >= 4 where find_best returns ANY opcode, and
            # whether that opcode is backref/F5-same-fixed/F5-diff-fixed/other.
            first_k = None; first_kind = None; first_sav = 0
            first_backref_k = None
            first_f5_same_k = None
            for k in range(4, cnt_max+1):
                pp = dec_pos + k*2
                if pp >= n: break
                r = find_best(dec_data, pp, n, hash_table)
                if r is None: continue
                enc, av = r
                sav = av - len(enc)
                if sav <= 0: continue
                op0 = enc[0]
                if first_k is None:
                    first_k = k
                    first_sav = sav
                    if op0 in (0xFC, 0xFD, 0xFE):
                        first_kind = 'backref'
                    elif op0 == 0xF5:
                        if len(enc) >= 3 and enc[2] == fixed:
                            first_kind = 'f5_same'
                        else:
                            first_kind = 'f5_diff'
                    else:
                        first_kind = f'op{op0:02x}'
                if first_backref_k is None and op0 in (0xFC, 0xFD, 0xFE):
                    first_backref_k = k
                if first_f5_same_k is None and op0 == 0xF5 and len(enc) >= 3 and enc[2] == fixed:
                    first_f5_same_k = k

            records.append({
                'file': name,
                'dec_pos': dec_pos,
                'ref_cnt': ref_cnt,
                'fixed': fixed,
                'cnt_q': cnt_q,
                'cnt_max': cnt_max,
                'next_op': opname(next_op),
                'first_k': first_k,
                'first_kind': first_kind,
                'first_sav': first_sav,
                'first_backref_k': first_backref_k,
                'first_f5_same_k': first_f5_same_k,
            })

        # Update hash table for the bytes this opcode covered
        for p in range(dec_pos, dec_pos+adv):
            add_to_hash(p)

    return records

# ──────────────────────────────────────────────────────────────────
# Run on all files, tally
# ──────────────────────────────────────────────────────────────────
files = ['ADDHERO','CARDA','CHECKPS','CLOAD','FIELD','GNAME','GOLEM','GOSUB',
         'GOVER','MENU','MOVIE','NIKI','SHOP','TITLE','ZUKAN']

all_records = []
for name in files:
    print(f'Processing {name}...', flush=True)
    recs = categorize_file(name)
    all_records.extend(recs)
    print(f'  {len(recs)} F5 instances')

print(f'\n=== TOTAL: {len(all_records)} F5 instances across all files ===\n')

# Tally 1: termination cause (next op in reference)
print('--- Termination cause (next opcode in reference) ---')
c = Counter(r['next_op'] for r in all_records)
for op, cnt in c.most_common():
    print(f'  next={op:6s}: {cnt}')

# Tally 2: ref_cnt vs cnt_max relationship
print('\n--- ref_cnt vs cnt_max ---')
exact = sum(1 for r in all_records if r['ref_cnt'] == r['cnt_max'])
short = sum(1 for r in all_records if r['ref_cnt'] < r['cnt_max'])
print(f'  ref_cnt == cnt_max (max-extended): {exact}')
print(f'  ref_cnt <  cnt_max (truncated):    {short}')

# Tally 3: ref_cnt is always exactly cnt_q?
exact_q = sum(1 for r in all_records if r['ref_cnt'] == r['cnt_q'])
under_q = sum(1 for r in all_records if r['ref_cnt'] < r['cnt_q'])
over_q = sum(1 for r in all_records if r['ref_cnt'] > r['cnt_q'])
print(f'\n--- ref_cnt vs cnt_q ---')
print(f'  ref_cnt == cnt_q: {exact_q}')
print(f'  ref_cnt <  cnt_q: {under_q}')
print(f'  ref_cnt >  cnt_q (uses degenerate ext): {over_q}')

# Tally 4: For TRUNCATED F5s, what's the relationship between ref_cnt and first_k?
print('\n--- Truncated F5: ref_cnt vs first_k (smallest k where competing op exists) ---')
trunc = [r for r in all_records if r['ref_cnt'] < r['cnt_max']]
print(f'  total truncated: {len(trunc)}')
match_first_k = sum(1 for r in trunc if r['first_k'] is not None and r['ref_cnt'] == r['first_k'])
match_first_k_minus_1 = sum(1 for r in trunc if r['first_k'] is not None and r['ref_cnt'] == r['first_k']-1)
match_first_k_plus_1 = sum(1 for r in trunc if r['first_k'] is not None and r['ref_cnt'] == r['first_k']+1)
print(f'  ref_cnt == first_k:    {match_first_k}')
print(f'  ref_cnt == first_k-1:  {match_first_k_minus_1}')
print(f'  ref_cnt == first_k+1:  {match_first_k_plus_1}')
print(f'  no first_k (no competing op): {sum(1 for r in trunc if r["first_k"] is None)}')

# Tally 5: What KIND of competing op stopped each truncated F5?
print('\n--- Truncated F5: kind of competing op at first_k ---')
kinds = Counter(r['first_kind'] for r in trunc if r['first_k'] is not None)
for k, c in kinds.most_common():
    print(f'  {k}: {c}')

# Tally 6: backref-stopped offset
print('\n--- Backref-stopped F5: ref_cnt - first_backref_k ---')
br_stopped = [r for r in trunc if r['first_backref_k'] is not None]
offsets = Counter(r['ref_cnt'] - r['first_backref_k'] for r in br_stopped)
print(f'  total: {len(br_stopped)}')
for off, c in sorted(offsets.items()):
    print(f'  offset={off:+d}: {c}')

# Tally 7: f5_same-stopped offset
print('\n--- F5-same-fixed-stopped: ref_cnt - first_f5_same_k ---')
f5s_stopped = [r for r in trunc if r['first_f5_same_k'] is not None and r['first_backref_k'] is None]
offsets = Counter(r['ref_cnt'] - r['first_f5_same_k'] for r in f5s_stopped)
print(f'  total (f5_same w/o earlier backref): {len(f5s_stopped)}')
for off, c in sorted(offsets.items()):
    print(f'  offset={off:+d}: {c}')

# Detail: a few examples in each category
print('\n--- Sample records ---')
for r in all_records[:20]:
    print(f"  {r['file']:8s} dec={r['dec_pos']:7d} ref_cnt={r['ref_cnt']:3d} cnt_q={r['cnt_q']:3d} cnt_max={r['cnt_max']:3d} next={r['next_op']:5s} first_k={r['first_k']} kind={r['first_kind']}")
