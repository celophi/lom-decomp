#include "common.h"
#include <libgte.h>

#include "../internal/wmap_actor_motion.h"
#include "../internal/wmap_cells.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_land_effect_23.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_model_render.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_view_effects.h"

extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_23_sequence_2_timer;
extern s32 g_wmap_land_effect_23_sequence_3_timer;
extern s32 g_wmap_land_effect_23_sequence_4_timer;
extern s32 g_wmap_land_effect_23_sequence_5_timer;
extern s32 g_wmap_land_effect_23_sequence_6_timer;
extern s32 g_wmap_land_effect_23_sequence_7_timer;
extern s32 g_wmap_land_effect_23_sequence_8_timer;
extern s32 g_wmap_land_effect_23_sequence_10_timer;
extern s32 g_wmap_land_effect_23_sequence_13_timer;
extern int rand(void);
extern s32 g_wmap_land_effect_23_sequence_14_timer;
extern s32 g_wmap_land_effect_23_timer;
extern WmapSequenceStep g_wmap_land_effect_23_steps[];
extern s32 g_wmap_land_effect_23_timeline_timer;
extern WmapSequenceStep g_wmap_land_effect_23_timeline_steps[];
extern s32 g_wmap_land_effect_23_sequence_1_timer;
extern WmapSequenceStep g_wmap_land_effect_23_sequence_1_steps[];
extern WmapSequenceStep g_wmap_land_effect_23_sequence_2_steps[];
extern WmapSequenceStep g_wmap_land_effect_23_sequence_3_steps[];
extern WmapSequenceStep g_wmap_land_effect_23_sequence_4_steps[];
extern WmapSequenceStep g_wmap_land_effect_23_sequence_5_steps[];
extern WmapSequenceStep g_wmap_land_effect_23_sequence_6_steps[];
extern WmapSequenceStep g_wmap_land_effect_23_sequence_7_steps[];
extern WmapSequenceStep g_wmap_land_effect_23_sequence_8_steps[];
extern s32 g_wmap_land_effect_23_sequence_9_timer;
extern WmapSequenceStep g_wmap_land_effect_23_sequence_9_steps[];
extern WmapSequenceStep g_wmap_land_effect_23_sequence_10_steps[];
extern s32 g_wmap_land_effect_23_sequence_11_timer;
extern WmapSequenceStep g_wmap_land_effect_23_sequence_11_steps[];
extern s32 g_wmap_land_effect_23_sequence_12_timer;
extern WmapSequenceStep g_wmap_land_effect_23_sequence_12_steps[];
extern WmapSequenceStep g_wmap_land_effect_23_sequence_13_steps[];
extern WmapSequenceStep g_wmap_land_effect_23_sequence_14_steps[];
extern u32 g_wmap_land_effect_23_sequence_2_step;
extern u32 g_wmap_land_effect_23_sequence_3_step;
extern u32 g_wmap_land_effect_23_sequence_4_step;
extern u32 g_wmap_land_effect_23_sequence_5_step;
extern u32 g_wmap_land_effect_23_sequence_6_step;

extern u32 g_wmap_land_effect_23_sequence_7_step;
extern u32 g_wmap_land_effect_23_sequence_8_step;
extern u32 g_wmap_land_effect_23_sequence_10_step;
extern u32 g_wmap_land_effect_23_sequence_13_step;
extern u32 g_wmap_land_effect_23_sequence_14_step;
extern u32 g_wmap_land_effect_23_step;
extern u32 g_wmap_land_effect_23_timeline_step;
extern u32 g_wmap_land_effect_23_sequence_1_step;
extern u32 g_wmap_land_effect_23_sequence_9_step;
extern u32 g_wmap_land_effect_23_sequence_11_step;
extern u32 g_wmap_land_effect_23_sequence_12_step;

