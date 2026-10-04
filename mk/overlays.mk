# Compile and link overlays using the inputs from overlay-inputs.mk.
# Compiler choices belong in overlay-registry.mk; objdiff lives in analysis.mk.
#
# $(1) is the overlay name. $$ preserves variables until eval's second pass.
# Keep the compiler recipes explicit so their flags can be read here.

define overlay-build-rules

# Clear the div-expansion flag for the G4 no-expand subset (target-specific var).
$$($(1)_GCC_280_G4_NOEXPAND_OBJS): MASPSX_DIV_FLAG_G4 :=

# GCC 2.7.2 CDK G0 + maspsx.
$$($(1)_GCC_272_CDK_G0_OBJS): $(STAGING)/$$($(1)_C_OBJ_DIR)/%.o: $$($(1)_SRC_DIR)/%.c $(COPY_SENTINEL) | $(1)-validate
	@mkdir -p $$(@D)
	cd $(STAGING) && $(CC_272_CDK) $(CFLAGS_272_CDK_G0) $(VERSION_CPP_FLAGS) $(INCLUDE_FLAGS) -c $$($(1)_SRC_DIR)/$$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS_272_CDK) -o $$($(1)_C_OBJ_DIR)/$$*.o

# GCC 2.8.0 G0 + maspsx.
$$($(1)_GCC_280_G0_OBJS): $(STAGING)/$$($(1)_C_OBJ_DIR)/%.o: $$($(1)_SRC_DIR)/%.c $(COPY_SENTINEL) | $(1)-validate
	@mkdir -p $$(@D)
	cd $(STAGING) && $(CC) $(CFLAGS_G0) $(VERSION_CPP_FLAGS) $(INCLUDE_FLAGS) -c $$($(1)_SRC_DIR)/$$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS) -o $$($(1)_C_OBJ_DIR)/$$*.o

# GCC 2.8.0 G0 at -O0 + maspsx.
$$($(1)_GCC_280_G0_O0_OBJS): $(STAGING)/$$($(1)_C_OBJ_DIR)/%.o: $$($(1)_SRC_DIR)/%.c $(COPY_SENTINEL) | $(1)-validate
	@mkdir -p $$(@D)
	cd $(STAGING) && $(CC) $(CFLAGS_G0_O0) $(VERSION_CPP_FLAGS) $(INCLUDE_FLAGS) -c $$($(1)_SRC_DIR)/$$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS) -o $$($(1)_C_OBJ_DIR)/$$*.o

# GCC 2.8.0 G4 + maspsx.
$$($(1)_GCC_280_G4_OBJS): $(STAGING)/$$($(1)_C_OBJ_DIR)/%.o: $$($(1)_SRC_DIR)/%.c $(COPY_SENTINEL) | $(1)-validate
	@mkdir -p $$(@D)
	cd $(STAGING) && $(CC) $(CFLAGS_G4) $(VERSION_CPP_FLAGS) $(INCLUDE_FLAGS) -c $$($(1)_SRC_DIR)/$$*.c -S -o - | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $$(MASPSX_FLAGS_G4) -o $$($(1)_C_OBJ_DIR)/$$*.o

# GCC 2.7.2 GNU G0 + its own assembler.
$$($(1)_GCC_272_GNU_G0_OBJS): $(STAGING)/$$($(1)_C_OBJ_DIR)/%.o: $$($(1)_SRC_DIR)/%.c $(COPY_SENTINEL) | $(1)-validate
	@mkdir -p $$(@D)
	cd $(STAGING) && $(CC_272_GNU) $(CFLAGS_272_GNU_G0) $(VERSION_CPP_FLAGS) $(INCLUDE_FLAGS) -S $$($(1)_SRC_DIR)/$$*.c -o - | \
		$(AS_272_GNU) $(ASFLAGS_272_GNU) $$(overlay_$(1)_gcc_272_gnu_as_extra_flags_$$*) $(INCLUDE_FLAGS) -o $$($(1)_C_OBJ_DIR)/$$*.o
	cd $(STAGING) && $(OBJCOPY) $$(overlay_$(1)_gcc_272_gnu_objcopy_flags_$$*) \
		$$($(1)_C_OBJ_DIR)/$$*.o \
		$$($(1)_C_OBJ_DIR)/$$*.normalized.o
	mv $(STAGING)/$$($(1)_C_OBJ_DIR)/$$*.normalized.o $$@

# Assemble the remaining code and data from splat.
$$($(1)_LINK_ASM_OBJS): $(STAGING)/$$($(1)_ASM_OBJ_DIR)/%.o: $$($(1)_ASM_DIR)/%.s $(COPY_SENTINEL) | $(1)-validate
	@mkdir -p $$(@D)
	cd $(STAGING) && cat $$($(1)_ASM_DIR)/$$*.s | \
		$(MASPSX) $(MASPSX_PP_FLAGS) | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS_272_CDK) $$(overlay_$(1)_target_as_extra_flags_$$*) -o $$($(1)_ASM_OBJ_DIR)/$$*.o

$$($(1)_DATA_OBJS): $(STAGING)/$$($(1)_ASM_OBJ_DIR)/%.o: $$($(1)_ASM_DIR)/%.s $(COPY_SENTINEL) | $(1)-validate
	@mkdir -p $$(@D)
