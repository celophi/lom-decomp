#!/usr/bin/env bash
# permute-setup.sh — set up a decomp-permuter directory for one function.
#
# Usage (run from /staging):
#   tools/permuter-scripts/permute-setup.sh <c_file> <asm_file> <func_name> [options]
#
# Options:
#   --toolchain  gcc280 | gcc280-g4 | gcc280-g4-noexpanddiv | cdk | gcc260 | gnuas  (default: gcc280)
#   --out-dir    output directory                            (default: /permute/<func_name>)
#
# Examples:
#   tools/permuter-scripts/permute-setup.sh src/cdrom.c asm/cdrom.s cdrom_complete_command
#   tools/permuter-scripts/permute-setup.sh src/overlays/menu/unk1.c asm/overlays/menu/menu.s menu_draw_window --toolchain cdk
#   tools/permuter-scripts/permute-setup.sh src/cdrom.c asm/cdrom.s cdrom_complete_command --out-dir /permute/test1

set -euo pipefail

# ── Defaults ────────────────────────────────────────────────────────────────
TOOLCHAIN="gcc280"
OUT_DIR=""

# ── Arg parsing ─────────────────────────────────────────────────────────────
if [[ $# -lt 3 ]]; then
    echo "Usage: $0 <c_file> <asm_file> <func_name> [--toolchain CHAIN] [--out-dir DIR]" >&2
    exit 1
fi

C_FILE="$1"
ASM_FILE="$2"
FUNC_NAME="$3"
shift 3

while [[ $# -gt 0 ]]; do
    case "$1" in
        --toolchain) TOOLCHAIN="$2"; shift 2 ;;
        --out-dir)   OUT_DIR="$2";   shift 2 ;;
        *) echo "Unknown argument: $1" >&2; exit 1 ;;
    esac
done

[[ -z "$OUT_DIR" ]] && OUT_DIR="/permute/${FUNC_NAME}"

# ── Validate inputs ──────────────────────────────────────────────────────────
[[ -f "$C_FILE"   ]] || { echo "ERROR: C file not found: $C_FILE" >&2; exit 1; }
[[ -f "$ASM_FILE" ]] || { echo "ERROR: asm file not found: $ASM_FILE" >&2; exit 1; }

# ── Toolchain definitions ────────────────────────────────────────────────────
# Each toolchain sets:
#   CC_CMD       — compiler invocation up to (but not including) the input file
#   AS_CMD       — assembler command (appended with: <input.s> -o <output.o>)
#   COMPILER_TYPE — passed to permuter for randomisation weights

case "$TOOLCHAIN" in
    gcc280)
        CC_CMD="/opt/psx-gcc-2.8.0/gcc -B/opt/psx-gcc-2.8.0/ -O2 -G0 -gcoff -fsigned-char -fno-builtin -Iinclude -Iinclude/psyq -S -o -"
        AS_PIPE="python3 tools/maspsx/maspsx.py --run-assembler -Iinclude -Iinclude/psyq -no-pad-sections --aspsx-version=2.77 --expand-div"
        AS_CMD="bash tools/permuter-scripts/maspsx_asm.sh --run-assembler -Iinclude -Iinclude/psyq -no-pad-sections --aspsx-version=2.77 --expand-div"
        COMPILER_TYPE="gcc"
        ;;
    gcc280-g4)
        CC_CMD="/opt/psx-gcc-2.8.0/gcc -B/opt/psx-gcc-2.8.0/ -O2 -G4 -gcoff -fsigned-char -Iinclude -Iinclude/psyq -S -o -"
        AS_PIPE="python3 tools/maspsx/maspsx.py --run-assembler -Iinclude -Iinclude/psyq -no-pad-sections --aspsx-version=2.77 --expand-div"
        AS_CMD="bash tools/permuter-scripts/maspsx_asm.sh --run-assembler -Iinclude -Iinclude/psyq -no-pad-sections --aspsx-version=2.77 --expand-div"
        COMPILER_TYPE="gcc"
        ;;
    gcc280-g4-noexpanddiv)
        # Same as gcc280-g4 but WITHOUT --expand-div: some -G4 objects use bare
        # `div $zero,...` with no div-by-zero / overflow break checks.
        CC_CMD="/opt/psx-gcc-2.8.0/gcc -B/opt/psx-gcc-2.8.0/ -O2 -G4 -gcoff -fsigned-char -Iinclude -Iinclude/psyq -S -o -"
        AS_PIPE="python3 tools/maspsx/maspsx.py --run-assembler -Iinclude -Iinclude/psyq -no-pad-sections --aspsx-version=2.77"
        AS_CMD="bash tools/permuter-scripts/maspsx_asm.sh --run-assembler -Iinclude -Iinclude/psyq -no-pad-sections --aspsx-version=2.77"
        COMPILER_TYPE="gcc"
        ;;
    cdk)
        CC_CMD="/opt/psx-gcc-2.7.2-cdk/gcc -B/opt/psx-gcc-2.7.2-cdk/ -O2 -G0 -msoft-float -gcoff -Iinclude -Iinclude/psyq -S -o -"
        AS_PIPE="python3 tools/maspsx/maspsx.py --run-assembler -Iinclude -Iinclude/psyq -no-pad-sections --aspsx-version=2.67 --expand-div"
        AS_CMD="bash tools/permuter-scripts/maspsx_asm.sh --run-assembler -Iinclude -Iinclude/psyq -no-pad-sections --aspsx-version=2.67 --expand-div"
        COMPILER_TYPE="gcc"
        ;;
    gcc260)
        CC_CMD="/opt/psx-gcc-2.6.0/gcc -B/opt/psx-gcc-2.6.0/ -O2 -G0 -gcoff -msoft-float -Iinclude -Iinclude/psyq -S -o -"
        AS_PIPE="python3 tools/maspsx/maspsx.py --run-assembler -Iinclude -Iinclude/psyq -no-pad-sections --aspsx-version=2.34 --expand-div"
        AS_CMD="bash tools/permuter-scripts/maspsx_asm.sh --run-assembler -Iinclude -Iinclude/psyq -no-pad-sections --aspsx-version=2.34 --expand-div"
        COMPILER_TYPE="gcc"
        ;;
    gnuas)
        CC_CMD="/opt/psx-gcc-2.7.2-gnuas/gcc -B/opt/psx-gcc-2.7.2-gnuas/ -O2 -G0 -Iinclude -Iinclude/psyq -S"
        AS_PIPE="/opt/psx-gcc-2.7.2-gnuas/as -O -EL"
        AS_CMD="/opt/psx-gcc-2.7.2-gnuas/as -O -EL"
        COMPILER_TYPE="gcc"
        ;;
    *)
        echo "ERROR: unknown toolchain '$TOOLCHAIN'" >&2
        echo "       valid options: gcc280 gcc280-g4 gcc280-g4-noexpanddiv cdk gcc260 gnuas" >&2
        exit 1
        ;;
