#ifndef _TITLE_H
#define _TITLE_H

#include "common.h"
#include "pad.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

typedef struct
{
    u16 unk0;
    u16 unk2;
    u32 unk4;
    u32 unk8;
    u32 unkC;
} S_801ED480;

/**
 * Current screen-fade colour. RGB only; the fade-target struct carries the
 * step counter. Counterpart to g_fadeCurrent in the CHECKPS overlay.
 */
typedef struct
{
    s32 red;
    s32 green;
    s32 blue;
} FadeCurrent;

/**
 * Target colour and remaining frames for the screen-fade interpolation.
 * Counterpart to g_fadeTarget in the CHECKPS overlay (which uses a single
 * FadeColor struct for both; here current/target have distinct sizes
 * because g_fadeCurrent is followed immediately by another global).
 */
typedef struct
{
    s32 red;
    s32 green;
    s32 blue;
    s32 steps;
} FadeTarget;

typedef struct
{
    char _pad[0x40];
    u_long otag_buffer[0x1000]; /* 0x0040 */
    DISPENV disp_env;           /* 0x4040 */
    DRAWENV draw_env;           /* 0x4054 */
    char _pad2[8];              /* 0x40B0 */
    u_long prim_buffer[0x1000]; /* 0x40B8 */
    u_long* next_prim_ptr;      /* 0x80B8 */
    char _pad3[0x3C10];         /* 0x80BC */

    char _pad4[0x40];
    u_long otag_buffer2[0x1000];

} MenuContext; /* 0xBCCC total */

extern u8 D_80042FD8[];
extern s32 D_80042FB4;
extern u8 g_titleSelectedItem;
extern s32 D_8003EC9C;
extern s32 g_titleMenuExitState;
extern u32 g_previousGameState;
extern s32 D_80102668;
extern unsigned char D_8003ECA0;
extern s32 g_titleIdleCountdown;
extern s32 g_debouncedInput;
extern u8 g_titleMenuItemFlags[];
extern u8 g_titleVisibleItemRank;
extern u8 g_titleAnimFrame;
extern u8 D_8007FD2C[];

/**
 * Timer used to implement input repeating (auto-repeat).
 * Controls the delay before a held button begins triggering actions rapidly.
 * Same semantics as the identically-named symbol in the CHECKPS overlay
 * (separate copy, different address).
 */
extern s32 g_inputRepeatTimer;
extern s32 g_lastInputState;
extern u32 D_800522E8[3];
extern u8 D_801ED600[];
extern s32 g_slotSlideX;
extern s32 g_slotSlideY;
extern s32 g_slotSelectedIndex;
extern s32 g_slotSlideFrames;
extern s32 g_slotHighlightX;
extern s32 g_slotHighlightTargetX;
extern s32 g_slotSlideXLerped;
extern s32 g_slotSlideYLerped;
extern s32 g_slotHighlightFrames;
extern u8 D_80043618[0x40];
extern u8 D_800F9BC4[];
extern u8 D_800F9AED;
extern u8 D_800F993C[0x200];
extern u8 D_800F97FC[];
extern u8 D_800F98AC[];
extern u8 D_800F98F4[];
extern s32 D_800F9E84;
extern s16 D_8003EC90;
extern s32 D_800FEF40;
extern s16 D_80046FDE;
extern s32 D_80042FC4;
extern s32 D_801023F0;
extern s32 D_801021A0;
extern s32 g_gameDataBasePtr;

extern FadeCurrent g_fadeCurrent;
extern FadeTarget g_fadeTarget;

extern void FUN_8002279c(undefined4 param_1, u_int param_2);
extern void func_80022040(s32 arg0);
extern void func_8002216C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

#endif
