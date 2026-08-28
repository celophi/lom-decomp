#include "common.h"

/**
 * @see decomp.me (100%) TODO
 */
s32 func_800CA808(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0x1, 0x0, arg0);
    sum += func_800CA364(0x2, 0x0, arg0);
    sum += func_800CA364(0x6, 0x1, arg0);
    sum += func_800CA364(0x7, 0x1, arg0);
    sum += func_800CA364(0x6, 0x2, arg0);
    sum += func_800CA364(0x7, 0x2, arg0);
    return sum > 0;
}
