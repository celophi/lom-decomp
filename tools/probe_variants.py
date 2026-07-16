#!/usr/bin/env python3
"""Batch-compile and score C variants against a target function (P0 probe_variants).

Tests many small hypothesis edits (operand flips, temp insertion, decl reorder,
statement moves) in ONE pass instead of one diff round-trip each. The target
function is extracted and assembled once; every variant is compiled and
objdiff'd against it inside the lom-mcp container, and the results come back
as a ranked table.

Usage:
    python tools/probe_variants.py \
        --base working/func_80149948/code.c \
        --func func_80149948 \
        --asm asm/overlays/menu/menu.s \
        --toolchain gcc272_cdk \
        --variants working/func_80149948/probes.json

The variants file is JSON:
    [
      {"name": "flip_current_y",
       "edits": [["(nav_x_packed >> 15) | ((nav_y_packed & 0xFF) << 1)",
                  "((nav_y_packed & 0xFF) << 1) | (nav_x_packed >> 15)"]]},
      {"name": "both_flips",
       "edits": [["...old...", "...new..."], ["...old2...", "...new2..."]]}
    ]

Each edit is an exact-string [old, new] replacement applied to the base file;
a variant fails fast (reported, not scored) if any `old` is not found exactly
once. The unmodified base is always scored too, as `BASE`.

Requires the lom-mcp container to be running (it is whenever the MCP is up).
"""

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

CONTAINER = "lom-mcp"
STAGING = "/staging"
LOM = "/lom"
INCLUDE = f"-I{STAGING}/include -I{STAGING}/include/psyq"
AS_INCLUDE = f"{INCLUDE} -I{STAGING}"
MASPSX = f"python3 {LOM}/tools/lom-dev-mcp/vendor/maspsx/maspsx.py"
PRELUDE = f"{LOM}/tools/lom-dev-mcp/scripts/permuter/prelude.inc"
EXTRACT = f"python3 {LOM}/tools/lom-dev-mcp/scripts/extract_func.py"
OBJDIFF = f"{LOM}/tools/lom-dev-mcp/bin/objdiff-cli-linux-x86_64"
SUMMARIZE = f"python3 {LOM}/tools/lom-dev-mcp/scripts/summarize_diff.py"

TOOLCHAINS = {
    "gcc272_cdk": {
        "cc": (
            f"/opt/psx-gcc-2.7.2-cdk/gcc -B/opt/psx-gcc-2.7.2-cdk/ "
            f"-O2 -G0 -msoft-float -gcoff {INCLUDE} -S {{src}} -o {{asm}}"
        ),
        "as": (
            f"cat {{asm}} | {MASPSX} --run-assembler -no-pad-sections "
            f"--aspsx-version=2.67 --expand-div {AS_INCLUDE} -o {{obj}}"
        ),
        "target_aspsx": "2.67",
    },
    "gcc280_g0": {
        "cc": (
            f"/opt/psx-gcc-2.8.0/gcc -B/opt/psx-gcc-2.8.0/ "
            f"-O2 -G0 -gcoff -msoft-float -fsigned-char {INCLUDE} -S {{src}} -o {{asm}}"
        ),
        "as": (
            f"cat {{asm}} | {MASPSX} --run-assembler -no-pad-sections "
            f"--aspsx-version=2.77 --expand-div {AS_INCLUDE} -o {{obj}}"
        ),
        "target_aspsx": "2.77",
    },
    "gcc280_g4": {
        "cc": (
            f"/opt/psx-gcc-2.8.0/gcc -B/opt/psx-gcc-2.8.0/ "
            f"-O2 -G4 -gcoff -msoft-float -fsigned-char {INCLUDE} -S {{src}} -o {{asm}}"
        ),
        "as": (
            f"cat {{asm}} | {MASPSX} --run-assembler -no-pad-sections "
            f"--aspsx-version=2.77 --expand-div {AS_INCLUDE} -o {{obj}}"
        ),
        "target_aspsx": "2.77",
    },
}


