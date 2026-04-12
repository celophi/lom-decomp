#ifndef _DECOMP1_H
#define _DECOMP1_H

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
void FUN_80011638(int param_1);
undefined* FUN_80015c18(void);
undefined* FUN_80015c28(void);
undefined* FUN_80015c38(void);
int FUN_80015c58(void);
undefined4 FUN_80021fbc(void);
void FUN_8002279c(undefined4 param_1, u_int param_2);
void FUN_80022aa8(void);
void FUN_80022ac8(void);
undefined4 FUN_8004fd14(undefined4 param_1);
u32 FUN_80060814();
u32 FUN_80140018(u32 param_1);
void GFX_Transition(s32 skipScreenClear);
void InitVSyncController(void);
void InitializeControllers(undefined1 controllerMode);

#endif