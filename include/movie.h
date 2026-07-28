#ifndef _MOVIE_H
#define _MOVIE_H

#include "common.h"

/**
 * @brief Play one of the MDEC cinematics.
 *
 * Sole public entry point of the MOVIE.BIN overlay. The remaining global
 * function symbols mirror the original overlay but are intentionally omitted
 * from this public header.
 *
 * @param movie_index Cinematic to play (0..4).
 *
 * @see https://decomp.me/scratch/gkEWm (100%)
 */
void movie_play(s32 movie_index);

#endif
