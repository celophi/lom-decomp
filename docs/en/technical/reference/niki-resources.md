# NIKI resources

[English index](../../README.md) | [Overlay extractors](../../../../tools/data/overlays/README.md) | [Japanese](../../../jp/technical/reference/niki-resources.md)

NIKI is the diary screen. FIELD opens it through `field_open_niki`, and it
works with the saves on either memory card. In mode 0 it lists the saves on a
card, shows the details of the selected one and loads it. In its other mode it
skips the list and works on one save: it reads the save back, copies a
256-byte block from memory into it and writes it again.

All of NIKI's initialized data sits in one `niki_data` databin, in both
regional YAMLs. It's the same set of card-screen resources ADDHERO carries:
messages, two card title templates, location names, party icons, card step
sequences, the character chart and the digit glyphs. The build links the blob
unchanged. To see what's inside, extract the splat assets first, then run:

```sh
make splat
make extract-niki

make splat VERSION=jp
make extract-niki VERSION=jp
```

Files go to `assets/exports/<version>/overlays/niki/`. Set
`NIKI_OUTPUT=/path/to/new-folder` to use another destination. The folder must
be new. The extractor reads everything first, then writes a temporary folder
and moves it into place when it's finished, so a failed run leaves nothing
behind. Editing the export doesn't change what the build links.

## What comes out

| Path | Contents |
| --- | --- |
| `byte-map.yaml` | Every range in the blob, including padding, the repeated icon word and the variables |
| `text/messages.yaml` | 91 US or 90 JP messages, with table indices, symbol names and raw bytes |
| `text/card_titles.yaml` | The two Shift-JIS memory card title templates, with what each one is for |
| `text/locations.yaml` | 63 location names, picked by the music track stored in a save |
| `icons/icons.yaml` | Icon ids, what kind of character each is, offsets and stored palettes |
| `icons/icon_00.png` ... | The 86 party icons, 48 x 48 each |
| `tables/card_steps.yaml` | Eight memory card sequences, with step names from `CardMenuStep` and `CardMenuExchangeStep` (card_menu.h) |
| `tables/text_conversion.yaml` | The character chart: what each text code becomes in Shift-JIS |
| `tables/digit_glyphs.yaml` | Full-width decimal and hexadecimal digits |

Neither version has an `unknown/` folder. Every byte of the blob is either a
decoded resource, zero padding or a variable. If a future change turns up
nonzero bytes in a range the tool doesn't recognize, it saves them under
`unknown/`, named by address.

## Blob layout

Addresses are the RAM addresses the overlay runs at. The blob is 158,088 bytes
in US and 161,652 bytes in JP.

| Range | US | JP | Symbol |
| --- | --- | --- | --- |
| Messages | `0x801470F8`, 0x499 bytes | `0x801470E4`, 0x5E1 bytes | `g_niki_text_table` |
| Card title templates | `0x80147594`, two of 0x15 bytes | `0x801476C8`, two of 0x15 bytes | none |
| Locations | `0x801475C4`, 0x1E0 bytes | `0x801476F8`, 0x175 bytes | `g_niki_location_names` |
| Party icons | `0x801477A8`, 0x18F1C bytes | `0x80147870`, 0x18F1C bytes | `g_niki_icon_offsets` is 4 bytes in |
| Card steps | `0x801606C8`, 0x34 bytes | `0x80160790`, 0x34 bytes | `g_niki_card_setup_sequence` |
| Character chart | `0x801606FC`, 0x338 bytes | `0x801607C4`, 0x1044 bytes | `g_glyph_single_byte_chart` |
| Digit glyphs | `0x80160A34`, 0x3C bytes | `0x80161808`, 0x40 bytes | `g_glyph_decimal_digits` |
| Variables | `0x80160A70`, 0xD010 bytes | `0x80161848`, 0xD010 bytes | `g_niki_dialog_state` |

Between these are 13 bytes of zero padding in US and 12 in JP, plus the four
bytes after the icon set that repeat its last word.

## Text

Both text tables use the usual overlay format: a list of u16 offsets from the
start of the table, then zero-terminated strings. The
[text tables](text-tables.md) page covers the codes. US text is shown as ASCII,
with dictionary pieces and control codes in braces, like `{16}` for "in". JP
text is decoded through NIKI's own character chart, so no other overlay is
needed. Every entry keeps its original bytes next to the text.

