#include "common.h"
#include <libgte.h>

#include "../internal/wmap_actor_motion.h"
#include "../internal/wmap_cells.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_land_effect_07.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_view_effects.h"

extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_07_sequence_1_timer;
extern s32 g_wmap_land_effect_07_sequence_2_timer;
extern s32 g_wmap_land_effect_07_sequence_3_timer;
extern s32 g_wmap_land_effect_07_sequence_10_timer;
extern s32 g_wmap_particle_shade;
extern s32 g_wmap_land_effect_07_sequence_11_timer;
extern s32 g_wmap_land_effect_07_timer;
extern WmapSequenceStep g_wmap_land_effect_07_steps[];
extern s32 g_wmap_land_effect_07_timeline_timer;
extern WmapSequenceStep g_wmap_land_effect_07_timeline_steps[];
extern WmapSequenceStep g_wmap_land_effect_07_sequence_1_steps[];
extern WmapSequenceStep g_wmap_land_effect_07_sequence_2_steps[];
extern WmapSequenceStep g_wmap_land_effect_07_sequence_3_steps[];
extern s32 g_wmap_land_effect_07_sequence_4_timer;
extern WmapSequenceStep g_wmap_land_effect_07_sequence_4_steps[];
extern s32 g_wmap_land_effect_07_sequence_5_timer;
extern WmapSequenceStep g_wmap_land_effect_07_sequence_5_steps[];
extern s32 g_wmap_land_effect_07_sequence_6_timer;
extern WmapSequenceStep g_wmap_land_effect_07_sequence_6_steps[];
extern s32 g_wmap_land_effect_07_sequence_7_timer;
extern WmapSequenceStep g_wmap_land_effect_07_sequence_7_steps[];
extern s32 g_wmap_land_effect_07_sequence_8_timer;
extern WmapSequenceStep g_wmap_land_effect_07_sequence_8_steps[];
extern s32 g_wmap_land_effect_07_sequence_9_timer;
extern WmapSequenceStep g_wmap_land_effect_07_sequence_9_steps[];
extern WmapSequenceStep g_wmap_land_effect_07_sequence_10_steps[];
extern WmapSequenceStep g_wmap_land_effect_07_sequence_11_steps[];

extern u32 g_wmap_land_effect_07_sequence_1_step;
extern u32 g_wmap_land_effect_07_sequence_2_step;
extern u32 g_wmap_land_effect_07_sequence_3_step;
extern u32 g_wmap_land_effect_07_sequence_10_step;
extern u32 g_wmap_land_effect_07_sequence_11_step;
extern u32 g_wmap_land_effect_07_step;
extern u32 g_wmap_land_effect_07_timeline_step;
extern u32 g_wmap_land_effect_07_sequence_4_step;
extern u32 g_wmap_land_effect_07_sequence_5_step;
extern u32 g_wmap_land_effect_07_sequence_6_step;
extern u32 g_wmap_land_effect_07_sequence_7_step;
extern u32 g_wmap_land_effect_07_sequence_8_step;
extern u32 g_wmap_land_effect_07_sequence_9_step;

extern s32* g_wmap_effect_params;

/** @brief Draw two oscillating effect layers and update their intensity. */
void wmap_land_effect_07_sequence_1_step_02(void)
{
    VECTOR* position;
    SVECTOR* first_rotation;
    SVECTOR* second_rotation;
    s32 frame;
    s32 intensity;
    s32 remaining;
    PushMatrix();
    position = &g_wmap_camera_translation;
    frame = (g_wmap_cosine_wave[g_wmap_effect_wave_phase] + 0x80) >> 5;
    first_rotation = &g_wmap_effect_model_e_rotation;
    wmap_set_model_transform(position, first_rotation);
    wmap_draw_model_default(g_wmap_load_buffer, frame, 4, 0x35, 0x7800, 0, g_wmap_effect_fade_f);
    frame = (g_wmap_cosine_wave[g_wmap_effect_fade_e] + 0x80) >> 5;
    first_rotation->vz = (first_rotation->vz + 0x14);
    second_rotation = &g_wmap_effect_model_f_rotation;
    wmap_set_model_transform(position, second_rotation);
    wmap_draw_model_default(g_wmap_load_buffer, frame, 4, 0x35, 0x7800, 0, g_wmap_effect_fade_f);
    second_rotation->vz = (second_rotation->vz + 0x30);
    PopMatrix();
    g_wmap_effect_wave_phase = (g_wmap_effect_wave_phase + 8) & 0xFF;
    g_wmap_effect_fade_e = (g_wmap_effect_fade_e + 4) & 0xFF;
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_f, 1, 0x80, intensity);
    remaining = g_wmap_land_effect_07_sequence_1_timer - 1;
    g_wmap_land_effect_07_sequence_1_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_07_sequence_1_step++;
    }
}

