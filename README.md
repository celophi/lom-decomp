# Legend of Mana PSX Decompilation

**Languages:** English | [日本語](README_JP.md)

[![US Progress]][us progress site]
[![JP Progress]][jp progress site]
[![Build and Progress](https://github.com/celophi/lom-decomp/actions/workflows/progress.yaml/badge.svg)](https://github.com/celophi/lom-decomp/actions/workflows/progress.yaml)

[US Progress]: https://decomp.dev/celophi/lom-decomp/SLUS_010.13.svg?mode=shield&measure=code&label=US%20Progress
[us progress site]: https://decomp.dev/celophi/lom-decomp/SLUS_010.13
[JP Progress]: https://decomp.dev/celophi/lom-decomp/SLPS_021.70.svg?mode=shield&measure=code&label=JP%20Progress
[jp progress site]: https://decomp.dev/celophi/lom-decomp/SLPS_021.70

A **matching decompilation** of the PlayStation game **Legend of Mana**. The North American release is **100% matched**; the Japanese release is in progress.

The project reconstructs readable C source code that compiles down to the original MIPS machine code that exists on the disc, byte-for-byte. Two regional releases are targeted:

- **North America** - `SLUS_010.13` (disc serial **SLUS-01013**). Complete: all 18 binaries are fully linked.
- **Japan** - `SLPS_021.70` (disc serial **SLPS-02170**). In progress: all 18 binaries rebuild byte-exact, and most of the code builds from the shared C sources.

Unless a section says otherwise, the build instructions, targets, and file names below refer to the North American version.

This is a decompilation project, **not a PC port**. The repository does not include the game executable, overlay binaries, artwork, audio, or other copyrighted game data. You must provide the required files from your own copy of the game.

The primary motivation for this project is to preserve the original game's logic and behavior for educational research and potential modding capabilities.

## Fully linked

For the North American version, every module - the main executable and all 17 overlays - is **fully linked**. A module is fully linked when two conditions hold:

1. The build produces an **ELF whose bytes match the original decompressed file**, and
2. Running the project's compressor on that ELF (stripped to a raw binary) **reproduces an exact replica of the `.BIN` file as it appears on the disc**.

In other words, the round-trip `original .BIN -> decompress -> C source -> compile -> ELF -> compress -> .BIN` is bit-identical.
The main executable is not compressed, so for `SLUS_010.13` condition 1 is the whole check: the linked ELF, converted to a raw binary, equals the disc file.
*(Check out the compressor! It's honestly really amazing that it is **bit identical** and kind of extraneous, but cool nonetheless!)*

Run `make verify-bins` to check every module. The Japanese version (`make verify-bins VERSION=jp`) matches byte-for-byte too, but is not fully linked yet: the compressor cannot reproduce four of its `.BIN` streams (FIELD, GNAME, GOSUB, TITLE), so those are checked against the decompressed image only.

## Roadmap

1. ✅ **100% matching** - *done.* The main executable (`SLUS_010.13`) and all 17 overlays are fully linked (see above). The Psy-Q SDK libraries are still linked from the original assembly.

2. 🚧 **Cleanup and documentation** - *in progress.* Remove decompilation artifacts and document functionality.

3. 🚧 **NTSC-J version** - *in progress.* Support the original Japanese release (`SLPS-02170`) alongside the North American one.

4. 💤 **Modding and source port** - build on the reconstructed source to make modding practical and to enable ports to other platforms.

## Supported game versions

| Item | North America | Japan |
|---|---|---|
| Status | ✅ Fully linked | 🚧 In progress |
| Disc serial | `SLUS-01013` | `SLPS-02170` |
| Main executable | `SLUS_010.13` | `SLPS_021.70` |
| Main executable SHA-1 | `d11dfdd50d412ac3fa3e2eb80fbde138da118f27` | `b067188a92e4de9a4db7bb7e5343c757e9884bfa` |
| Disc image (`.bin`) SHA-1 | `c1b536c99f0d390584eb30462a7e37f2bbef3902` | `7a314615be8a482cf3f81b4101cc19aaa738f36d` |
| Architecture | 32-bit little-endian MIPS / PlayStation | 32-bit little-endian MIPS / PlayStation |

The getting-started steps below use the North American version. The Japanese version works the same way, with its files under `disc/jp/` and `VERSION=jp` on the `make` command line. Other regional versions are not currently supported.

## Requirements

For the normal build you need:

- **Git**, including submodule support.
- **Docker** - Docker Desktop on Windows/macOS or Docker Engine on Linux.
- A **legally obtained copy of Legend of Mana**, North American or Japanese (see [Supported game versions](#supported-game-versions)).

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

Extract the main executable and the game's `BIN` directory from your North American disc/image into `disc/us/`, so the repository contains the following (the Japanese release goes in `disc/jp/` with the same layout):

```text
disc/
`-- us/
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
sha1sum disc/us/SLUS_010.13
```

Expected:

```text
d11dfdd50d412ac3fa3e2eb80fbde138da118f27  disc/us/SLUS_010.13
```

The splat configs also contain expected SHA-1 hashes for the overlay files.

> `disc/` is gitignored. Never commit original game files.

### 3. Build the historical compiler images

The project uses multiple historical GCC variants. Build the four local compiler images using the `old-gcc` submodule:

```bash
docker build -t old-gcc/gcc-2.8.0-psx -f tools/old-gcc/gcc-2.8.0-psx.Dockerfile tools/old-gcc
docker build -t old-gcc/gcc-2.7.2-cdk -f tools/old-gcc/gcc-2.7.2-cdk.Dockerfile tools/old-gcc
docker build -t old-gcc/gcc-2.6.0-psx -f tools/old-gcc/gcc-2.6.0-psx.Dockerfile tools/old-gcc
docker build -t old-gcc/gcc-2.7.2-psx-gnu -f dockerfiles/gnu-as.dockerfile tools/old-gcc
```

This is normally a one-time setup step. The development Dockerfile uses these local images; no access to a private compiler image is required.

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

This generates local build inputs such as `asm/us/`, `linker/us/`, and extracted assets under `assets/us/`. These files are intentionally not all stored in Git.

Run `make splat` again after changing splat configs, segment boundaries, symbol maps, or relocation overrides.

### 7. Build

Build the main executable:

```bash
make
```

Output:

```text
build/us/SLUS_010.13.elf
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

Use `make clean` only when you actually want to remove `build/us/` and the `/staging` copy.

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
| `make bin` | Also produce `build/us/SLUS_010.13.bin`. |
| `make <overlay>` | Build one registered overlay, such as `make field`. |
| `make overlays` | Build all registered overlays. |
| `make everything` | Build the main executable and all registered overlays. |
| `make splat` | Split the main executable and all overlay configs. |
| `make objdiff-objects` | Build target and reconstructed objects for objdiff. |
| `make objdiff-config` | Regenerate `objdiff.json`. |
| `make progress` | Generate `build/us/progress.json`. |
| `make diff-all` | Run objdiff across all configured units. |
| `make diff-text` | Generate compact text reports under `build/us/diffs/`. |
| `make dump-objs` | Disassemble built objects for code-generation analysis. |
| `make validate-assets` | Round-trip and validate format-aware assets. |
| `make verify-main` | Check that the linked main executable equals `disc/us/SLUS_010.13` (`verify-slus` is an alias). |
| `make verify-bins` | Run `verify-main` and all registered whole-overlay SHA-1 checks. |
| `make verify-compressor` | Verify the compressor against all 17 original overlay files. |
| `make recopy` | Force source/config files to be copied to `/staging` again. |
| `make clean` | Remove build output and `/staging`. |

Every target builds the North American version by default. Add `VERSION=<name>` to select another release, e.g. `make VERSION=jp`; see [Version layout](#version-layout).

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

The compact reports are written below `build/us/diffs/`.

You can also use [decomp.me](https://decomp.me) for collaborative matching. Existing source comments contain links to many decomp.me scratches; preserve those references when editing a function.

## Compiler and assembler toolchains

A critical detail of this project is that **not every source file uses the same compiler configuration**.
Honestly, It was a lot of "fun" dealing with this. Especially the GNU one let me tell you.

The build defines 7 pipeline variants across four historical compiler builds. Compiler and assembler flags live in [`mk/toolchains.mk`](mk/toolchains.mk). Source routing and per-file overrides are in [`mk/main.mk`](mk/main.mk) and [`mk/overlay-registry.mk`](mk/overlay-registry.mk); [`mk/overlays.mk`](mk/overlays.mk) applies the overlay variants.

| Pipeline | Compiler flags | Assembly path |
|---|---|---|
| GCC 2.8.0 G0 (default) | `-O2 -G0 -gcoff -fsigned-char` | maspsx, ASPSX 2.77, expanded division |
| GCC 2.8.0 G0, unoptimized | `-O0 -G0 -gcoff -fsigned-char` | maspsx, ASPSX 2.77, expanded division |
| GCC 2.8.0 G4 | `-O2 -G4 -gcoff -fsigned-char` | maspsx, ASPSX 2.77, expanded division |
| GCC 2.8.0 G4, no division expansion | `-O2 -G4 -gcoff -fsigned-char` | maspsx, ASPSX 2.77, bare division |
| GCC 2.7.2 CDK G0 | `-O2 -G0 -msoft-float -gcoff` | maspsx, ASPSX 2.67, expanded division |
| GCC 2.7.2 GNU G0 | `-O2 -G0` | Historical GNU `as` with `-O -EL` |
| GCC 2.6.0 G0 | `-O2 -G0 -gcoff -msoft-float` | maspsx, ASPSX 2.34, expanded division |


## Copyrighted data and assets

Some executable/overlay ranges contain artwork, text, layouts, and other copyrighted data that should not be committed.

The project uses a hybrid approach:

- understood program data can be represented as typed C;
- understood binary formats can use byte-exact extractors/builders;
- unknown or creative data can remain as named local `databin`/`rodatabin` assets referenced with `.incbin`.

See:

- [`docs/handling-copyrighted-data.md`](docs/handling-copyrighted-data.md)
- [`docs/asset-data-architecture.md`](docs/asset-data-architecture.md)

## Version layout

One C source tree builds every regional release. Anything that comes from a particular disc lives in a per-version folder, and `VERSION=<name>` on the `make` command line picks which one to build.

| `VERSION` | Release | Status |
|---|---|---|
| `us` (default) | North America, `SLUS-01013` | Fully linked |
| `jp` | Japan, `SLPS-02170` | In progress - all 18 binaries rebuild byte-exact. The main executable and 12 overlays build from the shared C sources, except the units whose JP code differs; CARDA, CLOAD, GNAME, TITLE and WSEL still link from first-pass splat assembly |

| Shared by all versions | Per version |
|---|---|
| `src/`, `include/`, `mk/`, `tools/`, `docs/` | `disc/<version>/`, `config/<version>/`, and the generated `asm/<version>/`, `linker/<version>/`, `assets/<version>/`, `build/<version>/` |

Where the code differs between releases, the shared C source uses `#if defined(VERSION_JP)` / `#if defined(VERSION_US)` blocks; the build defines exactly one of them (see [`mk/version.mk`](mk/version.mk) and [`include/version.h`](include/version.h)). Symbol names are shared, but addresses are not, so each version has its own `config/<version>/symbols/` files.

Each version gets its own objdiff progress report, and CI builds both (`SLUS_010.13_report` and `SLPS_021.70_report`). For JP, the modules listed in `TU_LAYOUT_jp` in [`mk/version.mk`](mk/version.mk) build from the shared C sources, minus the files in `config/jp/asm_units.txt`, which JP takes from assembly because their code differs. The JP report counts that assembly, and the modules not yet ported, as unmatched code.

## Repository layout

```text
lom-decomp/
|-- src/                    # Reconstructed C source
|   |-- overlays/           # Overlay source trees
|   `-- psyq/               # Reconstructed Psy-Q library code
|-- include/                # Project and Psy-Q headers/macros
|-- config/<version>/       # Splat configs, symbols, relocations
|-- mk/                     # Build rules and toolchain routing
|-- tools/                  # Decompilation, compiler, diff, and asset tools
|-- docs/                   # Architecture and matching documentation
|-- disc/<version>/         # Your local original game files (gitignored)
|-- assets/<version>/       # Local/generated asset data where required
|-- asm/<version>/          # Splat-generated target assembly (gitignored)
|-- linker/<version>/       # Splat-generated linker files (gitignored)
|-- build/<version>/        # Objects, ELFs, maps, diffs, and reports
|-- dockerfiles/            # Development and CI containers
|-- Makefile
`-- requirements.txt
```

Useful places to start exploring:

- `src/` - reconstructed game and Psy-Q code.
- `asm/us/nonmatchings/` and `asm/us/overlays/*/nonmatchings/` - generated target assembly for unmatched functions.
- `config/us/symbols/` - known function/global addresses.
- `config/us/relocations/` - relocation overrides used when splat needs help reconstructing symbolic references.
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

Make sure the files are at `disc/us/SLUS_010.13` and `disc/us/BIN/*.BIN` (or `disc/jp/SLPS_021.70` and `disc/jp/BIN/*.BIN` for the Japanese version), without renaming them.

**Docker cannot find an `old-gcc/...` image**

Initialize the Git submodules and build the four historical compiler images from the setup section before building `lom-dev`.

**GCC reports `Value too large for defined data type`**

Use the Makefile instead of compiling directly from `/lom`. Run `make recopy` if the staged tree needs to be refreshed.

**A symbol/config change is not reflected in generated assembly**

Run `make splat` again. Do not edit generated `asm/` or `linker/` files manually.

**A function matches with another compiler but not in the project build**

Check its routing in `mk/main.mk` or `mk/overlay-registry.mk`. The configured historical toolchain is the authoritative one.

**objdiff reports 100%, but whole-overlay verification fails**

Check data/rodata jump table or case targets, relocation addends, linker section order, the overlay segment's `align:` key, and generated assets. 

## Reverse-engineering provenance

This project was created independently by analyzing the publicly released retail version of Legend of Mana and reconstructing its behavior and machine code through disassembly, decompilation, binary comparison, runtime analysis, and publicly available technical documentation and tools.

No leaked or otherwise non-public *Legend of Mana* source code, debug symbol files, internal symbol maps, developer documentation, or other confidential materials from Square or Square Enix have been used in the creation of this project.

Function names, variable names, data structures, translation-unit boundaries, and other source-level details are reconstructed or inferred from the retail binaries and observed behavior unless otherwise documented. They should not be assumed to be the names or organization used by the original developers.

This repository will **never** include leaked source code, private debug symbols, confidential documentation, or other non-public materials from the original game's development.

## Legal

This repository is an independent reverse-engineering and preservation project. It is not affiliated with or endorsed by Square, Square Enix, Sony, or any other rights holder.

No original game executable, overlay binaries, artwork, audio, or other copyrighted game data should be committed to this repository. You must supply required data from your own legally obtained copy of the game.

*Legend of Mana* and related names and assets are the property of their respective owners.

## Thanks

A heartfelt thank you to Squaresoft and to everyone who had a hand in creating *Legend of Mana*. The game is full of imagination, experimentation, unusual ideas, beautiful artwork and music, and technical choices that still make it fascinating to study decades later. Projects like this exist because the original developers, artists, musicians, designers, writers, and support staff took chances and created something distinctive enough that people still care about understanding and preserving it today.

This decompilation is, above all, an expression of appreciation for that work. Thank you for making such a beautiful and memorable game, and for being willing to try something different.

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
