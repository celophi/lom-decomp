# GOSUB resources

[English index](../../README.md) | [Overlay extractors](../../../../tools/data/overlays/README.md) | [Japanese](../../../jp/technical/reference/gosub-resources.md)

GOSUB carries the workshop and companion-list screens: item selection, logic
blocks, pets, golems and eggs. Both regional YAMLs keep its artwork, text and
tables in one `gosub_data` databin. A small `gosub_groups` rodatabin holds the
equipment ranges before the code. The build links the original bytes, and a
separate extractor makes the contents readable.

```sh
make splat
make extract-gosub

make splat VERSION=jp
make extract-gosub VERSION=jp
```

Files go to `assets/exports/<version>/overlays/gosub/`. Set
`GOSUB_OUTPUT=/path/to/new-folder` to choose another destination. The folder
must be new. The extractor reads everything before writing, then moves its
temporary output into place. A failed write removes the temporary folder.

## What comes out

| Path | Contents |
| --- | --- |
| `byte-map.yaml` | Every range in the data and equipment-group blobs |
| `image/image.tim` | The original 256 x 256, 4-bit UI texture and its 16 palettes |
| `image/image.yaml` | Stored layout, palette words and glyph preview details |
| `image/palette_00.png` through `palette_0F.png` | The whole texture through each palette |
| `image/glyphs/` | 79 glyph previews, each showing all 16 palettes |
| `text/archive.bin` | The complete original text archive |
| `text/archive.yaml` | Its twelve section offsets |
| `text/*.yaml` | Decoded text, with indices, offsets and original codes |
| `portraits/archive.bin` | The original count, offset table and 84 portraits |
| `portraits/portraits.yaml` | Portrait indices, groups, offsets and palettes |
| `portraits/00.png` through `83.png` | The 48 x 48 portraits through their stored palettes |
| `font/image.tim`, `image.png` and `image.yaml` | The complete 64 x 16 font TIM, its preview and layout |
| `tables/glyph_metrics.yaml` | All 82 glyph records, including three empty entries |
| `tables/item_colors.yaml` | Color-name indices for the 37 color-material item ids |
| `tables/equipment_groups.yaml` | The first index and count for weapons, armor and instruments |

Each version produces 180 PNGs and 20 YAML files, plus the original resource
files. The UI texture and glyph records are exact copies of GOLEM's in both
regions. The portraits are a separate archive: indices 0 through 64 are pets,
65 through 71 are golems, and 72 through 83 are eggs.

## The text tables

Both regions have the same entry counts. Empty entries and repeated strings
keep their original indices.

| Section file | Entries |
| --- | ---: |
| `item_names.yaml` | 256 |
| `item_descriptions.yaml` | 256 |
| `equipment_types.yaml` | 32 |
| `spirit_names.yaml` | 256 |
| `spirit_descriptions.yaml` | 256 |
| `logic_block_names.yaml` | 58 |
| `logic_block_descriptions.yaml` | 58 |
| `messages.yaml` | 57 |
| `pet_species.yaml` | 65 |
| `golem_types.yaml` | 7 |
| `instrument_spells.yaml` | 112 |
| `color_names.yaml` | 33 |

Section names and message symbols come from
[`gosub_internal.h`](../../../../src/overlays/gosub/internal/gosub_internal.h).
Section offsets are relative to the archive; string offsets are relative to
their section. The original archive keeps the offset tables, terminators and
padding too.

JP text uses the same version's CLOAD character chart, which
`make splat VERSION=jp` also extracts. Its source is recorded in the byte map.
US dictionary and control codes stay in braces, as in the FIELD and CLOAD
exports. The equipment-group table selects eleven weapon types starting at
index 0, twelve armor types at 11, and four instrument types at 23.

## Looking at the images

The UI glyph records don't select a palette. The caller supplies one, so each
glyph PNG shows palettes 0 through 15 in a four-by-four grid, left to right,
then top to bottom. Records 0, 20 and 53 have zero dimensions and stay in the
YAML without producing empty PNGs. GOLEM and GOSUB use the same preview writer
in [`glyph_texture.py`](../../../../tools/data/overlays/glyph_texture.py).

PNGs show palette value zero as transparent and other colors as opaque.
Runtime tinting, blending and palette changes aren't applied. Original TIMs
and portrait data keep their palette words, including STP bits. The UI TIM
stores its pixels at (0, 0) and its palettes at (0, 480). GOSUB uploads the
pixels at (320, 0) and puts all 256 colors in one row at (0, 498).
The separate font strip is uploaded at (320, 240), with its palette at
(336, 255). It includes the panel-corner artwork used by the renderer.

## Where the old boundaries fall

Two symbols point inside resources. `g_gosub_portrait_archive` points at the
offset table, four bytes after its count of 84. The count belongs to the
portraits, even though the old split put it at the end of the text data.
The exported portrait archive starts at that count word, and its offsets are
relative to it.

`g_gosub_font_texture` points at the palette, twenty bytes into a TIM.
The upload reads 512 pixel bytes. Its last 92 bytes cross into the unused
prefix of `g_gosub_item_colors`, so cutting at that symbol would lose part of
the image. The export keeps the complete 576-byte TIM. The active color map
starts at item id 96 and ends at 132; those 37 bytes are exported separately.
The byte map counts each range once.

The data blob is 149,616 bytes in US and 149,228 bytes in JP. It ends at
`g_gosub_frame_parity`, where the existing BSS begins. Both versions keep
37,268 bytes of runtime variables and buffers with `gosub.c`. The equipment
rodata is 24 bytes in each version. Other rodata stays with its C unit.

Both exports account for every byte, including zero padding and repeated
words after the UI TIM and portrait archive. Neither needs an `unknown/`
folder. Unrecognized nonzero bytes would still be saved there unchanged.
[`gosub.py`](../../../../tools/data/overlays/gosub.py) follows the same resource
readers, `Part` records and writers as the other extractors. Editing an export
doesn't change what the build links.
