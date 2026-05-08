# Matching Decompiled C to MIPS Assembly
## GCC 2.7.2-970404 + ASPSX 2.67 (CCPSX), `-G0 -O2 -fsigned-char`

> This guide documents techniques for getting matching decompilation on PS1 MIPS assembly
> produced by the Psy-Q 4.1 toolchain. It focuses specifically on two closely related
> patterns involving shared `lui` instructions, register allocation priority, and the
> elimination of intermediate variables.

---

## 1. Background: How GCC 2.7.2 Allocates Registers

### 1.1 Caller-saved vs callee-saved

On MIPS, `t0–t9` are caller-saved (destroyed by `jal`). `s0–s7` are callee-saved (must
survive across `jal`). Any C variable whose value must be live across a function call
**must** be placed in an `s` register. GCC saves/restores these in the function prologue
and epilogue.

### 1.2 Global register allocation priority

GCC 2.7.2 uses a priority-based Chaitin-style graph coloring allocator for global
allocation (variables live across basic blocks or calls). The priority formula is roughly:

```
priority = reference_count / spill_cost
```

The variable with the **highest reference count** that must survive a `jal` gets the
**first** register in `REG_ALLOC_ORDER`. For MIPS this order is:

```
s0, s1, s2, s3, s4, s5, s6, s7, ...
```

So: **most-referenced live-across-call variable → s0, next → s1, next → s2.**

This means the register assignment in the output is a direct reflection of how many times
each variable is referenced in the C source. Changing reference counts changes register
assignments.

### 1.3 The shared `lui` trick

MIPS cannot load a 32-bit constant in one instruction. A full address requires two:

```mips
lui  rX, %hi(SYMBOL)       ; upper 16 bits
addiu rX, rX, %lo(SYMBOL)  ; lower 16 bits
```

When two variables share the same `%hi` bits (e.g., `&D_80052428` and `&D_80052428+4`
share the same upper 16 bits), GCC 2.7.2 with `-O2` will try to **emit only one `lui`**
and reuse those bits for both `addiu` computations. This is a key optimization to
recognise in the disassembly.

The pattern looks like this in the output:

```mips
4c: lui   s1, %hi(D_80052428)       ; s1 holds upper bits only — not complete yet
50: addiu s0, s1, %lo(D_80052428)   ; s0 = &D_80052428  (borrows s1's lui)
54: addiu s0, s0, 4                  ; s0 = &D_80052428 + 4  (offs, final)
...
64: addiu s1, s1, %lo(D_80052428)   ; s1 = &D_80052428  (base, now finalised)
                                     ; reuses the SAME lui from 0x4c
```

One `lui` does the work of two address computations. The variable finalised **later** holds
the `lui` because its register slot is free longest.

---

## 2. The Two Situations

### Situation A: Two pointers into the same symbol, one offset by +4

This is the most common pattern. You have a data block at some symbol address, and the
function needs both a pointer to the start (`base`) and a pointer to the first field
(`offs = base + 4`). The function also reads offsets out of the table to compute `src`,
`count`, and a final cursor position, all across `jal` boundaries.

**What the target assembly looks like:**

```mips
4c: lui   s1, %hi(D_80052428)        ; scaffold lui
50: addiu s0, s1, %lo(D_80052428)    ; s0 = base (intermediate)
54: addiu s0, s0, 4                   ; s0 = offs (final)
58: lui   a1, 0x8013                  ; (unrelated setup)
5c: ori   a1, a1, 0xc000
60: lui   s2, %hi(D_80061088)        ; ref setup
64: addiu s1, s1, %lo(D_80052428)    ; s1 = base (final, reusing lui)
68: sw    v0, %lo(D_80061088)(s2)    ; *ref = 0x8013C000
6c: lw    v0, 0(s0)                  ; offs[0]
70: lw    a2, 4(s0)                  ; offs[1]
74: addu  a0, v0, s1                  ; src = base + offs[0]
78: jal   func_80016E7C
7c: subu  a2, a2, v0                  ; [delay slot] count = offs[1] - offs[0]
...
98: addu  a0, a0, s1                  ; [delay slot] base + offs[1]
```

