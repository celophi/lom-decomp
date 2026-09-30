#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_land_effect_00.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"
#include "wmap_step_sequence.h"

/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

void wmap_land_effect_00_sequence_7_step_02(void);
void wmap_land_effect_00_sequence_8_step_02(void);
void wmap_land_effect_00_wait_idle_02(void);
void wmap_land_effect_00_step_03(void);
s32 wmap_land_effect_00_run_timeline(s32 arg0);
void wmap_land_effect_00_wait_idle_04(void);
void wmap_land_effect_00_step_05(void);
s32 wmap_land_effect_00_run_sequence_9(s32 arg0);
s32 wmap_land_effect_00_run_sequence_4(s32 arg0);
s32 wmap_land_effect_00_run_sequence_1(s32 arg0);
s32 wmap_land_effect_00_run_sequence_7(s32 arg0);
s32 wmap_land_effect_00_run_sequence_8(s32 arg0);
s32 wmap_land_effect_00_run_sequence_6(s32 arg0);
s32 wmap_land_effect_00_run_sequence_2(s32 arg0);
s32 wmap_land_effect_00_run_sequence_5(s32 arg0);
s32 wmap_land_effect_00_run_sequence_3(s32 arg0);
void wmap_land_effect_00_sequence_1_step_02(void);
void wmap_land_effect_00_sequence_2_step_02(void);
void wmap_land_effect_00_sequence_2_step_04(void);
void wmap_land_effect_00_sequence_3_step_02(void);
void wmap_land_effect_00_sequence_7_step_04(void);
void wmap_land_effect_00_sequence_7_step_06(void);
void wmap_land_effect_00_sequence_8_step_04(void);
void wmap_land_effect_00_sequence_9_step_02(void);
void wmap_land_effect_00_sequence_9_step_04(void);

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8* D_8011CF1C;
extern s32 g_wmap_land_effect_00_sequence_4_timer;
extern s32 D_80182DEC;
extern u8* D_8011CF24;
extern s32 g_wmap_land_effect_00_sequence_5_timer;
extern u8 D_800DCF18[];
extern s32 g_wmap_land_effect_00_sequence_6_timer;
extern s32 D_80182DF4;
extern s32 D_80139234;
extern s32 D_80182DF0;
extern s32 D_800DCEA8;
extern s32 D_800D9150;
extern s32 g_wmap_land_effect_00_sequence_7_timer;
extern s32 D_801B0FD0;
extern s32 g_wmap_land_effect_00_sequence_8_timer;
extern s32 g_wmap_land_effect_00_timer;
extern void (*D_800D6028[])(void);
extern void wmap_land_effect_00_step_03(void);
extern s32 g_wmap_land_effect_00_timeline_timer;
extern void (*D_800D6048[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 g_wmap_land_effect_00_sequence_1_timer;
extern void (*D_800D6090[])(void);
extern u8* D_801399AC;
extern u8 D_8011F538[];
extern s32 g_wmap_land_effect_00_sequence_2_timer;
extern void (*D_800D60A0[])(void);
extern u8* D_801399CC;
extern void wmap_land_effect_00_sequence_2_step_02(void);
extern void wmap_land_effect_00_sequence_2_step_04(void);
extern s32 g_wmap_land_effect_00_sequence_3_timer;
extern void (*D_800D60B8[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern void wmap_land_effect_00_sequence_3_step_02(void);
extern void (*D_800D60C8[])(void);
extern void (*D_800D60D8[])(void);
extern void (*D_800D60E8[])(void);
extern void (*D_800D6100[])(void);
extern void (*D_800D6120[])(void);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern void wmap_land_effect_00_sequence_8_step_04(void);
extern s32 g_wmap_land_effect_00_sequence_9_timer;
extern void (*D_800D6138[])(void);
extern u8 D_80123538[];
extern u8* D_801399D4;
extern void wmap_land_effect_00_sequence_9_step_02(void);
extern void wmap_land_effect_00_sequence_9_step_04(void);

extern u32 g_wmap_land_effect_00_sequence_4_step;
extern u32 g_wmap_land_effect_00_sequence_5_step;

extern u32 g_wmap_land_effect_00_sequence_6_step;
extern u8 D_80121538[];
extern u32 g_wmap_land_effect_00_sequence_7_step;
extern u32 g_wmap_land_effect_00_sequence_8_step;
extern u32 g_wmap_land_effect_00_step;
extern u32 g_wmap_land_effect_00_timeline_step;
extern u32 g_wmap_land_effect_00_sequence_1_step;
extern u32 g_wmap_land_effect_00_sequence_2_step;
extern u32 g_wmap_land_effect_00_sequence_3_step;
extern u32 g_wmap_land_effect_00_sequence_9_step;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;

extern SVECTOR D_80139258;
extern SVECTOR D_8013B240;
extern SVECTOR D_801B24A0;
extern SVECTOR D_801B24A8;

extern WmapSpriteActor D_800D9268[];
extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D93C8;
extern WmapSpriteActor D_800D93F4;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399C8;
extern WmapAnimationSlot D_801399D0;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* D_80139280;

extern WmapSlot14 D_801AFBD0[];

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_00_sequence_4_step_02(void)
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
        wmap_draw_model_default(D_8011CF1C, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE8);
        D_80182DE8 -= 0x4;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_00_sequence_4_timer == 0)
    {
        g_wmap_land_effect_00_sequence_4_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_00_sequence_5_step_02(void)
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
        wmap_draw_model_default(D_8011CF24, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DEC);
        D_80182DEC -= 0x2;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_00_sequence_5_timer == 0)
    {
        g_wmap_land_effect_00_sequence_5_step += 1;
    }
}

/** @brief Draw the expanding effect and advance its rotation and countdown. */
void wmap_land_effect_00_sequence_6_step_02(void)
{
    s32 scale;
    s32 intensity;
    s32 remaining;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B240);
    wmap_draw_model(D_800DCF18, 0, 10, 183, 0x7A40, 0x1001, D_80182DF4, 0, 5, D_80139234 / 16);
    scale = D_80139234 - 128;
    D_80139234 = scale;
    intensity = D_80182DF4 + 2;
    D_80182DF4 = intensity;
    if (intensity >= 130)
    {
        D_80182DF4 = 129;
    }
    if (scale < 16)
    {
        D_80139234 = 16;
    }
    PopMatrix();
    remaining = g_wmap_land_effect_00_sequence_6_timer - 1;
    D_8013B240.vz = (u16)(D_8013B240.vz + 220);
    g_wmap_land_effect_00_sequence_6_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_00_sequence_6_step++;
    }
}

