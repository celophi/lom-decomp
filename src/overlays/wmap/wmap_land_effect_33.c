#include "wmap_model_render.h"
#include "wmap_land_effect_33.h"
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

/** @brief Activity flag in a world-map motion slot. */
typedef struct
{
    s16 active;
    u8 unknown_02[0x12];
} WmapEffectMotionSlot;

void wmap_land_effect_33_sequence_6_step_02(void);
void wmap_land_effect_33_sequence_7_step_02(void);
s32 wmap_land_effect_33_run_timeline(s32 reset);
void wmap_land_effect_33_step_02(void);
void wmap_land_effect_33_step_03(void);
s32 wmap_land_effect_33_run_sequence_2(s32 reset);
s32 wmap_land_effect_33_run_sequence_3(s32 reset);
s32 wmap_land_effect_33_run_sequence_4(s32 reset);
s32 wmap_land_effect_33_run_sequence_5(s32 reset);
s32 wmap_land_effect_33_run_sequence_6(s32 reset);
s32 wmap_land_effect_33_run_sequence_7(s32 reset);
s32 wmap_land_effect_33_run_sequence_8(s32 reset);
s32 wmap_land_effect_33_run_sequence_9(s32 reset);
s32 wmap_land_effect_33_run_sequence_10(s32 reset);
s32 wmap_land_effect_33_run_sequence_1(s32 reset);
void wmap_land_effect_33_sequence_1_step_02(void);
void wmap_land_effect_33_sequence_1_step_04(void);
void wmap_land_effect_33_sequence_2_step_02(void);
void wmap_land_effect_33_sequence_2_step_04(void);
void wmap_land_effect_33_sequence_3_step_02(void);
void wmap_land_effect_33_sequence_3_step_04(void);
void wmap_land_effect_33_sequence_4_step_02(void);
void wmap_land_effect_33_sequence_4_step_04(void);
void wmap_land_effect_33_sequence_5_step_02(void);
void wmap_land_effect_33_sequence_5_step_04(void);
void wmap_land_effect_33_sequence_6_step_04(void);
void wmap_land_effect_33_sequence_7_step_04(void);
void wmap_land_effect_33_sequence_8_step_04(void);
void wmap_land_effect_33_sequence_9_step_02(void);
void wmap_land_effect_33_sequence_9_step_04(void);
void wmap_land_effect_33_sequence_8_step_02(void);

typedef struct
{
    s32 value;
    u8 pad[0x24];
} WmapCell;

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

typedef void (*WmapHandler)(void);

