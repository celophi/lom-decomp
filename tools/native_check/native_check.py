#!/usr/bin/env python3
"""Compile the decomp's C for the host, to see what a 64-bit build would hit.

No native build of the game exists. This check keeps one possible: it runs
the host's C compiler over every C file a version builds, syntax only, with
the warnings that matter when pointers are wider than four bytes. The PS1
build is not involved and does not change.

What it reports:
- files that do not compile at all (the gate: this list may only shrink)
- per category, the places that assume 32-bit pointers or rely on old C

The files that fail today are listed in baseline_<version>_gcc.txt, which CI
checks. gcc and clang reject different things, and clang's set moves between
versions, so other compilers only get a report. A file that compiles
must keep compiling; one that starts compiling should be removed from the
baseline (--update-baseline does that). Warning counts are reported but not
gated, since they vary between compilers.

Usage (normally through `make native-check`):
  native_check.py --version us [--cc cc] [--update-baseline] FILE...
"""
from __future__ import annotations

import argparse
import collections
import concurrent.futures
import pathlib
import re
import shlex
import subprocess
import sys

REPO = pathlib.Path(__file__).resolve().parents[2]
HERE = pathlib.Path(__file__).resolve().parent

# The warnings that point at 32-bit assumptions or at pre-C99 habits a modern
# compiler rejects. Newer gcc and clang make several of them errors by
# default; they are turned back into warnings so one of them does not hide
# the rest of a file.
CATEGORIES = {
    "pointer-to-int-cast": "pointer cast to a smaller integer",
    "int-to-pointer-cast": "integer of a different size cast to a pointer",
    "int-conversion": "integer and pointer mixed without a cast",
    "incompatible-pointer-types": "incompatible pointer types",
    "implicit-function-declaration": "call without a prototype",
    "implicit-int": "declaration without a type (implicit int)",
    "return-without-value": "return without a value in a function that returns one",
}
WARNINGS = [w for w in CATEGORIES if w != "return-without-value"] + ["return-type"]
# Only some compilers know these; they are passed when the compiler accepts them.
OPTIONAL_NO_ERROR = ["return-mismatch"]
# -DM2CTX drops the PS1-only assembly includes (see include/include_asm.h).
FLAGS = (["-fsyntax-only", "-std=gnu89", "-fno-builtin", "-DM2CTX", "-Iinclude", "-Iinclude/sdk"]
         + [f"-W{w}" for w in WARNINGS] + [f"-Wno-error={w}" for w in WARNINGS])


def category(flag: str | None, message: str) -> str | None:
    """The category of a diagnostic, or None if it is not one we count."""
    if flag in CATEGORIES:
        return flag
    if flag in ("return-type", "return-mismatch") and ("with no value" in message or "should return a value" in message):
        return "return-without-value"
    return None


def describe(message: str) -> str:
    """A shorter reason for a file that does not compile."""
    if "unknown register name" in message:
        return "MIPS inline assembly (GTE macros): " + message
    return message


def compiler_flags(cc: list[str]) -> list[str]:
    """FLAGS plus the optional ones this compiler accepts."""
    extra = []
    for w in OPTIONAL_NO_ERROR:
        r = subprocess.run(cc + ["-x", "c", "-fsyntax-only", f"-Wno-error={w}", "-"], input="",
                           capture_output=True, text=True)
        if r.returncode == 0 and not r.stderr.strip():
            extra.append(f"-Wno-error={w}")
    return FLAGS + extra


def family(version_line: str) -> str:
    """gcc or clang: they reject different things, so each has its own baseline."""
    return "clang" if "clang" in version_line.lower() else "gcc"


DIAGNOSTIC = re.compile(r"^(?P<file>[^:\s]+\.[ch]):\d+:\d+: (?P<kind>warning|error): (?P<msg>.*?)(?: \[-W(?P<flag>[\w-]+)[^\]]*\])?$")


