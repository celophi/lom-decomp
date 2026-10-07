# MENU resources

[English index](../../README.md) | [Overlay extractors](../../../../tools/data/overlays/README.md) | [Japanese](../../../jp/technical/reference/menu-resources.md)

MENU is the in-game menu you open from the field: the node tree on the left,
the pages it opens, the item and equipment lists, and the help line at the
bottom. Its texture, icons, text, page layouts and a few small tables sit in
one data blob after the code. Both regional YAMLs keep that blob as one
`menu_data` databin. The build links those bytes unchanged, and a separate
extractor makes them readable.

```sh
make splat
make extract-menu

make splat VERSION=jp
make extract-menu VERSION=jp
```

Files go to `assets/exports/<version>/overlays/menu/`. Set
`MENU_OUTPUT=/path/to/new-folder` to use another destination. The folder must
be new. The extractor reads everything first, then writes a temporary folder
and moves it into place when it's finished. A failed write removes that folder.

## What comes out

| Path | Contents |
| --- | --- |
| `byte-map.yaml` | Every range in the blob, with its address, offset and size |
| `image/asset.bin` | The texture resource as stored: a short header, the TIM and a second palette row |
| `image/image.tim` | The 256 x 256, 4-bit TIM on its own |
| `image/image.yaml` | Header values, stored TIM layout and every palette |
| `image/palette_00.png` through `palette_1F.png` | The whole texture through each of its 32 palettes |
| `image/grid.png`, `image/grid.yaml` | The parchment background, drawn from its 29 sprite records |
| `icons/icons.yaml` | All 113 icon records: texture rectangle and palette code |
| `icons/001.png` ... | 106 icons, each cropped with its own palette |
| `text/resource.bin` | The complete original text resource |
| `text/resource.yaml` | Its 34 table offsets, sizes and names |
| `text/00_general.yaml` ... `text/33.yaml` | One file per table, with every entry, its offset and its original bytes |
| `content/nodes.yaml` | The 44 tree nodes: content group, item count and which item set each opens |
| `content/group_ids.yaml` | The content group of each node |
| `content/item_counts.yaml` | The item count of each of the 37 content groups |
| `content/groups.yaml` | Each group's count and its eight action codes |
| `content/item_sets.yaml` | The 24 item sets the nodes point at, 1,000 items in all |
| `tables/label_ids.yaml` | The label-id bytes some pages swap into the tree, with the symbols that index them |
| `tables/input_scripts.yaml` | 16 scripted input sequences, one pad state per frame |
| `tables/cursor_icons.yaml` | The three icon ids of the cursor animation |

Table files keep their original bytes in a `bytes` field, and text files keep
each string's bytes next to its decoded text, so nothing depends on the
decoded fields being right.

## Where things are

The blob starts right after the code. These are the RAM addresses; the byte
map also has each range's offset inside `menu_data.databin.bin`.

| Resource | US | JP | Size |
| --- | --- | --- | --- |
| Icon rectangles | `0x8014FBF4` | `0x8014FBE8` | 452 |
| Icon palette codes | `0x8014FDB8` | `0x8014FDAC` | 113 |
| Content item counts | `0x8014FE2C` | `0x8014FE20` | 37 |
| Content action codes | `0x8014FE54` | `0x8014FE48` | 296 |
| Content item sets | `0x8014FF7C` | `0x8014FF70` | 8,000 |
| Text resource | `0x80151EBC` | `0x80151EB0` | 58,276 US, 55,948 JP |
| Texture resource | `0x80160260` | `0x8015F93C` | 33,836 |
| Label ids | `0x80168690` | `0x80167D6C` | 60 |
| Content group ids | `0x801686CC` | `0x80167DA8` | 44 |
| Content table | `0x801686F8` | `0x80167DD4` | 176 |
| Input scripts | `0x801687A8` | `0x80167E84` | 768 |
| Grid sprites | `0x80168AA8` | `0x80168184` | 348 |
| Cursor icon ids | `0x80168C04` | `0x801682E0` | 3 |
| Runtime variables | `0x80168C08` | `0x801682E8` | 11,732 |

The only gaps are zero padding: 11 bytes in US and 15 in JP. The last 11,732
bytes are the menu's variables, node tree, window slots and memory card
buffers. They're all zero on the disc, so the byte map lists them without
writing a file. Neither version needs an `unknown/` folder. Unrecognized
nonzero bytes would be saved there unchanged, named by address.

## The texture and its palettes

The texture resource starts with three words: an entry count of 2, the TIM's
offset (12), and the offset of a second 256-color palette row that follows
the TIM. The TIM is 256 x 256 pixels, 4 bits each, with 256 colors. The menu
uploads the pixels to (320, 0), the TIM's colors as one row at (0, 498) and the
second row at (0, 499).

Icons and background sprites pick a palette with a one-byte code. The high
nibble is the row and the low nibble is the 16-color palette in that row, so
`0x0B` is palette 11 of the TIM's own colors and `0x11` is palette 1 of the
second row. The preview PNGs are named by that code. Color zero is
transparent and the others are opaque. The upload sets the STP bit on every
nonzero color; the exported TIM keeps the stored words.

