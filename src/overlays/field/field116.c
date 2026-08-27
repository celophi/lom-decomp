#include "common.h"

void func_800BCB68(s32 flags, s32 duration, s32 field_id, s32 sub_id)
{
    akao_set_song_params(flags, duration, field_id, sub_id);
}

extern void func_8006B8DC(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

void func_800BCB88(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 flag;

    if (arg0 & 0x80)
    {
        flag = 1;
        arg0 &= 0x7F;
    }
    else
    {
        flag = 0;
    }
    func_8006B8DC(arg1, arg2, arg3, flag, arg0);
}
