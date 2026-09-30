# Select each overlay's sources, object paths and linker inputs.
# Compiler assignments live in overlay-registry.mk. Build recipes are in
# overlays.mk; objdiff rules are in analysis.mk.
#
# $(1) is the overlay name. eval expands each template twice, so $$ keeps
# per-overlay variables for the second pass: $$($(1)_SRC_DIR) becomes
# $(checkps_SRC_DIR) when this template is called for checkps.

# A version can use assembly for an entire overlay or just specific C files.
# Keep that selection here so the registry only needs to list the sources.
overlay-sources = $(if $(call has-tu-layout,$(1)),\
	$(filter-out $(ASM_UNITS),$(overlay_$(1)_$(2)_srcs)))

# Return every word that occurs more than once in a list.
duplicate-words = $(sort $(foreach item,$(1),$(if $(word 2,$(filter $(item),$(1))),$(item))))

# Registry validation is deferred until an overlay build requests it. This lets
# `make splat` parse in a clean checkout before generated directories exist.
DUPLICATE_OVERLAYS = $(call duplicate-words,$(OVERLAYS))
OVERLAY_REQUIRED_DIRS = $(foreach name,$(OVERLAYS),\
	src/overlays/$(name) $(ASM_DIR)/overlays/$(name) $(LINKER_DIR)/overlays/$(name))
MISSING_OVERLAY_DIRS = $(filter-out $(wildcard $(OVERLAY_REQUIRED_DIRS)),$(OVERLAY_REQUIRED_DIRS))

.PHONY: validate-overlay-registry
validate-overlay-registry:
	$(if $(DUPLICATE_OVERLAYS),$(error Overlays registered more than once: $(DUPLICATE_OVERLAYS)))
	$(if $(MISSING_OVERLAY_DIRS),$(error Registered overlay directories not found: $(MISSING_OVERLAY_DIRS)))
	@:

define overlay-input-rules

# Derived paths for overlay '$(1)'
$(1)_SRC_DIR   := src/overlays/$(1)
$(1)_ASM_DIR   := $(ASM_DIR)/overlays/$(1)
$(1)_LINK_DIR  := $(LINKER_DIR)/overlays/$(1)
$(1)_BUILD_DIR := $(BUILD_DIR)/overlays/$(1)
$(1)_TARGET    := $(STAGING)/$$($(1)_BUILD_DIR)/$(1).elf
# Object paths repeat the source tree under the overlay build directory.
# For CHECKPS, C objects go in build/us/overlays/checkps/src/overlays/checkps/.
$(1)_C_OBJ_DIR   := $$($(1)_BUILD_DIR)/$$($(1)_SRC_DIR)
$(1)_ASM_OBJ_DIR := $$($(1)_BUILD_DIR)/$$($(1)_ASM_DIR)
$(1)_LINKER_SCRIPTS := \
	$(STAGING)/$$($(1)_LINK_DIR)/$(1).ld \
	$(STAGING)/$$($(1)_LINK_DIR)/undefined_funcs_auto.txt \
	$(STAGING)/$$($(1)_LINK_DIR)/undefined_syms_auto.txt

# Discover and validate source routing
$(1)_GCC_272_CDK_G0_SRCS := $$(call overlay-sources,$(1),gcc_272_cdk_g0)
$(1)_GCC_272_GNU_G0_SRCS := $$(call overlay-sources,$(1),gcc_272_gnu_g0)
$(1)_GCC_280_G0_SRCS := $$(call overlay-sources,$(1),gcc_280_g0)
$(1)_GCC_280_G0_O0_SRCS := $$(call overlay-sources,$(1),gcc_280_g0_o0)
$(1)_GCC_280_G4_SRCS := $$(call overlay-sources,$(1),gcc_280_g4)
$(1)_GCC_280_G4_NOEXPAND_SRCS := $$(call overlay-sources,$(1),gcc_280_g4_noexpand)
$(1)_ROUTED_SRCS = \
	$$($(1)_GCC_272_CDK_G0_SRCS) \
	$$($(1)_GCC_272_GNU_G0_SRCS) \
	$$($(1)_GCC_280_G0_SRCS) \
	$$($(1)_GCC_280_G0_O0_SRCS) \
	$$($(1)_GCC_280_G4_SRCS) \
	$$($(1)_GCC_280_G4_NOEXPAND_SRCS)
