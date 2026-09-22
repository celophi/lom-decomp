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
    s32 field_00;
    void* resource;
} WmapScreenEntry;

typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapConfigEntry;

extern s32 D_800D9158;
extern WmapConfigA D_800D9268[];
extern s32 D_800DCEB0;
extern u8 D_8011D538;
extern WmapScreenEntry D_80139988[];
extern WmapConfigEntry D_801AFBD0[];
extern s32 D_801B25DC;
extern s32 D_801B3190;
extern s32 D_801B3194;

extern void func_800BCBFC(void);

void func_800BA9E4(void)
{
    s32 i;
    volatile WmapConfigA* config;
    WmapScreenEntry* screen_entry;
    WmapConfigEntry* config_entry;
    u8* resource;
    s32 field_06;
    s32 value;

    i = 0x78;
    resource = &D_8011D538;
    field_06 = 0xF;
    config = &D_800D9268[i];
    screen_entry = &D_80139988[i];
    config_entry = &D_801AFBD0[i];
    D_801B25DC = 1;
    D_800DCEB0 = 0xC;
    do
    {
        config_entry->field_00 = 0;
        screen_entry->resource = resource;
        value = i;
        config->field_0E = (value & 1) + 2;
        config->field_02 = 0;
        config->field_06 = field_06;
        value = -1;
        config->field_10 = value;
        config++;
        screen_entry++;
        i++;
        config_entry++;
    } while (i < 0x9C);

    D_800D9158 = 2;
    D_801B3194 = 0x10;
    D_801B3190++;
    func_800BCBFC();
}
