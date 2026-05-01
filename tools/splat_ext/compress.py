import sys


def _run_length(src: bytes, i: int, n: int, max_run: int) -> int:
    b = src[i]
    run = 1
    while i + run < n and src[i + run] == b and run < max_run:
        run += 1
    return run


def _find_best_match(src: bytes, i: int, n: int, max_off: int, max_len: int) -> tuple[int, int]:
    """Find longest back-reference at position i within constraints.

    Returns (match_length, offset) where offset = i - start - 1,
    matching the decompressor's: temp_ptr = len(dst) - offset,
    first read = dst[temp_ptr - 1] = dst[i - offset - 1] = src[start].
    """
    best_len, best_off = 0, 0
    search_lo = max(0, i - max_off - 1)
    for start in range(i - 1, search_lo - 1, -1):
        off = i - start - 1
        if off > max_off:
            break
        cap = min(max_len, n - i)
        mlen = 0
        while mlen < cap and src[start + mlen] == src[i + mlen]:
            mlen += 1
        if mlen > best_len:
            best_len, best_off = mlen, off
            if best_len == max_len:
                break
        elif mlen == best_len and mlen > 0:
            # When lengths are equal, prefer the larger offset (older/furthest match)
            best_off = off
    return best_len, best_off


def _savings(consumed: int, encoded_len: int) -> int:
    """Net bytes saved vs encoding consumed bytes as a literal run (cost = consumed + 1)."""
    return (consumed + 1) - encoded_len


def _savings_ratio(consumed: int, encoded_len: int) -> float:
    """Savings per encoded byte: higher means more efficient encoding."""
    s = (consumed + 1) - encoded_len
    if s <= 0 or encoded_len == 0:
        return 0.0
    return s / encoded_len


# ── Opcode encoders ────────────────────────────────────────────────────────────
# Each returns (bytes_consumed, encoded_bytes) or None if the opcode doesn't apply.

def _try_f0(src, i, n):
    # F0 [packed]: repeat upper-nibble value (0-15) for (lower nibble + 3) times. 2 bytes.
    b = src[i]
    if b > 0x0F:
        return None
    run = _run_length(src, i, n, 18)
    if run < 3:
        return None
    return run, bytes([0xF0, (b << 4) | (run - 3)])


def _try_f1(src, i, n):
    # F1 [count] [value]: repeat any byte for (count + 4) times. 3 bytes.
    run = _run_length(src, i, n, 259)
    if run < 4:
        return None
    return run, bytes([0xF1, run - 4, src[i]])


def _try_f2(src, i, n):
    # F2 [count] [packed]: alternate lo/hi nibbles as 2-byte pairs (count+2 pairs). 3 bytes.
    # Both bytes must fit in 4 bits (0x00-0x0F).
    if i + 1 >= n:
        return None
    b0, b1 = src[i], src[i + 1]
    if b0 > 0x0F or b1 > 0x0F:
        return None
    count = 0
    pos = i
    while pos + 1 < n and src[pos] == b0 and src[pos + 1] == b1 and count < 257:
        count += 1
        pos += 2
    if count < 2:
        return None
    return count * 2, bytes([0xF2, count - 2, (b1 << 4) | b0])


def _try_f3(src, i, n):
    # F3 [count] [b0] [b1]: repeat 2-byte pattern (count+2) times. 4 bytes.
    if i + 1 >= n:
        return None
    b0, b1 = src[i], src[i + 1]
    count = 0
    pos = i
    while pos + 1 < n and src[pos] == b0 and src[pos + 1] == b1 and count < 257:
        count += 1
        pos += 2
    if count < 2:
        return None
    return count * 2, bytes([0xF3, count - 2, b0, b1])


def _try_f4(src, i, n):
    # F4 [count] [b0] [b1] [b2]: repeat 3-byte pattern (count+2) times. 5 bytes.
    if i + 2 >= n:
        return None
    b0, b1, b2 = src[i], src[i + 1], src[i + 2]
    count = 0
    pos = i
    while pos + 2 < n and src[pos] == b0 and src[pos + 1] == b1 and src[pos + 2] == b2 and count < 257:
        count += 1
        pos += 3
    if count < 2:
        return None
    return count * 3, bytes([0xF4, count - 2, b0, b1, b2])


