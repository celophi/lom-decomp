#include "common.h"

s32 func_800CB3A0(s32 arg0)
{
    s32 sum;

    sum = func_800CA364(0xC, 0xB, arg0) + func_800CA364(0xD, 0xB, arg0);
    sum += func_800CA364(0x10, 0xB, arg0);
    sum += func_800CA364(0x10, 0xC, arg0);
    sum += func_800CA364(0x10, 0xD, arg0);
    return sum > 0;
}
