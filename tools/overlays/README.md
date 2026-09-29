# Overlay resource extractors

[Tools index](../README.md) | [Text tables](../../docs/en/technical/reference/text-tables.md) | [Save file format](../../docs/en/technical/reference/save-file.md)

Some overlays keep their text, icons and small tables in one data blob. The
build links that blob as-is, which is great for matching the original but not
much help if you just want to see what's in it. This tool opens the blob up and
writes out files you can look at.

The extractors handle ADDHERO, the 2P hero screen, and CARDA, the save and load
screen. Extract the version's assets with `make splat` or
`make splat VERSION=jp` first, then run:

```sh
make extract-addhero
make extract-addhero VERSION=jp
make extract-carda
make extract-carda VERSION=jp
```

`assets/` keeps two kinds of data apart. `assets/us/` and `assets/jp/` hold what
splat extracts for the build. `assets/exports/` holds data converted into
formats people can read. The output goes to
`assets/exports/<version>/overlays/addhero/` or
`assets/exports/<version>/overlays/carda/`. Set `ADDHERO_OUTPUT` or `CARDA_OUTPUT`
to pick another folder. The destination must be new; the tool won't overwrite an
existing folder.

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

US text is shown as ASCII, with anything else written as a code in braces.
`{16}` is a one-byte code, and `{1F 00}` is a code with its second byte.
Japanese text is decoded with the game's own character chart, so it reads as
Japanese. The raw bytes are next to each string too, so nothing is lost. The
[text tables](../../docs/en/technical/reference/text-tables.md) page explains
what the codes mean.

The icon PNGs use the palette stored with each icon, and palette value 0 is
transparent, like on the PlayStation. The guest hero and the golems are drawn
with palettes the game builds at runtime, so the golem icons all look the same
here.

Both blobs end with the overlay's variables and buffers. They're all zeros on
the disc, so the tools list that range in `byte-map.yaml` and skip exporting
it. Any bytes the tools don't recognize are saved in `unknown/`, named by
address. The byte map covers the whole blob, including padding and the repeated
words after the icon sets.

## How it's put together

`addhero.py` and `carda.py` read each blob the way you'd read its byte map,
top to bottom.
`read_blob` calls one `read_*` function per part, in address order. Each one
parses its bytes into a small dataclass and returns a `Part` that says where the
bytes are and what they hold. `cover_gaps` then fills whatever lies between the
parts, and a `write_part` function for each kind of content writes its file.
Nothing is written until everything has been read, and the output goes into a
temporary folder that's moved into place at the end, so a failed run leaves
nothing behind.

Each tool finds its input files through the overlay's splat config, and every
address through the version's symbol file. The YAML has one `databin` for the
data after the code. Short strings before the code remain in `rodatabin` files.
The export is a readable copy; editing it doesn't change what the build links.

Common changes:

| If you... | Change |
|---|---|
| rename a symbol the tool uses | its entry in `SYMBOL_NAMES` in the overlay's Python module |
| change a value the tool copies from C | the matching constant in `card_data.py` or the overlay's module |
| find out what a new part of the blob is | add a dataclass, a `read_*` function called from `read_blob`, and a `write_part` writer |
| add or rename a card step | nothing; the names are read from the overlay's internal header |

`card_data.py` holds the character chart, decoded-content dataclasses, common
readers and writers. Both overlays use it for text, party icons, digit glyphs
and byte maps. The smaller modules (`text_table.py`, `icon_set.py`, `png.py`,
`symbols.py` and `splat_config.py`) handle the underlying file formats.

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
