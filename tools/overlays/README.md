# Overlay resource extractors

[Tools index](../README.md) | [Text tables](../../docs/en/technical/reference/text-tables.md) | [Save file format](../../docs/en/technical/reference/save-file.md)

Some overlays keep their text, icons and small tables in one data blob. The
build links that blob as-is, which is great for matching the original but not
much help if you just want to see what's in it. This tool opens the blob up and
writes out files you can look at.

The extractors handle ADDHERO, the 2P hero screen; CARDA, the save and load
screen; CHECKPS, the startup screen and CD check; CLOAD, the load screen; and
FIELD, the resident field runtime; and GNAME, the name-entry screen.
Extract the version's assets with
`make splat` or `make splat VERSION=jp` first, then run:

```sh
make extract-addhero
make extract-addhero VERSION=jp
make extract-carda
make extract-carda VERSION=jp
make extract-checkps
make extract-checkps VERSION=jp
make extract-cload
make extract-cload VERSION=jp
make extract-field
make extract-field VERSION=jp
make extract-gname
make extract-gname VERSION=jp
```

`assets/` keeps two kinds of data apart. `assets/us/` and `assets/jp/` hold what
splat extracts for the build. `assets/exports/` holds data converted into
formats people can read. The output goes to
`assets/exports/<version>/overlays/<overlay>/`. Set `ADDHERO_OUTPUT`,
`CARDA_OUTPUT`, `CHECKPS_OUTPUT`, `CLOAD_OUTPUT`, `FIELD_OUTPUT` or `GNAME_OUTPUT` to pick another
folder. The destination must be new; the tool won't overwrite an existing folder.

## What you get

ADDHERO writes:

```text
addhero/
    byte-map.yaml          where everything came from in the blob
    text/
        messages.yaml      every message on the 2P screens, with its index and symbol
        locations.yaml     the location names shown next to each save
        card_titles.yaml   the memory card title templates, in Shift-JIS, with what each is for
        fixed_strings.yaml the number overflow text and the card path templates
    icons/
        icons.yaml         icon ids, what kind of character each is, original palettes
        icon_00.png ...    the 86 party icons, 48 x 48 each
    tables/
        card_steps.yaml    the memory card step sequences, named from the C code
        digit_glyphs.yaml  the full-width digits used for save numbers
        text_conversion.yaml   what each game text code becomes in Shift-JIS, plus the chart as stored
    unknown/               only created for bytes the tool does not recognize
```

CARDA writes the same kinds of files, plus item names and another set of icons:

```text
carda/
    text/items.yaml        the 256 item names used for Ring Ring Land rewards
    save_icons/
        icons.yaml         palettes and frame order for the memory card icons
        icon_00_0.png ...  ten icons, two 16 x 16 frames each
```

Its `text/messages.yaml` contains the save, load and PocketStation messages.
The `icons/` folder still holds the 86 party icons shown in the save browser;
`save_icons/` holds the smaller animated icons the console shows on a memory
card. Those are two separate sets in the blob.

CARDA's step table has several entry points inside other sequences. The export
follows each named entry to `CARDA_STEP_DONE`, so a shorter sequence can share
the end of a longer one. The byte map counts the table's bytes only once.

The card overlays' US text is shown as ASCII, with anything else written as a
code in braces.
`{16}` is a one-byte code, and `{1F 00}` is a code with its second byte.
Japanese text is decoded with the game's own character chart, so it reads as
Japanese. The raw bytes are next to each string too, so nothing is lost. The
[text tables](../../docs/en/technical/reference/text-tables.md) page explains
what the codes mean.

The icon PNGs use the palette stored with each icon, and palette value 0 is
transparent, like on the PlayStation. The guest hero and the golems are drawn
with palettes the game builds at runtime, so the golem icons all look the same
here.

CARDA and ADDHERO end with their own variables and buffers. They're all zeros on
the disc, so the tools list that range in `byte-map.yaml` and skip exporting
it. Any bytes the tools don't recognize are saved in `unknown/`, named by
address. The byte map covers the whole blob, including padding and the repeated
words after the icon sets.

CHECKPS writes:

```text
checkps/
    byte-map.yaml          every range in the data blob and warning rodata
    image/
        image.tim          the original TIM, without its trailing bytes
        image.yaml         image dimensions, stored layout and palettes
        palette_00.png ... the texture rendered once for each stored palette
    audio/
        container.yaml     section offsets and program header
        program.akao       the resident program data, unchanged
        bank.akao          the uploadable instrument bank, unchanged
        bank.yaml          bank header and articulation entries
        samples.adpcm      the bank's original SPU sample data
    text/warning.yaml      the Japanese warning, decoded with its original bytes
    tables/
        cd_commands.yaml   command opcodes, transfer counts and IRQ sums
        cd_registers.yaml  the four CD register pointers
        cd_state.yaml      initial buffers and check state
        pattern_sizes.yaml the warning pattern's width/height pairs
        pattern_signs.yaml the signs that reflect it into four quadrants
        digit_glyphs.yaml  Shift-JIS decimal and hexadecimal glyphs
        glyph_clut.yaml    the six stored colors at the start of the glyph CLUT
    unknown/               trailing bytes whose purpose is still unknown
```

