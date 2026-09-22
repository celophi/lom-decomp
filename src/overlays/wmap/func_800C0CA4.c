/* Partial WMAP decompilation: 93.679344% (gcc280_g0). */
#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

typedef struct
{
    s32 field_00, field_04, field_08, field_0C, field_10, field_14;
    s32 field_18, field_1C, field_20, field_24;
} WmapStateHead;
typedef struct
{
    s16 field_00, field_02; s32 field_04, field_08; s16 field_0C, field_0E;
    u8 pad_10[4];
} WmapConfigB;
typedef struct
{
    s16 field_00, field_02; u8 pad_04[2]; u8 field_06; u8 pad_07[7];
    s16 field_0E, field_10; u8 pad_12[0x10]; s16 field_22, field_24, field_26;
    u8 pad_28[4];
} WmapConfigA;
extern u8 D_800D9268[];
extern u8 D_80139988[];
extern u8 D_801AFBD0[];
extern void func_8006CC4C(void*, void*);
extern void func_80066F9C(void*, s32, s32, s32, s32);
extern s32 rand(void);

void func_800C0CA4(WmapStateHead* state)
{
    SVECTOR position;
    s32 screen_position;
    s32 index = state->field_00;
    WmapConfigB* config;
    s32 display_offset;
    s32 screen_offset;
    WmapConfigA* display;
    s32 draw_type;
    s32 radius;
    s32 base_radius;

    if (index < state->field_04)
    {
        u8* config_base;

        config_base = D_801AFBD0;
        config = (WmapConfigB*)((index * sizeof(WmapConfigB)) + (s32)config_base);
        display_offset = index * sizeof(WmapConfigA);
        do
        {
            if (config->field_00 != 0)
            {
                u8* display_base;

                display_base = D_800D9268;
                display = (WmapConfigA*)(display_offset + (s32)display_base);
                position.vx = (config->field_08 * (ccos(config->field_02) >> 5)) >> 0xC;
                position.vy = (config->field_08 * (csin(config->field_02) >> 5)) >> 0xC;
                position.vz = config->field_0E;
                gte_ldv0(&position);
                gte_rtps();
                screen_offset = index * 8;
                display_base = D_80139988;
                func_8006CC4C(display, (void*)(screen_offset + (s32)display_base));
                gte_stsxy(&screen_position);
                if (index < 0x78)
                {
                    func_80066F9C(display, screen_position, 0x16, 8, 0);
                }
                else
                {
                    func_80066F9C(display, screen_position, 0x19, 8, 0);
                }

                if (config->field_0C != 0)
                {
                    config->field_0C--;
                }
                else if (config->field_08 >= 0x579)
                {
                    config->field_08 -= config->field_04;
                }
                else if (display->field_24 >= 5)
                {
                    display->field_22 = 0;
                }
                else
                {
                    config->field_00 = 0;
                }
            }
            config++;
            index++;
            display_offset += sizeof(WmapConfigA);
        } while (index < state->field_04);
    }
    if (state->field_24 != 0)
    {
        index = state->field_00;
        if (index < state->field_04)
        {
            u8* free_base;

            free_base = D_800D9268;
            display = (WmapConfigA*)((index * sizeof(WmapConfigA)) + (s32)free_base);
            free_base = D_801AFBD0;
            config = (WmapConfigB*)((index * sizeof(WmapConfigB)) + (s32)free_base);
            do
            {
                index++;
                if (config->field_00 == 0)
                {
                    config->field_00 = 1;
                    display->field_06 = 0xF;
                    display->field_0E = 2;
                    display->field_02 = 0;
                    display->field_10 = -1;
                    display->field_26 = state->field_08;
                    display->field_22 = state->field_0C;
                    display->field_24 = state->field_10;
                    config->field_08 = state->field_14;
                    config->field_02 = rand() & 0xFFF;
                    radius = rand() * state->field_1C;
                    base_radius = state->field_18;
                    config->field_0E = 0;
                    config->field_04 = (radius >> 0xF) + base_radius;
                    config->field_0C = state->field_20;
                    return;
                }
                display++;
                config++;
            } while (index < state->field_04);
        }
    }
}
