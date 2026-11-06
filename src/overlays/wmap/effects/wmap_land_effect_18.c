#include "common.h"
#include <inline_c.h>
#include <libgte.h>
#include <rand.h>

#include "../internal/wmap_cells.h"
#include "../internal/wmap_effect_resources.h"
#include "../internal/wmap_land_effect_18.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_spark_effect.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_view_effects.h"
#include "sdk/gte_dmpsx_compat.h"
#include "../internal/wmap_effect_primitives.h"

extern s32 g_wmap_land_effect_18_sequence_1_timer;
extern s32 g_wmap_land_effect_18_sequence_2_timer;
extern s32 g_wmap_land_effect_18_sequence_3_timer;
extern s32 g_wmap_land_effect_18_depth_stage;
extern s32 g_wmap_land_effect_18_flash_2_fade;
extern s32 g_wmap_land_effect_18_sequence_4_timer;
extern s32 g_wmap_land_effect_18_sequence_6_timer;
extern s32 g_wmap_land_effect_18_sequence_9_timer;
extern s32 g_wmap_land_effect_18_sequence_10_timer;
extern s32 g_wmap_land_effect_18_sequence_11_timer;
extern WmapSequenceStep g_wmap_land_effect_18_sequence_1_steps[];
extern WmapSequenceStep g_wmap_land_effect_18_sequence_2_steps[];
extern WmapSequenceStep g_wmap_land_effect_18_sequence_3_steps[];
extern WmapSequenceStep g_wmap_land_effect_18_sequence_4_steps[];
extern s32 g_wmap_land_effect_18_sequence_5_timer;
extern WmapSequenceStep g_wmap_land_effect_18_sequence_5_steps[];
extern WmapSequenceStep g_wmap_land_effect_18_sequence_6_steps[];
extern s32 g_wmap_land_effect_18_sequence_7_timer;
extern WmapSequenceStep g_wmap_land_effect_18_sequence_7_steps[];
extern s32 g_wmap_land_effect_18_timeline_timer;
extern WmapSequenceStep g_wmap_land_effect_18_timeline_steps[];
extern s32 g_wmap_land_effect_18_sequence_8_timer;
extern WmapSequenceStep g_wmap_land_effect_18_sequence_8_steps[];
extern WmapSequenceStep g_wmap_land_effect_18_sequence_9_steps[];
extern WmapSequenceStep g_wmap_land_effect_18_sequence_10_steps[];
extern WmapSequenceStep g_wmap_land_effect_18_sequence_11_steps[];
extern s32 g_wmap_land_effect_18_timer;
extern WmapSequenceStep g_wmap_land_effect_18_steps[];
extern u32 g_wmap_land_effect_18_sequence_1_step;
extern u32 g_wmap_land_effect_18_sequence_2_step;
extern u32 g_wmap_land_effect_18_sequence_3_step;
extern u32 g_wmap_land_effect_18_sequence_4_step;
extern u32 g_wmap_land_effect_18_sequence_6_step;
extern u32 g_wmap_land_effect_18_sequence_9_step;
extern u32 g_wmap_land_effect_18_sequence_10_step;
extern u32 g_wmap_land_effect_18_sequence_11_step;
extern u32 g_wmap_land_effect_18_sequence_5_step;
extern u32 g_wmap_land_effect_18_sequence_7_step;
extern u32 g_wmap_land_effect_18_timeline_step;
extern u32 g_wmap_land_effect_18_sequence_8_step;
extern s32 g_wmap_land_effect_18_camera_spin_speed;
extern u32 g_wmap_land_effect_18_step;

/** @brief Compose the effect transform, draw it, and advance its rotation and countdown. */
void wmap_land_effect_18_sequence_1_step_02(void)
{
    MATRIX base;
    MATRIX effect;
    s32 remaining;

    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base);
    TransMatrix(&base, &g_wmap_camera_translation);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&g_wmap_effect_model_a_rotation, &effect);
    TransMatrix(&effect, &g_wmap_zero_translation);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    wmap_draw_model_default(g_wmap_effect_model_pack_2, 0, 36, 183, 0x7A40, 1, -1);
    PopMatrix();
    remaining = g_wmap_land_effect_18_sequence_1_timer - 1;
    g_wmap_effect_model_a_rotation.vz = (g_wmap_effect_model_a_rotation.vz + 320);
    g_wmap_land_effect_18_sequence_1_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_1_step++;
    }
}

/** @brief Draw and fade the transformed effect, then advance its countdown. */
void wmap_land_effect_18_sequence_1_step_04(void)
{
    MATRIX base;
    MATRIX effect;
    s32 intensity;
    s32 remaining;

    if (g_wmap_effect_wave_phase != 0)
    {
        PushMatrix();
        RotMatrix(&g_wmap_camera_rotation, &base);
        TransMatrix(&base, &g_wmap_camera_translation);
        SetRotMatrix(&base);
        SetTransMatrix(&base);
        RotMatrix(&g_wmap_effect_model_a_rotation, &effect);
        TransMatrix(&effect, &g_wmap_zero_translation);
        CompMatrix(&base, &effect, &effect);
        SetRotMatrix(&effect);
        SetTransMatrix(&effect);
        wmap_draw_model_default(g_wmap_effect_model_pack_2, 0, 0x24, 0xB7, 0x7A40, 1, g_wmap_effect_wave_phase);
        WMAP_MODEL_FADE_OUT(g_wmap_effect_wave_phase, 2, intensity);
        PopMatrix();
        g_wmap_effect_model_a_rotation.vz = (g_wmap_effect_model_a_rotation.vz + 0x12C);
    }

    remaining = g_wmap_land_effect_18_sequence_1_timer - 1;
    g_wmap_land_effect_18_sequence_1_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_1_step++;
    }
}

