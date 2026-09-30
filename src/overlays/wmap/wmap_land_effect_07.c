#include "wmap_main.h"
#include "wmap_land_effect_07.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_effect_primitives.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_step_sequence.h"

void wmap_land_effect_07_sequence_10_step_02(void);
void wmap_land_effect_07_wait_idle_02(void);
void wmap_land_effect_07_step_03(void);
s32 wmap_land_effect_07_run_timeline(s32 arg0);
void wmap_land_effect_07_wait_idle_04(void);
void wmap_land_effect_07_end(void);
s32 wmap_land_effect_07_run_sequence_2(s32 arg0);
s32 wmap_land_effect_07_run_sequence_1(s32 arg0);
s32 wmap_land_effect_07_run_sequence_4(s32 arg0);
s32 wmap_land_effect_07_run_sequence_6(s32 arg0);
s32 wmap_land_effect_07_run_sequence_7(s32 arg0);
s32 wmap_land_effect_07_run_sequence_8(s32 arg0);
s32 wmap_land_effect_07_run_sequence_9(s32 arg0);
s32 wmap_land_effect_07_run_sequence_3(s32 arg0);
s32 wmap_land_effect_07_run_sequence_5(s32 arg0);
s32 wmap_land_effect_07_run_sequence_11(s32 arg0);
s32 wmap_land_effect_07_run_sequence_10(s32 arg0);
void wmap_land_effect_07_sequence_4_step_02(void);
void wmap_land_effect_07_sequence_5_step_02(void);
void wmap_land_effect_07_sequence_6_step_02(void);
void wmap_land_effect_07_sequence_7_step_02(void);
void wmap_land_effect_07_sequence_8_step_02(void);
void wmap_land_effect_07_sequence_9_step_02(void);
void wmap_land_effect_07_sequence_10_step_04(void);
void wmap_land_effect_07_sequence_11_step_02(void);

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

typedef struct
{
    u32 value;
    u8 unknown_4[36];
} WmapValueRecord;

typedef struct
{
    s32 w[11];
} WmapBlk2C;

