# ============================================================================
#  Format-aware asset validation
# ============================================================================

PSX_TIM_ASSETS := $(call rwildcard,assets,*.tim)
PSX_TIM_DUPLICATE_WORD_BINARIES := $(call rwildcard,assets,*.tim_trail.bin)
ASSET_OFFSET_TABLE_SOURCES := $(call rwildcard,assets,*.asset_offset_table.yaml)
ASSET_OFFSET_TABLE_BINARIES := $(patsubst %.asset_offset_table.yaml,%.asset_offset_table.bin,$(ASSET_OFFSET_TABLE_SOURCES))
U8_SEQUENCE_SOURCES := $(call rwildcard,assets,*.u8_sequence.yaml)
U8_SEQUENCE_BINARIES := $(patsubst %.u8_sequence.yaml,%.u8_sequence.bin,$(U8_SEQUENCE_SOURCES))
TIM_UPLOAD_TABLE_SOURCES := $(call rwildcard,assets,*.tim_upload_table.yaml)
TIM_UPLOAD_TABLE_BINARIES := $(patsubst %.tim_upload_table.yaml,%.tim_upload_table.bin,$(TIM_UPLOAD_TABLE_SOURCES))
UV_RECT_TABLE_SOURCES := $(call rwildcard,assets,*.uv_rect_table.yaml)
UV_RECT_TABLE_BINARIES := $(patsubst %.uv_rect_table.yaml,%.uv_rect_table.bin,$(UV_RECT_TABLE_SOURCES))
SAVE_LAYOUT_TABLE_SOURCES := $(call rwildcard,assets,*.save_layout_table.yaml)
SAVE_LAYOUT_TABLE_BINARIES := $(patsubst %.save_layout_table.yaml,%.save_layout_table.bin,$(SAVE_LAYOUT_TABLE_SOURCES))
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

.PHONY: validate-assets validate-psx-tim-assets validate-asset-offset-table-assets validate-u8-sequence-assets validate-tim-upload-table-assets validate-uv-rect-table-assets validate-save-layout-table-assets validate-sprite-layout-assets validate-sprite-animation-assets validate-glyph-metrics-assets validate-tab-cursor-layout-assets validate-index-boundaries-assets validate-index-map-assets validate-name-entry-resource-assets

validate-assets: validate-psx-tim-assets validate-asset-offset-table-assets validate-u8-sequence-assets validate-tim-upload-table-assets validate-uv-rect-table-assets validate-save-layout-table-assets validate-sprite-layout-assets validate-sprite-animation-assets validate-glyph-metrics-assets validate-tab-cursor-layout-assets validate-index-boundaries-assets validate-index-map-assets validate-name-entry-resource-assets

%.tim_trail.bin: %.tim tools/assets/psx_tim.py
	python3 tools/assets/psx_tim.py build $< $@ --trailing-duplicate-word

validate-psx-tim-assets: $(PSX_TIM_DUPLICATE_WORD_BINARIES)
ifneq ($(strip $(PSX_TIM_ASSETS)),)
	python3 tools/assets/psx_tim.py roundtrip $(PSX_TIM_ASSETS)
else
	@:
endif

%.asset_offset_table.bin: %.asset_offset_table.yaml tools/assets/asset_offset_table.py
	python3 tools/assets/asset_offset_table.py build $< $@

validate-asset-offset-table-assets: $(ASSET_OFFSET_TABLE_BINARIES)
ifneq ($(strip $(ASSET_OFFSET_TABLE_SOURCES)),)
	python3 tools/assets/asset_offset_table.py validate $(ASSET_OFFSET_TABLE_SOURCES)
else
	@:
endif

%.u8_sequence.bin: %.u8_sequence.yaml tools/assets/u8_sequence.py
	python3 tools/assets/u8_sequence.py build $< $@

validate-u8-sequence-assets: $(U8_SEQUENCE_BINARIES)
ifneq ($(strip $(U8_SEQUENCE_SOURCES)),)
	python3 tools/assets/u8_sequence.py validate $(U8_SEQUENCE_SOURCES)
else
	@:
endif

%.tim_upload_table.bin: %.tim_upload_table.yaml tools/assets/tim_upload_table.py
	python3 tools/assets/tim_upload_table.py build $< $@

validate-tim-upload-table-assets: $(TIM_UPLOAD_TABLE_BINARIES)
ifneq ($(strip $(TIM_UPLOAD_TABLE_SOURCES)),)
	python3 tools/assets/tim_upload_table.py validate $(TIM_UPLOAD_TABLE_SOURCES)
else
	@:
endif

%.uv_rect_table.bin: %.uv_rect_table.yaml tools/assets/uv_rect_table.py
	python3 tools/assets/uv_rect_table.py build $< $@

validate-uv-rect-table-assets: $(UV_RECT_TABLE_BINARIES)
ifneq ($(strip $(UV_RECT_TABLE_SOURCES)),)
	python3 tools/assets/uv_rect_table.py validate $(UV_RECT_TABLE_SOURCES)
else
	@:
endif

%.save_layout_table.bin: %.save_layout_table.yaml tools/assets/save_layout_table.py
	python3 tools/assets/save_layout_table.py build $< $@

validate-save-layout-table-assets: $(SAVE_LAYOUT_TABLE_BINARIES)
ifneq ($(strip $(SAVE_LAYOUT_TABLE_SOURCES)),)
	python3 tools/assets/save_layout_table.py validate $(SAVE_LAYOUT_TABLE_SOURCES)
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
$(COPY_SENTINEL): $(PSX_TIM_DUPLICATE_WORD_BINARIES) $(ASSET_OFFSET_TABLE_BINARIES) $(U8_SEQUENCE_BINARIES) $(TIM_UPLOAD_TABLE_BINARIES) $(UV_RECT_TABLE_BINARIES) $(SAVE_LAYOUT_TABLE_BINARIES) $(SPRITE_LAYOUT_BINARIES) $(SPRITE_ANIMATION_BINARIES) $(GLYPH_METRICS_BINARIES) $(TAB_CURSOR_LAYOUT_BINARIES) $(INDEX_BOUNDARIES_BINARIES) $(INDEX_MAP_BINARIES) $(NAME_ENTRY_RESOURCE_BINARIES)
