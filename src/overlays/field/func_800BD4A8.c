#include "common.h"

extern u8 D_800F0E08[8];

s32 func_800BD318(s32 arg0, s32 arg1, s32 *arg2, s32 *arg3);
void func_800BD55C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);

/**
 * @brief Resolves a packed slot and forwards decremented slot metadata.
 *
 * @param arg0 Passed through to func_800BD318.
 * @param arg1 Packed value whose high half selects the slot.
 * @param arg2 Final argument forwarded to func_800BD55C.
 * @note 72.966670% match with gcc280_g0. The remaining four-row mismatch is
 *       scheduling around the first call; all 30 target instructions and the
 *       0x30-byte stack frame are otherwise represented.
 */
void func_800BD4A8(s32 arg0, s32 arg1, s32 arg2)
{
    s32 sp18;
    s32 sp1c;
    s32 index;
    s32 result;

    result = arg1 >> 0x10;
    index = result;
    /* Retain the original scheduler's saved-register ordering. */
    arg2++;
    arg2--;
    do
    {
        result = func_800BD318(arg0, index << 0x10, &sp18, &sp1c);
        func_800BD55C(2, result, sp18, sp1c,
                      D_800F0E08[((u32)index >> 0xC) & 7] - 1, arg2);
    } while (0);
}
