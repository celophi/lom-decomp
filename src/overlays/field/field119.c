#include "common.h"

extern s32 g_layout_option;
extern s32 g_layout_sub_mode;

void func_800681E4(s32 arg0, s32 arg1, s32 arg2);

void func_800BCF68(s32 arg0, s32 arg1, s32 arg2)
{
    s32 var_v0;
    s32 var_a3;

    var_v0 = -1;
    if (arg1 != 0xFF)
    {
        var_v0 = arg1;
    }
    arg1 = var_v0;

    var_a3 = -1;
    if (arg2 != 0xFF)
    {
        var_a3 = arg2;
    }

    g_layout_option = -1;

    arg2 = var_a3;

    g_layout_sub_mode = -1;

    func_800681E4(arg0, arg1, arg2);
}

void akao_cmd_a9(s32 arg0, s32 arg1);

void func_800BCFBC(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    if (arg2 == 0)
    {
        arg2 = 1;
    }
    akao_cmd_a9(arg2, arg3);
}

extern u8 *D_80123FB8;

void func_800BCFEC(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 sp10;
    s32 var_a0;

    var_a0 = arg0;
    if (arg2 == 0xFF)
    {
        sp10 = (s32) *D_80123FB8;
    }
    else
    {
        sp10 = arg2;
    }
    if (var_a0 == 0xFF)
    {
        var_a0 = *D_80123FB8;
    }
    func_8008B5D0(var_a0, arg1, 1, &sp10);
}