/** @brief Draw and fade the rotating effect, then advance its countdown. */
void wmap_land_effect_00_sequence_6_step_04(void)
{
    s32 intensity;
    s32 remaining;

    if (D_80182DF4 != 0)
    {
        PushMatrix();
        wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B240);
        wmap_draw_model(D_800DCF18, 0, 10, 183, 0x7A40, 0x1001, D_80182DF4, 0, 5, D_80139234 / 16);
        intensity = D_80182DF4 - 4;
        D_80182DF4 = intensity;
        if (intensity < 0)
        {
            D_80182DF4 = 0;
        }
        PopMatrix();
        D_8013B240.vz = (u16)(D_8013B240.vz + 220);
    }
    remaining = g_wmap_land_effect_00_sequence_6_timer - 1;
    g_wmap_land_effect_00_sequence_6_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_00_sequence_6_step++;
    }
}

/**
 * @brief Reset a range of world-map actor slots and kick the next handler.
 * @note Clears three parallel slot arrays for indices [0x64, 0x7C), seeds each
 *       actor's default fields, then advances the shared step counters.
 */
void wmap_land_effect_00_sequence_7_step_01(void)
{
    s32 i;
    u8* pa;
    u8* pb;

    D_80182DF0 = 1;
    D_800DCEA8 = 1;
    for (i = 0x64; i < 0x7C; i++)
    {
        *(s16*)((u8*)D_801AFBD0 + i * 0x14) = 0;
        pb = (u8*)D_80139988 + i * 0x8;
        *(s32*)(pb + 0x4) = (s32)&D_80121538;
        pa = (u8*)D_800D9268 + i * 0x2C;
        *(s16*)(pa + 0x2) = 0;
        *(s8*)(pa + 0x6) = 0xF;
        *(s16*)(pa + 0xE) = 0;
        *(s16*)(pa + 0x10) = -1;
    }
    D_800D9150 = 4;
    g_wmap_land_effect_00_sequence_7_timer = 0x10;
    g_wmap_land_effect_00_sequence_7_step += 1;
    wmap_land_effect_00_sequence_7_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_00_sequence_8_step_01(void)
{
    s32 i;

    D_801B0FD0 = 25;
    D_80139280[0x1F] = 4;
    D_80139280[0x20] = 4;
    D_80139280[0x21] = 72;
    D_80139280[0x22] = 0;
    D_80139280[0x23] = 4;
    D_80139280[0x24] = 1500;
    D_80139280[0x25] = 20;
    D_80139280[0x26] = 19;
    D_80139280[0x27] = 1;
    D_80139280[0x28] = 10000;
    for (i = 0; i < 25; i++)
    {
        D_801AFBD0[i + D_80139280[0x25]].field_00 = 0;
        D_80139988[i + 24].data = D_80121538;
    }
    g_wmap_land_effect_00_sequence_8_timer = 100;
    g_wmap_land_effect_00_sequence_8_step++;
    wmap_land_effect_00_sequence_8_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_00_run, D_800D6028, 0x8, g_wmap_land_effect_00_step, g_wmap_land_effect_00_timer)

WMAP_STEP_RESET(wmap_land_effect_00_reset, g_wmap_land_effect_00_step, g_wmap_land_effect_00_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_00_step_01, g_wmap_land_effect_00_step, wmap_run_land_focus, wmap_land_effect_00_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_00_wait_idle_02, g_wmap_land_effect_00_step, wmap_land_effect_00_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_00_step_03, g_wmap_land_effect_00_step, wmap_land_effect_00_run_timeline, wmap_land_effect_00_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_00_wait_idle_04, g_wmap_land_effect_00_step, wmap_land_effect_00_step_05)

/**
 * @brief Reset a world-map step slot: arm its wait and advance the step index.
 */
void wmap_land_effect_00_step_05(void)
{
    g_wmap_land_effect_00_timer = 5;
    g_wmap_land_effect_00_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_00_wait_06, g_wmap_land_effect_00_step, g_wmap_land_effect_00_timer)

WMAP_STEP_ADVANCE(wmap_land_effect_00_end, g_wmap_land_effect_00_step)

WMAP_STEP_RUNNER(wmap_land_effect_00_run_timeline, D_800D6048, 0x12, g_wmap_land_effect_00_timeline_step, g_wmap_land_effect_00_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_00_timeline_reset, g_wmap_land_effect_00_timeline_step, g_wmap_land_effect_00_timeline_timer)

/**
 * @brief Set up a tinted world-map sub-scene and register its handler.
 */
void wmap_land_effect_00_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_start_map_tint(0x404045);
    g_wmap_backdrop_target_level = 8;
    wmap_play_sound(0x28, 0x80);
    wmap_start_sequence(wmap_land_effect_00_run_sequence_9);
    g_wmap_land_effect_00_timeline_timer = 0x1E;
    g_wmap_land_effect_00_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_00_timeline_wait_02, g_wmap_land_effect_00_timeline_step, g_wmap_land_effect_00_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_00_timeline_step_03, g_wmap_land_effect_00_timeline_step, g_wmap_land_effect_00_timeline_timer,
                             wmap_land_effect_00_run_sequence_4, wmap_land_effect_00_run_sequence_1, 0x4)

