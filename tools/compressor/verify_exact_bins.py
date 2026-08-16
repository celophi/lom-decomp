#!/usr/bin/env python3
"""Regression verifier for the byte-exact Legend of Mana BIN compressor.

Each reference overlay in ``disc/BIN`` is decompressed with the project's
reference decoder (``tools/splat_ext/decompress.py``), recompressed with
``tools/compressor/compressor.py``, and compared byte-for-byte against the
original disc stream.  This exercises exactly the transformation the build
performs in ``mk/verification.mk`` without needing a toolchain or a linked ELF,
so it is the cheapest way to catch a compressor regression.

A disc overlay is laid out as::

    byte 0      0x01 compression-format tag
    byte 1..    the compressed stream itself

so the decoder is fed ``data[1:]`` and the encoder's output is compared against
``data[1:]``.

Usage:
    python3 tools/compressor/verify_exact_bins.py             # all overlays
    python3 tools/compressor/verify_exact_bins.py GOVER MOVIE # a named subset
"""

from __future__ import annotations

import argparse
import importlib.util
import sys
from pathlib import Path
from types import ModuleType

REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_BIN_DIR = REPO_ROOT / "disc" / "BIN"
COMPRESSOR_PATH = Path(__file__).resolve().parent / "compressor.py"
DECOMPRESSOR_PATH = REPO_ROOT / "tools" / "splat_ext" / "decompress.py"

# Byte 0 of every disc overlay selects the compression format; the stream that
# compressor.py produces and consumes begins at byte 1.
FORMAT_TAG = 0x01


def _load_module(name: str, path: Path) -> ModuleType:
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Unable to load Python module: {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _first_difference(expected: bytes, actual: bytes) -> int | None:
    """Return the first differing offset, or None when the streams are equal."""
    common = min(len(expected), len(actual))
    for offset in range(common):
        if expected[offset] != actual[offset]:
            return offset
    return common if len(expected) != len(actual) else None


def _select_references(bin_dir: Path, names: list[str]) -> list[Path]:
    if not names:
        return sorted(bin_dir.glob("*.BIN"))

    selected = []
    for name in names:
        stem = name.upper().removesuffix(".BIN")
        path = bin_dir / f"{stem}.BIN"
        if not path.is_file():
            raise SystemExit(f"No such reference overlay: {path}")
        selected.append(path)
    return selected


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Verify byte-exact recompression of the reference disc overlays."
    )
    parser.add_argument(
        "overlays",
        nargs="*",
        help="overlay names to check (default: every *.BIN in the BIN directory)",
    )
    parser.add_argument(
        "--bin-dir",
        type=Path,
        default=DEFAULT_BIN_DIR,
        help=f"directory holding the original *.BIN overlays (default: {DEFAULT_BIN_DIR})",
    )
    parser.add_argument(
        "--compressor",
        type=Path,
        default=COMPRESSOR_PATH,
        help="compressor module under test",
    )
    parser.add_argument(
        "--decompressor",
        type=Path,
        default=DECOMPRESSOR_PATH,
        help="reference decompressor module",
    )
    args = parser.parse_args()

    if not args.bin_dir.is_dir():
        parser.error(f"BIN directory not found: {args.bin_dir}")

    compressor = _load_module("lom_compressor_under_test", args.compressor)
    decompressor = _load_module("lom_decompressor_reference", args.decompressor)

    references = _select_references(args.bin_dir, args.overlays)
    if not references:
        parser.error(f"No *.BIN files found in {args.bin_dir}")

    failures = 0
    for reference_path in references:
        original = reference_path.read_bytes()
        if original[0] != FORMAT_TAG:
            failures += 1
            print(f"{reference_path.name:<12} SKIP  unexpected format tag 0x{original[0]:02X}")
            continue

        stream = original[1:]
        raw = bytes(decompressor.decompress(stream))
        recompressed = bytes(compressor.compress(raw))
        difference = _first_difference(stream, recompressed)

        if difference is None:
            print(
                f"{reference_path.name:<12} OK    "
                f"compressed={len(original):>7}  raw={len(raw):>8}"
            )
        else:
            failures += 1
            print(
                f"{reference_path.name:<12} FAIL  first_diff=0x{difference:X}  "
                f"expected={len(stream)}  actual={len(recompressed)}"
            )
        sys.stdout.flush()

    print()
    if failures:
        print(f"FAILED: {failures}/{len(references)} overlay(s) differed")
        return 1

    print(f"PASS: {len(references)}/{len(references)} overlays are byte-for-byte exact")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
