# ============================================================================
#  Format-aware asset validation
# ============================================================================

PSX_TIM_ASSETS := $(call rwildcard,assets,*.tim)
PSX_TIM_DUPLICATE_WORD_BINARIES := $(call rwildcard,assets,*.tim_trail.bin)
SPRITE_LAYOUT_SOURCES := $(call rwildcard,assets,*.sprite_layout.yaml)
SPRITE_LAYOUT_BINARIES := $(patsubst %.sprite_layout.yaml,%.sprite_layout.bin,$(SPRITE_LAYOUT_SOURCES))
SPRITE_ANIMATION_SOURCES := $(call rwildcard,assets,*.sprite_animation.yaml)
SPRITE_ANIMATION_BINARIES := $(patsubst %.sprite_animation.yaml,%.sprite_animation.bin,$(SPRITE_ANIMATION_SOURCES))
GLYPH_METRICS_SOURCES := $(call rwildcard,assets,*.glyph_metrics.yaml)
GLYPH_METRICS_BINARIES := $(patsubst %.glyph_metrics.yaml,%.glyph_metrics.bin,$(GLYPH_METRICS_SOURCES))
TAB_CURSOR_LAYOUT_SOURCES := $(call rwildcard,assets,*.tab_cursor_layout.yaml)
TAB_CURSOR_LAYOUT_BINARIES := $(patsubst %.tab_cursor_layout.yaml,%.tab_cursor_layout.bin,$(TAB_CURSOR_LAYOUT_SOURCES))
INDEX_BOUNDARIES_SOURCES := $(call rwildcard,assets,*.index_boundaries.yaml)
INDEX_BOUNDARIES_BINARIES := $(patsubst %.index_boundaries.yaml,%.index_boundaries.bin,$(INDEX_BOUNDARIES_SOURCES))
INDEX_MAP_SOURCES := $(call rwildcard,assets,*.index_map.yaml)
INDEX_MAP_BINARIES := $(patsubst %.index_map.yaml,%.index_map.bin,$(INDEX_MAP_SOURCES))
NAME_ENTRY_RESOURCE_SOURCES := $(call rwildcard,assets,*.name_entry_resource.yaml)
NAME_ENTRY_RESOURCE_BINARIES := $(patsubst %.name_entry_resource.yaml,%.name_entry_resource.bin,$(NAME_ENTRY_RESOURCE_SOURCES))

.PHONY: validate-assets validate-psx-tim-assets validate-sprite-layout-assets validate-sprite-animation-assets validate-glyph-metrics-assets validate-tab-cursor-layout-assets validate-index-boundaries-assets validate-index-map-assets validate-name-entry-resource-assets

validate-assets: validate-psx-tim-assets validate-sprite-layout-assets validate-sprite-animation-assets validate-glyph-metrics-assets validate-tab-cursor-layout-assets validate-index-boundaries-assets validate-index-map-assets validate-name-entry-resource-assets

%.tim_trail.bin: %.tim tools/assets/psx_tim.py
	python3 tools/assets/psx_tim.py build $< $@ --trailing-duplicate-word

validate-psx-tim-assets: $(PSX_TIM_DUPLICATE_WORD_BINARIES)
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

%.glyph_metrics.bin: %.glyph_metrics.yaml tools/assets/glyph_metrics.py
	python3 tools/assets/glyph_metrics.py build $< $@

validate-glyph-metrics-assets: $(GLYPH_METRICS_BINARIES)
ifneq ($(strip $(GLYPH_METRICS_SOURCES)),)
	python3 tools/assets/glyph_metrics.py validate $(GLYPH_METRICS_SOURCES)
else
	@:
endif

%.tab_cursor_layout.bin: %.tab_cursor_layout.yaml tools/assets/tab_cursor_layout.py
	python3 tools/assets/tab_cursor_layout.py build $< $@

validate-tab-cursor-layout-assets: $(TAB_CURSOR_LAYOUT_BINARIES)
ifneq ($(strip $(TAB_CURSOR_LAYOUT_SOURCES)),)
	python3 tools/assets/tab_cursor_layout.py validate $(TAB_CURSOR_LAYOUT_SOURCES)
else
	@:
endif

%.index_boundaries.bin: %.index_boundaries.yaml tools/assets/index_boundaries.py
	python3 tools/assets/index_boundaries.py build $< $@

validate-index-boundaries-assets: $(INDEX_BOUNDARIES_BINARIES)
ifneq ($(strip $(INDEX_BOUNDARIES_SOURCES)),)
	python3 tools/assets/index_boundaries.py validate $(INDEX_BOUNDARIES_SOURCES)
else
	@:
endif

%.index_map.bin: %.index_map.yaml tools/assets/index_map.py
	python3 tools/assets/index_map.py build $< $@

validate-index-map-assets: $(INDEX_MAP_BINARIES)
ifneq ($(strip $(INDEX_MAP_SOURCES)),)
	python3 tools/assets/index_map.py validate $(INDEX_MAP_SOURCES)
else
	@:
endif

%.name_entry_resource.bin: %.name_entry_resource.yaml tools/assets/name_entry_resource.py
	python3 tools/assets/name_entry_resource.py build $< $@

validate-name-entry-resource-assets: $(NAME_ENTRY_RESOURCE_BINARIES)
ifneq ($(strip $(NAME_ENTRY_RESOURCE_SOURCES)),)
	python3 tools/assets/name_entry_resource.py validate $(NAME_ENTRY_RESOURCE_SOURCES)
else
	@:
endif

# Structured assets must be rebuilt before staging copies linker inputs to the
# native Linux filesystem used by the legacy toolchain.
$(COPY_SENTINEL): $(PSX_TIM_DUPLICATE_WORD_BINARIES) $(SPRITE_LAYOUT_BINARIES) $(SPRITE_ANIMATION_BINARIES) $(GLYPH_METRICS_BINARIES) $(TAB_CURSOR_LAYOUT_BINARIES) $(INDEX_BOUNDARIES_BINARIES) $(INDEX_MAP_BINARIES) $(NAME_ENTRY_RESOURCE_BINARIES)
