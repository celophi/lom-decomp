#include "common.h"

s32 func_800CB100(s32 arg0)
{
    return func_800CA364(0x19, 0xA, arg0) > 0;
}

s32 func_800CB128(s32 arg0)
{
    return (func_800CA364(0x1A, 0xF, arg0) + func_800CA364(0x19, 0x18, arg0)) > 0;
}
