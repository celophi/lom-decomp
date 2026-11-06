#ifndef OVERLAYS_GOSUB_H
#define OVERLAYS_GOSUB_H

#include "common.h"

/**
 * @file gosub.h
 * @brief GOSUB's entry points, called by FIELD once the overlay is loaded.
 */

struct GosubRenderContext;

/**
 * @brief Open a sequence of GOSUB selection screens.
 * @param unused Work buffer FIELD passes; not read.
 * @param screen_sequence Screen ids, ending with a terminator.
 */
void gosub_open_screen_sequence(void* unused, s32* screen_sequence);

/**
 * @brief Update and draw one GOSUB frame.
 * @param render_context Render context of the frame (FIELD's render half).
 * @return Nonzero once GOSUB has finished.
 */
s32 gosub_update_frame(struct GosubRenderContext* render_context);

#endif
