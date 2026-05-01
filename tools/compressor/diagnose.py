"""
Decode a reference .BIN file opcode-by-opcode and compare with our compressor output.
Usage: python diagnose.py <name>   (e.g. python diagnose.py ZUKAN)
Shows the first N differing opcodes with context.
"""
import sys, os
sys.path.insert(0, 'd:/tmp')
from compressor import compress

def decode_opcodes(data):
    """Decode compressed data into list of (cmp_pos, decomp_pos, opcode_bytes, decomp_advance)."""
    ops = []
    src_ptr = 0
    decomp_pos = 0
    src = data
    n = len(src)
    while src_ptr < n:
        op_start = src_ptr
        opcode = src[src_ptr]
        if opcode == 0xFF:
            ops.append((op_start, decomp_pos, bytes([0xFF]), 0))
            break
        elif opcode == 0xF0:
            cnt = (src[src_ptr+1] & 0xF) + 3
            ops.append((op_start, decomp_pos, bytes(src[src_ptr:src_ptr+2]), cnt))
            src_ptr += 2; decomp_pos += cnt
        elif opcode == 0xF1:
            cnt = src[src_ptr+1] + 4
            ops.append((op_start, decomp_pos, bytes(src[src_ptr:src_ptr+3]), cnt))
            src_ptr += 3; decomp_pos += cnt
        elif opcode == 0xF2:
            cnt = (src[src_ptr+1] + 2) * 2
            ops.append((op_start, decomp_pos, bytes(src[src_ptr:src_ptr+3]), cnt))
            src_ptr += 3; decomp_pos += cnt
        elif opcode == 0xF3:
            cnt = (src[src_ptr+1] + 2) * 2
            ops.append((op_start, decomp_pos, bytes(src[src_ptr:src_ptr+4]), cnt))
            src_ptr += 4; decomp_pos += cnt
        elif opcode == 0xF4:
            cnt = (src[src_ptr+1] + 2) * 3
            ops.append((op_start, decomp_pos, bytes(src[src_ptr:src_ptr+5]), cnt))
            src_ptr += 5; decomp_pos += cnt
        elif opcode == 0xF5:
            cnt = src[src_ptr+1] + 4
            ops.append((op_start, decomp_pos, bytes(src[src_ptr:src_ptr+3+cnt]), cnt*2))
            src_ptr += 3 + cnt; decomp_pos += cnt * 2
        elif opcode == 0xF6:
            cnt = (src[src_ptr+1] + 3) * 3
            ops.append((op_start, decomp_pos, bytes(src[src_ptr:src_ptr+4+src[src_ptr+1]+3]), cnt))
            src_ptr += 4 + src[src_ptr+1] + 3; decomp_pos += cnt
        elif opcode == 0xF7:
            cnt = src[src_ptr+1] + 2
            ops.append((op_start, decomp_pos, bytes(src[src_ptr:src_ptr+5+cnt]), cnt*4))
            src_ptr += 5 + cnt; decomp_pos += cnt * 4
        elif opcode == 0xF8:
            cnt = src[src_ptr+1] + 4
            ops.append((op_start, decomp_pos, bytes(src[src_ptr:src_ptr+3]), cnt))
            src_ptr += 3; decomp_pos += cnt
        elif opcode == 0xF9:
            cnt = src[src_ptr+1] + 4
            ops.append((op_start, decomp_pos, bytes(src[src_ptr:src_ptr+3]), cnt))
            src_ptr += 3; decomp_pos += cnt
        elif opcode == 0xFA:
            cnt = src[src_ptr+1] + 5
            ops.append((op_start, decomp_pos, bytes(src[src_ptr:src_ptr+4]), cnt))
            src_ptr += 4; decomp_pos += cnt
        elif opcode == 0xFB:
            cnt = (src[src_ptr+1] + 3) * 2
            ops.append((op_start, decomp_pos, bytes(src[src_ptr:src_ptr+5]), cnt))
            src_ptr += 5; decomp_pos += cnt
        elif opcode == 0xFC:
            cnt = (src[src_ptr+2] >> 4) + 4
            ops.append((op_start, decomp_pos, bytes(src[src_ptr:src_ptr+3]), cnt))
            src_ptr += 3; decomp_pos += cnt
        elif opcode == 0xFD:
            cnt = src[src_ptr+2] + 0x14
            ops.append((op_start, decomp_pos, bytes(src[src_ptr:src_ptr+3]), cnt))
            src_ptr += 3; decomp_pos += cnt
        elif opcode == 0xFE:
            cnt = (src[src_ptr+1] & 0xF) + 3
            ops.append((op_start, decomp_pos, bytes(src[src_ptr:src_ptr+2]), cnt))
            src_ptr += 2; decomp_pos += cnt
        else:
            cnt = opcode + 1
            ops.append((op_start, decomp_pos, bytes(src[src_ptr:src_ptr+1+cnt]), cnt))
            src_ptr += 1 + cnt; decomp_pos += cnt
    return ops

