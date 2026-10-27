#include "common/overlay_audio.h"
#include "main/audio/akao.h"

/**
 * @brief Start the sequence loaded by load_song_from_disc at full volume.
 */
void play_loaded_song(void)
{
    akao_play_song((AkaoHeader*)g_overlay_song_buffer);
    akao_set_song_volume(0, AKAO_VOLUME_MAX);
}
