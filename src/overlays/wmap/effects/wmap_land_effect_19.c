#include "common.h"
#include <libgte.h>

#include "../internal/wmap_actor_motion.h"
#include "../internal/wmap_cells.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_land_effect_19.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_model_render.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_view_effects.h"

extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_19_sequence_3_timer;
extern s32 g_wmap_land_effect_19_sequence_4_timer;
extern s32 g_wmap_land_effect_19_sequence_5_timer;
extern s32 g_wmap_land_effect_19_sequence_6_timer;
extern s32 g_wmap_land_effect_19_sequence_7_timer;
extern s32 g_wmap_land_effect_19_sequence_8_timer;
extern s32 g_wmap_land_effect_19_sequence_9_timer;
extern s32 rand(void);
extern s32 g_wmap_land_effect_19_sequence_10_timer;
extern s32 g_wmap_land_effect_19_sequence_11_timer;
extern s32 g_wmap_land_effect_19_timer;
extern WmapSequenceStep g_wmap_land_effect_19_steps[];
extern s32 g_wmap_land_effect_19_timeline_timer;
extern WmapSequenceStep g_wmap_land_effect_19_timeline_steps[];
extern s32 g_wmap_land_effect_19_sequence_1_timer;
extern WmapSequenceStep g_wmap_land_effect_19_sequence_1_steps[];
extern s32 g_wmap_land_effect_19_sequence_2_timer;
extern WmapSequenceStep g_wmap_land_effect_19_sequence_2_steps[];
extern WmapSequenceStep g_wmap_land_effect_19_sequence_3_steps[];
extern WmapSequenceStep g_wmap_land_effect_19_sequence_4_steps[];
extern WmapSequenceStep g_wmap_land_effect_19_sequence_5_steps[];
extern WmapSequenceStep g_wmap_land_effect_19_sequence_6_steps[];
extern WmapSequenceStep g_wmap_land_effect_19_sequence_7_steps[];
extern WmapSequenceStep g_wmap_land_effect_19_sequence_8_steps[];
extern WmapSequenceStep g_wmap_land_effect_19_sequence_9_steps[];
extern WmapSequenceStep g_wmap_land_effect_19_sequence_10_steps[];
extern WmapSequenceStep g_wmap_land_effect_19_sequence_11_steps[];
extern s32 g_wmap_land_effect_19_sequence_12_timer;
extern WmapSequenceStep g_wmap_land_effect_19_sequence_12_steps[];

extern u32 g_wmap_land_effect_19_sequence_3_step;
extern u32 g_wmap_land_effect_19_sequence_4_step;
extern u32 g_wmap_land_effect_19_sequence_5_step;
extern u32 g_wmap_land_effect_19_sequence_6_step;
extern u32 g_wmap_land_effect_19_sequence_7_step;
extern u32 g_wmap_land_effect_19_sequence_8_step;
extern u32 g_wmap_land_effect_19_sequence_9_step;
extern u32 g_wmap_land_effect_19_sequence_10_step;
extern u32 g_wmap_land_effect_19_sequence_11_step;
extern u32 g_wmap_land_effect_19_step;
extern u32 g_wmap_land_effect_19_timeline_step;
extern u32 g_wmap_land_effect_19_sequence_1_step;
extern u32 g_wmap_land_effect_19_sequence_2_step;
extern u32 g_wmap_land_effect_19_sequence_12_step;

extern s32* g_wmap_effect_params;

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_19_sequence_3_step_02, g_wmap_land_effect_19_sequence_3_step, g_wmap_land_effect_19_sequence_3_timer,
                      g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, g_wmap_load_buffer, -3500, 2)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_19_sequence_4_step_02, g_wmap_land_effect_19_sequence_4_step, g_wmap_land_effect_19_sequence_4_timer,
                      g_wmap_effect_model_b_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, g_wmap_effect_model_pack_1, -3500, 1)

/**
 * @brief Draw and spin the model while fading it in.
 */
