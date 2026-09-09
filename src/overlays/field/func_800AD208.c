#include "common.h"

void *func_800AD42C(void *, s32 *, s32, u16 *, s32); /* extern */

/**
 * @brief Draw a right-aligned decimal value and append the draw-mode primitive.
 * @param arg0 Ordering-table entry receiving the primitives.
 * @param arg1 Writable primitive buffer.
 * @param arg2 Value to draw.
 * @param arg3 Number of digit positions.
 * @param arg4 Drawing position; its horizontal coordinate advances as digits are emitted.
 * @param arg5 Digit rendering style passed to the primitive builder.
 * @return Buffer address immediately after the emitted primitives.
 */
void *func_800AD208(s32 *arg0, void *arg1, s32 arg2, s32 arg3, u16 *arg4, s32 arg5)
{
    s32 temp_a0;
    s32 temp_lo;
    s32 var_s1;
    s32 var_s2;
    s32 var_s3;
    s32 var_v0;
    s32 var_v0_2;
    u8 *var_t0;

    var_t0 = arg1;
    var_s3 = arg2;
    var_s1 = 1;
    var_s2 = var_s1;
    if (var_s2 < arg3)
    {
        do
        {
            var_s1 *= 10;
            var_s2 += 1;
        } while (var_s2 < arg3);
    }
    for (var_s2 = 0; var_s2 < arg3; var_s2++)
    {
        if (var_s3 >= var_s1)
        {
            break;
        }
        var_s1 /= 10;
    }
    if (var_s2 != arg3)
    {
        *arg4 += var_s2 * 8;
        if (var_s2 < arg3)
        {
            do
            {
                temp_lo = var_s3 / var_s1;
                var_t0 = func_800AD42C(var_t0, arg0, temp_lo, arg4, arg5);
                var_s3 -= temp_lo * var_s1;
                var_s2 += 1;
                *arg4 += 8;
                var_s1 /= 0xA;
            } while (var_s2 < arg3);
        }
    }
    else
    {
        temp_a0 = *arg4 - 8;
        *arg4 = temp_a0 + arg3 * 8;
        var_t0 = func_800AD42C(var_t0, arg0, var_s3, arg4, arg5);
        *arg4 += 8;
    }
    *(u8 *)(var_t0 + 3) = 1;
    *(u32 *)(var_t0 + 4) = 0xE1000007;
    *(u32 *)var_t0 = (s32)((*(u32 *)var_t0 & 0xFF000000) | (*arg0 & 0xFFFFFF));
    *arg0 = (*arg0 & 0xFF000000) | ((s32)var_t0 & 0xFFFFFF);
    return var_t0 + 8;
}
