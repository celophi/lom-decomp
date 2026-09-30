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

/** @brief World-map cell record. */
typedef struct
{
    u32 value;
    u8 pad_04[36];
} WmapEffectCell;

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

extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern u8* D_8011CF1C;
extern s32 g_wmap_land_effect_27_sequence_4_timer;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 g_wmap_land_effect_27_sequence_3_timer;
extern s32 D_800D665C[];
extern void *D_8011CF24;
extern void *D_8011CF28;
extern void *D_8011CF2C;
extern s32 D_80139234;
extern s32 D_80182DF0;
extern s32 g_wmap_land_effect_27_sequence_5_timer;
extern s32 D_801B0FD0;
extern s32 g_wmap_land_effect_27_sequence_6_timer;
extern u8 D_80121538[];
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
extern s32 D_8013B208;
extern s32 D_80139244;
extern s32 D_801ADAE0;
extern WmapEffectCell D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 g_wmap_land_effect_27_sequence_1_timer;
extern void (*D_800D661C[])(void);
extern void *D_801399AC;
extern s32 g_wmap_land_effect_27_sequence_2_timer;
extern void (*D_800D662C[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern void wmap_land_effect_27_sequence_2_step_02(void);
extern void (*D_800D663C[])(void);
extern void (*D_800D664C[])(void);
extern void (*D_800D6674[])(void);
extern void (*D_800D668C[])(void);
extern u8 D_800DAB28[];
extern u8 D_80139E08[];
extern void wmap_land_effect_27_sequence_6_step_04(void);
extern s32 g_wmap_land_effect_27_sequence_7_timer;
extern void (*D_800D66A4[])(void);
extern u8* D_801399BC;
extern void wmap_land_effect_27_sequence_7_step_02(void);
extern void wmap_land_effect_27_sequence_7_step_04(void);
extern void (*D_800D66BC[])(void);
extern u8 D_800D95D8[];
extern WmapAnimationSlot D_80139A28[];
extern void (*D_800D66D4[])(void);
extern u8 D_800D9CB8[];
extern u8 D_80139B68[];
extern void wmap_land_effect_27_sequence_9_step_04(void);
extern void (*D_800D66EC[])(void);
extern u8 D_800DA398[];
extern u8 D_80139CA8[];
extern void wmap_land_effect_27_sequence_10_step_04(void);
extern void (*D_800D6704[])(void);
extern u8 D_800DB158[];
extern u8 D_80139F28[];
extern void wmap_land_effect_27_sequence_11_step_04(void);
extern s32 g_wmap_land_effect_27_sequence_12_timer;
extern void (*D_800D671C[])(void);
extern u8* D_801399CC;
extern void wmap_land_effect_27_sequence_12_step_02(void);
extern u32 g_wmap_land_effect_27_sequence_4_step;
extern u32 g_wmap_land_effect_27_sequence_3_step;

extern u32 g_wmap_land_effect_27_sequence_5_step;
extern u32 g_wmap_land_effect_27_sequence_6_step;
extern u32 g_wmap_land_effect_27_sequence_8_step;
extern u32 g_wmap_land_effect_27_sequence_9_step;
extern u32 g_wmap_land_effect_27_sequence_10_step;
extern u8 D_8011F538[];
extern u32 g_wmap_land_effect_27_sequence_11_step;
extern u32 g_wmap_land_effect_27_step;
extern u32 g_wmap_land_effect_27_timeline_step;
extern u32 g_wmap_land_effect_27_sequence_1_step;
extern u32 g_wmap_land_effect_27_sequence_2_step;
extern u32 g_wmap_land_effect_27_sequence_7_step;
extern u32 g_wmap_land_effect_27_sequence_12_step;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;

extern SVECTOR D_80139258;
extern SVECTOR D_801B24A0;
extern SVECTOR D_8013B238;
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

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_27_sequence_3_step_02(void)
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
        D_80182DE8 -= 0x4;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_27_sequence_3_timer == 0)
    {
        g_wmap_land_effect_27_sequence_3_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_27_sequence_4_step_02(void)
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
        D_80182DEC -= 0x4;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_27_sequence_4_timer == 0)
    {
        g_wmap_land_effect_27_sequence_4_step += 1;
    }
}

