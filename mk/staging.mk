# Linux staging
#
# The old 32-bit tools can fail with EOVERFLOW on Docker Desktop bind mounts.
# Copying to native Linux storage gives them inode values they can read. We
# also convert staged text to LF because checked-out files may contain CRLF.
#
# Each version gets a marker because its asm, linker and assets trees differ.
# The unversioned marker is still used by tools that check whether staging ran.
# `make recopy` forces a refresh; normal edits are tracked by STAGE_INPUTS.
COPY_SENTINEL := $(STAGING)/.sources_copied.$(VERSION)
LEGACY_COPY_SENTINEL := $(STAGING)/.sources_copied

# Project inputs that must exist before staging (splat generates the
# version's asm/ and linker/ trees; the rest are checked in).
STAGE_PATHS_REQUIRED := \
	src \
	$(ASM_DIR) \
	include \
	$(LINKER_DIR) \
	tools/maspsx

# Some overlays have no extracted assets. Don't require a missing asset tree.
STAGE_PATHS_OPTIONAL := $(ASSETS_DIR)

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
STAGE_INPUTS := Makefile mk/staging.mk mk/version.mk $(STAGE_PATHS) \
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
		rm -f "$(COPY_SENTINEL)"; \
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
	@touch $@ $(LEGACY_COPY_SENTINEL)
	@echo "Staging complete ($(VERSION))."
