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
	src/psyq/libspu/S_SNC.c \
	src/psyq/libspu/S_GVEX.c \
	src/psyq/libspu/S_SRMD.c \
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
	src/psyq/libsn/SNMAIN.c \
	src/psyq/libapi/C57.c \
	src/psyq/libpad/PADENTRY.c \
	src/psyq/libpad/PADMAIN.c \
	src/psyq/libpad/PADCMD.c \
	src/psyq/libpad/PADIF.c \
	src/psyq/libpad/PADPORTD.c \
	src/psyq/libpad/PADSEQD.c \
	src/psyq/libmcx/TMP_MCX.c \
	src/screen_transition.c \
	src/game_audio.c \
	src/akao_cmd.c \
	src/field_runtime.c \
	src/main.c

SRCS_G4 := \
	src/cdrom.c \
	src/cdrom_decompress.c \
	src/overlay_memory.c \
	src/akao_sequencer.c \
	src/akao_driver.c \
	src/akao_driver_init_state.c \
	src/akao_driver_boot.c \
	src/controller.c \
	src/akao_voice.c \
	src/akao_control.c \
	src/akao_xa_stream.c

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
	$(ASM_DIR)/header.s \
	$(ASM_DIR)/data/initialized.data.s \
	$(ASM_DIR)/data/sdata.data.s \
	$(ASM_DIR)/data/rodata_data0.rodata.s \
	$(ASM_DIR)/data/rodata_data1.rodata.s \
	$(ASM_DIR)/data/rodata_data2.rodata.s \
	$(ASM_DIR)/data/rodata_data3.rodata.s \
	$(ASM_DIR)/data/rodata_data4.rodata.s


# The lists above are the North American translation-unit layout. Versions
# without a split TU layout (see HAS_TU_LAYOUT in mk/version.mk) build no C
# objects yet; their main executable links purely from splat assembly. Splat's
# dependency file lists exactly the objects the linker script uses (it is
# absent until `make splat` has run for the version).
ifeq ($(HAS_TU_LAYOUT),)
SRCS_G0 :=
SRCS_G4 :=
SRCS_GCC_260_G0 :=
MAIN_LINK_DEPS := $(LINKER_DIR)/$(GAME).d
ASM_SRCS := $(sort $(patsubst $(BUILD_DIR)/%.o,%.s,$(filter $(BUILD_DIR)/$(ASM_DIR)/%.o,\
	$(if $(wildcard $(MAIN_LINK_DEPS)),$(file <$(MAIN_LINK_DEPS))))))
endif


# ─── Object File Paths ─────────────────────────────────────────────────────────
#
# patsubst turns  src/foo/bar.c  →  /staging/build/<version>/src/foo/bar.o
# This mirrors the source tree under the staging build directory.

OBJS_G0  			:= $(patsubst $(SRC_DIR)/%.c,$(STAGING)/$(BUILD_DIR)/$(SRC_DIR)/%.o,$(SRCS_G0))
OBJS_G4  			:= $(patsubst $(SRC_DIR)/%.c,$(STAGING)/$(BUILD_DIR)/$(SRC_DIR)/%.o,$(SRCS_G4))
OBJS_GCC_260_G0 	:= $(patsubst $(SRC_DIR)/%.c,$(STAGING)/$(BUILD_DIR)/$(SRC_DIR)/%.o,$(SRCS_GCC_260_G0))
OBJS_ASM 			:= $(patsubst $(ASM_DIR)/%.s,$(STAGING)/$(BUILD_DIR)/$(ASM_DIR)/%.o,$(ASM_SRCS))

# Preserve the original glyph instructions and explicit delay slots.
$(STAGING)/$(BUILD_DIR)/$(SRC_DIR)/field_runtime_glyph.o: MASPSX_FLAGS_260 += --passthrough

OBJECTS  := $(OBJS_G0) $(OBJS_G4) $(OBJS_GCC_260_G0) $(OBJS_ASM)

# ============================================================================
#  Compilation Rules — Main SLUS
# ============================================================================
#
# Static pattern rules:
#   $(TARGETS): $(STAGING)/$(BUILD_DIR)/src/%.o: src/%.c
#   reads as: "for each file in TARGETS, the .o comes from the matching .c"
#
# The recipe pipes GCC asm output directly into maspsx:
#   gcc -S -o -     → write asm to stdout
#   | maspsx.py ... → translate to ASPSX syntax and assemble into .o

# ── GCC 2.8.0, G0 (default) ──
$(OBJS_G0): $(STAGING)/$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && $(CC) $(CFLAGS_G0) $(VERSION_CPP_FLAGS) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS) -o $(BUILD_DIR)/$(SRC_DIR)/$*.o

# ── GCC 2.8.0, G4 ──
$(OBJS_G4): $(STAGING)/$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && $(CC) $(CFLAGS_G4) $(VERSION_CPP_FLAGS) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS_G4) -o $(BUILD_DIR)/$(SRC_DIR)/$*.o

# ── GCC 2.6.0, G0 ──
$(OBJS_GCC_260_G0): $(STAGING)/$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && $(CC_260) $(CFLAGS_260_G0) $(VERSION_CPP_FLAGS) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS_260) -o $(BUILD_DIR)/$(SRC_DIR)/$*.o

# ── Hand-written assembly (header, data sections) ──
# These use --macro-inc because they contain ASPSX directives (dlabel, etc.)
# The pipeline: cat .s | maspsx (preprocess) | maspsx --run-assembler (assemble)
$(OBJS_ASM): $(STAGING)/$(BUILD_DIR)/$(ASM_DIR)/%.o: $(ASM_DIR)/%.s $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && cat $(ASM_DIR)/$*.s | \
		$(MASPSX) $(MASPSX_PP_FLAGS) | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS) -o $(BUILD_DIR)/$(ASM_DIR)/$*.o


# ============================================================================
#  Linking — Main SLUS
# ============================================================================

$(TARGET): $(COPY_SENTINEL) $(OBJECTS) $(STAGING)/$(LINKER_DIR)/$(GAME).ld
	@mkdir -p $(STAGING)/$(BUILD_DIR)
	cd $(STAGING) && $(LD) -o $(BUILD_DIR)/$(GAME).elf \
		-T $(LINKER_DIR)/$(GAME).ld \
		-T $(LINKER_DIR)/undefined_syms_auto.txt \
		-T $(LINKER_DIR)/undefined_funcs_auto.txt \
		$(patsubst $(STAGING)/%,%,$(OBJECTS)) \
		-Map $(BUILD_DIR)/$(GAME).map

.PHONY: all bin

# Default target: build the main SLUS executable
all: $(TARGET)
	@mkdir -p $(BUILD_DIR)
	@cp -r $(STAGING)/$(BUILD_DIR)/* $(BUILD_DIR)/
	@echo "Build complete: $(BUILD_DIR)/$(GAME).elf"

# Produce a raw binary from the ELF (for running on real hardware / emulators)
bin: all
	$(OBJCOPY) -O binary $(BUILD_DIR)/$(GAME).elf $(BIN)