def _try_f5(src, i, n):
    # F5 [count] [fixed] + stream: write {fixed, next_src_byte} pairs (count+4) times.
    # Fixed byte repeats at every even position; varying bytes are embedded in stream.
    if i + 1 >= n:
        return None
    fixed = src[i]
    count = 0
    pos = i
    while pos + 1 < n and src[pos] == fixed and count < 259:
        count += 1
        pos += 2
    if count < 4:
        return None
    varying = bytes(src[i + 1 + j * 2] for j in range(count))
    return count * 2, bytes([0xF5, count - 4, fixed]) + varying


def _try_f6(src, i, n):
    # F6 [count] [b0] [b1] + stream: write {b0, b1, next_src_byte} triplets (count+3) times.
    if i + 2 >= n:
        return None
    b0, b1 = src[i], src[i + 1]
    count = 0
    pos = i
    while pos + 2 < n and src[pos] == b0 and src[pos + 1] == b1 and count < 258:
        count += 1
        pos += 3
    if count < 3:
        return None
    varying = bytes(src[i + 2 + j * 3] for j in range(count))
    return count * 3, bytes([0xF6, count - 3, b0, b1]) + varying


def _try_f7(src, i, n):
    # F7 [count] [b0] [b1] [b2] + stream: write {b0, b1, b2, next_src_byte} quads (count+2) times.
    if i + 3 >= n:
        return None
    b0, b1, b2 = src[i], src[i + 1], src[i + 2]
    count = 0
    pos = i
    while pos + 3 < n and src[pos] == b0 and src[pos + 1] == b1 and src[pos + 2] == b2 and count < 257:
        count += 1
        pos += 4
    if count < 2:
        return None
    varying = bytes(src[i + 3 + j * 4] for j in range(count))
    return count * 4, bytes([0xF7, count - 2, b0, b1, b2]) + varying


def _try_f8(src, i, n):
    # F8 [count] [start]: ascending arithmetic run (count+4 bytes). 3 bytes.
    run = 1
    while i + run < n and src[i + run] == (src[i + run - 1] + 1) & 0xFF and run < 259:
        run += 1
    if run < 4:
        return None
    return run, bytes([0xF8, run - 4, src[i]])


def _try_f9(src, i, n):
    # F9 [count] [start]: descending arithmetic run (count+4 bytes). 3 bytes.
    run = 1
    while i + run < n and src[i + run] == (src[i + run - 1] - 1) & 0xFF and run < 259:
        run += 1
    if run < 4:
        return None
    return run, bytes([0xF9, run - 4, src[i]])


def _try_fa(src, i, n):
    # FA [count] [start] [step]: arithmetic run with step (count+5 bytes). 4 bytes.
    # step=0 handled by F1, step=1 by F8, step=255 by F9 (all cheaper).
    if i + 1 >= n:
        return None
    start = src[i]
    step = (src[i + 1] - start) & 0xFF
    if step == 0 or step == 1 or step == 255:
        return None
    run = 1
    val = start
    while i + run < n and run < 260:
        val = (val + step) & 0xFF
        if src[i + run] != val:
            break
        run += 1
    if run < 5:
        return None
    return run, bytes([0xFA, run - 5, start, step])


def _try_fb(src, i, n):
    # FB [count] [lo] [hi] [delta]: 16-bit pair run, each pair incremented by signed delta.
    # iterations = count + 3, so minimum 3 pairs (6 bytes consumed). 5 bytes encoded.
    if i + 3 >= n:
        return None
    lo, hi = src[i], src[i + 1]
    lo2, hi2 = src[i + 2], src[i + 3]
    val = lo | (hi << 8)
    val2 = lo2 | (hi2 << 8)
    delta = (val2 - val) & 0xFFFF
    # delta must fit in signed 8 bits
    if delta > 127 and delta < 0xFF80:
        return None
    s8_delta = delta if delta <= 127 else delta - 256
    run = 1  # number of pairs already matched (the initial lo, hi)
    v = val
    pos = i + 2
    while pos + 1 < n and run < 258:
        v_next = (v + s8_delta) & 0xFFFF
        if src[pos] != (v_next & 0xFF) or src[pos + 1] != (v_next >> 8):
            break
        v = v_next
        run += 1
        pos += 2
    if run < 3:
        return None
    return run * 2, bytes([0xFB, run - 3, lo, hi, delta & 0xFF])


