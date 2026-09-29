# English documentation

[Documentation index](../README.md) | [Project README](../../README.md)

## Technical architecture

These guides begin with a high-level explanation, then cover implementation,
ownership, timing, and known limitations. Source links accompany the details.

- [Scene layouts and conditions](technical/architecture/scene-layouts.md) - how scene
  records decide what appears, with an ordinary chest as a worked example.
- [CD-ROM subsystem](technical/architecture/cdrom.md) - resource requests,
  command queueing, streaming, decompression, and drive recovery.
- [MOVIE overlay](technical/architecture/movie.md) - video decoding, audio
  streaming, buffer ownership, callbacks, and FIELD integration.
- [CARDA overlay](technical/architecture/carda.md) - saving, loading and
  PocketStation pet transfers, from the card screen back to FIELD.
- [ADDHERO overlay](technical/architecture/addhero.md) - the 2P hero screen:
  loading a friend's hero from a memory card and saving it back.

## Technical reference

- [Tools index](../../tools/README.md) - asset conversion, scene extraction and tests.
- [Scene extractor](technical/reference/scene-extractor.md) - scene IMG assets and byte maps.
- [Disc layout](technical/reference/disc-layout.md) - disc organization and resource tables.
- [Save file format](technical/reference/save-file.md) - what's inside a memory card save
  and how to fix its checksum.
- [Text tables](technical/reference/text-tables.md) - where menu text lives and how the US
  text is encoded.
- [Overlay ID prefix](technical/reference/overlay-id-prefix.md) - overlay identification and
  binary layout.
- [Compressor](../../tools/compressor/README.md) - compression format and tools.
