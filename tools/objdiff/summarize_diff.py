#!/usr/bin/env python3
"""
Summarize an objdiff JSON diff for one function into a compact report.

Designed for large functions where the full side-by-side diff is too big to
read in one pass. The report contains:

  - match percent, byte/instruction sizes for both sides
  - row counts by diff kind (exact / arg mismatch / replace / insert / delete)
  - stack frame size of both sides (first "addiu sp, sp, -X")
  - a mismatch-density map over the target address space
  - the largest structural runs (consecutive insert/delete/replace rows)
  - stack-slot traffic (lw/sw per sp offset) where the two sides disagree
  - optional score history with delta vs the previous run and best-so-far

Usage:
    python3 tools/objdiff/summarize_diff.py diff.json func_name
    python3 tools/objdiff/summarize_diff.py diff.json func_name \
        --history build/mcp-dumps/_history/func.jsonl --label f7_s14
"""

import argparse
import json
import re
import sys
import time
from collections import defaultdict
from pathlib import Path

MEM_RE = re.compile(
    r"^(lw|sw|lh|lhu|sh|lb|lbu|sb|lwl|lwr|swl|swr)\s+\w+, (-?(?:0x[0-9a-fA-F]+|\d+))\(sp\)"
)
FRAME_RE = re.compile(r"^addiu sp, sp, (-(?:0x[0-9a-fA-F]+|\d+))")

KIND_SHORT = {
    None: "exact",
    "DIFF_ARG_MISMATCH": "argdiff",
    "DIFF_REPLACE": "replace",
    "DIFF_DELETE": "target-only",
    "DIFF_INSERT": "yours-only",
}


def find_symbol(side, func_name):
    for s in side.get("symbols", []):
        if s.get("name") == func_name:
            return s
    return None


def fmt_of(entry: dict) -> str:
    ins = entry.get("instruction")
    return ins.get("formatted", "") if ins else ""


def addr_of(entry):
    ins = entry.get("instruction")
    if not ins:
        return None
    a = ins.get("address")
    return int(a) if a is not None else None


def frame_size(instrs: list) -> str:
    for e in instrs:
        m = FRAME_RE.match(fmt_of(e))
        if m:
            return m.group(1)
    return "?"


def slot_map(instrs: list) -> dict:
    slots: dict[int, list[int]] = defaultdict(lambda: [0, 0])
    for e in instrs:
        m = MEM_RE.match(fmt_of(e))
        if m:
            off = int(m.group(2), 0)
            slots[off][1 if m.group(1).startswith("s") else 0] += 1
    return slots


def density_map(left: list, out: list, size: int) -> None:
    # Pick a window so the map stays around <= 24 rows.
    window = 0x100
    while size // window > 24:
        window *= 2
    buckets: dict[int, list[int]] = {}
    laddr = 0
    for e in left:
        a = addr_of(e)
        if a is not None:
            laddr = a
        b = laddr // window
        t, x = buckets.get(b, (0, 0))
        buckets[b] = (t + 1, x + (1 if e.get("diff_kind") else 0))
    out.append(f"density (rows differing per 0x{window:X} of target):")
    for b in sorted(buckets):
        t, x = buckets[b]
        pct = 100.0 * x / t if t else 0.0
        bar = "#" * int(30 * x / t) if t else ""
        out.append(f"  0x{b * window:04X}  {x:3d}/{t:3d}  {pct:5.1f}%  {bar}")


def structural_runs(left: list, right: list, out: list, min_len: int = 4, top: int = 12) -> None:
    runs = []
    cur = None
    laddr = 0
    for le, re_ in zip(left, right):
        a = addr_of(le)
        if a is not None:
            laddr = a
        kind = le.get("diff_kind")
        if kind in ("DIFF_DELETE", "DIFF_INSERT", "DIFF_REPLACE"):
            if cur is None:
                cur = {"addr": laddr, "del": 0, "ins": 0, "rep": 0, "ctx": []}
            key = {"DIFF_DELETE": "del", "DIFF_INSERT": "ins", "DIFF_REPLACE": "rep"}[kind]
            cur[key] += 1
            if len(cur["ctx"]) < 2:
                ctx = fmt_of(le) or fmt_of(re_)
                if ctx and ctx != "nop":
                    cur["ctx"].append(ctx)
        else:
            if cur and cur["del"] + cur["ins"] + cur["rep"] >= min_len:
                runs.append(cur)
            cur = None
    if cur and cur["del"] + cur["ins"] + cur["rep"] >= min_len:
        runs.append(cur)
    runs.sort(key=lambda r: -(r["del"] + r["ins"] + r["rep"]))
    if not runs:
        out.append(f"structural runs (>= {min_len} rows): none")
        return
    out.append(f"structural runs (>= {min_len} rows, top {min(top, len(runs))} of {len(runs)}):")
    for r in runs[:top]:
        ctx = " ; ".join(r["ctx"])[:70]
        out.append(
            f"  tgt 0x{r['addr']:04X}  del={r['del']:2d} ins={r['ins']:2d} rep={r['rep']:2d}  | {ctx}"
        )


