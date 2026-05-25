#ifndef _MOVIE_H
#define _MOVIE_H

#include "common.h"

/**
 * @brief Play one of the MDEC cinematics.
 *
 * Sole public entry point of the MOVIE.BIN overlay. All other movie_*
 * functions, the MovieState block, ring buffers, AKAO/MDEC plumbing,
 * end-state sentinels, and skip-gate masks are private to movie.c.
 *
 * @param movie_index Cinematic to play (0..4).
 */
void movie_play(s32 movie_index);

#endif