/** @brief Draw two oscillating effect layers and update their intensity. */
void wmap_land_effect_07_sequence_1_step_04(void)
{
    VECTOR* position;
    SVECTOR* first_rotation;
    SVECTOR* second_rotation;
    s32 frame;
    s32 intensity;
    s32 remaining;
    PushMatrix();
    position = &g_wmap_camera_translation;
    frame = (g_wmap_cosine_wave[g_wmap_effect_wave_phase] + 0x80) >> 5;
    first_rotation = &g_wmap_effect_model_e_rotation;
    wmap_set_model_transform(position, first_rotation);
    wmap_draw_model_default(g_wmap_load_buffer, frame, 4, 0x35, 0x7800, 0, g_wmap_effect_fade_f);
    frame = (g_wmap_cosine_wave[g_wmap_effect_fade_e] + 0x80) >> 5;
    first_rotation->vz = (first_rotation->vz + 0x14);
    second_rotation = &g_wmap_effect_model_f_rotation;
    wmap_set_model_transform(position, second_rotation);
    wmap_draw_model_default(g_wmap_load_buffer, frame, 4, 0x35, 0x7800, 0, g_wmap_effect_fade_f);
    second_rotation->vz = (second_rotation->vz + 0x30);
    PopMatrix();
    g_wmap_effect_wave_phase = (g_wmap_effect_wave_phase + 8) & 0xFF;
    g_wmap_effect_fade_e = (g_wmap_effect_fade_e + 4) & 0xFF;
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_f, 2, intensity);
    remaining = g_wmap_land_effect_07_sequence_1_timer - 1;
    g_wmap_land_effect_07_sequence_1_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_07_sequence_1_step++;
    }
}

/** @brief Move the model away and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_07_sequence_2_step_02, g_wmap_land_effect_07_sequence_2_step, g_wmap_land_effect_07_sequence_2_timer,
                      g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, g_wmap_effect_model_pack_1, 3500, 8)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_07_sequence_3_step_02, g_wmap_land_effect_07_sequence_3_step, g_wmap_land_effect_07_sequence_3_timer,
                      g_wmap_effect_model_b_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, g_wmap_effect_model_pack_2, -3500, 8)

/** @brief Configure effect parameters and reset its animation resource slots. */
void wmap_land_effect_07_sequence_10_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 12;
    g_wmap_effect_wave_phase = 127;
    WMAP_SET_PARTICLE_SETTINGS(4, 4, 16, 0, 2, 1500, 100, 19, 1, 0x938801F4);
    WMAP_RESET_PARTICLE_SLOTS(i, 12, g_wmap_actor_motions[i + g_wmap_particle_first_motion].state, 104, g_wmap_animation_bank_1);
    g_wmap_land_effect_07_sequence_10_timer = 24;
    g_wmap_land_effect_07_sequence_10_step++;
    wmap_land_effect_07_sequence_10_step_02();
}

/** @brief Reduce the effect parameters, draw it, and advance its countdown. */
void wmap_land_effect_07_sequence_11_step_04(void)
{
    s32 value;
    s32 remaining;

    value = g_wmap_particle_shade - 2;
    g_wmap_particle_shade = value;

    if (value < 0)
    {
        g_wmap_particle_shade = 0;
    }

    if (!(g_wmap_frame_count & 3))
    {
        g_wmap_particle_intensity--;
    }

    if (g_wmap_particle_intensity < 0)
    {
        g_wmap_particle_intensity = 0;
    }

    wmap_update_flat_radial_burst(0xCC, 0xD2, 0x13, 0x20, 0x650, 8, 0x60, 0);
    remaining = g_wmap_land_effect_07_sequence_11_timer - 1;
    g_wmap_land_effect_07_sequence_11_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_07_sequence_11_step++;
    }
}

