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
extern u8 D_8004B430[];
extern s32 D_8003EC7C;
extern s32 D_8003EC6C;
extern s32 D_8003EC44;
extern u8 D_8004F760[];
extern s16 D_8003EC64;
extern s32 D_8003EC78;
extern s16 D_8003EC42;
extern s32 D_8003EC74;
extern s16 D_8003EC40;
extern s32 D_8003EC70;
extern s32 D_8003EC68;
extern s32 D_8003EC24;
extern s32 D_8003EC28;
extern void *D_8003EC5C;
extern void *D_8003EC58;
extern u8 D_8004C2D0[];
extern u8 D_8004D400[];
extern u8 D_8004F830[];
extern u8 D_8004F750[];
extern u8 D_8004D388[];
extern u8 D_8003EC30[];
extern u8 D_80049130[];
extern u8 D_8004C260[];

extern s32 func_80024230(s32);
extern s32 func_800235F8(void);

inline static u8* off(u8* p, int o)
{
    return p + o;
}

#endif