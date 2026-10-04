# Object analysis and diffing

.PHONY: dump-objs target-objects base-objects objdiff-objects objdiff-config

# Write disassembly beside each built object, e.g. build/us/src/main/cdrom.s.
dump-objs:
	@set -eu; \
		if [ ! -d $(BUILD_DIR) ]; then \
			echo "$(BUILD_DIR)/ does not exist. Run a build first." >&2; \
			exit 1; \
		fi; \
		objects=$$(find $(BUILD_DIR) -type f -name '*.o'); \
		if [ -z "$$objects" ]; then \
			echo "No object files found under $(BUILD_DIR)/. Run a build first." >&2; \
			exit 1; \
		fi; \
		for object in $$objects; do \
			output="$${object%.o}.s"; \
			$(OBJDUMP) -d -r --no-show-raw-insn "$$object" > "$$output"; \
			echo "  $$output"; \
		done
	@echo "dump-objs complete."

# Objdiff compares original assembly (target objects) with rebuilt C (base
# objects). Keep those names here because they are also used by objdiff.

# Gather all .s files, then exclude non-matchings, data, overlays, and
# hand-written asm that already has its own build rule (ASM_SRCS).
ALL_ASM_SRCS    := $(call rwildcard,$(ASM_DIR),*.s)
TARGET_ASM_SRCS := $(filter-out $(ASM_DIR)/nonmatchings/% $(ASM_DIR)/data/% $(ASM_DIR)/overlays/% $(ASM_SRCS),$(ALL_ASM_SRCS))
TARGET_OBJS     := $(patsubst $(ASM_DIR)/%.s,$(STAGING)/$(BUILD_DIR)/$(ASM_DIR)/%.o,$(TARGET_ASM_SRCS))
OBJDIFF_BASE_OBJS := $(OBJS_G0) $(OBJS_G4) $(OBJS_GCC_260_G0)

$(TARGET_OBJS): $(STAGING)/$(BUILD_DIR)/$(ASM_DIR)/%.o: $(ASM_DIR)/%.s $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && cat $(ASM_DIR)/$*.s | \
		$(MASPSX) $(MASPSX_PP_FLAGS) | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS) -o $(BUILD_DIR)/$(ASM_DIR)/$*.o

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

# OBJS_ASM is included because a version without a split TU layout assembles
# its main code through that rule (the objects are its objdiff targets).
target-objects: $(COPY_SENTINEL) $(TARGET_OBJS) $(OBJS_ASM)
	$(call copy-staged-objects,$(TARGET_OBJS) $(OBJS_ASM))
	@echo "Target objects built."

base-objects: $(COPY_SENTINEL) $(OBJDIFF_BASE_OBJS)
	$(call copy-staged-objects,$(OBJDIFF_BASE_OBJS))
	@echo "Base objects built."

# Overlay comparisons use the paths and C objects from overlay-inputs.mk.
# $(1) is the overlay name; $$ keeps variables for eval's second pass.
define overlay-objdiff-rules

# Objdiff rules for this overlay
$(1)_ALL_ASM    := $$(call rwildcard,$$($(1)_ASM_DIR),*.s)
$(1)_TGT_ASM   := $$(filter-out $$($(1)_ASM_DIR)/nonmatchings/% $$($(1)_ASM_DIR)/data/%,$$($(1)_ALL_ASM))
$(1)_TGT_OBJS  := $$(patsubst $$($(1)_ASM_DIR)/%.s,$(STAGING)/$$($(1)_BUILD_DIR)/target/%.o,$$($(1)_TGT_ASM))

$$($(1)_TGT_OBJS): $(STAGING)/$$($(1)_BUILD_DIR)/target/%.o: $$($(1)_ASM_DIR)/%.s $(COPY_SENTINEL) | $(1)-validate
	@mkdir -p $$(@D)
	cd $(STAGING) && cat $$($(1)_ASM_DIR)/$$*.s | \
		$(MASPSX) $(MASPSX_PP_FLAGS) | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_FLAGS_272_CDK) $$(overlay_$(1)_target_as_extra_flags_$$*) -o $$($(1)_BUILD_DIR)/target/$$*.o

$(1)-target-objects: $(1)-validate $(COPY_SENTINEL) $$($(1)_TGT_OBJS)
	@mkdir -p $$($(1)_BUILD_DIR)/target
	@if [ -n "$$(firstword $$($(1)_TGT_OBJS))" ]; then \
		cp -a "$(STAGING)/$$($(1)_BUILD_DIR)/target/." "$$($(1)_BUILD_DIR)/target/"; \
	fi

$(1)-base-objects: $(1)-validate $(COPY_SENTINEL) $$($(1)_C_OBJS)
	@mkdir -p $$($(1)_C_OBJ_DIR)
	@if [ -n "$$(firstword $$($(1)_C_OBJS))" ]; then \
		cp -a "$(STAGING)/$$($(1)_C_OBJ_DIR)/." \
			"$$($(1)_C_OBJ_DIR)/"; \
	fi

$(1)-objdiff: $(1)-target-objects $(1)-base-objects

.PHONY: $(1)-target-objects $(1)-base-objects $(1)-objdiff

endef

$(foreach ov,$(OVERLAYS),$(eval $(call overlay-objdiff-rules,$(ov))))

OBJDIFF_CLI ?= objdiff-cli
OBJDIFF_CONFIG_GENERATOR ?= tools/objdiff/generate_objdiff_config.py
PROGRESS_REPORT ?= $(BUILD_DIR)/progress.json

objdiff-objects: target-objects base-objects $(addsuffix -objdiff,$(OVERLAYS))

# Needs the objects: units that still include assembly functions are not marked
# complete, which the generator reads from their compiled objects.
objdiff-config: objdiff-objects
	python3 $(OBJDIFF_CONFIG_GENERATOR) --version $(VERSION)

# Write the progress report used by CI.
.PHONY: progress
progress: objdiff-config
	$(OBJDIFF_CLI) report generate -o $(PROGRESS_REPORT)
	@echo "Wrote $(PROGRESS_REPORT)"

# Run objdiff diff on every unit in objdiff.json and write JSON results under
# build/<version>/diffs/, mirroring the unit name as a path (e.g. main/cdrom.json).
# objdiff-config builds the objects before generating the config.
.PHONY: diff-all diff-text
diff-all: objdiff-config
	python3 tools/objdiff/run_diffs.py --cli $(OBJDIFF_CLI) --output-dir $(BUILD_DIR)/diffs

# Convert every build/<version>/diffs/**/*.json into a compact side-by-side
# text file at build/<version>/diffs/**/*.txt -- only non-100% functions, only differing lines.
diff-text: diff-all
	@set -eu; \
		diff_files=$$(find $(BUILD_DIR)/diffs -type f -name '*.json'); \
		if [ -z "$$diff_files" ]; then \
			echo "No JSON diffs found under $(BUILD_DIR)/diffs/." >&2; \
			exit 1; \
		fi; \
		for file in $$diff_files; do \
			python3 tools/objdiff/format_diffs.py --all "$$file" -o "$${file%.json}.txt"; \
		done
	@echo "Text diffs written to $(BUILD_DIR)/diffs/**/*.txt"
