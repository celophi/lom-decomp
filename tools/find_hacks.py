#!/usr/bin/env python3
"""
find_hacks.py -- Locate matching-decomp "hacks" in the C sources.

A "hack" here is a construct that exists only to steer GCC 2.x codegen so the
compiled output matches the original binary. These are not shapes a human would
normally write; they are scaffolding (inline asm, register pins) or contorted
control flow (do/while(0) wrappers, increment-then-decrement pairs) that nudge
scheduling, register allocation, or cross-jump merging. This script finds them
so they can be audited, tracked, or replaced with natural C once a better shape
is discovered.

Usage:
    python tools/find_hacks.py                     # scan src/ and include/
    python tools/find_hacks.py src/overlays/menu   # scan a subtree or file
    python tools/find_hacks.py --only inline_asm,register_pin
    python tools/find_hacks.py --exclude volatile_magic_addr
    python tools/find_hacks.py --json
    python tools/find_hacks.py --count             # per-category summary only
    python tools/find_hacks.py --list              # list categories and exit

Output (default): one line per finding, sorted by file then line:
    <file>:<line>: [<category>] <matched snippet>

Notes:
    - Comments and string/char literals are stripped before scanning for code
      patterns, so a `do { } while (0)` mentioned in a docblock is NOT flagged.
      Line numbers are preserved through stripping.
    - The `hack_comment` category is the exception: it scans the ORIGINAL text
      for explicit HACK/FAKE/etc. markers that a human left in a comment.
    - Categories differ in confidence. inline_asm, register_pin, do_while_zero,
      inc_dec and self_assign are almost always true scaffolding. The advisory
      ones (volatile_magic_addr, redundant_arith) can be legitimate; they are
      included so you can review them, not because they are always hacks.
"""

import argparse
import json
import re
import sys
from pathlib import Path

# ---------------------------------------------------------------------------
# Comment / string stripping
# ---------------------------------------------------------------------------

# Matches, in priority order: block comment, line comment, string literal,
# char literal. Anything matched is replaced by whitespace of equal length so
# byte offsets and (via newlines) line numbers stay identical.
_STRIP_RE = re.compile(
    r"""
      /\*.*?\*/            # block comment
    | //[^\n]*             # line comment
    | "(?:\\.|[^"\\])*"    # string literal
    | '(?:\\.|[^'\\])*'    # char literal
    """,
    re.DOTALL | re.VERBOSE,
)


def _blank_match(m: re.Match) -> str:
    """Replace a comment/literal with same-length whitespace, keeping newlines."""
    return "".join("\n" if ch == "\n" else " " for ch in m.group(0))


def strip_comments_and_strings(text: str) -> str:
    """Return text with comments and string/char literals blanked out.

    Newlines inside the removed spans are preserved so line numbers computed
    from character offsets remain correct.
    """
    return _STRIP_RE.sub(_blank_match, text)


# ---------------------------------------------------------------------------
# Detectors
# ---------------------------------------------------------------------------
#
# Each detector is (category, scans_raw, compiled_regex, description).
#   scans_raw = True  -> run against the ORIGINAL text (for comment markers)
#   scans_raw = False -> run against the comment/string-stripped code
#
# Every regex reports one finding per match at the match's start offset.

