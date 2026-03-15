# Makefile for PSX decomp project using maspsx + modern toolchain
# Uses /tmp_build at container root to avoid "Value too large" filesystem errors
# /tmp_build mirrors the exact same directory structure as the project

# ---------------- Configuration ----------------

# Working directory at container root - mirrors exact project structure
# /tmp_build/src/, /tmp_build/asm/, /tmp_build/include/, etc.
WORK_DIR      := /tmp_build

GAME          := slus_010.13
# Target ELF is built in /tmp_build/build/ then copied to mounted build/
TARGET        := $(WORK_DIR)/build/$(GAME).elf
MAPFILE       := $(WORK_DIR)/build/$(GAME).map
FINAL_TARGET  := /lom/build/$(GAME).elf
FINAL_MAPFILE := /lom/build/$(GAME).map

# Toolchain (adjust if using a different prefix, e.g. mips-linux-gnu-)
CROSS         := mipsel-linux-gnu-
CC            := gcc
LD            := $(CROSS)ld
OBJCOPY       := $(CROSS)objcopy

# Flags - tune these to match your game's original compiler settings
# NOTE: -Iinclude removed - we use -Iinclude when cd'd into $(WORK_DIR)
CFLAGS_G0     := -O2 -G0 -g -fsigned-char
CFLAGS_G4	  := -O2 -G4 -g -fsigned-char

INCLUDE_FLAGS := -Iinclude -Iinclude/psyq

# maspsx with --run-assembler flag (this replaces the AS variable)
# We call maspsx.py --run-assembler which internally calls the system assembler
MASPSX_AS     := python3 tools/maspsx/maspsx.py --run-assembler
MASPSX_AS_FLAGS := -no-pad-sections --aspsx-version=2.77

# maspsx for preprocessing only (no --run-assembler)
# --macro-inc is needed to define ASPSX directives like 'nonmatching', 'dlabel', etc.
MASPSX        := python3 tools/maspsx/maspsx.py
MASPSX_FLAGS  := 
MASPSX_FLAGS_C := $(MASPSX_FLAGS)
MASPSX_FLAGS_ASM := $(MASPSX_FLAGS) --macro-inc

# Directories (original mounted paths)
SRC_DIR       := src
ASM_DIR       := asm
NONMATCH_DIR  := $(ASM_DIR)/nonmatchings
DATA_DIR      := $(ASM_DIR)/data

# BIN output to produce a matching binary
BIN           := build/$(GAME).bin

# ---------------- Files ----------------