**Register assignment in the target:**
| Register | Variable | Reference count |
|----------|----------|-----------------|
| s0 | `offs` (&symbol + 4) | highest — used for all loads and final addu |
| s1 | `base` / `%hi` scaffold | medium — used for two `addu` results |
| s2 | `ref` (&D_80061088) | lowest — used for `sw` and `lw` only |

**The matching C:**

```c
extern u32 D_80052428;
extern s32 D_80061088;

void func_80050080(void)
{
    u8*  src;
    u8*  dst;
    u32  count;
    s32* ref;
    u32* offs;

    if (/* state guard */ ...)
        return;

    ref  = &D_80061088;
    *ref = 0x8013C000;
    offs = &D_80052428;     /* offs starts at base address */
    offs++;                  /* offs = &D_80052428 + 4 */

    src   = (u8*)&D_80052428 + offs[0];   /* &D_80052428 referenced AGAIN */
    dst   = (u8*)0x8013C000;
    count = offs[1] - offs[0];

    func_80016E7C(src, dst, count);
    func_80021FFC(*ref);
    func_80022AE8((u32)&D_80052428 + offs[1], 1); /* &D_80052428 referenced AGAIN */
}
```

**Why this works — critical details:**

1. **`base` is not a named variable.** The symbol `&D_80052428` is referenced directly
   in three places: the `offs` initialisation, the `src` computation, and the final
   `func_80022AE8` call. Because there is no named `base` variable, the compiler has no
   pseudoreg to allocate for it. Instead it recognises that all three references share the
   same `%hi` bits and emits one `lui` into `s1`, reusing it across all three sites.

2. **`offs++` rather than `offs = (u32*)((u8*)&D_80052428 + 4)`.** The increment after
   initialisation lets the compiler emit `addiu s0,s1,%lo(...)` then `addiu s0,s0,4` —
   two separate runtime instructions. If you write `offs = &D_80052428 + 1` (pointer
   arithmetic on u32*) or use a struct field, GCC may fold the offset into the `%lo`
   relocation as `%lo(D_80052428+0x4)`, which is a different linker relocation and will
   not match.

3. **`ref` is assigned before `offs++` but after `offs` initialisation.** This gives
   `offs` slightly more references overall, keeping it at s0, and pushes `ref` to s2.

---

### Situation B: Two hardcoded address constants, one offset by +4

This is the same structural pattern but with **literal addresses** instead of a linker
symbol. The function receives a fixed base address (e.g., `0x80180000`) and needs both
`*0x80180004` and `*0x80180008` — the same "base + offset table" idiom, but resolved at
compile time rather than link time.

**What the target assembly looks like:**

```mips
24: lui   s0, 0x8018                 ; delay slot of previous jal
28: ori   s0, s0, 0x4                ; s0 = 0x80180004 (offs)
2c: lui   a1, %hi(D_8005D088)
30: addiu a1, a1, %lo(D_8005D088)   ; dst arg
34: lui   s1, 0x8018                 ; s1 = 0x80180000 (ref/base)
38: lw    v0, 0(s0)                  ; offs[0]
3c: lw    a2, 4(s0)                  ; offs[1]
40: addu  a0, v0, s1                 ; src = ref + offs[0]
44: jal   func_80016E7C
48: subu  a2, a2, v0                 ; [delay slot] count
4c: lw    a0, 4(s0)                  ; offs[1] again
50: li    a1, 1
54: jal   func_80022AE8
58: addu  a0, a0, s1                 ; [delay slot] ref + offs[1]
```

**Register assignment:**
| Register | Value | Reference count |
|----------|-------|-----------------|
| s0 | `0x80180004` (offs) | highest — all loads go through here |
| s1 | `0x80180000` (ref/base) | medium — used for two `addu` results |

**The matching C:**

```c
extern u8 D_8005D088;

void func_80050138(s32 arg0)
{
    u32* offs;
    u8*  ref;

    FUN_800141ec((arg0 + 0x17) & 0xFFFF, 0x80180000);
    cdrom_wait_queue_empty();

    offs = (u32*)0x80180004;       /* anchor: the offset table */
    ref  = (u8*) 0x80180000;       /* base address */

    func_80016E7C(ref + offs[0], &D_8005D088, offs[1] - offs[0]);
    func_80022AE8(offs[1] + (u32)ref, 1);
}
```

