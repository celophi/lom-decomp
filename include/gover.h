#ifndef _GOVER_H
#define _GOVER_H

#include "common.h"
#include "gpu_packet.h"
#include "main.h"

/**
 * @brief Loads and presents the Game Over screen.
 *
 * Blocks while the screen fades in, waits for dismissal, and fades out.
 *
 * @param cd_load_address      Writable RAM staging address for the image resource.
 * @param image_resource_index Game Over image resource index.
 * @param music_resource_index Music resource index, or -1 to skip music.
 * @param audio_clip_index     Audio clip index, -1 to skip playback, or -2 to
 *                             reuse the currently staged audio.
 * @return void No return value.
 * @see decomp.me (100%) https://decomp.me/scratch/1qYnn
 */
void gover_show_screen(
    s32 cd_load_address, s32 image_resource_index, s32 music_resource_index, s32 audio_clip_index);

#endif
