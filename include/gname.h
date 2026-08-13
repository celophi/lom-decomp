#ifndef _GNAME_H
#define _GNAME_H

#include "render_context.h"

/**
 * @brief Run the name-entry UI until the user confirms or cancels.
 * @param render_buffers Double-buffered frame render contexts.
 * @param initial_name Name restored when the session resets.
 * @param active_name Name buffer edited by the UI.
 * @param source_mode Random-name source selector.
 * @param history_index History-list entry selector.
 * @param custom_name Custom random-name source.
 * @param allow_empty_cancel Whether cancel may exit with an empty name.
 * @return Final cancel or confirmation result.
 */
s32 gname_run(
    RenderContext* render_buffers,
    const u8* initial_name,
    u8* active_name,
    s32 source_mode,
    s32 history_index,
    const u8* custom_name,
    s32 allow_empty_cancel);

/** @brief Initialize name-entry resources and session state. */
void gname_init(void);

/**
 * @brief Render and update one name-entry frame.
 * @param render_ctx Frame render context.
 */
void gname_tick(RenderContext* render_ctx);

#endif