**Why this works:**

1. **`offs` points to `0x80180004` directly, not `0x80180000 + 1`.** The anchor is the
   offset table, not the base. `offs[0]` = `*(0x80180004)`, `offs[1]` = `*(0x80180008)`.
   Writing `(u32*)0x80180000 + 1` would also land at `0x80180004` but the constant
   `0x80180000` would be allocated as the anchor, swapping s0 and s1.

2. **No intermediate variables.** `temp_v0 = *base` as a named variable adds an extra
   pseudoreg and disturbs priority counting. Use `offs[0]` inline everywhere.

3. **`offs` before `ref`.** Because `offs` is assigned first and referenced more times
   (four uses vs two for `ref`), it wins s0. `ref` gets s1.

---

## 3. Diagnostic Process

When you have a near-match with wrong registers or wrong addressing modes, work through
these steps in order.

### Step 1: Identify the register map

For each `s` register in the target, trace what address or value it holds throughout the
function. Build a table:

```
target s0 = ?
target s1 = ?
target s2 = ?
```

Compare to what your current C produces. If s0 and s1 are swapped, a reference count
imbalance is the cause.

### Step 2: Check the load addressing mode

| Target instruction | Meaning |
|--------------------|---------|
| `lw v0, 0(s0)` | s0 is the anchor pointer; load is at `s0 + 0` |
| `lw v0, 4(s1)` | s1 is the anchor pointer; load is at `s1 + 4` |

`lw v0, 0(s0)` means the compiler chose `offs` (the +4 pointer) as s0 — the anchor.
`lw v0, 4(s1)` means the compiler chose `base` as s0 and derived offs from it.

If you see `4(s1)` and the target has `0(s0)`, the anchor and derived pointer are swapped
in your C.

### Step 3: Check for `move` instructions

A `move sX, sY` in your output means two C variables were initialised with `a = b` (a
direct register copy). The compiler saw them as aliases and emitted a copy instead of an
independent `addiu`. This always means one of the variables should be **eliminated** or
**initialised independently**.

### Step 4: Check `%lo` relocations

If you see `%lo(SYMBOL+0x4)` in your output instead of `%lo(SYMBOL)`, the +4 was folded
into the linker relocation. This happens with:
- Struct field access: `&mystruct.field`
- Pointer arithmetic on typed pointers before assignment: `(u32*)&sym + 1`

The fix is to use `offs = &sym; offs++;` so the +4 is a **runtime** `addiu`, not a
link-time relocation offset.

### Step 5: Check variable elimination

If a named variable is only ever used as an alias for a symbol address (e.g.,
`base = &D_80052428` and then only `base` is used, never modified), consider whether
referencing the symbol directly produces the right codegen. Named variables always consume
a pseudoreg. Direct symbol references let the compiler share a `lui` scaffold register
without allocating a persistent pseudoreg for it.

---

## 4. Common Mistakes and Their Symptoms

| Mistake in C | Symptom in ASM |
|---|---|
| `base = &sym; offs = base; offs++` | `move sX, sY` before the increment |
| `offs = (u32*)((u8*)&sym + 4)` | `%lo(sym+0x4)` relocation, no shared `lui` |
| `ref = &other_sym` before `offs`/`base` | `ref` gets s0 instead of s2 |
| Named `base` variable alongside `offs` | Extra pseudoreg, register swap |
| `temp = *ptr` as named variable | Extra pseudoreg, disturbs priority order |
| `(u32*)0x80180000 + 1` as offs anchor | Wrong anchor, `ori sX,sX,0x8` instead of `0x4` |

---

## 5. Quick Reference Rules

1. **Count references** to determine register order. Most-referenced live-across-call
   variable → s0.

2. **Eliminate named variables** that are pure aliases for a symbol address. Reference
   the symbol directly where needed — the compiler will share the `lui`.

3. **Use `ptr = &sym; ptr++`** rather than `ptr = (type*)((u8*)&sym + N)` to keep the
   +N as a runtime `addiu` rather than a linker relocation.