_DETECTORS = [
    (
        "inline_asm",
        False,
        # __asm__ ..., or a statement-position `asm`/`asm volatile` block.
        # `register x asm("v1")` is handled by register_pin and excluded here
        # by requiring asm to sit at statement position (after ; { } or line
        # start), never right after an identifier in a declaration.
        re.compile(
            r"""
              __asm__\b
            | (?:(?<=[;{}\s])) (?<!\w) asm \s* (?:__volatile__|volatile)? \s* \(
            """,
            re.VERBOSE,
        ),
        "Inline assembly (__asm__ / asm(...)) used as codegen scaffolding.",
    ),
    (
        "register_pin",
        False,
        # register <type...> <name> asm("reg");  -- pins a local to a register.
        # The "reg" string is already blanked by stripping, so match asm( only.
        re.compile(r"\bregister\b[^;=\n]*\basm\s*\("),
        "Local variable pinned to a hard register (register x asm(\"...\")).",
    ),
    (
        "do_while_zero",
        False,
        # while (0) / while(0) -- the tail of a do { ... } while (0) wrapper.
        re.compile(r"\bwhile\s*\(\s*0\s*\)"),
        "do { ... } while (0) wrapper forcing a basic-block boundary.",
    ),
    (
        "inc_dec",
        False,
        # x++; x--;  or  x--; x++;  or  x += 1; x -= 1;  (whitespace/newlines ok)
        re.compile(
            r"""
              \b(\w+)\s*\+\+\s*;\s*\1\s*--\s*;
            | \b(\w+)\s*--\s*;\s*\2\s*\+\+\s*;
            | \b(\w+)\s*\+=\s*1\s*;\s*\3\s*-=\s*1\s*;
            | \b(\w+)\s*-=\s*1\s*;\s*\4\s*\+=\s*1\s*;
            """,
            re.VERBOSE,
        ),
        "Increment immediately undone by decrement (or vice versa); dead effect.",
    ),
    (
        "self_assign",
        False,
        # x = x;  -- a no-op assignment used to force a copy / reload. The LHS
        # must be a bare identifier: the negative lookbehind rejects field or
        # array copies like `p->count = count;` (a natural param-to-field store,
        # not a self-assignment). `==`/`>=` etc. are excluded by requiring the
        # `=` not be preceded/followed by another comparison character.
        re.compile(r"(?<![\w.\)\]>])(\w+)\s*(?<![<>=!+\-*/%&|^])=(?![=])\s*\1\s*;"),
        "Self-assignment (x = x;), a no-op that forces a reload/copy.",
    ),
    (
        "volatile_magic_addr",
        False,
        # (volatile T*)0xNNNNNNNN  -- a hardcoded address cast through volatile.
        # Advisory: often a real fixed-address subsystem alias, not a hack.
        re.compile(r"\(\s*volatile\b[^)]*\)\s*0x[0-9A-Fa-f]{5,}"),
        "Cast of a hardcoded address through volatile (advisory; often legit).",
    ),
    (
        "hack_comment",
        True,
        # Explicit markers a human left in a comment.
        re.compile(
            r"(?i)(?://|/\*|\*)\s*[^\n]*?\b"
            r"(hack|fakematch|fake\s*match|nonmatch|non-?matching|no-code|"
            r"scaffold(?:ing)?|kludge|xxx\s*hack)\b"
        ),
        "Explicit HACK/FAKE/scaffold marker left in a comment.",
    ),
]

_CATEGORIES = [d[0] for d in _DETECTORS]


# ---------------------------------------------------------------------------
# Scanning
# ---------------------------------------------------------------------------


def _line_starts(text: str) -> list[int]:
    """Return a sorted list of character offsets at which each line begins."""
    starts = [0]
    for i, ch in enumerate(text):
        if ch == "\n":
            starts.append(i + 1)
    return starts


def _offset_to_line(starts: list[int], offset: int) -> int:
    """Map a character offset to a 1-based line number via binary search."""
    import bisect

    return bisect.bisect_right(starts, offset)