WMAP_STEP_WAIT(wmap_land_effect_00_timeline_wait_04, g_wmap_land_effect_00_timeline_step, g_wmap_land_effect_00_timeline_timer)

/** @brief World-map step: register a callback, set flags, advance the step. */
void wmap_land_effect_00_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_00_run_sequence_7);
    D_801ADAE0 = 1;
    wmap_start_map_tint(0x202540);
    g_wmap_backdrop_target_level = 3;
    g_wmap_land_effect_00_timeline_timer = 0x14;
    g_wmap_land_effect_00_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_00_timeline_wait_06, g_wmap_land_effect_00_timeline_step, g_wmap_land_effect_00_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_00_timeline_step_07, g_wmap_land_effect_00_timeline_step, g_wmap_land_effect_00_timeline_timer,
                         wmap_land_effect_00_run_sequence_8, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_00_timeline_wait_08, g_wmap_land_effect_00_timeline_step, g_wmap_land_effect_00_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_00_timeline_step_09, g_wmap_land_effect_00_timeline_step, g_wmap_land_effect_00_timeline_timer,
                         wmap_land_effect_00_run_sequence_6, 0x11)

WMAP_STEP_WAIT(wmap_land_effect_00_timeline_wait_10, g_wmap_land_effect_00_timeline_step, g_wmap_land_effect_00_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_00_timeline_step_11, g_wmap_land_effect_00_timeline_step, g_wmap_land_effect_00_timeline_timer,
                         wmap_land_effect_00_run_sequence_2, 0x26)

