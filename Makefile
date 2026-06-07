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


# ─── Toolchain ──────────────────────────────────────────────────────────────────
#
# CC       — GCC 2.8.0 cross-compiler for PSX (installed in /opt/psx-gcc/)
# LD       — modern mipsel linker (binutils from apt)
# OBJCOPY  — converts ELF ↔ raw binary

CROSS        	:= mipsel-linux-gnu-
CC           	:= /opt/psx-gcc/gcc -B/opt/psx-gcc/
CC_CDK       	:= /opt/cdk-gcc/gcc -B/opt/cdk-gcc/
CC_GNU       	:= /opt/psx-gnu-gcc/gcc -B/opt/psx-gnu-gcc/
CC_260_PSX		:= /opt/gcc-2.6.0-psx/gcc -B/opt/gcc-2.6.0-psx/
AS_GNU       	:= /opt/psx-gnu-gcc/as
LD           	:= $(CROSS)ld
OBJCOPY      	:= $(CROSS)objcopy


# ─── Compiler & Assembler Flags ─────────────────────────────────────────────────
#
# -O2            Optimization level that matches the original compiler output.
# -G0 / -G4     Controls the "small data" threshold. -G0 means nothing goes in
#                the $gp-relative section; -G4 allows data ≤4 bytes to use $gp.
#                Most files use -G0, but cdrom.c needs -G4 to match the original.
# -g             Emit debug info (doesn't affect code generation on this GCC).
# -fsigned-char  Treat bare 'char' as signed (PSX SDK convention).
#
# MASPSX_AS_FLAGS:
#   --run-assembler     Have maspsx invoke the system assembler directly.
#   -no-pad-sections    Don't pad sections to 16-byte alignment (matches ASPSX).
#   --aspsx-version     Target ASPSX 2.77 behavior for asm translation.
#   --macro-inc         Enable ASPSX directive macros (nonmatching, dlabel, etc.)
#                       Only used for hand-written .s files, NOT for GCC output.

CFLAGS_G0       := -O2 -G0 -gcoff -fsigned-char -fno-builtin
CFLAGS_G4       := -O2 -G4 -gcoff -fsigned-char

# CDK (GCC 2.7.2-970404) flags: no -g/-fsigned-char; uses -msoft-float and COFF debug.
CFLAGS_CDK_G0   := -O2 -G0 -msoft-float -gcoff

# GCC 2.6.0-psx flags.
CFLAGS_260_G0   := -O2 -G0 -gcoff -msoft-float
MASPSX_AS_FLAGS_260 := -no-pad-sections --aspsx-version=2.34 --expand-div

# PSX GNU GCC 2.7.2 flags: compiles to .s, then assembled with its own 'as'.
CFLAGS_GNU_G0   := -O2 -G0
AS_GNU_FLAGS    := -O -EL

INCLUDE_FLAGS   := -Iinclude -Iinclude/psyq

MASPSX_AS       	:= python3 tools/maspsx/maspsx.py --run-assembler
MASPSX_AS_FLAGS 	:= -no-pad-sections --aspsx-version=2.77 --expand-div
MASPSX_AS_FLAGS_CDK := -no-pad-sections --aspsx-version=2.67 --expand-div
MASPSX_PP       	:= python3 tools/maspsx/maspsx.py
MASPSX_PP_FLAGS 	:= --macro-inc


# ─── Source Files ───────────────────────────────────────────────────────────────
#
# C files are split by compiler flags. Most use CFLAGS_G0; only cdrom.c uses G4.
# If a future file needs different flags, add it to the appropriate list.
#
# "Non-matching" .s files (asm/nonmatchings/) are NOT listed here — they get
# pulled in automatically via the INCLUDE_ASM() macro inside C source files.

