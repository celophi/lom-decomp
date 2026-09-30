# PS1 storage types

[English documentation](../../README.md)

On the PlayStation a pointer is four bytes, like a `long`. Legend of Mana
keeps many of them inside its data: a script frame stores its program
counter, a resource entry points at its bytes, the sound driver keeps a
cursor into each sequence. Compile the same C for a 64-bit PC and every one
of those fields becomes eight bytes. Structures grow, arrays get a different
stride, and the data the game loaded no longer lines up with the code that
reads it.

A native port can avoid that by keeping the PS1 memory layout: the game's
data stays at its PS1 size, and only the pointers the code works with are
full width. To make that possible without rewriting the game, every pointer
the game *stores* is declared through a storage typedef. With the PS1
compiler these typedefs are plain pointers and nothing changes; a native
build redefines them in one place.

`include/ps1_storage.h` holds the hooks and the basic typedefs. `common.h`
includes it.

## The rule

- A pointer, code address or `long` that lives in memory the game keeps in
  PS1 layout uses a storage type: a struct or union member, a global, an
  array element.
- Locals and parameters are not stored. They stay ordinary pointers.

Memory in PS1 layout means the globals the C does not define (their
contents come from the game's own data) and every structure they hold by
value. A global defined in C belongs to the build that compiles it.
`make storage-check` works this set out and checks it (see below).

## The types

| Type | Stores |
| --- | --- |
| `void_ptr`, `u8_ptr`, `s8_ptr`, `u16_ptr`, `s16_ptr`, `u32_ptr`, `s32_ptr`, `u_long_ptr` | a pointer to that basic type |
| `<Type>Ptr` | a pointer to a structure or union, declared next to the type |
| `<Type>TablePtr` | a pointer to a table of `<Type>Ptr` entries |
| `<Name>Ptr` over a function pointer type | a function pointer that C code sets (a callback) |
| `<Name>Slot` | a PS1 code address read from the game data (a dispatch table entry) |
| `u_long`, `Ps1Long` | the SDK's unsigned and signed `long` |

Declare a structure's pointer typedef right after the structure:

```c
typedef struct
{
    u8_ptr pc;
    u32 flags;
} FieldScriptFrame;
typedef PS1_PTR(FieldScriptFrame) FieldScriptFramePtr;
```

The code that uses a stored pointer does not change. It reads, writes,
compares and indexes it like any pointer:

```c
u8* cursor = frame->pc;
frame->pc = cursor + 2;
opcode = *frame->pc++;
```

### Pointers to stored pointers

`u8_ptr*` points at a stored pointer; `u8**` points at an ordinary one. They
are the same on the PS1 and different on a 64-bit build. A local that walks
a table of stored pointers, or takes the address of one, uses the storage
type:

```c
FieldMapObjectPtr* objects = g_field_objects;   /* the table's own entries */
u8_ptr* heap = &g_field_actor_heap;             /* a stored heap cursor */
```

### Callbacks and code slots

A function pointer that C code assigns is an ordinary stored pointer. Give
its function pointer typedef a `Ptr` companion with `PS1_STORED`:

```c
typedef u8* (*CdCommandCallback)(s32 bytes_transferred, u32 bytes_remaining);
typedef PS1_STORED(CdCommandCallback) CdCommandCallbackPtr;
```

A dispatch table in the game data holds PS1 code addresses instead. The
table's type is a slot, and every call goes through `PS1_CALL`, which a
native build uses to map the address to its own function:

```c
typedef s32 (*FieldScriptCalcOp)(s32 left, s32 right);
typedef PS1_CODE(FieldScriptCalcOp) FieldScriptCalcOpSlot;
extern FieldScriptCalcOpSlot g_field_script_calc_ops[FIELD_SCRIPT_CALC_OP_COUNT];

return PS1_CALL(g_field_script_calc_ops[op])(left, right);
```

`PS1_CALL(slot)` is the slot itself on the PS1, so the generated code is the
same as a direct call. The world map's step tables share one slot type,
`WmapStepHandlerSlot`.

## What a native build defines

A build that keeps PS1 layouts defines these hooks before `ps1_storage.h` is
read, usually with a forced include:

| Hook | Default | Meaning |
| --- | --- | --- |
| `PS1_STORED(pointer_type)` | the type itself | a stored copy of a data or function pointer type |
| `PS1_PTR(type)` | `PS1_STORED(type*)` | a stored pointer to `type` |
| `PS1_CODE(type)` | `type` | a slot holding a PS1 code address |
| `PS1_CALL(slot)` | the slot itself | the function a slot holds, ready to call |
| `PS1_LONG` | `long` | the stored `long` |

`tools/storage_check/host_storage.h` is one such set of definitions, for
Clang on x86-64 (`__ptr32 __uptr` keeps a stored pointer four bytes and
zero-extends it on load). The decomp itself contains no compiler-specific
storage code.

## The check

`make storage-check` lays out every type twice with libclang: once for the
PS1 and once for x86-64 with `host_storage.h`. It reports:

- a structure or union in PS1 memory whose size or field offsets differ,
  with the first field that moved;
- a global whose size differs;
- a compile error that only the storage definitions cause, such as a slot
  called without `PS1_CALL` or a stored pointer's address assigned to a
  plain `T**`. These always fail.

Differences that are known and not fixed yet are listed in
`tools/storage_check/baseline_<version>.txt`. The list may only shrink; CI
runs the check for every version.

When the check fails:

- a new pointer member or global: declare it with a storage type;
- a call through a table: wrap it in `PS1_CALL`;
- an address-space error on a local: give the local the storage type
  (`u8_ptr*`, `<Type>Ptr*`).
