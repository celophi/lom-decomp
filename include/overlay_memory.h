#ifndef _OVERLAY_MEMORY_H
#define _OVERLAY_MEMORY_H

#include "common.h"

extern s32 g_overlay_load_base;
extern u32 g_field_render_buffers;
extern u32 g_world_map_overlay_end;
extern u32 g_title_menu_buffers;

s32* get_overlay_load_base(void);
u32* get_field_render_buffers(void);
u32* get_world_map_overlay_end(void);
u32* get_title_menu_buffers(void);

#endif