def apply_edits(base_text, edits, name):
    """Apply exact-string replacements; each old must occur exactly once."""
    text = base_text
    for i, (old, new) in enumerate(edits):
        n = text.count(old)
        if n != 1:
            return None, f"edit {i}: old-string found {n} times (need exactly 1)"
    for old, new in edits:
        text = text.replace(old, new, 1)
    return text, None


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--base", required=True, help="Base C file (repo-relative)")
    ap.add_argument("--func", required=True, help="Function/symbol name")
    ap.add_argument("--asm", required=True, help="Splat asm file with the target")
    ap.add_argument("--toolchain", default="gcc272_cdk", choices=sorted(TOOLCHAINS))
    ap.add_argument("--variants", required=True, help="JSON variants file")
    ap.add_argument("--keep", action="store_true", help="Keep probe .c files")
    args = ap.parse_args()

    repo = Path(__file__).resolve().parent.parent
    base_path = repo / args.base
    base_text = base_path.read_text()
    variants = json.loads((repo / args.variants).read_text())

    tc = TOOLCHAINS[args.toolchain]
    probes_dir = base_path.parent / "probes"
    probes_dir.mkdir(exist_ok=True)
    probes_rel = probes_dir.relative_to(repo).as_posix()

    # Materialize variant sources on the host (visible in-container via /lom).
    jobs = [("BASE", base_text)]
    skipped = []
    for v in variants:
        text, err = apply_edits(base_text, [tuple(e) for e in v["edits"]], v["name"])
        if err:
            skipped.append((v["name"], err))
        else:
            jobs.append((v["name"], text))
    for name, text in jobs:
        (probes_dir / f"{name}.c").write_text(text, newline="\n")

    # One in-container pass: extract+assemble the target once, then loop.
    work = f"{STAGING}/mcp-work/probe_{args.func}"
    target_s = f"{work}/target.s"
    target_o = f"{work}/target.o"
    target_cmd = (
        f"cat {PRELUDE} {target_s} | {MASPSX} --run-assembler {AS_INCLUDE} "
        f"-no-pad-sections --aspsx-version={tc['target_aspsx']} --expand-div -o {target_o}"
    )
    lines = [
        "set -u",
        f"mkdir -p {work}",
        f"cd {STAGING}",
        f"{EXTRACT} {args.asm} {args.func} --rel-offsets -o {target_s} || exit 3",
        f"{target_cmd} || exit 4",
    ]
    for name, _ in jobs:
        # Old gcc/cpp EOVERFLOWs stat()ing files on the Windows mount; copy
        # each probe into the container-local work dir first.
        src = f"{work}/{name}.c"
        asm = f"{work}/v.s"
        obj = f"{work}/v.o"
        dj = f"{work}/v.json"
        cc = f"cp {LOM}/{probes_rel}/{name}.c {src} && " + tc["cc"].format(src=src, asm=asm)
        asm_cmd = tc["as"].format(asm=asm, obj=obj)
        lines.append(
            f"rm -f {asm} {obj} {dj}; "
            f"if {cc} 2>{work}/cc.err && {asm_cmd} 2>>{work}/cc.err && "
            f"{OBJDIFF} diff -1 {target_o} -2 {obj} -o {dj} --format json; then "
            f"echo \"@@VARIANT {name}\"; {SUMMARIZE} {dj} {args.func} --no-density "
            f"| grep -E 'MATCH_PERCENT=|^size:|^rows:' ; "
            f"else echo \"@@VARIANT {name}\"; echo 'COMPILE_FAILED'; "
            f"tail -3 {work}/cc.err; fi"
        )
    script = "\n".join(lines)

    # Send as bytes so Windows text-mode piping cannot inject CRLFs.
    rb = subprocess.run(
        ["docker", "exec", "-i", CONTAINER, "bash", "-s"],
        input=script.encode("utf-8"), capture_output=True, timeout=1200,
    )

    class R:
        returncode = rb.returncode
        stdout = rb.stdout.decode("utf-8", "replace")
        stderr = rb.stderr.decode("utf-8", "replace")

    r = R()
    if r.returncode not in (0,):
        sys.stderr.write(r.stderr[-2000:] + "\n")
        if "@@VARIANT" not in r.stdout:
            sys.exit(f"probe run failed (exit {r.returncode})")

    # Parse per-variant results.
    results = []
    for chunk in r.stdout.split("@@VARIANT ")[1:]:
        body = chunk.splitlines()
        name = body[0].strip()
        text = "\n".join(body[1:])
        if "COMPILE_FAILED" in text:
            results.append((name, None, text.replace("COMPILE_FAILED", "").strip()))
            continue
        pct = re.search(r"MATCH_PERCENT=([\d.]+)", text)
        rows = re.search(r"rows:\s*exact (\d+) \| argdiff (\d+) \| replace (\d+) "
                         r"\| target-only (\d+) \| yours-only (\d+)", text)
        size = re.search(r"yours (\d+) bytes / (\d+) insns", text)
        if pct and rows:
            results.append((name, {
                "pct": float(pct.group(1)),
                "exact": int(rows.group(1)), "argdiff": int(rows.group(2)),
                "replace": int(rows.group(3)), "tonly": int(rows.group(4)),
                "yonly": int(rows.group(5)),
                "insns": int(size.group(2)) if size else -1,
            }, None))
        else:
            results.append((name, None, "unparseable summary"))

    ok = [(n, d) for n, d, _ in results if d]
    bad = [(n, e) for n, d, e in results if not d]
    base = next((d for n, d in ok if n == "BASE"), None)
    ok.sort(key=lambda x: (-x[1]["exact"], -x[1]["pct"]))

    print(f"{'variant':<28} {'pct':>7} {'exact':>5} {'arg':>4} {'rep':>4} "
          f"{'t-o':>4} {'y-o':>4} {'insn':>5}  delta-exact")
    for name, d in ok:
        de = d["exact"] - base["exact"] if base else 0
        mark = f"{de:+d}" if name != "BASE" else "--"
        print(f"{name:<28} {d['pct']:>7.3f} {d['exact']:>5} {d['argdiff']:>4} "
              f"{d['replace']:>4} {d['tonly']:>4} {d['yonly']:>4} {d['insns']:>5}  {mark}")
    for name, err in bad:
        print(f"{name:<28} FAILED: {err[:120]}")
    for name, err in skipped:
        print(f"{name:<28} SKIPPED: {err}")

    if not args.keep:
        for name, _ in jobs:
            (probes_dir / f"{name}.c").unlink(missing_ok=True)
        try:
            probes_dir.rmdir()
        except OSError:
            pass


if __name__ == "__main__":
    main()
