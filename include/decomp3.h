#ifndef _DECOMP3_H
#define _DECOMP3_H

#include "common.h"

extern s32 D_8004D430[];
extern s32 D_8004D400;
extern u8 D_8004B430[];

s32 FUN_80021fbc(void);
s32 func_80021FDC(void);
s32 akao_register_bank(s32 bankBase);
void func_80022040(s32 arg0);
void func_80022068(s32 arg0);
void func_80022090(void);
void func_800220B0(s32 arg0, s32 arg1);
s32 func_800220E4(s32 arg0, s32 arg1);
void func_8002213C(s32 arg0, s32 arg1);
void func_8002216C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
s32 func_800221BC(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_80022240(s32 arg0, s32 arg1);
void func_8002227C(s32 arg0);
s32 func_800222A8(void);
s32 func_80022310(s32 arg0);
void func_8002237C(s32 arg0);
void func_800223B0(s32 arg0);
void func_800223D8(s32 arg0);
void FUN_80022400(u32 param_1);
void func_8002246C(u32 arg0);

extern void func_80028E84(s32 arg);

#endif