def opcode_name(b):
    names = {0xF0:'F0',0xF1:'F1',0xF2:'F2',0xF3:'F3',0xF4:'F4',
             0xF5:'F5',0xF6:'F6',0xF7:'F7',0xF8:'F8',0xF9:'F9',
             0xFA:'FA',0xFB:'FB',0xFC:'FC',0xFD:'FD',0xFE:'FE',0xFF:'FF'}
    if b in names: return names[b]
    return f'RAW({b+1})'

def fmt_op(cmp_pos, decomp_pos, raw, adv):
    name = opcode_name(raw[0])
    hex_bytes = raw[:12].hex(' ') + ('...' if len(raw) > 12 else '')
    return f'  cmp={cmp_pos:6d} dec={decomp_pos:7d} adv={adv:4d}  {name:6s}  [{hex_bytes}]'

name = sys.argv[1] if len(sys.argv) > 1 else 'ZUKAN'
ref_path = f'd:/tmp/compressed/{name}.BIN'
dec_path = f'd:/tmp/decompressed/{name}.BIN.decompressed'

ref_data = open(ref_path, 'rb').read()
dec_data = open(dec_path, 'rb').read()

print(f"Compressing {name}...")
our_data = compress(dec_data)

ref_ops = decode_opcodes(ref_data)
our_ops = decode_opcodes(our_data)

# Find first differing compressed byte
first_diff = next((i for i,(a,b) in enumerate(zip(ref_data, our_data)) if a != b), None)
print(f"Compressed size: ref={len(ref_data)}, ours={len(our_data)}, diffs={sum(a!=b for a,b in zip(ref_data,our_data))+abs(len(ref_data)-len(our_data))}")
print(f"First diff at compressed byte: {first_diff}")

if first_diff is None:
    print("PERFECT MATCH!")
    sys.exit(0)

# Find which opcode in ref contains first_diff
ref_idx = next((i for i,(cp,_,raw,_) in enumerate(ref_ops) if cp > first_diff), 1) - 1
our_idx = next((i for i,(cp,_,raw,_) in enumerate(our_ops) if cp > first_diff), 1) - 1

print(f"\n--- Reference opcodes around first diff (ref_idx={ref_idx}) ---")
for op in ref_ops[max(0,ref_idx-3):ref_idx+6]:
    marker = ' <-- FIRST DIFF' if op[0] <= first_diff < op[0]+len(op[2]) else ''
    print(fmt_op(*op) + marker)

print(f"\n--- Our opcodes around first diff (our_idx={our_idx}) ---")
for op in our_ops[max(0,our_idx-3):our_idx+6]:
    marker = ' <-- FIRST DIFF' if op[0] <= first_diff < op[0]+len(op[2]) else ''
    print(fmt_op(*op) + marker)

# Show decompressed data around the divergence point
ref_decomp_pos = ref_ops[ref_idx][1]
print(f"\n--- Decompressed data at dec_pos={ref_decomp_pos} ---")
chunk = dec_data[ref_decomp_pos:ref_decomp_pos+48]
print('  ' + ' '.join(f'{b:02x}' for b in chunk))
print('  ' + ' '.join(f'{b:3d}' for b in chunk))

# Also show next 10 opcode pairs side by side
print(f"\n--- Side-by-side comparison (ref vs ours) from divergence ---")
ri, oi = ref_idx, our_idx
for _ in range(12):
    r = fmt_op(*ref_ops[ri]) if ri < len(ref_ops) else '  (end)'
    o = fmt_op(*our_ops[oi]) if oi < len(our_ops) else '  (end)'
    match = '==' if ri < len(ref_ops) and oi < len(our_ops) and ref_ops[ri][2] == our_ops[oi][2] else '!!'
    print(f'REF {r}')
    print(f'OUR {o}  {match}')
    print()
    ri += 1; oi += 1
