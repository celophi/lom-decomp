#ifndef _OVERLAY_MEMORY_H
#define _OVERLAY_MEMORY_H

#include "common.h"

/** @brief Linker symbol at the first byte past the main executable image. */
extern u8 g_overlay_load_base;
/** @brief Destination used for the primary overlay image. */
extern void* const g_overlay_load_address;

void* get_overlay_load_base(void);
void* get_field_render_buffers(void);
void* get_world_map_overlay_end(void);
void* get_title_menu_buffers(void);

#endif
