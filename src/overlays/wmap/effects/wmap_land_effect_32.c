#include "common.h"
#include <libgte.h>

#include "../internal/wmap_actor_motion.h"
#include "../internal/wmap_cells.h"
#include "../internal/wmap_land_effect_32.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_view_effects.h"

extern s32 g_wmap_land_effect_32_sequence_2_timer;
extern s32 g_wmap_land_effect_32_sequence_3_timer;
extern s32 g_wmap_land_effect_32_sequence_4_timer;
extern s32 g_wmap_land_effect_32_sequence_5_timer;
extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_32_sequence_6_timer;
extern s32 g_wmap_land_effect_32_sequence_7_timer;
extern s32 g_wmap_land_effect_32_sequence_10_timer;
extern s32 rand(void);
extern s32 g_wmap_land_effect_32_timer;
extern WmapSequenceStep g_wmap_land_effect_32_steps[];
extern s32 g_wmap_land_effect_32_timeline_timer;
extern WmapSequenceStep g_wmap_land_effect_32_timeline_steps[];
extern s32 g_wmap_land_effect_32_sequence_1_timer;
extern WmapSequenceStep g_wmap_land_effect_32_sequence_1_steps[];
extern WmapSequenceStep g_wmap_land_effect_32_sequence_2_steps[];
extern WmapSequenceStep g_wmap_land_effect_32_sequence_3_steps[];
extern WmapSequenceStep g_wmap_land_effect_32_sequence_4_steps[];
extern WmapSequenceStep g_wmap_land_effect_32_sequence_5_steps[];
extern WmapSequenceStep g_wmap_land_effect_32_sequence_6_steps[];
extern WmapSequenceStep g_wmap_land_effect_32_sequence_7_steps[];
extern s32 g_wmap_land_effect_32_sequence_8_timer;
extern WmapSequenceStep g_wmap_land_effect_32_sequence_8_steps[];
extern s32 g_wmap_land_effect_32_sequence_9_timer;
extern WmapSequenceStep g_wmap_land_effect_32_sequence_9_steps[];
extern WmapSequenceStep g_wmap_land_effect_32_sequence_10_steps[];
extern u32 g_wmap_land_effect_32_sequence_2_step;

extern u32 g_wmap_land_effect_32_sequence_3_step;

extern u32 g_wmap_land_effect_32_sequence_4_step;

extern u32 g_wmap_land_effect_32_sequence_5_step;
extern u32 g_wmap_land_effect_32_sequence_6_step;
extern u32 g_wmap_land_effect_32_sequence_7_step;
extern u32 g_wmap_land_effect_32_sequence_10_step;
extern u32 g_wmap_land_effect_32_step;
extern u32 g_wmap_land_effect_32_timeline_step;
extern u32 g_wmap_land_effect_32_sequence_1_step;
extern u32 g_wmap_land_effect_32_sequence_8_step;
extern u32 g_wmap_land_effect_32_sequence_9_step;

/** @brief Configure effect parameters and reset its animation resource slots. */
void wmap_land_effect_32_sequence_2_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 24;
    g_wmap_effect_wave_phase = 127;
    WMAP_SET_PARTICLE_SETTINGS(1, 10, 20, 2, 2, 1500, 100, 19, 0, 1000);
    WMAP_RESET_PARTICLE_SLOTS(i, 24, g_wmap_actor_motions[i + g_wmap_particle_first_motion].state, 104, g_wmap_animation_bank_0);
    g_wmap_land_effect_32_sequence_2_timer = 40;
    g_wmap_land_effect_32_sequence_2_step++;
    wmap_land_effect_32_sequence_2_step_02();
}

