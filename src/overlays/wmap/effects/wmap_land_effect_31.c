#include "common.h"
#include <libgte.h>

#include "../internal/wmap_actor_motion.h"
#include "../internal/wmap_cells.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_effect_resources.h"
#include "../internal/wmap_land_effect_31.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_model_render.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_view_effects.h"
#include "main/cdrom.h"

extern WmapSequenceStep g_wmap_land_effect_31_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_timeline_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_1_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_2_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_3_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_4_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_5_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_6_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_7_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_8_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_9_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_10_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_11_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_12_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_13_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_14_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_15_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_16_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_17_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_18_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_19_steps[];
extern WmapSequenceStep g_wmap_land_effect_31_sequence_20_steps[];
extern u32 g_wmap_land_effect_31_step;
extern s32 g_wmap_land_effect_31_timer;
extern u32 g_wmap_land_effect_31_timeline_step;
extern s32 g_wmap_land_effect_31_timeline_timer;
extern u32 g_wmap_land_effect_31_sequence_1_step;
extern s32 g_wmap_land_effect_31_sequence_1_timer;
extern u32 g_wmap_land_effect_31_sequence_2_step;
extern s32 g_wmap_land_effect_31_sequence_2_timer;
extern u32 g_wmap_land_effect_31_sequence_3_step;
extern s32 g_wmap_land_effect_31_sequence_3_timer;
extern u32 g_wmap_land_effect_31_sequence_4_step;
extern s32 g_wmap_land_effect_31_sequence_4_timer;
extern u32 g_wmap_land_effect_31_sequence_5_step;
extern s32 g_wmap_land_effect_31_sequence_5_timer;
extern u32 g_wmap_land_effect_31_sequence_6_step;
extern s32 g_wmap_land_effect_31_sequence_6_timer;
extern u32 g_wmap_land_effect_31_sequence_7_step;
extern s32 g_wmap_land_effect_31_sequence_7_timer;
extern u32 g_wmap_land_effect_31_sequence_8_step;
extern s32 g_wmap_land_effect_31_sequence_8_timer;
extern u32 g_wmap_land_effect_31_sequence_9_step;
extern s32 g_wmap_land_effect_31_sequence_9_timer;
extern u32 g_wmap_land_effect_31_sequence_10_step;
extern s32 g_wmap_land_effect_31_sequence_10_timer;
extern u32 g_wmap_land_effect_31_sequence_11_step;
extern s32 g_wmap_land_effect_31_sequence_11_timer;
extern u32 g_wmap_land_effect_31_sequence_12_step;
extern s32 g_wmap_land_effect_31_sequence_12_timer;
extern u32 g_wmap_land_effect_31_sequence_13_step;
extern s32 g_wmap_land_effect_31_sequence_13_timer;
extern u32 g_wmap_land_effect_31_sequence_14_step;
extern s32 g_wmap_land_effect_31_sequence_14_timer;
extern u32 g_wmap_land_effect_31_sequence_15_step;
extern s32 g_wmap_land_effect_31_sequence_15_timer;
extern u32 g_wmap_land_effect_31_sequence_16_step;
extern s32 g_wmap_land_effect_31_sequence_16_timer;
extern u32 g_wmap_land_effect_31_sequence_17_step;
extern s32 g_wmap_land_effect_31_sequence_17_timer;
extern u32 g_wmap_land_effect_31_sequence_18_step;
extern s32 g_wmap_land_effect_31_sequence_18_timer;
extern u32 g_wmap_land_effect_31_sequence_19_step;
extern s32 g_wmap_land_effect_31_sequence_19_timer;
extern u32 g_wmap_land_effect_31_sequence_20_step;
extern s32 g_wmap_land_effect_31_sequence_20_timer;

extern SVECTOR g_wmap_land_effect_31_sequence_11_rotation;

extern SVECTOR g_wmap_land_effect_31_sequence_5_rotation;

/** @brief Hide the map view, fade the backdrop to purple, then wait 56 frames. */
void wmap_land_effect_31_timeline_step_17(void)
{
    g_wmap_view_mode = WMAP_VIEW_MODE_HIDDEN;
    g_wmap_backdrop_target_level = 0;
    WMAP_SET_BACKDROP_TARGET(0xB4, 0, 0x96, 0xB4, 0, 0x96);
    g_wmap_land_effect_31_timeline_timer = 0x38;
    g_wmap_land_effect_31_timeline_step++;
}

/** @brief Hide the transition mesh, start sequence 7 and fade the backdrop to light grey. */
void wmap_land_effect_31_timeline_step_19(void)
{
    g_wmap_transition_mesh_hidden = 1;
    wmap_start_sequence(wmap_land_effect_31_run_sequence_7);
    g_wmap_backdrop_target_level = 0;
    WMAP_SET_BACKDROP_TARGET(0xC8, 0xC8, 0xC8, 0xC8, 0xC8, 0xC8);
    g_wmap_land_effect_31_timeline_timer = 0xF;
    g_wmap_land_effect_31_timeline_step++;
}

/** @brief Start sequences 15, 16, 14, 5 and 4 and fade the backdrop from blue at the top to grey at the bottom. */
void wmap_land_effect_31_timeline_step_23(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_15);
    wmap_start_sequence(wmap_land_effect_31_run_sequence_16);
    wmap_start_sequence(wmap_land_effect_31_run_sequence_14);
    wmap_start_sequence(wmap_land_effect_31_run_sequence_5);
    wmap_start_sequence(wmap_land_effect_31_run_sequence_4);
    g_wmap_backdrop_target_level = 0;
    WMAP_SET_BACKDROP_TARGET(0x60, 0xC, 0xC8, 0xA0, 0xA0, 0xA0);
    g_wmap_land_effect_31_timeline_timer = 0x14;
    g_wmap_land_effect_31_timeline_step++;
}

/** @brief Stop the camera move, tint the map, show the map view again and start sequence 19. */
void wmap_land_effect_31_timeline_step_37(void)
{
    g_wmap_backdrop_target_level = 0;
    WMAP_SET_BACKDROP_TARGET(0, 0x64, 0x78, 0, 0x64, 0x78);
    g_wmap_effect_camera_move_active = 0;
    wmap_start_map_tint(0x701050);
    g_wmap_view_mode = WMAP_VIEW_MODE_MAP;
    g_wmap_backdrop_target_level = 0;
    wmap_start_sequence(wmap_land_effect_31_run_sequence_19);
    g_wmap_land_effect_31_timeline_timer = 0x58;
    g_wmap_land_effect_31_timeline_step++;
}

