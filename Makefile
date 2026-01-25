# Makefile for PSX decomp project using maspsx + modern toolchain

# ---------------- Configuration ----------------

GAME          := slus_010.13
TARGET        := $(GAME).elf
MAPFILE       := build/$(GAME).map

# Toolchain (adjust if using a different prefix, e.g. mips-linux-gnu-)
CROSS         := mipsel-linux-gnu-
CC            := $(CROSS)gcc
AS            := $(CROSS)as
LD            := $(CROSS)ld
OBJCOPY       := $(CROSS)objcopy

# Flags - tune these to match your game's original compiler settings
CFLAGS        := -Iinclude -O2 -mips1 -mfp32 -mno-abicalls -fno-pic -mno-shared \
                 -G 0 -funsigned-char -fno-common \
                 -nostdinc -nostdlib -fno-builtin -fomit-frame-pointer \
                 -Wall

# AS_FLAGS only has assembler flags, no GCC flags
AS_FLAGS      := -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -EL

# maspsx - note: NO --run-assembler flag, we pipe to AS instead
# --macro-inc is needed to define ASPSX directives like 'nonmatching', 'dlabel', etc.
MASPSX        := python3 tools/maspsx/maspsx.py
MASPSX_FLAGS  := --expand-div --aspsx-version=2.78
MASPSX_FLAGS_C := $(MASPSX_FLAGS)
MASPSX_FLAGS_ASM := $(MASPSX_FLAGS) --macro-inc

# Directories
BUILD_DIR     := build
SRC_DIR       := src
ASM_DIR       := asm
NONMATCH_DIR  := $(ASM_DIR)/nonmatchings
DATA_DIR      := $(ASM_DIR)/data

# ---------------- Files ----------------

# Collect all .c files in src/
C_SOURCES     := $(wildcard $(SRC_DIR)/*.c)
C_OBJECTS     := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES))

# Collect non-matching asm files (e.g. asm/nonmatchings/subdir/func.s)
# NOTE: These are NOT built as separate objects because they're included via INCLUDE_ASM in C files
# NONMATCH_ASM  := $(wildcard $(NONMATCH_DIR)/**/*.s)
# NONMATCH_OBJ  := $(patsubst %.s,$(BUILD_DIR)/%.o,$(NONMATCH_ASM))

# Data / header asm (adjust paths if different)
# Exclude 2F800.bss.s and 2F800.sbss.s because they duplicate symbols in gp_data.sdata.s
OTHER_ASM     := $(ASM_DIR)/header.s \
                 $(ASM_DIR)/data/800.rodata.s \
                 $(ASM_DIR)/data/844.rodata.s \
                 $(ASM_DIR)/data/A50.rodata.s \
                 $(ASM_DIR)/data/B44.rodata.s \
                 $(ASM_DIR)/data/initialized.data.s \
                 $(ASM_DIR)/data/gp_data.sdata.s
OTHER_OBJ     := $(patsubst %.s,$(BUILD_DIR)/%.o,$(OTHER_ASM))

# All objects to link (nonmatching asm is included via INCLUDE_ASM, not as separate objects)
OBJECTS       := $(C_OBJECTS) $(OTHER_OBJ)

# ---------------- Rules ----------------

all: $(TARGET)

$(TARGET): $(OBJECTS) linker/$(GAME).ld
	$(LD) -o $@ \
		-T linker/$(GAME).ld \
		-T linker/undefined_syms_auto.txt \
		-T linker/undefined_funcs_auto.txt \
		$(OBJECTS) \
		-Map $(MAPFILE)
	@echo "Built $@"

# --- Decompiled C → GCC asm → maspsx → object ---
# Use pipes like Croc does: GCC | MASPSX | AS
# C files don't need --macro-inc since GCC output doesn't use ASPSX directives
$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -S -o - | $(MASPSX) $(MASPSX_FLAGS_C) | $(AS) $(AS_FLAGS) -o $@

# --- Asm files with ASPSX directives (non-matching + data + header) ---
# These need --macro-inc to handle directives like 'nonmatching', 'dlabel', etc.
$(BUILD_DIR)/$(ASM_DIR)/%.o: $(ASM_DIR)/%.s
	@mkdir -p $(@D)
	cat $< | $(MASPSX) $(MASPSX_FLAGS_ASM) | $(AS) $(AS_FLAGS) -o $@

clean:
	rm -rf build/ $(TARGET) $(MAPFILE)

.PHONY: all clean

# Optional: rebuild a single function quickly
# make build/nonmatchings/subdir/func.o
