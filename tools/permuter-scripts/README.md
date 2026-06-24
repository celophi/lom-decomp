# permuter-scripts

Helper scripts for running [decomp-permuter](../decomp-permuter/) against Legend of Mana functions.

All commands are run from `/staging` inside the dev container.

---

## Quick start

```bash
# Set up a permuter directory and start searching
tools/permuter-scripts/permute-setup.sh src/cdrom.c asm/cdrom.s cdrom_complete_command
python3 tools/permuter-scripts/permute-ctl.py start /permute/cdrom_complete_command --best-only

# Check progress
python3 tools/permuter-scripts/permute-ctl.py status /permute/cdrom_complete_command

# Print the best candidate found
python3 tools/permuter-scripts/permute-ctl.py best /permute/cdrom_complete_command

# Stop and wipe candidates when done
python3 tools/permuter-scripts/permute-ctl.py clean /permute/cdrom_complete_command
```

---

## Toolchain selection

Check the Makefile to find which list your source file appears in:

| Makefile variable   | `--toolchain` flag |
|---------------------|--------------------|
| `SRCS_G0` (default) | `gcc280`           |
| `SRCS_G4`           | `gcc280-g4`        |
| `SRCS_CDK_G0`       | `cdk`              |
| `SRCS_GCC_260_G0`   | `gcc260`           |
| (gnuas files)       | `gnuas`            |

```bash
# Example: src/cdrom.c is in SRCS_G4
tools/permuter-scripts/permute-setup.sh src/cdrom.c asm/cdrom.s cdrom_complete_command --toolchain gcc280-g4
```

---

## Scripts

### `permute-setup.sh`

Sets up a permuter directory for one function. Handles:
- Extracting the function + any jump tables from the splat-generated `.s` file
- Writing the correct `permuter_settings.toml` for the chosen toolchain
- Running `import.py` to produce `base.c`, `target.o`, and `compile.sh`

```
tools/permuter-scripts/permute-setup.sh <c_file> <asm_file> <func_name> [options]

Options:
  --toolchain  gcc280 | gcc280-g4 | cdk | gcc260 | gnuas  (default: gcc280)
  --out-dir    output directory                            (default: /permute/<func_name>)
```

### `permute-ctl.py`

Start/stop/query permuter sessions.

```
python3 tools/permuter-scripts/permute-ctl.py start  <dir> [-j N] [--best-only]
python3 tools/permuter-scripts/permute-ctl.py stop   <dir>
python3 tools/permuter-scripts/permute-ctl.py status <dir>
python3 tools/permuter-scripts/permute-ctl.py best   <dir>
python3 tools/permuter-scripts/permute-ctl.py clean  <dir>
```

| Command  | Description |
|----------|-------------|
| `start`  | Launch permuter in background, log to `<dir>/permuter.log` |
| `stop`   | Send SIGTERM to the running permuter |
| `status` | Show running state, best score, and last 20 log lines |
| `best`   | Print the source of the best candidate found so far |
| `clean`  | Stop permuter, delete all `output-*` dirs and log |

### `extract_func.py`

Extract a single function and its jump tables from a splat-generated `.s` file.
Called automatically by `permute-setup.sh` — you usually don't need this directly.

```
python3 tools/permuter-scripts/extract_func.py <asm_file> <func_name> [-o output.s]
```

Jump tables (`jtbl_XXXXXXXX`) are auto-detected from the function body and pulled from
the rodata files (`asm/data/*.rodata.s` or `asm/overlays/<name>/data/*.rodata.s`).

### `maspsx_asm.sh`

Argument-reorder wrapper around `maspsx.py` for use as the `assembler_command` in
permuter settings. `import.py` calls assemblers as `[flags] input.s -o output.o` but
maspsx expects the input file last — this wrapper fixes the order.

Called automatically by `permute-setup.sh`. Not meant for direct use.

### `prelude.inc`

Custom copy of `decomp-permuter/prelude.inc` with `.set gp=64` removed.
`mipsel-linux-gnu-as` rejects that directive in 32-bit mode and silently produces
no output, so it must be omitted.

---

## Notes

- The permuter directory is created at `/permute/<func_name>` by default. `/permute/` is
  ephemeral (container-local) — candidates are lost when the container is recreated.
- Scores are additive diffs vs the target assembly; **lower is better**, 0 = perfect match.
- The permuter occasionally logs `internal permuter failure` due to a type-inference edge
  case in `randomizer.py`. This is benign — it skips that candidate and continues.
- Always run from `/staging`, not from `/lom` — the compilers use relative include paths.
