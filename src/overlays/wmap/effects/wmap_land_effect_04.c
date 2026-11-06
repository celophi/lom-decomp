#include "common.h"
#include <inline_c.h>
#include <libgte.h>
#include <rand.h>

#include "../internal/wmap_actor_motion.h"
#include "../internal/wmap_cells.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_land_effect_04.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_star_effect.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_view_effects.h"
#include "sdk/gte_dmpsx_compat.h"

extern s32 g_wmap_land_effect_04_sequence_5_timer;
extern s32 g_wmap_land_effect_04_sequence_6_timer;
extern s32 g_wmap_land_effect_04_sequence_7_timer;
extern s32 g_wmap_land_effect_04_sequence_8_timer;
extern s32 g_wmap_land_effect_04_sequence_9_timer;
extern s32 g_wmap_land_effect_04_timer;
extern WmapSequenceStep g_wmap_land_effect_04_steps[];
extern s32 g_wmap_land_effect_04_timeline_timer;
extern WmapSequenceStep g_wmap_land_effect_04_timeline_steps[];
extern s32 g_wmap_land_effect_04_sequence_1_timer;
extern WmapSequenceStep g_wmap_land_effect_04_sequence_1_steps[];
extern s32 g_wmap_land_effect_04_sequence_2_timer;
extern WmapSequenceStep g_wmap_land_effect_04_sequence_2_steps[];
extern s32 g_wmap_land_effect_04_sequence_3_timer;
extern WmapSequenceStep g_wmap_land_effect_04_sequence_3_steps[];
extern s32 g_wmap_land_effect_04_sequence_4_timer;
extern WmapSequenceStep g_wmap_land_effect_04_sequence_4_steps[];
extern WmapSequenceStep g_wmap_land_effect_04_sequence_5_steps[];
extern WmapSequenceStep g_wmap_land_effect_04_sequence_6_steps[];
extern WmapSequenceStep g_wmap_land_effect_04_sequence_7_steps[];
extern WmapSequenceStep g_wmap_land_effect_04_sequence_8_steps[];
extern WmapSequenceStep g_wmap_land_effect_04_sequence_9_steps[];
extern u32 g_wmap_land_effect_04_sequence_5_step;

extern u32 g_wmap_land_effect_04_sequence_6_step;
extern u32 g_wmap_land_effect_04_sequence_7_step;
extern u32 g_wmap_land_effect_04_sequence_8_step;
extern u32 g_wmap_land_effect_04_sequence_9_step;
extern u32 g_wmap_land_effect_04_step;
extern u32 g_wmap_land_effect_04_timeline_step;
extern u32 g_wmap_land_effect_04_sequence_1_step;
extern u32 g_wmap_land_effect_04_sequence_2_step;
extern u32 g_wmap_land_effect_04_sequence_3_step;
extern u32 g_wmap_land_effect_04_sequence_4_step;

/** @brief Initialize four effect actors and their angular spacing. */
void wmap_land_effect_04_sequence_5_step_01(void)
{
    s32 i;
    s32* descriptor;
    WmapSpriteActor* actor;

    g_wmap_effect_model_e_rotation = g_wmap_zero_rotation;
    descriptor = g_wmap_effect_params;
    descriptor[0] = 2;
    descriptor[1] = 1;
    descriptor[2] = 2;
    g_wmap_effect_params[3] = 0;
    g_wmap_effect_params[4] = 3500;
    g_wmap_effect_params[5] = 96;
    g_wmap_effect_params[6] = 4;
    g_wmap_effect_params[7] = 0;
    g_wmap_effect_params[8] = 10;
    g_wmap_effect_params[9] = 8;

    for (i = 20; i < 80; i++)
    {
        g_wmap_actor_motions[i].state = 0;
    }

    for (i = 20; i < 80; i += 15)
    {
        actor = &g_wmap_sprite_actors[i];
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_2;
        actor->scale_index = WMAP_SPRITE_SCALE_FULL;
        actor->previous_sequence = -1;
        actor->resource_index = 0;
        actor->sequence = 0;
        actor->shade_step = 4;
        actor->target_shade = 1;
        actor->shade = 129;
        g_wmap_actor_motions[i].state = 1;
        g_wmap_actor_motions[i].radius = 150000;
        g_wmap_actor_motions[i].angle = g_wmap_effect_params[3];
        g_wmap_actor_motions[i].z = 0;
        g_wmap_effect_params[3] += 1024;
    }

    g_wmap_land_effect_04_sequence_5_timer = 40;
    g_wmap_land_effect_04_sequence_5_step++;
    wmap_land_effect_04_sequence_5_step_02();
}