/** @brief Draw and brighten two rotating layers, then advance their shared countdown. */
void wmap_land_effect_18_sequence_2_step_02(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_e_rotation);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 0, 4, 183, 0x7A80, 0, g_wmap_effect_fade_f);
    g_wmap_effect_model_e_rotation.vz = (g_wmap_effect_model_e_rotation.vz + 32);
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_f_rotation);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 0, 4, 183, 0x7A80, 0, g_wmap_effect_fade_f);
    g_wmap_effect_model_f_rotation.vz = (g_wmap_effect_model_f_rotation.vz + 16);
    PopMatrix();
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_f, 2, 128, intensity);
    remaining = g_wmap_land_effect_18_sequence_2_timer - 1;
    g_wmap_land_effect_18_sequence_2_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_2_step++;
    }
}

/** @brief Draw two rotating effect layers and advance their shared countdown. */
void wmap_land_effect_18_sequence_2_step_04(void)
{
    s32 remaining;
    s32 frame;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_e_rotation);
    frame = g_wmap_effect_model_frame + 1;
    g_wmap_effect_model_frame = frame;

    if (frame >= 8)
    {
        g_wmap_effect_model_frame = 7;
    }

    if (g_wmap_effect_fade_f >= 5)
    {
        g_wmap_effect_fade_f -= 4;
    }

    wmap_draw_model_default(g_wmap_effect_model_pack_1, g_wmap_effect_model_frame, 4, 183, 0x7A80, 0, g_wmap_effect_fade_f);
    g_wmap_effect_model_e_rotation.vz = (g_wmap_effect_model_e_rotation.vz + 32);
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_f_rotation);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, g_wmap_effect_model_frame, 4, 183, 0x7A80, 0, g_wmap_effect_fade_f);
    g_wmap_effect_model_f_rotation.vz = (g_wmap_effect_model_f_rotation.vz + 16);
    PopMatrix();
    remaining = g_wmap_land_effect_18_sequence_2_timer - 1;
    g_wmap_land_effect_18_sequence_2_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_2_step++;
    }
}

/** @brief Draw two rotating effect layers and advance their shared countdown. */
void wmap_land_effect_18_sequence_2_step_06(void)
{
    s32 remaining;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_e_rotation);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 7, 4, 183, 0x7A80, 0, g_wmap_effect_fade_f);
    g_wmap_effect_model_e_rotation.vz = (g_wmap_effect_model_e_rotation.vz + 32);
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_f_rotation);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 7, 4, 183, 0x7A80, 0, g_wmap_effect_fade_f);
    g_wmap_effect_model_f_rotation.vz = (g_wmap_effect_model_f_rotation.vz + 16);
    PopMatrix();
    remaining = g_wmap_land_effect_18_sequence_2_timer - 1;
    g_wmap_land_effect_18_sequence_2_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_2_step++;
    }
}

/** @brief Draw and fade two rotating layers, then advance their shared countdown. */
void wmap_land_effect_18_sequence_2_step_08(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_e_rotation);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 7, 4, 183, 0x7A80, 0, g_wmap_effect_fade_f);
    g_wmap_effect_model_e_rotation.vz = (g_wmap_effect_model_e_rotation.vz + 32);
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_f_rotation);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 7, 4, 183, 0x7A80, 0, g_wmap_effect_fade_f);
    g_wmap_effect_model_f_rotation.vz = (g_wmap_effect_model_f_rotation.vz + 16);
    PopMatrix();
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_f, 2, intensity);
    remaining = g_wmap_land_effect_18_sequence_2_timer - 1;
    g_wmap_land_effect_18_sequence_2_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_2_step++;
    }
}

/** @brief Draw the effect at successive depths and advance its countdown. */
void wmap_land_effect_18_sequence_3_step_02(void)
{
    MATRIX base;
    MATRIX effect;
    s32 remaining;

    switch (g_wmap_land_effect_18_depth_stage)

    {
        case 0:
            g_wmap_effect_model_c_position.vz = 8000;
            break;
        case 1:
            g_wmap_effect_model_c_position.vz = 18000;
            break;
        case 2:
            g_wmap_effect_model_c_position.vz = 10000;
            break;
    }

    g_wmap_land_effect_18_depth_stage++;
    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base);
    TransMatrix(&base, &g_wmap_effect_model_c_position);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&g_wmap_effect_model_c_rotation, &effect);
    TransMatrix(&effect, &g_wmap_zero_translation);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    wmap_draw_model_default(g_wmap_effect_model_pack_2 + WMAP_EFFECT_18_FLASH2_OFFSET, 0, 4, -1, -1, 1, -1);
    PopMatrix();
    remaining = g_wmap_land_effect_18_sequence_3_timer - 1;
    g_wmap_land_effect_18_sequence_3_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_3_step++;
    }
}

