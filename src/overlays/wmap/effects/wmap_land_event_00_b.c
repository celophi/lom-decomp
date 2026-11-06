#include "common.h"
#include <libgte.h>

#include "../internal/wmap_actor_motion.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_effect_resources.h"
#include "../internal/wmap_frame_render.h"
#include "../internal/wmap_land_effect_10.h"
#include "../internal/wmap_land_event_00_b.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_model_render.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_view_effects.h"
#include "common/saved_game.h"
#include "main/cdrom.h"

extern u32 g_wmap_land_event_00_b_sequence_1_step;
extern WmapSequenceStep g_wmap_land_event_00_b_sequence_1_steps[];
extern WmapSequenceStep g_wmap_land_event_00_b_timeline_steps[];
extern WmapSequenceStep g_wmap_land_event_00_b_sequence_2_steps[];
extern WmapSequenceStep g_wmap_land_event_00_b_sequence_3_steps[];
extern WmapSequenceStep g_wmap_land_event_00_b_sequence_4_steps[];
extern WmapSequenceStep g_wmap_land_event_00_b_sequence_5_steps[];
extern WmapSequenceStep g_wmap_land_event_00_b_sequence_6_steps[];
extern WmapSequenceStep g_wmap_land_event_00_b_sequence_7_steps[];
extern WmapSequenceStep g_wmap_land_event_00_b_sequence_8_steps[];
extern WmapSequenceStep g_wmap_land_event_00_b_sequence_9_steps[];
extern WmapSequenceStep g_wmap_land_event_00_b_sequence_10_steps[];
extern WmapSequenceStep g_wmap_land_event_00_b_sequence_11_steps[];
extern WmapSequenceStep g_wmap_land_event_00_b_sequence_12_steps[];
extern s32 g_wmap_land_event_00_b_sequence_1_timer;
extern u32 g_wmap_land_event_00_b_timeline_step;
extern s32 g_wmap_land_event_00_b_timeline_timer;
extern u32 g_wmap_land_event_00_b_sequence_2_step;
extern s32 g_wmap_land_event_00_b_sequence_2_timer;
extern u32 g_wmap_land_event_00_b_sequence_3_step;
extern s32 g_wmap_land_event_00_b_sequence_3_timer;
extern u32 g_wmap_land_event_00_b_sequence_4_step;
extern s32 g_wmap_land_event_00_b_sequence_4_timer;
extern u32 g_wmap_land_event_00_b_sequence_5_step;
extern s32 g_wmap_land_event_00_b_sequence_5_timer;
extern u32 g_wmap_land_event_00_b_sequence_6_step;
extern s32 g_wmap_land_event_00_b_sequence_6_timer;
extern u32 g_wmap_land_event_00_b_sequence_7_step;
extern s32 g_wmap_land_event_00_b_sequence_7_timer;
extern u32 g_wmap_land_event_00_b_sequence_8_step;
extern s32 g_wmap_land_event_00_b_sequence_8_timer;
extern u32 g_wmap_land_event_00_b_sequence_9_step;
extern s32 g_wmap_land_event_00_b_sequence_9_timer;
extern u32 g_wmap_land_event_00_b_sequence_10_step;
extern s32 g_wmap_land_event_00_b_sequence_10_timer;
extern u32 g_wmap_land_event_00_b_sequence_11_step;
extern s32 g_wmap_land_event_00_b_sequence_11_timer;
extern u32 g_wmap_land_event_00_b_sequence_12_step;
extern s32 g_wmap_land_event_00_b_sequence_12_timer;

