# func_80075C88: unverified assembler patch

`func_80075C88` in `src/overlays/field/field11.c` is recorded as **99.99%**
pending validation of a local assembler change. The final C produced a 100%
instruction match with the patched assembler: 2251/2251 instructions, 9004
bytes, and a 0xA8 stack frame. This is a conditional result, not confirmation
that the patch reproduces Sony's original ASPSX 2.67.

The final C has not been measured with the patch removed. The 99.99% annotation
is the requested provisional status, not a newly measured stock-toolchain
percentage. The implementation change and its added regression test have been
reverted from both assembler copies pending discussion with the maintainer.
The patch is preserved below for review.

## Files changed

The same seven-line implementation change was applied to
`MaspsxProcessor._handle_nop_before_next_instruction` in both assembler copies:

- `tools/maspsx/maspsx/__init__.py` (project assembler).
- `tools/lom-dev-mcp/vendor/maspsx/maspsx/__init__.py` (MCP assembler).

The same regression test was added to both copies of `tests/test_nop.py`.
Neither GCC toolchain source nor Sony's original assembler executable was
patched.

## Trigger and observed effect

GCC 2.7.2 CDK emitted this sequence in the palette-handling region:

```asm
lw      $9,188($sp)
$L287:
$L288:
lw      $2,40($9)
```

The second load immediately uses the first load's destination, so maspsx
inserts a load-delay NOP. Before the patch, maspsx placed it between the two
labels:

```asm
lw      $9,188($sp)
$L287:
nop
$L288:
lw      $2,40($9)
```

The patch places both labels before the inserted NOP:

```asm
lw      $9,188($sp)
$L287:
$L288:
nop
lw      $2,40($9)
```

In the affected comparison, the jump at function-relative offset `0x9B4`
changed destination from `0x9C8` to `0x9C4`, matching the original target.
This changes a jump operand even though the instruction sequence at the load
site is otherwise unchanged.

## Exact implementation change

Previously, the handler emitted the first pending label, set
`self.skip_instructions = 1`, then emitted the NOP. These seven lines were
inserted immediately after that assignment:

```python
# Adjacent labels are aliases. A source directive between
# labels can flush ASPSX's delay handling, so stop there.
index = self.lines.index(label, self.line_index + 1) + 1
while index < len(self.lines) and is_label(self.lines[index]):
    res.append(self.lines[index])
    self.skip_instructions += 1
    index += 1
```

The loop consumes additional physically consecutive label lines before the
existing NOP emission. It stops at any non-label line, including a directive
or blank line, and increments the skip count to avoid emitting labels twice.

The patch comment's statement about ASPSX directive handling is an inference
from existing maspsx tests. It has not been established with original ASPSX.
An initial broader change that crossed directives failed the existing
`test_lw_move_nop`; the retained patch only groups consecutive labels.

## Test coverage and limitations

The added `test_load_delay_keeps_consecutive_labels_together` feeds the load
sequence above to `MaspsxProcessor`, using both two-label and three-label
variants. After stripping comments, it expects all labels before the NOP.

Both assembler copies passed all 129 tests during the matching session using
`python3 -m unittest discover -s tests -t .` from each assembler root. The final
patched real-source comparison is retained locally in
`build/mcp-dumps/75c88_final_real/`.

These tests establish the implemented behavior and compatibility with the
existing test suite. Their expected output was chosen for this patch; they
are not an independent oracle for Sony assembler behavior. Matching this one
function also does not establish general assembler correctness.

## Validation still required

1. Assemble a minimal reproducer, including jumps to both labels, with the
   original ASPSX 2.67 executable and equivalent assembler options.
2. Compare emitted instructions, label addresses, relocations, and resolved
   jump destinations with both stock and patched maspsx.
3. Check variants with three labels and intervening directives or blank
   lines, then run the existing original-assembler fixtures and regressions.
4. Measure the final C with stock maspsx and record that baseline. Promote the
   function to a confirmed 100% only after resolving the assembler behavior
   with independent evidence, or obtaining a match without this patch.

An original executable is available at
`/mnt/storage/Projects/LOM/References/Psy-Q-4.1/PSSN/ASPSX.EXE`.
The existing oracle harness is under `tools/maspsx/aspsx/`; its 2.67 runner
requires Wine. No original-ASPSX oracle was run during this matching session.
