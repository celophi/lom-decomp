# Legend of Mana PSX Decompilation

[![Progress]][progress site]

[Progress]: https://decomp.dev/celophi/lom-decomp.svg?mode=shield&measure=code&category=all&label=Progress
[progress site]: https://decomp.dev/celophi/lom-decomp

## About

This project is an in-progress matching decompilation of Legend of Mana for PlayStation 1. Specifically, it targets the US North America edition (SLUS_010.13) released on June 6, 2000. The goal is to convert the original game binary back into human-readable C code that compiles to identical machine code, preserving the original game's logic and behavior while enabling preservation, modding, and educational research.

**Future Goals**: In time, support may be added for the JP version, which includes リングりんぐランド content for PocketStation.

## Prerequisites

- **Git** — For cloning the repository
- **Docker Desktop** — Required for the build environment ([Download here](https://www.docker.com/products/docker-desktop))
- **Legend of Mana ROM** — Your legally obtained copy of the US North America edition (SLUS_010.13)

## Getting Started

### 1. Clone the Repository

Open a terminal (PowerShell or Command Prompt on Windows) and run:

```bash
git clone https://github.com/celophi/lom-decomp.git
cd lom-decomp
git submodule update --init --recursive
```

This downloads the project and all its dependencies.

### 2. Add Your Game ROM

Extract the entire disc contents from your Legend of Mana disc and place all files into the `disc/` folder in the project directory. You can use tools like IsoBuster or similar disc extraction utilities to extract the complete disc image.

> **Note:** This process will be automated in the future for a better user experience.

### 3. Build the Compiler Containers

The project uses old PlayStation compilers that run in Docker. Build them with:

```bash
docker build -t old-gcc/gcc-2.8.0-psx -f tools/old-gcc/gcc-2.8.0-psx.Dockerfile tools/old-gcc
docker build -t old-gcc/gcc-2.7.2-cdk -f tools/old-gcc/gcc-2.7.2-cdk.Dockerfile tools/old-gcc
```

This step may take several minutes.

### 4. Build the Development Environment

```bash
docker build -t lom-dev .
```

### 5. Start the Development Environment

Launch the container with your project files mounted:

```bash
docker run --rm -ti -v "${PWD}:/lom" lom-dev
```

You should now be inside the container at a Linux terminal prompt.

### 6. Split the Game Binary

Inside the container, run splat to extract the game binary into assembly files:

```bash
splat split config/SLUS_010.13.yaml
splat split config/overlays/CHECKPS.BIN.yaml
```

### 7. Build the Project

```bash
make clean
make
make bin
```

The compiled output will be in the `build/` directory as both an ELF executable and a BIN file.

## Workflow

After initial setup, your typical development workflow is:

1. Start the container: `docker run --rm -ti -v "${PWD}:/lom" lom-dev`
2. Make your code changes (outside the container, in your editor)
3. Build inside the container: `make clean && make && make bin`
4. Test the output from the `build/` directory

### Make Targets

| Target | Description |
|---|---|
| `make` | Build the main SLUS executable |
| `make bin` | Also produce a raw `.bin` binary |
| `make checkps` | Build just the CHECKPS overlay |
| `make overlays` | Build all overlays |
| `make everything` | Build SLUS + all overlays |
| `make objdiff-objects` | Build target and base objects for diffing |
| `make clean` | Remove all build artifacts |
| `make recopy` | Force re-copy of source files to `/staging` |

## Technical Notes

- Current research points to a Psy-Q SDK version of 4.6 for the main binary, and 4.1 for the CHECKPS overlay
- Main executable functions match around GCC 2.8.0
- Target architecture: MIPS (PlayStation 1)

## Matching Functions

Use the objdiff tool for local function-by-function comparison:

```bash
make objdiff-objects
python3 tools/objdiff/generate_objdiff_config.py
```

Then open [objdiff](https://github.com/encounter/objdiff) and point it at the project root.

You can also use [decomp.me](https://decomp.me) for collaborative matching:

- **Platform:** PlayStation 1
- **Compiler:** `gcc 2.8.0-psx + maspsx`
- **Flags:** `-Wall -Wa,--aspsx-version=2.79,--expand-div -g3 -O2 -G0`

## Project Structure

```
lom-decomp/
├── .github/
│   └── workflows/
│       └── progress.yaml           # CI: builds objects and uploads a progress report
├── asm/                            # splat-generated MIPS assembly (gitignored)
│   ├── data/                       # Initialized data and sdata sections
│   ├── nonmatchings/               # Functions not yet matched to C
│   │   ├── psyq/                   # Unmatched PSY-Q SDK functions
│   │   └── unk1/ ... unk6/         # Unmatched functions in each translation unit
│   ├── overlays/
│   │   └── checkps/                # CHECKPS overlay assembly
│   └── psyq/                       # PSY-Q SDK library assembly
├── assets/
│   └── checkps.bin                 # CHECKPS overlay binary data segment
├── build/                          # All compiled build artifacts (gitignored)
│   ├── slus_010.13.elf             # Main executable (ELF)
│   ├── slus_010.13.bin             # Main executable (raw binary)
│   ├── asm/                        # Assembled target objects (for objdiff)
│   ├── src/                        # Compiled base objects (for objdiff)
│   └── overlays/
│       └── checkps/                # CHECKPS overlay build artifacts
├── config/                         # splat configuration files
│   ├── overlays/
│   │   └── CHECKPS.BIN.yaml        # splat config for the CHECKPS overlay
│   ├── SLUS_010.13.yaml            # Main splat config for the game binary
│   └── symbol_addrs.txt            # Known symbol addresses
├── disc/                           # ROM files go here (gitignored)
│   └── SLUS_010.13                 # Main game executable
├── docs/
│   └── disc-layout.md              # Notes on disc structure and file formats
├── include/                        # C header files
│   ├── psyq/                       # PSY-Q SDK headers (libcd, libetc, libgpu, etc.)
│   ├── common.h                    # Shared type definitions
│   └── *.h                         # Per-translation-unit headers
├── linker/                         # Linker scripts (generated by splat)
│   ├── overlays/
│   │   └── checkps/
│   ├── slus_010.13.ld              # Main linker script
│   ├── undefined_funcs_auto.txt    # Auto-generated undefined function references
│   └── undefined_syms_auto.txt     # Auto-generated undefined symbol references
├── src/                            # Decompiled C source files
│   ├── overlays/
│   │   └── checkps/                # CHECKPS overlay source (code.c, unk.c)
│   ├── psyq/                       # Decompiled PSY-Q SDK library source
│   │   ├── libcd/SYS.c
│   │   └── libetc/INTR.c
│   ├── cd.c                        # CD-ROM subsystem
│   ├── decompression.c             # Data decompression routines
│   ├── main.c                      # Entry point
│   └── unk1.c ... unk6.c           # Translation units pending identification
├── tools/
│   ├── decomp-permuter/            # Automated C permutation for function matching
│   ├── maspsx/                     # GCC → ASPSX assembly preprocessor (submodule)
│   ├── objdiff/                    # Function diffing and progress report generation
│   │   ├── generate_objdiff_config.py
│   │   └── objdiff-cli-linux-x86_64
│   ├── old-gcc/                    # Dockerized historical GCC compilers (submodule)
│   ├── splat/                      # Binary splitting and disassembly (submodule)
│   ├── splat_ext/                  # Custom splat extensions for LOM's compression
│   └── compile.sh                  # Compiler wrapper used by decomp-permuter
├── Dockerfile                      # Development environment definition
├── Makefile                        # Build system
└── requirements.txt                # Python dependencies
```

### Build Pipeline

When you compile a `.c` file, four things happen in order:

1. **GCC 2.8.0** compiles C source to MIPS assembly text (`gcc -S`)
2. **maspsx** translates GCC's assembly syntax into Sony ASPSX format
3. **The system assembler** turns that into a `.o` object file
4. **mipsel-linux-gnu-ld** links all `.o` files into an ELF executable

The CHECKPS overlay uses a different compiler: **CCPSX.EXE** (PSY-Q SDK 4.1 / GCC 2.7.2 CKD), run via **wibo** on Linux. Its output is converted from PSY-Q `.obj` format to ELF `.o` by **psyq-obj-parser** before linking.

Source files are compiled from a `/staging` directory (a native Linux path inside the container) rather than directly from the `/lom` mount, because GCC 2.8.0 cannot write to mounted Windows filesystems.

### Overlays

Overlays are small executables loaded on top of the main SLUS binary at runtime. Each overlay has its own source directory, linker scripts, and build artifacts under the corresponding subdirectories. To add a new overlay:

1. Create `config/overlays/<name>.yaml` (use `CHECKPS.BIN.yaml` as a template)
2. Run `splat split config/overlays/<name>.yaml`
3. Register it in the `Overlay Registry` section of the `Makefile`

## Contributing

I'm focused on improving this project and welcome contributions from anyone interested in helping. Here are specific ways you can help:

### Decompiling Functions

- Improve a match on [decomp.me](https://decomp.me)
- Test it locally with objdiff to verify correctness
- Submit a pull request with your improvements

### Documentation and Code Quality

- Document functions, variables, arguments, and globals
- Rename symbols to be more descriptive and meaningful
- Improve comments and explain complex logic
- Enhance overall code clarity and maintainability

### Tooling and Automation

- Improve the build process automation
- Enhance and refactor scripts
- Document workflows and pipelines
- Implement features that make development easier

### Other Ideas

I'm open to any other ideas or improvements you might have. Feel free to open an issue to discuss your thoughts!

## Troubleshooting

**Docker commands fail** - Ensure Docker Desktop is running.

**Windows issues** - Use PowerShell or Command Prompt, not Git Bash, for the `docker run` command.

**Path expansion** - The `${PWD}` variable should automatically expand to your current directory path.

**GCC can't write files** - This is expected when running GCC directly on the `/lom` mount. Always compile from inside the container where the `/staging` copy is used.

**Making an ELF fails** - Most likely reason is because overlapping regions caused by non-matching functions.

## Thanks

This project makes use of a variety of tools:

- [splat](https://github.com/ethteck/splat) — Binary splitting and disassembly
- [maspsx](https://github.com/mkst/maspsx) — GCC to ASPSX assembly preprocessor
- [spimdisasm](https://github.com/Decompollaborate/spimdisasm) — MIPS disassembly
- [objdiff](https://github.com/encounter/objdiff) — Function diffing and progress tracking
- [decomp-permuter](https://github.com/simonlindholm/decomp-permuter) — Automated C permutation for function matching
- [old-gcc](https://github.com/decompals/old-gcc) — Dockerized historical GCC compilers
- [wibo](https://github.com/decompals/wibo) — Windows binary runner (used to run CCPSX.EXE on Linux)
- [psyq-obj-parser](https://github.com/mkst/psyq-obj-parser) — PSY-Q `.obj` to ELF `.o` converter
- [decomp.me](https://decomp.me) — Collaborative decompilation platform
- [decomp.dev](https://decomp.dev) — Progress tracking and visualization

## Legal

This project is for educational and archival purposes only. You must own a legal copy of Legend of Mana to extract the necessary resources. No copyrighted assets are distributed.

<img src="docs/lil-cactus.png" width="84" alt="Lil' Cactus"/>
