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

`decomp.yaml` (repo root) points coddog at the already-built artifacts. It has one
`versions:` entry per binary -- the main executable (`us`) plus one per overlay:

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
  - name: menu
    fullname: Menu Overlay
    paths:
      target: disc/SLUS_010.13
      build_dir: build/overlays/menu/
      map: build/overlays/menu/menu.map
      compiled_target: build/overlays/menu/menu.elf
      elf: build/overlays/menu/menu.elf
  # ... one entry per remaining overlay: title, movie, gname, gover
```

`target`, `build_dir`, and `compiled_target` are required fields by the yaml schema even
though the overlay `elf` is what actually gets read -- they're just pointed at whatever
exists so parsing succeeds. `checkps` currently has no built `.elf`, so it isn't listed
yet; add it once that overlay builds one.

Since coddog reads the *built* ELF/map, re-run it after building if you want its
decompiled/undecompiled status to reflect recent work.

**Caveat:** because `decomp.yaml` now has more than one version, `match` and `cluster`
(which only auto-pick a version when there's exactly one) will drop into an interactive
"Which version do you want to use?" prompt every time they're run. Answer with `us` for
the previous single-binary behavior, or pick an overlay name to run `match`/`cluster`
against that overlay directly instead of via `compare2`.

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

`compare2` takes two `decomp.yaml` paths and a version name from each, so it can compare
any two binaries directly -- e.g. one overlay against another, or an overlay against the
main executable. Since `decomp.yaml` already lists every overlay as its own version (see
Setup above), pass the same yaml path twice with different version names:

```sh
tools/coddog/target/release/coddog.exe compare2 decomp.yaml menu decomp.yaml title -t 0.6 -m 5 --sort-by similarity
```

This prints paired function names and their similarity, e.g.:

```
Decompiled in Legend of Mana and Legend of Mana:
ResetFadeState - reset_fade_state (99.99%)
SetFadeTarget - set_fade_target (99.99%)
RenderFadeOverlay - render_fade_overlay (96.55%)
```

A near-100% hit between two *already-matched* functions in different overlays just
confirms a shared helper compiled into both (expected -- several overlays share small
utility routines like fade handling). The useful signal is a high score where **one side
is still a `func_XXXXXXXX` stub** -- that's a decompiled sibling in another overlay you can
use as a reference/starting point.

`-t` defaults to `0.985` for `compare2`; drop it to `0.4`-`0.6` when hunting for loose
structural similarity across unrelated overlays, since cross-overlay code tends to score
lower than same-binary duplicates even when related.

`compare-n` does the same thing across many "other" yamls at once against one main
version, but hardcodes its threshold to `0.99`, so it's only useful for near-exact
duplicate hunting, not loose cross-overlay similarity search. `compare-raw` compares a
raw binary blob against one or more yaml-described binaries; not currently useful for LOM
since we always have proper elf/map output to work from.

---

## Suggested workflow

1. Run `cluster` once after a build to get an overview of duplicate groups across the
   whole binary.
2. Before decompiling a `func_XXXXXXXX`, run `match` against any already-named sibling --
   a near-100% hit against an undecompiled function is a quick follow-up win.
3. For functions that are only partially templated, use `submatch` to isolate the shared
   skeleton from the unique parts.
4. When an overlay has few or no matches for its remaining `func_XXXXXXXX`/low-percentage
   functions, use `compare2` (see above) to cross-check it against the main binary and
   other overlays at a low threshold (`0.4`-`0.6`) -- a decompiled sibling elsewhere is a
   strong head start even if it never reaches 99%.
