#include "../internal/wmap_model_render.h"
#include "../internal/wmap_land_effect_26.h"
#include "../internal/wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_view_effects.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_cells.h"

/** @brief Screen coordinates and their packed renderer argument. */
typedef union
{
    s32 packed;
    struct
    {
        s16 x, y;
    } point;
} WmapScreenPoint;

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s16 field_10;
    s16 field_12;
} WmapMotion;

void wmap_land_effect_26_sequence_11_step_02(void);
void wmap_land_effect_26_wait_idle_02(void);
void wmap_land_effect_26_step_03(void);
s32 wmap_land_effect_26_run_timeline(s32 arg0);
void wmap_land_effect_26_wait_idle_04(void);
void wmap_land_effect_26_end(void);
s32 wmap_land_effect_26_run_sequence_8(s32 arg0);
s32 wmap_land_effect_26_run_sequence_2(s32 arg0);
s32 wmap_land_effect_26_run_sequence_1(s32 arg0);
s32 wmap_land_effect_26_run_sequence_7(s32 arg0);
s32 wmap_land_effect_26_run_sequence_9(s32 arg0);
s32 wmap_land_effect_26_run_sequence_4(s32 arg0);
s32 wmap_land_effect_26_run_sequence_5(s32 arg0);
s32 wmap_land_effect_26_run_sequence_6(s32 arg0);
s32 wmap_land_effect_26_run_sequence_3(s32 arg0);
s32 wmap_land_effect_26_run_sequence_11(s32 arg0);
s32 wmap_land_effect_26_run_sequence_10(s32 arg0);
void wmap_land_effect_26_sequence_7_step_02(void);
void wmap_land_effect_26_sequence_9_step_02(void);
void wmap_land_effect_26_sequence_9_step_04(void);
void wmap_land_effect_26_sequence_8_step_02(void);
void wmap_land_effect_26_sequence_8_step_04(void);
void wmap_land_effect_26_sequence_10_step_02(void);
void wmap_land_effect_26_sequence_11_step_04(void);
void wmap_land_effect_26_sequence_11_step_06(void);

extern u8 g_wmap_load_buffer[];
extern s32 D_801B2470;
extern s32 g_wmap_land_effect_26_sequence_1_timer;
extern void *g_wmap_effect_model_pack_1;
extern s32 D_801B2474;
extern s32 g_wmap_land_effect_26_sequence_2_timer;
extern void *g_wmap_effect_model_pack_2;
extern s32 g_wmap_land_effect_26_sequence_3_timer;
extern s32 D_801B24B4;
extern s32 D_80182DE4;
extern u8* g_wmap_effect_model_pack_3;
extern s32 g_wmap_land_effect_26_sequence_4_timer;
extern u8* D_8011CF2C;
extern s32 g_wmap_land_effect_26_sequence_5_timer;
extern u8* D_8011CF30;
extern s32 g_wmap_land_effect_26_sequence_6_timer;
extern u8 g_wmap_animation_bank_2[];
extern s32 g_wmap_land_effect_26_sequence_11_timer;
extern u32 rand(void);
extern s8 D_80051B4C[];
extern s32 g_wmap_land_effect_26_timer;
extern void (*D_800D5148[])(void);
extern void wmap_land_effect_26_step_03(void);
extern void wmap_land_effect_26_end(void);
extern s32 g_wmap_land_effect_26_timeline_timer;
extern void (*D_800D5160[])(void);
extern u32 g_wmap_selected_artifact;
extern void (*D_800D51B0[])(void);
extern void (*D_800D51C0[])(void);
extern void (*D_800D51D0[])(void);
extern void (*D_800D51E0[])(void);
extern void (*D_800D51F0[])(void);
extern void (*D_800D5200[])(void);
extern s32 g_wmap_land_effect_26_sequence_7_timer;
extern void (*D_800D5210[])(void);
extern u8 g_wmap_animation_bank_0[];
extern s32 g_wmap_land_effect_26_sequence_9_timer;
extern void (*D_800D5238[])(void);
extern s32 g_wmap_land_effect_26_sequence_8_timer;
extern void (*D_800D5220[])(void);
extern void wmap_land_effect_26_sequence_8_step_04(void);
extern s32 g_wmap_land_effect_26_sequence_10_timer;
extern void (*D_800D5250[])(void);
extern u8 g_wmap_animation_bank_1[];
extern void wmap_land_effect_26_sequence_10_step_02(void);
extern void (*D_800D5260[])(void);

