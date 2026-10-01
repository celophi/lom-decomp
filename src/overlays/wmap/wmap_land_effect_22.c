#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_land_effect_22.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"
#include "wmap_step_sequence.h"
#include "wmap_cells.h"

void wmap_land_effect_22_sequence_12_step_02(void);
void wmap_land_effect_22_sequence_13_step_02(void);
void wmap_land_effect_22_sequence_14_step_02(void);
s32 wmap_land_effect_22_run_timeline(s32 arg0);
void wmap_land_effect_22_wait_idle(void);
void wmap_land_effect_22_step_03(void);
s32 wmap_land_effect_22_run_sequence_13(s32 arg0);
s32 wmap_land_effect_22_run_sequence_6(s32 arg0);
s32 wmap_land_effect_22_run_sequence_12(s32 arg0);
s32 wmap_land_effect_22_run_sequence_2(s32 arg0);
s32 wmap_land_effect_22_run_sequence_10(s32 arg0);
s32 wmap_land_effect_22_run_sequence_7(s32 arg0);
s32 wmap_land_effect_22_run_sequence_1(s32 arg0);
s32 wmap_land_effect_22_run_sequence_9(s32 arg0);
s32 wmap_land_effect_22_run_sequence_11(s32 arg0);
s32 wmap_land_effect_22_run_sequence_3(s32 arg0);
s32 wmap_land_effect_22_run_sequence_14(s32 arg0);
s32 wmap_land_effect_22_run_sequence_4(s32 arg0);
s32 wmap_land_effect_22_run_sequence_5(s32 arg0);
s32 wmap_land_effect_22_run_sequence_8(s32 arg0);
void wmap_land_effect_22_sequence_1_step_02(void);
void wmap_land_effect_22_sequence_5_step_02(void);
void wmap_land_effect_22_sequence_7_step_02(void);
void wmap_land_effect_22_sequence_8_step_02(void);
void wmap_land_effect_22_sequence_12_step_04(void);
void wmap_land_effect_22_sequence_13_step_04(void);
void wmap_land_effect_22_sequence_14_step_04(void);

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

extern u8* g_wmap_effect_model_pack_1;
extern s32 g_wmap_land_effect_22_sequence_3_timer;
extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_22_sequence_2_timer;
extern s32 g_wmap_land_effect_22_sequence_4_timer;
extern int rand(void);
extern s16 D_801398C8[];
extern s32 D_80182D48[];
extern s32 D_80139268;
extern s32 g_wmap_land_effect_22_sequence_6_timer;
extern s32* D_8011CF2C;
extern s32 D_8013923C;
extern s32 D_80182DE4;
extern s32 g_wmap_land_effect_22_sequence_9_timer;
extern s32 D_80139264;
extern s32 g_wmap_land_effect_22_sequence_10_timer;
extern s32 D_801B25D8;
extern s32 D_80139234;
extern s32 D_80139260;
extern s32 g_wmap_land_effect_22_sequence_11_timer;
extern u8 g_wmap_animation_bank_0[];
extern s32 g_wmap_land_effect_22_sequence_12_timer;
extern s32 g_wmap_land_effect_22_sequence_13_timer;
extern s32 g_wmap_land_effect_22_sequence_14_timer;
extern s32 g_wmap_land_effect_22_timer;
extern void (*D_800D6904[])(void);
extern s32 D_80139978;
extern s32 D_8013B294;
extern s32 D_80139228;
extern s32 g_wmap_land_effect_22_timeline_timer;
extern void (*D_800D6914[])(void);
extern s32 D_800DBE70;
extern s32 g_wmap_land_effect_22_sequence_1_timer;
extern void (*D_800D696C[])(void);
extern u8 g_wmap_animation_bank_1[];
extern void (*D_800D697C[])(void);
extern void (*D_800D698C[])(void);
extern void (*D_800D699C[])(void);
extern s32 g_wmap_land_effect_22_sequence_5_timer;
extern void (*D_800D69AC[])(void);
extern u8 g_wmap_animation_bank_2[];
extern void wmap_land_effect_22_sequence_5_step_02(void);
extern void (*D_800D69BC[])(void);
extern s32 g_wmap_land_effect_22_sequence_7_timer;
extern void (*D_800D69D4[])(void);
extern u8 g_wmap_animation_bank_3[];
extern s32 g_wmap_land_effect_22_sequence_8_timer;
extern void (*D_800D69E4[])(void);
extern void (*D_800D69F4[])(void);
extern void (*D_800D6A0C[])(void);
extern void (*D_800D6A24[])(void);
extern void (*D_800D6A3C[])(void);
extern void (*D_800D6A54[])(void);
extern void wmap_land_effect_22_sequence_13_step_04(void);
extern void (*D_800D6A6C[])(void);
extern void wmap_land_effect_22_sequence_14_step_04(void);

