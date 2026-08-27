#include "common.h"

/**
 * @brief Thin stack-frame wrapper testing whether func_800CA364(8, 8, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if func_800CA364(8, 8, arg0) is greater than 0, otherwise 0.
 */
s32 func_800CAD50(s32 arg0)
{
    return func_800CA364(8, 8, arg0) > 0;
}

s32 func_800CAD78(s32 arg0)
{
    return (func_800CA364(0xE, 0x8, arg0) + func_800CA364(0x14, 0x8, arg0)) > 0;
}

s32 func_800CADC4(s32 arg0)
{
    return func_800CA364(0x17, 0x8, arg0) > 0;
}

s32 func_800CADEC(s32 arg0)
{
    s32 val = 0xA;

    return func_800CA364(val, val, arg0) > 0;
}

s32 func_800CAE14(s32 arg0)
{
    return func_800CA364(0x1A, 0xA, arg0) > 0;
}

s32 func_800CAE3C(s32 arg0)
{
    return (func_800CA364(0xB, 0xA, arg0) + func_800CA364(0x10, 0xA, arg0)) > 0;
}