extern u32 g_wmap_land_effect_26_sequence_1_step;
extern u32 g_wmap_land_effect_26_sequence_2_step;
extern u32 g_wmap_land_effect_26_sequence_3_step;
extern u32 g_wmap_land_effect_26_sequence_4_step;
extern u32 g_wmap_land_effect_26_sequence_5_step;
extern u32 g_wmap_land_effect_26_sequence_6_step;
extern u32 g_wmap_land_effect_26_sequence_11_step;
extern u32 g_wmap_land_effect_26_step;
extern u32 g_wmap_land_effect_26_timeline_step;
extern u32 g_wmap_land_effect_26_sequence_7_step;
extern u32 g_wmap_land_effect_26_sequence_9_step;
extern u32 g_wmap_land_effect_26_sequence_8_step;
extern u32 g_wmap_land_effect_26_sequence_10_step;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_80139898;
extern VECTOR D_801B2660;

extern SVECTOR D_801B2670;
extern SVECTOR D_801B2678;


extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;

extern WmapMotion g_wmap_actor_motions[];

/** @brief Approach the effect depth and draw its alternating-brightness fade. */
void wmap_land_effect_26_sequence_1_step_02(void)
{
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = g_wmap_effect_model_a_position.vz - 10000;
    g_wmap_effect_model_a_position.vz = depth;
    if (depth < 1000)
    {
        g_wmap_effect_model_a_position.vz = 1000;
    }
    PushMatrix();
    wmap_set_model_transform(&g_wmap_effect_model_a_position, &g_wmap_effect_model_a_rotation);
    if (D_801B2470 != 0)
    {
        if (g_wmap_frame_count & 1)
        {
            wmap_draw_model(g_wmap_load_buffer, 0, 4, -1, -1, 1, D_801B2470, 5, -20, -1);
        }
        else
        {
            wmap_draw_model(g_wmap_load_buffer, 0, 4, -1, -1, 1, D_801B2470 / 2, 5, -20, -1);
        }
        WMAP_MODEL_FADE_OUT(D_801B2470, 2, intensity);
    }
    PopMatrix();
    remaining = g_wmap_land_effect_26_sequence_1_timer - 1;
    g_wmap_land_effect_26_sequence_1_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_26_sequence_1_step++;
    }
}

/** @brief Move the effect toward the camera while fading it and advancing its countdown. */
void wmap_land_effect_26_sequence_2_step_02(void)
{
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = g_wmap_effect_model_b_position.vz - 2500;
    g_wmap_effect_model_b_position.vz = depth;
    if (depth < 100)
    {
        g_wmap_effect_model_b_position.vz = 100;
    }
    PushMatrix();
    wmap_set_model_transform(&g_wmap_effect_model_b_position, &g_wmap_effect_model_b_rotation);
    if (D_801B2474 != 0)
    {
        wmap_draw_model(g_wmap_effect_model_pack_1, 0, 4, -1, -1, 1, D_801B2474, 5, -20, -1);
    }
    PopMatrix();
    WMAP_MODEL_FADE_OUT(D_801B2474, 8, intensity);
    remaining = g_wmap_land_effect_26_sequence_2_timer - 1;
    g_wmap_land_effect_26_sequence_2_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_26_sequence_2_step++;
    }
}

/** @brief Draw and fade the transformed effect, then advance its countdown. */
void wmap_land_effect_26_sequence_3_step_02(void)
{
    MATRIX matrix;
    s32 intensity;
    s32 remaining;

    if (g_wmap_effect_model_c_position.vz < 10)
    {
        g_wmap_effect_model_c_position.vz = 10;
    }
    PushMatrix();
    RotMatrix(&g_wmap_effect_model_c_rotation, &matrix);
    TransMatrix(&matrix, &g_wmap_zero_translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    if (D_801B24B4 != 0)
    {
        wmap_draw_model_default(g_wmap_effect_model_pack_2, 0, 4, -1, -1, 1, D_801B24B4);
        WMAP_MODEL_FADE_OUT(D_801B24B4, 8, intensity);
    }
    PopMatrix();
    remaining = g_wmap_land_effect_26_sequence_3_timer - 1;
    g_wmap_land_effect_26_sequence_3_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_26_sequence_3_step++;
    }
}

