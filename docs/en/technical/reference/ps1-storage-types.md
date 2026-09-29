# PS1 storage types

On the PlayStation, a pointer is four bytes. Legend of Mana stores a lot of
them inside its data: a script frame keeps its program counter in a field,
resource headers point at tables, tables point at more tables. If you compile
that same C for a 64-bit PC, every one of those fields quietly becomes eight
bytes. Offsets move, arrays get a different stride, and the data the game loads
from the disc no longer lines up with the structs that read it.

There is no native PC build of Legend of Mana in this project, and the
decompilation itself doesn't need one. But if someone wants to try getting
this code running on a 64-bit system, that pointer-size problem is one of the
first walls they'll hit. The usual fix is to rewrite every stored pointer as a
`u32` and add casts wherever it's used, which is a lot of churn and makes the
code harder to read.

`include/ps1_types.h` is there so nobody has to do that. It gives the stored
pointers their own types that stay four bytes on a 64-bit build, while the PS1
build compiles exactly as it always has. The matching build is still 100%
byte-for-byte; these types expand to plain pointers for the historical
compiler, so nothing about the original code generation changes.

`common.h` includes the header, and it can also be included on its own without
the assembly macros.

## The types

| Type | Points at |
| --- | --- |
| `void_ptr` | `void` |
| `u8_ptr`, `s8_ptr` | Unsigned or signed byte |
| `u16_ptr`, `s16_ptr` | Unsigned or signed halfword |
| `u32_ptr`, `s32_ptr` | Unsigned or signed word |
| `u_long_ptr` | Psy-Q `u_long` |
| `<Type>Ptr` | A structure, declared next to the type it points at |
| `<Type>TablePtr` | A stored address of a table of `<Type>Ptr` slots |

With the PS1 compiler, each of these is just the original pointer type.

## Turning them on for a 64-bit build

Compile with Clang and pass:

```text
-DPS1_32BIT_STORAGE -fms-extensions
```

`PS1_32BIT_STORAGE` switches the typedefs over to Clang's `__ptr32 __uptr`
pointers. `__ptr32` keeps the pointer four bytes wide in storage, and `__uptr`
zero-extends it when it's loaded into a normal pointer. That second part
matters: PS1 addresses start at `0x80000000`, and that has to come out as
`0x0000000080000000`, not a sign-extended address somewhere near the top of
memory.

The header refuses to compile with the flag on unless the compiler actually
supports these extensions, and it asserts that the stored types really are
four bytes. If something is set up wrong, you find out at compile time instead
of at runtime.

`long` gets the same treatment. On a 64-bit Linux or macOS build `long` is
eight bytes, so with the flag on `u_long` becomes a four-byte unsigned int, and
`Ps1Long` becomes a four-byte signed int (it stays a plain `long` for the PS1).
The SDK's `MATRIX` and `VECTOR` fields use `Ps1Long`. The SDK function
declarations still use their original `long` parameters, so whatever provides
those functions on a native build needs to agree with that separately.

## Using them

A stored pointer goes in the struct; the code that works with it keeps using a
normal pointer:

```c
typedef struct FieldScriptFrame
{
    u8_ptr pc;
    u32 flags;
    union
    {
        u32 word;
        struct
        {
            u32 resume : 1;
            u32 frames : 31;
        } bits;
    } wait;
} FieldScriptFrame;

u8* cursor = frame->pc;
frame->pc = cursor + 2;
opcode = *frame->pc++;
```

`cursor` is an ordinary pointer at whatever width the host uses. Clang handles
the conversion at each field access, so there's no `TO_PTR`/`FROM_PTR` macro
dance and no casts.

For a pointer to a structure, declare its alias right after the structure's
forward declaration or definition:

```c
typedef struct FieldImageReq FieldImageReq;
typedef FieldImageReq* PS1_PTR32 FieldImageReqPtr;
```

`PS1_PTR32` is a qualifier for typedefs like this one, not something to wrap
around every declaration. It expands to nothing for the PS1 compiler. If the
structure is local to a function, its alias has to be declared in that same
scope; a typedef with the same name in another function is a different type.

### Tables of pointers

Tables need care at both levels, because the table's own address is stored and
so is every entry:

