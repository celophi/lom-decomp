# ============================================================================
# Linux staging
# ============================================================================
#
# Staging solves two independent compatibility problems:
#   1. The legacy 32-bit PSX compiler/preprocessor cannot stat Docker Desktop's
#      Windows bind-mount inodes and fails with EOVERFLOW ("Value too large for
#      defined data type"). Mirroring inputs to native Linux storage gives the
#      toolchain compatible filesystem metadata.
#   2. Checked-out or generated text inputs may use CRLF line endings. dos2unix
#      normalizes the staged copies to LF without modifying the host files.

# A sentinel tracks the last successful staging operation. Its prerequisites
# include every staged input, so host edits/additions/deletions automatically
# refresh /staging. Run `make recopy` to force a refresh.
COPY_SENTINEL := $(STAGING)/.sources_copied

# Project inputs that must exist before staging (splat generates asm/ and
# linker/; the rest are checked in).
STAGE_PATHS_REQUIRED := \
	src \
	asm \
	include \
	linker \
	tools/maspsx

# Optional inputs are staged only when present. `assets/` holds gitignored
# binary data (splat databin / asset_src) and is absent when no overlay embeds
# such data. Guarding with $(wildcard) keeps it out of the prerequisite list, so
# a missing assets/ does not abort staging with "No rule to make target 'assets'".
STAGE_PATHS_OPTIONAL := assets

# Project inputs needed by the Make build (required plus any present optional).
STAGE_PATHS := $(STAGE_PATHS_REQUIRED) $(wildcard $(STAGE_PATHS_OPTIONAL))

# These paths are wholly managed by the Makefile. Replacing them removes files
# deleted on the host while preserving /staging/build, /staging/mcp-work, and
# any tool-specific staging owned by developer tooling.
STAGE_MANAGED_PATHS := $(STAGE_PATHS)

# Text inputs that must use Linux line endings in the staged tree.
STAGE_TEXT_FIND_EXPR := \
	-name '*.c' -o \
	-name '*.h' -o \
	-name '*.s' -o \
	-name '*.inc' -o \
	-name '*.ld' -o \
	-name '*.txt' -o \
	-name '*.sh'

# A single find traversal is substantially faster on the Windows bind mount
# than recursively expanding one Make wildcard per directory. Directories are
# included so adding or deleting a staged file invalidates the sentinel.
# Changes to this file also invalidate the sentinel.
STAGE_INPUTS := Makefile mk/staging.mk $(STAGE_PATHS) \
	$(shell find $(STAGE_PATHS) -print 2>/dev/null)

.PHONY: recopy

recopy:
	rm -f $(COPY_SENTINEL)
	$(MAKE) $(COPY_SENTINEL)

# Replace the managed paths on native Linux storage, then normalize text inputs
# consumed by the compiler, assembler, linker, and shell to LF.
# The sentinel is only written after every copy and conversion succeeds.
$(COPY_SENTINEL): $(STAGE_INPUTS)
	@echo "Staging source files to $(STAGING)..."
	@set -eu; \
		mkdir -p "$(STAGING)"; \
		staging_abs=$$(readlink -f "$(STAGING)"); \
		if [ -z "$$staging_abs" ] || [ "$$staging_abs" = "/" ]; then \
			echo "Refusing unsafe staging path: '$(STAGING)'" >&2; \
			exit 1; \
		fi; \
		rm -f "$$staging_abs/.sources_copied"; \
		for path in $(STAGE_PATHS_REQUIRED); do \
			if [ ! -e "$$path" ]; then \
				echo "Missing required staging input: $$path" >&2; \
				exit 1; \
			fi; \
		done; \
		for path in $(STAGE_MANAGED_PATHS); do \
			rm -rf "$$staging_abs/$$path"; \
		done; \
		for path in $(STAGE_PATHS); do \
			mkdir -p "$$staging_abs/$$(dirname "$$path")"; \
			cp -a "$$path" "$$staging_abs/$$path"; \
		done
	@find $(addprefix $(STAGING)/,$(STAGE_MANAGED_PATHS)) -type f \
		\( $(STAGE_TEXT_FIND_EXPR) \) \
		-exec dos2unix -q {} +
	@touch $@
	@echo "Staging complete."
