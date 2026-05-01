# Compressor Reverse-Engineering Progress

## Goal
Produce a Python `compressor.py` that, when run on `*.inflated.bin` / `*.decompressed` files, produces byte-exact output matching the corresponding `*.compressed.bin` / `*.BIN` reference files.

## Current Status (2026-04-30)

| File | Ref Size | Diffs | First Diff (cmp byte) |
|------|----------|-------|----------------------|
| GOVER | 1,522 B | **0** ✓ | — |
| MOVIE | — | **0** ✓ | — |
| CLOAD | 18,870 B | 1,843 | 16,606 |
| SHOP | — | 3,820 | 8,173 |
| CHECKPS | — | 34,306 | 7,525 |
| GNAME | — | 31,098 | 8,638 |
| GOLEM | — | 23,701 | 3,472 |
| ZUKAN | — | 43,037 | 142 |
| ADDHERO | — | 81,479 | 18,219 |
| CARDA | — | 99,987 | 13,024 |
| GOSUB | — | 101,992 | 17,957 |
| MENU | — | 93,563 | 527 |
| NIKI | — | 90,879 | 9,024 |
| TITLE | — | 268,934 | 7,220 |
| FIELD | — | 379,055 | 3,040 |

**2/15 files match exactly.**

---

---

## Analysis: Compressor Design Philosophy

This compressor is a **greedy hybrid** — LZ-style back-references combined with a rich set of specialized run-length and arithmetic opcodes. It was clearly written by someone who knew exactly what data it would compress.

### Domain-specific opcode design

The opcodes are not general-purpose. They are tuned for specific game data types:

- **F8/F9/FA** (arithmetic runs: +1, -1, arbitrary step) — sequential tile IDs, incrementing indices, coordinate tables
- **FB** (16-bit LE pairs with constant delta) — almost certainly designed for **RGB555 color palettes**. A palette fade is exactly a 16-bit arithmetic sequence
- **F5** ({fixed, varying} pairs) — attribute/tilemap data where every other byte is constant (e.g., a tile attribute byte repeating while the tile index varies)
- **F0/F2** (nibble-based ops) — targets **4bpp tile data**, where pixel values are 0–15

### Small-window LZ

Back-references have tiny windows: FC ~4KB, FD ~256B, FE ~128B. Not trying to be DEFLATE. Optimized for fast decompression on constrained hardware with minimal RAM overhead.

### Greedy, single-pass

No optimal parsing, no dynamic programming. The decompressor runs on a game cartridge and must be fast and simple. The compressor runs offline at build time — the author chose simple greedy logic rather than optimal parsing.

### Overall philosophy

Compress game assets efficiently using *structural knowledge* of what those assets look like, rather than a general-purpose algorithm. The opcode set was hand-designed for this specific game's data.

---

## Analysis: One Compressor or Multiple?

**Most likely: one compressor, one version.** The differences in diff counts across files are better explained by data complexity than by versioning.

### Evidence for one compressor
- All files use the same opcode set — no file uses an opcode unseen in others
- GOVER and MOVIE match perfectly, but they are the simplest files (GOVER ~4KB inflated) and likely don't exercise the problematic F5 code paths
- The F5 count mismatch appears consistently across CLOAD and WMAP as the same *class* of bug — a single misunderstood rule rather than different behavior per file

### Evidence that could suggest variants
- The wide spread in diff counts (0 to 379k) is striking — but FIELD is a large map tileset with highly varied patterns, while GOVER is a simple screen. Data complexity explains this better than compressor differences.
- The F5 count splitting behavior (emitting cnt=4 then cnt=13 instead of cnt=17) could theoretically indicate a version difference, but it feels like a single consistent rule not yet decoded.

### Conclusion
The remaining diffs most likely trace back to one or two misunderstood rules — primarily the F5 count rule. Once that is resolved, many files should collapse toward zero diffs simultaneously.

---

## Why This Is Hard

We are reverse-engineering a **proprietary greedy compressor** from its output alone. We have:
- The decompressor (C source + Python port) — tells us what each opcode does
- Reference compressed files — tells us what the original compressor *chose* to emit
- No compressor source code

The difficulty is that **many encodings are valid** for any given input. The decompressor accepts all of them. We must reproduce the *exact same greedy choices* the original compressor made — including apparently suboptimal ones — without knowing its internal logic.

---

## What We Know Works (Confirmed Rules)

### Opcode Selection Rules:

