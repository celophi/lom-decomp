# ============================================================================
# ROM verification: main executable and compressed overlays
# ============================================================================
#
# To prove a compressed overlay is byte-perfect, reproduce the file stored in
# disc/<version>/BIN/ and compare its SHA1:
#
#   1. Link the overlay ELF.
#   2. Convert the ELF to its raw decompressed binary.
#   3. Compress the raw binary with tools/compressor/compressor.py.
#   4. Prepend the 0x01 compression-format byte skipped by the splat configs.
#   5. SHA1-compare the result with the original overlay BIN.
#
# On a match, the overlay name is added to build/<version>/complete_overlays.txt.
# generate_objdiff_config.py uses that manifest to mark its objdiff units as
# complete.
#
# The compressor reproduces the original 1999 encoder byte for byte on all 17
# disc overlays, so any overlay that links to an exact raw image can be
# verified this way. See tools/compressor/README.md.

# Overlays whose linked ELF reproduces the original decompressed image, and so
# can be compressed back into an exact replica of the disc file.
#
# Note that objdiff reporting 100% on every function is NOT sufficient: it pairs
# symbols by name and normalizes relocations, so a whole-TU section shift is
# invisible to it. Add a name here only once `make verify-<name>` actually
# passes.
VERIFIED_OVERLAYS_us := gover movie gname checkps title gosub golem niki addhero menu cload zukan carda shop wsel field wmap
VERIFIED_OVERLAYS_jp := gover movie checkps golem niki addhero menu cload zukan carda shop wsel wmap
VERIFIED_OVERLAYS := $(VERIFIED_OVERLAYS_$(VERSION))

# Overlays whose linked ELF reproduces the original decompressed image, but
# whose disc stream the compressor cannot reproduce yet (its selection rules
# were recovered from the US overlays). These are checked at the raw-image
# level only: the linked ELF against the decompressed disc file. Move a name to
# VERIFIED_OVERLAYS_<version> once `make verify-compressor` passes for it.
RAW_VERIFIED_OVERLAYS_us :=
RAW_VERIFIED_OVERLAYS_jp := field gname gosub title
RAW_VERIFIED_OVERLAYS := $(RAW_VERIFIED_OVERLAYS_$(VERSION))

# Make has no upper-case function; overlay BINs are named in upper case.
upper-case = $(shell echo '$(1)' | tr '[:lower:]' '[:upper:]')

.PHONY: verify-bins

# ── Per-overlay verification rules ───────────────────────────────────────────
#
#   $(1) = overlay name, lower case (e.g. "gover")
#   $(2) = overlay BIN basename, upper case (e.g. "GOVER")

define overlay-raw-rule

$(BUILD_DIR)/overlays/$(1)/$(1).raw: $(1)
	@mkdir -p $$(@D)
	$(OBJCOPY) -O binary $(STAGING)/$(BUILD_DIR)/overlays/$(1)/$(1).elf $$@.tmp
	mv $$@.tmp $$@

endef

define compressed-overlay-rules

.PHONY: verify-$(1)

$(call overlay-raw-rule,$(1))

$(BUILD_DIR)/overlays/$(1)/$(2).BIN: $(BUILD_DIR)/overlays/$(1)/$(1).raw
	python3 tools/compressor/compressor.py $$< $$@.payload
	{ printf '\001'; cat $$@.payload; } > $$@.tmp
	mv $$@.tmp $$@
	rm -f $$@.payload

verify-$(1): $(BUILD_DIR)/overlays/$(1)/$(2).BIN
	@mkdir -p $(BUILD_DIR)
	@set -eu; \
		expected=$$$$(sha1sum $(ROM_BIN_DIR)/$(2).BIN | awk '{print $$$$1}'); \
		actual=$$$$(sha1sum $$< | awk '{print $$$$1}'); \
		echo "$(2).BIN expected: $$$$expected"; \
		echo "$(2).BIN actual:   $$$$actual"; \
		if [ "$$$$expected" = "$$$$actual" ]; then \
			echo "[OK] $(2).BIN matches original ROM"; \
			grep -qxF $(1) $(COMPLETE_MANIFEST) 2>/dev/null || echo $(1) >> $(COMPLETE_MANIFEST); \
		else \
			echo "[FAIL] $(2).BIN sha1 mismatch"; \
			exit 1; \
		fi

