#ifndef ZUKAN_H
#define ZUKAN_H

#include "common.h"
#include "common/render_context.h"

/**
 * @brief Initialize the encyclopedia overlay state.
 * @param work_buffer Start of the overlay work buffer.
 * @param category Encyclopedia category to display.
 * @return First address after the encyclopedia's reserved work area.
 */
u8* zukan_initialize_state(u8* work_buffer, s32 category);

/**
 * @brief Build and render one encyclopedia frame.
 * @param render_ctx Render context for the current frame.
 */
void zukan_update_frame(RenderContext* render_ctx);

/** @brief Load the encyclopedia UI resource archive. */
void zukan_load_ui_resource(void);

#endif