/** @brief Move the model closer, draw its fade-in, and advance its countdown. */
void wmap_land_effect_32_sequence_3_step_02(void)
{
    VECTOR* position = &g_wmap_effect_model_d_position;
    SVECTOR* rotation;
    s32 v;
    s32 t;
    s32 c;

    v = position->vz - 0x5DC;
    position->vz = v;

    if (v <= 0x9C3F)
    {
        position->vz = 0x9C40;
    }

    PushMatrix();
    rotation = &g_wmap_effect_model_d_rotation;
    wmap_set_model_transform(position, rotation);
    wmap_draw_model_default(g_wmap_effect_model_pack_3, 0, 0xC, 0x35, 0x7800, 1, g_wmap_effect_fade_e);
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_e, 0x10, 0x81, t);
    rotation->vz += 2;
    PopMatrix();
    c = g_wmap_land_effect_32_sequence_3_timer - 1;
    g_wmap_land_effect_32_sequence_3_timer = c;

    if (c == 0)
    {
        g_wmap_land_effect_32_sequence_3_step++;
    }
}

/** @brief Move the model closer, draw its fade-out, and advance its countdown. */
void wmap_land_effect_32_sequence_3_step_04(void)
{
    VECTOR* position = &g_wmap_effect_model_d_position;
    SVECTOR* rotation;
    s32 v;
    s32 t;
    s32 c;

    v = position->vz - 0x5DC;
    position->vz = v;

    if (v <= 0x9C3F)
    {
        position->vz = 0x9C40;
    }

    PushMatrix();
    rotation = &g_wmap_effect_model_d_rotation;
    wmap_set_model_transform(position, rotation);

    if (g_wmap_effect_fade_e != 0)
    {
        wmap_draw_model_default(g_wmap_effect_model_pack_3, 0, 0xC, 0x35, 0x7800, 1, g_wmap_effect_fade_e);
        WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_e, 2, t);
        rotation->vz += 2;
    }

    PopMatrix();
    c = g_wmap_land_effect_32_sequence_3_timer - 1;
    g_wmap_land_effect_32_sequence_3_timer = c;

    if (c == 0)
    {
        g_wmap_land_effect_32_sequence_3_step++;
    }
}

/** @brief Move the model closer, draw its fade-in, and advance its countdown. */
void wmap_land_effect_32_sequence_4_step_02(void)
{
    VECTOR* position = &g_wmap_effect_model_g_position;
    SVECTOR* rotation;
    s32 v;
    s32 t;
    s32 c;

    v = position->vz - 0x5DC;
    position->vz = v;

    if (v <= 0x9C3F)
    {
        position->vz = 0x9C40;
    }

    PushMatrix();
    rotation = &g_wmap_effect_model_g_rotation;
    wmap_set_model_transform(position, rotation);
    wmap_draw_model_default(g_wmap_effect_model_pack_4, 0, 0xC, 0x35, 0x7800, 1, g_wmap_effect_fade_a);
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_a, 0x10, 0x81, t);
    rotation->vz -= 3;
    PopMatrix();
    c = g_wmap_land_effect_32_sequence_4_timer - 1;
    g_wmap_land_effect_32_sequence_4_timer = c;

    if (c == 0)
    {
        g_wmap_land_effect_32_sequence_4_step++;
    }
}

/** @brief Move the model closer, draw its fade-out, and advance its countdown. */
void wmap_land_effect_32_sequence_4_step_04(void)
{
    VECTOR* position = &g_wmap_effect_model_g_position;
    SVECTOR* rotation;
    s32 v;
    s32 t;
    s32 c;

    v = position->vz - 0x5DC;
    position->vz = v;

    if (v <= 0x9C3F)
    {
        position->vz = 0x9C40;
    }

    PushMatrix();
    rotation = &g_wmap_effect_model_g_rotation;
    wmap_set_model_transform(position, rotation);

    if (g_wmap_effect_fade_a != 0)
    {
        wmap_draw_model_default(g_wmap_effect_model_pack_4, 0, 0xC, 0x35, 0x7800, 1, g_wmap_effect_fade_a);
        WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_a, 2, t);
        rotation->vz -= 3;
    }

    PopMatrix();
    c = g_wmap_land_effect_32_sequence_4_timer - 1;
    g_wmap_land_effect_32_sequence_4_timer = c;

    if (c == 0)
    {
        g_wmap_land_effect_32_sequence_4_step++;
    }
}

