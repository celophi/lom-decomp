#include "common/overlay_audio.h"
#include "main/audio/akao_cmd.h"

/**
 * @brief Stop the background music.
 * @see decomp.me (100%, TITLE copy) https://decomp.me/scratch/1cta3
 */
void stop_song(void)
{
    akao_stop_song(0);
}