/** @brief Compose the effect transform, draw it at alternating depths, and advance its countdown. */
void wmap_land_effect_18_sequence_3_step_04(void)
{
    MATRIX base;
    MATRIX effect;
    s32 remaining;

    if (g_wmap_frame_count & 1)
    {
        g_wmap_effect_model_c_position.vz = 28000;
    }
    else
    {
        g_wmap_effect_model_c_position.vz = 18000;
    }

    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base);
    TransMatrix(&base, &g_wmap_effect_model_c_position);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&g_wmap_effect_model_c_rotation, &effect);
    TransMatrix(&effect, &g_wmap_zero_translation);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    wmap_draw_model_default(g_wmap_effect_model_pack_2 + WMAP_EFFECT_18_FLASH2_OFFSET, 0, 4, -1, -1, 1, -1);
    PopMatrix();
    remaining = g_wmap_land_effect_18_sequence_3_timer - 1;
    g_wmap_land_effect_18_sequence_3_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_3_step++;
    }
}

/** @brief Compose the effect transform, draw it at alternating depths, and advance its countdown. */
void wmap_land_effect_18_sequence_3_step_06(void)
{
    MATRIX base;
    MATRIX effect;
    s32 remaining;

    if (g_wmap_frame_count & 1)
    {
        g_wmap_effect_model_c_position.vz = 28000;
    }
    else
    {
        g_wmap_effect_model_c_position.vz = 18000;
    }

    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base);
    TransMatrix(&base, &g_wmap_effect_model_c_position);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&g_wmap_effect_model_c_rotation, &effect);
    TransMatrix(&effect, &g_wmap_zero_translation);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    wmap_draw_model_default(g_wmap_effect_model_pack_2 + WMAP_EFFECT_18_FLASH2_OFFSET, 0, 4, -1, -1, 1, g_wmap_land_effect_18_flash_2_fade);
    g_wmap_land_effect_18_flash_2_fade--;
    PopMatrix();
    remaining = g_wmap_land_effect_18_sequence_3_timer - 1;
    g_wmap_land_effect_18_sequence_3_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_3_step++;
    }
}

/** @brief Advance the effect depth, fade its intensity, and update the countdown. */
void wmap_land_effect_18_sequence_4_step_02(void)
{
    MATRIX base;
    MATRIX effect;
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = g_wmap_effect_model_b_position.vz - 0x960;
    g_wmap_effect_model_b_position.vz = depth;

    if (depth < 0x3E8)
    {
        g_wmap_effect_model_b_position.vz = g_wmap_camera_translation.vz;
        g_wmap_effect_flash_fade_b = g_wmap_effect_flash_fade_a;
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

    if (g_wmap_effect_flash_fade_b != 0)
    {
        wmap_draw_model_default(g_wmap_effect_model_pack_2 + WMAP_EFFECT_18_FLASH_C_OFFSET, 0, 4, -1, -1, 1, g_wmap_effect_flash_fade_b);
    }

    PopMatrix();
    WMAP_MODEL_FADE_OUT(g_wmap_effect_flash_fade_b, 9, intensity);
    remaining = g_wmap_land_effect_18_sequence_4_timer - 1;
    g_wmap_land_effect_18_sequence_4_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_4_step++;
    }
}

/** @brief Advance the effect depth, fade its intensity, and update the countdown. */
void wmap_land_effect_18_sequence_4_step_04(void)
{
    MATRIX base;
    MATRIX effect;
    s32 depth;
    s32 intensity;
    s32 remaining;
    s32 peak;

    depth = g_wmap_effect_model_b_position.vz - 0x960;
    g_wmap_effect_model_b_position.vz = depth;

    if (depth < 0x3E8)
    {
        g_wmap_effect_model_b_position.vz = g_wmap_camera_translation.vz;
        g_wmap_effect_flash_fade_b = g_wmap_effect_flash_fade_a;
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

    if (g_wmap_effect_flash_fade_b != 0)
    {
        wmap_draw_model_default(g_wmap_effect_model_pack_2 + WMAP_EFFECT_18_FLASH_C_OFFSET, 0, 4, -1, -1, 1, g_wmap_effect_flash_fade_b);
    }

    PopMatrix();
    peak = g_wmap_effect_flash_fade_a - 1;
    intensity = g_wmap_effect_flash_fade_b - 9;
    g_wmap_effect_flash_fade_a = peak;
    g_wmap_effect_flash_fade_b = intensity;

    if (intensity < 0)
    {
        g_wmap_effect_flash_fade_b = 0;
    }

    if (peak < 0)
    {
        g_wmap_effect_flash_fade_a = 0;
    }

    remaining = g_wmap_land_effect_18_sequence_4_timer - 1;
    g_wmap_land_effect_18_sequence_4_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_4_step++;
    }
}