/** @brief Draw and brighten two rotating layers, then advance their shared countdown. */
void wmap_land_effect_04_sequence_6_step_02(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_e_rotation);
    wmap_draw_model_default(g_wmap_load_buffer, 0, 16, 53, 0x7800, 1, g_wmap_effect_fade_f);
    g_wmap_effect_model_e_rotation.vz = (g_wmap_effect_model_e_rotation.vz + 12);
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_f_rotation);
    wmap_draw_model_default(g_wmap_load_buffer, 0, 16, 53, 0x7800, 1, g_wmap_effect_fade_f);
    g_wmap_effect_model_f_rotation.vz = (g_wmap_effect_model_f_rotation.vz - 4);
    PopMatrix();
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_f, 8, 129, intensity);
    remaining = g_wmap_land_effect_04_sequence_6_timer - 1;
    g_wmap_land_effect_04_sequence_6_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_04_sequence_6_step++;
    }
}

/** @brief Draw and fade two rotating layers, then advance their shared countdown. */
void wmap_land_effect_04_sequence_6_step_04(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_e_rotation);
    wmap_draw_model_default(g_wmap_load_buffer, 0, 16, 53, 0x7800, 1, g_wmap_effect_fade_f);
    g_wmap_effect_model_e_rotation.vz = (g_wmap_effect_model_e_rotation.vz + 12);
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_f_rotation);
    wmap_draw_model_default(g_wmap_load_buffer, 0, 16, 53, 0x7800, 1, g_wmap_effect_fade_f);
    g_wmap_effect_model_f_rotation.vz = (g_wmap_effect_model_f_rotation.vz - 4);
    PopMatrix();
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_f, 4, intensity);
    remaining = g_wmap_land_effect_04_sequence_6_timer - 1;
    g_wmap_land_effect_04_sequence_6_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_04_sequence_6_step++;
    }
}

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_04_sequence_7_step_02, g_wmap_land_effect_04_sequence_7_step, g_wmap_land_effect_04_sequence_7_timer,
                      g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, g_wmap_effect_model_pack_1, -3500, 1)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_04_sequence_8_step_02, g_wmap_land_effect_04_sequence_8_step, g_wmap_land_effect_04_sequence_8_timer,
                      g_wmap_effect_model_b_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, g_wmap_effect_model_pack_2, -3500, 2)

/** @brief Initialize randomized actors along a cosine depth curve. */
void wmap_land_effect_04_sequence_9_step_01(void)
{
    s32 i;
    WmapSpriteActor* actor;
    WmapMotion* motion;

    for (i = 100; i < 130; i++)
    {
        actor = &g_wmap_sprite_actors[i];
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_2;
        actor->scale_index = WMAP_SPRITE_SCALE_FULL;
        actor->sequence = 2;
        actor->previous_sequence = -1;
        actor->resource_index = 0;
        actor->shade_step = 1;
        actor->target_shade = 129;
        actor->shade = 1;
        motion = (WmapMotion*)((u8*)g_wmap_actor_motions + i * sizeof(WmapMotion));
        motion->angle = rand() & 4095;
        motion->z = ((i - 100) * 140) / 30;
        motion->radius = ccos(2048 - (((i - 100) * 1024) / 30)) * 140;
        motion->velocity = ((rand() * 100) >> 15) + 40;
    }

    g_wmap_land_effect_04_sequence_9_timer = 64;
    g_wmap_land_effect_04_sequence_9_step++;
    wmap_land_effect_04_sequence_9_step_02();
}

