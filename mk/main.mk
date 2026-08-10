# ============================================================================
# Main SLUS executable
# ============================================================================


# ─── Source Files ──────────────────────────────────────────────────────────────
#
# C files are grouped by compiler and flags. Most use the default GCC 2.8.0
# G0 configuration; add exceptions to the matching toolchain list.
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
	src/psyq/libspu/S_SRA.c \
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
	src/screen_transition.c \
	src/game_audio.c \
	src/akao_cmd.c \
	src/field_runtime.c \
	src/decomp9.c \
	src/decomp9-func_80024B00.c \
	src/decomp9-func_80024F60.c \
	src/decomp9-func_800253E8.c \
	src/decomp9-func_80025498.c \
	src/decomp9-func_80025500.c \
	src/decomp9-func_80025760.c \
	src/decomp9-func_800257E0.c \
	src/main.c

SRCS_G4 := \
	src/cdrom.c \
	src/overlay_memory.c \
	src/decomp4.c \
	src/akao_driver.c \
	src/controller.c

# Subset of SRCS_G4 (or any G4 object) whose original code uses bare
# `div $zero,...` and must be assembled WITHOUT --expand-div. List the .c here
# in addition to SRCS_G4; their objects get MASPSX_DIV_FLAG_G4 cleared below.
SRCS_G4_NOEXPAND :=

SRCS_CDK_G0 := \
	src/overlays/checkps/code.c \
	src/overlays/checkps/code2.c

SRCS_GCC_260_G0 := \
	src/field_runtime_text.c \
	src/card_callbacks.c

# Hand-written / splat-generated assembly (header, initialized data, sdata).
# Matched-C rodata (jump tables, including unk8's) is inlined into the C
# objects via ".rodata" linked subsegments; the rodata_data*.rodata.s files
# below are the standalone leftovers: pure data constants (D_ symbols
# referenced extern).
ASM_SRCS := \
	asm/header.s \
	asm/data/initialized.data.s \
	asm/data/sdata.data.s \
	asm/data/rodata_data0.rodata.s \
	asm/data/rodata_data1.rodata.s \
	asm/data/rodata_data2.rodata.s \
	asm/data/rodata_data3.rodata.s \
	asm/data/rodata_data4.rodata.s


# ─── Object File Paths ─────────────────────────────────────────────────────────
#
# patsubst turns  src/foo/bar.c  →  /staging/build/src/foo/bar.o
# This mirrors the source tree under the staging build directory.

OBJS_G0  			:= $(patsubst $(SRC_DIR)/%.c,$(STAGING)/build/$(SRC_DIR)/%.o,$(SRCS_G0))
OBJS_G4  			:= $(patsubst $(SRC_DIR)/%.c,$(STAGING)/build/$(SRC_DIR)/%.o,$(SRCS_G4))
OBJS_G4_NOEXPAND	:= $(patsubst $(SRC_DIR)/%.c,$(STAGING)/build/$(SRC_DIR)/%.o,$(SRCS_G4_NOEXPAND))
# Clear the div-expansion flag for the no-expand subset (target-specific var).
$(OBJS_G4_NOEXPAND): MASPSX_DIV_FLAG_G4 :=
OBJS_CDK_G0 		:= $(patsubst $(SRC_DIR)/%.c,$(STAGING)/build/$(SRC_DIR)/%.o,$(SRCS_CDK_G0))
OBJS_GCC_260_G0 	:= $(patsubst $(SRC_DIR)/%.c,$(STAGING)/build/$(SRC_DIR)/%.o,$(SRCS_GCC_260_G0))
OBJS_ASM 			:= $(patsubst $(ASM_DIR)/%.s,$(STAGING)/build/$(ASM_DIR)/%.o,$(ASM_SRCS))

OBJECTS  := $(OBJS_G0) $(OBJS_G4) $(OBJS_CDK_G0) $(OBJS_GCC_260_G0) $(OBJS_ASM)

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

# ── GCC 2.8.0, G0 (default) ──
$(OBJS_G0): $(STAGING)/build/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && $(CC) $(CFLAGS_G0) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS) -o build/$(SRC_DIR)/$*.o

# ── GCC 2.8.0, G4 ──
$(OBJS_G4): $(STAGING)/build/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && $(CC) $(CFLAGS_G4) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS_G4) -o build/$(SRC_DIR)/$*.o

# ── GCC 2.7.2 CDK, G0 ──
$(OBJS_CDK_G0): $(STAGING)/build/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && $(CC_272_CDK) $(CFLAGS_272_CDK_G0) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS_272_CDK) -o build/$(SRC_DIR)/$*.o

# ── GCC 2.6.0, G0 ──
$(OBJS_GCC_260_G0): $(STAGING)/build/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && $(CC_260) $(CFLAGS_260_G0) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS_260) -o build/$(SRC_DIR)/$*.o

# ── Hand-written assembly (header, data sections) ──
# These use --macro-inc because they contain ASPSX directives (dlabel, etc.)
# The pipeline: cat .s | maspsx (preprocess) | maspsx --run-assembler (assemble)
$(OBJS_ASM): $(STAGING)/build/$(ASM_DIR)/%.o: $(ASM_DIR)/%.s $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && cat $(ASM_DIR)/$*.s | \
		$(MASPSX) $(MASPSX_PP_FLAGS) | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS) -o build/$(ASM_DIR)/$*.o


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

.PHONY: all bin

# Default target: build the main SLUS executable
all: $(TARGET)
	@mkdir -p build
	@cp -r $(STAGING)/build/* build/
	@echo "Build complete: build/$(GAME).elf"

# Produce a raw binary from the ELF (for running on real hardware / emulators)
bin: all
	$(OBJCOPY) -O binary build/$(GAME).elf $(BIN)