/** @brief Draw and respawn the radial sparks for land effect 18. */
WMAP_DEFINE_RADIAL_SPARK_UPDATE(wmap_land_effect_18_update_sparks, 30, /* Motion slots. */
                                6,                                     /* First sprite actor. */
                                9,                                     /* Texture index. */
                                31,                                    /* Draw order. */
                                75,                                    /* Minimum lifetime in frames. */
                                g_wmap_particle_shade)

/** @brief Initialize and project the map effect before its first draw. */
void wmap_land_effect_18_sequence_6_step_01(void)
{
    SVECTOR position;

    g_wmap_actor_animations[4].data = g_wmap_animation_bank_0;
    g_wmap_sprite_actors[4].scale_index = WMAP_SPRITE_SCALE_FULL;
    g_wmap_sprite_actors[4].previous_sequence = -1;
    g_wmap_sprite_actors[4].resource_index = 0;
    g_wmap_sprite_actors[4].sequence = 0;
    g_wmap_sprite_actors[4].target_shade = WMAP_SPRITE_SHADE_NEUTRAL;
    g_wmap_sprite_actors[4].shade = WMAP_SPRITE_SHADE_NEUTRAL;
    position.vz = 0;
    position.vx = (((g_wmap_focus_cell_x - 1) * WMAP_CELL_SIZE - g_wmap_view.x * WMAP_MAP_PROJECTION_SCALE / g_wmap_view.projection_scale) * WMAP_VIEW_SCALE) /
                  g_wmap_view.projection_scale;
    position.vy = (((g_wmap_focus_cell_y - 1) * WMAP_CELL_SIZE - g_wmap_view.y * WMAP_MAP_PROJECTION_SCALE / g_wmap_view.projection_scale) * WMAP_VIEW_SCALE) /
                  g_wmap_view.projection_scale;
    gte_ldv0(&position);
    gte_rtps();
    gte_stsxy(&g_wmap_focus_screen_position);
    g_wmap_effect_fade_c = 128;
    g_wmap_land_effect_18_sequence_6_timer = 64;
    g_wmap_land_effect_18_sequence_6_step++;
    wmap_land_effect_18_sequence_6_step_02();
}

/** @brief Configure effect parameters and reset its animation resource slots. */
void wmap_land_effect_18_sequence_9_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 16;
    WMAP_SET_PARTICLE_SETTINGS(3, 1, 56, 0, 1, 5000, 60, 17, 1, 0);
    g_wmap_effect_fade_b = 127;
    WMAP_RESET_PARTICLE_SLOTS(i, 16, g_wmap_actor_motions[i + g_wmap_particle_first_motion].state, 104, g_wmap_animation_bank_1);
    g_wmap_land_effect_18_sequence_9_timer = 32;
    g_wmap_land_effect_18_sequence_9_step++;
    wmap_land_effect_18_sequence_9_step_02();
}

/** @brief Configure effect parameters and reset its animation resource slots. */
void wmap_land_effect_18_sequence_10_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 40;
    WMAP_SET_PARTICLE_SETTINGS(6, 2, 32, 2, 1, 5000, 120, 17, 2, 0);
    g_wmap_effect_fade_d = 127;
    WMAP_RESET_PARTICLE_SLOTS(i, 40, g_wmap_actor_motions[i + g_wmap_particle_first_motion].state, 204, g_wmap_animation_bank_1);
    g_wmap_land_effect_18_sequence_10_timer = 48;
    g_wmap_land_effect_18_sequence_10_step++;
    wmap_land_effect_18_sequence_10_step_02();
}

/** @brief Compose the effect transform, draw and brighten it, and advance its countdown. */
void wmap_land_effect_18_sequence_11_step_02(void)
{
    MATRIX base;
    MATRIX effect;
    s32 remaining;
    s32 intensity;

    /* The model holds still: the Z movement of the other drop steps, with a step of zero. */
    g_wmap_effect_model_d_position.vz += 0;
    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base);
    TransMatrix(&base, &g_wmap_effect_model_d_position);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&g_wmap_effect_model_d_rotation, &effect);
    TransMatrix(&effect, &g_wmap_zero_translation);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);

    if (g_wmap_effect_fade_e != 0)
    {
        wmap_draw_model_default(g_wmap_effect_model_pack_2 + WMAP_EFFECT_18_WST_FLA_OFFSET, 0, 4, -1, -1, 1, g_wmap_effect_fade_e);
    }

    PopMatrix();
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_e, 10, 255, intensity);
    remaining = g_wmap_land_effect_18_sequence_11_timer - 1;
    g_wmap_land_effect_18_sequence_11_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_11_step++;
    }
}

/** @brief Compose the effect transform, draw and fade it, and advance its countdown. */
void wmap_land_effect_18_sequence_11_step_04(void)
{
    MATRIX base;
    MATRIX effect;
    s32 remaining;
    s32 intensity;

    /* The model holds still: the Z movement of the other drop steps, with a step of zero. */
    g_wmap_effect_model_d_position.vz += 0;
    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base);
    TransMatrix(&base, &g_wmap_effect_model_d_position);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&g_wmap_effect_model_d_rotation, &effect);
    TransMatrix(&effect, &g_wmap_zero_translation);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);

    if (g_wmap_effect_fade_e != 0)
    {
        wmap_draw_model_default(g_wmap_effect_model_pack_2 + WMAP_EFFECT_18_WST_FLA_OFFSET, 0, 4, -1, -1, 1, g_wmap_effect_fade_e);
    }

    PopMatrix();
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_e, 10, intensity);
    remaining = g_wmap_land_effect_18_sequence_11_timer - 1;
    g_wmap_land_effect_18_sequence_11_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_11_step++;
    }
}

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_1, g_wmap_land_effect_18_sequence_1_steps, 0x6, g_wmap_land_effect_18_sequence_1_step,
                               g_wmap_land_effect_18_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_1_reset, g_wmap_land_effect_18_sequence_1_step, g_wmap_land_effect_18_sequence_1_timer)

