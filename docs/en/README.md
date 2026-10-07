# English documentation

[Documentation index](../README.md) | [Project README](../../README.md)

## Technical architecture

These guides begin with a high-level explanation, then cover implementation,
ownership, timing, and known limitations. Source links accompany the details.

- [Scene layouts and conditions](technical/architecture/scene-layouts.md) - how scene
  records decide what appears, with an ordinary chest as a worked example.
- [CD-ROM subsystem](technical/architecture/cdrom.md) - resource requests,
  command queueing, streaming, decompression, and drive recovery.
- [CHECKPS overlay](technical/architecture/checkps.md) - the startup screen,
  JP CD checks, drive handoff and hardware-modification warning.
- [MOVIE overlay](technical/architecture/movie.md) - video decoding, audio
  streaming, buffer ownership, callbacks, and FIELD integration.
- [CARDA overlay](technical/architecture/carda.md) - saving, loading and
  PocketStation pet transfers, from the card screen back to FIELD.
- [ADDHERO overlay](technical/architecture/addhero.md) - the 2P hero screen:
  loading a friend's hero from a memory card and saving it back.

## Technical reference

- [Tools index](../../tools/README.md) - asset conversion, scene extraction and tests.
- [Scene extractor](technical/reference/scene-extractor.md) - scene IMG assets and byte maps.
- [ZUKAN resources](technical/reference/zukan-resources.md) - encyclopedia textures, UI sprites, entry names and category tables.
- [WSEL resources](technical/reference/wsel-resources.md) - play-area screen images, sprite layers, hero poses and land-map grid tables.
- [WMAP resources](technical/reference/wmap-resources.md) - world-map tables, sound effects, input scripts and step tables.
- [TITLE resources](technical/reference/title-resources.md) - title menu artwork, the character and weapon screen, starting weapons and new-game states.
- [SHOP resources](technical/reference/shop-resources.md) - shop item text, equipment type names, instrument spells and sell prices.
- [NIKI resources](technical/reference/niki-resources.md) - diary save-screen text, party icons, card steps and character charts.
- [MENU resources](technical/reference/menu-resources.md) - in-game menu texture, icons, text tables, page layouts and input scripts.
- [GOSUB resources](technical/reference/gosub-resources.md) - workshop text, companion portraits, UI glyphs and equipment tables.
- [GOLEM resources](technical/reference/golem-resources.md) - logic-grid artwork, block text, glyphs and packed panel records.
- [GNAME resources](technical/reference/gname-resources.md) - name-entry artwork, character panels, name lists and layout tables.
- [FIELD resources](technical/reference/field-resources.md) - resident images, text, animation data and gameplay tables.
- [CLOAD resources](technical/reference/cload-resources.md) - load-screen text, card steps and character charts.
- [Disc layout](technical/reference/disc-layout.md) - disc organization and resource tables.
- [Save file format](technical/reference/save-file.md) - what's inside a memory card save
  and how to fix its checksum.
- [Text tables](technical/reference/text-tables.md) - where menu text lives and how the US
  text is encoded.
- [Overlay ID prefix](technical/reference/overlay-id-prefix.md) - overlay identification and
  binary layout.
- [Compressor](../../tools/compression/README.md) - compression format and tools.
