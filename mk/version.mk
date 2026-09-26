# ============================================================================
# Game version selection
# ============================================================================
#
# One C source tree builds every regional release. Everything derived from a
# particular disc lives in a per-version directory:
#
#   disc/<version>/      original executable and BIN/ overlays (gitignored)
#   config/<version>/    splat configs, symbol maps, relocation overrides
#   asm/<version>/       splat-generated assembly (gitignored)
#   linker/<version>/    splat-generated linker scripts (gitignored)
#   assets/<version>/    splat-extracted binary data (gitignored)
#   build/<version>/     objects, ELFs, maps, reports
#
# Select a version with VERSION=<name> on the command line, e.g.
#
#   make VERSION=jp
#
# C code tests the version with the VERSION_<NAME> macros defined below:
#
#   #if defined(VERSION_JP)
#   ...
#   #endif
#
# Supported versions:
#   us - North America, SLUS-01013 (default)
#   jp - Japan, SLPS-02170 (in progress: no splat configs yet)

SUPPORTED_VERSIONS := us jp

VERSION ?= us

ifeq ($(filter $(VERSION),$(SUPPORTED_VERSIONS)),)
$(error Unknown VERSION '$(VERSION)'. Supported versions: $(SUPPORTED_VERSIONS))
endif

# Main executable file name on each disc.
GAME_us := SLUS_010.13
GAME_jp := SLPS_021.70

GAME := $(GAME_$(VERSION))

# Upper-case version name for the VERSION_<NAME> preprocessor macro.
VERSION_UPPER := $(shell echo '$(VERSION)' | tr '[:lower:]' '[:upper:]')

# Per-version directories (relative; used for both the mount and /staging).
DISC_DIR     := disc/$(VERSION)
CONFIG_DIR   := config/$(VERSION)
ASM_DIR      := asm/$(VERSION)
LINKER_DIR   := linker/$(VERSION)
ASSETS_DIR   := assets/$(VERSION)
BUILD_DIR    := build/$(VERSION)

# Original overlay BINs for this version.
ROM_BIN_DIR  := $(DISC_DIR)/BIN

# Passed to every C compile. include/version.h derives ASM_VERSION_DIR, the
# prefix INCLUDE_ASM and INCLUDE_RODATA prepend to their folder argument.
VERSION_CPP_FLAGS := -DVERSION_$(VERSION_UPPER)
