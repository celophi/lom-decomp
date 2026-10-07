# Scene extractor

[Tools index](../../README.md) | [IMG format and byte map](../../../docs/en/technical/reference/scene-extractor.md)

Extract the assets from an `ANA/INFO_*/*.IMG` scene file:

```sh
make extract-scene SCENE=/path/to/ANA/INFO_PRT/WAL_B020.IMG
```

This writes to `assets/exports/us/scenes/WAL_B020/`. Use `VERSION=jp` for the JP
output folder, or `SCENE_OUTPUT=/path/to/scenes` to choose another parent folder.
To extract all supported scenes under an ANA directory:

```sh
make extract-scenes ANA=/path/to/ANA
make extract-scenes VERSION=jp ANA=/path/to/japanese/ANA
```

This reads `INFO_*/*.IMG` and keeps the grouping in the output, for example
`assets/exports/us/scenes/INFO_PRT/WAL_B020/`. Other IMG families are left alone.
Existing scene destinations stop the batch before extraction starts. An invalid
scene stops extraction and reports its filename; any earlier exports remain.

To choose an exact destination:

```sh
python3 -m tools.data.scenes.field_scene /path/to/scene.IMG output/scenes/example
python3 -m tools.data.scenes.field_scene --all /path/to/ANA output/scenes
```

The tool checks the section offsets and extracts whole TIMs, portraits and
recognized chest records. It saves the remaining bytes as unknown data.
`byte-map.yaml` lists each asset's original offset and size. Chest details go
in `chests/<layout-index>.yaml`, with the original record bytes saved as hex.
Other assets and unknown data are copied unchanged.

Run from the repository root with Python 3.10 or newer. The destination must be
new; the tool won't overwrite an existing directory. Generated files are ignored
by Git under `assets/exports/` and `output/`.
