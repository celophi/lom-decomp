#include "internal/wmap_main.h"
#include "internal/wmap_land_effect_05.h"
#include "internal/wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "internal/wmap_resource_support.h"
#include "internal/wmap_view_effects.h"
#include "internal/wmap_sprite_render.h"
#include "internal/wmap_effect_primitives.h"
#include "internal/wmap_step_sequence.h"
#include "internal/wmap_cells.h"

void wmap_land_effect_05_sequence_3_step_02(void);
void wmap_land_effect_05_sequence_10_step_02(void);
void wmap_land_effect_05_wait_idle_02(void);
void wmap_land_effect_05_step_03(void);
s32 wmap_land_effect_05_run_timeline(s32 arg0);
void wmap_land_effect_05_wait_idle_04(void);
void wmap_land_effect_05_end(void);
s32 wmap_land_effect_05_run_sequence_3(s32 arg0);
s32 wmap_land_effect_05_run_sequence_1(s32 arg0);
s32 wmap_land_effect_05_run_sequence_8(s32 arg0);
s32 wmap_land_effect_05_run_sequence_4(s32 arg0);
s32 wmap_land_effect_05_run_sequence_5(s32 arg0);
s32 wmap_land_effect_05_run_sequence_6(s32 arg0);
s32 wmap_land_effect_05_run_sequence_7(s32 arg0);
s32 wmap_land_effect_05_run_sequence_10(s32 arg0);
s32 wmap_land_effect_05_run_sequence_9(s32 arg0);
s32 wmap_land_effect_05_run_sequence_2(s32 arg0);
void wmap_land_effect_05_sequence_1_step_02(void);
void wmap_land_effect_05_sequence_2_step_02(void);
void wmap_land_effect_05_sequence_3_step_04(void);
void wmap_land_effect_05_sequence_10_step_04(void);
void wmap_land_effect_05_sequence_10_step_06(void);

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

extern s32 g_wmap_land_effect_05_sequence_3_timer;
extern s8 D_80051B4C[];
extern void *g_wmap_effect_model_pack_2;
extern s32 D_80182DE4;
extern s32 D_801B2468;
extern s32 D_801B24B4;
extern s32 g_wmap_land_effect_05_sequence_4_timer;
extern u8* g_wmap_effect_model_pack_3;
extern s32 g_wmap_land_effect_05_sequence_5_timer;
extern u8* D_8011CF2C;
extern s32 D_801B25D8;
extern s32 g_wmap_land_effect_05_sequence_6_timer;
extern s32 D_801B25DC;
extern s32 g_wmap_land_effect_05_sequence_7_timer;
extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_05_sequence_8_timer;
extern u8* g_wmap_effect_model_pack_1;
extern s32 g_wmap_land_effect_05_sequence_9_timer;
extern s32 D_800D9150;
extern s32 D_800DCEA8;
extern s32 g_wmap_land_effect_05_sequence_10_timer;
extern s32 g_wmap_land_effect_05_timer;
extern void (*D_800D5D98[])(void);
extern void wmap_land_effect_05_step_03(void);
extern void wmap_land_effect_05_end(void);
extern s32 g_wmap_land_effect_05_timeline_timer;
extern void (*D_800D5DB0[])(void);
extern u32 g_wmap_selected_artifact;
extern s32 g_wmap_land_effect_05_sequence_1_timer;
extern void (*D_800D5E08[])(void);
extern u8 g_wmap_animation_bank_0[];
extern void wmap_land_effect_05_sequence_1_step_02(void);
extern s32 g_wmap_land_effect_05_sequence_2_timer;
extern void (*D_800D5E18[])(void);
extern u8 g_wmap_animation_bank_1[];
extern void wmap_land_effect_05_sequence_2_step_02(void);
extern void (*D_800D5E28[])(void);
extern void (*D_800D5E40[])(void);
extern void (*D_800D5E58[])(void);
extern void (*D_800D5E70[])(void);
extern void (*D_800D5E88[])(void);
extern void (*D_800D5EA0[])(void);
extern void (*D_800D5EB0[])(void);
extern void (*D_800D5EC0[])(void);
extern u8 g_wmap_animation_bank_2[];
extern u32 g_wmap_land_effect_05_sequence_3_step;
extern u32 g_wmap_land_effect_05_sequence_4_step;
extern u32 g_wmap_land_effect_05_sequence_5_step;
extern u32 g_wmap_land_effect_05_sequence_6_step;
extern u8* D_8011CF30;
extern u32 g_wmap_land_effect_05_sequence_7_step;
extern u32 g_wmap_land_effect_05_sequence_8_step;
extern u32 g_wmap_land_effect_05_sequence_9_step;
extern u32 g_wmap_land_effect_05_sequence_10_step;
extern u32 g_wmap_land_effect_05_step;
extern u32 g_wmap_land_effect_05_timeline_step;
extern u32 g_wmap_land_effect_05_sequence_1_step;
extern u32 g_wmap_land_effect_05_sequence_2_step;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_80139898;
extern VECTOR D_801B2660;

