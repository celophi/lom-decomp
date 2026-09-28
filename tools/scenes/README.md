# Scene tools

[Tools index](../README.md) | [Scene format and extraction guide](../../docs/en/technical/reference/scene-extractor.md)

`field_scene.py` inspects the original game's `ANA/INFO_*/*.IMG` scene containers.
It is independent of the game build and reuses the TIM parser in `tools/assets`.
Run it as a Python module from the repository root:

```sh
python3 -m tools.scenes.field_scene info /path/to/scene.IMG
python3 -m tools.scenes.field_scene info /path/to/scene.IMG --json
python3 -m tools.scenes.field_scene extract /path/to/scene.IMG output/scenes/example
python3 -m tools.scenes.field_scene validate /path/to/ANA/INFO_PRT/*.IMG
make test-scenes
```

Extraction writes a manifest, a text summary, all original sections and separate
TIM files. It requires a new output directory and never modifies the input.
Tests are under `tests/`. No third-party extraction code is included.
