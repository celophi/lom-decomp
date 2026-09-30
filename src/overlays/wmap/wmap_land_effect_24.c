#include "wmap_model_render.h"
#include "wmap_land_effect_24.h"
#include "wmap_sprite_render.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_main.h"
#include "wmap_effect_primitives.h"
#include "wmap_sequence_runtime.h"
#include "wmap_effect_resources.h"
#include "cdrom.h"
#include "sdk/libgte.h"
#include "wmap_step_sequence.h"

void wmap_land_effect_24_step_02(void);
void wmap_land_effect_24_step_03(void);
void wmap_land_effect_24_step_04(void);
void wmap_land_effect_24_step_05(void);
s32 wmap_land_effect_24_run_timeline(s32 reset);
s32 wmap_land_effect_24_run_sequence_1(s32 reset);
s32 wmap_land_effect_24_run_sequence_2(s32 reset);
s32 wmap_land_effect_24_run_sequence_3(s32 reset);
void wmap_land_effect_24_sequence_12_step_06(void);
void wmap_land_effect_24_sequence_12_step_04(void);
void wmap_land_effect_24_sequence_11_step_04(void);
void wmap_land_effect_24_sequence_10_step_04(void);
void wmap_land_effect_24_sequence_15_step_02(void);
void wmap_land_effect_24_sequence_15_step_04(void);
s32 wmap_land_effect_24_run_sequence_5(s32 reset);
s32 wmap_land_effect_24_run_sequence_6(s32 reset);
s32 wmap_land_effect_24_run_sequence_7(s32 reset);
s32 wmap_land_effect_24_run_sequence_8(s32 reset);
s32 wmap_land_effect_24_run_sequence_9(s32 reset);
s32 wmap_land_effect_24_run_sequence_10(s32 reset);
s32 wmap_land_effect_24_run_sequence_11(s32 reset);
void wmap_land_effect_24_sequence_10_step_02(void);
void wmap_land_effect_24_sequence_11_step_02(void);
void wmap_land_effect_24_sequence_12_step_02(void);
s32 wmap_land_effect_24_run_sequence_12(s32 reset);
s32 wmap_land_effect_24_run_sequence_13(s32 reset);
s32 wmap_land_effect_24_run_sequence_14(s32 reset);
s32 wmap_land_effect_24_run_sequence_15(s32 reset);
s32 wmap_land_effect_24_run_sequence_4(s32 reset);
void wmap_land_effect_24_sequence_1_step_02(void);
void wmap_land_effect_24_sequence_1_step_04(void);
void wmap_land_effect_24_sequence_2_step_02(void);
void wmap_land_effect_24_sequence_2_step_04(void);
void wmap_land_effect_24_sequence_3_step_02(void);
void wmap_land_effect_24_sequence_3_step_04(void);
void wmap_land_effect_24_sequence_4_step_04(void);
void wmap_land_effect_24_sequence_5_step_02(void);
void wmap_land_effect_24_sequence_6_step_02(void);
void wmap_land_effect_24_sequence_4_step_02(void);

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s16 field_04;
} WmapShort3;

typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
} WmapInt3;

typedef struct
{
    s32 value;
    u8 pad[0x24];
} WmapCell;

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

typedef void (*WmapHandler)(void);

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapConfigEntry;

