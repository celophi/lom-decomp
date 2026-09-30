#include "wmap_main.h"
#include "wmap_land_effect_12.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"
#include "wmap_step_sequence.h"

void wmap_land_effect_12_sequence_5_step_02(void);
void wmap_land_effect_12_sequence_6_step_02(void);
void wmap_land_effect_12_sequence_7_step_02(void);
void wmap_land_effect_12_wait_idle_02(void);
void wmap_land_effect_12_step_03(void);
s32 wmap_land_effect_12_run_timeline(s32 arg0);
void wmap_land_effect_12_wait_idle_04(void);
void wmap_land_effect_12_end(void);
s32 wmap_land_effect_12_run_sequence_1(s32 arg0);
s32 wmap_land_effect_12_run_sequence_4(s32 arg0);
s32 wmap_land_effect_12_run_sequence_5(s32 arg0);
s32 wmap_land_effect_12_run_sequence_3(s32 arg0);
s32 wmap_land_effect_12_run_sequence_6(s32 arg0);
s32 wmap_land_effect_12_run_sequence_2(s32 arg0);
s32 wmap_land_effect_12_run_sequence_7(s32 arg0);
void wmap_land_effect_12_sequence_1_step_02(void);
void wmap_land_effect_12_sequence_2_step_02(void);
void wmap_land_effect_12_sequence_5_step_04(void);
void wmap_land_effect_12_sequence_6_step_04(void);
void wmap_land_effect_12_sequence_7_step_04(void);

/** @brief World-map actor configuration. */
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

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 field_00;
    s16 angle;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    s16 field_10;
    s16 pad_12;
} WmapMotion;

/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

extern u8* g_wmap_effect_model_pack_1;
extern s32 D_801B2468;
extern s32 g_wmap_land_effect_12_sequence_3_timer;
extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_12_sequence_4_timer;
extern u8 g_wmap_animation_bank_2[];
extern s32 g_wmap_land_effect_12_sequence_5_timer;
extern s32 g_wmap_land_effect_12_sequence_6_timer;
extern s32 g_wmap_land_effect_12_sequence_7_timer;
extern s32 g_wmap_land_effect_12_timer;
extern void (*D_800D5888[])(void);
extern void wmap_land_effect_12_step_03(void);
extern void wmap_land_effect_12_end(void);
extern s32 g_wmap_land_effect_12_timeline_timer;
extern void (*D_800D58A0[])(void);
extern WmapValueRecord g_wmap_cells[][6];
extern u32 g_wmap_selected_artifact;
extern s32 g_wmap_land_effect_12_sequence_1_timer;
extern void (*D_800D58D0[])(void);
extern u8 g_wmap_animation_bank_1[];
extern u8* D_801399AC;
extern void wmap_land_effect_12_sequence_1_step_02(void);
extern s32 g_wmap_land_effect_12_sequence_2_timer;
extern void (*D_800D58E0[])(void);
extern u8 g_wmap_animation_bank_0[];
extern u8* D_801399B4;
extern void wmap_land_effect_12_sequence_2_step_02(void);
extern void (*D_800D58F0[])(void);
extern s32 D_801B24B0;
extern void (*D_800D5908[])(void);
extern void (*D_800D5918[])(void);
extern u8 D_800DA398[];
extern u8 D_80139CA8[];
extern void (*D_800D5930[])(void);
extern u8 D_800D95D8[];
extern WmapAnimationSlot D_80139A28[];
extern void wmap_land_effect_12_sequence_6_step_04(void);
extern void (*D_800D5948[])(void);
extern u32 g_wmap_land_effect_12_sequence_3_step;
extern u32 g_wmap_land_effect_12_sequence_4_step;
extern u32 g_wmap_land_effect_12_sequence_5_step;
extern u32 g_wmap_land_effect_12_sequence_6_step;
extern u32 g_wmap_land_effect_12_sequence_7_step;
extern u32 g_wmap_land_effect_12_step;
extern u32 g_wmap_land_effect_12_timeline_step;
extern u32 g_wmap_land_effect_12_sequence_1_step;
extern u32 g_wmap_land_effect_12_sequence_2_step;

extern VECTOR g_wmap_camera_translation;