/** @brief Move the model closer, draw its fade-in, and advance its countdown. */
void wmap_land_effect_32_sequence_5_step_02(void)
{
    VECTOR* position = &g_wmap_effect_model_h_position;
    SVECTOR* rotation;
    s32 v;
    s32 t;
    s32 c;

    v = position->vz - 0x5DC;
    position->vz = v;

    if (v <= 0x9C3F)
    {
        position->vz = 0x9C40;
    }

    PushMatrix();
    rotation = &g_wmap_effect_model_h_rotation;
    wmap_set_model_transform(position, rotation);
    wmap_draw_model_default(g_wmap_effect_model_pack_5, 0, 0xC, 0x35, 0x7800, 1, g_wmap_effect_fade_b);
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_b, 0x10, 0x81, t);
    rotation->vz += 4;
    PopMatrix();
    c = g_wmap_land_effect_32_sequence_5_timer - 1;
    g_wmap_land_effect_32_sequence_5_timer = c;

    if (c == 0)
    {
        g_wmap_land_effect_32_sequence_5_step++;
    }
}

/** @brief Move the model closer, draw its fade-out, and advance its countdown. */
void wmap_land_effect_32_sequence_5_step_04(void)
{
    VECTOR* position = &g_wmap_effect_model_h_position;
    SVECTOR* rotation;
    s32 v;
    s32 t;
    s32 c;

    v = position->vz - 0x5DC;
    position->vz = v;

    if (v <= 0x9C3F)
    {
        position->vz = 0x9C40;
    }

    PushMatrix();
    rotation = &g_wmap_effect_model_h_rotation;
    wmap_set_model_transform(position, rotation);

    if (g_wmap_effect_fade_b != 0)
    {
        wmap_draw_model_default(g_wmap_effect_model_pack_5, 0, 0xC, 0x35, 0x7800, 1, g_wmap_effect_fade_b);
        WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_b, 2, t);
        rotation->vz += 4;
    }

    PopMatrix();
    c = g_wmap_land_effect_32_sequence_5_timer - 1;
    g_wmap_land_effect_32_sequence_5_timer = c;

    if (c == 0)
    {
        g_wmap_land_effect_32_sequence_5_step++;
    }
}

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_32_sequence_6_step_02, g_wmap_land_effect_32_sequence_6_step, g_wmap_land_effect_32_sequence_6_timer,
                      g_wmap_effect_model_c_rotation, g_wmap_effect_model_c_position, g_wmap_effect_fade_c, g_wmap_load_buffer, -3500, 5)

/** @brief Draw and fade the transformed effect, then advance its countdown. */
void wmap_land_effect_32_sequence_7_step_02(void)
{
    MATRIX base;
    MATRIX effect;
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = g_wmap_effect_model_b_position.vz - 0x7D0;
    g_wmap_effect_model_b_position.vz = depth;

    if (depth < 0x1F40)
    {
        g_wmap_effect_model_b_position.vz = 0x1F40;
    }

    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base);
    TransMatrix(&base, &g_wmap_effect_model_b_position);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&g_wmap_effect_model_b_rotation, &effect);
    TransMatrix(&effect, &g_wmap_zero_translation);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);

    if (g_wmap_effect_fade_d != 0)
    {
        wmap_draw_model_default(g_wmap_effect_model_pack_1, 0, 0xC, 0x35, 0x7800, 1, g_wmap_effect_fade_d);
        WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_d, 6, intensity);
    }

    PopMatrix();
    remaining = g_wmap_land_effect_32_sequence_7_timer - 1;
    g_wmap_land_effect_32_sequence_7_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_32_sequence_7_step++;
    }
}