/** @brief World-map step handler: decay a value, draw the model while active, then countdown-advance. */
void wmap_land_effect_26_sequence_4_step_02(void)
{
    s32 v1;

    v1 = g_wmap_effect_model_d_position.vz - 0x5DC;
    g_wmap_effect_model_d_position.vz = v1;
    if (v1 < 0x9C40)
    {
        g_wmap_effect_model_d_position.vz = 0x9C40;
    }

    PushMatrix();
    wmap_set_model_transform(&g_wmap_effect_model_d_position, &g_wmap_effect_model_d_rotation);

    if (D_80182DE4 != 0)
    {
        wmap_draw_model_default(g_wmap_effect_model_pack_3, 0, 0xC, 0x35, 0x7800, 1, D_80182DE4);
        if (D_80182DE4 < 0)
        {
            D_80182DE4 = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_26_sequence_4_timer == 0)
    {
        g_wmap_land_effect_26_sequence_4_step += 1;
    }
}

/** @brief World-map step handler: decay a value, draw the model while active, then countdown-advance. */
void wmap_land_effect_26_sequence_5_step_02(void)
{
    s32 v1;

    v1 = D_80139898.vz - 0x5DC;
    D_80139898.vz = v1;
    if (v1 < 0x9C40)
    {
        D_80139898.vz = 0x9C40;
    }

    PushMatrix();
    wmap_set_model_transform(&D_80139898, &D_801B2670);

    if (g_wmap_effect_fade_a != 0)
    {
        wmap_draw_model_default(D_8011CF2C, 0, 0xC, 0x35, 0x7800, 1, g_wmap_effect_fade_a);
        if (g_wmap_effect_fade_a < 0)
        {
            g_wmap_effect_fade_a = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_26_sequence_5_timer == 0)
    {
        g_wmap_land_effect_26_sequence_5_step += 1;
    }
}

/** @brief World-map step handler: decay a value, draw the model while active, then countdown-advance. */
void wmap_land_effect_26_sequence_6_step_02(void)
{
    s32 v1;

    v1 = D_801B2660.vz - 0x5DC;
    D_801B2660.vz = v1;
    if (v1 < 0x9C40)
    {
        D_801B2660.vz = 0x9C40;
    }

    PushMatrix();
    wmap_set_model_transform(&D_801B2660, &D_801B2678);

    if (g_wmap_effect_fade_b != 0)
    {
        wmap_draw_model_default(D_8011CF30, 0, 0xC, 0x35, 0x7800, 1, g_wmap_effect_fade_b);
        if (g_wmap_effect_fade_b < 0)
        {
            g_wmap_effect_fade_b = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_26_sequence_6_timer == 0)
    {
        g_wmap_land_effect_26_sequence_6_step += 1;
    }
}

/** @brief Initialize randomized motion for the effect actors. */
void wmap_land_effect_26_sequence_11_step_01(void)
{
    s32 i;
    WmapSpriteActor *actor;

    g_wmap_effect_fade_d = 1;
    for (i = 124; i < 170; i++)
    {
        actor = &g_wmap_sprite_actors[i];
        g_wmap_actor_motions[i].state = 1;
        g_wmap_actor_motions[i].angle = (rand() * 155) >> 10;
        g_wmap_actor_motions[i].field_0E = (rand() * 7) >> 6;
        g_wmap_actor_motions[i].x = ((s32)(rand() << 6) >> 15) + 16;
        g_wmap_actor_motions[i].scale = rand() >> 9;
        g_wmap_actor_motions[i].field_10 = ((s32)(rand() << 6) >> 15) + 4;
        g_wmap_actor_motions[i].field_12 = 0;
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_2;
        actor->scale_index = 15;
        actor->resource_index = 0;
        actor->previous_sequence = -1;
        actor->shade_step = 0;
        actor->target_shade = g_wmap_effect_fade_d;
        actor->shade = g_wmap_effect_fade_d;
        actor->sequence = i % 3;
    }
    g_wmap_land_effect_26_sequence_11_timer = 16;
    g_wmap_land_effect_26_sequence_11_step++;
    wmap_land_effect_26_sequence_11_step_02();
}

/** @brief Advance oscillating actors and draw their sprites.
 * @param start First actor index.
 * @param end Exclusive end index.
 * @param depth Rendering depth.
 */
void func_800773B8(s32 start, s32 end, s32 depth)
{
    s32 i;
    WmapMotion *motion;
    WmapSpriteActor *actor;
    WmapScreenPoint screen;

    for (i = start; i < end; i++)
    {
        actor = &g_wmap_sprite_actors[i];
        motion = &g_wmap_actor_motions[i];
        motion->field_0E += motion->x;
        if (motion->field_0E >= 3841)
        {
            motion->field_0E = 0;
        }
        motion->field_12 = (motion->field_12 + motion->field_10) & 4095;
        screen.point.x = (motion->angle + (D_80051B4C[motion->field_12 / 16] * motion->scale) / 16) / 16;
        screen.point.y = motion->field_0E / 16;
        actor->target_shade = g_wmap_effect_fade_d;
        actor->shade = g_wmap_effect_fade_d;
        wmap_step_actor_animation(actor, &g_wmap_actor_animations[i]);
        wmap_draw_actor_sprite(actor, screen.packed, depth, 4, 0);
    }
}

WMAP_STEP_RUNNER(wmap_land_effect_26_run, D_800D5148, 0x6, g_wmap_land_effect_26_step, g_wmap_land_effect_26_timer)

WMAP_STEP_RESET(wmap_land_effect_26_reset, g_wmap_land_effect_26_step, g_wmap_land_effect_26_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_26_step_01, g_wmap_land_effect_26_step, wmap_run_land_focus, wmap_land_effect_26_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_26_wait_idle_02, g_wmap_land_effect_26_step, wmap_land_effect_26_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_26_step_03, g_wmap_land_effect_26_step, wmap_land_effect_26_run_timeline, wmap_land_effect_26_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_26_wait_idle_04, g_wmap_land_effect_26_step, wmap_land_effect_26_end)

WMAP_STEP_ADVANCE(wmap_land_effect_26_end, g_wmap_land_effect_26_step)

WMAP_STEP_RUNNER(wmap_land_effect_26_run_timeline, D_800D5160, 0x14, g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_26_timeline_reset, g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer)

/** @brief World-map step handler: set up the actor, register callbacks, advance step. */
void wmap_land_effect_26_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x804020);
    wmap_start_sequence(wmap_land_effect_26_run_sequence_8);
    wmap_play_sound(0x19, 0x80);
    g_wmap_land_effect_26_timeline_timer = 2;
    g_wmap_land_effect_26_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_26_timeline_wait_02, g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_26_timeline_step_03, g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer,
                         wmap_land_effect_26_run_sequence_2, 0x4)

WMAP_STEP_WAIT(wmap_land_effect_26_timeline_wait_04, g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_26_timeline_step_05, g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer,
                         wmap_land_effect_26_run_sequence_1, 0x8)

WMAP_STEP_WAIT(wmap_land_effect_26_timeline_wait_06, g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer)

/**
 * @brief Hide the placement overlay, start the sequence, and set the wait timer.
 */
WMAP_STEP_HIDE_AND_START(wmap_land_effect_26_timeline_step_07,
    g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer,
    g_wmap_placement_overlay_hidden, wmap_land_effect_26_run_sequence_7, 0x8)

WMAP_STEP_WAIT(wmap_land_effect_26_timeline_wait_08, g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_26_timeline_step_09, g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer,
                         wmap_land_effect_26_run_sequence_9, 0x14)

WMAP_STEP_WAIT(wmap_land_effect_26_timeline_wait_10, g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_26_timeline_step_11, g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer,
                         wmap_land_effect_26_run_sequence_4, 0xC)

WMAP_STEP_WAIT(wmap_land_effect_26_timeline_wait_12, g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_26_timeline_step_13, g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer,
                         wmap_land_effect_26_run_sequence_5, 0xC)

WMAP_STEP_WAIT(wmap_land_effect_26_timeline_wait_14, g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_26_timeline_step_15, g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer,
                         wmap_land_effect_26_run_sequence_6, 0x10)

WMAP_STEP_WAIT(wmap_land_effect_26_timeline_wait_16, g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer)

/**
 * @brief Start three sequences and set the wait timer.
 */
WMAP_STEP_START_THREE_AND_WAIT(wmap_land_effect_26_timeline_step_17,
    g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer,
    wmap_land_effect_26_run_sequence_3, wmap_land_effect_26_run_sequence_11, wmap_land_effect_26_run_sequence_10, 0xAC)

WMAP_STEP_WAIT(wmap_land_effect_26_timeline_wait_18, g_wmap_land_effect_26_timeline_step, g_wmap_land_effect_26_timeline_timer)

/** @brief Mark the selected map cell with the active land and advance the effect. */
void wmap_land_effect_26_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    g_wmap_cells[g_wmap_focus_cell_x][g_wmap_focus_cell_y].land_id = g_wmap_selected_artifact | 0x100;
    g_wmap_land_effect_26_timeline_step += 1;
}

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_26_run_sequence_1, D_800D51B0, 0x4, g_wmap_land_effect_26_sequence_1_step,
                               g_wmap_land_effect_26_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_26_sequence_1_reset, g_wmap_land_effect_26_sequence_1_step, g_wmap_land_effect_26_sequence_1_timer)

