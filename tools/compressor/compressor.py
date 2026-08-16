import sys
from collections import defaultdict, deque


def compress(src):
    src = bytes(src)
    n = len(src)
    out = bytearray()
    i = 0
    raw_buf = bytearray()

    # Hash table: 4-gram -> deque of past positions (oldest first = largest offset first).
    hash_table = defaultdict(deque)

    def add_to_hash(pos):
        if pos + 4 <= n:
            hash_table[src[pos:pos + 4]].append(pos)

    def flush_raw():
        j = 0
        while j < len(raw_buf):
            chunk_len = min(240, len(raw_buf) - j)
            out.append(chunk_len - 1)
            out.extend(raw_buf[j:j + chunk_len])
            j += chunk_len
        raw_buf.clear()

    while i < n:
        result = find_best(src, i, n, hash_table, len(raw_buf))
        if result is not None:
            flush_raw()
            encoded, advance = result
            out.extend(encoded)
            for j in range(i, i + advance):
                add_to_hash(j)
            i += advance
        else:
            add_to_hash(i)
            raw_buf.append(src[i])
            i += 1
            if len(raw_buf) == 240:
                flush_raw()

    flush_raw()
    out.append(0xFF)
    return bytes(out)


def count_backref_match(src, i, match_start, max_count):
    """Count matching bytes for a non-overlapping back-reference (no self-referential copies)."""
    n = len(src)
    dist = i - match_start
    max_count = min(max_count, dist)  # no self-referential copies
    cnt = 0
    while cnt < max_count and i + cnt < n and src[match_start + cnt] == src[i + cnt]:
        cnt += 1
    return cnt


def f5_cnt_qualifies(src, i, fixed, cnt_q, cnt):
    """Return True if F5 qualifies based on the strict prefix/degenerate count."""
    if cnt_q >= 4:
        return True
    if cnt_q < 1:
        return False
    deg = sum(src[i + k * 2 + 1] == fixed for k in range(cnt_q, cnt))
    # The short-prefix forms are sharply constrained in the reference core:
    # one strict pair is followed by exactly two degenerate pairs; two/three
    # strict pairs allow exactly one.  This rejects long accidental F5s.
    if cnt_q == 1:
        if deg != 2:
            return False
        # For the minimum four-pair form, a degenerate final varying byte is
        # not taken as F5 in the reference core.
        if cnt == 4 and src[i + (cnt - 1) * 2 + 1] == fixed:
            return False
        return True
    return deg == 1


def fb_qualifies_at(src, pos, n):
    """Returns True if FB opcode would qualify starting at position pos (cnt >= 3)."""
    if pos + 3 >= n:
        return False
    v16_0 = (src[pos + 1] << 8) | src[pos]
    v16_1 = (src[pos + 3] << 8) | src[pos + 2]
    raw_delta = (v16_1 - v16_0) & 0xFFFF
    if raw_delta > 32767:
        raw_delta -= 65536
    if not (-128 <= raw_delta <= 127):
        return False
    v16 = v16_0
    cnt = 0
    while cnt < 258 and pos + cnt * 2 + 1 < n:
        if src[pos + cnt * 2] != v16 & 0xFF or src[pos + cnt * 2 + 1] != (v16 >> 8) & 0xFF:
            break
        v16 = (v16 + raw_delta) & 0xFFFF
        cnt += 1
    return cnt >= 3


