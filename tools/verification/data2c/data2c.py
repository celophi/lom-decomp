#!/usr/bin/env python3
"""Generate typed C for one of the game's .data regions.

The decomp keeps game data as splat output: blobs (databin) or data
assembly. That is fine for the PS1 build, but a port whose pointers are wider
than four bytes needs the data as C, so the compiler can lay out every stored
pointer itself (the approach sm64 and sotn-decomp take). This tool writes that
C at build time from the user's own game data; nothing it produces is
committed.

For the PS1 build (`make DATA_AS_C=1`) the output must be exact: GCC 2.8 has
to place every byte and relocation where the original had it. For a native
build, stored pointers must be real pointer initializers. How well the second
works depends on the decomp's declarations; --report lists where they fall
short.

Steps (see the modules for details):
  symbols.py       names and addresses, from the files the linker uses
  region.py        the region's bytes, and its relocations when assembled
  declarations.py  each symbol's type, from the image's own C with libclang
  writer.py        the C: structural types, initializers, GCC 2.8 placement
  verify.py        --verify: compile for mipsel and compare with the original
  hostcheck.py     --host --verify: compile for x86-64, compare every field's value

Usage:
  data2c.py --version us --image field --asm asm/us/overlays/field/data/field_data.s -o out.c
  data2c.py --version us --image wmap --asm asm/us/overlays/wmap/data/data.data.s \\
            --object build/us/overlays/wmap/asm/us/overlays/wmap/data/data.data.o -o out.c --verify
"""
from __future__ import annotations

import argparse
import bisect
import glob
import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import project  # noqa: E402
from declarations import Declarations, clang_args  # noqa: E402
from region import load_assembled, load_databin  # noqa: E402
from report import Report  # noqa: E402
from symbols import SymbolTable  # noqa: E402
from hostcheck import hostcheck  # noqa: E402
from verify import verify  # noqa: E402
from writer import RegionWriter  # noqa: E402


def parse_args():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--version", required=True, help="game version: us, jp")
    ap.add_argument("--image", required=True, help=f"'{project.EXECUTABLE}' or an overlay name, e.g. field")
    ap.add_argument("--asm", required=True, help="the splat data .s for the region")
    ap.add_argument("--object", help="the .s assembled (data assembly only): gives exact relocations")
    ap.add_argument("-o", "--out", required=True, help="C file to write")
    ap.add_argument("--verify", action="store_true", help="check the output reproduces the region")
    ap.add_argument("--host", action="store_true",
                    help="write the data for a host compiler (no PS1 padding or clusters; see writer.py)")
    ap.add_argument("--report", help="write what was found as JSON to this file")
    ap.add_argument("--quiet", action="store_true", help="do not print the report")
    return ap.parse_args()


def unconverted_address_words(region, symbols, writer) -> dict:
    """Aligned words in a databin whose value is a symbol's address but that the
    declarations did not make pointers, by owning symbol. Some are real pointers
    behind integer or byte declarations, some just look like addresses."""
    owners = {}
    starts = symbols.in_range(region.start, region.end)
    for addr in range(region.start, region.end - 3, 4):
        if addr in writer.inits.pointer_words or not symbols.names_at(region.word(addr)):
            continue
        i = bisect.bisect_right(starts, addr) - 1
        name = symbols.names_at(starts[i])[0] if i >= 0 else "?"
        owners[name] = owners.get(name, 0) + 1
    return owners


def main():
    args = parse_args()
    repo = project.REPO

    symbols = SymbolTable()
    for path in project.symbol_files(args.version, args.image):
        symbols.load(repo / path)
    if args.object:
        region = load_assembled(repo, args.object, args.asm, symbols)
    else:
        region = load_databin(repo, args.asm, symbols)

    sources = sorted({p for g in project.source_globs(args.image) for p in glob.glob(str(repo / g), recursive=True)})
    wanted = {n for a in symbols.in_range(region.start, region.end) for n in symbols.names_at(a)}
    decls = Declarations(sources, wanted, clang_args(args.version, project.include_dirs(args.image)))
    symbols.add_function_names(decls.function_names)

    report = Report(args.asm)
    writer = RegionWriter(region, symbols, decls, report, host=args.host)
    out = repo / args.out
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(writer.generate())

    if not region.from_object:
        left = unconverted_address_words(region, symbols, writer)
        if left:
            report.summary("address-like words not written as pointers", sum(left.values()),
                           (f"{name}: {n}" for name, n in sorted(left.items(), key=lambda kv: -kv[1])))
    if not args.quiet:
        report.print(f"{args.asm}: {len(region.data):#x} bytes at {region.start:#x}, "
                     f"{len(symbols.in_range(region.start, region.end))} symbol addresses, {len(sources)} source files")
    if args.report:
        report.save(repo / args.report, image=args.image, version=args.version,
                    start=region.start, size=len(region.data))
    if args.verify and args.host:
        if not hostcheck(out, region, symbols, writer, sources, args.version, project.include_dirs(args.image)):
            sys.exit(1)
    elif args.verify and not verify(out, region, symbols):
        sys.exit(1)


if __name__ == "__main__":
    main()
