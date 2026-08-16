# Overlay compressor

Legend of Mana stores its overlays on the disc in a compressed form
(`disc/BIN/*.BIN`) using a proprietary LZ + pattern-opcode scheme. This
directory holds the encoder that reproduces those streams **byte for byte**.

Reproducing the exact byte stream matters because many different encodings
decompress to the same data, but only one of them is the stream the original
1999 compressor emitted. Byte-exactness is what lets the build prove a rebuilt
overlay is an exact replica of the file on the disc, rather than merely an
equivalent one.

## Files

| File | Purpose |
|------|---------|
| `compressor.py` | The byte-exact encoder. Importable as a library (`compress(data) -> bytes`) or runnable as a CLI. |
| `verify_exact_bins.py` | Regression check: decompress each original overlay, recompress it, and compare byte for byte. |

The reference **decoder** lives at `tools/splat_ext/decompress.py`. It was
ported from the game's own assembly and is treated as ground truth - do not
modify it.

## Stream layout

A disc overlay is:

```
byte 0      0x01   compression-format tag
byte 1..           the compressed stream
```

`compressor.py` produces and consumes the stream *without* the leading tag
byte; the build prepends `0x01` when assembling the final `.BIN`.

The stream is a sequence of opcodes. Values `0x00`-`0xEF` are literal runs
copying `opcode + 1` bytes verbatim. The rest are:

| Opcode | Meaning |
|--------|---------|
| `F0` | Nibble RLE |
| `F1` | Byte RLE |
| `F2` | Nibble-pair RLE |
| `F3` | Byte-pair RLE |
| `F4` | Byte-triplet RLE |
| `F5` | Fixed byte interleaved with a variable byte |
| `F6` | Two fixed bytes interleaved with a variable byte |
| `F7` | Three fixed bytes interleaved with a variable byte |
| `F8` | Ascending byte run |
| `F9` | Descending byte run |
| `FA` | Arithmetic byte run with an arbitrary step |
| `FB` | Signed-delta 16-bit run |
| `FC` | Back-reference, 12-bit window |
| `FD` | Back-reference, 8-bit window, long counts |
| `FE` | Compact back-reference, 8-byte-aligned distances |
| `FF` | End of stream |

The original compressor's source does not exist. Its *selection* rules - which
opcode wins when several apply, how ties break, and the quirks in its scan
limits - were recovered by comparing candidate output against all 17 reference
overlays opcode by opcode. Those rules are documented inline in
`compressor.py`; several are deliberate compatibility behaviour rather than
anything the format requires, and they are marked as such.

## Usage

As a CLI:

```bash
python3 tools/compressor/compressor.py <raw-input> <compressed-output>
```

As a library:

```python
from compressor import compress
stream = compress(raw_bytes)
```

## Verifying

Check every overlay round-trips exactly (no build or toolchain needed - it
works straight from `disc/BIN`):

```bash
python3 tools/compressor/verify_exact_bins.py
```

Or a subset, which is much faster while iterating:

```bash
python3 tools/compressor/verify_exact_bins.py GOVER MOVIE GNAME CHECKPS
```

`FIELD`, `TITLE`, `WMAP` and `WSEL` are the large ones and dominate the runtime
of a full sweep.

This check must stay green after any change to `compressor.py`. It is stricter
than the build's own check, because it covers all 17 overlays rather than only
the ones that currently link.

## Where the build uses this

`mk/verification.mk` compresses each fully-linked overlay's raw image, prepends
the `0x01` tag, and SHA1-compares the result against `disc/BIN/<NAME>.BIN`. On
a match the overlay is recorded in `build/complete_overlays.txt`, which the
objdiff config generator reads to stamp those units complete. Run it with:

```bash
make verify-bins
```
