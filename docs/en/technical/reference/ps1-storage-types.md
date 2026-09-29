# PS1 storage types

`include/ps1_types.h` defines the scalar types and stored object-pointer types
shared by the matching build and native consumers. `common.h` includes it.
The header can also be included independently, without the assembly macros.

The native port previously rewrote many pointer fields and globals to `u32`
and inserted casts at their uses. The original slots are four bytes, while
ordinary pointers on a 64-bit host are eight bytes. Changing the slot size
would move adjacent fields and change array strides. The stored-pointer
typedefs preserve the four-byte slot and ordinary pointer expressions.

## Names and build selection

| Type | Pointee |
| --- | --- |
| `voidptr` | `void` |
| `u8ptr`, `s8ptr` | Unsigned or signed byte |
| `u16ptr`, `s16ptr` | Unsigned or signed halfword |
| `u32ptr`, `s32ptr` | Unsigned or signed word |
| `ulongptr` | Psy-Q `u_long` |
| `<Type>Ptr` | A structure, declared beside its owning type |
| `<Type>TablePtr` | A stored address of a table of `<Type>Ptr` slots |

With the historical compiler, each alias is the original pointer type. No
native flags are added to the PS1 toolchain, and no instructions, registers,
or generated assembly are patched.

A native consumer enables the types with:

```text
-DLOM_NATIVE -fms-extensions
```

This selects Clang's `__ptr32 __uptr` pointer representation. `__ptr32` keeps
storage at four bytes; `__uptr` widens an address with zero extension. In
particular, `0x80000000` must become `0x0000000080000000`, not a sign-extended
host address. The header rejects native builds without the required compiler
extensions and asserts the stored pointer width.

The same header gives `u_long` a four-byte unsigned representation on native
builds. `Ps1Long` preserves the historical `long` type in matching builds and
uses a four-byte signed integer in native builds. The SDK `MATRIX` and `VECTOR`
data fields use it; the scalar SDK function declarations still use their
original `long` signatures. Data layout and the native SDK's call ABI must be
checked separately.

## Declarations and access

```c
typedef struct FieldScriptFrame
{
    u8ptr pc;
    u32 flags;
    u32 wait_frames;
} FieldScriptFrame;

u8* cursor = frame->pc;
frame->pc = cursor + 2;
opcode = *frame->pc++;
```

The local `cursor` is an ordinary working pointer. Clang inserts width
conversions at the field accesses. No `TO_PTR` or `FROM_PTR` macros are needed
for these object pointers.

Define a structure-specific alias after its forward declaration or definition:

```c
typedef struct FieldImageReq FieldImageReq;
typedef FieldImageReq* PS1_PTR32 FieldImageReqPtr;
```

`PS1_PTR32` is a qualifier used in typedef definitions, not a function-like
wrapper around every declaration. It expands to nothing for the historical
compiler. A typedef for a function-local structure must be declared in that
same scope; another function's typedef of the same name is a separate type.

Pointer tables need care at both levels:

```c
typedef FieldPartDef* PS1_PTR32 FieldPartDefPtr;
typedef FieldPartDefPtr* PS1_PTR32 FieldPartDefTablePtr;

FieldPartDefTablePtr stored_table;
FieldPartDefPtr* table_cursor = stored_table;
```

The stored table address and each entry occupy four bytes. The local table
cursor has the host's normal pointer width and advances by four bytes per
entry. Likewise, `u8ptr*` can point at a stored byte-pointer slot, while `u8**`
points at a normal working pointer. Do not interchange these on native builds.
The FIELD scene/text packet-cursor APIs take `u8ptr*` because their caller passes
the address of `FieldRenderHalf.primitive_cursor`. Their ordinary local packet
pointers remain `u8*`; allocation helpers that take an ordinary local arena
cursor still use `u8**`.

## Scope and remaining native work

The initial migration covers the object-pointer storage identified by the
native patches in AKAO, FIELD scripts/text/actors/resources, WMAP resources and
rendering, and WSEL. Matching declarations of those globals were updated across
their consumers. WMAP repeats many extern declarations and private types inside
functions, so the declaration count is much larger than the distinct type count.

