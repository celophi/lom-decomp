# TITLE resources

[English index](../../README.md) | [Overlay extractors](../../../../tools/overlays/README.md) | [Japanese](../../../jp/technical/reference/title-resources.md)

TITLE runs the title menu and the character and weapon selection that follows
New Game. Its data holds the menu artwork, the selection screen's textures and
layout, the eleven starting weapons, and the game states a new game is built
from. The old YAML cut this into many typed segments. Both regions now keep it
in one `title_data` databin, the build links those bytes unchanged, and a
separate extractor makes them readable.

```sh
make splat
make extract-title

make splat VERSION=jp
make extract-title VERSION=jp
```

Files go to `assets/exports/<version>/overlays/title/`. Set
`TITLE_OUTPUT=/path/to/new-folder` to use another destination. The folder must
be new. The extractor reads everything first, then writes a temporary folder
and moves it into place when it's finished. A failed write removes that folder.

## What comes out

| Path | Contents |
| --- | --- |
| `byte-map.yaml` | Every range in the data blob, including the TIMs' repeated last words and the menu state |
| `title_menu/offsets.yaml` | The count and self-relative offsets of the two menu TIMs |
| `title_menu/menu_items/` | The 256 x 256, 4-bit menu text texture, as `image.tim` and one PNG per palette (16) |
| `title_menu/backdrop/` | The 320 x 240, 16-bit title backdrop, as `image.tim` and `image.png` |
| `title_menu/cursor_blink.yaml` | The cursor's texture U for each of its four blink frames |
| `save_slot_menu/textures.yaml` | Where each of the 11 selection-screen TIMs is uploaded in VRAM |
| `save_slot_menu/textures/00/` through `10/` | Those TIMs, each with its palette preview |
| `save_slot_menu/panel_uvs.yaml`, `sprite_uvs.yaml` | The 12 source rectangles for each side of the stage, in pixels |
| `save_slot_menu/layout.yaml` | The 27 primitives that build the screen, with their starting values |
| `save_slot_menu/starting_weapons.yaml` | The 11 weapon records, one per weapon type |
| `game_state/new_game.yaml`, `.bin` | The state New Game starts from, decoded and as original bytes |
| `game_state/alternate.yaml`, `.bin` | A second complete state, used by the other menu choices |
| `game_state/hero_default.yaml`, `hero_continue.yaml`, `.bin` | The two hero records the selection screen can copy into the party |

The code calls the selection screen the save-slot menu (`run_save_slot_menu`),
so the folder keeps that name. Its label texture says what it really is:
"Select your character." and "Select your weapon."

Every YAML file keeps the resource's address and original bytes next to the
decoded values. Editing an export doesn't change what the build links.

## Where things are

The blob is 721,892 bytes (`0xB03E4`) in both versions, and every resource sits
at the same offset in both. JP addresses are the US ones plus `0xC0`.

| Resource | Offset | Size | US address | JP address |
| --- | --- | --- | --- | --- |
| Menu TIM offsets | `0x0` | `0xC` | `0x800522E8` | `0x800523A8` |
| Menu text TIM | `0xC` | `0x8220` | `0x800522F4` | `0x800523B4` |
| Title backdrop TIM | `0x822C` | `0x25814` | `0x8005A514` | `0x8005A5D4` |
| Cursor blink offsets | `0x2DA44` | `0x4` | `0x8007FD2C` | `0x8007FDEC` |
| Selection TIMs 00 to 10 | `0x2DA48` | `0x79ACC` | `0x8007FD30` | `0x8007FDF0` |
| Texture upload table | `0xA7514` | `0xB0` | `0x800F97FC` | `0x800F98BC` |
| Panel UV table | `0xA75C4` | `0x48` | `0x800F98AC` | `0x800F996C` |
| Sprite UV table | `0xA760C` | `0x48` | `0x800F98F4` | `0x800F99B4` |
| Layout table | `0xA7654` | `0x288` | `0x800F993C` | `0x800F99FC` |
| Starting weapons | `0xA78DC` | `0x2C0` | `0x800F9BC4` | `0x800F9C84` |
| New-game state | `0xA7B9C` | `0x50BC` | `0x800F9E84` | `0x800F9F44` |
| Alternate state | `0xACC58` | `0x3260` | `0x800FEF40` | `0x800FF000` |
| Default hero record | `0xAFEB8` | `0x250` | `0x801021A0` | `0x80102260` |
| Continue hero record | `0xB0108` | `0x250` | `0x801023F0` | `0x801024B0` |
| Menu state | `0xB0358` | `0x8C` | `0x80102640` | `0x80102700` |

The TIMs are standard PlayStation TIM files. All of them except the menu text
texture are followed by a copy of their own last word; the byte map lists
those four-byte copies separately. The last 140 bytes are the menu's runtime
variables (exit state, fade colors, cursor and slide positions). They're all
zero on disc, so the byte map notes them without writing a file. There's no
padding and no unknown range in either version.

## The title menu

`offsets.yaml` is a count (2) followed by two offsets that count from the
table's own address: `0xC` for the menu text and `0x822C` for the backdrop.
TITLE ignores the coordinates stored in these TIMs. It uploads the menu text
pixels at (320, 0) with all 256 colors in one row at (0, 480), and the
backdrop at (320, 256).