/**
 * @brief Project and draw the world-map star field, spinning each entry each frame.
 */
WMAP_DEFINE_STAR_UPDATE(wmap_land_effect_04_sequence_9_step_02, g_wmap_land_effect_04_sequence_9_step, g_wmap_land_effect_04_sequence_9_timer, 100, 130, 8)

/**
 * @brief Project and draw the world-map star field, spinning each entry each frame.
 */
WMAP_DEFINE_STAR_UPDATE(wmap_land_effect_04_sequence_9_step_04, g_wmap_land_effect_04_sequence_9_step, g_wmap_land_effect_04_sequence_9_timer, 100, 130, 8)

WMAP_STEP_RUNNER(wmap_land_effect_04_run, g_wmap_land_effect_04_steps, 0x6, g_wmap_land_effect_04_step, g_wmap_land_effect_04_timer)

WMAP_STEP_RESET(wmap_land_effect_04_reset, g_wmap_land_effect_04_step, g_wmap_land_effect_04_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_04_step_01, g_wmap_land_effect_04_step, wmap_run_land_focus, wmap_land_effect_04_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_04_wait_idle_02, g_wmap_land_effect_04_step, wmap_land_effect_04_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_04_step_03, g_wmap_land_effect_04_step, wmap_land_effect_04_run_timeline, wmap_land_effect_04_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_04_wait_idle_04, g_wmap_land_effect_04_step, wmap_land_effect_04_end)

WMAP_STEP_ADVANCE(wmap_land_effect_04_end, g_wmap_land_effect_04_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_timeline, g_wmap_land_effect_04_timeline_steps, 0x14, g_wmap_land_effect_04_timeline_step,
                 g_wmap_land_effect_04_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_04_timeline_reset, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

/** @brief World-map step handler: kick a sub-task and advance the step counter. */
void wmap_land_effect_04_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 4;
    g_wmap_land_effect_04_timeline_timer = 0x14;
    g_wmap_land_effect_04_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_02, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

/** @brief World-map step handler: register a callback and advance the step counter. */
void wmap_land_effect_04_timeline_step_03(void)
{
    wmap_play_sound(0x24, 0x80);
    wmap_start_sequence(wmap_land_effect_04_run_sequence_3);
    g_wmap_land_effect_04_timeline_timer = 4;
    g_wmap_land_effect_04_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_04, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_04_timeline_step_05, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer,
                         wmap_land_effect_04_run_sequence_5, 0x1E)

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_06, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

/** @brief Register two callbacks, set sequence flags, and begin a one-tick delay. */
void wmap_land_effect_04_timeline_step_07(void)
{
    wmap_start_sequence(&wmap_land_effect_04_run_sequence_7);
    g_wmap_backdrop_target_level = 2;
    g_wmap_transition_mesh_hidden = 1;
    wmap_start_sequence(&wmap_land_effect_04_run_sequence_2);
    g_wmap_placement_overlay_hidden = 1;
    g_wmap_land_effect_04_timeline_timer = 1;
    g_wmap_land_effect_04_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_08, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_04_timeline_step_09, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer,
                         wmap_land_effect_04_run_sequence_6, 0x3)

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_10, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_04_timeline_step_11, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer,
                         wmap_land_effect_04_run_sequence_1, 0x18)

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_12, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_04_timeline_step_13, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer,
                         wmap_land_effect_04_run_sequence_9, 0x1E)

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_14, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_04_timeline_step_15, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer,
                         wmap_land_effect_04_run_sequence_4, 0x3C)

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_16, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

