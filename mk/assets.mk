# Asset rebuilds and validation

.PHONY: validate-assets

PSX_TIM_ASSETS := $(call rwildcard,$(ASSETS_DIR),*.tim)
PSX_TIM_DUPLICATE_WORD_BINARIES := $(call rwildcard,$(ASSETS_DIR),*.tim_trail.bin)
ASSET_BINARIES := $(PSX_TIM_DUPLICATE_WORD_BINARIES)

.PHONY: validate-psx-tim-assets
validate-assets: validate-psx-tim-assets

%.tim_trail.bin: %.tim tools/data/formats/psx_tim.py
	python3 tools/data/formats/psx_tim.py build $< $@ --trailing-duplicate-word

validate-psx-tim-assets: $(PSX_TIM_DUPLICATE_WORD_BINARIES)
ifneq ($(strip $(PSX_TIM_ASSETS)),)
	python3 tools/data/formats/psx_tim.py roundtrip $(PSX_TIM_ASSETS)
else
	@:
endif

ASSET_OFFSET_TABLE_SOURCES := $(call rwildcard,$(ASSETS_DIR),*.asset_offset_table.yaml)
ASSET_OFFSET_TABLE_BINARIES := $(patsubst %.asset_offset_table.yaml,%.asset_offset_table.bin,$(ASSET_OFFSET_TABLE_SOURCES))
ASSET_BINARIES += $(ASSET_OFFSET_TABLE_BINARIES)

.PHONY: validate-asset-offset-table-assets
validate-assets: validate-asset-offset-table-assets

%.asset_offset_table.bin: %.asset_offset_table.yaml tools/data/formats/asset_offset_table.py
	python3 tools/data/formats/asset_offset_table.py build $< $@

validate-asset-offset-table-assets: $(ASSET_OFFSET_TABLE_BINARIES)
ifneq ($(strip $(ASSET_OFFSET_TABLE_SOURCES)),)
	python3 tools/data/formats/asset_offset_table.py validate $(ASSET_OFFSET_TABLE_SOURCES)
else
	@:
endif

U8_SEQUENCE_SOURCES := $(call rwildcard,$(ASSETS_DIR),*.u8_sequence.yaml)
U8_SEQUENCE_BINARIES := $(patsubst %.u8_sequence.yaml,%.u8_sequence.bin,$(U8_SEQUENCE_SOURCES))
ASSET_BINARIES += $(U8_SEQUENCE_BINARIES)

.PHONY: validate-u8-sequence-assets
validate-assets: validate-u8-sequence-assets

%.u8_sequence.bin: %.u8_sequence.yaml tools/data/formats/u8_sequence.py
	python3 tools/data/formats/u8_sequence.py build $< $@

validate-u8-sequence-assets: $(U8_SEQUENCE_BINARIES)
ifneq ($(strip $(U8_SEQUENCE_SOURCES)),)
	python3 tools/data/formats/u8_sequence.py validate $(U8_SEQUENCE_SOURCES)
else
	@:
endif

TIM_UPLOAD_TABLE_SOURCES := $(call rwildcard,$(ASSETS_DIR),*.tim_upload_table.yaml)
TIM_UPLOAD_TABLE_BINARIES := $(patsubst %.tim_upload_table.yaml,%.tim_upload_table.bin,$(TIM_UPLOAD_TABLE_SOURCES))
ASSET_BINARIES += $(TIM_UPLOAD_TABLE_BINARIES)

.PHONY: validate-tim-upload-table-assets
validate-assets: validate-tim-upload-table-assets

%.tim_upload_table.bin: %.tim_upload_table.yaml tools/data/formats/tim_upload_table.py
	python3 tools/data/formats/tim_upload_table.py build $< $@

validate-tim-upload-table-assets: $(TIM_UPLOAD_TABLE_BINARIES)
ifneq ($(strip $(TIM_UPLOAD_TABLE_SOURCES)),)
	python3 tools/data/formats/tim_upload_table.py validate $(TIM_UPLOAD_TABLE_SOURCES)
else
	@:
endif

UV_RECT_TABLE_SOURCES := $(call rwildcard,$(ASSETS_DIR),*.uv_rect_table.yaml)
UV_RECT_TABLE_BINARIES := $(patsubst %.uv_rect_table.yaml,%.uv_rect_table.bin,$(UV_RECT_TABLE_SOURCES))
ASSET_BINARIES += $(UV_RECT_TABLE_BINARIES)

.PHONY: validate-uv-rect-table-assets
validate-assets: validate-uv-rect-table-assets

%.uv_rect_table.bin: %.uv_rect_table.yaml tools/data/formats/uv_rect_table.py
	python3 tools/data/formats/uv_rect_table.py build $< $@

validate-uv-rect-table-assets: $(UV_RECT_TABLE_BINARIES)
ifneq ($(strip $(UV_RECT_TABLE_SOURCES)),)
	python3 tools/data/formats/uv_rect_table.py validate $(UV_RECT_TABLE_SOURCES)
else
	@:
endif

