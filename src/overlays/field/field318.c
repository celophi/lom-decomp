#include "common.h"

/**
 * @see decomp.me (100%) TODO
 */
s32 func_800CB4A4(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0xE, 0xB, arg0);
    sum += func_800CA364(0x14, 0xB, arg0);
    sum += func_800CA364(0x10, 0xE, arg0);
    sum += func_800CA364(0x14, 0x10, arg0);
    return sum > 0;
}
