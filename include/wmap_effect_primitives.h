#ifndef WMAP_EFFECT_PRIMITIVES_H
#define WMAP_EFFECT_PRIMITIVES_H

#include "common.h"
#include "sdk/libgte.h"

void func_8006A2FC(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4_value, s32 arg5_value, u32 arg6, void* arg7);
void func_8006A9C4(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 arg8, u16 arg9, s32 arg10, s32 arg11);
void func_8006ADD0(VECTOR* translation, SVECTOR* rotation);
void func_8006AEE0(void);
void func_8006AFAC(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, u16 arg7);
void func_8006B328(s32 arg0, s32 arg1, s32 arg2, s16 arg3, s32 arg4, s32 arg5, u16 arg6, s32 arg7, s32 arg8, s32 arg9, s32 arg10, s32 arg11, s32 arg12,
                   u16 arg13, u16 arg14, u16 arg15, s32 arg16);
void func_8006B6EC(s32 first, s32 end, s32 frame, s32 z_step, s32 depth);
void func_8006B998(s32 first, s32 end, void* point_data, s32 frame, s32 depth);
void func_8006BC44(s32 arg0, s32 arg1, void* arg2, s32 arg3);
s32 func_8006C0EC(void);
void func_8006C448(void* arg0);

#endif
