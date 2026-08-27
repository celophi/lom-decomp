#include "common.h"

extern u8 *D_80122B74;
extern void func_800C2228(s32 arg0);
extern void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);

void func_800C2138(s32 arg0)
{
    u8 *p;
    u8 *q;

    if (arg0 < 0xFF)
    {
        p = D_80122B74 + arg0;
        p[0x25E0] += 1;
        q = D_80122B74 + arg0;
        if (q[0x25E0] >= 0x64)
        {
            q[0x25E0] = 0x63;
        }
        func_800C2228(arg0);
    }
    else
    {
        akao_set_song_params(0x8001, 0x71, arg0, 0);
    }
}
