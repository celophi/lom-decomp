#ifndef _MAIN_H
#define _MAIN_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libapi.h"

extern u32 D_800102AC;
extern u8* D_800351A0;
extern u32 D_8003522C;
extern u32 D_8003EC88;
extern s32 D_8003EC8C;
extern u16 D_8003EC90;
extern s32 D_8003EC94;
extern s32 D_8003EC98;
extern s32 D_8003EC9C;
extern u32 D_80042FB0;
extern s32 D_80042FC4;
extern s32 D_80042FCC;
extern s32 D_80042FD0;
extern u32 D_800435C8;
extern s32 D_80046FD8;
extern u16 D_80046FDE;
extern s32 D_800473E0;

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
u32 func_80140004(u32, u32, u32, s32, s32, u32, s32);
s32 func_801400C4(void);
void srand(u_int param_1);

#endif