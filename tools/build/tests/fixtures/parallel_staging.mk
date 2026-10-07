.DEFAULT_GOAL := check_staged_inputs

VERSION := us
STAGING := staging
ASM_DIR := asm/us
LINKER_DIR := linker/us
ASSETS_DIR := assets/us

include mk/staging.mk

# Request the copy and its outputs together, as the real link rules do.
.PHONY: check_staged_inputs
check_staged_inputs: $(COPY_SENTINEL) staging/linker/us/game.ld staging/assets/us/image.bin
	cmp linker/us/game.ld staging/linker/us/game.ld
	cmp assets/us/image.bin staging/assets/us/image.bin