# Generated unk*.c files are gitignored and splat does not remove outputs from
# older configurations. Treat tracked C files and explicitly routed generated
# files as build inputs so stale ignored files cannot enter the build by accident.
# An overlay without a C layout for this version does not build the C sources.
$(1)_EXISTING_C_SRCS := $$(wildcard $$($(1)_SRC_DIR)/*.c)
$(1)_TRACKED_C_SRCS := $$(if $$(call has-tu-layout,$(1)),\
	$$(shell git ls-files -- '$$($(1)_SRC_DIR)/*.c' 2>/dev/null))
$(1)_TRACKED_C_SRCS := $$(filter-out $(ASM_UNITS),\
	$$(filter $$($(1)_EXISTING_C_SRCS),$$($(1)_TRACKED_C_SRCS)))
$(1)_C_SRCS = $$(sort $$($(1)_TRACKED_C_SRCS) \
	$$(filter $$($(1)_ROUTED_SRCS),$$($(1)_EXISTING_C_SRCS)))
$(1)_UNROUTED_SRCS = $$(filter-out $$($(1)_ROUTED_SRCS),$$($(1)_C_SRCS))
$(1)_UNKNOWN_ROUTED_SRCS = $$(filter-out $$($(1)_C_SRCS),$$($(1)_ROUTED_SRCS))
$(1)_DUPLICATE_ROUTED_SRCS = $$(call duplicate-words,$$($(1)_ROUTED_SRCS))

.PHONY: $(1)-validate
$(1)-validate: validate-overlay-registry
	$$(if $$($(1)_UNROUTED_SRCS),$$(error Overlay $(1) has unrouted source(s): $$($(1)_UNROUTED_SRCS)))
	$$(if $$($(1)_UNKNOWN_ROUTED_SRCS),$$(error Overlay $(1) routes unknown source(s): $$($(1)_UNKNOWN_ROUTED_SRCS)))
	$$(if $$($(1)_DUPLICATE_ROUTED_SRCS),$$(error Overlay $(1) routes source(s) more than once: $$($(1)_DUPLICATE_ROUTED_SRCS)))
	@:

$(1)_GCC_280_G4_ALL_SRCS := $$($(1)_GCC_280_G4_SRCS) $$($(1)_GCC_280_G4_NOEXPAND_SRCS)

# Derive object paths
$(1)_GCC_280_G0_OBJS := $$(patsubst $$($(1)_SRC_DIR)/%.c,$(STAGING)/$$($(1)_C_OBJ_DIR)/%.o,$$($(1)_GCC_280_G0_SRCS))
$(1)_GCC_280_G0_O0_OBJS := $$(patsubst $$($(1)_SRC_DIR)/%.c,$(STAGING)/$$($(1)_C_OBJ_DIR)/%.o,$$($(1)_GCC_280_G0_O0_SRCS))
$(1)_GCC_280_G4_OBJS := $$(patsubst $$($(1)_SRC_DIR)/%.c,$(STAGING)/$$($(1)_C_OBJ_DIR)/%.o,$$($(1)_GCC_280_G4_ALL_SRCS))
$(1)_GCC_280_G4_NOEXPAND_OBJS := $$(patsubst $$($(1)_SRC_DIR)/%.c,$(STAGING)/$$($(1)_C_OBJ_DIR)/%.o,$$($(1)_GCC_280_G4_NOEXPAND_SRCS))
$(1)_GCC_272_GNU_G0_OBJS := $$(patsubst $$($(1)_SRC_DIR)/%.c,$(STAGING)/$$($(1)_C_OBJ_DIR)/%.o,$$($(1)_GCC_272_GNU_G0_SRCS))
$(1)_GCC_272_CDK_G0_OBJS := $$(patsubst $$($(1)_SRC_DIR)/%.c,$(STAGING)/$$($(1)_C_OBJ_DIR)/%.o,$$($(1)_GCC_272_CDK_G0_SRCS))
$(1)_C_OBJS := \
	$$($(1)_GCC_272_CDK_G0_OBJS) \
	$$($(1)_GCC_280_G0_OBJS) \
	$$($(1)_GCC_280_G0_O0_OBJS) \
	$$($(1)_GCC_272_GNU_G0_OBJS) \
	$$($(1)_GCC_280_G4_OBJS)

# Optional standalone binary object
# Use asset_src only when the linker script expects assets/<name>.o. This is
# separate from splat databin assets included by generated assembly.
$(1)_ASSET_SRC := $$(overlay_$(1)_asset_src)
$(1)_ASSET_OBJ := $(STAGING)/$$($(1)_BUILD_DIR)/assets/$(1).o

# Splat-generated data assembly
# These files may use .incbin to include gitignored data from assets/.
# The splat-generated linker script pulls these .o files in directly by path
# (e.g. build/us/overlays/<name>/asm/us/overlays/<name>/data/rodata.rodata.o), so
# they must be assembled to that exact location even though they're excluded
# from the objdiff target objects in analysis.mk.
$(1)_DATA_ASM  := $$(call rwildcard,$$($(1)_ASM_DIR)/data,*.s)
$(1)_DATA_OBJS := $$(patsubst $$($(1)_ASM_DIR)/%.s,$(STAGING)/$$($(1)_ASM_OBJ_DIR)/%.o,$$($(1)_DATA_ASM))

# Splat's dependency file identifies the standalone assembly segments still
# used by the linker. Do not glob all assembly: old splits remain on disk.
$(1)_LINK_INPUTS := $$(file <$$($(1)_LINK_DIR)/$(1).d)
$(1)_LINK_ASM_OBJS := $$(filter $$($(1)_ASM_OBJ_DIR)/%.o,$$($(1)_LINK_INPUTS))
$(1)_LINK_ASM_OBJS := $$(filter-out $$($(1)_ASM_OBJ_DIR)/data/%,$$($(1)_LINK_ASM_OBJS))
$(1)_LINK_ASM_OBJS := $$(addprefix $(STAGING)/,$$(sort $$($(1)_LINK_ASM_OBJS)))

endef

$(foreach ov,$(OVERLAYS),$(eval $(call overlay-input-rules,$(ov))))

.PHONY: validate-overlays
validate-overlays: $(addsuffix -validate,$(OVERLAYS))