extern s32* g_wmap_effect_params;

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_23_sequence_2_step_02, g_wmap_land_effect_23_sequence_2_step, g_wmap_land_effect_23_sequence_2_timer,
                      g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, g_wmap_load_buffer, -3500, 4)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_23_sequence_3_step_02, g_wmap_land_effect_23_sequence_3_step, g_wmap_land_effect_23_sequence_3_timer,
                      g_wmap_effect_model_b_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, g_wmap_effect_model_pack_1, -3500, 1)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_23_sequence_4_step_02, g_wmap_land_effect_23_sequence_4_step, g_wmap_land_effect_23_sequence_4_timer,
                      g_wmap_effect_model_c_rotation, g_wmap_effect_model_c_position, g_wmap_effect_fade_c, g_wmap_effect_model_pack_1, -3500, 2)

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_23_sequence_5_step_01(void)
{
    s32 i;

    WMAP_SET_PARTICLE_CONFIG(0, 1, 0, 0x20, 0, 4, 0x320, 0xB4, 8, 1, 0x124F8);
    WMAP_RESET_PARTICLE_SLOTS(i, 20, g_wmap_actor_motions[i + 180].state, 180, g_wmap_animation_bank_1);
    g_wmap_land_effect_23_sequence_5_timer = 80;
    g_wmap_land_effect_23_sequence_5_step++;
    wmap_land_effect_23_sequence_5_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_23_sequence_6_step_01(void)
{
    s32 i;

    WMAP_SET_PARTICLE_CONFIG(10, 0, 0, 0x40, 0, 5, 0x258, 0x1E, 8, 2, 0x2710);
    WMAP_RESET_PARTICLE_SLOTS(i, 90, g_wmap_actor_motions[i + 30].state, 30, g_wmap_animation_bank_1);
    g_wmap_land_effect_23_sequence_6_timer = 450;
    g_wmap_land_effect_23_sequence_6_step++;
    wmap_land_effect_23_sequence_6_step_02();
}

/**
 * @brief Draw the first animated element, ramp its intensity up, rotate it, and advance after its timer expires.
 */
void wmap_land_effect_23_sequence_7_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_c_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_4, g_wmap_particle_velocity_min & 3, 0xA, 0x36, 0x7900, 0x1001, g_wmap_aux_effect_fade_c, 0, 0xF, -1);
    WMAP_MODEL_FADE_IN(g_wmap_aux_effect_fade_c, 2, 0x81, value);
    timer = g_wmap_land_effect_23_sequence_7_timer;
    g_wmap_effect_model_c_rotation.vz += 0x14;
    next_timer = timer - 1;
    g_wmap_land_effect_23_sequence_7_timer = next_timer;
    g_wmap_particle_velocity_min++;

    if (next_timer == 0)
    {
        g_wmap_land_effect_23_sequence_7_step++;
    }
}

/**
 * @brief Draw an animated element, ramp its intensity down, rotate it, and advance after its timer expires.
 */
void wmap_land_effect_23_sequence_7_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_c_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_4, g_wmap_particle_velocity_min & 3, 0xA, 0x36, 0x7900, 0x1001, g_wmap_aux_effect_fade_c, 0, 0xF, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_aux_effect_fade_c, 4, value);
    timer = g_wmap_land_effect_23_sequence_7_timer;
    g_wmap_effect_model_c_rotation.vz += 0x14;
    next_timer = timer - 1;
    g_wmap_land_effect_23_sequence_7_timer = next_timer;
    g_wmap_particle_velocity_min++;

    if (next_timer == 0)
    {
        g_wmap_land_effect_23_sequence_7_step++;
    }
}

/**
 * @brief Draw the second animated element, ramp its intensity up, rotate it, and advance after its timer expires.
 */
void wmap_land_effect_23_sequence_8_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_b_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, g_wmap_particle_velocity_range & 3, 0x4, 0x35, 0x7800, 0x1001, g_wmap_effect_fade_b, 0, 0xA, -1);
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_b, 8, 0x81, value);
    timer = g_wmap_land_effect_23_sequence_8_timer;
    g_wmap_effect_model_b_rotation.vz += 0x38;
    next_timer = timer - 1;
    g_wmap_land_effect_23_sequence_8_timer = next_timer;
    g_wmap_particle_velocity_range++;

    if (next_timer == 0)
    {
        g_wmap_land_effect_23_sequence_8_step++;
    }
}

/**
 * @brief Draw the second animated element, ramp its intensity down, rotate it, and advance after its timer expires.
 */
void wmap_land_effect_23_sequence_8_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_b_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, g_wmap_particle_velocity_range & 3, 0x4, 0x35, 0x7800, 0x1001, g_wmap_effect_fade_b, 0, 0xA, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_b, 8, value);
    timer = g_wmap_land_effect_23_sequence_8_timer;
    g_wmap_effect_model_b_rotation.vz += 0x38;
    next_timer = timer - 1;
    g_wmap_land_effect_23_sequence_8_timer = next_timer;
    g_wmap_particle_velocity_range++;

    if (next_timer == 0)
    {
        g_wmap_land_effect_23_sequence_8_step++;
    }
}

