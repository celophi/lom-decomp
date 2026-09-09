#include "common.h"
#include "sdk/rand.h"

extern u8 g_menuLayoutBuffer[];
s32 func_80087F44(s32 arg0, void *arg1);

/**
 * @brief Select a distance-weighted index within the active layout bound.
 * @param arg0 Actor identifier whose first coordinate is compared with actor 2.
 * @return Selected index, or the distance bucket if no interval was selected.
 * @note Nonmatching m2c translation. The target reads incoming s0 when the
 * layout selector is 3 and accumulates into incoming s2 without initializing
 * it. The corresponding locals intentionally remain uninitialized; their
 * original source-level contract is unresolved.
 */
s32 func_800C9ED4(s32 arg0)
{
    s32 sp20[4];
    s32 sp10[4];
    s32 sp30[6];
    s32 *var_a0_2;
    s32 *var_v1_2;
    s32 temp_a0;
    s32 temp_s1;
    s32 temp_v1;
    s32 var_a0;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a1_3;
    s32 var_a2;
    s32 var_a2_2;
    s32 var_s1;
    s32 var_s2;
    s32 var_s3;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    u32 var_s0;
    u32 var_v1;

    var_s3 = -1;
    if (((s8 *)g_menuLayoutBuffer)[0x29D7] != 3)
    {
        var_s0 = g_menuLayoutBuffer[((s8 *)g_menuLayoutBuffer)[0x29D7] * 0x14C + 0x2B50] >> 4;
    }
    var_v1 = 4;
    if ((s32) var_s0 >= 4)
    {
        var_v1 = 6;
        if ((s32) var_s0 < 7)
        {
            var_v1 = var_s0;
        }
    }
    func_80087F44(2, sp10);
    func_80087F44(arg0, sp20);
    var_v0_2 = sp10[0] - sp20[0];
    if (var_v0_2 < 0)
    {
        var_v0_2 = -var_v0_2;
    }
    var_s1 = var_v0_2 >> 8;
    if (var_v0_2 < 0)
    {
        var_s1 = (s32) (var_v0_2 + 0xFF) >> 8;
    }
    if (var_s1 >= 0)
    {
        var_a0 = 0x95;
        if (var_s1 < 0x96)
        {
            var_a0 = var_s1;
        }
    } else
    {
        var_a0 = 0;
    }
    var_a2 = 1;
    temp_s1 = var_a0 / (s32) (0x96 / (s32) var_v1);
    var_a1 = 0;
    if ((s32) var_v1 > 0)
    {
        var_a0 = temp_s1;
        do
        {
            if (var_a0 < (s32) var_v1)
            {
                (&sp30[0])[var_a0] = 0x400 / var_a2;
            }
            temp_a0 = temp_s1 - var_a1;
            var_v0_3 = var_a2 * 2;
            if (temp_a0 >= 0)
            {
                (&sp30[0])[temp_a0] = 0x400 / var_a2;
                var_v0_3 = var_a2 * 2;
            }
            var_a2 += var_v0_3;
            var_a1 += 1;
            var_a0 = temp_s1 + var_a1;
        } while (var_a1 < (s32) var_v1);
    }
    var_a1_2 = 0;
    if ((s32) var_v1 > 0)
    {
        var_v1_2 = &sp30[0];
        do
        {
            var_a1_2 += 1;
            var_s2 += *var_v1_2;
            var_v1_2 += 1;
        } while (var_a1_2 < (s32) var_v1);
    }
    var_a2_2 = 0;
    var_a1_3 = 0;
    temp_v1 = (rand() * var_s2) / 32767;
    if ((s32) var_v1 > 0)
    {
        var_a0_2 = &sp30[0];
        do
        {
            if ((temp_v1 >= var_a2_2) && (temp_v1 < (var_a2_2 + *var_a0_2)))
            {
                var_s3 = var_a1_3;
            }
            var_a1_3 += 1;
            var_a2_2 += *var_a0_2;
            var_a0_2 += 1;
        } while (var_a1_3 < (s32) var_v1);
    }
    var_v0 = var_s3;
    if (var_s3 == -1)
    {
        var_v0 = temp_s1;
    }
    return var_v0;
}
