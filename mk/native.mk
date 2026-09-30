# Native compile check
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
