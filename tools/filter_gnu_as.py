#!/usr/bin/env python3
"""Filter compiler-only label keepers from legacy GCC assembly output."""

import argparse
import re
import sys


KEEP_LABEL = re.compile(r"^keep_[A-Za-z0-9_.$]+:$")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--drop-keeper-data", action="store_true")
    parser.add_argument("--lower-align-3", action="store_true")
    args = parser.parse_args()

    output: list[str] = []
    skip_word = False

    for line in sys.stdin:
        stripped = line.strip()

        # This assembler interprets .align as a power of two. The original
        # CHECKPS jump table begins four bytes into its object, so GCC's
        # emitted 8-byte alignment would insert four bytes not in the target.
        if args.lower_align_3 and stripped == ".align\t3":
            line = line.replace(".align\t3", ".align\t2")
            stripped = line.strip()

        if skip_word:
            if not stripped.startswith(".word"):
                raise SystemExit(f"expected .word after keeper label, got: {stripped}")
            skip_word = False
            continue

        if args.drop_keeper_data and KEEP_LABEL.fullmatch(stripped):
            if output and output[-1].strip().startswith(".align"):
                output.pop()
            skip_word = True
            continue

        output.append(line)

    if skip_word:
        raise SystemExit("keeper label at end of input")

    sys.stdout.writelines(output)


if __name__ == "__main__":
    main()
