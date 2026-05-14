# Splat Reloc Overrides: Fixing False Mismatches in `target.s`

## TL;DR

When `target.s` shows a literal offset like `sw v0,0x522c(s1)` but your `compiled.s`
shows the symbolic form `sw v0,%lo(g_gameState)(s1)`, **don't change the C source**
— add a `MIPS_LO16` entry to `config/relocations/slus_reloc_addrs.txt`. The opposite
case (target symbolic, compiled literal) is rare and usually means you should use
`MIPS_NONE` instead.

The mismatch is almost always a splat disassembly artifact, not a real codegen
difference. Both forms encode to the **exact same 32-bit instruction**; they only
differ in how the disassembler chose to print them.

## Why This Happens

We do not have the original object files for the retail PSX binary. Splat
disassembles the linked image, where every `lui`/`%hi` + `addi`/`%lo` pair has
already been resolved to raw 16-bit immediates. Splat reconstructs symbolic
references by walking the instruction stream and tracking which register holds
the `%hi` of which symbol — then it can pretty-print subsequent `sw/lw/...`
instructions that use that register as `%lo(symbol)(reg)`.

That tracking is heuristic. It loses state across a few situations:

1. **Interleaved rodata blocks.** When splat emits a `.rodata+0xNN` marker
   mid-function (because data lives between code blocks — typical for PSX jump
   tables), the disassembler resets its register state. After the marker it no
   longer knows that `s1` still holds `%hi(g_gameState)`, so it falls back to
   printing the raw offset `0x522c(s1)`.
2. **Register reuse through `move`.** A `move s1, s0` carries the `%hi` value but
   splat sometimes doesn't propagate it through.
3. **Long-lived `%hi` registers across many basic blocks.** If the `%hi` load is
   far away from the use, splat may give up tracking.

The C compiler, by contrast, emits an `R_MIPS_LO16` relocation for every
`%lo()` reference it generates — so `compiled.s` consistently shows the symbolic
form. Hence the false diff.

## How to Recognize It

Look at `target.s` for `lui REG,%hi(SYMBOL)` near function start, then literal
`NNNN(REG)` uses of that same register later. If `%hi(SYMBOL)` matches the
high half of `SYMBOL`'s address, and the literal `NNNN` matches the low half,
it's the same address — and a candidate for a reloc override.

Example from `Main`:

```mips
64:    lui     s0,%hi(g_gameState)     ; s0 = 0x8003 (g_gameState = 0x8003522C)
6c:    move    s1,s0                   ; s1 also holds 0x8003

154:   sw      v0,%lo(g_gameState)(s0) ; splat tracked s0 → symbolic
1a8:   lw      v1,%lo(g_gameState)(s1) ; splat still tracking s1 → symbolic
...
31c:   sw      v0,0x522c(s1)           ; AFTER a .rodata marker, splat lost s1 → literal
3bc:   sw      v0,0x522c(s1)           ; same
...
5e8:   lw      v1,%lo(g_gameState)(s1) ; tracking recovered → symbolic again
```

`0x522c` is exactly `%lo(0x8003522C)`, so every one of those `0x522c(s1)` lines
is a `g_gameState` access that splat failed to symbolize.

The compiler always emits all of these symbolically, so `compiled.s` shows
`%lo(g_gameState)(s1)` everywhere — diff fails on every literal-form line.

## The Fix: `config/relocations/<overlay>_reloc_addrs.txt`

Splat accepts override entries that force a specific relocation type on a
specific instruction address. The file format is one entry per line:

```
rom:0xROM_OFFSET reloc:RELOC_TYPE symbol:SYMBOL_NAME
```

For the case above, we tell splat "the instruction at ROM 0x1874 IS an
`R_MIPS_LO16` referencing `g_gameState`":

```
rom:0x1874 reloc:MIPS_LO16 symbol:g_gameState
```

After re-running splat, `target.s` regenerates with `sw v0,%lo(g_gameState)(s1)`
at that address — matching `compiled.s`.

### Computing ROM offsets

ROM offset is the byte position in the binary image. To convert a VRAM address
to ROM offset for the main SLUS overlay:

```
ROM = VRAM - 0x80010000 + 0x800
```

(The 0x800-byte PSX-EXE header is followed by code loaded at VRAM `0x80010000`.)

To find the ROM offset of a specific instruction inside a function:

1. Look up the function's VRAM in `config/symbols/shared_symbol_addrs.txt`
   (e.g. `Main = 0x80010D58`).
2. Convert to ROM: `0x10D58 - 0x10000 + 0x800 = 0x1558`.
3. Add the function-relative offset shown in `target.s` (e.g. `0x31c` → ROM
   `0x1558 + 0x31c = 0x1874`).

Other overlays use their own VRAM base; check the `vram:` field in the splat
yaml.

### Reloc types

| Type | Meaning | When to use |
|------|---------|-------------|
| `MIPS_LO16` | Forces `%lo(symbol)` form on the immediate | Splat printed a raw `0xNNNN(reg)` that should be `%lo(symbol)(reg)` |
| `MIPS_HI16` | Forces `%hi(symbol)` form on the immediate | Splat printed a raw `lui reg,0xNNNN` that should be `lui reg,%hi(symbol)` |
| `MIPS_NONE` | Strips any auto-detected reloc; prints raw value | Splat **wrongly** symbolized a literal (false positive on the symbol detector) |
| `MIPS_32` | Forces a full 32-bit pointer reloc in data | Used in rodata/data segments where a word should reference a symbol |

The override lines accept an optional `addend:` field — see
[`tools/splat/docs/Advanced-Reloc.md`](../../tools/splat/docs/Advanced-Reloc.md)
for the full spec.

## What This Fix Is NOT For

The reloc override is **only** the right tool when `target.s` is showing a
splat-disassembly artifact. It is NOT a fix for real codegen mismatches:

- Instruction reordering, different register allocation, extra/missing
  nops in delay slots → these are C-source issues. Fix the C, not the
  reloc file.
- A symbol genuinely referenced differently in the original (e.g. through a
  literal-address pointer cast) → in that case the `target.s` form is real
  and you need to change the C to match.

Tell-tale signs that it's a splat artifact and not real:

- The instruction encoding is the same on both sides (verify by hand: `%hi`
  of the symbol matches the upper byte of `lui ...,0xNNNN`; `%lo` matches the
  immediate displayed as `0xNNNN(reg)`).
- The register holding the base was set earlier in the function from a
  `lui ..., %hi(symbol)` and not clobbered since.
- The literal form appears between or after splat-inserted `.rodata+0xNN`
  markers in the function.

If those are all true, it's safe to add a `MIPS_LO16` override and move on.

## Working Example

See `config/relocations/slus_reloc_addrs.txt` for live overrides:

- `g_akao_seq_channel0` in `akao_upload_xa_program` / `akao_cmd_ec` uses
  `MIPS_NONE` (splat over-eagerly resolved a literal as a symbol — strip it).
- `g_gameState` / `g_previousGameState` in `Main` uses `MIPS_LO16` (splat
  lost tracking across rodata markers — force the symbol form back on).

The two cases together cover both directions of the mismatch.
