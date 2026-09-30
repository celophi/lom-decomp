#!/usr/bin/env python3
"""Run data2c over every .data region and summarize what it found.

This is the survey behind the decomp cleanup: each row says how much of a
region is typed, how many stored pointers it has, and which problems the
declarations still have for a build with wider pointers. The PS1 build does
not need any of this to be clean.

Regions are processed the way `make DATA_AS_C=1` processes them. Data
assembly is read from the object a normal build leaves in build/<version>,
so run `make` for the version first.

Usage: report_all.py [--version us] [--verify] [--out working/data2c/report]
Generated C and JSON reports go under --out (git-ignored working/ by
default; the C holds game data, so do not commit it).
"""
from __future__ import annotations

import argparse
import concurrent.futures
import json
import pathlib
import subprocess
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import project  # noqa: E402

COLUMNS = [
    ("typed", "typed symbols"),
    ("undeclared", "symbol without a C declaration"),
    ("fn ptrs", "pointer to function"),
    ("data ptrs", "pointer to data"),
    ("addr as int", "integer field holding an address"),
    ("addr words", "address-like words not written as pointers"),
    ("ptr conflicts", "conflicting declarations that disagree about pointers"),
    ("clustered", "cluster members"),
    ("too big", "declared type larger than the symbol's extent"),
]


def run(job):
    version, image, asm, obj, out, verify, host = job
    # data.data.s -> data.data.c: only the .s goes, since splat names use dots
    stem = str(out / version / pathlib.Path(asm).relative_to(f"asm/{version}"))[:-len(".s")]
    pathlib.Path(stem).parent.mkdir(parents=True, exist_ok=True)
    report = pathlib.Path(stem + ".json")
    cmd = [sys.executable, str(project.REPO / "tools/data2c/data2c.py"), "--quiet", "--version", version,
           "--image", image, "--asm", asm, "-o", stem + ".c", "--report", str(report)]
    if obj is not None:
        cmd += ["--object", str(obj)]
    if verify:
        cmd.append("--verify")
    if host:
        cmd.append("--host")
    r = subprocess.run(cmd, cwd=project.REPO, capture_output=True, text=True)
    return job, r.returncode, r.stdout + r.stderr, report


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--version", action="append", help="us and/or jp (default: both)")
    ap.add_argument("--verify", action="store_true", help="also check each region round-trips")
    ap.add_argument("--host", action="store_true", help="write host output (with --verify: run hostcheck.py)")
    ap.add_argument("--out", default="working/data2c/report")
    ap.add_argument("--strict", action="store_true", help="exit nonzero if any region fails, is skipped or differs")
    args = ap.parse_args()
    out = project.REPO / args.out

    jobs, skipped, failed = [], [], []
    for version in args.version or ["us", "jp"]:
        for image, asm, obj in project.data_files(version):
            text = (project.REPO / asm).read_text(errors="replace")
            is_databin = any(line.startswith(".incbin") for line in text.split("\n"))
            if not is_databin and not obj.exists():
                print(f"skip {asm}: build {obj.relative_to(project.REPO)} first", file=sys.stderr)
                skipped.append(asm)
                continue
            jobs.append((version, image, asm, None if is_databin else obj, out, args.verify, args.host))

    with concurrent.futures.ThreadPoolExecutor(8) as pool:
        results = list(pool.map(run, jobs))

    totals = {label: 0 for _, label in COLUMNS}
    print("version | region | size | " + " | ".join(c for c, _ in COLUMNS) + " | ok")
    for (version, image, asm, _obj, _out, _verify, _host), code, output, report in results:
        if code or not report.exists():
            print(f"{version} | {asm} | FAILED: {output.strip().splitlines()[-1] if output.strip() else code}")
            failed.append(asm)
            continue
        data = json.loads(report.read_text())
        counts = data["counts"]
        for _, label in COLUMNS:
            totals[label] += counts.get(label, 0)
        if not args.verify:
            ok = "yes"
        elif args.host:
            line = next((l for l in output.split("\n") if l.startswith("HOSTCHECK:") and "fields compared" in l), "")
            ok = "yes" if line and " 0 differ" in line and "not found" not in line else ("NO " + line[11:]).strip()
        else:
            ok = "yes" if "reproduces the region exactly" in output else "NO"
        print(f"{version} | {pathlib.Path(asm).name} ({image}) | {data['size']:#x} | "
              + " | ".join(str(counts.get(label, 0)) for _, label in COLUMNS) + f" | {ok}")
        if ok != "yes":
            failed.append(asm)
    print("total | | | " + " | ".join(str(totals[label]) for _, label in COLUMNS) + " |")
    if args.strict and (failed or skipped):
        print(f"{len(failed)} region(s) failed, {len(skipped)} skipped", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
