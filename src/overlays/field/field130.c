#include "common.h"

typedef struct
{
    s16 unk0;
    s16 unk2;
    s32 unk4;
    s32 unk8;
    s8 unkC[4];
    s32 unk10;
} UnkStruct800BE2F0;

void func_800BE2F0(s32 arg0, UnkStruct800BE2F0* arg1)
{
    arg1->unk0 = func_800C33E4(arg1->unk4, arg1->unk8, &arg1->unk10);
}
