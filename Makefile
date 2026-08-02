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
# A sentinel file tracks whether we've already copied sources to /staging.
# If it exists, Make skips the copy step. Run `make recopy` to force a refresh.

COPY_SENTINEL := $(STAGING)/.sources_copied

# Directories to mirror into /staging (add new ones here as needed)
STAGE_DIRS := src asm include linker tools assets

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
# Mirrors each directory listed in STAGE_DIRS from /lom/ → /staging/.
# The sentinel file prevents re-copying on every build.

$(COPY_SENTINEL):
	@echo "Staging source files to $(STAGING)..."
	@mkdir -p $(STAGING)
	@$(foreach dir,$(STAGE_DIRS), \
		if [ -d "$(dir)" ]; then \
			mkdir -p $(STAGING)/$(dir) && \
			cp -r $(dir)/* $(STAGING)/$(dir)/ 2>/dev/null || true; \
		fi; \
	)
	@# Normalize all overlay .s files to LF.
	@find $(STAGING)/asm/overlays -name '*.s' -exec dos2unix {} + 2>/dev/null || true
	@touch $@
	@echo "Staging complete."
