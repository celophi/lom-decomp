# GNAME resources

[English index](../../README.md) | [Overlay extractors](../../../../tools/data/overlays/README.md) | [Japanese](../../../jp/technical/reference/gname-resources.md)

GNAME carries the name-entry screen's artwork, character panels and name lists.
The old YAML split these into several asset types. Both regions now use one
`gname_data` databin, with a separate extractor to make the contents readable.
The build still links the original bytes.

```sh
make splat
make extract-gname

make splat VERSION=jp
make extract-gname VERSION=jp
```

The files go to `assets/exports/<version>/overlays/gname/`. Use
`GNAME_OUTPUT=/path/to/new-folder` to put them somewhere else. The destination
must be new. Everything is read before writing starts, and a failed write
removes the temporary output folder.

## What's in it

| Path | Contents |
| --- | --- |
| `byte-map.yaml` | Every range in the data blob, including padding and the TIM's repeated last word |
| `image/image.tim` | The original 256 x 256, 4-bit texture |
| `image/image.yaml` | Stored TIM layout, palette words and sprite coordinates |
| `image/palette_00.png` through `palette_0F.png` | The whole texture shown through each of its 16 palettes |
| `image/glyphs/00.png` through `38.png` | The 39 UI sprites, cropped with their own palette |
| `text/resource.bin` | The complete original name-record archive |
| `text/resource.yaml` | Its header and the offsets of its four tables |
| `text/*.yaml` | Character panels, kanji, history names and random names, with indices and original bytes |
| `tables/` | Panel boundaries, category map, glyph metrics, tab cursors, background sprite positions and append animation frames |

The full-texture previews are useful for finding artwork, but each sprite uses
its own palette in the game. The cropped images use that palette directly.
They show the source artwork; shadows, tinting and the rest of the screen's
runtime composition aren't applied.

The TIM stores its image at (0, 0) and its palettes at (0, 480), in a 16 x 16
block. GNAME actually uploads the image at (320, 0) and all 256 palette colors
in one row at (0, 498). It also sets the STP bit on every nonzero color. The
exported TIM keeps the words and coordinates as stored. PNGs show color zero
as transparent and other colors as opaque.

## The name tables

| Table | US records | JP records |
| --- | ---: | ---: |
| `panel_records.yaml` | 138 | 498 |
| `kanji_records.yaml` | 2,698 | 2,698 |
| `history_names.yaml` | 256 | 256 |
| `random_names.yaml` | 256 | 256 |

The panel table includes labels and help text as well as selectable characters.
The random-name table holds two sets of 128 names. All entries keep their
original index, table-relative offset and bytes, including terminators and
padding. Repeated strings and empty entries stay in place.

US still carries the entire Japanese kanji table, identical to JP's copy.
Its category map has ten entries, all unmapped. JP has fifty category slots,
with six empty slots and 44 groups of kanji. The 45 boundaries describe those
44 groups in both versions.

JP text is decoded with the same version's CLOAD character chart, which
`make splat VERSION=jp` also extracts. The byte map records that dependency;
GNAME doesn't contain the chart itself. US text stays in its English encoding,
with dictionary fragments expanded in the panel help text. The US chart lacks
the retained kanji mappings, so those entries remain visible as glyph codes,
for example `{1D D4}`. A zero second byte belongs to its glyph, not the string
terminator.

## Data and runtime storage

The databin is 51,992 bytes in US and 53,428 bytes in JP. It ends at
`g_custom_name_buf`, where the existing BSS section begins. Those runtime
buffers remain with `gname.c`: 296 bytes in US and 292 in JP.

Both blobs are fully accounted for. US has four zero padding bytes after the
seven-frame append animation; JP ends immediately after it. Neither export
needs an `unknown/` folder. If a reader leaves nonzero bytes it doesn't
recognize, they are saved there unchanged.

[`gname.py`](../../../../tools/data/overlays/gname.py) follows the other overlay
extractors: symbol-based boundaries, small resource readers and a complete
byte map. It reuses the format parsers in `tools/data/formats/`. The animation YAML
keeps all three sprite slots per frame. A zero glyph hides a slot, and only
the first slot's control byte gives the frame duration in render ticks.
Editing an export doesn't change what the build links.
