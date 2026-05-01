"""
Enumerate ALL F5 opcodes in reference files, compute what our code would produce,
and look for patterns in how the reference chooses its cnt.
"""
import sys, os
sys.path.insert(0, 'd:/tmp')

def decode_opcodes(data):
    ops = []
    src_ptr = 0
    decomp_pos = 0
    src = data
    n = len(src)
    while src_ptr < n:
        op_start = src_ptr
        opcode = src[src_ptr]
        if opcode == 0xFF:
            ops.append((op_start, decomp_pos, 0xFF, bytes([0xFF]), 0))
            break
        elif opcode == 0xF5:
            cnt = src[src_ptr+1] + 4
            ops.append((op_start, decomp_pos, 0xF5, bytes(src[src_ptr:src_ptr+3+cnt]), cnt*2))
            src_ptr += 3 + cnt; decomp_pos += cnt * 2
        elif opcode == 0xF0:
            cnt = (src[src_ptr+1] & 0xF) + 3
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+2]), cnt))
            src_ptr += 2; decomp_pos += cnt
        elif opcode == 0xF1:
            cnt = src[src_ptr+1] + 4
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+3]), cnt))
            src_ptr += 3; decomp_pos += cnt
        elif opcode == 0xF2:
            cnt = (src[src_ptr+1] + 2) * 2
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+3]), cnt))
            src_ptr += 3; decomp_pos += cnt
        elif opcode == 0xF3:
            cnt = (src[src_ptr+1] + 2) * 2
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+4]), cnt))
            src_ptr += 4; decomp_pos += cnt
        elif opcode == 0xF4:
            cnt = (src[src_ptr+1] + 2) * 3
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+5]), cnt))
            src_ptr += 5; decomp_pos += cnt
        elif opcode == 0xF6:
            c = src[src_ptr+1] + 3
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+4+c]), c*3))
            src_ptr += 4 + c; decomp_pos += c*3
        elif opcode == 0xF7:
            cnt = src[src_ptr+1] + 2
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+5+cnt]), cnt*4))
            src_ptr += 5 + cnt; decomp_pos += cnt*4
        elif opcode == 0xF8:
            cnt = src[src_ptr+1] + 4
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+3]), cnt))
            src_ptr += 3; decomp_pos += cnt
        elif opcode == 0xF9:
            cnt = src[src_ptr+1] + 4
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+3]), cnt))
            src_ptr += 3; decomp_pos += cnt
        elif opcode == 0xFA:
            cnt = src[src_ptr+1] + 5
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+4]), cnt))
            src_ptr += 4; decomp_pos += cnt
        elif opcode == 0xFB:
            cnt = (src[src_ptr+1] + 3) * 2
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+5]), cnt))
            src_ptr += 5; decomp_pos += cnt
        elif opcode == 0xFC:
            cnt = (src[src_ptr+2] >> 4) + 4
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+3]), cnt))
            src_ptr += 3; decomp_pos += cnt
        elif opcode == 0xFD:
            cnt = src[src_ptr+2] + 0x14
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+3]), cnt))
            src_ptr += 3; decomp_pos += cnt
        elif opcode == 0xFE:
            cnt = (src[src_ptr+1] & 0xF) + 3
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+2]), cnt))
            src_ptr += 2; decomp_pos += cnt
        else:
            cnt = opcode + 1
            ops.append((op_start, decomp_pos, opcode, bytes(src[src_ptr:src_ptr+1+cnt]), cnt))
            src_ptr += 1 + cnt; decomp_pos += cnt
    return ops

def compute_max_f5(dec, pos):
    """At dec[pos], compute cnt_q and max cnt for F5."""
    n = len(dec)
    if pos + 3 >= n:
        return 0, 0
    fixed = dec[pos]
    cnt_q = 0
    while cnt_q < 259 and pos + cnt_q*2+1 < n and dec[pos+cnt_q*2] == fixed and dec[pos+cnt_q*2+1] != fixed:
        cnt_q += 1
    if cnt_q < 1:
        return 0, 0
    cnt = cnt_q
    while cnt < 259 and pos + cnt*2+1 < n and dec[pos+cnt*2] == fixed:
        cnt += 1
    return cnt_q, cnt

files = ['ZUKAN','CHECKPS','CLOAD','SHOP','GNAME','GOLEM','ADDHERO','MENU']

for name in files:
    ref_path = f'd:/tmp/compressed/{name}.BIN'
    dec_path = f'd:/tmp/decompressed/{name}.BIN.decompressed'
    if not os.path.exists(ref_path) or not os.path.exists(dec_path):
        continue

    ref_data = open(ref_path,'rb').read()
    dec_data = open(dec_path,'rb').read()
    ops = decode_opcodes(ref_data)

    f5_ops = [(i,op) for i,op in enumerate(ops) if op[2] == 0xF5]
    print(f"\n=== {name}: {len(f5_ops)} F5 opcodes ===")
    print(f"{'dec_pos':>8} {'ref_cnt':>7} {'cnt_q':>5} {'max_cnt':>7} {'deg':>4} {'ratio':>6}  next_op  notes")

    # Show first 30 and any interesting ones
    shown = 0
    prev_dec_end = -1
    for idx, (oi, op) in enumerate(f5_ops[:60]):
        cmp_pos, dec_pos, _, raw, adv = op
        ref_cnt = len(raw) - 3  # F5 raw = [F5, cnt-4, fixed, ...varying]
        fixed = raw[2]
        cnt_q, max_cnt = compute_max_f5(dec_data, dec_pos)
        deg = max_cnt - cnt_q  # degenerate pairs extended

        # What follows in reference?
        next_op_code = ops[oi+1][2] if oi+1 < len(ops) else 0xFF
        next_op_name = {0xF5:'F5',0xF0:'F0',0xF1:'F1',0xF2:'F2',0xF3:'F3',
                        0xF4:'F4',0xF7:'F7',0xF8:'F8',0xF9:'F9',0xFA:'FA',
                        0xFB:'FB',0xFC:'FC',0xFD:'FD',0xFE:'FE',0xFF:'FF'}.get(next_op_code, f'{next_op_code:02x}')

        consecutive = (dec_pos == prev_dec_end)  # is this a continuation of prev F5?
        notes = 'CONSEC' if consecutive else ''
        if ref_cnt < max_cnt:
            notes += f' SHORT({ref_cnt}<{max_cnt})'
        if ref_cnt == cnt_q:
            notes += ' =cnt_q'
        elif ref_cnt < cnt_q:
            notes += ' <cnt_q!'

        print(f"{dec_pos:8d} {ref_cnt:7d} {cnt_q:5d} {max_cnt:7d} {deg:4d} {ref_cnt/max_cnt:6.2f}  {next_op_name:6s}  {notes}")
        prev_dec_end = dec_pos + adv
        shown += 1
