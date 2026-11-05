#include "common.h"
#include <libgte.h>

#include "../internal/wmap_actor_motion.h"
#include "../internal/wmap_cells.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_land_effect_25.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_model_render.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_view_effects.h"

extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_25_sequence_3_timer;
extern s32 g_wmap_land_effect_25_sequence_4_timer;
extern s32 g_wmap_land_effect_25_sequence_5_timer;
extern s32 g_wmap_land_effect_25_sequence_6_timer;
extern s32 g_wmap_land_effect_25_sequence_7_timer;
extern s32 g_wmap_land_effect_25_sequence_8_timer;
extern s32 g_wmap_land_effect_25_sequence_9_timer;
extern s32 g_wmap_land_effect_25_sequence_12_timer;
extern s32 rand(void);
extern s32 g_wmap_land_effect_25_sequence_13_timer;
extern s32 g_wmap_land_effect_25_sequence_14_timer;
extern s32 g_wmap_land_effect_25_sequence_15_timer;
extern s32 g_wmap_land_effect_25_sequence_17_timer;
extern s32 g_wmap_land_effect_25_timer;
extern WmapSequenceStep g_wmap_land_effect_25_steps[];
extern s32 g_wmap_land_effect_25_timeline_timer;
extern WmapSequenceStep g_wmap_land_effect_25_timeline_steps[];
extern s32 g_wmap_land_effect_25_sequence_1_timer;
extern WmapSequenceStep g_wmap_land_effect_25_sequence_1_steps[];
extern s32 g_wmap_land_effect_25_sequence_2_timer;
extern WmapSequenceStep g_wmap_land_effect_25_sequence_2_steps[];
extern WmapSequenceStep g_wmap_land_effect_25_sequence_3_steps[];
extern WmapSequenceStep g_wmap_land_effect_25_sequence_4_steps[];
extern WmapSequenceStep g_wmap_land_effect_25_sequence_5_steps[];
extern WmapSequenceStep g_wmap_land_effect_25_sequence_6_steps[];
extern WmapSequenceStep g_wmap_land_effect_25_sequence_7_steps[];
extern WmapSequenceStep g_wmap_land_effect_25_sequence_8_steps[];
extern WmapSequenceStep g_wmap_land_effect_25_sequence_9_steps[];
extern s32 g_wmap_land_effect_25_sequence_10_timer;
extern WmapSequenceStep g_wmap_land_effect_25_sequence_10_steps[];
extern s32 g_wmap_land_effect_25_sequence_11_timer;
extern WmapSequenceStep g_wmap_land_effect_25_sequence_11_steps[];
extern WmapSequenceStep g_wmap_land_effect_25_sequence_12_steps[];
extern WmapSequenceStep g_wmap_land_effect_25_sequence_13_steps[];
extern WmapSequenceStep g_wmap_land_effect_25_sequence_14_steps[];
extern WmapSequenceStep g_wmap_land_effect_25_sequence_15_steps[];
extern s32 g_wmap_land_effect_25_sequence_16_timer;
extern WmapSequenceStep g_wmap_land_effect_25_sequence_16_steps[];
extern WmapSequenceStep g_wmap_land_effect_25_sequence_17_steps[];
extern s32 g_wmap_land_effect_25_sequence_18_timer;
extern WmapSequenceStep g_wmap_land_effect_25_sequence_18_steps[];

extern u32 g_wmap_land_effect_25_sequence_3_step;
extern u32 g_wmap_land_effect_25_sequence_4_step;
extern u32 g_wmap_land_effect_25_sequence_5_step;
extern u32 g_wmap_land_effect_25_sequence_6_step;
extern u32 g_wmap_land_effect_25_sequence_7_step;
extern u32 g_wmap_land_effect_25_sequence_8_step;
extern u32 g_wmap_land_effect_25_sequence_9_step;
extern u32 g_wmap_land_effect_25_sequence_12_step;
extern u32 g_wmap_land_effect_25_sequence_13_step;
extern u32 g_wmap_land_effect_25_sequence_14_step;
extern u32 g_wmap_land_effect_25_sequence_15_step;
extern u32 g_wmap_land_effect_25_sequence_17_step;
extern u32 g_wmap_land_effect_25_step;
extern u32 g_wmap_land_effect_25_timeline_step;
extern u32 g_wmap_land_effect_25_sequence_1_step;
extern u32 g_wmap_land_effect_25_sequence_2_step;
extern u32 g_wmap_land_effect_25_sequence_10_step;
extern u32 g_wmap_land_effect_25_sequence_11_step;
extern u32 g_wmap_land_effect_25_sequence_16_step;
extern u32 g_wmap_land_effect_25_sequence_18_step;

extern s32* g_wmap_effect_params;

extern WmapMotion g_wmap_effect_actor_motions[];

/**
 * @brief Move the model along Z, draw it, and fade it until the timer expires.
 */
WMAP_STEP_DROP_UPDATE_WITH_DRAW(wmap_land_effect_25_sequence_3_step_02, g_wmap_land_effect_25_sequence_3_step, g_wmap_land_effect_25_sequence_3_timer,
                                g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, -0xDAC, 2,
                                wmap_draw_model(g_wmap_load_buffer, 0, 0x4, 0x35, 0x7800, 1, g_wmap_effect_fade_a, 0, 0, -1))

