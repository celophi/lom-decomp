# AKAO sound driver — review notes

LOM uses Square's AKAO sound driver. The CPU-side dispatch goes through one
central call, [akao_send_command](../src/decomp3.c) (0x80028E84), which reads
arguments out of `g_akaoCmdParams[]` and indexes a 256-entry handler table.
The wrappers in [decomp3.c](../src/decomp3.c) each pack arguments and dispatch
a specific opcode; the lower-level SPU plumbing lives in
[decomp5.c](../src/decomp5.c).

This doc lists the risky cleanups that were deferred for review and the
mechanical opcode-name renames that are still pending.

## 1. Five aliased struct typedefs that all described the same AKAO bank header (DONE)

`akao_upload_bank` (formerly `func_8002376C`) and the streaming-upload state
machine `func_80022B78` consume the same four fields at offsets 0x10–0x1F
of an AKAO instrument-bank file. Plus `func_800230C8` writes a fifth field
at offset 0x20:

| offset | field name           | meaning                                                                     |
|--------|----------------------|-----------------------------------------------------------------------------|
| 0x00   | (AkaoSeqHeader)      | 16-byte common AKAO header (magic + id + length + reverb + timestamp)       |
| 0x10   | `spu_dest_addr`      | SPU upload base address (passed to `SpuSetTransferStartAddr`)               |
| 0x14   | `sample_size`        | SPU upload byte count (passed to `akao_spu_write`)                          |
| 0x18   | `bank_id`            | Instrument-table index (multiplied by 0x10 to index `D_8004C340`)           |
| 0x1C   | `articulation_count` | Articulation entry count (each entry is 16 bytes; relocated by SPU base)    |
| 0x20   | `cached_spu_addr`    | SPU base address cached by `func_800230C8` after a streaming setup         |
| 0x24   | reserved             | unused / unknown (header is 0x40 bytes total)                              |

Consolidated into a single `AkaoBankHeader` in
[include/akao.h](../include/akao.h) that embeds `AkaoSeqHeader` as its first
member. `UnknownStruct`, `SomeStruct`, `D3C0_t`, `ArgStruct2` were removed
from [include/decomp5.h](../include/decomp5.h) and
[include/decomp3.h](../include/decomp3.h). All call sites in
[src/decomp5.c](../src/decomp5.c) and [src/decomp3.c](../src/decomp3.c) were
updated to use the named fields.

The four affected scratches were all already non-matching
(`akao_upload_bank` 99.90%, `func_80022B78` 98.59%, `func_800230C8` 99.80%,
`func_800231E4` 99.90%); a follow-up asm-diff pass should re-measure each.

### Note: `func_800230C8` looks like it has an off-by-one decompilation bug

After consolidation, this stuck out: `func_800230C8` calls

```c
akao_spu_write(arg0, ((AkaoBankHeader*)var1)->spu_dest_addr);  /* offset 0x10 */
```

but `akao_spu_write(addr, size)`'s second argument is a byte count. The
analogous site in `akao_upload_bank` passes `sample_size` (offset 0x14) for
the same role. Passing the SPU base as a byte count appears semantically
wrong — possibly the original C had a typo of `unk10` for `unk14`, which
would explain why this scratch is at 99.80% rather than 100%. A fix from
`spu_dest_addr` to `sample_size` should be tested against the original asm
before being committed; left as-is for now.

## 2. `D_8003EC5C` is a channel-state pointer, not a bank header (DONE)

The previous `D_8003EC5C_t` typedef in [include/decomp3.h](../include/decomp3.h)
modeled `unk0` as if it were the AKAO magic-word slot, but
`func_800230C8` and `func_800231E4` actually read `D_8003EC5C->unk0 & 0x40`
as a **flag-byte test**, not a magic check. `D_8003EC5C` is assigned in
`akao_driver_init_state` to point at the first slot of `D_8004C260`, the
0x118-byte sequence-channel-state buffer.

Replaced with a new `AkaoChannelState` typedef in
[include/akao.h](../include/akao.h):

| offset | field      | meaning                                          |
|--------|------------|--------------------------------------------------|
| 0x00   | `flags`    | channel flags (bit 0x40 = active/playing)        |
| 0x04   | `unk4`     | tested non-zero alongside `unk1C` for in-flight  |
| 0x1C   | `unk1C`    | tested non-zero alongside `unk4` for in-flight   |
| 0x20.. | (padded)   | rest of the 0x118 channel slot, fields TBD       |

Both [include/decomp3.h](../include/decomp3.h) and
[include/decomp5.h](../include/decomp5.h) now declare
`extern AkaoChannelState* D_8003EC5C;` (previously a `D_8003EC5C_t*` and a
`void*` respectively — they were inconsistent across translation units).
The two assignments in [src/decomp5.c](../src/decomp5.c)
(akao_driver_init_state) cast through `u8*` since the surrounding code does
raw byte-pointer arithmetic on the channel slot.

Touches the same two non-100% scratches (`func_800230C8` 99.80%,
`func_800231E4` 99.90%) as §1; needs an asm-diff re-measurement.

## 3. `akao_check_magic` prototype mismatch (DONE)

The definition has always been `s32 akao_check_magic(s32* data)`
([src/decomp5.c](../src/decomp5.c)) but the declaration in
[include/decomp5.h](../include/decomp5.h) was `s32 akao_check_magic(void)`,
and two call sites (`akao_register_bank` in [src/decomp3.c](../src/decomp3.c)
and `akao_submit` in [src/decomp5.c](../src/decomp5.c)) were invoking it
with no argument — relying on the previous instruction having left the
operand in `$a0`.

