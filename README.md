# Legend of Mana PSX Decompilation

**Languages:** English | [日本語](README_JP.md)

[![US Progress]][us progress site]
[![JP Progress]][jp progress site]
[![Build and Progress](https://github.com/celophi/lom-decomp/actions/workflows/progress.yaml/badge.svg)](https://github.com/celophi/lom-decomp/actions/workflows/progress.yaml)

[US Progress]: https://decomp.dev/celophi/lom-decomp/SLUS_010.13.svg?mode=shield&measure=code&label=US%20Progress
[us progress site]: https://decomp.dev/celophi/lom-decomp/SLUS_010.13
[JP Progress]: https://decomp.dev/celophi/lom-decomp/SLPS_021.70.svg?mode=shield&measure=code&label=JP%20Progress
[jp progress site]: https://decomp.dev/celophi/lom-decomp/SLPS_021.70

A **matching decompilation** of the PlayStation game **Legend of Mana**. The project reconstructs readable C source code that compiles to the same machine code as the original game. The North American and Japanese releases are both **fully linked** from the same C sources.

This project is a decompilation. It is **not** a recompilation. It is **not** a source port. The repository does not include the game executable, overlay binaries, artwork, audio, or other copyrighted game data. You must provide the required files from your own legally obtained copy of the game.

The goal of this project is to preserve the original game's logic and behavior for research and modding.

## Status

For both versions, every module - the main executable and all 17 overlays - is **fully linked**. This means the same thing as in other decompilation projects, with one additional requirement: the compressor output must match the `.BIN` file on the disc byte for byte. That extra check was a useful way to track progress. A module is fully linked when two conditions hold:

1. The build produces an **ELF whose bytes match the original decompressed file**, and
2. Running the project's compressor on that ELF (stripped to a raw binary) **reproduces an exact replica of the `.BIN` file as it appears on the disc**.

In other words, the round trip is bit-identical:

1. Start with the original `.BIN` from the disc.
2. Decompress it.
3. Compile the C source into an ELF.
4. Compress the ELF.
5. The result is the same `.BIN`.

The main executable is not compressed, so for `SLUS_010.13` and `SLPS_021.70` condition 1 is the whole check: the linked ELF, converted to a raw binary, equals the disc file.
*(Check out the compressor! It's honestly really amazing that it is **bit identical** and kind of extraneous, but cool nonetheless!)*

Run `make verify-bins` to check every module, or `make verify-bins VERSION=jp` for the Japanese version.

## Supported versions

| Item | North America | Japan |
|---|---|---|
| Disc serial | `SLUS-01013` | `SLPS-02170` |
| Main executable SHA-1 | `d11dfdd50d412ac3fa3e2eb80fbde138da118f27` | `b067188a92e4de9a4db7bb7e5343c757e9884bfa` |
| Disc image (`.bin`) SHA-1 | `c1b536c99f0d390584eb30462a7e37f2bbef3902` | `7a314615be8a482cf3f81b4101cc19aaa738f36d` |

Other regional versions are not currently supported.

## Documentation

Browse the [documentation index](docs/README.md) for architecture guides and
project references. Useful starting points include:

- [Tools index](tools/README.md) - asset tools, scene extraction and test commands.

## Requirements

You need:

- **Git**
- **Docker** - Docker Desktop on Windows/macOS or Docker Engine on Linux.
- A **legally obtained copy of Legend of Mana**, North American or Japanese.
- A tool that can extract files from a PlayStation disc image.

Everything else is provided by the development container.

## Getting started

These steps use the North American version. For the Japanese version, put its files under `disc/jp/` and add `VERSION=jp` to each `make` command.

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

Extract the main executable and the `BIN` directory from your disc image into `disc/us/`:

```text
disc/
`-- us/
    |-- SLUS_010.13
    `-- BIN/
        |-- ADDHERO.BIN
        |-- CARDA.BIN
        `-- ...
```

The build only needs these files. The scene extraction tools also read the disc's `ANA/` directory, which you can pass in from wherever you keep it.

> `disc/` is gitignored. Never commit original game files.

### 3. Build the Docker images

Run these from the repository root. You only need to do this once.

```bash
docker build -t old-gcc/gcc-2.8.0-psx -f tools/external/old-gcc/gcc-2.8.0-psx.Dockerfile tools/external/old-gcc
docker build -t old-gcc/gcc-2.7.2-cdk -f tools/external/old-gcc/gcc-2.7.2-cdk.Dockerfile tools/external/old-gcc
docker build -t old-gcc/gcc-2.6.0-psx -f tools/external/old-gcc/gcc-2.6.0-psx.Dockerfile tools/external/old-gcc
docker build -t old-gcc/gcc-2.7.2-psx-gnu -f dockerfiles/gnu-as.dockerfile tools/external/old-gcc
docker build -t lom-toolchain -f dockerfiles/toolchain.dockerfile .
docker build -t lom-dev -f dockerfiles/dev.dockerfile .
```

### 4. Start the container

Run this from the repository root.

PowerShell, bash, or zsh:

```bash
docker run --rm -it -v "${PWD}:/lom" lom-dev
```

Windows Command Prompt (`cmd.exe`):

```bat
docker run --rm -it -v "%cd%:/lom" lom-dev
```

The remaining commands run **inside the container**.

### 5. Split the original binaries

```bash
make splat
```

This checks your files against the expected SHA-1 hashes, then generates the assembly, linker scripts, and assets the build needs.

### 6. Build and verify

```bash
make everything
make verify-bins
```

`make everything` builds the main executable and all 17 overlays into `build/us/`. `make verify-bins` checks that every one of them is fully linked.

## Troubleshooting

**`make splat` reports a missing file or SHA-1 mismatch**

Make sure the files are at `disc/us/SLUS_010.13` and `disc/us/BIN/*.BIN` (or `disc/jp/SLPS_021.70` and `disc/jp/BIN/*.BIN` for the Japanese version), without renaming them. A SHA-1 mismatch with the right paths usually means the files come from a different release or a bad dump; compare them with [Supported versions](#supported-versions).

**Docker cannot find an `old-gcc/...` image**

Initialize the Git submodules, then build the Docker images in the order shown in step 3 of [Getting started](#getting-started).

**GCC reports `Value too large for defined data type`**

The old 32-bit compilers cannot read files on Windows-backed Docker mounts, so the Makefile copies sources to `/staging` inside the container and compiles from there. Use the Makefile instead of compiling directly from `/lom`. Run `make recopy` if the staged tree needs to be refreshed.

## Make targets

| Target | Purpose |
|---|---|
| `make` | Build the main `SLUS_010.13` ELF. |
| `make bin` | Also produce `build/us/SLUS_010.13.bin`. |
| `make <overlay>` | Build one overlay, such as `make field`. |
| `make overlays` | Build all 17 overlays. |
| `make everything` | Build the main executable and all 17 overlays. |
| `make splat` | Decompress the overlays and split every binary into assembly, linker scripts, and assets. |
| `make objdiff-objects` | Build target and reconstructed objects for objdiff. |
| `make objdiff-config` | Regenerate `objdiff.json`. Open the repository root in objdiff to compare functions. |
| `make progress` | Generate `build/us/progress.json`. |
| `make diff-all` | Run objdiff across all configured units. |
| `make diff-text` | Report functions that do not match yet, under `build/us/diffs/`. |
| `make dump-objs` | Disassemble built objects for code-generation analysis. |
| `make validate-assets` | Round-trip and validate format-aware assets. |
| `make verify-main` | Check that the linked main executable equals `disc/us/SLUS_010.13` (`verify-slus` is an alias). |
| `make verify-<overlay>` | Check one overlay against its disc file, such as `make verify-field`. |
| `make verify-bins` | Check the main executable and all 17 overlays against the disc files. |
| `make verify-compressor` | Verify the compressor against the 17 original overlay files of the selected version. |
| `make recopy` | Force source/config files to be copied to `/staging` again. |
| `make clean` | Remove build output and `/staging`. |

Every target builds the North American version by default. Add `VERSION=<name>` to select another release, e.g. `make VERSION=jp`.

Tool tests and data extraction targets are listed in [tools/README.md](tools/README.md).

## Repository layout

```text
lom-decomp/
|-- src/
|   |-- common/        # Code shared by several overlays
|   |-- main/          # Main executable
|   |-- overlays/      # The 17 overlays
|   `-- psyq/          # Psy-Q SDK libraries
|-- include/           # Headers
|-- config/<version>/  # Splat configs, symbols, relocations
|-- mk/                # Build rules
|-- tools/             # Build, data, and verification tools
|-- docs/              # Architecture and reference docs
|-- dockerfiles/       # Development and CI images
`-- disc/<version>/    # Your game files (gitignored)
```

`asm/`, `linker/`, `assets/` and `build/` are generated per version and gitignored.

## Compiler and assembler toolchains

A critical detail of this project is that **not every source file uses the same compiler configuration**.
Honestly, it was a lot of "fun" dealing with this. Especially the GNU one let me tell you.

The build defines 7 pipeline variants across four historical compiler builds. Compiler and assembler flags live in [`mk/toolchains.mk`](mk/toolchains.mk). Source routing is in [`mk/main-sources.mk`](mk/main-sources.mk) for the main executable and [`mk/overlay-registry.mk`](mk/overlay-registry.mk) for the overlays. Per-file overrides for the main executable are in [`mk/main.mk`](mk/main.mk); [`mk/overlays.mk`](mk/overlays.mk) applies the overlay variants.

| Pipeline | Compiler flags | Assembly path |
|---|---|---|
| GCC 2.8.0 G0 (default) | `-O2 -G0 -gcoff -fsigned-char` | maspsx, ASPSX 2.77, expanded division |
| GCC 2.8.0 G0, unoptimized | `-O0 -G0 -gcoff -fsigned-char` | maspsx, ASPSX 2.77, expanded division |
| GCC 2.8.0 G4 | `-O2 -G4 -gcoff -fsigned-char` | maspsx, ASPSX 2.77, expanded division |
| GCC 2.8.0 G4, no division expansion | `-O2 -G4 -gcoff -fsigned-char` | maspsx, ASPSX 2.77, bare division |
| GCC 2.7.2 CDK G0 | `-O2 -G0 -msoft-float -gcoff` | maspsx, ASPSX 2.67, expanded division |
| GCC 2.7.2 GNU G0 | `-O2 -G0` | Historical GNU `as` with `-O -EL` |
| GCC 2.6.0 G0 | `-O2 -G0 -gcoff -msoft-float` | maspsx, ASPSX 2.34, expanded division |

## Reverse-engineering provenance

This project was created independently by analyzing the publicly released retail version of Legend of Mana and reconstructing its behavior and machine code through disassembly, decompilation, binary comparison, runtime analysis, and publicly available technical documentation and tools.

No leaked or otherwise non-public *Legend of Mana* source code, debug symbol files, internal symbol maps, developer documentation, or other confidential materials from Square or Square Enix have been used in the creation of this project.

Function names, variable names, data structures, translation-unit boundaries, and other source-level details are reconstructed or inferred from the retail binaries and observed behavior unless otherwise documented. They should not be assumed to be the names or organization used by the original developers.

This repository will **never** include leaked source code, private debug symbols, confidential documentation, or other non-public materials from the original game's development.

## Legal

See [LICENSE](LICENSE) and [THIRD_PARTY_NOTICES](THIRD_PARTY_NOTICES.md) for licensing details.

The MIT License applies to original material authored for this project to the extent that the contributors hold the rights necessary to license that material. It does not grant rights to preexisting copyrighted material originating from the original game or from third-party SDKs, libraries, or development software.

This repository is an independent reverse-engineering and preservation project. It is not affiliated with or endorsed by Square, Square Enix, Sony, or any other rights holder.

No original game executable, overlay binaries, artwork, audio, or other copyrighted game data should be committed to this repository. You must supply required data from your own legally obtained copy of the game.

*Legend of Mana* and related names and assets are the property of their respective owners.

## Thanks

Thanks to Squaresoft and everyone who worked on *Legend of Mana*. The art, the music, and all the strange little ideas make this a game worth coming back to. You can tell the team was willing to try things, and that's a big part of what makes it interesting.

Figuring out how it all works has been a lot of fun. There's still plenty to understand, but hopefully this project helps preserve that work and gives other people a chance to explore it too.

## Tools and acknowledgements

This project builds on tools and research from the wider decompilation community, including:

- [splat](https://github.com/ethteck/splat)
- [spimdisasm](https://github.com/Decompollaborate/spimdisasm)
- [maspsx](https://github.com/mkst/maspsx)
- [old-gcc](https://github.com/decompals/old-gcc)
- [objdiff](https://github.com/encounter/objdiff)
- [decomp-permuter](https://github.com/simonlindholm/decomp-permuter)
- [m2c](https://github.com/matt-kempster/m2c)
- [decomp.me](https://decomp.me)
- [decomp.dev](https://decomp.dev)