extern WmapHandler D_800D76F4[];
extern WmapHandler D_800D770C[];
extern WmapHandler D_800D7774[];
extern WmapHandler D_800D778C[];
extern WmapHandler D_800D77A4[];
extern WmapHandler D_800D77BC[];
extern WmapHandler D_800D77D4[];
extern WmapHandler D_800D77E4[];
extern WmapHandler D_800D77F4[];
extern WmapHandler D_800D7804[];
extern WmapHandler D_800D7814[];
extern WmapHandler D_800D7824[];
extern WmapHandler D_800D78A4[];
extern WmapHandler D_800D788C[];
extern WmapHandler D_800D7874[];
extern WmapHandler D_800D7854[];
extern WmapHandler D_800D783C[];
extern u8 D_800D95D8[];
extern u8 D_800DA028[];
extern s32 D_800DCEB0;
extern WmapShort3 D_800DCEB8;
extern s32 g_wmap_load_buffer[];
extern s32 D_800D9158;
extern s32* g_wmap_effect_model_pack_1;
extern s32* g_wmap_effect_model_pack_2;
extern s32* g_wmap_effect_model_pack_3;
extern s32* D_8011CF2C;
extern s32 g_wmap_selected_artifact;
extern u8 g_wmap_animation_bank_0;
extern u8 g_wmap_animation_bank_1;
extern u8 g_wmap_animation_bank_2;
extern WmapInt3 D_80139200;
extern WmapShort3 D_80139210;
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139268;
extern WmapCell g_wmap_cells[][6];
extern WmapAnimationSlot D_80139A28[];
extern u8 D_80139C08[];
extern WmapInt3 D_80139968;
extern void* D_801399DC;
extern s32 D_8013B29C;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern s32 D_801B25D8;
extern s32 D_801B25DC;
extern u32 g_wmap_land_effect_24_step;
extern s32 g_wmap_land_effect_24_timer;
extern u32 g_wmap_land_effect_24_timeline_step;
extern s32 g_wmap_land_effect_24_timeline_timer;
extern u32 g_wmap_land_effect_24_sequence_1_step;
extern s32 g_wmap_land_effect_24_sequence_1_timer;
extern u32 g_wmap_land_effect_24_sequence_2_step;
extern s32 g_wmap_land_effect_24_sequence_2_timer;
extern u32 g_wmap_land_effect_24_sequence_3_step;
extern s32 g_wmap_land_effect_24_sequence_3_timer;
extern u32 g_wmap_land_effect_24_sequence_4_step;
extern s32 g_wmap_land_effect_24_sequence_4_timer;
extern u32 g_wmap_land_effect_24_sequence_5_step;
extern s32 g_wmap_land_effect_24_sequence_5_timer;
extern u32 g_wmap_land_effect_24_sequence_6_step;
extern s32 g_wmap_land_effect_24_sequence_6_timer;
extern u32 g_wmap_land_effect_24_sequence_7_step;
extern s32 g_wmap_land_effect_24_sequence_7_timer;
extern u32 g_wmap_land_effect_24_sequence_8_step;
extern s32 g_wmap_land_effect_24_sequence_8_timer;
extern u32 g_wmap_land_effect_24_sequence_9_step;
extern s32 g_wmap_land_effect_24_sequence_9_timer;
extern u32 g_wmap_land_effect_24_sequence_10_step;
extern s32 g_wmap_land_effect_24_sequence_10_timer;
extern u32 g_wmap_land_effect_24_sequence_11_step;
extern s32 g_wmap_land_effect_24_sequence_11_timer;
extern u32 g_wmap_land_effect_24_sequence_12_step;
extern s32 g_wmap_land_effect_24_sequence_12_timer;
extern u32 g_wmap_land_effect_24_sequence_13_step;
extern s32 g_wmap_land_effect_24_sequence_13_timer;
extern u32 g_wmap_land_effect_24_sequence_14_step;
extern s32 g_wmap_land_effect_24_sequence_14_timer;
extern u32 g_wmap_land_effect_24_sequence_15_step;
extern s32 g_wmap_land_effect_24_sequence_15_timer;
extern void wmap_land_effect_24_sequence_7_step_02();
extern void wmap_land_effect_24_sequence_8_step_02();
extern void wmap_land_effect_24_sequence_9_step_02();
extern void wmap_land_effect_24_sequence_10_step_02(void);
extern void wmap_land_effect_24_sequence_13_step_02();
extern void wmap_land_effect_24_sequence_13_step_04();
extern void wmap_land_effect_24_sequence_14_step_02();
extern void wmap_land_effect_24_sequence_14_step_04();
extern void wmap_land_effect_24_step_02();
extern void wmap_land_effect_24_step_03();
extern void wmap_land_effect_24_step_04();
extern void wmap_land_effect_24_step_05();
extern s32 wmap_land_effect_24_run_timeline(s32);
extern s32 wmap_land_effect_24_run_sequence_1(s32);
extern void wmap_land_effect_24_sequence_1_step_02();
extern void wmap_land_effect_24_sequence_1_step_04();
extern s32 wmap_land_effect_24_run_sequence_2(s32);
extern void wmap_land_effect_24_sequence_2_step_02();
extern void wmap_land_effect_24_sequence_2_step_04();
extern s32 wmap_land_effect_24_run_sequence_3(s32);
extern void wmap_land_effect_24_sequence_3_step_02();
extern void wmap_land_effect_24_sequence_3_step_04();
extern s32 wmap_land_effect_24_run_sequence_4(s32);
extern void wmap_land_effect_24_sequence_4_step_04();
extern s32 wmap_land_effect_24_run_sequence_5(s32);
extern void wmap_land_effect_24_sequence_5_step_02();
extern s32 wmap_land_effect_24_run_sequence_6(s32);
extern void wmap_land_effect_24_sequence_6_step_02();
extern s32 wmap_land_effect_24_run_sequence_7(s32);
extern s32 wmap_land_effect_24_run_sequence_8(s32);
extern s32 wmap_land_effect_24_run_sequence_9(s32);
extern s32 wmap_land_effect_24_run_sequence_10(s32);
extern void wmap_land_effect_24_sequence_10_step_04();
extern s32 wmap_land_effect_24_run_sequence_11(s32);
extern void wmap_land_effect_24_sequence_11_step_02(void);
extern void wmap_land_effect_24_sequence_11_step_04();
extern s32 wmap_land_effect_24_run_sequence_12(s32);
extern void wmap_land_effect_24_sequence_12_step_02(void);
extern void wmap_land_effect_24_sequence_12_step_04();
extern void wmap_land_effect_24_sequence_12_step_06();
extern s32 wmap_land_effect_24_run_sequence_13(s32);
extern s32 wmap_land_effect_24_run_sequence_14(s32);
extern s32 wmap_land_effect_24_run_sequence_15(s32);
extern void wmap_land_effect_24_sequence_15_step_02();
extern void wmap_land_effect_24_sequence_15_step_04();

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

extern VECTOR g_wmap_camera_translation;

extern SVECTOR D_801B2670;

extern WmapSpriteActor D_800D939C;
extern WmapSpriteActor D_800D93F4;
extern WmapSpriteActor D_800D9420;

extern WmapAnimationSlot g_wmap_actor_animations[];
extern WmapAnimationSlot D_801399C0;
extern WmapAnimationSlot D_801399D0;
extern WmapAnimationSlot D_801399D8;

extern WmapScreenPosition g_wmap_focus_screen_position;
extern WmapScreenPosition D_80182D58;

extern s32* g_wmap_effect_params;

extern WmapConfigEntry g_wmap_actor_motions[];

void wmap_land_effect_24_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_24_run_sequence_7);
    g_wmap_transition_mesh_hidden = 1;
    wmap_start_sequence(wmap_land_effect_24_run_sequence_2);
    wmap_start_sequence(wmap_land_effect_24_run_sequence_1);
    wmap_start_sequence(wmap_land_effect_24_run_sequence_3);
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 0;
    D_80182D74.field_00 = 0x38;
    D_80182D74.field_01 = 0;
    D_80182D74.field_02 = 0x18;
    D_80182D80.field_00 = 0x38;
    D_80182D80.field_01 = 0;
    D_80182D80.field_02 = 0x18;
    D_80182D8C.field_00 = 0x38;
    D_80182D8C.field_01 = 0;
    D_80182D8C.field_02 = 0x18;
    D_80182D94.field_00 = 0x38;
    D_80182D94.field_01 = 0;
    D_80182D94.field_02 = 0x18;
    g_wmap_land_effect_24_timeline_timer = 8;
    g_wmap_land_effect_24_timeline_step++;
}

