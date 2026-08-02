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
#   GCC 2.8.0 can't write files there (it fails to stat() mounted paths).
#   So we copy everything to /staging (a native Linux ext4 path) first,
#   compile there, then copy the results back to build/.
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
# STAGING  — native Linux directory inside the container where we compile.
#            Everything under src/, asm/, include/, etc. is mirrored here.
# MOUNT    — where Docker mounts the host project (read-only for GCC).

STAGING      := /staging
MOUNT        := /lom

GAME         := SLUS_010.13
TARGET       := $(STAGING)/build/$(GAME).elf
BIN          := build/$(GAME).bin

# Directories (relative — used for both the mount and staging mirror)
SRC_DIR      := src
ASM_DIR      := asm

.DEFAULT_GOAL := all

# ─── Staging Sentinel ──────────────────────────────────────────────────────────
#
# A sentinel file tracks the last successful staging operation. Its
# prerequisites include every staged input, so host edits/additions/deletions
# automatically refresh /staging. Run `make recopy` to force a refresh.

COPY_SENTINEL := $(STAGING)/.sources_copied

# Project inputs needed by the Make build.
STAGE_PATHS := \
	src \
	asm \
	include \
	linker \
	assets \
	tools/maspsx

# These paths are wholly managed by the Makefile. Replacing them removes files
# deleted on the host while preserving /staging/build, /staging/mcp-work, and
# any tool-specific staging owned by developer tooling.
STAGE_MANAGED_PATHS := src asm include linker assets tools/maspsx

# Configuration shared by build, analysis, and verification pipelines.
SPLAT_CONFIGS := config/$(GAME).yaml $(wildcard config/overlays/*.yaml)

# Use every processor available to the current host/container by default.
# SPLAT_JOBS remains overridable for constrained environments.
SPLAT_JOBS ?= $(shell nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)

# Original ROM overlay BINs live here in CI (copied from the container's /rom/BIN).
ROM_BIN_DIR       := disc/BIN

# Manifest of overlays whose rebuilt+compressed BIN matches the original ROM.
# Read by tools/objdiff/generate_objdiff_config.py to mark units complete.
COMPLETE_MANIFEST := build/complete_overlays.txt

# Recursive wildcard helper used while overlay rules are expanded.
rwildcard = $(foreach d,$(wildcard $1/*),$(call rwildcard,$d,$2)) \
            $(filter $(subst *,%,$2),$1)

# A single find traversal is substantially faster on the Windows bind mount
# than recursively expanding one Make wildcard per directory. Directories are
# included so adding or deleting a staged file invalidates the sentinel.
STAGE_INPUTS := Makefile $(STAGE_PATHS) \
	$(shell find $(STAGE_PATHS) -print 2>/dev/null)

include mk/toolchains.mk
include mk/main.mk
include mk/overlay-registry.mk
include mk/overlays.mk
include mk/analysis.mk

.PHONY: clean recopy splat

clean:
	rm -rf build/ $(STAGING)

recopy:
	rm -f $(COPY_SENTINEL)
	$(MAKE) $(COPY_SENTINEL)

splat:
	@printf '%s\n' $(SPLAT_CONFIGS) | xargs -n 1 -P $(SPLAT_JOBS) splat split

# ============================================================================
#  Staging (copy sources into /staging)
# ============================================================================
#
# Replaces the managed roots with the paths listed above, then normalizes every
# compiler/linker/shell input to LF. The sentinel is only written after every
# copy and conversion succeeds.

$(COPY_SENTINEL): $(STAGE_INPUTS)
	@echo "Staging source files to $(STAGING)..."
	@set -eu; \
		mkdir -p "$(STAGING)"; \
		staging_abs=$$(readlink -f "$(STAGING)"); \
		if [ -z "$$staging_abs" ] || [ "$$staging_abs" = "/" ]; then \
			echo "Refusing unsafe staging path: '$(STAGING)'" >&2; \
			exit 1; \
		fi; \
		rm -f "$$staging_abs/.sources_copied"; \
		for path in $(STAGE_PATHS); do \
			if [ ! -e "$$path" ]; then \
				echo "Missing required staging input: $$path" >&2; \
				exit 1; \
			fi; \
		done; \
		for path in $(STAGE_MANAGED_PATHS); do \
			rm -rf "$$staging_abs/$$path"; \
		done; \
		for path in $(STAGE_PATHS); do \
			mkdir -p "$$staging_abs/$$(dirname "$$path")"; \
			cp -a "$$path" "$$staging_abs/$$path"; \
		done
	@find $(addprefix $(STAGING)/,$(STAGE_MANAGED_PATHS)) -type f \
		\( -name '*.c' -o -name '*.h' -o -name '*.s' -o \
		   -name '*.inc' -o -name '*.ld' -o -name '*.txt' -o \
		   -name '*.sh' \) \
		-exec dos2unix -q {} +
	@touch $@
	@echo "Staging complete."