/** @brief Move the model along Z, draw it, and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE_WITH_DRAW(wmap_land_effect_25_sequence_4_step_02, g_wmap_land_effect_25_sequence_4_step, g_wmap_land_effect_25_sequence_4_timer,
                                g_wmap_effect_model_b_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, -0xDAC, 2,
                                wmap_draw_model(g_wmap_effect_model_pack_1, 0, 4, 0x35, 0x7800, 1, g_wmap_effect_fade_b, 0, 0, -1))

/** @brief Move the model along Z, draw it, and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE_WITH_DRAW(wmap_land_effect_25_sequence_5_step_02, g_wmap_land_effect_25_sequence_5_step, g_wmap_land_effect_25_sequence_5_timer,
                                g_wmap_effect_model_c_rotation, g_wmap_effect_model_c_position, g_wmap_effect_fade_c, -0xDAC, 4,
                                wmap_draw_model(g_wmap_effect_model_pack_2, 0, 4, 0x35, 0x7800, 1, g_wmap_effect_fade_c, 0, 0, -1))

/**
 * @brief Draw and spin the model while fading it in.
 */
void wmap_land_effect_25_sequence_6_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_f_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, 0, 0x7, 0x35, 0x7840, 0x1001, g_wmap_effect_fade_e, 0, 0x32, -1);
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_e, 2, 0x81, value);
    timer = g_wmap_land_effect_25_sequence_6_timer;
    g_wmap_effect_model_f_rotation.vz += 0x8;
    next_timer = timer - 1;
    g_wmap_land_effect_25_sequence_6_timer = next_timer;

    if (next_timer == 0)
    {
        g_wmap_land_effect_25_sequence_6_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it out.
 */
void wmap_land_effect_25_sequence_6_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_f_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, 0, 0x7, 0x35, 0x7840, 0x1001, g_wmap_effect_fade_e, 0, 0x32, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_e, 2, value);
    timer = g_wmap_land_effect_25_sequence_6_timer;
    g_wmap_effect_model_f_rotation.vz += 0x8;
    next_timer = timer - 1;
    g_wmap_land_effect_25_sequence_6_timer = next_timer;

    if (next_timer == 0)
    {
        g_wmap_land_effect_25_sequence_6_step++;
    }
}

/**
 * @brief Draw and brighten the rotating model, then advance its countdown.
 */
void wmap_land_effect_25_sequence_7_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_d_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_4, 0, 0x7, 0x35, 0x7840, 0x1, g_wmap_effect_fade_d, 0, 0x1E, -1);
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_d, 2, 0x81, value);
    timer = g_wmap_land_effect_25_sequence_7_timer;
    g_wmap_effect_model_d_rotation.vz -= 0x8;
    next_timer = timer - 1;
    g_wmap_land_effect_25_sequence_7_timer = next_timer;

    if (next_timer == 0)
    {
        g_wmap_land_effect_25_sequence_7_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it out.
 */
void wmap_land_effect_25_sequence_7_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_d_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_4, 0, 0x7, 0x35, 0x7840, 0x1, g_wmap_effect_fade_d, 0, 0x1E, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_d, 2, value);
    timer = g_wmap_land_effect_25_sequence_7_timer;
    g_wmap_effect_model_d_rotation.vz += -0x8;
    next_timer = timer - 1;
    g_wmap_land_effect_25_sequence_7_timer = next_timer;

    if (next_timer == 0)
    {
        g_wmap_land_effect_25_sequence_7_step++;
    }
}

/** @brief Draw and brighten the rotating model, then advance its countdown. */
void wmap_land_effect_25_sequence_8_step_02(void)
{
    s32 remaining;
    s32 intensity;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_g_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_5, (g_wmap_particle_velocity_min >> 4) & 3, 7, 0x36, 0x7980, 0x1001, g_wmap_aux_effect_fade_c, 0, -10, -1);
    g_wmap_particle_velocity_min += 16;
    WMAP_MODEL_FADE_IN(g_wmap_aux_effect_fade_c, 2, 0x81, intensity);
    remaining = g_wmap_land_effect_25_sequence_8_timer - 1;
    g_wmap_effect_model_g_rotation.vz = (g_wmap_effect_model_g_rotation.vz + 0x18);
    g_wmap_land_effect_25_sequence_8_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_25_sequence_8_step++;
    }
}

/** @brief Advance the world-map effect and its sequence state. */
void wmap_land_effect_25_sequence_8_step_04(void)
{
    s32 remaining;
    s32 intensity;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_g_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_5, (g_wmap_particle_velocity_min >> 4) & 3, 7, 0x36, 0x7980, 0x1001, g_wmap_aux_effect_fade_c, 0, -10, -1);
    intensity = g_wmap_aux_effect_fade_c - 4;
    g_wmap_particle_velocity_min += 16;
    g_wmap_aux_effect_fade_c = intensity;

    if (intensity < 0)
    {
        g_wmap_aux_effect_fade_c = 0;
    }

    remaining = g_wmap_land_effect_25_sequence_8_timer - 1;
    g_wmap_effect_model_g_rotation.vz = (g_wmap_effect_model_g_rotation.vz + 0x18);
    g_wmap_land_effect_25_sequence_8_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_25_sequence_8_step++;
    }
}

/** @brief Draw and brighten the rotating model, then advance its countdown. */
void wmap_land_effect_25_sequence_9_step_02(void)
{
    s32 remaining;
    s32 intensity;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_h_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_6, (g_wmap_particle_velocity_range >> 4) & 7, 10, 0x35, 0x7800, 0x1001, g_wmap_aux_effect_fade_a, -1, 7, -1);
    g_wmap_particle_velocity_range += 16;
    WMAP_MODEL_FADE_IN(g_wmap_aux_effect_fade_a, 2, 0x81, intensity);
    remaining = g_wmap_land_effect_25_sequence_9_timer - 1;
    g_wmap_effect_model_h_rotation.vz = (g_wmap_effect_model_h_rotation.vz + 0xC);
    g_wmap_land_effect_25_sequence_9_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_25_sequence_9_step++;
    }
}