/** @brief Initialize the effect actors and randomized motion angles. */
void wmap_land_effect_32_sequence_10_step_01(void)
{
    s32 i;
    WmapSpriteActor* actor;
    WmapMotion* motion;

    g_wmap_particle_intensity = 40;
    g_wmap_aux_effect_fade_c = 256;
    WMAP_SET_PARTICLE_SETTINGS(48, 10, 60, 2, -1, 8000, 200, 19, 1, 200);
    g_wmap_particle_arc_apex_lifetime = 54;
    g_wmap_particle_arc_peak = 1;
    g_wmap_particle_arc_divisor = 30;
    g_wmap_particle_arc_base = 1;
    g_wmap_particle_fall_range = 3;

    for (i = 0; i < 40; i++)
    {
        actor = &g_wmap_sprite_actors[204 + i];
        motion = &g_wmap_actor_motions_at_200[i];
        g_wmap_actor_animations[i + 204].data = g_wmap_animation_bank_0;
        actor->scale_index = WMAP_SPRITE_SCALE_FULL;
        actor->previous_sequence = -1;
        actor->target_shade = WMAP_SPRITE_SHADE_NEUTRAL;
        actor->shade = WMAP_SPRITE_SHADE_NEUTRAL;
        actor->resource_index = 0;
        actor->shade_step = 0;
        actor->sequence = g_wmap_particle_sequence;
        motion->state = 1;
        motion->angle = rand() & 0xFFF;
        motion->lifetime = 60;
        motion->z = 50;
        motion->velocity = 0;
        motion->radius = g_wmap_particle_radius;
    }

    g_wmap_land_effect_32_sequence_10_timer = 90;
    g_wmap_land_effect_32_sequence_10_step++;
    wmap_land_effect_32_sequence_10_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_32_run, g_wmap_land_effect_32_steps, 0x6, g_wmap_land_effect_32_step, g_wmap_land_effect_32_timer)

WMAP_STEP_RESET(wmap_land_effect_32_reset, g_wmap_land_effect_32_step, g_wmap_land_effect_32_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_32_step_01, g_wmap_land_effect_32_step, wmap_run_land_focus, wmap_land_effect_32_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_32_wait_idle_02, g_wmap_land_effect_32_step, wmap_land_effect_32_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_32_step_03, g_wmap_land_effect_32_step, wmap_land_effect_32_run_timeline, wmap_land_effect_32_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_32_wait_idle_04, g_wmap_land_effect_32_step, wmap_land_effect_32_end)

WMAP_STEP_ADVANCE(wmap_land_effect_32_end, g_wmap_land_effect_32_step)

WMAP_STEP_RUNNER(wmap_land_effect_32_run_timeline, g_wmap_land_effect_32_timeline_steps, 0x10, g_wmap_land_effect_32_timeline_step,
                 g_wmap_land_effect_32_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_32_timeline_reset, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer)

/** @brief Set color and flags, play sound 26, and register three sequence callbacks. */
void wmap_land_effect_32_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x202020);
    wmap_play_sound(0x1A, 0x80);
    wmap_start_sequence(&wmap_land_effect_32_run_sequence_1);
    g_wmap_placement_overlay_hidden = 1;
    wmap_start_sequence(&wmap_land_effect_32_run_sequence_8);
    wmap_start_sequence(&wmap_land_effect_32_run_sequence_7);
    g_wmap_land_effect_32_timeline_timer = 8;
    g_wmap_land_effect_32_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_32_timeline_wait_02, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_32_timeline_step_03, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer,
                         wmap_land_effect_32_run_sequence_3, 0x4)

WMAP_STEP_WAIT(wmap_land_effect_32_timeline_wait_04, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_32_timeline_step_05, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer,
                         wmap_land_effect_32_run_sequence_4, 0x4)

WMAP_STEP_WAIT(wmap_land_effect_32_timeline_wait_06, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_32_timeline_step_07, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer,
                         wmap_land_effect_32_run_sequence_5, 0xF)

WMAP_STEP_WAIT(wmap_land_effect_32_timeline_wait_08, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_32_timeline_step_09, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer,
                         wmap_land_effect_32_run_sequence_2, 0x30)

WMAP_STEP_WAIT(wmap_land_effect_32_timeline_wait_10, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_32_timeline_step_11, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer,
                             wmap_land_effect_32_run_sequence_6, wmap_land_effect_32_run_sequence_9, 0x42)