extern SVECTOR D_801B2498;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2670;
extern SVECTOR D_801B2678;


extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;

extern WmapMotion g_wmap_actor_motions[];

/** @brief Initialize the actor group and its animation resources, then advance. */
void wmap_land_effect_05_sequence_3_step_01(void)
{
    s32 i;
    WmapSpriteActor* config;

    g_wmap_particle_intensity = 5;
    for (i = 200; i < 205; i++)
    {
        config = &g_wmap_sprite_actors[i];
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_2;
        config->resource_index = 0;
        config->scale_index = 15;
        config->sequence = 0;
        config->previous_sequence = -1;
        config->target_shade = 63;
        config->shade = 2;
        config->shade_step = 2;
        g_wmap_actor_motions[i].field_00 = 1;
        g_wmap_actor_motions[i].angle = i * 0x333;
        g_wmap_actor_motions[i].field_08 = 340000;
        g_wmap_actor_motions[i].field_04 = 0;
        g_wmap_actor_motions[i].field_0C = 9999;
        g_wmap_actor_motions[i].field_0E = 0;
        g_wmap_actor_motions[i].field_10 = 80;
    }
    g_wmap_land_effect_05_sequence_3_timer = 180;
    g_wmap_land_effect_05_sequence_3_step++;
    wmap_land_effect_05_sequence_3_step_02();
}

/** @brief Draw two oscillating effect layers and update their intensity. */
void wmap_land_effect_05_sequence_4_step_02(void)
{
    s32 first_frame;
    s32 intensity;
    s32 remaining;

    PushMatrix();
    first_frame = (s32) (D_80051B4C[D_801B24B4] + 0x80) >> 5;
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(g_wmap_effect_model_pack_2, first_frame, 0x14, 0x35, 0x7800, 0x1000, D_801B2468);
    first_frame = (s32) (D_80051B4C[D_80182DE4] + 0x80) >> 5;
    D_801B2490.vz = (u16) (D_801B2490.vz - 0x18);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(g_wmap_effect_model_pack_2, first_frame, 0x14, 0x35, 0x7800, 0x1000, D_801B2468);
    D_801B2498.vz = (u16) (D_801B2498.vz + 0x30);
    PopMatrix();
    D_801B24B4 = (D_801B24B4 + 4) & 0xFF;
    D_80182DE4 = (D_80182DE4 + 2) & 0xFF;
    WMAP_MODEL_FADE_IN(D_801B2468, 1, 0x80, intensity);
    remaining = g_wmap_land_effect_05_sequence_4_timer - 1;
    g_wmap_land_effect_05_sequence_4_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_05_sequence_4_step += 1;
    }
}

/** @brief Animate and fade two counter-rotating effect layers. */
void wmap_land_effect_05_sequence_4_step_04(void)
{
    s32 first_frame;
    
    s32 intensity;
    s32 remaining;

    PushMatrix();
    first_frame = (s32) (D_80051B4C[D_801B24B4] + 0x80) >> 5;
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(g_wmap_effect_model_pack_2, first_frame, 0x14, 0x35, 0x7800, 0x1000, D_801B2468);
    first_frame = (s32) (D_80051B4C[D_80182DE4] + 0x80) >> 5;
    D_801B2490.vz = (u16) (D_801B2490.vz - 0x18);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(g_wmap_effect_model_pack_2, first_frame, 0x14, 0x35, 0x7800, 0x1000, D_801B2468);
    D_801B2498.vz = (u16) (D_801B2498.vz + 0x30);
    PopMatrix();
    D_801B24B4 = (D_801B24B4 + 4) & 0xFF;
    D_80182DE4 = (D_80182DE4 + 2) & 0xFF;
    WMAP_MODEL_FADE_OUT(D_801B2468, 4, intensity);
    remaining = g_wmap_land_effect_05_sequence_4_timer - 1;
    g_wmap_land_effect_05_sequence_4_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_05_sequence_4_step += 1;
    }
}