extern u32 g_wmap_land_effect_33_sequence_6_step;
extern s32 g_wmap_land_effect_33_sequence_6_timer;
extern u32 g_wmap_land_effect_33_sequence_7_step;
extern s32 g_wmap_land_effect_33_sequence_7_timer;
extern WmapHandler D_800D78BC[];
extern WmapHandler D_800D78CC[];
extern WmapHandler D_800D7924[];
extern WmapHandler D_800D793C[];
extern WmapHandler D_800D7954[];
extern WmapHandler D_800D796C[];
extern WmapHandler D_800D7984[];
extern WmapHandler D_800D799C[];
extern WmapHandler D_800D79B4[];
extern WmapHandler D_800D79CC[];
extern WmapHandler D_800D79E4[];
extern WmapHandler D_800D79FC[];
extern u8 D_800D95D8[];
extern u8 D_800DA028[];
extern u8 D_800DAA78[];
extern u8 D_800DB310[];
extern s32 D_800DBE70;
extern s32 g_wmap_load_buffer[];
extern u8 g_wmap_animation_bank_3;
extern u8 g_wmap_animation_bank_4;
extern u8 g_wmap_animation_bank_5;
extern s32 D_80139228;
extern WmapCell g_wmap_cells[][6];
extern WmapAnimationSlot D_80139A28[];
extern u8 D_80139C08[];
extern u8 D_80139DE8[];
extern u8 D_80139F78[];
extern s32 D_80139978;
extern s32 D_8013B294;
extern u32 g_wmap_land_effect_33_step;
extern s32 g_wmap_land_effect_33_timer;
extern u32 g_wmap_land_effect_33_timeline_step;
extern s32 g_wmap_land_effect_33_timeline_timer;
extern u32 g_wmap_land_effect_33_sequence_1_step;
extern s32 g_wmap_land_effect_33_sequence_1_timer;
extern u32 g_wmap_land_effect_33_sequence_2_step;
extern s32 g_wmap_land_effect_33_sequence_2_timer;
extern u32 g_wmap_land_effect_33_sequence_3_step;
extern s32 g_wmap_land_effect_33_sequence_3_timer;
extern u32 g_wmap_land_effect_33_sequence_4_step;
extern s32 g_wmap_land_effect_33_sequence_4_timer;
extern u32 g_wmap_land_effect_33_sequence_5_step;
extern s32 g_wmap_land_effect_33_sequence_5_timer;
extern u32 g_wmap_land_effect_33_sequence_8_step;
extern s32 g_wmap_land_effect_33_sequence_8_timer;
extern u32 g_wmap_land_effect_33_sequence_9_step;
extern s32 g_wmap_land_effect_33_sequence_9_timer;
extern u32 g_wmap_land_effect_33_sequence_10_step;
extern s32 g_wmap_land_effect_33_sequence_10_timer;
extern void akao_fade_all_sfx_volume(s32, s32);
extern void wmap_land_effect_33_sequence_10_step_02(void);
extern void wmap_land_effect_33_step_02(void);
extern void wmap_land_effect_33_step_03(void);
extern s32 wmap_land_effect_33_run_timeline(s32);
extern s32 wmap_land_effect_33_run_sequence_1(s32);
extern void wmap_land_effect_33_sequence_1_step_02(void);
extern void wmap_land_effect_33_sequence_1_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_2(s32);
extern void wmap_land_effect_33_sequence_2_step_02(void);
extern void wmap_land_effect_33_sequence_2_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_3(s32);
extern void wmap_land_effect_33_sequence_3_step_02(void);
extern void wmap_land_effect_33_sequence_3_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_4(s32);
extern void wmap_land_effect_33_sequence_4_step_02(void);
extern void wmap_land_effect_33_sequence_4_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_5(s32);
extern void wmap_land_effect_33_sequence_5_step_02(void);
extern void wmap_land_effect_33_sequence_5_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_6(s32);
extern void wmap_land_effect_33_sequence_6_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_7(s32);
extern void wmap_land_effect_33_sequence_7_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_8(s32);
extern void wmap_land_effect_33_sequence_8_step_02(void);
extern void wmap_land_effect_33_sequence_8_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_9(s32);
extern void wmap_land_effect_33_sequence_9_step_02(void);
extern void wmap_land_effect_33_sequence_9_step_04(void);
extern s32 wmap_land_effect_33_run_sequence_10(s32);
extern u8 g_wmap_animation_bank_2[];

extern VECTOR g_wmap_camera_translation;


extern WmapSpriteActor D_800D939C;
extern WmapSpriteActor D_800D93C8;
extern WmapSpriteActor D_800D93F4;

extern WmapAnimationSlot g_wmap_actor_animations[];
extern WmapAnimationSlot D_801399C0;
extern WmapAnimationSlot D_801399C8;
extern WmapAnimationSlot D_801399D0;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* g_wmap_effect_params;

extern WmapEffectMotionSlot g_wmap_actor_motions[];

/** @brief Configure the first world-map effect state and reset its fifty resource slots. */
void wmap_land_effect_33_sequence_6_step_01(void)
{
    s32 i;

    g_wmap_effect_params[1] = 0;
    g_wmap_effect_params[2] = 3;
    g_wmap_effect_params[3] = 0x20;
    g_wmap_effect_params[4] = 0;
    g_wmap_effect_params[5] = 2;
    g_wmap_effect_params[6] = 0x190;
    g_wmap_effect_params[7] = 0x14;
    g_wmap_effect_params[8] = 0x2C;
    g_wmap_effect_params[9] = 3;
    g_wmap_effect_params[10] = 0x32C8;

    for (i = 0; i < 50; i++)
    {
        g_wmap_actor_motions[i + 20].active = 0;
        g_wmap_actor_animations[i + 20].data = g_wmap_animation_bank_2;
    }

    g_wmap_land_effect_33_sequence_6_timer = 100;
    g_wmap_land_effect_33_sequence_6_step++;
    wmap_land_effect_33_sequence_6_step_02();
}