/** @brief Restore the effect state and begin a one-tick sequence step. */
void wmap_land_effect_18_sequence_1_step_01(void)
{
    g_wmap_effect_wave_phase = 128;
    g_wmap_effect_model_a_rotation = g_wmap_zero_rotation;
    g_wmap_land_effect_18_sequence_1_timer = 1;
    g_wmap_land_effect_18_sequence_1_step++;
    wmap_land_effect_18_sequence_1_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_1_step_03, g_wmap_land_effect_18_sequence_1_step, g_wmap_land_effect_18_sequence_1_timer, 0x40,
                    wmap_land_effect_18_sequence_1_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_1_end, g_wmap_land_effect_18_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_18_run_sequence_2, g_wmap_land_effect_18_sequence_2_steps, 0xA, g_wmap_land_effect_18_sequence_2_step,
                 g_wmap_land_effect_18_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_2_reset, g_wmap_land_effect_18_sequence_2_step, g_wmap_land_effect_18_sequence_2_timer)

/** @brief Clear effect state and two vectors, then begin a 60-tick sequence step. */
void wmap_land_effect_18_sequence_2_step_01(void)
{
    g_wmap_effect_fade_f = 0;
    g_wmap_effect_model_frame = 0;
    g_wmap_effect_model_e_rotation.vx = 0;
    g_wmap_effect_model_e_rotation.vy = 0;
    g_wmap_effect_model_e_rotation.vz = 0;
    g_wmap_effect_model_f_rotation.vx = 0;
    g_wmap_effect_model_f_rotation.vy = 0;
    g_wmap_effect_model_f_rotation.vz = 0;
    g_wmap_land_effect_18_sequence_2_timer = 0x3C;
    g_wmap_land_effect_18_sequence_2_step++;
    wmap_land_effect_18_sequence_2_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_2_step_03, g_wmap_land_effect_18_sequence_2_step, g_wmap_land_effect_18_sequence_2_timer, 0x9,
                    wmap_land_effect_18_sequence_2_step_04)

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_2_step_05, g_wmap_land_effect_18_sequence_2_step, g_wmap_land_effect_18_sequence_2_timer, 0x30,
                    wmap_land_effect_18_sequence_2_step_06)

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_2_step_07, g_wmap_land_effect_18_sequence_2_step, g_wmap_land_effect_18_sequence_2_timer, 0x40,
                    wmap_land_effect_18_sequence_2_step_08)

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_2_end, g_wmap_land_effect_18_sequence_2_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_3, g_wmap_land_effect_18_sequence_3_steps, 0x8, g_wmap_land_effect_18_sequence_3_step,
                               g_wmap_land_effect_18_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_3_reset, g_wmap_land_effect_18_sequence_3_step, g_wmap_land_effect_18_sequence_3_timer)

/** @brief Restore the default transform and advance the sequence. */
void wmap_land_effect_18_sequence_3_step_01(void)
{
    g_wmap_effect_model_c_rotation = g_wmap_zero_rotation;
    g_wmap_effect_model_c_position = g_wmap_camera_translation;
    g_wmap_land_effect_18_depth_stage = 0;
    g_wmap_land_effect_18_flash_2_fade = 0x80;
    g_wmap_land_effect_18_sequence_3_timer = 4;
    g_wmap_land_effect_18_sequence_3_step++;
    wmap_land_effect_18_sequence_3_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_3_step_03, g_wmap_land_effect_18_sequence_3_step, g_wmap_land_effect_18_sequence_3_timer, 0x20,
                    wmap_land_effect_18_sequence_3_step_04)

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_3_step_05, g_wmap_land_effect_18_sequence_3_step, g_wmap_land_effect_18_sequence_3_timer, 0x80,
                    wmap_land_effect_18_sequence_3_step_06)

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_3_end, g_wmap_land_effect_18_sequence_3_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_4, g_wmap_land_effect_18_sequence_4_steps, 0x6, g_wmap_land_effect_18_sequence_4_step,
                               g_wmap_land_effect_18_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_4_reset, g_wmap_land_effect_18_sequence_4_step, g_wmap_land_effect_18_sequence_4_timer)

