#include "common.h"

extern u8 *D_80123FB8;

extern void func_8008A580(s32 arg0, s32 arg1);
extern void func_8008B500(s32 arg0, s32 arg1);

void func_800BC65C(s32 arg0, s32 arg1)
{
    if (arg0 == 0xFF)
    {
        arg0 = *D_80123FB8;
    }

    if (arg1 & 0x8000)
    {
        func_8008A580(arg0, arg1 & 0x7FFF);
    }
    else
    {
        func_8008B500(arg0, arg1);
    }
}