/** @brief Configure the second world-map effect state and reset its forty resource slots. */
void wmap_land_effect_33_sequence_7_step_01(void)
{
    s32 i;

    g_wmap_effect_params[11] = 1;
    g_wmap_effect_params[12] = 2;
    g_wmap_effect_params[13] = 0x80;
    g_wmap_effect_params[14] = 0;
    g_wmap_effect_params[15] = 1;
    g_wmap_effect_params[16] = 0x190;
    g_wmap_effect_params[17] = 0x50;
    g_wmap_effect_params[18] = 0x2C;
    g_wmap_effect_params[19] = 4;
    g_wmap_effect_params[20] = 0x4650;

    for (i = 0; i < 40; i++)
    {
        g_wmap_actor_motions[i + 80].active = 0;
        g_wmap_actor_animations[i + 80].data = g_wmap_animation_bank_2;
    }

    g_wmap_land_effect_33_sequence_7_timer = 40;
    g_wmap_land_effect_33_sequence_7_step++;
    wmap_land_effect_33_sequence_7_step_02();
}

void wmap_land_effect_33_sequence_8_step_01(void)
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
    resource = &g_wmap_animation_bank_4;
    screen_offset = 0x460;
    config_offset = 0xAF0;
    g_wmap_effect_params[21] = 0;
    g_wmap_effect_params[22] = 0;
    g_wmap_effect_params[23] = 0x80;
    g_wmap_effect_params[24] = 0;
    g_wmap_effect_params[25] = 3;
    g_wmap_effect_params[26] = 0x3E8;
    g_wmap_effect_params[27] = 0x8C;
    g_wmap_effect_params[28] = 0x13;
    g_wmap_effect_params[29] = 0;
    g_wmap_effect_params[30] = 0x32C8;

    do
    {
        screen_entry = (u8*)(screen_offset + (s32)screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + (s32)config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        *(u8**)(screen_entry + 4) = resource;
    } while (index < 34);

    g_wmap_land_effect_33_sequence_8_timer = 102;
    g_wmap_land_effect_33_sequence_8_step++;
    wmap_land_effect_33_sequence_8_step_02();
}

void wmap_land_effect_33_sequence_9_step_01(void)
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
    screen_offset = 0x5F0;
    config_offset = 0xED8;
    g_wmap_effect_params[41] = 1;
    g_wmap_effect_params[42] = 3;
    g_wmap_effect_params[43] = 0x40;
    g_wmap_effect_params[44] = 0;
    g_wmap_effect_params[45] = 3;
    g_wmap_effect_params[46] = -0x1C2;
    g_wmap_effect_params[47] = 0xBE;
    g_wmap_effect_params[48] = 0x2C;
    g_wmap_effect_params[49] = 5;
    g_wmap_effect_params[50] = 0x6D60;

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

    g_wmap_land_effect_33_sequence_9_timer = 120;
    g_wmap_land_effect_33_sequence_9_step++;
    wmap_land_effect_33_sequence_9_step_02();
}

void wmap_land_effect_33_sequence_10_step_02(void)
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

    timer = g_wmap_land_effect_33_sequence_10_timer - 1;
    g_wmap_land_effect_33_sequence_10_timer = timer;
    if (timer == 0)
    {
        g_wmap_land_effect_33_sequence_10_step++;
    }
}

WMAP_STEP_RUNNER(wmap_land_effect_33_run, D_800D78BC, 4, g_wmap_land_effect_33_step, g_wmap_land_effect_33_timer)

WMAP_STEP_RESET(wmap_land_effect_33_step_00, g_wmap_land_effect_33_step, g_wmap_land_effect_33_timer)

