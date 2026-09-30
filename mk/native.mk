# ============================================================================
#  Native compile check
# ============================================================================
# Compiles every C file this version builds with the host's C compiler, to
# keep a 64-bit build possible (see tools/native_check/native_check.py). The
# PS1 build is not involved. The Psy-Q SDK sources are left out: a port
# replaces the SDK.
#
#   make native-check                         check against the baseline
#   make native-check NATIVE_CC=clang         with another compiler
#   make native-check NATIVE_CHECK_FLAGS=--update-baseline

NATIVE_CC ?= cc
NATIVE_CHECK_FLAGS ?=
NATIVE_SRCS = $(filter-out src/psyq/%,$(SRCS_G0) $(SRCS_G4) $(SRCS_GCC_260_G0)) \
	$(foreach name,$(OVERLAYS),$($(name)_C_SRCS))

.PHONY: native-check
native-check:
	python3 tools/native_check/native_check.py --version $(VERSION) --cc '$(NATIVE_CC)' \
		$(NATIVE_CHECK_FLAGS) $(NATIVE_SRCS)

# ============================================================================
#  Storage layout check
# ============================================================================
# Checks that every type the game keeps in PS1 memory has the same layout on a
# 64-bit host that keeps PS1 storage (see tools/storage_check/storage_check.py
# and include/ps1_storage.h). Needs only the libclang Python package.
#
#   make storage-check                         check against the baseline
#   make storage-check STORAGE_CHECK_FLAGS=--verbose

STORAGE_CHECK_FLAGS ?=

.PHONY: storage-check
storage-check:
	python3 tools/storage_check/storage_check.py --version $(VERSION) $(STORAGE_CHECK_FLAGS) $(NATIVE_SRCS)