def best_pattern_savings(src, i, n):
    """Compute the best single-step pattern savings at position i (no back-refs)."""
    if i >= n:
        return 0
    v0 = src[i]
    best = 0

    def chk(enc_len, adv):
        nonlocal best
        sav = adv - enc_len
        if sav > best:
            best = sav

    if v0 <= 15:
        cnt = 1
        while cnt < 18 and i + cnt < n and src[i + cnt] == v0:
            cnt += 1
        if cnt >= 3:
            chk(2, cnt)

    cnt = 1
    while cnt < 259 and i + cnt < n and src[i + cnt] == v0:
        cnt += 1
    if cnt >= 4:
        chk(3, cnt)

    if i + 1 < n:
        b0, b1 = src[i], src[i + 1]
        if b0 <= 15 and b1 <= 15:
            cnt = 1
            while cnt < 257 and i + cnt * 2 + 1 < n and src[i + cnt * 2] == b0 and src[i + cnt * 2 + 1] == b1:
                cnt += 1
            if cnt >= 2:
                chk(3, cnt * 2)

    if i + 1 < n:
        b0, b1 = src[i], src[i + 1]
        cnt = 1
        while cnt < 257 and i + cnt * 2 + 1 < n and src[i + cnt * 2] == b0 and src[i + cnt * 2 + 1] == b1:
            cnt += 1
        if cnt >= 3:
            chk(4, cnt * 2)

    if i + 2 < n:
        b0, b1, b2 = src[i], src[i + 1], src[i + 2]
        cnt = 1
        while cnt < 257 and i + cnt * 3 + 2 < n and src[i + cnt * 3] == b0 and src[i + cnt * 3 + 1] == b1 and src[i + cnt * 3 + 2] == b2:
            cnt += 1
        if cnt >= 2:
            chk(5, cnt * 3)

    if i + 1 < n:
        fixed = v0
        # Extend pair-by-pair; stop right after a pair whose varying byte
        # (odd alignment) starts a qualifying FB run - the reference
        # compressor yields to FB there.
        cnt = 0
        while cnt < 259 and i + cnt * 2 + 1 < n and src[i + cnt * 2] == fixed:
            cnt += 1
            if fb_qualifies_at(src, i + cnt * 2 - 1, n):
                break
        cnt_q = 0
        while cnt_q < cnt and src[i + cnt_q * 2 + 1] != fixed:
            cnt_q += 1
        if cnt >= 4 and cnt_q >= 1 and f5_cnt_qualifies(src, i, fixed, cnt_q, cnt):
            chk(3 + cnt, cnt * 2)

    if i + 2 < n:
        b0, b1 = src[i], src[i + 1]
        cnt = 1
        while (not (b0 == 0 and b1 == 0) and cnt < 258 and i + cnt * 3 + 2 < n
               and src[i + cnt * 3] == b0
               and src[i + cnt * 3 + 1] == b1):
            cnt += 1
        if cnt >= 3:
            deg = sum(src[i + k * 3 + 2] == b0 for k in range(cnt)) if b0 == b1 else 0
            if not (b0 == b1 and deg >= 3):
                chk(4 + cnt, cnt * 3)

    if n - i >= 1028:
        b0, b1, b2 = src[i], src[i + 1], src[i + 2]
        if not (b0 == b1 == b2 == 0):
            cnt = 1
            while (cnt < 257 and i + cnt * 4 + 3 < n
                   and src[i + cnt * 4] == b0
                   and src[i + cnt * 4 + 1] == b1
                   and src[i + cnt * 4 + 2] == b2):
                cnt += 1
            if cnt >= 2:
                chk(5 + cnt, cnt * 4)

    cnt = 1
    while cnt < 259 and i + cnt < n and src[i + cnt] == (v0 + cnt) & 0xFF:
        cnt += 1
    if cnt >= 4:
        chk(3, cnt)

    cnt = 1
    while cnt < 259 and i + cnt < n and src[i + cnt] == (v0 - cnt) & 0xFF:
        cnt += 1
    if cnt >= 4:
        chk(3, cnt)

    if i + 1 < n:
        step = (src[i + 1] - v0) & 0xFF
        cnt = 1
        while cnt < 260 and i + cnt < n and src[i + cnt] == (v0 + step * cnt) & 0xFF:
            cnt += 1
        if cnt >= 5:
            chk(4, cnt)

    if i + 3 < n:
        v16_0 = (src[i + 1] << 8) | src[i]
        v16_1 = (src[i + 3] << 8) | src[i + 2]
        raw_delta = (v16_1 - v16_0) & 0xFFFF
        if raw_delta > 32767:
            raw_delta -= 65536
        if -128 <= raw_delta <= 127:
            v16 = v16_0
            cnt = 0
            while cnt < 258 and i + cnt * 2 + 1 < n:
                if src[i + cnt * 2] != v16 & 0xFF or src[i + cnt * 2 + 1] != (v16 >> 8) & 0xFF:
                    break
                v16 = (v16 + raw_delta) & 0xFFFF
                cnt += 1
            if cnt >= 3:
                chk(5, cnt * 2)

    return best


