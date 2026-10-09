# Overlay resource extractors

[Tools index](../../README.md) | [Text tables](../../../docs/en/technical/reference/text-tables.md) | [Save file format](../../../docs/en/technical/reference/save-file.md)

Some overlays keep their text, icons and small tables in one data blob. The
build links that blob as-is, which is great for matching the original but not
much help if you just want to see what's in it. This tool opens the blob up and
writes out files you can look at.

The extractors handle ADDHERO, the 2P hero screen; CARDA, the save and load
screen; CHECKPS, the startup screen and CD check; CLOAD, the load screen;
FIELD, the resident field runtime; GNAME, the name-entry screen; GOLEM, the
logic-grid editor; GOSUB, the workshop and companion-list screens; MENU, the
in-game menu; NIKI, the diary save screen; SHOP, the shop screen; TITLE, the
title menu and character selection; WMAP, the world map; WSEL, the play-area
selection screen; and ZUKAN, the encyclopedia. GOVER and MOVIE have no data
blob to export. Extract the version's assets with `make splat` or
`make splat VERSION=jp` first, then run:

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
make extract-golem
make extract-golem VERSION=jp
make extract-gosub
make extract-gosub VERSION=jp
make extract-menu
make extract-menu VERSION=jp
make extract-niki
make extract-niki VERSION=jp
make extract-shop
make extract-shop VERSION=jp
make extract-title
make extract-title VERSION=jp
make extract-wmap
make extract-wmap VERSION=jp
make extract-wsel
make extract-wsel VERSION=jp
make extract-zukan
make extract-zukan VERSION=jp
```

`assets/` keeps two kinds of data apart. `assets/us/` and `assets/jp/` hold what
splat extracts for the build. `assets/exports/` holds data converted into
formats people can read. The output goes to
`assets/exports/<version>/overlays/<overlay>/`. Set `ADDHERO_OUTPUT`,
`CARDA_OUTPUT`, `CHECKPS_OUTPUT`, `CLOAD_OUTPUT`, `FIELD_OUTPUT`, `GNAME_OUTPUT`,
`GOLEM_OUTPUT`, `GOSUB_OUTPUT`, `MENU_OUTPUT`, `NIKI_OUTPUT`, `SHOP_OUTPUT`,
`TITLE_OUTPUT`, `WMAP_OUTPUT`, `WSEL_OUTPUT` or `ZUKAN_OUTPUT` to pick another
folder. The destination must be
new; the tool won't overwrite an existing folder.

## What you get

ADDHERO writes:

```text
addhero/
    byte-map.yaml          where everything came from in the blob
    text/
        messages.yaml      every message on the 2P screens, with its index and symbol
        locations.yaml     the location names shown next to each save
        card_titles.yaml   the memory card title templates, in Shift-JIS, with what each is for
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
follows each named entry to `CARD_MENU_STEP_DONE`, so a shorter sequence can share
the end of a longer one. The byte map counts the table's bytes only once.

The card overlays' US text is shown as ASCII, with anything else written as a
code in braces.
`{16}` is a one-byte code, and `{1F 00}` is a code with its second byte.
Japanese text is decoded with the game's own character chart, so it reads as
Japanese. The raw bytes are next to each string too, so nothing is lost. The
[text tables](../../../docs/en/technical/reference/text-tables.md) page explains
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
saved there. See the [CLOAD resource notes](../../../docs/en/technical/reference/cload-resources.md)
([Japanese](../../../docs/jp/technical/reference/cload-resources.md)) for the layout.

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
[FIELD resources](../../../docs/en/technical/reference/field-resources.md)
([Japanese](../../../docs/jp/technical/reference/field-resources.md)).

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
without needing an `unknown/` folder. See [GNAME resources](../../../docs/en/technical/reference/gname-resources.md)
([Japanese](../../../docs/jp/technical/reference/gname-resources.md)) for the layout
and the difference between stored and runtime texture placement.

GOLEM writes:

