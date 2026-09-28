# Host-side Python tests; no disc files or historical toolchain required.
.PHONY: test-tools test-assets extract-scene

test-tools: test-assets

test-assets:
	python3 -m unittest discover -s tools/assets/tests -t . -v

# SCENE is an extracted ANA/INFO_*/*.IMG file; VERSION selects the output folder.
SCENE_OUTPUT ?= assets/exports/$(VERSION)/scenes

extract-scene:
	@test -n "$(SCENE)" || { echo 'Usage: make extract-scene SCENE=/path/to/scene.IMG'; exit 1; }
	scene_name=$$(basename -- "$(SCENE)" .IMG); \
	python3 -m tools.scenes.field_scene "$(SCENE)" "$(SCENE_OUTPUT)/$$scene_name"
