# data2c

Generates typed C for the game's `.data` regions at build time, from the
user's own game data.

## Why

The decomp keeps game data as splat output: blobs (`databin`) or data
assembly. That is enough for the PS1 build. A port whose pointers are wider
than four bytes needs the data as C instead, so the compiler lays out every
stored pointer itself. sm64 and sotn-decomp work the same way. Generating
the C at build time keeps game data out of the repository.

## Using it

```sh
make verify-data-as-c                 # the check CI runs: every binary from generated .data
make verify-data-host                 # CI too: the host build of the data holds the PS1 values
make DATA_AS_C=1 verify-bins          # the same build, keeping its objects
python3 tools/data2c/report_all.py    # survey all regions (needs a normal build first)
python3 tools/data2c/data2c.py --help # one region
python3 -m unittest discover -s tools/data2c/tests
```

`DATA_AS_C=1` must still reproduce the original images byte for byte, and CI
checks that for both versions. `verify-data-as-c` removes the data objects
before and after, including the copies the overlay targets make in the
project's own `build/`, so a normal build never picks up generated ones.
data2c also refuses an object that was built from its own output. The
generated C goes to `build/<version>/**/datac/`. It needs libclang (`libclang`
in requirements.txt); the unit tests also need `clang` and a host `cc`.

## Host output

A port that builds its data from C (the sotn/sm64 model) compiles the same
values with its host compiler. `data2c.py --host` writes them for that:

- every symbol is its own object: no packed clusters, which exist only for
  GCC 2.8's placement rules;
- no PS1 padding members: the host compiler pads by its own rules;
- each integer keeps its declared C type, so a `long` stays a `long` and the
  data lays out as host-compiled game code expects;
- a second name at an address becomes a host assembler alias;
- undeclared words that hold addresses become pointer-sized entries.

`--host --verify` runs `hostcheck.py`: it compiles the output for x86-64 and
reads every symbol back through the host layout of the decomp's own
declaration, comparing each field with the PS1 data read through the PS1
layout (integers by value, pointers by target symbol). `make verify-data-host`
does that for every region of a version, and CI runs it for US and JP.

What it cannot see: whether code relies on two adjacent symbols being one
object (indexing past the end of one into the next). On the PS1 they are
contiguous; on a host they are separate objects.

## Tests

`tests/test_data2c.py` covers each rule below on small made-up types and
bytes (never game data), round-tripping every case through `verify.py`, and
host output through `hostcheck.py`, including a changed value and a
mismatched layout that it must catch. The GCC 2.8 half of the rules
(alignment, clusters) can only be checked by `make verify-data-as-c`.

## How a region becomes C

| Step | Module | What it does |
|---|---|---|
| Symbols | `symbols.py` | Names and addresses from the files the linker uses: `<image>_symbol_addrs.txt` and splat's `undefined_*_auto.txt` |
| Region | `region.py` | The bytes. A databin's pointers are recognized by value; assembled data gives exact relocations |
| Types | `declarations.py` | Each symbol's type from the image's own C, parsed with libclang for mipsel so layouts are the PS1 ones |
| C text | `writer.py` | Structural types, initializers, and placement GCC 2.8 reproduces |
| Check | `verify.py` | `--verify`: compile for mipsel, relocate, compare with the original bytes |
| Host check | `hostcheck.py` | `--host --verify`: compile for x86-64, compare every field's value through the decomp's own types |

`project.py` says where each image keeps these inputs; `elf.py` reads the
object files (ELF32 for the PS1, ELF64 for the host check).

## Decisions a maintainer needs to know

- **Types are written structurally** (`D2C_S000`, ...), because the decomp's
  types are often private to one `.c` file. Only the layout has to match.
- **When files disagree** about a symbol's type, `declarations.choose()` picks
  a complete type that fits, preferring pointers and structs. The report lists
  the conflicts.
- **Bytes always win.** A declaration that does not fit, cannot be parsed, or
  hides an address falls back to plain bytes or words, so the PS1 build stays
  exact whatever the C says. Only the native usefulness suffers.
- **Padding and unions:** structs get explicit padding members (some of the
  game's padding bytes are not zero), and unions put their widest member
  first, because C89 initializes only the first member.
- **Bitfields** are read from the bytes they span (little-endian), so one
  that crosses a word boundary still reads right.
- **Assembled data is placed by its labels:** the symbol files give their
  addresses; spimdisasm's address comment is only a fallback. Labels that
  disagree stop the build.
- **GCC 2.8 word-aligns every array and struct variable.** Symbols that start
  on an address it could not give them are merged into one packed struct (a
  cluster); they are really fields of one object. Their names become
  `#define`s for C and assembler aliases for the linker.

## What the report means

Nothing in the report breaks the PS1 build. It lists where the decomp's
declarations are not yet good enough for wider pointers:

- conflicting declarations that disagree about pointers
- addresses held in integer fields, or in bytes with no declaration
- symbols that are really fields of one struct (clusters)
- declared types that do not fit the symbol's space in the data
