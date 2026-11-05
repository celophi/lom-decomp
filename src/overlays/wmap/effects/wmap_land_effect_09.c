#include "common.h"
#include <libgte.h>

#include "../internal/wmap_actor_motion.h"
#include "../internal/wmap_cells.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_land_effect_09.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_view_effects.h"

extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_09_sequence_3_timer;
extern s32 g_wmap_land_effect_09_sequence_4_timer;
extern s32 g_wmap_land_effect_09_sequence_5_timer;
extern s32 g_wmap_land_effect_09_sequence_6_timer;
extern s32 g_wmap_land_effect_09_sequence_7_timer;
extern s32 g_wmap_land_effect_09_sequence_8_timer;
extern s32 g_wmap_land_effect_09_sequence_9_timer;
extern s32 g_wmap_land_effect_09_timer;
extern WmapSequenceStep g_wmap_land_effect_09_steps[];
extern s32 g_wmap_land_effect_09_timeline_timer;
extern WmapSequenceStep g_wmap_land_effect_09_timeline_steps[];
extern s32 g_wmap_land_effect_09_sequence_1_timer;
extern WmapSequenceStep g_wmap_land_effect_09_sequence_1_steps[];
extern s32 g_wmap_land_effect_09_sequence_2_timer;
extern WmapSequenceStep g_wmap_land_effect_09_sequence_2_steps[];
extern WmapSequenceStep g_wmap_land_effect_09_sequence_3_steps[];
extern WmapSequenceStep g_wmap_land_effect_09_sequence_4_steps[];
extern WmapSequenceStep g_wmap_land_effect_09_sequence_5_steps[];
extern WmapSequenceStep g_wmap_land_effect_09_sequence_6_steps[];
extern WmapSequenceStep g_wmap_land_effect_09_sequence_7_steps[];
extern WmapSequenceStep g_wmap_land_effect_09_sequence_8_steps[];
extern WmapSequenceStep g_wmap_land_effect_09_sequence_9_steps[];
extern u32 g_wmap_land_effect_09_sequence_3_step;
extern u32 g_wmap_land_effect_09_sequence_4_step;
extern u32 g_wmap_land_effect_09_sequence_5_step;
extern u32 g_wmap_land_effect_09_sequence_6_step;
extern u32 g_wmap_land_effect_09_sequence_7_step;
extern u32 g_wmap_land_effect_09_sequence_8_step;
extern u32 g_wmap_land_effect_09_sequence_9_step;
extern u32 g_wmap_land_effect_09_step;
extern u32 g_wmap_land_effect_09_timeline_step;
extern u32 g_wmap_land_effect_09_sequence_1_step;
extern u32 g_wmap_land_effect_09_sequence_2_step;

extern s32* g_wmap_effect_params;

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_09_sequence_3_step_02, g_wmap_land_effect_09_sequence_3_step, g_wmap_land_effect_09_sequence_3_timer,
                      g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, g_wmap_load_buffer, -3500, 2)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_09_sequence_4_step_02, g_wmap_land_effect_09_sequence_4_step, g_wmap_land_effect_09_sequence_4_timer,
                      g_wmap_effect_model_b_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, g_wmap_effect_model_pack_1, -3500, 2)

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_09_sequence_5_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 50;
    WMAP_SET_PARTICLE_CONFIG(0, 0, 0, 255, 0, 2, 1000, 200, 8, 0, 18200);
    WMAP_RESET_PARTICLE_SLOTS(i, 50, g_wmap_actor_motions[i + g_wmap_effect_params[0x7]].state, 204, g_wmap_animation_bank_2);
    g_wmap_land_effect_09_sequence_5_timer = 100;
    g_wmap_land_effect_09_sequence_5_step++;
    wmap_land_effect_09_sequence_5_step_02();
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void wmap_land_effect_09_sequence_6_step_01(void)
{
    s32 i;

    g_wmap_effect_fade_c = 1;
    g_wmap_falling_spawn_counts[0] = 1;

    for (i = 150; i < 158; i++)
    {
        g_wmap_actor_motions[i].state = 0;
        WMAP_INIT_PARTICLE_ACTOR(i, g_wmap_animation_bank_2, 1);
    }

    g_wmap_falling_spawn_timers[0] = 3;
    g_wmap_land_effect_09_sequence_6_timer = 0x10;
    g_wmap_land_effect_09_sequence_6_step++;
    wmap_land_effect_09_sequence_6_step_02();
}

