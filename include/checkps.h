#ifndef _CHECKPS_H
#define _CHECKPS_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/memory.h"

extern s32 D_8005D060;
extern u32 D_80052428;
extern s32 D_80042FB0;
extern s32 D_80061088;
extern u8 D_8005D088;
extern s32 D_8005D068[4];
extern s32 D_8005D078[3];

void func_80050080(void);
void func_8004FEE8(int param_1);
void func_8004FD68(int param_1);

#endif