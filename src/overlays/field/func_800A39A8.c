#include "common.h"

extern s32 D_80119F00;

/**
 * @see decomp.me (100%) TODO
 */
void func_800A39A8(s32 sfx_index, s32 pan, s32 arg2, s32 channel_group)
{
    s32 *table;
    s32 base;
    s32 i;
    s32 mask;
    s32 buf;

    if (D_80119F00 != 0)
    {
        if (channel_group >= 8)
        {
            channel_group = 7;
        }
        table = (s32 *)((s32)&D_80119F00 + D_80119F00);
        if ((u32)sfx_index < (u32)(table[0] - 1))
        {
            buf = (s32)table;
            i = 0;
            base = channel_group * 3;
            buf += ((s32 *)buf)[sfx_index + 1];
            for (; i < 3; i++)
            {
                mask = 1 << (base + i);
                if (!akao_is_sfx_playing(mask))
                {
                    akao_play_sfx_from_buffer(buf, mask, pan, 0x7F);
                    break;
                }
            }
        }
    }
}