The US and JP textures differ only in one 12 x 8 pixel patch at (64, 80),
and the palettes are identical. Five icon records (34-37 and 42) use other
texture cells or palettes in JP, and 46 content items sit a pixel or two
apart. The grid, group, count, label and script tables are identical.

## Icons and the background grid

An icon record is four bytes: U, V, width and height inside the texture. A
parallel array gives each icon its palette code. Records 0, 25, 32 and 96-99
are empty; they keep their index in `icons.yaml` but don't get a PNG. The
cursor animation uses icons 0x6B, 0x6C and 0x6D, stored in
`tables/cursor_icons.yaml`.

The background is 29 sprites, 12 bytes each: packed U/V, two reserved bytes,
then X, Y, width and height on screen. Sprites 17 and later use palette `0x01`
and the rest use `0x00`. `grid.png` puts them together at their screen
positions, 312 x 224 pixels. It doesn't apply the game's tinting.

## Text tables

The text resource is a table count (34), 34 byte offsets from the start of
the resource, then the tables. Each table is the usual list of u16 offsets
followed by zero-terminated strings; see [text tables](text-tables.md). The
export parses every table with the version's two-byte codes. US dictionary
and control codes stay in braces. JP text is decoded through the same
version's CLOAD character chart, and the byte map records where the chart came
from.

Table names come from the `MenuTextTable` enum in
[`menu_internal.h`](../../../../src/overlays/menu/internal/menu_internal.h). Tables
without a name there get only their number, like `02.yaml`. A few of those
names don't match what's in the table yet. Table 3 is called spell names but
holds moves like Jump and Defend, and tables 4 and 5, called ability help and
ability names, hold the artifacts, starting with the Mailbox.

Some entries read past their own table. In the US version of table 3, only
the first 82 offsets land inside the table. The other 174 all point at its
end, so they read the first bytes of table 4. Those entries are marked
`outside_table: true`.

Tables 19 and 20 hold numbered Japanese placeholders ("monster technique 00"
and its help text) in both versions. They use the Japanese character codes,
so the US export shows them as codes in braces.

## Content pages

Each of the 44 tree nodes has a content group and a pointer in the content
table. A pointer of 0 means the node has no page. The value 1 (node 32) ends
the menu instead: it sets `g_menu_load_request`. Everything else points at an
item set. Several nodes share a set; nodes 0, 3 and 6 all open the same one,
one for each character slot.

A content item is eight bytes:

| Offset | Size | Field |
| --- | --- | --- |
| 0 | 2 | Bits 0-8: X position. Bits 9-11: style. Bits 12-15: type |
| 2 | 1 | Y position |
| 3 | 1 | Action type, read by the page that draws the item |
| 4 | 4 | Parameters; their meaning depends on the page |

Type 5 opens a submenu and type 0xF runs an action; those two names come from
the C defines. A type 5 or 0xF item with style 7 is the page's active entry,
where the cursor starts. The action code for an item comes from `groups.yaml`:
its row is the node's content group, and its column is the item's style.

`item_counts.yaml` has one count per group, 37 in all. Entry 34 doubles as
the count the menu uses when the scene type is -1: it adds one and reads that
many items, four, from `g_menu_default_content_items`. Every item set starts
with an all-zero item. `item_sets.yaml` lists each set's items up to the next
set, with the nodes that use it.

## Labels and input scripts

`tables/label_ids.yaml` holds 60 bytes of label ids. The code doesn't read
them from one start address. It reads six bases, each indexed by an action
type, a node number or a party-order slot, and the bases overlap. The YAML
lists each base symbol and `first_index`, the index that reads the first
byte. For example, the base at `0x80168659` in US is only read with action
types 0x37 through 0x54, so its first byte is index 55.

The input scripts replay pad input, one 16-bit button mask per frame, until
`0xFFFF`. Each script has 24 slots. Script 0 means no script, so its row is
never read: those 48 bytes are the last twelve content-table entries, and the
export lists them there. Scripts 1, 2, 3, 4, 5, 6, 10, 11 and 12 have an end
marker. The others are all zeros. Button names come from the SDK's
`libetc.h`.

## What we don't know yet

- The item parameters and action types. Each page reads them differently,
  and most pages aren't mapped yet. TODO: name them from the page callbacks.
- Why the stored item count is usually one less than the set size, and what
  happens when it isn't. Node 42 uses group 18 (22 items) but points at the
  4-item default set. TODO: follow `menu_draw_scene_content`.
- The groups no node uses: 4 and 23 to 34, apart from 34's count serving as
  the default above. They may be reached another way.
- Names for the 20 text tables the enum leaves out.
- Scripts 7-9 and 13-16 are empty rows. TODO: check what starts them.

[`menu.py`](../../../../tools/data/overlays/menu.py) follows the same reader,
`Part` and writer structure as the other overlay extractors. The two 4-byte
alignment rodata blobs and the short card strings before the code stay with
their existing C units. Editing these exports doesn't change what the build
links.
