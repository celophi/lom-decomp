# ============================================================================
# Legend of Mana PSX Decompilation — Makefile
# ============================================================================
#
# Build pipeline (what happens when you compile a .c file):
#
#   1. GCC 2.8.0 compiles your C code to MIPS assembly text      (gcc -S)
#   2. maspsx translates GCC's asm syntax into Sony ASPSX format  (maspsx.py)
#   3. The system assembler turns that into a .o object file      (as)
#   4. The linker combines all .o files into an ELF executable    (ld)
#
#   Steps 2+3 are combined: maspsx.py --run-assembler does both in one pass.
#
# Why /staging?
#   The Docker container mounts your project at /lom (a Windows filesystem).
#   The legacy 32-bit PSX compiler/preprocessor cannot stat the bind mount's
#   inode values and fails with EOVERFLOW. Copying inputs to /staging gives the
#   toolchain native Linux filesystem metadata. Staging also normalizes CRLF
#   text inputs to LF without changing files on the host.
#
# Quick reference:
#   make              — build the main SLUS ELF
#   make bin          — also produce a raw .bin binary
#   make checkps      — build just the CHECKPS overlay
#   make overlays     — build all overlays
#   make everything   — build SLUS + all overlays
#   make clean        — remove all build artifacts
#   make recopy       — force re-copy of source files to /staging
#
# ============================================================================


# ─── Paths ──────────────────────────────────────────────────────────────────────
#
# STAGING  — native Linux filesystem path inside the container where we compile.
#            Everything under src/, asm/, include/, etc. is mirrored here.
# MOUNT    — where Docker mounts the host project (not stat-compatible with the
#            legacy 32-bit compiler/preprocessor).

STAGING      := /staging
MOUNT        := /lom

GAME         := SLUS_010.13
TARGET       := $(STAGING)/build/$(GAME).elf
BIN          := build/$(GAME).bin

# Directories (relative — used for both the mount and staging mirror)
SRC_DIR      := src
ASM_DIR      := asm

.DEFAULT_GOAL := all

# Original ROM overlay BINs live here in CI (copied from the container's /rom/BIN).
ROM_BIN_DIR       := disc/BIN

# Manifest of overlays whose rebuilt+compressed BIN matches the original ROM.
# Read by tools/objdiff/generate_objdiff_config.py to mark units complete.
COMPLETE_MANIFEST := build/complete_overlays.txt

# Recursive wildcard helper used while overlay rules are expanded.
rwildcard = $(foreach d,$(wildcard $1/*),$(call rwildcard,$d,$2)) \
            $(filter $(subst *,%,$2),$1)

include mk/staging.mk
include mk/splat.mk
include mk/toolchains.mk
include mk/main.mk
include mk/overlay-registry.mk
include mk/overlays.mk
include mk/analysis.mk
include mk/verification.mk

.PHONY: clean

clean:
	rm -rf build/ $(STAGING)