extern s8 D_80051B4C[];
extern u8 D_800DCF18[];
extern s32 D_80182DE4;
extern s32 D_801B2468;
extern s32 D_801B24B4;
extern s32 g_wmap_land_effect_07_sequence_1_timer;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8* D_8011CF1C;
extern s32 g_wmap_land_effect_07_sequence_2_timer;
extern s32 D_80182DEC;
extern u8* D_8011CF24;
extern s32 g_wmap_land_effect_07_sequence_3_timer;
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern s32 D_801B0FD0;
extern s32 g_wmap_land_effect_07_sequence_10_timer;
extern u8 D_8011F538[];
extern s32 D_8011CF74;
extern s32 D_80139980;
extern s32 g_wmap_land_effect_07_sequence_11_timer;
extern s32 g_wmap_land_effect_07_timer;
extern void (*D_800D55B8[])(void);
extern void wmap_land_effect_07_step_03(void);
extern void wmap_land_effect_07_end(void);
extern s32 g_wmap_land_effect_07_timeline_timer;
extern void (*D_800D55D0[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern void (*D_800D5628[])(void);
extern void (*D_800D5640[])(void);
extern void (*D_800D5650[])(void);
extern s32 g_wmap_land_effect_07_sequence_4_timer;
extern void (*D_800D5660[])(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern void wmap_land_effect_07_sequence_4_step_02(void);
extern s32 g_wmap_land_effect_07_sequence_5_timer;
extern void (*D_800D5670[])(void);
extern u8 D_80121538[];
extern u8* D_801399B4;
extern void wmap_land_effect_07_sequence_5_step_02(void);
extern s32 g_wmap_land_effect_07_sequence_6_timer;
extern void (*D_800D5680[])(void);
extern s32 g_wmap_land_effect_07_sequence_7_timer;
extern void (*D_800D5690[])(void);
extern s32 g_wmap_land_effect_07_sequence_8_timer;
extern void (*D_800D56A0[])(void);
extern s32 g_wmap_land_effect_07_sequence_9_timer;
extern void (*D_800D56B0[])(void);
extern u8 D_800D9478[];
extern void (*D_800D56C0[])(void);
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern void (*D_800D56D8[])(void);

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
extern WmapSpriteActor D_800D9370[];
extern WmapSpriteActor D_800D93C8[];
extern WmapSpriteActor D_800D9420[];

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8[];
extern WmapAnimationSlot D_801399C8[];
extern WmapAnimationSlot D_801399D8[];
extern WmapAnimationSlot D_801399E8[];

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* D_80139280;

extern WmapSlot14 D_801AFBD0[];

/** @brief Draw two oscillating effect layers and update their intensity. */
void wmap_land_effect_07_sequence_1_step_02(void)
{
    VECTOR *position;
    SVECTOR *first_rotation;
    SVECTOR *second_rotation;
    s32 frame;
    s32 intensity;
    s32 remaining;
    PushMatrix();
    position = &g_wmap_camera_translation;
    frame = (s32)(D_80051B4C[D_801B24B4] + 0x80) >> 5;
    first_rotation = &D_801B2490;
    wmap_set_model_transform(position, first_rotation);
    wmap_draw_model_default(D_800DCF18, frame, 4, 0x35, 0x7800, 0, D_801B2468);
    frame = (s32)(D_80051B4C[D_80182DE4] + 0x80) >> 5;
    first_rotation->vz = (u16)(first_rotation->vz + 0x14);
    second_rotation = &D_801B2498;
    wmap_set_model_transform(position, second_rotation);
    wmap_draw_model_default(D_800DCF18, frame, 4, 0x35, 0x7800, 0, D_801B2468);
    second_rotation->vz = (u16)(second_rotation->vz + 0x30);
    PopMatrix();
    D_801B24B4 = (D_801B24B4 + 8) & 0xFF;
    D_80182DE4 = (D_80182DE4 + 4) & 0xFF;
    intensity = D_801B2468 + 1;
    D_801B2468 = intensity;
    if (intensity >= 0x81)
    {
        D_801B2468 = 0x80;
    }
    remaining = g_wmap_land_effect_07_sequence_1_timer - 1;
    g_wmap_land_effect_07_sequence_1_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_07_sequence_1_step += 1;
    }
}

/** @brief Draw two oscillating effect layers and update their intensity. */
void wmap_land_effect_07_sequence_1_step_04(void)
{
    VECTOR *position;
    SVECTOR *first_rotation;
    SVECTOR *second_rotation;
    s32 frame;
    s32 intensity;
    s32 remaining;
    PushMatrix();
    position = &g_wmap_camera_translation;
    frame = (s32)(D_80051B4C[D_801B24B4] + 0x80) >> 5;
    first_rotation = &D_801B2490;
    wmap_set_model_transform(position, first_rotation);
    wmap_draw_model_default(D_800DCF18, frame, 4, 0x35, 0x7800, 0, D_801B2468);
    frame = (s32)(D_80051B4C[D_80182DE4] + 0x80) >> 5;
    first_rotation->vz = (u16)(first_rotation->vz + 0x14);
    second_rotation = &D_801B2498;
    wmap_set_model_transform(position, second_rotation);
    wmap_draw_model_default(D_800DCF18, frame, 4, 0x35, 0x7800, 0, D_801B2468);
    second_rotation->vz = (u16)(second_rotation->vz + 0x30);
    PopMatrix();
    D_801B24B4 = (D_801B24B4 + 8) & 0xFF;
    D_80182DE4 = (D_80182DE4 + 4) & 0xFF;
    intensity = D_801B2468 - 2;
    D_801B2468 = intensity;
    if (intensity < 0)
    {
        D_801B2468 = 0;
    }
    remaining = g_wmap_land_effect_07_sequence_1_timer - 1;
    g_wmap_land_effect_07_sequence_1_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_07_sequence_1_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_07_sequence_2_step_02(void)
{
    MATRIX m;
    s32 x;

    x = D_801B2650.vz + 0xDAC;
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
        D_80182DE8 -= 0x8;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_07_sequence_2_timer == 0)
    {
        g_wmap_land_effect_07_sequence_2_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_07_sequence_3_step_02(void)
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
        D_80182DEC -= 0x8;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_07_sequence_3_timer == 0)
    {
        g_wmap_land_effect_07_sequence_3_step += 1;
    }
}

/** @brief Configure effect parameters and reset its animation resource slots. */
void wmap_land_effect_07_sequence_10_step_01(void)
{
    s32 i;

    D_801B0FD0 = 12;
    D_801B24B4 = 127;
    D_80139234 = 4;
    D_8013923C = 4;
    D_80139240 = 16;
    D_8013924C = 0;
    D_80139250 = 2;
    D_80139260 = 1500;
    D_80139264 = 100;
    D_80139268 = 19;
    D_8013926C = 1;
    D_80139284 = 0x938801F4;
    for (i = 0; i < 12; i++)
    {
        D_801AFBD0[i + D_80139264].field_00 = 0;
        D_80139988[i + 104].data = D_8011F538;
    }
    g_wmap_land_effect_07_sequence_10_timer = 24;
    g_wmap_land_effect_07_sequence_10_step++;
    wmap_land_effect_07_sequence_10_step_02();
}

/** @brief Reduce the effect parameters, draw it, and advance its countdown. */
void wmap_land_effect_07_sequence_11_step_04(void)
{
    s32 value;
    s32 remaining;

    value = D_80139980 - 2;
    D_80139980 = value;
    if (value < 0)
    {
        D_80139980 = 0;
    }
    if (!(D_8011CF74 & 3))
    {
        D_801B0FD0 -= 1;
    }
    if (D_801B0FD0 < 0)
    {
        D_801B0FD0 = 0;
    }
    func_8006AFAC(0xCC, 0xD2, 0x13, 0x20, 0x650, 8, 0x60, 0);
    remaining = g_wmap_land_effect_07_sequence_11_timer - 1;
    g_wmap_land_effect_07_sequence_11_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_07_sequence_11_step += 1;
    }
}

WMAP_STEP_RUNNER(wmap_land_effect_07_run, D_800D55B8, 0x6, g_wmap_land_effect_07_step, g_wmap_land_effect_07_timer)

WMAP_STEP_RESET(wmap_land_effect_07_reset, g_wmap_land_effect_07_step, g_wmap_land_effect_07_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_07_step_01, g_wmap_land_effect_07_step, wmap_run_land_focus, wmap_land_effect_07_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_07_wait_idle_02, g_wmap_land_effect_07_step, wmap_land_effect_07_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_07_step_03, g_wmap_land_effect_07_step, wmap_land_effect_07_run_timeline, wmap_land_effect_07_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_07_wait_idle_04, g_wmap_land_effect_07_step, wmap_land_effect_07_end)

WMAP_STEP_ADVANCE(wmap_land_effect_07_end, g_wmap_land_effect_07_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_timeline, D_800D55D0, 0x16, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_07_timeline_reset, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer)

/** @brief World-map state entry: load resources, register callbacks, advance. */
void wmap_land_effect_07_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_start_map_tint(0x803030);
    g_wmap_backdrop_target_level = 4;
    wmap_play_sound(0x1D, 0x80);
    wmap_start_sequence(wmap_land_effect_07_run_sequence_2);
    wmap_start_sequence(wmap_land_effect_07_run_sequence_1);
    g_wmap_land_effect_07_timeline_timer = 8;
    g_wmap_land_effect_07_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_07_timeline_wait_02, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_07_timeline_step_03, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer,
                         wmap_land_effect_07_run_sequence_4, 0x8)