extern u32 g_wmap_land_effect_22_sequence_3_step;
extern u32 g_wmap_land_effect_22_sequence_2_step;
extern u32 g_wmap_land_effect_22_sequence_4_step;
extern u32 g_wmap_land_effect_22_sequence_6_step;
extern u32 g_wmap_land_effect_22_sequence_9_step;
extern void *g_wmap_effect_model_pack_3;
extern u32 g_wmap_land_effect_22_sequence_10_step;
extern u32 g_wmap_land_effect_22_sequence_11_step;
extern u32 g_wmap_land_effect_22_sequence_12_step;
extern u32 g_wmap_land_effect_22_sequence_13_step;
extern u32 g_wmap_land_effect_22_sequence_14_step;
extern u32 g_wmap_land_effect_22_step;
extern u32 g_wmap_land_effect_22_timeline_step;
extern u32 g_wmap_land_effect_22_sequence_1_step;
extern u32 g_wmap_land_effect_22_sequence_5_step;
extern u32 g_wmap_land_effect_22_sequence_7_step;
extern u32 g_wmap_land_effect_22_sequence_8_step;

extern VECTOR g_wmap_camera_translation;

extern SVECTOR D_801B2498;
extern SVECTOR D_801B2670;


extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* g_wmap_effect_params;

extern WmapSlot14 g_wmap_actor_motions[];

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_22_sequence_2_step_02,
    g_wmap_land_effect_22_sequence_2_step, g_wmap_land_effect_22_sequence_2_timer,
    g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position,
    g_wmap_effect_fade_a, g_wmap_load_buffer, -3500, 2)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_22_sequence_3_step_02,
    g_wmap_land_effect_22_sequence_3_step, g_wmap_land_effect_22_sequence_3_timer,
    g_wmap_effect_model_b_rotation, g_wmap_effect_model_b_position,
    g_wmap_effect_fade_b, g_wmap_effect_model_pack_1, -3500, 2)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_22_sequence_4_step_02,
    g_wmap_land_effect_22_sequence_4_step, g_wmap_land_effect_22_sequence_4_timer,
    g_wmap_effect_model_c_rotation, g_wmap_effect_model_c_position,
    g_wmap_effect_fade_c, g_wmap_effect_model_pack_1, -3500, 2)

/**
 * @brief World-map step handler: jitter the actor position with a ramping random
 *        amplitude, then countdown-advance the step.
 */
void wmap_land_effect_22_sequence_6_step_02(void)
{
    D_801398C8[0] = rand() * D_80139268 / 16 >> 15;
    D_801398C8[1] = rand() * D_80139268 / 16 >> 15;
    D_80182D48[0] = rand() * D_80139268 / 16 >> 15;
    D_80182D48[1] = rand() * D_80139268 / 16 >> 15;
    if (D_80139268 < 0x80)
    {
        D_80139268 += 4;
    }
    if (--g_wmap_land_effect_22_sequence_6_timer == 0)
    {
        g_wmap_land_effect_22_sequence_6_step += 1;
    }
}

/**
 * @brief World-map step handler: jitter the actor with a random amplitude, and when
 *        the amplitude ramp expires, zero the offsets; otherwise countdown-advance.
 */
void wmap_land_effect_22_sequence_6_step_04(void)
{
    D_801398C8[0] = rand() * D_80139268 / 16 >> 15;
    D_801398C8[1] = rand() * D_80139268 / 16 >> 15;
    D_80182D48[0] = rand() * D_80139268 / 16 >> 15;
    D_80182D48[1] = rand() * D_80139268 / 16 >> 15;
    if (--D_80139268 < 0)
    {
        D_80182D48[1] = 0;
        D_80182D48[0] = 0;
        D_801398C8[1] = 0;
        D_801398C8[0] = 0;
        g_wmap_land_effect_22_sequence_6_step += 1;
    }
    else if (--g_wmap_land_effect_22_sequence_6_timer == 0)
    {
        g_wmap_land_effect_22_sequence_6_step += 1;
    }
}

