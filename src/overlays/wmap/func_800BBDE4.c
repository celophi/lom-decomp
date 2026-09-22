#include "common.h"

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

extern WmapConfigA D_800D9268[];
extern WmapConfigA D_800D939C;
extern u8 D_8011F538;
extern s32 D_80139268;
extern void* D_801399C4;
extern s32 D_801B3150;
extern s32 D_801B3154;

extern void func_800BBE9C();

void func_800BBDE4(void)
{
    s32 i;
    s32 value;
    s32 config_index;
    s32 loop_value;
    WmapConfigA* config;
    WmapConfigA* configs;

    i = 0xAA;
    value = -1;
    config = &D_800D939C;
    configs = (WmapConfigA*)((u8*)config - 0x134);
    D_801399C4 = &D_8011F538;
    D_80139268 = 0x1E;
    config->field_06 = 0xF;
    config->field_0E = 3;
    config->field_10 = value;
    config->field_26 = 8;
    config->field_22 = 0x81;
    loop_value = value;
    config->field_02 = 0;
    config->field_24 = 1;
    for (i = 0xAA; i < 0xB9; i++)
    {
        config_index = i + 4;
        configs[config_index].field_02 = loop_value;
    }
    D_801B3154 = 0xAA;
    D_801B3150++;
    func_800BBE9C();
}
