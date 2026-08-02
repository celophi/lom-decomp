# ============================================================================
# Object analysis, diffing, and ROM verification
# ============================================================================

.PHONY: dump-objs target-objects base-objects objdiff-objects objdiff-config
.PHONY: verify-bins verify-gover verify-movie

# Disassemble every compiled .o in build/ and write a matching .s alongside it.
# Run this after a build to get compiler output for the find_idioms.py tool.
#   make dump-objs
# The .s files end up at e.g. build/src/cdrom.s, build/overlays/gname/gname.s etc.
dump-objs:
	@find build -name '*.o' | while read f; do \
		out="$${f%.o}.s"; \
		$(OBJDUMP) -d -r --no-show-raw-insn "$$f" > "$$out" 2>/dev/null && echo "  $$out" || true; \
	done
	@echo "dump-objs complete."

# ============================================================================
#  Objdiff — Main SLUS  (progress tracking / function matching)
# ============================================================================
#
# "Target objects" = assembled from splat's original .s disassembly (the goal).
# "Base objects"   = compiled from your decompiled .c source (your progress).
# objdiff compares them function-by-function to show what matches.

# Gather all .s files, then exclude non-matchings, data, overlays, and
# hand-written asm that already has its own build rule (ASM_SRCS).
ALL_ASM    := $(call rwildcard,$(ASM_DIR),*.s)
TARGET_ASM := $(filter-out $(ASM_DIR)/nonmatchings/% $(ASM_DIR)/data/% $(ASM_DIR)/overlays/% $(ASM_SRCS),$(ALL_ASM))
TARGET_OBJ := $(patsubst $(ASM_DIR)/%.s,$(STAGING)/build/$(ASM_DIR)/%.o,$(TARGET_ASM))

$(TARGET_OBJ): $(STAGING)/build/$(ASM_DIR)/%.o: $(ASM_DIR)/%.s $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && cat $(ASM_DIR)/$*.s | \
		$(MASPSX) $(MASPSX_PP_FLAGS) | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS) -o build/$(ASM_DIR)/$*.o

target-objects: $(COPY_SENTINEL) $(TARGET_OBJ)
	@mkdir -p build/asm
	@cp -r $(STAGING)/build/$(ASM_DIR)/* build/asm/ 2>/dev/null || true
	@echo "Target objects built."

base-objects: $(COPY_SENTINEL) $(OBJS_G0) $(OBJS_G4) $(OBJS_CDK_G0) $(OBJS_GCC_260_G0)
	@mkdir -p build/src
	@cp -r $(STAGING)/build/$(SRC_DIR)/* build/src/ 2>/dev/null || true
	@echo "Base objects built."

OBJDIFF_CLI := tools/objdiff/objdiff-cli-linux-x86_64

objdiff-objects: target-objects base-objects $(addsuffix -objdiff,$(OVERLAYS))

objdiff-config:
	python3 tools/objdiff/generate_objdiff_config.py

# Run objdiff diff on every unit in objdiff.json and write JSON results under
# build/diffs/, mirroring the unit name as a path (e.g. main/cdrom.json).
# Depends on objdiff-objects and objdiff-config so .o files and config are
# up to date before diffing.
.PHONY: diff-all diff-text
diff-all: objdiff-objects objdiff-config
	python3 tools/objdiff/run_diffs.py --cli $(OBJDIFF_CLI)

# Convert every build/diffs/**/*.json into a compact side-by-side text file
# at build/diffs/**/*.txt -- only non-100% functions, only differing lines.
diff-text: diff-all
	@find build/diffs -name '*.json' | while read f; do \
		python3 tools/objdiff/format_diffs.py --all "$$f" -o "$${f%.json}.txt"; \
	done
	@echo "Text diffs written to build/diffs/**/*.txt"

# ============================================================================
#  ROM Verification — compressed overlay matching
# ============================================================================
#
# Some overlays are stored compressed in the original ROM (e.g. GOVER.BIN).
# To prove a 100% byte-perfect match we must reproduce the exact compressed
# file the disc contains:
#
#   1. Link the overlay ELF                       (handled by the overlay rule)
#   2. objcopy ELF -> raw decompressed binary
#   3. Compress with tools/compressor/compressor.py
#   4. Prepend the 1-byte algorithm-selector (0x01) the loader expects
#   5. SHA1-compare against the original BIN in disc/BIN/
#
# On match, the overlay's name is appended to build/complete_overlays.txt.
# generate_objdiff_config.py reads that manifest and stamps metadata.complete
# on every unit of a matched overlay.

# --- gover -------------------------------------------------------------------
# The original decompressed GOVER starts with a 0x00 byte before the actual
# overlay code (objcopy strips it because it's not in any output section), so
# we prepend it to the raw .bin before compressing.
build/overlays/gover/gover.raw: gover
	@mkdir -p build/overlays/gover
	$(OBJCOPY) -O binary $(STAGING)/build/overlays/gover/gover.elf $@.tmp
	printf '\0' > $@
	cat $@.tmp >> $@
	rm -f $@.tmp

build/overlays/gover/GOVER.BIN: build/overlays/gover/gover.raw
	python3 tools/compressor/compressor.py $< $@

verify-gover: build/overlays/gover/GOVER.BIN
	@mkdir -p build
	@expected=$$(sha1sum $(ROM_BIN_DIR)/GOVER.BIN | awk '{print $$1}'); \
	 actual=$$(sha1sum build/overlays/gover/GOVER.BIN  | awk '{print $$1}'); \
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
# Like GOVER, the original decompressed MOVIE starts with a 0x00 byte that
# objcopy strips, so we prepend it to the raw .bin before compressing.
build/overlays/movie/movie.raw: movie
	@mkdir -p build/overlays/movie
	$(OBJCOPY) -O binary $(STAGING)/build/overlays/movie/movie.elf $@.tmp
	printf '\0' > $@
	cat $@.tmp >> $@
	rm -f $@.tmp

build/overlays/movie/MOVIE.BIN: build/overlays/movie/movie.raw
	python3 tools/compressor/compressor.py $< $@

verify-movie: build/overlays/movie/MOVIE.BIN
	@mkdir -p build
	@expected=$$(sha1sum $(ROM_BIN_DIR)/MOVIE.BIN | awk '{print $$1}'); \
	 actual=$$(sha1sum build/overlays/movie/MOVIE.BIN  | awk '{print $$1}'); \
	 echo "MOVIE.BIN expected: $$expected"; \
	 echo "MOVIE.BIN actual:   $$actual"; \
	 if [ "$$expected" = "$$actual" ]; then \
	   echo "[OK] MOVIE.BIN matches original ROM"; \
	   grep -qxF movie $(COMPLETE_MANIFEST) 2>/dev/null || echo movie >> $(COMPLETE_MANIFEST); \
	 else \
	   echo "[FAIL] MOVIE.BIN sha1 mismatch"; \
	   exit 1; \
	 fi

# Aggregate target — extend as more overlays reach 100%.
verify-bins: verify-gover verify-movie
	@echo "Verified compressed overlays: $$(cat $(COMPLETE_MANIFEST) 2>/dev/null | tr '\n' ' ')"
