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
#   jp - Japan, SLPS-02170 (in progress: see docs/jp-version-port.md)

SUPPORTED_VERSIONS := us jp

VERSION ?= us

ifeq ($(filter $(VERSION),$(SUPPORTED_VERSIONS)),)
$(error Unknown VERSION '$(VERSION)'. Supported versions: $(SUPPORTED_VERSIONS))
endif

# Modules whose code is split into C translation units, per version: `main`
# for the main executable, overlay names for overlays, or `all`. The source
# lists in mk/main.mk and the routing in mk/overlay-registry.mk describe this
# layout, and a module listed here must use it in its config/<version>/ yaml
# (tools/versions/port_us_layout.py ports the US yamls). A module not listed
# builds no C objects: it links from splat assembly alone and its
# objdiff/progress units are that assembly (0% matched).
TU_LAYOUT_us := all
TU_LAYOUT_jp := golem gover zukan menu shop

# $(call has-tu-layout,<module>) is non-empty when <module> uses the C layout.
has-tu-layout = $(or $(filter all,$(TU_LAYOUT_$(VERSION))),$(filter $(1),$(TU_LAYOUT_$(VERSION))))

# C files that this version builds from splat assembly instead, because their
# code or data differs from the US release (one path per line; `#` comments).
# mk/main.mk and mk/overlay-registry.mk leave them out of the C build.
ASM_UNITS_FILE := config/$(VERSION)/asm_units.txt
ASM_UNITS := $(if $(wildcard $(ASM_UNITS_FILE)),$(filter-out #%,$(shell grep -v '^\s*\#' $(ASM_UNITS_FILE))))

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