/** @brief Restore the default transform and advance the sequence. */
void wmap_land_effect_18_sequence_4_step_01(void)
{
    g_wmap_effect_model_b_rotation = g_wmap_zero_rotation;
    g_wmap_effect_model_b_position = g_wmap_camera_translation;
    g_wmap_effect_flash_fade_a = 0x80;
    g_wmap_land_effect_18_sequence_4_timer = 1;
    g_wmap_land_effect_18_sequence_4_step++;
    wmap_land_effect_18_sequence_4_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_4_step_03, g_wmap_land_effect_18_sequence_4_step, g_wmap_land_effect_18_sequence_4_timer, 0x20,
                    wmap_land_effect_18_sequence_4_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_4_end, g_wmap_land_effect_18_sequence_4_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_5, g_wmap_land_effect_18_sequence_5_steps, 0x6, g_wmap_land_effect_18_sequence_5_step,
                               g_wmap_land_effect_18_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_5_reset, g_wmap_land_effect_18_sequence_5_step, g_wmap_land_effect_18_sequence_5_timer)

/**
 * @brief Initialize the particle slots and run their first update.
 */
void wmap_land_effect_18_sequence_5_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 0;
    g_wmap_particle_shade = 0x80;
    WMAP_RESET_PARTICLE_SLOTS(i, 30, g_wmap_actor_motions[i].state, 6, g_wmap_animation_bank_1);
    g_wmap_land_effect_18_sequence_5_timer = 0xC;
    g_wmap_land_effect_18_sequence_5_step++;
    wmap_land_effect_18_sequence_5_step_02();
}

