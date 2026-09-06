#include "common.h"

extern s32 D_8011BF00;

/**
 * @brief Play a sound effect in the first available channel of a channel group.
 * @param sfx_index Sound-effect table index.
 * @param pan Pan value passed to the sound-effect player.
 * @param arg2 Sound-effect table selector.
 * @param channel_group Three-channel group selector.
 */
void func_800A3A90(s32 sfx_index, s32 pan, s32 arg2, s32 channel_group)
{
    s32 *table;
    s32 base;
    s32 i;
    s32 mask;
    s32 buf;
    u8 *p;

    if (arg2 < 2)
    {
        p = (u8 *)&D_8011BF00;
        table = (s32 *)(p + arg2 * 0x1A00);
        if (table[0] != 0)
        {
            if ((u32)sfx_index < (u32)table[0])
            {
                i = 0;
                base = arg2 * 3;
                buf = (s32)table + table[sfx_index + 1];
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
}