WMAP_STEP_WAIT(wmap_land_effect_07_timeline_wait_04, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_07_timeline_step_05, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer,
                         wmap_land_effect_07_run_sequence_6, 0x14)

WMAP_STEP_WAIT(wmap_land_effect_07_timeline_wait_06, g_wmap_land_effect_07_timeline_step, g_wmap_land_effect_07_timeline_timer)

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void wmap_land_effect_07_timeline_step_07(void)
{
    D_801ADAE0 = 1;
    wmap_start_sequence(wmap_land_effect_07_run_sequence_7);
    g_wmap_land_effect_07_timeline_timer = 0x14;
    g_wmap_land_effect_07_timeline_step += 1;
}

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

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_07_timeline_finish, g_wmap_land_effect_07_timeline_step, D_80139290, D_8011D510, D_8011D530, D_8011D4FC)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_1, D_800D5628, 0x6, g_wmap_land_effect_07_sequence_1_step, g_wmap_land_effect_07_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_1_reset, g_wmap_land_effect_07_sequence_1_step, g_wmap_land_effect_07_sequence_1_timer)

/** @brief World-map step: reset the sprite counters and advance to the next handler. */
void wmap_land_effect_07_sequence_1_step_01(void)
{
    D_801B2468 = 1;
    D_801B24B4 = 0;
    D_80182DE4 = 0;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    D_801B2498.vx = 0;
    D_801B2498.vy = 0;
    D_801B2498.vz = 0;
    g_wmap_land_effect_07_sequence_1_timer = 0x80;
    g_wmap_land_effect_07_sequence_1_step += 1;
    wmap_land_effect_07_sequence_1_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_07_sequence_1_step_03, g_wmap_land_effect_07_sequence_1_step, g_wmap_land_effect_07_sequence_1_timer, 0x40,
                    wmap_land_effect_07_sequence_1_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_1_end, g_wmap_land_effect_07_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_2, D_800D5640, 0x4, g_wmap_land_effect_07_sequence_2_step, g_wmap_land_effect_07_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_2_reset, g_wmap_land_effect_07_sequence_2_step, g_wmap_land_effect_07_sequence_2_timer)