/**
 * @brief World-map step handler: decay a timer with a floor, draw the model, ramp a
 *        secondary counter to its cap, scroll the field, then countdown-advance the step.
 */
void wmap_land_effect_05_sequence_5_step_02(void)
{
    s32 v;

    v = g_wmap_effect_model_d_position.vz - 0x5DC;
    g_wmap_effect_model_d_position.vz = v;
    if (v < 0x7530)
    {
        g_wmap_effect_model_d_position.vz = 0x7530;
    }
    PushMatrix();
    wmap_set_model_transform(&g_wmap_effect_model_d_position, &g_wmap_effect_model_d_rotation);
    wmap_draw_model_default(g_wmap_effect_model_pack_3, 0, 0xC, 0x35, 0x7840, 1, g_wmap_effect_fade_d);
    g_wmap_effect_fade_d += 1;
    if (g_wmap_effect_fade_d >= 0x42)
    {
        g_wmap_effect_fade_d = 0x41;
    }
    g_wmap_effect_model_d_rotation.vz += 2;
    PopMatrix();
    if (--g_wmap_land_effect_05_sequence_5_timer == 0)
    {
        g_wmap_land_effect_05_sequence_5_step += 1;
    }
}

/**
 * @brief World-map step handler: clamp a fade level, draw a sprite, and expire the step.
 */
void wmap_land_effect_05_sequence_5_step_04(void)
{
    u8 *p;
    u8 *q;

    p = &g_wmap_effect_model_d_position;
    if (*(s32 *)(p + 8) < 0x7530)
    {
        *(s32 *)(p + 8) = 0x7530;
    }
    PushMatrix();
    q = &g_wmap_effect_model_d_rotation;
    wmap_set_model_transform(p, q);
    if (g_wmap_effect_fade_d != 0)
    {
        wmap_draw_model_default(g_wmap_effect_model_pack_3, 0, 0xC, 0x35, 0x7840, 1, g_wmap_effect_fade_d);
        g_wmap_effect_fade_d -= 1;
        if (g_wmap_effect_fade_d < 0)
        {
            g_wmap_effect_fade_d = 0;
        }
        *(s16 *)(q + 4) += 2;
    }
    PopMatrix();
    if (--g_wmap_land_effect_05_sequence_5_timer == 0)
    {
        g_wmap_land_effect_05_sequence_5_step += 1;
    }
}

/**
 * @brief World-map step handler: decay a timer with a floor, draw the model, ramp a
 *        secondary counter to its cap, scroll the field, then countdown-advance the step.
 */
void wmap_land_effect_05_sequence_6_step_02(void)
{
    s32 v;

    v = D_80139898.vz - 0x5DC;
    D_80139898.vz = v;
    if (v < 0x7530)
    {
        D_80139898.vz = 0x7530;
    }
    PushMatrix();
    wmap_set_model_transform(&D_80139898, &D_801B2670);
    wmap_draw_model_default(D_8011CF2C, 0, 0xC, 0x35, 0x7840, 1, D_801B25D8);
    D_801B25D8 += 1;
    if (D_801B25D8 >= 0x42)
    {
        D_801B25D8 = 0x41;
    }
    D_801B2670.vz -= 3;
    PopMatrix();
    if (--g_wmap_land_effect_05_sequence_6_timer == 0)
    {
        g_wmap_land_effect_05_sequence_6_step += 1;
    }
}

/**
 * @brief World-map step handler: clamp a fade level, draw a sprite, and expire the step.
 */
void wmap_land_effect_05_sequence_6_step_04(void)
{
    u8 *p;
    u8 *q;

    p = &D_80139898;
    if (*(s32 *)(p + 8) < 0x7530)
    {
        *(s32 *)(p + 8) = 0x7530;
    }
    PushMatrix();
    q = &D_801B2670;
    wmap_set_model_transform(p, q);
    if (D_801B25D8 != 0)
    {
        wmap_draw_model_default(D_8011CF2C, 0, 0xC, 0x35, 0x7840, 1, D_801B25D8);
        D_801B25D8 -= 1;
        if (D_801B25D8 < 0)
        {
            D_801B25D8 = 0;
        }
        *(s16 *)(q + 4) += -3;
    }
    PopMatrix();
    if (--g_wmap_land_effect_05_sequence_6_timer == 0)
    {
        g_wmap_land_effect_05_sequence_6_step += 1;
    }
}

