#include "common.h"

typedef struct
{
    s16 unk0;
    s16 unk2;
    u16 unk4;
} UnkStruct80122C00;

extern UnkStruct80122C00 D_80122C00;
extern s32 g_gosub_result_values[];
extern u8 g_menuLayoutBuffer[];

/**
 * @see decomp.me (100%)
 */
void func_800C6F60(void)
{
    s32 *result_values;
    u8 *menu;
    u8 *rec;

    result_values = g_gosub_result_values;
    menu = g_menuLayoutBuffer;
    rec = (u8 *)((result_values[0] * 0x40) + (s32)menu);
    D_80122C00.unk0 = rec[0xD00];
    D_80122C00.unk2 = rec[0xD01];
    D_80122C00.unk4 = rec[0xD02];
}
