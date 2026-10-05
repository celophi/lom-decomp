#ifndef WMAP_FRAME_RENDER_H
#define WMAP_FRAME_RENDER_H

#include "common.h"
#include <libgpu.h>

#define WMAP_OT_COUNT 179
#define WMAP_PACKET_LIMIT 32000
/** @brief Map tiles per grid row and column. */
#define WMAP_MAP_TILES 26
/** @brief Drop-shadow polygons under the map edges. */
#define WMAP_SHADOW_POLYS 184

/** @brief Drawing environments, depth buckets, and polygon storage for one map frame. */
typedef struct
{
    DRAWENV draw_env;
    DISPENV disp_env;
    u_long ordering_table[WMAP_OT_COUNT];
    u8* packet_cursor;
    union
    {
        POLY_FT4 flat[WMAP_MAP_TILES * WMAP_MAP_TILES];
        POLY_FT4 rows[WMAP_MAP_TILES][WMAP_MAP_TILES];
    } tiles;
    POLY_F4 shadow[WMAP_SHADOW_POLYS];
    /** @brief Off-screen triangle that selects the shadow texture page and blend mode. */
    POLY_FT3 tpage_select;
} WmapFrame;

/** @brief Double-buffered world-map drawing state. */
extern WmapFrame g_wmap_frames[2];
/** @brief Frame receiving the current ordering table and GPU packets. */
extern WmapFrame* g_wmap_current_frame;
/** @brief Bytes committed to the current frame's dynamic packet buffer. */
extern s32 g_wmap_packet_bytes;

void func_800641DC(void);
void func_8006454C(void);
void func_80064AF8(void);
void func_80064BF8(void);
s32 func_80064D64(s32 initialize);

/** @brief Packed RGB color used to shade the captured exit panels. */
extern u32 g_wmap_exit_panel_color;
/** @brief Dynamic GPU packet storage for the first map frame. */
extern u8 g_wmap_packet_buffer_0[];
/** @brief Dynamic GPU packet storage for the second map frame. */
extern u8 g_wmap_packet_buffer_1[];
/** @brief Current shade of the screen-covering fade quad. */
extern s32 g_wmap_overlay_fade_level;
/** @brief Shade toward which the screen-covering fade moves. */
extern s32 g_wmap_overlay_fade_target;
/** @brief Shade of the entry overlay, reduced by eight each frame. */
extern s32 g_wmap_entry_fade_level;
/** @brief Ordering-table bucket used by the screen-covering fade. */
extern s32 g_wmap_overlay_fade_depth;

#endif