/** @brief Advance the world-map effect and its sequence state. */
void wmap_land_effect_25_sequence_9_step_04(void)
{
    s32 remaining;
    s32 intensity;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_h_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_6, (g_wmap_particle_velocity_range >> 4) & 7, 10, 0x35, 0x7800, 0x1001, g_wmap_aux_effect_fade_a, -1, 7, -1);
    intensity = g_wmap_aux_effect_fade_a - 2;
    g_wmap_particle_velocity_range += 16;
    g_wmap_aux_effect_fade_a = intensity;

    if (intensity < 0)
    {
        g_wmap_aux_effect_fade_a = 0;
    }

    remaining = g_wmap_land_effect_25_sequence_9_timer - 1;
    g_wmap_effect_model_h_rotation.vz = (g_wmap_effect_model_h_rotation.vz + 0xC);
    g_wmap_land_effect_25_sequence_9_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_25_sequence_9_step++;
    }
}

/** @brief Initialize effect actors with randomized angles and speeds. */
void wmap_land_effect_25_sequence_12_step_01(void)
{
    s32 i;
    WmapMotion* motion;
    WmapSpriteActor* actor;

    g_wmap_particle_intensity = 10;
    WMAP_SET_PARTICLE_CONFIG(10, 0, -2, 220, 32, -1, -900, 20, 8, 0, 102000);

    for (i = 0; i < 10; i++)
    {
        g_wmap_actor_animations[i + 20].data = g_wmap_animation_bank_3;
        actor = &g_wmap_sprite_actors[20 + i];
        motion = &g_wmap_effect_actor_motions[i];
        motion->state = 1;
        actor->scale_index = 15;
        actor->previous_sequence = -1;
        actor->resource_index = 0;
        actor->sequence = 0;
        actor->target_shade = 255;
        actor->shade = 1;
        actor->shade_step = 0;
        motion->state = 1;
        motion->angle = rand() & 4095;
        motion->radius = 102000;
        motion->velocity = (-(rand() * 2) >> 15);
        motion->z = ((rand() * 50) >> 15) + 220;
    }

    g_wmap_land_effect_25_sequence_12_timer = 32;
    g_wmap_land_effect_25_sequence_12_step++;
    wmap_land_effect_25_sequence_12_step_02();
}