```text
golem/
    byte-map.yaml          every data range, plus the source of the JP character chart
    image/
        image.tim          original 256 x 256 texture and its 16 palettes
        image.yaml         stored layout, palette words and glyph preview details
        palette_00.png ... whole texture through each palette
        glyphs/01.png ...  79 glyph previews, each through all 16 palettes
    text/
        archive.bin        original two-section text archive
        archive.yaml       section offsets
        names.yaml         58 logic-block names
        descriptions.yaml  58 descriptions, using the same indices
    tables/
        glyph_metrics.yaml all 82 records, including three empty entries
        panels.yaml        154 packed panel records, decoded alongside the original bytes
```

A glyph's palette is supplied by the caller. Each glyph PNG therefore shows
all 16 palettes in a four-by-four grid, ordered left to right, then top to
bottom. Empty records 0, 20 and 53 keep their indices but have no PNG. Previews
show source colors without the game's tinting or blending. Composite block
shapes come from FIELD; they aren't another embedded GOLEM resource.

The texture and layout tables are identical in US and JP. JP text uses CLOAD's
character chart, and the byte map records that source. US dictionary and
control codes stay in braces. The final 292 bytes of both blobs are zero-filled
editor state, listed without another output file. Neither version has unknown
gaps. See [GOLEM resources](../../../docs/en/technical/reference/golem-resources.md)
([Japanese](../../../docs/jp/technical/reference/golem-resources.md)) for the packed
panel fields and texture upload details.

GOSUB writes:

```text
gosub/
    byte-map.yaml          every data and equipment-rodata range
    image/                 original UI TIM, 16 palette previews and 79 glyph previews
    text/
        archive.bin        original twelve-section text archive
        archive.yaml       section offsets
        *.yaml             item, spirit and block text, messages, species, spells and colors
    portraits/
        archive.bin        original count, offsets and 84 portraits
        portraits.yaml     indices, groups and stored palettes
        00.png ...         65 pet, seven golem and twelve egg portraits
    font/                  complete 64 x 16 TIM, PNG and layout
    tables/
        glyph_metrics.yaml all 82 UI glyph records
        item_colors.yaml   color-name indices for 37 color-material item ids
        equipment_groups.yaml first indices and counts for weapons, armor and instruments
```

The UI texture and glyph records are exact copies of GOLEM's. Both extractors
use the same palette-grid previews. JP text uses CLOAD's character chart; US
dictionary and control codes stay in braces. All twelve text sections keep
their original indices, including empty entries.

The portrait symbol points four bytes past its count, and the font symbol
points twenty bytes into a TIM. The font's pixels also extend into the unused
prefix of the item-color table. The extractor follows the complete resources
across those old boundaries, then exports the active color entries separately.
BSS stays with `gosub.c`. Both byte maps are complete, with no unknown gaps.
See [GOSUB resources](../../../docs/en/technical/reference/gosub-resources.md)
([Japanese](../../../docs/jp/technical/reference/gosub-resources.md)) for the text
counts, resource boundaries and texture placement.

MENU writes:

```text
menu/
    byte-map.yaml          every data range, plus the source of the JP character chart
    image/
        asset.bin          the texture resource as stored: header, TIM and second palette row
        image.tim          original 256 x 256, 4-bit texture
        image.yaml         header values, stored layout and all 32 palettes
        palette_00.png ... whole texture through each palette, named by palette code
        grid.png           the background grid assembled from its 29 sprites
        grid.yaml          the grid sprite records
    icons/
        icons.yaml         113 icon rectangles and palette codes
        001.png ...        106 icons, each cropped with its own palette
    text/
        resource.bin       original 34-table text resource
        resource.yaml      table offsets, sizes and names
        00_general.yaml ...  one file per table, with codes and raw bytes
    content/
        nodes.yaml         the 44 tree nodes and the item set each opens
        group_ids.yaml     the content group of each node
        item_counts.yaml   item counts for the 37 content groups
        groups.yaml        each group's action codes
        item_sets.yaml     24 item sets, 1,000 items, with decoded fields
    tables/                label ids, input scripts and cursor icon ids
```

