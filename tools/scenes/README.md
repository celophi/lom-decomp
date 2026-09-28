# Scene extractor

[Tools index](../README.md) | [IMG format and byte map](../../docs/en/technical/reference/scene-extractor.md)

Extract the assets from an `ANA/INFO_*/*.IMG` scene file:

```sh
make extract-scene SCENE=/path/to/ANA/INFO_PRT/WAL_B020.IMG
```

This writes to `assets/exports/us/scenes/WAL_B020/`. Use `VERSION=jp` for the JP
output folder, or `SCENE_OUTPUT=/path/to/scenes` to choose another parent folder.
To choose an exact destination:

```sh
python3 -m tools.scenes.field_scene /path/to/scene.IMG output/scenes/example
```

The tool checks the section offsets and extracts whole TIMs, portraits and
recognized chest records. It saves the remaining bytes as unknown data.
`byte-map.yaml` lists each file's offset and size, plus the position, item ID
and collection flag for common chests. All bytes are preserved once.

Run from the repository root with Python 3.10 or newer. The destination must be
new; the tool won't overwrite an existing directory. Generated files are ignored
by Git under `assets/exports/` and `output/`.
