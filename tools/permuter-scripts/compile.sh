#!/usr/bin/env bash
# Used for decomp permuter

INPUT="$(realpath "$1")"
OUTPUT="$(realpath "$3")"
TMPASM=$(mktemp --suffix=.s)

cd /staging

# Stage 1: Compile to assembly
/opt/psx-gcc-2.8.0/gcc -B/opt/psx-gcc-2.8.0/ -O2 -G4 -msoft-float -gcoff -fsigned-char -Iinclude -S "$INPUT" -o "$TMPASM"

# Stage 2: Assemble with maspsx pipeline (like your build does)
cat "$TMPASM" | python3 tools/maspsx/maspsx.py --run-assembler -Iinclude -no-pad-sections --aspsx-version=2.77 --expand-div -o "$OUTPUT"

# cleanup
rm "$TMPASM"