This is not a conversion of every pointer in the game. Runtime scene records
that the native port allocates with host layouts retain their existing pointer
types. Locals, call parameters, native callbacks, encoded GPU links, and numeric
fields that sometimes hold addresses require their own representation review.

Stored pointers require a valid mapped address below 4 GiB. Assigning an
ordinary host allocation outside that range truncates the address. The typedefs
do not allocate low memory, map PS1 RAM, resolve retail code addresses to native
callbacks, decode tagged links, or fix integer-to-pointer arithmetic elsewhere.
An integer field used as an address may still need an explicit conversion.

The `lom-native` submodule pin and patch series are unchanged by this work.
Adopting this revision there requires the following dependency update:

1. Enable `LOM_NATIVE` and `-fms-extensions` for the relevant native game units.
2. Include `ps1_types.h` in native compilation shims. Existing shims that define
   `_COMMON_H` suppress `common.h`, so its transitive include is not sufficient;
   the native build can explicitly force-include `ps1_types.h` instead.
3. Retire matching width-only patch hunks and their redundant conversion casts.
   Mixed-purpose patches still need their platform or behavioral changes.
4. Verify the prepared native sources, SDK ABI, optimized regressions, and
   gameplay routes before removing any downstream patch permanently.

The original `INCLUDE_ASM` and platform service adaptations remain native build
responsibilities. There is no automatic patch refresh or native pin update.

## Verification

Run native type checks on the host:

```sh
python3 tools/verify_ps1_types.py
```

The checks compile the real shared headers for Linux, Windows, and macOS on
x86-64 and ARM64, for both US and JP definitions. The two existing FIELD actor
views are checked in separate translation units because their shared symbols
have different C declarations. The checks assert layouts, offsets, global slot
widths, and pointer-table widths, and emit object code that uses the pointers.

On Linux, runtime checks map RAM at `0x81000000` and run at `-O0` and `-O2`.
They cover zero extension, assignment, dereferencing, incrementing, typed
arithmetic, pointer-table stride, linked records, union storage, nulls, and
negative sentinel representation. They also compile the original
`field_script_branch` and `akao_seq_op_set_pitch_jitter_depth` consumers and
execute their reads/writes while checking adjacent fields. Unused functions
from those translation units are discarded at link time. No game functions are
copied into the tests.

Use `--clang PATH` to select a compiler. `--no-runtime` performs only the
SDK-free cross-target checks. Windows/macOS results establish compilation and
layout with the tested upstream Clang, not linked applications, platform
memory-mapping behavior, or a test of Apple's separately shipped compiler.

For matching verification, stage before starting the parallel build:

```sh
docker exec lom-mcp make recopy VERSION=us
docker exec lom-mcp make -j6 verify-bins VERSION=us
docker exec lom-mcp make recopy VERSION=jp
docker exec lom-mcp make -j6 verify-bins VERSION=jp
```

Baseline and final verification for this migration both passed on 2026-09-28:

| Check | Result |
| --- | --- |
| US executable and all 17 overlays | Exact original disc bytes |
| JP executable and 13 overlays | Exact original disc bytes |
| JP FIELD, GNAME, GOSUB, TITLE | Exact decompressed original images |
| Clang 22.1.8, six platform/architecture targets | Real layout assertions and object generation pass |
| Linux runtime at `-O0` and `-O2` | Stored-pointer and original game-consumer checks pass |

Representative real-source assembly diffs also retained their baseline match:

| Function | Baseline | Final |
| --- | --- | --- |
| `field_script_branch` | 100% | 100% |
| `akao_seq_op_set_pitch_jitter_depth` | 100% | 100% |
| `func_80064AF8` (WMAP frame rendering) | 100% | 100% |

JP's four raw-image checks retain the existing compressor limitation. Whole
binary matching covers all PS1 consumers, including data and section layout;
native tests cover the declared contracts and selected consumers, not the full
native game.