void wmap_land_effect_24_timeline_step_15(void)
{
    wmap_start_sequence(wmap_land_effect_24_run_sequence_12);
    wmap_install_callback(func_8006C0EC);
    D_80139210.field_00 = 5;
    D_80139210.field_02 = 0;
    D_80139210.field_04 = 0;
    D_80139968.field_00 = 0;
    D_80139968.field_04 = 2;
    D_80139968.field_08 = 0;
    D_800DCEB8.field_00 = 0xFA;
    D_800DCEB8.field_02 = 0;
    D_800DCEB8.field_04 = 0;
    D_80139200.field_00 = 0;
    D_80139200.field_04 = 0x64;
    D_80139200.field_08 = 0;
    g_wmap_backdrop_target_level = 0;
    D_80182D74.field_00 = 0x38;
    D_80182D74.field_01 = 0;
    D_80182D74.field_02 = 0x18;
    D_80182D80.field_00 = 0x38;
    D_80182D80.field_01 = 0;
    D_80182D80.field_02 = 0x18;
    D_80182D8C.field_00 = 0;
    D_80182D8C.field_01 = 0;
    D_80182D8C.field_02 = 0;
    D_80182D94.field_00 = 0;
    D_80182D94.field_01 = 0;
    D_80182D94.field_02 = 0;
    g_wmap_land_effect_24_timeline_timer = 0x70;
    g_wmap_land_effect_24_timeline_step++;
}

/** @brief Update active effects and initialize the next available config slot. */
void func_800BA408(void)
{
    s32 index;
    u8* config_offset;
    u16 value;
    u8* config_base;
    WmapConfigA* config;
    void* entry;

    index = 0xAA;

    do
    {
        config_offset = index * 0x2C + 0xB0;
        config_base = (u8*)g_wmap_sprite_actors;
        entry = (void*)(config_offset + (u32)config_base);
        config = &g_wmap_sprite_actors[index + 4];
        if (*(s16*)(entry + 2) == 0)
        {
            wmap_draw_actor_sprite((void*)config, g_wmap_focus_screen_position.packed, 0xB, 2, 0);
            value = config->field_24 - 5;
            config->field_24 = value;
            if ((s32)(value << 0x10) <= 0)
            {
                *(s16*)(entry + 2) = (value = -1);
            }
        }
        index++;
    } while (index < 0xB9);

    index = 0xAA;
    if ((g_wmap_frame_count & 1) == 0)
    {
        void* entry;
        u8* copy_end;
        u8* config_offset;
        s32 screen_offset;
        u8* scan_base;
        u8* screen_base;

        scan_base = (u8*)g_wmap_sprite_actors;
        copy_end = scan_base + 0x154;
        screen_base = g_wmap_actor_animations;
        config_offset = 0x1DE8;
        screen_offset = 0x570;
        do
        {
            entry = (void*)(config_offset + (u32)scan_base);
            if (((WmapConfigA*)entry)->field_02 != 0)
            {
                typedef struct
                {
                    s32 words[4];
                } Chunk;
                typedef struct
                {
                    s32 words[3];
                } Tail;
                u8* source;
                source = scan_base + 0x134;
                do
                {
                    *(Chunk*)entry = *(Chunk*)source;
                    source += 16;
                    entry += 16;
                } while (source != copy_end);
                *(Tail*)entry = *(Tail*)source;
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

void wmap_land_effect_24_sequence_7_step_02(void)
{
    s32 value;
    s32 fade;
    s32 timer;

    value = g_wmap_effect_model_a_position.vz - 0xDAC;
    g_wmap_effect_model_a_position.vz = value;
    if (value < 0x2710)
    {
        g_wmap_effect_model_a_position.vz = 0x2710;
    }
    PushMatrix();
    wmap_set_map_rotation(&g_wmap_effect_model_a_rotation);
    if (g_wmap_effect_fade_a != 0)
    {
        wmap_draw_model(g_wmap_load_buffer, 0, 4, 0x35, 0x7800, 1, g_wmap_effect_fade_a, 0, 0, -1);
        fade = g_wmap_effect_fade_a - 2;
        g_wmap_effect_fade_a = fade;
        if (fade < 0)
        {
            g_wmap_effect_fade_a = 0;
        }
    }
    PopMatrix();
    timer = g_wmap_land_effect_24_sequence_7_timer - 1;
    g_wmap_land_effect_24_sequence_7_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_7_step++;
    }
}

void wmap_land_effect_24_sequence_8_step_02(void)
{
    s32 value;
    s32 fade;
    s32 timer;

    value = g_wmap_effect_model_b_position.vz - 0xDAC;
    g_wmap_effect_model_b_position.vz = value;
    if (value < 0x2710)
    {
        g_wmap_effect_model_b_position.vz = 0x2710;
    }
    PushMatrix();
    wmap_set_map_rotation(&g_wmap_effect_model_b_rotation);
    if (g_wmap_effect_fade_b != 0)
    {
        wmap_draw_model(g_wmap_effect_model_pack_1, 0, 4, 0x35, 0x7800, 1, g_wmap_effect_fade_b, 0, 0, -1);
        fade = g_wmap_effect_fade_b - 1;
        g_wmap_effect_fade_b = fade;
        if (fade < 0)
        {
            g_wmap_effect_fade_b = 0;
        }
    }
    PopMatrix();
    timer = g_wmap_land_effect_24_sequence_8_timer - 1;
    g_wmap_land_effect_24_sequence_8_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_8_step++;
    }
}

void wmap_land_effect_24_sequence_9_step_02(void)
{
    s32 value;
    s32 fade;
    s32 timer;

    value = g_wmap_effect_model_c_position.vz - 0xDAC;
    g_wmap_effect_model_c_position.vz = value;
    if (value < 0x2710)
    {
        g_wmap_effect_model_c_position.vz = 0x2710;
    }
    PushMatrix();
    wmap_set_map_rotation(&g_wmap_effect_model_c_rotation);
    if (g_wmap_effect_fade_c != 0)
    {
        wmap_draw_model(g_wmap_effect_model_pack_2, 0, 4, 0x35, 0x7800, 1, g_wmap_effect_fade_c, 0, 0, -1);
        fade = g_wmap_effect_fade_c - 8;
        g_wmap_effect_fade_c = fade;
        if (fade < 0)
        {
            g_wmap_effect_fade_c = 0;
        }
    }
    PopMatrix();
    timer = g_wmap_land_effect_24_sequence_9_timer - 1;
    g_wmap_land_effect_24_sequence_9_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_9_step++;
    }
}

void wmap_land_effect_24_sequence_10_step_01(void)
{
    s32 i;

    g_wmap_effect_params[1] = 0;
    g_wmap_effect_params[2] = 0;
    g_wmap_effect_params[3] = 0x80;
    g_wmap_effect_params[4] = 0;
    g_wmap_effect_params[5] = 3;
    g_wmap_effect_params[6] = 0x384;
    g_wmap_effect_params[7] = 0x14;
    g_wmap_effect_params[8] = 8;
    g_wmap_effect_params[9] = 1;
    g_wmap_effect_params[10] = 0x1F40;
    for (i = 0; i < 60; i++)
    {
        g_wmap_actor_motions[i + 20].field_00 = 0;
        g_wmap_actor_animations[i + 20].data = &g_wmap_animation_bank_2;
    }
    g_wmap_land_effect_24_sequence_10_timer = 0xB4;
    g_wmap_land_effect_24_sequence_10_step++;
    wmap_land_effect_24_sequence_10_step_02();
}

void wmap_land_effect_24_sequence_11_step_01(void)
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
    config_base = g_wmap_actor_motions;
    screen_base = g_wmap_actor_animations;
    resource = &g_wmap_animation_bank_2;
    screen_offset = 0x280;
    config_offset = 0x640;
    g_wmap_effect_params[11] = 1;
    g_wmap_effect_params[12] = 4;
    g_wmap_effect_params[13] = 0x40;
    g_wmap_effect_params[15] = 3;
    g_wmap_effect_params[16] = -0x1C2;
    g_wmap_effect_params[17] = 0x50;
    g_wmap_effect_params[18] = 8;
    g_wmap_effect_params[19] = 2;
    g_wmap_effect_params[14] = 0;
    g_wmap_effect_params[20] = 0x61A8;

    do
    {
        screen_entry = (u8*)(screen_offset + (s32)screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + (s32)config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        *(u8**)(screen_entry + 4) = resource;
    } while (index < 0x1E);

    g_wmap_land_effect_24_sequence_11_timer = 0x5A;
    g_wmap_land_effect_24_sequence_11_step++;
    wmap_land_effect_24_sequence_11_step_02();
}

void wmap_land_effect_24_sequence_12_step_01(void)
{
    s32 i;

    D_801B25DC = 1;
    D_800DCEB0 = 12;
    for (i = 120; i < 156; i++)
    {
        g_wmap_actor_motions[i].field_00 = 0;
        g_wmap_actor_animations[i].data = &g_wmap_animation_bank_0;
        g_wmap_sprite_actors[i].sequence = (i & 1) + 2;
        g_wmap_sprite_actors[i].resource_index = 0;
        g_wmap_sprite_actors[i].scale_index = 15;
        g_wmap_sprite_actors[i].previous_sequence = -1;
    }

    D_800D9158 = 2;
    g_wmap_land_effect_24_sequence_12_timer = 0x10;
    g_wmap_land_effect_24_sequence_12_step++;
    wmap_land_effect_24_sequence_12_step_02();
}

void wmap_land_effect_24_sequence_13_step_02(void)
{
    s32 value;
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(g_wmap_effect_model_pack_3, (D_80139240 >> 4) & 7, 0xA, 0x35, 0x7800, 1, g_wmap_effect_fade_d, 0, 0, -1);
    value = g_wmap_effect_fade_d + 8;
    g_wmap_effect_fade_d = value;
    if (value >= 0x82)
    {
        g_wmap_effect_fade_d = 0x81;
    }
    D_80139240 += 0x10;
    timer = g_wmap_land_effect_24_sequence_13_timer - 1;
    g_wmap_land_effect_24_sequence_13_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_13_step++;
    }
}