def slot_mismatches(lsym: dict, rsym: dict, out: list) -> None:
    lslots = slot_map(lsym.get("instructions", []))
    rslots = slot_map(rsym.get("instructions", []))
    rows = []
    for off in sorted(set(lslots) | set(rslots)):
        lt = lslots.get(off, [0, 0])
        rt = rslots.get(off, [0, 0])
        if list(lt) != list(rt):
            missing = "" if (off in lslots) == (off in rslots) else "  <- one side only"
            rows.append(f"  0x{off:03X}  target {lt[0]}/{lt[1]}  yours {rt[0]}/{rt[1]}{missing}")
    if not rows:
        out.append("sp slots (loads/stores per offset): all match")
        return
    out.append("sp slots where traffic differs (target lw/sw vs yours lw/sw):")
    out.extend(rows)


def history(path: Path, label: str, stats: dict, out: list) -> None:
    prev = None
    best = None
    entries = []
    if path.exists():
        for line in path.read_text(encoding="utf-8").splitlines():
            line = line.strip()
            if not line:
                continue
            try:
                entries.append(json.loads(line))
            except json.JSONDecodeError:
                continue
    if entries:
        prev = entries[-1]
        best = max(entries, key=lambda e: e.get("match", 0.0))
    lines = []
    if prev is not None:
        delta = stats["match"] - prev.get("match", 0.0)
        dexact = stats["exact"] - prev.get("exact", 0)
        lines.append(
            f"history: prev {prev.get('match', 0.0):.3f}% ({prev.get('label', '?')})"
            f"  delta {delta:+.3f}%  exact {dexact:+d}"
        )
    if best is not None and best.get("match", 0.0) > stats["match"]:
        lines.append(
            f"history: BEST so far {best.get('match', 0.0):.3f}% ({best.get('label', '?')})"
            " - this run is below best"
        )
    if lines:
        out.extend(lines)
    else:
        out.append("history: first recorded run")
    path.parent.mkdir(parents=True, exist_ok=True)
    entry = dict(stats)
    entry["label"] = label
    entry["ts"] = time.strftime("%Y-%m-%dT%H:%M:%S")
    with open(path, "a", encoding="utf-8") as f:
        f.write(json.dumps(entry) + "\n")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("input", help="objdiff JSON file")
    parser.add_argument("func_name", help="Function symbol to summarize")
    parser.add_argument("--history", help="JSONL score-history file to read and append to")
    parser.add_argument("--label", default="scratch", help="Label recorded in the history entry")
    args = parser.parse_args()

    data = json.loads(Path(args.input).read_text(encoding="utf-8"))
    lsym = find_symbol(data.get("left", {}), args.func_name)
    rsym = find_symbol(data.get("right", {}), args.func_name)
    if lsym is None or rsym is None:
        print(f"MATCH_PERCENT=SYMBOL_NOT_FOUND ({args.func_name})")
        return 1

    L = lsym.get("instructions", [])
    R = rsym.get("instructions", [])
    match = float(lsym.get("match_percent", 0.0) or 0.0)
    lsize = int(lsym.get("size", 0))
    rsize = int(rsym.get("size", 0))

    counts: dict[str, int] = defaultdict(int)
    for e in L:
        counts[KIND_SHORT.get(e.get("diff_kind"), e.get("diff_kind"))] += 1

    out: list[str] = []
    out.append(f"MATCH_PERCENT={match:.6f}")
    stats = {
        "match": match,
        "exact": counts["exact"],
        "argdiff": counts["argdiff"],
        "replace": counts["replace"],
        "del": counts["target-only"],
        "ins": counts["yours-only"],
        "right_size": rsize,
    }
    if args.history:
        history(Path(args.history), args.label, stats, out)

    linsns = sum(1 for e in L if e.get("instruction"))
    rinsns = sum(1 for e in R if e.get("instruction"))
    out.append(
        f"size: target {lsize} bytes / {linsns} insns; yours {rsize} bytes / {rinsns} insns"
        f" ({rinsns - linsns:+d} insns)"
    )
    out.append(
        f"rows: exact {counts['exact']} | argdiff {counts['argdiff']}"
        f" | replace {counts['replace']} | target-only {counts['target-only']}"
        f" | yours-only {counts['yours-only']}"
    )
    lframe = frame_size(L)
    rframe = frame_size(R)
    mark = "" if lframe == rframe else "  <- MISMATCH"
    out.append(f"frame: target {lframe} | yours {rframe}{mark}")
    out.append("")
    density_map(L, out, lsize)
    out.append("")
    structural_runs(L, R, out)
    out.append("")
    slot_mismatches(lsym, rsym, out)
    print("\n".join(out))
    return 0


if __name__ == "__main__":
    sys.exit(main())
