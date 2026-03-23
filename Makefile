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

GAME         := slus_010.13
TARGET       := $(STAGING)/build/$(GAME).elf
BIN          := build/$(GAME).bin

# Directories (relative — used for both the mount and staging mirror)
SRC_DIR      := src
ASM_DIR      := asm


# ─── Toolchain ──────────────────────────────────────────────────────────────────
#
# CC       — GCC 2.8.0 cross-compiler for PSX (installed in /opt/psx-gcc/)
# LD       — modern mipsel linker (binutils from apt)
# OBJCOPY  — converts ELF ↔ raw binary

CROSS        	:= mipsel-linux-gnu-
CC           	:= gcc
CCPSX		 	:= wibo /opt/psyq4.1/CCPSX.EXE
PSYQ_OBJ_PARSER := /opt/psyq4.1/psyq-obj-parser
LD           	:= $(CROSS)ld
OBJCOPY      	:= $(CROSS)objcopy


# ─── Compiler & Assembler Flags ─────────────────────────────────────────────────
#
# -O2            Optimization level that matches the original compiler output.
# -G0 / -G4     Controls the "small data" threshold. -G0 means nothing goes in
#                the $gp-relative section; -G4 allows data ≤4 bytes to use $gp.
#                Most files use -G0, but cd.c needs -G4 to match the original.
# -g             Emit debug info (doesn't affect code generation on this GCC).
# -fsigned-char  Treat bare 'char' as signed (PSX SDK convention).
#
# MASPSX_AS_FLAGS:
#   --run-assembler     Have maspsx invoke the system assembler directly.
#   -no-pad-sections    Don't pad sections to 16-byte alignment (matches ASPSX).
#   --aspsx-version     Target ASPSX 2.77 behavior for asm translation.
#   --macro-inc         Enable ASPSX directive macros (nonmatching, dlabel, etc.)
#                       Only used for hand-written .s files, NOT for GCC output.

CFLAGS_G0       := -O2 -G0 -g -fsigned-char
CFLAGS_G4       := -O2 -G4 -g -fsigned-char

# CCPSX flags omit -g: psyq-obj-parser can't handle debug info from complex functions.
CFLAGS_CCPSX_G0 := -O2 -G0 -fsigned-char

INCLUDE_FLAGS   := -Iinclude -Iinclude/psyq

MASPSX_AS       	:= python3 tools/maspsx/maspsx.py --run-assembler
MASPSX_AS_FLAGS 	:= -no-pad-sections --aspsx-version=2.77
MASPSX_PP       	:= python3 tools/maspsx/maspsx.py
MASPSX_PP_FLAGS 	:= --macro-inc


# ─── Source Files ───────────────────────────────────────────────────────────────
#
# C files are split by compiler flags. Most use CFLAGS_G0; only cd.c uses G4.
# If a future file needs different flags, add it to the appropriate list.
#
# "Non-matching" .s files (asm/nonmatchings/) are NOT listed here — they get
# pulled in automatically via the INCLUDE_ASM() macro inside C source files.