The menu text texture is a stack of 16-pixel rows. Row 0 is the copyright
header and row N is the item in menu slot N - 1: New Game, Load Game, then
entries such as Quick Continue and Encyclopedia that the menu never switches
on. Row 7 holds the cursor's three frames. Palette 1 draws the header and the
selected item, palette 2 the other items and palette 0 the cursor. Only the first two slots are switched on when the
menu starts. The cursor's U offset cycles through `0, 16, 32, 16`, one step
every fourth frame.

## The character and weapon screen

The eleven textures are uploaded where `textures.yaml` says, not at the
coordinates stored in each TIM. The upload also fills each entry's control
word from its TIM (pixel mode, width and height), so the stored control words
are zero. Texture 01 has the screen's text, which differs between US and JP;
the other ten textures are identical in both versions.

`layout.yaml` lists 27 primitives. TITLE links each one at the head of the
ordering table, so the list runs from front to back: entry 26, the stone wall
from texture 00, is drawn first. Each entry has flags (move with the stage,
semi-transparency, blend mode), a primitive type, a texture, a screen
position, a separate position for solid tiles, a texture U/V and a size. The
types are `hidden`, `sprite_strip` (sprites of up to 128 pixels side by side),
`tile` (a solid dark tile), `panel_quad` and `slot_sprite`.

| Entries | Texture | What they draw |
| --- | --- | --- |
| 0, 3 to 8, 10 to 15 | 01 | The prompt, the weapon lists, small markers and two dark tiles |
| 22, 23 | 01 | A dark ellipse under each hero |
| 1 | 08 | A 320 x 80 strip at the top of the screen |
| 2, 9 | 04 | The bar beside each weapon list |
| 16, 17 | 05 | The stone panels behind the weapon lists |
| 18, 19 | 06, 07 | The light over the left or right character |
| 20, 21 | 09, 10 | The two heroes' poses |
| 24, 25 | 02, 03 | The two character portraits |
| 26 | 00 | The wall with both portrait frames |

TITLE changes entries 0, 2 to 15, 18 and 19 while the screen runs, so the file
has their starting values. Left and right on the character step swap which of
entries 18 and 19 is showing, and that decides which way the stage slides on
confirm. The only difference between the versions is the prompt's Y position
(entry 0): 18 in US and 16 in JP.

The two UV tables have 12 rectangles each, stored in 8-pixel units; the YAML
already shows them in pixels. Entry 20 takes its pose from `panel_uvs.yaml`:
entry 0 until the stage has slid right, then entry `weapon + 1`. Entry 21 does
the same with `sprite_uvs.yaml` once the stage has slid left.

Each starting weapon is an ordinary 64-byte item record, the same layout as
an inventory entry in the [save file](save-file.md). The record index is the
weapon type, from knife (0) to bow (10). US names them `MenosKnife` through
`MenosBow`; JP's Japanese names are Bronze Knife and so on. When the player
confirms a weapon, TITLE copies the chosen record into the hero's weapon slot
and clears the technique masks of every other weapon type.

## Game states

New Game copies the first 12,904 bytes (`0x3268`) of `new_game.bin` over the
game state, sets field scene `0xD` and opens the character screen. The copy is
eight bytes longer than the 12,896-byte saved-game layout, the same length a
save file copies. Confirming a weapon first copies `hero_default` or
`hero_continue` over party slot 0: `hero_default` after the stage slides
right, `hero_continue` after it slides left. The second also sets bit 0 of
the mode-flags word. Their character types are 0 and 1.

`alternate.bin` is exactly one saved-game layout. TITLE loads it for any menu
choice other than New Game, Load Game or the idle timeout, then goes straight
to the field (scene 0). Its copy is also `0x3268` bytes, so it reads the first
eight bytes of `hero_default` too. It starts with 100,000 money, 32 placed
lands and 50 of each item kind from 0 to 254. JP's copy also has 41 inventory
items; in US every inventory record has an empty name, which marks it free.

The YAML decodes the fields [saved_game.h](../../../../include/saved_game.h)
names: the summary, money, techniques, proficiencies, identity, lands, party
characters with their equipment, inventory and item counts. Everything else,
such as the menu slots, golems and pets, is only in the `.bin`. Free item
records and unused lands are left out of the YAML.

JP text is decoded with the same version's CLOAD character chart.
`make splat VERSION=jp` extracts that blob too, and the byte map records where
the chart came from. US text is shown as ASCII with other codes in braces. The
US game states and hero records still hold Japanese-coded names, such as the
hero name `{B9}{AF}{B5}` and the sword in the hero's weapon slot. The weapon
choice overwrites the sword, and the name-entry screen runs next. A zero
second byte of a two-byte character belongs to the character, not the
terminator.

## What we don't know yet

- **The rest of the new-game state.** Bytes `0x3260` to `0x50BB` of
  `new_game.bin` are outside the saved-game layout and aren't copied by
  `load_saved_game_template`. Nothing in TITLE reads them. They look like more item
  and slot records, but we haven't found an owner.
- **The alternate state.** TITLE never switches on the menu slots that lead
  to it, and we haven't found anything else that does.
- **The hero record names.** `hero_default` and `hero_continue` are named
  after the C symbols. Which on-screen character each one is hasn't been
  checked against the running game.

[`title.py`](../../../../tools/overlays/title.py) follows the same reader,
`Part` and writer structure as the other overlay extractors, and reuses the
TIM, offset table, sequence, upload table, UV and layout parsers from
`tools/assets/`. The saved-game records are read by
[`saved_game.py`](../../../../tools/overlays/saved_game.py), whose offsets the
tests compare with the C header.
