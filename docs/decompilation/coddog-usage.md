# Using coddog to Find Duplicate and Similar Functions

> [coddog](https://github.com/ethteck/coddog) is a Rust CLI that compares functions inside
> (and across) compiled binaries by their instruction sequences. It reads project metadata
> from `decomp.yaml` and works directly off the built ELF and map file, so it needs no
> source changes to run.

---

## Setup

coddog lives as a git submodule at `tools/coddog`. Build the CLI once:

```sh
cd tools/coddog
cargo build --release -p coddog-cli
```

The binary is produced at `tools/coddog/target/release/coddog.exe`. Add that directory to
your `PATH`, or invoke it by full path, from the **repo root** (`decomp.yaml` lives there
and coddog walks up from the current directory looking for it).

`decomp.yaml` (repo root) points coddog at the already-built artifacts:

```yaml
name: Legend of Mana
platform: psx
build_system: make
versions:
  - name: us
    fullname: SLUS-01013 (US)
    sha1: d11dfdd50d412ac3fa3e2eb80fbde138da118f27
    paths:
      target: disc/SLUS_010.13
      build_dir: build/
      map: build/SLUS_010.13.map
      compiled_target: build/SLUS_010.13.bin
      elf: build/SLUS_010.13.elf
```

Since coddog reads the *built* ELF/map, re-run it after building if you want its
decompiled/undecompiled status to reflect recent work.

---

## Commands

### `cluster` -- find duplicate or near-duplicate functions

```sh
tools/coddog/target/release/coddog.exe cluster
```

Groups functions in the binary by similarity. Useful for spotting candidates for a shared
helper, or a routine that's already been decompiled elsewhere under a different name.
Flags:

- `-t, --threshold <0.0-1.0>` -- similarity cutoff (default `0.985`); lower to catch looser
  matches.
- `-m, --min-len <N>` -- minimum instruction count to consider (default `5`); raise to
  ignore trivial stub functions.

### `match <symbol>` -- find what else looks like a known function

```sh
tools/coddog/target/release/coddog.exe match akao_play_song
```

Lists other functions similar to `akao_play_song`, ranked by similarity percentage, and
notes whether each has already been decompiled. If you've just matched a `func_XXXXXXXX`
and it comes back 99%+ similar to several other `func_` stubs, those are near-zero-effort
functions to decompile next -- likely copy-pasted or templated from the same source.

### `submatch <symbol> <window_size>` -- find shared instruction chunks

```sh
tools/coddog/target/release/coddog.exe submatch some_func 8
```

Instead of whole-function similarity, finds shared runs of `window_size` instructions
across functions. Useful when only part of a function is templated (e.g. a repeated loop
idiom inside otherwise-distinct opcode handlers), and complements the `find_idioms.py`
snippet-search workflow.

### `compare2` / `compare-n` / `compare-raw` -- cross-binary comparison

Experimental; compares this project's binary against another project's `decomp.yaml` (or a
raw binary). Not currently used for LOM, but could help identify functions shared with
other Square PS1 titles (e.g. Front Mission, SaGa) via their `decomp.yaml`s once such
projects exist.

---

## Suggested workflow

1. Run `cluster` once after a build to get an overview of duplicate groups across the
   whole binary.
2. Before decompiling a `func_XXXXXXXX`, run `match` against any already-named sibling --
   a near-100% hit against an undecompiled function is a quick follow-up win.
3. For functions that are only partially templated, use `submatch` to isolate the shared
   skeleton from the unique parts.