SRCS_G0 := \
	src/psyq/libcd/SYS.c \
	src/psyq/libetc/INTR.c \
	src/psyq/libc2/bcopy.c \
	src/psyq/libc2/bzero.c \
	src/psyq/libc2/memcpy.c \
	src/psyq/libc2/memset.c \
	src/psyq/libc2/rand.c \
	src/psyq/libc2/strcat.c \
	src/psyq/libc2/strcmp.c \
	src/psyq/libc2/strcpy.c \
	src/psyq/libc2/strlen.c \
	src/psyq/libc2/strncmp.c \
	src/psyq/libc2/strncpy.c \
	src/psyq/libc2/exit.c \
	src/psyq/libcard/C171.c \
	src/psyq/libcard/C172.c \
	src/psyq/libcard/A78.c \
	src/psyq/libcard/A79.c \
	src/psyq/libcard/A80.c \
	src/psyq/libcard/A93.c \
	src/psyq/libcard/CARD.c \
	src/psyq/libcard/INIT.c \
	src/psyq/libapi/PAD.c \
	src/psyq/libapi/A18.c \
	src/psyq/libapi/A19.c \
	src/psyq/libapi/A20.c \
	src/psyq/libapi/A21.c \
	src/psyq/libapi/L02.c \
	src/psyq/libapi/L03.c \
	src/psyq/libapi/PATCH.c \
	src/psyq/libapi/C68.c \
	src/psyq/libapi/CHCLRPAD.c \
	src/psyq/libcard/A74.c \
	src/psyq/libcard/A75.c \
	src/psyq/libcard/A76.c \
	src/psyq/libcard/PATCH.c \
	src/psyq/libcard/END.c \
	src/psyq/libcard/FORMAT.c \
	src/psyq/libcard/A92.c \
	src/psyq/libpress/PRESS.c \
	src/psyq/libc2/PRINTF.c \
	src/psyq/libc2/PRNT.c \
	src/psyq/libc2/CTYPE.c \
	src/psyq/libc2/MEMCHR.c \
	src/psyq/libc2/PUTCHAR.c \
	src/psyq/libpress/VLC_C.c \
	src/psyq/libpress/BUILD.c \
	src/psyq/libgpu/SYS.c \
	src/psyq/libapi/C73.c \
	src/psyq/libgpu/BREAK.c \
	src/psyq/libgpu/EXT.c \
	src/psyq/libgpu/P17.c \
	src/psyq/libgpu/P18.c \
	src/psyq/libgte/GEO_00.c \
	src/psyq/libgte/GEO_01.c \
	src/psyq/libgte/COR_02.c \
	src/psyq/libgte/COR_01.c \
	src/psyq/libgte/COR_03.c \
	src/psyq/libgte/MSC00.c \
	src/psyq/libgte/MSC01.c \
	src/psyq/libgte/MSC02.c \
	src/psyq/libgte/MTX_003.c \
	src/psyq/libgte/MTX_004.c \
	src/psyq/libgte/MTX_006.c \
	src/psyq/libgte/MTX_07.c \
	src/psyq/libgte/MTX_08.c \
	src/psyq/libgte/MTX_09.c \
	src/psyq/libgte/MTX_12.c \
	src/psyq/libgte/REG12.c \
	src/decompression.c \
	src/unk1.c \
	src/unk2.c \
	src/unk4.c \
	src/unk5.c \
	src/unk6.c \
	src/unk7.c \
	src/decomp1.c \
	src/main.c

SRCS_G4 := \
	src/cd.c

SRCS_PSYQ41_G0 := \
	src/overlays/checkps/code.c

# Hand-written assembly (header + initialized data sections).
# Rodata is NOT here — it's inlined into C files via asm directives.
ASM_SRCS := \
	asm/header.s \
	asm/data/initialized.data.s \
	asm/data/sdata.data.s


# ─── Object File Paths ─────────────────────────────────────────────────────────
#
# patsubst turns  src/foo/bar.c  →  /staging/build/src/foo/bar.o
# This mirrors the source tree under the staging build directory.

OBJS_G0  		:= $(patsubst $(SRC_DIR)/%.c,$(STAGING)/build/$(SRC_DIR)/%.o,$(SRCS_G0))
OBJS_G4  		:= $(patsubst $(SRC_DIR)/%.c,$(STAGING)/build/$(SRC_DIR)/%.o,$(SRCS_G4))
OBJS_PSYQ41_G0 	:= $(patsubst $(SRC_DIR)/%.c,$(STAGING)/build/$(SRC_DIR)/%.o,$(SRCS_PSYQ41_G0))
OBJS_ASM 		:= $(patsubst $(ASM_DIR)/%.s,$(STAGING)/build/$(ASM_DIR)/%.o,$(ASM_SRCS))

