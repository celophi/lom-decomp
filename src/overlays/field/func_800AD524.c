#include "common.h"

void *func_800AD658(s32 *, void *, s32);

/**
 * @brief Emit a digit sprite and optionally append the auxiliary primitive.
 * @param arg0 Next free primitive-buffer address.
 * @param arg1 Ordering-table entry receiving the primitive chain.
 * @param arg2 Value whose decimal units select the digit glyph.
 * @param arg3 Packed destination coordinates.
 * @param arg4 Palette selector and optional-primitive flag (bit 7).
 * @return The next free primitive-buffer address.
 * @note WIP: arithmetic scheduling and temporary-register differences remain.
 */
void *func_800AD524(u8 *arg0, s32 *arg1, s32 arg2, s32 *arg3, s32 arg4)
{
    s16 var_v0;
    s16 var_v0_2;
    s32 temp_v0;
    s32 temp_v1;
    u8 *var_t0;

    *(u32 *)(arg0 + 4) = 0x808080;
    arg0[3] = 4;
    arg0[7] = 0x64;
    *(s32 *)(arg0 + 8) = *arg3;
    temp_v0 = arg2 % 10;
    if (arg2 >= 0xA)
    {
        var_v0 = temp_v0 * 8 + 0x2800;
    }
    else
    {
        var_v0 = temp_v0 * 8 + 0x2000;
    }
    *(s16 *)(arg0 + 0xC) = var_v0;
    *(u32 *)(arg0 + 0x10) = 0x80008;
    temp_v1 = arg4 & 0x7F;
    if (temp_v1 != 1)
    {
        var_v0_2 = 0x7B03;
        if (temp_v1 >= 2)
        {
            if (temp_v1 != 2)
            {
                *(s16 *)(arg0 + 0xE) = 0x7B03;
            }
            else
            {
                var_v0_2 = 0x7AC9;
                goto block_9;
            }
        }
        else
        {
            goto block_9;
        }
    }
    else
    {
        var_v0_2 = 0x7AC8;
    block_9:
        *(s16 *)(arg0 + 0xE) = var_v0_2;
    }
    *(s32 *)(arg0 + 0) = (*(s32 *)(arg0 + 0) & 0xFF000000) | (*arg1 & 0xFFFFFF);
    *arg1 = (*arg1 & 0xFF000000) | ((s32)arg0 & 0xFFFFFF);
    var_t0 = arg0 + 0x14;
    if (arg4 & 0x80)
    {
        var_t0 = func_800AD658(arg1, var_t0, 1);
    }
    return var_t0;
}