/** @brief Initialize effect actors with randomized angles and speeds. */
void wmap_land_effect_25_sequence_13_step_01(void)
{
    s32 i;
    WmapMotion* motion;
    WmapSpriteActor* actor;

    g_wmap_particle_intensity = 30;
    WMAP_SET_PARTICLE_CONFIG(20, -1, -2, 180, 32, -1, -1600, 50, 8, 1, 102000);

    for (i = 0; i < 30; i++)
    {
        g_wmap_actor_animations[i + 50].data = g_wmap_animation_bank_3;
        actor = &g_wmap_sprite_actors[50 + i];
        motion = &g_wmap_actor_motions_at_50[i];
        motion->state = 1;
        actor->scale_index = 15;
        actor->previous_sequence = -1;
        actor->resource_index = 0;
        actor->sequence = 1;
        actor->target_shade = 255;
        actor->shade = 1;
        actor->shade_step = 0;
        motion->state = 1;
        motion->angle = rand() & 4095;
        motion->radius = 102000;
        motion->velocity = (-(rand() * 2) >> 15) - 1;
        motion->z = ((rand() * 100) >> 15) + 180;
    }

    g_wmap_land_effect_25_sequence_13_timer = 32;
    g_wmap_land_effect_25_sequence_13_step++;
    wmap_land_effect_25_sequence_13_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_25_sequence_14_step_01(void)
{
    s32 i;

    g_wmap_effect_params[0x28] = 40;
    g_wmap_effect_params[0x29] = 200;
    g_wmap_effect_params[0x2A] = -280;
    g_wmap_effect_params[0x2B] = 560;
    g_wmap_effect_params[0x2C] = -280;
    g_wmap_effect_params[0x2D] = 560;
    g_wmap_effect_params[0x2E] = 200;
    g_wmap_effect_params[0x2F] = 80;
    g_wmap_effect_params[0x30] = 129;
    g_wmap_effect_params[0x31] = 1;
    g_wmap_effect_params[0x32] = 2;
    g_wmap_effect_params[0x33] = 40;
    g_wmap_effect_params[0x34] = 8;
    g_wmap_effect_params[0x35] = 2;
    WMAP_RESET_PARTICLE_SLOTS(i, 40, g_wmap_actor_motions[i + 200].state, 200, g_wmap_animation_bank_3);
    g_wmap_land_effect_25_sequence_14_timer = 160;
    g_wmap_land_effect_25_sequence_14_step++;
    wmap_land_effect_25_sequence_14_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_25_sequence_15_step_01(void)
{
    s32 i;

    WMAP_SET_PARTICLE_CONFIG(0, 1, 8, 0x40, 0, 1, 0x5DC, 0xAA, 8, 3, 0x32C8);
    WMAP_RESET_PARTICLE_SLOTS(i, 24, g_wmap_actor_motions[i + 170].state, 170, g_wmap_animation_bank_3);
    g_wmap_land_effect_25_sequence_15_timer = 24;
    g_wmap_land_effect_25_sequence_15_step++;
    wmap_land_effect_25_sequence_15_step_02();
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void wmap_land_effect_25_sequence_17_step_01(void)
{
    s32 i;

    g_wmap_aux_effect_fade_b = 1;
    g_wmap_falling_spawn_counts[1] = 1;

    for (i = 110; i < 155; i++)
    {
        g_wmap_actor_motions[i].state = 0;
        WMAP_INIT_PARTICLE_ACTOR(i, g_wmap_animation_bank_3, 1);
    }

    g_wmap_falling_spawn_timers[1] = 2;
    g_wmap_land_effect_25_sequence_17_timer = 0x10;
    g_wmap_land_effect_25_sequence_17_step++;
    wmap_land_effect_25_sequence_17_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_25_run, g_wmap_land_effect_25_steps, 0x6, g_wmap_land_effect_25_step, g_wmap_land_effect_25_timer)

WMAP_STEP_RESET(wmap_land_effect_25_reset, g_wmap_land_effect_25_step, g_wmap_land_effect_25_timer)

/**
 * @brief Register a world-map callback and advance to the next step.
 */
void wmap_land_effect_25_step_01(void)
{
    g_wmap_models_use_effect_camera = 1;
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_25_step++;
    wmap_land_effect_25_wait_idle_02();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_25_wait_idle_02, g_wmap_land_effect_25_step, wmap_land_effect_25_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_25_step_03, g_wmap_land_effect_25_step, wmap_land_effect_25_run_timeline, wmap_land_effect_25_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_25_wait_idle_04, g_wmap_land_effect_25_step, wmap_land_effect_25_step_05)

/**
 * @brief World-map step handler: clear the shared flag and advance the step counter.
 */
void wmap_land_effect_25_step_05(void)
{
    g_wmap_models_use_effect_camera = 0;
    g_wmap_land_effect_25_step++;
}

WMAP_STEP_RUNNER(wmap_land_effect_25_run_timeline, g_wmap_land_effect_25_timeline_steps, 0x20, g_wmap_land_effect_25_timeline_step,
                 g_wmap_land_effect_25_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_25_timeline_reset, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

/** @brief World-map step: mark active, request a resource, seed the sub-counter, tick. */
void wmap_land_effect_25_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_play_sound(0x2F, 0x80);
    g_wmap_land_effect_25_timeline_timer = 2;
    g_wmap_land_effect_25_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_02, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_25_timeline_step_03, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer,
                         wmap_land_effect_25_run_sequence_1, 0x10)

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_04, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

/** @brief Register callbacks, set effect flags and color, and begin an eight-tick delay. */
void wmap_land_effect_25_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_25_run_sequence_15);
    wmap_start_sequence(wmap_land_effect_25_run_sequence_3);
    g_wmap_transition_mesh_hidden = 1;
    g_wmap_backdrop_target_level = 1;
    wmap_start_map_tint(0x701020);
    g_wmap_placement_overlay_hidden = 1;
    g_wmap_land_effect_25_timeline_timer = 8;
    g_wmap_land_effect_25_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_06, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

/** @brief Initialize the sequence transforms and start a forty-tick countdown. */
void wmap_land_effect_25_timeline_step_07(void)
{
    g_wmap_effect_camera_move_active = 1;
    wmap_install_callback(&wmap_run_effect_camera_move);
    g_wmap_effect_camera_rotation_step.vx = 5;
    g_wmap_effect_camera_rotation_step.vy = 0;
    g_wmap_effect_camera_rotation_step.vz = 0;
    g_wmap_effect_camera_translation_step.vx = 0;
    g_wmap_effect_camera_translation_step.vy = 2;
    g_wmap_effect_camera_translation_step.vz = 0;
    g_wmap_effect_camera_rotation_target.vx = 0xFA;
    g_wmap_effect_camera_rotation_target.vy = 0;
    g_wmap_effect_camera_rotation_target.vz = 0;
    g_wmap_effect_camera_translation_target.vx = 0;
    g_wmap_effect_camera_translation_target.vy = 0x64;
    g_wmap_effect_camera_translation_target.vz = 0;
    g_wmap_land_effect_25_timeline_timer = 0x28;
    g_wmap_land_effect_25_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_08, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

/**
 * @brief Start three sequences and set the wait timer.
 */
WMAP_STEP_START_THREE_AND_WAIT(wmap_land_effect_25_timeline_step_09, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer,
                               wmap_land_effect_25_run_sequence_6, wmap_land_effect_25_run_sequence_7, wmap_land_effect_25_run_sequence_14, 0x7)

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_10, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

/** @brief Set two effect colors, clear two colors, and begin a 35-tick sequence step. */
void wmap_land_effect_25_timeline_step_11(void)
{
    g_wmap_backdrop_target_level = 0;
    WMAP_SET_BACKDROP_TARGET(0x32, 0, 0xA0, 0, 0, 0);
    g_wmap_land_effect_25_timeline_timer = 0x23;
    g_wmap_land_effect_25_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_12, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_25_timeline_step_13, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer,
                             wmap_land_effect_25_run_sequence_18, wmap_land_effect_25_run_sequence_8, 0x20)

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_14, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_25_timeline_step_15, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer,
                         wmap_land_effect_25_run_sequence_12, 0x20)

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_16, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_25_timeline_step_17, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer,
                         wmap_land_effect_25_run_sequence_13, 0x5E)

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_18, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

/** @brief Register an effect callback, initialize four vectors, and advance the sequence. */
void wmap_land_effect_25_timeline_step_19(void)
{
    wmap_start_sequence(&wmap_land_effect_25_run_sequence_4);
    g_wmap_effect_camera_rotation_target.vx = 0;
    g_wmap_effect_camera_rotation_target.vy = 0;
    g_wmap_effect_camera_rotation_target.vz = 0;
    g_wmap_effect_camera_translation_target.vx = 0;
    g_wmap_effect_camera_translation_target.vy = 0;
    g_wmap_effect_camera_translation_target.vz = 0;
    g_wmap_effect_camera_rotation_step.vx = 0xFA;
    g_wmap_effect_camera_rotation_step.vy = 0;
    g_wmap_effect_camera_rotation_step.vz = 0;
    g_wmap_effect_camera_translation_step.vx = 0;
    g_wmap_effect_camera_translation_step.vy = 0x64;
    g_wmap_effect_camera_translation_step.vz = 0;
    g_wmap_land_effect_25_timeline_timer = 1;
    g_wmap_land_effect_25_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_20, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

/** @brief World-map step handler: register three callbacks, reset state, advance the step. */
void wmap_land_effect_25_timeline_step_21(void)
{
    wmap_start_sequence(wmap_land_effect_25_run_sequence_17);
    wmap_start_sequence(wmap_land_effect_25_run_sequence_11);
    wmap_start_sequence(wmap_land_effect_25_run_sequence_9);
    g_wmap_effect_camera_move_active = 0;
    g_wmap_backdrop_target_level = 3;
    g_wmap_land_effect_25_timeline_timer = 0x10;
    g_wmap_land_effect_25_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_22, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_25_timeline_step_23, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer,
                         wmap_land_effect_25_run_sequence_16, 0x40)

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_24, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_25_timeline_step_25, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer,
                         wmap_land_effect_25_run_sequence_2, 0x9C)

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_26, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_25_timeline_step_27, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer,
                         wmap_land_effect_25_run_sequence_10, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_28, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

/** @brief Register a callback, update the selected map cell, and begin a 80-tick delay. */
void wmap_land_effect_25_timeline_step_29(void)
{
    wmap_start_sequence(&wmap_land_effect_25_run_sequence_5);
    g_wmap_transition_mesh_hidden = 0;
    g_wmap_backdrop_target_level = 15;
    wmap_start_map_tint(0x606070);
    g_wmap_land_effect_25_timeline_timer = 0x50;
    g_wmap_cells[g_wmap_focus_cell_x][g_wmap_focus_cell_y].land_id = g_wmap_selected_artifact | WMAP_LAND_FULL_SIZE;
    g_wmap_land_effect_25_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_30, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

/**
 * @brief Clear the blocking flag and finish the timeline.
 */
WMAP_STEP_FINISH_BLOCKING(wmap_land_effect_25_timeline_finish, g_wmap_land_effect_25_timeline_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_1, g_wmap_land_effect_25_sequence_1_steps, 0x6, g_wmap_land_effect_25_sequence_1_step,
                 g_wmap_land_effect_25_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_1_reset, g_wmap_land_effect_25_sequence_1_step, g_wmap_land_effect_25_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_25_sequence_1_step_01, g_wmap_land_effect_25_sequence_1_step, g_wmap_land_effect_25_sequence_1_timer, 4,
                      g_wmap_animation_bank_0, 3, 1, 0x81, 8, 0x80, wmap_land_effect_25_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_1_step_02, g_wmap_land_effect_25_sequence_1_step, g_wmap_land_effect_25_sequence_1_timer,
                              g_wmap_sprite_actors[4], g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0x10, 0x5, 0)

/**
 * @brief Start the sprite fade and run its first update.
 */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_25_sequence_1_step_03, g_wmap_land_effect_25_sequence_1_step, g_wmap_land_effect_25_sequence_1_timer,
                     g_wmap_sprite_actors[4], 4, 0x20, wmap_land_effect_25_sequence_1_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_1_step_04, g_wmap_land_effect_25_sequence_1_step, g_wmap_land_effect_25_sequence_1_timer,
                              g_wmap_sprite_actors[4], g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0x10, 0x5, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_1_end, g_wmap_land_effect_25_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_2, g_wmap_land_effect_25_sequence_2_steps, 0x4, g_wmap_land_effect_25_sequence_2_step,
                 g_wmap_land_effect_25_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_2_reset, g_wmap_land_effect_25_sequence_2_step, g_wmap_land_effect_25_sequence_2_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_25_sequence_2_step_01, g_wmap_land_effect_25_sequence_2_step, g_wmap_land_effect_25_sequence_2_timer, 5,
                      g_wmap_animation_bank_0, 0, 0x80, 0x80, 2, 0xA0, wmap_land_effect_25_sequence_2_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_2_step_02, g_wmap_land_effect_25_sequence_2_step, g_wmap_land_effect_25_sequence_2_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x10, 0x8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_2_end, g_wmap_land_effect_25_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_3, g_wmap_land_effect_25_sequence_3_steps, 0x4, g_wmap_land_effect_25_sequence_3_step,
                 g_wmap_land_effect_25_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_3_reset, g_wmap_land_effect_25_sequence_3_step, g_wmap_land_effect_25_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_25_sequence_3_step_01, g_wmap_land_effect_25_sequence_3_step, g_wmap_land_effect_25_sequence_3_timer,
                     g_wmap_effect_model_a_rotation, g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40,
                     wmap_land_effect_25_sequence_3_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_3_end, g_wmap_land_effect_25_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_4, g_wmap_land_effect_25_sequence_4_steps, 0x4, g_wmap_land_effect_25_sequence_4_step,
                 g_wmap_land_effect_25_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_4_reset, g_wmap_land_effect_25_sequence_4_step, g_wmap_land_effect_25_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_25_sequence_4_step_01, g_wmap_land_effect_25_sequence_4_step, g_wmap_land_effect_25_sequence_4_timer,
                     g_wmap_effect_model_b_rotation, g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x40,
                     wmap_land_effect_25_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_4_end, g_wmap_land_effect_25_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_5, g_wmap_land_effect_25_sequence_5_steps, 0x4, g_wmap_land_effect_25_sequence_5_step,
                 g_wmap_land_effect_25_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_5_reset, g_wmap_land_effect_25_sequence_5_step, g_wmap_land_effect_25_sequence_5_timer)

WMAP_STEP_DROP_START(wmap_land_effect_25_sequence_5_step_01, g_wmap_land_effect_25_sequence_5_step, g_wmap_land_effect_25_sequence_5_timer,
                     g_wmap_effect_model_c_rotation, g_wmap_zero_rotation, g_wmap_effect_model_c_position, g_wmap_effect_fade_c, 0x80, 0xAFC8, 0x20,
                     wmap_land_effect_25_sequence_5_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_5_end, g_wmap_land_effect_25_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_6, g_wmap_land_effect_25_sequence_6_steps, 0x6, g_wmap_land_effect_25_sequence_6_step,
                 g_wmap_land_effect_25_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_6_reset, g_wmap_land_effect_25_sequence_6_step, g_wmap_land_effect_25_sequence_6_timer)

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void wmap_land_effect_25_sequence_6_step_01(void)
{
    g_wmap_effect_fade_e = 1;
    g_wmap_effect_model_f_rotation = g_wmap_zero_rotation;
    g_wmap_land_effect_25_sequence_6_timer = 0xB4;
    g_wmap_land_effect_25_sequence_6_step++;
    wmap_land_effect_25_sequence_6_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_25_sequence_6_step_03, g_wmap_land_effect_25_sequence_6_step, g_wmap_land_effect_25_sequence_6_timer, 0x40,
                    wmap_land_effect_25_sequence_6_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_6_end, g_wmap_land_effect_25_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_7, g_wmap_land_effect_25_sequence_7_steps, 0x6, g_wmap_land_effect_25_sequence_7_step,
                 g_wmap_land_effect_25_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_7_reset, g_wmap_land_effect_25_sequence_7_step, g_wmap_land_effect_25_sequence_7_timer)

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void wmap_land_effect_25_sequence_7_step_01(void)
{
    g_wmap_effect_fade_d = 1;
    g_wmap_effect_model_d_rotation = g_wmap_zero_rotation;
    g_wmap_land_effect_25_sequence_7_timer = 0xAE;
    g_wmap_land_effect_25_sequence_7_step++;
    wmap_land_effect_25_sequence_7_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_25_sequence_7_step_03, g_wmap_land_effect_25_sequence_7_step, g_wmap_land_effect_25_sequence_7_timer, 0x40,
                    wmap_land_effect_25_sequence_7_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_7_end, g_wmap_land_effect_25_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_8, g_wmap_land_effect_25_sequence_8_steps, 0x6, g_wmap_land_effect_25_sequence_8_step,
                 g_wmap_land_effect_25_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_8_reset, g_wmap_land_effect_25_sequence_8_step, g_wmap_land_effect_25_sequence_8_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_effect_25_sequence_8_step_01, g_wmap_land_effect_25_sequence_8_step, g_wmap_land_effect_25_sequence_8_timer,
                      g_wmap_effect_model_g_rotation, g_wmap_zero_rotation, g_wmap_aux_effect_fade_c, 1, g_wmap_particle_velocity_min, 0x84,
                      wmap_land_effect_25_sequence_8_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_25_sequence_8_step_03, g_wmap_land_effect_25_sequence_8_step, g_wmap_land_effect_25_sequence_8_timer, 0x20,
                    wmap_land_effect_25_sequence_8_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_8_end, g_wmap_land_effect_25_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_9, g_wmap_land_effect_25_sequence_9_steps, 0x6, g_wmap_land_effect_25_sequence_9_step,
                 g_wmap_land_effect_25_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_9_reset, g_wmap_land_effect_25_sequence_9_step, g_wmap_land_effect_25_sequence_9_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_effect_25_sequence_9_step_01, g_wmap_land_effect_25_sequence_9_step, g_wmap_land_effect_25_sequence_9_timer,
                      g_wmap_effect_model_h_rotation, g_wmap_zero_rotation, g_wmap_aux_effect_fade_a, 1, g_wmap_particle_velocity_range, 0x84,
                      wmap_land_effect_25_sequence_9_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_25_sequence_9_step_03, g_wmap_land_effect_25_sequence_9_step, g_wmap_land_effect_25_sequence_9_timer, 0x40,
                    wmap_land_effect_25_sequence_9_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_9_end, g_wmap_land_effect_25_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_10, g_wmap_land_effect_25_sequence_10_steps, 0x4, g_wmap_land_effect_25_sequence_10_step,
                 g_wmap_land_effect_25_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_10_reset, g_wmap_land_effect_25_sequence_10_step, g_wmap_land_effect_25_sequence_10_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_25_sequence_10_step_01, g_wmap_land_effect_25_sequence_10_step, g_wmap_land_effect_25_sequence_10_timer, 7,
                      g_wmap_animation_bank_0, 2, 0x81, 0x81, 8, 0x24, wmap_land_effect_25_sequence_10_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_10_step_02, g_wmap_land_effect_25_sequence_10_step, g_wmap_land_effect_25_sequence_10_timer,
                              g_wmap_sprite_actors[7], g_wmap_actor_animations[7], g_wmap_focus_screen_position, 0x10, 0x4, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_10_end, g_wmap_land_effect_25_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_11, g_wmap_land_effect_25_sequence_11_steps, 0x6, g_wmap_land_effect_25_sequence_11_step,
                 g_wmap_land_effect_25_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_11_reset, g_wmap_land_effect_25_sequence_11_step, g_wmap_land_effect_25_sequence_11_timer)

/** @brief Start the sprite animation and run its first update. */
WMAP_STEP_START_ACTOR(wmap_land_effect_25_sequence_11_step_01, g_wmap_land_effect_25_sequence_11_step, g_wmap_land_effect_25_sequence_11_timer, 6,
                      g_wmap_animation_bank_0, 1, 1, 0x81, 4, 0x50, wmap_land_effect_25_sequence_11_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_11_step_02, g_wmap_land_effect_25_sequence_11_step, g_wmap_land_effect_25_sequence_11_timer,
                              g_wmap_sprite_actors[6], g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x10, 0x7, 0)

/**
 * @brief Start the sprite fade and run its first update.
 */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_25_sequence_11_step_03, g_wmap_land_effect_25_sequence_11_step, g_wmap_land_effect_25_sequence_11_timer,
                     g_wmap_sprite_actors[6], 2, 0x40, wmap_land_effect_25_sequence_11_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_11_step_04, g_wmap_land_effect_25_sequence_11_step, g_wmap_land_effect_25_sequence_11_timer,
                              g_wmap_sprite_actors[6], g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x10, 0x7, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_11_end, g_wmap_land_effect_25_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_12, g_wmap_land_effect_25_sequence_12_steps, 0x6, g_wmap_land_effect_25_sequence_12_step,
                 g_wmap_land_effect_25_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_12_reset, g_wmap_land_effect_25_sequence_12_step, g_wmap_land_effect_25_sequence_12_timer)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_25_sequence_12_step_02, g_wmap_land_effect_25_sequence_12_step, g_wmap_land_effect_25_sequence_12_timer,
                          wmap_update_particle_system(&g_wmap_sprite_actors[20], &g_wmap_actor_animations[20], 0xA, 1, 0xFF, 2, 6, &g_wmap_effect_params[10]))

