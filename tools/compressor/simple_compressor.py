"""
simple_compressor.py — A simpler, "good enough" compressor for this format.

Goal:
    Produce valid compressed output that decompresses (via decompressor.py) to
    bytes identical to the input. Does NOT try to match the original reference
    files byte-for-byte. Instead, it focuses on:
      - Correctness (round-trips perfectly)
      - Reasonable compression ratio
      - Simple, readable greedy logic

This compressor uses a small subset of the available opcodes:
    RAW (0x00..0xEF) — literal copy of N+1 bytes
    F0               — nibble (0..15) repeated 3..18 times          (2 bytes)
    F1               — any byte repeated 4..259 times               (3 bytes)
    F3               — 2-byte pattern repeated 2..257 times         (4 bytes)
    F8               — ascending +1 run, length 4..259              (3 bytes)
    F9               — descending -1 run, length 4..259             (3 bytes)
    FC               — back-reference, offset 0..4095, length 4..19 (3 bytes)
    FD               — back-reference, offset 0..255,  length 20..275 (3 bytes)
    FF               — end of stream

The remaining opcodes (F2, F4, F5, F6, F7, FA, FB, FE) are skipped. They
provide additional compression for specific patterns, but adding them costs
complexity without proportional benefit for a "good enough" baseline.

Strategy (greedy, single pass):
    For each position i in the input:
      1. Compute the best opcode candidate at i — the one that yields the
         highest savings (advance - encoded_length).
      2. If savings > 0, emit it and skip ahead by `advance`.
      3. Otherwise, append the byte at i to a raw buffer (flushed lazily).

Back-reference matching uses a 4-gram hash table mapping each 4-byte window
to the list of positions where that window has been seen. This lets us find
candidate back-references in O(1) lookup time.
"""

from collections import defaultdict, deque


# ────────────────────────────────────────────────────────────────────────
#  Helper: count how many bytes match between two windows in `src`
#  (capped to avoid self-referential overlap, which the FC/FD opcodes do
#  not support — they read from the already-emitted output buffer).
# ────────────────────────────────────────────────────────────────────────
def count_match(src, i, match_start, max_len):
    """Count matching bytes starting at src[i] vs src[match_start]."""
    n = len(src)
    # Cap at the distance: we can't reference bytes that are still ahead of us.
    max_len = min(max_len, i - match_start, n - i)
    cnt = 0
    while cnt < max_len and src[match_start + cnt] == src[i + cnt]:
        cnt += 1
    return cnt


# ────────────────────────────────────────────────────────────────────────
#  Find the single best opcode at position i.
#  Returns (encoded_bytes, advance) or None if nothing beats raw copy.
# ────────────────────────────────────────────────────────────────────────
def find_best_op(src, i, hash_table):
    n = len(src)
    if i >= n:
        return None

    v0 = src[i]
    best_enc = None
    best_adv = 0
    best_sav = 0  # savings = advance - len(encoded)

    def consider(enc, adv):
        nonlocal best_enc, best_adv, best_sav
        sav = adv - len(enc)
        if sav > best_sav:
            best_enc, best_adv, best_sav = enc, adv, sav

    # ── F0: nibble (0..15) repeated 3..18 times → 2 bytes ──────────────
    if v0 <= 15:
        cnt = 1
        while cnt < 18 and i + cnt < n and src[i + cnt] == v0:
            cnt += 1
        if cnt >= 3:
            consider(bytes([0xF0, (v0 << 4) | (cnt - 3)]), cnt)

    # ── F1: any byte repeated 4..259 times → 3 bytes ───────────────────
    cnt = 1
    while cnt < 259 and i + cnt < n and src[i + cnt] == v0:
        cnt += 1
    if cnt >= 4:
        consider(bytes([0xF1, cnt - 4, v0]), cnt)

    # ── F3: 2-byte pattern (b0,b1) repeated 2..257 times → 4 bytes ─────
    if i + 1 < n:
        b0, b1 = src[i], src[i + 1]
        cnt = 1
        while (cnt < 257 and i + cnt * 2 + 1 < n
               and src[i + cnt * 2] == b0 and src[i + cnt * 2 + 1] == b1):
            cnt += 1
        if cnt >= 3:  # at cnt=2 sav=0; need cnt>=3 for positive savings
            consider(bytes([0xF3, cnt - 2, b0, b1]), cnt * 2)

    # ── F8: ascending +1 run, length 4..259 → 3 bytes ──────────────────
    cnt = 1
    while cnt < 259 and i + cnt < n and src[i + cnt] == (v0 + cnt) & 0xFF:
        cnt += 1
    if cnt >= 4:
        consider(bytes([0xF8, cnt - 4, v0]), cnt)

    # ── F9: descending -1 run, length 4..259 → 3 bytes ─────────────────
    cnt = 1
    while cnt < 259 and i + cnt < n and src[i + cnt] == (v0 - cnt) & 0xFF:
        cnt += 1
    if cnt >= 4:
        consider(bytes([0xF9, cnt - 4, v0]), cnt)

    # ── FC / FD: back-references via 4-gram hash table ─────────────────
    # FC: offset 0..4095 (12-bit), length 4..19 (4-bit)   → 3 bytes
    # FD: offset 0..255  (8-bit),  length 20..275 (8-bit) → 3 bytes
    if i + 4 <= n:
        key = bytes(src[i:i + 4])
        candidates = hash_table.get(key)
        if candidates:
            # Drop entries that are out of FC's reach (offset > 4095).
            # FC offset = (i - match_start - 1), so match_start < i - 4096
            # means out of range.
            while candidates and (i - candidates[-1] - 1) > 4095:
                candidates.pop()

            best_match_len = 0
            best_match_start = -1
            # Walk newest-first (smallest offset first) for speed; we just
            # want any sufficiently long match.
            for ms in reversed(candidates):
                offset = i - ms - 1
                if offset > 4095:
                    break  # FC can't reach further; FD is even more limited
                ml = count_match(src, i, ms, 275)
                if ml > best_match_len:
                    best_match_len = ml
                    best_match_start = ms
                    # Heuristic: a length-19+ match already saturates FC.
                    if ml >= 19 and offset > 255:
                        # FD wouldn't fit (offset too large), no need to keep
                        # searching for longer matches.
                        pass

            if best_match_len >= 4 and best_match_start >= 0:
                offset = i - best_match_start - 1
                # FD path: offset fits in 8 bits AND match is long
                if offset <= 255 and best_match_len >= 20:
                    fd_len = min(best_match_len, 275)
                    consider(bytes([0xFD, offset, fd_len - 0x14]), fd_len)
                # FC path: offset fits in 12 bits, match length 4..19
                if offset <= 4095:
                    fc_len = min(best_match_len, 19)
                    off_lo = offset & 0xFF
                    off_hi = (offset >> 8) & 0xF
                    consider(
                        bytes([0xFC, off_lo, ((fc_len - 4) << 4) | off_hi]),
                        fc_len,
                    )

    if best_enc is None:
        return None
    return best_enc, best_adv


