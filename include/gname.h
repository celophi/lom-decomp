#ifndef _GNAME_H
#define _GNAME_H

#include "render_context.h"

/** @brief Initialize name-entry resources and session state. */
void gname_init(void);

/**
 * @brief Render and update one name-entry frame.
 * @param render_ctx Frame render context.
 */
void gname_tick(RenderContext* render_ctx);

#endif