/** @brief Configure the effect and reset its forty-eight resource slots. */
void wmap_land_effect_23_sequence_10_step_01(void)
{
    s32 i;

    WMAP_SET_PARTICLE_CONFIG(20, 1, 4, 0x20, 0, 4, 1, 0x82, 8, 0, 0x4650);
    WMAP_RESET_PARTICLE_SLOTS(i, 48, g_wmap_actor_motions[i + 130].state, 130, g_wmap_animation_bank_1);
    g_wmap_land_effect_23_sequence_10_timer = 0xC0;
    g_wmap_land_effect_23_sequence_10_step++;
    wmap_land_effect_23_sequence_10_step_02();
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void wmap_land_effect_23_sequence_13_step_01(void)
{
    s32 i;

    g_wmap_effect_fade_d = 1;
    g_wmap_falling_spawn_counts[1] = 8;

    for (i = 0xB4; i < 0xF0; i++)
    {
        g_wmap_actor_motions[i].state = 0;
        WMAP_INIT_PARTICLE_ACTOR(i, g_wmap_animation_bank_2, 0);
    }

    g_wmap_falling_spawn_timers[1] = 1;
    g_wmap_land_effect_23_sequence_13_timer = 0x10;
    g_wmap_land_effect_23_sequence_13_step++;
    wmap_land_effect_23_sequence_13_step_02();
}

/**
 * @brief World-map step handler: jitter the actor position with a ramping random
 *        amplitude, then countdown-advance the step.
 */
void wmap_land_effect_23_sequence_14_step_02(void)
{
    g_wmap_effect_camera_rotation_offset.vx = rand() * g_wmap_particle_first_motion / 16 >> 15;
    g_wmap_effect_camera_rotation_offset.vy = rand() * g_wmap_particle_first_motion / 16 >> 15;
    g_wmap_effect_camera_translation_offset.vx = rand() * g_wmap_particle_first_motion / 16 >> 15;
    g_wmap_effect_camera_translation_offset.vy = rand() * g_wmap_particle_first_motion / 16 >> 15;

    if (g_wmap_particle_first_motion < 0x80)
    {
        g_wmap_particle_first_motion += 4;
    }

    if (--g_wmap_land_effect_23_sequence_14_timer == 0)
    {
        g_wmap_land_effect_23_sequence_14_step++;
    }
}

/**
 * @brief World-map step handler: jitter the actor with a random amplitude, and when
 *        the amplitude ramp expires, zero the offsets; otherwise countdown-advance.
 */
void wmap_land_effect_23_sequence_14_step_04(void)
{
    g_wmap_effect_camera_rotation_offset.vx = rand() * g_wmap_particle_first_motion / 16 >> 15;
    g_wmap_effect_camera_rotation_offset.vy = rand() * g_wmap_particle_first_motion / 16 >> 15;
    g_wmap_effect_camera_translation_offset.vx = rand() * g_wmap_particle_first_motion / 16 >> 15;
    g_wmap_effect_camera_translation_offset.vy = rand() * g_wmap_particle_first_motion / 16 >> 15;

    if (--g_wmap_particle_first_motion < 0)
    {
        g_wmap_effect_camera_translation_offset.vy = 0;
        g_wmap_effect_camera_translation_offset.vx = 0;
        g_wmap_effect_camera_rotation_offset.vy = 0;
        g_wmap_effect_camera_rotation_offset.vx = 0;
        g_wmap_land_effect_23_sequence_14_step++;
    }
    else if (--g_wmap_land_effect_23_sequence_14_timer == 0)
    {
        g_wmap_land_effect_23_sequence_14_step++;
    }
}

WMAP_STEP_RUNNER(wmap_land_effect_23_run, g_wmap_land_effect_23_steps, 0x4, g_wmap_land_effect_23_step, g_wmap_land_effect_23_timer)

WMAP_STEP_RESET(wmap_land_effect_23_reset, g_wmap_land_effect_23_step, g_wmap_land_effect_23_timer)

/** @brief World-map step: arm a timed callback, flag it active, then tick the sub-counter. */
void wmap_land_effect_23_step_01(void)
{
    g_wmap_forced_animated_land_id = 0x10;
    g_wmap_focus_screen_position.point.x = WMAP_FOCUS_DEFAULT_X;
    g_wmap_focus_screen_position.point.y = WMAP_FOCUS_DEFAULT_Y;
    wmap_start_sequence(wmap_land_effect_23_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_23_step++;
    wmap_land_effect_23_wait_idle();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_23_wait_idle, g_wmap_land_effect_23_step, wmap_land_effect_23_step_03)

/** @brief Start the world-map exit and advance the sequence. */
WMAP_STEP_BEGIN_EXIT(wmap_land_effect_23_step_03, g_wmap_land_effect_23_step, WMAP_EXIT_FADE_OVERLAY)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_timeline, g_wmap_land_effect_23_timeline_steps, 0x24, g_wmap_land_effect_23_timeline_step,
                 g_wmap_land_effect_23_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_23_timeline_reset, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/** @brief Set effect flags and color, play sound 41, and begin a 24-tick delay. */
void wmap_land_effect_23_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 8;
    g_wmap_placement_overlay_hidden = 1;
    wmap_play_sound(0x29, 0x80);
    g_wmap_land_effect_23_timeline_timer = 0x18;
    g_wmap_land_effect_23_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_02, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_23_timeline_step_03, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer,
                         wmap_land_effect_23_run_sequence_14, 0x18)

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_04, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_23_timeline_step_05, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer,
                         wmap_land_effect_23_run_sequence_5, 0x65)

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_06, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/**
 * @brief Start three sequences and set the wait timer.
 */