The texture has 32 palettes: the TIM's own 256 colors and a second 256-color
row stored after it. Icons and grid sprites pick one with a one-byte code,
row in the high nibble and palette in the low one, so the previews are named
by that code. Icon PNGs use the icon's own palette; color zero is
transparent. The grid preview draws the background at its screen positions
without the game's tinting.

Text table names come from the `MenuTextTable` enum; the other tables are
numbered. Some US entries in table 3 point past the table's end and read the
next table; they're marked `outside_table`. JP text decodes through the same
version's CLOAD blob, like FIELD and GOLEM. US dictionary and control codes
stay in braces.

A few MENU symbols are index bases, not object starts. The label-id bases are
read at an offset, so the byte map starts that table where the lowest index
lands, and the YAML lists each base with the index of its first byte. The
input-script table's row 0 is never played; its bytes are the last twelve
content-table entries and are exported there. Every item set starts with an
all-zero item, and a set runs to the next address the content table uses.

The final 11,732 bytes are zero-filled menu variables and memory card
buffers, listed in the byte map without another file. The gaps are zero
padding, 11 bytes in US and 15 in JP; neither export needs an `unknown/`
folder. See [MENU resources](../../../docs/en/technical/reference/menu-resources.md)
([Japanese](../../../docs/jp/technical/reference/menu-resources.md)) for the
layout and item fields.

NIKI writes:

```text
niki/
    byte-map.yaml          every range in the data blob, including padding and variables
    text/
        messages.yaml      the card-screen messages, with slot symbols and raw bytes
        card_titles.yaml   the two memory card title templates, in Shift-JIS
        locations.yaml     the 63 location names picked by music track
    icons/
        icons.yaml         icon ids, character groups, offsets and stored palettes
        icon_00.png ...    the 86 party icons, 48 x 48 each
    tables/
        card_steps.yaml    eight card sequences, named from CardMenuStep
        digit_glyphs.yaml  full-width decimal and hexadecimal digits
        text_conversion.yaml   the character chart and its Shift-JIS mappings
```

NIKI is the diary screen. It carries the same card-screen resources as
ADDHERO: 91 messages in US and 90 in JP, the location table, the party icons,
the card title templates, the chart and the digits. About 30 messages have a
`g_niki_text_*` symbol, and the export shows it next to the entry. JP text is
decoded through NIKI's own chart, so CLOAD isn't needed.

The step export follows each named sequence to `CARD_MENU_STEP_DONE`. One byte
just before `g_niki_write_save_sequence` is never run, since the code starts
one byte later. It's listed under `unreached`, and the table's raw bytes are
kept too. Command 14, the idle wait, has no name in the enum and shows as
`0x0E`.

The blob runs to the end of the overlay. Its final 53,264 bytes are NIKI's
variables and buffers, all zero on disc, recorded in the byte map without
another file. The other gaps are zero padding in both versions, so neither
export has an `unknown/` folder. The `bu00:` paths and the full-width MAX text
stay with the C code that defines them. See [NIKI resources](../../../docs/en/technical/reference/niki-resources.md)
([Japanese](../../../docs/jp/technical/reference/niki-resources.md)) for the layout.

SHOP writes:

```text
shop/
    byte-map.yaml          every data range, plus the source of the JP character chart
    text/
        archive.bin        original four-section text archive
        archive.yaml       section offsets
        item_descriptions.yaml  256 item class lines, one per item kind
        item_names.yaml    256 item names; the first 64 are also equipment materials
        equipment_types.yaml    32 weapon, armor and instrument type names
        instrument_spells.yaml  112 spell names, 14 for each of 8 spirits
    tables/
        sell_prices.yaml   the sell price of every item kind, with its name
```

SHOP has no images. Its text archive holds what the list and detail windows
show, and each entry keeps its index, so item names and descriptions line up
with item kinds. Equipment type entries note their category and type number,
and spell entries note their spirit and spell number, which is how the shop
looks them up. JP text uses CLOAD's character chart; US dictionary and control
codes stay in braces. Buy prices don't live in SHOP: the field script that
opens a shop passes the stock and its prices.