void wmap_land_effect_19_sequence_5_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_c_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_2, 0, 0xA, 0x36, 0x7880, 0x1001, g_wmap_effect_fade_c, 0, 0xA, -1);
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_c, 4, 0x81, value);
    timer = g_wmap_land_effect_19_sequence_5_timer;
    g_wmap_effect_model_c_rotation.vz += 0x16;
    next_timer = timer - 1;
    g_wmap_land_effect_19_sequence_5_timer = next_timer;

    if (next_timer == 0)
    {
        g_wmap_land_effect_19_sequence_5_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it out.
 */
void wmap_land_effect_19_sequence_5_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_c_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_2, 0, 0xA, 0x36, 0x7880, 0x1001, g_wmap_effect_fade_c, 0, 0xA, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_c, 2, value);
    timer = g_wmap_land_effect_19_sequence_5_timer;
    g_wmap_effect_model_c_rotation.vz += 0x16;
    next_timer = timer - 1;
    g_wmap_land_effect_19_sequence_5_timer = next_timer;

    if (next_timer == 0)
    {
        g_wmap_land_effect_19_sequence_5_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it in.
 */
void wmap_land_effect_19_sequence_6_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_d_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_2, 0, 0xA, 0x36, 0x78C0, 0x1001, g_wmap_effect_fade_d, 0, 0xA, -1);
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_d, 8, 0x81, value);
    timer = g_wmap_land_effect_19_sequence_6_timer;
    g_wmap_effect_model_d_rotation.vz += 0x16;
    next_timer = timer - 1;
    g_wmap_land_effect_19_sequence_6_timer = next_timer;

    if (next_timer == 0)
    {
        g_wmap_land_effect_19_sequence_6_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it out.
 */
void wmap_land_effect_19_sequence_6_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_d_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_2, 0, 0xA, 0x36, 0x78C0, 0x1001, g_wmap_effect_fade_d, 0, 0xA, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_d, 2, value);
    timer = g_wmap_land_effect_19_sequence_6_timer;
    g_wmap_effect_model_d_rotation.vz += 0x16;
    next_timer = timer - 1;
    g_wmap_land_effect_19_sequence_6_timer = next_timer;

    if (next_timer == 0)
    {
        g_wmap_land_effect_19_sequence_6_step++;
    }
}

/**
 * @brief Reset a range of world-map actor slots and kick the next handler.
 * @note Clears three parallel slot arrays for indices [0xA, 0x14), seeds each
 *       actor's default fields, then advances the shared step counters.
 */
void wmap_land_effect_19_sequence_7_step_01(void)
{
    s32 i;

    D_80182DE4 = 1;
    g_wmap_falling_spawn_counts[0] = 1;

    for (i = 0xA; i < 0x14; i++)
    {
        *(s16*)((u8*)g_wmap_actor_motions + i * 0x14) = 0;
        WMAP_INIT_PARTICLE_ACTOR(i, (u8*)&g_wmap_animation_bank_1, 0);
    }

    g_wmap_falling_spawn_timers[0] = 6;
    g_wmap_land_effect_19_sequence_7_timer = 0x10;
    g_wmap_land_effect_19_sequence_7_step++;
    wmap_land_effect_19_sequence_7_step_02();
}

/** @brief Configure the effect block, reset its twelve resource slots, and advance the step. */
void wmap_land_effect_19_sequence_8_step_01(void)
{
    s32 i;

    g_wmap_effect_params[1] = 0;
    g_wmap_effect_params[2] = 0;
    g_wmap_effect_params[3] = 0x20;
    g_wmap_effect_params[4] = 0;
    g_wmap_effect_params[5] = 2;
    g_wmap_effect_params[6] = 0x384;
    g_wmap_effect_params[7] = 0x14;
    g_wmap_effect_params[8] = 8;
    g_wmap_effect_params[9] = 1;
    g_wmap_effect_params[10] = 0x32C8;

    WMAP_RESET_PARTICLE_SLOTS(i, 0xC, g_wmap_actor_motions[i + 20].state, 20, g_wmap_animation_bank_3);

    g_wmap_land_effect_19_sequence_8_timer = 0x18;
    g_wmap_land_effect_19_sequence_8_step++;
    wmap_land_effect_19_sequence_8_step_02();
}