/** @brief Initialize the actor group and its animation resources, then advance. */
void wmap_land_effect_09_sequence_7_step_01(void)
{
    s32 i;
    WmapSpriteActor* actor;

    g_wmap_particle_intensity = 4;

    for (i = 140; i < 144; i++)
    {
        actor = &g_wmap_sprite_actors[i];
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_3;
        actor->resource_index = 0;
        actor->scale_index = 15;
        actor->sequence = 1;
        actor->previous_sequence = -1;
        actor->target_shade = 129;
        actor->shade = 129;
        actor->shade_step = 8;
        g_wmap_actor_motions[i].state = 1;
        g_wmap_actor_motions[i].angle = i << 10;
        g_wmap_actor_motions[i].velocity = -20000;
        g_wmap_actor_motions[i].radius = 990000;
        g_wmap_actor_motions[i].lifetime = 9999;
        g_wmap_actor_motions[i].z = 240;
        g_wmap_actor_motions[i].angular_velocity.low = 160;
    }

    g_wmap_land_effect_09_sequence_7_timer = 28;
    g_wmap_land_effect_09_sequence_7_step++;
    wmap_land_effect_09_sequence_7_step_02();
}

/** @brief World-map step: fill a spawn descriptor, clear its slot run, then advance. */
void wmap_land_effect_09_sequence_8_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 12;
    WMAP_SET_PARTICLE_CONFIG(30, 1, 3, 64, 30, -2, 1000, 20, 15, 0, 10000);
    WMAP_RESET_PARTICLE_SLOTS(i, 12, g_wmap_actor_motions[i + g_wmap_effect_params[0x25]].state, 24, g_wmap_animation_bank_3);
    g_wmap_land_effect_09_sequence_8_timer = 64;
    g_wmap_land_effect_09_sequence_8_step++;
    wmap_land_effect_09_sequence_8_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_09_sequence_9_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 20;
    g_wmap_effect_params[0xB] = 0;
    g_wmap_effect_params[0xC] = 0;
    g_wmap_effect_params[0xF] = 1;
    g_wmap_effect_params[0x10] = 120;
    g_wmap_effect_params[0x11] = 100;
    g_wmap_effect_params[0x12] = 15;
    g_wmap_effect_params[0x13] = 2;
    g_wmap_effect_params[0x14] = 18500;
    WMAP_RESET_PARTICLE_SLOTS(i, 20, g_wmap_actor_motions[i + g_wmap_effect_params[0x11]].state, 104, g_wmap_animation_bank_3);
    g_wmap_land_effect_09_sequence_9_timer = 20;
    g_wmap_land_effect_09_sequence_9_step++;
    wmap_land_effect_09_sequence_9_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_09_run, g_wmap_land_effect_09_steps, 0x6, g_wmap_land_effect_09_step, g_wmap_land_effect_09_timer)

WMAP_STEP_RESET(wmap_land_effect_09_reset, g_wmap_land_effect_09_step, g_wmap_land_effect_09_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_09_step_01, g_wmap_land_effect_09_step, wmap_run_land_focus, wmap_land_effect_09_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_09_wait_idle_02, g_wmap_land_effect_09_step, wmap_land_effect_09_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_09_step_03, g_wmap_land_effect_09_step, wmap_land_effect_09_run_timeline, wmap_land_effect_09_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_09_wait_idle_04, g_wmap_land_effect_09_step, wmap_land_effect_09_end)