/** @brief World-map step handler: arm a config range, set the timer, and advance. */
void wmap_land_effect_25_sequence_12_step_03(void)
{
    s32 i;

    for (i = 0; i < 0xA; i++)
    {
        g_wmap_sprite_actors[i + 0x14].shade_step = 2;
    }

    g_wmap_land_effect_25_sequence_12_timer = 0x64;
    g_wmap_land_effect_25_sequence_12_step++;
    wmap_land_effect_25_sequence_12_step_04();
}

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_25_sequence_12_step_04, g_wmap_land_effect_25_sequence_12_step, g_wmap_land_effect_25_sequence_12_timer,
                          wmap_update_particle_system(&g_wmap_sprite_actors[20], &g_wmap_actor_animations[20], 0xA, 1, 0xFF, 2, 7, &g_wmap_effect_params[10]))

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_12_end, g_wmap_land_effect_25_sequence_12_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_13, g_wmap_land_effect_25_sequence_13_steps, 0x6, g_wmap_land_effect_25_sequence_13_step,
                 g_wmap_land_effect_25_sequence_13_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_13_reset, g_wmap_land_effect_25_sequence_13_step, g_wmap_land_effect_25_sequence_13_timer)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_25_sequence_13_step_02, g_wmap_land_effect_25_sequence_13_step, g_wmap_land_effect_25_sequence_13_timer,
                          wmap_update_particle_system(&g_wmap_sprite_actors[50], &g_wmap_actor_animations[50], 0x1E, 1, 0xFF, 4, 6, &g_wmap_effect_params[20]))

