#include "wmap_main.h"
#include "wmap_land_effect_11.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"
#include "wmap_step_sequence.h"

/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

void wmap_land_effect_11_sequence_3_step_02(void);
void wmap_land_effect_11_sequence_5_step_02(void);
void wmap_land_effect_11_wait_idle_02(void);
void wmap_land_effect_11_step_03(void);
s32 wmap_land_effect_11_run_timeline(s32 arg0);
void wmap_land_effect_11_wait_idle_04(void);
void wmap_land_effect_11_end(void);
s32 wmap_land_effect_11_run_sequence_3(s32 arg0);
s32 wmap_land_effect_11_run_sequence_4(s32 arg0);
s32 wmap_land_effect_11_run_sequence_6(s32 arg0);
s32 wmap_land_effect_11_run_sequence_1(s32 arg0);
s32 wmap_land_effect_11_run_sequence_8(s32 arg0);
s32 wmap_land_effect_11_run_sequence_7(s32 arg0);
s32 wmap_land_effect_11_run_sequence_5(s32 arg0);
s32 wmap_land_effect_11_run_sequence_2(s32 arg0);
s32 wmap_land_effect_11_run_sequence_9(s32 arg0);
void wmap_land_effect_11_sequence_1_step_02(void);
void wmap_land_effect_11_sequence_2_step_02(void);
void wmap_land_effect_11_sequence_3_step_04(void);
void wmap_land_effect_11_sequence_4_step_02(void);
void wmap_land_effect_11_sequence_5_step_04(void);
void wmap_land_effect_11_sequence_9_step_02(void);
void wmap_land_effect_11_sequence_9_step_04(void);

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