/** @brief Reset the effect actors with randomized animation variants. */
void wmap_land_effect_19_sequence_9_step_01(void)
{
    s32 i;

    D_801B25D8 = 1;
    g_wmap_falling_spawn_counts[1] = 1;

    for (i = 80; i < 140; i++)
    {
        g_wmap_actor_motions[i].state = 0;
        WMAP_INIT_PARTICLE_ACTOR(i, g_wmap_animation_bank_1, rand() % 3 + 2);
    }

    g_wmap_falling_spawn_timers[1] = 2;
    g_wmap_land_effect_19_sequence_9_timer = 16;
    g_wmap_land_effect_19_sequence_9_step++;
    wmap_land_effect_19_sequence_9_step_02();
}

/** @brief Initialize sixty alternating actors and begin their timed effect. */
void wmap_land_effect_19_sequence_10_step_01(void)
{
    s32 i;

    g_wmap_aux_effect_fade_a = 1;
    g_wmap_falling_spawn_counts[2] = 12;

    for (i = 150; i < 210; i++)
    {
        g_wmap_actor_motions[i].state = 0;
        WMAP_INIT_PARTICLE_ACTOR(i, g_wmap_animation_bank_2, i & 1);
    }

    g_wmap_falling_spawn_timers[2] = 1;
    g_wmap_land_effect_19_sequence_10_timer = 16;
    g_wmap_land_effect_19_sequence_10_step++;
    wmap_land_effect_19_sequence_10_step_02();
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void wmap_land_effect_19_sequence_11_step_01(void)
{
    s32 i;

    D_801B25D8 = 1;
    g_wmap_falling_spawn_counts[3] = 8;

    for (i = 0xD2; i < 0xE6; i++)
    {
        g_wmap_actor_motions[i].state = 0;
        WMAP_INIT_PARTICLE_ACTOR(i, g_wmap_animation_bank_3, 0);
    }

    g_wmap_falling_spawn_timers[3] = 2;
    g_wmap_land_effect_19_sequence_11_timer = 0x10;
    g_wmap_land_effect_19_sequence_11_step++;
    wmap_land_effect_19_sequence_11_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_19_run, g_wmap_land_effect_19_steps, 0x6, g_wmap_land_effect_19_step, g_wmap_land_effect_19_timer)

WMAP_STEP_RESET(wmap_land_effect_19_reset, g_wmap_land_effect_19_step, g_wmap_land_effect_19_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_19_step_01, g_wmap_land_effect_19_step, wmap_run_land_focus, wmap_land_effect_19_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_19_wait_idle_02, g_wmap_land_effect_19_step, wmap_land_effect_19_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_19_step_03, g_wmap_land_effect_19_step, wmap_land_effect_19_run_timeline, wmap_land_effect_19_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_19_wait_idle_04, g_wmap_land_effect_19_step, wmap_land_effect_19_end)

WMAP_STEP_ADVANCE(wmap_land_effect_19_end, g_wmap_land_effect_19_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_timeline, g_wmap_land_effect_19_timeline_steps, 0x18, g_wmap_land_effect_19_timeline_step,
                 g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_19_timeline_reset, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

/**
 * @brief Set up a tinted world-map sub-scene and register its handler.
 */
void wmap_land_effect_19_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x702540);
    g_wmap_backdrop_target_level = 8;
    wmap_play_sound(0x2B, 0x80);
    wmap_start_sequence(wmap_land_effect_19_run_sequence_7);
    g_wmap_land_effect_19_timeline_timer = 0x20;
    g_wmap_land_effect_19_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_02, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

/** @brief Register callbacks, set effect flags and color, and begin a ten-tick delay. */
void wmap_land_effect_19_timeline_step_03(void)
{
    wmap_start_sequence(&wmap_land_effect_19_run_sequence_3);
    g_wmap_transition_mesh_hidden = 1;
    g_wmap_placement_overlay_hidden = 1;
    wmap_start_sequence(&wmap_land_effect_19_run_sequence_1);
    g_wmap_backdrop_target_level = 1;
    wmap_start_map_tint(0x201010);
    g_wmap_land_effect_19_timeline_timer = 0xA;
    g_wmap_land_effect_19_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_04, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_19_timeline_step_05, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer,
                         wmap_land_effect_19_run_sequence_5, 0x1E)

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_06, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_19_timeline_step_07, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer,
                         wmap_land_effect_19_run_sequence_6, 0xC)

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_08, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_19_timeline_step_09, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer,
                             wmap_land_effect_19_run_sequence_10, wmap_land_effect_19_run_sequence_11, 0x44)

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_10, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_19_timeline_step_11, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer,
                         wmap_land_effect_19_run_sequence_4, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_12, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