extern SVECTOR D_801B2498;
extern SVECTOR D_801B2490;

extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;

extern WmapAnimationSlot g_wmap_actor_animations[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* g_wmap_effect_params;

extern WmapMotion g_wmap_actor_motions[];

/**
 * @brief World-map step handler: draw two overlaid actor sprites within a matrix push,
 *        ramp the shared size up to a cap, then countdown-advance the step.
 */
void wmap_land_effect_12_sequence_3_step_02(void)
{
    s32 value;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    D_801B2490.vz += 0xC;
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    D_801B2498.vz += 0x4;
    PopMatrix();
    value = D_801B2468 + 2;
    D_801B2468 = value;
    if (value >= 0x82)
    {
        D_801B2468 = 0x81;
    }
    if (--g_wmap_land_effect_12_sequence_3_timer == 0)
    {
        g_wmap_land_effect_12_sequence_3_step += 1;
    }
}

/**
 * @brief World-map step handler: draw two frames of the animated actor, scroll each
 *        sub-field, decay the shared frame index with a floor, then advance the step.
 */
void wmap_land_effect_12_sequence_3_step_04(void)
{
    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    D_801B2490.vz += 0xC;
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    D_801B2498.vz += 4;
    PopMatrix();
    D_801B2468 -= 1;
    if (D_801B2468 < 0)
    {
        D_801B2468 = 0;
    }
    if (--g_wmap_land_effect_12_sequence_3_timer == 0)
    {
        g_wmap_land_effect_12_sequence_3_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_12_sequence_4_step_02(void)
{
    MATRIX m;
    s32 x;

    x = g_wmap_effect_model_a_position.vz - 0xDAC;
    g_wmap_effect_model_a_position.vz = x;
    if (x < 0x2710)
    {
        g_wmap_effect_model_a_position.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&g_wmap_effect_model_a_rotation, &m);
    TransMatrix(&m, &g_wmap_zero_translation);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (g_wmap_effect_fade_a != 0)
    {
        wmap_draw_model_default((s32)g_wmap_load_buffer, 0, 0x4, 0x35, 0x7800, 0x1, g_wmap_effect_fade_a);
        g_wmap_effect_fade_a -= 0x4;
        if (g_wmap_effect_fade_a < 0)
        {
            g_wmap_effect_fade_a = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_12_sequence_4_timer == 0)
    {
        g_wmap_land_effect_12_sequence_4_step += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_12_sequence_5_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 100;
    g_wmap_effect_params[0x1] = 2;
    g_wmap_effect_params[0x2] = 4;
    g_wmap_effect_params[0x3] = 0x20;
    g_wmap_effect_params[0x4] = 8;
    g_wmap_effect_params[0x5] = 4;
    g_wmap_effect_params[0x6] = 0x3E8;
    g_wmap_effect_params[0x7] = 0x64;
    g_wmap_effect_params[0x8] = 8;
    g_wmap_effect_params[0x9] = 2;
    g_wmap_effect_params[0xA] = 0xFA0;
    for (i = 0; i < 100; i++)
    {
        g_wmap_actor_motions[i + 100].field_00 = 0;
        g_wmap_actor_animations[i + 100].data = g_wmap_animation_bank_2;
    }
    g_wmap_land_effect_12_sequence_5_timer = 144;
    g_wmap_land_effect_12_sequence_5_step++;
    wmap_land_effect_12_sequence_5_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_12_sequence_6_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 5;
    g_wmap_effect_params[0xB] = 4;
    g_wmap_effect_params[0xC] = 2;
    g_wmap_effect_params[0xD] = 32;
    g_wmap_effect_params[0xE] = 1;
    g_wmap_effect_params[0xF] = 2;
    g_wmap_effect_params[0x10] = 2000;
    g_wmap_effect_params[0x11] = 20;
    g_wmap_effect_params[0x12] = 8;
    g_wmap_effect_params[0x13] = 1;
    g_wmap_effect_params[0x14] = 1000;
    for (i = 0; i < 5; i++)
    {
        g_wmap_actor_motions[i + 20].field_00 = 0;
        g_wmap_actor_animations[i + 20].data = g_wmap_animation_bank_2;
    }
    g_wmap_land_effect_12_sequence_6_timer = 10;
    g_wmap_land_effect_12_sequence_6_step++;
    wmap_land_effect_12_sequence_6_step_02();
}

/** @brief Initialize the actor group and its animation resources, then advance. */
void wmap_land_effect_12_sequence_7_step_01(void)
{
    s32 i;
    WmapConfigA* config;

    g_wmap_particle_intensity = 5;
    for (i = 200; i < 205; i++)
    {
        config = &g_wmap_sprite_actors[i];
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_2;
        config->field_02 = 0;
        config->field_06 = 15;
        config->field_0E = 0;
        config->field_10 = -1;
        config->field_22 = 127;
        config->field_24 = 1;
        config->field_26 = 8;
        g_wmap_actor_motions[i].field_00 = 1;
        g_wmap_actor_motions[i].angle = i * 0x333;
        g_wmap_actor_motions[i].field_08 = 380000;
        g_wmap_actor_motions[i].field_04 = 0;
        g_wmap_actor_motions[i].field_0C = 9999;
        g_wmap_actor_motions[i].field_0E = 0;
        g_wmap_actor_motions[i].field_10 = 80;
    }
    g_wmap_land_effect_12_sequence_7_timer = 64;
    g_wmap_land_effect_12_sequence_7_step++;
    wmap_land_effect_12_sequence_7_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_12_run, D_800D5888, 0x6, g_wmap_land_effect_12_step, g_wmap_land_effect_12_timer)

WMAP_STEP_RESET(wmap_land_effect_12_reset, g_wmap_land_effect_12_step, g_wmap_land_effect_12_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_12_step_01, g_wmap_land_effect_12_step, wmap_run_land_focus, wmap_land_effect_12_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_12_wait_idle_02, g_wmap_land_effect_12_step, wmap_land_effect_12_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_12_step_03, g_wmap_land_effect_12_step, wmap_land_effect_12_run_timeline, wmap_land_effect_12_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_12_wait_idle_04, g_wmap_land_effect_12_step, wmap_land_effect_12_end)

WMAP_STEP_ADVANCE(wmap_land_effect_12_end, g_wmap_land_effect_12_step)

WMAP_STEP_RUNNER(wmap_land_effect_12_run_timeline, D_800D58A0, 0xC, g_wmap_land_effect_12_timeline_step, g_wmap_land_effect_12_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_12_timeline_reset, g_wmap_land_effect_12_timeline_step, g_wmap_land_effect_12_timeline_timer)

/** @brief World-map state entry: load resources, register callbacks, advance. */
void wmap_land_effect_12_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x304010);
    g_wmap_backdrop_target_level = 4;
    wmap_play_sound(0x1F, 0x80);
    wmap_start_sequence(wmap_land_effect_12_run_sequence_1);
    wmap_start_sequence(wmap_land_effect_12_run_sequence_4);
    g_wmap_land_effect_12_timeline_timer = 8;
    g_wmap_land_effect_12_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_12_timeline_wait_02, g_wmap_land_effect_12_timeline_step, g_wmap_land_effect_12_timeline_timer)

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void wmap_land_effect_12_timeline_step_03(void)
{
    g_wmap_placement_overlay_hidden = 1;
    wmap_start_sequence(wmap_land_effect_12_run_sequence_5);
    g_wmap_land_effect_12_timeline_timer = 0x18;
    g_wmap_land_effect_12_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_12_timeline_wait_04, g_wmap_land_effect_12_timeline_step, g_wmap_land_effect_12_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_12_timeline_step_05, g_wmap_land_effect_12_timeline_step, g_wmap_land_effect_12_timeline_timer,
                         wmap_land_effect_12_run_sequence_3, 0x3C)

WMAP_STEP_WAIT(wmap_land_effect_12_timeline_wait_06, g_wmap_land_effect_12_timeline_step, g_wmap_land_effect_12_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_12_timeline_step_07, g_wmap_land_effect_12_timeline_step, g_wmap_land_effect_12_timeline_timer,
                         wmap_land_effect_12_run_sequence_6, 0x1E)

WMAP_STEP_WAIT(wmap_land_effect_12_timeline_wait_08, g_wmap_land_effect_12_timeline_step, g_wmap_land_effect_12_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_12_timeline_step_09, g_wmap_land_effect_12_timeline_step, g_wmap_land_effect_12_timeline_timer,
                             wmap_land_effect_12_run_sequence_2, wmap_land_effect_12_run_sequence_7, 0x78)

WMAP_STEP_WAIT(wmap_land_effect_12_timeline_wait_10, g_wmap_land_effect_12_timeline_step, g_wmap_land_effect_12_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_12_timeline_finish, g_wmap_land_effect_12_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y, g_wmap_selected_artifact)

WMAP_STEP_RUNNER(wmap_land_effect_12_run_sequence_1, D_800D58D0, 0x4, g_wmap_land_effect_12_sequence_1_step, g_wmap_land_effect_12_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_12_sequence_1_reset, g_wmap_land_effect_12_sequence_1_step, g_wmap_land_effect_12_sequence_1_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_12_sequence_1_step_01(void)
{
    D_801399AC = g_wmap_animation_bank_1;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 8;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.target_shade = 0x80;
    D_800D9318.shade = 0;
    g_wmap_land_effect_12_sequence_1_timer = 0x7E;
    g_wmap_land_effect_12_sequence_1_step += 1;
    wmap_land_effect_12_sequence_1_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_12_sequence_1_step_02, g_wmap_land_effect_12_sequence_1_step, g_wmap_land_effect_12_sequence_1_timer, D_800D9318,
                              D_801399A8, g_wmap_focus_screen_position, 0x19, 0xE, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_12_sequence_1_end, g_wmap_land_effect_12_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_12_run_sequence_2, D_800D58E0, 0x4, g_wmap_land_effect_12_sequence_2_step, g_wmap_land_effect_12_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_12_sequence_2_reset, g_wmap_land_effect_12_sequence_2_step, g_wmap_land_effect_12_sequence_2_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_12_sequence_2_step_01(void)
{
    D_801399B4 = g_wmap_animation_bank_0;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 8;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0x80;
    g_wmap_land_effect_12_sequence_2_timer = 0x79;
    g_wmap_land_effect_12_sequence_2_step += 1;
    wmap_land_effect_12_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_12_sequence_2_step_02, g_wmap_land_effect_12_sequence_2_step, g_wmap_land_effect_12_sequence_2_timer, D_800D9344,
                              D_801399B0, g_wmap_focus_screen_position, 0x1A, 0xF, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_12_sequence_2_end, g_wmap_land_effect_12_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_12_run_sequence_3, D_800D58F0, 0x6, g_wmap_land_effect_12_sequence_3_step, g_wmap_land_effect_12_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_12_sequence_3_reset, g_wmap_land_effect_12_sequence_3_step, g_wmap_land_effect_12_sequence_3_timer)

/** @brief World-map step: reset counters and advance to the next handler. */
void wmap_land_effect_12_sequence_3_step_01(void)
{
    D_801B24B0 = 0;
    D_801B2468 = 1;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    D_801B2498.vx = 0;
    D_801B2498.vy = 0;
    D_801B2498.vz = 0;
    g_wmap_land_effect_12_sequence_3_timer = 0x40;
    g_wmap_land_effect_12_sequence_3_step += 1;
    wmap_land_effect_12_sequence_3_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_12_sequence_3_step_03, g_wmap_land_effect_12_sequence_3_step, g_wmap_land_effect_12_sequence_3_timer, 0x80,
                    wmap_land_effect_12_sequence_3_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_12_sequence_3_end, g_wmap_land_effect_12_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_12_run_sequence_4, D_800D5908, 0x4, g_wmap_land_effect_12_sequence_4_step, g_wmap_land_effect_12_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_12_sequence_4_reset, g_wmap_land_effect_12_sequence_4_step, g_wmap_land_effect_12_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_12_sequence_4_step_01, g_wmap_land_effect_12_sequence_4_step, g_wmap_land_effect_12_sequence_4_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x20, wmap_land_effect_12_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_12_sequence_4_end, g_wmap_land_effect_12_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_12_run_sequence_5, D_800D5918, 0x6, g_wmap_land_effect_12_sequence_5_step, g_wmap_land_effect_12_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_12_sequence_5_reset, g_wmap_land_effect_12_sequence_5_step, g_wmap_land_effect_12_sequence_5_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_12_sequence_5_step_02(void)
{
    func_8006A2FC(D_800DA398, D_80139CA8, 0x64, 0, 0x7F, 0x4, 0, (s32)g_wmap_effect_params);
    if (--g_wmap_land_effect_12_sequence_5_timer == 0)
    {
        g_wmap_land_effect_12_sequence_5_step += 1;
    }
}

void wmap_land_effect_12_sequence_5_step_03(void)
{
    g_wmap_land_effect_12_sequence_5_timer = 0x20;
    g_wmap_effect_params[5] = -1;
    g_wmap_land_effect_12_sequence_5_step += 1;
    wmap_land_effect_12_sequence_5_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_12_sequence_5_step_04(void)
{
    func_8006A2FC(D_800DA398, D_80139CA8, 0x64, 0, 0x7F, 0x4, 0, (s32)g_wmap_effect_params);
    if (--g_wmap_land_effect_12_sequence_5_timer == 0)
    {
        g_wmap_land_effect_12_sequence_5_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_12_sequence_5_end, g_wmap_land_effect_12_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_12_run_sequence_6, D_800D5930, 0x6, g_wmap_land_effect_12_sequence_6_step, g_wmap_land_effect_12_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_12_sequence_6_reset, g_wmap_land_effect_12_sequence_6_step, g_wmap_land_effect_12_sequence_6_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_12_sequence_6_step_02(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0x5, 0, 0x7F, 0x4, 0, (s32)((u8*)g_wmap_effect_params + 0x28));
    if (--g_wmap_land_effect_12_sequence_6_timer == 0)
    {
        g_wmap_land_effect_12_sequence_6_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_12_sequence_6_step_03(void)
{
    g_wmap_land_effect_12_sequence_6_timer = 0x20;
    g_wmap_effect_params[15] = -1;
    g_wmap_land_effect_12_sequence_6_step += 1;
    wmap_land_effect_12_sequence_6_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_12_sequence_6_step_04(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0x5, 0, 0x7F, 0x4, 0, (s32)((u8*)g_wmap_effect_params + 0x28));
    if (--g_wmap_land_effect_12_sequence_6_timer == 0)
    {
        g_wmap_land_effect_12_sequence_6_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_12_sequence_6_end, g_wmap_land_effect_12_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_12_run_sequence_7, D_800D5948, 0x6, g_wmap_land_effect_12_sequence_7_step, g_wmap_land_effect_12_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_12_sequence_7_reset, g_wmap_land_effect_12_sequence_7_step, g_wmap_land_effect_12_sequence_7_timer)

/**
 * @brief Draw the world-map cursor via func_8006B6EC, then advance after the wait expires.
 */
void wmap_land_effect_12_sequence_7_step_02(void)
{
    func_8006B6EC(0xC8, 0xCD, 0x8, 0, 0xE);
    if (--g_wmap_land_effect_12_sequence_7_timer == 0)
    {
        g_wmap_land_effect_12_sequence_7_step += 1;
    }
}

/**
 * @brief Reset a range of world-map actor configs, arm the timer, and advance the step.
 */
void wmap_land_effect_12_sequence_7_step_03(void)
{
    s32 i;

    for (i = 0xC8; i < 0xCD; i++)
    {
        g_wmap_sprite_actors[i].target_shade = 0;
        g_wmap_sprite_actors[i].shade_step = 8;
    }
    g_wmap_land_effect_12_sequence_7_timer = 0x10;
    g_wmap_land_effect_12_sequence_7_step += 1;
    wmap_land_effect_12_sequence_7_step_04();
}

/**
 * @brief Draw the world-map cursor via func_8006B6EC, then advance after the wait expires.
 */
void wmap_land_effect_12_sequence_7_step_04(void)
{
    func_8006B6EC(0xC8, 0xCD, 0x8, 0, 0xE);
    if (--g_wmap_land_effect_12_sequence_7_timer == 0)
    {
        g_wmap_land_effect_12_sequence_7_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_12_sequence_7_end, g_wmap_land_effect_12_sequence_7_step)
