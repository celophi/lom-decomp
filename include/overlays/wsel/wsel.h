#ifndef WSEL_H
#define WSEL_H

#include "common.h"
#include <libgte.h>
#include <libgpu.h>

#include "main/display.h"

/** @brief One of the two double-buffered display buffers WSEL draws into. */
typedef struct
{
    ScreenFrame screen;
    u8 unknown_0x80bc[0x10];
} WselRenderBuffer;

/**
 * @brief Run world selection until the player chooses an exit state.
 * @param buffers The two display buffers, back to back.
 * @return Next top-level game state.
 */
s32 wsel_main(WselRenderBuffer* buffers);

#endif