/** @brief World-map step: kick a sub-request, set the next state, and arm the timer. */
void wmap_land_effect_19_timeline_step_13(void)
{
    g_wmap_transition_mesh_hidden = 0;
    wmap_start_map_tint(0x504060);
    g_wmap_backdrop_target_level = 8;
    g_wmap_land_effect_19_timeline_timer = 0x1E;
    g_wmap_land_effect_19_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_14, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_19_timeline_step_15, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer,
                         wmap_land_effect_19_run_sequence_9, 0x10)

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_16, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_19_timeline_step_17, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer,
                             wmap_land_effect_19_run_sequence_8, wmap_land_effect_19_run_sequence_2, 0x64)

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_18, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_19_timeline_step_19, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer,
                         wmap_land_effect_19_run_sequence_12, 0x4)

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_20, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_19_timeline_step_21, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer,
                         wmap_land_effect_19_run_sequence_8, 0x7C)

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_22, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_19_timeline_finish, g_wmap_land_effect_19_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y,
                           g_wmap_selected_artifact)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_1, g_wmap_land_effect_19_sequence_1_steps, 0x4, g_wmap_land_effect_19_sequence_1_step,
                 g_wmap_land_effect_19_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_1_reset, g_wmap_land_effect_19_sequence_1_step, g_wmap_land_effect_19_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_19_sequence_1_step_01, g_wmap_land_effect_19_sequence_1_step, g_wmap_land_effect_19_sequence_1_timer, 4,
                      g_wmap_animation_bank_1, 1, 1, 0x81, 8, 0x80, wmap_land_effect_19_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_19_sequence_1_step_02, g_wmap_land_effect_19_sequence_1_step, g_wmap_land_effect_19_sequence_1_timer,
                              g_wmap_sprite_actors[4], g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0x19, 0x8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_1_end, g_wmap_land_effect_19_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_2, g_wmap_land_effect_19_sequence_2_steps, 0x4, g_wmap_land_effect_19_sequence_2_step,
                 g_wmap_land_effect_19_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_2_reset, g_wmap_land_effect_19_sequence_2_step, g_wmap_land_effect_19_sequence_2_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_19_sequence_2_step_01, g_wmap_land_effect_19_sequence_2_step, g_wmap_land_effect_19_sequence_2_timer, 5,
                      g_wmap_animation_bank_0, 0, 0, 0x80, 4, 0x88, wmap_land_effect_19_sequence_2_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_19_sequence_2_step_02, g_wmap_land_effect_19_sequence_2_step, g_wmap_land_effect_19_sequence_2_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x1F, 0x8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_2_end, g_wmap_land_effect_19_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_3, g_wmap_land_effect_19_sequence_3_steps, 0x4, g_wmap_land_effect_19_sequence_3_step,
                 g_wmap_land_effect_19_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_3_reset, g_wmap_land_effect_19_sequence_3_step, g_wmap_land_effect_19_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_19_sequence_3_step_01, g_wmap_land_effect_19_sequence_3_step, g_wmap_land_effect_19_sequence_3_timer,
                     g_wmap_effect_model_a_rotation, g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40,
                     wmap_land_effect_19_sequence_3_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_3_end, g_wmap_land_effect_19_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_4, g_wmap_land_effect_19_sequence_4_steps, 0x4, g_wmap_land_effect_19_sequence_4_step,
                 g_wmap_land_effect_19_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_4_reset, g_wmap_land_effect_19_sequence_4_step, g_wmap_land_effect_19_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_19_sequence_4_step_01, g_wmap_land_effect_19_sequence_4_step, g_wmap_land_effect_19_sequence_4_timer,
                     g_wmap_effect_model_b_rotation, g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x80,
                     wmap_land_effect_19_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_4_end, g_wmap_land_effect_19_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_5, g_wmap_land_effect_19_sequence_5_steps, 0x6, g_wmap_land_effect_19_sequence_5_step,
                 g_wmap_land_effect_19_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_5_reset, g_wmap_land_effect_19_sequence_5_step, g_wmap_land_effect_19_sequence_5_timer)

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void wmap_land_effect_19_sequence_5_step_01(void)
{
    g_wmap_effect_fade_c = 1;
    g_wmap_effect_model_c_rotation = g_wmap_zero_rotation;
    g_wmap_land_effect_19_sequence_5_timer = 0x40;
    g_wmap_land_effect_19_sequence_5_step++;
    wmap_land_effect_19_sequence_5_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_19_sequence_5_step_03, g_wmap_land_effect_19_sequence_5_step, g_wmap_land_effect_19_sequence_5_timer, 0x40,
                    wmap_land_effect_19_sequence_5_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_5_end, g_wmap_land_effect_19_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_6, g_wmap_land_effect_19_sequence_6_steps, 0x6, g_wmap_land_effect_19_sequence_6_step,
                 g_wmap_land_effect_19_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_6_reset, g_wmap_land_effect_19_sequence_6_step, g_wmap_land_effect_19_sequence_6_timer)

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void wmap_land_effect_19_sequence_6_step_01(void)
{
    g_wmap_effect_fade_d = 1;
    g_wmap_effect_model_d_rotation = g_wmap_zero_rotation;
    g_wmap_land_effect_19_sequence_6_timer = 0x14;
    g_wmap_land_effect_19_sequence_6_step++;
    wmap_land_effect_19_sequence_6_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_19_sequence_6_step_03, g_wmap_land_effect_19_sequence_6_step, g_wmap_land_effect_19_sequence_6_timer, 0x40,
                    wmap_land_effect_19_sequence_6_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_6_end, g_wmap_land_effect_19_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_7, g_wmap_land_effect_19_sequence_7_steps, 0x8, g_wmap_land_effect_19_sequence_7_step,
                 g_wmap_land_effect_19_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_7_reset, g_wmap_land_effect_19_sequence_7_step, g_wmap_land_effect_19_sequence_7_timer)

