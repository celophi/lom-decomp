# Legend of Mana PSX build
#
# Run `make help` for targets. VERSION=us is the default; VERSION=jp builds Japan.
# The compiler writes assembly, then maspsx translates it and runs GNU as.
# mk/toolchains.mk keeps the compiler versions and flags needed for matching.
#
# We build in /staging because the old 32-bit tools cannot read some Docker
# bind-mount inode values. mk/staging.mk copies the inputs and fixes CRLF there.

STAGING      := /staging
MOUNT        := /lom

# Select the release before defining any build paths.
include mk/version.mk

TARGET       := $(STAGING)/$(BUILD_DIR)/$(GAME).elf
BIN          := $(BUILD_DIR)/$(GAME).bin

# Paths below are relative to the project and its staging copy.
SRC_DIR      := src

.DEFAULT_GOAL := all

# Records the main executable and overlays that match their disc files.
# The objdiff config generator uses this to mark units complete.
COMPLETE_MANIFEST := $(BUILD_DIR)/complete_overlays.txt

# Recursive wildcard helper used while overlay rules are expanded.
rwildcard = $(foreach d,$(wildcard $1/*),$(call rwildcard,$d,$2)) \
            $(filter $(subst *,%,$2),$1)

include mk/staging.mk
include mk/splat.mk
include mk/assets.mk
include mk/tools.mk
include mk/toolchains.mk
include mk/main.mk
# Load overlay assignments and inputs before expanding build and objdiff rules.
include mk/overlay-registry.mk
include mk/overlay-inputs.mk
include mk/overlays.mk
include mk/analysis.mk
include mk/verification.mk
include mk/native.mk
include mk/help.mk

.PHONY: clean

# This removes this version's host build and the entire shared staging tree.
clean:
	rm -rf $(BUILD_DIR)/ $(STAGING)
