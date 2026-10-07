# Set the default once; recursive makes share the parent's jobserver.
ifeq ($(MAKELEVEL),0)
# GNU Make 4.3 omits job flags from MAKEFLAGS while reading makefiles. Check
# the incoming environment too; command-line -j takes precedence over this
# default when Make finishes reading the makefiles.
ifeq ($(filter -j% --jobs%,$(MAKEFLAGS) $(shell printf '%s' "$$MAKEFLAGS $$GNUMAKEFLAGS")),)
MAKEFLAGS += -j$(shell nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)
endif
endif
