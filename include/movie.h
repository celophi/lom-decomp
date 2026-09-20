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

void movie_service_video_ops(void);

#endif