/** @brief World-map step handler: arm a config range, set the timer, and advance. */
void wmap_land_effect_25_sequence_13_step_03(void)
{
    s32 i;

    for (i = 0; i < 0x1E; i++)
    {
        g_wmap_sprite_actors[i + 0x32].shade_step = 4;
    }

    g_wmap_land_effect_25_sequence_13_timer = 0x40;
    g_wmap_land_effect_25_sequence_13_step++;
    wmap_land_effect_25_sequence_13_step_04();
}

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_25_sequence_13_step_04, g_wmap_land_effect_25_sequence_13_step, g_wmap_land_effect_25_sequence_13_timer,
                          wmap_update_particle_system(&g_wmap_sprite_actors[50], &g_wmap_actor_animations[50], 0x1E, 1, 0xFF, 4, 7, &g_wmap_effect_params[20]))

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_13_end, g_wmap_land_effect_25_sequence_13_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_14, g_wmap_land_effect_25_sequence_14_steps, 0x6, g_wmap_land_effect_25_sequence_14_step,
                 g_wmap_land_effect_25_sequence_14_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_14_reset, g_wmap_land_effect_25_sequence_14_step, g_wmap_land_effect_25_sequence_14_timer)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_25_sequence_14_step_02, g_wmap_land_effect_25_sequence_14_step, g_wmap_land_effect_25_sequence_14_timer,
                          wmap_update_box_particles((WmapBoxParticleConfig*)&g_wmap_effect_params[40]))

