#ifndef _OVERLAY_MEMORY_H
#define _OVERLAY_MEMORY_H

#include "common.h"

/** @brief Destination used for the primary overlay image. */
extern void* g_overlay_load_address;

s32* get_overlay_load_base(void);
void* get_field_render_buffers(void);
u32* get_world_map_overlay_end(void);
void* get_title_menu_buffers(void);

#endif
