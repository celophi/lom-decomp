/* Partial WMAP decompilation: 96.078430% (gcc280_g0). */
#include "common.h"

typedef struct
{
    s32 field_00; s32 field_04; s32 field_08; s32 field_0C; s32 field_10;
    s32 field_14; s32 field_18; s32 field_1C; s32 field_20; s32 state_24;
    s32 field_28; s32 field_2C; s32 field_30; s32 field_34; s32 field_38;
    s32 field_3C; s32 field_40; s32 field_44; s32 field_48; s32 field_4C;
    s32 field_50;
} WmapState;

extern u8 D_80121538;
extern WmapState* D_80139280;
extern u8 D_80139988[];
extern u8 D_801AFBD0[];
extern s32 D_801B3180;
extern s32 D_801B3184;

extern void func_800BC7F4(void);

void func_800BA844(void)
{
    s32 index;
    s32 screen_offset;
    s32 config_offset;
    u8* config_base;
    u8* screen_base;
    u8* resource;
    u8* screen_entry;
    s16* config_entry;

    index = 0;
    config_base = D_801AFBD0;
    screen_base = D_80139988;
    resource = &D_80121538;
    screen_offset = 0xA0;
    config_offset = 0x190;
    D_80139280->field_0C = 0x80;
    D_80139280->field_14 = 3;
    D_80139280->field_18 = 0x384;
    D_80139280->field_1C = 0x14;
    D_80139280->field_20 = 8;
    D_80139280->state_24 = 1;
    D_80139280->field_04 = 0;
    D_80139280->field_08 = 0;
    D_80139280->field_10 = 0;
    D_80139280->field_28 = 0x1F40;
    do
    {
        screen_entry = (u8*)(screen_offset + (s32)screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + (s32)config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        *(u8**)(screen_entry + 4) = resource;
    } while (index < 0x3C);
    D_801B3184 = 0xB4;
    D_801B3180++;
    func_800BC7F4();
}
