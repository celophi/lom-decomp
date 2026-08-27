#include "common.h"

s32 func_800CA620(s32 arg0)
{
    s32 val = 0xD;

    return func_800CA364(val, val, arg0) > 0;
}

s32 func_800CA648(s32 arg0)
{
    return func_800CA364(0xD, 0xC, arg0) > 0;
}

s32 func_800CA670(s32 arg0)
{
    return (func_800CA364(0x17, 0x12, arg0) + func_800CA364(0x17, 0x13, arg0)) > 0;
}