def check(cc: list[str], flags: list[str], version: str, path: str):
    """Compile one file; return (path, first error or None, Counter of categories)."""
    r = subprocess.run(cc + flags + [f"-DVERSION_{version.upper()}", path], cwd=REPO,
                       capture_output=True, text=True)
    counts = collections.Counter()
    first_error = None
    for line in r.stderr.split("\n"):
        m = DIAGNOSTIC.match(line)
        if not m:
            continue
        what = category(m.group("flag"), m.group("msg"))
        if what:
            counts[what] += 1
        elif m.group("kind") == "error" and first_error is None:
            first_error = f"{m.group('file')}: {describe(m.group('msg'))}"
    if r.returncode and first_error is None:
        first_error = (r.stderr.strip().split("\n") or ["compiler failed"])[-1]
    return path, first_error, counts


def read_baseline(path: pathlib.Path) -> set[str]:
    if not path.exists():
        return set()
    return {line.split("#")[0].strip() for line in path.read_text().split("\n")} - {""}


def ascii_reason(path: str, reason: str) -> str:
    """The reason as short plain ASCII: without the file name, the 'aka' type
    spellings or the compiler's curly quotes."""
    reason = reason.split(": ", 1)[1] if reason.startswith(path + ": ") else reason
    reason = reason.split(" {aka")[0].replace("\u2018", "'").replace("\u2019", "'")
    reason = reason.encode("ascii", "replace").decode()
    return reason if len(reason) <= 110 else reason[:107] + "..."


def write_baseline(path: pathlib.Path, failing: dict):
    lines = [f"# Files that do not yet compile for the host with {path.stem.rsplit('_', 1)[1]} (make native-check).",
             "# This list may only shrink; see tools/native_check/native_check.py.", ""]
    lines += [f"{p}  # {ascii_reason(p, failing[p])}" for p in sorted(failing)]
    path.write_text("\n".join(lines) + "\n")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--version", required=True)
    ap.add_argument("--cc", default="cc", help="host C compiler command (gcc, clang, ...)")
    ap.add_argument("--update-baseline", action="store_true", help="rewrite the baseline from this run")
    ap.add_argument("--verbose", action="store_true", help="list the warnings by file")
    ap.add_argument("files", nargs="+")
    args = ap.parse_args()
    cc = shlex.split(args.cc)
    compiler = subprocess.run(cc + ["--version"], capture_output=True, text=True).stdout.split("\n")[0]
    flags = compiler_flags(cc)

    with concurrent.futures.ThreadPoolExecutor() as pool:
        results = list(pool.map(lambda f: check(cc, flags, args.version, f), sorted(set(args.files))))

    failing = {p: e for p, e, _ in results if e}
    totals = collections.Counter()
    for _, _, counts in results:
        totals.update(counts)

    print(f"native-check ({args.version}, {compiler}): {len(results) - len(failing)}/{len(results)} files compile")
    for flag, label in CATEGORIES.items():
        print(f"  {totals[flag]:6}  {label}")
    if args.verbose:
        for path, _, counts in results:
            if counts:
                print(f"  {path}: " + ", ".join(f"{CATEGORIES[f]}: {n}" for f, n in counts.most_common()))

    baseline_path = HERE / f"baseline_{args.version}_{family(compiler)}.txt"
    if args.update_baseline:
        write_baseline(baseline_path, failing)
        print(f"wrote {baseline_path.relative_to(REPO)} ({len(failing)} files)")
        return
    if not baseline_path.exists():
        # No baseline for this compiler family: report, do not gate.
        for p in sorted(failing):
            print(f"  does not compile: {p}: {failing[p]}")
        print(f"(no {baseline_path.relative_to(REPO)}: nothing to compare against)")
        return
    baseline = read_baseline(baseline_path)
    new = sorted(set(failing) - baseline)
    fixed = sorted(baseline & {p for p, e, _ in results if not e})
    for p in fixed:
        print(f"  now compiles, remove from the baseline: {p}")
    for p in new:
        print(f"  NEW FAILURE: {p}: {failing[p]}")
    if new:
        print(f"native-check: {len(new)} file(s) stopped compiling for the host")
        sys.exit(1)


if __name__ == "__main__":
    main()