/** @brief Queue world-map effect resources and initialize their map position. */
void wmap_land_event_00_b_sequence_1_step_01(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_2000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x3000;
    g_wmap_effect_model_pack_3 = g_wmap_effect_model_pack_2 + 0x3000;
    g_wmap_effect_model_pack_4 = g_wmap_effect_model_pack_3 + 0x3000;
    g_wmap_effect_model_pack_5 = g_wmap_effect_model_pack_4 + 0x3000;
    g_wmap_effect_model_pack_6 = g_wmap_effect_model_pack_5 + 0x3000;
    g_wmap_effect_model_pack_7 = g_wmap_effect_model_pack_6 + 0x3000;
    wmap_load_tim_file(0x1220);
    wmap_load_tim_file(0x1221);
    wmap_load_tim_file(0x1222);
    cdrom_queue_read(0x1223, g_wmap_animation_bank_0);
    cdrom_queue_read(0x1224, g_wmap_effect_model_buffer_2000 - 0x2000);
    cdrom_queue_read(0x1225, g_wmap_effect_model_pack_1);
    cdrom_queue_read(0x1226, g_wmap_effect_model_pack_2);
    cdrom_queue_read(0x1227, g_wmap_effect_model_pack_3);
    cdrom_queue_read(0x1228, g_wmap_effect_model_pack_4);
    cdrom_queue_read(0x1229, g_wmap_effect_model_pack_7);
    cdrom_queue_read(0x122A, g_wmap_effect_model_pack_5);
    cdrom_queue_read(0x122B, g_wmap_effect_model_pack_6);
    wmap_find_land_cell(FIELD_LAND_HOME, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    WMAP_START_SCROLL_TO_CELL(g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y);
    g_wmap_land_event_00_b_sequence_1_step++;
    wmap_land_event_00_b_sequence_1_step_02();
}

/**
 * @brief Update and draw the model while fading its color in.
 */
void wmap_land_event_00_b_sequence_4_step_02(void)
{
    s32 value;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_e_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_1, (g_wmap_particle_velocity_min / 0x10) & 3, 5, 0x37, 0x7800, 1, g_wmap_effect_wave_phase, -0x5A, -0xA, -1);
    g_wmap_particle_velocity_min += 4;
    WMAP_MODEL_FADE_IN(g_wmap_effect_wave_phase, 2, 0x81, value);

    if (wmap_tick_model_motion(&g_wmap_effect_model_e_rotation, 0, &g_wmap_land_event_00_b_sequence_4_timer) == 0)
    {
        g_wmap_land_event_00_b_sequence_4_step++;
    }
}

/**
 * @brief Update and draw the model while fading its color out.
 */
void wmap_land_event_00_b_sequence_4_step_04(void)
{
    s32 value;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_e_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_1, (g_wmap_particle_velocity_min / 0x10) & 3, 5, 0x37, 0x7800, 1, g_wmap_effect_wave_phase, -0x5A, -0xA, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_wave_phase, 4, value);
    g_wmap_particle_velocity_min += 4;

    if (wmap_tick_model_motion(&g_wmap_effect_model_e_rotation, 0, &g_wmap_land_event_00_b_sequence_4_timer) == 0)
    {
        g_wmap_land_event_00_b_sequence_4_step++;
    }
}

/**
 * @brief Update and draw the model while fading its color in.
 */
void wmap_land_event_00_b_sequence_5_step_02(void)
{
    s32 value;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_f_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_2, (g_wmap_particle_velocity_range / 0x10) & 3, 5, 0x37, 0x7800, 1, g_wmap_effect_fade_e, 0x5F, -0x14, -1);
    g_wmap_particle_velocity_range += 4;
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_e, 4, 0x71, value);

    if (wmap_tick_model_motion(&g_wmap_effect_model_f_rotation, 0, &g_wmap_land_event_00_b_sequence_5_timer) == 0)
    {
        g_wmap_land_event_00_b_sequence_5_step++;
    }
}

/**
 * @brief Update and draw the model while fading its color out.
 */
void wmap_land_event_00_b_sequence_5_step_04(void)
{
    s32 value;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_f_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_2, (g_wmap_particle_velocity_range / 0x10) & 3, 5, 0x37, 0x7800, 1, g_wmap_effect_fade_e, 0x5F, -0x14, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_e, 4, value);
    g_wmap_particle_velocity_range += 4;

    if (wmap_tick_model_motion(&g_wmap_effect_model_f_rotation, 0, &g_wmap_land_event_00_b_sequence_5_timer) == 0)
    {
        g_wmap_land_event_00_b_sequence_5_step++;
    }
}

/**
 * @brief Update and draw the model while fading its color in.
 */
void wmap_land_event_00_b_sequence_6_step_02(void)
{
    s32 value;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_a_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, (g_wmap_particle_lifetime_min / 0x10) & 3, 5, 0x37, 0x7800, 1, g_wmap_effect_fade_a, 0x14, 0xA, -1);
    g_wmap_particle_lifetime_min += 4;
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_a, 2, 0x81, value);

    if (wmap_tick_model_motion(&g_wmap_effect_model_a_rotation, 0, &g_wmap_land_event_00_b_sequence_6_timer) == 0)
    {
        g_wmap_land_event_00_b_sequence_6_step++;
    }
}

/**
 * @brief Update and draw the model while fading its color out.
 */
void wmap_land_event_00_b_sequence_6_step_04(void)
{
    s32 value;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_a_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, (g_wmap_particle_lifetime_min / 0x10) & 3, 5, 0x37, 0x7800, 1, g_wmap_effect_fade_a, 0x14, 0xA, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_a, 4, value);
    g_wmap_particle_lifetime_min += 4;

    if (wmap_tick_model_motion(&g_wmap_effect_model_a_rotation, 0, &g_wmap_land_event_00_b_sequence_6_timer) == 0)
    {
        g_wmap_land_event_00_b_sequence_6_step++;
    }
}