WMAP_STEP_ADVANCE(wmap_land_effect_09_end, g_wmap_land_effect_09_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_timeline, g_wmap_land_effect_09_timeline_steps, 0x10, g_wmap_land_effect_09_timeline_step,
                 g_wmap_land_effect_09_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_09_timeline_reset, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

/**
 * @brief Set up a world-map sub-scene: request assets and register its handler.
 */
void wmap_land_effect_09_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_play_sound(0x30, 0x80);
    wmap_start_sequence(wmap_land_effect_09_run_sequence_7);
    g_wmap_land_effect_09_timeline_timer = 0x28;
    g_wmap_land_effect_09_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_02, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

/** @brief Register four callbacks, set effect color and flags, and begin a one-tick delay. */
void wmap_land_effect_09_timeline_step_03(void)
{
    wmap_start_sequence(&wmap_land_effect_09_run_sequence_3);
    wmap_start_sequence(&wmap_land_effect_09_run_sequence_5);
    wmap_start_sequence(&wmap_land_effect_09_run_sequence_8);
    g_wmap_transition_mesh_hidden = 1;
    wmap_start_sequence(&wmap_land_effect_09_run_sequence_1);
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 4;
    g_wmap_land_effect_09_timeline_timer = 1;
    g_wmap_land_effect_09_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_04, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

/**
 * @brief Hide the placement overlay and set the wait timer.
 */
WMAP_STEP_HIDE_AND_WAIT(wmap_land_effect_09_timeline_step_05, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer,
                        g_wmap_placement_overlay_hidden, 0x23)

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_06, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_09_timeline_step_07, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer,
                         wmap_land_effect_09_run_sequence_6, 0x8)

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_08, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_09_timeline_step_09, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer,
                         wmap_land_effect_09_run_sequence_9, 0x32)

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_10, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

/** @brief Register a callback, set world-map state and color, and begin a two-tick delay. */
void wmap_land_effect_09_timeline_step_11(void)
{
    wmap_start_sequence(&wmap_land_effect_09_run_sequence_4);
    g_wmap_transition_mesh_hidden = 0;
    g_wmap_backdrop_target_level = 0x10;
    wmap_start_map_tint(0x808080);
    g_wmap_land_effect_09_timeline_timer = 2;
    g_wmap_land_effect_09_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_12, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_09_timeline_step_13, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer,
                         wmap_land_effect_09_run_sequence_2, 0xAB)

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_14, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_09_timeline_finish, g_wmap_land_effect_09_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y,
                           g_wmap_selected_artifact)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_1, g_wmap_land_effect_09_sequence_1_steps, 0x4, g_wmap_land_effect_09_sequence_1_step,
                 g_wmap_land_effect_09_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_1_reset, g_wmap_land_effect_09_sequence_1_step, g_wmap_land_effect_09_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_09_sequence_1_step_01, g_wmap_land_effect_09_sequence_1_step, g_wmap_land_effect_09_sequence_1_timer, 4,
                      g_wmap_animation_bank_1, 0, 0x81, 0x81, 0, 0x4C, wmap_land_effect_09_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_09_sequence_1_step_02, g_wmap_land_effect_09_sequence_1_step, g_wmap_land_effect_09_sequence_1_timer,
                              g_wmap_sprite_actors[4], g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0xF, 0x4, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_1_end, g_wmap_land_effect_09_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_2, g_wmap_land_effect_09_sequence_2_steps, 0x4, g_wmap_land_effect_09_sequence_2_step,
                 g_wmap_land_effect_09_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_2_reset, g_wmap_land_effect_09_sequence_2_step, g_wmap_land_effect_09_sequence_2_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_09_sequence_2_step_01, g_wmap_land_effect_09_sequence_2_step, g_wmap_land_effect_09_sequence_2_timer, 5,
                      g_wmap_animation_bank_0, 0, 0, 0x80, 8, 0xAC, wmap_land_effect_09_sequence_2_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_09_sequence_2_step_02, g_wmap_land_effect_09_sequence_2_step, g_wmap_land_effect_09_sequence_2_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x17, 0x8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_2_end, g_wmap_land_effect_09_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_3, g_wmap_land_effect_09_sequence_3_steps, 0x4, g_wmap_land_effect_09_sequence_3_step,
                 g_wmap_land_effect_09_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_3_reset, g_wmap_land_effect_09_sequence_3_step, g_wmap_land_effect_09_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_09_sequence_3_step_01, g_wmap_land_effect_09_sequence_3_step, g_wmap_land_effect_09_sequence_3_timer,
                     g_wmap_effect_model_a_rotation, g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40,
                     wmap_land_effect_09_sequence_3_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_3_end, g_wmap_land_effect_09_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_4, g_wmap_land_effect_09_sequence_4_steps, 0x4, g_wmap_land_effect_09_sequence_4_step,
                 g_wmap_land_effect_09_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_4_reset, g_wmap_land_effect_09_sequence_4_step, g_wmap_land_effect_09_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_09_sequence_4_step_01, g_wmap_land_effect_09_sequence_4_step, g_wmap_land_effect_09_sequence_4_timer,
                     g_wmap_effect_model_b_rotation, g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x40,
                     wmap_land_effect_09_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_4_end, g_wmap_land_effect_09_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_5, g_wmap_land_effect_09_sequence_5_steps, 0x6, g_wmap_land_effect_09_sequence_5_step,
                 g_wmap_land_effect_09_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_5_reset, g_wmap_land_effect_09_sequence_5_step, g_wmap_land_effect_09_sequence_5_timer)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_5_step_02, g_wmap_land_effect_09_sequence_5_step, g_wmap_land_effect_09_sequence_5_timer,
                          wmap_update_particle_system(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 0x32, 0x7F, 1, 4, 5, g_wmap_effect_params))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_09_sequence_5_step_03, g_wmap_land_effect_09_sequence_5_step, g_wmap_land_effect_09_sequence_5_timer,
                       g_wmap_effect_params[5], 0x40, wmap_land_effect_09_sequence_5_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_5_step_04, g_wmap_land_effect_09_sequence_5_step, g_wmap_land_effect_09_sequence_5_timer,
                          wmap_update_particle_system(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 0x32, 0x7F, 1, 4, 5, g_wmap_effect_params))

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_5_end, g_wmap_land_effect_09_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_6, g_wmap_land_effect_09_sequence_6_steps, 0x8, g_wmap_land_effect_09_sequence_6_step,
                 g_wmap_land_effect_09_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_6_reset, g_wmap_land_effect_09_sequence_6_step, g_wmap_land_effect_09_sequence_6_timer)

