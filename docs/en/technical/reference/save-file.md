# Save file format

[English documentation](../../README.md) | [日本語](../../../jp/technical/reference/save-file.md) | [CARDA overlay](../architecture/carda.md) | [ADDHERO overlay](../architecture/addhero.md) | [Text tables](text-tables.md)

Legend of Mana keeps each save as one file on the memory card. If you've ever
opened a card in an emulator's memory card manager, you've seen these files as
two-block entries with the little animated icon and the hero's name. This page
covers what's inside that file, where the interesting values live, and what you
have to fix up if you edit it.

All numbers are little-endian, like everything else on the PlayStation. Offsets
are from the start of the save file. If your tool exports saves with its own
header in front, skip it first; the save file itself starts with the letters
`SC`.

## The big picture

An ordinary game save is 16 KiB, exactly two 8 KiB memory card blocks.
Ring Ring Land uses a separate six-block transfer file, covered by the
[CARDA guide](../architecture/carda.md#sending-and-returning-pets).
The ordinary save has four parts:

| Offset | Size | What it is |
|---|---|---|
| `0x0000` | `0x180` | Memory card header: title, icon palette and icon frames |
| `0x0180` | `0x3260` | The saved game itself |
| `0x33E0` | 4 | Checksum |
| `0x33E4` | 4 | Magic value, the bytes `ANA` followed by a zero |
| `0x33E8` | the rest | Unused, fills the file up to `0x4000` |

The header is the standard PlayStation one, so the BIOS and memory card
managers can show it. Everything after it is Legend of Mana's own data.

## File names

The game names its saves after its product code, followed by a counter:

```text
BASLUS-01013 1A +3
^            ^  ^^
|            |  |+-- save number shown in the list ("No-3")
|            |  +--- "+" or "-", see below
|            +------ save counter in hex, no leading zeros
+------------------- product code prefix
```

There are no spaces in the real name; they're only there to make the parts
easier to see.

| Name | Meaning |
|---|---|
| `BASLUS-01013...` | A normal US save |
| `BASLUSP01013...` | The PocketStation mini-game's save (*Ring Ring Land*); the US release doesn't use it |
| `BASLUS-01013DUMMY` | Temporary file used while a save is being written |
| `AKIdummy` | The empty "New Save" slot shown in the list (*aki* is Japanese for empty) |

The Japanese release uses `BISLPS-02170` in the same places.

A new save gets a higher counter than the saves already on the card, and the
load screens use it to mark the newest and oldest saves. The `+` or `-` follows
option bit 2 at `0x01A8`, which the game's new-game script turns on together
with bit 3. Saves with the `+` show a small marker in the save list. When both
bits are set, loading the save goes to the world select screen instead of
straight into a scene. We haven't pinned down what that means for the player
yet.

If a game crashed or lost power mid-save, you might find a leftover `DUMMY`
file. The save screens erase those before they read the card.

## Memory card header

| Offset | Size | Contents |
|---|---|---|
| `0x0000` | 2 | `SC`, the PlayStation save marker |
| `0x0002` | 1 | Icon flags; Legend of Mana uses a two-frame animated icon |
| `0x0003` | 1 | Block count |
| `0x0004` | 64 | Title shown by the memory card manager, two lines of 32 bytes, Shift-JIS |
| `0x0060` | 32 | Icon palette, 16 colors |
| `0x0080` | 256 | Two 16 x 16 icon frames, 4 bits per pixel |

The title is Shift-JIS, which is the Japanese text encoding the BIOS uses. It is
not the same encoding as the game's own text; see [text tables](text-tables.md)
for that.

A finished save's title starts with `MANA--No.` and goes on with the save
number, play time and hero name, all in full-width characters. When option bit 2
is set, a musical note sign replaces the second dash; it's the same flag that
puts `+` in the file name.

While the game is writing a save, the title says `MANA-BAD.` instead. Its last
step is to put the real title back. So if you find a save still titled
`MANA-BAD.`, the save was interrupted, and its checksum won't match either,
because the checksum was worked out with the real title.

## The saved game

The saved game starts at `0x0180`. The first part is a short summary. The save
screens read only this much of each file to fill in the details window, so
changing these values changes what the load screen shows, not necessarily what
happens in the game.

### Summary and progress

| Offset | Size | Contents |
|---|---|---|
| `0x0180` | 21 | Hero name, copied from the hero's character record when you save |
| `0x0195` | 1 | Hero level, copied when you save |
| `0x0196` | 1 | A byte from the hero's weapon (the first of its item values), copied when you save |
| `0x0197` | 1 | Number of in-use summary records (see `0x32E0`) |
| `0x0198` | 4 | Spawn point of the current scene (low 25 bits) and the first party icon (top 7 bits) |
| `0x019C` | 2 | Sound bank of the current scene |
| `0x019E` | 1 | Secondary music of the current scene |
| `0x019F` | 1 | Palette of the party icons on the load screen |
| `0x01A0` | 4 | Music track (low 18 bits) and the second and third party icons (7 bits each above it) |
| `0x01A4` | 2 | Scene id |
| `0x01A6` | 1 | Object id |
| `0x01A7` | 1 | Music id |
| `0x01A8` | 4 | Options: bit 0 vibration, bit 1 mono sound, bits 2 and 3 set by the new-game script |
| `0x01AC` | 4 | Money, capped at 10,000,000 |
| `0x01B0` | 4 | Play time in 1/60 second ticks |

Play time is simple to read: divide by 216,000 for hours, and by 3,600 for
minutes. A value of 216,000 is exactly one hour.

The load screen doesn't store the location name. It uses the music track at
`0x01A0` to look up the name in a table, so the track and the location go
together.

A party icon of `0x7F` means the slot is empty. Icons 0 and 1 are the two
heroes, 2 to `0x0D` are the other characters who can join the party, values
from `0x0E` are pets and values from `0x4F` are golems.

### Skills

| Offset | Size | Contents |
|---|---|---|
| `0x01B4` | 44 | Learned special techniques, one 32-bit mask per weapon type |
| `0x01E0` | 12 | Learned abilities, one bit each |
| `0x01EC` | 88 | Training level of each ability, 0 to 100 |
| `0x0244` | 11 | Training level of each weapon type, 0 to 100 |

### Identity

Every game has an identity, and it matters for the 2P hero feature.

| Offset | Size | Contents |
|---|---|---|
| `0x024F` | 1 | Compatibility tag; `0xFF` in ordinary saves (see below) |
| `0x0254` | 2 | Game id, a random number picked when you start a new game |
| `0x0256` | 2 | Save id, a random number picked each time you save |
| `0x0258` | 4 | Game id and save id of the save your current guest hero came from |
| `0x025E` | 2 | Set to 1 once a guest hero has been loaded |
| `0x0260` | 4 | World map cell chosen on the world select screen (column + row x 19) |

When a friend loads your hero into their game, the 2P screen checks the game id
first. If it matches their own game, it refuses, because it's the same hero.
Later, when they save the guest back, the screen looks for the save whose game
id and save id match `0x0258`. That's how it finds your file again even if the
card has other Legend of Mana saves on it. If you edit these values, the guest
can't go home.

The byte at `0x024F` is a compatibility tag (`compatibility_tag` in the code).
Starting a new game sets the running game's tag to `0xFF`, loading a
save takes the tag from that save, and saving writes the current tag back. So
ordinary saves all carry `0xFF`. The load screens refuse a save whose tag
doesn't match the running game's, unless one of the two is `0xFF`; that's where
"Save Data version is wrong" comes from.

### World state

| Offset | Size | Contents |
|---|---|---|
| `0x0264` | 512 | 128 script variables, 32 bits each |
| `0x0464` | 1 | Number of lands placed so far |
| `0x0465` | 1 | Hero level |
| `0x0466` | 2 | Day of the week (low 7 bits) |
| `0x0468` | 8 | Event flag bits |
| `0x0470` | 768 | 64 land records, 12 bytes each |

### Party

There are three party slots, each a 592-byte (`0x250`) character record:

| Offset | Slot |
|---|---|
| `0x0770` | Hero |
| `0x09C0` | Guest (the 2P hero, when one has joined) |
| `0x0C10` | Companion (a pet or golem) |

Inside a character record:

| Offset in record | Size | Contents |
|---|---|---|
| `0x00` | 24 | Name; an empty name means the slot is empty |
| `0x18` | 1 | Character type in the low 7 bits; bit 7 is set when a controller drives the character |
| `0x50` | ... | Equipment |

A golem companion has type 4.

Bit 7 decides who moves the character. When it's set, a player's controller
drives it; when it's clear, the game moves it as a computer-controlled
companion. When a friend loads a hero into the guest slot, the game keeps the
guest slot's bit, so the guest stays under the same control. When the guest is
saved back, the bit is set, because that hero goes back to being the other
player's own hero.

### Items

| Offset | Size | Contents |
|---|---|---|
| `0x0E60` | 6400 | 100 item records, 64 bytes each |
| `0x2760` | 256 | How many of each consumable item you hold |

### Golems and pets

| Offset | Size | Contents |
|---|---|---|
| `0x2B54` | 1 | Low 4 bits: number of golems; high 4 bits: the golem that last left the party |
| `0x2B55` | 1 | Golems created so far, stops at 200 |
| `0x2B56` | 1 | Number of logic blocks in use |
| `0x2B57` | 1 | Golem in the party, or 3 for none |
| `0x2B58` | 3 | Display order of the three golem records |
| `0x2B5C` | 160 | 40 logic blocks, 4 bytes each |
| `0x2BFC` | 144 | Logic block grid of the golem companion |
| `0x2C8C` | 996 | 3 golem records, 332 bytes each |
| `0x3070` | 4 | Pet in the party, or 5 for none |
| `0x3074` | 480 | 5 pet records, 96 bytes each |

A golem's color (0 to 31) is at `0x48` in its record.

In a pet record, the name is the first 21 bytes, the species is at `0x15`, and
a unique id is at `0x5C`. Bit 31 of the status word at `0x44` means the pet is
still an egg, and the egg's hatch counter is at `0x42`.

### Everything else

| Offset | Size | Contents |
|---|---|---|
| `0x3254` | 128 | Encyclopedia entries unlocked, one bit each |
| `0x32DC` | 4 | Battle retry counter; -1 stops counting |
| `0x32E0` | 256 | 4 summary records, 64 bytes each; the first byte is nonzero while one is in use |

## The checksum

If you change anything in the first `0x33E0` bytes, you have to fix the
checksum, or the game will refuse the save.

The checksum adds up every byte from `0x0000` to `0x33DF`, doubles the total,
and adds `0x0414E410`. The result goes at `0x33E0` as a 32-bit little-endian
number. The magic value at `0x33E4` must stay `41 4E 41 00`.

Here it is in Python:

```python
import struct

with open("save.bin", "rb") as f:
    data = bytearray(f.read())

checksum = (sum(data[:0x33E0]) * 2 + 0x0414E410) & 0xFFFFFFFF
struct.pack_into("<I", data, 0x33E0, checksum)

with open("save.bin", "wb") as f:
    f.write(data)
```

The header is part of the checksum too, so changing the title or the icon also
needs a new checksum.

## In memory

While the game runs, the saved game (everything from `0x0180` on, without the
card header) is kept in RAM at `0x80042FD8` in the US release and `0x80043140`
in the Japanese one. To find a value in an emulator's memory viewer, take its
offset on this page, subtract `0x180` and add that address. Money at `0x01AC`,
for example, is at `0x80043004` in the US release.

## What we don't know yet

- **Where a tag other than `0xFF` comes from.** The game sets its tag to 7
  when it starts up, in both releases, but ordinary play replaces that with
  `0xFF` or a loaded save's tag. We haven't found what writes any other value.
- **The `+` saves.** We know which flag controls the `+`, but not what it
  means to the player.
- **Byte 3 of a land record.** The code stores the placement order there, but
  we haven't checked every place that reads it, so it may hold more than that.
- **The rest of a pet's status word.** Only bit 31, the egg flag, is mapped.
  The other bytes at `0x44` in a pet record aren't named yet.
- **The gaps.** A few ranges, such as `0x0250` to `0x0253`, `0x025C` to
  `0x025D`, and the space around the menu data between the item counts and the
  golem data, aren't mapped yet.

The C definitions behind this page are in
[saved_game.h](../../../../include/common/saved_game.h), if you'd rather read the
structures directly.
