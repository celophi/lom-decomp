#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_land_effect_27.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"
#include "wmap_step_sequence.h"
#include "wmap_cells.h"

void wmap_land_effect_27_sequence_6_step_02(void);
void wmap_land_effect_27_sequence_8_step_02(void);
void wmap_land_effect_27_sequence_9_step_02(void);
void wmap_land_effect_27_sequence_10_step_02(void);
void wmap_land_effect_27_sequence_11_step_02(void);
void wmap_land_effect_27_wait_idle_02(void);
void wmap_land_effect_27_step_03(void);
s32 wmap_land_effect_27_run_timeline(s32 arg0);
void wmap_land_effect_27_wait_idle_04(void);
void wmap_land_effect_27_end(void);
s32 wmap_land_effect_27_run_sequence_6(s32 arg0);
s32 wmap_land_effect_27_run_sequence_9(s32 arg0);
s32 wmap_land_effect_27_run_sequence_3(s32 arg0);
s32 wmap_land_effect_27_run_sequence_8(s32 arg0);
s32 wmap_land_effect_27_run_sequence_5(s32 arg0);
s32 wmap_land_effect_27_run_sequence_1(s32 arg0);
s32 wmap_land_effect_27_run_sequence_7(s32 arg0);
s32 wmap_land_effect_27_run_sequence_12(s32 arg0);
s32 wmap_land_effect_27_run_sequence_4(s32 arg0);
s32 wmap_land_effect_27_run_sequence_2(s32 arg0);
s32 wmap_land_effect_27_run_sequence_11(s32 arg0);
s32 wmap_land_effect_27_run_sequence_10(s32 arg0);
void wmap_land_effect_27_sequence_1_step_02(void);
void wmap_land_effect_27_sequence_2_step_02(void);
void wmap_land_effect_27_sequence_6_step_04(void);
void wmap_land_effect_27_sequence_7_step_02(void);
void wmap_land_effect_27_sequence_7_step_04(void);
void wmap_land_effect_27_sequence_8_step_04(void);
void wmap_land_effect_27_sequence_9_step_04(void);
void wmap_land_effect_27_sequence_10_step_04(void);
void wmap_land_effect_27_sequence_11_step_04(void);
void wmap_land_effect_27_sequence_12_step_02(void);

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

extern u8* g_wmap_effect_model_pack_1;
extern s32 g_wmap_land_effect_27_sequence_4_timer;
extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_27_sequence_3_timer;
extern s32 D_800D665C[];
extern void *g_wmap_effect_model_pack_2;
extern void *g_wmap_effect_model_pack_3;
extern void *D_8011CF2C;
extern s32 D_80139234;
extern s32 g_wmap_land_effect_27_sequence_5_timer;
extern s32 g_wmap_land_effect_27_sequence_6_timer;
extern u8 g_wmap_animation_bank_2[];
extern s32 g_wmap_land_effect_27_sequence_8_timer;
extern s32 g_wmap_land_effect_27_sequence_9_timer;
extern s32 g_wmap_land_effect_27_sequence_10_timer;
extern s32 g_wmap_land_effect_27_sequence_11_timer;
extern s32 g_wmap_land_effect_27_timer;
extern void (*D_800D65A4[])(void);
extern void wmap_land_effect_27_step_03(void);
extern void wmap_land_effect_27_end(void);
extern s32 g_wmap_land_effect_27_timeline_timer;
extern void (*D_800D65BC[])(void);
extern u32 g_wmap_selected_artifact;
extern s32 g_wmap_land_effect_27_sequence_1_timer;
extern void (*D_800D661C[])(void);
extern s32 g_wmap_land_effect_27_sequence_2_timer;
extern void (*D_800D662C[])(void);
extern u8 g_wmap_animation_bank_0[];
extern void wmap_land_effect_27_sequence_2_step_02(void);
extern void (*D_800D663C[])(void);
extern void (*D_800D664C[])(void);
extern void (*D_800D6674[])(void);
extern void (*D_800D668C[])(void);
extern void wmap_land_effect_27_sequence_6_step_04(void);
extern s32 g_wmap_land_effect_27_sequence_7_timer;
extern void (*D_800D66A4[])(void);
extern void wmap_land_effect_27_sequence_7_step_02(void);
extern void wmap_land_effect_27_sequence_7_step_04(void);
extern void (*D_800D66BC[])(void);
extern void (*D_800D66D4[])(void);
extern void wmap_land_effect_27_sequence_9_step_04(void);
extern void (*D_800D66EC[])(void);
extern void wmap_land_effect_27_sequence_10_step_04(void);
extern void (*D_800D6704[])(void);
extern void wmap_land_effect_27_sequence_11_step_04(void);
extern s32 g_wmap_land_effect_27_sequence_12_timer;
extern void (*D_800D671C[])(void);
extern void wmap_land_effect_27_sequence_12_step_02(void);
extern u32 g_wmap_land_effect_27_sequence_4_step;
extern u32 g_wmap_land_effect_27_sequence_3_step;