4. **Assign higher-priority variables first** in the C source only when reference counts
   alone do not determine the ordering. Source order is a secondary tiebreaker.

5. **Remove all intermediate temporaries** (`temp_v0`, `base`, etc.) that exist only to
   hold a loaded value used once. Inline the load expression directly into the call
   argument.

6. **Do not introduce `do { } while(0)` or self-assignments** (`x = x`) unless they were
   present in an earlier version that matched. These can shift scheduling in
   unpredictable ways.

7. **Watch the delay slot.** In MIPS the instruction after a `jal` executes before the
   call. GCC fills delay slots with the last argument computation. If a `subu` or `addu`
   appears after a `jal` in the target, it is the last argument to that call, not
   something that runs after it returns.

---

## 6. Constant Folding into `%lo` Relocations

### 6.1 What is it?

"Constant folding into a relocation" means GCC has taken a compile-time constant offset
and baked it into the symbol's `%lo()` relocation instead of emitting a separate `addiu`
at runtime. You see it in the assembler output as:

```mips
addiu v0, v0, %lo(D_80142E0C+0x8)   ; constant 8 folded in — WRONG
addiu v0, v1, %lo(D_80142EF8-0x4)   ; constant -4 folded in — WRONG
```

The target instead wants two separate instructions — one to materialise the symbol
address and one to apply the offset at runtime:

```mips
addiu v0, v0, %lo(D_80142E0C)       ; materialise symbol address
addiu v0, t0, 2                      ; add 2 to index (separately)
sll   v0, v0, 2                      ; scale by element size
addu  v0, v0, base                   ; add into base
```

or:

```mips
addiu v1, a2, %lo(D_80142EF8)       ; materialise symbol address into v1
addiu v1, v1, -4                     ; subtract 4 at runtime
```

This matters because `%lo(D_80142EF8-0x4)` is a different linker relocation encoding
than `%lo(D_80142EF8)`. Splat and the overlay relocation system treat them differently,
and a folded relocation will not assemble identically to the original binary.

---

### 6.2 Why does GCC fold?

GCC's internal representation (RTL) describes a pointer to `&sym + N` as:

```
(const (plus (symbol_ref "sym") (const_int N)))
```

This is a single CONST node — a fully resolved compile-time constant. The MIPS backend
sees it when printing the `%lo()` operand and emits it as `%lo(sym+N)` in one shot.
GCC never had a chance to emit a runtime `addiu` because from its point of view,
the address was always a single constant.

This happens whenever you write **pointer arithmetic on an extern symbol** where the
offset is a compile-time constant:

```c
extern u32 D_80142E0C[];
u32 val = D_80142E0C[f8ac + 2];    // GCC: &D_80142E0C + (f8ac+2)*4
                                    //       = &D_80142E0C + f8ac*4 + 8
                                    // Emits: %lo(D_80142E0C+0x8) + runtime f8ac*4
```

```c
extern s32 D_80142EF8;
u8 *base = (u8 *)&D_80142EF8 - 4; // GCC: CONST(D_80142EF8 + -4)
                                    // Emits: %lo(D_80142EF8-0x4) immediately
```

The key insight: **GCC distributes a mixed constant+variable expression into
`constant_part + variable_part`, folds `constant_part` into the relocation, and emits
only the `variable_part` as a runtime operation.** The target compiler (ASPSX/CCPSX) did
not do this — it computed the full address as a sequence of runtime additions.

---

### 6.3 Fix 1: Array indexing — `%lo(SYM+0x8)` fold

**Problem:** `D_80142E0C[f8ac + 2]` causes GCC to fold the `+2 * sizeof(u32) = +8`
byte offset into the relocation.

**Target wants:**
```mips
1c:    lui     a2, %hi(D_80142EF8)
20:    lui     v1, %hi(D_80142E0C)
24:    addiu   v1, v1, %lo(D_80142E0C)   ; base address of array, no offset folded
28:    addiu   v0, t0, 2                  ; add 2 to index at runtime
2c:    sll     v0, v0, 2                  ; scale by 4
30:    addu    v0, v0, v1                 ; add into array base
34:    lw      v0, 0(v0)                  ; load
```