The last 2,860 bytes of both blobs are the shop's zero-filled variables and
list buffer, listed in the byte map without another output file. Neither
version has unknown gaps. See [SHOP resources](../../../docs/en/technical/reference/shop-resources.md)
([Japanese](../../../docs/jp/technical/reference/shop-resources.md)) for the
archive layout and the spacing codes in the item descriptions.

TITLE writes:

```text
title/
    byte-map.yaml          every data range, plus the source of the JP character chart
    title_menu/
        offsets.yaml       count and self-relative offsets of the two menu TIMs
        menu_items/        256 x 256 4-bit menu text: image.tim, image.yaml, 16 palette PNGs
        backdrop/          320 x 240 16-bit title backdrop: image.tim, image.yaml, image.png
        cursor_blink.yaml  the cursor's texture U for its four blink frames
    save_slot_menu/
        textures.yaml      VRAM position of each of the 11 selection-screen TIMs
        textures/00 ... 10 each TIM with its image.yaml and palette preview
        panel_uvs.yaml     12 pose rectangles for the right side, in pixels
        sprite_uvs.yaml    12 pose rectangles for the left side, in pixels
        layout.yaml        the 27 screen primitives with their starting values
        starting_weapons.yaml  11 item records, one per weapon type
    game_state/
        new_game.yaml/.bin     the New Game state (SavedGameLayout plus 0x1E5C more bytes)
        alternate.yaml/.bin    a second complete SavedGameLayout
        hero_default.yaml/.bin, hero_continue.yaml/.bin  the two hero records
```

The extractor reads the TIMs, offset table, blink sequence, upload table, UV
tables and layout with the parsers in `tools/data/formats/`. The selection screen keeps the code's save-slot-menu name. Its TIMs
are found through the upload table's pointers, which must match the
`g_save_layout_tim_NN` symbols. Previews cover 4-bit and 8-bit images through
each stored palette and 16-bit images directly; color zero is transparent.

Starting weapons, hero records and game states are decoded by `saved_game.py`
from the layouts in `include/common/saved_game.h`, with the original bytes beside
them. JP names use CLOAD's character chart; the US records still hold some
Japanese-coded names, which stay as codes in braces. The last 140 bytes are
zero-filled menu state, listed without an output file. Neither version has
unknown gaps. See [TITLE resources](../../../docs/en/technical/reference/title-resources.md)
([Japanese](../../../docs/jp/technical/reference/title-resources.md)) for offsets and what each table drives.

WMAP writes:

```text
wmap/
    byte-map.yaml          every data range, including the zero-filled runtime state
    tables/                56 fixed tables: primitive templates, map game rounds, information panel,
                           land texture slots, artifact images and paths, land attributes, labels, backdrop
    sounds/
        sound_01.akao ...  the 63 original AKAO sound-effect buffers, numbered by sound id
        sounds.yaml        sizes, entry counts, bank keys and channel sequence offsets
    scripts/
        input_scripts.yaml the three scripted-input sequences, decoded, with their bytes
    handlers/              453 step tables, one file per C source, entries named by function
    unknown/               one 136-byte gap no code names, preserved by RAM address
```

Most of the 978 KB blob is runtime state: only the first 79,672 bytes in US
(79,668 in JP) hold data, and the rest is all zero on disc. The byte map lists
that area once without writing it out. Table layouts come from the C types in
`wmap_tables.py`. Step tables are found by their declarations in
`src/overlays/wmap`, so a new declaration there shows up in the export without
touching the tool. Entries print function names; one entry in land effect 18
points into WMAP's data and is kept as a data reference.

The sound buffers are AKAO sound-effect lists. The export decodes their entry
tables and leaves the sequence bytes in the `.akao` files. US and JP share the
same sounds; the input scripts differ only in the confirm button they press.
See [WMAP resources](../../../docs/en/technical/reference/wmap-resources.md)
([Japanese](../../../docs/jp/technical/reference/wmap-resources.md)) for offsets,
the script format and what's still unknown.

