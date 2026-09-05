# Overlay ID Prefix

Status: implemented. Every overlay with C sources in the build carries its
header word in a dedicated `overlay_header.c` translation unit that is linked
first. The nine verified overlays reproduce their disc SHA1s under this layout.

## Summary

Legend of mana has multiple disc overlays that all start with a 4-byte word representing 
that specific overlays's module ID number. Immediately following that value, the overlay's 
code starts after it at offset `0x4`.

Current research shows that nothing at runtime reads the value, and the main executable 
streams a given overlay to a specific load address. Fixed addresses are called within it. 
So, the leading hypothesis at this time is that the overlay ID prefix is an artifact 
of a tooling system, and not something specific to the game code.

Each overlay thus models this as a separate file named `overlay_header.c` within the overlay folder.

## Background

### Overlay ID Placement & Value

| Overlay | ID  | Load address | Word at offset 0x4 |
|---------|-----|--------------|------------------|
| FIELD   | 4   | 0x8004FC70   | 0x80053EE0 (pointer) |
| GNAME   | 5   | 0x80140000   | 0x27BDFFC8 (first instruction) |
| MENU    | 6   | 0x80140000   | 0x8014198C (jump table) |
| SHOP    | 7   | 0x80140000   | 0x27BDFFD0 (first instruction) |
| ZUKAN   | 8   | 0x80140000   | 0x80141C5C (jump table) |
| GOLEM   | 9   | 0x80140000   | 0x80141C9C (jump table) |
| GOVER   | 10  | 0x80140000   | 0x27BDFFC0 (first instruction) |
| GOSUB   | 11  | 0x80140000   | 0x8014028C (jump table) |
| CARDA   | 12  | 0x80140000   | 0x80140B98 (jump table) |
| WSEL    | 13  | 0x8004FC70   | 0x800508E0 (pointer) |
| MOVIE   | 14  | 0x80140000   | 0x8014011C (jump table) |
| WMAP    | 15  | 0x8004FC70   | 0x001C008B (data) |
| TITLE   | 16  | 0x8004FC70   | 0x27BDFFD8 (first instruction) |
| CHECKPS | 17  | 0x8004FC70   | 0x80051424 (pointer) |
| CLOAD   | 18  | 0x80140000   | 0x80140E14 (jump table) |
| NIKI    | 19  | 0x80140000   | 0x80140DBC (jump table) |
| ADDHERO | 20  | 0x80140000   | 0x80140DF0 (jump table) |

*At this time, no understanding of what values 0 to 3 are.*

### SKCDPOSE

The overlay ID numbers do not seem to correlate directly to the `SKCDPOSE.DAT` 
resource table on disc. Those values are for the purpose of locating specific files 
on the disc.

### Existing Byte Prefix

Each `.BIN` on disc is a compressed image with a one-byte format tag in front:

    byte 0      0x01   compression-format tag
    byte 1..    compressed payload

This tag is noted to be different from the overlay ID prefix because the tag 
never gets loaded into RAM, and it's completely consumed by the decompressor. 
However, the overlay ID prefix does indeed exist in RAM. Therefore, it's more 
than likely not part of the compressor/decompressor logic.

## Evidence about where the header word came from

### 1. Jump table alignment inside the rodata

GCC's MIPS backend emits every switch jump table as:

    .rdata
    .align 3
    $Lnnn:
    .word ...

On MIPS assemblers the `.align` operand is a power of two, so `.align 3` means
8 bytes. Alignment is measured from the start of the current section in the
object being assembled, not from the final absolute address.

ADDHERO's rodata, read straight from the decompressed disc file, contains four
jump tables and two zero words:

    0x04  jtbl (8 entries)      ends 0x24
    0x24  jtbl (5 entries)      ends 0x38
    0x38  00000000              padding
    0x3C  jtbl (5 entries)      ends 0x50
    0x50  00000000              padding
    0x54  jtbl (13 entries)     ends 0x88

Measured from 0x04 the tables sit at relative offsets 0x00, 0x20, 0x38 and
0x50, all multiples of 8, and the zero words are exactly the padding an
assembler inserts to honour `.align 3`. Measured from 0x00 none of the offsets
are multiples of 8. The compiler's rodata section therefore began at offset 4.
The word at offset 0 was not part of that section.

NIKI shows the same pattern. GOLEM has a single jump table at offset 4 and no
padding words, which is consistent with either reading, but it is built with
the same compiler and flags as ADDHERO and NIKI.

### 2. The same layout under different compilers

MOVIE was compiled with GCC 2.8.0 at `-G4`. ADDHERO, NIKI, GOLEM and most of
the others were compiled with GCC 2.7.2 CDK at `-G0`. Both families have the
header word at 0 and compiler rodata at 4.

### 3. Overlays at a different load address

FIELD, WSEL, WMAP, TITLE and CHECKPS load at 0x8004FC70 rather than
0x80140000. They still have the word at offset 0 and their own content at
offset 4.

### 4. Empirical build results in this repository

