#ifndef _GNAME_H
#define _GNAME_H

#include "common.h"

// Structure for the global data blocks D_8014F818 and D_8014F828
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
} DataStruct;

// Structure for the argument object
typedef struct {
    s32 unk0;               // offset 0x00
    char pad[0x4040 - 4];   // padding up to offset 0x4040
    void* unk4040;          // offset 0x4040
} ArgStruct;

extern DataStruct D_8014F818;
extern DataStruct D_8014F828;
extern s32 D_8014F880;
extern u8 D_80147494[];
extern s32 D_800F22AC;
extern s32 D_8014F8A4;
extern s32 D_8014F8BC;
extern s32 D_8014F8A8;
extern s32 D_80122988;
extern s32 D_8014F844;
extern s32 D_8014F7E4;

extern void func_800A3938(int, int);
extern void func_8014139C(void);
extern s32 func_80142720(s32);
extern s32 func_80142C50(s32);
extern s32 func_80140AB8(s32, s32);  
extern void func_801428A4(s32, void*); 

#endif