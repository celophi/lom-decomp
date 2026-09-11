#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
extern u8 D_800FDF58[];
extern u8 D_80105AE0[];
/**
 * @brief Find the first eligible actor within the adjusted GTE distance threshold.
 * @param arg0 Reference position in fixed-point coordinates.
 * @param arg1 Distance threshold before adding half the candidate radius.
 * @param arg2 Actor selecting the search group and optional self exclusion.
 * @param arg3 Nonzero to select the opposing group and exclude the input actor.
 * @return Matching actor index, or -1 when no eligible actor is close enough.
 */
s32 func_8009980C(s32 *arg0, s32 arg1, u8 *arg2, s32 arg3)
{
    s32 *delta = (s32 *)0x1F800080;
    s32 *sqr = (s32 *)0x1F800090;
    u8 *var_s3;
    s32 var_s0;
    s32 var_s4;
    s32 var_v0;
    u8 temp_v1;
    u8 *var_s1;
    u8 *var_s2;
    u8 *actor_base;
    if (arg3 != 0)
    {
        if (*(u16 *)(*(u8 **)(arg2 + 0xC) + 0x18) & 1)
        {
            var_s0 = 0;
            if (arg2[0x228] >= 3U)
            {
                var_s0 = 3;
                goto block_5;
            }
            goto block_7;
        }
        var_s0 = 3;
        if (arg2[0x228] < 3U)
        {
        block_5:
            var_s4 = 0xD;
        }
        else
        {
            goto block_6;
        }
    }
    else
    {
    block_6:
        var_s0 = 0;
    block_7:
        var_s4 = 3;
    }
    var_s3 = var_s0 * 0x54 + D_800FDF58;
    actor_base = var_s0 * 0x23C + D_80105AE0;
    var_v0 = -1;
    if (var_s0 < var_s4)
    {
        var_s2 = actor_base + 0x12E;
        var_s1 = var_s3 + 8;
    loop_10:
        temp_v1 = var_s1[0x1D];
        if (temp_v1 == 0xFF || *(s32 *)(var_s2 - 0x12A) == 0 || temp_v1 == 0xFE ||
            (arg3 != 0 && arg2[0x228] == var_s0))
        {
            goto next;
        }
        {
            s32 actor_x = *(s32 *)var_s3;
            s32 reference_x;
            do
            {
                reference_x = arg0[0];
            } while (0);
            delta[0] = (actor_x - reference_x) >> 8;
        }
        delta[1] = (*(s32 *)(var_s1 - 4) - arg0[1]) >> 8;
        delta[2] = (*(s32 *)var_s1 - arg0[2]) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(sqr);
        if (SquareRoot0(sqr[0] + sqr[1] + sqr[2]) < arg1 + ((s32)(*(u16 *)var_s2 << 16) >> 17))
        {
            return var_s0;
        }
    next:
        var_s0++;
        var_s1 += 0x54;
        var_s3 += 0x54;
        var_s2 += 0x23C;
        if (var_s0 < var_s4)
        {
            goto loop_10;
        }
    }
    return -1;
}