WMAP_STEP_WAIT(wmap_land_effect_32_timeline_wait_12, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_32_timeline_step_13, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer,
                         wmap_land_effect_32_run_sequence_10, 0x83)

WMAP_STEP_WAIT(wmap_land_effect_32_timeline_wait_14, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_32_timeline_finish, g_wmap_land_effect_32_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y,
                           g_wmap_selected_artifact)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_1, g_wmap_land_effect_32_sequence_1_steps, 0x8, g_wmap_land_effect_32_sequence_1_step,
                               g_wmap_land_effect_32_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_1_reset, g_wmap_land_effect_32_sequence_1_step, g_wmap_land_effect_32_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_32_sequence_1_step_01, g_wmap_land_effect_32_sequence_1_step, g_wmap_land_effect_32_sequence_1_timer, 5,
                      g_wmap_animation_bank_0, 2, 1, 0x81, 0x10, 0x10, wmap_land_effect_32_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_32_sequence_1_step_02, g_wmap_land_effect_32_sequence_1_step, g_wmap_land_effect_32_sequence_1_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x13, 0xB, 0)

WMAP_STEP_ARM_TIMER(wmap_land_effect_32_sequence_1_step_03, g_wmap_land_effect_32_sequence_1_step, g_wmap_land_effect_32_sequence_1_timer, 0x8,
                    wmap_land_effect_32_sequence_1_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_32_sequence_1_step_04, g_wmap_land_effect_32_sequence_1_step, g_wmap_land_effect_32_sequence_1_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x13, 0xB, 0)

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void wmap_land_effect_32_sequence_1_step_05(void)
{
    g_wmap_sprite_actors[5].target_shade = 1;
    g_wmap_land_effect_32_sequence_1_timer = 0x10;
    g_wmap_land_effect_32_sequence_1_step++;
    wmap_land_effect_32_sequence_1_step_06();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_32_sequence_1_step_06, g_wmap_land_effect_32_sequence_1_step, g_wmap_land_effect_32_sequence_1_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x13, 0xB, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_1_end, g_wmap_land_effect_32_sequence_1_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_2, g_wmap_land_effect_32_sequence_2_steps, 0x6, g_wmap_land_effect_32_sequence_2_step,
                               g_wmap_land_effect_32_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_2_reset, g_wmap_land_effect_32_sequence_2_step, g_wmap_land_effect_32_sequence_2_timer)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_32_sequence_2_step_02, g_wmap_land_effect_32_sequence_2_step, g_wmap_land_effect_32_sequence_2_timer,
                          wmap_update_default_particles_from_settings(&g_wmap_sprite_actors[104], &g_wmap_actor_animations[104], 0x18, g_wmap_effect_wave_phase,
                                                                      g_wmap_effect_wave_phase, 0))

WMAP_STEP_ARM_TIMER(wmap_land_effect_32_sequence_2_step_03, g_wmap_land_effect_32_sequence_2_step, g_wmap_land_effect_32_sequence_2_timer, 0x10,
                    wmap_land_effect_32_sequence_2_step_04)

