/* Partial WMAP decompilation: 91.659580% (gcc280_g0). */
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

typedef struct
{
    s32 words[11];
} WmapConfigBlock;

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

extern WmapConfigA D_800D9268[];
extern s32 D_8011CF4C;
extern s32 D_8011CF74;
extern u8 D_80139988[];

extern void func_80066F9C(void*, s32, s32, s32, s32);

void func_800BA408(void)
{
    s32 index;
    s32 config_offset;
    u16 value;
    u8* config_base;
    volatile WmapConfigA* config;
    void* entry;

    index = 0xAA;
    config_base = (u8*)D_800D9268;
    config = (WmapConfigA*)(config_base + 0x1DE8);
    config_offset = 0x1DE8;
    do
    {
        entry = (void*)(config_offset + (u32)config_base);
        if (*(s16*)(entry + 2) == 0)
        {
            func_80066F9C((void*)config, D_8011CF4C, 0xB, 2, 0);
            value = config->field_24 - 5;
            config->field_24 = value;
            if ((s32)(value << 0x10) <= 0)
            {
                *(s16*)(entry + 2) = (value = -1);
            }
        }
        config++;
        index++;
        config_offset += sizeof(WmapConfigA);
    } while (index < 0xB9);

    index = 0xAA;
    if ((D_8011CF74 & 1) == 0)
    {
        s32 screen_offset;
        u8* scan_base;
        u8* screen_base;

        scan_base = (u8*)D_800D9268;
        screen_base = D_80139988;
        config_offset = 0x1DE8;
        screen_offset = 0x570;
        do
        {
            entry = (void*)(config_offset + (u32)scan_base);
            if (((WmapConfigA*)entry)->field_02 != 0)
            {
                *(WmapConfigBlock*)entry =
                    *(WmapConfigBlock*)(scan_base + 0x134);
                *(WmapAlignedPair*)(screen_offset + (u32)screen_base) = *(WmapAlignedPair*)(screen_base + 0x38);
                ((WmapConfigA*)(config_offset + (u32)scan_base))->field_22 = 0;
                return;
            }
            config_offset += 0x2C;
            index++;
            screen_offset += 8;
        } while (index < 0xB9);
    }
}