/** @brief Register a callback, update the selected map cell, and begin a 68-tick delay. */
void wmap_land_effect_04_timeline_step_17(void)
{
    wmap_start_sequence(&wmap_land_effect_04_run_sequence_8);
    g_wmap_transition_mesh_hidden = 0;
    g_wmap_land_effect_04_timeline_timer = 0x44;
    g_wmap_cells[g_wmap_focus_cell_x][g_wmap_focus_cell_y].land_id = g_wmap_selected_artifact | WMAP_LAND_FULL_SIZE;
    g_wmap_land_effect_04_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_18, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

/**
 * @brief Clear the blocking flag and finish the timeline.
 */
WMAP_STEP_FINISH_BLOCKING(wmap_land_effect_04_timeline_finish, g_wmap_land_effect_04_timeline_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_1, g_wmap_land_effect_04_sequence_1_steps, 0x4, g_wmap_land_effect_04_sequence_1_step,
                 g_wmap_land_effect_04_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_1_reset, g_wmap_land_effect_04_sequence_1_step, g_wmap_land_effect_04_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_04_sequence_1_step_01, g_wmap_land_effect_04_sequence_1_step, g_wmap_land_effect_04_sequence_1_timer, 4,
                      g_wmap_animation_bank_1, 0, 0, 0x80, 8, 0x7C, wmap_land_effect_04_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_04_sequence_1_step_02, g_wmap_land_effect_04_sequence_1_step, g_wmap_land_effect_04_sequence_1_timer,
                              g_wmap_sprite_actors[4], g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0xB, 0x9, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_1_end, g_wmap_land_effect_04_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_2, g_wmap_land_effect_04_sequence_2_steps, 0x4, g_wmap_land_effect_04_sequence_2_step,
                 g_wmap_land_effect_04_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_2_reset, g_wmap_land_effect_04_sequence_2_step, g_wmap_land_effect_04_sequence_2_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_04_sequence_2_step_01(void)
{
    s32 one = 1;
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    g_wmap_actor_animations[6].data = g_wmap_animation_bank_1;
    actor->scale_index = WMAP_SPRITE_SCALE_FULL;
    actor->sequence = 1;
    actor->previous_sequence = -one;
    actor->shade_step = 2;
    actor->resource_index = 0;
    actor->target_shade = 0x81;
    actor->shade = 1;
    g_wmap_land_effect_04_sequence_2_timer = 0x7C;
    g_wmap_land_effect_04_sequence_2_step += one;
    wmap_land_effect_04_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_04_sequence_2_step_02, g_wmap_land_effect_04_sequence_2_step, g_wmap_land_effect_04_sequence_2_timer,
                              g_wmap_sprite_actors[6], g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0xB, 0xA, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_2_end, g_wmap_land_effect_04_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_3, g_wmap_land_effect_04_sequence_3_steps, 0x4, g_wmap_land_effect_04_sequence_3_step,
                 g_wmap_land_effect_04_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_3_reset, g_wmap_land_effect_04_sequence_3_step, g_wmap_land_effect_04_sequence_3_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_04_sequence_3_step_01, g_wmap_land_effect_04_sequence_3_step, g_wmap_land_effect_04_sequence_3_timer, 7,
                      g_wmap_animation_bank_1, 2, 1, 0x81, 0x10, 0x28, wmap_land_effect_04_sequence_3_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_04_sequence_3_step_02, g_wmap_land_effect_04_sequence_3_step, g_wmap_land_effect_04_sequence_3_timer,
                              g_wmap_sprite_actors[7], g_wmap_actor_animations[7], g_wmap_focus_screen_position, 0xB, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_3_end, g_wmap_land_effect_04_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_4, g_wmap_land_effect_04_sequence_4_steps, 0x4, g_wmap_land_effect_04_sequence_4_step,
                 g_wmap_land_effect_04_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_4_reset, g_wmap_land_effect_04_sequence_4_step, g_wmap_land_effect_04_sequence_4_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_04_sequence_4_step_01, g_wmap_land_effect_04_sequence_4_step, g_wmap_land_effect_04_sequence_4_timer, 5,
                      g_wmap_animation_bank_0, 0, 0, 0x80, 8, 0x3D, wmap_land_effect_04_sequence_4_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_04_sequence_4_step_02, g_wmap_land_effect_04_sequence_4_step, g_wmap_land_effect_04_sequence_4_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x1C, 0x8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_4_end, g_wmap_land_effect_04_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_5, g_wmap_land_effect_04_sequence_5_steps, 0x4, g_wmap_land_effect_04_sequence_5_step,
                 g_wmap_land_effect_04_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_5_reset, g_wmap_land_effect_04_sequence_5_step, g_wmap_land_effect_04_sequence_5_timer)

/** @brief Draw the sequence effect and advance when its countdown expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_04_sequence_5_step_02, g_wmap_land_effect_04_sequence_5_step, g_wmap_land_effect_04_sequence_5_timer,
                          wmap_update_particle_trails(0x14, 0x3C, (WmapTrailConfig*)g_wmap_effect_params, 0))

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_5_end, g_wmap_land_effect_04_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_6, g_wmap_land_effect_04_sequence_6_steps, 0x6, g_wmap_land_effect_04_sequence_6_step,
                 g_wmap_land_effect_04_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_6_reset, g_wmap_land_effect_04_sequence_6_step, g_wmap_land_effect_04_sequence_6_timer)

/** @brief World-map step: reset counters and advance to the next handler. */
void wmap_land_effect_04_sequence_6_step_01(void)
{
    g_wmap_effect_model_frame = 0;
    g_wmap_effect_fade_f = 1;
    g_wmap_effect_model_e_rotation.vx = 0;
    g_wmap_effect_model_e_rotation.vy = 0;
    g_wmap_effect_model_e_rotation.vz = 0;
    g_wmap_effect_model_f_rotation.vx = 0;
    g_wmap_effect_model_f_rotation.vy = 0;
    g_wmap_effect_model_f_rotation.vz = 0;
    g_wmap_land_effect_04_sequence_6_timer = 0x54;
    g_wmap_land_effect_04_sequence_6_step++;
    wmap_land_effect_04_sequence_6_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_04_sequence_6_step_03, g_wmap_land_effect_04_sequence_6_step, g_wmap_land_effect_04_sequence_6_timer, 0x20,
                    wmap_land_effect_04_sequence_6_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_6_end, g_wmap_land_effect_04_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_7, g_wmap_land_effect_04_sequence_7_steps, 0x4, g_wmap_land_effect_04_sequence_7_step,
                 g_wmap_land_effect_04_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_7_reset, g_wmap_land_effect_04_sequence_7_step, g_wmap_land_effect_04_sequence_7_timer)

WMAP_STEP_DROP_START(wmap_land_effect_04_sequence_7_step_01, g_wmap_land_effect_04_sequence_7_step, g_wmap_land_effect_04_sequence_7_timer,
                     g_wmap_effect_model_a_rotation, g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x80,
                     wmap_land_effect_04_sequence_7_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_7_end, g_wmap_land_effect_04_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_8, g_wmap_land_effect_04_sequence_8_steps, 0x4, g_wmap_land_effect_04_sequence_8_step,
                 g_wmap_land_effect_04_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_8_reset, g_wmap_land_effect_04_sequence_8_step, g_wmap_land_effect_04_sequence_8_timer)

WMAP_STEP_DROP_START(wmap_land_effect_04_sequence_8_step_01, g_wmap_land_effect_04_sequence_8_step, g_wmap_land_effect_04_sequence_8_timer,
                     g_wmap_effect_model_b_rotation, g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x40,
                     wmap_land_effect_04_sequence_8_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_8_end, g_wmap_land_effect_04_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_9, g_wmap_land_effect_04_sequence_9_steps, 0x6, g_wmap_land_effect_04_sequence_9_step,
                 g_wmap_land_effect_04_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_9_reset, g_wmap_land_effect_04_sequence_9_step, g_wmap_land_effect_04_sequence_9_timer)

/**
 * @brief Start fading the sprite range and run its first update.
 */
WMAP_STEP_FADE_ACTOR_RANGE(wmap_land_effect_04_sequence_9_step_03, g_wmap_land_effect_04_sequence_9_step, g_wmap_land_effect_04_sequence_9_timer, 0x64, 0x82, 2,
                           0x40, wmap_land_effect_04_sequence_9_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_9_end, g_wmap_land_effect_04_sequence_9_step)