/**
 * @brief Draw and spin the model while fading it in.
 */
void wmap_land_effect_22_sequence_9_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model(D_8011CF2C, (D_8013923C / 0x10) & 3, 0xA, 0x35, 0x7800, 0x1001, D_80182DE4, 0, 0x14, -1);
    D_8013923C += 0x10;
    WMAP_MODEL_FADE_IN(D_80182DE4, 2, 0x81, value);
    timer = g_wmap_land_effect_22_sequence_9_timer;
    D_801B2498.vz += 0x38;
    next_timer = timer - 1;
    g_wmap_land_effect_22_sequence_9_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_effect_22_sequence_9_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it out.
 */
void wmap_land_effect_22_sequence_9_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model(D_8011CF2C, (D_8013923C / 0x10) & 3, 0xA, 0x35, 0x7800, 0x1001, D_80182DE4, 0, 0x14, -1);
    WMAP_MODEL_FADE_OUT(D_80182DE4, 2, value);
    D_8013923C += 0x10;
    timer = g_wmap_land_effect_22_sequence_9_timer;
    D_801B2498.vz += 0x38;
    next_timer = timer - 1;
    g_wmap_land_effect_22_sequence_9_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_effect_22_sequence_9_step++;
    }
}

/** @brief Draw and brighten the rotating effect while reducing its scale. */
void wmap_land_effect_22_sequence_10_step_02(void)
{
    s32 scale;
    s32 intensity;
    s32 remaining;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2670);
    wmap_draw_model(g_wmap_effect_model_pack_3, D_80139264 & 3, 4, 54, 0x78C0, 0x1001, D_801B25D8, 0, 0, D_80139234 / 16);
    scale = D_80139234 - 32;
    D_80139234 = scale;
    intensity = D_801B25D8 + 8;
    D_801B25D8 = intensity;
    D_80139264++;
    if (intensity >= 98)
    {
        D_801B25D8 = 97;
    }
    if (scale < 16)
    {
        D_80139234 = 16;
    }
    PopMatrix();
    remaining = g_wmap_land_effect_22_sequence_10_timer - 1;
    D_801B2670.vz = (u16)(D_801B2670.vz + 30);
    g_wmap_land_effect_22_sequence_10_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_22_sequence_10_step++;
    }
}

/** @brief Draw and fade the rotating effect, then advance its countdown. */
void wmap_land_effect_22_sequence_10_step_04(void)
{
    s32 intensity;
    s32 remaining;

    if (D_801B25D8 != 0)
    {
        PushMatrix();
        wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2670);
        wmap_draw_model(g_wmap_effect_model_pack_3, D_80139264 & 3, 4, 54, 0x78C0, 0x1001, D_801B25D8, 0, 0, D_80139234 / 16);
        intensity = D_801B25D8 - 8;
        D_801B25D8 = intensity;
        D_80139264++;
        if (intensity < 0)
        {
            D_801B25D8 = 0;
        }
        PopMatrix();
        D_801B2670.vz = (u16)(D_801B2670.vz + 30);
    }
    remaining = g_wmap_land_effect_22_sequence_10_timer - 1;
    g_wmap_land_effect_22_sequence_10_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_22_sequence_10_step++;
    }
}

/** @brief Draw and brighten the rotating model, then advance its countdown. */
void wmap_land_effect_22_sequence_11_step_02(void)
{
    s32 remaining;
    s32 intensity;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_d_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, D_80139260 & 3, 4, 0x36, 0x7900, 0x1001, g_wmap_effect_fade_d, 0, 0, -1);
    D_80139260 += 1;
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_d, 2, 0x61, intensity);
    remaining = g_wmap_land_effect_22_sequence_11_timer - 1;
    g_wmap_effect_model_d_rotation.vz = (u16) (g_wmap_effect_model_d_rotation.vz + 0x14);
    g_wmap_land_effect_22_sequence_11_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_22_sequence_11_step += 1;
    }
}