WSEL writes:

```text
wsel/
    byte-map.yaml          every data range, including repeated TIM words and screen state
    images/
        land_map/          416 x 416 8-bit land map: image.tim, image.yaml, palette_00.png
        world_map/         320 x 224 8-bit world map screen
        cursor/            120 x 118 4-bit cursor frame
        world_overlay/     320 x 224 4-bit vignette, drawn subtractively
        hero_default/      256 x 256 8-bit pose sheet, plus poses/00.png ... 11.png
        hero_alternate/    the other hero's sheet and poses
        hero_shadow/       256 x 256 4-bit sheet holding the hero's drop shadow
        prompt/            184 x 88 4-bit "Select play area." prompt
    tables/
        sprite_layers.yaml eight layer records: texture page, palette, source and screen rectangles
        hero_poses_default.yaml, hero_poses_alternate.yaml  12 pose cells per hero sheet
        cell_occupied.yaml 19 x 19 flags for cells that can't be chosen
        cell_tile_masks.yaml 6 x 6 tile masks for all 361 cells
```

`wsel_load_resources` uploads TIM n to sprite layer n, and the game uploads
pixels and palette to the layer record's VRAM coordinates, not the TIM's own.
Each `image.yaml` lists both. Layer names come from the `WSEL_SPRITE_*`
defines in `wsel.c`. Each TIM has one palette; previews show value zero as
transparent and don't apply the game's blending or brightness. Only pose 0 of
each hero sheet is drawn, but all twelve are cropped.

US and JP differ only in the prompt image. The final 16,500 bytes of both
blobs are zero-filled screen state and the music sequence buffer, listed
without another output file. The only gap is three padding bytes after the
cell flags, so neither version has unknown data. See
[WSEL resources](../../../docs/en/technical/reference/wsel-resources.md)
([Japanese](../../../docs/jp/technical/reference/wsel-resources.md)) for the
record layouts, hero placement and grid indexing.

ZUKAN writes:

```text
zukan/
    byte-map.yaml          every data and entry-table range, plus the source of the JP character chart
    archive/archive.yaml   the four archive sections and their offsets
    images/
        page/              original page TIM and previews through its six used palettes
        border/            original border TIM and previews through the same palettes
        sprites/
            sprites.yaml   21 UI sprite records, decoded next to their words
            00.png ...     each sprite cropped from its texture
    text/
        entry_names.yaml   1,014 entry names
        category_names.yaml    the 12 category titles
    tables/
        categories.yaml    where each category starts and ends, with the screen's overrides
        history_groups.yaml    the six World History groups and their unlock bits
        entries.yaml       1,018 entries with name, category and page resource
        display_order.yaml the 718-position list order
```

Both textures upload their palettes to the same CLUT row, and the page texture
goes second. So every sprite is drawn through the page texture's palettes, and
the previews use those colors; the border TIM still keeps its own stored
palette words. Palettes without any colors get no PNG. JP text uses CLOAD's
character chart; US dictionary and control codes stay in braces.

The entry tables sit in a `zukan_entry_tables` rodatabin before the code. The
overlay's entry function, `zukan_run`, follows them in its own
`zukan_run_code` rodatabin; it's code, so the extractor leaves it alone. The
data blob's last 2,140 bytes are zero-filled screen state, listed in the byte
map without another file. JP has no unknown gaps; US has six bytes after the
category titles, saved in `unknown/`. See
[ZUKAN resources](../../../docs/en/technical/reference/zukan-resources.md)
([Japanese](../../../docs/jp/technical/reference/zukan-resources.md)) for the
sprite fields, category overrides and unlock bits.

## How it's put together

