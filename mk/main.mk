# Main executable

include mk/main-sources.mk

# Jump tables used by matched C live in the C objects. These rodata files
# contain the remaining standalone constants referenced by D_ symbols.
ASM_SRCS := \
	$(ASM_DIR)/header.s \
	$(ASM_DIR)/data/initialized.data.s \
	$(ASM_DIR)/data/sdata.data.s \
	$(ASM_DIR)/data/rodata_data0.rodata.s \
	$(ASM_DIR)/data/rodata_data1.rodata.s \
	$(ASM_DIR)/data/rodata_data2.rodata.s \
	$(ASM_DIR)/data/rodata_data3.rodata.s \
	$(ASM_DIR)/data/rodata_data4.rodata.s


# JP contains the pet-transfer implementation; the US unit is a compatibility stub.
ifeq ($(VERSION),jp)
SRCS_GCC_260_G0 := $(filter-out src/main/card_callbacks.c,$(SRCS_GCC_260_G0))
SRCS_G0 += src/main/card_callbacks.c
endif

# Some versions still use assembly for individual files or the whole image.
SRCS_G0 := $(filter-out $(ASM_UNITS),$(SRCS_G0))
SRCS_G4 := $(filter-out $(ASM_UNITS),$(SRCS_G4))
SRCS_GCC_260_G0 := $(filter-out $(ASM_UNITS),$(SRCS_GCC_260_G0))

# Read splat's object list so old assembly left by previous splits stays out.
# The dependency file won't exist until `make splat` has run.
MAIN_LINK_DEPS := $(LINKER_DIR)/$(GAME).d
MAIN_LINK_INPUTS := $(if $(wildcard $(MAIN_LINK_DEPS)),$(file <$(MAIN_LINK_DEPS)))
MAIN_LINK_ASM_OBJS := $(filter $(BUILD_DIR)/$(ASM_DIR)/%.o,$(MAIN_LINK_INPUTS))
MAIN_LINK_ASM_SRCS := $(patsubst $(BUILD_DIR)/%.o,%.s,$(MAIN_LINK_ASM_OBJS))

ifneq ($(filter-out src/overlays/%,$(ASM_UNITS)),)
ASM_SRCS := $(sort $(ASM_SRCS) $(MAIN_LINK_ASM_SRCS))
endif

ifeq ($(call has-tu-layout,main),)
SRCS_G0 :=
SRCS_G4 :=
SRCS_GCC_260_G0 :=
ASM_SRCS := $(sort $(MAIN_LINK_ASM_SRCS))
endif

# Mirror source paths under the staging build directory.
OBJS_G0         := $(patsubst $(SRC_DIR)/%.c,$(STAGING)/$(BUILD_DIR)/$(SRC_DIR)/%.o,$(SRCS_G0))
OBJS_G4         := $(patsubst $(SRC_DIR)/%.c,$(STAGING)/$(BUILD_DIR)/$(SRC_DIR)/%.o,$(SRCS_G4))
OBJS_GCC_260_G0  := $(patsubst $(SRC_DIR)/%.c,$(STAGING)/$(BUILD_DIR)/$(SRC_DIR)/%.o,$(SRCS_GCC_260_G0))
OBJS_ASM        := $(patsubst $(ASM_DIR)/%.s,$(STAGING)/$(BUILD_DIR)/$(ASM_DIR)/%.o,$(ASM_SRCS))

# Preserve the original glyph instructions and explicit delay slots.
$(STAGING)/$(BUILD_DIR)/$(SRC_DIR)/main/field_runtime_glyph.o: MASPSX_FLAGS_260 += --passthrough

OBJECTS  := $(OBJS_G0) $(OBJS_G4) $(OBJS_GCC_260_G0) $(OBJS_ASM)

# Compile C to assembly, then pipe it through maspsx to make the object.

# GCC 2.8.0, G0 (default)
$(OBJS_G0): $(STAGING)/$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && $(CC) $(CFLAGS_G0) $(VERSION_CPP_FLAGS) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS) -o $(BUILD_DIR)/$(SRC_DIR)/$*.o

# GCC 2.8.0, G4
$(OBJS_G4): $(STAGING)/$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && $(CC) $(CFLAGS_G4) $(VERSION_CPP_FLAGS) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS_G4) -o $(BUILD_DIR)/$(SRC_DIR)/$*.o

