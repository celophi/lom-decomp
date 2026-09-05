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

typedef struct
{
    /* 0x0 */ u16 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s32 unk8;
    /* 0xC */ s32 unkC;
} UnkStruct800BE324;

extern u8 *g_field_script;

extern void func_80087D8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

void func_800BE324(s32 arg0, UnkStruct800BE324 *arg1)
{
    s32 var_a0;

    if (arg1->unk0 == 0xFF)
    {
        var_a0 = *g_field_script;
    }
    else
    {
        var_a0 = arg1->unk0;
    }

    func_80087D8C(var_a0, arg1->unk4, -arg1->unk8, arg1->unkC);
}