WMAP_STEP_START_THREE_AND_WAIT(wmap_land_effect_23_timeline_step_07, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer,
                               wmap_land_effect_23_run_sequence_10, wmap_land_effect_23_run_sequence_6, wmap_land_effect_23_run_sequence_2, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_08, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_23_timeline_step_09, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer,
                         wmap_land_effect_23_run_sequence_1, 1)

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_10, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/** @brief World-map step handler: seed timers and advance the counter. */
void wmap_land_effect_23_timeline_step_11(void)
{
    g_wmap_land_display_limit = 0;
    g_wmap_forced_animated_land_id = -1;
    g_wmap_land_effect_23_timeline_timer = 0x2D;
    g_wmap_land_effect_23_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_12, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_23_timeline_step_13, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer,
                         wmap_land_effect_23_run_sequence_7, 0x1C)

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_14, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_23_timeline_step_15, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer,
                         wmap_land_effect_23_run_sequence_3, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_16, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/** @brief Set world-map flags and color, then begin a 48-tick delay. */
void wmap_land_effect_23_timeline_step_17(void)
{
    g_wmap_transition_mesh_hidden = 1;
    g_wmap_backdrop_target_level = 1;
    wmap_start_map_tint(0x201010);
    g_wmap_land_effect_23_timeline_timer = 0x30;
    g_wmap_land_effect_23_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_18, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_23_timeline_step_19, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer,
                         wmap_land_effect_23_run_sequence_11, 0x50)

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_20, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/** @brief World-map step handler: register a callback, kick a job, advance the step. */
void wmap_land_effect_23_timeline_step_21(void)
{
    wmap_start_sequence(wmap_land_effect_23_run_sequence_13);
    wmap_start_map_tint(0x302050);
    g_wmap_backdrop_target_level = 3;
    g_wmap_land_effect_23_timeline_timer = 8;
    g_wmap_land_effect_23_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_22, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/** @brief World-map step: set fade colour then advance to the next handler. */
void wmap_land_effect_23_timeline_step_23(void)
{
    wmap_start_map_tint(0x252035);
    g_wmap_land_effect_23_timeline_timer = 0x38;
    g_wmap_land_effect_23_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_24, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_23_timeline_step_25, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer,
                         wmap_land_effect_23_run_sequence_12, 0x22)

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_26, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_23_timeline_step_27, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer,
                             wmap_land_effect_23_run_sequence_9, wmap_land_effect_23_run_sequence_8, 0x48)

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_28, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_23_timeline_step_29, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer,
                         wmap_land_effect_23_run_sequence_4, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_30, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/** @brief World-map step: kick a sub-request, set the next state, and arm the timer. */
