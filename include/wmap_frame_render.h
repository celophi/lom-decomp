#ifndef WMAP_FRAME_RENDER_H
#define WMAP_FRAME_RENDER_H

#include "common.h"
#include "sdk/libgpu.h"

#define WMAP_OT_COUNT 179
#define WMAP_PACKET_LIMIT 32000

/** @brief Drawing environments, depth buckets, and polygon storage for one map frame. */
typedef struct
{
    DRAWENV draw_env;
    DISPENV disp_env;
    u_long ordering_table[WMAP_OT_COUNT];
    u8* packet_cursor;
    union
    {
        POLY_FT4 flat[26 * 26];
        POLY_FT4 rows[26][26];
    } tiles;
    POLY_F4 fade[184];
    u8 tail[32];
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

#endif