# Collect all .c files in src/ (from mounted directory before copying)
rwildcard = $(foreach d,$(wildcard $1/*),$(call rwildcard,$d,$2)) \
            $(filter $(subst *,%,$2),$1)

C_SOURCES_G0 := \
	src/psyq/libcd/SYS.c \
	src/psyq/libetc/INTR.c \
	src/decompression.c \
	src/unk1.c \
	src/unk2.c \
	src/unk4.c \
	src/unk5.c \
	src/unk6.c \
	src/decomp1.c \
	src/main.c

C_SOURCES_G4 := src/cd.c

# Objects will be built in /tmp_build/build/src/*.o
C_OBJECTS_G0   := $(patsubst $(SRC_DIR)/%.c,$(WORK_DIR)/build/$(SRC_DIR)/%.o,$(C_SOURCES_G0))
C_OBJECTS_G4   := $(patsubst $(SRC_DIR)/%.c,$(WORK_DIR)/build/$(SRC_DIR)/%.o,$(C_SOURCES_G4))
C_OBJECTS      := $(C_OBJECTS_G0) $(C_OBJECTS_G4)

# Collect non-matching asm files (e.g. asm/nonmatchings/subdir/func.s)
# NOTE: These are NOT built as separate objects because they're included via INCLUDE_ASM in C files
# NONMATCH_ASM  := $(wildcard $(NONMATCH_DIR)/**/*.s)
# NONMATCH_OBJ  := $(patsubst %.s,$(WORK_DIR)/build/%.o,$(NONMATCH_ASM))

# Data / header asm
# Note: rodata files are NOT here - they're included in main.c via inline asm
OTHER_ASM     := $(ASM_DIR)/header.s \
                 $(ASM_DIR)/data/initialized.data.s \
                 $(ASM_DIR)/data/sdata.data.s 
OTHER_OBJ     := $(patsubst $(ASM_DIR)/%.s,$(WORK_DIR)/build/$(ASM_DIR)/%.o,$(OTHER_ASM))

# All objects to link (nonmatching asm is included via INCLUDE_ASM, not as separate objects)
OBJECTS       := $(C_OBJECTS) $(OTHER_OBJ)

# Sentinel file to track when sources have been copied
COPY_SENTINEL := $(WORK_DIR)/.sources_copied

# ---------------- Rules ----------------

all: $(TARGET)
	@echo "Copying build artifacts from $(WORK_DIR)/build/ to build/..."
	@mkdir -p build
	@cp -r $(WORK_DIR)/build/* build/
	@echo "Build complete: $(FINAL_TARGET)"

# Copy all source files to /tmp_build mirroring exact directory structure
$(COPY_SENTINEL):
	@echo "Copying source files to $(WORK_DIR) (mirroring directory structure)..."
	@mkdir -p $(WORK_DIR)
	@# Copy src/ directory
	@if [ -d "$(SRC_DIR)" ]; then \
		mkdir -p $(WORK_DIR)/$(SRC_DIR); \
		cp -r $(SRC_DIR)/* $(WORK_DIR)/$(SRC_DIR)/ 2>/dev/null || true; \
	fi
	@# Copy asm/ directory
	@if [ -d "$(ASM_DIR)" ]; then \
		mkdir -p $(WORK_DIR)/$(ASM_DIR); \
		cp -r $(ASM_DIR)/* $(WORK_DIR)/$(ASM_DIR)/ 2>/dev/null || true; \
	fi
	@# Copy include/ directory
	@if [ -d "include" ]; then \
		mkdir -p $(WORK_DIR)/include; \
		cp -r include/* $(WORK_DIR)/include/ 2>/dev/null || true; \
	fi
	@# Copy linker/ directory
	@if [ -d "linker" ]; then \
		mkdir -p $(WORK_DIR)/linker; \
		cp -r linker/* $(WORK_DIR)/linker/ 2>/dev/null || true; \
	fi
	@# Copy tools/ directory (for maspsx)
	@if [ -d "tools" ]; then \
		mkdir -p $(WORK_DIR)/tools; \
		cp -r tools/* $(WORK_DIR)/tools/ 2>/dev/null || true; \
	fi
	@# Copy assets/ directory (splat-generated binary assets for overlays)
	@if [ -d "assets" ]; then \
		mkdir -p $(WORK_DIR)/assets; \
		cp -r assets/* $(WORK_DIR)/assets/ 2>/dev/null || true; \
	fi
	@touch $(COPY_SENTINEL)
	@echo "Source files copied successfully to $(WORK_DIR)"

$(TARGET): $(COPY_SENTINEL) $(OBJECTS) $(WORK_DIR)/linker/$(GAME).ld
	@mkdir -p $(WORK_DIR)/build
	cd $(WORK_DIR) && $(LD) -o build/$(GAME).elf \
		-T linker/$(GAME).ld \
		-T linker/undefined_syms_auto.txt \
		-T linker/undefined_funcs_auto.txt \
		$(patsubst $(WORK_DIR)/%,%,$(OBJECTS)) \
		-Map build/$(GAME).map
	@echo "Linked $@"

# --- Decompiled C → GCC asm → maspsx --run-assembler → object ---
# Call maspsx.py with --run-assembler flag which handles assembly internally
# C files don't need --macro-inc since GCC output doesn't use ASPSX directives
# Compile from /tmp_build/src/*.c with includes from /tmp_build/include
# Static pattern rule: for each src/X.c, build /tmp_build/build/src/X.o from /tmp_build/src/X.c
# Static pattern rule: CFLAGS_A sources
$(C_OBJECTS_G0): $(WORK_DIR)/build/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(WORK_DIR) && $(CC) $(CFLAGS_G0) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS) -o build/$(SRC_DIR)/$*.o

# Static pattern rule: CFLAGS_B sources
$(C_OBJECTS_G4): $(WORK_DIR)/build/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(WORK_DIR) && $(CC) $(CFLAGS_G4) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS) -o build/$(SRC_DIR)/$*.o

# --- Asm files with ASPSX directives (non-matching + data + header) ---
# These need --macro-inc to handle directives like 'nonmatching', 'dlabel', etc.
# Compile from /tmp_build/asm/*.s with includes from /tmp_build/include
# Static pattern rule: for each asm/X.s, build /tmp_build/build/asm/X.o from /tmp_build/asm/X.s
$(OTHER_OBJ): $(WORK_DIR)/build/$(ASM_DIR)/%.o: $(ASM_DIR)/%.s $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(WORK_DIR) && cat $(ASM_DIR)/$*.s | \
		python3 tools/maspsx/maspsx.py $(MASPSX_FLAGS_ASM) | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS) -o build/$(ASM_DIR)/$*.o

# ---------------- Binary output + padding ----------------

$(BIN): $(FINAL_TARGET)
	$(OBJCOPY) -O binary $(FINAL_TARGET) $@

bin: $(BIN)

clean:
	rm -rf build/ $(WORK_DIR) $(FINAL_TARGET) $(FINAL_MAPFILE)

# Force recopy of sources (use if you modified source files)
recopy:
	rm -f $(COPY_SENTINEL)
	$(MAKE) $(COPY_SENTINEL)

# ---------------- Target Objects (for objdiff) ----------------

# Target assembly files (full .s files from splat)
# Exclude nonmatchings, data, and overlay asm from SLUS targets
ALL_ASM := $(call rwildcard,$(ASM_DIR),*.s)
TARGET_ASM := $(filter-out $(ASM_DIR)/nonmatchings/% $(ASM_DIR)/data/% $(ASM_DIR)/overlays/%, $(ALL_ASM))
TARGET_OBJ := $(patsubst $(ASM_DIR)/%.s,$(WORK_DIR)/build/$(ASM_DIR)/%.o,$(TARGET_ASM))

# Build target objects from asm/*.s files (these are already processed by splat)
$(TARGET_OBJ): $(WORK_DIR)/build/$(ASM_DIR)/%.o: $(ASM_DIR)/%.s $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(WORK_DIR) && cat $(ASM_DIR)/$*.s | \
		python3 tools/maspsx/maspsx.py $(MASPSX_FLAGS_ASM) | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS) -o build/$(ASM_DIR)/$*.o

# Build all target objects (for objdiff progress tracking)
target-objects: $(COPY_SENTINEL) $(TARGET_OBJ)
	@echo "Copying target objects from $(WORK_DIR)/build/asm/ to build/asm/..."
	@mkdir -p build/asm
	@cp -r $(WORK_DIR)/build/$(ASM_DIR)/* build/asm/ 2>/dev/null || true
	@echo "Target objects built successfully"

# Build all base objects (for objdiff progress tracking)
base-objects: $(COPY_SENTINEL) $(C_OBJECTS)
	@echo "Copying base objects from $(WORK_DIR)/build/src/ to build/src/..."
	@mkdir -p build/src
	@cp -r $(WORK_DIR)/build/$(SRC_DIR)/* build/src/ 2>/dev/null || true
	@echo "Base objects built successfully"

# Build both target and base objects
objdiff-objects: target-objects base-objects
	@echo "All objdiff objects built successfully"

# Generate objdiff.json configuration file
objdiff-config:
	python3 tools/generate_objdiff_config.py

# ================================================================
# OVERLAYS
# ================================================================
# Each overlay gets its own directories:
#   asm/overlays/<name>/         - splat asm output
#   src/overlays/<name>/         - decompiled C source
#   linker/overlays/<name>/      - linker scripts from splat
#   build/overlays/<name>/       - build objects and ELF
#
# To add a new overlay:
#   1. Create config/overlays/<NAME>.yaml with paths like:
#        asm_path: asm/overlays/<name>
#        src_path: src/overlays/<name>
#        build_path: build/overlays/<name>
#        ld_script_path: linker/overlays/<name>/<name>.ld
#   2. Run: splat split config/overlays/<NAME>.yaml
#   3. Add a build section below following the CHECKPS pattern
# ================================================================

# ---- CHECKPS.BIN Overlay ----
CHECKPS_NAME      := checkps
CHECKPS_SRC_DIR   := src/overlays/$(CHECKPS_NAME)
CHECKPS_ASM_DIR   := asm/overlays/$(CHECKPS_NAME)
CHECKPS_LINK_DIR  := linker/overlays/$(CHECKPS_NAME)
CHECKPS_BUILD_DIR := build/overlays/$(CHECKPS_NAME)
CHECKPS_TARGET    := $(WORK_DIR)/$(CHECKPS_BUILD_DIR)/$(CHECKPS_NAME).elf
CHECKPS_MAPFILE   := $(WORK_DIR)/$(CHECKPS_BUILD_DIR)/$(CHECKPS_NAME).map

# Source files (update after running splat split)
# Objects go to build_path/src_path/name.o to match splat's linker script references
CHECKPS_C_SOURCES := $(wildcard $(CHECKPS_SRC_DIR)/*.c)
CHECKPS_C_OBJECTS := $(patsubst $(CHECKPS_SRC_DIR)/%.c,$(WORK_DIR)/$(CHECKPS_BUILD_DIR)/$(CHECKPS_SRC_DIR)/%.o,$(CHECKPS_C_SOURCES))

# Binary assets - splat puts .bin at assets/ (project root), linker expects .o at build_path/assets/
CHECKPS_ASSET_SRC := assets/checkps.bin
CHECKPS_ASSET_DIR := $(CHECKPS_BUILD_DIR)/assets
CHECKPS_ASSET_OBJ := $(WORK_DIR)/$(CHECKPS_ASSET_DIR)/checkps.o

# Convert binary asset to linkable object
$(CHECKPS_ASSET_OBJ): $(WORK_DIR)/$(CHECKPS_ASSET_SRC)
	@mkdir -p $(@D)
	cd $(WORK_DIR) && $(OBJCOPY) -I binary -O elf32-tradlittlemips -B mips \
		$(CHECKPS_ASSET_SRC) $(CHECKPS_ASSET_DIR)/checkps.o

# Compile CHECKPS C sources (uses -G0 by default; adjust if needed)
$(CHECKPS_C_OBJECTS): $(WORK_DIR)/$(CHECKPS_BUILD_DIR)/$(CHECKPS_SRC_DIR)/%.o: $(CHECKPS_SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(WORK_DIR) && $(CC) $(CFLAGS_G0) $(INCLUDE_FLAGS) -c $(CHECKPS_SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS) -o $(CHECKPS_BUILD_DIR)/$(CHECKPS_SRC_DIR)/$*.o

# Link CHECKPS overlay
$(CHECKPS_TARGET): $(COPY_SENTINEL) $(CHECKPS_C_OBJECTS) $(CHECKPS_ASSET_OBJ) $(WORK_DIR)/$(CHECKPS_LINK_DIR)/$(CHECKPS_NAME).ld
	@mkdir -p $(@D)
	cd $(WORK_DIR) && $(LD) -o $(CHECKPS_BUILD_DIR)/$(CHECKPS_NAME).elf \
		-T $(CHECKPS_LINK_DIR)/$(CHECKPS_NAME).ld \
		-T $(CHECKPS_LINK_DIR)/undefined_funcs_auto.txt \
		-T $(CHECKPS_LINK_DIR)/undefined_syms_auto.txt \
		$(patsubst $(WORK_DIR)/%,%,$(CHECKPS_C_OBJECTS)) \
		-Map $(CHECKPS_BUILD_DIR)/$(CHECKPS_NAME).map
	@echo "Linked CHECKPS overlay: $@"

# CHECKPS target objects for objdiff (from splat-generated asm)
CHECKPS_ALL_ASM    := $(call rwildcard,$(CHECKPS_ASM_DIR),*.s)
CHECKPS_TARGET_ASM := $(filter-out $(CHECKPS_ASM_DIR)/nonmatchings/% $(CHECKPS_ASM_DIR)/data/%, $(CHECKPS_ALL_ASM))
CHECKPS_TARGET_OBJ := $(patsubst $(CHECKPS_ASM_DIR)/%.s,$(WORK_DIR)/$(CHECKPS_BUILD_DIR)/target/%.o,$(CHECKPS_TARGET_ASM))

$(CHECKPS_TARGET_OBJ): $(WORK_DIR)/$(CHECKPS_BUILD_DIR)/target/%.o: $(CHECKPS_ASM_DIR)/%.s $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(WORK_DIR) && cat $(CHECKPS_ASM_DIR)/$*.s | \
		python3 tools/maspsx/maspsx.py $(MASPSX_FLAGS_ASM) | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS) -o $(CHECKPS_BUILD_DIR)/target/$*.o

# Build CHECKPS overlay
checkps: $(CHECKPS_TARGET)
	@mkdir -p $(CHECKPS_BUILD_DIR)
	@cp -r $(WORK_DIR)/$(CHECKPS_BUILD_DIR)/* $(CHECKPS_BUILD_DIR)/ 2>/dev/null || true
	@echo "CHECKPS overlay build complete"

# CHECKPS objdiff targets
checkps-target-objects: $(COPY_SENTINEL) $(CHECKPS_TARGET_OBJ)
	@mkdir -p $(CHECKPS_BUILD_DIR)/target
	@cp -r $(WORK_DIR)/$(CHECKPS_BUILD_DIR)/target/* $(CHECKPS_BUILD_DIR)/target/ 2>/dev/null || true
	@echo "CHECKPS target objects built"

checkps-base-objects: $(COPY_SENTINEL) $(CHECKPS_C_OBJECTS)
	@mkdir -p $(CHECKPS_BUILD_DIR)
	@cp -r $(WORK_DIR)/$(CHECKPS_BUILD_DIR)/*.o $(CHECKPS_BUILD_DIR)/ 2>/dev/null || true
	@echo "CHECKPS base objects built"

checkps-objdiff: checkps-target-objects checkps-base-objects
	@echo "CHECKPS objdiff objects built"

# ---- Build all overlays ----
overlays: checkps
	@echo "All overlays built"

# Build everything (SLUS + all overlays)
everything: all overlays
	@echo "Full build complete"

.PHONY: all clean bin recopy target-objects base-objects objdiff-objects objdiff-config
.PHONY: checkps checkps-target-objects checkps-base-objects checkps-objdiff
.PHONY: overlays everything

# Optional: rebuild a single function quickly
# make /tmp_build/build/src/main.o