void wmap_land_effect_33_step_01(void)
{
    D_800DBE70 = 0;
    wmap_reset_focus_screen_position();
    g_wmap_focus_screen_position.point.x = 0xA4;
    g_wmap_focus_screen_position.point.y = 0x69;
    wmap_start_sequence(wmap_land_effect_33_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_33_step++;
    wmap_land_effect_33_step_02();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_33_step_02, g_wmap_land_effect_33_step, wmap_land_effect_33_step_03)

void wmap_land_effect_33_step_03(void)
{
    D_8013B294 = 1;
    D_80139228 = 1;
    g_wmap_land_effect_33_step++;
}

WMAP_STEP_RUNNER(wmap_land_effect_33_run_timeline, D_800D78CC, 0x16, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_33_timeline_step_00, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

void wmap_land_effect_33_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    g_wmap_placement_overlay_hidden = 1;
    wmap_play_sound(0x36, 0x80);
    g_wmap_land_effect_33_timeline_timer = 0xF;
    g_wmap_land_effect_33_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_02, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_33_timeline_step_03, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer,
                         wmap_land_effect_33_run_sequence_9, 0x23)

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_04, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

void wmap_land_effect_33_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_33_run_sequence_10);
    wmap_start_map_tint(0x703040);
    g_wmap_backdrop_target_level = 0xA;
    g_wmap_land_effect_33_timeline_timer = 0x19;
    g_wmap_land_effect_33_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_06, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_33_timeline_step_07, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer,
                             wmap_land_effect_33_run_sequence_1, wmap_land_effect_33_run_sequence_7, 0x20)

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_08, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

void wmap_land_effect_33_timeline_step_09(void)
{
    D_80139978 = -1;
    wmap_start_sequence(wmap_land_effect_33_run_sequence_3);
    g_wmap_land_effect_33_timeline_timer = 0x36;
    g_wmap_land_effect_33_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_10, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_33_timeline_step_11, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer,
                         wmap_land_effect_33_run_sequence_6, 0x45)

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_12, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_33_timeline_step_13, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer,
                         wmap_land_effect_33_run_sequence_2, 0x12)

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_14, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_33_timeline_step_15, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer,
                         wmap_land_effect_33_run_sequence_8, 0x2B)

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_16, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

void wmap_land_effect_33_timeline_step_17(void)
{
    wmap_start_sequence(wmap_land_effect_33_run_sequence_5);
    wmap_start_sequence(wmap_land_effect_33_run_sequence_4);
    wmap_start_sequence(wmap_land_effect_33_run_sequence_10);
    g_wmap_land_effect_33_timeline_timer = 0x64;
    g_wmap_land_effect_33_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_18, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

void wmap_land_effect_33_timeline_step_19(void)
{
    g_wmap_cells[g_wmap_focus_cell_x][g_wmap_focus_cell_y].value = 0x121;
    akao_fade_all_sfx_volume(0x3C, 0);
    g_wmap_land_effect_33_timeline_timer = 0x5A;
    g_wmap_land_effect_33_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_33_timeline_step_20, g_wmap_land_effect_33_timeline_step, g_wmap_land_effect_33_timeline_timer)

void wmap_land_effect_33_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    g_wmap_land_effect_33_timeline_step++;
}

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_1, D_800D7924, 6, g_wmap_land_effect_33_sequence_1_step, g_wmap_land_effect_33_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_1_step_00, g_wmap_land_effect_33_sequence_1_step, g_wmap_land_effect_33_sequence_1_timer)

void wmap_land_effect_33_sequence_1_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    g_wmap_actor_animations[5].data = &g_wmap_animation_bank_5;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 4;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->target_shade = 0x80;
    actor->shade = 0;
    g_wmap_land_effect_33_sequence_1_timer = 0xA0;
    g_wmap_land_effect_33_sequence_1_step++;
    wmap_land_effect_33_sequence_1_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_1_step_02, g_wmap_land_effect_33_sequence_1_step, g_wmap_land_effect_33_sequence_1_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x2C, 8, 0)