/** @brief Draw and fade the effect, then advance when its countdown expires. */
void wmap_land_effect_32_sequence_2_step_04(void)
{
    s32 value;
    s32 remaining_ticks;

    if (g_wmap_effect_wave_phase != 0)
    {
        wmap_update_default_particles_from_settings(&g_wmap_sprite_actors[104], &g_wmap_actor_animations[104], 0x18, g_wmap_effect_wave_phase,
                                                    g_wmap_effect_wave_phase, 0);
        value = g_wmap_effect_wave_phase - 8;
        g_wmap_effect_wave_phase = value;

        if (value < 0)
        {
            g_wmap_effect_wave_phase = 0;
        }
    }

    remaining_ticks = g_wmap_land_effect_32_sequence_2_timer - 1;
    g_wmap_land_effect_32_sequence_2_timer = remaining_ticks;

    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_32_sequence_2_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_2_end, g_wmap_land_effect_32_sequence_2_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_3, g_wmap_land_effect_32_sequence_3_steps, 0x6, g_wmap_land_effect_32_sequence_3_step,
                               g_wmap_land_effect_32_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_3_reset, g_wmap_land_effect_32_sequence_3_step, g_wmap_land_effect_32_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_32_sequence_3_step_01, g_wmap_land_effect_32_sequence_3_step, g_wmap_land_effect_32_sequence_3_timer,
                     g_wmap_effect_model_d_rotation, g_wmap_zero_rotation, g_wmap_effect_model_d_position, g_wmap_effect_fade_e, 0x1, 0xA410, 0x42,
                     wmap_land_effect_32_sequence_3_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_32_sequence_3_step_03, g_wmap_land_effect_32_sequence_3_step, g_wmap_land_effect_32_sequence_3_timer, 0x40,
                    wmap_land_effect_32_sequence_3_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_3_end, g_wmap_land_effect_32_sequence_3_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_4, g_wmap_land_effect_32_sequence_4_steps, 0x6, g_wmap_land_effect_32_sequence_4_step,
                               g_wmap_land_effect_32_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_4_reset, g_wmap_land_effect_32_sequence_4_step, g_wmap_land_effect_32_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_32_sequence_4_step_01, g_wmap_land_effect_32_sequence_4_step, g_wmap_land_effect_32_sequence_4_timer,
                     g_wmap_effect_model_g_rotation, g_wmap_zero_rotation, g_wmap_effect_model_g_position, g_wmap_effect_fade_a, 0x1, 0xA410, 0x3E,
                     wmap_land_effect_32_sequence_4_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_32_sequence_4_step_03, g_wmap_land_effect_32_sequence_4_step, g_wmap_land_effect_32_sequence_4_timer, 0x40,
                    wmap_land_effect_32_sequence_4_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_4_end, g_wmap_land_effect_32_sequence_4_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_5, g_wmap_land_effect_32_sequence_5_steps, 0x6, g_wmap_land_effect_32_sequence_5_step,
                               g_wmap_land_effect_32_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_5_reset, g_wmap_land_effect_32_sequence_5_step, g_wmap_land_effect_32_sequence_5_timer)

WMAP_STEP_DROP_START(wmap_land_effect_32_sequence_5_step_01, g_wmap_land_effect_32_sequence_5_step, g_wmap_land_effect_32_sequence_5_timer,
                     g_wmap_effect_model_h_rotation, g_wmap_zero_rotation, g_wmap_effect_model_h_position, g_wmap_effect_fade_b, 0x1, 0xA410, 0x3A,
                     wmap_land_effect_32_sequence_5_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_32_sequence_5_step_03, g_wmap_land_effect_32_sequence_5_step, g_wmap_land_effect_32_sequence_5_timer, 0x40,
                    wmap_land_effect_32_sequence_5_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_5_end, g_wmap_land_effect_32_sequence_5_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_6, g_wmap_land_effect_32_sequence_6_steps, 0x4, g_wmap_land_effect_32_sequence_6_step,
                               g_wmap_land_effect_32_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_6_reset, g_wmap_land_effect_32_sequence_6_step, g_wmap_land_effect_32_sequence_6_timer)

WMAP_STEP_DROP_START(wmap_land_effect_32_sequence_6_step_01, g_wmap_land_effect_32_sequence_6_step, g_wmap_land_effect_32_sequence_6_timer,
                     g_wmap_effect_model_c_rotation, g_wmap_zero_rotation, g_wmap_effect_model_c_position, g_wmap_effect_fade_c, 0x80, 0xAFC8, 0x28,
                     wmap_land_effect_32_sequence_6_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_6_end, g_wmap_land_effect_32_sequence_6_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_7, g_wmap_land_effect_32_sequence_7_steps, 0x4, g_wmap_land_effect_32_sequence_7_step,
                               g_wmap_land_effect_32_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_7_reset, g_wmap_land_effect_32_sequence_7_step, g_wmap_land_effect_32_sequence_7_timer)