**Fix:** Cast the array's address to `u32` before doing arithmetic. This turns it from
pointer arithmetic (which GCC understands as CONST+variable) into plain integer
arithmetic (which GCC cannot fold into a relocation):

```c
// BAD — GCC distributes the +2 into %lo(D_80142E0C+0x8):
u32 temp = D_80142E0C[f8ac + 2];

// GOOD — GCC keeps (f8ac+2)*4 as a fully runtime computation:
u32 temp = *(u32 *)((u32)D_80142E0C + (u32)(f8ac + 2) * 4);
```

The `(u32)D_80142E0C` cast tells GCC "this is just a number, not a pointer with
semantic structure". It can no longer separate the constant `+2` from the variable
`f8ac` because they are now both inside a plain integer multiplication.

This pattern is already used in the project. See `gname.c`:
```c
temp_v1_4 = (void*)(((reg_s1 + 2) * 4) + ((u32)(&D_80142E0C)));
```

---

### 6.4 Fix 2: Pointer minus constant — `%lo(SYM-0x4)` fold

**Problem:** `(u8 *)&D_80142EF8 - 4` causes GCC to fold the `-4` into the symbol
relocation immediately, because the entire expression is a compile-time CONST.

**Target wants:**
```mips
1c:    lui     a2, %hi(D_80142EF8)
...
34:    addiu   v1, a2, %lo(D_80142EF8)   ; materialise full address into v1
38:    addiu   v1, v1, -4                ; subtract 4 at runtime (separate addiu!)
```

**Why casting to `u32` doesn't help:**
```c
u8 *base = (u8 *)((u32)&D_80142EF8 - 4);  // Still folds! u32 is same width as pointer.
```
On 32-bit MIPS, `u32` and a pointer are both 32 bits. GCC's RTL sees the cast as a
no-op and the expression is still CONST(SYMBOL_REF + -4) internally.

**Fix:** Force the address into a live register by loading through it first. GCC cannot
fold a value that is already in a pseudoregister:

```c
// GOOD — the lw keeps ef8_ptr live as a register; the -4 becomes a runtime addiu:
s32 *ef8_ptr = &D_80142EF8;
s32  ef8_val = *ef8_ptr;           // lw t0, %lo(D_80142EF8)(a2)  — keeps ef8_ptr in a reg
u8  *base    = (u8 *)ef8_ptr - 4;  // addiu v1, v1, -4            — runtime subtraction
```

The load `*ef8_ptr` is what forces GCC to materialise the full address into a register.
Once the address is in a pseudoregister (RTL `(reg)`), the subsequent `- 4` becomes
`(plus (reg) (const_int -4))` rather than `(const (plus (symbol_ref) (const_int -4)))`,
and the MIPS backend emits it as a separate `addiu`.

This is exactly the sequence the target disassembly reveals:
- `lui a2, %hi(D_80142EF8)` — hoisted early (a2 holds the %hi bits)
- `addiu v1, a2, %lo(D_80142EF8)` — materialise full address into v1
- `lw t0, %lo(D_80142EF8)(a2)` — load the value (this is `ef8_val = *ef8_ptr`)
- `addiu v1, v1, -4` — runtime subtract (this is the `base = ef8_ptr - 4`)

---

### 6.5 Fix 3: Mixed pointer arithmetic — wrong byte scaling

**Problem:** When you add an integer to a non-`char` pointer in C, the compiler
silently **scales the integer by `sizeof(*ptr)`**. If `D_80142EF8` is `extern s32`,
then `&D_80142EF8` has type `s32 *`, and:

```c
val16 = *((u16 *)((&D_80142EF8 + ef8_val) + (f848 << 1) + 0x10));
//                 ^s32*        ^s32       ^s32            ^int
// Each addition is scaled by sizeof(s32) = 4 !
// Actual byte offset = ef8_val*4 + (f848<<1)*4 + 0x10*4
```

The target assembly for a typical case (from `func_80141C34`):

