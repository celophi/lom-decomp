# Compressor Reverse-Engineering - Status and Continuation Guide

Last updated: 2026-07-01.

If you are picking this up cold: read this whole file, then
jump to "How to continue" at the bottom. Everything you need is here.

## What we are trying to do

This repo is a byte-for-byte matching decompilation of Legend of Mana (PSX).
Game overlays are stored COMPRESSED on the disc (`disc/BIN/*.BIN`) using a
proprietary LZ + pattern-opcode scheme. To prove a rebuilt overlay matches
the original disc we must:

1. Link the overlay ELF (build system does this).
2. `objcopy -O binary` it to a raw image.
3. Prepend one 0x00 byte (present in the original decompressed images but
   stripped by objcopy because it precedes any output section).
4. Compress it with `tools/compressor/compressor.py`.
5. SHA1-compare against the original `disc/BIN/<NAME>.BIN`.

Steps 1-3 are solved. Step 4 is this effort: `compressor.py` must reproduce
the EXACT byte stream the original 1999 compressor produced - same opcode
choices, same tie-breaks, same quirks - because many different encodings
decompress to the same data but only one matches the disc.

The decompressor is fully understood (`tools/splat_ext/decompress.py`, ported
from the game's assembly; treat it as ground truth, do not modify). The
original compressor's source does not exist; we infer its rules by comparing
our output against the 17 reference BINs opcode-by-opcode.

Key verified fact (2026-07-01): `decompress(GNAME.BIN)` equals
`0x00 + build/overlays/gname/gname.raw.tmp` byte-for-byte, so the linked
GNAME overlay itself is already perfect; ONLY the compressor blocks
`verify-gname`. (The Makefile still needs a gname rule mirroring the
gover/movie 0x00-prefix rules around line 790.)

## Where everything lives

```
tools/compressor/compressor.py    - the compressor under development
tools/compressor/test_roundtrip.py- canonical progress metric (see below)
tools/compressor/region_diff.py   - opcode-level diff vs reference (see below)
tools/compressor/backref_candidates.py - candidate inspector for the one
                                    remaining unsolved rule
tools/splat_ext/decompress.py     - reference decompressor (frozen, correct)
disc/BIN/*.BIN                    - 17 original compressed overlays (targets)
build/overlays/<name>/            - linked ELFs / raw images (only for
                                    overlays that currently build)
Makefile (~line 760-835)          - GOVER/MOVIE compress+verify rules; the
                                    pattern GNAME needs to copy
```

Older scripts (`diagnose.py`, `f5_*.py`, `simple_compressor.py`, `test_all.py`,
`test_simple.py`) are from an earlier phase; they reference a `decompressed/`
directory that no longer exists and `test_all.py` is superseded by
`test_roundtrip.py`. Historical only - do not trust their conclusions (their
opcode decoder had the F6 length bug described below).

## How to test

```
cd tools/compressor
python test_roundtrip.py                # all BINs (FIELD/TITLE are slow,
                                        # several minutes total)
python test_roundtrip.py GNAME GOVER MOVIE CLOAD SHOP   # the fast core set
```

`test_roundtrip.py` decompresses each reference BIN and recompresses that
data - identical input to what the build pipeline produces, so no build is
needed. GOVER and MOVIE must stay MATCH after ANY change (they are the
regression suite); GNAME/CLOAD/SHOP are the active targets.

Byte diff counts are a MISLEADING metric: one wrong opcode shifts every
subsequent byte. The honest metric is divergence REGIONS:

```
python region_diff.py GNAME             # list divergence regions
python region_diff.py GNAME --dump 5    # + opcode/data dump per region
python region_diff.py GNAME --at 21985  # dump around one decompressed pos
```

Each region = one wrong decision (plus resync noise). Work a region by
looking at what the reference chose vs what we chose at the region start.

## Opcode format summary (from decompress.py)

| Op   | Meaning                                   | Enc bytes | Advance |
|------|-------------------------------------------|-----------|---------|
| 0x00-0xEF | raw copy of (op+1) literal bytes     | 1+n       | n (1-240) |
| F0   | nibble value repeated cnt=3..18           | 2         | cnt     |
| F1   | byte repeated cnt=4..259                  | 3         | cnt     |
| F2   | nibble pair repeated cnt=2..257           | 3         | 2*cnt   |
| F3   | byte pair repeated cnt=2..257             | 4         | 2*cnt   |
| F4   | 3-byte pattern repeated cnt=2..257        | 5         | 3*cnt   |
| F5   | {fixed, varying} pairs, cnt=4..259        | 3+cnt     | 2*cnt   |
| F6   | {b0, b1, varying} triples, cnt=3..258     | 4+cnt     | 3*cnt   |
| F7   | {b0, b1, b2, varying} quads, cnt=2..257   | 5+cnt     | 4*cnt   |
| F8   | +1 arithmetic run, cnt=4..259             | 3         | cnt     |
| F9   | -1 arithmetic run, cnt=4..259             | 3         | cnt     |
| FA   | +step arithmetic run, cnt=5..260          | 4         | cnt     |
| FB   | 16-bit LE arithmetic pairs, cnt=3..258, signed 8-bit delta | 5 | 2*cnt |
| FC   | backref, offset<=4095, cnt=4..19          | 3         | cnt     |
| FD   | backref, offset<=255, cnt=20..275         | 3         | cnt     |
| FE   | backref, dist=(nib+1)*8 (8..128), cnt=3..18 | 2       | cnt     |
| FF   | end of stream                             | 1         | -       |

WARNING: F6 iterations = param + 3 (not +2). An earlier version of our
analysis tooling decoded F6 with +2, silently corrupting every opcode-stream
analysis after any F6 and producing the false conclusion "F6 is never
generated". F6 IS generated (first confirmed at GNAME dec 18247).

## Current status (2026-07-01)

Roundtrip results with all rules below implemented:

| File    | Result                                                  |
|---------|---------------------------------------------------------|
| GOVER   | MATCH                                                   |
| MOVIE   | MATCH                                                   |
| SHOP    | 85 diffs (was 3,820 at session start)                   |
| CLOAD   | 905 diffs (was 1,843)                                   |
| GNAME   | 21,200 diffs, ~123 regions (was 31,098, first diff 8638)|
| ZUKAN   | 34,609 diffs, first 8,466 (was first diff at byte 142 - old "Issue 2" is fixed) |
| CHECKPS | 33,777 diffs, first 7,854 (was 34,306 / 7,525)          |
| GOLEM   | 25,514 diffs, 136 regions, first 3,472 (regions are a mix of the backref class and more sav-0 F3 interplay, e.g. dec 10408 ref picks F3(cnt 2, sav 0) over our F5, dec 10594 ref picks F3(2) over raw) |
| others  | not re-measured since the new rules landed              |

Essentially all remaining GNAME/SHOP/CLOAD regions are ONE class: backref
candidate selection (see "Open problem" below).

## Confirmed rules (all implemented in compressor.py)

Architecture: greedy single pass. At each position collect pattern
candidates (F0-FB) and backref candidates (FC/FD/FE), pick the best, else
emit a raw literal byte. A hash table (4-gram -> deque of past positions)
accelerates FC/FD.

1. Pattern selection: maximize `savings + best_pattern_savings(next_pos)`
   (1-level lookahead, patterns only in the lookahead).
2. Candidates with `savings + lookahead <= 0` never fire. This gates the
   zero-savings F3 (rule 10) without breaking anything else.
3. Pattern ties (equal lookahead total): the candidate with the LARGER
   iteration count wins; remaining ties go to the first in F0..FB order.
   Evidence: FA(cnt 8) beats F3(cnt 4) at GNAME dec 12502; F1(19) beats
   F0(18) at dec 34420; F1(186) beats F2(93) at dec 27352.
4. Pattern vs backref: compare raw efficiency (savings/advance); backref
   wins only if STRICTLY higher; patterns win ties.
5. F5 truncation (THE major 2026-07-01 discovery; solves old "Issue 1"):
   while extending F5 pair-by-pair, stop RIGHT AFTER a pair whose varying
   byte (odd alignment, i+2k+1) starts a qualifying FB run (fb cnt >= 3).
   - Subsumes the old "FB qualifying at i+1 suppresses F5" rule (that is
     the pair-0 case: cnt=1 < 4 minimum, F5 dies).
   - Explains the old CLOAD 24240 mystery of F5(4)+F5(13): after the
     truncated F5 the scan resumes on a pair boundary where the odd-aligned
     FB is unreachable, so a second F5 fires instead of FB.
   - Evidence: GNAME dec 12042/12060/12324/12591; fixed most of SHOP.
6. F5 qualification (pre-existing, still valid): cnt_q = leading strict
   pairs (varying != fixed); qualifies if cnt_q >= 4, or cnt_q in [1,3]
   with >= 2 degenerate pairs in the run; total cnt >= 4.
7. F5 beats F7 whenever F5 is viable (F7 candidates dropped), even at lower
   savings (MENU dec 1115).
8. F6 is a normal candidate: {b0,b1,varying} triples, min cnt 3 (savings 2
   at minimum). Confirmed GNAME dec 18247.
9. F7 skipped when b0 == b1 == b2.
10. F3 minimum cnt is 2 (4 enc bytes covering 4 = zero savings); it fires
    via rule 2 only when lookahead makes the total positive. Evidence:
    GNAME dec 30209/32766/34869 emit `f3 00 xx yy`. Unconditional sav-0 F3
    regresses GNAME (first diff moves from 8789 to 6224), so the rule-2
    gate matters. NOTE: the gate is believed but not proven - GNAME dec
    30209/34869 still diverge, and GOLEM dec 10408 shows the ref picking
    F3(cnt 2, sav 0) over a viable F5 while dec 10594 picks F3(2) over
    raw; revisit this rule if the backref fix does not resolve them.
11. Backrefs: no self-referential copies (match length capped at distance).
12. Backref selection (as currently implemented; KNOWN INCOMPLETE, see open
    problem): max savings, ties by largest distance, then largest advance.
    FE candidates checked at all 16 distances; FC/FD candidates from the
    hash chain, oldest first. Raw literals and all bytes covered by emitted
    opcodes are inserted into the hash.

## Open problem: backref candidate selection (the last GNAME blocker)

When several FC/FD/FE candidates have EQUAL savings, which one did the
original pick? Current evidence is contradictory and this is the one rule
still unknown:

- GNAME mid-file (11 sampled FC sites, e.g. dec 21985/22045/23221/26116):
  the reference picks the NEWEST matching candidate (smallest offset) at
  10/11 sites. Our oldest-first pick is wrong there.
- BUT flipping to newest-first globally breaks GOVER (22 diffs, first
  region at dec 246) and MOVIE (1,769) and early GNAME (first diff 311).
  Early-file the reference behaves oldest-first/largest-offset.
- The 11th site (GNAME dec 21985) is also anomalous for newest-first: the
  newest candidate (p=21133, covered by FC+3 of an adv-4 FC) is skipped in
  favor of the 2nd newest (p=20269). Reproduce the table with:
  `python backref_candidates.py GNAME 21985 --ref-off 0x6b3`
- FE beats FC on savings ties (GNAME dec 32770: ref `fe ff` over an
  equal-savings FC; also dec 34361). Our current dist-based tie-break gets
  these wrong but fixing it via "first candidate wins, FE evaluated first"
  was only tested together with newest-first, so retest it standalone.

Hypotheses TESTED AND REJECTED (2026-07-01):
- Global newest-first (breaks GOVER/MOVIE, see above).
- Hash insertion only at scan positions (opcode starts + raw literal
  bytes): reference picks at GNAME dec 23221/26116/27186/30876/30891 are
  INTERIOR to emitted FC/FE opcodes, so interiors are inserted too.
- Insertion only when the 4-gram fits inside the covering opcode's span:
  GNAME dec 23221 (FC+3 of adv-4, does not fit) is still the ref pick.

Hypotheses NOT yet tested:
- A real bounded hash table like the era's LZ coders: small table indexed
  by a WEAK hash of 2-4 bytes (collisions between different grams), single
  slot or short chain per bucket, entries overwritten by newer insertions.
  Collision-eviction would naturally produce "sometimes the near match,
  sometimes a far one" depending on what else hashed into the bucket since.
  This is the most promising direction. Try: table sizes 256/1024/4096,
  keys of 2/3/4 bytes, hash = sum/xor/shift combos, 1-2 slots per bucket.
  Fit against the GOVER first-region case (dec 246: ref picks offset 0x33,
  newest-first would pick 0x1b) AND the GNAME site table simultaneously.
- Chain search depth limits interacting with a weak hash.
- Insertion cadence tied to opcode advance (e.g. inserting every K-th byte
  of long opcodes).
- Remaining non-FC regions (GNAME dec 30415 F0-vs-FE, 32995 raw-split,
  30209/34869 sav-0 F3) may share a root cause with the backref rule -
  our FE/FC choice feeds the efficiency comparison of rule 4 - so fix
  backrefs first, then re-triage.

## Methodology that worked (keep doing this)

1. Run `region_diff.py <NAME>` and take the FIRST few regions (later ones
   can be cascades).
2. For each region, decode by hand what the reference did vs what we did;
   identify the decision point and enumerate the candidates our compressor
   saw (add temporary prints to find_best if needed).
3. Form the smallest rule that explains the reference choice; check it
   against the OTHER regions in the same file before implementing.
4. Implement, run `test_roundtrip.py GNAME GOVER MOVIE CLOAD SHOP`.
   GOVER/MOVIE must stay MATCH. Region count must not increase.
5. If a change helps one metric but breaks another, the rule is
   conditional - bisect which sub-change causes what (this found the
   lookahead gate on sav-0 F3 and the cnt tie-break).
6. Byte-diff counts can go UP while region count goes DOWN; trust regions.

One session of this (2026-07-01) took GNAME from 5 huge region classes to
one, and nearly matched SHOP as a side effect. The remaining rule is
well-scoped; expect several files to collapse to MATCH once it lands.

## How to continue

1. `cd tools/compressor && python test_roundtrip.py GNAME GOVER MOVIE CLOAD SHOP`
   to confirm the baseline in "Current status" still holds.
2. Attack the open problem above. Start with the bounded/weak-hash
   hypothesis; the two anchor cases to satisfy simultaneously are
   GOVER dec 246 and the GNAME 11-site table (backref_candidates.py).
3. After any win, re-run the full suite (`python test_roundtrip.py`) and
   update the status table here.
4. When GNAME reaches MATCH: add gname compress+verify rules to the
   Makefile (copy the gover block at ~line 790, including the printf '\0'
   prefix step) and wire it into the overlays flow.
5. Files beyond GNAME (ZUKAN, MENU, FIELD, ...) should then be mostly or
   fully fixed; re-measure and triage whatever remains with region_diff.py.
