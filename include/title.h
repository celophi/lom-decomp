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

typedef struct
{
    u32 unk0;
    u32 unk4;
    u32 unk8;
} D_80102658_t;

typedef struct
{
    u32 unk0;
    u32 unk4;
    u32 unk8;
    u32 unkC;
} D_80102648_t;

typedef struct
{
    char pad0[0x40];
    u32 unk40;
    char pad1[0x80B8 - 0x44];
    void* unk80B8;
} ArgStruct;

typedef struct
{
    unsigned char pad[0xB8]; // padding to offset 0xB8
    s32 unk80B8;             // desired field at offset 0xB8
} InnerStruct;

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
extern u8 D_80102692;
extern s32 D_8003EC9C;
extern s32 D_80102640;
extern u32 g_previousGameState;
extern s32 D_80102668;
extern unsigned char D_8003ECA0;
extern s32 D_801026A0;
extern s32 D_8010269C;
extern u8 D_80102670[];
extern u8 D_80102690;
extern u8 D_80102691;
extern u8 D_8007FD2C[];

/**
 * Timer used to implement input repeating (auto-repeat).
 * Controls the delay before a held button begins triggering actions rapidly.
 */
extern s32 g_inputRepeatTimer;
extern s32 D_80102694;
extern u32 D_800522E8[3];
extern u8 D_801ED600[];
extern s32 D_801026A8;
extern s32 D_801026AC;
extern s32 D_801026B0;
extern s32 D_801026B4;
extern s32 D_801026B8;
extern s32 D_801026BC;
extern s32 D_801026C0;
extern s32 D_801026C4;
extern s32 D_801026C8;
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

extern D_80102658_t D_80102658;
extern D_80102648_t D_80102648;

extern void FUN_8002279c(undefined4 param_1, u_int param_2);
extern void func_80022040(s32 arg0);
extern void func_8002216C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

#endif