/**
 * @brief Update the particles and increase the shared fade value until the timer expires.
 */
WMAP_STEP_UPDATE_AND_RAMP(wmap_land_effect_19_sequence_7_step_02, g_wmap_land_effect_19_sequence_7_step, g_wmap_land_effect_19_sequence_7_timer, D_80182DE4, 8,
                          wmap_update_falling_particles(0xA, 0x14, 6, -1, -1, -6, 0, 0x19, -0x32, 0x64, -0x32, 0x64, 1, 0x7F, 0x7F, 0, 0))

WMAP_STEP_ARM_TIMER(wmap_land_effect_19_sequence_7_step_03, g_wmap_land_effect_19_sequence_7_step, g_wmap_land_effect_19_sequence_7_timer, 0x3C,
                    wmap_land_effect_19_sequence_7_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_19_sequence_7_step_04, g_wmap_land_effect_19_sequence_7_step, g_wmap_land_effect_19_sequence_7_timer,
                          wmap_update_falling_particles(0xA, 0x14, 6, -1, -1, -6, 0, 0x19, -0x32, 0x64, -0x32, 0x64, 1, 0x7F, 0x7F, 0, 0))

/**
 * @brief Stop spawning particles and keep updating those already active.
 */
WMAP_STEP_STOP_PARTICLE_SPAWNS(wmap_land_effect_19_sequence_7_step_05, g_wmap_land_effect_19_sequence_7_step, g_wmap_land_effect_19_sequence_7_timer,
                               g_wmap_falling_spawn_counts[0], 0x14, wmap_land_effect_19_sequence_7_step_06)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_19_sequence_7_step_06, g_wmap_land_effect_19_sequence_7_step, g_wmap_land_effect_19_sequence_7_timer,
                          wmap_update_falling_particles(0xA, 0x14, 6, -1, -1, -6, 0, 0x19, -0x32, 0x64, -0x32, 0x64, 1, 0x7F, 0x7F, 0, 0))

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_7_end, g_wmap_land_effect_19_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_8, g_wmap_land_effect_19_sequence_8_steps, 0x6, g_wmap_land_effect_19_sequence_8_step,
                 g_wmap_land_effect_19_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_8_reset, g_wmap_land_effect_19_sequence_8_step, g_wmap_land_effect_19_sequence_8_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_19_sequence_8_step_02, g_wmap_land_effect_19_sequence_8_step, g_wmap_land_effect_19_sequence_8_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[20], &g_wmap_actor_animations[20], 0xC, 0xFF, 0x1, 0x8, 0,
                                                          g_wmap_effect_params))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_19_sequence_8_step_03, g_wmap_land_effect_19_sequence_8_step, g_wmap_land_effect_19_sequence_8_timer,
                       g_wmap_effect_params[5], 0x20, wmap_land_effect_19_sequence_8_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_19_sequence_8_step_04, g_wmap_land_effect_19_sequence_8_step, g_wmap_land_effect_19_sequence_8_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[20], &g_wmap_actor_animations[20], 0xC, 0xFF, 0x1, 0x8, 0,
                                                          g_wmap_effect_params))

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_8_end, g_wmap_land_effect_19_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_9, g_wmap_land_effect_19_sequence_9_steps, 0x8, g_wmap_land_effect_19_sequence_9_step,
                 g_wmap_land_effect_19_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_9_reset, g_wmap_land_effect_19_sequence_9_step, g_wmap_land_effect_19_sequence_9_timer)

