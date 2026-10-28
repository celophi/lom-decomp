# ZUKAN resources

[English index](../../README.md) | [Overlay extractors](../../../../tools/data/overlays/README.md) | [Japanese](../../../jp/technical/reference/zukan-resources.md)

ZUKAN is the encyclopedia: the category lists (Lands, Artifacts, Monsters and
the rest) and the page you get when you open an entry. Almost everything it
draws is inside the overlay itself. That's the two textures, every entry and
category name, and the sprites for the frame and buttons. The pages are the
exception. Each one is its own CD resource that gets loaded when you open the
entry.

You can export all of it:

```sh
make splat
make extract-zukan

make splat VERSION=jp
make extract-zukan VERSION=jp
```

The files end up in `assets/exports/<version>/overlays/zukan/`. Set
`ZUKAN_OUTPUT=/path/to/new-folder` if you want them somewhere else; the folder
can't already exist. If anything fails, you don't get a half-written export.
The export is for reading. Editing it doesn't change what the build makes.

| Path | Contents |
| --- | --- |
| `archive/archive.yaml` | The archive header and what each section holds |
| `images/page/`, `images/border/` | The two original TIMs, with a PNG per palette |
| `images/sprites/` | The 21 UI sprites, decoded, with a PNG of each |
| `text/entry_names.yaml` | 1,014 entry names |
| `text/category_names.yaml` | The 12 category titles |
| `tables/categories.yaml` | Where each category starts and ends |
| `tables/history_groups.yaml` | The six World History groups |
| `tables/entries.yaml` | All 1,018 entries, with their name, category and page |
| `tables/display_order.yaml` | The order entries are listed in |
| `byte-map.yaml` | Where each of those came from |

Both regions have the same entries and categories.

## Textures

The data starts with a small archive: a count of 4, then four offsets from the
start of the archive.

| Section | Offset | Contents |
| ---: | --- | --- |
| 0 | `0x14` | Page texture, uploaded at (320, 0) |
| 1 | `0x8234` | Border texture, uploaded at (832, 256) |
| 2 | `0x10454` | Entry names |
| 3 | US `0x12C70`, JP `0x123E0` | Category titles |

The page texture has the parchment, its edges and the PREV / NEXT / END
buttons. The border texture is the decorated frame around it. Both are
256 x 256 at 4 bits per pixel.

The palettes are a bit odd. Both TIMs upload their whole CLUT to the same VRAM
row at (0, 498), and the page texture goes second, so it wipes out the
border's palettes. Everything you see, the frame included, is drawn with the
page texture's colors. The PNGs use those colors since that's what the game
shows. Only palettes 0 to 5 have anything in them, so those are the only ones
you get a PNG for. The TIM files still have the border's own palettes if you
need them.

When you leave the encyclopedia, it may have to clean up after itself. The
border texture lands at (832, 256), which is on top of the field map's
texture if that texture is wide enough to reach that far. In that case ZUKAN
reloads those columns from CD resource `0x5E3` before going back. That
resource only holds the columns for Home's map, `MHM_ALC2`.

## Names

Entry names and category titles use the usual u16 offset tables (see
[text tables](text-tables.md)). Entry 0 is just a row of dashes, and a lot of
names are empty: 274 in US and 266 in JP. JP text is decoded with CLOAD's
character chart, which `make splat VERSION=jp` extracts along with everything
else. US dictionary and control codes stay in braces, the same as the other
exports.

## UI sprites

There are 21 sprite records of 12 bytes each: two packed words, then a screen
position.

| Bits | Field |
| --- | --- |
| word 0, bits 0-7 | Texture: 0 is the page texture, 1 is the border texture |
| word 0, bits 8-15 | U |
| word 1, bits 0-7 | V |
| word 1, bits 8-13 | Palette row |
| word 1, bits 14-22 | Width |
| word 1, bits 23-31 | Height |
| u16, u16 | Screen X and Y |

The screen draws every sprite 8 pixels to the right of its stored X.

Sprites 0 to 5 are the controls along the bottom of the page. Sprite 0 lights
up when you go to the previous entry, 3 when you go to the next one and 5 when
you go back to the list. In the list view, 0 to 3 are dimmed. Sprites 6 to 20
make up the frame and the page. The PNGs don't include any of that tinting.

## Categories and entries

Every entry has an index. That index picks its name in `entry_names.yaml` and
its page in `entries.yaml`. A page value of N opens CD resource `0xBFC` + N,
and 0 means there's no page. Entry 0 is the only one without a page.

There are 13 category boundaries. Category N lists the entries from the Nth
boundary up to, but not including, the next one. The screen overrides three of
them:

- Characters stops at 454 instead of 480.
- World History stops at 576 instead of 640.
- Techniques also gets entries 454 to 479 added after its own entries.

So entries 454 to 479 look like characters if you only go by the boundaries,
but they're really abilities, listed under Techniques. Entries 576 to 639 are
never listed at all. They're second pages for the artifacts (more on that
below).

World History is split again into six groups, with offsets counted from entry
480. The export lists each group's entries.

The display order decides the order in every list. The screen goes down it and
picks up each entry that belongs to the category. If an entry isn't in the
display order, it never shows up. The list has 718 positions and ends with
`0x400`.

## Unlock bits

Whether you can open an entry comes from the encyclopedia bits in your save:
1,024 bits at offset `0x3254` of the [save file](save-file.md). A locked entry
still takes up its row in the list, but it shows a placeholder and won't open.

| Category | Entries | Bit |
| --- | --- | --- |
| Lands | 1-64 | `0x200` + (entry - 1) |
| Artifacts | 65-128 | `0x240` + (entry - 65), or `0x280` + (entry - 65) |
| Monsters | 230-308 | entry - 230 |
| Monsters | 310-349 | 300 + (entry - 310) |
| Characters | 350-453 | 100 + (entry - 350) |
| Techniques (abilities) | 454-479 | `0x3C8` + (entry - 454) |
| World History | 480-575 | one per group, `0x122` to `0x127` |
| Cactus Diaries | 640-739 | 200 + (entry - 640) |
| Techniques | 750-1012 | `0x2C0` + (entry - 750) |

Monster 309 is always open. Equipment, Items, Produce and Basic Golemology
don't check anything, so every entry in them opens.

A few categories set their bits right before checking them. Lands and
Artifacts set theirs from your land records. Techniques sets the ability bits
from the abilities you've learned (Jump, Defend, then Lunge through Slide, then
Flip Kick) and the technique bits from the techniques you've learned for each
weapon.

Artifacts are the only ones with two bits. The `0x240` one gets set for each
land you've placed, the `0x280` one from another land flag. The artifact opens
if either is set, and when the `0x280` one is, you get its second page
instead: the entry 511 places further on. So artifact 65 opens the page of
entry 576.

## TODO

- The six US bytes after the category titles (`00 00 73 00 00 00`). JP
  has zeros there. The export saves them in `unknown/8015741A.bin`.
- What the land flag (bit 1) that unlocks Lands and the second artifact
  pages actually means.
- The entry pages are CD resources outside ZUKAN. Each one has a
  header, the text lines, sprite records, the previous and next entry ids, and
  a TIM. There's no extractor for them yet.