US has a 256 x 48 image and one palette. JP has a 256 x 256 texture and 16
palettes. Each PNG shows the whole stored texture through one palette; the JP
animation chooses parts of that texture at runtime. Palette value zero is
transparent, and the other colors are shown opaque. The TIM and YAML keep the
original palette words, including their transparency flags.

The audio export describes the container and its 32 articulation entries.
Program bytecode and ADPCM samples stay in their original formats; the tool
doesn't synthesize sound effects. The warning's actual glyph bitmaps come from
the console's BIOS Kanji ROM, so they aren't embedded in CHECKPS.

CHECKPS's `checkps_data` blob ends where BSS begins. Its runtime buffers remain
with the C or assembly units that define them. The warning and quadrant signs
stay in a small `rodatabin` before the code. Both extracted files have complete
byte maps. There are eight unidentified trailing bytes in US and twelve in JP;
those bytes are saved in `unknown/`.

CLOAD writes:

```text
cload/
    byte-map.yaml          every range in the original data blob
    text/
        messages.yaml      the card-screen messages, with codes and raw bytes
        locations.yaml     the 63 location entries selected by music track
    tables/
        card_steps.yaml    the six load sequences, named from the C enum
        digit_glyphs.yaml  full-width decimal and hexadecimal digits
        text_conversion.yaml   the character chart and its Shift-JIS mappings
```

It carries the same message and location tables as ADDHERO and CARDA: 91
messages in US, 90 in JP. This includes messages CLOAD doesn't use. The party
icons come from CD resource `0x5E4`, loaded by `cload_load_icon_resources`, so
there is no embedded icon set to export here. The card path strings remain
with the C code that defines them.

Both CLOAD YAMLs already use one `databin`. Its final 149,920 bytes are
zero-filled variables and buffers, recorded in the byte map without writing
another file. All remaining gaps are zero padding in both versions; neither
export needs an `unknown/` folder. Unrecognized nonzero bytes would still be
saved there. See the [CLOAD resource notes](../../docs/en/technical/reference/cload-resources.md)
([Japanese](../../docs/jp/technical/reference/cload-resources.md)) for the layout.

FIELD writes:

```text
field/
    byte-map.yaml          all data ranges, plus the source of the JP character chart
    images/
        common/            original TIM and 64 palette previews of the common texture
        transition/        original 16-bit TIM containing both fade tiles, plus a PNG
        menu_frame/        original frame data and previews through both palettes
    palettes/              golem and portrait palettes, their headers and color swatches
    text/                  UI messages, techniques, commands, item names, weekdays and save filenames
    animations/            built-in animation directory and definitions, plus the original resource
    actions/               four action descriptor tables and the original bank
    tables/                HUD, input, progression, script dispatch, window and golem tables
    unknown/               gaps the readers haven't identified, preserved by RAM address
```

The common texture is 512 x 256, with 64 stored 16-color palettes. Each PNG
shows the whole texture through one palette; the game chooses palettes for
individual sprites. The transition TIM holds two 32 x 32 tiles stacked
vertically. These are embedded FIELD resources. Scene-specific images and
scripts still come from scene IMGs and use `make extract-scene`.

The text export includes two item-name tables. US uses compact names in one
and spaced names in the other; JP carries identical copies. JP text decoding
also reads `cload_data.databin.bin` and its symbol/config files from the same
version. `make splat VERSION=jp` extracts both blobs. The byte map records
CLOAD as the chart source, so it isn't mistaken for data embedded in FIELD.

The animation directory and its 202 definitions are decoded, but the part,
curve and frame payload stays in the original resource. There is no animation
player here. Dispatch tables keep their numeric values and attach symbol names
when an address is known; some tables mix function pointers and resource ids.

Both versions have five gaps saved under `unknown/`: 188 bytes in US and 189
in JP. That counts the gaps, not the undecoded payload inside a known resource.
The runtime variables are all zero on disc: 204,144 bytes in US and 191,824 in
JP, listed in the byte map without another output file. More details are in
[FIELD resources](../../docs/en/technical/reference/field-resources.md)
([Japanese](../../docs/jp/technical/reference/field-resources.md)).

GNAME writes:

