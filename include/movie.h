#ifndef _MOVIE_H
#define _MOVIE_H

#include "common.h"

/**
 * @brief Cinematic indices selected by the main game-state dispatcher.
 *
 * GAME_STATE_ATTRACT_2 plays its three stream segments consecutively.
 */
typedef enum
{
    MOVIE_INDEX_INTRO = 0,
    MOVIE_INDEX_ATTRACT_1 = 1,
    MOVIE_INDEX_ATTRACT_2_PART_1 = 2,
    MOVIE_INDEX_ATTRACT_2_PART_2 = 3,
    MOVIE_INDEX_ATTRACT_2_PART_3 = 4
} MovieIndex;

/**
 * @brief Play one of the MDEC cinematics.
 *
 * @param movie_index Cinematic to play, a @ref MovieIndex.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/gkEWm
 */
void movie_play(u16 movie_index);

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