1. **Pattern opcodes (F0–FB)**: Among all viable pattern candidates, pick by **1-level lookahead** — maximize `savings + best_pattern_savings(next_position)`. First-found wins ties.

2. **Back-reference opcodes (FE/FD/FC)**: Pick by **maximum savings**, then by **largest distance** (`i - match_start`) to break ties, then by largest advance.

3. **Pattern vs Back-ref**: Compare by **efficiency** (`savings / advance`). Back-ref wins only if *strictly* higher efficiency; patterns win ties.

4. **F5 qualification** (two-phase count):
   - `cnt_q` = consecutive pairs at start where `varying != fixed`
   - `cnt` extended to include degenerate pairs (where `varying == fixed`)
   - Must satisfy `f5_cnt_qualifies`: either `cnt_q >= 4`, OR (`cnt_q in [1,3]` AND at least 2 degenerate pairs in the run)
   - Minimum `cnt >= 4`

5. **FB suppresses F5**: If FB qualifies at position `i+1` (cnt >= 3), F5 at position `i` is suppressed. This was confirmed by CLOAD pos 24232: data `[1,8,1,27,1,46,...]` — FB at pos+1 has v16=0x0108, delta=19, cnt=3, so reference skips F5 and uses RAW+FB instead.

6. **F5 disqualification (degenerate)**: When `cnt_q=1` and only 1 degenerate pair found (deg=1), F5 does not fire. Confirmed by WMAP pos 0: `cnt_q=1, cnt=11, deg=1` → reference uses RAW.

7. **F7 disqualification**: Skip if `b0 == b1 == b2`.

8. **F6**: Never generated.

9. **No self-referential back-refs**: `count_backref_match` caps at `dist = i - match_start`.

10. **Hash table**: 4-gram → deque of positions, oldest-first (largest offset first for tie-breaking within FC candidates).

---

## Open Issues

### F5 Categorization Study (2026-04-30)

Analyzed all 1,131 F5 instances across 15 reference files (`f5_categorize.py`):

**Distribution of ref_cnt vs cnt_max:**
- 721 (64%) max-extended (ref_cnt == cnt_max) — our current logic gets these right
- 376 (33%) truncated (ref_cnt < cnt_max) — bug

**ref_cnt vs cnt_q:**
- 501 (44%) ref_cnt == cnt_q (uses strict pairs only, no degenerate extension)
- 280 (25%) ref_cnt > cnt_q (degenerate extension used)
- 350 (31%) ref_cnt < cnt_q (truncated below strict pair count)

**For truncated F5s (376 cases): kind of competing opcode at first qualifying k:**
- f5_same (another F5, same fixed byte): 249
- backref (FC/FD/FE): 60
- F3: 29; f5_diff: 18; rest <10 each

**Backref-stopped offsets (ref_cnt − first_backref_k):** wide spread −230 to +52. Peaks at +0 (34), −3 (28), +1 (16). Reference does NOT consistently stop at first available backref.

**f5_same-stopped offsets:** peak at +0 (50), then flat +2..+6 (~75). The ref often continues past a same-fixed F5 boundary.

**Conclusion:** No single simple rule explains the truncation pattern. Multiple factors interact. The 44% "ref_cnt == cnt_q" signal is the strongest single hypothesis but only covers half the cases.

### Issue 1: F5 Count Mismatch (ROOT CAUSE of most remaining diffs)

The reference compressor emits F5 runs with **shorter counts** than the maximum possible. Our code always extends to the maximum. Examples:

- **CLOAD decompressed pos 24240**: `cnt_q=17`, max `cnt=17`, but reference emits `F5(cnt=4)` then immediately `F5(cnt=13)`. We emit `F5(cnt=17)`.
- **WMAP decompressed pos 4424**: `cnt_ext=259` possible, but reference emits `F5(cnt=64)`.
- **WMAP decompressed pos 7552**: ref uses a shorter count than max.
- **WMAP decompressed pos 529254, 530470**: similar short-count F5 cases.

**What we've ruled out:**
- Not explained by 1-level lookahead (max-count F5 always wins in efficiency)
- Not explained by FB qualification at sub-positions within the run
- No clean threshold (cnt_q vs cnt_ext ratio, etc.) works universally

**Hypothesis**: The reference compressor may build the F5 candidate differently — perhaps it does NOT extend past the strict `cnt_q` pairs when the degenerate-pair extension is large. Or it may have a hard cap on F5 length in certain conditions. Or the "greedy scan" restarts after emitting each opcode, and there's an internal state variable that caps the count.