SRCS_G0 := \
	src/psyq/libapi/C114.c \
	src/psyq/libapi/A81.c \
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
	src/psyq/libgte/REG13.c \
	src/psyq/libgte/SMP_05.c \
	src/psyq/libgte/FGO_01.c \
	src/psyq/libgte/FGO_04.c \
	src/psyq/libgte/FGO_05.c \
	src/psyq/libgte/FGO_06.c \
	src/psyq/libgte/RMAT_01.c \
	src/psyq/libgte/RATAN.c \
	src/psyq/libgte/PATCHGTE.c \
	src/psyq/libcd/EVENT.c \
	src/psyq/libapi/A07.c \
	src/psyq/libcd/SYS.c \
	src/psyq/libcd/BIOS.c \
	src/psyq/libc2/PUTS.c \
	src/psyq/libcd/TYPE.c \
	src/psyq/libcd/S_002.c \
	src/psyq/libetc/VSYNC.c \
	src/psyq/libapi/L10.c \
	src/psyq/libetc/INTR.c \
	src/psyq/libapi/A23.c \
	src/psyq/libapi/A24.c \
	src/psyq/libapi/A25.c \
	src/psyq/libc2/SETJMP.c \
	src/psyq/libetc/INTR_VB.c \
	src/psyq/libetc/INTR_DMA.c \
	src/psyq/libetc/VMODE.c \
	src/psyq/libspu/S_I.c \
	src/psyq/libspu/S_INI.c \
	src/psyq/libspu/SPU.c \
	src/psyq/libspu/S_DCB.c \
	src/psyq/libspu/S_SI.c \
	src/psyq/libspu/S_SIA.c \
	src/psyq/libspu/S_STSA.c \
	src/psyq/libapi/A13.c \
	src/psyq/libapi/A32.c \
	src/psyq/libapi/COUNTER.c \
	src/psyq/libspu/S_Q.c \
	src/psyq/libspu/S_M_INIT.c \
	src/psyq/libspu/S_SR.c \
	src/psyq/libspu/S_M_UTIL.c \
	src/psyq/libspu/S_SIC.c \
	src/psyq/libspu/S_CB.c \
	src/psyq/libspu/S_R.c \
	src/psyq/libspu/S_W.c \
	src/psyq/libspu/S_STM.c \
	src/psyq/libspu/S_STC.c \
	src/psyq/libcard/C112.c \
	src/psyq/libapi/C159.c \
	src/psyq/libapi/A08.c \
	src/psyq/libapi/A09.c \
	src/psyq/libapi/A11.c \
	src/psyq/libapi/A12.c \
	src/psyq/libapi/A36.c \
	src/psyq/libapi/A37.c \
	src/psyq/libapi/A50.c \
	src/psyq/libapi/A52.c \
	src/psyq/libapi/A53.c \
	src/psyq/libapi/A54.c \
	src/psyq/libapi/A67.c \
	src/psyq/libapi/A68.c \
	src/psyq/libapi/A69.c \
	src/psyq/libapi/A91.c \
	src/psyq/libapi/SC2B.c \
	src/psyq/libapi/FIRST.c \
	src/psyq/libapi/A66.c \
	src/psyq/libspu/S_SRMT.c \
	src/psyq/libspu/S_CRWA.c \
	src/psyq/libapi/A10.c \
	src/psyq/libspu/S_GRMT.c \
	src/unk8.c \
	src/unk9.c \
	src/decomp1.c \
	src/akao_cmd.c \
	src/decomp7.c \
	src/main.c

SRCS_G4 := \
	src/cdrom.c \
	src/decomp2.c \
	src/decomp4.c \
	src/akao_driver.c \
	src/decomp6.c 

SRCS_CDK_G0 := \
	src/overlays/checkps/code.c \
	src/overlays/checkps/code2.c 

SRCS_GCC_260_G0 := \
	src/decomp8.c

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

OBJS_G0  			:= $(patsubst $(SRC_DIR)/%.c,$(STAGING)/build/$(SRC_DIR)/%.o,$(SRCS_G0))
OBJS_G4  			:= $(patsubst $(SRC_DIR)/%.c,$(STAGING)/build/$(SRC_DIR)/%.o,$(SRCS_G4))
OBJS_CDK_G0 		:= $(patsubst $(SRC_DIR)/%.c,$(STAGING)/build/$(SRC_DIR)/%.o,$(SRCS_CDK_G0))
OBJS_GCC_260_G0 	:= $(patsubst $(SRC_DIR)/%.c,$(STAGING)/build/$(SRC_DIR)/%.o,$(SRCS_GCC_260_G0))	
OBJS_ASM 			:= $(patsubst $(ASM_DIR)/%.s,$(STAGING)/build/$(ASM_DIR)/%.o,$(ASM_SRCS))

OBJECTS  := $(OBJS_G0) $(OBJS_G4) $(OBJS_CDK_G0) $(OBJS_GCC_260_G0) $(OBJS_ASM)


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
#   overlay_<name>_gcc_srcs  — files to compile with GCC+maspsx instead of CDK gcc
#                              (use for non-matching stubs that use INCLUDE_ASM)

OVERLAYS += addhero
overlay_addhero_gcc_srcs   := src/overlays/addhero/unk1.c

OVERLAYS += carda
overlay_carda_gcc_srcs   := src/overlays/carda/unk1.c

