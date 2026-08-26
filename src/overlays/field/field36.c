#include "common.h"

typedef struct
{
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s32 unk8;
} UnkStruct_8008B724;

extern UnkStruct_8008B724 D_8010A020;

void func_8008B724(void)
{
    D_8010A020.unk8 = 0;
    D_8010A020.unk4 = 0;
    D_8010A020.unk0 = 0;
}