def scan_file(path: Path, categories: list[str]) -> list[dict]:
    """Return a list of finding dicts for one file."""
    try:
        raw = path.read_text(encoding="utf-8", errors="replace")
    except OSError as exc:
        print(f"warning: cannot read {path}: {exc}", file=sys.stderr)
        return []

    stripped = strip_comments_and_strings(raw)
    starts = _line_starts(raw)  # raw and stripped share offsets/newlines
    raw_lines = raw.splitlines()
    findings = []

    # A `register x asm(...)` declaration contains `asm(`, which inline_asm would
    # otherwise match too. Pre-compute the lines that hold a register pin so the
    # declaration is reported only as register_pin, never double-counted.
    pin_regex = next(d[2] for d in _DETECTORS if d[0] == "register_pin")
    pin_lines = {_offset_to_line(starts, m.start()) for m in pin_regex.finditer(stripped)}

    for category, scans_raw, regex, _desc in _DETECTORS:
        if category not in categories:
            continue
        target = raw if scans_raw else stripped
        for m in regex.finditer(target):
            line = _offset_to_line(starts, m.start())
            if category == "inline_asm" and line in pin_lines:
                continue  # this `asm(` belongs to a register pin
            line_text = raw_lines[line - 1].strip() if line - 1 < len(raw_lines) else ""
            findings.append(
                {
                    "file": str(path).replace("\\", "/"),
                    "line": line,
                    "category": category,
                    "snippet": line_text[:120],
                }
            )
    return findings


def iter_source_files(paths: list[Path]) -> list[Path]:
    """Expand the given paths into a sorted list of .c/.h files."""
    exts = {".c", ".h"}
    out: set[Path] = set()
    for p in paths:
        if p.is_file():
            if p.suffix in exts:
                out.add(p)
        elif p.is_dir():
            for ext in exts:
                out.update(p.rglob(f"*{ext}"))
        else:
            print(f"warning: no such path: {p}", file=sys.stderr)
    return sorted(out)


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description="Find matching-decomp codegen hacks in C sources.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument(
        "paths",
        nargs="*",
        help="Files or directories to scan (default: src include).",
    )
    parser.add_argument(
        "--only",
        metavar="CATS",
        help="Comma-separated categories to include (default: all).",
    )
    parser.add_argument(
        "--exclude",
        metavar="CATS",
        help="Comma-separated categories to skip.",
    )
    parser.add_argument("--json", action="store_true", help="Emit JSON.")
    parser.add_argument(
        "--count",
        action="store_true",
        help="Print per-category totals instead of individual findings.",
    )
    parser.add_argument(
        "--list",
        action="store_true",
        help="List available categories with descriptions and exit.",
    )
    args = parser.parse_args(argv)

    if args.list:
        width = max(len(c) for c in _CATEGORIES)
        for category, _raw, _rx, desc in _DETECTORS:
            print(f"  {category.ljust(width)}  {desc}")
        return 0

    # Resolve category selection.
    categories = list(_CATEGORIES)
    if args.only:
        requested = [c.strip() for c in args.only.split(",") if c.strip()]
        unknown = [c for c in requested if c not in _CATEGORIES]
        if unknown:
            parser.error(f"unknown category in --only: {', '.join(unknown)}")
        categories = requested
    if args.exclude:
        drop = [c.strip() for c in args.exclude.split(",") if c.strip()]
        unknown = [c for c in drop if c not in _CATEGORIES]
        if unknown:
            parser.error(f"unknown category in --exclude: {', '.join(unknown)}")
        categories = [c for c in categories if c not in drop]

    # Default scan roots.
    repo_root = Path(__file__).resolve().parent.parent
    if args.paths:
        roots = [Path(p) for p in args.paths]
    else:
        roots = [repo_root / "src", repo_root / "include"]
        roots = [r for r in roots if r.exists()]

    files = iter_source_files(roots)

    findings: list[dict] = []
    for f in files:
        findings.extend(scan_file(f, categories))

    findings.sort(key=lambda d: (d["file"], d["line"], d["category"]))

    if args.json:
        print(json.dumps(findings, indent=2))
        return 0

    if args.count:
        by_cat: dict[str, int] = {}
        for d in findings:
            by_cat[d["category"]] = by_cat.get(d["category"], 0) + 1
        width = max((len(c) for c in _CATEGORIES), default=0)
        for category in _CATEGORIES:
            if category in categories:
                print(f"  {category.ljust(width)}  {by_cat.get(category, 0)}")
        print(f"  {'TOTAL'.ljust(width)}  {len(findings)}")
        return 0

    for d in findings:
        print(f"{d['file']}:{d['line']}: [{d['category']}] {d['snippet']}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