```c
typedef FieldPartDef* PS1_PTR32 FieldPartDefPtr;
typedef FieldPartDefPtr* PS1_PTR32 FieldPartDefTablePtr;

FieldPartDefTablePtr stored_table;
FieldPartDefPtr* table_cursor = stored_table;
```

The stored table address and each entry are four bytes. `table_cursor` is a
normal host pointer, and it steps four bytes per entry because that's the size
of a `FieldPartDefPtr`.

The same idea applies one level down: `u8_ptr*` points at a stored byte-pointer
slot, and `u8**` points at an ordinary working pointer. They're the same thing
on the PS1 and very different things on a 64-bit build, so don't mix them up.
For example, the FIELD scene and text drawing functions take `u8_ptr*`, because
the caller passes the address of `FieldRenderHalf.primitive_cursor`, which is a
stored field. Their local packet pointers are still plain `u8*`, and the
allocation helpers that take the address of a local cursor still use `u8**`.

## What this covers, and what it doesn't

So far the stored pointers in AKAO, FIELD (scripts, text, actors and
resources), WMAP (resources and rendering) and WSEL use these types. WMAP
repeats a lot of `extern` declarations and private types inside functions, so
you'll see far more declarations than distinct types there.

This isn't every pointer in the game. Locals, function parameters, callbacks,
the tagged GPU ordering-table links, and integer fields that sometimes hold an
address all keep their current types and would each need their own look on a
native build.

A few things the types can't do for you:

- A stored pointer can only hold an address below 4 GiB. Assigning an ordinary
  host allocation above that silently truncates it, so the memory the game
  works in has to be mapped low.
- They don't map the PS1's RAM, turn retail code addresses into native
  callbacks, decode tagged links, or fix integer-to-pointer arithmetic
  elsewhere. An integer field used as an address may still need an explicit
  conversion.

## If you want to try a native build

Roughly, the pieces are:

1. Build the game sources with Clang, `-DPS1_32BIT_STORAGE` and
   `-fms-extensions`.
2. Make sure `ps1_types.h` is included everywhere. If your build replaces
   `common.h` with its own shim (for example by defining `_COMMON_H`), you lose
   the include that comes with it, so force-include `ps1_types.h` instead.
3. Provide everything the PS1 build still takes from assembly. That's the
   Psy-Q SDK libraries now - all of the game's own code is C - plus the
   platform services the SDK stands for: graphics, CD-ROM, sound and
   controllers.
4. Keep the matching build green while you go. Any change to shared headers
   should still pass `make verify-bins` for both versions.

## Checking the types

To check the native side on your machine:

```sh
python3 tools/verify_ps1_types.py
```

This compiles the real shared headers for Linux, Windows and macOS on x86-64
and ARM64, for both the US and JP definitions. It asserts struct layouts,
offsets, the widths of stored globals and pointer tables, and emits object code
that actually uses the pointers. The two FIELD actor views are checked in
separate translation units, since their shared symbols have different C
declarations.

On Linux it also runs code. It maps memory at `0x81000000` and checks zero
extension, assignment, dereferencing, incrementing, pointer arithmetic, table
stride, linked records, unions, nulls and negative sentinels, at both `-O0` and
`-O2`. It then compiles two real game functions, `field_script_branch` and
`akao_seq_op_set_pitch_jitter_depth`, and runs their reads and writes while
checking the fields around them.

Use `--clang PATH` to pick a compiler, and `--no-runtime` for only the
cross-target compile checks. The Windows and macOS results tell you that the
code compiles with the right layout on upstream Clang. They aren't linked
programs, and they don't test Apple's own Clang.

The PS1 side is covered by the normal matching check. Run it for both versions
after touching these types:

```sh
docker exec lom-mcp make recopy VERSION=us
docker exec lom-mcp make -j6 verify-bins VERSION=us
docker exec lom-mcp make recopy VERSION=jp
docker exec lom-mcp make -j6 verify-bins VERSION=jp
```

## What we don't know yet

- TODO: which of the remaining pointer-like fields (integers holding addresses,
  GPU ordering-table links) should get their own storage types, and which are
  better left to a native build's own code.
- TODO: whether the SDK's `long` parameters line up with what a native
  replacement for those functions would expect; the types above only fix the
  data layout.