/** @brief Restore effect data, reset its vector, and begin a 48-tick sequence step. */
void wmap_land_effect_26_sequence_1_step_01(void)
{
    g_wmap_effect_model_a_rotation = g_wmap_zero_rotation;
    D_801B2470 = 0x80;
    g_wmap_effect_model_a_position.vz = 0x2710;
    g_wmap_effect_model_a_position.vy = 0;
    g_wmap_effect_model_a_position.vx = 0;
    g_wmap_land_effect_26_sequence_1_timer = 0x30;
    g_wmap_land_effect_26_sequence_1_step += 1;
    wmap_land_effect_26_sequence_1_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_effect_26_sequence_1_end, g_wmap_land_effect_26_sequence_1_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_26_run_sequence_2, D_800D51C0, 0x4, g_wmap_land_effect_26_sequence_2_step,
                               g_wmap_land_effect_26_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_26_sequence_2_reset, g_wmap_land_effect_26_sequence_2_step, g_wmap_land_effect_26_sequence_2_timer)

/** @brief Restore effect data, reset its vector, and begin a 16-tick sequence step. */
void wmap_land_effect_26_sequence_2_step_01(void)
{
    g_wmap_effect_model_b_rotation = g_wmap_zero_rotation;
    D_801B2474 = 0x80;
    g_wmap_effect_model_b_position.vz = 0xC350;
    g_wmap_effect_model_b_position.vy = 0;
    g_wmap_effect_model_b_position.vx = 0;
    g_wmap_land_effect_26_sequence_2_timer = 0x10;
    g_wmap_land_effect_26_sequence_2_step += 1;
    wmap_land_effect_26_sequence_2_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_effect_26_sequence_2_end, g_wmap_land_effect_26_sequence_2_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_26_run_sequence_3, D_800D51D0, 0x4, g_wmap_land_effect_26_sequence_3_step,
                               g_wmap_land_effect_26_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_26_sequence_3_reset, g_wmap_land_effect_26_sequence_3_step, g_wmap_land_effect_26_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_26_sequence_3_step_01, g_wmap_land_effect_26_sequence_3_step, g_wmap_land_effect_26_sequence_3_timer, g_wmap_effect_model_c_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_c_position, D_801B24B4, 0x80, 1000, 0x80, wmap_land_effect_26_sequence_3_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_26_sequence_3_end, g_wmap_land_effect_26_sequence_3_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_26_run_sequence_4, D_800D51E0, 0x4, g_wmap_land_effect_26_sequence_4_step,
                               g_wmap_land_effect_26_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_26_sequence_4_reset, g_wmap_land_effect_26_sequence_4_step, g_wmap_land_effect_26_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_26_sequence_4_step_01, g_wmap_land_effect_26_sequence_4_step, g_wmap_land_effect_26_sequence_4_timer, g_wmap_effect_model_d_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_d_position, D_80182DE4, 0x80, 0xAFC8, 0x2A, wmap_land_effect_26_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_26_sequence_4_end, g_wmap_land_effect_26_sequence_4_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_26_run_sequence_5, D_800D51F0, 0x4, g_wmap_land_effect_26_sequence_5_step,
                               g_wmap_land_effect_26_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_26_sequence_5_reset, g_wmap_land_effect_26_sequence_5_step, g_wmap_land_effect_26_sequence_5_timer)