void wmap_land_effect_24_sequence_13_step_04(void)
{
    s32 value;
    s32 timer;

    func_8006AEE0();
    wmap_draw_model(g_wmap_effect_model_pack_3, (D_80139240 >> 4) & 7, 0xA, 0x35, 0x7800, 1, g_wmap_effect_fade_d, 0, 0, -1);
    value = g_wmap_effect_fade_d - 4;
    g_wmap_effect_fade_d = value;
    if (value < 0)
    {
        g_wmap_effect_fade_d = 0;
    }
    D_80139240 += 0x10;
    timer = g_wmap_land_effect_24_sequence_13_timer - 1;
    g_wmap_land_effect_24_sequence_13_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_13_step++;
    }
}

void wmap_land_effect_24_sequence_14_step_02(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&D_801B2670);
    wmap_draw_model(D_8011CF2C, (D_8013924C >> 4) & 7, 0xB0, 0x35, 0x7800, 1, D_801B25D8, 4, -0x44, -1);
    value = D_801B25D8 + 2;
    D_801B25D8 = value;
    if (value >= 0x81)
    {
        D_801B25D8 = 0x80;
    }
    D_8013924C += 0x18;
    timer = g_wmap_land_effect_24_sequence_14_timer - 1;
    g_wmap_land_effect_24_sequence_14_timer = timer;
    D_801B2670.vz += 0x28;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_14_step++;
    }
}

void wmap_land_effect_24_sequence_14_step_04(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&D_801B2670);
    wmap_draw_model(D_8011CF2C, (D_8013924C >> 4) & 7, 0xB0, 0x35, 0x7800, 1, D_801B25D8, 4, -0x44, -1);
    value = D_801B25D8 - 0x10;
    D_801B25D8 = value;
    if (value < 0)
    {
        D_801B25D8 = 0;
    }
    D_8013924C += 0x18;
    timer = g_wmap_land_effect_24_sequence_14_timer - 1;
    g_wmap_land_effect_24_sequence_14_timer = timer;
    D_801B2670.vz += 0x28;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_14_step++;
    }
}

WMAP_STEP_RUNNER(wmap_land_effect_24_run, D_800D76F4, 6, g_wmap_land_effect_24_step, g_wmap_land_effect_24_timer)

WMAP_STEP_RESET(wmap_land_effect_24_step_00, g_wmap_land_effect_24_step, g_wmap_land_effect_24_timer)

void wmap_land_effect_24_step_01(void)
{
    s32 value;

    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = value = 1;
    g_wmap_land_effect_24_step += value;
    wmap_land_effect_24_step_02();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_24_step_02, g_wmap_land_effect_24_step, wmap_land_effect_24_step_03)

void wmap_land_effect_24_step_03(void)
{
    s32 value;

    wmap_start_sequence(wmap_land_effect_24_run_timeline);
    g_wmap_sequence_busy = value = 1;
    g_wmap_land_effect_24_step += value;
    wmap_land_effect_24_step_04();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_24_step_04, g_wmap_land_effect_24_step, wmap_land_effect_24_step_05)

