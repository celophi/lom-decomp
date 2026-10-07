# Text tables

[English documentation](../../README.md) | [日本語](../../../jp/technical/reference/text-tables.md) | [Save file format](save-file.md) | [Compressor](../../../../tools/compression/README.md)

Most of the words you see on Legend of Mana's menu screens don't live in the
code. They sit in small string tables inside each overlay, and the code asks
for "string number 26" rather than holding the text itself. If you want to
translate a screen, fix a typo or just find out what a message says, these
tables are where to look.

This page describes how the tables are laid out and how the US text is encoded.
It uses the 2P hero screen (ADDHERO) as the example, because its table was the
first one we decoded completely.

## Finding a table

If you only want to read ADDHERO's text, there's a shortcut. After `make splat`,
`make extract-addhero` writes every message and location name to YAML files,
along with the party icons as PNGs. `make extract-carda` does the same for the
save screen, including its item names and memory card icons. Add `VERSION=jp`
to either command for the Japanese data. See the
[overlay resource extractor](../../../../tools/data/overlays/README.md) for the
details. The rest of this section is for working with the bytes directly.

Overlays are stored compressed on the disc. The first byte of the disc file is
a format tag, and the compressed data starts right after it. The project's
decoder unpacks it:

```sh
size=$(stat -c %s disc/us/BIN/ADDHERO.BIN)
python3 tools/compression/decompress.py disc/us/BIN/ADDHERO.BIN 1 $((size - 1)) addhero.bin
```

The decompressed file is the overlay exactly as it sits in memory. An overlay
loaded at `0x80140000` has its first byte at that address, so a table at
`0x80146FA4` starts at offset `0x6FA4` in the decompressed file.

| Table | Release | Address | Offset in the decompressed file | Entries |
|---|---|---|---|---|
| ADDHERO messages | US | `0x80146FA4` | `0x6FA4` in ADDHERO.BIN | 91 |
| ADDHERO messages | Japan | `0x80146F9C` | `0x6F9C` in ADDHERO.BIN | 90 |
| ADDHERO location names | US | `0x80147470` | `0x7470` in ADDHERO.BIN | 63 |
| ADDHERO location names | Japan | `0x801475B0` | `0x75B0` in ADDHERO.BIN | 63 |
| FIELD menu strings | US | `0x800EC3C4` | `0x9C754` in FIELD.BIN | 37 |
| FIELD menu strings | Japan | `0x800EBF90` | `0x9C1B8` in FIELD.BIN | 37 |

FIELD loads at `0x8004FC70` in the US release and `0x8004FDD8` in the Japanese
one, which is why its offsets are smaller than its addresses by that amount.
For the Japanese release, decompress from `disc/jp/BIN/` instead.

The location names are the ones the save screens show next to each save. They
are picked by the save's music track (see the [save file format](save-file.md)).

## How a table is laid out

A table is a list of 16-bit offsets followed by the strings:

```text
offset of string 0   (2 bytes)
offset of string 1   (2 bytes)
...
offset of string N-1 (2 bytes)
string 0, then a zero byte
string 1, then a zero byte
...
```

Each offset counts from the start of the table, not from the start of the file.
There's no entry count anywhere. The first offset points just past the last
offset, so dividing it by 2 tells you how many strings there are. In the
ADDHERO message table, the first offset is `0xB6`, which gives 91 strings.

To read string 26, take the 16-bit value at table start + 26 x 2, add it to the
table start, and read until the zero byte.

Some offsets can point at an empty string. ADDHERO has several unused entries
like that, and one message that's empty in the US release but has text in the
Japanese one.

## US text encoding

US strings are mostly plain ASCII, so you can often read them straight from a
hex editor. The bytes below `0x20` are where it gets interesting. This is how
FIELD's text renderer treats them:

| Bytes | Meaning |
|---|---|
| `0x00` | End of string |
| `0x01` to `0x13` | Formatting commands: new line, wait for a button, clear, choices, runs of spaces, delays, color changes, indent |
| `0x14` to `0x18`, `0x1A` to `0x1E` | A common piece of a word, stored once in a dictionary |
| `0x19` + 1 byte | A two-byte character |
| `0x1F` + 1 byte | A dictionary entry beyond the one-byte ones |
| `0x20` to `0x7F` | ASCII |
| `0x80` and up | Special characters with a fixed width, such as the symbols in Yes/No prompts |

The dictionary is a simple compression trick. Words like "in" and "is " show up
all over the game's text, so they get a single byte each. For example, byte
`0x16` stands for "in": the bytes for `Load`, `0x16` and `g` read as "Loading".
That's why a string in a hex editor often looks like English with a few odd
bytes where letters should be.

The dictionary only exists in the US release, and the game loads it into RAM
from somewhere we haven't documented yet. The renderer in
[field_text.c](../../../../src/overlays/field/ui/field_text.c) is the authority if
a byte doesn't behave the way this table says.

### Japanese text

The Japanese release uses the same table layout, but its characters aren't
ASCII. A byte from `0x20` up is one character, mostly kana. A byte from `0x19`
to `0x1F` plus the next byte is a kanji: the first byte picks one of seven pages
of 256 characters, and the second byte picks the character.

The overlays that write memory card titles carry the whole character set as a
chart, because they convert names to Shift-JIS. ADDHERO has a copy too, and
`make extract-addhero VERSION=jp` uses it to print the Japanese messages as
real text. The chart itself ends up in `tables/text_conversion.yaml`.

### Memory card titles are different

The save titles you see in a memory card manager aren't game text. They're
stored in the card header in Shift-JIS, the standard Japanese encoding the
PlayStation BIOS uses. When the game saves, it builds the title from the play
time and the hero's name and converts it to Shift-JIS. The 2P hero screen draws
these titles with the BIOS font when it lists a save from another game. See the
[save file format](save-file.md#memory-card-header) for where the title is
stored.

## Editing a table

Changing a string without changing its length is easy: overwrite the bytes and
keep the zero at the end.

Anything longer takes more care:

- **The later strings have to move.** If string 26 gets longer, every string
  after it shifts, and every offset after entry 26 has to be updated.
- **The table can't grow past its space.** Tables are packed in with the rest
  of the overlay's data. The bytes after the last string belong to something
  else, so the whole table has to fit in its original size unless you move it.
- **The code finds strings by number.** It never looks at the text, so the
  order of the offsets has to stay the same. Keep the number of entries the
  same, even for strings you don't use.
- **The overlay has to be compressed again.** The disc copy is compressed, so
  an edited overlay needs to go back through the
  [compressor](../../../../tools/compression/README.md) before the game can load
  it.

Shorter strings are the safe option. Pad them with the zero byte and leave the
offsets alone.

## Where the code uses them

In the source, each string the code uses by name gets a symbol, such as
`g_addhero_text_no_card` for the "No memory card" message. The ADDHERO names
and what each message is for are listed in
[addhero_internal.h](../../../../src/overlays/addhero/internal/addhero_internal.h). If
you're hunting for a message, searching the overlay's source for a string's
symbol name is usually quicker than scanning the table.
