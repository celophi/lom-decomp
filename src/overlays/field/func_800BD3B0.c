#include "common.h"

extern u8 D_800F0E08[8];

s32 func_800BD318(s32 arg0, s32 arg1, s32 *arg2, s32 *arg3);
void func_800BD650(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/**
 * @brief Adds a value into a per-slot record and forwards it through two calls.
 *
 * Extracts the high 16 bits of @p arg1 as a slot index, calls func_800BD318
 * with that index re-packed into the high half (@p arg0 is passed straight
 * through), then forwards the result plus the two out-params to func_800BD650,
 * indexing @c D_800F0E08 by bits 28-30 of @p arg1.
 *
 * @param arg0 Passed through unchanged to func_800BD318 (kept in a0).
 * @param arg1 Packed value; high 16 bits select the slot, bits 28-30 index
 *             D_800F0E08.
 * @note 68.0% match (gcc280_g0). The residue is a coupled scheduling/register
 *       decision: the target keeps @c arg1>>16 in caller-saved a1 with an s0
 *       copy, flipping both the sra-before-prologue placement and the jal
 *       delay-slot fill. The `result`/`index` split and the `do {} while (0)`
 *       wrapper are required to reproduce the target's separate copy insn
 *       (without them the match drops to 47.6%); do not remove them.
 */
void func_800BD3B0(s32 arg0, s32 arg1)
{
    s32 sp18;
    s32 sp1c;
    s32 index;
    s32 result;

    result = arg1 >> 0x10;
    index = result;
    do
    {
        result = func_800BD318(arg0, index << 0x10, &sp18, &sp1c);
        func_800BD650(2, result, sp18, sp1c, D_800F0E08[((u32) index >> 0xC) & 7]);
    } while (0);
}