WMAP_STEP_DROP_START(wmap_land_effect_26_sequence_5_step_01, g_wmap_land_effect_26_sequence_5_step, g_wmap_land_effect_26_sequence_5_timer, D_801B2670,
                     g_wmap_zero_rotation, D_80139898, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x1E, wmap_land_effect_26_sequence_5_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_26_sequence_5_end, g_wmap_land_effect_26_sequence_5_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_26_run_sequence_6, D_800D5200, 0x4, g_wmap_land_effect_26_sequence_6_step,
                               g_wmap_land_effect_26_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_26_sequence_6_reset, g_wmap_land_effect_26_sequence_6_step, g_wmap_land_effect_26_sequence_6_timer)

WMAP_STEP_DROP_START(wmap_land_effect_26_sequence_6_step_01, g_wmap_land_effect_26_sequence_6_step, g_wmap_land_effect_26_sequence_6_timer, D_801B2678,
                     g_wmap_zero_rotation, D_801B2660, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x12, wmap_land_effect_26_sequence_6_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_26_sequence_6_end, g_wmap_land_effect_26_sequence_6_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_26_run_sequence_7, D_800D5210, 0x4, g_wmap_land_effect_26_sequence_7_step,
                               g_wmap_land_effect_26_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_26_sequence_7_reset, g_wmap_land_effect_26_sequence_7_step, g_wmap_land_effect_26_sequence_7_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_26_sequence_7_step_01,
    g_wmap_land_effect_26_sequence_7_step, g_wmap_land_effect_26_sequence_7_timer,
    4, g_wmap_animation_bank_0, 0,
    0x80, 0x80, 0,
    0x4A, wmap_land_effect_26_sequence_7_step_02)

