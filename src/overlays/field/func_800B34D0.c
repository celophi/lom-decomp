#include "common.h"

extern s32 D_8010D020;

void func_800B3580(void);
s32 func_800B37D4(void);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
s32 func_800B3DF4(s32 arg0);
void func_800B4390(void);

/**
 * @see decomp.me (100%)
 */
void func_800B34D0(s32 arg0)
{
    s32 value;

    if (arg0 != 0)
    {
        func_800B3580();
        value = func_800B37D4();
        if (D_8010D020 != 0)
        {
            func_800BD520(0, 0x4280, 1);
        }
        else
        {
            func_800BD520(0, 0x4280, value);
        }
        value = func_800B3DF4(arg0);
        if (D_8010D020 != 0)
        {
            func_800BD520(0, 0x4284, 1);
        }
        else
        {
            func_800BD520(0, 0x4284, value);
        }
    }
    else
    {
        func_800B4390();
    }
}