void wmap_land_effect_23_timeline_step_31(void)
{
    g_wmap_transition_mesh_hidden = 0;
    wmap_start_map_tint(0x602050);
    g_wmap_backdrop_target_level = 7;
    g_wmap_land_effect_23_timeline_timer = 0x75;
    g_wmap_land_effect_23_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_32, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/** @brief Set the selected record value and advance to a 50-tick delay. */
void wmap_land_effect_23_timeline_step_33(void)
{
    g_wmap_land_effect_23_timeline_timer = 50;
    g_wmap_cells[g_wmap_focus_cell_x][g_wmap_focus_cell_y].land_id = WMAP_LAND_FULL_SIZE | 0x17;
    g_wmap_land_effect_23_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_34, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/**
 * @brief Clear the blocking flag and finish the timeline.
 */
WMAP_STEP_FINISH_BLOCKING(wmap_land_effect_23_timeline_finish, g_wmap_land_effect_23_timeline_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_1, g_wmap_land_effect_23_sequence_1_steps, 0x4, g_wmap_land_effect_23_sequence_1_step,
                 g_wmap_land_effect_23_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_1_reset, g_wmap_land_effect_23_sequence_1_step, g_wmap_land_effect_23_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_23_sequence_1_step_01, g_wmap_land_effect_23_sequence_1_step, g_wmap_land_effect_23_sequence_1_timer, 5,
                      g_wmap_animation_bank_0, 0, 0x81, 0x81, 8, 0x206, wmap_land_effect_23_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_23_sequence_1_step_02, g_wmap_land_effect_23_sequence_1_step, g_wmap_land_effect_23_sequence_1_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x17, 0x8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_1_end, g_wmap_land_effect_23_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_2, g_wmap_land_effect_23_sequence_2_steps, 0x4, g_wmap_land_effect_23_sequence_2_step,
                 g_wmap_land_effect_23_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_2_reset, g_wmap_land_effect_23_sequence_2_step, g_wmap_land_effect_23_sequence_2_timer)

WMAP_STEP_DROP_START(wmap_land_effect_23_sequence_2_step_01, g_wmap_land_effect_23_sequence_2_step, g_wmap_land_effect_23_sequence_2_timer,
                     g_wmap_effect_model_a_rotation, g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x20,
                     wmap_land_effect_23_sequence_2_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_2_end, g_wmap_land_effect_23_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_3, g_wmap_land_effect_23_sequence_3_steps, 0x4, g_wmap_land_effect_23_sequence_3_step,
                 g_wmap_land_effect_23_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_3_reset, g_wmap_land_effect_23_sequence_3_step, g_wmap_land_effect_23_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_23_sequence_3_step_01, g_wmap_land_effect_23_sequence_3_step, g_wmap_land_effect_23_sequence_3_timer,
                     g_wmap_effect_model_b_rotation, g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x80,
                     wmap_land_effect_23_sequence_3_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_3_end, g_wmap_land_effect_23_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_4, g_wmap_land_effect_23_sequence_4_steps, 0x4, g_wmap_land_effect_23_sequence_4_step,
                 g_wmap_land_effect_23_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_4_reset, g_wmap_land_effect_23_sequence_4_step, g_wmap_land_effect_23_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_23_sequence_4_step_01, g_wmap_land_effect_23_sequence_4_step, g_wmap_land_effect_23_sequence_4_timer,
                     g_wmap_effect_model_c_rotation, g_wmap_zero_rotation, g_wmap_effect_model_c_position, g_wmap_effect_fade_c, 0x80, 0xAFC8, 0x40,
                     wmap_land_effect_23_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_4_end, g_wmap_land_effect_23_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_5, g_wmap_land_effect_23_sequence_5_steps, 0x6, g_wmap_land_effect_23_sequence_5_step,
                 g_wmap_land_effect_23_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_5_reset, g_wmap_land_effect_23_sequence_5_step, g_wmap_land_effect_23_sequence_5_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_23_sequence_5_step_02, g_wmap_land_effect_23_sequence_5_step, g_wmap_land_effect_23_sequence_5_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[180], &g_wmap_actor_animations[180], 0x14, 0xFF, 0x1, 0x8, 0,
                                                          g_wmap_effect_params))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_23_sequence_5_step_03, g_wmap_land_effect_23_sequence_5_step, g_wmap_land_effect_23_sequence_5_timer,
                       g_wmap_effect_params[5], 0x20, wmap_land_effect_23_sequence_5_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_23_sequence_5_step_04, g_wmap_land_effect_23_sequence_5_step, g_wmap_land_effect_23_sequence_5_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[180], &g_wmap_actor_animations[180], 0x14, 0xFF, 0x1, 0x8, 0,
                                                          g_wmap_effect_params))

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_5_end, g_wmap_land_effect_23_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_6, g_wmap_land_effect_23_sequence_6_steps, 0x6, g_wmap_land_effect_23_sequence_6_step,
                 g_wmap_land_effect_23_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_6_reset, g_wmap_land_effect_23_sequence_6_step, g_wmap_land_effect_23_sequence_6_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_23_sequence_6_step_02, g_wmap_land_effect_23_sequence_6_step, g_wmap_land_effect_23_sequence_6_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[30], &g_wmap_actor_animations[30], 0x5A, 0xFF, 0x1, 0x4, 0,
                                                          &g_wmap_effect_params[10]))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_23_sequence_6_step_03, g_wmap_land_effect_23_sequence_6_step, g_wmap_land_effect_23_sequence_6_timer,
                       g_wmap_effect_params[15], 0x40, wmap_land_effect_23_sequence_6_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_23_sequence_6_step_04, g_wmap_land_effect_23_sequence_6_step, g_wmap_land_effect_23_sequence_6_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[30], &g_wmap_actor_animations[30], 0x5A, 0xFF, 0x1, 0x4, 0,
                                                          &g_wmap_effect_params[10]))

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_6_end, g_wmap_land_effect_23_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_7, g_wmap_land_effect_23_sequence_7_steps, 0x6, g_wmap_land_effect_23_sequence_7_step,
                 g_wmap_land_effect_23_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_7_reset, g_wmap_land_effect_23_sequence_7_step, g_wmap_land_effect_23_sequence_7_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_effect_23_sequence_7_step_01, g_wmap_land_effect_23_sequence_7_step, g_wmap_land_effect_23_sequence_7_timer,
                      g_wmap_effect_model_c_rotation, g_wmap_zero_rotation, g_wmap_aux_effect_fade_c, 1, g_wmap_particle_velocity_min, 0x12C,
                      wmap_land_effect_23_sequence_7_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_23_sequence_7_step_03, g_wmap_land_effect_23_sequence_7_step, g_wmap_land_effect_23_sequence_7_timer, 0x20,
                    wmap_land_effect_23_sequence_7_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_7_end, g_wmap_land_effect_23_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_8, g_wmap_land_effect_23_sequence_8_steps, 0x6, g_wmap_land_effect_23_sequence_8_step,
                 g_wmap_land_effect_23_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_8_reset, g_wmap_land_effect_23_sequence_8_step, g_wmap_land_effect_23_sequence_8_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_effect_23_sequence_8_step_01, g_wmap_land_effect_23_sequence_8_step, g_wmap_land_effect_23_sequence_8_timer,
                      g_wmap_effect_model_b_rotation, g_wmap_zero_rotation, g_wmap_effect_fade_b, 1, g_wmap_particle_velocity_range, 0x3C,
                      wmap_land_effect_23_sequence_8_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_23_sequence_8_step_03, g_wmap_land_effect_23_sequence_8_step, g_wmap_land_effect_23_sequence_8_timer, 0x10,
                    wmap_land_effect_23_sequence_8_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_8_end, g_wmap_land_effect_23_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_9, g_wmap_land_effect_23_sequence_9_steps, 0x6, g_wmap_land_effect_23_sequence_9_step,
                 g_wmap_land_effect_23_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_9_reset, g_wmap_land_effect_23_sequence_9_step, g_wmap_land_effect_23_sequence_9_timer)

