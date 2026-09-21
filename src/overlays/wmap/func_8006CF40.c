#include "common.h"

#include "sdk/libgte.h"

/**
 * @brief Scale packed RGB channels, preserving the high byte.
 * @param color Packed RGB value and high byte.
 * @param scale Scale in units of 1/128, or -1 to preserve the color.
 * @return Packed scaled color; channel results wrap to eight bits.
 */
s32 func_8006CF40(s32 color, s32 scale)
{
    CVECTOR *channels = (CVECTOR *)&color;
    s32 red;
    s32 green;
    s32 blue;
    if (scale == -1)
    {
        return color;
    }
    else
    {
        red = channels->r * scale;
        green = channels->g * scale;
        blue = channels->b * scale;
        channels->r = red >> 7;
        channels->g = green >> 7;
        channels->b = blue >> 7;
        return color;
    }
}
