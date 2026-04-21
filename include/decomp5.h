#ifndef _DECOMP5_H
#define _DECOMP5_H

#include "common.h"

typedef struct
{
    char pad0[0x10];
    s32 unk10;
    char pad1[0x4];
    s32 unk18;
} UnknownStruct;

typedef struct
{
    char _pad0[0x14];
    u32 unk14;
    char _pad1[0x4];
    u32 unk1C;
} SomeStruct;

extern s32 D_8003EC4C;
extern u8 D_8004C340[];

extern s32 func_80024230(s32);
extern s32 func_800235F8(void);

#endif