| Layout tried | Result |
|--------------|--------|
| Plain `const s32` in the same C file as the jump table (GOLEM) | 4 bytes of padding inserted after the word, every later address shifts, SHA1 mismatch |
| Separate C file holding only the const, linked first | byte-exact on all nine verified overlays |
| `rodatabin` blob for the word (GOSUB, before conversion) | byte-exact |
| `__attribute__((section(".sdata")))` on the const, `.sdata` first in section order (GOLEM, NIKI, ADDHERO, before conversion) | byte-exact |
| Plain const, `-G4` on a `-G0` overlay (ADDHERO) | word moves to `.sdata` but 159 instructions of the text change, mismatch |

Every layout that keeps the word out of the same rodata section as the jump
table matches. The one layout that puts it inside does not.

## Hypotheses

### A. A plain constant in the main C file

    const s32 overlay_id = 20;

Ruled out for any overlay whose rodata begins with a jump table. The compiler
places the constant in `.rdata`, then pads to 8 bytes before the table. The
disc shows no such padding. It survives only in overlays such as GNAME, GOVER
and TITLE, where the first rodata item after the word happens to need only
4-byte alignment, and even there it is a reconstruction rather than evidence.

### B. Compiler small-data placement

Under `-G4` GCC puts a 4-byte constant in `.sdata` on its own, giving it a
separate section that the linker can place first. It cannot be the general
mechanism because most overlays were built at `-G0`, and forcing `-G4` on them
changes the generated code. It also requires the link script to place `.sdata`
ahead of `.rdata`, which is not the PSY-Q default order.

### C. A separate object linked first

A one-line C file per overlay, containing only the word, listed first in the
PSY-Q link file. The link origin is the load address. The header object's
rodata is complete at 4 bytes, and the next object's rodata begins fresh at
load-address-plus-4 with its own alignment relative to that point. This
reproduces the observed bytes on every overlay.

### D. A post-link step that prepends the word

The overlay is linked with its origin at load-address-plus-4. After linking
and flattening to a raw image, a build step writes the 4-byte ID in front, and
the result is handed to the compressor. This also reproduces the observed
bytes.

On its face, this seems the most likely option to me; however, it would imply 
that developers would have had to deal with some supposed four-byte offset detail 
when debugging or looking at symbols. Because of the complexity, this theory 
seems less practical if it imposed something that was harder for the team to work with.

## Assessment

C and D link to identical images, so the bytes cannot separate them. The
choice is about which is the more believable engineering practice, and C is
adopted for these reasons:

- Under C the link origin is the load address. Under D every link file must
  carry an origin four bytes past the load address, and every tool or person
  computing addresses from file offsets must remember the header. A team would
  notice that friction; nothing in the recovered code suggests they lived with
  it.
- The recovered tree is plain C throughout, with no inline assembly. A
  one-line C file is consistent with that. A post-link stamping tool is a
  separate piece of machinery that needs a per-overlay ID table and a
  little-endian writer, in addition to the compressor that already existed.
- The IDs run in development order rather than disc order, which fits a
  numbered list kept alongside the source. A stub file per module is where such
  a number would be typed.
- The uniformity of the layout across compilers, flags and load addresses is
  explained equally well by C, since a stub object is independent of all
  three.

D's remaining advantage is that it leaves no source artefact. That is outweighed
by the off-by-four cost above. Hypotheses A and B are excluded by the evidence.

This is a modelling decision, not a proof. If non-byte evidence ever appears,
such as leftover map or symbol files on a disc, or a debug string naming the
original stub object or stamping tool, the model should be revisited.

## Implementation

### Source

Each overlay has `src/overlays/<name>/overlay_header.c` containing only:

    const s32 g_<name>_overlay_id = <id>;

plus a docblock. The file must hold nothing else. Any other content would
change the size or alignment of the header object's `.rodata` and shift the
whole overlay.

The constant is compiled with GCC 2.7.2 CDK at `-G0` for every overlay,
including MOVIE, so it always lands in `.rodata`. Under `-G4` a 4-byte const
would move to `.sdata` and require a non-default section order.

### Splat configuration

Every overlay yaml keeps `vram` at the load address and the segment `start` at
0x1 (skipping the compression tag). The first two subsegments are:

    - [auto, c, overlay_header]
    - [0x1, .rodata, overlay_header]

The `auto` C subsegment gives splat a translation unit to emit a linker entry
and an objdiff target for; the `.rodata` subsegment maps file offsets 0x1 to
0x5 to it. The overlay's real content begins at 0x5 as before. `section_order`
is the default (`.rodata` first); no `.sdata`-first ordering is needed.

### Registry

`overlay_header.c` is the first entry of `overlay_<name>_gcc_272_cdk_g0_srcs`
in `mk/overlay-registry.mk`. For MOVIE, whose code is built with GCC 2.8.0
`-G4`, the header file has its own `gcc_272_cdk_g0` line.

### Symbols

Each `config/symbols/<name>_symbol_addrs.txt` declares
`g_<name>_overlay_id` at the load address (0x80140000 or 0x8004FC70).

## Unknowns

- Which routine, if any, in a development build read the header word? The
  shipped executable has no consumer, so this was most likely a debugging or
  disc-verification convenience.
- Do IDs 0 to 3 correspond to anything on the disc? The main executable does
  not carry such a word at its entry point.