`addhero.py`, `carda.py`, `checkps.py`, `cload.py`, `field.py`, `gname.py`,
`golem.py`, `gosub.py`, `menu.py`, `niki.py`, `shop.py`, `title.py`,
`wmap.py`, `wsel.py` and `zukan.py` use one reader per resource format. Each reader parses its bytes
into a small dataclass and returns a `Part` that says where the bytes are and what they hold. `read_blob`
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
blob, part, byte-map and output code all fifteen extractors share. The smaller
modules (`text_table.py`, `icon_set.py`, `png.py`, `symbols.py` and
`splat_config.py`) handle the underlying file formats.
`field_tables.py` lists FIELD's small array layouts; its larger resources have
separate readers in `field.py`. GNAME reuses the typed format readers in
`tools/data/formats/` for its tables, name-record archive and TIM. `glyph_texture.py`
holds the eight-byte glyph format and palette-grid writer shared by GOLEM and
GOSUB.
`wmap_tables.py` lists WMAP's fixed table layouts, the way `field_tables.py`
does for FIELD. `tim_preview.py` writes PNG previews of 4-, 8- and 16-bit TIMs
for TITLE, and `saved_game.py` decodes the saved-game records TITLE stores,
with offsets from `include/common/saved_game.h`.

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

The GOLEM tests cover packed panel fields, the split cell-height bits, glyph
palette grids, empty records, JP decoding through CLOAD and complete byte
coverage. Bad offsets, invalid image rectangles and failed writes leave no
partial output. Source tests check both regional maps and the C record layouts.

The GOSUB tests cover all twelve text sections, portrait count boundaries,
font pixels that cross into the color-table prefix, active color indices and
complete byte coverage of both inputs. They check PNG pixels, JP decoding,
invalid offsets and cleanup after a failed write. Source tests keep both
regional layouts and copied C constants in step with the extractor.

The MENU tests cover both palette rows, icon crops, the grid preview, text
tables with entries past their end, JP decoding through CLOAD, content nodes
and item sets, label bases and input scripts, and complete byte coverage.
Bad offsets, invalid rectangles, missing symbols and failed writes leave no
partial output. Source tests check both regional layouts and the C record
layouts and constants.

The NIKI tests check both regional layouts, the slot and step symbols, and the
chart and icon sizes against `niki_internal.h`. The synthetic overlay covers
Japanese chart decoding, icon PNG pixels, unreached step bytes, complete byte
coverage and unknown gaps. Bad text or icon offsets, a sequence without a stop
byte, missing inputs and failed writes must leave no partial output.

The SHOP tests cover all four text sections, the category and spirit indices
the renderer uses, section padding, sell prices named from the item table, JP
decoding through CLOAD and complete byte coverage. Bad section offsets, short
tables, missing inputs and failed writes leave no partial output. Source tests
check both regional maps, the archive header struct and the counts copied from C.

The TITLE tests cover 4-, 8- and 16-bit TIM previews, the texture table's
pointers, UV scaling, layout primitives, starting weapons, the saved-game
records, JP decoding through CLOAD and complete byte coverage. Bad offsets,
broken trailing words, symbols out of order and failed writes leave no partial
output. Source tests check both regional maps, the C counts, and every
saved-game offset against `include/common/saved_game.h` through libclang.

The WMAP tests cover every table layout, AKAO entry offsets with empty
channels, repeated sound pointers, input-script commands, step tables with
null and data entries, and complete byte coverage. Unknown step targets, bad
sound headers, scripts without an end code, missing inputs and failed writes
must leave no partial export. Source tests check both regional configs, the
C record sizes and counts, and the script enum.

The WSEL tests cover 4- and 8-bit palette previews, TIM upload destinations
from the layer records, hero pose crops, the grid rows and tile masks, and
complete byte coverage with padding and runtime state. Bad offsets, poses
outside their sheet, a TIM whose depth differs from its layer and failed
writes leave no partial output. Source tests check both regional maps, the C
record layouts and counts, and that each TIM symbol is uploaded to the layer
named after it.

The ZUKAN tests cover the archive header, the shared runtime palette on both
textures, sprite crops, category overrides, the display-order terminator, JP
decoding through CLOAD and complete byte coverage of both inputs. Bad archive
offsets, sprites outside their texture, a missing terminator and failed writes
leave no partial output. Source tests check both regional layouts, the table
lengths from the C structs and the constants copied from the C code.
