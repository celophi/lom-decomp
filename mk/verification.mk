# Compare rebuilt files with the originals on disc.
#
# An objdiff score of 100% can hide section shifts because it normalizes
# relocations. Add an overlay here only after `make verify-<name>` passes.
# We convert its ELF to a raw binary, compress it, and restore the 0x01 format
# byte before comparing SHA1s. Successful checks go in COMPLETE_MANIFEST for
# the objdiff config generator.

VERIFIED_OVERLAYS_us := gover movie gname checkps title gosub golem niki addhero menu cload zukan carda shop wsel field wmap
VERIFIED_OVERLAYS_jp := gover movie gname checkps title gosub golem niki addhero menu cload zukan carda shop wsel field wmap
VERIFIED_OVERLAYS := $(VERIFIED_OVERLAYS_$(VERSION))

# Use these lists when the raw image matches but the compressor cannot yet
# reproduce the disc stream. Raw checks do not mark an overlay complete.
RAW_VERIFIED_OVERLAYS_us :=
RAW_VERIFIED_OVERLAYS_jp :=
RAW_VERIFIED_OVERLAYS := $(RAW_VERIFIED_OVERLAYS_$(VERSION))

# Make has no upper-case function; overlay BINs are named in upper case.
upper-case = $(shell echo '$(1)' | tr '[:lower:]' '[:upper:]')

.PHONY: verify-bins

# Per-overlay verification rules
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
	python3 tools/compression/compressor.py $$< $$@.payload
	{ printf '\001'; cat $$@.payload; } > $$@.tmp
	mv $$@.tmp $$@
	rm -f $$@.payload

verify-$(1): $(BUILD_DIR)/overlays/$(1)/$(2).BIN
	python3 tools/verification/verify_file.py $(ROM_BIN_DIR)/$(2).BIN $$< \
		--manifest $(COMPLETE_MANIFEST) --name $(1)

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
	python3 tools/compression/decompress.py $$< 1 $$$$(($$$$(stat -c%s $$<) - 1)) $$@

verify-$(1): $(BUILD_DIR)/overlays/$(1)/$(1).raw $(BUILD_DIR)/overlays/$(1)/$(2).orig.raw
	python3 tools/verification/verify_file.py $(BUILD_DIR)/overlays/$(1)/$(2).orig.raw \
		$(BUILD_DIR)/overlays/$(1)/$(1).raw --raw

endef

$(foreach name,$(RAW_VERIFIED_OVERLAYS),\
	$(eval $(call raw-overlay-rules,$(name),$(call upper-case,$(name)))))

# Main executable
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
	python3 tools/verification/verify_file.py $(DISC_DIR)/$(GAME) $< \
		--manifest $(COMPLETE_MANIFEST) --name main

# Aggregate
#
# Register a new overlay by adding it to VERIFIED_OVERLAYS_<version> above.
verify-bins: verify-main $(foreach name,$(VERIFIED_OVERLAYS) $(RAW_VERIFIED_OVERLAYS),verify-$(name))
	@echo "Verified disc files: $$(cat $(COMPLETE_MANIFEST) 2>/dev/null | tr '\n' ' ')"

# The normal and DATA_AS_C builds share object paths. Remove data objects
# before and after the check, including when it fails, so neither build can
# reuse the other's output. Generated C contains game data and stays in staging.
.PHONY: clean-data-as-c
clean-data-as-c:
	@set -eu; \
		for root in $(STAGING)/$(BUILD_DIR) $(BUILD_DIR); do \
			if [ -d "$$root" ]; then \
				find "$$root" -path '*/data/*.o' -delete; \
				find "$$root" -type d -name datac -prune -exec rm -rf {} +; \
			fi; \
		done
	rm -rf $(STAGING)/datac/$(BUILD_DIR)

# Check the host build of the data (data2c --host): every .data region's
# generated C, compiled for x86-64 and read back through the decomp's own
# types, holds the PS1 values field by field. The overlays are built first so
# their data objects exist; the generated C (game data) is removed afterwards.
.PHONY: verify-data-host
verify-data-host: all $(OVERLAYS)
	@set -eu; \
		cleanup() { \
			if [ -d "$(BUILD_DIR)/data-host" ]; then \
				find "$(BUILD_DIR)/data-host" \( -name '*.c' -o -name '*.o' \) -delete; \
			fi; \
		}; \
		trap cleanup EXIT; \
		python3 tools/verification/data2c/report_all.py --version $(VERSION) --host --verify --strict --out $(BUILD_DIR)/data-host

.PHONY: verify-data-as-c
verify-data-as-c:
	$(MAKE) clean-data-as-c
	$(MAKE) DATA_AS_C=1 verify-bins || { $(MAKE) clean-data-as-c; exit 1; }
	$(MAKE) clean-data-as-c

# Check the compressor itself against all 17 original overlays, without needing
# a build. Run this after any change to tools/compression/compressor.py.
.PHONY: verify-compressor
verify-compressor:
	python3 tools/compression/verify_exact_bins.py --bin-dir $(ROM_BIN_DIR)