WMAP_STEP_DROP_START(wmap_land_effect_07_sequence_2_step_01, g_wmap_land_effect_07_sequence_2_step, g_wmap_land_effect_07_sequence_2_timer, D_801B24A0,
                     D_80139258, D_801B2650, D_80182DE8, 0x80, 0xAFC8, 0x10, wmap_land_effect_07_sequence_2_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_2_end, g_wmap_land_effect_07_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_3, D_800D5650, 0x4, g_wmap_land_effect_07_sequence_3_step, g_wmap_land_effect_07_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_3_reset, g_wmap_land_effect_07_sequence_3_step, g_wmap_land_effect_07_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_07_sequence_3_step_01, g_wmap_land_effect_07_sequence_3_step, g_wmap_land_effect_07_sequence_3_timer, D_801B24A8,
                     D_80139258, D_801B2478, D_80182DEC, 0x80, 0xAFC8, 0x10, wmap_land_effect_07_sequence_3_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_3_end, g_wmap_land_effect_07_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_4, D_800D5660, 0x4, g_wmap_land_effect_07_sequence_4_step, g_wmap_land_effect_07_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_4_reset, g_wmap_land_effect_07_sequence_4_step, g_wmap_land_effect_07_sequence_4_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_07_sequence_4_step_01(void)
{
    D_801399AC = D_8011D538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 8;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.target_shade = 0x80;
    D_800D9318.shade = 0;
    g_wmap_land_effect_07_sequence_4_timer = 0x70;
    g_wmap_land_effect_07_sequence_4_step += 1;
    wmap_land_effect_07_sequence_4_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_07_sequence_4_step_02, g_wmap_land_effect_07_sequence_4_step, g_wmap_land_effect_07_sequence_4_timer, D_800D9318,
                              D_801399A8, g_wmap_focus_screen_position, 0x13, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_4_end, g_wmap_land_effect_07_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_5, D_800D5670, 0x4, g_wmap_land_effect_07_sequence_5_step, g_wmap_land_effect_07_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_5_reset, g_wmap_land_effect_07_sequence_5_step, g_wmap_land_effect_07_sequence_5_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_07_sequence_5_step_01(void)
{
    D_801399B4 = D_80121538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 8;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0;
    g_wmap_land_effect_07_sequence_5_timer = 0x88;
    g_wmap_land_effect_07_sequence_5_step += 1;
    wmap_land_effect_07_sequence_5_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_07_sequence_5_step_02, g_wmap_land_effect_07_sequence_5_step, g_wmap_land_effect_07_sequence_5_timer, D_800D9344,
                              D_801399B0, g_wmap_focus_screen_position, 0x17, 0x70, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_5_end, g_wmap_land_effect_07_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_6, D_800D5680, 0x4, g_wmap_land_effect_07_sequence_6_step, g_wmap_land_effect_07_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_6_reset, g_wmap_land_effect_07_sequence_6_step, g_wmap_land_effect_07_sequence_6_timer)

/**
 * @brief Populate a world-map actor pair and schedule its animation step.
 */
void wmap_land_effect_07_sequence_6_step_01(void)
{
    u8* src = (u8*)D_800D9370;
    u8* dst = (u8*)D_800D9370 + 0x2C;
    u8* tbl = D_80139988;

    *(u8**)&tbl[0x34] = D_8011F538;
    *(u8**)&tbl[0x3C] = D_8011F538;
    src[0x6] = 0xF;
    *(s16*)&src[0xE] = 2;
    *(s16*)&src[0x10] = -1;
    *(s16*)&src[0x26] = 8;
    *(s16*)&src[0x22] = 0x81;
    *(s16*)&src[0x2] = 0;
    *(s16*)&src[0x24] = 1;
    *(WmapBlk2C*)dst = *(WmapBlk2C*)src;
    *(s16*)&dst[0xE] = 3;
    g_wmap_land_effect_07_sequence_6_timer = 0x68;
    g_wmap_land_effect_07_sequence_6_step += 1;
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
    wmap_step_actor_animation(D_800D9370, D_801399B8);
    wmap_draw_actor_sprite(D_800D9370, pos, 0x13, 1, 0);
    wmap_step_actor_animation(&D_800D9370[1], &D_801399B8[1]);
    wmap_draw_actor_sprite(&D_800D9370[1], pos, 0x13, 1, 0);
    if (--g_wmap_land_effect_07_sequence_6_timer == 0)
    {
        g_wmap_land_effect_07_sequence_6_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_6_end, g_wmap_land_effect_07_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_7, D_800D5690, 0x4, g_wmap_land_effect_07_sequence_7_step, g_wmap_land_effect_07_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_7_reset, g_wmap_land_effect_07_sequence_7_step, g_wmap_land_effect_07_sequence_7_timer)

/**
 * @brief Populate a world-map actor pair and schedule its animation step.
 */
void wmap_land_effect_07_sequence_7_step_01(void)
{
    u8* src = (u8*)D_800D93C8;
    u8* dst = (u8*)D_800D93C8 + 0x2C;
    u8* tbl = D_80139988;

    *(u8**)&tbl[0x44] = D_8011F538;
    *(u8**)&tbl[0x4C] = D_8011F538;
    src[0x6] = 0xF;
    *(s16*)&src[0xE] = 2;
    *(s16*)&src[0x10] = -1;
    *(s16*)&src[0x26] = 8;
    *(s16*)&src[0x22] = 0x81;
    *(s16*)&src[0x2] = 0;
    *(s16*)&src[0x24] = 1;
    *(WmapBlk2C*)dst = *(WmapBlk2C*)src;
    *(s16*)&dst[0xE] = 3;
    g_wmap_land_effect_07_sequence_7_timer = 0x54;
    g_wmap_land_effect_07_sequence_7_step += 1;
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
    wmap_step_actor_animation(D_800D93C8, D_801399C8);
    wmap_draw_actor_sprite(D_800D93C8, pos, 0x13, 1, 0);
    wmap_step_actor_animation(&D_800D93C8[1], &D_801399C8[1]);
    wmap_draw_actor_sprite(&D_800D93C8[1], pos, 0x13, 1, 0);
    if (--g_wmap_land_effect_07_sequence_7_timer == 0)
    {
        g_wmap_land_effect_07_sequence_7_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_7_end, g_wmap_land_effect_07_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_8, D_800D56A0, 0x4, g_wmap_land_effect_07_sequence_8_step, g_wmap_land_effect_07_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_8_reset, g_wmap_land_effect_07_sequence_8_step, g_wmap_land_effect_07_sequence_8_timer)

/**
 * @brief Populate a world-map actor pair and schedule its animation step.
 */
void wmap_land_effect_07_sequence_8_step_01(void)
{
    u8* src = (u8*)D_800D9420;
    u8* dst = (u8*)D_800D9420 + 0x2C;
    u8* tbl = D_80139988;

    *(u8**)&tbl[0x54] = D_8011F538;
    *(u8**)&tbl[0x5C] = D_8011F538;
    src[0x6] = 0xF;
    *(s16*)&src[0xE] = 2;
    *(s16*)&src[0x10] = -1;
    *(s16*)&src[0x26] = 8;
    *(s16*)&src[0x22] = 0x81;
    *(s16*)&src[0x2] = 0;
    *(s16*)&src[0x24] = 1;
    *(WmapBlk2C*)dst = *(WmapBlk2C*)src;
    *(s16*)&dst[0xE] = 3;
    g_wmap_land_effect_07_sequence_8_timer = 0x40;
    g_wmap_land_effect_07_sequence_8_step += 1;
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
    wmap_step_actor_animation(D_800D9420, D_801399D8);
    wmap_draw_actor_sprite(D_800D9420, pos, 0x13, 1, 0);
    wmap_step_actor_animation(&D_800D9420[1], &D_801399D8[1]);
    wmap_draw_actor_sprite(&D_800D9420[1], pos, 0x13, 1, 0);
    if (--g_wmap_land_effect_07_sequence_8_timer == 0)
    {
        g_wmap_land_effect_07_sequence_8_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_8_end, g_wmap_land_effect_07_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_9, D_800D56B0, 0x4, g_wmap_land_effect_07_sequence_9_step, g_wmap_land_effect_07_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_9_reset, g_wmap_land_effect_07_sequence_9_step, g_wmap_land_effect_07_sequence_9_timer)

/**
 * @brief Populate a world-map actor pair and schedule its animation step.
 */
void wmap_land_effect_07_sequence_9_step_01(void)
{
    u8* src = D_800D9478;
    u8* dst = D_800D9478 + 0x2C;
    u8* tbl = D_80139988;

    *(u8**)&tbl[0x64] = D_8011F538;
    *(u8**)&tbl[0x6C] = D_8011F538;
    src[0x6] = 0xF;
    *(s16*)&src[0xE] = 2;
    *(s16*)&src[0x10] = -1;
    *(s16*)&src[0x26] = 8;
    *(s16*)&src[0x22] = 0x81;
    *(s16*)&src[0x2] = 0;
    *(s16*)&src[0x24] = 1;
    *(WmapBlk2C*)dst = *(WmapBlk2C*)src;
    *(s16*)&dst[0xE] = 3;
    g_wmap_land_effect_07_sequence_9_timer = 0x2C;
    g_wmap_land_effect_07_sequence_9_step += 1;
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
    wmap_step_actor_animation(D_800D9478, D_801399E8);
    wmap_draw_actor_sprite(D_800D9478, pos, 0x13, 1, 0);
    wmap_step_actor_animation(&D_800D9478[0x2C], &D_801399E8[1]);
    wmap_draw_actor_sprite(&D_800D9478[0x2C], pos, 0x13, 1, 0);
    if (--g_wmap_land_effect_07_sequence_9_timer == 0)
    {
        g_wmap_land_effect_07_sequence_9_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_9_end, g_wmap_land_effect_07_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_10, D_800D56C0, 0x6, g_wmap_land_effect_07_sequence_10_step, g_wmap_land_effect_07_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_10_reset, g_wmap_land_effect_07_sequence_10_step, g_wmap_land_effect_07_sequence_10_timer)

/** @brief World-map step handler: spawn a sub-object and expire the step counter. */
void wmap_land_effect_07_sequence_10_step_02(void)
{
    func_8006CFE4(D_800DA448, D_80139CC8, 0xC, D_801B24B4, D_801B24B4, 0);
    if (--g_wmap_land_effect_07_sequence_10_timer == 0)
    {
        g_wmap_land_effect_07_sequence_10_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_07_sequence_10_step_03, g_wmap_land_effect_07_sequence_10_step, g_wmap_land_effect_07_sequence_10_timer, 0x30,
                    wmap_land_effect_07_sequence_10_step_04)

/** @brief Draw the active effect and advance when the countdown expires. */
void wmap_land_effect_07_sequence_10_step_04(void)
{
    s32 remaining_ticks;

    D_80139280[5] = -1;
    if (D_801B24B4 != 0)
    {
        func_8006CFE4((s32)D_800DA448, (s32)D_80139CC8, 0xC, D_801B24B4, D_801B24B4, 0);
    }
    remaining_ticks = g_wmap_land_effect_07_sequence_10_timer - 1;
    g_wmap_land_effect_07_sequence_10_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_07_sequence_10_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_10_end, g_wmap_land_effect_07_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_07_run_sequence_11, D_800D56D8, 0x6, g_wmap_land_effect_07_sequence_11_step, g_wmap_land_effect_07_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_07_sequence_11_reset, g_wmap_land_effect_07_sequence_11_step, g_wmap_land_effect_07_sequence_11_timer)

/** @brief Seed six world-map effect slots and advance the sequence step. */
void wmap_land_effect_07_sequence_11_step_01(void)
{
    s32 i;

    D_801B0FD0 = 0;
    D_80139980 = 0x7F;
    for (i = 0; i < 6; i++)
    {
        D_801AFBD0[i].field_00 = 0;
        D_80139988[i + 204].data = D_8011F538;
    }
    g_wmap_land_effect_07_sequence_11_timer = 0x20;
    g_wmap_land_effect_07_sequence_11_step += 1;
    wmap_land_effect_07_sequence_11_step_02();
}

/** @brief Advance the effect on odd frames, draw it, and update the countdown. */
void wmap_land_effect_07_sequence_11_step_02(void)
{
    s32 remaining_ticks;

    if (D_8011CF74 & 1)
    {
        D_801B0FD0 += 1;
    }
    func_8006AFAC(0xCC, 0xD2, 0x13, 0x20, 0x650, 8, 0x60, 0);
    remaining_ticks = g_wmap_land_effect_07_sequence_11_timer - 1;
    g_wmap_land_effect_07_sequence_11_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_07_sequence_11_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_07_sequence_11_step_03, g_wmap_land_effect_07_sequence_11_step, g_wmap_land_effect_07_sequence_11_timer, 0x40,
                    wmap_land_effect_07_sequence_11_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_07_sequence_11_end, g_wmap_land_effect_07_sequence_11_step)