/**
 * @brief World-map step handler: decay a timer with a floor, draw the model, ramp a
 *        secondary counter to its cap, scroll the field, then countdown-advance the step.
 */
void wmap_land_effect_05_sequence_7_step_02(void)
{
    s32 v;

    v = D_801B2660.vz - 0x5DC;
    D_801B2660.vz = v;
    if (v < 0x7530)
    {
        D_801B2660.vz = 0x7530;
    }
    PushMatrix();
    wmap_set_model_transform(&D_801B2660, &D_801B2678);
    wmap_draw_model_default(D_8011CF30, 0, 0xC, 0x35, 0x7840, 1, D_801B25DC);
    D_801B25DC += 1;
    if (D_801B25DC >= 0x42)
    {
        D_801B25DC = 0x41;
    }
    D_801B2678.vz += 4;
    PopMatrix();
    if (--g_wmap_land_effect_05_sequence_7_timer == 0)
    {
        g_wmap_land_effect_05_sequence_7_step += 1;
    }
}

/** @brief Draw the fading effect and advance its countdown. */
void wmap_land_effect_05_sequence_7_step_04(void)
{
    s32 intensity;
    s32 remaining;

    if (D_801B2660.vz < 0x7530)
    {
        D_801B2660.vz = 0x7530;
    }
    PushMatrix();
    wmap_set_model_transform(&D_801B2660, &D_801B2678);
    if (D_801B25DC != 0)
    {
        wmap_draw_model_default(D_8011CF30, 0, 0xC, 0x35, 0x7840, 1, D_801B25DC);
        WMAP_MODEL_FADE_OUT(D_801B25DC, 1, intensity);
        D_801B2678.vz = (u16) (D_801B2678.vz + 4);
    }
    PopMatrix();
    remaining = g_wmap_land_effect_05_sequence_7_timer - 1;
    g_wmap_land_effect_05_sequence_7_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_05_sequence_7_step += 1;
    }
}

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_05_sequence_8_step_02,
    g_wmap_land_effect_05_sequence_8_step, g_wmap_land_effect_05_sequence_8_timer,
    g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position,
    g_wmap_effect_fade_a, g_wmap_load_buffer, -3500, 2)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_05_sequence_9_step_02,
    g_wmap_land_effect_05_sequence_9_step, g_wmap_land_effect_05_sequence_9_timer,
    g_wmap_effect_model_b_rotation, g_wmap_effect_model_b_position,
    g_wmap_effect_fade_b, g_wmap_effect_model_pack_1, -3500, 2)

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void wmap_land_effect_05_sequence_10_step_01(void)
{
    s32 i;

    i = 100;
    g_wmap_effect_fade_c = 1;
    D_800DCEA8 = 1;

    do
    {
        g_wmap_actor_motions[i].field_00 = 0;
        WMAP_INIT_PARTICLE_ACTOR(i, g_wmap_animation_bank_2, 1);
        i++;
    } while (i < 200);

    D_800D9150 = 1;
    g_wmap_land_effect_05_sequence_10_timer = 0x10;
    g_wmap_land_effect_05_sequence_10_step += 1;
    wmap_land_effect_05_sequence_10_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_05_run, D_800D5D98, 0x6, g_wmap_land_effect_05_step, g_wmap_land_effect_05_timer)

WMAP_STEP_RESET(wmap_land_effect_05_reset, g_wmap_land_effect_05_step, g_wmap_land_effect_05_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_05_step_01, g_wmap_land_effect_05_step, wmap_run_land_focus, wmap_land_effect_05_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_05_wait_idle_02, g_wmap_land_effect_05_step, wmap_land_effect_05_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_05_step_03, g_wmap_land_effect_05_step, wmap_land_effect_05_run_timeline, wmap_land_effect_05_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_05_wait_idle_04, g_wmap_land_effect_05_step, wmap_land_effect_05_end)

WMAP_STEP_ADVANCE(wmap_land_effect_05_end, g_wmap_land_effect_05_step)

WMAP_STEP_RUNNER(wmap_land_effect_05_run_timeline, D_800D5DB0, 0x16, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_05_timeline_reset, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer)

