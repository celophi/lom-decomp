#!/usr/bin/env bash
# Wrapper for maspsx --run-assembler that reorders arguments.
#
# import.py's compile_asm calls assemblers as: [flags] input.s -o output.o
# But maspsx expects the input file LAST: [flags] -o output.o input.s
# (it pops the last unknown arg as the input file when not reading from stdin)
#
# This wrapper accepts the import.py convention and reorders for maspsx.
set -e

all=("$@")
n=${#all[@]}

# Last 3 args are: input.s -o output.o
input="${all[$((n-3))]}"
output="${all[$((n-1))]}"
flags=("${all[@]:0:$((n-3))}")

cd /staging
exec python3 tools/maspsx/maspsx.py "${flags[@]}" -o "$output" "$input"