/**
 * @brief Update the particles and increase the shared fade value until the timer expires.
 */
WMAP_STEP_UPDATE_AND_RAMP(wmap_land_effect_19_sequence_9_step_02, g_wmap_land_effect_19_sequence_9_step, g_wmap_land_effect_19_sequence_9_timer, D_801B25D8, 8,
                          wmap_update_falling_particles(0x50, 0x8C, 2, -1, 0x14, 4, 0x168, 0x19, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 1))

WMAP_STEP_ARM_TIMER(wmap_land_effect_19_sequence_9_step_03, g_wmap_land_effect_19_sequence_9_step, g_wmap_land_effect_19_sequence_9_timer, 0x18,
                    wmap_land_effect_19_sequence_9_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_19_sequence_9_step_04, g_wmap_land_effect_19_sequence_9_step, g_wmap_land_effect_19_sequence_9_timer,
                          wmap_update_falling_particles(0x50, 0x8C, 2, -1, 0x14, 4, 0x168, 0x19, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 1))

/**
 * @brief Stop spawning particles and keep updating those already active.
 */
WMAP_STEP_STOP_PARTICLE_SPAWNS(wmap_land_effect_19_sequence_9_step_05, g_wmap_land_effect_19_sequence_9_step, g_wmap_land_effect_19_sequence_9_timer,
                               g_wmap_falling_spawn_counts[1], 0x18, wmap_land_effect_19_sequence_9_step_06)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_19_sequence_9_step_06, g_wmap_land_effect_19_sequence_9_step, g_wmap_land_effect_19_sequence_9_timer,
                          wmap_update_falling_particles(0x50, 0x8C, 2, -1, 0x14, 4, 0x168, 0x19, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 1))

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_9_end, g_wmap_land_effect_19_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_10, g_wmap_land_effect_19_sequence_10_steps, 0x8, g_wmap_land_effect_19_sequence_10_step,
                 g_wmap_land_effect_19_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_10_reset, g_wmap_land_effect_19_sequence_10_step, g_wmap_land_effect_19_sequence_10_timer)

/**
 * @brief Update the particles and increase the shared fade value until the timer expires.
 */
WMAP_STEP_UPDATE_AND_RAMP(wmap_land_effect_19_sequence_10_step_02, g_wmap_land_effect_19_sequence_10_step, g_wmap_land_effect_19_sequence_10_timer,
                          g_wmap_aux_effect_fade_a, 8,
                          wmap_update_falling_particles(0x96, 0xD2, 1, -1, -1, -3, 0, 8, -0xB4, 0x190, -0xA0, 0x190, 1, 1, 0x81, 4, 2))

WMAP_STEP_ARM_TIMER(wmap_land_effect_19_sequence_10_step_03, g_wmap_land_effect_19_sequence_10_step, g_wmap_land_effect_19_sequence_10_timer, 0x30,
                    wmap_land_effect_19_sequence_10_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_19_sequence_10_step_04, g_wmap_land_effect_19_sequence_10_step, g_wmap_land_effect_19_sequence_10_timer,
                          wmap_update_falling_particles(0x96, 0xD2, 1, -1, -1, -3, 0, 8, -0xB4, 0x190, -0xA0, 0x190, 1, 1, 0x81, 4, 2))

