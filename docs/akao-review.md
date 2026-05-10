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

## 5. Pending mechanical opcode renames in decomp3.c

Existing convention in [config/symbols/shared_symbol_addrs.txt](../config/symbols/shared_symbol_addrs.txt)
is `akao_cmd_<hex>[_optional_description]` (see `akao_cmd_c8`,
`akao_cmd_e4_set_cd_volume`, `akao_cmd_e8_start_xa_stream`). The following
file-local wrappers in [src/decomp3.c](../src/decomp3.c) are still
`func_*` / `FUN_*` and could be renamed by opcode number. Each is a plain
"pack args into `g_akaoCmdParams`, dispatch one opcode" wrapper.

| current name      | address     | opcode  | proposed                |
|-------------------|-------------|---------|-------------------------|
| `func_800220B0`   | 0x800220B0  | 0x14    | `akao_cmd_14`           |
| `func_800220E4`   | 0x800220E4  | 0x19+0xC0 | `akao_cmd_19_c0`      |
| `func_8002213C`   | 0x8002213C  | 0x12    | `akao_cmd_12`           |
| `func_800221BC`   | 0x800221BC  | 0x24    | `akao_cmd_24`           |
| `func_80022240`   | 0x80022240  | 0x21    | `akao_cmd_21`           |
| `func_8002227C`   | 0x8002227C  | 0x30    | `akao_cmd_30_stop_sfx`  |
| `func_800223B0`   | 0x800223B0  | 0x90    | `akao_cmd_90`           |
| `func_800223D8`   | 0x800223D8  | 0x92    | `akao_cmd_92`           |
| `FUN_80022400`    | 0x80022400  | 0x99/9B/9D/9F | `akao_cmd_99_9b_9d_9f` |
| `func_8002246C`   | 0x8002246C  | 0x98/9A/9C/9E | `akao_cmd_98_9a_9c_9e` |
| `func_800224D8`   | 0x800224D8  | 0xA8    | `akao_cmd_a8`           |
| `func_80022504`   | 0x80022504  | 0xA9    | `akao_cmd_a9`           |
| `func_80022538`   | 0x80022538  | 0xA0    | `akao_cmd_a0`           |
| `func_8002257C`   | 0x8002257C  | 0xA1    | `akao_cmd_a1`           |
| `func_800225C4`   | 0x800225C4  | 0xAA    | `akao_cmd_aa`           |
| `func_800225F0`   | 0x800225F0  | 0xAB    | `akao_cmd_ab`           |
| `func_80022624`   | 0x80022624  | 0xA2    | `akao_cmd_a2`           |
| `func_80022668`   | 0x80022668  | 0xA3    | `akao_cmd_a3`           |
| `func_800226B0`   | 0x800226B0  | 0xAC    | `akao_cmd_ac`           |
| `func_800226DC`   | 0x800226DC  | 0xAD    | `akao_cmd_ad`           |
| `func_80022710`   | 0x80022710  | 0xA4    | `akao_cmd_a4`           |
| `func_80022754`   | 0x80022754  | 0xA5    | `akao_cmd_a5`           |
| `FUN_8002279c`    | 0x8002279C  | 0xC0    | `akao_cmd_c0`           |
| `func_800227D0`   | 0x800227D0  | 0xC1    | `akao_cmd_c1`           |
| `func_80022808`   | 0x80022808  | 0xC2    | `akao_cmd_c2`           |
| `func_80022870`   | 0x80022870  | 0xC9    | `akao_cmd_c9`           |
| `func_800228A0`   | 0x800228A0  | 0xCA    | `akao_cmd_ca`           |
| `func_800228D4`   | 0x800228D4  | 0xD0    | `akao_cmd_d0`           |
| `func_80022900`   | 0x80022900  | 0xD1    | `akao_cmd_d1`           |
| `func_80022934`   | 0x80022934  | 0xD2    | `akao_cmd_d2`           |
| `func_80022970`   | 0x80022970  | 0xD4    | `akao_cmd_d4`           |
| `func_8002299C`   | 0x8002299C  | 0xD5    | `akao_cmd_d5`           |
| `func_800229D0`   | 0x800229D0  | 0xD6    | `akao_cmd_d6`           |
| `func_80022A0C`   | 0x80022A0C  | 0xD8    | `akao_cmd_d8`           |
| `func_80022A38`   | 0x80022A38  | 0xD9    | `akao_cmd_d9`           |
| `func_80022A6C`   | 0x80022A6C  | 0xDA    | `akao_cmd_da`           |
| `FUN_80022aa8`    | 0x80022AA8  | 0xF0    | `akao_cmd_f0`           |
| `FUN_80022ac8`    | 0x80022AC8  | 0xF1    | `akao_cmd_f1`           |
| `func_80022B48`   | 0x80022B48  | (none)  | `akao_get_state`        |
| `func_80022B58`   | 0x80022B58  | (none)  | `akao_reset_pending`    |
| `func_80022B78`   | 0x80022B78  | (none)  | `akao_streaming_upload` |
| `func_80022D8C`   | 0x80022D8C  | (none)  | `akao_play_sequence_blocking_v` |
| `func_80022DAC`   | 0x80022DAC  | (none)  | `akao_upload_indexed_bank` |
| `func_80022FAC`   | 0x80022FAC  | 0xE0    | `akao_cmd_e0`           |
| `FUN_80023010`    | 0x80023010  | 0xE2    | `akao_cmd_e2`           |
| `func_80023060`   | 0x80023060  | 0xE5    | `akao_cmd_e5`           |
| `func_80023098`   | 0x80023098  | 0xE6    | `akao_cmd_e6`           |
| `func_800230C8`   | 0x800230C8  | (none)  | `akao_setup_xa_buffer`  |
| `func_800231AC`   | 0x800231AC  | 0xED    | `akao_cmd_ed`           |
| `func_800231E4`   | 0x800231E4  | 0xEC    | `akao_cmd_ec`           |

**Why deferred:** several of these (`FUN_80022400`, `FUN_8002279c`,
`FUN_80022aa8`, `FUN_80022ac8`, `FUN_80023010`, `func_800227D0`,
`func_8002246C`, `func_800224D8`) are referenced from many overlay translation
units (`cdrom.c`, `main.c`, `decomp7.c`, `title.c`, `movie1.c`, `gover/code1.c`,
`checkps/code.c`, `decomp1.c`) and from the duplicated extern declarations in
the per-overlay headers (`cd.h`, `decomp1.h`, `decomp7.h`, `gover.h`,
`title.h`). Each rename is mechanically simple but the cross-overlay churn is
large; do them as one focused PR.

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
