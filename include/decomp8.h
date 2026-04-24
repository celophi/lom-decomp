#ifndef _DECOMP8_H
#define _DECOMP8_H

#include "common.h"
#include "psyq/strings.h"
#include "psyq/memory.h"

typedef struct
{
    u32 unk0;
    u32 unk4;
    u32 unk8;
    u32 unkC;
    u32 unk10;
} CommandBuffer;
typedef struct
{
    u8 pad[16];
    u32 unk10;
} NodeWithOffset16;

extern s32 D_80047404;
extern s32 D_80047408;
extern u8 D_800102B0[17];
extern CommandBuffer *D_800473EC;
extern NodeWithOffset16 *D_800473F4;
extern s32 D_80047400;


extern void func_800165CC(int, int, s32);

#endif