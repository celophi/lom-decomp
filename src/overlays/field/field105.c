#include "common.h"

extern u8 *D_80123FB8;

extern s32 func_80087770(s32 arg0, s32 arg1);
extern void func_800BD520(s32 arg0, u32 arg1, s32 arg2);

void func_800BC86C(s32 arg0, s32 arg1)
{
    arg0 = (arg0 == 0xFF) ? *D_80123FB8 : arg0;
    arg1 = (arg1 == 0xFF) ? *D_80123FB8 : arg1;

    func_800BD520(0, 0x7100, func_80087770(arg0, arg1));
}

void func_800BC8CC(s32 arg0, s32 arg1)
{
    func_800BD520(*D_80123FB8, arg0, func_800BD414(*D_80123FB8, arg1 & 0xFFFF));
}

extern void func_800A3904(s32 arg0, s32 arg1, s32 arg2);

void func_800BC91C(s32 arg0, s32 arg1)
{
    arg1 = (arg1 != 0) ? arg1 : 1;
    if (arg0 >= 0x80)
    {
        arg0 = 0x7F;
    }
    func_800A3904(1, arg1, arg0);
}
