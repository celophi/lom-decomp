#include "common.h"

void func_800C28B8(s32 arg0);
void func_80087A9C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5,
                   s32 arg6, s32 arg7, s32 arg8, s32 arg9);

void func_800BCBD0(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 var_s0;
    s32 var_s1;

    if (arg0 & 0x80)
    {
        var_s1 = 1;
        var_s0 = arg0 & 0x7F;
    }
    else
    {
        var_s1 = 0;
        var_s0 = arg0;
    }
    func_800C28B8(var_s0);
    func_80087A9C(var_s0, arg1, arg2, arg3, 0, -1, -1, -1, 0, var_s1);
}