/** @brief World-map step handler: clear a small entry table, set the timer, advance the step. */
void wmap_land_effect_25_sequence_14_step_03(void)
{
    WmapSpriteActor* entries;
    s32 i;

    entries = &g_wmap_sprite_actors[200];

    for (i = 0; i < 0x28; i++)
    {
        entries[i].target_shade = 0;
        entries[i].shade_step = 2;
    }

    g_wmap_land_effect_25_sequence_14_timer = 0x40;
    g_wmap_land_effect_25_sequence_14_step++;
    wmap_land_effect_25_sequence_14_step_04();
}

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_25_sequence_14_step_04, g_wmap_land_effect_25_sequence_14_step, g_wmap_land_effect_25_sequence_14_timer,
                          wmap_update_box_particles((WmapBoxParticleConfig*)&g_wmap_effect_params[40]))

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_14_end, g_wmap_land_effect_25_sequence_14_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_15, g_wmap_land_effect_25_sequence_15_steps, 0x6, g_wmap_land_effect_25_sequence_15_step,
                 g_wmap_land_effect_25_sequence_15_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_15_reset, g_wmap_land_effect_25_sequence_15_step, g_wmap_land_effect_25_sequence_15_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_25_sequence_15_step_02, g_wmap_land_effect_25_sequence_15_step, g_wmap_land_effect_25_sequence_15_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[170], &g_wmap_actor_animations[170], 0x18, 0xFF, 0x1, 0x4, 0,
                                                          g_wmap_effect_params))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_25_sequence_15_step_03, g_wmap_land_effect_25_sequence_15_step, g_wmap_land_effect_25_sequence_15_timer,
                       g_wmap_effect_params[5], 0x40, wmap_land_effect_25_sequence_15_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_25_sequence_15_step_04, g_wmap_land_effect_25_sequence_15_step, g_wmap_land_effect_25_sequence_15_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[170], &g_wmap_actor_animations[170], 0x18, 0xFF, 0x1, 0x4, 0,
                                                          g_wmap_effect_params))

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_15_end, g_wmap_land_effect_25_sequence_15_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_16, g_wmap_land_effect_25_sequence_16_steps, 0x6, g_wmap_land_effect_25_sequence_16_step,
                 g_wmap_land_effect_25_sequence_16_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_16_reset, g_wmap_land_effect_25_sequence_16_step, g_wmap_land_effect_25_sequence_16_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_25_sequence_16_step_01, g_wmap_land_effect_25_sequence_16_step, g_wmap_land_effect_25_sequence_16_timer, 9,
                      g_wmap_animation_bank_2, 0, 1, 0x81, 4, 0x78, wmap_land_effect_25_sequence_16_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_16_step_02, g_wmap_land_effect_25_sequence_16_step, g_wmap_land_effect_25_sequence_16_timer,
                              g_wmap_sprite_actors[9], g_wmap_actor_animations[9], g_wmap_focus_screen_position, 0x8, 0x5, 0)

