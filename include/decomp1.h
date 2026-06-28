#ifndef _DECOMP1_H
#define _DECOMP1_H

#include "common.h"
#include "akao.h"
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

/** @brief Handle returned by akao_play_song(), used for subsequent AKAO commands. */
extern s32 g_current_song_handle;
/** @brief Destination buffer for CD-loaded music/song sequence data. */
extern u8 g_music_data_buffer;
/** @brief AKAO song command parameter 0 (bitmask, always 0x8001 from field overlay). */
extern s16 g_akao_song_cmd_arg0;
/** @brief Alternate copy of arg0 when negative. */
extern s16 g_akao_song_cmd_neg_arg0;
/** @brief AKAO song command parameter 1 (often 0x320 / 800). */
extern s16 g_akao_song_cmd_arg1;
/** @brief AKAO song command parameter 2 (small int, often character slot index). */
extern s16 g_akao_song_cmd_arg2;
/** @brief AKAO song command parameter 3 (often 0 or entity index). */
extern s16 g_akao_song_cmd_arg3;

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
void load_and_play_song(s32 song_index);
undefined* FUN_80015c18(void);
undefined* FUN_80015c38(void);
int FUN_80015c58(void);
undefined4 FUN_80021fbc(void);
void akao_song_cmd_12c(void);
void akao_set_song_params(int flags, s16 duration, s16 field_id, s16 sub_id);
void akao_cmd_c0(undefined4 param_1, u_int param_2);
void akao_cmd_f0(void);
void akao_cmd_f1(void);
s32 RunCheckPS(s32 baseAddress);
u32 FUN_80060814();
u32 movie_play(u32 param_1);
void GFX_Transition(s32 skipScreenClear);
void InitVSyncController(void);
void InitializeControllers(undefined1 controllerMode);

#endif