void wmap_land_effect_33_sequence_1_step_03(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    actor->shade_step = 0x80;
    actor->target_shade = 0;
    g_wmap_land_effect_33_sequence_1_timer = 1;
    g_wmap_land_effect_33_sequence_1_step++;
    wmap_land_effect_33_sequence_1_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_1_step_04, g_wmap_land_effect_33_sequence_1_step, g_wmap_land_effect_33_sequence_1_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x2C, 8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_1_step_05, g_wmap_land_effect_33_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_2, D_800D793C, 6, g_wmap_land_effect_33_sequence_2_step, g_wmap_land_effect_33_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_2_step_00, g_wmap_land_effect_33_sequence_2_step, g_wmap_land_effect_33_sequence_2_timer)

void wmap_land_effect_33_sequence_2_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    g_wmap_actor_animations[6].data = &g_wmap_animation_bank_3;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->shade_step = 0x80;
    actor->target_shade = 0x80;
    actor->shade = 0;
    g_wmap_land_effect_33_sequence_2_timer = 0x3E;
    g_wmap_land_effect_33_sequence_2_step++;
    wmap_land_effect_33_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_2_step_02, g_wmap_land_effect_33_sequence_2_step, g_wmap_land_effect_33_sequence_2_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x2C, 8, 0)

void wmap_land_effect_33_sequence_2_step_03(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    actor->shade_step = 0x80;
    actor->target_shade = 0;
    g_wmap_land_effect_33_sequence_2_timer = 1;
    g_wmap_land_effect_33_sequence_2_step++;
    wmap_land_effect_33_sequence_2_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_2_step_04, g_wmap_land_effect_33_sequence_2_step, g_wmap_land_effect_33_sequence_2_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x2C, 8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_2_step_05, g_wmap_land_effect_33_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_3, D_800D7954, 6, g_wmap_land_effect_33_sequence_3_step, g_wmap_land_effect_33_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_3_step_00, g_wmap_land_effect_33_sequence_3_step, g_wmap_land_effect_33_sequence_3_timer)

void wmap_land_effect_33_sequence_3_step_01(void)
{
    g_wmap_actor_animations[7].data = g_wmap_animation_bank_2;
    D_800D939C.scale_index = 0xF;
    D_800D939C.previous_sequence = -1;
    D_800D939C.shade_step = 0x80;
    D_800D939C.target_shade = 0x81;
    D_800D939C.resource_index = 0;
    D_800D939C.sequence = 0;
    D_800D939C.shade = 1;
    g_wmap_land_effect_33_sequence_3_timer = 0xBC;
    g_wmap_land_effect_33_sequence_3_step++;
    wmap_land_effect_33_sequence_3_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_3_step_02, g_wmap_land_effect_33_sequence_3_step, g_wmap_land_effect_33_sequence_3_timer, D_800D939C,
                              D_801399C0, g_wmap_focus_screen_position, 0x2E, 0xC, 0)

void wmap_land_effect_33_sequence_3_step_03(void)
{
    D_800D939C.shade_step = 0x80;
    D_800D939C.target_shade = 0;
    g_wmap_land_effect_33_sequence_3_timer = 1;
    g_wmap_land_effect_33_sequence_3_step++;
    wmap_land_effect_33_sequence_3_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_3_step_04, g_wmap_land_effect_33_sequence_3_step, g_wmap_land_effect_33_sequence_3_timer, D_800D939C,
                              D_801399C0, g_wmap_focus_screen_position, 0x2E, 0xC, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_3_step_05, g_wmap_land_effect_33_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_4, D_800D796C, 6, g_wmap_land_effect_33_sequence_4_step, g_wmap_land_effect_33_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_4_step_00, g_wmap_land_effect_33_sequence_4_step, g_wmap_land_effect_33_sequence_4_timer)

void wmap_land_effect_33_sequence_4_step_01(void)
{
    s32 value;

    g_wmap_actor_animations[8].data = g_wmap_animation_bank_2;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.sequence = value = 1;
    D_800D93C8.previous_sequence = -value;
    D_800D93C8.shade_step = 8;
    D_800D93C8.shade = value;
    D_800D93C8.resource_index = 0;
    D_800D93C8.target_shade = 0x81;
    g_wmap_land_effect_33_sequence_4_timer = 0x65;
    g_wmap_land_effect_33_sequence_4_step++;
    wmap_land_effect_33_sequence_4_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_4_step_02, g_wmap_land_effect_33_sequence_4_step, g_wmap_land_effect_33_sequence_4_timer, D_800D93C8,
                              D_801399C8, g_wmap_focus_screen_position, 0x2E, 0xC, 0)