/** @brief Draw three rotating effect layers and update their fade and animation. */
void wmap_land_effect_27_sequence_5_step_02(void)
{
    s32 intensity;
    s32 frame;
    s32 remaining;
    u16 angle;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF24, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -10, -1);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B24A8);
    wmap_draw_model(D_8011CF28, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -40, -1);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF2C, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -70, -1);
    PopMatrix();
    intensity = D_80182DF0 + 2;
    D_80182DF0 = intensity;
    if (intensity >= 130)
    {
        D_80182DF0 = 129;
    }
    angle = D_8013B238.vz;
    D_8013B238.vz = angle + 330;
    D_801B24A8.vz += 10;
    D_8013B238.vz = angle + 340;
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
    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF24, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -10, -1);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B24A8);
    wmap_draw_model(D_8011CF28, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -40, -1);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF2C, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -70, -1);
    PopMatrix();
    intensity = D_80182DF0 - 4;
    D_80182DF0 = intensity;
    if (intensity < 0)
    {
        D_80182DF0 = 0;
    }
    angle = D_8013B238.vz;
    D_8013B238.vz = angle + 330;
    D_801B24A8.vz += 10;
    D_8013B238.vz = angle + 340;
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

    D_801B0FD0 = 40;
    D_80139280[0x1F] = 1;
    D_80139280[0x20] = 4;
    D_80139280[0x21] = 32;
    D_80139280[0x22] = 0;
    D_80139280[0x23] = 2;
    D_80139280[0x24] = 0;
    D_80139280[0x25] = 140;
    D_80139280[0x26] = 8;
    D_80139280[0x27] = 3;
    D_80139280[0x28] = 8000;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + 140].field_00 = 0;
        D_80139988[i + 144].data = D_80121538;
    }
    g_wmap_land_effect_27_sequence_6_timer = 80;
    g_wmap_land_effect_27_sequence_6_step++;
    wmap_land_effect_27_sequence_6_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_27_sequence_8_step_01(void)
{
    s32 i;

    D_80139280[0x1] = 1;
    D_80139280[0x2] = 4;
    D_80139280[0x3] = 0x20;
    D_80139280[0x4] = 0;
    D_80139280[0x5] = 2;
    D_80139280[0x6] = 0;
    D_80139280[0x7] = 0x14;
    D_80139280[0x8] = 8;
    D_80139280[0x9] = 0;
    D_80139280[0xA] = 0x2EE0;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + 20].field_00 = 0;
        D_80139988[i + 20].data = D_80121538;
    }
    g_wmap_land_effect_27_sequence_8_timer = 80;
    g_wmap_land_effect_27_sequence_8_step++;
    wmap_land_effect_27_sequence_8_step_02();
}

/** @brief World-map step: init hero struct fields, clear tables, advance step. */
void wmap_land_effect_27_sequence_9_step_01(void)
{
    s32 i;
    void *base = D_80139280;

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
        D_801AFBD0[60 + i].field_00 = 0;
        D_80139988[60 + i].data = D_80121538;
    }

    g_wmap_land_effect_27_sequence_9_timer = 0x38;
    g_wmap_land_effect_27_sequence_9_step += 1;
    wmap_land_effect_27_sequence_9_step_02();
}

/** @brief World-map step handler: configure a model descriptor and clear two entry tables, then advance. */
void wmap_land_effect_27_sequence_10_step_01(void)
{
    s32 i;

    D_80139280[0x15] = 1;
    D_80139280[0x17] = 0x20;
    D_80139280[0x1A] = 0x3E8;
    D_80139280[0x1B] = 0x64;
    D_80139280[0x1C] = 8;
    D_80139280[0x1D] = 2;
    D_80139280[0x16] = 1;
    D_80139280[0x18] = 0;
    D_80139280[0x19] = 1;
    D_80139280[0x1E] = 0x6D60;

    for (i = 0; i < 20; i++)
    {
        D_801AFBD0[100 + i].field_00 = 0;
        D_80139988[100 + i].data = D_80121538;
    }

    g_wmap_land_effect_27_sequence_10_timer = 20;
    g_wmap_land_effect_27_sequence_10_step += 1;
    wmap_land_effect_27_sequence_10_step_02();
}