WMAP_STEP_RUNNER(wmap_land_effect_07_run, g_wmap_land_effect_07_steps, 0x6, g_wmap_land_effect_07_step, g_wmap_land_effect_07_timer)

WMAP_STEP_RESET(wmap_land_effect_07_reset, g_wmap_land_effect_07_step, g_wmap_land_effect_07_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_07_step_01, g_wmap_land_effect_07_step, wmap_run_land_focus, wmap_land_effect_07_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_07_wait_idle_02, g_wmap_land_effect_07_step, wmap_land_effect_07_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_07_step_03, g_wmap_land_effect_07_step, wmap_land_effect_07_run_timeline, wmap_land_effect_07_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_07_wait_idle_04, g_wmap_land_effect_07_step, wmap_land_effect_07_end)

WMAP_STEP_ADVANCE(wmap_land_effect_07_end, g_wmap_land_effect_07_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_timeline, g_wmap_land_effect_07_timeline_steps, 0x16, g_wmap_land_effect_07_timeline_step,
                 g_wmap_land_effect_07_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_07_timeline_reset, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer)

/** @brief World-map state entry: load resources, register callbacks, advance. */
void wmap_land_effect_07_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x803030);
    g_wmap_backdrop_target_level = 4;
    wmap_play_sound(0x1D, 0x80);
    wmap_start_sequence(wmap_land_effect_07_run_sequence_2);
    wmap_start_sequence(wmap_land_effect_07_run_sequence_1);
    g_wmap_land_effect_07_timeline_timer = 8;
    g_wmap_land_effect_07_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_07_timeline_wait_02, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_07_timeline_step_03, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer,
                         wmap_land_effect_07_run_sequence_4, 0x8)

WMAP_STEP_WAIT(wmap_land_effect_07_timeline_wait_04, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_07_timeline_step_05, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer,
                         wmap_land_effect_07_run_sequence_6, 0x14)

WMAP_STEP_WAIT(wmap_land_effect_07_timeline_wait_06, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer)

/**
 * @brief Hide the placement overlay, start the sequence, and set the wait timer.
 */
WMAP_STEP_HIDE_AND_START(wmap_land_effect_07_timeline_step_07, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer,
                         g_wmap_placement_overlay_hidden, wmap_land_effect_07_run_sequence_7, 0x14)

WMAP_STEP_WAIT(wmap_land_effect_07_timeline_wait_08, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_07_timeline_step_09, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer,
                         wmap_land_effect_07_run_sequence_8, 0x14)

WMAP_STEP_WAIT(wmap_land_effect_07_timeline_wait_10, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_07_timeline_step_11, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer,
                         wmap_land_effect_07_run_sequence_9, 0x28)

WMAP_STEP_WAIT(wmap_land_effect_07_timeline_wait_12, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_07_timeline_step_13, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer,
                         wmap_land_effect_07_run_sequence_3, 1)

WMAP_STEP_WAIT(wmap_land_effect_07_timeline_wait_14, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_07_timeline_step_15, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer,
                         wmap_land_effect_07_run_sequence_5, 0x12)

WMAP_STEP_WAIT(wmap_land_effect_07_timeline_wait_16, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_07_timeline_step_17, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer,
                         wmap_land_effect_07_run_sequence_11, 0x10)

WMAP_STEP_WAIT(wmap_land_effect_07_timeline_wait_18, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_07_timeline_step_19, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer,
                         wmap_land_effect_07_run_sequence_10, 0x63)

WMAP_STEP_WAIT(wmap_land_effect_07_timeline_wait_20, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_07_timeline_finish, g_wmap_land_effect_07_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y,
                           g_wmap_selected_artifact)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_1, g_wmap_land_effect_07_sequence_1_steps, 0x6, g_wmap_land_effect_07_sequence_1_step,
                 g_wmap_land_effect_07_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_1_reset, g_wmap_land_effect_07_sequence_1_step, g_wmap_land_effect_07_sequence_1_timer)