/** @brief Advance the world-map effect and its sequence state. */
void wmap_land_effect_22_sequence_11_step_04(void)
{
    s32 remaining;
    s32 intensity;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_d_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, D_80139260 & 3, 4, 0x36, 0x7900, 0x1001, g_wmap_effect_fade_d, 0, 0, -1);
    intensity = g_wmap_effect_fade_d - 4;
    D_80139260 += 1;
    g_wmap_effect_fade_d = intensity;
    if (intensity < 0)
    {
        g_wmap_effect_fade_d = 0;
    }
    remaining = g_wmap_land_effect_22_sequence_11_timer - 1;
    g_wmap_effect_model_d_rotation.vz = (u16) (g_wmap_effect_model_d_rotation.vz + 0x14);
    g_wmap_land_effect_22_sequence_11_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_22_sequence_11_step += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_22_sequence_12_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 46;
    g_wmap_effect_params[0xB] = 0;
    g_wmap_effect_params[0xC] = 0;
    g_wmap_effect_params[0xF] = 6;
    g_wmap_effect_params[0x10] = 400;
    g_wmap_effect_params[0x11] = 20;
    g_wmap_effect_params[0x12] = 21;
    g_wmap_effect_params[0x13] = 0;
    g_wmap_effect_params[0x14] = 12000;
    WMAP_RESET_PARTICLE_SLOTS(i, 46,
                              g_wmap_actor_motions[i + g_wmap_effect_params[0x11]].field_00,
                              24, g_wmap_animation_bank_0);
    g_wmap_land_effect_22_sequence_12_timer = 276;
    g_wmap_land_effect_22_sequence_12_step++;
    wmap_land_effect_22_sequence_12_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_22_sequence_13_step_01(void)
{
    s32 i;

    g_wmap_effect_params[0x15] = 1;
    g_wmap_effect_params[0x16] = 4;
    g_wmap_effect_params[0x17] = 0x20;
    g_wmap_effect_params[0x18] = 0;
    g_wmap_effect_params[0x19] = 2;
    g_wmap_effect_params[0x1A] = 0;
    g_wmap_effect_params[0x1B] = 0x64;
    g_wmap_effect_params[0x1C] = 0x15;
    g_wmap_effect_params[0x1D] = 2;
    g_wmap_effect_params[0x1E] = 0x1F40;
    WMAP_RESET_PARTICLE_SLOTS(i, 40,
                              g_wmap_actor_motions[i + 100].field_00,
                              104, g_wmap_animation_bank_0);
    g_wmap_land_effect_22_sequence_13_timer = 80;
    g_wmap_land_effect_22_sequence_13_step++;
    wmap_land_effect_22_sequence_13_step_02();
}

/** @brief World-map step handler: configure a model descriptor and clear two entry tables, then advance. */
void wmap_land_effect_22_sequence_14_step_01(void)
{
    s32 i;

    g_wmap_effect_params[0x1F] = 1;
    g_wmap_effect_params[0x20] = 5;
    g_wmap_effect_params[0x21] = 0x20;
    g_wmap_effect_params[0x22] = 0;
    g_wmap_effect_params[0x23] = 8;
    g_wmap_effect_params[0x24] = 0;
    g_wmap_effect_params[0x25] = 0x8C;
    g_wmap_effect_params[0x26] = 0x15;
    g_wmap_effect_params[0x27] = 1;
    g_wmap_effect_params[0x28] = 0x1F40;
    WMAP_RESET_PARTICLE_SLOTS(i, 10,
                              g_wmap_actor_motions[i + 140].field_00,
                              144, g_wmap_animation_bank_0);
    g_wmap_land_effect_22_sequence_14_timer = 80;
    g_wmap_land_effect_22_sequence_14_step++;
    wmap_land_effect_22_sequence_14_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_22_run, D_800D6904, 0x4, g_wmap_land_effect_22_step, g_wmap_land_effect_22_timer)

WMAP_STEP_RESET(wmap_land_effect_22_reset, g_wmap_land_effect_22_step, g_wmap_land_effect_22_timer)

/** @brief World-map step: arm a timed callback, flag it active, then tick the sub-counter. */
void wmap_land_effect_22_step_01(void)
{
    D_80139978 = 0x17;
    g_wmap_focus_screen_position.point.x = 0xA4;
    g_wmap_focus_screen_position.point.y = 0x69;
    wmap_start_sequence(wmap_land_effect_22_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_22_step += 1;
    wmap_land_effect_22_wait_idle();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_22_wait_idle, g_wmap_land_effect_22_step, wmap_land_effect_22_step_03)

/** @brief Start the world-map exit and advance the sequence. */
WMAP_STEP_BEGIN_EXIT(wmap_land_effect_22_step_03, g_wmap_land_effect_22_step, 1)

WMAP_STEP_RUNNER(wmap_land_effect_22_run_timeline, D_800D6914, 0x16, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_22_timeline_reset, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer)

/** @brief World-map step handler: kick two jobs and advance the step. */
void wmap_land_effect_22_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x601040);
    g_wmap_backdrop_target_level = 8;
    wmap_play_sound(0x2C, 0x80);
    g_wmap_land_effect_22_timeline_timer = 0xF;
    g_wmap_land_effect_22_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_22_timeline_wait_02, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_22_timeline_step_03, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer,
                         wmap_land_effect_22_run_sequence_13, 0x14)