/** @brief World-map step tick: bump a counter, run the sub-step, and expire the timer. */
void wmap_land_effect_18_sequence_5_step_02(void)
{
    if (g_wmap_frame_count & 1)
    {
        g_wmap_particle_intensity++;
    }

    wmap_land_effect_18_update_sparks();

    if (--g_wmap_land_effect_18_sequence_5_timer == 0)
    {
        g_wmap_land_effect_18_sequence_5_step++;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_5_step_03, g_wmap_land_effect_18_sequence_5_step, g_wmap_land_effect_18_sequence_5_timer, 0x40,
                    wmap_land_effect_18_sequence_5_step_04)

/**
 * @brief World-map step tick: age two timers with zero clamps, run the sub-step,
 *        and expire the step counter.
 */
void wmap_land_effect_18_sequence_5_step_04(void)
{
    g_wmap_particle_shade--;

    if (g_wmap_particle_shade < 0)
    {
        g_wmap_particle_shade = 0;
    }

    if ((g_wmap_frame_count & 3) == 0)
    {
        g_wmap_particle_intensity--;
    }

    if (g_wmap_particle_intensity < 0)
    {
        g_wmap_particle_intensity = 0;
    }

    wmap_land_effect_18_update_sparks();

    if (--g_wmap_land_effect_18_sequence_5_timer == 0)
    {
        g_wmap_land_effect_18_sequence_5_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_5_end, g_wmap_land_effect_18_sequence_5_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_6, g_wmap_land_effect_18_sequence_6_steps, 0x6, g_wmap_land_effect_18_sequence_6_step,
                               g_wmap_land_effect_18_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_6_reset, g_wmap_land_effect_18_sequence_6_step, g_wmap_land_effect_18_sequence_6_timer)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_18_sequence_6_step_02, g_wmap_land_effect_18_sequence_6_step, g_wmap_land_effect_18_sequence_6_timer,
                              g_wmap_sprite_actors[4], g_wmap_actor_animations[4], g_wmap_focus_screen_position, 8, 0x2E, 0)

/** @brief Start fading out sprite actor 4 slowly, then run step 4. */
void wmap_land_effect_18_sequence_6_step_03(void)
{
    g_wmap_sprite_actors[4].target_shade = 0;
    g_wmap_sprite_actors[4].shade_step = 2;
    g_wmap_land_effect_18_sequence_6_timer = 0x5D;
    g_wmap_land_effect_18_sequence_6_step++;
    wmap_land_effect_18_sequence_6_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_18_sequence_6_step_04, g_wmap_land_effect_18_sequence_6_step, g_wmap_land_effect_18_sequence_6_timer,
                              g_wmap_sprite_actors[4], g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0x8, 0x2E, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_6_end, g_wmap_land_effect_18_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_18_run_sequence_7, g_wmap_land_effect_18_sequence_7_steps, 0x4, g_wmap_land_effect_18_sequence_7_step,
                 g_wmap_land_effect_18_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_7_reset, g_wmap_land_effect_18_sequence_7_step, g_wmap_land_effect_18_sequence_7_timer)

/** @brief World-map step handler: init a UI descriptor block, set the timer, advance the step. */
void wmap_land_effect_18_sequence_7_step_01(void)
{
    WmapSpriteActor* actors = g_wmap_sprite_actors;

    g_wmap_actor_animations[5].data = g_wmap_animation_bank_0;
    actors[5].scale_index = WMAP_SPRITE_SCALE_FULL;
    actors[5].sequence = 1;
    actors[5].previous_sequence = -1;
    actors[5].resource_index = 0;
    actors[5].target_shade = WMAP_SPRITE_SHADE_NEUTRAL;
    actors[5].shade = WMAP_SPRITE_SHADE_NEUTRAL;
    g_wmap_land_effect_18_sequence_7_timer = 0xC7;
    g_wmap_land_effect_18_sequence_7_step++;
    wmap_land_effect_18_sequence_7_step_02();
}

/** @brief Draw the projected effect and advance after its countdown. */
void wmap_land_effect_18_sequence_7_step_02(void)
{
    s32 screen_position;
    s32 remaining;
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    wmap_project_focus_cell();
    gte_stsxy(&screen_position);
    wmap_step_actor_animation(actor, &g_wmap_actor_animations[5]);
    wmap_draw_actor_sprite(actor, screen_position, 8, 0x2E, 0);
    remaining = g_wmap_land_effect_18_sequence_7_timer - 1;
    g_wmap_land_effect_18_sequence_7_timer = remaining;

    if (remaining == 0)
    {
        g_wmap_land_effect_18_sequence_7_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_7_end, g_wmap_land_effect_18_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_18_run_timeline, g_wmap_land_effect_18_timeline_steps, 0x16, g_wmap_land_effect_18_timeline_step,
                 g_wmap_land_effect_18_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_18_timeline_reset, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

/** @brief Set world-map flags and color, then begin an eight-tick delay. */
void wmap_land_effect_18_timeline_step_01(void)
{
    g_wmap_transition_mesh_hidden = 1;
    g_wmap_backdrop_target_level = 8;
    wmap_start_map_tint(0x262726);
    g_wmap_land_effect_18_timeline_timer = 8;
    g_wmap_land_effect_18_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_02, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

/** @brief Play sound 18, register two callbacks, and begin a seven-tick delay. */
void wmap_land_effect_18_timeline_step_03(void)
{
    wmap_play_sound(0x12, 0x80);
    wmap_start_sequence(&wmap_land_effect_18_run_sequence_11);
    g_wmap_placement_overlay_hidden = 1;
    wmap_start_sequence(&wmap_land_effect_18_run_sequence_6);
    g_wmap_land_effect_18_timeline_timer = 7;
    g_wmap_land_effect_18_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_04, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_18_timeline_step_05, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer,
                         wmap_land_effect_18_run_sequence_5, 0x34)

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_06, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_18_timeline_step_07, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer,
                         wmap_land_effect_18_run_sequence_2, 0x3A)

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_08, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

/**
 * @brief Start three sequences and set the wait timer.
 */
WMAP_STEP_START_THREE_AND_WAIT(wmap_land_effect_18_timeline_step_09, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer,
                               wmap_land_effect_18_run_sequence_3, wmap_land_effect_18_run_sequence_1, wmap_land_effect_18_run_sequence_10, 0x10)

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_10, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_18_timeline_step_11, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer,
                             wmap_land_effect_18_run_sequence_4, wmap_land_effect_18_run_sequence_8, 0x18)

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_12, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_18_timeline_step_13, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer,
                         wmap_land_effect_18_run_sequence_7, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_14, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

/** @brief World-map step: set fade colour then advance to the next handler. */
void wmap_land_effect_18_timeline_step_15(void)
{
    wmap_start_map_tint(0x808080);
    g_wmap_land_effect_18_timeline_timer = 0x54;
    g_wmap_land_effect_18_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_16, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

/** @brief World-map step handler: seed timers and advance the counter. */
void wmap_land_effect_18_timeline_step_17(void)
{
    g_wmap_transition_mesh_hidden = 0;
    g_wmap_backdrop_target_level = 0x10;
    g_wmap_land_effect_18_timeline_timer = 0x28;
    g_wmap_land_effect_18_timeline_step++;
}

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_18, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_18_timeline_step_19, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer,
                         wmap_land_effect_18_run_sequence_9, 0x46)

WMAP_STEP_WAIT(wmap_land_effect_18_timeline_wait_20, g_wmap_land_effect_18_timeline_step, g_wmap_land_effect_18_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_18_timeline_finish, g_wmap_land_effect_18_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y,
                           g_wmap_selected_artifact)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_8, g_wmap_land_effect_18_sequence_8_steps, 0x6, g_wmap_land_effect_18_sequence_8_step,
                               g_wmap_land_effect_18_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_8_reset, g_wmap_land_effect_18_sequence_8_step, g_wmap_land_effect_18_sequence_8_timer)

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_18_sequence_8_step_01(void)
{
    g_wmap_land_effect_18_camera_spin_speed = 0;
    g_wmap_land_effect_18_sequence_8_timer = 0x3D;
    g_wmap_land_effect_18_sequence_8_step++;
    wmap_land_effect_18_sequence_8_step_02();
}

/** @brief Adjust the Z angle and advance when the countdown reaches zero. */
void wmap_land_effect_18_sequence_8_step_02(void)
{
    s32 remaining_ticks;
    g_wmap_effect_camera_rotation_offset.vz = (g_wmap_effect_camera_rotation_offset.vz + g_wmap_land_effect_18_camera_spin_speed);
    remaining_ticks = g_wmap_land_effect_18_sequence_8_timer - 1;
    g_wmap_land_effect_18_sequence_8_timer = remaining_ticks;

    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_18_sequence_8_step++;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_8_step_03, g_wmap_land_effect_18_sequence_8_step, g_wmap_land_effect_18_sequence_8_timer, 0x80,
                    wmap_land_effect_18_sequence_8_step_04)

/** @brief Reduce angular speed toward zero, update the Z angle, and count down the step. */
void wmap_land_effect_18_sequence_8_step_04(void)
{
    s32 speed;
    s32 remaining_ticks;
    speed = g_wmap_land_effect_18_camera_spin_speed - 5;
    g_wmap_land_effect_18_camera_spin_speed = speed;

    if (speed < 0)
    {
        g_wmap_land_effect_18_camera_spin_speed = 0;
    }

    g_wmap_effect_camera_rotation_offset.vz = (g_wmap_effect_camera_rotation_offset.vz + g_wmap_land_effect_18_camera_spin_speed);
    remaining_ticks = g_wmap_land_effect_18_sequence_8_timer - 1;
    g_wmap_land_effect_18_sequence_8_timer = remaining_ticks;

    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_18_sequence_8_step++;
    }
}

