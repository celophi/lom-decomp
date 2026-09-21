#include "common.h"

extern void akao_play_sfx_from_buffer(s32, s32, s32, s32);
extern s32 D_800CB1FC[];

/**
 * @brief Play a sound selected from the world-map sound table.
 * @param sound_index One-based sound index; invalid indices select sound one.
 * @param volume Volume passed to the audio player.
 */
void func_800652A8(s32 sound_index, s32 volume)
{
    if ((u32)(sound_index - 1) >= 0x41U)
    {
        sound_index = 1;
    }
    akao_play_sfx_from_buffer(D_800CB1FC[sound_index - 1], 0, volume, 0x7F);
}
