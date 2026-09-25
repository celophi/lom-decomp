# Legend of Mana PSX Decompilation

[![Progress]][progress site]
[![Build and Progress](https://github.com/celophi/lom-decomp/actions/workflows/progress.yaml/badge.svg)](https://github.com/celophi/lom-decomp/actions/workflows/progress.yaml)

[Progress]: https://decomp.dev/celophi/lom-decomp.svg?mode=shield&measure=code&category=all&label=Progress
[progress site]: https://decomp.dev/celophi/lom-decomp

A complete **100% matching decompilation** of the North American PlayStation release of **Legend of Mana**.

The current target is `SLUS_010.13` (disc serial **SLUS-01013**). The project reconstructs readable C source code that compiles down to the original MIPS machine code that exists on the disc for all 18 binaries byte-for-byte.

This repository is the decompilation itself, **not a PC port or a mod**; those will live in a separate repository (see the [roadmap](#roadmap)). The repository does not include the game executable, overlay binaries, artwork, audio, or other copyrighted game data. You must provide the required files from your own copy of the game.

The primary motivation for this project is to preserve the original game's logic and behavior for educational research and potential modding capabilities.

## Fully linked

Every module - the main executable and all 17 overlays - is **fully linked**. A module is fully linked when two conditions hold:

1. The build produces an **ELF whose bytes match the original decompressed file**, and
2. Running the project's compressor on that ELF (stripped to a raw binary) **reproduces an exact replica of the `.BIN` file as it appears on the disc**.

In other words, the round-trip `original .BIN -> decompress -> C source -> compile -> ELF -> compress -> .BIN` is bit-identical.
The main executable is not compressed, so for `SLUS_010.13` condition 1 is the whole check: the linked ELF, converted to a raw binary, equals the disc file.
*(Check out the compressor! It's honestly really amazing that it is **bit identical** and kind of extraneous, but cool nonetheless!)*

Run `make verify-bins` to check every module.

<details>
<summary>Module status</summary>

| Module | | Status |
|---|:---:|---|
| SLUS_010.13 | 🔒 | Fully linked |
| ADDHERO.BIN | 🔒 | Fully linked |
| CARDA.BIN | 🔒 | Fully linked |
| CHECKPS.BIN | 🔒 | Fully linked |
| CLOAD.BIN | 🔒 | Fully linked |
| FIELD.BIN | 🔒 | Fully linked |
| GNAME.BIN | 🔒 | Fully linked |
| GOLEM.BIN | 🔒 | Fully linked |
| GOSUB.BIN | 🔒 | Fully linked |
| GOVER.BIN | 🔒 | Fully linked |
| MENU.BIN | 🔒 | Fully linked |
| MOVIE.BIN | 🔒 | Fully linked |
| NIKI.BIN | 🔒 | Fully linked |
| SHOP.BIN | 🔒 | Fully linked |
| TITLE.BIN | 🔒 | Fully linked |
| WMAP.BIN | 🔒 | Fully linked |
| WSEL.BIN | 🔒 | Fully linked |
| ZUKAN.BIN | 🔒 | Fully linked |

</details>

## Roadmap

1. ✅ **100% matching** - *done.* The main executable (`SLUS_010.13`) and all 17 overlays are fully linked (see above). The Psy-Q SDK libraries are still linked from the original assembly.

2. 🚧 **Cleanup and documentation** - *in progress.* Remove decompilation artifacts (goto-built loops, register levers, placeholder names), give functions, globals, and structs meaningful names, and document them.

3. 💤 **NTSC-J version** - support the original Japanese release (`SLPS-02170`) alongside the North American one.

4. 💤 **Modding and source ports** - build on the reconstructed source to make modding practical and to enable ports to other platforms. This work will happen in a **separate repository**; this one stays a byte-matching decompilation of the original PlayStation binaries.

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

Build one overlay:

```bash
make field
make menu
make checkps
```

Build all 17 overlays:

```bash
make overlays
```

Build the main executable and all overlays:

```bash
make everything
```

`mk/overlay-registry.mk` lists each overlay's sources and toolchain assignments.

### 8. Verify

```bash
make verify-bins
```

This rebuilds the main executable and every overlay, recompresses the overlays, and checks each result against your files in `disc/`. Each module should print an `[OK] ... matches original ROM` line.

## Development workflow

With everything matching, the job of each change (renaming, documenting, cleaning up artifacts) is to keep the output byte-identical:

```text
edit source on host
        |
        v
make <smallest relevant target> inside lom-dev
        |
        v
make verify-bins
```

If `make verify-bins` fails, `make diff-all` / `make diff-text` (or opening objdiff at the repository root after `make objdiff-objects objdiff-config`) will show which function or object changed.

Source comments contain `@see` links to [decomp.me](https://decomp.me) scratches recording where each function's match work lives; preserve them when editing.

The Makefile automatically stages changed inputs before compiling, so you generally do **not** need to clean between edits. If the staged copy ever appears stale, run `make recopy`. Use `make clean` only when you actually want to remove `build/` and the `/staging` copy.

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
| `make <overlay>` | Build one overlay, such as `make field`. |
| `make overlays` | Build all overlays. |
| `make everything` | Build the main executable and all overlays. |
| `make splat` | Split the main executable and all overlay configs. |
| `make objdiff-objects` | Build target and reconstructed objects for objdiff. |
| `make objdiff-config` | Regenerate `objdiff.json`. |
| `make progress` | Generate `build/progress.json`. |
| `make diff-all` | Run objdiff across all configured units. |
| `make diff-text` | Generate compact text reports under `build/diffs/`. |
| `make dump-objs` | Disassemble built objects for code-generation analysis. |
| `make validate-assets` | Round-trip and validate format-aware assets. |
| `make verify-slus` | Check that the linked main executable equals `disc/SLUS_010.13`. |
| `make verify-bins` | Run `verify-slus` and the whole-overlay SHA-1 check for every overlay. |
| `make verify-compressor` | Verify the compressor against all 17 original overlay files. |
| `make recopy` | Force source/config files to be copied to `/staging` again. |
| `make clean` | Remove build output and `/staging`. |

## Compiler and assembler toolchains

A critical detail of this project is that **not every source file uses the same compiler configuration**.

The build defines 12 pipeline variants across four historical compiler builds. Compiler and assembler flags live in [`mk/toolchains.mk`](mk/toolchains.mk). Source routing and per-file overrides are in [`mk/main.mk`](mk/main.mk) and [`mk/overlay-registry.mk`](mk/overlay-registry.mk); [`mk/overlays.mk`](mk/overlays.mk) applies the overlay variants.

| Pipeline | Compiler flags | Assembly path | Review |
|---|---|---|:---:|
| GCC 2.8.0 G0 (default) | `-O2 -G0 -gcoff -fsigned-char -fno-builtin` | maspsx, ASPSX 2.77, expanded division |  |
| GCC 2.8.0 G0, builtins enabled | `-O2 -G0 -gcoff -fsigned-char` | maspsx, ASPSX 2.77, expanded division | \* |
| GCC 2.8.0 G0, unoptimized | `-O0 -G0 -gcoff -fsigned-char -fno-builtin` | maspsx, ASPSX 2.77, expanded division |  |
| GCC 2.8.0 G0, unoptimized with builtins enabled | `-O0 -G0 -gcoff -fsigned-char` | maspsx, ASPSX 2.77, expanded division | \* |
| GCC 2.8.0 G4 | `-O2 -G4 -gcoff -fsigned-char` | maspsx, ASPSX 2.77, expanded division |  |
| GCC 2.8.0 G4, no division expansion | `-O2 -G4 -gcoff -fsigned-char` | maspsx, ASPSX 2.77, bare division |  |
| GCC 2.7.2 CDK G0 | `-O2 -G0 -msoft-float -gcoff` | maspsx, ASPSX 2.67, expanded division |  |
| GCC 2.7.2 CDK G0, scheduling disabled | `-O2 -G0 -msoft-float -gcoff -fno-schedule-insns` | maspsx, ASPSX 2.67, expanded division | \* |
| GCC 2.7.2 CDK G0, strength reduction disabled | `-O2 -G0 -msoft-float -gcoff -fno-strength-reduce` | maspsx, ASPSX 2.67, expanded division | \* |
| GCC 2.7.2 CDK G0, no division expansion | `-O2 -G0 -msoft-float -gcoff` | maspsx, ASPSX 2.67, bare division | \* |
| GCC 2.7.2 GNU G0 | `-O2 -G0` | Historical GNU `as` with `-O -EL` |  |
| GCC 2.6.0 G0 | `-O2 -G0 -gcoff -msoft-float` | maspsx, ASPSX 2.34, expanded division |  |

\* These variants need further investigation to establish whether their compiler and assembler settings reflect the original build. A match with altered optimization levels, builtin handling, scheduling, strength reduction, or division expansion does not by itself establish a distinct historical toolchain. Cleanup work may find source that produces the same bytes under an established configuration and make those settings unnecessary.

`-G0` and `-G4` select the small-data threshold for GP-relative addressing. The builtin-enabled GCC 2.8.0 variants omit `-fno-builtin`. All maspsx pipelines use `-no-pad-sections`; expanded division adds `--expand-div`, while the no-expansion variants omit it. Modern `mipsel-linux-gnu-` binutils handle linking, binary conversion, and object inspection.

Current examples of the specialized routes include:

- FIELD's `field_select_distance_bucket.c`: GCC 2.8.0 G0 with builtins enabled.
- WMAP's `wmap_effect_resources.c` and `wmap_pathfinding.c`: GCC 2.8.0 G0 at `-O0` with builtins enabled.
- ZUKAN's `zukan_category.c`: GCC 2.8.0 G0 at `-O0` with builtins disabled.
- FIELD's `field_subsystem_init.c`: GCC 2.7.2 CDK with `-fno-schedule-insns`.
- FIELD's `field_actor_action_defaults.c`: GCC 2.7.2 CDK with `-fno-strength-reduce`.
- FIELD's G4 source group: GCC 2.8.0 without division expansion.

The CDK no-division-expansion route is supported, but its source list is currently empty. Per-file assembler and object-conversion overrides, such as CHECKPS's GNU `cdrom.c` route, are also recorded in the overlay registry.

When editing a function, **check the result with the exact toolchain selected for its source file**. Do not substitute the host GCC, Clang/LLVM, a different GCC release, or a different assembler and treat that result as authoritative.

## Copyrighted data and assets

Some executable/overlay ranges contain artwork, text, layouts, and other copyrighted data that should not be committed.

The project uses a hybrid approach:

- understood program data can be represented as typed C;
- understood binary formats can use byte-exact extractors/builders;
- unknown or creative data can remain as named local `databin`/`rodatabin` assets referenced with `.incbin`.

## Repository layout

```text
lom-decomp/
|-- src/                    # Reconstructed C source for the main executable
|   |-- overlays/           # One source tree per overlay (addhero/ ... zukan/)
|   `-- psyq/               # Psy-Q SDK library objects (still original assembly)
|-- include/                # Project headers and assembly macros
|   `-- sdk/                # Psy-Q SDK headers
|-- config/
|   |-- SLUS_010.13.yaml    # Splat config for the main executable
|   |-- overlays/           # Splat config for each overlay
|   |-- symbols/            # Function and global addresses
|   `-- relocations/        # Relocation overrides for splat
|-- mk/                     # Build rules, toolchain definitions, overlay registry
|-- tools/                  # Compressor, asset codecs, splat extensions, objdiff scripts, submodules
|-- docs/                   # Architecture and matching notes
|-- dockerfiles/            # Development and CI containers
|-- disc/                   # Your original game files (gitignored)
|-- asm/                    # Splat-generated assembly (gitignored)
|-- assets/                 # Splat-extracted game data (gitignored)
|-- linker/                 # Splat-generated linker scripts (gitignored)
|-- build/                  # Objects, ELFs, maps, diffs, and reports (gitignored)
|-- Makefile
`-- requirements.txt
```

Useful places to start exploring:

- `src/` and `src/overlays/` - the reconstructed game code.
- `include/` - shared types and structures such as `main.h`, `saved_game.h`, and the per-subsystem field and world map headers.
- `config/symbols/` - known function and global addresses. Update these when renaming a symbol with a fixed address.
- `mk/overlay-registry.mk` - each overlay's source list and toolchain assignments.
- `tools/compressor/` - the overlay compressor and the whole-binary verification script.
- `docs/` - disc layout, CD system architecture, and matching notes.

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

Initialize the Git submodules and build the four historical compiler images from the setup section before building `lom-dev`.

**GCC reports `Value too large for defined data type`**

Use the Makefile instead of compiling directly from `/lom`. Run `make recopy` if the staged tree needs to be refreshed.

**A symbol/config change is not reflected in generated assembly**

Run `make splat` again. Do not edit generated `asm/` or `linker/` files manually.

**objdiff reports 100%, but `make verify-bins` fails**

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