The message table is a copy of the one ADDHERO, CARDA and CLOAD carry, so it
also contains text NIKI never shows. NIKI draws about 30 of the entries. Those
have a `g_niki_text_*` symbol for their slot in the offset table, and the
export puts that name next to the entry, for example
`g_niki_text_select_save_data` on entry 34. The yes/no choice text comes from
FIELD, not from this table.

Entries 29 and 33 are empty in US. JP has "Ring Ring Land" and "This is not a
PocketStation" there. NIKI uses entry 29 as the list label for Ring Ring Land
files, so those rows have no label in US. Entry 33 belongs to status dialog 3,
which only opens when `CARD_MENU_STEP_CHECK_POCKETSTATION` fails.

The card title templates are the two Shift-JIS titles a memory card save uses.
NIKI keeps the title a save already has and never writes these, so this is
just a copy of what CARDA uses.

## Icons

The icon set starts with a count (86) and a list of byte offsets counted from
that count word. Each icon is a 16-color palette followed by a 48 x 48, 4-bit
image, low nibble first. NIKI uploads the palette and pixels for the selected
save's party. The PNGs use the stored palette with color 0 transparent. The
guest hero and the golems are drawn with palettes the game builds at runtime,
so those icons don't look right here.

## Card steps

A card sequence is a list of one-byte commands, run in order until
`CARD_MENU_STEP_DONE` (0). The export follows every symbol in the table to its
stop byte and names each command from the `CardMenuStep` and
`CardMenuExchangeStep` enums in `include/common/card_menu.h`, which ADDHERO,
CARDA and CLOAD share. `g_card_steps_idle` holds `CARD_MENU_STEP_WAIT` (14),
which has no handler, so the sequence waits there until the menu picks another
one.

One byte in the table is never run. The byte just before
`g_niki_write_save_sequence` (`0x801606F4` in US, `0x801607BC` in JP) is
`CARD_MENU_STEP_CLEAR_SOFTWARE_EVENTS`, but the code starts the write sequence one byte
later. The export lists it under `unreached`, and the whole table is kept in
`bytes`.

## Chart, digits and variables

The character chart has 33-byte rows: 16 two-byte Shift-JIS characters and a
newline. One-byte codes from 0x20 index the first rows. Lead bytes 0x19 to 0x1F
pick a 16-row page counted from `g_glyph_chart_page_base`. That symbol is where
page 0 would be, and since no page below 0x19 exists, it lands inside the icon
set. The JP chart has all seven pages. The US chart is only 824 bytes: the
one-byte rows and most of page 0x19. The export lists the rows that fall inside
the chart and keeps the stored lines as they are. NIKI includes the conversion
helper that reads this chart, but never calls it. The extractor still uses it
to decode the JP text.

The digit glyphs are full-width `0` to `9`, then `0` to `F`, each list ended by
a zero pair.

The last 53,264 bytes are NIKI's variables and buffers: the dialog state, the
save transfer buffer, the card directory entries and the glyph cache, ending
with `g_glyph_cursor_y`. They're all zero on the disc. The byte map lists the
range without writing another file.

The `bu00:` device prefix, the `bu00:*` search path and the full-width `MAX`
text aren't in this blob. The paths are defined in
[`niki.c`](../../../../src/overlays/niki/niki.c) and
[`niki_io.c`](../../../../src/overlays/niki/niki_io.c). `MAX` is a string in
the shared [`format_decimal.inc.c`](../../../../src/common/sjis/format_decimal.inc.c).

## What we don't know yet

- TODO: what the 256-byte block NIKI's other mode copies into a save holds. It
  is four 64-byte records, and that page asks message 57, "Trade data will not
  be saved. Is this OK?" (`g_niki_text_trade_data_not_saved`).
- TODO: what mode 1 was for. `niki_draw_header_label` shows message 35,
  "Select Item.", in mode 1, but NIKI only creates the header in mode 0.
- TODO: `CARD_MENU_STEP_CHECK_POCKETSTATION` has a handler, but no sequence in
  the table uses it, so it's unclear when status dialog 3 can appear.

[`niki.py`](../../../../tools/data/overlays/niki.py) reads the blob in address order
with the card readers ADDHERO, CARDA and CLOAD share. It gets addresses from
each version's symbol file and command names from the C enum. The tests check
both regional layouts and the chart and icon sizes against the `#define`s and
structs in `niki_internal.h`.
