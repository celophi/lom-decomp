#ifndef _MAIN_H
#define _MAIN_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libapi.h"
#include "psyq/libetc.h"

extern u32 g_gameState;
extern u32 g_previousGameState;
extern u32 g_overlayLoadAddress;
extern u32 g_gameDataBasePtr;

extern u8* D_800351A0;
extern u32 D_8003EC88;
extern s32 D_8003EC8C;
extern u16 D_8003EC90;
extern s32 D_8003EC94;
extern s32 D_8003EC98;
extern s32 D_8003EC9C;
extern s32 D_80042FC4;
extern s32 D_80042FCC;
extern s32 D_80042FD0;
extern s32 D_80046FD8;
extern u16 D_80046FDE;
extern s32 D_800473E0;

/*
 * Shared input / frame / script state.
 *
 * These globals live in the main executable's .bss (below the 0x80140000
 * overlay slot) and are read/written by multiple overlays (menu, gname,
 * gover, ...). They are declared here rather than per-overlay so the
 * overlays share a single definition.
 */

/** @brief Global frame counter, advanced once per rendered frame. */
extern s32 g_frame_counter;

/** @brief Base of the primitive-rect scratch buffer (stride 0x4A0 per record). */
extern u8 g_prim_rect_buf[];

/**
 * @brief Pointer to the controller/pad context object.
 * @note Only fields at +0x840 (u8) and +0x858 (u32, bit 0x80) are referenced
 *       so far; full layout TODO.
 */
extern void* g_pad_ctx;

/** @brief Number of times the active script repeats on reaching its terminator. */
extern s32 g_script_repeat_count;

/** @brief Active script id; selects a row of @c g_script_table (0 = none). */
extern s32 g_active_script;

/** @brief Current frame's debounced pad button bitmask. */
extern s32 g_pad_input;

/** @brief Extra button bits OR'd into @c g_pad_input when the pad context requests it. */
extern s32 g_pad_input_inject;

typedef struct {
    u8 u_0x0[24];
    s32 u_0x18;
    s16 u_0x1C;
    s8 u_0x1E;
    u8 u_0x1F;
    u32 u_0x20;
    u16 u_0x24;
    u8 u_0x26;
    u8 u_0x27;
    u32 u_0x28;
    u8 u_0x2A[1504];
    u8 u_608;
} tempU;

void __main(void);
void _bu_init(void);
s32 func_80015C48(void);
u32 func_8004FC74(void);
u32 func_8004FC8C(u32);
void func_80051FBC(u32);
void func_800A3534(void);
u32 run_overlay(u32, u32, u32, s32, s32, u32, s32);
s32 func_801400C4(void);
void srand(u_int param_1);

#endif