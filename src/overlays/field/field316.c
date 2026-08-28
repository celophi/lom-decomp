#include "common.h"

/**
 * @see decomp.me (100%) TODO
 */
s32 func_800CAF3C(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0xE, 0xC, arg0);
    sum += func_800CA364(0x14, 0xC, arg0);
    sum += func_800CA364(0xE, 0xD, arg0);
    sum += func_800CA364(0x14, 0xD, arg0);
    return sum > 0;
}

/**
 * @see decomp.me (100%) TODO
 */
s32 func_800CAFB8(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0x11, 0x5, arg0);
    sum += func_800CA364(0x16, 0x5, arg0);
    sum += func_800CA364(0x11, 0x9, arg0);
    sum += func_800CA364(0x16, 0x9, arg0);
    return sum > 0;
}
