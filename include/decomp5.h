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

extern s32 D_8003EC4C;

extern s32 func_80024230(s32);
extern s32 func_800235F8(void);

#endif