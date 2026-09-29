# CLOAD resources

[English index](../../README.md) | [Overlay extractors](../../../../tools/overlays/README.md) | [Japanese](../../../jp/technical/reference/cload-resources.md)

CLOAD keeps its messages and small tables in one data blob. The build already
links it that way in both versions. To see what's inside, extract the splat
assets first, then run:

```sh
make splat
make extract-cload

make splat VERSION=jp
make extract-cload VERSION=jp
```

The output goes to `assets/exports/<version>/overlays/cload/`. Use
`CLOAD_OUTPUT=/path/to/new-folder` to choose another folder. The extractor won't
overwrite an existing export. These files are for inspection; the build still
reads the original blob.

## What's in the blob

| Export | Contents |
| --- | --- |
| `text/messages.yaml` | 91 US or 90 JP messages, with table indices, known symbols and raw bytes |
| `text/locations.yaml` | 63 location entries, selected by the music track stored in the save |
| `tables/card_steps.yaml` | Six card sequences, with names from `CloadLoadStep` |
| `tables/text_conversion.yaml` | The character chart and its Shift-JIS mappings |
| `tables/digit_glyphs.yaml` | Full-width decimal and hexadecimal digits |
| `byte-map.yaml` | Every byte range, including padding and runtime buffers |

The messages and locations are copies of the tables in ADDHERO and CARDA.
That means the message export also contains text CLOAD doesn't use. US text
keeps dictionary and control codes in braces; JP text is decoded through
CLOAD's own chart. Both keep the original bytes beside the text.

The final 149,920 bytes are CLOAD's variables and buffers, all zero on disc.
They stay in the byte map without becoming another output file. The gaps
between resources are also zero-filled in both releases, so these exports have
no unknown data. If nonzero bytes turn up in an unrecognized range, the tool
preserves them under `unknown/`.

## Where are the icons?

CLOAD loads its party icons from CD resource `0x5E4` through
[`cload_load_icon_resources`](../../../../src/overlays/cload/cload_widgets.c).
They aren't embedded in this blob, so this extractor doesn't write PNGs.
There are no memory card title templates in the blob either. The fixed card
path strings are defined in
[`cload_card.c`](../../../../src/overlays/cload/cload_card.c).

[`cload.py`](../../../../tools/overlays/cload.py) reads the blob in address
order, using the shared card-format readers. It gets addresses from each
version's symbol file and step names from the C enum. The tests also check
that its two-byte chart base agrees with
[`cload_glyph.c`](../../../../src/overlays/cload/cload_glyph.c).
