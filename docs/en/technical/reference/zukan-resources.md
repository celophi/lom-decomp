# ZUKAN resources

[English index](../../README.md) | [Overlay extractors](../../../../tools/data/overlays/README.md) | [Japanese](../../../jp/technical/reference/zukan-resources.md)

ZUKAN is the encyclopedia screen: the category lists and the entry pages for
lands, artifacts, monsters, characters, world history and the rest. Both
regional YAMLs keep its artwork, names and sprite layout in one `zukan_data`
databin after the code. The tables that decide which entries each category
lists sit before the code in one `zukan_entry_tables` rodatabin. The build
links the original bytes, and a separate extractor makes them readable.

```sh
make splat
make extract-zukan

make splat VERSION=jp
make extract-zukan VERSION=jp
```

Files go to `assets/exports/<version>/overlays/zukan/`. Set
`ZUKAN_OUTPUT=/path/to/new-folder` to choose another destination. The folder
must be new. The extractor reads everything before writing, then moves its
temporary output into place. A failed write removes the temporary folder.

## What comes out

| Path | Contents |
| --- | --- |
| `byte-map.yaml` | Every range in the data blob and the entry-table rodata |
| `archive/archive.yaml` | The archive header: four section offsets and what each holds |
| `images/page/image.tim` | The original 256 x 256, 4-bit page texture |
| `images/border/image.tim` | The original 256 x 256, 4-bit border texture |
| `images/*/image.yaml` | Stored layout, VRAM upload position and stored palette words |
| `images/*/palette_00.png` through `palette_05.png` | Each texture through the six palettes the screen uses |
| `images/sprites/sprites.yaml` | The 21 fixed UI sprites, decoded next to their original words |
| `images/sprites/00.png` through `20.png` | Each sprite cropped from its texture |
| `text/entry_names.yaml` | 1,014 entry names, with offsets and original codes |
| `text/category_names.yaml` | The 12 category titles |
| `tables/categories.yaml` | Where each category starts and ends in the entry list |
| `tables/history_groups.yaml` | The six World History groups and the bits that unlock them |
| `tables/entries.yaml` | 1,018 entries: name, category and the CD resource of its page |
| `tables/display_order.yaml` | The order entries are listed in, 718 positions |

Each version produces 33 PNGs and 11 YAML files, plus the two TIMs. Entry and
category counts are the same in both regions.

## The archive

`g_zukan_resource_archive` starts with a count of 4 and four offsets, all
relative to the start of the archive:

| Section | Offset | Contents |
| ---: | --- | --- |
| 0 | `0x14` | Page texture, uploaded at (320, 0) |
| 1 | `0x8234` | Border texture, uploaded at (832, 256) |
| 2 | `0x10454` | Entry names |
| 3 | US `0x12C70`, JP `0x123E0` | Category titles |

The page texture holds the parchment page, its edges and the PREV / NEXT / END
buttons. The border texture holds the decorated frame around the page. Both
TIMs upload all 256 colors of their CLUT to the same row at (0, 498), and the
page texture goes second. So the border's own palettes never reach the screen,
and every sprite is drawn through the page texture's palettes. The previews
use those colors. Only palettes 0 through 5 have any colors; the empty ones
get no PNG. The TIM files keep the original palette words anyway, border
palettes included.

The two text sections use the usual u16 offset tables (see
[text tables](text-tables.md)). Entry name 0 is a row of dashes, and 274 US
names and 266 JP names are empty. JP text is decoded through the same
version's CLOAD character chart, which `make splat VERSION=jp` also extracts.
The byte map records that source. US dictionary and control codes stay in
braces, like the other exports.

On the way out, the screen can read CD resource `0x5E3` into the same buffer
and upload it at (832, 256), where the border texture went. The archive is
only in that buffer until then.

## UI sprites

`g_zukan_ui_sprites` has 21 records of 12 bytes: two packed words and a
screen position.