/** @brief World-map step: init hero struct fields, clear tables, advance step. */
void wmap_land_effect_27_sequence_11_step_01(void)
{
    s32 i;

    D_80139280[0x29] = 1;
    D_80139280[0x2A] = 4;
    D_80139280[0x2B] = 0x20;
    D_80139280[0x2E] = 0x3E8;
    D_80139280[0x2F] = 0xB4;
    D_80139280[0x30] = 0x15;
    D_80139280[0x31] = 2;
    D_80139280[0x2C] = 0;
    D_80139280[0x2D] = 1;
    D_80139280[0x32] = 0x1F40;

    for (i = 0; i < 20; i++)
    {
        D_801AFBD0[180 + i].field_00 = 0;
        D_80139988[180 + i].data = D_8011F538;
    }

    g_wmap_land_effect_27_sequence_11_timer = 20;
    g_wmap_land_effect_27_sequence_11_step += 1;
    wmap_land_effect_27_sequence_11_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_27_run, D_800D65A4, 0x6, g_wmap_land_effect_27_step, g_wmap_land_effect_27_timer)

WMAP_STEP_RESET(wmap_land_effect_27_reset, g_wmap_land_effect_27_step, g_wmap_land_effect_27_timer)

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_27_step_01(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_27_step += 1;
    wmap_land_effect_27_wait_idle_02();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_27_wait_idle_02, g_wmap_land_effect_27_step, wmap_land_effect_27_step_03)

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_27_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_27_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_27_step += 1;
    wmap_land_effect_27_wait_idle_04();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_27_wait_idle_04, g_wmap_land_effect_27_step, wmap_land_effect_27_end)