/** @brief Start sequence 6, fade the backdrop to black and reset the gradient from its template. */
void wmap_land_effect_31_timeline_step_41(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_6);
    WMAP_SET_BACKDROP_TARGET(0, 0, 0, 0, 0, 0);
    g_wmap_backdrop_gradient = g_wmap_backdrop_gradient_template;
    g_wmap_land_effect_31_timeline_timer = 0x78;
    g_wmap_land_effect_31_timeline_step++;
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_31_sequence_1_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 40;
    WMAP_SET_PARTICLE_CONFIG(30, -8, 20, 20, 30, 1, 3900, 60, 15, 2, 10000);
    WMAP_RESET_PARTICLE_SLOTS(i, 40, g_wmap_actor_motions[i + g_wmap_effect_params[0x25]].state, 64, g_wmap_animation_bank_1);
    g_wmap_land_effect_31_sequence_1_timer = 40;
    g_wmap_land_effect_31_sequence_1_step++;
    wmap_land_effect_31_sequence_1_step_02();
}

/**
 * @brief Update and draw the model while fading its color in.
 */
void wmap_land_effect_31_sequence_4_step_02(void)
{
    s32 value;
    s32 timer;

    wmap_set_effect_camera_transform();
    wmap_draw_model(g_wmap_effect_model_pack_1, (g_wmap_particle_velocity_min / 0x10) & 3, 0xA, 0x36, 0x7940, 0x1001, g_wmap_effect_wave_phase, 0, 0, -1);
    g_wmap_particle_velocity_min += 0x10;
    WMAP_MODEL_FADE_IN(g_wmap_effect_wave_phase, 2, 0x81, value);
    g_wmap_effect_model_e_rotation.vz += 0x40;
    timer = g_wmap_land_effect_31_sequence_4_timer - 1;
    g_wmap_land_effect_31_sequence_4_timer = timer;

    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_4_step++;
    }
}

/**
 * @brief Update and draw the model while fading its color out.
 */
void wmap_land_effect_31_sequence_4_step_04(void)
{
    s32 value;
    s32 timer;
    SVECTOR* rotation;

    wmap_set_effect_camera_transform();
    wmap_draw_model(g_wmap_effect_model_pack_1, (g_wmap_particle_velocity_min / 0x10) & 3, 0xA, 0x36, 0x7940, 0x1001, g_wmap_effect_wave_phase, 0, 0, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_wave_phase, 2, value);
    rotation = &g_wmap_effect_model_e_rotation;
    g_wmap_particle_velocity_min += 0x10;
    timer = g_wmap_land_effect_31_sequence_4_timer - 1;
    g_wmap_land_effect_31_sequence_4_timer = timer;
    rotation->vz += 0x40;

    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_4_step++;
    }
}

/** @brief Configure the effect and its resource slots, then advance the sequence. */
void wmap_land_effect_31_sequence_5_step_01(void)
{
    s32 i;

    g_wmap_effect_params[0] = 40;
    g_wmap_effect_params[1] = 20;
    g_wmap_effect_params[2] = -210;
    g_wmap_effect_params[3] = 420;
    g_wmap_effect_params[4] = -210;
    g_wmap_effect_params[5] = 420;
    g_wmap_effect_params[6] = 0;
    g_wmap_effect_params[7] = 80;
    g_wmap_effect_params[8] = 129;
    g_wmap_effect_params[9] = 1;
    g_wmap_effect_params[10] = 8;
    g_wmap_effect_params[11] = 80;
    g_wmap_effect_params[12] = 15;
    g_wmap_effect_params[13] = 0;
    g_wmap_land_effect_31_sequence_5_rotation = g_wmap_zero_rotation;

    for (i = 0; i < 40; i++)
    {
        g_wmap_actor_animations[i + 20].data = g_wmap_animation_bank_1;
    }

    g_wmap_land_effect_31_sequence_5_timer = 204;
    g_wmap_land_effect_31_sequence_5_step++;
    wmap_land_effect_31_sequence_5_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_31_sequence_6_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 32;
    WMAP_SET_PARTICLE_CONFIG(20, -3, 8, 20, 30, 1, 1500, 60, 15, 1, 10000);
    WMAP_RESET_PARTICLE_SLOTS(i, 32, g_wmap_actor_motions[i + g_wmap_effect_params[0x1B]].state, 64, g_wmap_animation_bank_1);
    g_wmap_land_effect_31_sequence_6_timer = 32;
    g_wmap_land_effect_31_sequence_6_step++;
    wmap_land_effect_31_sequence_6_step_02();
}

/**
 * @brief Move the model along Z, draw it, and fade it until the timer expires.
 */
WMAP_STEP_MAP_DROP_UPDATE(wmap_land_effect_31_sequence_7_step_02, g_wmap_land_effect_31_sequence_7_step, g_wmap_land_effect_31_sequence_7_timer,
                          g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, g_wmap_effect_model_pack_6, -0xDAC, 1)

/**
 * @brief Move the model along Z, draw it, and fade it until the timer expires.
 */
WMAP_STEP_MAP_DROP_UPDATE(wmap_land_effect_31_sequence_8_step_02, g_wmap_land_effect_31_sequence_8_step, g_wmap_land_effect_31_sequence_8_timer,
                          g_wmap_effect_model_b_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, g_wmap_effect_model_pack_8, -0xDAC, 4)

/**
 * @brief Move the model along Z, draw it, and fade it until the timer expires.
 */
WMAP_STEP_MAP_DROP_UPDATE(wmap_land_effect_31_sequence_9_step_02, g_wmap_land_effect_31_sequence_9_step, g_wmap_land_effect_31_sequence_9_timer,
                          g_wmap_effect_model_c_rotation, g_wmap_effect_model_c_position, g_wmap_effect_fade_c, g_wmap_effect_model_pack_4, -0xDAC, 2)

/**
 * @brief Draw and spin the model while fading it in.
 */
void wmap_land_effect_31_sequence_10_step_02(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&g_wmap_effect_model_f_rotation);
    wmap_draw_model(g_wmap_load_buffer, (g_wmap_particle_velocity_range / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1001, g_wmap_effect_fade_e, 6, -0x18, -1);
    g_wmap_particle_velocity_range += 0x10;
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_e, 4, 0x81, value);
    g_wmap_effect_model_f_rotation.vz += 0x10;
    timer = g_wmap_land_effect_31_sequence_10_timer - 1;
    g_wmap_land_effect_31_sequence_10_timer = timer;

    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_10_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it out.
 */
