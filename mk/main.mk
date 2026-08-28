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
	src/sdk/libapi/C114.c \
	src/sdk/libapi/A81.c \
	src/sdk/libc2/bcopy.c \
	src/sdk/libc2/bzero.c \
	src/sdk/libc2/memcpy.c \
	src/sdk/libc2/memset.c \
	src/sdk/libc2/rand.c \
	src/sdk/libc2/strcat.c \
	src/sdk/libc2/strcmp.c \
	src/sdk/libc2/strcpy.c \
	src/sdk/libc2/strlen.c \
	src/sdk/libc2/strncmp.c \
	src/sdk/libc2/strncpy.c \
	src/sdk/libc2/exit.c \
	src/sdk/libcard/C171.c \
	src/sdk/libcard/C172.c \
	src/sdk/libcard/A78.c \
	src/sdk/libcard/A79.c \
	src/sdk/libcard/A80.c \
	src/sdk/libcard/A93.c \
	src/sdk/libcard/CARD.c \
	src/sdk/libcard/INIT.c \
	src/sdk/libapi/PAD.c \
	src/sdk/libapi/A18.c \
	src/sdk/libapi/A19.c \
	src/sdk/libapi/A20.c \
	src/sdk/libapi/A21.c \
	src/sdk/libapi/L02.c \
	src/sdk/libapi/L03.c \
	src/sdk/libapi/PATCH.c \
	src/sdk/libapi/C68.c \
	src/sdk/libapi/CHCLRPAD.c \
	src/sdk/libcard/A74.c \
	src/sdk/libcard/A75.c \
	src/sdk/libcard/A76.c \
	src/sdk/libcard/PATCH.c \
	src/sdk/libcard/END.c \
	src/sdk/libcard/FORMAT.c \
	src/sdk/libcard/A92.c \
	src/sdk/libpress/PRESS.c \
	src/sdk/libc2/PRINTF.c \
	src/sdk/libc2/PRNT.c \
	src/sdk/libc2/CTYPE.c \
	src/sdk/libc2/MEMCHR.c \
	src/sdk/libc2/PUTCHAR.c \
	src/sdk/libpress/VLC_C.c \
	src/sdk/libpress/BUILD.c \
	src/sdk/libgpu/SYS.c \
	src/sdk/libapi/C73.c \
	src/sdk/libgpu/BREAK.c \
	src/sdk/libgpu/EXT.c \
	src/sdk/libgpu/P17.c \
	src/sdk/libgpu/P18.c \
	src/sdk/libgte/GEO_00.c \
	src/sdk/libgte/GEO_01.c \
	src/sdk/libgte/COR_02.c \
	src/sdk/libgte/COR_01.c \
	src/sdk/libgte/COR_03.c \
	src/sdk/libgte/MSC00.c \
	src/sdk/libgte/MSC01.c \
	src/sdk/libgte/MSC02.c \
	src/sdk/libgte/MTX_003.c \
	src/sdk/libgte/MTX_004.c \
	src/sdk/libgte/MTX_006.c \
	src/sdk/libgte/MTX_07.c \
	src/sdk/libgte/MTX_08.c \
	src/sdk/libgte/MTX_09.c \
	src/sdk/libgte/MTX_12.c \
	src/sdk/libgte/REG12.c \
	src/sdk/libgte/REG13.c \
	src/sdk/libgte/SMP_05.c \
	src/sdk/libgte/FGO_01.c \
	src/sdk/libgte/FGO_04.c \
	src/sdk/libgte/FGO_05.c \
	src/sdk/libgte/FGO_06.c \
	src/sdk/libgte/RMAT_01.c \
	src/sdk/libgte/RATAN.c \
	src/sdk/libgte/PATCHGTE.c \
	src/sdk/libcd/EVENT.c \
	src/sdk/libapi/A07.c \
	src/sdk/libcd/SYS.c \
	src/sdk/libcd/BIOS.c \
	src/sdk/libc2/PUTS.c \
	src/sdk/libcd/TYPE.c \
	src/sdk/libcd/S_002.c \
	src/sdk/libetc/VSYNC.c \
	src/sdk/libapi/L10.c \
	src/sdk/libetc/INTR.c \
	src/sdk/libapi/A23.c \
	src/sdk/libapi/A24.c \
	src/sdk/libapi/A25.c \
	src/sdk/libc2/SETJMP.c \
	src/sdk/libetc/INTR_VB.c \
	src/sdk/libetc/INTR_DMA.c \
	src/sdk/libetc/VMODE.c \
	src/sdk/libspu/S_I.c \
	src/sdk/libspu/S_INI.c \
	src/sdk/libspu/SPU.c \
	src/sdk/libspu/S_DCB.c \
	src/sdk/libspu/S_SI.c \
	src/sdk/libspu/S_SIA.c \
	src/sdk/libspu/S_STSA.c \
	src/sdk/libspu/S_SRA.c \
	src/sdk/libapi/A13.c \
	src/sdk/libapi/A32.c \
	src/sdk/libapi/COUNTER.c \
	src/sdk/libspu/S_Q.c \
	src/sdk/libspu/S_M_INIT.c \
	src/sdk/libspu/S_SR.c \
	src/sdk/libspu/S_M_UTIL.c \
	src/sdk/libspu/S_SIC.c \
	src/sdk/libspu/S_CB.c \
	src/sdk/libspu/S_R.c \
	src/sdk/libspu/S_W.c \
	src/sdk/libspu/S_STM.c \
	src/sdk/libspu/S_STC.c \
	src/sdk/libcard/C112.c \
	src/sdk/libapi/C159.c \
	src/sdk/libapi/A08.c \
	src/sdk/libapi/A09.c \
	src/sdk/libapi/A11.c \
	src/sdk/libapi/A12.c \
	src/sdk/libapi/A36.c \
	src/sdk/libapi/A37.c \
	src/sdk/libapi/A50.c \
	src/sdk/libapi/A52.c \
	src/sdk/libapi/A53.c \
	src/sdk/libapi/A54.c \
	src/sdk/libapi/A67.c \
	src/sdk/libapi/A68.c \
	src/sdk/libapi/A69.c \
	src/sdk/libapi/A91.c \
	src/sdk/libapi/SC2B.c \
	src/sdk/libapi/FIRST.c \
	src/sdk/libapi/A66.c \
	src/sdk/libspu/S_SRMT.c \
	src/sdk/libspu/S_CRWA.c \
	src/sdk/libapi/A10.c \
	src/sdk/libspu/S_GRMT.c \
	src/sdk/libsn/SNMAIN.c \
	src/sdk/libapi/C57.c \
	src/sdk/libpad/PADENTRY.c \
	src/sdk/libpad/PADMAIN.c \
	src/sdk/libpad/PADCMD.c \
	src/sdk/libpad/PADIF.c \
	src/sdk/libpad/PADPORTD.c \
	src/sdk/libpad/PADSEQD.c \
	src/sdk/libmcx/TMP_MCX.c \
	src/screen_transition.c \
	src/game_audio.c \
	src/akao_cmd.c \
	src/field_runtime.c \
	src/main.c

SRCS_G4 := \
	src/cdrom.c \
	src/overlay_memory.c \
	src/akao_sequencer.c \
	src/akao_driver.c \
	src/akao_driver_init_state.c \
	src/akao_driver_boot.c \
	src/controller.c \
	src/akao_voice.c \
	src/akao_xa_stream.c

# Subset of SRCS_G4 (or any G4 object) whose original code uses bare
# `div $zero,...` and must be assembled WITHOUT --expand-div. List the .c here
# in addition to SRCS_G4; their objects get MASPSX_DIV_FLAG_G4 cleared below.
SRCS_G4_NOEXPAND :=

SRCS_CDK_G0 := \
	src/overlays/checkps/init.c \
	src/overlays/checkps/font.c

SRCS_GCC_260_G0 := \
	src/field_runtime_text.c \
	src/field_runtime_glyph.c \
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

# field_runtime_glyph.c uses the same 2.6.0 rule as the rest of the 260 list,
# but built at -O1 (its glyph helpers only match at -O1). Target-specific
# override, mirroring the MASPSX_DIV_FLAG_G4 pattern above.
$(STAGING)/build/$(SRC_DIR)/field_runtime_glyph.o: CFLAGS_260_G0 := $(CFLAGS_260_G0_O1)

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
