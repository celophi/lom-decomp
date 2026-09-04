#include "common.h"

extern u8 *D_80122B74;
extern u8 D_800F198C[];
extern u16 g_music_track_index;

s32 func_800C3518(s32 arg0);
s32 func_800C3688(s32 arg0);
void func_800C2138(s32 arg0);

s32 func_800C33E4(s32 arg0, s32 arg1, s32 *arg2)
{
    s32 *s0;
    s32 s1;
    s32 count;
    s32 i;
    u32 temp;

    s0 = arg2;
    count = 0;
    for (i = 0; i < 0x40; i++)
    {
        temp = D_80122B74[i * 0xC + 0x2F0];
        if ((temp & 1) && !((temp >> 1) & 1))
        {
            count += 1;
        }
    }

    s1 = 0;
    if (count < 3)
    {
        if (func_800C3518(arg0) >= 0)
        {
            s1 = 1;
            *s0 = arg0;
            s0++;
        }
        if (func_800C3518(arg1) >= 0)
        {
            s1 += 1;
            *s0 = arg1;
            goto block_12;
        }
    }
    else if (func_800C3518(arg0) >= 0)
    {
        s1 = 1;
        *s0 = arg0;
    block_12:
        s0++;
    }

    if (s1 == 0)
    {
        i = func_800C3688(g_music_track_index);
        temp = 0x1F;
        if (i < 0x20)
        {
            temp = i;
        }
        func_800C2138(D_800F198C[temp]);
    }
    *s0 = 0xFF;
    return s1;
}