WMAP_STEP_WAIT(wmap_land_effect_00_timeline_wait_12, g_wmap_land_effect_00_timeline_step, g_wmap_land_effect_00_timeline_timer)

/** @brief World-map step handler: register a callback, kick a job, advance the step. */
void wmap_land_effect_00_timeline_step_13(void)
{
    wmap_start_sequence(wmap_land_effect_00_run_sequence_5);
    wmap_start_map_tint(0x404050);
    g_wmap_backdrop_target_level = 8;
    g_wmap_land_effect_00_timeline_timer = 4;
    g_wmap_land_effect_00_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_00_timeline_wait_14, g_wmap_land_effect_00_timeline_step, g_wmap_land_effect_00_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_00_timeline_step_15, g_wmap_land_effect_00_timeline_step, g_wmap_land_effect_00_timeline_timer,
                         wmap_land_effect_00_run_sequence_3, 0x9C)

WMAP_STEP_WAIT(wmap_land_effect_00_timeline_wait_16, g_wmap_land_effect_00_timeline_step, g_wmap_land_effect_00_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_00_timeline_finish, g_wmap_land_effect_00_timeline_step, D_80139290, D_8011D510, D_8011D530, D_8011D4FC)

WMAP_STEP_RUNNER(wmap_land_effect_00_run_sequence_1, D_800D6090, 0x4, g_wmap_land_effect_00_sequence_1_step, g_wmap_land_effect_00_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_00_sequence_1_reset, g_wmap_land_effect_00_sequence_1_step, g_wmap_land_effect_00_sequence_1_timer)

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void wmap_land_effect_00_sequence_1_step_01(void)
{
    D_801399AC = D_8011F538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.shade_step = 0;
    D_800D9318.target_shade = 0x81;
    D_800D9318.shade = 0x81;
    g_wmap_land_effect_00_sequence_1_timer = 0x30;
    g_wmap_land_effect_00_sequence_1_step += 1;
    wmap_land_effect_00_sequence_1_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_00_sequence_1_step_02, g_wmap_land_effect_00_sequence_1_step, g_wmap_land_effect_00_sequence_1_timer, D_800D9318,
                              D_801399A8, g_wmap_focus_screen_position, 0x13, 0x14, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_00_sequence_1_end, g_wmap_land_effect_00_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_00_run_sequence_2, D_800D60A0, 0x6, g_wmap_land_effect_00_sequence_2_step, g_wmap_land_effect_00_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_00_sequence_2_reset, g_wmap_land_effect_00_sequence_2_step, g_wmap_land_effect_00_sequence_2_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_00_sequence_2_step_01(void)
{
    D_801399CC = D_8011F538;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.sequence = 1;
    D_800D93C8.previous_sequence = -1;
    D_800D93C8.resource_index = 0;
    D_800D93C8.shade_step = 0;
    D_800D93C8.target_shade = 0x81;
    D_800D93C8.shade = 0x81;
    g_wmap_land_effect_00_sequence_2_timer = 0x60;
    g_wmap_land_effect_00_sequence_2_step += 1;
    wmap_land_effect_00_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_00_sequence_2_step_02, g_wmap_land_effect_00_sequence_2_step, g_wmap_land_effect_00_sequence_2_timer, D_800D93C8,
                              D_801399C8, g_wmap_focus_screen_position, 0x13, 0x14, 0)

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_00_sequence_2_step_03(void)
{
    D_800D93C8.shade_step = 4;
    D_800D93C8.target_shade = 0;
    g_wmap_land_effect_00_sequence_2_timer = 0x20;
    g_wmap_land_effect_00_sequence_2_step += 1;
    wmap_land_effect_00_sequence_2_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_00_sequence_2_step_04, g_wmap_land_effect_00_sequence_2_step, g_wmap_land_effect_00_sequence_2_timer, D_800D93C8,
                              D_801399C8, g_wmap_focus_screen_position, 0x13, 0x14, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_00_sequence_2_end, g_wmap_land_effect_00_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_00_run_sequence_3, D_800D60B8, 0x4, g_wmap_land_effect_00_sequence_3_step, g_wmap_land_effect_00_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_00_sequence_3_reset, g_wmap_land_effect_00_sequence_3_step, g_wmap_land_effect_00_sequence_3_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_00_sequence_3_step_01(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 8;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0;
    g_wmap_land_effect_00_sequence_3_timer = 0x9D;
    g_wmap_land_effect_00_sequence_3_step += 1;
    wmap_land_effect_00_sequence_3_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_00_sequence_3_step_02, g_wmap_land_effect_00_sequence_3_step, g_wmap_land_effect_00_sequence_3_timer, D_800D9344,
                              D_801399B0, g_wmap_focus_screen_position, 0x14, 0x1F, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_00_sequence_3_end, g_wmap_land_effect_00_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_00_run_sequence_4, D_800D60C8, 0x4, g_wmap_land_effect_00_sequence_4_step, g_wmap_land_effect_00_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_00_sequence_4_reset, g_wmap_land_effect_00_sequence_4_step, g_wmap_land_effect_00_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_00_sequence_4_step_01, g_wmap_land_effect_00_sequence_4_step, g_wmap_land_effect_00_sequence_4_timer, D_801B24A0,
                     D_80139258, D_801B2650, D_80182DE8, 0x80, 0xAFC8, 0x20, wmap_land_effect_00_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_00_sequence_4_end, g_wmap_land_effect_00_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_00_run_sequence_5, D_800D60D8, 0x4, g_wmap_land_effect_00_sequence_5_step, g_wmap_land_effect_00_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_00_sequence_5_reset, g_wmap_land_effect_00_sequence_5_step, g_wmap_land_effect_00_sequence_5_timer)

WMAP_STEP_DROP_START(wmap_land_effect_00_sequence_5_step_01, g_wmap_land_effect_00_sequence_5_step, g_wmap_land_effect_00_sequence_5_timer, D_801B24A8,
                     D_80139258, D_801B2478, D_80182DEC, 0x80, 0xAFC8, 0x40, wmap_land_effect_00_sequence_5_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_00_sequence_5_end, g_wmap_land_effect_00_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_00_run_sequence_6, D_800D60E8, 0x6, g_wmap_land_effect_00_sequence_6_step, g_wmap_land_effect_00_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_00_sequence_6_reset, g_wmap_land_effect_00_sequence_6_step, g_wmap_land_effect_00_sequence_6_timer)

/** @brief Restore effect state and begin a 66-tick sequence step. */
void wmap_land_effect_00_sequence_6_step_01(void)
{
    D_80182DF4 = 1;
    D_8013B240 = D_80139258;
    D_80139234 = 0x200;
    g_wmap_land_effect_00_sequence_6_timer = 0x42;
    g_wmap_land_effect_00_sequence_6_step += 1;
    wmap_land_effect_00_sequence_6_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_00_sequence_6_step_03, g_wmap_land_effect_00_sequence_6_step, g_wmap_land_effect_00_sequence_6_timer, 0x20,
                    wmap_land_effect_00_sequence_6_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_00_sequence_6_end, g_wmap_land_effect_00_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_00_run_sequence_7, D_800D6100, 0x8, g_wmap_land_effect_00_sequence_7_step, g_wmap_land_effect_00_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_00_sequence_7_reset, g_wmap_land_effect_00_sequence_7_step, g_wmap_land_effect_00_sequence_7_timer)

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void wmap_land_effect_00_sequence_7_step_02(void)
{
    s32 remaining;

    func_8006B328(0x64, 0x7C, 4, -1, -3, -4, 0, 0x13, -0xA0, 0x140, -0xA0, 0x140, 0x32, 1, 0x81, 4, 0);
    D_80182DF0 += 8;
    remaining = g_wmap_land_effect_00_sequence_7_timer - 1;
    g_wmap_land_effect_00_sequence_7_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_00_sequence_7_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_00_sequence_7_step_03, g_wmap_land_effect_00_sequence_7_step, g_wmap_land_effect_00_sequence_7_timer, 0x60,
                    wmap_land_effect_00_sequence_7_step_04)

/** @brief World-map effect spawn: submit a request and tick the refcount. */
void wmap_land_effect_00_sequence_7_step_04(void)
{
    s32 c;

    func_8006B328(0x64, 0x7C, 4, -1, -3, -4, 0, 0x13, -0xA0, 0x140, -0xA0,
                  0x140, 0x32, 1, 0x81, 4, 0);
    c = g_wmap_land_effect_00_sequence_7_timer - 1;
    g_wmap_land_effect_00_sequence_7_timer = c;
    if (c == 0)
    {
        g_wmap_land_effect_00_sequence_7_step += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_00_sequence_7_step_05(void)
{
    D_800DCEA8 = 0;
    g_wmap_land_effect_00_sequence_7_timer = 0x40;
    g_wmap_land_effect_00_sequence_7_step += 1;
    wmap_land_effect_00_sequence_7_step_06();
}

/** @brief World-map effect spawn: submit a request and tick the refcount. */
void wmap_land_effect_00_sequence_7_step_06(void)
{
    s32 c;

    func_8006B328(0x64, 0x7C, 4, -1, -3, -4, 0, 0x13, -0xA0, 0x140, -0xA0,
                  0x140, 0x32, 1, 0x81, 4, 0);
    c = g_wmap_land_effect_00_sequence_7_timer - 1;
    g_wmap_land_effect_00_sequence_7_timer = c;
    if (c == 0)
    {
        g_wmap_land_effect_00_sequence_7_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_00_sequence_7_end, g_wmap_land_effect_00_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_00_run_sequence_8, D_800D6120, 0x6, g_wmap_land_effect_00_sequence_8_step, g_wmap_land_effect_00_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_00_sequence_8_reset, g_wmap_land_effect_00_sequence_8_step, g_wmap_land_effect_00_sequence_8_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_00_sequence_8_step_02(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x19, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--g_wmap_land_effect_00_sequence_8_timer == 0)
    {
        g_wmap_land_effect_00_sequence_8_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_00_sequence_8_step_03(void)
{
    g_wmap_land_effect_00_sequence_8_timer = 0x20;
    D_80139280[35] = -1;
    g_wmap_land_effect_00_sequence_8_step += 1;
    wmap_land_effect_00_sequence_8_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_00_sequence_8_step_04(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x19, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--g_wmap_land_effect_00_sequence_8_timer == 0)
    {
        g_wmap_land_effect_00_sequence_8_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_00_sequence_8_end, g_wmap_land_effect_00_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_00_run_sequence_9, D_800D6138, 0x6, g_wmap_land_effect_00_sequence_9_step, g_wmap_land_effect_00_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_00_sequence_9_reset, g_wmap_land_effect_00_sequence_9_step, g_wmap_land_effect_00_sequence_9_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_00_sequence_9_step_01(void)
{
    D_801399D4 = D_80123538;
    D_800D93F4.scale_index = 0xF;
    D_800D93F4.previous_sequence = -1;
    D_800D93F4.shade_step = 2;
    D_800D93F4.target_shade = 0x81;
    D_800D93F4.resource_index = 0;
    D_800D93F4.sequence = 0;
    D_800D93F4.shade = 1;
    g_wmap_land_effect_00_sequence_9_timer = 0x88;
    g_wmap_land_effect_00_sequence_9_step += 1;
    wmap_land_effect_00_sequence_9_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_00_sequence_9_step_02, g_wmap_land_effect_00_sequence_9_step, g_wmap_land_effect_00_sequence_9_timer, D_800D93F4,
                              D_801399D0, g_wmap_focus_screen_position, 0x8, 0x1E, 0)

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_00_sequence_9_step_03(void)
{
    D_800D93F4.shade_step = 4;
    D_800D93F4.target_shade = 0;
    g_wmap_land_effect_00_sequence_9_timer = 0x20;
    g_wmap_land_effect_00_sequence_9_step += 1;
    wmap_land_effect_00_sequence_9_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_00_sequence_9_step_04, g_wmap_land_effect_00_sequence_9_step, g_wmap_land_effect_00_sequence_9_timer, D_800D93F4,
                              D_801399D0, g_wmap_focus_screen_position, 0x8, 0x1E, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_00_sequence_9_end, g_wmap_land_effect_00_sequence_9_step)