WMAP_STEP_ADVANCE(wmap_land_effect_27_end, g_wmap_land_effect_27_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_timeline, D_800D65BC, 0x18, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_27_timeline_reset, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

/** @brief World-map step: mark active, request a resource, seed the sub-counter, tick. */
void wmap_land_effect_27_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_play_sound(0x2E, 0x80);
    g_wmap_land_effect_27_timeline_timer = 4;
    g_wmap_land_effect_27_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_02, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_27_timeline_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_27_run_sequence_6);
    g_wmap_land_effect_27_timeline_timer = 0xF;
    g_wmap_land_effect_27_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_04, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_27_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_27_run_sequence_9);
    g_wmap_land_effect_27_timeline_timer = 0xC;
    g_wmap_land_effect_27_timeline_step += 1;
}

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
    D_80139244 = 1;
    g_wmap_land_effect_27_timeline_timer = 0x14;
    g_wmap_land_effect_27_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_10, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_27_timeline_step_11(void)
{
    wmap_start_sequence(wmap_land_effect_27_run_sequence_8);
    wmap_start_sequence(wmap_land_effect_27_run_sequence_5);
    g_wmap_land_effect_27_timeline_timer = 0x28;
    g_wmap_land_effect_27_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_12, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

/** @brief Register two callbacks around the sequence flag update and begin a 30-tick delay. */
void wmap_land_effect_27_timeline_step_13(void)
{
    wmap_start_sequence(&wmap_land_effect_27_run_sequence_7);
    D_801ADAE0 = 1;
    wmap_start_sequence(&wmap_land_effect_27_run_sequence_1);
    g_wmap_land_effect_27_timeline_timer = 0x1E;
    g_wmap_land_effect_27_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_14, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_27_timeline_step_15(void)
{
    wmap_start_sequence(wmap_land_effect_27_run_sequence_12);
    g_wmap_land_effect_27_timeline_timer = 0x38;
    g_wmap_land_effect_27_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_16, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_27_timeline_step_17(void)
{
    wmap_start_sequence(wmap_land_effect_27_run_sequence_4);
    g_wmap_land_effect_27_timeline_timer = 0x2;
    g_wmap_land_effect_27_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_18, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

/** @brief Register a sequence callback, clear the world-map value, and start a 64-tick delay. */
void wmap_land_effect_27_timeline_step_19(void)
{
    wmap_start_sequence(&wmap_land_effect_27_run_sequence_2);
    D_80139244 = 0;
    g_wmap_land_effect_27_timeline_timer = 0x40;
    g_wmap_land_effect_27_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_20, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_27_timeline_step_21(void)
{
    wmap_start_sequence(wmap_land_effect_27_run_sequence_11);
    wmap_start_sequence(wmap_land_effect_27_run_sequence_10);
    g_wmap_land_effect_27_timeline_timer = 0xAC;
    g_wmap_land_effect_27_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_27_timeline_wait_22, g_wmap_land_effect_27_timeline_step, g_wmap_land_effect_27_timeline_timer)

void wmap_land_effect_27_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    g_wmap_land_effect_27_timeline_step += 1;
}

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_1, D_800D661C, 0x4, g_wmap_land_effect_27_sequence_1_step, g_wmap_land_effect_27_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_1_reset, g_wmap_land_effect_27_sequence_1_step, g_wmap_land_effect_27_sequence_1_timer)

void wmap_land_effect_27_sequence_1_step_01(void)
{
    D_801399AC = &D_8011F538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.sequence = 1;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 8;
    D_800D9318.resource_index = 0;
    D_800D9318.target_shade = 0x81;
    D_800D9318.shade = 1;
    g_wmap_land_effect_27_sequence_1_timer = 0x24;
    g_wmap_land_effect_27_sequence_1_step += 1;
    wmap_land_effect_27_sequence_1_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_1_step_02(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0x15, 0x2, 0);
    if (--g_wmap_land_effect_27_sequence_1_timer == 0)
    {
        g_wmap_land_effect_27_sequence_1_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_1_end, g_wmap_land_effect_27_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_2, D_800D662C, 0x4, g_wmap_land_effect_27_sequence_2_step, g_wmap_land_effect_27_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_2_reset, g_wmap_land_effect_27_sequence_2_step, g_wmap_land_effect_27_sequence_2_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_27_sequence_2_step_01(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.sequence = 1;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 2;
    D_800D9344.resource_index = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0;
    g_wmap_land_effect_27_sequence_2_timer = 0xEE;
    g_wmap_land_effect_27_sequence_2_step += 1;
    wmap_land_effect_27_sequence_2_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_2_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x12, 0x5, 0);
    if (--g_wmap_land_effect_27_sequence_2_timer == 0)
    {
        g_wmap_land_effect_27_sequence_2_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_2_end, g_wmap_land_effect_27_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_3, D_800D663C, 0x4, g_wmap_land_effect_27_sequence_3_step, g_wmap_land_effect_27_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_3_reset, g_wmap_land_effect_27_sequence_3_step, g_wmap_land_effect_27_sequence_3_timer)

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_27_sequence_3_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    g_wmap_land_effect_27_sequence_3_timer = 0x20;
    g_wmap_land_effect_27_sequence_3_step += 1;
    wmap_land_effect_27_sequence_3_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_3_end, g_wmap_land_effect_27_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_4, D_800D664C, 0x4, g_wmap_land_effect_27_sequence_4_step, g_wmap_land_effect_27_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_4_reset, g_wmap_land_effect_27_sequence_4_step, g_wmap_land_effect_27_sequence_4_timer)

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_27_sequence_4_step_01(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    g_wmap_land_effect_27_sequence_4_timer = 0x20;
    g_wmap_land_effect_27_sequence_4_step += 1;
    wmap_land_effect_27_sequence_4_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_4_end, g_wmap_land_effect_27_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_5, D_800D6674, 0x6, g_wmap_land_effect_27_sequence_5_step, g_wmap_land_effect_27_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_5_reset, g_wmap_land_effect_27_sequence_5_step, g_wmap_land_effect_27_sequence_5_timer)

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void wmap_land_effect_27_sequence_5_step_01(void)
{
    D_80182DF0 = 1;
    D_8013B238 = D_80139258;
    D_80139234 = 0;
    g_wmap_land_effect_27_sequence_5_timer = 0x60;
    g_wmap_land_effect_27_sequence_5_step += 1;
    wmap_land_effect_27_sequence_5_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_27_sequence_5_step_03(void)
{
    g_wmap_land_effect_27_sequence_5_timer = 0x20;
    g_wmap_land_effect_27_sequence_5_step += 1;
    wmap_land_effect_27_sequence_5_step_04();
}

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_5_end, g_wmap_land_effect_27_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_6, D_800D668C, 0x6, g_wmap_land_effect_27_sequence_6_step, g_wmap_land_effect_27_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_6_reset, g_wmap_land_effect_27_sequence_6_step, g_wmap_land_effect_27_sequence_6_timer)

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_6_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DAB28, D_80139E08, 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x78));
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
    D_80139280[35] = -1;
    g_wmap_land_effect_27_sequence_6_step += 1;
    wmap_land_effect_27_sequence_6_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_6_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DAB28, D_80139E08, 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x78));
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
    D_801399BC = D_8011F538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 4;
    D_800D9370.target_shade = 0x81;
    D_800D9370.resource_index = 0;
    D_800D9370.sequence = 0;
    D_800D9370.shade = 1;
    g_wmap_land_effect_27_sequence_7_timer = 0x80;
    g_wmap_land_effect_27_sequence_7_step += 1;
    wmap_land_effect_27_sequence_7_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_7_step_02(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x15, 0x5, 0);
    if (--g_wmap_land_effect_27_sequence_7_timer == 0)
    {
        g_wmap_land_effect_27_sequence_7_step += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_27_sequence_7_step_03(void)
{
    D_800D9370.shade_step = 4;
    D_800D9370.target_shade = 0;
    g_wmap_land_effect_27_sequence_7_timer = 0x80;
    g_wmap_land_effect_27_sequence_7_step += 1;
    wmap_land_effect_27_sequence_7_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_7_step_04(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x15, 0x5, 0);
    if (--g_wmap_land_effect_27_sequence_7_timer == 0)
    {
        g_wmap_land_effect_27_sequence_7_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_7_end, g_wmap_land_effect_27_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_27_run_sequence_8, D_800D66BC, 0x6, g_wmap_land_effect_27_sequence_8_step, g_wmap_land_effect_27_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_27_sequence_8_reset, g_wmap_land_effect_27_sequence_8_step, g_wmap_land_effect_27_sequence_8_timer)

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_8_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0x28, 0xFF, 0x1, 0x8, 0, (s32)D_80139280);
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
    D_80139280[5] = -1;
    g_wmap_land_effect_27_sequence_8_step += 1;
    wmap_land_effect_27_sequence_8_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_8_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0x28, 0xFF, 0x1, 0x8, 0, (s32)D_80139280);
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
    func_8006A2FC(D_800D9CB8, D_80139B68, 0x18, 0xFF, 0x1, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
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
    D_80139280[15] = -1;
    g_wmap_land_effect_27_sequence_9_step += 1;
    wmap_land_effect_27_sequence_9_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_9_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D9CB8, D_80139B68, 0x18, 0xFF, 0x1, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
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
    func_8006A2FC(D_800DA398, D_80139CA8, 0x14, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x50));
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
    D_80139280[25] = -1;
    g_wmap_land_effect_27_sequence_10_step += 1;
    wmap_land_effect_27_sequence_10_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_10_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA398, D_80139CA8, 0x14, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x50));
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
    func_8006A2FC(D_800DB158, D_80139F28, 0x14, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0xA0));
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
    D_80139280[45] = -1;
    g_wmap_land_effect_27_sequence_11_step += 1;
    wmap_land_effect_27_sequence_11_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_11_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB158, D_80139F28, 0x14, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0xA0));
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
    D_801399CC = D_8011D538;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.previous_sequence = -1;
    D_800D93C8.shade_step = 2;
    D_800D93C8.target_shade = 0x81;
    D_800D93C8.resource_index = 0;
    D_800D93C8.sequence = 0;
    D_800D93C8.shade = 1;
    g_wmap_land_effect_27_sequence_12_timer = 0x5A;
    g_wmap_land_effect_27_sequence_12_step += 1;
    wmap_land_effect_27_sequence_12_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_27_sequence_12_step_02(void)
{
    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, g_wmap_focus_screen_position.packed, 0x1E, 0x3, 0);
    if (--g_wmap_land_effect_27_sequence_12_timer == 0)
    {
        g_wmap_land_effect_27_sequence_12_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_27_sequence_12_end, g_wmap_land_effect_27_sequence_12_step)
