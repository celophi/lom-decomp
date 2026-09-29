# Host-side Python tests; no disc files or historical toolchain required.
.PHONY: test-tools test-assets test-overlay-tools extract-scene extract-scenes extract-addhero extract-carda extract-checkps

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