/** @brief World-map step: reset the sprite counters and advance to the next handler. */
void wmap_land_effect_07_sequence_1_step_01(void)
{
    g_wmap_effect_fade_f = 1;
    g_wmap_effect_wave_phase = 0;
    g_wmap_effect_fade_e = 0;
    g_wmap_effect_model_e_rotation.vx = 0;
    g_wmap_effect_model_e_rotation.vy = 0;
    g_wmap_effect_model_e_rotation.vz = 0;
    g_wmap_effect_model_f_rotation.vx = 0;
    g_wmap_effect_model_f_rotation.vy = 0;
    g_wmap_effect_model_f_rotation.vz = 0;
    g_wmap_land_effect_07_sequence_1_timer = 0x80;
    g_wmap_land_effect_07_sequence_1_step++;
    wmap_land_effect_07_sequence_1_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_07_sequence_1_step_03, g_wmap_land_effect_07_sequence_1_step, g_wmap_land_effect_07_sequence_1_timer, 0x40,
                    wmap_land_effect_07_sequence_1_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_1_end, g_wmap_land_effect_07_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_2, g_wmap_land_effect_07_sequence_2_steps, 0x4, g_wmap_land_effect_07_sequence_2_step,
                 g_wmap_land_effect_07_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_2_reset, g_wmap_land_effect_07_sequence_2_step, g_wmap_land_effect_07_sequence_2_timer)

WMAP_STEP_DROP_START(wmap_land_effect_07_sequence_2_step_01, g_wmap_land_effect_07_sequence_2_step, g_wmap_land_effect_07_sequence_2_timer,
                     g_wmap_effect_model_a_rotation, g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x10,
                     wmap_land_effect_07_sequence_2_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_2_end, g_wmap_land_effect_07_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_3, g_wmap_land_effect_07_sequence_3_steps, 0x4, g_wmap_land_effect_07_sequence_3_step,
                 g_wmap_land_effect_07_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_3_reset, g_wmap_land_effect_07_sequence_3_step, g_wmap_land_effect_07_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_07_sequence_3_step_01, g_wmap_land_effect_07_sequence_3_step, g_wmap_land_effect_07_sequence_3_timer,
                     g_wmap_effect_model_b_rotation, g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x10,
                     wmap_land_effect_07_sequence_3_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_3_end, g_wmap_land_effect_07_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_4, g_wmap_land_effect_07_sequence_4_steps, 0x4, g_wmap_land_effect_07_sequence_4_step,
                 g_wmap_land_effect_07_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_4_reset, g_wmap_land_effect_07_sequence_4_step, g_wmap_land_effect_07_sequence_4_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_07_sequence_4_step_01, g_wmap_land_effect_07_sequence_4_step, g_wmap_land_effect_07_sequence_4_timer, 4,
                      g_wmap_animation_bank_0, 0, 0, 0x80, 8, 0x70, wmap_land_effect_07_sequence_4_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_07_sequence_4_step_02, g_wmap_land_effect_07_sequence_4_step, g_wmap_land_effect_07_sequence_4_timer,
                              g_wmap_sprite_actors[4], g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0x13, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_4_end, g_wmap_land_effect_07_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_5, g_wmap_land_effect_07_sequence_5_steps, 0x4, g_wmap_land_effect_07_sequence_5_step,
                 g_wmap_land_effect_07_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_5_reset, g_wmap_land_effect_07_sequence_5_step, g_wmap_land_effect_07_sequence_5_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_07_sequence_5_step_01, g_wmap_land_effect_07_sequence_5_step, g_wmap_land_effect_07_sequence_5_timer, 5,
                      g_wmap_animation_bank_2, 0, 0, 0x80, 8, 0x88, wmap_land_effect_07_sequence_5_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_07_sequence_5_step_02, g_wmap_land_effect_07_sequence_5_step, g_wmap_land_effect_07_sequence_5_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x17, 0x70, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_5_end, g_wmap_land_effect_07_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_6, g_wmap_land_effect_07_sequence_6_steps, 0x4, g_wmap_land_effect_07_sequence_6_step,
                 g_wmap_land_effect_07_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_6_reset, g_wmap_land_effect_07_sequence_6_step, g_wmap_land_effect_07_sequence_6_timer)