/**
 * @brief Update the particles and increase the shared fade value until the timer expires.
 */
WMAP_STEP_UPDATE_AND_RAMP(wmap_land_effect_09_sequence_6_step_02, g_wmap_land_effect_09_sequence_6_step, g_wmap_land_effect_09_sequence_6_timer,
                          g_wmap_effect_fade_c, 8, wmap_update_falling_particles(0x96, 0x9E, 3, -1, -1, -5, 0, 8, -0x50, 0xA0, -0x50, 0xA0, 1, 0x7F, 1, 4, 0))

WMAP_STEP_ARM_TIMER(wmap_land_effect_09_sequence_6_step_03, g_wmap_land_effect_09_sequence_6_step, g_wmap_land_effect_09_sequence_6_timer, 0x18,
                    wmap_land_effect_09_sequence_6_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_6_step_04, g_wmap_land_effect_09_sequence_6_step, g_wmap_land_effect_09_sequence_6_timer,
                          wmap_update_falling_particles(0x96, 0x9E, 3, -1, -1, -5, 0, 8, -0x50, 0xA0, -0x50, 0xA0, 1, 0x7F, 1, 4, 0))

/**
 * @brief Stop spawning particles and keep updating those already active.
 */
WMAP_STEP_STOP_PARTICLE_SPAWNS(wmap_land_effect_09_sequence_6_step_05, g_wmap_land_effect_09_sequence_6_step, g_wmap_land_effect_09_sequence_6_timer,
                               g_wmap_falling_spawn_counts[0], 0x14, wmap_land_effect_09_sequence_6_step_06)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_6_step_06, g_wmap_land_effect_09_sequence_6_step, g_wmap_land_effect_09_sequence_6_timer,
                          wmap_update_falling_particles(0x96, 0x9E, 3, -1, -1, -5, 0, 8, -0x50, 0xA0, -0x50, 0xA0, 1, 0x7F, 1, 4, 0))

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_6_end, g_wmap_land_effect_09_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_7, g_wmap_land_effect_09_sequence_7_steps, 0x6, g_wmap_land_effect_09_sequence_7_step,
                 g_wmap_land_effect_09_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_7_reset, g_wmap_land_effect_09_sequence_7_step, g_wmap_land_effect_09_sequence_7_timer)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_7_step_02, g_wmap_land_effect_09_sequence_7_step, g_wmap_land_effect_09_sequence_7_timer,
                          wmap_update_spiral_particles(0x8C, 0x90, 0xF, -6, 4))

