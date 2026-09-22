/* Partial WMAP decompilation: 94.211540% (gcc280_g0). */
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
extern s32 D_801B31F0;
extern s32 D_801B31F4;

extern void func_800BEDE0();

void func_800BD4E8(void)
{
    s32 index;
    s32 screen_offset;
    s32 config_offset;
    register s32 state_value;
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
    screen_offset = 0x280;
    config_offset = 0x640;
    state_value = 1;
    state = D_80139280;
    state->field_30 = 2;
    state->field_34 = 0x80;
    state->field_40 = 0x190;
    state->field_44 = 0x50;
    state->field_48 = 0x2C;
    state->tail_state = 4;
    state->field_2C = state_value;
    state->field_38 = 0;
    state->field_3C = state_value;
    state[1].field_00 = 0x4650;

    do
    {
        screen_entry = (u8*)(screen_offset + (s32)screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + (s32)config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        *(u8**)(screen_entry + 4) = resource;
    } while (index < 40);

    D_801B31F4 = 40;
    D_801B31F0++;
    func_800BEDE0();
}