SAVE_LAYOUT_TABLE_SOURCES := $(call rwildcard,$(ASSETS_DIR),*.save_layout_table.yaml)
SAVE_LAYOUT_TABLE_BINARIES := $(patsubst %.save_layout_table.yaml,%.save_layout_table.bin,$(SAVE_LAYOUT_TABLE_SOURCES))
ASSET_BINARIES += $(SAVE_LAYOUT_TABLE_BINARIES)

.PHONY: validate-save-layout-table-assets
validate-assets: validate-save-layout-table-assets

%.save_layout_table.bin: %.save_layout_table.yaml tools/data/formats/save_layout_table.py
	python3 tools/data/formats/save_layout_table.py build $< $@

validate-save-layout-table-assets: $(SAVE_LAYOUT_TABLE_BINARIES)
ifneq ($(strip $(SAVE_LAYOUT_TABLE_SOURCES)),)
	python3 tools/data/formats/save_layout_table.py validate $(SAVE_LAYOUT_TABLE_SOURCES)
else
	@:
endif

STARTING_WEAPON_TABLE_SOURCES := $(call rwildcard,$(ASSETS_DIR),*.starting_weapon_table.yaml)
STARTING_WEAPON_TABLE_BINARIES := $(patsubst %.starting_weapon_table.yaml,%.starting_weapon_table.bin,$(STARTING_WEAPON_TABLE_SOURCES))
ASSET_BINARIES += $(STARTING_WEAPON_TABLE_BINARIES)

.PHONY: validate-starting-weapon-table-assets
validate-assets: validate-starting-weapon-table-assets

%.starting_weapon_table.bin: %.starting_weapon_table.yaml tools/data/formats/starting_weapon_table.py
	python3 tools/data/formats/starting_weapon_table.py build $< $@

validate-starting-weapon-table-assets: $(STARTING_WEAPON_TABLE_BINARIES)
ifneq ($(strip $(STARTING_WEAPON_TABLE_SOURCES)),)
	python3 tools/data/formats/starting_weapon_table.py validate $(STARTING_WEAPON_TABLE_SOURCES)
else
	@:
endif

GAME_STATE_TEMPLATE_SOURCES := $(call rwildcard,$(ASSETS_DIR),*.game_state_template.yaml)
GAME_STATE_TEMPLATE_BINARIES := $(patsubst %.game_state_template.yaml,%.game_state_template.bin,$(GAME_STATE_TEMPLATE_SOURCES))
ASSET_BINARIES += $(GAME_STATE_TEMPLATE_BINARIES)

.PHONY: validate-game-state-template-assets
validate-assets: validate-game-state-template-assets

%.game_state_template.bin: %.game_state_template.yaml %.payload.bin tools/data/formats/game_state_template.py
	python3 tools/data/formats/game_state_template.py build $< $@

validate-game-state-template-assets: $(GAME_STATE_TEMPLATE_BINARIES)
ifneq ($(strip $(GAME_STATE_TEMPLATE_SOURCES)),)
	python3 tools/data/formats/game_state_template.py validate $(GAME_STATE_TEMPLATE_SOURCES)
else
	@:
endif

SPRITE_LAYOUT_SOURCES := $(call rwildcard,$(ASSETS_DIR),*.sprite_layout.yaml)
SPRITE_LAYOUT_BINARIES := $(patsubst %.sprite_layout.yaml,%.sprite_layout.bin,$(SPRITE_LAYOUT_SOURCES))
ASSET_BINARIES += $(SPRITE_LAYOUT_BINARIES)

.PHONY: validate-sprite-layout-assets
validate-assets: validate-sprite-layout-assets

%.sprite_layout.bin: %.sprite_layout.yaml tools/data/formats/sprite_layout.py
	python3 tools/data/formats/sprite_layout.py build $< $@

validate-sprite-layout-assets: $(SPRITE_LAYOUT_BINARIES)
ifneq ($(strip $(SPRITE_LAYOUT_SOURCES)),)
	python3 tools/data/formats/sprite_layout.py validate $(SPRITE_LAYOUT_SOURCES)
else
	@:
endif

SPRITE_ANIMATION_SOURCES := $(call rwildcard,$(ASSETS_DIR),*.sprite_animation.yaml)
SPRITE_ANIMATION_BINARIES := $(patsubst %.sprite_animation.yaml,%.sprite_animation.bin,$(SPRITE_ANIMATION_SOURCES))
ASSET_BINARIES += $(SPRITE_ANIMATION_BINARIES)

.PHONY: validate-sprite-animation-assets
validate-assets: validate-sprite-animation-assets

%.sprite_animation.bin: %.sprite_animation.yaml tools/data/formats/sprite_animation.py
	python3 tools/data/formats/sprite_animation.py build $< $@

validate-sprite-animation-assets: $(SPRITE_ANIMATION_BINARIES)
ifneq ($(strip $(SPRITE_ANIMATION_SOURCES)),)
	python3 tools/data/formats/sprite_animation.py validate $(SPRITE_ANIMATION_SOURCES)
else
	@:
endif

