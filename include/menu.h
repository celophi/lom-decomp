#ifndef _MENU_H
#define _MENU_H

#include "common.h"

extern void func_800AA02C(void);
extern void func_80140968(void);
extern void func_801410B0(void);
extern void func_801410E8(void);
extern void func_80141324(void);
extern void func_801423D8(void);

extern s32 D_801690AC;
extern s32 D_801690E8;
extern s32 D_801690F4;
extern s32 D_80169120;
extern s32 D_800F22AC;
extern s32 D_80122988;
extern s32 D_8016955C;
extern s32 D_801228C8;
extern u8 D_80168778[];
extern s32 D_80122730;
extern s32 D_80169100;
extern void* D_8012271C;
extern s32 D_801229FC;
extern u8 D_800FE778[];
extern u8 D_80160260[];
extern s32 D_8016910C;

typedef struct
{
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u16 unk6;
} ArgStruct;

#endif