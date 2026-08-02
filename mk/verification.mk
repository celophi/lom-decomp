# ============================================================================
# Compressed overlay ROM verification
# ============================================================================
#
# To prove a compressed overlay is byte-perfect, reproduce the file stored in
# disc/BIN/ and compare its SHA1:
#
#   1. Link the overlay ELF.
#   2. Convert the ELF to its raw decompressed binary.
#   3. Compress the raw binary.
#   4. Prepend the 0x01 compression-format byte skipped by the splat configs.
#   5. SHA1-compare the result with the original overlay BIN.
#
# On a match, the overlay name is added to build/complete_overlays.txt.
# generate_objdiff_config.py uses that manifest to mark its objdiff units as
# complete.

.PHONY: verify-bins verify-gover verify-movie

# --- gover -------------------------------------------------------------------

build/overlays/gover/gover.raw: gover
	@mkdir -p $(@D)
	$(OBJCOPY) -O binary $(STAGING)/build/overlays/gover/gover.elf $@.tmp
	mv $@.tmp $@

build/overlays/gover/GOVER.BIN: build/overlays/gover/gover.raw
	python3 tools/compressor/compressor.py $< $@.payload
	{ printf '\001'; cat $@.payload; } > $@.tmp
	mv $@.tmp $@
	rm -f $@.payload

verify-gover: build/overlays/gover/GOVER.BIN
	@mkdir -p build
	@set -eu; \
		expected=$$(sha1sum $(ROM_BIN_DIR)/GOVER.BIN | awk '{print $$1}'); \
		actual=$$(sha1sum $< | awk '{print $$1}'); \
		echo "GOVER.BIN expected: $$expected"; \
		echo "GOVER.BIN actual:   $$actual"; \
		if [ "$$expected" = "$$actual" ]; then \
			echo "[OK] GOVER.BIN matches original ROM"; \
			grep -qxF gover $(COMPLETE_MANIFEST) 2>/dev/null || echo gover >> $(COMPLETE_MANIFEST); \
		else \
			echo "[FAIL] GOVER.BIN sha1 mismatch"; \
			exit 1; \
		fi

# --- movie -------------------------------------------------------------------

build/overlays/movie/movie.raw: movie
	@mkdir -p $(@D)
	$(OBJCOPY) -O binary $(STAGING)/build/overlays/movie/movie.elf $@.tmp
	mv $@.tmp $@

build/overlays/movie/MOVIE.BIN: build/overlays/movie/movie.raw
	python3 tools/compressor/compressor.py $< $@.payload
	{ printf '\001'; cat $@.payload; } > $@.tmp
	mv $@.tmp $@
	rm -f $@.payload

verify-movie: build/overlays/movie/MOVIE.BIN
	@mkdir -p build
	@set -eu; \
		expected=$$(sha1sum $(ROM_BIN_DIR)/MOVIE.BIN | awk '{print $$1}'); \
		actual=$$(sha1sum $< | awk '{print $$1}'); \
		echo "MOVIE.BIN expected: $$expected"; \
		echo "MOVIE.BIN actual:   $$actual"; \
		if [ "$$expected" = "$$actual" ]; then \
			echo "[OK] MOVIE.BIN matches original ROM"; \
			grep -qxF movie $(COMPLETE_MANIFEST) 2>/dev/null || echo movie >> $(COMPLETE_MANIFEST); \
		else \
			echo "[FAIL] MOVIE.BIN sha1 mismatch"; \
			exit 1; \
		fi

# Extend this aggregate when another compressed overlay becomes byte-perfect.
verify-bins: verify-gover verify-movie
	@echo "Verified compressed overlays: $$(cat $(COMPLETE_MANIFEST) 2>/dev/null | tr '\n' ' ')"
