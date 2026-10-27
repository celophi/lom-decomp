#ifndef _ZUKAN_H
#define _ZUKAN_H

#include "common/render_context.h"

/**
 * @brief Run the encyclopedia screen until the player leaves it.
 * @param render_buffers Storage for the two render contexts, followed by the encyclopedia work area.
 * @param category Encyclopedia category to display.
 */
void zukan_run(RenderContext* render_buffers, s32 category);

#endif