WMAP_STEP_WAIT(wmap_land_effect_22_timeline_wait_04, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_22_timeline_step_05, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer,
                             wmap_land_effect_22_run_sequence_6, wmap_land_effect_22_run_sequence_12, 0x78)

WMAP_STEP_WAIT(wmap_land_effect_22_timeline_wait_06, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer)

/** @brief World-map step: register a callback, set flags, advance the step. */
void wmap_land_effect_22_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_22_run_sequence_2);
    g_wmap_transition_mesh_hidden = 1;
    wmap_start_map_tint(0x351040);
    g_wmap_backdrop_target_level = 3;
    g_wmap_land_effect_22_timeline_timer = 2;
    g_wmap_land_effect_22_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_22_timeline_wait_08, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer)

/** @brief World-map step handler: register three callbacks, reset state, advance the step. */
void wmap_land_effect_22_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_22_run_sequence_10);
    wmap_start_sequence(wmap_land_effect_22_run_sequence_7);
    wmap_start_sequence(wmap_land_effect_22_run_sequence_1);
    D_800DBE70 = 0;
    D_80139978 = -1;
    g_wmap_land_effect_22_timeline_timer = 0xF;
    g_wmap_land_effect_22_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_22_timeline_wait_10, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_22_timeline_step_11, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer,
                         wmap_land_effect_22_run_sequence_9, 0x2D)

WMAP_STEP_WAIT(wmap_land_effect_22_timeline_wait_12, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_22_timeline_step_13, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer,
                         wmap_land_effect_22_run_sequence_11, 0x78)

WMAP_STEP_WAIT(wmap_land_effect_22_timeline_wait_14, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer)

/** @brief Register two callbacks around a color update and begin a 136-tick delay. */
void wmap_land_effect_22_timeline_step_15(void)
{
    wmap_start_sequence(&wmap_land_effect_22_run_sequence_3);
    g_wmap_transition_mesh_hidden = 0;
    wmap_start_map_tint(0x601550);
    g_wmap_backdrop_target_level = 8;
    wmap_start_sequence(&wmap_land_effect_22_run_sequence_14);
    g_wmap_land_effect_22_timeline_timer = 0x88;
    g_wmap_land_effect_22_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_22_timeline_wait_16, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_22_timeline_step_17, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer,
                             wmap_land_effect_22_run_sequence_4, wmap_land_effect_22_run_sequence_5, 0x5)

WMAP_STEP_WAIT(wmap_land_effect_22_timeline_wait_18, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_22_timeline_step_19, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer,
                         wmap_land_effect_22_run_sequence_8, 0x84)

WMAP_STEP_WAIT(wmap_land_effect_22_timeline_wait_20, g_wmap_land_effect_22_timeline_step, g_wmap_land_effect_22_timeline_timer)

/** @brief Set the selected record value, clear the flag, and advance the sequence. */
void wmap_land_effect_22_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    g_wmap_cells[g_wmap_focus_cell_x][g_wmap_focus_cell_y].land_id = 0x110;
    g_wmap_land_effect_22_timeline_step += 1;
}

WMAP_STEP_RUNNER(wmap_land_effect_22_run_sequence_1, D_800D696C, 0x4, g_wmap_land_effect_22_sequence_1_step, g_wmap_land_effect_22_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_22_sequence_1_reset, g_wmap_land_effect_22_sequence_1_step, g_wmap_land_effect_22_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_22_sequence_1_step_01,
    g_wmap_land_effect_22_sequence_1_step, g_wmap_land_effect_22_sequence_1_timer,
    5, g_wmap_animation_bank_1, 0,
    0x7F, 0x7F, 0,
    0x159, wmap_land_effect_22_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_22_sequence_1_step_02, g_wmap_land_effect_22_sequence_1_step, g_wmap_land_effect_22_sequence_1_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x16, 0xB, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_22_sequence_1_end, g_wmap_land_effect_22_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_22_run_sequence_2, D_800D697C, 0x4, g_wmap_land_effect_22_sequence_2_step, g_wmap_land_effect_22_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_22_sequence_2_reset, g_wmap_land_effect_22_sequence_2_step, g_wmap_land_effect_22_sequence_2_timer)