WMAP_STEP_ADVANCE(wmap_land_effect_24_step_05, g_wmap_land_effect_24_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_timeline, D_800D770C, 0x1A, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_24_timeline_step_00, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

void wmap_land_effect_24_timeline_step_01(void)
{
    s32 value;

    g_wmap_event_active = value = 1;
    wmap_play_sound(0x35, 0x80);
    D_8013B29C = value;
    g_wmap_land_effect_24_timeline_timer = 4;
    g_wmap_land_effect_24_timeline_step += value;
}

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_02, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

void wmap_land_effect_24_timeline_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_24_run_sequence_4);
    wmap_start_map_tint(0x605060);
    g_wmap_backdrop_target_level = 0xA;
    g_wmap_land_effect_24_timeline_timer = 0x12;
    g_wmap_land_effect_24_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_04, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

void wmap_land_effect_24_timeline_step_05(void)
{
    g_wmap_placement_overlay_hidden = 1;
    g_wmap_backdrop_target_level = 5;
    g_wmap_land_effect_24_timeline_timer = 0xE;
    g_wmap_land_effect_24_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_06, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_08, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_24_timeline_step_09, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer,
                         wmap_land_effect_24_run_sequence_10, 4)

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_10, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_24_timeline_step_11, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer,
                         wmap_land_effect_24_run_sequence_13, 0x5C)

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_12, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_24_timeline_step_13, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer,
                         wmap_land_effect_24_run_sequence_15, 0x1E)

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_14, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_16, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_24_timeline_step_17, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer,
                         wmap_land_effect_24_run_sequence_14, 0x5A)

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_18, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

void wmap_land_effect_24_timeline_step_19(void)
{
    wmap_start_sequence(wmap_land_effect_24_run_sequence_8);
    g_wmap_backdrop_target_level = 7;
    g_wmap_transition_mesh_hidden = 0;
    wmap_start_map_tint(0x703080);
    D_8013B29C = 0;
    wmap_start_sequence(wmap_land_effect_24_run_sequence_5);
    wmap_start_sequence(wmap_land_effect_24_run_sequence_6);
    g_wmap_land_effect_24_timeline_timer = 4;
    g_wmap_land_effect_24_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_20, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_24_timeline_step_21, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer,
                         wmap_land_effect_24_run_sequence_11, 0xB7)

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_22, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

void wmap_land_effect_24_timeline_step_23(void)
{
    wmap_start_sequence(wmap_land_effect_24_run_sequence_9);
    wmap_start_map_tint(0x808080);
    g_wmap_backdrop_target_level = 0x10;
    g_wmap_land_effect_24_timeline_timer = 0x3C;
    g_wmap_cells[g_wmap_focus_cell_x][g_wmap_focus_cell_y].value = g_wmap_selected_artifact | 0x100;
    g_wmap_land_effect_24_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_24_timeline_step_24, g_wmap_land_effect_24_timeline_step, g_wmap_land_effect_24_timeline_timer)

void wmap_land_effect_24_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    g_wmap_land_effect_24_timeline_step++;
}

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_1, D_800D7774, 6, g_wmap_land_effect_24_sequence_1_step, g_wmap_land_effect_24_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_1_step_00, g_wmap_land_effect_24_sequence_1_step, g_wmap_land_effect_24_sequence_1_timer)

void wmap_land_effect_24_sequence_1_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[4];

    g_wmap_actor_animations[4].data = &g_wmap_animation_bank_1;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 4;
    actor->target_shade = 0x81;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->shade = 1;
    g_wmap_land_effect_24_sequence_1_timer = 0x88;
    g_wmap_land_effect_24_sequence_1_step++;
    wmap_land_effect_24_sequence_1_step_02();
}

void wmap_land_effect_24_sequence_1_step_02(void)
{
    s32 timer;

    wmap_step_actor_animation(&g_wmap_sprite_actors[4], &g_wmap_actor_animations[4]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[4], g_wmap_focus_screen_position.packed, 0xB, 2, 0);
    timer = g_wmap_land_effect_24_sequence_1_timer - 1;
    g_wmap_land_effect_24_sequence_1_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_1_step++;
    }
}

void wmap_land_effect_24_sequence_1_step_03(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[4];

    actor->target_shade = 0;
    actor->shade_step = 8;
    g_wmap_land_effect_24_sequence_1_timer = 0x10;
    g_wmap_land_effect_24_sequence_1_step++;
    wmap_land_effect_24_sequence_1_step_04();
}

void wmap_land_effect_24_sequence_1_step_04(void)
{
    s32 timer;

    wmap_step_actor_animation(&g_wmap_sprite_actors[4], &g_wmap_actor_animations[4]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[4], g_wmap_focus_screen_position.packed, 0xB, 2, 0);
    timer = g_wmap_land_effect_24_sequence_1_timer - 1;
    g_wmap_land_effect_24_sequence_1_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_1_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_1_step_05, g_wmap_land_effect_24_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_2, D_800D778C, 6, g_wmap_land_effect_24_sequence_2_step, g_wmap_land_effect_24_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_2_step_00, g_wmap_land_effect_24_sequence_2_step, g_wmap_land_effect_24_sequence_2_timer)

void wmap_land_effect_24_sequence_2_step_01(void)
{
    s32 value;
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    g_wmap_actor_animations[5].data = &g_wmap_animation_bank_1;
    actor->scale_index = 0xF;
    actor->sequence = value = 1;
    actor->previous_sequence = -value;
    actor->shade_step = 8;
    actor->shade = value;
    actor->resource_index = 0;
    actor->target_shade = 0x81;
    g_wmap_land_effect_24_sequence_2_timer = 0x88;
    g_wmap_land_effect_24_sequence_2_step++;
    wmap_land_effect_24_sequence_2_step_02();
}

void wmap_land_effect_24_sequence_2_step_02(void)
{
    s32 timer;

    wmap_step_actor_animation(&g_wmap_sprite_actors[5], &g_wmap_actor_animations[5]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[5], g_wmap_focus_screen_position.packed, 0xB, 2, 0);
    timer = g_wmap_land_effect_24_sequence_2_timer - 1;
    g_wmap_land_effect_24_sequence_2_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_2_step++;
    }
}