GLYPH_METRICS_SOURCES := $(call rwildcard,$(ASSETS_DIR),*.glyph_metrics.yaml)
GLYPH_METRICS_BINARIES := $(patsubst %.glyph_metrics.yaml,%.glyph_metrics.bin,$(GLYPH_METRICS_SOURCES))
ASSET_BINARIES += $(GLYPH_METRICS_BINARIES)

.PHONY: validate-glyph-metrics-assets
validate-assets: validate-glyph-metrics-assets

%.glyph_metrics.bin: %.glyph_metrics.yaml tools/data/formats/glyph_metrics.py
	python3 tools/data/formats/glyph_metrics.py build $< $@

validate-glyph-metrics-assets: $(GLYPH_METRICS_BINARIES)
ifneq ($(strip $(GLYPH_METRICS_SOURCES)),)
	python3 tools/data/formats/glyph_metrics.py validate $(GLYPH_METRICS_SOURCES)
else
	@:
endif

TAB_CURSOR_LAYOUT_SOURCES := $(call rwildcard,$(ASSETS_DIR),*.tab_cursor_layout.yaml)
TAB_CURSOR_LAYOUT_BINARIES := $(patsubst %.tab_cursor_layout.yaml,%.tab_cursor_layout.bin,$(TAB_CURSOR_LAYOUT_SOURCES))
ASSET_BINARIES += $(TAB_CURSOR_LAYOUT_BINARIES)

.PHONY: validate-tab-cursor-layout-assets
validate-assets: validate-tab-cursor-layout-assets

%.tab_cursor_layout.bin: %.tab_cursor_layout.yaml tools/data/formats/tab_cursor_layout.py
	python3 tools/data/formats/tab_cursor_layout.py build $< $@

validate-tab-cursor-layout-assets: $(TAB_CURSOR_LAYOUT_BINARIES)
ifneq ($(strip $(TAB_CURSOR_LAYOUT_SOURCES)),)
	python3 tools/data/formats/tab_cursor_layout.py validate $(TAB_CURSOR_LAYOUT_SOURCES)
else
	@:
endif

INDEX_BOUNDARIES_SOURCES := $(call rwildcard,$(ASSETS_DIR),*.index_boundaries.yaml)
INDEX_BOUNDARIES_BINARIES := $(patsubst %.index_boundaries.yaml,%.index_boundaries.bin,$(INDEX_BOUNDARIES_SOURCES))
ASSET_BINARIES += $(INDEX_BOUNDARIES_BINARIES)

.PHONY: validate-index-boundaries-assets
validate-assets: validate-index-boundaries-assets

%.index_boundaries.bin: %.index_boundaries.yaml tools/data/formats/index_boundaries.py
	python3 tools/data/formats/index_boundaries.py build $< $@

validate-index-boundaries-assets: $(INDEX_BOUNDARIES_BINARIES)
ifneq ($(strip $(INDEX_BOUNDARIES_SOURCES)),)
	python3 tools/data/formats/index_boundaries.py validate $(INDEX_BOUNDARIES_SOURCES)
else
	@:
endif

INDEX_MAP_SOURCES := $(call rwildcard,$(ASSETS_DIR),*.index_map.yaml)
INDEX_MAP_BINARIES := $(patsubst %.index_map.yaml,%.index_map.bin,$(INDEX_MAP_SOURCES))
ASSET_BINARIES += $(INDEX_MAP_BINARIES)

.PHONY: validate-index-map-assets
validate-assets: validate-index-map-assets

%.index_map.bin: %.index_map.yaml tools/data/formats/index_map.py
	python3 tools/data/formats/index_map.py build $< $@

validate-index-map-assets: $(INDEX_MAP_BINARIES)
ifneq ($(strip $(INDEX_MAP_SOURCES)),)
	python3 tools/data/formats/index_map.py validate $(INDEX_MAP_SOURCES)
else
	@:
endif

NAME_ENTRY_RESOURCE_SOURCES := $(call rwildcard,$(ASSETS_DIR),*.name_entry_resource.yaml)
NAME_ENTRY_RESOURCE_BINARIES := $(patsubst %.name_entry_resource.yaml,%.name_entry_resource.bin,$(NAME_ENTRY_RESOURCE_SOURCES))
ASSET_BINARIES += $(NAME_ENTRY_RESOURCE_BINARIES)

.PHONY: validate-name-entry-resource-assets
validate-assets: validate-name-entry-resource-assets

%.name_entry_resource.bin: %.name_entry_resource.yaml tools/data/formats/name_entry_resource.py
	python3 tools/data/formats/name_entry_resource.py build $< $@

validate-name-entry-resource-assets: $(NAME_ENTRY_RESOURCE_BINARIES)
ifneq ($(strip $(NAME_ENTRY_RESOURCE_SOURCES)),)
	python3 tools/data/formats/name_entry_resource.py validate $(NAME_ENTRY_RESOURCE_SOURCES)
else
	@:
endif

# Rebuild assets before staging so the linker sees the edited data.
$(COPY_SENTINEL): $(ASSET_BINARIES)
