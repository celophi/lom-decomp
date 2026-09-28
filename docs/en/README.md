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

## Technical reference

- [Tools index](../../tools/README.md) - asset conversion, scene extraction and tests.
- [Scene extractor](technical/reference/scene-extractor.md) - scene IMG assets and byte maps.
- [Disc layout](technical/reference/disc-layout.md) - disc organization and resource tables.
- [Overlay ID prefix](technical/reference/overlay-id-prefix.md) - overlay identification and
  binary layout.
- [Compressor](../../tools/compressor/README.md) - compression format and tools.