def find_best(src, i, n, hash_table, raw_len=0):
    """
    Select the best encoding opcode:
    - Among pattern opcodes (F0-FB): pick by max 1-level lookahead total
      (savings + best_next_pattern_savings); first found wins ties.
    - Among back-references (FE/FD/FC): pick by max efficiency (savings/advance).
    - Compare best pattern vs best back-ref by raw efficiency (savings/advance);
      back-ref wins only if strictly higher efficiency; patterns win ties.
    Returns None if no opcode improves on raw copy.
    """
    v0 = src[i]

    # ──────────────────────────────────────────────────────────────────
    # Pattern opcodes F0–FB (collected; best chosen by lookahead below)
    # ──────────────────────────────────────────────────────────────────
    candidates = []  # list of (enc, adv, sav)

    def pat(enc, adv, cnt):
        # sav == 0 qualifies: the reference emits zero-savings F3 (cnt=2)
        # rather than falling back to raw (GNAME dec 30209/32766/34869).
        sav = adv - len(enc)
        if sav >= 0:
            candidates.append((bytearray(enc), adv, sav, cnt))

    p_enc = None
    p_sav = 0
    p_adv = 0

    # F0: nibble (0-15) repeated 3-18 times → 2 bytes
    if v0 <= 15:
        cnt = 1
        while cnt < 18 and i + cnt < n and src[i + cnt] == v0:
            cnt += 1
        if cnt >= 3:
            pat([0xF0, (v0 << 4) | (cnt - 3)], cnt, cnt)

    # F1: any byte repeated 4-259 times → 3 bytes
    cnt = 1
    while cnt < 259 and i + cnt < n and src[i + cnt] == v0:
        cnt += 1
    if cnt >= 4:
        pat([0xF1, cnt - 4, v0], cnt, cnt)

    # F2: nibble pair (both 0-15) repeated 2-257 times → 3 bytes
    if i + 1 < n:
        b0, b1 = src[i], src[i + 1]
        if b0 <= 15 and b1 <= 15:
            cnt = 1
            while (cnt < 257 and i + cnt * 2 + 1 < n
                   and src[i + cnt * 2] == b0 and src[i + cnt * 2 + 1] == b1):
                cnt += 1
            if cnt >= 2:
                pat([0xF2, cnt - 2, (b1 << 4) | b0], cnt * 2, cnt)

    # F3: 2-byte pattern repeated 2-257 times → 4 bytes
    if i + 1 < n:
        b0, b1 = src[i], src[i + 1]
        cnt = 1
        while (cnt < 257 and i + cnt * 2 + 1 < n
               and src[i + cnt * 2] == b0 and src[i + cnt * 2 + 1] == b1):
            cnt += 1
        if cnt >= 2:
            pat([0xF3, cnt - 2, b0, b1], cnt * 2, cnt)

    # F4: 3-byte pattern repeated 2-257 times → 5 bytes
    if i + 2 < n:
        b0, b1, b2 = src[i], src[i + 1], src[i + 2]
        cnt = 1
        while (cnt < 257 and i + cnt * 3 + 2 < n
               and src[i + cnt * 3] == b0
               and src[i + cnt * 3 + 1] == b1
               and src[i + cnt * 3 + 2] == b2):
            cnt += 1
        if cnt >= 2:
            pat([0xF4, cnt - 2, b0, b1, b2], cnt * 3, cnt)

    # F5: {fixed, var} pairs, 4-259 pairs → 3+cnt bytes
    # Extension stops right after a pair whose varying byte (odd alignment)
    # starts a qualifying FB run - the reference compressor yields to FB
    # there (confirmed at GNAME dec 12042/12060/12324/12591). The pair-0
    # case reproduces the old "FB at i+1 suppresses F5" rule (cnt=1 < 4),
    # and the truncation explains the CLOAD 24240 F5(4)+F5(13) split: the
    # scan resumes at a pair boundary where the odd-aligned FB no longer
    # applies, so another F5 fires.
    if i + 1 < n:
        fixed = v0
        cnt = 0
        while cnt < 259 and i + cnt * 2 + 1 < n and src[i + cnt * 2] == fixed:
            cnt += 1
            if fb_qualifies_at(src, i + cnt * 2 - 1, n):
                break
        cnt_q = 0
        while cnt_q < cnt and src[i + cnt_q * 2 + 1] != fixed:
            cnt_q += 1
        if cnt >= 4 and cnt_q >= 1 and f5_cnt_qualifies(src, i, fixed, cnt_q, cnt):
            varying = bytes(src[i + k * 2 + 1] for k in range(cnt))
            pat(bytes([0xF5, cnt - 4, fixed]) + varying, cnt * 2, cnt)

    # F6: {b0, b1, var} triples, 3-258 triples, 4+cnt enc bytes
    # (confirmed in reference output at GNAME dec 18247)
    if i + 2 < n:
        b0, b1 = src[i], src[i + 1]
        cnt = 1
        while (not (b0 == 0 and b1 == 0) and cnt < 258 and i + cnt * 3 + 2 < n
               and src[i + cnt * 3] == b0
               and src[i + cnt * 3 + 1] == b1):
            cnt += 1
        if cnt >= 3:
            varying = bytes(src[i + k * 3 + 2] for k in range(cnt))
            if not (b0 == b1 and sum(v == b0 for v in varying) >= 3):
                pat(bytes([0xF6, cnt - 3, b0, b1]) + varying, cnt * 3, cnt)

    # F7: {b0, b1, b2, var} quads, 2-257 quads → 5+cnt bytes
    # Not used when b0==b1==b2 (degenerate case handled by other opcodes).
    if n - i >= 1028:
        b0, b1, b2 = src[i], src[i + 1], src[i + 2]
        if not (b0 == b1 == b2 == 0):
            cnt = 1
            while (cnt < 257 and i + cnt * 4 + 3 < n
                   and src[i + cnt * 4] == b0
                   and src[i + cnt * 4 + 1] == b1
                   and src[i + cnt * 4 + 2] == b2):
                cnt += 1
            if cnt >= 2:
                varying = bytes(src[i + k * 4 + 3] for k in range(cnt))
                pat(bytes([0xF7, cnt - 2, b0, b1, b2]) + varying, cnt * 4, cnt)

    # F8: ascending +1 run, 4-259 bytes → 3 bytes
    cnt = 1
    while cnt < 259 and i + cnt < n and src[i + cnt] == (v0 + cnt) & 0xFF:
        cnt += 1
    if cnt >= 4:
        pat([0xF8, cnt - 4, v0], cnt, cnt)

    # F9: descending -1 run, 4-259 bytes → 3 bytes
    cnt = 1
    while cnt < 259 and i + cnt < n and src[i + cnt] == (v0 - cnt) & 0xFF:
        cnt += 1
    if cnt >= 4:
        pat([0xF9, cnt - 4, v0], cnt, cnt)

    # FA: arithmetic run with step, 5-260 bytes → 4 bytes
    if i + 1 < n:
        step = (src[i + 1] - v0) & 0xFF
        cnt = 1
        while cnt < 260 and i + cnt < n and src[i + cnt] == (v0 + step * cnt) & 0xFF:
            cnt += 1
        if cnt >= 5:
            pat([0xFA, cnt - 5, v0, step], cnt, cnt)

    # FB: 16-bit pair run with signed delta, 3-258 pairs → 5 bytes
    if i + 3 < n:
        v16_0 = (src[i + 1] << 8) | src[i]
        v16_1 = (src[i + 3] << 8) | src[i + 2]
        raw_delta = (v16_1 - v16_0) & 0xFFFF
        if raw_delta > 32767:
            raw_delta -= 65536
        if -128 <= raw_delta <= 127:
            v16 = v16_0
            cnt = 0
            while cnt < 258 and i + cnt * 2 + 1 < n:
                if src[i + cnt * 2] != v16 & 0xFF or src[i + cnt * 2 + 1] != (v16 >> 8) & 0xFF:
                    break
                v16 = (v16 + raw_delta) & 0xFFFF
                cnt += 1
            if cnt >= 3:
                pat([0xFB, cnt - 3, src[i], src[i + 1], raw_delta & 0xFF], cnt * 2, cnt)

    # ──────────────────────────────────────────────────────────────────
    # Back-reference opcodes FE / FD / FC
    # Pick best by max savings, then max advance, then largest offset.
    # ──────────────────────────────────────────────────────────────────
    b_enc = None; b_sav = 0; b_adv = 0; b_ms = -1; b_same_parity = False

    def bref(enc, adv, match_start):
        nonlocal b_enc, b_sav, b_adv, b_ms, b_same_parity
        sav = adv - len(enc)
        dist = i - match_start
        # Equal-quality backrefs prefer a match start with the same byte
        # parity as the current source position.  This reproduces all 2,798
        # multi-candidate pure-FC decisions in the reference streams; within
        # the preferred parity, the oldest/largest-distance match wins.
        same_parity = ((match_start ^ i) & 1) == 0
        if (sav > b_sav
                or (sav == b_sav and same_parity and not b_same_parity)
                or (sav == b_sav and same_parity == b_same_parity and dist > b_ms)
                or (sav == b_sav and same_parity == b_same_parity and dist == b_ms and adv > b_adv)):
            b_enc, b_sav, b_adv, b_ms, b_same_parity = bytearray(enc), sav, adv, dist, same_parity

    # FE ---------------------------------------------------------------
    for upper_nibble in range(15, -1, -1):  # largest distance first for tie-break
        dist = (upper_nibble + 1) * 8
        if dist > i:
            continue
        match_start = i - dist
        if src[match_start] != v0:
            continue
        mc = count_backref_match(src, i, match_start, 18)
        if mc < 3:
            continue
        fe_cnt = min(mc, 18)
        packed = (upper_nibble << 4) | (fe_cnt - 3)
        bref([0xFE, packed], fe_cnt, match_start)

    # FD + FC ----------------------------------------------------------
    if i >= 1 and i + 4 <= n:
        key = src[i:i + 4]
        hash_candidates = hash_table.get(key)
        if hash_candidates:
            while hash_candidates and i - hash_candidates[0] - 1 > 4095:
                hash_candidates.popleft()
            for match_start in hash_candidates:  # oldest first = largest offset first for tie-break
                offset = i - match_start - 1
                mc = count_backref_match(src, i, match_start, 275)
                if mc == 0:
                    continue
                if offset <= 255 and mc >= 20:
                    fd_cnt = min(mc, 275)
                    bref([0xFD, offset, fd_cnt - 0x14], fd_cnt, match_start)
                if mc >= 4:
                    fc_cnt = min(mc, 19)
                    off_lo = offset & 0xFF
                    off_hi = (offset >> 8) & 0xF
                    bref([0xFC, off_lo, ((fc_cnt - 4) << 4) | off_hi], fc_cnt, match_start)

    # When F5 is viable, the reference compressor prefers it over F7 even
    # when F7 yields more raw savings (confirmed at MENU dec=1115). Drop F7
    # candidates whenever F5 is available.
    has_f5 = any(c[0][0] == 0xF5 for c in candidates)
    if has_f5:
        candidates = [c for c in candidates if c[0][0] != 0xF7]

    # F3 has priority over the variable-stream families F5/F6/F7 whenever
    # it is itself eligible.  A zero-savings F3 is eligible only at a clean
    # token boundary (raw_len == 0); positive-savings F3 remains eligible
    # with pending literals. Reference evidence: GOLEM 10408 (F3 over F5)
    # and 28128 (F3 over F7). All reference F5s overlapping F3 have cnt=2
    # and begin after a pending raw run, where the zero-savings F3 is gated.
    eligible_f3 = any(c[0][0] == 0xF3 and (c[2] > 0 or raw_len == 0)
                      for c in candidates)
    if eligible_f3:
        candidates = [c for c in candidates if c[0][0] not in (0xF5, 0xF6, 0xF7)]

    # Pick best pattern using 1-level lookahead. Ties go to the candidate
    # with the larger iteration count (confirmed: FA cnt=8 beats F3 cnt=4 at
    # GNAME dec 12502, F1 cnt=19 beats F0 cnt=18 at dec 34420, F1 cnt=186
    # beats F2 cnt=93 at dec 27352); remaining ties to the first candidate
    # in F0..FB order.
    p_cnt = 0
    for enc, adv, sav, cnt in candidates:
        total = sav + best_pattern_savings(src, i + adv, n)
        # Zero-savings F3 is only useful when it can replace a standalone
        # literal run.  When literal bytes are already pending, folding four
        # bytes into that existing run costs no extra header, so do not let
        # lookahead make the zero-savings F3 fire early.
        if sav == 0 and enc[0] == 0xF3 and raw_len != 0:
            continue
        if total <= 0:
            if not (total == 0 and enc[0] == 0xF3 and raw_len == 0):
                continue
        if p_enc is None or total > p_sav or (total == p_sav and cnt > p_cnt):
            p_enc, p_sav, p_adv, p_cnt = enc, total, adv, cnt

    # Compare best pattern vs best back-ref by raw efficiency (savings/advance).
    # Back-ref wins only if strictly higher efficiency; patterns win ties.
    if b_enc is not None and p_enc is not None:
        # p_sav here holds the lookahead total; need raw savings for efficiency
        p_raw_sav = p_adv - len(p_enc)
        if b_sav * p_adv > p_raw_sav * b_adv:
            # F1 is checked directly against the backref when the lookahead-
            # selected pattern loses; F0 rescues only an exact efficiency tie.
            for want, strict in ((0xF1, True), (0xF0, False)):
                for enc, adv, sav, cnt in candidates:
                    if enc[0] != want:
                        continue
                    rel = sav * b_adv - b_sav * adv
                    if (strict and rel > 0) or ((not strict) and rel == 0):
                        return bytes(enc), adv
            return bytes(b_enc), b_adv
        return bytes(p_enc), p_adv
    if p_enc is not None:
        return bytes(p_enc), p_adv
    if b_enc is not None:
        return bytes(b_enc), b_adv
    return None


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python compressor.py input_file output_file")
        sys.exit(1)
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    with open(input_file, 'rb') as f:
        data = f.read()
    compressed = compress(data)
    with open(output_file, 'wb') as f:
        f.write(compressed)
    print(f"Compressed {len(data)} bytes -> {len(compressed)} bytes "
          f"(ratio: {len(compressed)/len(data):.3f})")