endef

$(foreach name,$(VERIFIED_OVERLAYS),\
	$(eval $(call compressed-overlay-rules,$(name),$(call upper-case,$(name)))))

# Raw-image check for RAW_VERIFIED_OVERLAYS: decompress the disc BIN (skipping
# the 0x01 format byte) with the reference decoder and compare it with the
# linked ELF's raw binary. These are not added to the complete manifest.
define raw-overlay-rules

.PHONY: verify-$(1)

$(call overlay-raw-rule,$(1))

$(BUILD_DIR)/overlays/$(1)/$(2).orig.raw: $(ROM_BIN_DIR)/$(2).BIN
	@mkdir -p $$(@D)
	python3 tools/splat_ext/decompress.py $$< 1 $$$$(($$$$(stat -c%s $$<) - 1)) $$@

verify-$(1): $(BUILD_DIR)/overlays/$(1)/$(1).raw $(BUILD_DIR)/overlays/$(1)/$(2).orig.raw
	@set -eu; \
		expected=$$$$(sha1sum $(BUILD_DIR)/overlays/$(1)/$(2).orig.raw | awk '{print $$$$1}'); \
		actual=$$$$(sha1sum $(BUILD_DIR)/overlays/$(1)/$(1).raw | awk '{print $$$$1}'); \
		echo "$(2).BIN raw expected: $$$$expected"; \
		echo "$(2).BIN raw actual:   $$$$actual"; \
		if [ "$$$$expected" = "$$$$actual" ]; then \
			echo "[OK] $(2).BIN raw image matches original ROM (compressed stream not yet reproducible)"; \
		else \
			echo "[FAIL] $(2).BIN raw image sha1 mismatch"; \
			exit 1; \
		fi

endef

$(foreach name,$(RAW_VERIFIED_OVERLAYS),\
	$(eval $(call raw-overlay-rules,$(name),$(call upper-case,$(name)))))

# ── Main executable ──────────────────────────────────────────────────────────
#
# The main executable (SLUS_010.13, SLPS_021.70) is not compressed: its linked
# ELF, converted to a raw binary (which includes the 0x800-byte PS-X EXE
# header), must equal the disc file. verify-slus is kept as an alias of
# verify-main for existing scripts and CI.
.PHONY: verify-main verify-slus

$(BUILD_DIR)/$(GAME).raw: all
	@mkdir -p $(@D)
	$(OBJCOPY) -O binary $(TARGET) $@.tmp
	mv $@.tmp $@

verify-slus: verify-main

verify-main: $(BUILD_DIR)/$(GAME).raw
	@mkdir -p $(BUILD_DIR)
	@set -eu; \
		expected=$$(sha1sum $(DISC_DIR)/$(GAME) | awk '{print $$1}'); \
		actual=$$(sha1sum $< | awk '{print $$1}'); \
		echo "$(GAME) expected: $$expected"; \
		echo "$(GAME) actual:   $$actual"; \
		if [ "$$expected" = "$$actual" ]; then \
			echo "[OK] $(GAME) matches original ROM"; \
			grep -qxF main $(COMPLETE_MANIFEST) 2>/dev/null || echo main >> $(COMPLETE_MANIFEST); \
		else \
			echo "[FAIL] $(GAME) sha1 mismatch"; \
			exit 1; \
		fi

# ── Aggregate ────────────────────────────────────────────────────────────────
#
# Register a new overlay by adding it to VERIFIED_OVERLAYS_<version> above.
verify-bins: verify-main $(foreach name,$(VERIFIED_OVERLAYS) $(RAW_VERIFIED_OVERLAYS),verify-$(name))
	@echo "Verified compressed overlays: $$(cat $(COMPLETE_MANIFEST) 2>/dev/null | tr '\n' ' ')"

# Check the compressor itself against all 17 original overlays, without needing
# a build. Run this after any change to tools/compressor/compressor.py.
.PHONY: verify-compressor
verify-compressor:
	python3 tools/compressor/verify_exact_bins.py --bin-dir $(ROM_BIN_DIR)