/** @brief World-map step: init a sub-object then count down a timer. */
void wmap_land_effect_26_sequence_7_step_02(void)
{
    s32 n = 0xB;

    wmap_step_actor_animation(&g_wmap_sprite_actors[4], &g_wmap_actor_animations[4]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[4], g_wmap_focus_screen_position.packed, n, n, 0);
    if (--g_wmap_land_effect_26_sequence_7_timer == 0)
    {
        g_wmap_land_effect_26_sequence_7_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_26_sequence_7_end, g_wmap_land_effect_26_sequence_7_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_26_run_sequence_9, D_800D5238, 0x6, g_wmap_land_effect_26_sequence_9_step,
                               g_wmap_land_effect_26_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_26_sequence_9_reset, g_wmap_land_effect_26_sequence_9_step, g_wmap_land_effect_26_sequence_9_timer)

/** @brief Initialize twenty effect records and begin a 32-tick sequence step. */
void wmap_land_effect_26_sequence_9_step_01(void)
{
    s32 index;

    g_wmap_particle_intensity = 20;
    g_wmap_effect_fade_c = 255;
    for (index = 14; index < 34; index++)
    {
        *(s16 *)((u8*)g_wmap_actor_motions + index * 20) = 0;
        g_wmap_actor_animations[index + 14].data = g_wmap_animation_bank_0;
    }
    g_wmap_land_effect_26_sequence_9_timer = 32;
    g_wmap_land_effect_26_sequence_9_step++;
    wmap_land_effect_26_sequence_9_step_02();
}

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_26_sequence_9_step_02,
    g_wmap_land_effect_26_sequence_9_step, g_wmap_land_effect_26_sequence_9_timer,
    func_8006A9C4(&g_wmap_sprite_actors[14], &g_wmap_actor_animations[14], 0xE, 0x22, g_wmap_effect_fade_c, 0x1FF6, 0x200, 0x30, 2, 0x320, 0xB, 4))

WMAP_STEP_ARM_TIMER(wmap_land_effect_26_sequence_9_step_03, g_wmap_land_effect_26_sequence_9_step, g_wmap_land_effect_26_sequence_9_timer, 0x40,
                    wmap_land_effect_26_sequence_9_step_04)

