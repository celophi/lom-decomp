#ifndef WSEL_H
#define WSEL_H

#include "common.h"
#include <libgte.h>
#include <libgpu.h>

#include "main/display.h"

/** @brief One of the two double-buffered display buffers WSEL draws into. */
typedef struct
{
    u8 unknown_0x0000[0x40];
    u_long ot[FRAME_ORDERING_TABLE_LENGTH];
    DISPENV disp_env;
    DRAWENV draw_env;
    RECT clear_rect;
    u_long packets[FRAME_PRIMITIVE_BUFFER_SIZE / sizeof(u_long)];
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
