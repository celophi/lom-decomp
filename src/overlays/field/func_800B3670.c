#include "common.h"

typedef struct
{
    u8 pad[0x2E5];
    u8 unk2E5;
} StructB74;

extern u8 D_800F0AE8[];
extern StructB74 *D_80122B74;
extern u16 g_music_track_index;

u32 func_800BD414(s32 arg0, s32 arg1);
s32 func_800C3688(s32 arg0);

/**
 * @see decomp.me (100%)
 */
s32 func_800B3670(s32 arg0)
{
    s32 flag;
    s32 mode;
    s32 index;
    s32 value;
    u32 lo;
    u32 hi;

    flag = arg0;
    if (func_800BD414(0, 0x52F0) & 0x80)
    {
        flag = 1;
    }
    mode = func_800BD414(0, 0x2938);

    if (flag != 0)
    {
        switch (mode)
        {
            case 1:
                index = D_80122B74->unk2E5 + 0x14;
                break;
            case 2:
                index = 0x3F;
                break;
            default:
                index = D_80122B74->unk2E5;
                break;
        }
        index = (index * 3) / 2;
    }
    else
    {
        index = func_800C3688(g_music_track_index);
        switch (mode)
        {
            case 1:
                index += 0x14;
                break;
            case 2:
                index = 0x3F;
                break;
        }
    }

    if (index >= 0x40)
    {
        index = 0x3F;
    }

    value = D_800F0AE8[index];
    lo = func_800BD414(0, 0x52E0);
    hi = func_800BD414(0, 0x52E8);
    if (value < lo)
    {
        value = lo;
    }
    else if (value > hi)
    {
        value = hi;
    }

    if (value >= 0x64)
    {
        value = 0x63;
    }
    return value;
}