WMAP_STEP_DROP_START(wmap_land_effect_22_sequence_2_step_01, g_wmap_land_effect_22_sequence_2_step, g_wmap_land_effect_22_sequence_2_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40, wmap_land_effect_22_sequence_2_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_22_sequence_2_end, g_wmap_land_effect_22_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_22_run_sequence_3, D_800D698C, 0x4, g_wmap_land_effect_22_sequence_3_step, g_wmap_land_effect_22_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_22_sequence_3_reset, g_wmap_land_effect_22_sequence_3_step, g_wmap_land_effect_22_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_22_sequence_3_step_01, g_wmap_land_effect_22_sequence_3_step, g_wmap_land_effect_22_sequence_3_timer, g_wmap_effect_model_b_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x40, wmap_land_effect_22_sequence_3_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_22_sequence_3_end, g_wmap_land_effect_22_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_22_run_sequence_4, D_800D699C, 0x4, g_wmap_land_effect_22_sequence_4_step, g_wmap_land_effect_22_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_22_sequence_4_reset, g_wmap_land_effect_22_sequence_4_step, g_wmap_land_effect_22_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_22_sequence_4_step_01, g_wmap_land_effect_22_sequence_4_step, g_wmap_land_effect_22_sequence_4_timer, g_wmap_effect_model_c_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_c_position, g_wmap_effect_fade_c, 0x80, 0xAFC8, 0x40, wmap_land_effect_22_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_22_sequence_4_end, g_wmap_land_effect_22_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_22_run_sequence_5, D_800D69AC, 0x4, g_wmap_land_effect_22_sequence_5_step, g_wmap_land_effect_22_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_22_sequence_5_reset, g_wmap_land_effect_22_sequence_5_step, g_wmap_land_effect_22_sequence_5_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_22_sequence_5_step_01,
    g_wmap_land_effect_22_sequence_5_step, g_wmap_land_effect_22_sequence_5_timer,
    6, g_wmap_animation_bank_2, 0,
    0, 0x80, 0x10,
    0x8C, wmap_land_effect_22_sequence_5_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_22_sequence_5_step_02, g_wmap_land_effect_22_sequence_5_step, g_wmap_land_effect_22_sequence_5_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x16, 0xA, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_22_sequence_5_end, g_wmap_land_effect_22_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_22_run_sequence_6, D_800D69BC, 0x6, g_wmap_land_effect_22_sequence_6_step, g_wmap_land_effect_22_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_22_sequence_6_reset, g_wmap_land_effect_22_sequence_6_step, g_wmap_land_effect_22_sequence_6_timer)

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_22_sequence_6_step_01(void)
{
    D_80139268 = 0;
    g_wmap_land_effect_22_sequence_6_timer = 0xFA;
    g_wmap_land_effect_22_sequence_6_step += 1;
    wmap_land_effect_22_sequence_6_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_22_sequence_6_step_03, g_wmap_land_effect_22_sequence_6_step, g_wmap_land_effect_22_sequence_6_timer, 0x40,
                    wmap_land_effect_22_sequence_6_step_04)

/**
 * @brief Reset the world-map cursor state and bump the transition counter.
 */
void wmap_land_effect_22_sequence_6_step_05(void)
{
    D_80182D48[1] = 0;
    D_80182D48[0] = 0;
    D_801398C8[1] = 0;
    D_801398C8[0] = 0;
    g_wmap_land_effect_22_sequence_6_step += 1;
}

WMAP_STEP_RUNNER(wmap_land_effect_22_run_sequence_7, D_800D69D4, 0x4, g_wmap_land_effect_22_sequence_7_step, g_wmap_land_effect_22_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_22_sequence_7_reset, g_wmap_land_effect_22_sequence_7_step, g_wmap_land_effect_22_sequence_7_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_22_sequence_7_step_01,
    g_wmap_land_effect_22_sequence_7_step, g_wmap_land_effect_22_sequence_7_timer,
    7, g_wmap_animation_bank_3, 1,
    0x7F, 0x7F, 0,
    0xBE, wmap_land_effect_22_sequence_7_step_02)

