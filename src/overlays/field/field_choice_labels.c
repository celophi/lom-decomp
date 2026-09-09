#include "common.h"
extern u8 D_800EC3EA[];
extern s32 D_801226D8;
s32 func_800A88A0(s32, s32, u8 *, s32, s32, s32, s32);

/**
 * @brief Draw two choice labels with the active choice highlighted.
 * @param arg0 Ordering-table address.
 * @param arg1 Primitive buffer address.
 * @param arg2 Horizontal scroll offset.
 * @param arg3 Vertical scroll offset.
 */
void func_800AED20(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 var_a3;
    s32 var_t0;
    s32 temp_a1;
    s32 temp_s2;
    u8 *temp_s1;

    var_t0 = 5;
    temp_s1 = D_800EC3EA - 0x26;
    if (D_801226D8 == 0)
    {
        var_t0 = 4;
    }
    temp_s2 = 0x50 - arg2;
    temp_a1 = func_800A88A0(arg1, arg0, D_800EC3EA[0] + ((D_800EC3EA[1] << 8) + temp_s1), var_t0, temp_s2, 1 - arg3, 2);
    var_a3 = 5;
    if (D_801226D8 == 1)
    {
        var_a3 = 4;
    }
    func_800A88A0(temp_a1, arg0, temp_s1[0x28] + ((temp_s1[0x29] << 8) + temp_s1), var_a3, temp_s2, 0x11 - arg3, 2);
}