/**
 * @brief Start the sprite fade and run its first update.
 */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_25_sequence_16_step_03, g_wmap_land_effect_25_sequence_16_step, g_wmap_land_effect_25_sequence_16_timer,
                     g_wmap_sprite_actors[9], 2, 0x40, wmap_land_effect_25_sequence_16_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_16_step_04, g_wmap_land_effect_25_sequence_16_step, g_wmap_land_effect_25_sequence_16_timer,
                              g_wmap_sprite_actors[9], g_wmap_actor_animations[9], g_wmap_focus_screen_position, 0x8, 0x5, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_16_end, g_wmap_land_effect_25_sequence_16_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_17, g_wmap_land_effect_25_sequence_17_steps, 0x8, g_wmap_land_effect_25_sequence_17_step,
                 g_wmap_land_effect_25_sequence_17_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_17_reset, g_wmap_land_effect_25_sequence_17_step, g_wmap_land_effect_25_sequence_17_timer)

/**
 * @brief Update the particles and increase the shared fade value until the timer expires.
 */
WMAP_STEP_UPDATE_AND_RAMP(wmap_land_effect_25_sequence_17_step_02, g_wmap_land_effect_25_sequence_17_step, g_wmap_land_effect_25_sequence_17_timer,
                          g_wmap_aux_effect_fade_b, 8,
                          wmap_update_falling_particles(0x6E, 0x9B, 2, -1, 3, 2, 0x168, 8, -0x78, 0xF0, -0x78, 0xF0, 0x64, 0x81, 0x81, 4, 1))

WMAP_STEP_ARM_TIMER(wmap_land_effect_25_sequence_17_step_03, g_wmap_land_effect_25_sequence_17_step, g_wmap_land_effect_25_sequence_17_timer, 0x5A,
                    wmap_land_effect_25_sequence_17_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_25_sequence_17_step_04, g_wmap_land_effect_25_sequence_17_step, g_wmap_land_effect_25_sequence_17_timer,
                          wmap_update_falling_particles(0x6E, 0x9B, 2, -1, 3, 2, 0x168, 8, -0x78, 0xF0, -0x78, 0xF0, 0x64, 0x81, 0x81, 4, 1))

/**
 * @brief Stop spawning particles and keep updating those already active.
 */
WMAP_STEP_STOP_PARTICLE_SPAWNS(wmap_land_effect_25_sequence_17_step_05, g_wmap_land_effect_25_sequence_17_step, g_wmap_land_effect_25_sequence_17_timer,
                               g_wmap_falling_spawn_counts[1], 0x64, wmap_land_effect_25_sequence_17_step_06)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_25_sequence_17_step_06, g_wmap_land_effect_25_sequence_17_step, g_wmap_land_effect_25_sequence_17_timer,
                          wmap_update_falling_particles(0x6E, 0x9B, 2, -1, 3, 2, 0x168, 8, -0x78, 0xF0, -0x78, 0xF0, 0x64, 0x81, 0x81, 4, 1))

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_17_end, g_wmap_land_effect_25_sequence_17_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_18, g_wmap_land_effect_25_sequence_18_steps, 0x6, g_wmap_land_effect_25_sequence_18_step,
                 g_wmap_land_effect_25_sequence_18_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_18_reset, g_wmap_land_effect_25_sequence_18_step, g_wmap_land_effect_25_sequence_18_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_25_sequence_18_step_01, g_wmap_land_effect_25_sequence_18_step, g_wmap_land_effect_25_sequence_18_timer, 8,
                      g_wmap_animation_bank_1, 0, 1, 0x81, 4, 0x96, wmap_land_effect_25_sequence_18_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_18_step_02, g_wmap_land_effect_25_sequence_18_step, g_wmap_land_effect_25_sequence_18_timer,
                              g_wmap_sprite_actors[8], g_wmap_actor_animations[8], g_wmap_focus_screen_position, 0x11, 0x4, 0)

/**
 * @brief Start the sprite fade and run its first update.
 */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_25_sequence_18_step_03, g_wmap_land_effect_25_sequence_18_step, g_wmap_land_effect_25_sequence_18_timer,
                     g_wmap_sprite_actors[8], 4, 0x20, wmap_land_effect_25_sequence_18_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_18_step_04, g_wmap_land_effect_25_sequence_18_step, g_wmap_land_effect_25_sequence_18_timer,
                              g_wmap_sprite_actors[8], g_wmap_actor_animations[8], g_wmap_focus_screen_position, 0x11, 0x4, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_18_end, g_wmap_land_effect_25_sequence_18_step)
