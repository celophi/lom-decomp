#include "common.h"

/**
 * @see decomp.me (100%) TODO
 */
s32 func_800CAB0C(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0xC, 0x5, arg0);
    sum += func_800CA364(0xD, 0x5, arg0);
    sum += func_800CA364(0xC, 0x9, arg0);
    sum += func_800CA364(0xD, 0x9, arg0);
    return sum > 0;
}

/**
 * @see decomp.me (100%) TODO
 */
s32 func_800CAB88(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0xF, 0x5, arg0);
    sum += func_800CA364(0x15, 0x5, arg0);
    sum += func_800CA364(0xF, 0x9, arg0);
    sum += func_800CA364(0x15, 0x9, arg0);
    return sum > 0;
}

/**
 * @see decomp.me (100%) TODO
 */
s32 func_800CAC04(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0x0, 0x0, arg0);
    sum += func_800CA364(0x6, 0x6, arg0);
    sum += func_800CA364(0x7, 0x6, arg0);
    sum += func_800CA364(0x7, 0x7, arg0);
    return sum > 0;
}
