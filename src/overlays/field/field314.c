#include "common.h"

/**
 * @see decomp.me (100%) TODO
 */
s32 func_800CA960(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0x3, 0x1, arg0);
    sum += func_800CA364(0x4, 0x1, arg0);
    sum += func_800CA364(0x3, 0x2, arg0);
    sum += func_800CA364(0x4, 0x2, arg0);
    return sum > 0;
}

/**
 * @see decomp.me (100%) TODO
 */
s32 func_800CA9DC(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0xF, 0x3, arg0);
    sum += func_800CA364(0x15, 0x3, arg0);
    sum += func_800CA364(0xF, 0x4, arg0);
    sum += func_800CA364(0x15, 0x4, arg0);
    return sum > 0;
}
