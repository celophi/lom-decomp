# Host-side Python tests; no disc files or historical toolchain required.
.PHONY: test-tools test-assets test-overlay-tools extract-scene extract-scenes extract-addhero extract-carda extract-checkps extract-cload extract-field extract-gname extract-golem extract-gosub extract-menu extract-niki extract-shop extract-title extract-wmap extract-wsel extract-zukan

test-tools: test-assets test-overlay-tools

test-assets:
	python3 -m unittest discover -s tools/assets/tests -t . -v

test-overlay-tools:
	python3 -m unittest discover -s tools/overlays/tests -t . -v

# SCENE is an extracted ANA/INFO_*/*.IMG file; VERSION selects the output folder.
SCENE_OUTPUT ?= assets/exports/$(VERSION)/scenes

extract-scene:
	@test -n "$(SCENE)" || { echo 'Usage: make extract-scene SCENE=/path/to/scene.IMG'; exit 1; }
	scene_name=$$(basename -- "$(SCENE)" .IMG); \
	python3 -m tools.scenes.field_scene "$(SCENE)" "$(SCENE_OUTPUT)/$$scene_name"

extract-scenes:
	@test -n "$(ANA)" || { echo 'Usage: make extract-scenes ANA=/path/to/ANA'; exit 1; }
	python3 -m tools.scenes.field_scene --all "$(ANA)" "$(SCENE_OUTPUT)"

# ADDHERO's data blob comes from `make splat`; VERSION selects the blob and the output folder.
ADDHERO_OUTPUT ?= assets/exports/$(VERSION)/overlays/addhero

extract-addhero:
	@test -f "$(ASSETS_DIR)/addhero_data.databin.bin" || { echo 'Run make splat first to extract $(ASSETS_DIR)/addhero_data.databin.bin'; exit 1; }
	python3 -m tools.overlays.addhero --version $(VERSION) "$(ADDHERO_OUTPUT)"

# CARDA follows the same export path; the build still reads its original data blob.
CARDA_OUTPUT ?= assets/exports/$(VERSION)/overlays/carda

extract-carda:
	@test -f "$(ASSETS_DIR)/carda_data.databin.bin" || { echo 'Run make splat first to extract $(ASSETS_DIR)/carda_data.databin.bin'; exit 1; }
	python3 -m tools.overlays.carda --version $(VERSION) "$(CARDA_OUTPUT)"

# CHECKPS keeps its initialized resources together; BSS stays with the code.
CHECKPS_OUTPUT ?= assets/exports/$(VERSION)/overlays/checkps

extract-checkps:
	@test -f "$(ASSETS_DIR)/checkps_data.databin.bin" || { echo 'Run make splat first to extract $(ASSETS_DIR)/checkps_data.databin.bin'; exit 1; }
	python3 -m tools.overlays.checkps --version $(VERSION) "$(CHECKPS_OUTPUT)"

# CLOAD carries text and tables; its icons are loaded from a separate CD resource.
CLOAD_OUTPUT ?= assets/exports/$(VERSION)/overlays/cload

extract-cload:
	@test -f "$(ASSETS_DIR)/cload_data.databin.bin" || { echo 'Run make splat first to extract $(ASSETS_DIR)/cload_data.databin.bin'; exit 1; }
	python3 -m tools.overlays.cload --version $(VERSION) "$(CLOAD_OUTPUT)"

# FIELD exports its resident resources; scene IMGs use extract-scene instead.
FIELD_OUTPUT ?= assets/exports/$(VERSION)/overlays/field

extract-field:
	@test -f "$(ASSETS_DIR)/field_data.databin.bin" || { echo 'Run make splat first to extract $(ASSETS_DIR)/field_data.databin.bin'; exit 1; }
	python3 -m tools.overlays.field --version $(VERSION) "$(FIELD_OUTPUT)"

# GNAME keeps its name-entry resources together; BSS stays with gname.c.
GNAME_OUTPUT ?= assets/exports/$(VERSION)/overlays/gname

extract-gname:
	@test -f "$(ASSETS_DIR)/gname_data.databin.bin" || { echo 'Run make splat first to extract $(ASSETS_DIR)/gname_data.databin.bin'; exit 1; }
	python3 -m tools.overlays.gname --version $(VERSION) "$(GNAME_OUTPUT)"

