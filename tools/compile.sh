#!/usr/bin/env bash
# Used for decomp permuter

INPUT="$(realpath "$1")"
OUTPUT="$(realpath "$3")"
TMPASM=$(mktemp --suffix=.s)

cd /staging

# Stage 1: Compile to assembly
/opt/psx-gcc/gcc -B/opt/psx-gcc/ -O2 -G0 -g -fsigned-char -Iinclude -S "$INPUT" -o "$TMPASM"

# Stage 2: Assemble with maspsx pipeline (like your build does)
cat "$TMPASM" | python3 tools/maspsx/maspsx.py --run-assembler -Iinclude -no-pad-sections --aspsx-version=2.77 -o "$OUTPUT"

# cleanup
rm "$TMPASM"