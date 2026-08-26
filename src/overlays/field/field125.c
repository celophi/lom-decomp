#include "common.h"

void func_800BD434(s32 arg0, s32 arg1, s32 arg2);
void func_800BD4A8(s32 arg0, s32 arg1, s32 arg2);

void func_800BD520(s32 arg0, u32 arg1, s32 arg2)
{
    if (arg1 <= 0xFFFFU)
    {
        func_800BD434(arg0, arg1 << 0x10, arg2);
        return;
    }
    func_800BD4A8(arg0, arg1 << 0x10, arg2);
}