WMAP_STEP_DROP_START(wmap_land_effect_32_sequence_7_step_01, g_wmap_land_effect_32_sequence_7_step, g_wmap_land_effect_32_sequence_7_timer,
                     g_wmap_effect_model_b_rotation, g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_d, 0x80, 0xAFC8, 0x28,
                     wmap_land_effect_32_sequence_7_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_7_end, g_wmap_land_effect_32_sequence_7_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_8, g_wmap_land_effect_32_sequence_8_steps, 0x4, g_wmap_land_effect_32_sequence_8_step,
                               g_wmap_land_effect_32_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_8_reset, g_wmap_land_effect_32_sequence_8_step, g_wmap_land_effect_32_sequence_8_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_32_sequence_8_step_01, g_wmap_land_effect_32_sequence_8_step, g_wmap_land_effect_32_sequence_8_timer, 6,
                      g_wmap_animation_bank_0, 3, 0x81, 0x81, 0, 0x4D, wmap_land_effect_32_sequence_8_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_32_sequence_8_step_02, g_wmap_land_effect_32_sequence_8_step, g_wmap_land_effect_32_sequence_8_timer,
                              g_wmap_sprite_actors[6], g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x13, 0xB, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_8_end, g_wmap_land_effect_32_sequence_8_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_9, g_wmap_land_effect_32_sequence_9_steps, 0x4, g_wmap_land_effect_32_sequence_9_step,
                               g_wmap_land_effect_32_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_9_reset, g_wmap_land_effect_32_sequence_9_step, g_wmap_land_effect_32_sequence_9_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_32_sequence_9_step_01, g_wmap_land_effect_32_sequence_9_step, g_wmap_land_effect_32_sequence_9_timer, 4,
                      g_wmap_animation_bank_1, 0, 0x80, 0x80, 0, 0xC8, wmap_land_effect_32_sequence_9_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_32_sequence_9_step_02, g_wmap_land_effect_32_sequence_9_step, g_wmap_land_effect_32_sequence_9_timer,
                              g_wmap_sprite_actors[4], g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0x14, 0xB, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_9_end, g_wmap_land_effect_32_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_32_run_sequence_10, g_wmap_land_effect_32_sequence_10_steps, 0x6, g_wmap_land_effect_32_sequence_10_step,
                 g_wmap_land_effect_32_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_10_reset, g_wmap_land_effect_32_sequence_10_step, g_wmap_land_effect_32_sequence_10_timer)

/** @brief Draw the sequence effect and advance when its countdown expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_32_sequence_10_step_02, g_wmap_land_effect_32_sequence_10_step, g_wmap_land_effect_32_sequence_10_timer,
                          wmap_update_particles_from_settings(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 0x28, 0, g_wmap_aux_effect_fade_c, 8,
                                                              1))

WMAP_STEP_ARM_TIMER(wmap_land_effect_32_sequence_10_step_03, g_wmap_land_effect_32_sequence_10_step, g_wmap_land_effect_32_sequence_10_timer, 0x10,
                    wmap_land_effect_32_sequence_10_step_04)

/** @brief Draw and fade the effect, then advance when its countdown expires. */
void wmap_land_effect_32_sequence_10_step_04(void)
{
    s32 value;
    s32 remaining_ticks;

    if (g_wmap_aux_effect_fade_c != 0)
    {
        wmap_update_particles_from_settings(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 0x28, 1, g_wmap_aux_effect_fade_c, 8, 1);
        value = g_wmap_aux_effect_fade_c - 8;
        g_wmap_aux_effect_fade_c = value;

        if (value < 0)
        {
            g_wmap_aux_effect_fade_c = 0;
        }
    }

    remaining_ticks = g_wmap_land_effect_32_sequence_10_timer - 1;
    g_wmap_land_effect_32_sequence_10_timer = remaining_ticks;

    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_32_sequence_10_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_10_end, g_wmap_land_effect_32_sequence_10_step)