/**
 * @brief Populate a world-map actor pair and schedule its animation step.
 */
void wmap_land_effect_07_sequence_6_step_01(void)
{
    WmapSpriteActor* src = &g_wmap_sprite_actors[6];
    WmapSpriteActor* dst = &g_wmap_sprite_actors[7];
    WmapAnimationSlot* animations = g_wmap_actor_animations;

    animations[6].data = g_wmap_animation_bank_1;
    animations[7].data = g_wmap_animation_bank_1;
    src->scale_index = 0xF;
    src->sequence = 2;
    src->previous_sequence = -1;
    src->shade_step = 8;
    src->target_shade = 0x81;
    src->resource_index = 0;
    src->shade = 1;
    *dst = *src;
    dst->sequence = 3;
    g_wmap_land_effect_07_sequence_6_timer = 0x68;
    g_wmap_land_effect_07_sequence_6_step++;
    wmap_land_effect_07_sequence_6_step_02();
}

/**
 * @brief Draw a world-map actor pair at an offset copy of the cursor position.
 * @note Repacks the cursor coordinate word, nudging each 16-bit half, then
 *       renders the primary actor and its shadow twin.
 */
void wmap_land_effect_07_sequence_6_step_02(void)
{
    s32 pos;
    s32 x_bits;
    s32 y;

    pos = g_wmap_focus_screen_position.packed;
    x_bits = pos - 0xA;
    pos &= 0xFFFF0000;
    x_bits &= 0xFFFF;
    pos |= x_bits;
    y = pos >> 16;
    pos &= 0xFFFF;
    pos |= (y - 0xA) << 16;
    wmap_step_actor_animation(&g_wmap_sprite_actors[6], &g_wmap_actor_animations[6]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[6], pos, 0x13, 1, 0);
    wmap_step_actor_animation(&g_wmap_sprite_actors[7], &g_wmap_actor_animations[7]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[7], pos, 0x13, 1, 0);

    if (--g_wmap_land_effect_07_sequence_6_timer == 0)
    {
        g_wmap_land_effect_07_sequence_6_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_6_end, g_wmap_land_effect_07_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_7, g_wmap_land_effect_07_sequence_7_steps, 0x4, g_wmap_land_effect_07_sequence_7_step,
                 g_wmap_land_effect_07_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_7_reset, g_wmap_land_effect_07_sequence_7_step, g_wmap_land_effect_07_sequence_7_timer)

/**
 * @brief Populate a world-map actor pair and schedule its animation step.
 */
void wmap_land_effect_07_sequence_7_step_01(void)
{
    WmapSpriteActor* src = &g_wmap_sprite_actors[8];
    WmapSpriteActor* dst = &g_wmap_sprite_actors[9];
    WmapAnimationSlot* animations = g_wmap_actor_animations;

    animations[8].data = g_wmap_animation_bank_1;
    animations[9].data = g_wmap_animation_bank_1;
    src->scale_index = 0xF;
    src->sequence = 2;
    src->previous_sequence = -1;
    src->shade_step = 8;
    src->target_shade = 0x81;
    src->resource_index = 0;
    src->shade = 1;
    *dst = *src;
    dst->sequence = 3;
    g_wmap_land_effect_07_sequence_7_timer = 0x54;
    g_wmap_land_effect_07_sequence_7_step++;
    wmap_land_effect_07_sequence_7_step_02();
}

/** @brief Draw a world-map actor pair at an offset copy of the cursor position. */
void wmap_land_effect_07_sequence_7_step_02(void)
{
    s32 pos;
    s32 x_bits;
    s32 y;

    pos = g_wmap_focus_screen_position.packed;
    x_bits = pos + 2;
    pos &= 0xFFFF0000;
    x_bits &= 0xFFFF;
    pos |= x_bits;
    y = pos >> 16;
    pos &= 0xFFFF;
    pos |= (y - 0x1C) << 16;
    wmap_step_actor_animation(&g_wmap_sprite_actors[8], &g_wmap_actor_animations[8]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[8], pos, 0x13, 1, 0);
    wmap_step_actor_animation(&g_wmap_sprite_actors[9], &g_wmap_actor_animations[9]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[9], pos, 0x13, 1, 0);

    if (--g_wmap_land_effect_07_sequence_7_timer == 0)
    {
        g_wmap_land_effect_07_sequence_7_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_7_end, g_wmap_land_effect_07_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_8, g_wmap_land_effect_07_sequence_8_steps, 0x4, g_wmap_land_effect_07_sequence_8_step,
                 g_wmap_land_effect_07_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_8_reset, g_wmap_land_effect_07_sequence_8_step, g_wmap_land_effect_07_sequence_8_timer)

/**
 * @brief Populate a world-map actor pair and schedule its animation step.
 */
void wmap_land_effect_07_sequence_8_step_01(void)
{
    WmapSpriteActor* src = &g_wmap_sprite_actors[10];
    WmapSpriteActor* dst = &g_wmap_sprite_actors[11];
    WmapAnimationSlot* animations = g_wmap_actor_animations;

    animations[10].data = g_wmap_animation_bank_1;
    animations[11].data = g_wmap_animation_bank_1;
    src->scale_index = 0xF;
    src->sequence = 2;
    src->previous_sequence = -1;
    src->shade_step = 8;
    src->target_shade = 0x81;
    src->resource_index = 0;
    src->shade = 1;
    *dst = *src;
    dst->sequence = 3;
    g_wmap_land_effect_07_sequence_8_timer = 0x40;
    g_wmap_land_effect_07_sequence_8_step++;
    wmap_land_effect_07_sequence_8_step_02();
}

/** @brief Draw a world-map actor pair at an offset copy of the cursor position. */
void wmap_land_effect_07_sequence_8_step_02(void)
{
    s32 pos;
    s32 x_bits;
    s32 y;

    pos = g_wmap_focus_screen_position.packed;
    x_bits = pos + 0x14;
    pos &= 0xFFFF0000;
    x_bits &= 0xFFFF;
    pos |= x_bits;
    y = pos >> 16;
    pos &= 0xFFFF;
    pos |= (y - 0xF) << 16;
    wmap_step_actor_animation(&g_wmap_sprite_actors[10], &g_wmap_actor_animations[10]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[10], pos, 0x13, 1, 0);
    wmap_step_actor_animation(&g_wmap_sprite_actors[11], &g_wmap_actor_animations[11]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[11], pos, 0x13, 1, 0);

    if (--g_wmap_land_effect_07_sequence_8_timer == 0)
    {
        g_wmap_land_effect_07_sequence_8_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_8_end, g_wmap_land_effect_07_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_9, g_wmap_land_effect_07_sequence_9_steps, 0x4, g_wmap_land_effect_07_sequence_9_step,
                 g_wmap_land_effect_07_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_9_reset, g_wmap_land_effect_07_sequence_9_step, g_wmap_land_effect_07_sequence_9_timer)

/**
 * @brief Populate a world-map actor pair and schedule its animation step.
 */
void wmap_land_effect_07_sequence_9_step_01(void)
{
    WmapSpriteActor* src = &g_wmap_sprite_actors[12];
    WmapSpriteActor* dst = &g_wmap_sprite_actors[13];
    WmapAnimationSlot* animations = g_wmap_actor_animations;

    animations[12].data = g_wmap_animation_bank_1;
    animations[13].data = g_wmap_animation_bank_1;
    src->scale_index = 0xF;
    src->sequence = 2;
    src->previous_sequence = -1;
    src->shade_step = 8;
    src->target_shade = 0x81;
    src->resource_index = 0;
    src->shade = 1;
    *dst = *src;
    dst->sequence = 3;
    g_wmap_land_effect_07_sequence_9_timer = 0x2C;
    g_wmap_land_effect_07_sequence_9_step++;
    wmap_land_effect_07_sequence_9_step_02();
}

/** @brief Draw a world-map actor pair at an offset copy of the cursor position. */
void wmap_land_effect_07_sequence_9_step_02(void)
{
    s32 pos;
    s32 x_bits;
    s32 y;

    pos = g_wmap_focus_screen_position.packed;
    x_bits = pos + 0x8;
    pos &= 0xFFFF0000;
    x_bits &= 0xFFFF;
    pos |= x_bits;
    y = pos >> 16;
    pos &= 0xFFFF;
    pos |= (y - 0x16) << 16;
    wmap_step_actor_animation(&g_wmap_sprite_actors[12], &g_wmap_actor_animations[12]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[12], pos, 0x13, 1, 0);
    wmap_step_actor_animation(&g_wmap_sprite_actors[13], &g_wmap_actor_animations[13]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[13], pos, 0x13, 1, 0);

    if (--g_wmap_land_effect_07_sequence_9_timer == 0)
    {
        g_wmap_land_effect_07_sequence_9_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_9_end, g_wmap_land_effect_07_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_10, g_wmap_land_effect_07_sequence_10_steps, 0x6, g_wmap_land_effect_07_sequence_10_step,
                 g_wmap_land_effect_07_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_10_reset, g_wmap_land_effect_07_sequence_10_step, g_wmap_land_effect_07_sequence_10_timer)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_07_sequence_10_step_02, g_wmap_land_effect_07_sequence_10_step, g_wmap_land_effect_07_sequence_10_timer,
                          wmap_update_default_particles_from_settings(&g_wmap_sprite_actors[104], &g_wmap_actor_animations[104], 0xC, g_wmap_effect_wave_phase,
                                                                      g_wmap_effect_wave_phase, 0))

WMAP_STEP_ARM_TIMER(wmap_land_effect_07_sequence_10_step_03, g_wmap_land_effect_07_sequence_10_step, g_wmap_land_effect_07_sequence_10_timer, 0x30,
                    wmap_land_effect_07_sequence_10_step_04)

/** @brief Draw the active effect and advance when the countdown expires. */
void wmap_land_effect_07_sequence_10_step_04(void)
{
    s32 remaining_ticks;

    g_wmap_effect_params[5] = -1;

    if (g_wmap_effect_wave_phase != 0)
    {
        wmap_update_default_particles_from_settings(&g_wmap_sprite_actors[104], &g_wmap_actor_animations[104], 0xC, g_wmap_effect_wave_phase,
                                                    g_wmap_effect_wave_phase, 0);
    }

    remaining_ticks = g_wmap_land_effect_07_sequence_10_timer - 1;
    g_wmap_land_effect_07_sequence_10_timer = remaining_ticks;

    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_07_sequence_10_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_10_end, g_wmap_land_effect_07_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_11, g_wmap_land_effect_07_sequence_11_steps, 0x6, g_wmap_land_effect_07_sequence_11_step,
                 g_wmap_land_effect_07_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_11_reset, g_wmap_land_effect_07_sequence_11_step, g_wmap_land_effect_07_sequence_11_timer)

/** @brief Seed six world-map effect slots and advance the sequence step. */
void wmap_land_effect_07_sequence_11_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 0;
    g_wmap_particle_shade = 0x7F;
    WMAP_RESET_PARTICLE_SLOTS(i, 6, g_wmap_actor_motions[i].state, 204, g_wmap_animation_bank_1);
    g_wmap_land_effect_07_sequence_11_timer = 0x20;
    g_wmap_land_effect_07_sequence_11_step++;
    wmap_land_effect_07_sequence_11_step_02();
}

/** @brief Advance the effect on odd frames, draw it, and update the countdown. */
void wmap_land_effect_07_sequence_11_step_02(void)
{
    s32 remaining_ticks;

    if (g_wmap_frame_count & 1)
    {
        g_wmap_particle_intensity++;
    }

    wmap_update_flat_radial_burst(0xCC, 0xD2, 0x13, 0x20, 0x650, 8, 0x60, 0);
    remaining_ticks = g_wmap_land_effect_07_sequence_11_timer - 1;
    g_wmap_land_effect_07_sequence_11_timer = remaining_ticks;

    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_07_sequence_11_step++;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_07_sequence_11_step_03, g_wmap_land_effect_07_sequence_11_step, g_wmap_land_effect_07_sequence_11_timer, 0x40,
                    wmap_land_effect_07_sequence_11_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_11_end, g_wmap_land_effect_07_sequence_11_step)
