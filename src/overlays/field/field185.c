#include "common.h"

typedef struct
{
    s16 unk0;
    s16 unk2;
    u16 unk4;
} UnkStruct80122C12;

extern s32 D_801227F0;
extern UnkStruct80122C12 D_80122C12;
extern u16 g_gosub_result_count;
extern s32 g_gosub_result_values[];

void func_800C7278(void)
{
    s32 temp;

    D_801227F0 = 0;
    temp = g_gosub_result_values[0];
    D_80122C12.unk0 = temp;
    D_80122C12.unk4 = g_gosub_result_count;
}
