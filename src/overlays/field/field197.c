#include "common.h"

extern u8 D_80043818;
extern void func_800BD520(s32 arg0, s32 arg1, s32 arg2);

void func_800C963C(void)
{
    if (D_80043818 != 0)
    {
        func_800BD520(0, 0x2F08, 0x80);
    }
    else
    {
        func_800BD520(0, 0x2F08, 0xFF);
    }
}
