# Legend of Mana PSX Decompilation

[![Progress]][progress site]
[![Build and Progress](https://github.com/celophi/lom-decomp/actions/workflows/progress.yaml/badge.svg)](https://github.com/celophi/lom-decomp/actions/workflows/progress.yaml)

[Progress]: https://decomp.dev/celophi/lom-decomp.svg?mode=shield&measure=code&category=all&label=Progress
[progress site]: https://decomp.dev/celophi/lom-decomp

A work-in-progress **matching decompilation** of the North American PlayStation release of **Legend of Mana**.

The current target is `SLUS_010.13` (disc serial **SLUS-01013**). The goal is to reconstruct readable C source that reproduces the original MIPS machine code and, where supported, rebuilds the original game binaries byte-for-byte.

This is a decompilation project, **not a PC port**. The repository does not include the game executable, overlay binaries, artwork, audio, or other copyrighted game data. You must provide the required files from your own copy of the game.

The primary motivation for this project is to preserve the original game's logic and behavior for educational research and potential modding capabilities.

## Progress

The project ships the main executable (`SLUS_010.13`) plus 17 overlays. Each module moves through roughly these states:

- 💤 **Not started** - splat config may exist, but no meaningful C has been written; the module is still almost entirely assembly stubs.

- 🌱 **In progress** - some functions have been examined and have written C, but the module does not yet build as a byte-identical replacement.

- 🪲 **Non-matching** - every function has a C implementation, but is not yet matching the target.

- ☑️ **Matching** - every function has a C implementation that matches the target byte-for-byte.

- 🔒 **Fully linked** - two conditions hold:

  1. The build produces an **ELF whose bytes match the original decompressed file**, and
  2. Running the project's compressor on that ELF (stripped to a raw binary) **reproduces an exact replica of the `.BIN` file as it appears on the disc**.

  In other words, the round-trip `original .BIN -> decompress -> C source -> compile -> ELF -> compress -> .BIN` is bit-identical.

| Module | | Status |
|---|:---:|---|
| SLUS_010.13 | 🪲 | Non-matching |
| ADDHERO.BIN | 💤 | Not started |
| CARDA.BIN | 💤 | Not started |
| CHECKPS.BIN | 🔒 | Fully linked |
| CLOAD.BIN | 🪲 | Non-matching |
| FIELD.BIN | 🌱 | In progress |
| GNAME.BIN | 🔒 | Fully linked |
| GOLEM.BIN | 🌱 | In progress |
| GOSUB.BIN | ☑️ | Matching |
| GOVER.BIN | 🔒 | Fully linked |
| MENU.BIN | 🪲 | Non-matching |
| MOVIE.BIN | 🔒 | Fully linked |
| NIKI.BIN | 🌱 | In progress |
| SHOP.BIN | 💤 | Not started |
| TITLE.BIN | 🔒 | Fully linked |
| WMAP.BIN | 💤 | Not started |
| WSEL.BIN | 💤 | Not started |
| ZUKAN.BIN | 💤 | Not started |

## Supported game version

| Item | Value |
|---|---|
| Region | North America |
| Disc serial | `SLUS-01013` |
| Main executable | `SLUS_010.13` |
| Main executable SHA-1 | `d11dfdd50d412ac3fa3e2eb80fbde138da118f27` |
| Architecture | 32-bit little-endian MIPS / PlayStation |

Other regional versions are not currently supported by the build configs.

## Requirements

For the normal build you need:

- **Git**, including submodule support.
- **Docker** - Docker Desktop on Windows/macOS or Docker Engine on Linux.
- A **legally obtained North American copy of Legend of Mana**.

You do not need to install the historical PSX compilers, Psy-Q tools, Python packages, or a MIPS cross-compiler directly on your host. The development container provides them.

## Getting started

### 1. Clone the repository

```bash
git clone --recursive https://github.com/celophi/lom-decomp.git
cd lom-decomp
```

If you already cloned without submodules:

```bash
git submodule update --init --recursive
```

### 2. Add the original game files

Extract the main executable and the game's `BIN` directory from your North American disc/image so the repository contains:

```text
disc/
|-- SLUS_010.13
`-- BIN/
    |-- ADDHERO.BIN
    |-- CARDA.BIN
    |-- CHECKPS.BIN
    |-- CLOAD.BIN
    |-- FIELD.BIN
    |-- GNAME.BIN
    |-- GOLEM.BIN
    |-- GOSUB.BIN
    |-- GOVER.BIN
    |-- MENU.BIN
    |-- MOVIE.BIN
    |-- NIKI.BIN
    |-- SHOP.BIN
    |-- TITLE.BIN
    |-- WMAP.BIN
    |-- WSEL.BIN
    `-- ZUKAN.BIN
```

You do not need to copy the rest of the disc into the repository.

To confirm the main executable is the expected version:

```bash
sha1sum disc/SLUS_010.13
```

Expected:

```text
d11dfdd50d412ac3fa3e2eb80fbde138da118f27  disc/SLUS_010.13
```

The splat configs also contain expected SHA-1 hashes for the overlay files.

> `disc/` is gitignored. Never commit original game files.

### 3. Build the historical compiler images

The project uses multiple historical GCC variants. Build the three local compiler images from the `old-gcc` submodule:

```bash
docker build -t old-gcc/gcc-2.8.0-psx -f tools/old-gcc/gcc-2.8.0-psx.Dockerfile tools/old-gcc
docker build -t old-gcc/gcc-2.7.2-cdk -f tools/old-gcc/gcc-2.7.2-cdk.Dockerfile tools/old-gcc
docker build -t old-gcc/gcc-2.6.0-psx -f tools/old-gcc/gcc-2.6.0-psx.Dockerfile tools/old-gcc
```

This is normally a one-time setup step. The GCC 2.7.2 GNU-as toolchain used by a small number of sources is pulled by the development Dockerfile.

### 4. Build the development container

```bash
docker build -t lom-dev -f dockerfiles/dev.dockerfile .
```

The image contains the compilers, Psy-Q tooling, MIPS binutils, splat, maspsx, objdiff support, and Python dependencies used by the project.

### 5. Start the container

Run this from the repository root.

PowerShell, bash, or zsh:

```bash
docker run --rm -it -v "${PWD}:/lom" lom-dev
```

Windows Command Prompt (`cmd.exe`):

```bat
docker run --rm -it -v "%cd%:/lom" lom-dev
```

The remaining setup commands are run **inside the container**.

### 6. Split the original binaries

```bash
make splat
```

This generates local build inputs such as `asm/`, `linker/`, and extracted assets. These files are intentionally not all stored in Git.

Run `make splat` again after changing splat configs, segment boundaries, symbol maps, or relocation overrides.

### 7. Build

Build the main executable:

```bash
make
```

Output:

```text
build/SLUS_010.13.elf
```

To also produce a flat binary:

```bash
make bin
```

Build one registered overlay:

```bash
make field
make menu
make checkps
```

Build all overlays currently registered in `mk/overlay-registry.mk`:

```bash
make overlays
```

Build the main executable and all registered overlays:

```bash
make everything
```

`mk/overlay-registry.mk` is the authoritative list of overlays currently wired into the linkable build.

## Normal development workflow

After the initial setup, you generally do **not** need to clean the project after every edit:

```text
edit source on host
        |
        v
make <smallest relevant target> inside lom-dev
        |
        v
inspect objdiff / diff output
        |
        v
edit and repeat
```

The Makefile automatically stages changed inputs before compiling. If the staged copy ever appears stale, run:

```bash
make recopy
```

Use `make clean` only when you actually want to remove `build/` and the `/staging` copy.

## Why `/staging` exists

The repository is mounted into Docker at `/lom`, but historical compilation happens from `/staging`, a native Linux filesystem inside the container.

Some legacy 32-bit compiler/preprocessor binaries cannot safely `stat()` files on Windows-backed Docker bind mounts and may fail with:

```text
Value too large for defined data type
```

The Makefile solves this by copying required inputs to `/staging` and normalizing text files to LF line endings before compiling.

For that reason, do not bypass the build system and invoke the old compiler directly against files under `/lom`.

## Useful Make targets

| Target | Purpose |
|---|---|
| `make` | Build the main `SLUS_010.13` ELF. |
| `make bin` | Also produce `build/SLUS_010.13.bin`. |
| `make <overlay>` | Build one registered overlay, such as `make field`. |
| `make overlays` | Build all registered overlays. |
| `make everything` | Build the main executable and all registered overlays. |
| `make splat` | Split the main executable and all overlay configs. |
| `make objdiff-objects` | Build target and reconstructed objects for objdiff. |
| `make objdiff-config` | Regenerate `objdiff.json`. |
| `make progress` | Generate `build/progress.json`. |
| `make diff-all` | Run objdiff across all configured units. |
| `make diff-text` | Generate compact text reports under `build/diffs/`. |
| `make dump-objs` | Disassemble built objects for code-generation analysis. |
| `make validate-assets` | Round-trip and validate format-aware assets. |
| `make verify-bins` | Run all registered whole-overlay SHA-1 checks. |
| `make verify-compressor` | Verify the compressor against all 17 original overlay files. |
| `make recopy` | Force source/config files to be copied to `/staging` again. |
| `make clean` | Remove build output and `/staging`. |

## Matching functions

The project uses [objdiff](https://github.com/encounter/objdiff) for local function and object comparison.

Build both sides and generate the objdiff config:

```bash
make objdiff-objects
make objdiff-config
```

Then open objdiff and point it at the repository root.

For a command-line workflow:

```bash
make diff-all
make diff-text
```

The compact reports are written below `build/diffs/`.

You can also use [decomp.me](https://decomp.me) for collaborative matching. Existing source comments contain links to many decomp.me scratches; preserve those references when editing a function.

## Compiler and assembler toolchains

A critical detail of this project is that **not every source file uses the same compiler configuration**.

The compiler definitions live in `mk/toolchains.mk`. Source routing for the main executable is in `mk/main.mk`, and overlay routing is in `mk/overlay-registry.mk`.

| Pipeline | Compiler flags | Assembly path |
|---|---|---|
| GCC 2.8.0 G0 | `-O2 -G0 -gcoff -fsigned-char -fno-builtin` | maspsx, ASPSX 2.77 behavior |
| GCC 2.8.0 G4 | `-O2 -G4 -gcoff -fsigned-char` | maspsx, ASPSX 2.77 behavior |
| GCC 2.7.2 CDK G0 | `-O2 -G0 -msoft-float -gcoff` | maspsx, ASPSX 2.67 behavior |
| GCC 2.6.0 G0 | `-O2 -G0 -gcoff -msoft-float` | maspsx, ASPSX 2.34 behavior |
| GCC 2.7.2 GNU | `-O2 -G0` | GNU `as` with `-O -EL` |

Some individual sources also change instruction scheduling, optimization level, or `div` expansion behavior. The Makefiles contain those exceptions.

When matching a function, **use the exact toolchain selected for its source file**. Do not substitute the host GCC, Clang/LLVM, a different GCC release, or a different assembler and treat that result as authoritative.

## Rules for matching contributions

This is a byte-for-byte matching decompilation. C that is logically equivalent can still generate different MIPS instructions.

Keep these rules in mind:

- Do not edit generated `.s` or `.ld` files. Change the source/configuration and rerun `make splat`.
- Do not "clean up" strange C without checking the assembly. Temporaries, casts, branch shape, repeated loads, and awkward expressions may be required for the original code generation.
- Preserve types, signedness, struct layouts, and control flow carefully.
- When renaming an addressed function or global, update the corresponding file under `config/symbols/`.
- Preserve existing decomp.me links in function documentation.
- Build and diff with the Makefile-selected historical toolchain before considering a change matched.

For difficult functions, the repository also contains m2c, decomp-permuter, code-generation analysis scripts, and project-specific matching tools under `tools/`.

## Copyrighted data and assets

Some executable/overlay ranges contain artwork, text, layouts, and other copyrighted data that should not be committed.

The project uses a hybrid approach:

- understood program data can be represented as typed C;
- understood binary formats can use byte-exact extractors/builders;
- unknown or creative data can remain as named local `databin`/`rodatabin` assets referenced with `.incbin`.

See:

- [`docs/handling-copyrighted-data.md`](docs/handling-copyrighted-data.md)
- [`docs/asset-data-architecture.md`](docs/asset-data-architecture.md)

## Repository layout

```text
lom-decomp/
|-- src/                    # Reconstructed C source
|   |-- overlays/           # Overlay source trees
|   `-- psyq/               # Reconstructed Psy-Q library code
|-- include/                # Project and Psy-Q headers/macros
|-- config/                 # Splat configs, symbols, relocations
|-- mk/                     # Build rules and toolchain routing
|-- tools/                  # Decompilation, compiler, diff, and asset tools
|-- docs/                   # Architecture and matching documentation
|-- disc/                   # Your local original game files (gitignored)
|-- assets/                 # Local/generated asset data where required
|-- asm/                    # Splat-generated target assembly (gitignored)
|-- linker/                 # Splat-generated linker files (gitignored)
|-- build/                  # Objects, ELFs, maps, diffs, and reports
|-- dockerfiles/            # Development and CI containers
|-- Makefile
`-- requirements.txt
```

Useful places to start exploring:

- `src/` - reconstructed game and Psy-Q code.
- `asm/nonmatchings/` and `asm/overlays/*/nonmatchings/` - generated target assembly for unmatched functions.
- `config/symbols/` - known function/global addresses.
- `config/relocations/` - relocation overrides used when splat needs help reconstructing symbolic references.
- `mk/overlay-registry.mk` - overlay source/toolchain assignments.
- `docs/decompilation/` - project-specific matching notes.

## Documentation

Useful project-specific references include:

- [`docs/decompilation/gcc-272-matching-techniques.md`](docs/decompilation/gcc-272-matching-techniques.md)
- [`docs/decompilation/splat-reloc-overrides.md`](docs/decompilation/splat-reloc-overrides.md)
- [`docs/decompilation/psyq-gpu-primitives.md`](docs/decompilation/psyq-gpu-primitives.md)
- [`tools/compressor/README.md`](tools/compressor/README.md)

## Troubleshooting

**`make splat` reports a missing file or SHA-1 mismatch**

Make sure you extracted the North American version and placed the files at `disc/SLUS_010.13` and `disc/BIN/*.BIN` without renaming them.

**Docker cannot find an `old-gcc/...` image**

Initialize the Git submodules and build the three historical compiler images from the setup section before building `lom-dev`.

**GCC reports `Value too large for defined data type`**

Use the Makefile instead of compiling directly from `/lom`. Run `make recopy` if the staged tree needs to be refreshed.

**A symbol/config change is not reflected in generated assembly**

Run `make splat` again. Do not edit generated `asm/` or `linker/` files manually.

**A function matches with another compiler but not in the project build**

Check its routing in `mk/main.mk` or `mk/overlay-registry.mk`. The configured historical toolchain is the authoritative one.

**objdiff reports 100%, but whole-overlay verification fails**

Check data/rodata, relocations, linker section order, alignment, and generated assets. Function-level matching does not prove whole-file identity.

## Contributing

Contributions are welcome, including:

- matching or improving functions;
- identifying and renaming functions, globals, structures, and fields;
- documenting subsystems and data formats;
- creating byte-exact asset extractors/builders;
- improving build, diffing, and analysis tools;
- investigating compiler and Psy-Q provenance;
- improving documentation and setup instructions.

Before opening a pull request, build the affected target and run the most relevant objdiff and/or whole-overlay verification available.

If you are new to matching decompilation, starting with a small function or a naming/documentation improvement around understood code is usually much easier than starting with a large unmatched routine.

## Tools and acknowledgements

This project builds on tools and research from the wider decompilation community, including:

- [splat](https://github.com/ethteck/splat)
- [spimdisasm](https://github.com/Decompollaborate/spimdisasm)
- [maspsx](https://github.com/mkst/maspsx)
- [old-gcc](https://github.com/decompals/old-gcc)
- [objdiff](https://github.com/encounter/objdiff)
- [decomp-permuter](https://github.com/simonlindholm/decomp-permuter)
- [m2c](https://github.com/matt-kempster/m2c)
- [wibo](https://github.com/decompals/wibo)
- [psyq-obj-parser](https://github.com/mkst/psyq-obj-parser)
- [decomp.me](https://decomp.me)
- [decomp.dev](https://decomp.dev)


## Thanks

A heartfelt thank you to Squaresoft and to everyone who had a hand in creating *Legend of Mana*. The game is full of imagination, experimentation, unusual ideas, beautiful artwork and music, and technical choices that still make it fascinating to study decades later. Projects like this exist because the original developers, artists, musicians, designers, writers, and support staff took chances and created something distinctive enough that people still care about understanding and preserving it today.

This decompilation is, above all, an expression of appreciation for that work. Thank you for making such a beautiful and memorable game, and for being willing to try something different.

## Legal

This repository is an independent reverse-engineering and preservation project. It is not affiliated with or endorsed by Square, Square Enix, Sony, or any other rights holder.

No original game executable, overlay binaries, artwork, audio, or other copyrighted game data should be committed to this repository. You must supply required data from your own legally obtained copy of the game.

*Legend of Mana* and related names and assets are the property of their respective owners.

<img src="docs/lil-cactus.png" width="84" alt="Lil' Cactus"/>