extern u32 g_wmap_land_effect_27_sequence_5_step;
extern u32 g_wmap_land_effect_27_sequence_6_step;
extern u32 g_wmap_land_effect_27_sequence_8_step;
extern u32 g_wmap_land_effect_27_sequence_9_step;
extern u32 g_wmap_land_effect_27_sequence_10_step;
extern u8 g_wmap_animation_bank_1[];
extern u32 g_wmap_land_effect_27_sequence_11_step;
extern u32 g_wmap_land_effect_27_step;
extern u32 g_wmap_land_effect_27_timeline_step;
extern u32 g_wmap_land_effect_27_sequence_1_step;
extern u32 g_wmap_land_effect_27_sequence_2_step;
extern u32 g_wmap_land_effect_27_sequence_7_step;
extern u32 g_wmap_land_effect_27_sequence_12_step;

extern VECTOR g_wmap_camera_translation;



extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* g_wmap_effect_params;

extern WmapSlot14 g_wmap_actor_motions[];

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_27_sequence_3_step_02,
    g_wmap_land_effect_27_sequence_3_step, g_wmap_land_effect_27_sequence_3_timer,
    g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position,
    g_wmap_effect_fade_a, g_wmap_load_buffer, -3500, 4)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_27_sequence_4_step_02,
    g_wmap_land_effect_27_sequence_4_step, g_wmap_land_effect_27_sequence_4_timer,
    g_wmap_effect_model_b_rotation, g_wmap_effect_model_b_position,
    g_wmap_effect_fade_b, g_wmap_effect_model_pack_1, -3500, 4)

/** @brief Draw three rotating effect layers and update their fade and animation. */
void wmap_land_effect_27_sequence_5_step_02(void)
{
    s32 intensity;
    s32 frame;
    s32 remaining;
    u16 angle;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_c_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_2, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, g_wmap_effect_fade_c, 0, -10, -1);
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_b_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, g_wmap_effect_fade_c, 0, -40, -1);
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_c_rotation);
    wmap_draw_model(D_8011CF2C, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, g_wmap_effect_fade_c, 0, -70, -1);
    PopMatrix();
    intensity = g_wmap_effect_fade_c + 2;
    g_wmap_effect_fade_c = intensity;
    if (intensity >= 130)
    {
        g_wmap_effect_fade_c = 129;
    }
    angle = g_wmap_effect_model_c_rotation.vz;
    g_wmap_effect_model_c_rotation.vz = angle + 330;
    g_wmap_effect_model_b_rotation.vz += 10;
    g_wmap_effect_model_c_rotation.vz = angle + 340;
    frame = D_80139234 + 1;
    D_80139234 = frame;
    if (frame >= 6)
    {
        D_80139234 = 0;
    }
    remaining = g_wmap_land_effect_27_sequence_5_timer - 1;
    g_wmap_land_effect_27_sequence_5_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_27_sequence_5_step++;
    }
}

