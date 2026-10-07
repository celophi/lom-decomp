# Splat ROM extraction

OVERLAY_SPLAT_CONFIGS := $(wildcard $(CONFIG_DIR)/overlays/*.yaml)
SPLAT_CONFIGS := $(wildcard $(CONFIG_DIR)/$(GAME).yaml) $(OVERLAY_SPLAT_CONFIGS)
DECOMPRESSED_OVERLAYS := $(patsubst $(CONFIG_DIR)/overlays/%.yaml,$(BUILD_DIR)/decompressed/%,$(OVERLAY_SPLAT_CONFIGS))

# Keep the format byte so splat's existing offsets still start at 0x1. The
# preparation tool checks the compressed disc hash before writing any output.
$(BUILD_DIR)/decompressed/%.BIN: $(ROM_BIN_DIR)/%.BIN $(CONFIG_DIR)/overlays/%.BIN.yaml \
		tools/compression/decompress_overlay.py tools/compression/decompress.py
	python3 -m tools.compression.decompress_overlay --config $(word 2,$^) $< $@

# Use every processor available to the current host/container by default.
# SPLAT_JOBS remains overridable for constrained environments.
SPLAT_JOBS ?= $(shell nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)

.PHONY: decompress splat

decompress: $(DECOMPRESSED_OVERLAYS)

splat: decompress
	$(if $(SPLAT_CONFIGS),,$(error No splat configs found under $(CONFIG_DIR)/ for VERSION=$(VERSION)))
	@printf '%s\n' $(SPLAT_CONFIGS) | xargs -n 1 -P $(SPLAT_JOBS) splat split
