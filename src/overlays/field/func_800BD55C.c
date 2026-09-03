#include "common.h"

void func_800BD55C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    s32 clearMask;
    s32 valueMask;
    u8 *p8;
    u16 *p16;
    s32 *p32;

    if (arg1 == 0)
    {
        return;
    }
    if (arg1 == -1)
    {
        return;
    }

    if (arg4 < 0x20)
    {
        clearMask = ~(((1 << arg4) - 1) << arg3);
    }
    else
    {
        clearMask = 0;
    }

    if (arg4 < 0x20)
    {
        valueMask = ((1 << arg4) - 1) << arg3;
    }
    else
    {
        valueMask = -1;
    }

    switch (arg0)
    {
    case 0:
        p8 = (u8 *)(arg1 + arg2);
        *p8 = (*p8 & clearMask) | (valueMask & (arg5 << arg3));
        break;
    case 1:
        p16 = (u16 *)((arg2 * 2) + arg1);
        *p16 = (*p16 & clearMask) | (valueMask & (arg5 << arg3));
        break;
    case 2:
        p32 = (s32 *)((arg2 * 4) + arg1);
        *p32 = (*p32 & clearMask) | (valueMask & (arg5 << arg3));
        break;
    }
}