void wmap_land_effect_33_sequence_4_step_03(void)
{
    D_800D93C8.shade_step = 0x80;
    D_800D93C8.target_shade = 0;
    g_wmap_land_effect_33_sequence_4_timer = 1;
    g_wmap_land_effect_33_sequence_4_step++;
    wmap_land_effect_33_sequence_4_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_4_step_04, g_wmap_land_effect_33_sequence_4_step, g_wmap_land_effect_33_sequence_4_timer, D_800D93C8,
                              D_801399C8, g_wmap_focus_screen_position, 0x2E, 0xC, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_4_step_05, g_wmap_land_effect_33_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_5, D_800D7984, 6, g_wmap_land_effect_33_sequence_5_step, g_wmap_land_effect_33_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_5_step_00, g_wmap_land_effect_33_sequence_5_step, g_wmap_land_effect_33_sequence_5_timer)

void wmap_land_effect_33_sequence_5_step_01(void)
{
    g_wmap_actor_animations[9].data = g_wmap_animation_bank_2;
    D_800D93F4.scale_index = 0xF;
    D_800D93F4.sequence = 2;
    D_800D93F4.previous_sequence = -1;
    D_800D93F4.shade_step = 4;
    D_800D93F4.resource_index = 0;
    D_800D93F4.target_shade = 0x80;
    D_800D93F4.shade = 0;
    g_wmap_land_effect_33_sequence_5_timer = 0x66;
    g_wmap_land_effect_33_sequence_5_step++;
    wmap_land_effect_33_sequence_5_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_5_step_02, g_wmap_land_effect_33_sequence_5_step, g_wmap_land_effect_33_sequence_5_timer, D_800D93F4,
                              D_801399D0, g_wmap_focus_screen_position, 0x2C, 8, 0)

void wmap_land_effect_33_sequence_5_step_03(void)
{
    D_800D93F4.shade_step = 2;
    D_800D93F4.target_shade = 0;
    g_wmap_land_effect_33_sequence_5_timer = 0x40;
    g_wmap_land_effect_33_sequence_5_step++;
    wmap_land_effect_33_sequence_5_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_33_sequence_5_step_04, g_wmap_land_effect_33_sequence_5_step, g_wmap_land_effect_33_sequence_5_timer, D_800D93F4,
                              D_801399D0, g_wmap_focus_screen_position, 0x2C, 8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_5_step_05, g_wmap_land_effect_33_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_6, D_800D799C, 6, g_wmap_land_effect_33_sequence_6_step, g_wmap_land_effect_33_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_6_step_00, g_wmap_land_effect_33_sequence_6_step, g_wmap_land_effect_33_sequence_6_timer)

void wmap_land_effect_33_sequence_6_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0x32, 0xFF, 1, 8, 0, (WmapState*)g_wmap_effect_params);
    if (--g_wmap_land_effect_33_sequence_6_timer == 0)
    {
        g_wmap_land_effect_33_sequence_6_step++;
    }
}

void wmap_land_effect_33_sequence_6_step_03(void)
{
    g_wmap_land_effect_33_sequence_6_timer = 0x20;
    g_wmap_effect_params[5] = -1;
    g_wmap_land_effect_33_sequence_6_step++;
    wmap_land_effect_33_sequence_6_step_04();
}

void wmap_land_effect_33_sequence_6_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0x32, 0xFF, 1, 8, 0, (WmapState*)g_wmap_effect_params);
    if (--g_wmap_land_effect_33_sequence_6_timer == 0)
    {
        g_wmap_land_effect_33_sequence_6_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_6_step_05, g_wmap_land_effect_33_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_7, D_800D79B4, 6, g_wmap_land_effect_33_sequence_7_step, g_wmap_land_effect_33_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_7_step_00, g_wmap_land_effect_33_sequence_7_step, g_wmap_land_effect_33_sequence_7_timer)