/** @brief Draw three rotating effect layers and update their fade and animation. */
void wmap_land_effect_27_sequence_5_step_04(void)
{
    s32 intensity;
    s32 frame;
    s32 remaining;
    u16 angle;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_c_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_2, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, g_wmap_effect_fade_c, 0, -10, -1);
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_b_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, g_wmap_effect_fade_c, 0, -40, -1);
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_c_rotation);
    wmap_draw_model(D_8011CF2C, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, g_wmap_effect_fade_c, 0, -70, -1);
    PopMatrix();
    intensity = g_wmap_effect_fade_c - 4;
    g_wmap_effect_fade_c = intensity;
    if (intensity < 0)
    {
        g_wmap_effect_fade_c = 0;
    }
    angle = g_wmap_effect_model_c_rotation.vz;
    g_wmap_effect_model_c_rotation.vz = angle + 330;
    g_wmap_effect_model_b_rotation.vz += 10;
    g_wmap_effect_model_c_rotation.vz = angle + 340;
    frame = D_80139234 + 1;
    D_80139234 = frame;
    if (frame >= 6)
    {
        D_80139234 = 0;
    }
    remaining = g_wmap_land_effect_27_sequence_5_timer - 1;
    g_wmap_land_effect_27_sequence_5_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_27_sequence_5_step++;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_27_sequence_6_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 40;
    g_wmap_effect_params[0x1F] = 1;
    g_wmap_effect_params[0x20] = 4;
    g_wmap_effect_params[0x21] = 32;
    g_wmap_effect_params[0x22] = 0;
    g_wmap_effect_params[0x23] = 2;
    g_wmap_effect_params[0x24] = 0;
    g_wmap_effect_params[0x25] = 140;
    g_wmap_effect_params[0x26] = 8;
    g_wmap_effect_params[0x27] = 3;
    g_wmap_effect_params[0x28] = 8000;
    for (i = 0; i < 40; i++)
    {
        g_wmap_actor_motions[i + 140].field_00 = 0;
        g_wmap_actor_animations[i + 144].data = g_wmap_animation_bank_2;
    }
    g_wmap_land_effect_27_sequence_6_timer = 80;
    g_wmap_land_effect_27_sequence_6_step++;
    wmap_land_effect_27_sequence_6_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_27_sequence_8_step_01(void)
{
    s32 i;

    g_wmap_effect_params[0x1] = 1;
    g_wmap_effect_params[0x2] = 4;
    g_wmap_effect_params[0x3] = 0x20;
    g_wmap_effect_params[0x4] = 0;
    g_wmap_effect_params[0x5] = 2;
    g_wmap_effect_params[0x6] = 0;
    g_wmap_effect_params[0x7] = 0x14;
    g_wmap_effect_params[0x8] = 8;
    g_wmap_effect_params[0x9] = 0;
    g_wmap_effect_params[0xA] = 0x2EE0;
    for (i = 0; i < 40; i++)
    {
        g_wmap_actor_motions[i + 20].field_00 = 0;
        g_wmap_actor_animations[i + 20].data = g_wmap_animation_bank_2;
    }
    g_wmap_land_effect_27_sequence_8_timer = 80;
    g_wmap_land_effect_27_sequence_8_step++;
    wmap_land_effect_27_sequence_8_step_02();
}

/** @brief World-map step: init hero struct fields, clear tables, advance step. */
void wmap_land_effect_27_sequence_9_step_01(void)
{
    s32 i;
    void *base = g_wmap_effect_params;

    *(s32 *)((u8 *)base + 0x2C) = 0;
    *(s32 *)((u8 *)base + 0x30) = 0;
    *(s32 *)((u8 *)base + 0x38) = 0;
    *(s32 *)((u8 *)base + 0x34) = 0x80;
    *(s32 *)((u8 *)base + 0x3C) = 2;
    *(s32 *)((u8 *)base + 0x40) = 0x64;
    *(s32 *)((u8 *)base + 0x44) = 0x3C;
    *(s32 *)((u8 *)base + 0x48) = 8;
    *(s32 *)((u8 *)base + 0x4C) = 1;
    *(s32 *)((u8 *)base + 0x50) = 0x5DC0;

    for (i = 0; i < 0x18; i++)
    {
        g_wmap_actor_motions[60 + i].field_00 = 0;
        g_wmap_actor_animations[60 + i].data = g_wmap_animation_bank_2;
    }

    g_wmap_land_effect_27_sequence_9_timer = 0x38;
    g_wmap_land_effect_27_sequence_9_step += 1;
    wmap_land_effect_27_sequence_9_step_02();
}

