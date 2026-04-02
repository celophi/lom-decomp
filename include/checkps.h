#ifndef _CHECKPS_H
#define _CHECKPS_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/memory.h"

typedef struct {
    u8  deviceState;     // 0x00 - status / mode flag
    u8  _pad1;
    u16 buttonData;      // 0x02 - raw 16-bit input (pre-remap)

    u8  _pad2[0x28];     // 0x04–0x2B - unused here

    s16 axisX;           // 0x2C - signed axis (negative/positive thresholded)
    s16 axisY;           // 0x2E - signed axis (negative/positive thresholded)
} SCDRegs;

extern s32 g_previousGameState;

extern s32 D_8005D060;
extern u32 D_80052428;
extern s32 D_80061088;
extern u8 D_8005D088;
extern s32 D_8005D068[4];
extern s32 D_8005D078[3];
extern s32 D_800610A0;
extern s32 D_80061094;
extern s32 D_80061098;
extern u8  D_8005B744[];
extern u8 D_801ED600;
extern s32 D_80061090;
extern s32 D_800610A4;
extern s32 D_800610A8;

void func_80050080(void);
void func_8004FEE8(int param_1);
void func_8004FD68(int param_1);

#endif