void wmap_land_effect_33_sequence_7_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA028, D_80139C08, 0x28, 0xFF, 1, 2, 0, &g_wmap_effect_params[10]);
    if (--g_wmap_land_effect_33_sequence_7_timer == 0)
    {
        g_wmap_land_effect_33_sequence_7_step++;
    }
}

void wmap_land_effect_33_sequence_7_step_03(void)
{
    g_wmap_land_effect_33_sequence_7_timer = 0x80;
    g_wmap_effect_params[15] = -1;
    g_wmap_land_effect_33_sequence_7_step++;
    wmap_land_effect_33_sequence_7_step_04();
}

void wmap_land_effect_33_sequence_7_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA028, D_80139C08, 0x28, 0xFF, 1, 2, 0, &g_wmap_effect_params[10]);
    if (--g_wmap_land_effect_33_sequence_7_timer == 0)
    {
        g_wmap_land_effect_33_sequence_7_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_7_step_05, g_wmap_land_effect_33_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_8, D_800D79CC, 6, g_wmap_land_effect_33_sequence_8_step, g_wmap_land_effect_33_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_8_step_00, g_wmap_land_effect_33_sequence_8_step, g_wmap_land_effect_33_sequence_8_timer)

void wmap_land_effect_33_sequence_8_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DAA78, D_80139DE8, 0x22, 0xFF, 1, 2, 0, (WmapState*)g_wmap_effect_params + 1);
    if (--g_wmap_land_effect_33_sequence_8_timer == 0)
    {
        g_wmap_land_effect_33_sequence_8_step++;
    }
}

void wmap_land_effect_33_sequence_8_step_03(void)
{
    g_wmap_land_effect_33_sequence_8_timer = 0x80;
    g_wmap_effect_params[25] = -1;
    g_wmap_land_effect_33_sequence_8_step++;
    wmap_land_effect_33_sequence_8_step_04();
}

void wmap_land_effect_33_sequence_8_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DAA78, D_80139DE8, 0x22, 0xFF, 1, 2, 0, (WmapState*)g_wmap_effect_params + 1);
    if (--g_wmap_land_effect_33_sequence_8_timer == 0)
    {
        g_wmap_land_effect_33_sequence_8_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_8_step_05, g_wmap_land_effect_33_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_9, D_800D79E4, 6, g_wmap_land_effect_33_sequence_9_step, g_wmap_land_effect_33_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_9_step_00, g_wmap_land_effect_33_sequence_9_step, g_wmap_land_effect_33_sequence_9_timer)

void wmap_land_effect_33_sequence_9_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB310, D_80139F78, 0x28, 0xFF, 1, 4, 0, (WmapState*)g_wmap_effect_params + 2);
    if (--g_wmap_land_effect_33_sequence_9_timer == 0)
    {
        g_wmap_land_effect_33_sequence_9_step++;
    }
}

void wmap_land_effect_33_sequence_9_step_03(void)
{
    g_wmap_land_effect_33_sequence_9_timer = 0x40;
    g_wmap_effect_params[45] = -1;
    g_wmap_land_effect_33_sequence_9_step++;
    wmap_land_effect_33_sequence_9_step_04();
}

void wmap_land_effect_33_sequence_9_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB310, D_80139F78, 0x28, 0xFF, 1, 4, 0, (WmapState*)g_wmap_effect_params + 2);
    if (--g_wmap_land_effect_33_sequence_9_timer == 0)
    {
        g_wmap_land_effect_33_sequence_9_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_9_step_05, g_wmap_land_effect_33_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_33_run_sequence_10, D_800D79FC, 4, g_wmap_land_effect_33_sequence_10_step, g_wmap_land_effect_33_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_33_sequence_10_step_00, g_wmap_land_effect_33_sequence_10_step, g_wmap_land_effect_33_sequence_10_timer)

WMAP_STEP_DROP_START(wmap_land_effect_33_sequence_10_step_01, g_wmap_land_effect_33_sequence_10_step, g_wmap_land_effect_33_sequence_10_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40, wmap_land_effect_33_sequence_10_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_33_sequence_10_step_03, g_wmap_land_effect_33_sequence_10_step)