# ────────────────────────────────────────────────────────────────────────
#  Main compression loop
# ────────────────────────────────────────────────────────────────────────
def compress(src):
    """Compress `src` (bytes-like) and return the compressed bytes."""
    src = bytes(src)
    n = len(src)
    out = bytearray()
    raw_buf = bytearray()
    i = 0

    # 4-gram → list of past positions where that 4-gram started.
    # We keep newest at the right end (deque) so matching can walk from the
    # closest match backwards toward older ones.
    hash_table = defaultdict(deque)

    def add_to_hash(pos):
        if pos + 4 <= n:
            hash_table[bytes(src[pos:pos + 4])].append(pos)

    def flush_raw():
        # RAW opcode 0x00..0xEF emits N+1 literal bytes, where the opcode byte
        # itself encodes (N) i.e. count-1. So a chunk of length L is encoded
        # as [L-1, byte0, byte1, ..., byteL-1]. Max L per chunk is 240.
        j = 0
        while j < len(raw_buf):
            chunk = min(240, len(raw_buf) - j)
            out.append(chunk - 1)
            out.extend(raw_buf[j:j + chunk])
            j += chunk
        raw_buf.clear()

    while i < n:
        result = find_best_op(src, i, hash_table)
        if result is not None:
            # Found a useful opcode — flush any pending raw bytes first,
            # then emit the opcode and add the consumed bytes to the hash
            # table (so future positions can back-reference them).
            flush_raw()
            enc, adv = result
            out.extend(enc)
            for j in range(i, i + adv):
                add_to_hash(j)
            i += adv
        else:
            # Nothing useful — accumulate the byte into the raw buffer.
            add_to_hash(i)
            raw_buf.append(src[i])
            i += 1

    # Flush any trailing raw bytes, then emit the end-of-stream marker.
    flush_raw()
    out.append(0xFF)
    return bytes(out)


# ────────────────────────────────────────────────────────────────────────
#  CLI entry point
# ────────────────────────────────────────────────────────────────────────
if __name__ == "__main__":
    import sys

    if len(sys.argv) != 3:
        print("Usage: python simple_compressor.py <input_file> <output_file>")
        sys.exit(1)

    with open(sys.argv[1], "rb") as f:
        data = f.read()
    compressed = compress(data)
    with open(sys.argv[2], "wb") as f:
        f.write(compressed)

    ratio = len(compressed) / len(data) if data else 0.0
    print(f"Compressed {len(data):,} bytes -> {len(compressed):,} bytes "
          f"(ratio: {ratio:.3f})")
