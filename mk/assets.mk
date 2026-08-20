# ============================================================================
#  Format-aware asset validation
# ============================================================================

PSX_TIM_ASSETS := $(call rwildcard,assets,*.tim)
SPRITE_LAYOUT_SOURCES := $(call rwildcard,assets,*.sprite_layout.yaml)
SPRITE_LAYOUT_BINARIES := $(patsubst %.sprite_layout.yaml,%.sprite_layout.bin,$(SPRITE_LAYOUT_SOURCES))
SPRITE_ANIMATION_SOURCES := $(call rwildcard,assets,*.sprite_animation.yaml)
SPRITE_ANIMATION_BINARIES := $(patsubst %.sprite_animation.yaml,%.sprite_animation.bin,$(SPRITE_ANIMATION_SOURCES))

.PHONY: validate-assets validate-psx-tim-assets validate-sprite-layout-assets validate-sprite-animation-assets

validate-assets: validate-psx-tim-assets validate-sprite-layout-assets validate-sprite-animation-assets

validate-psx-tim-assets:
ifneq ($(strip $(PSX_TIM_ASSETS)),)
	python3 tools/assets/psx_tim.py roundtrip $(PSX_TIM_ASSETS)
else
	@:
endif

%.sprite_layout.bin: %.sprite_layout.yaml tools/assets/sprite_layout.py
	python3 tools/assets/sprite_layout.py build $< $@

validate-sprite-layout-assets: $(SPRITE_LAYOUT_BINARIES)
ifneq ($(strip $(SPRITE_LAYOUT_SOURCES)),)
	python3 tools/assets/sprite_layout.py validate $(SPRITE_LAYOUT_SOURCES)
else
	@:
endif

%.sprite_animation.bin: %.sprite_animation.yaml tools/assets/sprite_animation.py
	python3 tools/assets/sprite_animation.py build $< $@

validate-sprite-animation-assets: $(SPRITE_ANIMATION_BINARIES)
ifneq ($(strip $(SPRITE_ANIMATION_SOURCES)),)
	python3 tools/assets/sprite_animation.py validate $(SPRITE_ANIMATION_SOURCES)
else
	@:
endif

# Structured assets must be rebuilt before staging copies linker inputs to the
# native Linux filesystem used by the legacy toolchain.
$(COPY_SENTINEL): $(SPRITE_LAYOUT_BINARIES) $(SPRITE_ANIMATION_BINARIES)
