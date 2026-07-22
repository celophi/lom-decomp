#ifndef _DECOMP2_H
#define _DECOMP2_H

#include "common.h"

extern u32 g_field_render_buffers;
extern u32 g_world_map_overlay_end;
extern u32 g_title_menu_buffers;

u32* get_field_render_buffers(void);
u32* get_world_map_overlay_end(void);
u32* get_title_menu_buffers(void);

#endif
