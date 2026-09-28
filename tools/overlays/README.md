# Overlay resource extractor

[Tools index](../README.md) | [Text tables](../../docs/en/technical/reference/text-tables.md) | [Save file format](../../docs/en/technical/reference/save-file.md)

Some overlays keep their text, icons and small tables in one data blob. The
build links that blob as-is, which is great for matching the original but not
much help if you just want to see what's in it. This tool opens the blob up and
writes out files you can look at.

Right now it handles ADDHERO, the 2P hero screen. Extract the version's assets
with `make splat` first, then run:

```sh
make extract-addhero
make extract-addhero VERSION=jp
```

`assets/` keeps two kinds of data apart. `assets/us/` and `assets/jp/` hold what
splat extracts for the build. `assets/exports/` holds data converted into
formats people can read, so this tool writes to
`assets/exports/us/overlays/addhero/` (or `jp`). Set `ADDHERO_OUTPUT` to pick
another folder. The destination must be new; the tool won't overwrite an
existing folder.

## What you get

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
    unknown/               only if some bytes aren't recognized (none are, at the moment)
```

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

The last 53 KB of the blob is ADDHERO's own variables and buffers. It's all
zeros on the disc, so the tool lists it in `byte-map.yaml` and skips it. Any
bytes the tool doesn't recognize are saved in `unknown/`, named by address.

## How it's put together

`addhero.py` reads the blob the way you'd read its byte map, top to bottom.
`read_blob` calls one `read_*` function per part, in address order. Each one
parses its bytes into a small dataclass and returns a `Part` that says where the
bytes are and what they hold. `cover_gaps` then fills whatever lies between the
parts, and a `write_part` function for each kind of content writes its file.
Nothing is written until everything has been read, and the output goes into a
temporary folder that's moved into place at the end, so a failed run leaves
nothing behind.

The tool never hard-codes an address or a file name. It finds the blob and the
two small data files through the overlay's splat config, and every address
through the version's symbol file.

Common changes:

| If you... | Change |
|---|---|
| rename a symbol the tool uses | its entry in `SYMBOL_NAMES` in `addhero.py` |
| change a value the tool copies from C | the matching constant at the top of `addhero.py` |
| find out what a new part of the blob is | add a dataclass, a `read_*` function called from `read_blob`, and a `write_part` writer |
| add or rename a card step | nothing; the names are read from `addhero_internal.h` |

The shared readers (`text_table.py`, `icon_set.py`, `png.py`, `symbols.py` and
`splat_config.py`) don't know anything about ADDHERO, so the other card
overlays can use them too.

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
