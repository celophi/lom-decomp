#ifndef WMAP_VIEW_EFFECTS_H
#define WMAP_VIEW_EFFECTS_H

#include "common.h"
#include <libgte.h>

#include "wmap_sprite_render.h"

/** @brief g_wmap_view_mode values. */
#define WMAP_VIEW_MODE_HIDDEN (-1)
#define WMAP_VIEW_MODE_MAP 0
#define WMAP_VIEW_MODE_SPIRITS 1
#define WMAP_VIEW_MODE_ZOOM_IN 2
#define WMAP_VIEW_MODE_ZOOM_OUT 3

/** @brief g_wmap_view_scroll_mode values: idle, cursor scroll stopped at the map edges, scripted scroll. */
#define WMAP_SCROLL_NONE 0
#define WMAP_SCROLL_CURSOR 1
#define WMAP_SCROLL_SCRIPTED 2

/** @brief View distance of one map cell; one cursor move scrolls this far. */
#define WMAP_VIEW_SCROLL_STEP 48

/**
 * @brief Start a scripted scroll that brings a map cell to the view origin.
 * @param cell_x Map cell x.
 * @param cell_y Map cell y.
 */
#define WMAP_START_SCROLL_TO_CELL(cell_x, cell_y)                                                                                                              \
    (g_wmap_view_scroll_mode = WMAP_SCROLL_SCRIPTED, g_wmap_scroll_remaining_x = (((cell_x) - 1) * WMAP_VIEW_SCROLL_STEP) - g_wmap_view.x,                     \
     g_wmap_scroll_remaining_y = (((cell_y) - 1) * WMAP_VIEW_SCROLL_STEP) - g_wmap_view.y)

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

/** @brief Screen position of the focused land, projected by wmap_project_focus_position. */
extern WmapScreenPosition g_wmap_focus_screen_position;

/** @brief Default screen position of the focused land. */
#define WMAP_FOCUS_DEFAULT_X 164
#define WMAP_FOCUS_DEFAULT_Y 105

/** @brief Screen position of the focused land while a land event plays. */
#define WMAP_EVENT_FOCUS_X 148
#define WMAP_EVENT_FOCUS_Y 49

/** @brief Rotation added to the camera while projecting lands and effects. */
extern SVECTOR g_wmap_effect_camera_rotation_offset;

/** @brief Translation added to the camera while projecting lands and effects. */
extern VECTOR g_wmap_effect_camera_translation_offset;

/** @brief Camera pose and view scrolling. */
extern VECTOR g_wmap_camera_translation;
extern SVECTOR g_wmap_camera_rotation;
extern s32 g_wmap_view_mode;
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_view_scroll_enabled;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;

#endif
