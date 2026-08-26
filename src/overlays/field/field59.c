#include "common.h"

void akao_play_sfx(s32, s32, s32, s32);

/**
 * @brief Play a FIELD sound effect at maximum volume.
 * @param sound_id Sound id forwarded to akao_play_sfx's arg0.
 * @param pan Forwarded to akao_play_sfx's arg2.
 */
void func_800A3938(s32 sound_id, s32 pan)
{
    akao_play_sfx(sound_id, 0, pan, 0x7F);
}
