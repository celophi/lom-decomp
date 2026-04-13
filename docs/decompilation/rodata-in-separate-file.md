# Rodata in a Separate File: Data Matching for Compiler-Visible Constants

## The Problem

Some functions reference a global `const` array that, when defined in the **same
translation unit** as that function, causes the compiler to inline the constants and
generate wrong code.

The specific case is `DrawSymmetricTestPattern` in `src/overlays/checkps/code6.c`:

```c
__builtin_memcpy(mirrorSignBuffer, g_testPatternVertexTable, 8);
```

The target binary produces:

```mips
lui   $v0, %hi(g_testPatternVertexTable)
addiu $v0, $v0, %lo(g_testPatternVertexTable)
lwl   $a0, 0($v0)
lwr   $a0, 3($v0)
lwl   $a1, 4($v0)
lwr   $a1, 7($v0)
swl   $a0, 0($sp)
swr   $a0, 3($sp)
swl   $a1, 4($sp)
swr   $a1, 7($sp)
```

That is: load the address of the table at runtime, then copy 8 bytes with `lwl/lwr/swl/swr`
pairs. This is what the compiler emits when `g_testPatternVertexTable` is an **extern** —
it cannot see the values, so it must load the address and copy from memory.

If instead the array definition is visible in the same translation unit as `code6.c`,
GCC sees the actual bytes `{ 0x01, 0x01, 0xFF, 0x01, 0x01, 0xFF, 0xFF, 0xFF }` at
compile time and replaces the `__builtin_memcpy` with immediate constant stores:

```mips
li    $a0, 0x0101FF01
sw    $a0, 0($sp)
li    $a1, 0x01FFFFFF
sw    $a1, 4($sp)
```

This is shorter and faster, but it does **not** match the target binary. The fix is to
move the definition of `g_testPatternVertexTable` into a separate C file so the compiler
cannot see its values when compiling `code6.c`.

---

## The Solution: `data.c`

The array is defined in `src/overlays/checkps/data.c`:

```c
#include "common.h"

const u8 g_testPatternVertexTable[8] = {
    0x01, 0x01, 0xFF, 0x01, 0x01, 0xFF, 0xFF, 0xFF,
};

const u8 g_testPatternVertexTablePadding[12] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
```

`include/checkps.h` declares it as `extern` so all other translation units (including
`code6.c`) see only the declaration — never the values.

---

## Why `.rodata` Cannot Be Used in `CHECKPS.BIN.yaml`

splat offers two segment types for read-only data:

| yaml type | splat behaviour | linker script generated |
|-----------|----------------|------------------------|
| `rodata`  | Disassembles the binary into `<name>.rodata.s`. The linker script points at the **assembled `.o`** of that `.s` file. | `build/overlays/checkps/asm/…/data.rodata.o(.rodata)` |
| `.rodata` | No `.s` file generated. The linker script points at the **compiled `.o`** of the corresponding C file. | `build/overlays/checkps/src/…/data.o(.rodata)` |

At first glance `.rodata` looks like the right choice: just compile `data.c` and let the
linker use that object. However:

- splat **overwrites** the linker script every time you run `splat split`. It would
  regenerate the entry pointing at `data.o`.
- More importantly, **there is no `.s` reference file** with `.rodata`. objdiff's
  `target` side (the disassembled ground-truth) needs a `.o` assembled from the
  original binary bytes. Without a `.s` file there is nothing to assemble a
  `target/data.o` from.

So `.rodata` is unusable here.

Using `rodata` keeps `data.rodata.s` (the binary-extracted reference) and keeps the
linker script pointing at the assembled reference object — but then there are **two**
objects that both define `g_testPatternVertexTable`: the assembled reference and the
compiled `data.c`. Linking both causes a duplicate-symbol error.

---

## The Full Solution: `nolink_srcs` + Manual `target/data.o` Rule

### 1. yaml — keep `rodata`

`config/overlays/CHECKPS.BIN.yaml` keeps:

```yaml
- [0x91, rodata, data]
```

This tells splat to:
- Disassemble the binary into `asm/overlays/checkps/data/data.rodata.s`.
- Write a linker entry for the assembled `data.rodata.o`.

### 2. Linker script — unchanged

`linker/overlays/checkps/checkps.ld` uses `data.rodata.o` (the assembled reference) for
the actual binary. `data.c` is never passed to the linker.