void wmap_land_effect_24_sequence_2_step_03(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    actor->shade_step = 8;
    actor->target_shade = 0;
    g_wmap_land_effect_24_sequence_2_timer = 0x10;
    g_wmap_land_effect_24_sequence_2_step++;
    wmap_land_effect_24_sequence_2_step_04();
}

void wmap_land_effect_24_sequence_2_step_04(void)
{
    s32 timer;

    wmap_step_actor_animation(&g_wmap_sprite_actors[5], &g_wmap_actor_animations[5]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[5], g_wmap_focus_screen_position.packed, 0xB, 2, 0);
    timer = g_wmap_land_effect_24_sequence_2_timer - 1;
    g_wmap_land_effect_24_sequence_2_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_2_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_2_step_05, g_wmap_land_effect_24_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_3, D_800D77A4, 6, g_wmap_land_effect_24_sequence_3_step, g_wmap_land_effect_24_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_3_step_00, g_wmap_land_effect_24_sequence_3_step, g_wmap_land_effect_24_sequence_3_timer)

void wmap_land_effect_24_sequence_3_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    g_wmap_actor_animations[6].data = &g_wmap_animation_bank_1;
    actor->scale_index = 0xF;
    actor->sequence = 2;
    actor->previous_sequence = -1;
    actor->shade_step = 8;
    actor->resource_index = 0;
    actor->target_shade = 0x81;
    actor->shade = 0x81;
    g_wmap_land_effect_24_sequence_3_timer = 0x88;
    g_wmap_land_effect_24_sequence_3_step++;
    wmap_land_effect_24_sequence_3_step_02();
}

void wmap_land_effect_24_sequence_3_step_02(void)
{
    s32 timer;

    wmap_step_actor_animation(&g_wmap_sprite_actors[6], &g_wmap_actor_animations[6]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[6], g_wmap_focus_screen_position.packed, 0xB, 2, 0);
    timer = g_wmap_land_effect_24_sequence_3_timer - 1;
    g_wmap_land_effect_24_sequence_3_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_3_step++;
    }
}

void wmap_land_effect_24_sequence_3_step_03(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    actor->shade_step = 2;
    actor->target_shade = 0;
    g_wmap_land_effect_24_sequence_3_timer = 0x40;
    g_wmap_land_effect_24_sequence_3_step++;
    wmap_land_effect_24_sequence_3_step_04();
}

void wmap_land_effect_24_sequence_3_step_04(void)
{
    s32 timer;

    wmap_step_actor_animation(&g_wmap_sprite_actors[6], &g_wmap_actor_animations[6]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[6], g_wmap_focus_screen_position.packed, 0xB, 2, 0);
    timer = g_wmap_land_effect_24_sequence_3_timer - 1;
    g_wmap_land_effect_24_sequence_3_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_3_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_3_step_05, g_wmap_land_effect_24_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_4, D_800D77BC, 6, g_wmap_land_effect_24_sequence_4_step, g_wmap_land_effect_24_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_4_step_00, g_wmap_land_effect_24_sequence_4_step, g_wmap_land_effect_24_sequence_4_timer)

void wmap_land_effect_24_sequence_4_step_01(void)
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
    g_wmap_actor_animations[7].data = &g_wmap_animation_bank_1;
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
    g_wmap_land_effect_24_sequence_4_timer = 0xAA;
    g_wmap_land_effect_24_sequence_4_step++;
    wmap_land_effect_24_sequence_4_step_02();
}

void wmap_land_effect_24_sequence_4_step_02(void)
{
    s32 timer;

    wmap_step_actor_animation(&D_800D939C, &D_801399C0);
    wmap_draw_actor_sprite(&D_800D939C, g_wmap_focus_screen_position.packed, 0xB, 2, 0);
    if (D_80139268 > 0)
    {
        func_800BA408();
    }
    timer = g_wmap_land_effect_24_sequence_4_timer - 1;
    D_80139268--;
    g_wmap_land_effect_24_sequence_4_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_4_step++;
    }
}

void wmap_land_effect_24_sequence_4_step_03(void)
{
    D_800D939C.shade_step = 8;
    D_800D939C.target_shade = 0;
    g_wmap_land_effect_24_sequence_4_timer = 0x10;
    g_wmap_land_effect_24_sequence_4_step++;
    wmap_land_effect_24_sequence_4_step_04();
}

void wmap_land_effect_24_sequence_4_step_04(void)
{
    s32 timer;

    wmap_step_actor_animation(&D_800D939C, &D_801399C0);
    wmap_draw_actor_sprite(&D_800D939C, g_wmap_focus_screen_position.packed, 0xB, 2, 0);
    timer = g_wmap_land_effect_24_sequence_4_timer - 1;
    g_wmap_land_effect_24_sequence_4_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_4_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_4_step_05, g_wmap_land_effect_24_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_5, D_800D77D4, 4, g_wmap_land_effect_24_sequence_5_step, g_wmap_land_effect_24_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_5_step_00, g_wmap_land_effect_24_sequence_5_step, g_wmap_land_effect_24_sequence_5_timer)

void wmap_land_effect_24_sequence_5_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    g_wmap_actor_animations[5].data = &g_wmap_animation_bank_0;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 8;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->target_shade = 0x80;
    actor->shade = 0;
    g_wmap_land_effect_24_sequence_5_timer = 0xDC;
    g_wmap_land_effect_24_sequence_5_step++;
    wmap_land_effect_24_sequence_5_step_02();
}

void wmap_land_effect_24_sequence_5_step_02(void)
{
    s32 timer;

    wmap_step_actor_animation(&g_wmap_sprite_actors[5], &g_wmap_actor_animations[5]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[5], g_wmap_focus_screen_position.packed, 0x1F, 0xC, 0);
    timer = g_wmap_land_effect_24_sequence_5_timer - 1;
    g_wmap_land_effect_24_sequence_5_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_5_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_5_step_03, g_wmap_land_effect_24_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_6, D_800D77E4, 4, g_wmap_land_effect_24_sequence_6_step, g_wmap_land_effect_24_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_6_step_00, g_wmap_land_effect_24_sequence_6_step, g_wmap_land_effect_24_sequence_6_timer)