/** @brief World-map step: mark active, request a resource, seed the sub-counter, tick. */
void wmap_land_effect_05_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_play_sound(0x25, 0x80);
    g_wmap_land_effect_05_timeline_timer = 8;
    g_wmap_land_effect_05_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_05_timeline_wait_02, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_05_timeline_step_03, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer,
                         wmap_land_effect_05_run_sequence_3, 0x1E)

WMAP_STEP_WAIT(wmap_land_effect_05_timeline_wait_04, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer)

/** @brief Register two callbacks around a color and state update and begin a two-tick delay. */
void wmap_land_effect_05_timeline_step_05(void)
{
    wmap_start_sequence(&wmap_land_effect_05_run_sequence_8);
    wmap_start_map_tint(0x501040);
    g_wmap_backdrop_target_level = 4;
    g_wmap_placement_overlay_hidden = 1;
    wmap_start_sequence(&wmap_land_effect_05_run_sequence_1);
    g_wmap_land_effect_05_timeline_timer = 2;
    g_wmap_land_effect_05_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_05_timeline_wait_06, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer)

/**
 * @brief Hide the transition mesh and set the wait timer.
 */
WMAP_STEP_HIDE_AND_WAIT(wmap_land_effect_05_timeline_step_07,
    g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer,
    g_wmap_transition_mesh_hidden, 0x16)

WMAP_STEP_WAIT(wmap_land_effect_05_timeline_wait_08, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_05_timeline_step_09, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer,
                             wmap_land_effect_05_run_sequence_4, wmap_land_effect_05_run_sequence_5, 0x4)

WMAP_STEP_WAIT(wmap_land_effect_05_timeline_wait_10, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_05_timeline_step_11, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer,
                         wmap_land_effect_05_run_sequence_6, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_05_timeline_wait_12, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_05_timeline_step_13, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer,
                         wmap_land_effect_05_run_sequence_7, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_05_timeline_wait_14, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_05_timeline_step_15, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer,
                         wmap_land_effect_05_run_sequence_10, 0x74)

WMAP_STEP_WAIT(wmap_land_effect_05_timeline_wait_16, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_05_timeline_step_17, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer,
                             wmap_land_effect_05_run_sequence_9, wmap_land_effect_05_run_sequence_2, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_05_timeline_wait_18, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer)

/** @brief Clear the world-map value, set the drawing color, and start a 140-tick delay. */
void wmap_land_effect_05_timeline_step_19(void)
{
    g_wmap_transition_mesh_hidden = 0;
    wmap_start_map_tint(0x403060);
    g_wmap_land_effect_05_timeline_timer = 0x8C;
    g_wmap_land_effect_05_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_05_timeline_wait_20, g_wmap_land_effect_05_timeline_step, g_wmap_land_effect_05_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_05_timeline_finish, g_wmap_land_effect_05_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y, g_wmap_selected_artifact)

WMAP_STEP_RUNNER(wmap_land_effect_05_run_sequence_1, D_800D5E08, 0x4, g_wmap_land_effect_05_sequence_1_step, g_wmap_land_effect_05_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_05_sequence_1_reset, g_wmap_land_effect_05_sequence_1_step, g_wmap_land_effect_05_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_05_sequence_1_step_01,
    g_wmap_land_effect_05_sequence_1_step, g_wmap_land_effect_05_sequence_1_timer,
    4, g_wmap_animation_bank_0, 0,
    1, 0x81, 8,
    0x96, wmap_land_effect_05_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_05_sequence_1_step_02, g_wmap_land_effect_05_sequence_1_step, g_wmap_land_effect_05_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0xF, 0x6, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_05_sequence_1_end, g_wmap_land_effect_05_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_05_run_sequence_2, D_800D5E18, 0x4, g_wmap_land_effect_05_sequence_2_step, g_wmap_land_effect_05_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_05_sequence_2_reset, g_wmap_land_effect_05_sequence_2_step, g_wmap_land_effect_05_sequence_2_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_05_sequence_2_step_01,
    g_wmap_land_effect_05_sequence_2_step, g_wmap_land_effect_05_sequence_2_timer,
    5, g_wmap_animation_bank_1, 0,
    0, 0x80, 8,
    0x90, wmap_land_effect_05_sequence_2_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_05_sequence_2_step_02, g_wmap_land_effect_05_sequence_2_step, g_wmap_land_effect_05_sequence_2_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x18, 0xA, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_05_sequence_2_end, g_wmap_land_effect_05_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_05_run_sequence_3, D_800D5E28, 0x6, g_wmap_land_effect_05_sequence_3_step, g_wmap_land_effect_05_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_05_sequence_3_reset, g_wmap_land_effect_05_sequence_3_step, g_wmap_land_effect_05_sequence_3_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_05_sequence_3_step_02,
    g_wmap_land_effect_05_sequence_3_step, g_wmap_land_effect_05_sequence_3_timer,
    func_8006B6EC(0xC8, 0xCD, 0xF, 0, 0x6))

