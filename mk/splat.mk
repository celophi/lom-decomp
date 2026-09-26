# ============================================================================
# Splat ROM extraction
# ============================================================================

SPLAT_CONFIGS := $(wildcard $(CONFIG_DIR)/$(GAME).yaml $(CONFIG_DIR)/overlays/*.yaml)

# Use every processor available to the current host/container by default.
# SPLAT_JOBS remains overridable for constrained environments.
SPLAT_JOBS ?= $(shell nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)

.PHONY: splat

splat:
	$(if $(SPLAT_CONFIGS),,$(error No splat configs found under $(CONFIG_DIR)/ for VERSION=$(VERSION)))
	@printf '%s\n' $(SPLAT_CONFIGS) | xargs -n 1 -P $(SPLAT_JOBS) splat split
