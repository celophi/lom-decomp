#ifndef WSEL_H
#define WSEL_H

#include "common.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

#define WSEL_OT_LENGTH 0x1000
#define WSEL_PACKET_WORDS 0x1000

/** @brief One of the two double-buffered display buffers WSEL draws into. */
typedef struct
{
    u8 unknown_0x0000[0x40];
    u_long ot[WSEL_OT_LENGTH];
    DISPENV disp_env;
    DRAWENV draw_env;
    RECT clear_rect;
    u_long packets[WSEL_PACKET_WORDS];
    u_long* prim_cursor;
    u8 unknown_0x80bc[0x10];
} WselRenderBuffer;

/**
 * @brief Run world selection until the player chooses an exit state.
 * @param buffers The two display buffers, back to back.
 * @return Next top-level game state.
 */
s32 wsel_main(WselRenderBuffer* buffers);

#endif
