#!/usr/bin/env python3
"""Verify CHECKPS's linked image against the decompressed original overlay."""

import argparse
import hashlib
import importlib.util
from pathlib import Path


def load_decompress():
    path = Path(__file__).resolve().parent / "splat_ext" / "decompress.py"
    spec = importlib.util.spec_from_file_location("checkps_decompress", path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"could not load {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module.decompress


def sha1(data: bytes) -> str:
    return hashlib.sha1(data).hexdigest()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("built_raw", type=Path)
    parser.add_argument("original_bin", type=Path)
    args = parser.parse_args()

    built = args.built_raw.read_bytes()
    compressed = args.original_bin.read_bytes()
    target = bytes(load_decompress()(compressed[1:]))

    print(f"CHECKPS raw expected: {sha1(target)} ({len(target)} bytes)")
    print(f"CHECKPS raw actual:   {sha1(built)} ({len(built)} bytes)")
    if built == target:
        print("[OK] CHECKPS linked image matches the decompressed original")
        return

    common = min(len(built), len(target))
    first = next((index for index in range(common) if built[index] != target[index]), common)
    raise SystemExit(f"[FAIL] CHECKPS raw mismatch at offset 0x{first:X}")


if __name__ == "__main__":
    main()