| Bits | Field |
| --- | --- |
| word 0, bits 0-7 | Texture: 0 is the page texture, 1 is the border texture |
| word 0, bits 8-15 | U |
| word 1, bits 0-7 | V |
| word 1, bits 8-13 | Palette, a row in the shared CLUT at (0, 498) |
| word 1, bits 14-22 | Width |
| word 1, bits 23-31 | Height |
| u16, u16 | Screen X and Y; the screen adds 8 to X |

Sprites 0 through 5 are the page controls along the bottom. Sprite 0 lights up
when you go to the previous entry, 3 for the next one and 5 for going back to
the list. In the list view, sprites 0 through 3 are dimmed. Sprites 6 through
20 build the frame and page. The PNGs don't include that tinting.

## Categories and entries

Every entry has one index. That index picks its name in `entry_names.yaml` and
its value in `g_zukan_entry_values`. A nonzero value opens CD resource
`0xBFC` + value as the entry's page; zero means the entry has no page. Only
entry 0 has zero in both regions.

`g_zukan_category_ranges` holds 13 entry indices. Category N lists the
entries from the Nth index up to, but not including, the next one. The screen
changes three of them:

- Characters (7) stops at 454 instead of 480.
- World History (8) stops at 576 instead of 640.
- Techniques (11) gets entries 454 through 479 added after its own range.

So the "characters" from 454 to 479 are actually techniques. Entries 576
through 639 are never listed on their own; they're a second set of artifact
values.

Which listed entries you can open comes from the saved game's encyclopedia
bits, 1,024 bits at offset `0x3254` of the
[save file](save-file.md). Lands, Artifacts, Monsters, Characters, Cactus
Diaries and Techniques test one bit per entry, at a fixed distance from the
entry index. World History tests one bit per group, below. Equipment, Items,
Produce and Basic Golemology don't test any: every listed entry opens. A
locked entry still takes up its row, but shows a placeholder instead of its
name and doesn't open. Artifacts check two bits: when the second one is set,
the screen uses the value 511 entries further on, so artifact 65 opens the
page of entry 576.

`g_zukan_group_ranges` splits World History into six groups. The offsets
count from entry 480. A group's entries stay locked until its bit (`0x122`
through `0x127`) is set. The export lists each group's entry range and bit.

`g_zukan_display_order` sets the order in the lists. The screen goes down this
list and picks up every entry that belongs to the category, so an entry that
isn't in it is never listed. It ends at `0x400`, after 718 positions.

## Where things are

The data blob is 79,456 bytes in US and 77,272 bytes in JP. After the archive
and the sprites, its last 2,140 bytes are the screen's variables and the
visible entry list. They're all zero on disc, so the byte map lists them
without writing a file. The entry-table rodata is 3,556 bytes in both
versions, ending with two bytes of padding.

The 660 bytes between the tables and the C code are the overlay's entry
function, `zukan_run`. That's code, so it sits in its own `zukan_run_code`
rodatabin and isn't exported.

Both byte maps account for every byte. JP has no unknown gaps. US has one:
six bytes after the category titles, saved in `unknown/8015741A.bin`.
[`zukan.py`](../../../../tools/data/overlays/zukan.py) follows the same resource
readers, `Part` records and writers as the other extractors. Editing an
export doesn't change what the build links.

## What we don't know yet

- TODO: the six US bytes after the category titles (`00 00 73 00 00 00`).
  JP has zeros there.
- TODO: the exact bit each category tests is still only in
  `zukan_build_category_entries`; the export doesn't list it per entry. Some
  categories also set bits first, from other parts of the saved game.
- TODO: the check that decides whether resource `0x5E3` is reloaded on the way
  out reads a word at `0x8018000C` that has no name yet.
- TODO: the entry pages themselves are CD resources, not part of ZUKAN. Each
  one holds a header, the text lines, sprite records, the previous and next
  entry ids, and a TIM. There's no extractor for them yet.
