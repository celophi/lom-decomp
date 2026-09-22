/* Partial WMAP decompilation: 85.080000% (gcc280_g0). */
#include "common.h"

typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
    s32 field_0C;
    s32 field_10;
    s32 field_14;
    s32 field_18;
    s32 field_1C;
    s32 field_20;
    s32 state_24;
    s32 field_28;
    s32 field_2C;
    s32 field_30;
    s32 field_34;
    s32 field_38;
    s32 field_3C;
    s32 field_40;
    s32 field_44;
    s32 field_48;
    s32 tail_state;
} WmapState;

extern u8 D_80121538;
extern WmapState* D_80139280;
extern u8 D_80139988[];
extern u8 D_801AFBD0[];
extern s32 D_801B31E8;
extern s32 D_801B31EC;

extern void func_800BEBE0();

void func_800BD41C(void)
{
    s32 index;
    s32 screen_offset;
    s32 config_offset;
    s32 state_value;
    u8* config_base;
    u8* screen_base;
    u8* resource;
    u8* screen_entry;
    s16* config_entry;
    volatile WmapState* state;

    index = 0;
    config_base = D_801AFBD0;
    screen_base = D_80139988;
    resource = &D_80121538;
    screen_offset = 0xA0;
    config_offset = 0x190;
    state_value = 3;
    state = D_80139280;
    state->field_0C = 0x20;
    state->field_14 = 2;
    state->field_18 = config_offset;
    state->field_1C = 0x14;
    state->field_20 = 0x2C;
    state->field_04 = 0;
    state->field_08 = state_value;
    state->field_10 = 0;
    state->state_24 = state_value;
    state->field_28 = 0x32C8;

    do
    {
        screen_entry = (u8*)(screen_offset + (s32)screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + (s32)config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        *(u8**)(screen_entry + 4) = resource;
    } while (index < 50);

    D_801B31EC = 100;
    D_801B31E8++;
    func_800BEBE0();
}