# GOLEM exports the logic-grid editor's artwork, text and packed panel records.
GOLEM_OUTPUT ?= assets/exports/$(VERSION)/overlays/golem

extract-golem:
	@test -f "$(ASSETS_DIR)/golem_data.databin.bin" || { echo 'Run make splat first to extract $(ASSETS_DIR)/golem_data.databin.bin'; exit 1; }
	python3 -m tools.overlays.golem --version $(VERSION) "$(GOLEM_OUTPUT)"

# GOSUB's data blob and equipment rodata are separate from its runtime BSS.
GOSUB_OUTPUT ?= assets/exports/$(VERSION)/overlays/gosub

extract-gosub:
	@test -f "$(ASSETS_DIR)/gosub_data.databin.bin" || { echo 'Run make splat first to extract $(ASSETS_DIR)/gosub_data.databin.bin'; exit 1; }
	python3 -m tools.overlays.gosub --version $(VERSION) "$(GOSUB_OUTPUT)"

# MENU exports the in-game menu's texture, icons, text tables and page layouts.
MENU_OUTPUT ?= assets/exports/$(VERSION)/overlays/menu

extract-menu:
	@test -f "$(ASSETS_DIR)/menu_data.databin.bin" || { echo 'Run make splat first to extract $(ASSETS_DIR)/menu_data.databin.bin'; exit 1; }
	python3 -m tools.overlays.menu --version $(VERSION) "$(MENU_OUTPUT)"

# NIKI carries the card-screen text, party icons and tables in one blob; its variables follow them.
NIKI_OUTPUT ?= assets/exports/$(VERSION)/overlays/niki

extract-niki:
	@test -f "$(ASSETS_DIR)/niki_data.databin.bin" || { echo 'Run make splat first to extract $(ASSETS_DIR)/niki_data.databin.bin'; exit 1; }
	python3 -m tools.overlays.niki --version $(VERSION) "$(NIKI_OUTPUT)"

# SHOP exports the shop screen's item text and sell prices.
SHOP_OUTPUT ?= assets/exports/$(VERSION)/overlays/shop

extract-shop:
	@test -f "$(ASSETS_DIR)/shop_data.databin.bin" || { echo 'Run make splat first to extract $(ASSETS_DIR)/shop_data.databin.bin'; exit 1; }
	python3 -m tools.overlays.shop --version $(VERSION) "$(SHOP_OUTPUT)"

# TITLE exports the menu artwork, character-selection screen and new-game states.
TITLE_OUTPUT ?= assets/exports/$(VERSION)/overlays/title

extract-title:
	@test -f "$(ASSETS_DIR)/title_data.databin.bin" || { echo 'Run make splat first to extract $(ASSETS_DIR)/title_data.databin.bin'; exit 1; }
	python3 -m tools.overlays.title --version $(VERSION) "$(TITLE_OUTPUT)"

# WMAP exports its world-map tables, sound effects, input scripts and step tables.
WMAP_OUTPUT ?= assets/exports/$(VERSION)/overlays/wmap

extract-wmap:
	@test -f "$(ASSETS_DIR)/wmap_data.databin.bin" || { echo 'Run make splat first to extract $(ASSETS_DIR)/wmap_data.databin.bin'; exit 1; }
	python3 -m tools.overlays.wmap --version $(VERSION) "$(WMAP_OUTPUT)"

# WSEL exports the play-area screen's eight TIMs, sprite layers and land-map grid tables.
WSEL_OUTPUT ?= assets/exports/$(VERSION)/overlays/wsel

extract-wsel:
	@test -f "$(ASSETS_DIR)/wsel_data.databin.bin" || { echo 'Run make splat first to extract $(ASSETS_DIR)/wsel_data.databin.bin'; exit 1; }
	python3 -m tools.overlays.wsel --version $(VERSION) "$(WSEL_OUTPUT)"

# ZUKAN's data blob and entry-table rodata sit on either side of its code.
ZUKAN_OUTPUT ?= assets/exports/$(VERSION)/overlays/zukan

extract-zukan:
	@test -f "$(ASSETS_DIR)/zukan_data.databin.bin" || { echo 'Run make splat first to extract $(ASSETS_DIR)/zukan_data.databin.bin'; exit 1; }
	python3 -m tools.overlays.zukan --version $(VERSION) "$(ZUKAN_OUTPUT)"
