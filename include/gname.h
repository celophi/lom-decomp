#ifndef _GNAME_H
#define _GNAME_H

#include "render_context.h"

/**
 * @brief Initialize name-entry resources and per-run state.
 *
 * Called once by @c gname_run before its per-frame loop begins.
 */
void gname_init(void);

/**
 * @brief Per-frame tick: render the frame, advance the frame counter, and
 *        update the overlay state machine.
 *
 * Called each frame by @c gname_run while the name-entry UI is active.
 *
 * @param ctx Render context for this frame's primitives.
 */
void gname_tick(RenderContext* ctx);

#endif
