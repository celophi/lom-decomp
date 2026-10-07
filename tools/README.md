# Project tools

Run commands from the repository root. Project Python tools use Python 3.10 or
newer; the build container includes their dependencies. Asset YAML converters
also need PyYAML (`python3 -m pip install -r requirements.txt` installs the
project's Python dependencies).

| Directory | Purpose |
| --- | --- |
| [data/formats/](data/formats/README.md) | Byte-exact asset parsers, builders and validators used by the build |
| [data/scenes/](data/scenes/README.md) | IMG scene extraction and asset byte maps |
| [data/overlays/](data/overlays/README.md) | Viewable exports of overlay data blobs |
| [compression/](compression/README.md) | Exact overlay compressor, reference compressor and decoder |
| [verification/data2c/](verification/data2c/README.md) | Generate typed C data and verify PS1 and host output |
| `verification/` | Binary comparison and native compile checks |
| `build/tests/` | Make staging, parallelism and generated-data cleanup tests |
| `splat_ext/` | Project-specific splat segment adapters |
| `objdiff/` | Generate objdiff configuration, run comparisons and format reports |
| `external/` | Third-party submodules |

Tests live beside the tools they exercise. Scene exports go under the ignored
`assets/exports/` tree. Other extracted game data can go under `output/` or
outside the repository.

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
make extract-field
make extract-gname
make extract-golem
make extract-gosub
make extract-menu
make extract-niki
make extract-shop
make extract-title
make extract-wmap
make extract-wsel
make extract-zukan

# Choose an exact output directory.
python3 -m tools.data.scenes.field_scene /path/to/scene.IMG output/scenes/example

# Inspect a standard TIM image.
python3 tools/data/formats/psx_tim.py info /path/to/image.tim

# Validate the selected version's generated build assets (in the build container).
make validate-assets
make validate-assets VERSION=jp
```

## External dependencies

`external/splat/`, `external/maspsx/`, `external/old-gcc/`,
`external/m2c/`, and `external/decomp-permuter/` are third-party
submodules. Keep their upstream directory layouts; consult each project's README
for its own tools and tests. `.gitmodules` is the source of truth for dependencies.