/**
 * @brief Start fading the sprite range and run its first update.
 */
WMAP_STEP_FADE_ACTOR_RANGE(wmap_land_effect_05_sequence_3_step_03,
    g_wmap_land_effect_05_sequence_3_step, g_wmap_land_effect_05_sequence_3_timer,
    0xC8, 0xCD, 8, 0x10, wmap_land_effect_05_sequence_3_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_05_sequence_3_step_04,
    g_wmap_land_effect_05_sequence_3_step, g_wmap_land_effect_05_sequence_3_timer,
    func_8006B6EC(0xC8, 0xCD, 0xF, 0, 0x6))

WMAP_STEP_ADVANCE(wmap_land_effect_05_sequence_3_end, g_wmap_land_effect_05_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_05_run_sequence_4, D_800D5E40, 0x6, g_wmap_land_effect_05_sequence_4_step, g_wmap_land_effect_05_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_05_sequence_4_reset, g_wmap_land_effect_05_sequence_4_step, g_wmap_land_effect_05_sequence_4_timer)

/** @brief World-map step: reset the sprite counters and advance to the next handler. */
void wmap_land_effect_05_sequence_4_step_01(void)
{
    D_801B2468 = 1;
    D_801B24B4 = 0;
    D_80182DE4 = 0;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    D_801B2498.vx = 0;
    D_801B2498.vy = 0;
    D_801B2498.vz = 0;
    g_wmap_land_effect_05_sequence_4_timer = 0x60;
    g_wmap_land_effect_05_sequence_4_step += 1;
    wmap_land_effect_05_sequence_4_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_05_sequence_4_step_03, g_wmap_land_effect_05_sequence_4_step, g_wmap_land_effect_05_sequence_4_timer, 0x20,
                    wmap_land_effect_05_sequence_4_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_05_sequence_4_end, g_wmap_land_effect_05_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_05_run_sequence_5, D_800D5E58, 0x6, g_wmap_land_effect_05_sequence_5_step, g_wmap_land_effect_05_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_05_sequence_5_reset, g_wmap_land_effect_05_sequence_5_step, g_wmap_land_effect_05_sequence_5_timer)

WMAP_STEP_DROP_START(wmap_land_effect_05_sequence_5_step_01, g_wmap_land_effect_05_sequence_5_step, g_wmap_land_effect_05_sequence_5_timer, g_wmap_effect_model_d_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_d_position, g_wmap_effect_fade_d, 1, 0xBB8, 0x48, wmap_land_effect_05_sequence_5_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_05_sequence_5_step_03, g_wmap_land_effect_05_sequence_5_step, g_wmap_land_effect_05_sequence_5_timer, 0x40,
                    wmap_land_effect_05_sequence_5_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_05_sequence_5_end, g_wmap_land_effect_05_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_05_run_sequence_6, D_800D5E70, 0x6, g_wmap_land_effect_05_sequence_6_step, g_wmap_land_effect_05_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_05_sequence_6_reset, g_wmap_land_effect_05_sequence_6_step, g_wmap_land_effect_05_sequence_6_timer)

WMAP_STEP_DROP_START(wmap_land_effect_05_sequence_6_step_01, g_wmap_land_effect_05_sequence_6_step, g_wmap_land_effect_05_sequence_6_timer, D_801B2670,
                     g_wmap_zero_rotation, D_80139898, D_801B25D8, 1, 0x7530, 0x7C, wmap_land_effect_05_sequence_6_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_05_sequence_6_step_03, g_wmap_land_effect_05_sequence_6_step, g_wmap_land_effect_05_sequence_6_timer, 0x40,
                    wmap_land_effect_05_sequence_6_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_05_sequence_6_end, g_wmap_land_effect_05_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_05_run_sequence_7, D_800D5E88, 0x6, g_wmap_land_effect_05_sequence_7_step, g_wmap_land_effect_05_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_05_sequence_7_reset, g_wmap_land_effect_05_sequence_7_step, g_wmap_land_effect_05_sequence_7_timer)

