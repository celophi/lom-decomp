#include "common.h"

s32 func_800C19D0(s32 arg0, s32 arg1, s32 arg2)
{
    s32 result;

    if ((u32) arg1 >= 6 && (arg2 & 1))
    {
        result = arg0 + ((u32) (arg1 - 5) >> 1) + 5;
    }
    else
    {
        result = arg0 + arg1;
    }
    if ((arg2 & 2) && (u32) result >= 0x3E8)
    {
        result = 0x3E7;
    }
    return result;
}