void wmap_land_effect_31_sequence_10_step_04(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&g_wmap_effect_model_f_rotation);
    wmap_draw_model(g_wmap_load_buffer, (g_wmap_particle_velocity_range / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1001, g_wmap_effect_fade_e, 6, -0x18, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_e, 0x10, value);
    g_wmap_particle_velocity_range += 0x10;
    timer = g_wmap_land_effect_31_sequence_10_timer - 1;
    g_wmap_land_effect_31_sequence_10_timer = timer;
    g_wmap_effect_model_f_rotation.vz += 0x10;

    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_10_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it in.
 */
void wmap_land_effect_31_sequence_11_step_02(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&g_wmap_land_effect_31_sequence_11_rotation);
    wmap_draw_model(g_wmap_load_buffer, (g_wmap_particle_sequence / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1001, g_wmap_aux_effect_fade_b, 4, -0x14, -1);
    g_wmap_particle_sequence -= 0x10;
    WMAP_MODEL_FADE_IN(g_wmap_aux_effect_fade_b, 2, 0x81, value);
    g_wmap_land_effect_31_sequence_11_rotation.vz += 0x10;
    timer = g_wmap_land_effect_31_sequence_11_timer - 1;
    g_wmap_land_effect_31_sequence_11_timer = timer;

    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_11_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it out.
 */
void wmap_land_effect_31_sequence_11_step_04(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&g_wmap_land_effect_31_sequence_11_rotation);
    wmap_draw_model(g_wmap_load_buffer, (g_wmap_particle_sequence / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1001, g_wmap_aux_effect_fade_b, 4, -0x14, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_aux_effect_fade_b, 0x80, value);
    g_wmap_particle_sequence -= 0x10;
    timer = g_wmap_land_effect_31_sequence_11_timer - 1;
    g_wmap_land_effect_31_sequence_11_timer = timer;
    g_wmap_land_effect_31_sequence_11_rotation.vz += 0x10;

    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_11_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it in.
 */
void wmap_land_effect_31_sequence_12_step_02(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&g_wmap_effect_model_d_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, (g_wmap_particle_radial_step / 0x10) & 3, 0xA, 0x36, 0x7900, 0x1001, g_wmap_effect_fade_d, 4, -0x14, -1);
    g_wmap_particle_radial_step -= 0x20;
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_d, 2, 0x81, value);
    g_wmap_effect_model_d_rotation.vz += 0x90;
    timer = g_wmap_land_effect_31_sequence_12_timer - 1;
    g_wmap_land_effect_31_sequence_12_timer = timer;

    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_12_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it out.
 */
void wmap_land_effect_31_sequence_12_step_04(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&g_wmap_effect_model_d_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, (g_wmap_particle_radial_step / 0x10) & 3, 0xA, 0x36, 0x7900, 0x1001, g_wmap_effect_fade_d, 4, -0x14, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_d, 8, value);
    g_wmap_particle_radial_step -= 0x20;
    timer = g_wmap_land_effect_31_sequence_12_timer - 1;
    g_wmap_land_effect_31_sequence_12_timer = timer;
    g_wmap_effect_model_d_rotation.vz += 0x90;

    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_12_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it in.
 */
void wmap_land_effect_31_sequence_13_step_02(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&g_wmap_effect_model_g_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_7, (g_wmap_particle_first_motion / 0x10) & 3, 0xA, 0x36, 0x7900, 0x1001, g_wmap_aux_effect_fade_c, 6, -0x18, -1);
    g_wmap_particle_first_motion += 0x10;
    WMAP_MODEL_FADE_IN(g_wmap_aux_effect_fade_c, 2, 0x81, value);
    g_wmap_effect_model_g_rotation.vz += 0x60;
    timer = g_wmap_land_effect_31_sequence_13_timer - 1;
    g_wmap_land_effect_31_sequence_13_timer = timer;

    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_13_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it out.
 */
void wmap_land_effect_31_sequence_13_step_04(void)
{
    s32 value;
    s32 timer;

    wmap_set_map_rotation(&g_wmap_effect_model_g_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_7, (g_wmap_particle_first_motion / 0x10) & 3, 0xA, 0x36, 0x7900, 0x1001, g_wmap_aux_effect_fade_c, 6, -0x18, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_aux_effect_fade_c, 0x10, value);
    g_wmap_particle_first_motion += 0x10;
    timer = g_wmap_land_effect_31_sequence_13_timer - 1;
    g_wmap_land_effect_31_sequence_13_timer = timer;
    g_wmap_effect_model_g_rotation.vz += 0x60;

    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_13_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it in.
 */
void wmap_land_effect_31_sequence_14_step_02(void)
{
    s32 value;

    wmap_set_effect_camera_transform();
    wmap_draw_model(g_wmap_effect_model_pack_9, (g_wmap_particle_texture_index / 0x10) & 3, 0xA, 0x36, 0x7880, 1, g_wmap_aux_effect_fade_a, 0, 0, -1);
    g_wmap_particle_texture_index += 8;
    WMAP_MODEL_FADE_IN(g_wmap_aux_effect_fade_a, 2, 0x81, value);

    if (wmap_tick_model_motion(&g_wmap_effect_model_h_rotation, 0, &g_wmap_land_effect_31_sequence_14_timer) == 0)
    {
        g_wmap_land_effect_31_sequence_14_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it out.
 */
void wmap_land_effect_31_sequence_14_step_04(void)
{
    s32 value;

    wmap_set_effect_camera_transform();
    wmap_draw_model(g_wmap_effect_model_pack_9, (g_wmap_particle_texture_index / 0x10) & 3, 0xA, 0x36, 0x7880, 1, g_wmap_aux_effect_fade_a, 0, 0, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_aux_effect_fade_a, 4, value);
    g_wmap_particle_texture_index += 8;

    if (wmap_tick_model_motion(&g_wmap_effect_model_h_rotation, 0, &g_wmap_land_effect_31_sequence_14_timer) == 0)
    {
        g_wmap_land_effect_31_sequence_14_step++;
    }
}

/**
 * @brief Assign animation resources to the effect actors and start the next step.
 */
void wmap_land_effect_31_sequence_20_step_01(void)
{
    s32 index;
    s32 config_offset;
    s32 screen_offset;
    uintptr_t config_base;
    uintptr_t screen_base;
    u8* resource;
    u8* screen_entry;
    s16* config_entry;

    index = 0;
    config_base = (uintptr_t)g_wmap_actor_motions;
    screen_base = (uintptr_t)g_wmap_actor_animations;
    resource = g_wmap_animation_bank_2;
    screen_offset = 0x640;
    config_offset = 0xFA0;
    g_wmap_effect_params[40] = 0x28;
    g_wmap_effect_params[41] = 0xC8;
    g_wmap_effect_params[42] = -0x136;
    g_wmap_effect_params[43] = 0x28;
    g_wmap_effect_params[44] = -0x64;
    g_wmap_effect_params[45] = 0x28;
    g_wmap_effect_params[46] = 0;
    g_wmap_effect_params[47] = 0x2D;
    g_wmap_effect_params[48] = 0x81;
    g_wmap_effect_params[49] = 1;
    g_wmap_effect_params[50] = 0x20;
    g_wmap_effect_params[51] = 4;
    g_wmap_effect_params[52] = 0xF;
    g_wmap_effect_params[53] = 0;

    while (index < 0x28)
    {
        screen_entry = (u8*)(screen_offset + screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        ((WmapAnimationSlot*)screen_entry)->data = resource;
    }

    g_wmap_land_effect_31_sequence_20_timer = 0x64;
    g_wmap_land_effect_31_sequence_20_step++;
    wmap_land_effect_31_sequence_20_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_31_run, g_wmap_land_effect_31_steps, 6, g_wmap_land_effect_31_step, g_wmap_land_effect_31_timer)

WMAP_STEP_RESET(wmap_land_effect_31_reset, g_wmap_land_effect_31_step, g_wmap_land_effect_31_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_31_step_01, g_wmap_land_effect_31_step, wmap_run_land_focus, wmap_land_effect_31_step_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_31_step_02, g_wmap_land_effect_31_step, wmap_land_effect_31_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_31_step_03, g_wmap_land_effect_31_step, wmap_land_effect_31_run_timeline, wmap_land_effect_31_step_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_31_step_04, g_wmap_land_effect_31_step, wmap_land_effect_31_step_05)

WMAP_STEP_ADVANCE(wmap_land_effect_31_step_05, g_wmap_land_effect_31_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_timeline, g_wmap_land_effect_31_timeline_steps, 0x2C, g_wmap_land_effect_31_timeline_step,
                 g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_31_timeline_reset, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

/** @brief Play the opening sound, take over the map and start sequence 18. */
void wmap_land_effect_31_timeline_step_01(void)
{
    wmap_play_sound(0x34, 0x80);
    g_wmap_event_active = 1;
    wmap_start_sequence(wmap_land_effect_31_run_sequence_18);
    g_wmap_land_effect_31_timeline_timer = 0xF;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_02, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_31_timeline_step_03, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer,
                         wmap_land_effect_31_run_sequence_20, 0xF)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_04, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_31_timeline_step_05, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer,
                         wmap_land_effect_31_run_sequence_2, 0x16)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_06, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

/** @brief Start sequence 13, raise the backdrop level to 12 and tint the map. */
void wmap_land_effect_31_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_13);
    g_wmap_backdrop_target_level = 0xC;
    wmap_start_map_tint(0x704060);
    g_wmap_land_effect_31_timeline_timer = 0xB;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_08, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

/**
 * @brief Hide the placement overlay and set the wait timer.
 */
WMAP_STEP_HIDE_AND_WAIT(wmap_land_effect_31_timeline_step_09, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer,
                        g_wmap_placement_overlay_hidden, 0x28)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_10, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

/** @brief Start sequences 17 and 10 and tint the map darker. */
void wmap_land_effect_31_timeline_step_11(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_17);
    wmap_start_map_tint(0x352030);
    wmap_start_sequence(wmap_land_effect_31_run_sequence_10);
    g_wmap_land_effect_31_timeline_timer = 0xA;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_12, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

/** @brief Start the decreasing screen fade and the effect camera move. */
void wmap_land_effect_31_timeline_step_13(void)
{
    g_wmap_screen_fade_mode = WMAP_FADE_DECREASE;
    g_wmap_effect_camera_move_active = 1;
    g_wmap_land_effect_31_timeline_timer = 4;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_14, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

/** @brief Clear the map tint and set the backdrop level to 1. */
void wmap_land_effect_31_timeline_step_15(void)
{
    wmap_start_map_tint(0);
    g_wmap_backdrop_target_level = 1;
    g_wmap_land_effect_31_timeline_timer = 0xF;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_16, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_18, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_20, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

/** @brief Install the effect camera move with its rotation and translation steps and targets. */
void wmap_land_effect_31_timeline_step_21(void)
{
    wmap_install_callback(wmap_run_effect_camera_move);
    g_wmap_effect_camera_rotation_step.vx = 0x132;
    g_wmap_effect_camera_rotation_step.vy = 0;
    g_wmap_effect_camera_rotation_step.vz = 0x1C2;
    g_wmap_effect_camera_translation_step.vx = 0x26C;
    g_wmap_effect_camera_translation_step.vy = 0;
    g_wmap_effect_camera_translation_step.vz = 0;
    g_wmap_effect_camera_rotation_target.vx = 0x132;
    g_wmap_effect_camera_rotation_target.vy = 0;
    g_wmap_effect_camera_rotation_target.vz = -0x1C2;
    g_wmap_effect_camera_translation_target.vx = 0x26C;
    g_wmap_effect_camera_translation_target.vy = 0;
    g_wmap_effect_camera_translation_target.vz = 0;
    g_wmap_land_effect_31_timeline_timer = 0xA;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_22, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_24, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

/** @brief Slow the camera move to a small x translation step with zero targets. */
void wmap_land_effect_31_timeline_step_25(void)
{
    g_wmap_effect_camera_rotation_step.vx = 0;
    g_wmap_effect_camera_rotation_step.vy = 0;
    g_wmap_effect_camera_rotation_step.vz = 0;
    g_wmap_effect_camera_translation_step.vx = 5;
    g_wmap_effect_camera_translation_step.vy = 0;
    g_wmap_effect_camera_translation_step.vz = 0;
    g_wmap_effect_camera_rotation_target.vx = 0;
    g_wmap_effect_camera_rotation_target.vy = 0;
    g_wmap_effect_camera_rotation_target.vz = 0;
    g_wmap_effect_camera_translation_target.vx = 0;
    g_wmap_effect_camera_translation_target.vy = 0;
    g_wmap_effect_camera_translation_target.vz = 0;
    g_wmap_land_effect_31_timeline_timer = 0x1E;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_26, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_31_timeline_step_27, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer,
                         wmap_land_effect_31_run_sequence_3, 0xA0)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_28, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_31_timeline_step_29, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer,
                         wmap_land_effect_31_run_sequence_8, 7)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_30, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_31_timeline_step_31, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer,
                         wmap_land_effect_31_run_sequence_11, 0x14)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_32, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_31_timeline_step_33, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer,
                         wmap_land_effect_31_run_sequence_1, 0x28)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_34, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

/** @brief Start sequence 12 and fade the backdrop to black. */
void wmap_land_effect_31_timeline_step_35(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_12);
    g_wmap_backdrop_target_level = 0;
    WMAP_SET_BACKDROP_TARGET(0, 0, 0, 0, 0, 0);
    g_wmap_land_effect_31_timeline_timer = 0x1E;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_36, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_38, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

/** @brief Start sequence 9, restore the map tint and backdrop, show the transition mesh and place the selected artifact's land in the focused cell. */
void wmap_land_effect_31_timeline_step_39(void)
{
    wmap_start_sequence(wmap_land_effect_31_run_sequence_9);
    wmap_start_map_tint(0x808080);
    g_wmap_backdrop_target_level = 0x10;
    g_wmap_transition_mesh_hidden = 0;
    g_wmap_land_effect_31_timeline_timer = 0x1C;
    g_wmap_cells[g_wmap_focus_cell_x][g_wmap_focus_cell_y].land_id = g_wmap_selected_artifact | WMAP_LAND_FULL_SIZE;
    g_wmap_land_effect_31_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_40, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

WMAP_STEP_WAIT(wmap_land_effect_31_timeline_step_42, g_wmap_land_effect_31_timeline_step, g_wmap_land_effect_31_timeline_timer)

/**
 * @brief Clear the blocking flag and finish the timeline.
 */
WMAP_STEP_FINISH_BLOCKING(wmap_land_effect_31_timeline_finish, g_wmap_land_effect_31_timeline_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_1, g_wmap_land_effect_31_sequence_1_steps, 6, g_wmap_land_effect_31_sequence_1_step,
                 g_wmap_land_effect_31_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_1_reset, g_wmap_land_effect_31_sequence_1_step, g_wmap_land_effect_31_sequence_1_timer)

/**
 * @brief Run the effect update until the timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_31_sequence_1_step_02, g_wmap_land_effect_31_sequence_1_step, g_wmap_land_effect_31_sequence_1_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[64], &g_wmap_actor_animations[64], 0x28, 0, 0x7F, 4, 0,
                                                          &g_wmap_effect_params[30]))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_31_sequence_1_step_03, g_wmap_land_effect_31_sequence_1_step, g_wmap_land_effect_31_sequence_1_timer,
                       g_wmap_effect_params[35], 0x20, wmap_land_effect_31_sequence_1_step_04)

/**
 * @brief Run the effect update until the timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_31_sequence_1_step_04, g_wmap_land_effect_31_sequence_1_step, g_wmap_land_effect_31_sequence_1_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[64], &g_wmap_actor_animations[64], 0x28, 0, 0x7F, 4, 0,
                                                          &g_wmap_effect_params[30]))

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_1_end, g_wmap_land_effect_31_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_2, g_wmap_land_effect_31_sequence_2_steps, 4, g_wmap_land_effect_31_sequence_2_step,
                 g_wmap_land_effect_31_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_2_reset, g_wmap_land_effect_31_sequence_2_step, g_wmap_land_effect_31_sequence_2_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_31_sequence_2_step_01, g_wmap_land_effect_31_sequence_2_step, g_wmap_land_effect_31_sequence_2_timer, 5,
                      g_wmap_animation_bank_1, 3, 1, 0x81, 8, 0x64, wmap_land_effect_31_sequence_2_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_31_sequence_2_step_02, g_wmap_land_effect_31_sequence_2_step, g_wmap_land_effect_31_sequence_2_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0xF, 2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_2_end, g_wmap_land_effect_31_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_3, g_wmap_land_effect_31_sequence_3_steps, 6, g_wmap_land_effect_31_sequence_3_step,
                 g_wmap_land_effect_31_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_3_reset, g_wmap_land_effect_31_sequence_3_step, g_wmap_land_effect_31_sequence_3_timer)

/** @brief Set up sprite actor 6 with animation bank 0 and fade it in, then run step 2. */
void wmap_land_effect_31_sequence_3_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    g_wmap_actor_animations[6].data = g_wmap_animation_bank_0;
    actor->previous_sequence = -1;
    actor->shade_step = 2;
    actor->target_shade = 0x81;
    actor->resource_index = 0;
    actor->scale_index = 0;
    actor->sequence = 0;
    actor->shade = 1;
    g_wmap_land_effect_31_sequence_3_timer = 0x10E;
    g_wmap_land_effect_31_sequence_3_step++;
    wmap_land_effect_31_sequence_3_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_31_sequence_3_step_02, g_wmap_land_effect_31_sequence_3_step, g_wmap_land_effect_31_sequence_3_timer,
                              g_wmap_sprite_actors[6], g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x10, 8, 1)

/** @brief Start fading out sprite actor 6 and reset its growth, then run step 4. */
void wmap_land_effect_31_sequence_3_step_03(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    actor->shade_step = 2;
    actor->target_shade = 0;
    g_wmap_particle_texture_index = 0;
    g_wmap_land_effect_31_sequence_3_timer = 0x40;
    g_wmap_land_effect_31_sequence_3_step++;
    wmap_land_effect_31_sequence_3_step_04();
}

/** @brief Animate and draw sprite actor 6 at the focus while it grows, until the timer ends. */
void wmap_land_effect_31_sequence_3_step_04(void)
{
    s32 timer;

    wmap_step_actor_animation(&g_wmap_sprite_actors[6], &g_wmap_actor_animations[6]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[6], g_wmap_focus_screen_position.packed, 0x10, 8, 1);
    g_wmap_sprite_actors[6].scale_index = g_wmap_particle_texture_index >> 4;
    g_wmap_particle_texture_index += 0x10;

    if (g_wmap_particle_texture_index >= 0xF1)
    {
        g_wmap_particle_texture_index = 0xF0;
    }

    timer = g_wmap_land_effect_31_sequence_3_timer - 1;
    g_wmap_land_effect_31_sequence_3_timer = timer;

    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_3_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_3_end, g_wmap_land_effect_31_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_4, g_wmap_land_effect_31_sequence_4_steps, 6, g_wmap_land_effect_31_sequence_4_step,
                 g_wmap_land_effect_31_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_4_reset, g_wmap_land_effect_31_sequence_4_step, g_wmap_land_effect_31_sequence_4_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_effect_31_sequence_4_step_01, g_wmap_land_effect_31_sequence_4_step, g_wmap_land_effect_31_sequence_4_timer,
                      g_wmap_effect_model_e_rotation, g_wmap_zero_rotation, g_wmap_effect_wave_phase, 1, g_wmap_particle_velocity_min, 0x92,
                      wmap_land_effect_31_sequence_4_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_31_sequence_4_step_03, g_wmap_land_effect_31_sequence_4_step, g_wmap_land_effect_31_sequence_4_timer, 0x40,
                    wmap_land_effect_31_sequence_4_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_4_end, g_wmap_land_effect_31_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_5, g_wmap_land_effect_31_sequence_5_steps, 6, g_wmap_land_effect_31_sequence_5_step,
                 g_wmap_land_effect_31_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_5_reset, g_wmap_land_effect_31_sequence_5_step, g_wmap_land_effect_31_sequence_5_timer)

/** @brief Rotate the box-particle transform and update the particles until the timer ends. */
void wmap_land_effect_31_sequence_5_step_02(void)
{
    s32 timer;

    wmap_set_effect_model_transform(&g_wmap_camera_translation, &g_wmap_land_effect_31_sequence_5_rotation);
    wmap_update_box_particles((WmapBoxParticleConfig*)g_wmap_effect_params);
    g_wmap_land_effect_31_sequence_5_rotation.vz += 0x18;
    timer = g_wmap_land_effect_31_sequence_5_timer - 1;
    g_wmap_land_effect_31_sequence_5_timer = timer;

    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_5_step++;
    }
}

/** @brief Fade out the 40 particle sprites from actor 20, then run step 4. */
void wmap_land_effect_31_sequence_5_step_03(void)
{
    s32 i;
    WmapSpriteActor* actors;

    actors = &g_wmap_sprite_actors[20];

    for (i = 0; i < 0x28; i++)
    {
        actors[i].target_shade = 0;
        actors[i].shade_step = 8;
    }

    g_wmap_land_effect_31_sequence_5_timer = 0x10;
    g_wmap_land_effect_31_sequence_5_step++;
    wmap_land_effect_31_sequence_5_step_04();
}

/** @brief Keep rotating and updating the box particles while they fade out. */
void wmap_land_effect_31_sequence_5_step_04(void)
{
    s32 timer;

    wmap_set_effect_model_transform(&g_wmap_camera_translation, &g_wmap_land_effect_31_sequence_5_rotation);
    wmap_update_box_particles((WmapBoxParticleConfig*)g_wmap_effect_params);
    g_wmap_land_effect_31_sequence_5_rotation.vz += 0x18;
    timer = g_wmap_land_effect_31_sequence_5_timer - 1;
    g_wmap_land_effect_31_sequence_5_timer = timer;

    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_5_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_5_end, g_wmap_land_effect_31_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_6, g_wmap_land_effect_31_sequence_6_steps, 6, g_wmap_land_effect_31_sequence_6_step,
                 g_wmap_land_effect_31_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_6_reset, g_wmap_land_effect_31_sequence_6_step, g_wmap_land_effect_31_sequence_6_timer)

/**
 * @brief Run the effect update until the timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_31_sequence_6_step_02, g_wmap_land_effect_31_sequence_6_step, g_wmap_land_effect_31_sequence_6_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[64], &g_wmap_actor_animations[64], 0x20, 0, 0x7F, 2, 0,
                                                          &g_wmap_effect_params[20]))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_31_sequence_6_step_03, g_wmap_land_effect_31_sequence_6_step, g_wmap_land_effect_31_sequence_6_timer,
                       g_wmap_effect_params[25], 0x20, wmap_land_effect_31_sequence_6_step_04)

/**
 * @brief Run the effect update until the timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_31_sequence_6_step_04, g_wmap_land_effect_31_sequence_6_step, g_wmap_land_effect_31_sequence_6_timer,
                              wmap_set_effect_camera_transform(),
                              wmap_update_particle_system(&g_wmap_sprite_actors[64], &g_wmap_actor_animations[64], 0x20, 0, 0x7F, 2, 0,
                                                          &g_wmap_effect_params[20]))

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_6_end, g_wmap_land_effect_31_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_7, g_wmap_land_effect_31_sequence_7_steps, 4, g_wmap_land_effect_31_sequence_7_step,
                 g_wmap_land_effect_31_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_7_reset, g_wmap_land_effect_31_sequence_7_step, g_wmap_land_effect_31_sequence_7_timer)

WMAP_STEP_DROP_START(wmap_land_effect_31_sequence_7_step_01, g_wmap_land_effect_31_sequence_7_step, g_wmap_land_effect_31_sequence_7_timer,
                     g_wmap_effect_model_a_rotation, g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x80,
                     wmap_land_effect_31_sequence_7_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_7_end, g_wmap_land_effect_31_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_8, g_wmap_land_effect_31_sequence_8_steps, 4, g_wmap_land_effect_31_sequence_8_step,
                 g_wmap_land_effect_31_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_8_reset, g_wmap_land_effect_31_sequence_8_step, g_wmap_land_effect_31_sequence_8_timer)

WMAP_STEP_DROP_START(wmap_land_effect_31_sequence_8_step_01, g_wmap_land_effect_31_sequence_8_step, g_wmap_land_effect_31_sequence_8_timer,
                     g_wmap_effect_model_b_rotation, g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x20,
                     wmap_land_effect_31_sequence_8_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_8_end, g_wmap_land_effect_31_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_9, g_wmap_land_effect_31_sequence_9_steps, 4, g_wmap_land_effect_31_sequence_9_step,
                 g_wmap_land_effect_31_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_9_reset, g_wmap_land_effect_31_sequence_9_step, g_wmap_land_effect_31_sequence_9_timer)

WMAP_STEP_DROP_START(wmap_land_effect_31_sequence_9_step_01, g_wmap_land_effect_31_sequence_9_step, g_wmap_land_effect_31_sequence_9_timer,
                     g_wmap_effect_model_c_rotation, g_wmap_zero_rotation, g_wmap_effect_model_c_position, g_wmap_effect_fade_c, 0x80, 0xAFC8, 0x40,
                     wmap_land_effect_31_sequence_9_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_9_end, g_wmap_land_effect_31_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_10, g_wmap_land_effect_31_sequence_10_steps, 6, g_wmap_land_effect_31_sequence_10_step,
                 g_wmap_land_effect_31_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_10_reset, g_wmap_land_effect_31_sequence_10_step, g_wmap_land_effect_31_sequence_10_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_effect_31_sequence_10_step_01, g_wmap_land_effect_31_sequence_10_step, g_wmap_land_effect_31_sequence_10_timer,
                      g_wmap_effect_model_f_rotation, g_wmap_zero_rotation, g_wmap_effect_fade_e, 1, g_wmap_particle_velocity_range, 0x5A,
                      wmap_land_effect_31_sequence_10_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_31_sequence_10_step_03, g_wmap_land_effect_31_sequence_10_step, g_wmap_land_effect_31_sequence_10_timer, 8,
                    wmap_land_effect_31_sequence_10_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_10_end, g_wmap_land_effect_31_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_11, g_wmap_land_effect_31_sequence_11_steps, 6, g_wmap_land_effect_31_sequence_11_step,
                 g_wmap_land_effect_31_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_11_reset, g_wmap_land_effect_31_sequence_11_step, g_wmap_land_effect_31_sequence_11_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_effect_31_sequence_11_step_01, g_wmap_land_effect_31_sequence_11_step, g_wmap_land_effect_31_sequence_11_timer,
                      g_wmap_land_effect_31_sequence_11_rotation, g_wmap_zero_rotation, g_wmap_aux_effect_fade_b, 1, g_wmap_particle_sequence, 0xB6,
                      wmap_land_effect_31_sequence_11_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_31_sequence_11_step_03, g_wmap_land_effect_31_sequence_11_step, g_wmap_land_effect_31_sequence_11_timer, 1,
                    wmap_land_effect_31_sequence_11_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_11_end, g_wmap_land_effect_31_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_12, g_wmap_land_effect_31_sequence_12_steps, 6, g_wmap_land_effect_31_sequence_12_step,
                 g_wmap_land_effect_31_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_12_reset, g_wmap_land_effect_31_sequence_12_step, g_wmap_land_effect_31_sequence_12_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_effect_31_sequence_12_step_01, g_wmap_land_effect_31_sequence_12_step, g_wmap_land_effect_31_sequence_12_timer,
                      g_wmap_effect_model_d_rotation, g_wmap_zero_rotation, g_wmap_effect_fade_d, 0x81, g_wmap_particle_radial_step, 0x68,
                      wmap_land_effect_31_sequence_12_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_31_sequence_12_step_03, g_wmap_land_effect_31_sequence_12_step, g_wmap_land_effect_31_sequence_12_timer, 0x10,
                    wmap_land_effect_31_sequence_12_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_12_end, g_wmap_land_effect_31_sequence_12_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_13, g_wmap_land_effect_31_sequence_13_steps, 6, g_wmap_land_effect_31_sequence_13_step,
                 g_wmap_land_effect_31_sequence_13_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_13_reset, g_wmap_land_effect_31_sequence_13_step, g_wmap_land_effect_31_sequence_13_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_effect_31_sequence_13_step_01, g_wmap_land_effect_31_sequence_13_step, g_wmap_land_effect_31_sequence_13_timer,
                      g_wmap_effect_model_g_rotation, g_wmap_zero_rotation, g_wmap_aux_effect_fade_c, 1, g_wmap_particle_first_motion, 0x87,
                      wmap_land_effect_31_sequence_13_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_31_sequence_13_step_03, g_wmap_land_effect_31_sequence_13_step, g_wmap_land_effect_31_sequence_13_timer, 8,
                    wmap_land_effect_31_sequence_13_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_13_end, g_wmap_land_effect_31_sequence_13_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_14, g_wmap_land_effect_31_sequence_14_steps, 6, g_wmap_land_effect_31_sequence_14_step,
                 g_wmap_land_effect_31_sequence_14_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_14_reset, g_wmap_land_effect_31_sequence_14_step, g_wmap_land_effect_31_sequence_14_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_effect_31_sequence_14_step_01, g_wmap_land_effect_31_sequence_14_step, g_wmap_land_effect_31_sequence_14_timer,
                      g_wmap_effect_model_h_rotation, g_wmap_zero_rotation, g_wmap_aux_effect_fade_a, 1, g_wmap_particle_texture_index, 0xF0,
                      wmap_land_effect_31_sequence_14_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_31_sequence_14_step_03, g_wmap_land_effect_31_sequence_14_step, g_wmap_land_effect_31_sequence_14_timer, 0x20,
                    wmap_land_effect_31_sequence_14_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_14_end, g_wmap_land_effect_31_sequence_14_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_15, g_wmap_land_effect_31_sequence_15_steps, 6, g_wmap_land_effect_31_sequence_15_step,
                 g_wmap_land_effect_31_sequence_15_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_15_reset, g_wmap_land_effect_31_sequence_15_step, g_wmap_land_effect_31_sequence_15_timer)

/** @brief Reset the model fade level, then run step 2. */
void wmap_land_effect_31_sequence_15_step_01(void)
{
    g_wmap_particle_lifetime_min = 0;
    g_wmap_land_effect_31_sequence_15_timer = 0xF8;
    g_wmap_land_effect_31_sequence_15_step++;
    wmap_land_effect_31_sequence_15_step_02();
}

/** @brief Draw model pack 5 while fading it in, until the timer ends. */
void wmap_land_effect_31_sequence_15_step_02(void)
{
    s32 timer;

    wmap_set_effect_camera_transform();
    wmap_draw_model(g_wmap_effect_model_pack_5, 0, 0xB0, 0x35, 0x7800, 1, g_wmap_particle_lifetime_min, -0x32, 0, -1);
    g_wmap_particle_lifetime_min += 8;

    if (g_wmap_particle_lifetime_min >= 0x81)
    {
        g_wmap_particle_lifetime_min = 0x80;
    }

    timer = g_wmap_land_effect_31_sequence_15_timer - 1;
    g_wmap_land_effect_31_sequence_15_timer = timer;

    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_15_step++;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_31_sequence_15_step_03, g_wmap_land_effect_31_sequence_15_step, g_wmap_land_effect_31_sequence_15_timer, 0x10,
                    wmap_land_effect_31_sequence_15_step_04)

/** @brief Draw model pack 5 while fading it out, until the timer ends. */
void wmap_land_effect_31_sequence_15_step_04(void)
{
    s32 timer;

    wmap_set_effect_camera_transform();
    wmap_draw_model(g_wmap_effect_model_pack_5, 0, 0xB0, 0x35, 0x7800, 1, g_wmap_particle_lifetime_min, -0x32, 0, -1);
    g_wmap_particle_lifetime_min -= 8;

    if (g_wmap_particle_lifetime_min < 0)
    {
        g_wmap_particle_lifetime_min = 0;
    }

    timer = g_wmap_land_effect_31_sequence_15_timer - 1;
    g_wmap_land_effect_31_sequence_15_timer = timer;

    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_15_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_15_end, g_wmap_land_effect_31_sequence_15_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_16, g_wmap_land_effect_31_sequence_16_steps, 6, g_wmap_land_effect_31_sequence_16_step,
                 g_wmap_land_effect_31_sequence_16_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_16_reset, g_wmap_land_effect_31_sequence_16_step, g_wmap_land_effect_31_sequence_16_timer)

/** @brief Set the model fade level to 1, then run step 2. */
void wmap_land_effect_31_sequence_16_step_01(void)
{
    g_wmap_particle_lifetime_range = 1;
    g_wmap_land_effect_31_sequence_16_timer = 0x70;
    g_wmap_land_effect_31_sequence_16_step++;
    wmap_land_effect_31_sequence_16_step_02();
}

/** @brief Draw model pack 2 while fading it in, until the timer ends. */
void wmap_land_effect_31_sequence_16_step_02(void)
{
    s32 timer;

    wmap_set_effect_camera_transform();
    wmap_draw_model(g_wmap_effect_model_pack_2, 0, 0xA, 0x35, 0x7840, 1, g_wmap_particle_lifetime_range, 0, 0, -1);
    g_wmap_particle_lifetime_range += 8;

    if (g_wmap_particle_lifetime_range >= 0x82)
    {
        g_wmap_particle_lifetime_range = 0x81;
    }

    timer = g_wmap_land_effect_31_sequence_16_timer - 1;
    g_wmap_land_effect_31_sequence_16_timer = timer;

    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_16_step++;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_31_sequence_16_step_03, g_wmap_land_effect_31_sequence_16_step, g_wmap_land_effect_31_sequence_16_timer, 0x20,
                    wmap_land_effect_31_sequence_16_step_04)

/** @brief Draw model pack 2 while fading it out, until the timer ends. */
void wmap_land_effect_31_sequence_16_step_04(void)
{
    s32 timer;

    wmap_set_effect_camera_transform();
    wmap_draw_model(g_wmap_effect_model_pack_2, 0, 0xA, 0x35, 0x7840, 1, g_wmap_particle_lifetime_range, 0, 0, -1);
    g_wmap_particle_lifetime_range -= 4;

    if (g_wmap_particle_lifetime_range < 0)
    {
        g_wmap_particle_lifetime_range = 0;
    }

    timer = g_wmap_land_effect_31_sequence_16_timer - 1;
    g_wmap_land_effect_31_sequence_16_timer = timer;

    if (timer == 0)
    {
        g_wmap_land_effect_31_sequence_16_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_16_end, g_wmap_land_effect_31_sequence_16_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_17, g_wmap_land_effect_31_sequence_17_steps, 4, g_wmap_land_effect_31_sequence_17_step,
                 g_wmap_land_effect_31_sequence_17_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_17_reset, g_wmap_land_effect_31_sequence_17_step, g_wmap_land_effect_31_sequence_17_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_31_sequence_17_step_01, g_wmap_land_effect_31_sequence_17_step, g_wmap_land_effect_31_sequence_17_timer, 7,
                      g_wmap_animation_bank_0, 1, 1, 0x61, 4, 0x64, wmap_land_effect_31_sequence_17_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_31_sequence_17_step_02, g_wmap_land_effect_31_sequence_17_step, g_wmap_land_effect_31_sequence_17_timer,
                              g_wmap_sprite_actors[7], g_wmap_actor_animations[7], g_wmap_focus_screen_position, 0x2B, 8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_17_end, g_wmap_land_effect_31_sequence_17_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_18, g_wmap_land_effect_31_sequence_18_steps, 6, g_wmap_land_effect_31_sequence_18_step,
                 g_wmap_land_effect_31_sequence_18_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_18_reset, g_wmap_land_effect_31_sequence_18_step, g_wmap_land_effect_31_sequence_18_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_31_sequence_18_step_01, g_wmap_land_effect_31_sequence_18_step, g_wmap_land_effect_31_sequence_18_timer, 8,
                      g_wmap_animation_bank_0, 2, 1, 0x81, 2, 0x6B, wmap_land_effect_31_sequence_18_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_31_sequence_18_step_02, g_wmap_land_effect_31_sequence_18_step, g_wmap_land_effect_31_sequence_18_timer,
                              g_wmap_sprite_actors[8], g_wmap_actor_animations[8], g_wmap_focus_screen_position, 0x10, 8, 0)

/** @brief Start the sprite fade and run its first update. */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_31_sequence_18_step_03, g_wmap_land_effect_31_sequence_18_step, g_wmap_land_effect_31_sequence_18_timer,
                     g_wmap_sprite_actors[8], 8, 0x10, wmap_land_effect_31_sequence_18_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_31_sequence_18_step_04, g_wmap_land_effect_31_sequence_18_step, g_wmap_land_effect_31_sequence_18_timer,
                              g_wmap_sprite_actors[8], g_wmap_actor_animations[8], g_wmap_focus_screen_position, 0x10, 8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_18_end, g_wmap_land_effect_31_sequence_18_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_19, g_wmap_land_effect_31_sequence_19_steps, 6, g_wmap_land_effect_31_sequence_19_step,
                 g_wmap_land_effect_31_sequence_19_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_19_reset, g_wmap_land_effect_31_sequence_19_step, g_wmap_land_effect_31_sequence_19_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_31_sequence_19_step_01, g_wmap_land_effect_31_sequence_19_step, g_wmap_land_effect_31_sequence_19_timer, 9,
                      g_wmap_animation_bank_0, 3, 1, 0x81, 2, 0xB4, wmap_land_effect_31_sequence_19_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_31_sequence_19_step_02, g_wmap_land_effect_31_sequence_19_step, g_wmap_land_effect_31_sequence_19_timer,
                              g_wmap_sprite_actors[9], g_wmap_actor_animations[9], g_wmap_focus_screen_position, 0x10, 8, 0)

/** @brief Start the sprite fade and run its first update. */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_31_sequence_19_step_03, g_wmap_land_effect_31_sequence_19_step, g_wmap_land_effect_31_sequence_19_timer,
                     g_wmap_sprite_actors[9], 4, 0x20, wmap_land_effect_31_sequence_19_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_31_sequence_19_step_04, g_wmap_land_effect_31_sequence_19_step, g_wmap_land_effect_31_sequence_19_timer,
                              g_wmap_sprite_actors[9], g_wmap_actor_animations[9], g_wmap_focus_screen_position, 0x10, 8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_19_end, g_wmap_land_effect_31_sequence_19_step)

WMAP_STEP_RUNNER(wmap_land_effect_31_run_sequence_20, g_wmap_land_effect_31_sequence_20_steps, 6, g_wmap_land_effect_31_sequence_20_step,
                 g_wmap_land_effect_31_sequence_20_timer)

WMAP_STEP_RESET(wmap_land_effect_31_sequence_20_reset, g_wmap_land_effect_31_sequence_20_step, g_wmap_land_effect_31_sequence_20_timer)

/**
 * @brief Run the effect update until the timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_31_sequence_20_step_02, g_wmap_land_effect_31_sequence_20_step, g_wmap_land_effect_31_sequence_20_timer,
                          wmap_update_box_particles((WmapBoxParticleConfig*)&g_wmap_effect_params[40]))

/** @brief Quickly fade out the 40 sprite actors from actor 200, then run step 4. */
void wmap_land_effect_31_sequence_20_step_03(void)
{
    s32 i;
    WmapSpriteActor* actors = &g_wmap_sprite_actors[200];

    for (i = 0; i < 0x28; i++)
    {
        actors[i].target_shade = 0;
        actors[i].shade_step = 0x20;
    }

    g_wmap_land_effect_31_sequence_20_timer = 4;
    g_wmap_land_effect_31_sequence_20_step++;
    wmap_land_effect_31_sequence_20_step_04();
}

/**
 * @brief Run the effect update until the timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_31_sequence_20_step_04, g_wmap_land_effect_31_sequence_20_step, g_wmap_land_effect_31_sequence_20_timer,
                          wmap_update_box_particles((WmapBoxParticleConfig*)&g_wmap_effect_params[40]))

WMAP_STEP_ADVANCE(wmap_land_effect_31_sequence_20_end, g_wmap_land_effect_31_sequence_20_step)
