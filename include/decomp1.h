#ifndef _DECOMP1_H
#define _DECOMP1_H

#include "common.h"
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

extern s32 D_80042FB8;
extern u8 D_80046FE0;
extern s16 D_80042FBC;
extern s16 D_80046FDC;
extern s16 D_80042FBE;
extern s16 D_80042FC0;
extern s16 D_80042FC2;

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

// prototypes
void func_800157DC(void);
void func_800158E0(void);

#endif