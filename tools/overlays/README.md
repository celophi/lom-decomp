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

## How it finds things

The tool looks up table addresses in the version's symbol file
(`config/<version>/symbols/addhero_symbol_addrs.txt`), so the US and Japanese
blobs work the same way. The card step names are read from
`src/overlays/addhero/addhero_internal.h`, so they always match the code.

Run the tests with:

```sh
make test-overlay-tools
```

They use made-up data, not game files.