/** @brief World-map step handler: configure a model descriptor and clear two entry tables, then advance. */
void wmap_land_effect_27_sequence_10_step_01(void)
{
    s32 i;

    g_wmap_effect_params[0x15] = 1;
    g_wmap_effect_params[0x17] = 0x20;
    g_wmap_effect_params[0x1A] = 0x3E8;
    g_wmap_effect_params[0x1B] = 0x64;
    g_wmap_effect_params[0x1C] = 8;
    g_wmap_effect_params[0x1D] = 2;
    g_wmap_effect_params[0x16] = 1;
    g_wmap_effect_params[0x18] = 0;
    g_wmap_effect_params[0x19] = 1;
    g_wmap_effect_params[0x1E] = 0x6D60;

    for (i = 0; i < 20; i++)
    {
        g_wmap_actor_motions[100 + i].field_00 = 0;
        g_wmap_actor_animations[100 + i].data = g_wmap_animation_bank_2;
    }

    g_wmap_land_effect_27_sequence_10_timer = 20;
    g_wmap_land_effect_27_sequence_10_step += 1;
    wmap_land_effect_27_sequence_10_step_02();
}

/** @brief World-map step: init hero struct fields, clear tables, advance step. */
void wmap_land_effect_27_sequence_11_step_01(void)
{
    s32 i;

    g_wmap_effect_params[0x29] = 1;
    g_wmap_effect_params[0x2A] = 4;
    g_wmap_effect_params[0x2B] = 0x20;
    g_wmap_effect_params[0x2E] = 0x3E8;
    g_wmap_effect_params[0x2F] = 0xB4;
    g_wmap_effect_params[0x30] = 0x15;
    g_wmap_effect_params[0x31] = 2;
    g_wmap_effect_params[0x2C] = 0;
    g_wmap_effect_params[0x2D] = 1;
    g_wmap_effect_params[0x32] = 0x1F40;

    for (i = 0; i < 20; i++)
    {
        g_wmap_actor_motions[180 + i].field_00 = 0;
        g_wmap_actor_animations[180 + i].data = g_wmap_animation_bank_1;
    }

    g_wmap_land_effect_27_sequence_11_timer = 20;
    g_wmap_land_effect_27_sequence_11_step += 1;
    wmap_land_effect_27_sequence_11_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_27_run, D_800D65A4, 0x6, g_wmap_land_effect_27_step, g_wmap_land_effect_27_timer)

WMAP_STEP_RESET(wmap_land_effect_27_reset, g_wmap_land_effect_27_step, g_wmap_land_effect_27_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_27_step_01, g_wmap_land_effect_27_step, wmap_run_land_focus, wmap_land_effect_27_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_27_wait_idle_02, g_wmap_land_effect_27_step, wmap_land_effect_27_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_27_step_03, g_wmap_land_effect_27_step, wmap_land_effect_27_run_timeline, wmap_land_effect_27_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_27_wait_idle_04, g_wmap_land_effect_27_step, wmap_land_effect_27_end)

WMAP_STEP_ADVANCE(wmap_land_effect_27_end, g_wmap_land_effect_27_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_timeline, D_800D65BC, 0x18, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_27_timeline_reset, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

/** @brief World-map step: mark active, request a resource, seed the sub-counter, tick. */
void wmap_land_effect_27_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_play_sound(0x2E, 0x80);
    g_wmap_land_effect_27_timeline_timer = 4;
    g_wmap_land_effect_27_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_02, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_27_timeline_step_03, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer,
                         wmap_land_effect_27_run_sequence_6, 0xF)

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_04, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_27_timeline_step_05, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer,
                         wmap_land_effect_27_run_sequence_9, 0xC)

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_06, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

/** @brief World-map step handler: register a callback, kick a job, advance the step. */
void wmap_land_effect_27_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_27_run_sequence_3);
    wmap_start_map_tint(0x651035);
    g_wmap_backdrop_target_level = 4;
    g_wmap_land_effect_27_timeline_timer = 2;
    g_wmap_land_effect_27_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_08, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void wmap_land_effect_27_timeline_step_09(void)
{
    g_wmap_transition_mesh_hidden = 1;
    g_wmap_land_effect_27_timeline_timer = 0x14;
    g_wmap_land_effect_27_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_10, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_27_timeline_step_11, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer,
                             wmap_land_effect_27_run_sequence_8, wmap_land_effect_27_run_sequence_5, 0x28)

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_12, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

/** @brief Register two callbacks around the sequence flag update and begin a 30-tick delay. */
void wmap_land_effect_27_timeline_step_13(void)
{
    wmap_start_sequence(&wmap_land_effect_27_run_sequence_7);
    g_wmap_placement_overlay_hidden = 1;
    wmap_start_sequence(&wmap_land_effect_27_run_sequence_1);
    g_wmap_land_effect_27_timeline_timer = 0x1E;
    g_wmap_land_effect_27_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_14, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_27_timeline_step_15, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer,
                         wmap_land_effect_27_run_sequence_12, 0x38)

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_16, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_27_timeline_step_17, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer,
                         wmap_land_effect_27_run_sequence_4, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_18, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

