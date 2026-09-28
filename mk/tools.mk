# Host-side Python tests; no disc files or historical toolchain required.
.PHONY: test-tools test-assets test-scenes

test-tools: test-assets test-scenes

test-assets:
	python3 -m unittest discover -s tools/assets/tests -t . -v

test-scenes:
	python3 -m unittest discover -s tools/scenes/tests -t . -v
