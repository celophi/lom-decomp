#ifndef _MOVIE_H
#define _MOVIE_H

#include "common.h"

/* Public entry point: the only symbol exported from MOVIE.BIN to the rest of
 * the game. Everything else (MovieState, ring buffers, AKAO/MDEC plumbing,
 * end-state sentinels, skip-gate masks, ...) is private to movie.c. */
void movie_play(s32 movie_index);

#endif
