#include "common.h"

extern u8 *g_field_script;

s32 func_800C2DC0(void);
s32 func_800C2D08(void);
void func_8006AD04(s32 arg0, s32 arg1, s32 arg2);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);

void func_800BC3DC(s32 arg0, s32 arg1)
{
    s32 var_s1;
    s32 var_s0;

    if (arg0 == 0xFF)
    {
        var_s1 = *g_field_script;
    }
    else
    {
        var_s1 = arg0;
    }
    if (arg1 == 1)
    {
        var_s0 = func_800C2DC0();
    }
    else
    {
        var_s0 = func_800C2D08();
    }
    if (var_s0 != 0xFF)
    {
        func_8006AD04(var_s1, var_s0, 1);
    }
    func_800BD520(0, 0x2F00, var_s0);
}
