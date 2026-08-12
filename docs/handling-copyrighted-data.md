# Handling Copyrighted Data

Within some `.BIN` files, creative work is embedded that the code accesses in a structured way. 
Because this content is copyrighted, it cannot be committed to the repository; 
it is nonetheless valuable to represent the structure of that data in source, 
so that the decompiled code which reads it stays clear and readable. 
What is required, therefore, is an architecture that commits the structure of the data while keeping 
the copyrighted content out of source control, generating that content locally from the original game files.

---

## The limitation of the direct approach

The straightforward way to keep copyrighted bytes out of source control is splat's
`databin` mechanism: the raw bytes are extracted to a gitignored file and pulled
into the build with an `.incbin` directive. This satisfies the copyright
constraint, but it contributes nothing to readability. The data enters the build
as an opaque blob. It cannot be inspected, it cannot be modified, and its presence
says nothing about whether its layout is understood. A blob of the correct size is
indistinguishable from a correct decompilation.

The architecture described here keeps the same copyright guarantee while
representing the data as ordinary, typed C, so that the surrounding code reads
naturally and the data itself becomes a first-class part of the decompilation.

## Approach

The data is separated into two parts.

The first is its **structure**: the type definitions, field names, and symbol
layout that describe how the bytes are organized. This is original
reverse-engineering work, contains no creative content, and is committed to the
repository.

The second is its **content**: the actual values. This is the copyrighted
material and is never committed. A generation step reads the original disc image,
which every contributor already possesses locally, extracts the content, and
writes it into gitignored files. The committed C sources include those files.

Consequently, the structure is versioned in the repository while the content is
regenerated on each machine and never stored in source control. Because the
content is compiled from C rather than pasted in as a blob, an incorrect structure
produces incorrect bytes, and the error is caught by the verification steps below.

## Architecture

There are three components.

### 1. splat configuration

The data subsegment is declared with a `.data` marker rather than `databin`. For
the GNAME overlay, this is:

```yaml
- [0x2C99, .data, gname_data]
```

This is a linker-only marker, of the same family splat already uses for `.rodata`
and `.bss`. It instructs splat to treat the range as belonging to a compiled C
object rather than a raw blob. splat regenerates the linker script to draw the
section from `src/overlays/<ov>/<name>.o` and emits no `.incbin`. No generated
linker script or assembly file is edited by hand; the behaviour follows from the
single configuration change.

### 2. Source layout

The overlay gains a committed data source, `<name>_data.c`, containing one typed
declaration per data symbol, in ascending address order. Each declaration includes
a gitignored fragment that supplies its values. Where useful, a shared header
defines the record types so that the data source and the code that reads the data
agree on their layout. For example:

```c
GlyphInfo g_glyph_table[] = {
#include "gen/g_glyph_table.inc"
};
```

The committed file therefore expresses only the structure; the included fragment,
which is gitignored, supplies the content.

### 3. Data generation

A per-overlay script, `tools/gen_<ov>_data.py`, decompresses the overlay, divides
the data region into one fragment per symbol using the symbol map, and writes the
gitignored fragments. It is invoked automatically by `make splat` through
`mk/overlay-data.mk`, so it is never run directly; `make splat` produces the
fragments alongside the assembly and linker scripts it already generates.

```
  original disc image (present locally, gitignored)
        |
        |  generation: decompress, then slice per symbol
        v
  gitignored value fragments  ---+
                                 |
  committed C structure  --------+--> compiler --> data bytes --> overlay
```

## Verification

Two checks belong to this architecture.

### Compilation

The structure is valid C and produces a section of the expected size. This is the
minimum bar.

### Structural comparison (objdiff)

objdiff compares the compiled result against a known-correct reference and reports
a match percentage. For code, splat produces that reference automatically, since
the disassembly is the reference. For data, splat does not; therefore the
generation step additionally emits a gitignored reference object containing the
original bytes under the same symbol labels, and the objdiff configuration is
extended with a unit that compares this reference against the compiled data. Each
data symbol is then graded individually.

It should be noted that objdiff's data percentage is a heuristic rather than a
definitive measure. It operates on pre-link objects and pairs symbols by name, and
can therefore understate data that is in fact correct - a compiler-generated jump
table, for instance, reports a mismatch even when the linked overlay is
byte-perfect. It is best read as a progress indicator, not as proof.

### A note on whole-overlay correctness

Whether the overlay as a whole reproduces the original is established separately
from this architecture and is a property of the overlay rather than of the data:
the overlay is rebuilt, compressed, and its SHA1 is compared against the original
`.BIN` file (the `verify-bins` flow), exactly as for any other overlay. That check
covers code and data together and is the authoritative measure of correctness.

Where a working compressor for an overlay is not yet available - as is currently
the case for GNAME - a byte-for-byte comparison of the decompressed image serves as
a temporary stand-in (`make verify-<ov>`) until the compressed comparison can be
performed. This is a stopgap for the missing compressor, not a component of the
data architecture.

## Applying this to a new overlay

1. Change the data subsegment to a `.data` marker in the overlay's yaml, and remove
   the stale `databin` assembly file, which splat leaves in place.
2. Add the committed data source (typed declarations that include the fragments),
   route it in `mk/overlay-registry.mk`, and register the overlay in
   `mk/overlay-data.mk`.
3. Write the generation script: decompress the overlay, slice the region per
   symbol, and write the gitignored fragments. It should also emit the objdiff
   reference assembly.
4. Confirm the generated outputs are gitignored; the `gen/` directory and `asm/`
   already are.
5. No change to the objdiff configuration generator is required: it already emits a
   unit for a standalone `.data` subsegment once the reference object is built.
6. Build, then grade the data with `make verify-<ov>-data` (objdiff). Whole-overlay
   correctness is confirmed separately by the overlay's own verification (the
   compressed-BIN SHA1 comparison, or a decompressed comparison where no compressor
   exists yet).

## Considerations

- **A region may be a serialized blob split across symbols.** These regions are
  frequently a single packed file that splat has carved into separate symbols, and
  the code sometimes indexes past one symbol into the next. The structure must
  account for this rather than assuming each symbol is an independent array.
- **Some content has no meaningful structure.** Raw pixel or image data has no
  layout beyond a header. Such regions are best represented as byte arrays.
- **Copyrighted bytes exist only in gitignored files** - the value fragments and
  the objdiff reference assembly. Both the `gen/` directory and `asm/` are
  gitignored, so the committed structure source and the generation script are the
  only new tracked files.
- **A green objdiff result is not equivalent to completion.** objdiff grades the
  data in isolation, and its data percentage is heuristic. Whole-overlay
  correctness is established by the overlay's own verification, not by this
  architecture.