/** @brief Initialize the actor and its screen coordinates, then advance the sequence. */
void wmap_land_effect_23_sequence_9_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    g_wmap_actor_animations[6].data = g_wmap_animation_bank_4;
    g_wmap_particle_lifetime_range = 0x280;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 4;
    actor->target_shade = 0x81;
    actor->shade = 1;
    actor->resource_index = 0;
    actor->sequence = 0;
    g_wmap_land_effect_23_sequence_9_timer = 0x9C;
    g_wmap_effect_sprite_position_c.packed = g_wmap_focus_screen_position.packed;
    g_wmap_land_effect_23_sequence_9_step++;
    wmap_land_effect_23_sequence_9_step_02();
}

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position downward and advance when the frame counter expires.
 */
void wmap_land_effect_23_sequence_9_step_02(void)
{
    wmap_step_actor_animation(&g_wmap_sprite_actors[6], &g_wmap_actor_animations[6]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[6], g_wmap_effect_sprite_position_c.packed, 0x17, 0x2, 0);
    g_wmap_effect_sprite_position_c.point.y = g_wmap_particle_lifetime_range / 0x10;

    if (g_wmap_particle_lifetime_range < 0x640)
    {
        g_wmap_particle_lifetime_range += 0x10;
    }

    if (--g_wmap_land_effect_23_sequence_9_timer == 0)
    {
        g_wmap_land_effect_23_sequence_9_step++;
    }
}

/**
 * @brief Start the sprite fade and run its first update.
 */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_23_sequence_9_step_03, g_wmap_land_effect_23_sequence_9_step, g_wmap_land_effect_23_sequence_9_timer,
                     g_wmap_sprite_actors[6], 2, 0x40, wmap_land_effect_23_sequence_9_step_04)

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position and advance when the frame counter expires.
 */
void wmap_land_effect_23_sequence_9_step_04(void)
{
    wmap_step_actor_animation(&g_wmap_sprite_actors[6], &g_wmap_actor_animations[6]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[6], g_wmap_effect_sprite_position_c.packed, 0x17, 0x2, 0);
    g_wmap_effect_sprite_position_c.point.y = g_wmap_particle_lifetime_range / 0x10;

    if (g_wmap_particle_lifetime_range < 0x640)
    {
        g_wmap_particle_lifetime_range += 0x10;
    }

    if (--g_wmap_land_effect_23_sequence_9_timer == 0)
    {
        g_wmap_land_effect_23_sequence_9_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_9_end, g_wmap_land_effect_23_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_10, g_wmap_land_effect_23_sequence_10_steps, 0x6, g_wmap_land_effect_23_sequence_10_step,
                 g_wmap_land_effect_23_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_10_reset, g_wmap_land_effect_23_sequence_10_step, g_wmap_land_effect_23_sequence_10_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_23_sequence_10_step_02, g_wmap_land_effect_23_sequence_10_step, g_wmap_land_effect_23_sequence_10_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[130], &g_wmap_actor_animations[130], 0x30, 0xFF, 0x1, 0x8, 0,
                                                          &g_wmap_effect_params[20]))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_23_sequence_10_step_03, g_wmap_land_effect_23_sequence_10_step, g_wmap_land_effect_23_sequence_10_timer,
                       g_wmap_effect_params[25], 0x20, wmap_land_effect_23_sequence_10_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_23_sequence_10_step_04, g_wmap_land_effect_23_sequence_10_step, g_wmap_land_effect_23_sequence_10_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[130], &g_wmap_actor_animations[130], 0x30, 0xFF, 0x1, 0x8, 0,
                                                          &g_wmap_effect_params[20]))

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_10_end, g_wmap_land_effect_23_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_11, g_wmap_land_effect_23_sequence_11_steps, 0x6, g_wmap_land_effect_23_sequence_11_step,
                 g_wmap_land_effect_23_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_11_reset, g_wmap_land_effect_23_sequence_11_step, g_wmap_land_effect_23_sequence_11_timer)