OVERLAYS += checkps
overlay_checkps_asset      := assets/checkps.bin
overlay_checkps_gcc_srcs   := src/overlays/checkps/code3.c
overlay_checkps_gnu_srcs   := src/overlays/checkps/code4.c src/overlays/checkps/code6.c src/overlays/checkps/code7.c

OVERLAYS += cload
overlay_cload_gcc_srcs   := src/overlays/cload/unk1.c

OVERLAYS += field
overlay_field_gcc_srcs      := src/overlays/field/unk1.c src/overlays/field/unk2.c src/overlays/field/unk3.c
overlay_field_gcc_g4_srcs   := src/overlays/field/field1.c src/overlays/field/field2.c 

OVERLAYS += gname

OVERLAYS += golem
overlay_golem_gcc_srcs   := src/overlays/golem/unk1.c

OVERLAYS += gosub
overlay_gosub_gcc_srcs   := src/overlays/gosub/unk1.c

OVERLAYS += gover

OVERLAYS += menu
overlay_menu_gcc_srcs   := src/overlays/menu/unk1.c

OVERLAYS += movie
overlay_movie_gcc_g4_srcs   := src/overlays/movie/movie.c

OVERLAYS += niki
overlay_niki_gcc_srcs   := src/overlays/niki/unk1.c

OVERLAYS += shop
overlay_shop_gcc_srcs   := src/overlays/shop/unk1.c

OVERLAYS += title

OVERLAYS += wsel
overlay_wsel_gcc_srcs    := src/overlays/wsel/unk1.c

OVERLAYS += zukan
overlay_zukan_gcc_srcs    := src/overlays/zukan/unk1.c

# ============================================================================
#  Top-Level Targets
# ============================================================================

