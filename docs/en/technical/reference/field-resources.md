# FIELD resources

[English index](../../README.md) | [Overlay extractors](../../../../tools/overlays/README.md) | [Japanese](../../../jp/technical/reference/field-resources.md)

FIELD carries the common graphics and tables the field runtime needs between
scene loads. Both regional YAMLs now keep that data in one `field_data`
databin. The build links the original bytes; a separate extractor makes them
readable.

```sh
make splat
make extract-field

make splat VERSION=jp
make extract-field VERSION=jp
```

Files go to `assets/exports/<version>/overlays/field/`. Set
`FIELD_OUTPUT=/path/to/new-folder` for another destination. The folder must be
new. The extractor reads everything before writing and removes its temporary
folder if a write fails.

JP also reads CLOAD's character chart to turn the game codes into Japanese.
`make splat VERSION=jp` produces that blob too. The chart's source and address
are recorded in `byte-map.yaml`; FIELD itself doesn't contain that conversion
table.

## What comes out

| Folder | Contents |
| --- | --- |
| `images/common/` | The original 512 x 256 TIM, its layout and 64 palette previews |
| `images/transition/` | A 32 x 64 direct-color TIM containing the two fade tiles, plus a PNG |
| `images/menu_frame/` | The 64 x 32 menu frame artwork, two palettes and previews |
| `palettes/` | Golem and portrait palettes, offset headers and color swatches |
| `text/` | UI, technique, command, item and weekday text, plus memory-card filenames and recognized product codes |
| `animations/` | The built-in animation directory and 202 definitions, with the original resource |
| `actions/` | The four default action descriptor tables, with the original bank |
| `tables/` | HUD colors, input and animation maps, unlock rules, dispatch tables, window rectangles, golem shapes and other small arrays |
| `unknown/` | Remaining gaps saved unchanged, named by their RAM address |

Each common-texture PNG uses one palette for the whole image. The game picks
palettes per sprite, so no single preview shows every sprite in its runtime
colors. The original TIM keeps its VRAM coordinates and all palette bits.
FIELD uploads that texture at (384, 0), which differs from the coordinates
stored in the TIM. Palette value zero is transparent in the previews; other
colors are shown opaque.

The animation export describes the directory and definitions. Part, curve and
frame data remain in `animations/resource.bin`, at the offsets listed in the
YAML. It doesn't play animations. The action export decodes the common fields
and keeps the parameter words whose interpretation depends on the handler.

## Text tables

| File | Entries |
| --- | ---: |
| `ui.yaml` | 37 |
| `techniques.yaml` | 264 |
| `commands.yaml` | 256 |
| `items.yaml` | 256 |
| `item_names.yaml` | 256 |
| `weekdays.yaml` | 6 |

Both versions have those counts. Duplicate offsets stay as separate entries,
so the indices still match what the code uses. The two item tables have compact
and spaced names in US, such as `MenosBronze` and `Menos Bronze`. JP has two
identical copies. Every string keeps its original bytes. US dictionary and
control codes stay in braces, like the other overlay exports.

## What stays in the byte map

| Region | Data blob | Zero-filled runtime area | Remaining gaps |
| --- | ---: | ---: | ---: |
| US | 360,596 bytes | 204,144 bytes | 188 bytes in five files |
| JP | 347,984 bytes | 191,824 bytes | 189 bytes in five files |

The byte map covers every byte, including padding and repeated words after
resources. The zero-filled runtime area isn't written out again. The gap
counts don't include undecoded data inside a known resource, such as the
animation tracks. Unknown bytes are preserved rather than assigned a guessed
format. Small array exports keep stored padding where the original array
length isn't established. Function-pointer tables retain their values and add
known symbol names; tables that mix pointers and resource ids keep both.

[`field.py`](../../../../tools/overlays/field.py) reads the resources and writes
the exports. [`field_tables.py`](../../../../tools/overlays/field_tables.py)
holds the small table layouts. Jump tables and other rodata before the code
stay with their existing owners. Nothing in this export feeds back into the
build.

Scene-specific assets aren't embedded here. Use the
[scene extractor](scene-extractor.md) for an `ANA/INFO_*/*.IMG` file.