/**
 * @brief Stop spawning particles and keep updating those already active.
 */
WMAP_STEP_STOP_PARTICLE_SPAWNS(wmap_land_effect_19_sequence_10_step_05, g_wmap_land_effect_19_sequence_10_step, g_wmap_land_effect_19_sequence_10_timer,
                               g_wmap_falling_spawn_counts[2], 0xA0, wmap_land_effect_19_sequence_10_step_06)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_19_sequence_10_step_06, g_wmap_land_effect_19_sequence_10_step, g_wmap_land_effect_19_sequence_10_timer,
                          wmap_update_falling_particles(0x96, 0xD2, 1, -1, -1, -3, 0, 8, -0xB4, 0x190, -0xA0, 0x190, 1, 1, 0x81, 4, 2))

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_10_end, g_wmap_land_effect_19_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_11, g_wmap_land_effect_19_sequence_11_steps, 0x8, g_wmap_land_effect_19_sequence_11_step,
                 g_wmap_land_effect_19_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_11_reset, g_wmap_land_effect_19_sequence_11_step, g_wmap_land_effect_19_sequence_11_timer)

/**
 * @brief Update the particles and increase the shared fade value until the timer expires.
 */
WMAP_STEP_UPDATE_AND_RAMP(wmap_land_effect_19_sequence_11_step_02, g_wmap_land_effect_19_sequence_11_step, g_wmap_land_effect_19_sequence_11_timer, D_801B25D8,
                          8, wmap_update_falling_particles(0xD2, 0xE6, 2, -1, -1, -8, 0x28, 8, -0xB4, 0x168, -0xB4, 0x168, 1, 1, 0x81, 4, 3))

WMAP_STEP_ARM_TIMER(wmap_land_effect_19_sequence_11_step_03, g_wmap_land_effect_19_sequence_11_step, g_wmap_land_effect_19_sequence_11_timer, 0x28,
                    wmap_land_effect_19_sequence_11_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_19_sequence_11_step_04, g_wmap_land_effect_19_sequence_11_step, g_wmap_land_effect_19_sequence_11_timer,
                          wmap_update_falling_particles(0xD2, 0xE6, 2, -1, -1, -8, 0x28, 8, -0xB4, 0x168, -0xB4, 0x168, 1, 1, 0x81, 4, 3))

/**
 * @brief Stop spawning particles and keep updating those already active.
 */
WMAP_STEP_STOP_PARTICLE_SPAWNS(wmap_land_effect_19_sequence_11_step_05, g_wmap_land_effect_19_sequence_11_step, g_wmap_land_effect_19_sequence_11_timer,
                               g_wmap_falling_spawn_counts[3], 0xA0, wmap_land_effect_19_sequence_11_step_06)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_19_sequence_11_step_06, g_wmap_land_effect_19_sequence_11_step, g_wmap_land_effect_19_sequence_11_timer,
                          wmap_update_falling_particles(0xD2, 0xE6, 2, -1, -1, -8, 0x28, 8, -0xB4, 0x168, -0xB4, 0x168, 1, 1, 0x81, 4, 3))

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_11_end, g_wmap_land_effect_19_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_12, g_wmap_land_effect_19_sequence_12_steps, 0x4, g_wmap_land_effect_19_sequence_12_step,
                 g_wmap_land_effect_19_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_12_reset, g_wmap_land_effect_19_sequence_12_step, g_wmap_land_effect_19_sequence_12_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_19_sequence_12_step_01, g_wmap_land_effect_19_sequence_12_step, g_wmap_land_effect_19_sequence_12_timer, 6,
                      g_wmap_animation_bank_4, 0, 0x80, 0x80, 0, 0x82, wmap_land_effect_19_sequence_12_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_19_sequence_12_step_02, g_wmap_land_effect_19_sequence_12_step, g_wmap_land_effect_19_sequence_12_timer,
                              g_wmap_sprite_actors[6], g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x1F, 0x7, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_12_end, g_wmap_land_effect_19_sequence_12_step)