/** @brief World-map step: init a sub-object then count down a timer. */
void wmap_land_effect_22_sequence_7_step_02(void)
{
    s32 n = 0x8;

    wmap_step_actor_animation(&g_wmap_sprite_actors[7], &g_wmap_actor_animations[7]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[7], g_wmap_focus_screen_position.packed, n, n, 0);
    if (--g_wmap_land_effect_22_sequence_7_timer == 0)
    {
        g_wmap_land_effect_22_sequence_7_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_22_sequence_7_end, g_wmap_land_effect_22_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_22_run_sequence_8, D_800D69E4, 0x4, g_wmap_land_effect_22_sequence_8_step, g_wmap_land_effect_22_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_22_sequence_8_reset, g_wmap_land_effect_22_sequence_8_step, g_wmap_land_effect_22_sequence_8_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_22_sequence_8_step_01,
    g_wmap_land_effect_22_sequence_8_step, g_wmap_land_effect_22_sequence_8_timer,
    8, g_wmap_animation_bank_3, 0,
    0x81, 0, 8,
    0x10, wmap_land_effect_22_sequence_8_step_02)

/** @brief World-map step: init a sub-object then count down a timer. */
void wmap_land_effect_22_sequence_8_step_02(void)
{
    s32 n = 0x8;

    wmap_step_actor_animation(&g_wmap_sprite_actors[8], &g_wmap_actor_animations[8]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[8], g_wmap_focus_screen_position.packed, n, n, 0);
    if (--g_wmap_land_effect_22_sequence_8_timer == 0)
    {
        g_wmap_land_effect_22_sequence_8_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_22_sequence_8_end, g_wmap_land_effect_22_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_22_run_sequence_9, D_800D69F4, 0x6, g_wmap_land_effect_22_sequence_9_step, g_wmap_land_effect_22_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_22_sequence_9_reset, g_wmap_land_effect_22_sequence_9_step, g_wmap_land_effect_22_sequence_9_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_effect_22_sequence_9_step_01,
    g_wmap_land_effect_22_sequence_9_step, g_wmap_land_effect_22_sequence_9_timer,
    D_801B2498, g_wmap_zero_rotation,
    D_80182DE4, 1, D_8013923C,
    0x7C, wmap_land_effect_22_sequence_9_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_22_sequence_9_step_03, g_wmap_land_effect_22_sequence_9_step, g_wmap_land_effect_22_sequence_9_timer, 0x40,
                    wmap_land_effect_22_sequence_9_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_22_sequence_9_end, g_wmap_land_effect_22_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_22_run_sequence_10, D_800D6A0C, 0x6, g_wmap_land_effect_22_sequence_10_step, g_wmap_land_effect_22_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_22_sequence_10_reset, g_wmap_land_effect_22_sequence_10_step, g_wmap_land_effect_22_sequence_10_timer)

/** @brief Reset effect state and begin a 96-tick sequence step. */
void wmap_land_effect_22_sequence_10_step_01(void)
{
    D_801B25D8 = 1;
    D_801B2670 = g_wmap_zero_rotation;
    D_80139234 = 0;
    D_80139264 = 0;
    g_wmap_land_effect_22_sequence_10_timer = 0x60;
    g_wmap_land_effect_22_sequence_10_step += 1;
    wmap_land_effect_22_sequence_10_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_22_sequence_10_step_03, g_wmap_land_effect_22_sequence_10_step, g_wmap_land_effect_22_sequence_10_timer, 0x10,
                    wmap_land_effect_22_sequence_10_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_22_sequence_10_end, g_wmap_land_effect_22_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_22_run_sequence_11, D_800D6A24, 0x6, g_wmap_land_effect_22_sequence_11_step, g_wmap_land_effect_22_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_22_sequence_11_reset, g_wmap_land_effect_22_sequence_11_step, g_wmap_land_effect_22_sequence_11_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_effect_22_sequence_11_step_01,
    g_wmap_land_effect_22_sequence_11_step, g_wmap_land_effect_22_sequence_11_timer,
    g_wmap_effect_model_d_rotation, g_wmap_zero_rotation,
    g_wmap_effect_fade_d, 1, D_80139260,
    0x6C, wmap_land_effect_22_sequence_11_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_effect_22_sequence_11_step_03, g_wmap_land_effect_22_sequence_11_step, g_wmap_land_effect_22_sequence_11_timer, 0x20,
                    wmap_land_effect_22_sequence_11_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_22_sequence_11_end, g_wmap_land_effect_22_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_effect_22_run_sequence_12, D_800D6A3C, 0x6, g_wmap_land_effect_22_sequence_12_step, g_wmap_land_effect_22_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_effect_22_sequence_12_reset, g_wmap_land_effect_22_sequence_12_step, g_wmap_land_effect_22_sequence_12_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_22_sequence_12_step_02,
    g_wmap_land_effect_22_sequence_12_step, g_wmap_land_effect_22_sequence_12_timer,
    func_8006A2FC(&g_wmap_sprite_actors[24], &g_wmap_actor_animations[24], 0x2E, 0x1, 0xFF, 0x4, 0, (s32)((u8*)g_wmap_effect_params + 0x28)))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_22_sequence_12_step_03,
    g_wmap_land_effect_22_sequence_12_step, g_wmap_land_effect_22_sequence_12_timer,
    g_wmap_effect_params[15], 0x40, wmap_land_effect_22_sequence_12_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_22_sequence_12_step_04,
    g_wmap_land_effect_22_sequence_12_step, g_wmap_land_effect_22_sequence_12_timer,
    func_8006A2FC(&g_wmap_sprite_actors[24], &g_wmap_actor_animations[24], 0x2E, 0x1, 0xFF, 0x4, 0, (s32)((u8*)g_wmap_effect_params + 0x28)))

WMAP_STEP_ADVANCE(wmap_land_effect_22_sequence_12_end, g_wmap_land_effect_22_sequence_12_step)

WMAP_STEP_RUNNER(wmap_land_effect_22_run_sequence_13, D_800D6A54, 0x6, g_wmap_land_effect_22_sequence_13_step, g_wmap_land_effect_22_sequence_13_timer)

WMAP_STEP_RESET(wmap_land_effect_22_sequence_13_reset, g_wmap_land_effect_22_sequence_13_step, g_wmap_land_effect_22_sequence_13_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_22_sequence_13_step_02,
    g_wmap_land_effect_22_sequence_13_step, g_wmap_land_effect_22_sequence_13_timer,
    func_8006AEE0(),
    func_8006A2FC(&g_wmap_sprite_actors[104], &g_wmap_actor_animations[104], 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)g_wmap_effect_params + 0x50)))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_22_sequence_13_step_03,
    g_wmap_land_effect_22_sequence_13_step, g_wmap_land_effect_22_sequence_13_timer,
    g_wmap_effect_params[25], 0x20, wmap_land_effect_22_sequence_13_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_22_sequence_13_step_04,
    g_wmap_land_effect_22_sequence_13_step, g_wmap_land_effect_22_sequence_13_timer,
    func_8006AEE0(),
    func_8006A2FC(&g_wmap_sprite_actors[104], &g_wmap_actor_animations[104], 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)g_wmap_effect_params + 0x50)))