esac

echo "==> Function:  $FUNC_NAME"
echo "==> C file:    $C_FILE"
echo "==> Asm file:  $ASM_FILE"
echo "==> Toolchain: $TOOLCHAIN"
echo "==> Output:    $OUT_DIR"
echo ""

# ── Step 1: extract function asm ─────────────────────────────────────────────
TARGET_S="/tmp/${FUNC_NAME}.s"
echo "[1/3] Extracting $FUNC_NAME from $ASM_FILE..."
python3 tools/permuter-scripts/extract_func.py "$ASM_FILE" "$FUNC_NAME" -o "$TARGET_S"

# ── Step 2: write permuter_settings.toml ────────────────────────────────────
# Must live at /staging/permuter_settings.toml so import.py can find the
# project root by walking up from the C file (Makefile isn't staged to /staging).
SETTINGS_TOML="/staging/permuter_settings.toml"
echo "[2/3] Writing settings ($TOOLCHAIN)..."

# import.py uses | as a pipe separator in compiler_command.
# Left of | = gcc (gets $INPUT inserted before |)
# Right of | = assembler (gets -o $OUTPUT appended at end)
# gnuas doesn't pipe, so we write a custom compile.sh after import.py runs.
cat > "$SETTINGS_TOML" <<EOF
compiler_type = "${COMPILER_TYPE}"
asm_prelude_file = "tools/permuter-scripts/prelude.inc"
objdump_command = "mipsel-linux-gnu-objdump -drz"

compiler_command = "${CC_CMD} | ${AS_PIPE}"
assembler_command = "${AS_CMD}"
EOF

# ── Step 3: run import.py ────────────────────────────────────────────────────
echo "[3/3] Running import.py..."
mkdir -p "$OUT_DIR"

# import.py creates its output under nonmatchings/ relative to the root it finds.
# We capture the output directory it prints so we can move it to OUT_DIR.
IMPORT_OUT=$(python3 tools/decomp-permuter/import.py \
    "$C_FILE" \
    "$TARGET_S" \
    --settings "$SETTINGS_TOML" \
    --keep 2>&1 | tee /dev/stderr | grep "^Done. Imported into" | sed 's/Done. Imported into //')

if [[ -z "$IMPORT_OUT" ]]; then
    echo "" >&2
    echo "ERROR: import.py did not report a successful output directory." >&2
    echo "       Check the output above for errors." >&2
    exit 1
fi

# Move from nonmatchings/<func>/ to OUT_DIR
if [[ "$IMPORT_OUT" != "$OUT_DIR" ]]; then
    rm -rf "$OUT_DIR"
    mv "$IMPORT_OUT" "$OUT_DIR"
fi

# ── gnuas: replace compile.sh with a two-step version ───────────────────────
if [[ "$TOOLCHAIN" == "gnuas" ]]; then
    cat > "$OUT_DIR/compile.sh" <<'GNUAS_EOF'
#!/usr/bin/env bash
INPUT="$(realpath "$1")"
OUTPUT="$(realpath "$3")"
TMPASM=$(mktemp --suffix=.s)
cd /staging
/opt/psx-gcc-2.7.2-gnuas/gcc -B/opt/psx-gcc-2.7.2-gnuas/ -O2 -G0 -Iinclude -Iinclude/psyq -S "$INPUT" -o "$TMPASM"
/opt/psx-gcc-2.7.2-gnuas/as -O -EL -o "$OUTPUT" "$TMPASM"
rm "$TMPASM"
GNUAS_EOF
    chmod +x "$OUT_DIR/compile.sh"
    echo "(gnuas: replaced compile.sh with two-step version)"
fi

# ── Cleanup ──────────────────────────────────────────────────────────────────
rm -f "$TARGET_S"
# Leave SETTINGS_TOML in place — import.py needs it to anchor root detection.
# It's ephemeral (/staging is a container-only dir) so no cleanup needed.

echo ""
echo "Done! Permuter directory ready at: $OUT_DIR"
echo ""
echo "To run the permuter:"
echo "  python3 tools/decomp-permuter/permuter.py -j\$(nproc) --best-only $OUT_DIR"