**Most promising lead**: At CLOAD pos 24240, `cnt_q=17` and the reference *does* use F5 at cnt=17 total (4+13=17 pairs), but split into two opcodes. This means the split is not random — perhaps there is a maximum of `cnt_q` pairs per F5 opcode? Or maybe the first F5 is limited to 4 pairs (the minimum) when some condition holds, then the remainder is emitted separately.

### Issue 2: ZUKAN First Diff at Compressed Byte 142 (very early)

ZUKAN has its first diff at byte 142. This is early enough that it may reveal a completely different bug. Worth examining independently before assuming it's the same F5 count issue.

### Issue 3: MENU First Diff at Compressed Byte 527

Similarly early — may be a different root cause worth examining directly.

### Issue 4: FIELD First Diff at Compressed Byte 3,040

379k diffs is the largest by far. Likely a common pattern the file is heavy on (perhaps F5 or a back-ref rule) that cascades massively.

---

## Key Data Points

### CLOAD pos 24240 (cnt mismatch detail):
- Data: `[1, 8, 1, 27, 1, 46, ...]` (fixed=1, varying bytes are 8, 27, 46, ...)
- `cnt_q=17` (all strict pairs, no degenerate pairs)
- Max cnt=17 total pairs
- Reference: emits `F5(cnt=4)` then `F5(cnt=13)` — total coverage same (17 pairs)
- Our output: emits `F5(cnt=17)` — decompresses to same bytes but encoded differently

### WMAP pos 4424:
- Data: `[0, 3, 0, 0, 0, 5, 0, 4, ...]` (fixed=0)
- `cnt_q=1`, many degenerate pairs, `cnt_ext=259` possible
- Reference: emits `F5(cnt=64)` — not max

### F5 false positive conditions (what NOT to do):
- WMAP pos 0: `cnt_q=1, cnt=11, deg=1` → reference uses RAW (correctly excluded by `deg < 2` rule)
- CLOAD pos 24232: `cnt_q=21, cnt=21, sav=18` → reference uses RAW+FB (correctly excluded by `fb_qualifies_at` check)

### Reference F5 statistics:
- F6 is never used in any reference file
- F5 is used aggressively even for small savings (sav=1, cnt=4 minimum)

---

## Files

```
compressor.py         — Main file under development
decompressor.py       — Reference decompressor (correct, do not modify)
decompress.c          — C source for opcode reference
test_all.py           — Test suite: compresses all 15 files and reports diffs
decompressed/         — Input files for compressor (*.BIN.decompressed)
compressed/           — Reference target files (*.BIN)
PROGRESS.md           — This file
```

---

## Recommended Next Steps

### 1. Investigate ZUKAN first diff (byte 142) — quick win candidate
ZUKAN is small (~43k diffs) with a very early first diff. Decode the reference and our output around byte 142 to identify the root cause. If it's a different bug from F5 count mismatch, fixing it may unlock many files.

### 2. Investigate MENU first diff (byte 527) — another early diff
Same reasoning — early first diff in a medium-sized file may reveal a distinct bug.

### 3. Crack the F5 count rule
Look at many more F5 instances in the reference and try to find the pattern. Specifically:
- Enumerate ALL F5 opcodes in CLOAD's reference and record `(decompressed_pos, cnt, cnt_q, deg)` for each
- Look for a relationship between `cnt_q` and the emitted `cnt` (e.g., does `cnt` always equal `cnt_q` when `cnt_q >= 4`?)
- Check whether the "extended" degenerate pairs are NEVER included in the reference F5 count (i.e., the reference might stop at `cnt_q`, not extending to include degenerate pairs)

**Key test**: If the reference always uses `cnt = cnt_q` (strict pairs only, never extending to include degenerate pairs), then:
- CLOAD pos 24240: `cnt_q=17` → F5(cnt=17) — but ref uses 4+13, so this alone doesn't explain it
- CLOAD pos 3598: `cnt_q=1, deg>=2` → F5(cnt=4) — ref uses F5(cnt=4), so `cnt` must include deg pairs here

### 4. Write a reference decoder for diagnostics
Build a script that decodes a reference .BIN file opcode-by-opcode and prints each opcode with its decompressed position and parameters. This makes it fast to look up "what does the reference emit at decompressed position X?" without manual decoding.

### 5. Binary-search early first-diff files
For files with early first diffs (ZUKAN byte 142, MENU byte 527, FIELD byte 3040), use the reference decoder to find the first opcode that differs and analyze why.