/** @brief Register a sequence callback, clear the world-map value, and start a 64-tick delay. */
void wmap_land_effect_27_timeline_step_19(void)
{
    wmap_start_sequence(&wmap_land_effect_27_run_sequence_2);
    g_wmap_transition_mesh_hidden = 0;
    g_wmap_land_effect_27_timeline_timer = 0x40;
    g_wmap_land_effect_27_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_20, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_27_timeline_step_21, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer,
                             wmap_land_effect_27_run_sequence_11, wmap_land_effect_27_run_sequence_10, 0xAC)

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_22, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_27_timeline_finish, g_wmap_land_effect_27_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y, g_wmap_selected_artifact)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_1, D_800D661C, 0x4, g_wmap_land_effect_27_sequence_1_step, g_wmap_land_effect_27_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_1_reset, g_wmap_land_effect_27_sequence_1_step, g_wmap_land_effect_27_sequence_1_timer)

void wmap_land_effect_27_sequence_1_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[4];

    g_wmap_actor_animations[4].data = g_wmap_animation_bank_1;
    actor->scale_index = 0xF;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    actor->shade_step = 8;
    actor->resource_index = 0;
    actor->target_shade = 0x81;
    actor->shade = 1;
    g_wmap_land_effect_27_sequence_1_timer = 0x24;
    g_wmap_land_effect_27_sequence_1_step += 1;
    wmap_land_effect_27_sequence_1_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_27_sequence_1_step_02, g_wmap_land_effect_27_sequence_1_step, g_wmap_land_effect_27_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0x15, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_1_end, g_wmap_land_effect_27_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_2, D_800D662C, 0x4, g_wmap_land_effect_27_sequence_2_step, g_wmap_land_effect_27_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_2_reset, g_wmap_land_effect_27_sequence_2_step, g_wmap_land_effect_27_sequence_2_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_27_sequence_2_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    g_wmap_actor_animations[5].data = g_wmap_animation_bank_0;
    actor->scale_index = 0xF;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    actor->shade_step = 2;
    actor->resource_index = 0;
    actor->target_shade = 0x80;
    actor->shade = 0;
    g_wmap_land_effect_27_sequence_2_timer = 0xEE;
    g_wmap_land_effect_27_sequence_2_step += 1;
    wmap_land_effect_27_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_27_sequence_2_step_02, g_wmap_land_effect_27_sequence_2_step, g_wmap_land_effect_27_sequence_2_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x12, 0x5, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_2_end, g_wmap_land_effect_27_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_3, D_800D663C, 0x4, g_wmap_land_effect_27_sequence_3_step, g_wmap_land_effect_27_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_3_reset, g_wmap_land_effect_27_sequence_3_step, g_wmap_land_effect_27_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_27_sequence_3_step_01, g_wmap_land_effect_27_sequence_3_step, g_wmap_land_effect_27_sequence_3_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x20, wmap_land_effect_27_sequence_3_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_3_end, g_wmap_land_effect_27_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_4, D_800D664C, 0x4, g_wmap_land_effect_27_sequence_4_step, g_wmap_land_effect_27_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_4_reset, g_wmap_land_effect_27_sequence_4_step, g_wmap_land_effect_27_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_27_sequence_4_step_01, g_wmap_land_effect_27_sequence_4_step, g_wmap_land_effect_27_sequence_4_timer, g_wmap_effect_model_b_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x20, wmap_land_effect_27_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_4_end, g_wmap_land_effect_27_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_5, D_800D6674, 0x6, g_wmap_land_effect_27_sequence_5_step, g_wmap_land_effect_27_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_5_reset, g_wmap_land_effect_27_sequence_5_step, g_wmap_land_effect_27_sequence_5_timer)

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void wmap_land_effect_27_sequence_5_step_01(void)
{
    g_wmap_effect_fade_c = 1;
    g_wmap_effect_model_c_rotation = g_wmap_zero_rotation;
    D_80139234 = 0;
    g_wmap_land_effect_27_sequence_5_timer = 0x60;
    g_wmap_land_effect_27_sequence_5_step += 1;
    wmap_land_effect_27_sequence_5_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_27_sequence_5_step_03, g_wmap_land_effect_27_sequence_5_step, g_wmap_land_effect_27_sequence_5_timer, 0x20,
                    wmap_land_effect_27_sequence_5_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_5_end, g_wmap_land_effect_27_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_6, D_800D668C, 0x6, g_wmap_land_effect_27_sequence_6_step, g_wmap_land_effect_27_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_6_reset, g_wmap_land_effect_27_sequence_6_step, g_wmap_land_effect_27_sequence_6_timer)

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_6_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(&g_wmap_sprite_actors[144], &g_wmap_actor_animations[144], 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)g_wmap_effect_params + 0x78));
    if (--g_wmap_land_effect_27_sequence_6_timer == 0)
    {
        g_wmap_land_effect_27_sequence_6_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_27_sequence_6_step_03(void)
{
    g_wmap_land_effect_27_sequence_6_timer = 0x20;
    g_wmap_effect_params[35] = -1;
    g_wmap_land_effect_27_sequence_6_step += 1;
    wmap_land_effect_27_sequence_6_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_6_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(&g_wmap_sprite_actors[144], &g_wmap_actor_animations[144], 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)g_wmap_effect_params + 0x78));
    if (--g_wmap_land_effect_27_sequence_6_timer == 0)
    {
        g_wmap_land_effect_27_sequence_6_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_6_end, g_wmap_land_effect_27_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_7, D_800D66A4, 0x6, g_wmap_land_effect_27_sequence_7_step, g_wmap_land_effect_27_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_7_reset, g_wmap_land_effect_27_sequence_7_step, g_wmap_land_effect_27_sequence_7_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_27_sequence_7_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    g_wmap_actor_animations[6].data = g_wmap_animation_bank_1;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 4;
    actor->target_shade = 0x81;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->shade = 1;
    g_wmap_land_effect_27_sequence_7_timer = 0x80;
    g_wmap_land_effect_27_sequence_7_step += 1;
    wmap_land_effect_27_sequence_7_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_27_sequence_7_step_02, g_wmap_land_effect_27_sequence_7_step, g_wmap_land_effect_27_sequence_7_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x15, 0x5, 0)

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_27_sequence_7_step_03(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    actor->shade_step = 4;
    actor->target_shade = 0;
    g_wmap_land_effect_27_sequence_7_timer = 0x80;
    g_wmap_land_effect_27_sequence_7_step += 1;
    wmap_land_effect_27_sequence_7_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_27_sequence_7_step_04, g_wmap_land_effect_27_sequence_7_step, g_wmap_land_effect_27_sequence_7_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x15, 0x5, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_7_end, g_wmap_land_effect_27_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_8, D_800D66BC, 0x6, g_wmap_land_effect_27_sequence_8_step, g_wmap_land_effect_27_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_8_reset, g_wmap_land_effect_27_sequence_8_step, g_wmap_land_effect_27_sequence_8_timer)

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_8_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(&g_wmap_sprite_actors[20], &g_wmap_actor_animations[20], 0x28, 0xFF, 0x1, 0x8, 0, (s32)g_wmap_effect_params);
    if (--g_wmap_land_effect_27_sequence_8_timer == 0)
    {
        g_wmap_land_effect_27_sequence_8_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_27_sequence_8_step_03(void)
{
    g_wmap_land_effect_27_sequence_8_timer = 0x20;
    g_wmap_effect_params[5] = -1;
    g_wmap_land_effect_27_sequence_8_step += 1;
    wmap_land_effect_27_sequence_8_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_8_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(&g_wmap_sprite_actors[20], &g_wmap_actor_animations[20], 0x28, 0xFF, 0x1, 0x8, 0, (s32)g_wmap_effect_params);
    if (--g_wmap_land_effect_27_sequence_8_timer == 0)
    {
        g_wmap_land_effect_27_sequence_8_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_8_end, g_wmap_land_effect_27_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_9, D_800D66D4, 0x6, g_wmap_land_effect_27_sequence_9_step, g_wmap_land_effect_27_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_9_reset, g_wmap_land_effect_27_sequence_9_step, g_wmap_land_effect_27_sequence_9_timer)

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_9_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(&g_wmap_sprite_actors[60], &g_wmap_actor_animations[60], 0x18, 0xFF, 0x1, 0x2, 0, (s32)((u8*)g_wmap_effect_params + 0x28));
    if (--g_wmap_land_effect_27_sequence_9_timer == 0)
    {
        g_wmap_land_effect_27_sequence_9_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_27_sequence_9_step_03(void)
{
    g_wmap_land_effect_27_sequence_9_timer = 0x80;
    g_wmap_effect_params[15] = -1;
    g_wmap_land_effect_27_sequence_9_step += 1;
    wmap_land_effect_27_sequence_9_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_9_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(&g_wmap_sprite_actors[60], &g_wmap_actor_animations[60], 0x18, 0xFF, 0x1, 0x2, 0, (s32)((u8*)g_wmap_effect_params + 0x28));
    if (--g_wmap_land_effect_27_sequence_9_timer == 0)
    {
        g_wmap_land_effect_27_sequence_9_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_9_end, g_wmap_land_effect_27_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_10, D_800D66EC, 0x6, g_wmap_land_effect_27_sequence_10_step, g_wmap_land_effect_27_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_10_reset, g_wmap_land_effect_27_sequence_10_step, g_wmap_land_effect_27_sequence_10_timer)

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_10_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(&g_wmap_sprite_actors[100], &g_wmap_actor_animations[100], 0x14, 0xFF, 0x1, 0x8, 0, (s32)((u8*)g_wmap_effect_params + 0x50));
    if (--g_wmap_land_effect_27_sequence_10_timer == 0)
    {
        g_wmap_land_effect_27_sequence_10_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_27_sequence_10_step_03(void)
{
    g_wmap_land_effect_27_sequence_10_timer = 0x20;
    g_wmap_effect_params[25] = -1;
    g_wmap_land_effect_27_sequence_10_step += 1;
    wmap_land_effect_27_sequence_10_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_10_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(&g_wmap_sprite_actors[100], &g_wmap_actor_animations[100], 0x14, 0xFF, 0x1, 0x8, 0, (s32)((u8*)g_wmap_effect_params + 0x50));
    if (--g_wmap_land_effect_27_sequence_10_timer == 0)
    {
        g_wmap_land_effect_27_sequence_10_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_10_end, g_wmap_land_effect_27_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_11, D_800D6704, 0x6, g_wmap_land_effect_27_sequence_11_step, g_wmap_land_effect_27_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_11_reset, g_wmap_land_effect_27_sequence_11_step, g_wmap_land_effect_27_sequence_11_timer)

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_11_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(&g_wmap_sprite_actors[180], &g_wmap_actor_animations[180], 0x14, 0xFF, 0x1, 0x8, 0, (s32)((u8*)g_wmap_effect_params + 0xA0));
    if (--g_wmap_land_effect_27_sequence_11_timer == 0)
    {
        g_wmap_land_effect_27_sequence_11_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_27_sequence_11_step_03(void)
{
    g_wmap_land_effect_27_sequence_11_timer = 0x20;
    g_wmap_effect_params[45] = -1;
    g_wmap_land_effect_27_sequence_11_step += 1;
    wmap_land_effect_27_sequence_11_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_11_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(&g_wmap_sprite_actors[180], &g_wmap_actor_animations[180], 0x14, 0xFF, 0x1, 0x8, 0, (s32)((u8*)g_wmap_effect_params + 0xA0));
    if (--g_wmap_land_effect_27_sequence_11_timer == 0)
    {
        g_wmap_land_effect_27_sequence_11_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_11_end, g_wmap_land_effect_27_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_12, D_800D671C, 0x4, g_wmap_land_effect_27_sequence_12_step, g_wmap_land_effect_27_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_12_reset, g_wmap_land_effect_27_sequence_12_step, g_wmap_land_effect_27_sequence_12_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_27_sequence_12_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[8];

    g_wmap_actor_animations[8].data = g_wmap_animation_bank_0;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 2;
    actor->target_shade = 0x81;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->shade = 1;
    g_wmap_land_effect_27_sequence_12_timer = 0x5A;
    g_wmap_land_effect_27_sequence_12_step += 1;
    wmap_land_effect_27_sequence_12_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_27_sequence_12_step_02, g_wmap_land_effect_27_sequence_12_step, g_wmap_land_effect_27_sequence_12_timer,
                              g_wmap_sprite_actors[8], g_wmap_actor_animations[8], g_wmap_focus_screen_position, 0x1E, 0x3, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_12_end, g_wmap_land_effect_27_sequence_12_step)
