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
AS            := $(CROSS)as
LD            := $(CROSS)ld
OBJCOPY       := $(CROSS)objcopy

# Flags - tune these to match your game's original compiler settings
# NOTE: -Iinclude removed - we use -Iinclude when cd'd into $(WORK_DIR)
CFLAGS        := -O2 -mips1 -mfp32 -mno-abicalls -fno-pic \
                 -G0 -funsigned-char -fno-common \
                 -nostdinc -nostdlib -fno-builtin -fomit-frame-pointer \
                 -Wall -EL \
				 -mno-gpopt

# AS_FLAGS only has assembler flags, no GCC flags
# NOTE: -Iinclude removed - we use -Iinclude when cd'd into $(WORK_DIR)
AS_FLAGS      := -march=r3000 -mtune=r3000 -no-pad-sections -EL

# maspsx - note: NO --run-assembler flag, we pipe to AS instead
# --macro-inc is needed to define ASPSX directives like 'nonmatching', 'dlabel', etc.
MASPSX        := python3 tools/maspsx/maspsx.py
MASPSX_FLAGS  := --expand-div --aspsx-version=2.78
MASPSX_FLAGS_C := $(MASPSX_FLAGS)
MASPSX_FLAGS_ASM := $(MASPSX_FLAGS) --macro-inc

# Directories (original mounted paths)
SRC_DIR       := src
ASM_DIR       := asm
NONMATCH_DIR  := $(ASM_DIR)/nonmatchings
DATA_DIR      := $(ASM_DIR)/data

# BIN output to produce a matching binary
BIN           := build/$(GAME).bin
PAD_SIZE      := 992

# ---------------- Files ----------------

# Collect all .c files in src/ (from mounted directory before copying)
C_SOURCES     := $(wildcard $(SRC_DIR)/*.c)
# Objects will be built in /tmp_build/build/src/*.o
C_OBJECTS     := $(patsubst $(SRC_DIR)/%.c,$(WORK_DIR)/build/$(SRC_DIR)/%.o,$(C_SOURCES))

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

# --- Decompiled C → GCC asm → maspsx → object ---
# Use pipes like Croc does: GCC | MASPSX | AS
# C files don't need --macro-inc since GCC output doesn't use ASPSX directives
# Compile from /tmp_build/src/*.c with includes from /tmp_build/include
# Static pattern rule: for each src/X.c, build /tmp_build/build/src/X.o from /tmp_build/src/X.c
$(C_OBJECTS): $(WORK_DIR)/build/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(WORK_DIR) && $(CC) $(CFLAGS) -Iinclude -c $(SRC_DIR)/$*.c -S -o - | python3 tools/maspsx/maspsx.py $(MASPSX_FLAGS_C) | $(AS) $(AS_FLAGS) -Iinclude -o build/$(SRC_DIR)/$*.o

# --- Asm files with ASPSX directives (non-matching + data + header) ---
# These need --macro-inc to handle directives like 'nonmatching', 'dlabel', etc.
# Compile from /tmp_build/asm/*.s with includes from /tmp_build/include
# Static pattern rule: for each asm/X.s, build /tmp_build/build/asm/X.o from /tmp_build/asm/X.s
$(OTHER_OBJ): $(WORK_DIR)/build/$(ASM_DIR)/%.o: $(ASM_DIR)/%.s $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(WORK_DIR) && cat $(ASM_DIR)/$*.s | python3 tools/maspsx/maspsx.py $(MASPSX_FLAGS_ASM) | $(AS) $(AS_FLAGS) -Iinclude -o build/$(ASM_DIR)/$*.o

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

.PHONY: all clean bin recopy

# Optional: rebuild a single function quickly
# make /tmp_build/build/src/main.o