WMAP_STEP_DROP_START(wmap_land_effect_05_sequence_7_step_01, g_wmap_land_effect_05_sequence_7_step, g_wmap_land_effect_05_sequence_7_timer, D_801B2678,
                     g_wmap_zero_rotation, D_801B2660, D_801B25DC, 1, 0x7530, 0x7C, wmap_land_effect_05_sequence_7_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_05_sequence_7_step_03, g_wmap_land_effect_05_sequence_7_step, g_wmap_land_effect_05_sequence_7_timer, 0x40,
                    wmap_land_effect_05_sequence_7_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_05_sequence_7_end, g_wmap_land_effect_05_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_05_run_sequence_8, D_800D5EA0, 0x4, g_wmap_land_effect_05_sequence_8_step, g_wmap_land_effect_05_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_05_sequence_8_reset, g_wmap_land_effect_05_sequence_8_step, g_wmap_land_effect_05_sequence_8_timer)

WMAP_STEP_DROP_START(wmap_land_effect_05_sequence_8_step_01, g_wmap_land_effect_05_sequence_8_step, g_wmap_land_effect_05_sequence_8_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40, wmap_land_effect_05_sequence_8_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_05_sequence_8_end, g_wmap_land_effect_05_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_05_run_sequence_9, D_800D5EB0, 0x4, g_wmap_land_effect_05_sequence_9_step, g_wmap_land_effect_05_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_05_sequence_9_reset, g_wmap_land_effect_05_sequence_9_step, g_wmap_land_effect_05_sequence_9_timer)

WMAP_STEP_DROP_START(wmap_land_effect_05_sequence_9_step_01, g_wmap_land_effect_05_sequence_9_step, g_wmap_land_effect_05_sequence_9_timer, g_wmap_effect_model_b_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x40, wmap_land_effect_05_sequence_9_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_05_sequence_9_end, g_wmap_land_effect_05_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_05_run_sequence_10, D_800D5EC0, 0x8, g_wmap_land_effect_05_sequence_10_step, g_wmap_land_effect_05_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_05_sequence_10_reset, g_wmap_land_effect_05_sequence_10_step, g_wmap_land_effect_05_sequence_10_timer)

/**
 * @brief Update the particles and increase the shared fade value until the timer expires.
 */
WMAP_STEP_UPDATE_AND_RAMP(wmap_land_effect_05_sequence_10_step_02, g_wmap_land_effect_05_sequence_10_step, g_wmap_land_effect_05_sequence_10_timer,
                          g_wmap_effect_fade_c, 8,
                          func_8006B328(0x64, 0xC8, 1, -1, -2, -2, 0, 0xF, -0xAF, 0x15E, -0xAF, 0x15E, 0x32, 1, 0x81, 2, 0))

WMAP_STEP_ARM_TIMER(wmap_land_effect_05_sequence_10_step_03, g_wmap_land_effect_05_sequence_10_step, g_wmap_land_effect_05_sequence_10_timer, 0x40,
                    wmap_land_effect_05_sequence_10_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_05_sequence_10_step_04,
    g_wmap_land_effect_05_sequence_10_step, g_wmap_land_effect_05_sequence_10_timer,
    func_8006B328(0x64, 0xC8, 1, -1, -2, -2, 0, 0xF, -0xAF, 0x15E, -0xAF, 0x15E,
                  0x32, 1, 0x81, 2, 0))

/**
 * @brief Stop spawning particles and keep updating those already active.
 */
WMAP_STEP_STOP_PARTICLE_SPAWNS(wmap_land_effect_05_sequence_10_step_05, g_wmap_land_effect_05_sequence_10_step, g_wmap_land_effect_05_sequence_10_timer,
                              D_800DCEA8, 0x40, wmap_land_effect_05_sequence_10_step_06)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_05_sequence_10_step_06,
    g_wmap_land_effect_05_sequence_10_step, g_wmap_land_effect_05_sequence_10_timer,
    func_8006B328(0x64, 0xC8, 1, -1, -2, -2, 0, 0xF, -0xAF, 0x15E, -0xAF, 0x15E,
                  0x32, 1, 0x81, 2, 0))

WMAP_STEP_ADVANCE(wmap_land_effect_05_sequence_10_end, g_wmap_land_effect_05_sequence_10_step)
