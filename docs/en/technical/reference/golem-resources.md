# GOLEM resources

[English index](../../README.md) | [Overlay extractors](../../../../tools/overlays/README.md) | [Japanese](../../../jp/technical/reference/golem-resources.md)

GOLEM is the logic-block grid editor. It carries the editor artwork, block
names and descriptions, and the records that put the screen together.
Both regional YAMLs keep these in one `golem_data` databin. The build links
those bytes unchanged, and a separate extractor makes them readable.

```sh
make splat
make extract-golem

make splat VERSION=jp
make extract-golem VERSION=jp
```

Files go to `assets/exports/<version>/overlays/golem/`. Set
`GOLEM_OUTPUT=/path/to/new-folder` to use another destination. The folder must
be new. The extractor reads everything first, then writes a temporary folder
and moves it into place when it's finished. A failed write removes that folder.

## What comes out

| Path | Contents |
| --- | --- |
| `byte-map.yaml` | Every range in the data blob, including the repeated TIM word and editor state |
| `image/image.tim` | The original 256 x 256, 4-bit texture and all 16 palettes |
| `image/image.yaml` | Stored image layout, palette words and glyph preview details |
| `image/palette_00.png` through `palette_0F.png` | The whole texture shown through each palette |
| `image/glyphs/` | 79 glyph previews, each showing its source rectangle through all 16 palettes |
| `text/archive.bin` | The complete original text archive |
| `text/archive.yaml` | Its two section offsets |
| `text/names.yaml` | 58 logic-block names, with indices and original codes |
| `text/descriptions.yaml` | 58 descriptions, indexed the same way as the names |
| `tables/glyph_metrics.yaml` | All 82 glyph records, including the three empty entries |
| `tables/panels.yaml` | All 154 panel records, with decoded fields and their original bytes |

The texture, glyph metrics and panel records are identical in US and JP.
The text archive is different. Both versions still have 58 entries in each
text table, including empty entries. The export keeps every index and
section-relative string offset, so these line up with the logic-block ids.

JP text uses the same version's CLOAD character chart. `make splat VERSION=jp`
extracts that blob too, and `byte-map.yaml` records where the chart came from.
US dictionary and control codes stay in braces, as in the FIELD and CLOAD
exports. The original archive keeps the offset tables, terminators and padding.

## Looking at the images

GOLEM's glyph records don't select a palette. The caller supplies one when it
draws a glyph, so choosing one for the export would hide the other uses.
Each glyph PNG has a four-by-four grid: palettes 0 through 15, left to right,
then top to bottom. For a 16 x 16 glyph, that makes a 64 x 64 preview.
Records 0, 20 and 53 have zero dimensions. They stay in the YAML but don't
produce empty PNGs.

The previews show the source colors, with value zero transparent and other
colors opaque. Runtime tinting, blending and backing tiles aren't applied.
The original TIM keeps its palette words, including STP bits. Its stored
coordinates are (0, 0) for the image and (0, 480) for a 16 x 16 palette block.
GOLEM uploads the image at (320, 0) and flattens the 256 palette colors into
one row at (0, 498).

A complete logic-block icon also uses a shape layout from FIELD and values
from the saved game. Those shapes aren't embedded in this blob. The
[FIELD extractor](field-resources.md) exports them as `tables/golem_shapes.yaml`.

## Panel records and runtime state

A panel record is 20 bytes. Its packed words select the texture cell, palette,
blend mode, visibility behavior and flash timer. The export also shows the
screen rectangle separately from the source cell size. The renderer repeats
that cell to fill the rectangle and adds eight pixels to the stored X position.
The cell height crosses a word boundary, so both pieces are combined in the
YAML. Original words and reserved fields remain available beside the decoded
values. Behavior names come from the C enum in `golem.c`.

The data blob is 39,440 bytes in US and 39,000 bytes in JP. Both include 292
zero-filled bytes for editor variables and buffers. The byte map records this
area without writing another file. Every byte is accounted for, so neither
version needs an `unknown/` folder. Unrecognized nonzero data would still be
saved there unchanged.

[`golem.py`](../../../../tools/overlays/golem.py) follows the same reader, `Part`
and writer structure as the other overlay extractors. The rodata before the
code stays with its existing C unit. Editing these exports doesn't change
what the build links.