/** @brief Initialize the actor, save its screen position, and begin a 142-tick sequence step. */
void wmap_land_effect_23_sequence_11_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[7];

    g_wmap_actor_animations[7].data = g_wmap_animation_bank_3;
    g_wmap_particle_spawn_interval = 0x780;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 1;
    actor->target_shade = 0x7F;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->shade = 0;
    g_wmap_land_effect_23_sequence_11_timer = 0x8E;
    g_wmap_effect_sprite_position_a.packed = g_wmap_focus_screen_position.packed;
    g_wmap_land_effect_23_sequence_11_step++;
    wmap_land_effect_23_sequence_11_step_02();
}

/** @brief Draw a world-map actor, then wind down a scrolling scalar. */
void wmap_land_effect_23_sequence_11_step_02(void)
{
    wmap_step_actor_animation(&g_wmap_sprite_actors[7], &g_wmap_actor_animations[7]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[7], g_wmap_effect_sprite_position_a.packed, 0x8, 0x2, 0);
    g_wmap_effect_sprite_position_a.point.y = g_wmap_particle_spawn_interval / 16;

    if (g_wmap_particle_spawn_interval >= 0x321)
    {
        g_wmap_particle_spawn_interval -= 0x10;
    }

    if (--g_wmap_land_effect_23_sequence_11_timer == 0)
    {
        g_wmap_land_effect_23_sequence_11_step++;
    }
}

/**
 * @brief Start the sprite fade and run its first update.
 */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_23_sequence_11_step_03, g_wmap_land_effect_23_sequence_11_step, g_wmap_land_effect_23_sequence_11_timer,
                     g_wmap_sprite_actors[7], 4, 0x20, wmap_land_effect_23_sequence_11_step_04)

/** @brief Draw a world-map actor, then wind down a scrolling scalar. */
void wmap_land_effect_23_sequence_11_step_04(void)
{
    wmap_step_actor_animation(&g_wmap_sprite_actors[7], &g_wmap_actor_animations[7]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[7], g_wmap_effect_sprite_position_a.packed, 0x8, 0x2, 0);
    g_wmap_effect_sprite_position_a.point.y = g_wmap_particle_spawn_interval / 16;

    if (g_wmap_particle_spawn_interval >= 0x321)
    {
        g_wmap_particle_spawn_interval -= 0x10;
    }

    if (--g_wmap_land_effect_23_sequence_11_timer == 0)
    {
        g_wmap_land_effect_23_sequence_11_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_11_end, g_wmap_land_effect_23_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_12, g_wmap_land_effect_23_sequence_12_steps, 0x6, g_wmap_land_effect_23_sequence_12_step,
                 g_wmap_land_effect_23_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_12_reset, g_wmap_land_effect_23_sequence_12_step, g_wmap_land_effect_23_sequence_12_timer)

/** @brief Initialize the actor and begin a 48-tick sequence step. */
void wmap_land_effect_23_sequence_12_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[8];

    g_wmap_actor_animations[8].data = g_wmap_animation_bank_3;
    g_wmap_particle_radial_step = 0x320;
    actor->scale_index = 0xF;
    actor->resource_index = 0;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    actor->shade_step = 4;
    actor->target_shade = 0x81;
    actor->shade = 1;
    g_wmap_land_effect_23_sequence_12_timer = 0x30;
    g_wmap_effect_sprite_position_b.packed = g_wmap_focus_screen_position.packed;
    g_wmap_land_effect_23_sequence_12_step++;
    wmap_land_effect_23_sequence_12_step_02();
}

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position and advance when the frame counter expires.
 */
void wmap_land_effect_23_sequence_12_step_02(void)
{
    wmap_step_actor_animation(&g_wmap_sprite_actors[8], &g_wmap_actor_animations[8]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[8], g_wmap_effect_sprite_position_b.packed, 0x8, 0x2, 0);
    g_wmap_effect_sprite_position_b.point.y = g_wmap_particle_radial_step / 0x10;

    if (g_wmap_particle_radial_step < 0xA00)
    {
        g_wmap_particle_radial_step += 0x8;
    }

    if (--g_wmap_land_effect_23_sequence_12_timer == 0)
    {
        g_wmap_land_effect_23_sequence_12_step++;
    }
}

