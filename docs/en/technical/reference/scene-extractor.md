# Scene IMG extractor

[English documentation](../../README.md) | [日本語](../../../jp/technical/reference/scene-extractor.md) | [Scene tools](../../../../tools/scenes/README.md) | [Scene layouts and conditions](../architecture/scene-layouts.md)

The extractor reads the game's `ANA/INFO_*/*.IMG` scene files. It checks their
section offsets, extracts recognized assets and saves the remaining bytes as
unknown data. The input file is never modified.

```sh
make extract-scene SCENE=/path/to/ANA/INFO_PRT/WAL_B020.IMG
```

Output goes to `assets/exports/us/scenes/WAL_B020/`. `VERSION=jp` selects the JP
output folder, and `SCENE_OUTPUT` changes the parent directory. Extraction needs
a new destination directory. The files are generated and ignored by Git.

For all supported scenes in an extracted ANA directory:

```sh
make extract-scenes ANA=/path/to/ANA
```

The batch target reads `INFO_*/*.IMG`. Output keeps the group and scene names,
for example `assets/exports/us/scenes/INFO_PRT/WAL_B020/`. Use `VERSION=jp` with a
Japanese ANA directory. `SCENE_OUTPUT` also applies to batch extraction.

Every scene destination must be new. The tool checks for existing destinations
before starting. Invalid scenes stop the batch with a filename; files already
extracted remain in place. Other IMG families aren't included.

## Output

| File | Contents |
| --- | --- |
| `byte-map.yaml` | Original IMG offsets, sizes and extracted filenames |
| `header.bin` | The 40-byte section-offset header |
| `textures/000.tim`, etc. | Whole TIM assets in file order |
| `portraits/000.bin`, etc. | Whole portrait records, including palette and pixels |
| `chests/<layout-index>.yaml` | Chest details and original record bytes |
| `unknown/<offset>.bin` | Data not extracted as a recognized asset |

Each YAML entry covers one range of bytes. `offset` is its hexadecimal position
in the original IMG, and `size` is its byte count. The range ends at
`offset + size`, exclusive. `file` is relative to the output directory. For a
chest, `size` is the original 48-byte record size, not the YAML file's length.

TIMs, portraits and recognized chest records each get one entry. Repeated
references to the same TIM produce one extracted file. The other ranges are
labeled `unknown_data`; this includes scripts, text, geometry and container tables. Some of their fields are
understood, but this tool leaves them as raw bytes.

Chest details live in their own YAML files, so the byte map only records where
each asset belongs. Other assets and unknown data remain binary. To recover the
original IMG, decode each chest's `record_bytes` hex string and concatenate it
with the other files in byte-map order.

## Chest records

The layout section starts with a u32 count followed by 48-byte
[`FieldLayoutRecord`](../../../../include/field_interaction_start.h) records.
A chest uses the same record structure as other actors. The extractor recognizes
the common chest resource selector and initializer script before labeling a
record as `chest`.

Its YAML file shows every layout field, including `condition.variable_ref` and
its minimum and maximum, decoded control and position fields, the resource
selector, enabled events, and all 16 script/parameter slots. It also shows the
chest settings `x`, `z`, `item_id`, `collection_variable_ref` and `alternate_facing`.
The item comes from `scripts[4]`. The collection reference uses bits 0-14 of
`scripts[5]`, and bit 15 selects the alternate facing. `record_bytes` keeps the
complete original record as hex.
The decoded fields describe that saved record; editing them does not change
`record_bytes`.

For example, `WAL_B020.IMG` has a chest at `(395, 180)` with item `0x96` and
collection-variable reference `0x0BC0`. It is exported as `chests/002.yaml` because it is layout
record 2. The event scripts remain in the unknown data; the tool doesn't execute
them or resolve item names. Records using other initializers stay raw until we
understand them.

## Supported format

The header contains ten little-endian u32 offsets: layout, event scripts,
strings, actor scripts, records, actors, geometry, images, portraits and group
bounds. The first section starts at byte 40. Offsets must be aligned, in order
and within the file.

The images section has an offset table followed by TIMs, checked with the
existing TIM parser. The portraits section has a count followed by 1,184-byte
records: a 32-byte palette and 48-by-48 pixels at 4 bits per pixel. Portraits
aren't TIMs.

This supports the scene IMG layout, not every IMG family or compressed file.
See the [scene loader](../../../../src/overlays/field/field_scene_transition.c)
and [portrait representation](../../../../src/overlays/field/field_text.c) for
the code that reads these resources.

The Python code follows these structures: `SceneHeader` documents the header
fields and `FieldLayoutRecord` documents the layout record. `read_chests()`,
`read_textures()` and `read_portraits()` handle their respective sections.
`AssetRange` describes an extracted byte range; it isn't a disk structure.
