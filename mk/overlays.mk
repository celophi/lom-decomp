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
# G4 sources: the normal (--expand-div) set plus the no-expand subset. Files in
# overlay_<name>_gcc_g4_noexpand_srcs build -G4 but WITHOUT --expand-div (bare
# `div $zero,...`); list them only there, not in overlay_<name>_gcc_g4_srcs.
$(1)_GCC_G4_NOEXPAND_SRCS := $$(overlay_$(1)_gcc_g4_noexpand_srcs)
$(1)_GCC_G4_SRCS := $$(overlay_$(1)_gcc_g4_srcs) $$($(1)_GCC_G4_NOEXPAND_SRCS)
$(1)_CDK_SRCS    := $$(filter-out $$($(1)_GCC_SRCS) $$($(1)_GNU_SRCS) $$($(1)_GCC_G4_SRCS),$$($(1)_C_SRCS))
$(1)_GCC_OBJS    := $$(patsubst $$($(1)_SRC_DIR)/%.c,$(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/%.o,$$($(1)_GCC_SRCS))
$(1)_GNU_OBJS    := $$(patsubst $$($(1)_SRC_DIR)/%.c,$(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/%.o,$$($(1)_GNU_SRCS))
$(1)_GCC_G4_OBJS := $$(patsubst $$($(1)_SRC_DIR)/%.c,$(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/%.o,$$($(1)_GCC_G4_SRCS))
$(1)_GCC_G4_NOEXPAND_OBJS := $$(patsubst $$($(1)_SRC_DIR)/%.c,$(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/%.o,$$($(1)_GCC_G4_NOEXPAND_SRCS))
# Clear the div-expansion flag for the no-expand subset (target-specific var).
$$($(1)_GCC_G4_NOEXPAND_OBJS): MASPSX_DIV_FLAG_G4 :=
$(1)_CDK_OBJS    := $$(patsubst $$($(1)_SRC_DIR)/%.c,$(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/%.o,$$($(1)_CDK_SRCS))
$(1)_C_OBJS      := $$($(1)_CDK_OBJS) $$($(1)_GCC_OBJS) $$($(1)_GNU_OBJS) $$($(1)_GCC_G4_OBJS)

# ── Binary asset (only if overlay_<name>_asset is defined) ──
$(1)_ASSET_SRC := $$(overlay_$(1)_asset)
$(1)_ASSET_OBJ := $(STAGING)/$$($(1)_BUILD_DIR)/assets/$(1).o

# ── Hand-written/unmatched data asm (e.g. data/rodata.rodata.s) ──
# The splat-generated linker script pulls these .o files in directly by path
# (e.g. build/overlays/<name>/asm/overlays/<name>/data/rodata.rodata.o), so
# they must be assembled to that exact location even though they're excluded
# from the objdiff target-object set above.
$(1)_DATA_ASM  := $$(wildcard $$($(1)_ASM_DIR)/data/*.s)
$(1)_DATA_OBJS := $$(patsubst $$($(1)_ASM_DIR)/%.s,$(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_ASM_DIR)/%.o,$$($(1)_DATA_ASM))

$$($(1)_DATA_OBJS): $(STAGING)/$$($(1)_BUILD_DIR)/$$($(1)_ASM_DIR)/%.o: $$($(1)_ASM_DIR)/%.s $(COPY_SENTINEL)
	@mkdir -p $$(@D)
	cd $(STAGING) && cat $$($(1)_ASM_DIR)/$$*.s | \
		$(MASPSX_PP) $(MASPSX_PP_FLAGS) | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS_CDK) -o $$($(1)_BUILD_DIR)/$$($(1)_ASM_DIR)/$$*.o

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
		$(MASPSX_AS) $(INCLUDE_FLAGS) -no-pad-sections --aspsx-version=2.77 $$(MASPSX_DIV_FLAG_G4) -o $$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)/$$*.o

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
$$($(1)_TARGET): $(COPY_SENTINEL) $$($(1)_C_OBJS) $$($(1)_DATA_OBJS) $$(if $$($(1)_ASSET_SRC),$$($(1)_ASSET_OBJ)) $(STAGING)/$$($(1)_LINK_DIR)/$(1).ld
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
