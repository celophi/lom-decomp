#include "common.h"

/**
 * @see decomp.me (100%) TODO
 */
s32 func_800CB174(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0x12, 0xE, arg0);
    sum += func_800CA364(0x13, 0xE, arg0);
    sum += func_800CA364(0x14, 0x12, arg0);
    sum += func_800CA364(0x14, 0x13, arg0);
    return sum > 0;
}
