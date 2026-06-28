#include "common.h"

typedef struct
{
    s16 unk1;
    s16 unk2;
} f_struct;

/**
 * decomp.me (100%) https://decomp.me/scratch/lKkom
 */
void func_80024260(u32 arg0)
{
    *(u16*)0x1F801D88 = arg0;
    *(u16*)0x1F801D8A = arg0 >> 0x10;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/957fv
 */
void func_8002427C(u32 arg0)
{
    *(u16*)0x1F801D8C = arg0;
    *(s16*)0x1F801D8E = (s16)(arg0 >> 0x10);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/3KFgT
 */
void func_80024298(u32 arg0)
{
    *(u16*)0x1F801D98 = arg0;
    *(s16*)0x1F801D9A = (s16)(arg0 >> 0x10);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/HFDSO
 */
void func_800242B4(u32 arg0)
{
    *(u16*)0x1F801D94 = arg0;
    *(s16*)0x1F801D96 = (s16)(arg0 >> 0x10);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/ZyBKt
 */
void func_800242D0(u32 arg0)
{
    *(u16*)0x1F801D90 = arg0;
    *(s16*)0x1F801D92 = (s16)(arg0 >> 0x10);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/WzWyo
 */
void func_800242EC(s32 arg0, u32 arg1, u32 arg2, s32 arg3)
{
    s32 temp_v0;
    u32 var_a1;
    u32 var_a2;
    f_struct* ptr;

    var_a1 = arg1;
    var_a2 = arg2;

    if (arg3 != 0)
    {
        var_a1 = (var_a1 * arg3);
        var_a2 = (var_a2 * arg3);

        var_a1 = (u32)var_a1 >> 7;
        var_a2 = (u32)var_a2 >> 7;
    }

    ptr = (f_struct*)0x1F801C00;
    temp_v0 = arg0 * 0x4;
    ((f_struct*)(temp_v0 + ptr))->unk1 = (s16)(var_a1 & 0x7FFF);
    ((f_struct*)(temp_v0 + ptr))->unk2 = (s16)(var_a2 & 0x7FFF);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/3fXi9
 */
void func_80024334(s32 arg0, s16 arg1)
{
    s32 ptr = (s32)0x1F801C04;
    arg0 = arg0 << 4;
    *(s16*)(ptr + arg0) = arg1;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/4xQ5z
 */
void func_8002434C(s32 arg0, u32 arg1)
{
    s32 ptr = (s32)0x1F801C06;
    arg0 = arg0 << 4;
    *(s16*)(ptr + arg0) = (s16)(arg1 >> 3);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/UI7qr
 */
void func_80024368(s32 arg0, u32 arg1)
{
    s32 ptr = (s32)0x1F801C0E;
    arg0 = arg0 << 4;
    *(s16*)(ptr + arg0) = (s16)(arg1 >> 3);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/ghHQZ
 */
void func_80024384(s32 arg0, s16 arg1)
{
    s32 ptr = (s32)0x1F801C08;
    arg0 = arg0 << 4;
    *(s16*)(ptr + arg0) = arg1;
}