def _try_fc(src, i, n):
    # FC [offLo] [offHi_cnt]: back-reference, 12-bit offset, count = upper nibble + 4. 3 bytes.
    best_len, best_off = _find_best_match(src, i, n, max_off=4095, max_len=19)
    if best_len < 4:
        return None
    count_nib = best_len - 4
    return best_len, bytes([0xFC, best_off & 0xFF, (count_nib << 4) | ((best_off >> 8) & 0xF)])


def _try_fd(src, i, n):
    # FD [offset] [count]: back-reference, 8-bit offset, count + 0x14 bytes. 3 bytes.
    best_len, best_off = _find_best_match(src, i, n, max_off=255, max_len=275)
    if best_len < 20:
        return None
    return best_len, bytes([0xFD, best_off, best_len - 0x14])


def _try_fe(src, i, n):
    # FE [packed]: compact back-reference. 2 bytes.
    # offset (internal) = upper_nib * 8; reads from dst[i - offset - 8] forward.
    # Equivalent FC offset = upper_nib * 8 + 7; count 3-18.
    # Cheaper than FC (2 vs 3 bytes) for matches at these specific offsets.
    best = None
    for upper_nib in range(16):
        # effective start position = i - (upper_nib * 8) - 8
        eff_back = upper_nib * 8 + 8
        if eff_back > i:
            continue
        start = i - eff_back
        mlen = 0
        while mlen < 18 and i + mlen < n and src[start + mlen] == src[i + mlen]:
            mlen += 1
        if mlen >= 3 and (best is None or mlen >= best[0]):
            best = (mlen, upper_nib)
    if best is None:
        return None
    mlen, upper_nib = best
    return mlen, bytes([0xFE, (upper_nib << 4) | (mlen - 3)])


_ALL_TRIES = (
    _try_f0, _try_f1, _try_f2, _try_f3, _try_f4,
    _try_f5, _try_f6, _try_f7, _try_f8, _try_f9,
    _try_fa, _try_fb, _try_fc, _try_fd, _try_fe,
)


def _best_opcode(src, i, n):
    """Try all opcodes; return (consumed, encoded) for the highest-savings option, or None.

    Primary metric: savings per encoded byte (higher = more efficient).
    Tie-break: first match in _ALL_TRIES order wins (preserves original compressor behaviour).
    """
    best_spe = 0.0
    best_consumed = 0
    best_encoded = None
    for try_fn in _ALL_TRIES:
        result = try_fn(src, i, n)
        if result is None:
            continue
        consumed, encoded = result
        s = _savings(consumed, len(encoded))
        if s <= 0:
            continue
        spe = _savings_ratio(consumed, len(encoded))
        if spe > best_spe:
            best_spe = spe
            best_consumed = consumed
            best_encoded = encoded
    return (best_consumed, best_encoded) if best_encoded is not None else None


def compress(src: bytes) -> bytearray:
    dst = bytearray()
    i = 0
    n = len(src)

    while i < n:
        result = _best_opcode(src, i, n)
        if result is not None:
            consumed, encoded = result
            dst.extend(encoded)
            i += consumed
            continue

        # Literal run: collect up to 240 bytes until a compressible position is found.
        lit_start = i
        i += 1
        while i < n and i - lit_start < 240:
            if _best_opcode(src, i, n) is not None:
                break
            i += 1
        dst.append(i - lit_start - 1)
        dst.extend(src[lit_start:i])

    dst.append(0xFF)
    return dst


if __name__ == "__main__":
    if len(sys.argv) not in (3, 4):
        print("Usage: compress.py <input> <output> [prefix_byte_hex]")
        print("  prefix_byte_hex  optional byte prepended before compressed data (e.g. 0x01)")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2]
    prefix = bytes([int(sys.argv[3], 0)]) if len(sys.argv) == 4 else b""

    with open(input_path, "rb") as f:
        raw = f.read()

    compressed = compress(raw)

    with open(output_path, "wb") as f:
        f.write(prefix + compressed)

    print(f"compressed {len(raw)} → {len(prefix) + len(compressed)} bytes")