SPLAT_CONFIGS := config/$(GAME).yaml $(wildcard config/overlays/*.yaml)

# Original ROM overlay BINs live here in CI (copied from the container's /rom/BIN).
ROM_BIN_DIR       := disc/BIN

# Manifest of overlays whose rebuilt+compressed BIN matches the original ROM.
# Read by tools/objdiff/generate_objdiff_config.py to mark units complete.
COMPLETE_MANIFEST := build/complete_overlays.txt

.PHONY: all bin clean recopy splat dump-objs
.PHONY: target-objects base-objects objdiff-objects objdiff-config
.PHONY: overlays everything
.PHONY: verify-bins verify-gover verify-movie

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

# Disassemble every compiled .o in build/ and write a matching .s alongside it.
# Run this after a build to get compiler output for the find_idioms.py tool.
#   make dump-objs
# The .s files end up at e.g. build/src/cdrom.s, build/overlays/gname/gname.s etc.
OBJDUMP := $(CROSS)objdump
dump-objs:
	@find build -name '*.o' | while read f; do \
		out="$${f%.o}.s"; \
		$(OBJDUMP) -d -r --no-show-raw-insn "$$f" > "$$out" 2>/dev/null && echo "  $$out" || true; \
	done
	@echo "dump-objs complete."

recopy:
	rm -f $(COPY_SENTINEL)
	$(MAKE) $(COPY_SENTINEL)

splat:
	@for cfg in $(SPLAT_CONFIGS); do splat split $$cfg || exit 1; done


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

# ── C files compiled with -G4 (cdrom.c) ──
$(OBJS_G4): $(STAGING)/build/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && $(CC) $(CFLAGS_G4) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS) -o build/$(SRC_DIR)/$*.o

# ── C files compiled with CDK GCC 2.7.2 + maspsx -G0 (checkps overlay) ──
$(OBJS_CDK_G0): $(STAGING)/build/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && $(CC_CDK) $(CFLAGS_CDK_G0) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS_CDK) -o build/$(SRC_DIR)/$*.o

# ── C files compiled with GCC 2.7.2 + maspsx -G0 (new overlay) ──
$(OBJS_GNU_G0): $(STAGING)/build/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && $(CC_GNU) $(CFLAGS_GNU_G0) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(AS_GNU) $(AS_GNU_FLAGS) -o build/$(SRC_DIR)/$*.o

$(OBJS_GCC_260_G0): $(STAGING)/build/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && $(CC_260_PSX) $(CFLAGS_260_G0) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS_260) -o build/$(SRC_DIR)/$*.o

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

base-objects: $(COPY_SENTINEL) $(OBJS_G0) $(OBJS_G4) $(OBJS_CDK_G0) $(OBJS_GCC_260_G0)
	@mkdir -p build/src
	@cp -r $(STAGING)/build/$(SRC_DIR)/* build/src/ 2>/dev/null || true
	@echo "Base objects built."

OBJDIFF_CLI := tools/objdiff/objdiff-cli-linux-x86_64

objdiff-objects: target-objects base-objects $(addsuffix -objdiff,$(OVERLAYS))

objdiff-config:
	python3 tools/objdiff/generate_objdiff_config.py

# Run objdiff diff on every unit in objdiff.json and write JSON results under
# build/diffs/, mirroring the unit name as a path (e.g. main/cdrom.json).
# Depends on objdiff-objects and objdiff-config so .o files and config are
# up to date before diffing.
.PHONY: diff-all diff-text
diff-all: objdiff-objects objdiff-config
	python3 tools/objdiff/run_diffs.py --cli $(OBJDIFF_CLI)

# Convert every build/diffs/**/*.json into a compact side-by-side text file
# at build/diffs/**/*.txt -- only non-100% functions, only differing lines.
diff-text: diff-all
	@find build/diffs -name '*.json' | while read f; do \
		python3 tools/objdiff/format_diffs.py --all "$$f" -o "$${f%.json}.txt"; \
	done
	@echo "Text diffs written to build/diffs/**/*.txt"


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
# Split into CDK (matched), GCC+maspsx (non-matching), and GNU gcc 2.7.2 groups.
# Files listed in overlay_<name>_gcc_srcs use GCC+maspsx; overlay_<name>_gnu_srcs
# use PSX GNU GCC 2.7.2 + its own assembler; everything else uses CDK gcc+maspsx.
$(1)_C_SRCS      := $$(wildcard $$($(1)_SRC_DIR)/*.c)
$(1)_GCC_SRCS    := $$(overlay_$(1)_gcc_srcs)
$(1)_GNU_SRCS    := $$(overlay_$(1)_gnu_srcs)
$(1)_GCC_G4_SRCS := $$(overlay_$(1)_gcc_g4_srcs)
$(1)_CDK_SRCS    := $$(filter-out $$($(1)_GCC_SRCS) $$($(1)_GNU_SRCS) $$($(1)_GCC_G4_SRCS),$$($(1)_C_SRCS))
$(1)_GCC_OBJS    := $$(patsubst $$($(1)_SRC_DIR)/%.c,$(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/%.o,$$($(1)_GCC_SRCS))
$(1)_GNU_OBJS    := $$(patsubst $$($(1)_SRC_DIR)/%.c,$(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/%.o,$$($(1)_GNU_SRCS))
$(1)_GCC_G4_OBJS := $$(patsubst $$($(1)_SRC_DIR)/%.c,$(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/%.o,$$($(1)_GCC_G4_SRCS))
$(1)_CDK_OBJS    := $$(patsubst $$($(1)_SRC_DIR)/%.c,$(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/%.o,$$($(1)_CDK_SRCS))
$(1)_C_OBJS      := $$($(1)_CDK_OBJS) $$($(1)_GCC_OBJS) $$($(1)_GNU_OBJS) $$($(1)_GCC_G4_OBJS)

# ── Binary asset (only if overlay_<name>_asset is defined) ──
$(1)_ASSET_SRC := $$(overlay_$(1)_asset)
$(1)_ASSET_OBJ := $(STAGING)/$$($(1)_BUILD_DIR)/assets/$(1).o

# Rule: compile matched C files with CDK GCC 2.7.2 + maspsx
$$($(1)_CDK_OBJS): $(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/%.o: $$($(1)_SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $$(@D)
	cd $(STAGING) && $(CC_CDK) $(CFLAGS_CDK_G0) $(INCLUDE_FLAGS) -c $$($(1)_SRC_DIR)/$$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS_CDK) -o $$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/$$*.o

# Rule: compile non-matching C files with GCC+maspsx (handles GNU asm syntax in INCLUDE_ASM)
$$($(1)_GCC_OBJS): $(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/%.o: $$($(1)_SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $$(@D)
	cd $(STAGING) && $(CC) $$($(1)_CFLAGS) $(INCLUDE_FLAGS) -c $$($(1)_SRC_DIR)/$$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS) -o $$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/$$*.o

# Rule: compile C files with GCC 2.8.0 + maspsx -G4 (e.g. movie.c)
$$($(1)_GCC_G4_OBJS): $(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/%.o: $$($(1)_SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $$(@D)
	cd $(STAGING) && $(CC) $(CFLAGS_G4) $(INCLUDE_FLAGS) -c $$($(1)_SRC_DIR)/$$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS) -o $$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/$$*.o

# Rule: compile C files with PSX GNU GCC 2.7.2 + its own assembler (no maspsx)
$$($(1)_GNU_OBJS): $(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/%.o: $$($(1)_SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $$(@D)
	cd $(STAGING) && $(CC_GNU) $(CFLAGS_GNU_G0) $(INCLUDE_FLAGS) -S $$($(1)_SRC_DIR)/$$*.c -o /tmp/$$*.s && \
		$(AS_GNU) $(AS_GNU_FLAGS) $(INCLUDE_FLAGS) -o $$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/$$*.o /tmp/$$*.s

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
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS_CDK) -o $$($(1)_BUILD_DIR)/target/$$*.o

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


# ============================================================================
#  ROM Verification — compressed overlay matching
# ============================================================================
#
# Some overlays are stored compressed in the original ROM (e.g. GOVER.BIN).
# To prove a 100% byte-perfect match we must reproduce the exact compressed
# file the disc contains:
#
#   1. Link the overlay ELF                       (handled by the overlay rule)
#   2. objcopy ELF -> raw decompressed binary
#   3. Compress with tools/compressor/compressor.py
#   4. Prepend the 1-byte algorithm-selector (0x01) the loader expects
#   5. SHA1-compare against the original BIN in disc/BIN/
#
# On match, the overlay's name is appended to build/complete_overlays.txt.
# generate_objdiff_config.py reads that manifest and stamps metadata.complete
# on every unit of a matched overlay.

# --- gover -------------------------------------------------------------------
# The original decompressed GOVER starts with a 0x00 byte before the actual
# overlay code (objcopy strips it because it's not in any output section), so
# we prepend it to the raw .bin before compressing.
build/overlays/gover/gover.raw: gover
	@mkdir -p build/overlays/gover
	$(OBJCOPY) -O binary $(STAGING)/build/overlays/gover/gover.elf $@.tmp
	printf '\0' > $@
	cat $@.tmp >> $@
	rm -f $@.tmp

build/overlays/gover/GOVER.BIN: build/overlays/gover/gover.raw
	python3 tools/compressor/compressor.py $< $@

verify-gover: build/overlays/gover/GOVER.BIN
	@mkdir -p build
	@expected=$$(sha1sum $(ROM_BIN_DIR)/GOVER.BIN | awk '{print $$1}'); \
	 actual=$$(sha1sum build/overlays/gover/GOVER.BIN  | awk '{print $$1}'); \
	 echo "GOVER.BIN expected: $$expected"; \
	 echo "GOVER.BIN actual:   $$actual"; \
	 if [ "$$expected" = "$$actual" ]; then \
	   echo "[OK] GOVER.BIN matches original ROM"; \
	   grep -qxF gover $(COMPLETE_MANIFEST) 2>/dev/null || echo gover >> $(COMPLETE_MANIFEST); \
	 else \
	   echo "[FAIL] GOVER.BIN sha1 mismatch"; \
	   exit 1; \
	 fi

# --- movie -------------------------------------------------------------------
# Like GOVER, the original decompressed MOVIE starts with a 0x00 byte that
# objcopy strips, so we prepend it to the raw .bin before compressing.
build/overlays/movie/movie.raw: movie
	@mkdir -p build/overlays/movie
	$(OBJCOPY) -O binary $(STAGING)/build/overlays/movie/movie.elf $@.tmp
	printf '\0' > $@
	cat $@.tmp >> $@
	rm -f $@.tmp

build/overlays/movie/MOVIE.BIN: build/overlays/movie/movie.raw
	python3 tools/compressor/compressor.py $< $@

verify-movie: build/overlays/movie/MOVIE.BIN
	@mkdir -p build
	@expected=$$(sha1sum $(ROM_BIN_DIR)/MOVIE.BIN | awk '{print $$1}'); \
	 actual=$$(sha1sum build/overlays/movie/MOVIE.BIN  | awk '{print $$1}'); \
	 echo "MOVIE.BIN expected: $$expected"; \
	 echo "MOVIE.BIN actual:   $$actual"; \
	 if [ "$$expected" = "$$actual" ]; then \
	   echo "[OK] MOVIE.BIN matches original ROM"; \
	   grep -qxF movie $(COMPLETE_MANIFEST) 2>/dev/null || echo movie >> $(COMPLETE_MANIFEST); \
	 else \
	   echo "[FAIL] MOVIE.BIN sha1 mismatch"; \
	   exit 1; \
	 fi

# Aggregate target — extend as more overlays reach 100%.
verify-bins: verify-gover verify-movie
	@echo "Verified compressed overlays: $$(cat $(COMPLETE_MANIFEST) 2>/dev/null | tr '\n' ' ')"
