#ifndef _TITLE_H
#define _TITLE_H

#include "common.h"
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
extern s32 D_80102698;
extern s32 D_80102694;
extern u32 D_800522E8[3];
extern  u8 D_801ED600;

extern D_80102658_t D_80102658;
extern D_80102648_t D_80102648;

extern void FUN_8002279c(undefined4 param_1, u_int param_2);
extern void func_80022040(s32 arg0);
extern void func_8002216C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

#endif