extern s32 D_801B0FD0;
extern s32 g_wmap_land_effect_11_sequence_3_timer;
extern s32 g_wmap_land_effect_11_sequence_5_timer;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern u8* D_8011CF1C;
extern s32 g_wmap_land_effect_11_sequence_7_timer;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 g_wmap_land_effect_11_sequence_6_timer;
extern void *D_8011CF24;
extern void *D_8011CF28;
extern s32 D_80182DF0;
extern s32 g_wmap_land_effect_11_sequence_8_timer;
extern s32 g_wmap_land_effect_11_timer;
extern void (*D_800D5960[])(void);
extern void wmap_land_effect_11_step_03(void);
extern void wmap_land_effect_11_end(void);
extern s32 g_wmap_land_effect_11_timeline_timer;
extern void (*D_800D5978[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 g_wmap_land_effect_11_sequence_1_timer;
extern void (*D_800D59B0[])(void);
extern u8* D_801399AC;
extern void wmap_land_effect_11_sequence_1_step_02(void);
extern s32 g_wmap_land_effect_11_sequence_2_timer;
extern void (*D_800D59C0[])(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern void wmap_land_effect_11_sequence_2_step_02(void);
extern void (*D_800D59D0[])(void);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32 g_wmap_land_effect_11_sequence_4_timer;
extern void (*D_800D59E8[])(void);
extern void *D_801399BC;
extern void (*D_800D59F8[])(void);
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern void wmap_land_effect_11_sequence_5_step_04(void);
extern void (*D_800D5A10[])(void);
extern void (*D_800D5A20[])(void);
extern void (*D_800D5A30[])(void);
extern s32 g_wmap_land_effect_11_sequence_9_timer;
extern void (*D_800D5A48[])(void);
extern u8* D_801399CC;
extern void wmap_land_effect_11_sequence_9_step_02(void);
extern void wmap_land_effect_11_sequence_9_step_04(void);

extern u8 D_8011D538[];
extern u32 g_wmap_land_effect_11_sequence_3_step;
extern u32 g_wmap_land_effect_11_sequence_5_step;
extern u32 g_wmap_land_effect_11_sequence_7_step;
extern u32 g_wmap_land_effect_11_sequence_6_step;

extern u32 g_wmap_land_effect_11_sequence_8_step;
extern u32 g_wmap_land_effect_11_step;
extern u32 g_wmap_land_effect_11_timeline_step;
extern u32 g_wmap_land_effect_11_sequence_1_step;
extern u32 g_wmap_land_effect_11_sequence_2_step;
extern u32 g_wmap_land_effect_11_sequence_4_step;
extern u32 g_wmap_land_effect_11_sequence_9_step;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;

extern SVECTOR D_80139258;
extern SVECTOR D_801B2498;
extern SVECTOR D_801B24A0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B24A8;

extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370;
extern WmapSpriteActor D_800D93C8;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8;
extern WmapAnimationSlot D_801399C8;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* D_80139280;

extern WmapSlot14 D_801AFBD0[];

/** @brief World-map step: fill a spawn descriptor, clear its slot run, then advance. */
void wmap_land_effect_11_sequence_3_step_01(void)
{
    s32 i;

    D_801B0FD0 = 0x46;
    D_80139280[1] = 2;
    D_80139280[2] = 8;
    D_80139280[3] = 0x40;
    D_80139280[4] = 2;
    D_80139280[5] = 2;
    D_80139280[6] = 0x190;
    D_80139280[7] = 0x14;
    D_80139280[8] = 0xD;
    D_80139280[9] = 1;
    D_80139280[10] = 0x3E8;
    for (i = 0; i < 0x46; i++)
    {
        D_801AFBD0[i + D_80139280[7]].field_00 = 0;
        D_80139988[i + 0x18].data = &D_8011D538;
    }
    g_wmap_land_effect_11_sequence_3_timer = 0x8D;
    g_wmap_land_effect_11_sequence_3_step += 1;
    wmap_land_effect_11_sequence_3_step_02();
}

/** @brief World-map step: fill a spawn descriptor, clear its slot run, then advance. */
void wmap_land_effect_11_sequence_5_step_01(void)
{
    s32 i;

    D_801B0FD0 = 0xA;
    D_80139280[11] = 1;
    D_80139280[12] = 6;
    D_80139280[13] = 0x90;
    D_80139280[14] = 2;
    D_80139280[15] = 3;
    D_80139280[16] = 0x60;
    D_80139280[17] = 0x64;
    D_80139280[18] = 0xD;
    D_80139280[19] = 0;
    D_80139280[20] = 0x2EE0;
    for (i = 0; i < 0xA; i++)
    {
        D_801AFBD0[i + D_80139280[17]].field_00 = 0;
        D_80139988[i + 0x68].data = &D_8011D538;
    }
    g_wmap_land_effect_11_sequence_5_timer = 0x29;
    g_wmap_land_effect_11_sequence_5_step += 1;
    wmap_land_effect_11_sequence_5_step_02();
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_11_sequence_6_step_02(void)
{
    MATRIX m;
    s32 x;

    x = D_801B2650.vz - 0xDAC;
    D_801B2650.vz = x;
    if (x < 0x2710)
    {
        D_801B2650.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_801B24A0, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DE8 != 0)
    {
        wmap_draw_model_default((s32)D_800DCF18, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE8);
        D_80182DE8 -= 0x5;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_11_sequence_6_timer == 0)
    {
        g_wmap_land_effect_11_sequence_6_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_11_sequence_7_step_02(void)
{
    MATRIX m;
    s32 x;

    x = D_801B2478.vz - 0xDAC;
    D_801B2478.vz = x;
    if (x < 0x2710)
    {
        D_801B2478.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_801B24A8, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DEC != 0)
    {
        wmap_draw_model_default(D_8011CF1C, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DEC);
        D_80182DEC -= 0x2;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_11_sequence_7_timer == 0)
    {
        g_wmap_land_effect_11_sequence_7_step += 1;
    }
}

/** @brief Draw and brighten two rotating effect layers and advance their shared countdown. */
void wmap_land_effect_11_sequence_8_step_02(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(D_8011CF24, 0, 4, 54, 0x7900, 1, D_80182DF0);
    D_801B2490.vz = (u16)(D_801B2490.vz + 24);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(D_8011CF28, 0, 4, 54, 0x7900, 1, D_80182DF0);
    D_801B2498.vz = (u16)(D_801B2498.vz - 16);
    PopMatrix();
    intensity = D_80182DF0 + 2;
    D_80182DF0 = intensity;
    if (intensity >= 130)
    {
        D_80182DF0 = 129;
    }
    remaining = g_wmap_land_effect_11_sequence_8_timer - 1;
    g_wmap_land_effect_11_sequence_8_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_11_sequence_8_step++;
    }
}

/** @brief Draw two rotating effect layers and advance their shared countdown. */
void wmap_land_effect_11_sequence_8_step_04(void)
{
    s32 remaining;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(D_8011CF24, 0, 4, 54, 0x7900, 1, D_80182DF0);
    D_801B2490.vz = (u16)(D_801B2490.vz + 24);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(D_8011CF28, 0, 4, 54, 0x7900, 1, D_80182DF0);
    D_801B2498.vz = (u16)(D_801B2498.vz - 16);
    PopMatrix();
    remaining = g_wmap_land_effect_11_sequence_8_timer - 1;
    D_80182DF0 -= 2;
    g_wmap_land_effect_11_sequence_8_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_11_sequence_8_step++;
    }
}

WMAP_STEP_RUNNER(wmap_land_effect_11_run, D_800D5960, 0x6, g_wmap_land_effect_11_step, g_wmap_land_effect_11_timer)

WMAP_STEP_RESET(wmap_land_effect_11_reset, g_wmap_land_effect_11_step, g_wmap_land_effect_11_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_11_step_01, g_wmap_land_effect_11_step, wmap_run_land_focus, wmap_land_effect_11_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_11_wait_idle_02, g_wmap_land_effect_11_step, wmap_land_effect_11_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_11_step_03, g_wmap_land_effect_11_step, wmap_land_effect_11_run_timeline, wmap_land_effect_11_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_11_wait_idle_04, g_wmap_land_effect_11_step, wmap_land_effect_11_end)

WMAP_STEP_ADVANCE(wmap_land_effect_11_end, g_wmap_land_effect_11_step)

WMAP_STEP_RUNNER(wmap_land_effect_11_run_timeline, D_800D5978, 0xE, g_wmap_land_effect_11_timeline_step, g_wmap_land_effect_11_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_11_timeline_reset, g_wmap_land_effect_11_timeline_step, g_wmap_land_effect_11_timeline_timer)

/**
 * @brief Set up a world-map sub-scene: request assets and register its handler.
 */
void wmap_land_effect_11_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_play_sound(0x20, 0x80);
    wmap_start_sequence(wmap_land_effect_11_run_sequence_3);
    g_wmap_land_effect_11_timeline_timer = 8;
    g_wmap_land_effect_11_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_11_timeline_wait_02, g_wmap_land_effect_11_timeline_step, g_wmap_land_effect_11_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_11_timeline_step_03, g_wmap_land_effect_11_timeline_step, g_wmap_land_effect_11_timeline_timer,
                         wmap_land_effect_11_run_sequence_4, 0x1E)

WMAP_STEP_WAIT(wmap_land_effect_11_timeline_wait_04, g_wmap_land_effect_11_timeline_step, g_wmap_land_effect_11_timeline_timer)

/** @brief Register world-map callbacks, seed a mode value, and advance step counters. */
void wmap_land_effect_11_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_11_run_sequence_6);
    wmap_start_map_tint(0x801045);
    g_wmap_backdrop_target_level = 4;
    wmap_start_sequence(wmap_land_effect_11_run_sequence_1);
    g_wmap_land_effect_11_timeline_timer = 2;
    g_wmap_land_effect_11_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_11_timeline_wait_06, g_wmap_land_effect_11_timeline_step, g_wmap_land_effect_11_timeline_timer)

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void wmap_land_effect_11_timeline_step_07(void)
{
    D_801ADAE0 = 1;
    wmap_start_sequence(wmap_land_effect_11_run_sequence_8);
    g_wmap_land_effect_11_timeline_timer = 0x3C;
    g_wmap_land_effect_11_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_11_timeline_wait_08, g_wmap_land_effect_11_timeline_step, g_wmap_land_effect_11_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_11_timeline_step_09, g_wmap_land_effect_11_timeline_step, g_wmap_land_effect_11_timeline_timer,
                             wmap_land_effect_11_run_sequence_7, wmap_land_effect_11_run_sequence_5, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_11_timeline_wait_10, g_wmap_land_effect_11_timeline_step, g_wmap_land_effect_11_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_11_timeline_step_11, g_wmap_land_effect_11_timeline_step, g_wmap_land_effect_11_timeline_timer,
                             wmap_land_effect_11_run_sequence_2, wmap_land_effect_11_run_sequence_9, 0xBF)

WMAP_STEP_WAIT(wmap_land_effect_11_timeline_wait_12, g_wmap_land_effect_11_timeline_step, g_wmap_land_effect_11_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_11_timeline_finish, g_wmap_land_effect_11_timeline_step, D_80139290, D_8011D510, D_8011D530, D_8011D4FC)

WMAP_STEP_RUNNER(wmap_land_effect_11_run_sequence_1, D_800D59B0, 0x4, g_wmap_land_effect_11_sequence_1_step, g_wmap_land_effect_11_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_11_sequence_1_reset, g_wmap_land_effect_11_sequence_1_step, g_wmap_land_effect_11_sequence_1_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_11_sequence_1_step_01(void)
{
    D_801399AC = D_8011D538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.sequence = 3;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 8;
    D_800D9318.resource_index = 0;
    D_800D9318.target_shade = 0x80;
    D_800D9318.shade = 0;
    g_wmap_land_effect_11_sequence_1_timer = 0x40;
    g_wmap_land_effect_11_sequence_1_step += 1;
    wmap_land_effect_11_sequence_1_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_11_sequence_1_step_02, g_wmap_land_effect_11_sequence_1_step, g_wmap_land_effect_11_sequence_1_timer, D_800D9318,
                              D_801399A8, g_wmap_focus_screen_position, 0xD, 0x1E, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_11_sequence_1_end, g_wmap_land_effect_11_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_11_run_sequence_2, D_800D59C0, 0x4, g_wmap_land_effect_11_sequence_2_step, g_wmap_land_effect_11_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_11_sequence_2_reset, g_wmap_land_effect_11_sequence_2_step, g_wmap_land_effect_11_sequence_2_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_11_sequence_2_step_01(void)
{
    D_801399B4 = D_8011F538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 2;
    D_800D9344.target_shade = 0x81;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.shade = 1;
    g_wmap_land_effect_11_sequence_2_timer = 0xC0;
    g_wmap_land_effect_11_sequence_2_step += 1;
    wmap_land_effect_11_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_11_sequence_2_step_02, g_wmap_land_effect_11_sequence_2_step, g_wmap_land_effect_11_sequence_2_timer, D_800D9344,
                              D_801399B0, g_wmap_focus_screen_position, 0x18, 0xA, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_11_sequence_2_end, g_wmap_land_effect_11_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_11_run_sequence_3, D_800D59D0, 6, g_wmap_land_effect_11_sequence_3_step, g_wmap_land_effect_11_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_11_sequence_3_reset, g_wmap_land_effect_11_sequence_3_step, g_wmap_land_effect_11_sequence_3_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_11_sequence_3_step_02(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x46, 0, 0x7F, 0x2, 0, (s32)D_80139280);
    if (--g_wmap_land_effect_11_sequence_3_timer == 0)
    {
        g_wmap_land_effect_11_sequence_3_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_11_sequence_3_step_03(void)
{
    g_wmap_land_effect_11_sequence_3_timer = 0x40;
    D_80139280[5] = -1;
    g_wmap_land_effect_11_sequence_3_step += 1;
    wmap_land_effect_11_sequence_3_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_11_sequence_3_step_04(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x46, 0, 0x7F, 0x2, 0, (s32)D_80139280);
    if (--g_wmap_land_effect_11_sequence_3_timer == 0)
    {
        g_wmap_land_effect_11_sequence_3_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_11_sequence_3_end, g_wmap_land_effect_11_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_11_run_sequence_4, D_800D59E8, 0x4, g_wmap_land_effect_11_sequence_4_step, g_wmap_land_effect_11_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_11_sequence_4_reset, g_wmap_land_effect_11_sequence_4_step, g_wmap_land_effect_11_sequence_4_timer)

/** @brief Configure the world-map actor and advance to its draw step. */
void wmap_land_effect_11_sequence_4_step_01(void)
{
    D_801399BC = &D_8011D538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.sequence = 2;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 2;
    D_800D9370.target_shade = 0x81;
    D_800D9370.resource_index = 0;
    D_800D9370.shade = 1;
    g_wmap_land_effect_11_sequence_4_timer = 0x62;
    g_wmap_land_effect_11_sequence_4_step += 1;
    wmap_land_effect_11_sequence_4_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_11_sequence_4_step_02, g_wmap_land_effect_11_sequence_4_step, g_wmap_land_effect_11_sequence_4_timer, D_800D9370,
                              D_801399B8, g_wmap_focus_screen_position, 0xD, 0x3, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_11_sequence_4_end, g_wmap_land_effect_11_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_11_run_sequence_5, D_800D59F8, 0x6, g_wmap_land_effect_11_sequence_5_step, g_wmap_land_effect_11_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_11_sequence_5_reset, g_wmap_land_effect_11_sequence_5_step, g_wmap_land_effect_11_sequence_5_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_11_sequence_5_step_02(void)
{
    func_8006A2FC(D_800DA448, D_80139CC8, 0xA, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--g_wmap_land_effect_11_sequence_5_timer == 0)
    {
        g_wmap_land_effect_11_sequence_5_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_11_sequence_5_step_03(void)
{
    g_wmap_land_effect_11_sequence_5_timer = 0x20;
    D_80139280[15] = -1;
    g_wmap_land_effect_11_sequence_5_step += 1;
    wmap_land_effect_11_sequence_5_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_11_sequence_5_step_04(void)
{
    func_8006A2FC(D_800DA448, D_80139CC8, 0xA, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--g_wmap_land_effect_11_sequence_5_timer == 0)
    {
        g_wmap_land_effect_11_sequence_5_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_11_sequence_5_end, g_wmap_land_effect_11_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_11_run_sequence_6, D_800D5A10, 0x4, g_wmap_land_effect_11_sequence_6_step, g_wmap_land_effect_11_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_11_sequence_6_reset, g_wmap_land_effect_11_sequence_6_step, g_wmap_land_effect_11_sequence_6_timer)

WMAP_STEP_DROP_START(wmap_land_effect_11_sequence_6_step_01, g_wmap_land_effect_11_sequence_6_step, g_wmap_land_effect_11_sequence_6_timer, D_801B24A0,
                     D_80139258, D_801B2650, D_80182DE8, 0x80, 0xAFC8, 0x28, wmap_land_effect_11_sequence_6_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_11_sequence_6_end, g_wmap_land_effect_11_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_11_run_sequence_7, D_800D5A20, 0x4, g_wmap_land_effect_11_sequence_7_step, g_wmap_land_effect_11_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_11_sequence_7_reset, g_wmap_land_effect_11_sequence_7_step, g_wmap_land_effect_11_sequence_7_timer)

WMAP_STEP_DROP_START(wmap_land_effect_11_sequence_7_step_01, g_wmap_land_effect_11_sequence_7_step, g_wmap_land_effect_11_sequence_7_timer, D_801B24A8,
                     D_80139258, D_801B2478, D_80182DEC, 0x80, 0xAFC8, 0x40, wmap_land_effect_11_sequence_7_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_11_sequence_7_end, g_wmap_land_effect_11_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_11_run_sequence_8, D_800D5A30, 0x6, g_wmap_land_effect_11_sequence_8_step, g_wmap_land_effect_11_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_11_sequence_8_reset, g_wmap_land_effect_11_sequence_8_step, g_wmap_land_effect_11_sequence_8_timer)

/** @brief Clear two rotation vectors, set the flag, and start a 128-tick sequence step. */
void wmap_land_effect_11_sequence_8_step_01(void)
{
    D_80182DF0 = 1;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    D_801B2498.vx = 0;
    D_801B2498.vy = 0;
    D_801B2498.vz = 0;
    g_wmap_land_effect_11_sequence_8_timer = 0x80;
    g_wmap_land_effect_11_sequence_8_step += 1;
    wmap_land_effect_11_sequence_8_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_11_sequence_8_step_03, g_wmap_land_effect_11_sequence_8_step, g_wmap_land_effect_11_sequence_8_timer, 0x40,
                    wmap_land_effect_11_sequence_8_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_11_sequence_8_end, g_wmap_land_effect_11_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_11_run_sequence_9, D_800D5A48, 0x6, g_wmap_land_effect_11_sequence_9_step, g_wmap_land_effect_11_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_11_sequence_9_reset, g_wmap_land_effect_11_sequence_9_step, g_wmap_land_effect_11_sequence_9_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_11_sequence_9_step_01(void)
{
    D_801399CC = D_8011F538;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.sequence = 1;
    D_800D93C8.previous_sequence = -1;
    D_800D93C8.shade_step = 2;
    D_800D93C8.resource_index = 0;
    D_800D93C8.target_shade = 0x81;
    D_800D93C8.shade = 0x81;
    g_wmap_land_effect_11_sequence_9_timer = 0xBF;
    g_wmap_land_effect_11_sequence_9_step += 1;
    wmap_land_effect_11_sequence_9_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_11_sequence_9_step_02, g_wmap_land_effect_11_sequence_9_step, g_wmap_land_effect_11_sequence_9_timer, D_800D93C8,
                              D_801399C8, g_wmap_focus_screen_position, 0x18, 0x9, 0)

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_11_sequence_9_step_03(void)
{
    D_800D93C8.shade_step = 16;
    D_800D93C8.target_shade = 0;
    g_wmap_land_effect_11_sequence_9_timer = 0x8;
    g_wmap_land_effect_11_sequence_9_step += 1;
    wmap_land_effect_11_sequence_9_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_11_sequence_9_step_04, g_wmap_land_effect_11_sequence_9_step, g_wmap_land_effect_11_sequence_9_timer, D_800D93C8,
                              D_801399C8, g_wmap_focus_screen_position, 0x18, 0x9, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_11_sequence_9_end, g_wmap_land_effect_11_sequence_9_step)
