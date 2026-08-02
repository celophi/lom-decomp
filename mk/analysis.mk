# ============================================================================
# Object analysis and diffing
# ============================================================================

.PHONY: dump-objs target-objects base-objects objdiff-objects objdiff-config

# Disassemble every compiled .o in build/ and write a matching .s alongside it.
# Run this after a build to get compiler output for the find_idioms.py tool.
#   make dump-objs
# The .s files end up at e.g. build/src/cdrom.s, build/overlays/gname/gname.s etc.
dump-objs:
	@set -eu; \
		if [ ! -d build ]; then \
			echo "build/ does not exist. Run a build first." >&2; \
			exit 1; \
		fi; \
		objects=$$(find build -type f -name '*.o'); \
		if [ -z "$$objects" ]; then \
			echo "No object files found under build/. Run a build first." >&2; \
			exit 1; \
		fi; \
		for object in $$objects; do \
			output="$${object%.o}.s"; \
			$(OBJDUMP) -d -r --no-show-raw-insn "$$object" > "$$output"; \
			echo "  $$output"; \
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
ALL_ASM_SRCS    := $(call rwildcard,$(ASM_DIR),*.s)
TARGET_ASM_SRCS := $(filter-out $(ASM_DIR)/nonmatchings/% $(ASM_DIR)/data/% $(ASM_DIR)/overlays/% $(ASM_SRCS),$(ALL_ASM_SRCS))
TARGET_OBJS     := $(patsubst $(ASM_DIR)/%.s,$(STAGING)/build/$(ASM_DIR)/%.o,$(TARGET_ASM_SRCS))
OBJDIFF_BASE_OBJS := $(OBJS_G0) $(OBJS_G4) $(OBJS_CDK_G0) $(OBJS_GCC_260_G0)

$(TARGET_OBJS): $(STAGING)/build/$(ASM_DIR)/%.o: $(ASM_DIR)/%.s $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && cat $(ASM_DIR)/$*.s | \
		$(MASPSX) $(MASPSX_PP_FLAGS) | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS) -o build/$(ASM_DIR)/$*.o

# Copy only declared objdiff inputs out of staging. This avoids importing stale
# objects and makes a missing compiler or assembler output fail the target.
define copy-staged-objects
	@set -eu; \
		for source in $(1); do \
			destination=$${source#$(STAGING)/}; \
			mkdir -p "$$(dirname "$$destination")"; \
			cp -a "$$source" "$$destination"; \
		done
endef

target-objects: $(COPY_SENTINEL) $(TARGET_OBJS)
	$(call copy-staged-objects,$(TARGET_OBJS))
	@echo "Target objects built."

base-objects: $(COPY_SENTINEL) $(OBJDIFF_BASE_OBJS)
	$(call copy-staged-objects,$(OBJDIFF_BASE_OBJS))
	@echo "Base objects built."

OBJDIFF_CLI ?= tools/objdiff/objdiff-cli-linux-x86_64
OBJDIFF_CONFIG_GENERATOR ?= tools/objdiff/generate_objdiff_config.py

objdiff-objects: target-objects base-objects $(addsuffix -objdiff,$(OVERLAYS))

objdiff-config:
	python3 $(OBJDIFF_CONFIG_GENERATOR)

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
	@set -eu; \
		diff_files=$$(find build/diffs -type f -name '*.json'); \
		if [ -z "$$diff_files" ]; then \
			echo "No JSON diffs found under build/diffs/." >&2; \
			exit 1; \
		fi; \
		for file in $$diff_files; do \
			python3 tools/objdiff/format_diffs.py --all "$$file" -o "$${file%.json}.txt"; \
		done
	@echo "Text diffs written to build/diffs/**/*.txt"