/**
 * @brief Start fading the sprite range and run its first update.
 */
WMAP_STEP_FADE_ACTOR_RANGE(wmap_land_effect_09_sequence_7_step_03, g_wmap_land_effect_09_sequence_7_step, g_wmap_land_effect_09_sequence_7_timer, 0x8C, 0x90, 8,
                           0x10, wmap_land_effect_09_sequence_7_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_7_step_04, g_wmap_land_effect_09_sequence_7_step, g_wmap_land_effect_09_sequence_7_timer,
                          wmap_update_spiral_particles(0x8C, 0x90, 0xF, -6, 4))

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_7_end, g_wmap_land_effect_09_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_8, g_wmap_land_effect_09_sequence_8_steps, 0x6, g_wmap_land_effect_09_sequence_8_step,
                 g_wmap_land_effect_09_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_8_reset, g_wmap_land_effect_09_sequence_8_step, g_wmap_land_effect_09_sequence_8_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_8_step_02, g_wmap_land_effect_09_sequence_8_step, g_wmap_land_effect_09_sequence_8_timer,
                          wmap_update_particle_system(&g_wmap_sprite_actors[24], &g_wmap_actor_animations[24], 0xC, 0, 0x7F, 0x2, 0, &g_wmap_effect_params[30]))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_09_sequence_8_step_03, g_wmap_land_effect_09_sequence_8_step, g_wmap_land_effect_09_sequence_8_timer,
                       g_wmap_effect_params[35], 0x20, wmap_land_effect_09_sequence_8_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_8_step_04, g_wmap_land_effect_09_sequence_8_step, g_wmap_land_effect_09_sequence_8_timer,
                          wmap_update_particle_system(&g_wmap_sprite_actors[24], &g_wmap_actor_animations[24], 0xC, 0, 0x7F, 0x2, 0, &g_wmap_effect_params[30]))

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_8_end, g_wmap_land_effect_09_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_9, g_wmap_land_effect_09_sequence_9_steps, 0x6, g_wmap_land_effect_09_sequence_9_step,
                 g_wmap_land_effect_09_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_9_reset, g_wmap_land_effect_09_sequence_9_step, g_wmap_land_effect_09_sequence_9_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_9_step_02, g_wmap_land_effect_09_sequence_9_step, g_wmap_land_effect_09_sequence_9_timer,
                          wmap_update_particle_system(&g_wmap_sprite_actors[104], &g_wmap_actor_animations[104], 0x14, 0x7F, 0x7F, 0x4, 0,
                                                      &g_wmap_effect_params[10]))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_09_sequence_9_step_03, g_wmap_land_effect_09_sequence_9_step, g_wmap_land_effect_09_sequence_9_timer,
                       g_wmap_effect_params[15], 0x20, wmap_land_effect_09_sequence_9_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_9_step_04, g_wmap_land_effect_09_sequence_9_step, g_wmap_land_effect_09_sequence_9_timer,
                          wmap_update_particle_system(&g_wmap_sprite_actors[104], &g_wmap_actor_animations[104], 0x14, 0x7F, 0x7F, 0x4, 0,
                                                      &g_wmap_effect_params[10]))

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_9_end, g_wmap_land_effect_09_sequence_9_step)
