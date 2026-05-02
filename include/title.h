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

extern u8 D_80042FD8[];
extern s32 D_80042FB4;
extern u8  D_80102692;
extern s32 D_8003EC9C;
extern s32 D_80102640;
extern u32 g_previousGameState;
extern s32 D_80102668;
extern unsigned char D_8003ECA0;

#endif