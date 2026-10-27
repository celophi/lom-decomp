#include "common/overlay_audio.h"
#include "main/audio/akao.h"
#include "main/audio/akao_cmd.h"

/**
 * @brief Play a sound effect at full volume.
 * @param sound_id Sound effect ID, such as SFX_CURSOR.
 * @param pan Stereo pan; AKAO_PAN_CENTER for a centred sound.
 * @see decomp.me (100%, TITLE copy) https://decomp.me/scratch/ZuKeL
 */
void play_sfx(s32 sound_id, s32 pan)
{
    akao_play_sfx(sound_id, 0, pan, AKAO_VOLUME_MAX);
}