/**
 * @brief Clear the sequence flag and advance the step counter.
 */
void wmap_land_effect_18_sequence_8_step_05(void)
{
    g_wmap_effect_camera_rotation_offset.vz = 0;
    g_wmap_land_effect_18_sequence_8_step++;
}

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_9, g_wmap_land_effect_18_sequence_9_steps, 0x4, g_wmap_land_effect_18_sequence_9_step,
                               g_wmap_land_effect_18_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_9_reset, g_wmap_land_effect_18_sequence_9_step, g_wmap_land_effect_18_sequence_9_timer)

/** @brief World-map step: draw the active overlay while its timer runs, then tick down. */
void wmap_land_effect_18_sequence_9_step_02(void)
{
    s32 c;

    if (g_wmap_effect_fade_b > 0)
    {
        wmap_update_default_particles_from_settings(&g_wmap_sprite_actors[104], &g_wmap_actor_animations[104], 0x10, 0, g_wmap_effect_fade_b, 3);
    }

    g_wmap_effect_fade_b -= 4;
    c = g_wmap_land_effect_18_sequence_9_timer - 1;
    g_wmap_land_effect_18_sequence_9_timer = c;

    if (c == 0)
    {
        g_wmap_land_effect_18_sequence_9_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_9_end, g_wmap_land_effect_18_sequence_9_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_10, g_wmap_land_effect_18_sequence_10_steps, 0x4, g_wmap_land_effect_18_sequence_10_step,
                               g_wmap_land_effect_18_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_10_reset, g_wmap_land_effect_18_sequence_10_step, g_wmap_land_effect_18_sequence_10_timer)

/** @brief World-map step: draw the active overlay while its timer runs, then tick down. */
void wmap_land_effect_18_sequence_10_step_02(void)
{
    s32 c;

    if (g_wmap_effect_fade_d > 0)
    {
        wmap_update_default_particles_from_settings(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 0x28, 0, g_wmap_effect_fade_d, 3);
    }

    g_wmap_effect_fade_d -= 4;
    c = g_wmap_land_effect_18_sequence_10_timer - 1;
    g_wmap_land_effect_18_sequence_10_timer = c;

    if (c == 0)
    {
        g_wmap_land_effect_18_sequence_10_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_10_end, g_wmap_land_effect_18_sequence_10_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_18_run_sequence_11, g_wmap_land_effect_18_sequence_11_steps, 0x6, g_wmap_land_effect_18_sequence_11_step,
                               g_wmap_land_effect_18_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_18_sequence_11_reset, g_wmap_land_effect_18_sequence_11_step, g_wmap_land_effect_18_sequence_11_timer)

WMAP_STEP_DROP_START(wmap_land_effect_18_sequence_11_step_01, g_wmap_land_effect_18_sequence_11_step, g_wmap_land_effect_18_sequence_11_timer,
                     g_wmap_effect_model_d_rotation, g_wmap_zero_rotation, g_wmap_effect_model_d_position, g_wmap_effect_fade_e, 0, 0, 12,
                     wmap_land_effect_18_sequence_11_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_18_sequence_11_step_03, g_wmap_land_effect_18_sequence_11_step, g_wmap_land_effect_18_sequence_11_timer, 0x28,
                    wmap_land_effect_18_sequence_11_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_18_sequence_11_step_05, g_wmap_land_effect_18_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_effect_18_run, g_wmap_land_effect_18_steps, 0x6, g_wmap_land_effect_18_step, g_wmap_land_effect_18_timer)

WMAP_STEP_RESET(wmap_land_effect_18_reset, g_wmap_land_effect_18_step, g_wmap_land_effect_18_timer)

/**
 * @brief Register a world-map callback and advance to the next step.
 */
void wmap_land_effect_18_step_01(void)
{
    g_wmap_input_locked = 1;
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_18_step++;
    wmap_land_effect_18_wait_idle_02();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_18_wait_idle_02, g_wmap_land_effect_18_step, wmap_land_effect_18_step_03)

/** @brief World-map step: register the next draw callback and advance to the next handler. */
void wmap_land_effect_18_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_18_run_timeline);
    g_wmap_land_effect_18_step++;
    wmap_land_effect_18_wait_idle_04();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_18_wait_idle_04, g_wmap_land_effect_18_step, wmap_land_effect_18_end)

WMAP_STEP_ADVANCE(wmap_land_effect_18_end, g_wmap_land_effect_18_step)
