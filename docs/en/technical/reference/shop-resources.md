# SHOP resources

[English index](../../README.md) | [Overlay extractors](../../../../tools/overlays/README.md) | [Japanese](../../../jp/technical/reference/shop-resources.md)

SHOP is the buy and sell screen that field shops open. Its data is small: the
text for the item list and the detail window, the price the shop pays for each
kind of item, and the shop's own variables. Both regional YAMLs keep all of it
in one `shop_data` databin. The build links those bytes unchanged, and a
separate extractor makes them readable.

```sh
make splat
make extract-shop

make splat VERSION=jp
make extract-shop VERSION=jp
```

Files go to `assets/exports/<version>/overlays/shop/`. Set
`SHOP_OUTPUT=/path/to/new-folder` to use another destination. The folder must
be new. The extractor reads everything first, then writes a temporary folder
and moves it into place when it's finished. A failed write removes that folder.

## What comes out

| Path | Contents |
| --- | --- |
| `byte-map.yaml` | Every range in the data blob, including padding and the shop's variables |
| `text/archive.bin` | The complete original text archive |
| `text/archive.yaml` | Its four section offsets and the file for each section |
| `text/item_descriptions.yaml` | 256 item class lines, one per item kind |
| `text/item_names.yaml` | 256 item names, one per item kind |
| `text/equipment_types.yaml` | 32 weapon, armor and instrument type names |
| `text/instrument_spells.yaml` | 112 instrument spell names, 14 for each of 8 spirits |
| `tables/sell_prices.yaml` | The sell price of each item kind, with its name |

Every text entry keeps its index, its offset inside the section, the decoded
text and the original bytes. JP text uses the same version's CLOAD character
chart. `make splat VERSION=jp` extracts that blob too, and `byte-map.yaml`
records where the chart came from. US dictionary and control codes stay in
braces, as in the other overlay exports. Zero bytes after a section's last
string are listed as `padding` in that section's YAML.

## Where things are

SHOP loads at `0x80140000`, so an address minus `0x80140000` is its offset in
the decompressed SHOP.BIN. The [text tables](text-tables.md) page shows how to
decompress an overlay.

| Range | US address | JP address | Size |
| --- | --- | --- | --- |
| Text archive | `0x80142D04` | `0x80142CF8` | `0x22B0` US, `0x1E44` JP |
| Sell prices | `0x80144FB4` | `0x80144B3C` | `0x200` |
| Zero padding | `0x801451B4` | `0x80144D3C` | 4 bytes |
| Shop variables | `0x801451B8` | `0x80144D40` | `0xB2C` |

The whole blob is 12,256 bytes in US and 11,124 bytes in JP. The last 2,860
bytes are the shop's variables and its list buffer. They're all zero on the
disc, so the byte map lists them without writing another file. Every other
byte belongs to the archive, the price table or the padding between them, so
neither version needs an `unknown/` folder.

## The text archive

The archive starts with a 20-byte header: a 16-bit section count (always 4), a
16-bit zero, then four 32-bit offsets counted from the start of the archive.
Each section is an ordinary text table, with 16-bit string offsets counted from
the start of that section.

| Section | Symbol | Entries | What the shop does with it |
| --- | --- | --- | --- |
| 0 | `SHOP_TEXT_ITEM_DESCRIPTIONS` | 256 | Detail window title when the cursor is on a plain item |
| 1 | `SHOP_TEXT_ITEM_NAMES` | 256 | List rows for plain items, and the material part of an equipment name |
| 2 | `SHOP_TEXT_EQUIPMENT_TYPES` | 32 | The type part of an equipment name |
| 3 | `SHOP_TEXT_INSTRUMENT_SPELLS` | 112 | The spell shown in an instrument's detail window |

Sections 0 and 1 are indexed by item kind, the same number the inventory counts
use. The first 64 item names double as materials: an equipment record keeps its
material in six bits, and the shop builds its name as the material name, a
separator from FIELD's UI strings, then the type name. The US separator is one
space, so a record made of item 0 with weapon type 0 shows as
"MenosBronze Knife" in the US release.

The equipment types are one table for three categories. Weapons start at index
0, armor at 11 and instruments at 23, and the record's type number is added to
that start. The US and JP tables both have 11 weapons, 12 armor types and 4
instruments; entries 27 to 31 are empty. The export writes the category and
type number next to each entry.

The spell section is a grid. An instrument record stores a spirit number and a
spell number, and the shop reads entry `spirit * 14 + spell`. The export shows
both numbers next to each name. In US, the last spell in each row starts with
the spirit's name (`Wisp{18}Blaze`, `Shade{18}Scythe` and so on), which is a
handy way to tell the rows apart.

The item descriptions look odd in a hex editor because they line up their
second column with spacing codes. In US, `{08}` is two spaces, `{09}` three,
`{0A}` four, and `{0B}` takes the next byte as a count, so
`METAL{08}Primary{0B}{0F}` is "METAL", two spaces, "Primary" and fifteen spaces.
FIELD's text renderer in `field_text.c` defines these codes.

If you edit the archive, keep every entry count the same; the shop finds text
by index. The price table starts right after the archive, so the archive can't
grow past its current size unless everything after it moves. A longer string
needs the later string offsets in its section updated, and a section that grows
needs the section offsets after it updated too.

## Sell prices

The price table is 256 little-endian 16-bit values, one per item kind. When
the player sells, SHOP lists each item kind they hold and offers this price
for each one. The list multiplies it by the chosen quantity. Equipment records
aren't in this table; each record carries its own value.

Buy prices aren't here either. The field script that opens a shop passes SHOP
the stock to show, and every entry in that stock has its own price.

[`shop.py`](../../../../tools/overlays/shop.py) follows the same reader, `Part`
and writer structure as the other overlay extractors. Section names come from
the `ShopTextSection` enum in `shop_internal.h`. Editing these exports doesn't
change what the build links.

## What we don't know yet

- TODO: some item kinds look unused. Kinds 151 to 159, for example, have empty
  descriptions and a repeated neighbor's name in US. JP gives them placeholder
  names that read "item" plus the kind number in hexadecimal, so kind 151 is
  "item 97". We haven't checked whether any stock list or inventory can contain
  them.
- TODO: the shop's list buffer holds 336 entries, while a sell list could
  collect up to 100 equipment records plus 256 item kinds. We haven't traced
  what keeps the list inside the buffer in practice.