void wmap_land_effect_24_sequence_6_step_01(void)
{
    s32 value;

    g_wmap_actor_animations[9].data = &g_wmap_animation_bank_0;
    D_800D93F4.scale_index = 0xF;
    D_800D93F4.sequence = value = 1;
    D_800D93F4.previous_sequence = -value;
    D_800D93F4.shade_step = 8;
    D_800D93F4.shade = value;
    D_800D93F4.resource_index = 0;
    D_800D93F4.target_shade = 0x81;
    g_wmap_land_effect_24_sequence_6_timer = 0xBD;
    g_wmap_land_effect_24_sequence_6_step++;
    wmap_land_effect_24_sequence_6_step_02();
}

void wmap_land_effect_24_sequence_6_step_02(void)
{
    s32 timer;

    wmap_step_actor_animation(&D_800D93F4, &D_801399D0);
    wmap_draw_actor_sprite(&D_800D93F4, g_wmap_focus_screen_position.packed, 0x2D, 0x1E, 0);
    timer = g_wmap_land_effect_24_sequence_6_timer - 1;
    g_wmap_land_effect_24_sequence_6_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_6_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_6_step_03, g_wmap_land_effect_24_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_7, D_800D77F4, 4, g_wmap_land_effect_24_sequence_7_step, g_wmap_land_effect_24_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_7_step_00, g_wmap_land_effect_24_sequence_7_step, g_wmap_land_effect_24_sequence_7_timer)

WMAP_STEP_DROP_START(wmap_land_effect_24_sequence_7_step_01, g_wmap_land_effect_24_sequence_7_step, g_wmap_land_effect_24_sequence_7_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40, wmap_land_effect_24_sequence_7_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_7_step_03, g_wmap_land_effect_24_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_8, D_800D7804, 4, g_wmap_land_effect_24_sequence_8_step, g_wmap_land_effect_24_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_8_step_00, g_wmap_land_effect_24_sequence_8_step, g_wmap_land_effect_24_sequence_8_timer)

WMAP_STEP_DROP_START(wmap_land_effect_24_sequence_8_step_01, g_wmap_land_effect_24_sequence_8_step, g_wmap_land_effect_24_sequence_8_timer, g_wmap_effect_model_b_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x80, wmap_land_effect_24_sequence_8_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_8_step_03, g_wmap_land_effect_24_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_9, D_800D7814, 4, g_wmap_land_effect_24_sequence_9_step, g_wmap_land_effect_24_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_9_step_00, g_wmap_land_effect_24_sequence_9_step, g_wmap_land_effect_24_sequence_9_timer)

WMAP_STEP_DROP_START(wmap_land_effect_24_sequence_9_step_01, g_wmap_land_effect_24_sequence_9_step, g_wmap_land_effect_24_sequence_9_timer, g_wmap_effect_model_c_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_c_position, g_wmap_effect_fade_c, 0x80, 0xAFC8, 0x10, wmap_land_effect_24_sequence_9_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_9_step_03, g_wmap_land_effect_24_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_10, D_800D7824, 6, g_wmap_land_effect_24_sequence_10_step, g_wmap_land_effect_24_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_10_step_00, g_wmap_land_effect_24_sequence_10_step, g_wmap_land_effect_24_sequence_10_timer)

void wmap_land_effect_24_sequence_10_step_02(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0x3C, 0xFF, 1, 2, 0, (WmapState*)g_wmap_effect_params);
    timer = g_wmap_land_effect_24_sequence_10_timer - 1;
    g_wmap_land_effect_24_sequence_10_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_10_step++;
    }
}

void wmap_land_effect_24_sequence_10_step_03(void)
{
    g_wmap_land_effect_24_sequence_10_timer = 0x80;
    g_wmap_effect_params[5] = -1;
    g_wmap_land_effect_24_sequence_10_step++;
    wmap_land_effect_24_sequence_10_step_04();
}

void wmap_land_effect_24_sequence_10_step_04(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0x3C, 0xFF, 1, 2, 0, (WmapState*)g_wmap_effect_params);
    timer = g_wmap_land_effect_24_sequence_10_timer - 1;
    g_wmap_land_effect_24_sequence_10_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_10_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_10_step_05, g_wmap_land_effect_24_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_11, D_800D783C, 6, g_wmap_land_effect_24_sequence_11_step, g_wmap_land_effect_24_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_11_step_00, g_wmap_land_effect_24_sequence_11_step, g_wmap_land_effect_24_sequence_11_timer)

void wmap_land_effect_24_sequence_11_step_02(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800DA028, D_80139C08, 0x1E, 0xFF, 1, 4, 0, (u8*)g_wmap_effect_params + 0x28);
    timer = g_wmap_land_effect_24_sequence_11_timer - 1;
    g_wmap_land_effect_24_sequence_11_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_11_step++;
    }
}

void wmap_land_effect_24_sequence_11_step_03(void)
{
    g_wmap_land_effect_24_sequence_11_timer = 0x40;
    g_wmap_effect_params[15] = -1;
    g_wmap_land_effect_24_sequence_11_step++;
    wmap_land_effect_24_sequence_11_step_04();
}

void wmap_land_effect_24_sequence_11_step_04(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800DA028, D_80139C08, 0x1E, 0xFF, 1, 4, 0, (u8*)g_wmap_effect_params + 0x28);
    timer = g_wmap_land_effect_24_sequence_11_timer - 1;
    g_wmap_land_effect_24_sequence_11_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_11_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_11_step_05, g_wmap_land_effect_24_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_12, D_800D7854, 8, g_wmap_land_effect_24_sequence_12_step, g_wmap_land_effect_24_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_12_step_00, g_wmap_land_effect_24_sequence_12_step, g_wmap_land_effect_24_sequence_12_timer)

void wmap_land_effect_24_sequence_12_step_02(void)
{
    s32 timer;

    func_8006B328(0x78, 0x9C, 2, -1, -3, -8, 0, 0x1F, -0xB4, 0x190, -0xA0, 0x190, 0, 0x80, 0, 8, 2);
    D_801B25DC += 8;
    timer = g_wmap_land_effect_24_sequence_12_timer - 1;
    g_wmap_land_effect_24_sequence_12_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_12_step++;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_24_sequence_12_step_03, g_wmap_land_effect_24_sequence_12_step, g_wmap_land_effect_24_sequence_12_timer, 0x12,
                    wmap_land_effect_24_sequence_12_step_04)

