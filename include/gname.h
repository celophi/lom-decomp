#ifndef _GNAME_H
#define _GNAME_H

#include "render_context.h"

/**
 * @brief Overlay boot entry: upload glyph TIM, init engine state, seed run state.
 *
 * Called once by the FIELD overlay (which pages GNAME.BIN into 0x80140000)
 * before the per-frame loop begins.
 */
void gname_init(void);

/**
 * @brief Per-frame tick: render the frame, advance the frame counter, and
 *        update the overlay state machine.
 *
 * Driven each frame by the FIELD overlay while the name-entry UI is active.
 *
 * @param ctx Render context for this frame's primitives.
 */
void gname_tick(RenderContext* ctx);

#endif