/**
 * @brief Start the sprite fade and run its first update.
 */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_23_sequence_12_step_03, g_wmap_land_effect_23_sequence_12_step, g_wmap_land_effect_23_sequence_12_timer,
                     g_wmap_sprite_actors[8], 4, 0x20, wmap_land_effect_23_sequence_12_step_04)

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position and advance when the frame counter expires.
 */
void wmap_land_effect_23_sequence_12_step_04(void)
{
    wmap_step_actor_animation(&g_wmap_sprite_actors[8], &g_wmap_actor_animations[8]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[8], g_wmap_effect_sprite_position_b.packed, 0x8, 0x2, 0);
    g_wmap_effect_sprite_position_b.point.y = g_wmap_particle_radial_step / 0x10;

    if (g_wmap_particle_radial_step < 0xA00)
    {
        g_wmap_particle_radial_step += 0x8;
    }

    if (--g_wmap_land_effect_23_sequence_12_timer == 0)
    {
        g_wmap_land_effect_23_sequence_12_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_12_end, g_wmap_land_effect_23_sequence_12_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_13, g_wmap_land_effect_23_sequence_13_steps, 0x8, g_wmap_land_effect_23_sequence_13_step,
                 g_wmap_land_effect_23_sequence_13_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_13_reset, g_wmap_land_effect_23_sequence_13_step, g_wmap_land_effect_23_sequence_13_timer)

/**
 * @brief Update the particles and increase the shared fade value until the timer expires.
 */
WMAP_STEP_UPDATE_AND_RAMP(wmap_land_effect_23_sequence_13_step_02, g_wmap_land_effect_23_sequence_13_step, g_wmap_land_effect_23_sequence_13_timer,
                          g_wmap_effect_fade_d, 8, wmap_update_falling_particles(0xB4, 0xF0, 1, -1, -1, -4, 0, 0xB, -0xFA, 0xFA, -0xDC, 0x8C, 1, 0x81, 1, 8, 1))

WMAP_STEP_ARM_TIMER(wmap_land_effect_23_sequence_13_step_03, g_wmap_land_effect_23_sequence_13_step, g_wmap_land_effect_23_sequence_13_timer, 0x28,
                    wmap_land_effect_23_sequence_13_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_23_sequence_13_step_04, g_wmap_land_effect_23_sequence_13_step, g_wmap_land_effect_23_sequence_13_timer,
                          wmap_update_falling_particles(0xB4, 0xF0, 1, -1, -1, -4, 0, 0xB, -0xFA, 0xFA, -0xDC, 0x8C, 1, 0x81, 1, 8, 1))

/**
 * @brief Stop spawning particles and keep updating those already active.
 */
WMAP_STEP_STOP_PARTICLE_SPAWNS(wmap_land_effect_23_sequence_13_step_05, g_wmap_land_effect_23_sequence_13_step, g_wmap_land_effect_23_sequence_13_timer,
                               g_wmap_falling_spawn_counts[1], 0x40, wmap_land_effect_23_sequence_13_step_06)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_23_sequence_13_step_06, g_wmap_land_effect_23_sequence_13_step, g_wmap_land_effect_23_sequence_13_timer,
                          wmap_update_falling_particles(0xB4, 0xF0, 1, -1, -1, -4, 0, 0xB, -0xFA, 0xFA, -0xDC, 0x8C, 1, 0x81, 1, 8, 1))

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_13_end, g_wmap_land_effect_23_sequence_13_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_14, g_wmap_land_effect_23_sequence_14_steps, 0x6, g_wmap_land_effect_23_sequence_14_step,
                 g_wmap_land_effect_23_sequence_14_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_14_reset, g_wmap_land_effect_23_sequence_14_step, g_wmap_land_effect_23_sequence_14_timer)

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_23_sequence_14_step_01(void)
{
    g_wmap_particle_first_motion = 0;
    g_wmap_land_effect_23_sequence_14_timer = 0x8C;
    g_wmap_land_effect_23_sequence_14_step++;
    wmap_land_effect_23_sequence_14_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_23_sequence_14_step_03, g_wmap_land_effect_23_sequence_14_step, g_wmap_land_effect_23_sequence_14_timer, 0x40,
                    wmap_land_effect_23_sequence_14_step_04)

/**
 * @brief Clear the world-map translation and rotation offsets, then advance the transition counter.
 */
void wmap_land_effect_23_sequence_14_step_05(void)
{
    g_wmap_effect_camera_translation_offset.vy = 0;
    g_wmap_effect_camera_translation_offset.vx = 0;
    g_wmap_effect_camera_rotation_offset.vy = 0;
    g_wmap_effect_camera_rotation_offset.vx = 0;
    g_wmap_land_effect_23_sequence_14_step++;
}
