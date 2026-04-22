#ifndef _DECOMP7_H
#define _DECOMP7_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

typedef struct obj_struct
{
  u8 pad0[0x40B8];
  u32 unk40B8;
  u8 pad1[0x7CC4 - 0x40BC];
} ObjStruct;

extern u32 *FUN_80015c28(void);
extern void FUN_80022aa8(void);
extern void FUN_80022ac8(void);
extern void func_80015D6C(s32);
extern void func_80015F88(s32);
extern void func_80067EB4(s32, s32, s32, s32);
extern void func_8009AFE0(s32, s32, u32, s32, s32, s32);
extern void func_800A379C(void);
extern s32 D_8003EC90;
extern s32 D_80042FCC;
extern u32 D_8003EC88;
extern s32 D_80042FC4;
extern s32 D_8003EC94;
extern s32 D_80046FD8;
extern s32 D_8010D018;
extern s32 D_801158A4;
extern void *D_800473F4;
extern void *D_800473EC;
extern s32 D_80035248;
extern s32 D_800473E8;

#endif