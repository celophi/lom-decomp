#include "common.h"

s32 func_800CB03C(s32 arg0)
{
    return func_800CA364(0x17, 0x17, arg0) > 0;
}

s32 func_800CB064(s32 arg0)
{
    return func_800CA364(0x18, 0x17, arg0) > 0;
}

s32 func_800CB08C(s32 arg0)
{
    return (func_800CA364(0x17, 0xC, arg0) + func_800CA364(0x17, 0xD, arg0)) > 0;
}

s32 func_800CB0D8(s32 arg0)
{
    s32 val = 0x19;

    return func_800CA364(val, val, arg0) > 0;
}

s32 func_800CB100(s32 arg0)
{
    return func_800CA364(0x19, 0xA, arg0) > 0;
}

s32 func_800CB128(s32 arg0)
{
    return (func_800CA364(0x1A, 0xF, arg0) + func_800CA364(0x19, 0x18, arg0)) > 0;
}