WMAP_STEP_ADVANCE(wmap_land_effect_22_sequence_13_end, g_wmap_land_effect_22_sequence_13_step)

WMAP_STEP_RUNNER(wmap_land_effect_22_run_sequence_14, D_800D6A6C, 0x6, g_wmap_land_effect_22_sequence_14_step, g_wmap_land_effect_22_sequence_14_timer)

WMAP_STEP_RESET(wmap_land_effect_22_sequence_14_reset, g_wmap_land_effect_22_sequence_14_step, g_wmap_land_effect_22_sequence_14_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_22_sequence_14_step_02,
    g_wmap_land_effect_22_sequence_14_step, g_wmap_land_effect_22_sequence_14_timer,
    func_8006AEE0(),
    func_8006A2FC(&g_wmap_sprite_actors[144], &g_wmap_actor_animations[144], 0xA, 0xFF, 0x1, 0x8, 0, (s32)((u8*)g_wmap_effect_params + 0x78)))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_22_sequence_14_step_03,
    g_wmap_land_effect_22_sequence_14_step, g_wmap_land_effect_22_sequence_14_timer,
    g_wmap_effect_params[35], 0x20, wmap_land_effect_22_sequence_14_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_22_sequence_14_step_04,
    g_wmap_land_effect_22_sequence_14_step, g_wmap_land_effect_22_sequence_14_timer,
    func_8006AEE0(),
    func_8006A2FC(&g_wmap_sprite_actors[144], &g_wmap_actor_animations[144], 0xA, 0xFF, 0x1, 0x8, 0, (s32)((u8*)g_wmap_effect_params + 0x78)))

WMAP_STEP_ADVANCE(wmap_land_effect_22_sequence_14_end, g_wmap_land_effect_22_sequence_14_step)
