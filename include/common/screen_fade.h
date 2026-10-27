#ifndef SCREEN_FADE_H
#define SCREEN_FADE_H

#include "common.h"
#include <libgpu.h>

#include "main/display.h"

/**
 * @file screen_fade.h
 * @brief Full-screen colour fade compiled into GOLEM and ZUKAN.
 *
 * The current fade level moves toward the target by an equal step each frame.
 * Levels above FADE_NEUTRAL brighten the screen additively and levels below it
 * darken it subtractively. Each overlay includes these functions from
 * src/common/screen_fade/<function>.inc.c at the point where they sit in its
 * binary, and defines the two globals in its own data.
 */

/** @brief Steps of the fades in the golem editor and the encyclopedia. */
#define SCREEN_FADE_STEPS 6

/** @brief Fade level being approached and the steps left to reach it. */
extern ScreenFade g_screen_fade_target;

/** @brief Fade level drawn this frame. */
extern ScreenFade g_screen_fade_current;

/**
 * @brief Set the target level of each channel and the number of steps to reach it.
 * @param red Target red level.
 * @param green Target green level.
 * @param blue Target blue level.
 * @param steps Number of frames the fade takes.
 */
void set_screen_fade_target(s16 red, s16 green, s16 blue, s16 steps);

/**
 * @brief Step the current fade level toward the target, then emit the full-screen
 *        fade tile and its blend-mode packet unless the level is neutral.
 * @param tile Next free position in the packet buffer.
 * @param ordering_table Ordering-table entry receiving the fade.
 * @return First free primitive-buffer address after the emitted packets.
 */
void* render_screen_fade(TILE* tile, u_long* ordering_table);

#endif