OBJECTS  := $(OBJS_G0) $(OBJS_G4) $(OBJS_PSYQ41_G0) $(OBJS_ASM)


# ─── Staging Sentinel ──────────────────────────────────────────────────────────
#
# A sentinel file tracks whether we've already copied sources to /staging.
# If it exists, Make skips the copy step. Run `make recopy` to force a refresh.

COPY_SENTINEL := $(STAGING)/.sources_copied

# Directories to mirror into /staging (add new ones here as needed)
STAGE_DIRS := src asm include linker tools assets


# ─── Overlay Registry ──────────────────────────────────────────────────────────
#
# Register overlays here so they're available to all rules below.
# The name must match the directory under src/overlays/, asm/overlays/, etc.
# See the "Overlay System" section further below for full documentation.
#
# Optional per-overlay settings:
#   overlay_<name>_cflags    — compiler flags (default: CFLAGS_G0)
#   overlay_<name>_asset     — path to a .bin asset file (omit if none)
#   overlay_<name>_gcc_srcs  — files to compile with GCC+maspsx instead of CCPSX
#                              (use for non-matching stubs that use INCLUDE_ASM)

OVERLAYS += checkps
overlay_checkps_asset    := assets/checkps.bin
overlay_checkps_gcc_srcs := src/overlays/checkps/unk.c


# ============================================================================
#  Top-Level Targets
# ============================================================================

.PHONY: all bin clean recopy
.PHONY: target-objects base-objects objdiff-objects objdiff-config
.PHONY: overlays everything

