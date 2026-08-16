# ============================================================================
# Compressed overlay ROM verification
# ============================================================================
#
# To prove a compressed overlay is byte-perfect, reproduce the file stored in
# disc/BIN/ and compare its SHA1:
#
#   1. Link the overlay ELF.
#   2. Convert the ELF to its raw decompressed binary.
#   3. Compress the raw binary with tools/compressor/compressor.py.
#   4. Prepend the 0x01 compression-format byte skipped by the splat configs.
#   5. SHA1-compare the result with the original overlay BIN.
#
# On a match, the overlay name is added to build/complete_overlays.txt.
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
VERIFIED_OVERLAYS := gover movie gname checkps

# Make has no upper-case function; overlay BINs are named in upper case.
upper-case = $(shell echo '$(1)' | tr '[:lower:]' '[:upper:]')

.PHONY: verify-bins

# ── Per-overlay verification rules ───────────────────────────────────────────
#
#   $(1) = overlay name, lower case (e.g. "gover")
#   $(2) = overlay BIN basename, upper case (e.g. "GOVER")

define compressed-overlay-rules

.PHONY: verify-$(1)

build/overlays/$(1)/$(1).raw: $(1)
	@mkdir -p $$(@D)
	$(OBJCOPY) -O binary $(STAGING)/build/overlays/$(1)/$(1).elf $$@.tmp
	mv $$@.tmp $$@

build/overlays/$(1)/$(2).BIN: build/overlays/$(1)/$(1).raw
	python3 tools/compressor/compressor.py $$< $$@.payload
	{ printf '\001'; cat $$@.payload; } > $$@.tmp
	mv $$@.tmp $$@
	rm -f $$@.payload

verify-$(1): build/overlays/$(1)/$(2).BIN
	@mkdir -p build
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

# ── Aggregate ────────────────────────────────────────────────────────────────
#
# Register a new overlay by adding it to VERIFIED_OVERLAYS above.
verify-bins: $(foreach name,$(VERIFIED_OVERLAYS),verify-$(name))
	@echo "Verified compressed overlays: $$(cat $(COMPLETE_MANIFEST) 2>/dev/null | tr '\n' ' ')"

# Check the compressor itself against all 17 original overlays, without needing
# a build. Run this after any change to tools/compressor/compressor.py.
.PHONY: verify-compressor
verify-compressor:
	python3 tools/compressor/verify_exact_bins.py
