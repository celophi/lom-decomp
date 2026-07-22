#ifndef _SCREEN_TRANSITION_H
#define _SCREEN_TRANSITION_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

typedef struct {
    u_long ot[4];
    u32 packetBuffer[64];
    DISPENV dispenv;
    DRAWENV drawenv;
} GfxBuffer;

typedef struct {
    GfxBuffer buf;
    RECT frame;
} FrameBuffer;

typedef struct {
    RECT frameA;
    GfxBuffer bufB;
    RECT frameB;
} FrameBufferOverlap;

/**
 * This allows for addressing in the middle of the double frame buffer.
 */
typedef union {
    FrameBuffer fb;
    FrameBufferOverlap overlap;
} FrameBufferUnion;

/**
 * These are the two frame buffers used for double buffering. 
 * The game likely uses one buffer for rendering while the other is being displayed, 
 * and then swaps them each frame to achieve smooth graphics without tearing.
 */
extern FrameBufferUnion g_GfxDoubleBuffer;

/**
 * This points to the first frame (frameA) in the double buffer.
 */
extern FrameBufferUnion g_GfxPrimaryFrame;

void update_controllers(void);
void reset_controller_vsync_state(void);
void GFX_Transition(s32 skipScreenClear);

#endif