User reports that **the matching ASM requires the argument to be passed
explicitly** — the no-argument register-allocation hack does not actually
match for those call sites. Both call sites updated:

- `akao_register_bank`: now calls `akao_check_magic((s32*)bank)`.
- `akao_submit`: now calls `akao_check_magic((s32*)sequenceData)`.

The decomp5.h prototype now matches the definition
(`extern s32 akao_check_magic(s32 *data);`) and the stale `@note` describing
the no-arg hack was removed from the function's docblock.

## 4. `g_akaoCmdParams[]` element type (DONE)

The buffer was previously typed `s32[]`, but several wrappers store
**pointers** in slot 0:

- `akao_play_song` (a sequence buffer pointer)
- `akao_register_bank` indirectly (via opcode 0xE0)
- `func_80022FAC` (opcode 0xE0, an AKAO buffer)
- `func_800231E4` (opcode 0xEC, an AKAO buffer)

Retyped to `void *g_akaoCmdParams[]` in
[include/decomp3.h](../include/decomp3.h) with an updated docstring. All 101
assignment sites in [src/decomp3.c](../src/decomp3.c) were wrapped with an
explicit `(void*)(...)` cast on the right-hand side, e.g.

```c
g_akaoCmdParams[0] = (void*)(arg0);
g_akaoCmdParams[1] = (void*)(arg1 & 0xFFFFFF);
```

The casts are codegen-neutral on PSX (a 32-bit `sw` either way) but keep
modern parsers/lints quiet about implicit int→pointer conversions. GCC 2.7.2
would have accepted the unconverted form too.

## 5. Mechanical opcode renames in decomp3.c (DONE)

Applied via a single bulk sweep across [src/decomp3.c](../src/decomp3.c),
[include/decomp3.h](../include/decomp3.h), the per-overlay headers
([cd.h](../include/cd.h), [decomp1.h](../include/decomp1.h),
[decomp7.h](../include/decomp7.h), [gover.h](../include/gover.h),
[title.h](../include/title.h)), and every overlay/source caller
(`cdrom.c`, `main.c`, `decomp7.c`, `decomp1.c`, `title.c`, `movie1.c`,
`gover/code1.c`, `checkps/code.c`).

Names follow `akao_cmd_<hex>` for the wrappers whose semantics are not yet
nailed down, and use a descriptive name only where the LOM call shape itself
makes the meaning obvious (e.g. `akao_play_sfx_from_buffer` for opcode 0x24
because the wrapper magic-checks an AKAO buffer pointer; `akao_stop_sfx_by_id`
for 0x30 because the masking matches `akao_play_sfx`'s id-arg shape).

A new `AkaoCmd` enum was added to [include/akao.h](../include/akao.h) listing
every opcode LOM issues, with one-line `@see` blurbs documenting whatever the
LOM call shape reveals about each opcode (parameter widths, magic-checked or
not, etc.). This is the single source of truth — when better names emerge we
update the enum and reflect them in the wrapper names.

The mapping (50 wrappers) lives authoritatively in
[config/symbols/shared_symbol_addrs.txt](../config/symbols/shared_symbol_addrs.txt)
and [config/symbol_addrs.txt](../config/symbol_addrs.txt); see those files
for the address↔name pairs. Highlights of the non-mechanical names chosen:

- `0x24` → `akao_play_sfx_from_buffer` (wrapper magic-checks the buffer)
- `0x30` → `akao_stop_sfx_by_id` (10-bit id mask mirrors `akao_play_sfx`)
- `0xE0`/`0xEC` → `akao_cmd_e0`/`akao_cmd_ec` (both magic-check buffers, but
  the precise driver action is unclear)
- streaming/upload state machine `func_80022B78` → `akao_streaming_upload_tick`
- `func_80022DAC` → `akao_upload_bank_slot` (routes between 6 SPU
  slot+bank-id pairs)
- `func_800230C8` → `akao_upload_xa_program` (sets up an XA buffer at SPU
  0x50900 / 0x43100, magic-checks the AKAO buffer, caches the SPU base on
  the buffer at offset 0x20)

## 6. Param-bit-width hints (for naming the opcode wrappers)

Field widths surfaced by the masks the wrappers apply tell us how the AKAO
driver decodes each opcode payload. Useful when fleshing out the descriptions
on the renames in §5:

- 7-bit slot ⇒ "volume / pan" (0–127)
- 8-bit slot ⇒ "byte parameter" (often duration in ticks, or a count)
- 10-bit slot ⇒ "sound / sequence id"
- 24-bit slot ⇒ "frequency / pitch / wide param"
- left-shift-by-8 (`<<8`) ⇒ "big-endian 16-bit parameter packed into the high
  byte of a word" (seen in 0xE4/0xE5/0xE6/0xED)

## 7. References

- [AKAO_sequence — ff7-flat-wiki](https://ff7-mods.github.io/ff7-flat-wiki/FF7/PSX/Sound/AKAO_sequence.html)
- [PSX Sound Code Map — ff7-flat-wiki](https://ff7-mods.github.io/ff7-flat-wiki/FF7/PSX/Sound/Code_Map.html)
  (confirms the 256-entry `MESSAGE_HANDLERS` dispatch table at 0x80049548 in FF7,
  i.e. exactly the `akao_send_command(opcode)` dispatch we have here)