/** @brief Update the actor effect and advance the sequence after its countdown. */
void wmap_land_effect_26_sequence_9_step_04(void)
{
    func_8006A9C4(&g_wmap_sprite_actors[14], &g_wmap_actor_animations[14], 0xE, 0x22, g_wmap_effect_fade_c, 0x1FF6, 0x200, 0x30, 2, 0x320, 0xB, 4);
    g_wmap_effect_fade_c -= 4;
    if (g_wmap_effect_fade_c < 0)
    {
        g_wmap_effect_fade_c = 0;
    }
    if (--g_wmap_land_effect_26_sequence_9_timer == 0)
    {
        g_wmap_land_effect_26_sequence_9_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_26_sequence_9_end, g_wmap_land_effect_26_sequence_9_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_26_run_sequence_8, D_800D5220, 0x6, g_wmap_land_effect_26_sequence_8_step,
                               g_wmap_land_effect_26_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_26_sequence_8_reset, g_wmap_land_effect_26_sequence_8_step, g_wmap_land_effect_26_sequence_8_timer)

/** @brief Start the sprite animation and run its first update. */
WMAP_STEP_START_ACTOR(wmap_land_effect_26_sequence_8_step_01,
    g_wmap_land_effect_26_sequence_8_step, g_wmap_land_effect_26_sequence_8_timer,
    6, g_wmap_animation_bank_0, 2,
    0x90, 0x81, -1,
    0x14, wmap_land_effect_26_sequence_8_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_26_sequence_8_step_02, g_wmap_land_effect_26_sequence_8_step, g_wmap_land_effect_26_sequence_8_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0xB, 0x4, 0)

/**
 * @brief Start the sprite fade and run its first update.
 */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_26_sequence_8_step_03,
    g_wmap_land_effect_26_sequence_8_step, g_wmap_land_effect_26_sequence_8_timer,
    g_wmap_sprite_actors[6], 8, 0x10, wmap_land_effect_26_sequence_8_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_26_sequence_8_step_04, g_wmap_land_effect_26_sequence_8_step, g_wmap_land_effect_26_sequence_8_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0xB, 0x4, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_26_sequence_8_end, g_wmap_land_effect_26_sequence_8_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_26_run_sequence_10, D_800D5250, 0x4, g_wmap_land_effect_26_sequence_10_step,
                               g_wmap_land_effect_26_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_26_sequence_10_reset, g_wmap_land_effect_26_sequence_10_step, g_wmap_land_effect_26_sequence_10_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_26_sequence_10_step_01,
    g_wmap_land_effect_26_sequence_10_step, g_wmap_land_effect_26_sequence_10_timer,
    5, g_wmap_animation_bank_1, 0,
    0xFC, 0x80, 4,
    0xAF, wmap_land_effect_26_sequence_10_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_26_sequence_10_step_02, g_wmap_land_effect_26_sequence_10_step, g_wmap_land_effect_26_sequence_10_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x12, 0xB, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_26_sequence_10_end, g_wmap_land_effect_26_sequence_10_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_26_run_sequence_11, D_800D5260, 0x8, g_wmap_land_effect_26_sequence_11_step,
                               g_wmap_land_effect_26_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_26_sequence_11_reset, g_wmap_land_effect_26_sequence_11_step, g_wmap_land_effect_26_sequence_11_timer)

/** @brief Draw the effect, raise its value to at most 129, and update the countdown. */
void wmap_land_effect_26_sequence_11_step_02(void)
{
    s32 value;
    s32 remaining_ticks;

    func_800773B8(0x7C, 0xAA, 0xD);
    value = g_wmap_effect_fade_d + 8;
    g_wmap_effect_fade_d = value;
    if (value >= 0x82)
    {
        g_wmap_effect_fade_d = 0x81;
    }
    remaining_ticks = g_wmap_land_effect_26_sequence_11_timer - 1;
    g_wmap_land_effect_26_sequence_11_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_26_sequence_11_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_26_sequence_11_step_03, g_wmap_land_effect_26_sequence_11_step, g_wmap_land_effect_26_sequence_11_timer, 0x64,
                    wmap_land_effect_26_sequence_11_step_04)

/** @brief Update the sequence effect and advance when its countdown expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_26_sequence_11_step_04, g_wmap_land_effect_26_sequence_11_step, g_wmap_land_effect_26_sequence_11_timer,
                          func_800773B8(0x7C, 0xAA, 0xD))

WMAP_STEP_ARM_TIMER(wmap_land_effect_26_sequence_11_step_05, g_wmap_land_effect_26_sequence_11_step, g_wmap_land_effect_26_sequence_11_timer, 0x10,
                    wmap_land_effect_26_sequence_11_step_06)

/** @brief Draw the effect, reduce its value toward zero, and update the countdown. */
void wmap_land_effect_26_sequence_11_step_06(void)
{
    s32 value;
    s32 remaining_ticks;

    func_800773B8(0x7C, 0xAA, 0xD);
    value = g_wmap_effect_fade_d - 8;
    g_wmap_effect_fade_d = value;
    if (value < 0)
    {
        g_wmap_effect_fade_d = 0;
    }
    remaining_ticks = g_wmap_land_effect_26_sequence_11_timer - 1;
    g_wmap_land_effect_26_sequence_11_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_26_sequence_11_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_26_sequence_11_end, g_wmap_land_effect_26_sequence_11_step)