void wmap_land_effect_24_sequence_12_step_04(void)
{
    s32 timer;

    func_8006B328(0x78, 0x9C, 2, -1, -3, -8, 0, 0x1F, -0xB4, 0x190, -0xA0, 0x190, 0, 0x80, 0, 8, 2);
    timer = g_wmap_land_effect_24_sequence_12_timer - 1;
    g_wmap_land_effect_24_sequence_12_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_12_step++;
    }
}

void wmap_land_effect_24_sequence_12_step_05(void)
{
    D_800DCEB0 = 0;
    g_wmap_land_effect_24_sequence_12_timer = 0x64;
    g_wmap_land_effect_24_sequence_12_step++;
    wmap_land_effect_24_sequence_12_step_06();
}

void wmap_land_effect_24_sequence_12_step_06(void)
{
    s32 timer;

    func_8006B328(0x78, 0x9C, 2, -1, -3, -8, 0, 0x1F, -0xB4, 0x190, -0xA0, 0x190, 0, 0x80, 0, 8, 2);
    timer = g_wmap_land_effect_24_sequence_12_timer - 1;
    g_wmap_land_effect_24_sequence_12_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_12_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_12_step_07, g_wmap_land_effect_24_sequence_12_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_13, D_800D7874, 6, g_wmap_land_effect_24_sequence_13_step, g_wmap_land_effect_24_sequence_13_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_13_step_00, g_wmap_land_effect_24_sequence_13_step, g_wmap_land_effect_24_sequence_13_timer)

void wmap_land_effect_24_sequence_13_step_01(void)
{
    D_80139240 = 0;
    g_wmap_effect_fade_d = 1;
    g_wmap_land_effect_24_sequence_13_timer = 0x5A;
    g_wmap_land_effect_24_sequence_13_step++;
    wmap_land_effect_24_sequence_13_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_24_sequence_13_step_03, g_wmap_land_effect_24_sequence_13_step, g_wmap_land_effect_24_sequence_13_timer, 0x20,
                    wmap_land_effect_24_sequence_13_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_13_step_05, g_wmap_land_effect_24_sequence_13_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_14, D_800D788C, 6, g_wmap_land_effect_24_sequence_14_step, g_wmap_land_effect_24_sequence_14_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_14_step_00, g_wmap_land_effect_24_sequence_14_step, g_wmap_land_effect_24_sequence_14_timer)

void wmap_land_effect_24_sequence_14_step_01(void)
{
    D_8013924C = 0;
    D_801B2670 = g_wmap_zero_rotation;
    D_801B25D8 = 0;
    g_wmap_land_effect_24_sequence_14_timer = 0x58;
    g_wmap_land_effect_24_sequence_14_step++;
    wmap_land_effect_24_sequence_14_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_24_sequence_14_step_03, g_wmap_land_effect_24_sequence_14_step, g_wmap_land_effect_24_sequence_14_timer, 8,
                    wmap_land_effect_24_sequence_14_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_14_step_05, g_wmap_land_effect_24_sequence_14_step)

WMAP_STEP_RUNNER(wmap_land_effect_24_run_sequence_15, D_800D78A4, 6, g_wmap_land_effect_24_sequence_15_step, g_wmap_land_effect_24_sequence_15_timer)

WMAP_STEP_RESET(wmap_land_effect_24_sequence_15_step_00, g_wmap_land_effect_24_sequence_15_step, g_wmap_land_effect_24_sequence_15_timer)

void wmap_land_effect_24_sequence_15_step_01(void)
{
    D_801399DC = &g_wmap_animation_bank_2;
    D_80139250 = 0x780;
    D_800D9420.scale_index = 0xF;
    D_800D9420.previous_sequence = -1;
    D_800D9420.shade_step = 4;
    D_800D9420.target_shade = 0x81;
    D_800D9420.shade = 1;
    D_800D9420.resource_index = 0;
    D_800D9420.sequence = 0;
    g_wmap_land_effect_24_sequence_15_timer = 0xE1;
    D_80182D58.packed = g_wmap_focus_screen_position.packed;
    g_wmap_land_effect_24_sequence_15_step++;
    wmap_land_effect_24_sequence_15_step_02();
}

void wmap_land_effect_24_sequence_15_step_02(void)
{
    s32 value;
    s32 adjusted;
    s32 timer;

    wmap_step_actor_animation(&D_800D9420, &D_801399D8);
    wmap_draw_actor_sprite(&D_800D9420, D_80182D58.packed, 8, 0x14, 0);

    value = D_80139250;
    adjusted = value;
    if (value < 0)
    {
        adjusted = value + 0xF;
    }
    D_80182D58.point.y = adjusted >> 4;
    if (value >= 0x321)
    {
        D_80139250 = value - 0x10;
    }

    timer = g_wmap_land_effect_24_sequence_15_timer - 1;
    g_wmap_land_effect_24_sequence_15_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_15_step++;
    }
}

void wmap_land_effect_24_sequence_15_step_03(void)
{
    D_800D9420.shade_step = 8;
    D_800D9420.target_shade = 0;
    g_wmap_land_effect_24_sequence_15_timer = 0x10;
    g_wmap_land_effect_24_sequence_15_step++;
    wmap_land_effect_24_sequence_15_step_04();
}

void wmap_land_effect_24_sequence_15_step_04(void)
{
    s32 value;
    s32 adjusted;
    s32 timer;

    wmap_step_actor_animation(&D_800D9420, &D_801399D8);
    wmap_draw_actor_sprite(&D_800D9420, D_80182D58.packed, 8, 0x14, 0);

    value = D_80139250;
    adjusted = value;
    if (value < 0)
    {
        adjusted = value + 0xF;
    }
    D_80182D58.point.y = adjusted >> 4;
    if (value >= 0x321)
    {
        D_80139250 = value - 0x10;
    }

    timer = g_wmap_land_effect_24_sequence_15_timer - 1;
    g_wmap_land_effect_24_sequence_15_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_24_sequence_15_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_24_sequence_15_step_05, g_wmap_land_effect_24_sequence_15_step)