### 3. Makefile — `nolink_srcs` mechanism

The overlay rule template in the Makefile gains a new per-overlay variable:

```makefile
# Per-overlay list of C files to compile but NOT pass to the overlay linker.
$(1)_NOLINK_SRCS := $(overlay_$(1)_nolink_srcs)
$(1)_NOLINK_OBJS := $(patsubst ...)
$(1)_LINK_OBJS   := $(filter-out $($(1)_NOLINK_OBJS), $($(1)_C_OBJS))
```

The linker rule uses `LINK_OBJS` instead of `C_OBJS`, so `data.o` is compiled (and
therefore exists for objdiff) but is silently excluded from the `ld` invocation.

`data.c` is registered as a nolink source in the overlay registry:

```makefile
overlay_checkps_nolink_srcs := src/overlays/checkps/data.c
```

### 4. Makefile — explicit `target/data.o` rule

The generated overlay rules filter out `asm/overlays/checkps/data/` when building target
objects (because that directory normally contains only reference data, not code). So
`target/data.o` would never be built automatically. An explicit rule is added after the
template instantiation:

```makefile
$(STAGING)/build/overlays/checkps/target/data.o: $(COPY_SENTINEL)
	@mkdir -p $(@D)
	cd $(STAGING) && cat asm/overlays/checkps/data/data.rodata.s | \
		$(MASPSX_PP) $(MASPSX_PP_FLAGS) | \
		$(MASPSX_AS) $(INCLUDE_FLAGS) $(MASPSX_AS_FLAGS_CDK) -o build/overlays/checkps/target/data.o

checkps-target-objects: $(STAGING)/build/overlays/checkps/target/data.o
```

This assembles `data.rodata.s` (the splat-generated reference) into `target/data.o`,
which is the ground-truth that objdiff compares against.

### 5. `generate_objdiff_config.py`

The script auto-generates `objdiff.json` from the splat yaml configs. It only walked
`c`-type subsegments, so `data` (type `rodata`) was never emitted as a unit — meaning
the pipeline would overwrite the manually-added `checkps/data` entry on every run.

Two additions:

- `extract_rodata_subsegments()` — same as `extract_c_subsegments` but filters for
  `subseg[1] == "rodata"`.
- `OVERLAY_NOLINK_SRCS` dict — a registry of which rodata segment names have a compiled
  C counterpart. Only segments listed here are emitted as objdiff units (to avoid
  emitting every binary-only rodata reference segment as a unit).

```python
OVERLAY_NOLINK_SRCS: dict[str, set[str]] = {
    "checkps": {"data"},
}
```

`build_overlay_units()` walks `extract_rodata_subsegments()` and emits a unit for any
name that appears in `OVERLAY_NOLINK_SRCS[overlay_name]`, pointing at:

- `target_path`: `build/overlays/checkps/target/data.o` — assembled from `data.rodata.s`
- `base_path`: `build/overlays/checkps/src/overlays/checkps/data.o` — compiled from `data.c`

---

## Data Flow Summary

```
splat split
  └─ asm/overlays/checkps/data/data.rodata.s   (binary reference)

make checkps-objdiff
  ├─ assemble data.rodata.s  →  target/data.o   (ground truth for objdiff)
  ├─ compile  data.c         →  base/data.o      (your C reconstruction)
  │                                                objdiff diffs these two ✓
  └─ link overlay (data.o excluded from ld)
       uses data.rodata.o instead                  binary is correct ✓

generate_objdiff_config.py
  └─ emits checkps/data unit pointing at both .o files above
       survives pipeline re-runs ✓
```

---

## Adding Another Overlay with the Same Pattern

1. Move the problematic `const` array into its own `<overlay>/data.c`.
2. Declare it `extern` in the overlay's header.
3. Keep the splat yaml entry as `rodata`.
4. Add the file to the Makefile registry:
   ```makefile
   overlay_myoverlay_nolink_srcs := src/overlays/myoverlay/data.c
   ```
5. Add an explicit `target/data.o` rule (copy the checkps one, substituting the overlay
   name).
6. Register it in `generate_objdiff_config.py`:
   ```python
   OVERLAY_NOLINK_SRCS: dict[str, set[str]] = {
       "checkps": {"data"},
       "myoverlay": {"data"},
   }
   ```
