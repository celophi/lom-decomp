# Keep this list beside the makefiles so target names are easy to find.
.PHONY: help
help:
	@printf '%s\n' \
		'Usage: make [target] [VERSION=us|jp]' \
		'' \
		'Build (inside the toolchain container):' \
		'  all / bin             Main executable ELF / raw binary (all is the default)' \
		'  <overlay>             One overlay, e.g. make checkps' \
		'  overlays / everything All overlays / main executable and all overlays' \
		'  decompress            Prepare decompressed overlay inputs under build/<version>/decompressed/' \
		'  splat                 Prepare inputs and extract assembly, linker scripts and assets' \
		'  recopy                Refresh the staged inputs' \
		'  clean                 Remove the selected build and all shared staging files' \
		'' \
		'Checks:' \
		'  validate-overlays     Check overlay directories and compiler assignments' \
		'  validate-assets       Rebuild and validate structured assets' \
		'  verify-bins           Compare the main executable and overlays with disc files' \
		'  verify-main           Check just the executable (verify-slus is an alias)' \
		'  verify-<overlay>      Check one registered verification target' \
		'  verify-compressor     Test compression against original overlay files' \
		'  verify-data-as-c      Rebuild with generated C data and compare disc files' \
		'  clean-data-as-c       Remove data objects and temporary generated C' \
		'  verify-data-host      Check generated data values with the host compiler' \
		'  native-check          Compile game C with the host compiler' \
		'  test-tools            Run asset, scene, overlay and verification-helper tests' \
		'' \
		'Object comparison:' \
		'  target-objects        Assemble original main-executable code for objdiff' \
		'  base-objects          Compile main-executable C for objdiff' \
		'  <overlay>-objdiff     Build both sets of objects for one overlay' \
		'  objdiff-objects       Build comparison objects for every image' \
		'  objdiff-config        Build objects and generate objdiff.json' \
		'  progress              Write the objdiff progress report' \
		'  diff-all / diff-text  Write JSON / text comparisons' \
		'  dump-objs             Disassemble existing build objects' \
		'' \
		'Asset exports (host Python):' \
		'  identify-img IMG=path/to/file-or-directory' \
		'  inspect-scene-strings SCENE=path/to/scene.IMG [SCENE_DATA_FORMAT=yaml]' \
		'  inspect-scene-geometry SCENE=path/to/scene.IMG [SCENE_DATA_FORMAT=yaml]' \
		'  inspect-scene-resources SCENE=path/to/scene.IMG [SCENE_DATA_FORMAT=yaml]' \
		'  inspect-scene-actors SCENE=path/to/scene.IMG' \
		'  inspect-scene-objects SCENE=path/to/scene.IMG [SCENE_REPORT_FORMAT=yaml] [SCENE_REFERENCE_VERSION=us|jp]' \
		'  inspect-scene-event-scripts SCENE=path/to/scene.IMG [SCENE_EVENT_FORMAT=text]' \
		'  inspect-scene-actor-scripts SCENE=path/to/scene.IMG [SCENE_SCRIPT_FORMAT=text]' \
		'  extract-scene SCENE=path/to/scene.IMG' \
		'  extract-scenes ANA=path/to/ANA' \
		'  extract-<overlay>     Export resources; see mk/tools.mk for supported overlays' \
		'' \
		'Compiler flags: mk/toolchains.mk' \
		'Main source lists: mk/main-sources.mk' \
		'Overlay source lists: mk/overlay-registry.mk' \
		'Overlays: $(OVERLAYS)'