```text
gname/
    byte-map.yaml          every data range, plus the source of the JP character chart
    image/
        image.tim          original 256 x 256 texture
        image.yaml         stored layout, palette words and sprite coordinates
        palette_00.png ... whole texture through each of its 16 palettes
        glyphs/00.png ...  39 UI sprites, each cropped with its own palette
    text/
        resource.bin       original name-record archive
        resource.yaml      header and table offsets
        panel_records.yaml labels, help text and selectable characters
        kanji_records.yaml the full 2,698-entry kanji table
        history_names.yaml 256 history-name records
        random_names.yaml  two sets of 128 random names
    tables/                panel boundaries, category map, glyph metrics, cursors,
                           background sprite positions and append animation frames
```

US has 138 panel records; JP has 498. Both carry the same kanji table, though
US has only ten category slots and all are unmapped. US kanji entries stay as
glyph codes because its character chart lacks those mappings. JP decodes text
through the same version's CLOAD blob, like FIELD. The chart's source is listed
in the byte map. English dictionary fragments in panel help text are expanded.
All text keeps its original bytes, including terminators and padding.

The cropped PNGs show each UI sprite with its stored palette. They don't apply
runtime shadows or tinting. The full-texture previews use one palette each;
color zero is transparent and other colors are opaque. The TIM stays unchanged.

The blob ends where GNAME's existing BSS begins. US has four padding bytes
after the append animation; JP has none. Both exports account for every byte
without needing an `unknown/` folder. See [GNAME resources](../../docs/en/technical/reference/gname-resources.md)
([Japanese](../../docs/jp/technical/reference/gname-resources.md)) for the layout
and the difference between stored and runtime texture placement.

## How it's put together

`addhero.py`, `carda.py`, `checkps.py`, `cload.py`, `field.py` and `gname.py` use one reader
per resource format. Each reader parses its bytes into a small dataclass and
returns a `Part` that says where the bytes are and what they hold. `read_blob`
collects the parts in address order, and `cover_gaps` fills whatever lies
between them. A `write_part` function for each kind of content writes its file.
Nothing is written until everything has been read, and the output goes into a
temporary folder that's moved into place at the end, so a failed run leaves
nothing behind.

Each tool finds its input files through the overlay's splat config, and every
address through the version's symbol file. The YAML has one `databin` for the
data after the code. Short strings before the code stay with their existing
C definitions or `rodatabin` files.
The export is a readable copy; editing it doesn't change what the build links.

Common changes:

| If you... | Change |
|---|---|
| rename a symbol the tool uses | its entry in `SYMBOL_NAMES` in the overlay's Python module |
| change a value the tool copies from C | the matching constant in `card_data.py` or the overlay's module |
| find out what a new part of the blob is | add a dataclass, a `read_*` function called from `read_blob`, and a `write_part` writer |
| add or rename a card step | nothing; the names are read from the overlay's C source or internal header |

`card_data.py` holds the character chart, decoded-content dataclasses, common
readers and writers used by the card overlays. `resources.py` supplies the
blob, part, byte-map and output helpers all six extractors share. The smaller
modules (`text_table.py`, `icon_set.py`, `png.py`, `symbols.py` and
`splat_config.py`) handle the underlying file formats.
`field_tables.py` lists FIELD's small array layouts; its larger resources have
separate readers in `field.py`. GNAME reuses the typed format readers in
`tools/assets/` for its tables, name-record archive and TIM.

## Tests

```sh
make test-overlay-tools
```

`test_addhero_sources.py` checks the tool against this repository's config and
C sources: every symbol it needs exists in both versions, and every constant it
copies still matches its `#define`. If you rename something in C and forget the
tool, this is the test that tells you what to update. `test_addhero_extract.py`
runs the whole extractor on a small made-up overlay. None of the tests read game
files.

The CARDA tests check both regional configs and run a synthetic overlay through
the exporter. They cover Japanese character codes, both icon frames, sequence
entry points, complete byte coverage, and cleanup after a failed write.

The CHECKPS tests exercise both palettes of a synthetic TIM, the AKAO section
boundaries, decoded warning and tables, and complete byte coverage of both
input files. They also check that truncated or invalid resources leave no
partial export behind. Source tests keep the regional layouts and copied C
constants in step with the extractor.

The CLOAD tests cover both regional configs, the chart base used by the C
text decoder, complete byte coverage, shared sequence tails and Japanese codes
with a zero second byte. Missing inputs, bad boundaries and failed writes must
leave no partial output.

The FIELD tests check PNG pixels, TIM boundaries, Japanese decoding through
CLOAD, animation and action offsets, palette headers, complete byte coverage
and failed-write cleanup. Source tests check both regional layouts and the
counts copied from C. Like the other extractor tests, they use made-up data.

The GNAME tests cover both regional layouts, dictionary text, JP chart decoding,
palette previews, sprite crops, signed layout coordinates and complete byte
coverage. Bad resource offsets, invalid sprite bounds, missing inputs and
failed writes must leave no partial export.
