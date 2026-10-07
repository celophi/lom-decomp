# Report the flags used by a parent Make and its recursive child.
include parallel.mk

.PHONY: report_flags child

report_flags:
	@printf 'parent=%s\n' '$(MAKEFLAGS)'
	+@$(MAKE) --no-print-directory child

child:
	@printf 'child=%s\n' '$(MAKEFLAGS)'