```mips
a0:    lui   v0, %hi(D_80142EF8)
a4:    addiu v1, v0, %lo(D_80142EF8)  ; v1 = address of D_80142EF8
a8:    lw    t0, %lo(D_80142EF8)(v0)  ; t0 = *D_80142EF8 (the value)
ac:    sll   v0, a2, 0x1              ; v0 = f848 << 1  (byte distance, not element distance)
b0:    addu  v0, v0, t0               ; v0 = (f848<<1) + value
b4:    addu  v0, v1, v0               ; v0 = addr + (f848<<1) + value
b8:    lhu   a2, 0x10(v0)             ; val16 = *(v0 + 0x10)
```

**Fix:** Cast to `u8 *` to force byte-level arithmetic, and use the live-pointer
pattern from Fix 2 to keep the address in a register:

```c
// BAD — s32* arithmetic, each addend is multiplied by 4:
val16 = *((u16 *)((&D_80142EF8 + ef8_val) + (f848 << 1) + 0x10));

// GOOD — u8* arithmetic, byte-for-byte:
s32 *ef8_ptr = &D_80142EF8;
s32  ef8_val = *ef8_ptr;                        // lw t0 — keeps ef8_ptr in a register
val16 = *(u16 *)((u8 *)ef8_ptr + (f848 << 1) + ef8_val + 0x10);
```

The `0x10` at the end is the last constant in the addition chain. GCC will fold it
into the load instruction's offset field as `lhu a2, 0x10(v0)`. This is **good
folding** — it is how the target is encoded and exactly what you want. See the
distinction in §6.7 below.

---

### 6.6 Good folding vs bad folding

Not all constant folding is wrong. There are two completely different places a constant
can be folded, with opposite consequences:

| Kind | Where it appears | Example | Correct? |
|------|-----------------|---------|----------|
| **Bad** | Inside `%lo()` relocation | `addiu v0, v0, %lo(sym-0x4)` | No — different linker encoding |
| **Good** | Inside load/store displacement | `lhu a2, 0x10(v0)` | Yes — expected and desired |

**Bad folding** happens at *address computation* time: the constant is absorbed into the
symbol reference, changing the relocation type in the object file. This will never
match the original binary encoding.

**Good folding** happens at *load/store* time: after the address is fully computed in a
register, a small trailing constant becomes the load instruction's immediate offset. MIPS
load/store instructions have a 16-bit signed displacement field; GCC always uses it to
absorb the last constant in an address computation. The target binary uses this too.

**The rule of thumb:** a constant that appears *before* the final `(u16 *)` cast must end
up in the load displacement, not in a `%lo()`. Write your additions so the small fixed
offset is the last thing added before casting:

```c
// constant 0x10 is last — goes into lhu displacement (good):
val16 = *(u16 *)((u8 *)ef8_ptr + (f848 << 1) + ef8_val + 0x10);

// constant 0x10 folded early — may end up in %lo relocation (bad):
u8 *addr = (u8 *)&D_80142EF8 + 0x10;   // &D_80142EF8+0x10 is a CONST — folded!
val16 = *(u16 *)(addr + (f848 << 1) + ef8_val);
```

---

### 6.7 Diagnostic: how to tell you have a folding problem

Look for `%lo(SYMBOL±N)` in your compiled output (N ≠ 0). One quick way:

```
grep '%lo.*[+-][0-9]' build/your_file.s
```

Any hit means a constant was folded. Then compare against the target `.s` to see whether
the target also has the fold or has two separate instructions.

---

### 6.8 Summary table

| C expression | What GCC emits | What target wants | Fix |
|---|---|---|---|
| `sym[var + 2]` | `%lo(sym+0x8)` + runtime `var*4` | `%lo(sym)` + runtime `(var+2)*4` | `*(T*)((u32)sym + (u32)(var+2) * sizeof(T))` |
| `(u8*)&sym - 4` | `%lo(sym-0x4)` | `%lo(sym)` then `addiu reg,-4` | Load through a pointer first; subtract from the live reg |
| `&sym + 1` (typed ptr) | `%lo(sym+0x4)` | `%lo(sym)` then `addiu reg,+4` | Use `ptr = &sym; ptr++;` (runtime increment) |