ifeq ($(DATA_AS_C),1)
	@# DATA_AS_C=1: .data comes from C that tools/data2c generates at build time,
	@# typed by this overlay's own declarations (see tools/data2c/README.md).
	@#   databin (one .incbin): data2c reads the blob itself.
	@#   data assembly:         assembled first, so data2c gets its relocations.
	@#   anything else (.rodata): assembled as usual.
	@mkdir -p $(STAGING)/datac/$$($(1)_BUILD_DIR)/$$(dir $$*)
	if grep -q '^\.section \.data' $$($(1)_ASM_DIR)/$$*.s && grep -q '^\.incbin' $$($(1)_ASM_DIR)/$$*.s; then \
		python3 tools/data2c/data2c.py --quiet --version $(VERSION) --image $(1) \
			--asm $$($(1)_ASM_DIR)/$$*.s -o $(STAGING)/datac/$$($(1)_BUILD_DIR)/$$*.c && \
		cd $(STAGING) && $(CC) $(CFLAGS_G0) -c datac/$$($(1)_BUILD_DIR)/$$*.c -S -o - | \
			$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS) -o $$($(1)_ASM_OBJ_DIR)/$$*.o; \
	elif grep -q '^\.section \.data' $$($(1)_ASM_DIR)/$$*.s; then \
		(cd $(STAGING) && cat $$($(1)_ASM_DIR)/$$*.s | \
			$(MASPSX) $(MASPSX_PP_FLAGS) | \
			$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS_272_CDK) -o datac/$$($(1)_BUILD_DIR)/$$*.asm.o) && \
		python3 tools/data2c/data2c.py --quiet --version $(VERSION) --image $(1) \
			--asm $$($(1)_ASM_DIR)/$$*.s --object $(STAGING)/datac/$$($(1)_BUILD_DIR)/$$*.asm.o \
			-o $(STAGING)/datac/$$($(1)_BUILD_DIR)/$$*.c && \
		cd $(STAGING) && $(CC) $(CFLAGS_G0) -c datac/$$($(1)_BUILD_DIR)/$$*.c -S -o - | \
			$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS) -o $$($(1)_ASM_OBJ_DIR)/$$*.o; \
	else \
		cd $(STAGING) && cat $$($(1)_ASM_DIR)/$$*.s | \
			$(MASPSX) $(MASPSX_PP_FLAGS) | \
			$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS_272_CDK) -o $$($(1)_ASM_OBJ_DIR)/$$*.o; \
	fi
else
	cd $(STAGING) && cat $$($(1)_ASM_DIR)/$$*.s | \
		$(MASPSX) $(MASPSX_PP_FLAGS) | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS_272_CDK) -o $$($(1)_ASM_OBJ_DIR)/$$*.o
endif

# The generated data takes its types from the overlay's C declarations and the
# headers they include, so it is rebuilt when any of them change.
ifeq ($(DATA_AS_C),1)
$$($(1)_DATA_OBJS): $$($(1)_C_SRCS) $$(call rwildcard,$$($(1)_SRC_DIR),*.h) $$(DATA_AS_C_HEADERS)
endif

# A standalone asset needs an object only when the registry requests one.
ifneq ($$($(1)_ASSET_SRC),)
$$($(1)_ASSET_OBJ): $(STAGING)/$$($(1)_ASSET_SRC) | $(1)-validate
	@mkdir -p $$(@D)
	cd $(STAGING) && $(OBJCOPY) -I binary -O elf32-tradlittlemips -B mips \
		$$($(1)_ASSET_SRC) $$($(1)_BUILD_DIR)/assets/$(1).o
endif

# Link the overlay.
# Track every object and linker script consumed by the link command.
# The standalone asset object is included only when asset_src is configured.
$$($(1)_TARGET): $(COPY_SENTINEL) \
	$$($(1)_C_OBJS) \
	$$($(1)_DATA_OBJS) \
	$$($(1)_LINK_ASM_OBJS) \
	$$(if $$($(1)_ASSET_SRC),$$($(1)_ASSET_OBJ)) \
	$$($(1)_LINKER_SCRIPTS) | $(1)-validate validate-assets
	@mkdir -p $$(@D)
	cd $(STAGING) && $(LD) -o $$($(1)_BUILD_DIR)/$(1).elf \
		-T $$($(1)_LINK_DIR)/$(1).ld \
		-T $$($(1)_LINK_DIR)/undefined_funcs_auto.txt \
		-T $$($(1)_LINK_DIR)/undefined_syms_auto.txt \
		$$(patsubst $(STAGING)/%,%,$$($(1)_C_OBJS)) \
		-Map $$($(1)_BUILD_DIR)/$(1).map
	@echo "Linked overlay: $(1)"

.PHONY: $(1)

$(1): $(1)-validate $$($(1)_TARGET)
	@mkdir -p $$($(1)_BUILD_DIR)
	@cp -a "$(STAGING)/$$($(1)_BUILD_DIR)/." "$$($(1)_BUILD_DIR)/"
	@echo "Overlay $(1) build complete."

endef

$(foreach ov,$(OVERLAYS),$(eval $(call overlay-build-rules,$(ov))))

.PHONY: overlays everything

overlays: $(OVERLAYS)
	@echo "All overlays built."

everything: all overlays
	@echo "Full build complete."