# GCC 2.6.0, G0
$(OBJS_GCC_260_G0): $(STAGING)/$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && $(CC_260) $(CFLAGS_260_G0) $(VERSION_CPP_FLAGS) $(INCLUDE_FLAGS) -c $(SRC_DIR)/$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS_260) -o $(BUILD_DIR)/$(SRC_DIR)/$*.o

# Hand-written assembly (header, data sections)
# These use --macro-inc because they contain ASPSX directives (dlabel, etc.)
# The pipeline: cat .s | maspsx (preprocess) | maspsx --run-assembler (assemble)
$(OBJS_ASM): $(STAGING)/$(BUILD_DIR)/$(ASM_DIR)/%.o: $(ASM_DIR)/%.s $(COPY_SENTINEL)
	@mkdir -p $(@D)
ifeq ($(DATA_AS_C),1)
	@# DATA_AS_C=1: the executable's .data (initialized.data, sdata.data) comes
	@# from C generated at build time, as for overlays in mk/overlays.mk.
	@mkdir -p $(STAGING)/datac/$(BUILD_DIR)/$(dir $*)
	if case '$*' in data/*) true;; *) false;; esac && grep -q '^\.section \.data' $(ASM_DIR)/$*.s; then \
		(cd $(STAGING) && cat $(ASM_DIR)/$*.s | \
			$(MASPSX) $(MASPSX_PP_FLAGS) | \
			$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS) -o datac/$(BUILD_DIR)/$*.asm.o) && \
		python3 tools/verification/data2c/data2c.py --quiet --version $(VERSION) --image slus \
			--asm $(ASM_DIR)/$*.s --object $(STAGING)/datac/$(BUILD_DIR)/$*.asm.o \
			-o $(STAGING)/datac/$(BUILD_DIR)/$*.c && \
		cd $(STAGING) && $(CC) $(CFLAGS_G0) -c datac/$(BUILD_DIR)/$*.c -S -o - | \
			$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS) -o $(BUILD_DIR)/$(ASM_DIR)/$*.o; \
	else \
		cd $(STAGING) && cat $(ASM_DIR)/$*.s | \
			$(MASPSX) $(MASPSX_PP_FLAGS) | \
			$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS) -o $(BUILD_DIR)/$(ASM_DIR)/$*.o; \
	fi
else
	cd $(STAGING) && cat $(ASM_DIR)/$*.s | \
		$(MASPSX) $(MASPSX_PP_FLAGS) | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS) -o $(BUILD_DIR)/$(ASM_DIR)/$*.o
endif


# DATA_AS_C=1: the executable's generated data is typed by its C and headers.
ifeq ($(DATA_AS_C),1)
$(filter $(STAGING)/$(BUILD_DIR)/$(ASM_DIR)/data/%,$(OBJS_ASM)): \
	$(call rwildcard,src/main,*.c) $(wildcard src/psyq/*/*.c) $(DATA_AS_C_HEADERS)
endif

# Link the main executable

MAIN_LINKER_SCRIPTS := $(addprefix $(STAGING)/$(LINKER_DIR)/,\
	$(GAME).ld undefined_syms_auto.txt undefined_funcs_auto.txt)

$(TARGET): $(COPY_SENTINEL) $(OBJECTS) $(MAIN_LINKER_SCRIPTS)
	@mkdir -p $(STAGING)/$(BUILD_DIR)
	cd $(STAGING) && $(LD) -o $(BUILD_DIR)/$(GAME).elf \
		-T $(LINKER_DIR)/$(GAME).ld \
		-T $(LINKER_DIR)/undefined_syms_auto.txt \
		-T $(LINKER_DIR)/undefined_funcs_auto.txt \
		$(patsubst $(STAGING)/%,%,$(OBJECTS)) \
		-Map $(BUILD_DIR)/$(GAME).map

.PHONY: all bin

# Copy the completed build back out of staging.
all: $(TARGET)
	@mkdir -p $(BUILD_DIR)
	@cp -r $(STAGING)/$(BUILD_DIR)/* $(BUILD_DIR)/
	@echo "Build complete: $(BUILD_DIR)/$(GAME).elf"

# Convert the ELF to the raw executable image.
bin: all
	$(OBJCOPY) -O binary $(BUILD_DIR)/$(GAME).elf $(BIN)
