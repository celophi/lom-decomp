#include "common.h"

extern s32 D_8013986C;
extern s32 D_80139950[];

/**
 * @brief Classify a point against two world-map coordinate regions.
 * @param x Point X coordinate.
 * @param y Point Y coordinate.
 * @return Region code: zero, one, or two.
 */
s32 func_80058400(s32 x, s32 y)
{
    s32 dy;
    u32 dx;

    dx = x - D_80139950[0];
    dy = y - D_80139950[1];
    if ((dx < 0x61U) && (dy >= 0) && (dy < 0x61))
    {
        if ((D_8013986C == 3) || (D_8013986C == 1))
        {
            return 0;
        }
        return 2;
    }
    if ((u32)(dx - 16) < 97 && dy >= 16 && dy < 113)
    {
        return 1;
    }
    return 0;
}