# Default target: build the main SLUS executable
all: $(TARGET)
	@mkdir -p build
	@cp -r $(STAGING)/build/* build/
	@echo "Build complete: build/$(GAME).elf"

# Produce a raw binary from the ELF (for running on real hardware / emulators)
bin: all
	$(OBJCOPY) -O binary build/$(GAME).elf $(BIN)

clean:
	rm -rf build/ $(STAGING)

recopy:
	rm -f $(COPY_SENTINEL)
	$(MAKE) $(COPY_SENTINEL)


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
	@# ASPSX (inside CCPSX) requires CRLF line endings for .s files it processes.
	@# In CI, git checks out with LF — convert overlay .s files so ASPSX can parse them.
	@# Only .s files need this; the C preprocessor/compiler handles LF fine.
	@find $(STAGING)/asm/overlays -name '*.s' -exec unix2dos {} + 2>/dev/null || true
	@touch $@
	@echo "Staging complete."


# ============================================================================
#  Compilation Rules — Main SLUS
# ============================================================================
#
# Static pattern rules:
#   $(TARGETS): $(STAGING)/build/src/%.o: src/%.c
#   reads as: "for each file in TARGETS, the .o comes from the matching .c"
#
# The recipe pipes GCC asm output directly into maspsx:
#   gcc -S -o -     → write asm to stdout
#   | maspsx.py ... → translate to ASPSX syntax and assemble into .o

# ── C files compiled with -G0 (most files) ──
$(OBJS_G0): $(STAGING)/build/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && $(CC) $(CFLAGS_G0) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS) -o build/$(SRC_DIR)/$*.o

# ── C files compiled with -G4 (cd.c) ──
$(OBJS_G4): $(STAGING)/build/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && $(CC) $(CFLAGS_G4) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS) -o build/$(SRC_DIR)/$*.o

# ── C files compiled with PSYQ 4.1 (GCC 2.7.2 CKD) -G0 (checkps overlay) ──
$(OBJS_PSYQ41_G0): $(STAGING)/build/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && $(CCPSX) -DCCPSX $(CFLAGS_CCPSX_G0) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -o build/$(SRC_DIR)/$*.obj && \
		$(PSYQ_OBJ_PARSER) build/$(SRC_DIR)/$*.obj -o build/$(SRC_DIR)/$*.o

# ── Hand-written assembly (header, data sections) ──
# These use --macro-inc because they contain ASPSX directives (dlabel, etc.)
# The pipeline: cat .s | maspsx (preprocess) | maspsx --run-assembler (assemble)
$(OBJS_ASM): $(STAGING)/build/$(ASM_DIR)/%.o: $(ASM_DIR)/%.s $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && cat $(ASM_DIR)/$*.s | \
		$(MASPSX_PP) $(MASPSX_PP_FLAGS) | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS) -o build/$(ASM_DIR)/$*.o


# ============================================================================
#  Linking — Main SLUS
# ============================================================================

$(TARGET): $(COPY_SENTINEL) $(OBJECTS) $(STAGING)/linker/$(GAME).ld
	@mkdir -p $(STAGING)/build
	cd $(STAGING) && $(LD) -o build/$(GAME).elf \
		-T linker/$(GAME).ld \
		-T linker/undefined_syms_auto.txt \
		-T linker/undefined_funcs_auto.txt \
		$(patsubst $(STAGING)/%,%,$(OBJECTS)) \
		-Map build/$(GAME).map


# ============================================================================
#  Objdiff — Main SLUS  (progress tracking / function matching)
# ============================================================================
#
# "Target objects" = assembled from splat's original .s disassembly (the goal).
# "Base objects"   = compiled from your decompiled .c source (your progress).
# objdiff compares them function-by-function to show what matches.

# Recursive wildcard helper (finds files in nested subdirectories)
rwildcard = $(foreach d,$(wildcard $1/*),$(call rwildcard,$d,$2)) \
            $(filter $(subst *,%,$2),$1)

# Gather all .s files, then exclude non-matchings, data, overlays, and
# hand-written asm that already has its own build rule (ASM_SRCS).
ALL_ASM    := $(call rwildcard,$(ASM_DIR),*.s)
TARGET_ASM := $(filter-out $(ASM_DIR)/nonmatchings/% $(ASM_DIR)/data/% $(ASM_DIR)/overlays/% $(ASM_SRCS),$(ALL_ASM))
TARGET_OBJ := $(patsubst $(ASM_DIR)/%.s,$(STAGING)/build/$(ASM_DIR)/%.o,$(TARGET_ASM))

$(TARGET_OBJ): $(STAGING)/build/$(ASM_DIR)/%.o: $(ASM_DIR)/%.s $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && cat $(ASM_DIR)/$*.s | \
		$(MASPSX_PP) $(MASPSX_PP_FLAGS) | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS) -o build/$(ASM_DIR)/$*.o

target-objects: $(COPY_SENTINEL) $(TARGET_OBJ)
	@mkdir -p build/asm
	@cp -r $(STAGING)/build/$(ASM_DIR)/* build/asm/ 2>/dev/null || true
	@echo "Target objects built."

base-objects: $(COPY_SENTINEL) $(OBJS_G0) $(OBJS_G4) $(OBJS_PSYQ41_G0)
	@mkdir -p build/src
	@cp -r $(STAGING)/build/$(SRC_DIR)/* build/src/ 2>/dev/null || true
	@echo "Base objects built."

objdiff-objects: target-objects base-objects $(addsuffix -objdiff,$(OVERLAYS))

objdiff-config:
	python3 tools/objdiff/generate_objdiff_config.py


# ============================================================================
#  Overlay System
# ============================================================================
#
# Overlays are small executables loaded on top of the main SLUS at runtime.
# Each overlay follows the same directory convention:
#
#   src/overlays/<name>/         — decompiled C source files
#   asm/overlays/<name>/         — splat-generated disassembly
#   linker/overlays/<name>/      — linker scripts from splat
#   build/overlays/<name>/       — compiled objects and output ELF
#   assets/<name>.bin            — optional raw binary data segment
#
# ── How to add a new overlay ─────────────────────────────────────────────────
#
#   1. Create config/overlays/<NAME>.yaml  (follow CHECKPS.BIN.yaml as a template)
#   2. Run: splat split config/overlays/<NAME>.yaml
#   3. Register it in the "Overlay Registry" section near the top of this file:
#
#        OVERLAYS += myoverlay
#        overlay_myoverlay_asset := assets/myoverlay.bin   # only if it has one
#
#      That's it. All build rules are generated automatically.
#
# ─────────────────────────────────────────────────────────────────────────────


# ── Overlay rule template ────────────────────────────────────────────────────
#
# This define block is a "macro" that generates all Make rules for one overlay.
# It's called once per entry in OVERLAYS via $(eval $(call ...)) at the bottom.
#
# Inside the template:
#   $(1)  = overlay name (e.g. "checkps")
#   $$    = escaped $ (needed because eval expands variables twice)
#
# The $$(or ...) pattern provides a default value: if overlay_<name>_cflags
# isn't set, it falls back to CFLAGS_G0.

define overlay-rules

# ── Derived paths for overlay '$(1)' ──
$(1)_SRC_DIR   := src/overlays/$(1)
$(1)_ASM_DIR   := asm/overlays/$(1)
$(1)_LINK_DIR  := linker/overlays/$(1)
$(1)_BUILD_DIR := build/overlays/$(1)
$(1)_CFLAGS    := $$(or $$(overlay_$(1)_cflags),$(CFLAGS_G0))
$(1)_TARGET    := $(STAGING)/$$($(1)_BUILD_DIR)/$(1).elf

# ── Discover source files ──
# Split into CCPSX (matched) and GCC+maspsx (non-matching) groups.
# Files listed in overlay_<name>_gcc_srcs use GCC+maspsx; everything else uses CCPSX.
$(1)_C_SRCS      := $$(wildcard $$($(1)_SRC_DIR)/*.c)
$(1)_GCC_SRCS    := $$(overlay_$(1)_gcc_srcs)
$(1)_CCPSX_SRCS  := $$(filter-out $$($(1)_GCC_SRCS),$$($(1)_C_SRCS))
$(1)_GCC_OBJS    := $$(patsubst $$($(1)_SRC_DIR)/%.c,$(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/%.o,$$($(1)_GCC_SRCS))
$(1)_CCPSX_OBJS  := $$(patsubst $$($(1)_SRC_DIR)/%.c,$(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/%.o,$$($(1)_CCPSX_SRCS))
$(1)_C_OBJS      := $$($(1)_CCPSX_OBJS) $$($(1)_GCC_OBJS)

# ── Binary asset (only if overlay_<name>_asset is defined) ──
$(1)_ASSET_SRC := $$(overlay_$(1)_asset)
$(1)_ASSET_OBJ := $(STAGING)/$$($(1)_BUILD_DIR)/assets/$(1).o

# Rule: compile matched C files with CCPSX → .obj → ELF .o
$$($(1)_CCPSX_OBJS): $(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/%.o: $$($(1)_SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $$(@D)
	cd $(STAGING) && $(CCPSX) -DCCPSX $(CFLAGS_CCPSX_G0) $(INCLUDE_FLAGS) -c $$($(1)_SRC_DIR)/$$*.c -o $$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/$$*.obj && \
		$(PSYQ_OBJ_PARSER) $$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/$$*.obj -o $$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/$$*.o

# Rule: compile non-matching C files with GCC+maspsx (handles GNU asm syntax in INCLUDE_ASM)
$$($(1)_GCC_OBJS): $(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/%.o: $$($(1)_SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $$(@D)
	cd $(STAGING) && $(CC) $$($(1)_CFLAGS) $(INCLUDE_FLAGS) -c $$($(1)_SRC_DIR)/$$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS) -o $$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/$$*.o

# Rule: convert binary asset → linkable .o  (only if asset is defined)
ifneq ($$($(1)_ASSET_SRC),)
$$($(1)_ASSET_OBJ): $(STAGING)/$$($(1)_ASSET_SRC)
	@mkdir -p $$(@D)
	cd $(STAGING) && $(OBJCOPY) -I binary -O elf32-tradlittlemips -B mips \
		$$($(1)_ASSET_SRC) $$($(1)_BUILD_DIR)/assets/$(1).o
endif

# Rule: link the overlay ELF
# Dependencies include the asset object only if the overlay has an asset.
$$($(1)_TARGET): $(COPY_SENTINEL) $$($(1)_C_OBJS) $$(if $$($(1)_ASSET_SRC),$$($(1)_ASSET_OBJ)) $(STAGING)/$$($(1)_LINK_DIR)/$(1).ld
	@mkdir -p $$(@D)
	cd $(STAGING) && $(LD) -o $$($(1)_BUILD_DIR)/$(1).elf \
		-T $$($(1)_LINK_DIR)/$(1).ld \
		-T $$($(1)_LINK_DIR)/undefined_funcs_auto.txt \
		-T $$($(1)_LINK_DIR)/undefined_syms_auto.txt \
		$$(patsubst $(STAGING)/%,%,$$($(1)_C_OBJS)) \
		-Map $$($(1)_BUILD_DIR)/$(1).map
	@echo "Linked overlay: $(1)"

# ── Objdiff rules for this overlay ──
$(1)_ALL_ASM    := $$(call rwildcard,$$($(1)_ASM_DIR),*.s)
$(1)_TGT_ASM   := $$(filter-out $$($(1)_ASM_DIR)/nonmatchings/% $$($(1)_ASM_DIR)/data/%,$$($(1)_ALL_ASM))
$(1)_TGT_OBJS  := $$(patsubst $$($(1)_ASM_DIR)/%.s,$(STAGING)/$$($(1)_BUILD_DIR)/target/%.o,$$($(1)_TGT_ASM))

$$($(1)_TGT_OBJS): $(STAGING)/$$($(1)_BUILD_DIR)/target/%.o: $$($(1)_ASM_DIR)/%.s $(COPY_SENTINEL)
	@mkdir -p $$(@D)
	cd $(STAGING) && cat $$($(1)_ASM_DIR)/$$*.s | \
		$(MASPSX_PP) $(MASPSX_PP_FLAGS) | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS_CKD) -o $$($(1)_BUILD_DIR)/target/$$*.o

# ── Phony convenience targets ──
.PHONY: $(1) $(1)-target-objects $(1)-base-objects $(1)-objdiff

$(1): $$($(1)_TARGET)
	@mkdir -p $$($(1)_BUILD_DIR)
	@cp -r $(STAGING)/$$($(1)_BUILD_DIR)/* $$($(1)_BUILD_DIR)/ 2>/dev/null || true
	@echo "Overlay $(1) build complete."

$(1)-target-objects: $(COPY_SENTINEL) $$($(1)_TGT_OBJS)
	@mkdir -p $$($(1)_BUILD_DIR)/target
	@cp -r $(STAGING)/$$($(1)_BUILD_DIR)/target/* $$($(1)_BUILD_DIR)/target/ 2>/dev/null || true

$(1)-base-objects: $(COPY_SENTINEL) $$($(1)_C_OBJS)
	@mkdir -p $$($(1)_BUILD_DIR)
	@cp -r $(STAGING)/$$($(1)_BUILD_DIR)/* $$($(1)_BUILD_DIR)/ 2>/dev/null || true

$(1)-objdiff: $(1)-target-objects $(1)-base-objects

endef

# ── Instantiate rules for every registered overlay ──
# This line loops over OVERLAYS and calls the template above for each one.
# $(eval) tells Make to treat the output as real Makefile syntax.
$(foreach ov,$(OVERLAYS),$(eval $(call overlay-rules,$(ov))))

# ── Aggregate overlay targets ──
overlays: $(OVERLAYS)
	@echo "All overlays built."

everything: all overlays
	@echo "Full build complete."