/**
 * @brief Update and draw the model while fading its color in.
 */
void wmap_land_event_00_b_sequence_7_step_02(void)
{
    s32 value;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_b_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_4, (g_wmap_particle_lifetime_range / 0x10) & 3, 0xA, 0x37, 0x7800, 1, g_wmap_effect_fade_b, 0, 0x14, -1);
    g_wmap_particle_lifetime_range += 4;
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_b, 8, 0x81, value);

    if (wmap_tick_model_motion(&g_wmap_effect_model_b_rotation, 0, &g_wmap_land_event_00_b_sequence_7_timer) == 0)
    {
        g_wmap_land_event_00_b_sequence_7_step++;
    }
}

/**
 * @brief Update and draw the model while fading its color out.
 */
void wmap_land_event_00_b_sequence_7_step_04(void)
{
    s32 value;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_b_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_4, (g_wmap_particle_lifetime_range / 0x10) & 3, 0xA, 0x37, 0x7800, 1, g_wmap_effect_fade_b, 0, 0x14, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_b, 2, value);
    g_wmap_particle_lifetime_range += 4;

    if (wmap_tick_model_motion(&g_wmap_effect_model_b_rotation, 0, &g_wmap_land_event_00_b_sequence_7_timer) == 0)
    {
        g_wmap_land_event_00_b_sequence_7_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it in.
 */
void wmap_land_event_00_b_sequence_8_step_02(void)
{
    s32 value;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_c_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_5, (g_wmap_particle_spawn_interval / 0x10) & 3, 0xA, 0x35, 0x7800, 1, g_wmap_effect_fade_c, 0, -0xA, -1);
    g_wmap_particle_spawn_interval += 0x10;
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_c, 8, 0x81, value);

    if (wmap_tick_model_motion(&g_wmap_effect_model_c_rotation, 32, &g_wmap_land_event_00_b_sequence_8_timer) == 0)
    {
        g_wmap_land_event_00_b_sequence_8_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it out.
 */
void wmap_land_event_00_b_sequence_8_step_04(void)
{
    s32 value;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_c_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_5, (g_wmap_particle_spawn_interval / 0x10) & 3, 0xA, 0x35, 0x7800, 1, g_wmap_effect_fade_c, 0, -0xA, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_c, 2, value);
    g_wmap_particle_spawn_interval += 0x10;

    if (wmap_tick_model_motion(&g_wmap_effect_model_c_rotation, 32, &g_wmap_land_event_00_b_sequence_8_timer) == 0)
    {
        g_wmap_land_event_00_b_sequence_8_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it in.
 */
void wmap_land_event_00_b_sequence_9_step_02(void)
{
    s32 value;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_d_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_6, (g_wmap_particle_radial_step / 0x10) & 3, 0xA, 0x35, 0x7800, 1, g_wmap_effect_fade_d, 0, 0, -1);
    g_wmap_particle_radial_step += 8;
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_d, 2, 0x81, value);

    if (wmap_tick_model_motion(&g_wmap_effect_model_d_rotation, 4, &g_wmap_land_event_00_b_sequence_9_timer) == 0)
    {
        g_wmap_land_event_00_b_sequence_9_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it out.
 */
void wmap_land_event_00_b_sequence_9_step_04(void)
{
    s32 value;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_d_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_6, (g_wmap_particle_radial_step / 0x10) & 3, 0xA, 0x35, 0x7800, 1, g_wmap_effect_fade_d, 0, 0, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_d, 4, value);
    g_wmap_particle_radial_step += 8;

    if (wmap_tick_model_motion(&g_wmap_effect_model_d_rotation, 4, &g_wmap_land_event_00_b_sequence_9_timer) == 0)
    {
        g_wmap_land_event_00_b_sequence_9_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it in.
 */
void wmap_land_event_00_b_sequence_10_step_02(void)
{
    s32 value;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_g_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_7, (g_wmap_particle_first_motion / 0x10) & 3, 0xA, 0x35, 0x7800, 1, g_wmap_aux_effect_fade_c, 0, -0xA, -1);
    g_wmap_particle_first_motion += 8;
    WMAP_MODEL_FADE_IN(g_wmap_aux_effect_fade_c, 8, 0x81, value);

    if (wmap_tick_model_motion(&g_wmap_effect_model_g_rotation, 4, &g_wmap_land_event_00_b_sequence_10_timer) == 0)
    {
        g_wmap_land_event_00_b_sequence_10_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it out.
 */
void wmap_land_event_00_b_sequence_10_step_04(void)
{
    s32 value;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_g_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_7, (g_wmap_particle_first_motion / 0x10) & 3, 0xA, 0x35, 0x7800, 1, g_wmap_aux_effect_fade_c, 0, -0xA, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_aux_effect_fade_c, 2, value);
    g_wmap_particle_first_motion += 8;

    if (wmap_tick_model_motion(&g_wmap_effect_model_g_rotation, 4, &g_wmap_land_event_00_b_sequence_10_timer) == 0)
    {
        g_wmap_land_event_00_b_sequence_10_step++;
    }
}

/**
 * @brief Move the model along Z, draw it, and fade it until the timer expires.
 */
WMAP_STEP_DROP_UPDATE_WITH_DRAW(wmap_land_event_00_b_sequence_11_step_02, g_wmap_land_event_00_b_sequence_11_step, g_wmap_land_event_00_b_sequence_11_timer,
                                g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, -0xDAC, 8,
                                wmap_draw_model(g_wmap_load_buffer, 0, 4, 0x35, 0x7800, 1, g_wmap_effect_fade_a, 0, 0, -1))

/**
 * @brief World-map step handler: seed a 100-entry table, init an actor, advance.
 */
void wmap_land_event_00_b_sequence_12_step_01(void)
{
    WmapMotion* motion;
    WmapAnimationSlot* animation;
    s32* params;
    void* resource;
    s32 index;
    s32 setting;
    s32 current;

    index = 0x14;
    resource = g_wmap_animation_bank_2;
    animation = &g_wmap_actor_animations[index];
    motion = &g_wmap_actor_motions[index];

    for (; index < 0x78; index++)
    {
        motion->state = 0;
        animation->data = resource;
        animation++;
        motion++;
    }

    g_wmap_land_event_00_b_sequence_12_timer = 0xB4;
    params = g_wmap_effect_params;
    params[31] = setting = 5;
    params[35] = setting;
    params[30] = 1;
    params[31] = setting;
    current = g_wmap_land_event_00_b_sequence_12_step;
    params[32] = -0x3E8;
    params[33] = 0x1770;
    params[34] = 0x1388;
    params[36] = -0x64;
    params[38] = -0x12C;
    params[37] = 0;
    params[40] = -1;
    params[41] = 0;
    g_wmap_land_event_00_b_sequence_12_step = current + 1;
    wmap_land_event_00_b_sequence_12_step_02();
}

WMAP_STEP_RUNNER(wmap_land_event_00_b_run_sequence_1, g_wmap_land_event_00_b_sequence_1_steps, 6, g_wmap_land_event_00_b_sequence_1_step,
                 g_wmap_land_event_00_b_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_event_00_b_sequence_1_step_00, g_wmap_land_event_00_b_sequence_1_step, g_wmap_land_event_00_b_sequence_1_timer)

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(wmap_land_event_00_b_sequence_1_step_02, g_wmap_land_event_00_b_sequence_1_step, wmap_land_event_00_b_sequence_1_step_03)

/** @brief Wait for the CD queue, move the focus, cover the screen with the overlay fade and start the timeline. */
void wmap_land_event_00_b_sequence_1_step_03(void)
{
    cdrom_wait_queue_empty();
    g_wmap_focus_screen_position.point.x = WMAP_EVENT_FOCUS_X;
    g_wmap_focus_screen_position.point.y = WMAP_EVENT_FOCUS_Y;
    g_wmap_overlay_fade_depth = 4;
    g_wmap_overlay_fade_level = 0xFF;
    g_wmap_overlay_fade_target = 0xFF;
    wmap_start_sequence(wmap_land_event_00_b_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_event_00_b_sequence_1_step++;
    wmap_land_event_00_b_sequence_1_step_04();
}

WMAP_STEP_WAIT_IDLE(wmap_land_event_00_b_sequence_1_step_04, g_wmap_land_event_00_b_sequence_1_step, wmap_land_event_00_b_sequence_1_step_05)

/** @brief Start the world-map exit and advance the sequence. */
WMAP_STEP_BEGIN_EXIT(wmap_land_event_00_b_sequence_1_step_05, g_wmap_land_event_00_b_sequence_1_step, WMAP_EXIT_DIRECT)

WMAP_STEP_RUNNER(wmap_land_event_00_b_run_timeline, g_wmap_land_event_00_b_timeline_steps, 0x16, g_wmap_land_event_00_b_timeline_step,
                 g_wmap_land_event_00_b_timeline_timer)

WMAP_STEP_RESET(wmap_land_event_00_b_timeline_step_00, g_wmap_land_event_00_b_timeline_step, g_wmap_land_event_00_b_timeline_timer)

/** @brief Take over the map, play the event sound, start sequences 10, 8 and 12, and tint the map. */
void wmap_land_event_00_b_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_play_sound(0x38, 0x80);
    g_wmap_overlay_fade_target = 0x7F;
    wmap_start_sequence(wmap_land_event_00_b_run_sequence_10);
    wmap_start_sequence(wmap_land_event_00_b_run_sequence_8);
    wmap_start_map_tint(0x904060);
    g_wmap_backdrop_target_level = 0xE;
    wmap_start_sequence(wmap_land_event_00_b_run_sequence_12);
    g_wmap_land_event_00_b_timeline_timer = 0x5A;
    g_wmap_land_event_00_b_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_event_00_b_timeline_step_02, g_wmap_land_event_00_b_timeline_step, g_wmap_land_event_00_b_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_event_00_b_timeline_step_03, g_wmap_land_event_00_b_timeline_step, g_wmap_land_event_00_b_timeline_timer,
                         wmap_land_event_00_b_run_sequence_2, 4)

WMAP_STEP_WAIT(wmap_land_event_00_b_timeline_step_04, g_wmap_land_event_00_b_timeline_step, g_wmap_land_event_00_b_timeline_timer)

/** @brief Start sequence 2 and set the backdrop level to 9. */
void wmap_land_event_00_b_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_event_00_b_run_sequence_2);
    g_wmap_backdrop_target_level = 9;
    g_wmap_land_event_00_b_timeline_timer = 0x77;
    g_wmap_land_event_00_b_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_event_00_b_timeline_step_06, g_wmap_land_event_00_b_timeline_step, g_wmap_land_event_00_b_timeline_timer)

/** @brief Fade the overlay almost fully out and start sequence 9. */
void wmap_land_event_00_b_timeline_step_07(void)
{
    g_wmap_overlay_fade_target = 1;
    wmap_start_sequence(wmap_land_event_00_b_run_sequence_9);
    g_wmap_land_event_00_b_timeline_timer = 0x14;
    g_wmap_land_event_00_b_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_event_00_b_timeline_step_08, g_wmap_land_event_00_b_timeline_step, g_wmap_land_event_00_b_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_event_00_b_timeline_step_09, g_wmap_land_event_00_b_timeline_step, g_wmap_land_event_00_b_timeline_timer,
                         wmap_land_event_00_b_run_sequence_3, 0x24)

WMAP_STEP_WAIT(wmap_land_event_00_b_timeline_step_10, g_wmap_land_event_00_b_timeline_step, g_wmap_land_event_00_b_timeline_timer)

/** @brief Raise the backdrop level, tint the map, darken the overlay slightly and start sequence 6. */
void wmap_land_event_00_b_timeline_step_11(void)
{
    g_wmap_backdrop_target_level = 0x11;
    wmap_start_map_tint(0x755085);
    g_wmap_overlay_fade_target = 0x23;
    wmap_start_sequence(wmap_land_event_00_b_run_sequence_6);
    g_wmap_land_event_00_b_timeline_timer = 0x14;
    g_wmap_land_event_00_b_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_event_00_b_timeline_step_12, g_wmap_land_event_00_b_timeline_step, g_wmap_land_event_00_b_timeline_timer)

/** @brief Set the backdrop level to 14 and wait four frames. */
void wmap_land_event_00_b_timeline_step_13(void)
{
    g_wmap_backdrop_target_level = 0xE;
    g_wmap_land_event_00_b_timeline_timer = 4;
    g_wmap_land_event_00_b_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_event_00_b_timeline_step_14, g_wmap_land_event_00_b_timeline_step, g_wmap_land_event_00_b_timeline_timer)

/** @brief Tint the map, set the backdrop level to 10 and start sequence 4. */
void wmap_land_event_00_b_timeline_step_15(void)
{
    wmap_start_map_tint(0x654080);
    g_wmap_backdrop_target_level = 0xA;
    wmap_start_sequence(wmap_land_event_00_b_run_sequence_4);
    g_wmap_land_event_00_b_timeline_timer = 0x1E;
    g_wmap_land_event_00_b_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_event_00_b_timeline_step_16, g_wmap_land_event_00_b_timeline_step, g_wmap_land_event_00_b_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_event_00_b_timeline_step_17, g_wmap_land_event_00_b_timeline_step, g_wmap_land_event_00_b_timeline_timer,
                         wmap_land_event_00_b_run_sequence_5, 0x64)

WMAP_STEP_WAIT(wmap_land_event_00_b_timeline_step_18, g_wmap_land_event_00_b_timeline_step, g_wmap_land_event_00_b_timeline_timer)

/** @brief Fade the overlay back to full cover. */
void wmap_land_event_00_b_timeline_step_19(void)
{
    g_wmap_overlay_fade_target = 0xFF;
    g_wmap_land_event_00_b_timeline_timer = 0x70;
    g_wmap_land_event_00_b_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_event_00_b_timeline_step_20, g_wmap_land_event_00_b_timeline_step, g_wmap_land_event_00_b_timeline_timer)

/**
 * @brief Clear the blocking flag and finish the timeline.
 */
WMAP_STEP_FINISH_BLOCKING(wmap_land_event_00_b_timeline_finish, g_wmap_land_event_00_b_timeline_step)

WMAP_STEP_RUNNER(wmap_land_event_00_b_run_sequence_2, g_wmap_land_event_00_b_sequence_2_steps, 4, g_wmap_land_event_00_b_sequence_2_step,
                 g_wmap_land_event_00_b_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_event_00_b_sequence_2_step_00, g_wmap_land_event_00_b_sequence_2_step, g_wmap_land_event_00_b_sequence_2_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_event_00_b_sequence_2_step_01, g_wmap_land_event_00_b_sequence_2_step, g_wmap_land_event_00_b_sequence_2_timer, 4,
                      g_wmap_animation_bank_0, 0, 0x81, 1, 0x10, 0x10, wmap_land_event_00_b_sequence_2_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_event_00_b_sequence_2_step_02, g_wmap_land_event_00_b_sequence_2_step, g_wmap_land_event_00_b_sequence_2_timer,
                              g_wmap_sprite_actors[4], g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0xF, 2, 0)

WMAP_STEP_ADVANCE(wmap_land_event_00_b_sequence_2_step_03, g_wmap_land_event_00_b_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_event_00_b_run_sequence_3, g_wmap_land_event_00_b_sequence_3_steps, 4, g_wmap_land_event_00_b_sequence_3_step,
                 g_wmap_land_event_00_b_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_event_00_b_sequence_3_step_00, g_wmap_land_event_00_b_sequence_3_step, g_wmap_land_event_00_b_sequence_3_timer)

/** @brief Set up sprite actor 5 with animation bank 0 and fade it in slowly, then run step 2. */
WMAP_STEP_START_ACTOR(wmap_land_event_00_b_sequence_3_step_01, g_wmap_land_event_00_b_sequence_3_step, g_wmap_land_event_00_b_sequence_3_timer, 5,
                      g_wmap_animation_bank_0, 1, 1, 0x81, 2, 0x118, wmap_land_event_00_b_sequence_3_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_event_00_b_sequence_3_step_02, g_wmap_land_event_00_b_sequence_3_step, g_wmap_land_event_00_b_sequence_3_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0xF, 5, 0)

WMAP_STEP_ADVANCE(wmap_land_event_00_b_sequence_3_step_03, g_wmap_land_event_00_b_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_event_00_b_run_sequence_4, g_wmap_land_event_00_b_sequence_4_steps, 6, g_wmap_land_event_00_b_sequence_4_step,
                 g_wmap_land_event_00_b_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_event_00_b_sequence_4_step_00, g_wmap_land_event_00_b_sequence_4_step, g_wmap_land_event_00_b_sequence_4_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_event_00_b_sequence_4_step_01, g_wmap_land_event_00_b_sequence_4_step, g_wmap_land_event_00_b_sequence_4_timer,
                      g_wmap_effect_model_e_rotation, g_wmap_zero_rotation, g_wmap_effect_wave_phase, 1, g_wmap_particle_velocity_min, 0xA9,
                      wmap_land_event_00_b_sequence_4_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_event_00_b_sequence_4_step_03, g_wmap_land_event_00_b_sequence_4_step, g_wmap_land_event_00_b_sequence_4_timer, 0x20,
                    wmap_land_event_00_b_sequence_4_step_04)

WMAP_STEP_ADVANCE(wmap_land_event_00_b_sequence_4_step_05, g_wmap_land_event_00_b_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_event_00_b_run_sequence_5, g_wmap_land_event_00_b_sequence_5_steps, 6, g_wmap_land_event_00_b_sequence_5_step,
                 g_wmap_land_event_00_b_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_event_00_b_sequence_5_step_00, g_wmap_land_event_00_b_sequence_5_step, g_wmap_land_event_00_b_sequence_5_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_event_00_b_sequence_5_step_01, g_wmap_land_event_00_b_sequence_5_step, g_wmap_land_event_00_b_sequence_5_timer,
                      g_wmap_effect_model_f_rotation, g_wmap_zero_rotation, g_wmap_effect_fade_e, 1, g_wmap_particle_velocity_range, 0xA8,
                      wmap_land_event_00_b_sequence_5_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_event_00_b_sequence_5_step_03, g_wmap_land_event_00_b_sequence_5_step, g_wmap_land_event_00_b_sequence_5_timer, 0x20,
                    wmap_land_event_00_b_sequence_5_step_04)

WMAP_STEP_ADVANCE(wmap_land_event_00_b_sequence_5_step_05, g_wmap_land_event_00_b_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_event_00_b_run_sequence_6, g_wmap_land_event_00_b_sequence_6_steps, 6, g_wmap_land_event_00_b_sequence_6_step,
                 g_wmap_land_event_00_b_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_event_00_b_sequence_6_step_00, g_wmap_land_event_00_b_sequence_6_step, g_wmap_land_event_00_b_sequence_6_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_event_00_b_sequence_6_step_01, g_wmap_land_event_00_b_sequence_6_step, g_wmap_land_event_00_b_sequence_6_timer,
                      g_wmap_effect_model_a_rotation, g_wmap_zero_rotation, g_wmap_effect_fade_a, 1, g_wmap_particle_lifetime_min, 0xBE,
                      wmap_land_event_00_b_sequence_6_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_event_00_b_sequence_6_step_03, g_wmap_land_event_00_b_sequence_6_step, g_wmap_land_event_00_b_sequence_6_timer, 0x20,
                    wmap_land_event_00_b_sequence_6_step_04)

WMAP_STEP_ADVANCE(wmap_land_event_00_b_sequence_6_step_05, g_wmap_land_event_00_b_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_event_00_b_run_sequence_7, g_wmap_land_event_00_b_sequence_7_steps, 6, g_wmap_land_event_00_b_sequence_7_step,
                 g_wmap_land_event_00_b_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_event_00_b_sequence_7_step_00, g_wmap_land_event_00_b_sequence_7_step, g_wmap_land_event_00_b_sequence_7_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_event_00_b_sequence_7_step_01, g_wmap_land_event_00_b_sequence_7_step, g_wmap_land_event_00_b_sequence_7_timer,
                      g_wmap_effect_model_b_rotation, g_wmap_zero_rotation, g_wmap_effect_fade_b, 1, g_wmap_particle_lifetime_range, 0x1E,
                      wmap_land_event_00_b_sequence_7_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_event_00_b_sequence_7_step_03, g_wmap_land_event_00_b_sequence_7_step, g_wmap_land_event_00_b_sequence_7_timer, 0x40,
                    wmap_land_event_00_b_sequence_7_step_04)

WMAP_STEP_ADVANCE(wmap_land_event_00_b_sequence_7_step_05, g_wmap_land_event_00_b_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_event_00_b_run_sequence_8, g_wmap_land_event_00_b_sequence_8_steps, 6, g_wmap_land_event_00_b_sequence_8_step,
                 g_wmap_land_event_00_b_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_event_00_b_sequence_8_step_00, g_wmap_land_event_00_b_sequence_8_step, g_wmap_land_event_00_b_sequence_8_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_event_00_b_sequence_8_step_01, g_wmap_land_event_00_b_sequence_8_step, g_wmap_land_event_00_b_sequence_8_timer,
                      g_wmap_effect_model_c_rotation, g_wmap_zero_rotation, g_wmap_effect_fade_c, 1, g_wmap_particle_spawn_interval, 0xB4,
                      wmap_land_event_00_b_sequence_8_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_event_00_b_sequence_8_step_03, g_wmap_land_event_00_b_sequence_8_step, g_wmap_land_event_00_b_sequence_8_timer, 0x40,
                    wmap_land_event_00_b_sequence_8_step_04)

WMAP_STEP_ADVANCE(wmap_land_event_00_b_sequence_8_step_05, g_wmap_land_event_00_b_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_event_00_b_run_sequence_9, g_wmap_land_event_00_b_sequence_9_steps, 6, g_wmap_land_event_00_b_sequence_9_step,
                 g_wmap_land_event_00_b_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_event_00_b_sequence_9_step_00, g_wmap_land_event_00_b_sequence_9_step, g_wmap_land_event_00_b_sequence_9_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_event_00_b_sequence_9_step_01, g_wmap_land_event_00_b_sequence_9_step, g_wmap_land_event_00_b_sequence_9_timer,
                      g_wmap_effect_model_d_rotation, g_wmap_zero_rotation, g_wmap_effect_fade_d, 1, g_wmap_particle_radial_step, 0xF8,
                      wmap_land_event_00_b_sequence_9_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_event_00_b_sequence_9_step_03, g_wmap_land_event_00_b_sequence_9_step, g_wmap_land_event_00_b_sequence_9_timer, 0x20,
                    wmap_land_event_00_b_sequence_9_step_04)

WMAP_STEP_ADVANCE(wmap_land_event_00_b_sequence_9_step_05, g_wmap_land_event_00_b_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_event_00_b_run_sequence_10, g_wmap_land_event_00_b_sequence_10_steps, 6, g_wmap_land_event_00_b_sequence_10_step,
                 g_wmap_land_event_00_b_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_event_00_b_sequence_10_step_00, g_wmap_land_event_00_b_sequence_10_step, g_wmap_land_event_00_b_sequence_10_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_event_00_b_sequence_10_step_01, g_wmap_land_event_00_b_sequence_10_step, g_wmap_land_event_00_b_sequence_10_timer,
                      g_wmap_effect_model_g_rotation, g_wmap_zero_rotation, g_wmap_aux_effect_fade_c, 1, g_wmap_particle_first_motion, 0xB4,
                      wmap_land_event_00_b_sequence_10_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_event_00_b_sequence_10_step_03, g_wmap_land_event_00_b_sequence_10_step, g_wmap_land_event_00_b_sequence_10_timer, 0x40,
                    wmap_land_event_00_b_sequence_10_step_04)

WMAP_STEP_ADVANCE(wmap_land_event_00_b_sequence_10_step_05, g_wmap_land_event_00_b_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_event_00_b_run_sequence_11, g_wmap_land_event_00_b_sequence_11_steps, 4, g_wmap_land_event_00_b_sequence_11_step,
                 g_wmap_land_event_00_b_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_event_00_b_sequence_11_step_00, g_wmap_land_event_00_b_sequence_11_step, g_wmap_land_event_00_b_sequence_11_timer)

WMAP_STEP_DROP_START(wmap_land_event_00_b_sequence_11_step_01, g_wmap_land_event_00_b_sequence_11_step, g_wmap_land_event_00_b_sequence_11_timer,
                     g_wmap_effect_model_a_rotation, g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x10,
                     wmap_land_event_00_b_sequence_11_step_02)

WMAP_STEP_ADVANCE(wmap_land_event_00_b_sequence_11_step_03, g_wmap_land_event_00_b_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_event_00_b_run_sequence_12, g_wmap_land_event_00_b_sequence_12_steps, 6, g_wmap_land_event_00_b_sequence_12_step,
                 g_wmap_land_event_00_b_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_event_00_b_sequence_12_step_00, g_wmap_land_event_00_b_sequence_12_step, g_wmap_land_event_00_b_sequence_12_timer)

/** @brief Update and draw the falling sparks until the timer ends. */
void wmap_land_event_00_b_sequence_12_step_02(void)
{
    s32 timer;

    wmap_set_effect_camera_transform();
    wmap_land_effect_10_update_falling_sparks(0x14, 0x78, g_wmap_effect_model_pack_4, &g_wmap_effect_params[30]);
    wmap_queue_texture_page(0x25, 5);
    timer = g_wmap_land_event_00_b_sequence_12_timer - 1;
    g_wmap_land_event_00_b_sequence_12_timer = timer;

    if (timer == 0)
    {
        g_wmap_land_event_00_b_sequence_12_step++;
    }
}

/** @brief Stop new sparks by raising their spawn period, then run step 4. */
void wmap_land_event_00_b_sequence_12_step_03(void)
{
    g_wmap_land_event_00_b_sequence_12_timer = 0x20;
    g_wmap_effect_params[30] = 0x270F;
    g_wmap_land_event_00_b_sequence_12_step++;
    wmap_land_event_00_b_sequence_12_step_04();
}

/** @brief Update and draw the remaining falling sparks until the timer ends. */
void wmap_land_event_00_b_sequence_12_step_04(void)
{
    s32 timer;

    wmap_set_effect_camera_transform();
    wmap_land_effect_10_update_falling_sparks(0x14, 0x78, g_wmap_effect_model_pack_4, &g_wmap_effect_params[30]);
    wmap_queue_texture_page(0x25, 5);
    timer = g_wmap_land_event_00_b_sequence_12_timer - 1;
    g_wmap_land_event_00_b_sequence_12_timer = timer;

    if (timer == 0)
    {
        g_wmap_land_event_00_b_sequence_12_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_event_00_b_sequence_12_step_05, g_wmap_land_event_00_b_sequence_12_step)
