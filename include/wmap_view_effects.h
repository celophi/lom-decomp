#ifndef WMAP_VIEW_EFFECTS_H
#define WMAP_VIEW_EFFECTS_H

#include "common.h"

/** @brief g_wmap_view_mode values. */
#define WMAP_VIEW_MODE_HIDDEN (-1)
#define WMAP_VIEW_MODE_MAP 0
#define WMAP_VIEW_MODE_SPIRITS 1
#define WMAP_VIEW_MODE_ZOOM_IN 2
#define WMAP_VIEW_MODE_ZOOM_OUT 3

s32 wmap_run_view_sequence(s32 initialize);
void wmap_reset_view_sequence(void);
void wmap_advance_view_sequence(void);
void func_800654EC(void);
void wmap_init_map_packets(void);
s32 wmap_update_map_view(s32 initialize);
void wmap_update_view_zoom(void);
s32 wmap_start_map_tint(s32 color);
s32 wmap_update_burst_particles(void);
void wmap_init_burst_particles(void);
void wmap_begin_cell_focus(void);
void wmap_project_focus_position(void);

#endif
