# Project tools

Run commands from the repository root. Project Python tools use Python 3.10 or
newer; the build container includes their dependencies. Asset YAML converters
also need PyYAML (`python3 -m pip install -r requirements.txt` installs the
project's Python dependencies).

| Directory | Purpose |
| --- | --- |
| [assets/](assets/README.md) | Byte-exact asset parsers, builders and validators used by the build |
| [scenes/](scenes/README.md) | Standalone scene inspection and extraction |
| [compressor/](compressor/README.md) | Original overlay compression and whole-file verification |
| `splat_ext/` | Project-specific splat segments and reference compression routines |
| `objdiff/` | Generate objdiff configuration, run comparisons and format reports |

`assets/tests/` and `scenes/tests/` contain synthetic fixtures and tests. Extracted
game data belongs under the ignored `output/` directory or outside the repository.

## Common commands

```sh
# Run asset and scene unit tests without a disc or compiler container.
make test-tools

# Inspect a scene, then extract it into a new directory.
python3 -m tools.scenes.field_scene info /path/to/scene.IMG
python3 -m tools.scenes.field_scene extract /path/to/scene.IMG output/scenes/example

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
