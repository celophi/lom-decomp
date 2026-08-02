# ============================================================================
# Splat ROM extraction
# ============================================================================

SPLAT_CONFIGS := config/$(GAME).yaml $(wildcard config/overlays/*.yaml)

# Use every processor available to the current host/container by default.
# SPLAT_JOBS remains overridable for constrained environments.
SPLAT_JOBS ?= $(shell nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)

.PHONY: splat

splat:
	@printf '%s\n' $(SPLAT_CONFIGS) | xargs -n 1 -P $(SPLAT_JOBS) splat split
