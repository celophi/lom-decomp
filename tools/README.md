# Project tools

Run commands from the repository root. Project Python tools use Python 3.10 or
newer; the build container includes their dependencies. Asset YAML converters
also need PyYAML (`python3 -m pip install -r requirements.txt` installs the
project's Python dependencies).

| Directory | Purpose |
| --- | --- |
| [assets/](assets/README.md) | Byte-exact asset parsers, builders and validators used by the build |
| [scenes/](scenes/README.md) | IMG scene extraction and asset byte maps |
| [overlays/](overlays/README.md) | Viewable exports of overlay data blobs (ADDHERO, CARDA, CHECKPS and CLOAD resources) |
| [compressor/](compressor/README.md) | Original overlay compression and whole-file verification |
| `splat_ext/` | Project-specific splat segments and reference compression routines |
| `objdiff/` | Generate objdiff configuration, run comparisons and format reports |

`assets/tests/` contains the asset format tests. Scene exports go under the
ignored `assets/exports/` tree. Other extracted game data can go under `output/`
or outside the repository.

## Common commands

```sh
# Run asset format tests without a disc or compiler container.
make test-tools

# Extract an IMG scene and its asset byte map.
make extract-scene SCENE=/path/to/scene.IMG

# Extract all supported scenes from an ANA directory.
make extract-scenes ANA=/path/to/ANA

# Export overlay text, icons and tables (after make splat).
make extract-addhero
make extract-carda
make extract-checkps
make extract-cload

# Choose an exact output directory.
python3 -m tools.scenes.field_scene /path/to/scene.IMG output/scenes/example

# Inspect a standard TIM image.
python3 tools/assets/psx_tim.py info /path/to/image.tim

# Validate the selected version's generated build assets (in the build container).
make validate-assets
make validate-assets VERSION=jp
```

## External dependencies

`splat/`, `maspsx/`, `old-gcc/`, `m2c/`, and `decomp-permuter/` are third-party
submodules. Keep their upstream directory layouts; consult each project's README
for its own tools and tests. `.gitmodules` is the source of truth for dependencies.
