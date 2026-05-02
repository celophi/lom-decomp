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

typedef struct {
    u32 unk0;
    u32 unk4;
    u32 unk8;
} D_80102658_t;

typedef struct {
    u32 unk0;
    u32 unk4;
    u32 unk8;
    u32 unkC;
} D_80102648_t;

extern u8 D_80042FD8[];
extern s32 D_80042FB4;
extern u8  D_80102692;
extern s32 D_8003EC9C;
extern s32 D_80102640;
extern u32 g_previousGameState;
extern s32 D_80102668;
extern unsigned char D_8003ECA0;

extern D_80102658_t D_80102658;
extern D_80102648_t D_80102648;

extern void FUN_8002279c(undefined4 param_1, u_int param_2);
extern void func_80022040(s32 arg0);
extern void func_8002216C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

#endif