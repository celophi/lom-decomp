#!/usr/bin/env python3
"""Compare the GNAME overlay ELF against the original disc BIN in DECOMPRESSED form.

GNAME has no working compressor, so we cannot reproduce the compressed BIN for a
SHA1 match the way verify-bins does. Instead we objcopy the linked ELF to its raw
loadable image (gname.raw) and compare it byte-for-byte against the decompressed
original overlay. GNAME is a work in progress, so this is an informative report
rather than a pass/fail gate; in particular the gname_data.c .data region should
already match completely.

Usage: python tools/verify_gname_raw.py <gname.raw> <GNAME.BIN>
"""

import importlib.util
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
VRAM_BASE = 0x80140000
# gname_data.c .data region (the tables built from C).
DATA_LO = 0x80142C98
DATA_HI = 0x8014F7B0


def _load_decompress():
    ext = REPO / "tools" / "splat_ext" / "decompress.py"
    spec = importlib.util.spec_from_file_location("decompress", ext)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod.decompress


def _diff_runs(a: bytes, b: bytes):
    """Return list of (start, end) half-open byte ranges where a != b."""
    runs = []
    n = min(len(a), len(b))
    i = 0
    while i < n:
        if a[i] != b[i]:
            j = i
            while j < n and a[j] != b[j]:
                j += 1
            runs.append((i, j))
            i = j
        else:
            i += 1
    return runs


def main() -> None:
    if len(sys.argv) != 3:
        sys.exit("Usage: python tools/verify_gname_raw.py <gname.raw> <GNAME.BIN>")
    raw = Path(sys.argv[1]).read_bytes()
    comp = Path(sys.argv[2]).read_bytes()

    # Byte 0 is the 0x01 compression-format tag; the stream starts at byte 1.
    dec = bytes(_load_decompress()(comp[1:]))
    n = min(len(raw), len(dec))

    match = sum(1 for i in range(n) if raw[i] == dec[i])
    print(f"raw ELF image:   {len(raw)} bytes  "
          f"(VRAM 0x{VRAM_BASE:08X}..0x{VRAM_BASE + len(raw):08X})")
    print(f"decompressed:    {len(dec)} bytes")
    if len(raw) != len(dec):
        print(f"NOTE: length differs by {abs(len(raw) - len(dec))} bytes; "
              f"comparing the first {n}.")
    print(f"overall match:   {match}/{n} = {100.0 * match / n:.2f}%")

    # Report the gname_data .data region specifically (our C-built tables).
    lo, hi = DATA_LO - VRAM_BASE, DATA_HI - VRAM_BASE
    d_raw, d_dec = raw[lo:hi], dec[lo:hi]
    d_match = sum(1 for x, y in zip(d_raw, d_dec) if x == y)
    print(f"gname_data .data (0x{DATA_LO:08X}..0x{DATA_HI:08X}): "
          f"{d_match}/{hi - lo} = {100.0 * d_match / (hi - lo):.2f}%  "
          f"{'[OK]' if d_raw == d_dec else '[DIFF]'}")

    runs = _diff_runs(raw, dec)
    if not runs and len(raw) == len(dec):
        print("[OK] ELF matches the decompressed original exactly.")
        return

    diff_bytes = sum(e - s for s, e in runs)
    print(f"\n{len(runs)} differing run(s), {diff_bytes} bytes total. First runs:")
    for s, e in runs[:15]:
        print(f"  0x{VRAM_BASE + s:08X}..0x{VRAM_BASE + e:08X}  ({e - s} bytes)")
    if len(runs) > 15:
        print(f"  ... and {len(runs) - 15} more")


if __name__ == "__main__":
    main()
