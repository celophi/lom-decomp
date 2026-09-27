#ifndef _MOVIE_H
#define _MOVIE_H

#include "common.h"

/**
 * @brief Play one of the MDEC cinematics.
 *
 * @param movie_index Cinematic to play (0..4).
 *
 * @see https://decomp.me/scratch/gkEWm (100%)
 */
void movie_play(s32 movie_index);

/**
 * @brief Initialize a cinematic or a movie drawn into the FIELD scene.
 * @param resource_index CD resource containing the movie.
 * @param flags GPU mode in the low seven bits; bit 7 enables streamed audio.
 * @param total_frames Last frame to play.
 * @param init_buffer_idx Initial display buffer for a FIELD movie.
 */
void movie_init(s32 resource_index, s32 flags, s32 total_frames, s32 init_buffer_idx);
/** @brief Advance video decoding and streamed audio playback. */
void movie_update(void);
/** @brief Submit pending MDEC output and GPU uploads when space is available. */
void movie_service_video_ops(void);

#endif
