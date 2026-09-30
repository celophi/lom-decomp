#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_land_effect_23.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"
#include "wmap_step_sequence.h"

void wmap_land_effect_23_sequence_5_step_02(void);
void wmap_land_effect_23_sequence_6_step_02(void);
void wmap_land_effect_23_sequence_10_step_02(void);
void wmap_land_effect_23_sequence_13_step_02(void);
s32 wmap_land_effect_23_run_timeline(s32 arg0);
void wmap_land_effect_23_wait_idle(void);
void wmap_land_effect_23_step_03(void);
s32 wmap_land_effect_23_run_sequence_14(s32 arg0);
s32 wmap_land_effect_23_run_sequence_5(s32 arg0);
s32 wmap_land_effect_23_run_sequence_10(s32 arg0);
s32 wmap_land_effect_23_run_sequence_6(s32 arg0);
s32 wmap_land_effect_23_run_sequence_2(s32 arg0);
s32 wmap_land_effect_23_run_sequence_1(s32 arg0);
s32 wmap_land_effect_23_run_sequence_7(s32 arg0);
s32 wmap_land_effect_23_run_sequence_3(s32 arg0);
s32 wmap_land_effect_23_run_sequence_11(s32 arg0);
s32 wmap_land_effect_23_run_sequence_13(s32 arg0);
s32 wmap_land_effect_23_run_sequence_12(s32 arg0);
s32 wmap_land_effect_23_run_sequence_9(s32 arg0);
s32 wmap_land_effect_23_run_sequence_8(s32 arg0);
s32 wmap_land_effect_23_run_sequence_4(s32 arg0);
void wmap_land_effect_23_sequence_1_step_02(void);
void wmap_land_effect_23_sequence_5_step_04(void);
void wmap_land_effect_23_sequence_6_step_04(void);
void wmap_land_effect_23_sequence_9_step_02(void);
void wmap_land_effect_23_sequence_9_step_04(void);
void wmap_land_effect_23_sequence_10_step_04(void);
void wmap_land_effect_23_sequence_11_step_02(void);
void wmap_land_effect_23_sequence_11_step_04(void);
void wmap_land_effect_23_sequence_12_step_02(void);
void wmap_land_effect_23_sequence_12_step_04(void);
void wmap_land_effect_23_sequence_13_step_04(void);
void wmap_land_effect_23_sequence_13_step_06(void);

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

typedef struct
{
    s16 unk0;
    s16 unk2;
    u8 unk4[2];
    u8 unk6;
    u8 unk7[7];
    s16 unkE;
    s16 unk10;
    u8 unk12[0x10];
    s16 unk22;
    s16 unk24;
    s16 unk26;
    u8 unk28[4];
} WmapD94Entry;

/** @brief Record with a leading value and a 40-byte stride. */
typedef struct
{
    s32 value;
    u8 unknown_4[36];
} WmapValueRecord;

extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 g_wmap_land_effect_23_sequence_2_timer;
extern s32 D_80182DEC;
extern u8* D_8011CF1C;
extern s32 g_wmap_land_effect_23_sequence_3_timer;
extern s32 D_80182DF0;
extern s32 g_wmap_land_effect_23_sequence_4_timer;
extern u8 D_8011F538[];
extern s32 g_wmap_land_effect_23_sequence_5_timer;
extern s32 g_wmap_land_effect_23_sequence_6_timer;
extern s32* D_8011CF2C;
extern s32 D_80139234;
extern s32 D_801B25D8;
extern s32 g_wmap_land_effect_23_sequence_7_timer;
extern s32* D_8011CF28;
extern s32 D_8013923C;
extern s32 g_wmap_land_effect_23_sequence_8_timer;
extern s32 g_wmap_land_effect_23_sequence_10_timer;
extern s32 D_80121538[];
extern s32 D_800D9154;
extern s32 D_800DCEAC;
extern s32 D_80182DF4;
extern s32 g_wmap_land_effect_23_sequence_13_timer;
extern int rand(void);
extern s32 D_80139264;
extern s32 g_wmap_land_effect_23_sequence_14_timer;
extern s32 g_wmap_land_effect_23_timer;
extern void (*D_800D672C[])(void);
extern s32 D_80139978;
extern s32 D_8013B294;
extern s32 D_80139228;
extern s32 g_wmap_land_effect_23_timeline_timer;
extern void (*D_800D673C[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern s32 D_800DBE70;
extern s32 D_80139244;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern WmapValueRecord D_80139290[][6];
extern s32 g_wmap_land_effect_23_sequence_1_timer;
extern void (*D_800D67CC[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern void wmap_land_effect_23_sequence_1_step_02(void);
extern void (*D_800D67DC[])(void);
extern void (*D_800D67EC[])(void);
extern void (*D_800D67FC[])(void);
extern void (*D_800D680C[])(void);
extern u8 D_800DB158[];
extern u8 D_80139F28[];
extern void (*D_800D6824[])(void);
extern u8 D_800D9790[];
extern u8 D_80139A78[];
extern void wmap_land_effect_23_sequence_6_step_04(void);
extern void (*D_800D683C[])(void);
extern void (*D_800D6854[])(void);
extern s32 g_wmap_land_effect_23_sequence_9_timer;
extern void (*D_800D686C[])(void);
extern u8 D_80125538[];
extern s32 D_8013924C;
extern u8 *D_801399BC;
extern void (*D_800D6884[])(void);
extern u8 D_800DA8C0[];
extern u8 D_80139D98[];
extern void wmap_land_effect_23_sequence_10_step_04(void);
extern s32 g_wmap_land_effect_23_sequence_11_timer;
extern void (*D_800D689C[])(void);
extern u8 D_80123538[];
extern s32 D_80139250;
extern u8 *D_801399C4;
extern s32 g_wmap_land_effect_23_sequence_12_timer;
extern void (*D_800D68B4[])(void);
extern s32 D_80139260;
extern u8 *D_801399CC;
extern void (*D_800D68CC[])(void);
extern void (*D_800D68EC[])(void);
extern u32 g_wmap_land_effect_23_sequence_2_step;
extern u32 g_wmap_land_effect_23_sequence_3_step;
extern u32 g_wmap_land_effect_23_sequence_4_step;
extern u32 g_wmap_land_effect_23_sequence_5_step;
extern u32 g_wmap_land_effect_23_sequence_6_step;

extern u32 g_wmap_land_effect_23_sequence_7_step;
extern u32 g_wmap_land_effect_23_sequence_8_step;
extern u32 g_wmap_land_effect_23_sequence_10_step;
extern u32 g_wmap_land_effect_23_sequence_13_step;
extern u32 g_wmap_land_effect_23_sequence_14_step;
extern u32 g_wmap_land_effect_23_step;
extern u32 g_wmap_land_effect_23_timeline_step;
extern u32 g_wmap_land_effect_23_sequence_1_step;
extern u32 g_wmap_land_effect_23_sequence_9_step;
extern u32 g_wmap_land_effect_23_sequence_11_step;
extern u32 g_wmap_land_effect_23_sequence_12_step;

extern WmapSpriteActor D_800D9268[];
extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370;
extern WmapSpriteActor D_800D939C;
extern WmapSpriteActor D_800D93C8;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8;
extern WmapAnimationSlot D_801399C0;
extern WmapAnimationSlot D_801399C8;

extern WmapScreenPosition g_wmap_focus_screen_position;
extern WmapScreenPosition D_80182D58;
extern WmapScreenPosition D_80182D60;
extern WmapScreenPosition D_80182D64;

extern s32* D_80139280;

extern WmapSlot14 D_801AFBD0[];

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;
extern VECTOR D_80139870;
extern VECTOR D_80182D48;

extern SVECTOR D_80139258;
extern SVECTOR D_801B24A0;
extern SVECTOR D_8013B238;
extern SVECTOR D_801B24A8;
extern SVECTOR D_801398C8;

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_23_sequence_2_step_02(void)
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
    if (--g_wmap_land_effect_23_sequence_2_timer == 0)
    {
        g_wmap_land_effect_23_sequence_2_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_23_sequence_3_step_02(void)
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
        D_80182DEC -= 1;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_23_sequence_3_timer == 0)
    {
        g_wmap_land_effect_23_sequence_3_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_23_sequence_4_step_02(void)
{
    MATRIX m;
    s32 x;

    x = D_80139870.vz - 0xDAC;
    D_80139870.vz = x;
    if (x < 0x2710)
    {
        D_80139870.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_8013B238, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DF0 != 0)
    {
        wmap_draw_model_default(D_8011CF1C, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DF0);
        D_80182DF0 -= 0x2;
        if (D_80182DF0 < 0)
        {
            D_80182DF0 = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_23_sequence_4_timer == 0)
    {
        g_wmap_land_effect_23_sequence_4_step += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_23_sequence_5_step_01(void)
{
    s32 i;

    D_80139280[0x1] = 1;
    D_80139280[0x2] = 0;
    D_80139280[0x3] = 0x20;
    D_80139280[0x4] = 0;
    D_80139280[0x5] = 4;
    D_80139280[0x6] = 0x320;
    D_80139280[0x7] = 0xB4;
    D_80139280[0x8] = 8;
    D_80139280[0x9] = 1;
    D_80139280[0xA] = 0x124F8;
    for (i = 0; i < 20; i++)
    {
        D_801AFBD0[i + 180].field_00 = 0;
        D_80139988[i + 180].data = D_8011F538;
    }
    g_wmap_land_effect_23_sequence_5_timer = 80;
    g_wmap_land_effect_23_sequence_5_step++;
    wmap_land_effect_23_sequence_5_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_23_sequence_6_step_01(void)
{
    s32 i;

    D_80139280[0xB] = 0;
    D_80139280[0xC] = 0;
    D_80139280[0xD] = 0x40;
    D_80139280[0xE] = 0;
    D_80139280[0xF] = 5;
    D_80139280[0x10] = 0x258;
    D_80139280[0x11] = 0x1E;
    D_80139280[0x12] = 8;
    D_80139280[0x13] = 2;
    D_80139280[0x14] = 0x2710;
    for (i = 0; i < 90; i++)
    {
        D_801AFBD0[i + 30].field_00 = 0;
        D_80139988[i + 30].data = D_8011F538;
    }
    g_wmap_land_effect_23_sequence_6_timer = 450;
    g_wmap_land_effect_23_sequence_6_step++;
    wmap_land_effect_23_sequence_6_step_02();
}

/**
 * @brief Draw the first animated element, ramp its intensity up, rotate it, and advance after its timer expires.
 */
void wmap_land_effect_23_sequence_7_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF2C, D_80139234 & 3, 0xA, 0x36, 0x7900, 0x1001, D_801B25D8, 0, 0xF, -1);
    value = D_801B25D8 + 2;
    D_801B25D8 = value;
    if (value >= 0x82)
    {
        D_801B25D8 = 0x81;
    }
    timer = g_wmap_land_effect_23_sequence_7_timer;
    D_8013B238.vz += 0x14;
    next_timer = timer - 1;
    g_wmap_land_effect_23_sequence_7_timer = next_timer;
    D_80139234 += 1;
    if (next_timer == 0)
    {
        g_wmap_land_effect_23_sequence_7_step += 1;
    }
}

/**
 * @brief Draw an animated element, ramp its intensity down, rotate it, and advance after its timer expires.
 */
void wmap_land_effect_23_sequence_7_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF2C, D_80139234 & 3, 0xA, 0x36, 0x7900, 0x1001, D_801B25D8, 0, 0xF, -1);
    value = D_801B25D8 - 4;
    D_801B25D8 = value;
    if (value < 0)
    {
        D_801B25D8 = 0;
    }
    timer = g_wmap_land_effect_23_sequence_7_timer;
    D_8013B238.vz += 0x14;
    next_timer = timer - 1;
    g_wmap_land_effect_23_sequence_7_timer = next_timer;
    D_80139234 += 1;
    if (next_timer == 0)
    {
        g_wmap_land_effect_23_sequence_7_step += 1;
    }
}

/**
 * @brief Draw the second animated element, ramp its intensity up, rotate it, and advance after its timer expires.
 */
void wmap_land_effect_23_sequence_8_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B24A8);
    wmap_draw_model(D_8011CF28, D_8013923C & 3, 0x4, 0x35, 0x7800, 0x1001, D_80182DEC, 0, 0xA, -1);
    value = D_80182DEC + 8;
    D_80182DEC = value;
    if (value >= 0x82)
    {
        D_80182DEC = 0x81;
    }
    timer = g_wmap_land_effect_23_sequence_8_timer;
    D_801B24A8.vz += 0x38;
    next_timer = timer - 1;
    g_wmap_land_effect_23_sequence_8_timer = next_timer;
    D_8013923C += 1;
    if (next_timer == 0)
    {
        g_wmap_land_effect_23_sequence_8_step += 1;
    }
}

/**
 * @brief Draw the second animated element, ramp its intensity down, rotate it, and advance after its timer expires.
 */
void wmap_land_effect_23_sequence_8_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B24A8);
    wmap_draw_model(D_8011CF28, D_8013923C & 3, 0x4, 0x35, 0x7800, 0x1001, D_80182DEC, 0, 0xA, -1);
    value = D_80182DEC - 8;
    D_80182DEC = value;
    if (value < 0)
    {
        D_80182DEC = 0;
    }
    timer = g_wmap_land_effect_23_sequence_8_timer;
    D_801B24A8.vz += 0x38;
    next_timer = timer - 1;
    g_wmap_land_effect_23_sequence_8_timer = next_timer;
    D_8013923C += 1;
    if (next_timer == 0)
    {
        g_wmap_land_effect_23_sequence_8_step += 1;
    }
}

/** @brief Configure the effect and reset its forty-eight resource slots. */
void wmap_land_effect_23_sequence_10_step_01(void)
{
    s32 i;

    D_80139280[21] = 1;
    D_80139280[22] = 4;
    D_80139280[23] = 0x20;
    D_80139280[24] = 0;
    D_80139280[25] = 4;
    D_80139280[26] = 1;
    D_80139280[27] = 0x82;
    D_80139280[28] = 8;
    D_80139280[29] = 0;
    D_80139280[30] = 0x4650;
    for (i = 0; i < 48; i++)
    {
        D_801AFBD0[i + 130].field_00 = 0;
        D_80139988[i + 130].data = D_8011F538;
    }
    g_wmap_land_effect_23_sequence_10_timer = 0xC0;
    g_wmap_land_effect_23_sequence_10_step++;
    wmap_land_effect_23_sequence_10_step_02();
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void wmap_land_effect_23_sequence_13_step_01(void)
{
    s32 i;
    WmapD94Entry *entry;

    i = 0xB4;
    D_80182DF4 = 1;
    D_800DCEAC = 8;

    do
    {
        D_801AFBD0[i].field_00 = 0;
        D_80139988[i].data = D_80121538;
        entry = &D_800D9268[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 0;
        entry->unk10 = -1;
        i++;
    } while (i < 0xF0);

    D_800D9154 = 1;
    g_wmap_land_effect_23_sequence_13_timer = 0x10;
    g_wmap_land_effect_23_sequence_13_step += 1;
    wmap_land_effect_23_sequence_13_step_02();
}

/**
 * @brief World-map step handler: jitter the actor position with a ramping random
 *        amplitude, then countdown-advance the step.
 */
void wmap_land_effect_23_sequence_14_step_02(void)
{
    D_801398C8.vx = rand() * D_80139264 / 16 >> 15;
    D_801398C8.vy = rand() * D_80139264 / 16 >> 15;
    D_80182D48.vx = rand() * D_80139264 / 16 >> 15;
    D_80182D48.vy = rand() * D_80139264 / 16 >> 15;
    if (D_80139264 < 0x80)
    {
        D_80139264 += 4;
    }
    if (--g_wmap_land_effect_23_sequence_14_timer == 0)
    {
        g_wmap_land_effect_23_sequence_14_step += 1;
    }
}

/**
 * @brief World-map step handler: jitter the actor with a random amplitude, and when
 *        the amplitude ramp expires, zero the offsets; otherwise countdown-advance.
 */
void wmap_land_effect_23_sequence_14_step_04(void)
{
    D_801398C8.vx = rand() * D_80139264 / 16 >> 15;
    D_801398C8.vy = rand() * D_80139264 / 16 >> 15;
    D_80182D48.vx = rand() * D_80139264 / 16 >> 15;
    D_80182D48.vy = rand() * D_80139264 / 16 >> 15;
    if (--D_80139264 < 0)
    {
        D_80182D48.vy = 0;
        D_80182D48.vx = 0;
        D_801398C8.vy = 0;
        D_801398C8.vx = 0;
        g_wmap_land_effect_23_sequence_14_step += 1;
    }
    else if (--g_wmap_land_effect_23_sequence_14_timer == 0)
    {
        g_wmap_land_effect_23_sequence_14_step += 1;
    }
}

WMAP_STEP_RUNNER(wmap_land_effect_23_run, D_800D672C, 0x4, g_wmap_land_effect_23_step, g_wmap_land_effect_23_timer)

WMAP_STEP_RESET(wmap_land_effect_23_reset, g_wmap_land_effect_23_step, g_wmap_land_effect_23_timer)

/** @brief World-map step: arm a timed callback, flag it active, then tick the sub-counter. */
void wmap_land_effect_23_step_01(void)
{
    D_80139978 = 0x10;
    g_wmap_focus_screen_position.point.x = 0xA4;
    g_wmap_focus_screen_position.point.y = 0x69;
    wmap_start_sequence(wmap_land_effect_23_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_23_step += 1;
    wmap_land_effect_23_wait_idle();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_23_wait_idle, g_wmap_land_effect_23_step, wmap_land_effect_23_step_03)

/** @brief World-map trigger: set two flags and bump a counter. */
void wmap_land_effect_23_step_03(void)
{
    D_8013B294 = 1;
    D_80139228 = 1;
    g_wmap_land_effect_23_step += 1;
}

WMAP_STEP_RUNNER(wmap_land_effect_23_run_timeline, D_800D673C, 0x24, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_23_timeline_reset, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/** @brief Set effect flags and color, play sound 41, and begin a 24-tick delay. */
void wmap_land_effect_23_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 8;
    D_801ADAE0 = 1;
    wmap_play_sound(0x29, 0x80);
    g_wmap_land_effect_23_timeline_timer = 0x18;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_02, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_23_timeline_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_23_run_sequence_14);
    g_wmap_land_effect_23_timeline_timer = 0x18;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_04, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_23_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_23_run_sequence_5);
    g_wmap_land_effect_23_timeline_timer = 0x65;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_06, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_23_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_23_run_sequence_10);
    wmap_start_sequence(wmap_land_effect_23_run_sequence_6);
    wmap_start_sequence(wmap_land_effect_23_run_sequence_2);
    g_wmap_land_effect_23_timeline_timer = 0x2;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_08, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/** @brief World-map step handler: register a callback and advance the step. */
void wmap_land_effect_23_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_23_run_sequence_1);
    g_wmap_land_effect_23_timeline_timer = 1;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_10, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/** @brief World-map step handler: seed timers and advance the counter. */
void wmap_land_effect_23_timeline_step_11(void)
{
    D_800DBE70 = 0;
    D_80139978 = -1;
    g_wmap_land_effect_23_timeline_timer = 0x2D;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_12, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_23_timeline_step_13(void)
{
    wmap_start_sequence(wmap_land_effect_23_run_sequence_7);
    g_wmap_land_effect_23_timeline_timer = 0x1C;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_14, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_23_timeline_step_15(void)
{
    wmap_start_sequence(wmap_land_effect_23_run_sequence_3);
    g_wmap_land_effect_23_timeline_timer = 0x2;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_16, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/** @brief Set world-map flags and color, then begin a 48-tick delay. */
void wmap_land_effect_23_timeline_step_17(void)
{
    D_80139244 = 1;
    g_wmap_backdrop_target_level = 1;
    wmap_start_map_tint(0x201010);
    g_wmap_land_effect_23_timeline_timer = 0x30;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_18, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_23_timeline_step_19(void)
{
    wmap_start_sequence(wmap_land_effect_23_run_sequence_11);
    g_wmap_land_effect_23_timeline_timer = 0x50;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_20, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/** @brief World-map step handler: register a callback, kick a job, advance the step. */
void wmap_land_effect_23_timeline_step_21(void)
{
    wmap_start_sequence(wmap_land_effect_23_run_sequence_13);
    wmap_start_map_tint(0x302050);
    g_wmap_backdrop_target_level = 3;
    g_wmap_land_effect_23_timeline_timer = 8;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_22, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/** @brief World-map step: set fade colour then advance to the next handler. */
void wmap_land_effect_23_timeline_step_23(void)
{
    wmap_start_map_tint(0x252035);
    g_wmap_land_effect_23_timeline_timer = 0x38;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_24, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_23_timeline_step_25(void)
{
    wmap_start_sequence(wmap_land_effect_23_run_sequence_12);
    g_wmap_land_effect_23_timeline_timer = 0x22;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_26, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_23_timeline_step_27(void)
{
    wmap_start_sequence(wmap_land_effect_23_run_sequence_9);
    wmap_start_sequence(wmap_land_effect_23_run_sequence_8);
    g_wmap_land_effect_23_timeline_timer = 0x48;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_28, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_23_timeline_step_29(void)
{
    wmap_start_sequence(wmap_land_effect_23_run_sequence_4);
    g_wmap_land_effect_23_timeline_timer = 0x2;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_30, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/** @brief World-map step: kick a sub-request, set the next state, and arm the timer. */
void wmap_land_effect_23_timeline_step_31(void)
{
    D_80139244 = 0;
    wmap_start_map_tint(0x602050);
    g_wmap_backdrop_target_level = 7;
    g_wmap_land_effect_23_timeline_timer = 0x75;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_32, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/** @brief Set the selected record value and advance to a 50-tick delay. */
void wmap_land_effect_23_timeline_step_33(void)
{
    g_wmap_land_effect_23_timeline_timer = 50;
    D_80139290[D_8011D510][D_8011D530].value = 0x117;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_23_timeline_wait_34, g_wmap_land_effect_23_timeline_step, g_wmap_land_effect_23_timeline_timer)

/**
 * @brief World-map step handler: clear the shared flag and advance the step counter.
 */
void wmap_land_effect_23_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    g_wmap_land_effect_23_timeline_step += 1;
}

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_1, D_800D67CC, 0x4, g_wmap_land_effect_23_sequence_1_step, g_wmap_land_effect_23_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_1_reset, g_wmap_land_effect_23_sequence_1_step, g_wmap_land_effect_23_sequence_1_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_23_sequence_1_step_01(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 8;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x81;
    D_800D9344.shade = 0x81;
    g_wmap_land_effect_23_sequence_1_timer = 0x206;
    g_wmap_land_effect_23_sequence_1_step += 1;
    wmap_land_effect_23_sequence_1_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_23_sequence_1_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x17, 0x8, 0);
    if (--g_wmap_land_effect_23_sequence_1_timer == 0)
    {
        g_wmap_land_effect_23_sequence_1_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_1_end, g_wmap_land_effect_23_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_2, D_800D67DC, 0x4, g_wmap_land_effect_23_sequence_2_step, g_wmap_land_effect_23_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_2_reset, g_wmap_land_effect_23_sequence_2_step, g_wmap_land_effect_23_sequence_2_timer)

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_23_sequence_2_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    g_wmap_land_effect_23_sequence_2_timer = 0x20;
    g_wmap_land_effect_23_sequence_2_step += 1;
    wmap_land_effect_23_sequence_2_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_2_end, g_wmap_land_effect_23_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_3, D_800D67EC, 0x4, g_wmap_land_effect_23_sequence_3_step, g_wmap_land_effect_23_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_3_reset, g_wmap_land_effect_23_sequence_3_step, g_wmap_land_effect_23_sequence_3_timer)

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_23_sequence_3_step_01(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    g_wmap_land_effect_23_sequence_3_timer = 0x80;
    g_wmap_land_effect_23_sequence_3_step += 1;
    wmap_land_effect_23_sequence_3_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_3_end, g_wmap_land_effect_23_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_4, D_800D67FC, 0x4, g_wmap_land_effect_23_sequence_4_step, g_wmap_land_effect_23_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_4_reset, g_wmap_land_effect_23_sequence_4_step, g_wmap_land_effect_23_sequence_4_timer)

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_23_sequence_4_step_01(void)
{
    D_8013B238 = D_80139258;
    D_80139870 = g_wmap_camera_translation;
    D_80182DF0 = 0x80;
    D_80139870.vz = 0xAFC8;
    g_wmap_land_effect_23_sequence_4_timer = 0x40;
    g_wmap_land_effect_23_sequence_4_step += 1;
    wmap_land_effect_23_sequence_4_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_4_end, g_wmap_land_effect_23_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_5, D_800D680C, 0x6, g_wmap_land_effect_23_sequence_5_step, g_wmap_land_effect_23_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_5_reset, g_wmap_land_effect_23_sequence_5_step, g_wmap_land_effect_23_sequence_5_timer)

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_23_sequence_5_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB158, D_80139F28, 0x14, 0xFF, 0x1, 0x8, 0, (s32)D_80139280);
    if (--g_wmap_land_effect_23_sequence_5_timer == 0)
    {
        g_wmap_land_effect_23_sequence_5_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_23_sequence_5_step_03(void)
{
    g_wmap_land_effect_23_sequence_5_timer = 0x20;
    D_80139280[5] = -1;
    g_wmap_land_effect_23_sequence_5_step += 1;
    wmap_land_effect_23_sequence_5_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_23_sequence_5_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB158, D_80139F28, 0x14, 0xFF, 0x1, 0x8, 0, (s32)D_80139280);
    if (--g_wmap_land_effect_23_sequence_5_timer == 0)
    {
        g_wmap_land_effect_23_sequence_5_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_5_end, g_wmap_land_effect_23_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_6, D_800D6824, 0x6, g_wmap_land_effect_23_sequence_6_step, g_wmap_land_effect_23_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_6_reset, g_wmap_land_effect_23_sequence_6_step, g_wmap_land_effect_23_sequence_6_timer)

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_23_sequence_6_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D9790, D_80139A78, 0x5A, 0xFF, 0x1, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--g_wmap_land_effect_23_sequence_6_timer == 0)
    {
        g_wmap_land_effect_23_sequence_6_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_23_sequence_6_step_03(void)
{
    g_wmap_land_effect_23_sequence_6_timer = 0x40;
    D_80139280[15] = -1;
    g_wmap_land_effect_23_sequence_6_step += 1;
    wmap_land_effect_23_sequence_6_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_23_sequence_6_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D9790, D_80139A78, 0x5A, 0xFF, 0x1, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--g_wmap_land_effect_23_sequence_6_timer == 0)
    {
        g_wmap_land_effect_23_sequence_6_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_6_end, g_wmap_land_effect_23_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_7, D_800D683C, 0x6, g_wmap_land_effect_23_sequence_7_step, g_wmap_land_effect_23_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_7_reset, g_wmap_land_effect_23_sequence_7_step, g_wmap_land_effect_23_sequence_7_timer)

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void wmap_land_effect_23_sequence_7_step_01(void)
{
    D_801B25D8 = 1;
    D_8013B238 = D_80139258;
    D_80139234 = 0;
    g_wmap_land_effect_23_sequence_7_timer = 0x12C;
    g_wmap_land_effect_23_sequence_7_step += 1;
    wmap_land_effect_23_sequence_7_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_23_sequence_7_step_03(void)
{
    g_wmap_land_effect_23_sequence_7_timer = 0x20;
    g_wmap_land_effect_23_sequence_7_step += 1;
    wmap_land_effect_23_sequence_7_step_04();
}

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_7_end, g_wmap_land_effect_23_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_8, D_800D6854, 0x6, g_wmap_land_effect_23_sequence_8_step, g_wmap_land_effect_23_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_8_reset, g_wmap_land_effect_23_sequence_8_step, g_wmap_land_effect_23_sequence_8_timer)

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void wmap_land_effect_23_sequence_8_step_01(void)
{
    D_80182DEC = 1;
    D_801B24A8 = D_80139258;
    D_8013923C = 0;
    g_wmap_land_effect_23_sequence_8_timer = 0x3C;
    g_wmap_land_effect_23_sequence_8_step += 1;
    wmap_land_effect_23_sequence_8_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_23_sequence_8_step_03(void)
{
    g_wmap_land_effect_23_sequence_8_timer = 0x10;
    g_wmap_land_effect_23_sequence_8_step += 1;
    wmap_land_effect_23_sequence_8_step_04();
}

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_8_end, g_wmap_land_effect_23_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_9, D_800D686C, 0x6, g_wmap_land_effect_23_sequence_9_step, g_wmap_land_effect_23_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_9_reset, g_wmap_land_effect_23_sequence_9_step, g_wmap_land_effect_23_sequence_9_timer)

/** @brief Initialize the actor and its screen coordinates, then advance the sequence. */
void wmap_land_effect_23_sequence_9_step_01(void)
{
    D_801399BC = D_80125538;
    D_8013924C = 0x280;
    D_800D9370.scale_index = 0xF;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 4;
    D_800D9370.target_shade = 0x81;
    D_800D9370.shade = 1;
    D_800D9370.resource_index = 0;
    D_800D9370.sequence = 0;
    g_wmap_land_effect_23_sequence_9_timer = 0x9C;
    D_80182D64.packed = g_wmap_focus_screen_position.packed;
    g_wmap_land_effect_23_sequence_9_step += 1;
    wmap_land_effect_23_sequence_9_step_02();
}

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position downward and advance when the frame counter expires.
 */
void wmap_land_effect_23_sequence_9_step_02(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, D_80182D64.packed, 0x17, 0x2, 0);
    D_80182D64.point.y = D_8013924C / 0x10;
    if (D_8013924C < 0x640)
    {
        D_8013924C += 0x10;
    }
    if (--g_wmap_land_effect_23_sequence_9_timer == 0)
    {
        g_wmap_land_effect_23_sequence_9_step += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_23_sequence_9_step_03(void)
{
    D_800D9370.shade_step = 2;
    D_800D9370.target_shade = 0;
    g_wmap_land_effect_23_sequence_9_timer = 0x40;
    g_wmap_land_effect_23_sequence_9_step += 1;
    wmap_land_effect_23_sequence_9_step_04();
}

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position and advance when the frame counter expires.
 */
void wmap_land_effect_23_sequence_9_step_04(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, D_80182D64.packed, 0x17, 0x2, 0);
    D_80182D64.point.y = D_8013924C / 0x10;
    if (D_8013924C < 0x640)
    {
        D_8013924C += 0x10;
    }
    if (--g_wmap_land_effect_23_sequence_9_timer == 0)
    {
        g_wmap_land_effect_23_sequence_9_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_9_end, g_wmap_land_effect_23_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_10, D_800D6884, 0x6, g_wmap_land_effect_23_sequence_10_step, g_wmap_land_effect_23_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_10_reset, g_wmap_land_effect_23_sequence_10_step, g_wmap_land_effect_23_sequence_10_timer)

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_23_sequence_10_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA8C0, D_80139D98, 0x30, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--g_wmap_land_effect_23_sequence_10_timer == 0)
    {
        g_wmap_land_effect_23_sequence_10_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_23_sequence_10_step_03(void)
{
    g_wmap_land_effect_23_sequence_10_timer = 0x20;
    D_80139280[25] = -1;
    g_wmap_land_effect_23_sequence_10_step += 1;
    wmap_land_effect_23_sequence_10_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_23_sequence_10_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA8C0, D_80139D98, 0x30, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--g_wmap_land_effect_23_sequence_10_timer == 0)
    {
        g_wmap_land_effect_23_sequence_10_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_10_end, g_wmap_land_effect_23_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_11, D_800D689C, 0x6, g_wmap_land_effect_23_sequence_11_step, g_wmap_land_effect_23_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_11_reset, g_wmap_land_effect_23_sequence_11_step, g_wmap_land_effect_23_sequence_11_timer)

/** @brief Initialize the actor, save its screen position, and begin a 142-tick sequence step. */
void wmap_land_effect_23_sequence_11_step_01(void)
{
    D_801399C4 = D_80123538;
    D_80139250 = 0x780;
    D_800D939C.scale_index = 0xF;
    D_800D939C.previous_sequence = -1;
    D_800D939C.shade_step = 1;
    D_800D939C.target_shade = 0x7F;
    D_800D939C.resource_index = 0;
    D_800D939C.sequence = 0;
    D_800D939C.shade = 0;
    g_wmap_land_effect_23_sequence_11_timer = 0x8E;
    D_80182D58.packed = g_wmap_focus_screen_position.packed;
    g_wmap_land_effect_23_sequence_11_step += 1;
    wmap_land_effect_23_sequence_11_step_02();
}

/** @brief Draw a world-map actor, then wind down a scrolling scalar. */
void wmap_land_effect_23_sequence_11_step_02(void)
{
    wmap_step_actor_animation(&D_800D939C, &D_801399C0);
    wmap_draw_actor_sprite(&D_800D939C, D_80182D58.packed, 0x8, 0x2, 0);
    D_80182D58.point.y = D_80139250 / 16;
    if (D_80139250 >= 0x321)
    {
        D_80139250 -= 0x10;
    }
    if (--g_wmap_land_effect_23_sequence_11_timer == 0)
    {
        g_wmap_land_effect_23_sequence_11_step += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_23_sequence_11_step_03(void)
{
    D_800D939C.shade_step = 4;
    D_800D939C.target_shade = 0;
    g_wmap_land_effect_23_sequence_11_timer = 0x20;
    g_wmap_land_effect_23_sequence_11_step += 1;
    wmap_land_effect_23_sequence_11_step_04();
}

/** @brief Draw a world-map actor, then wind down a scrolling scalar. */
void wmap_land_effect_23_sequence_11_step_04(void)
{
    wmap_step_actor_animation(&D_800D939C, &D_801399C0);
    wmap_draw_actor_sprite(&D_800D939C, D_80182D58.packed, 0x8, 0x2, 0);
    D_80182D58.point.y = D_80139250 / 16;
    if (D_80139250 >= 0x321)
    {
        D_80139250 -= 0x10;
    }
    if (--g_wmap_land_effect_23_sequence_11_timer == 0)
    {
        g_wmap_land_effect_23_sequence_11_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_11_end, g_wmap_land_effect_23_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_12, D_800D68B4, 0x6, g_wmap_land_effect_23_sequence_12_step, g_wmap_land_effect_23_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_12_reset, g_wmap_land_effect_23_sequence_12_step, g_wmap_land_effect_23_sequence_12_timer)

/** @brief Initialize the actor and begin a 48-tick sequence step. */
void wmap_land_effect_23_sequence_12_step_01(void)
{
    D_801399CC = D_80123538;
    D_80139260 = 0x320;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.resource_index = 0;
    D_800D93C8.sequence = 1;
    D_800D93C8.previous_sequence = -1;
    D_800D93C8.shade_step = 4;
    D_800D93C8.target_shade = 0x81;
    D_800D93C8.shade = 1;
    g_wmap_land_effect_23_sequence_12_timer = 0x30;
    D_80182D60.packed = g_wmap_focus_screen_position.packed;
    g_wmap_land_effect_23_sequence_12_step += 1;
    wmap_land_effect_23_sequence_12_step_02();
}

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position and advance when the frame counter expires.
 */
void wmap_land_effect_23_sequence_12_step_02(void)
{
    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, D_80182D60.packed, 0x8, 0x2, 0);
    D_80182D60.point.y = D_80139260 / 0x10;
    if (D_80139260 < 0xA00)
    {
        D_80139260 += 0x8;
    }
    if (--g_wmap_land_effect_23_sequence_12_timer == 0)
    {
        g_wmap_land_effect_23_sequence_12_step += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_23_sequence_12_step_03(void)
{
    D_800D93C8.shade_step = 4;
    D_800D93C8.target_shade = 0;
    g_wmap_land_effect_23_sequence_12_timer = 0x20;
    g_wmap_land_effect_23_sequence_12_step += 1;
    wmap_land_effect_23_sequence_12_step_04();
}

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position and advance when the frame counter expires.
 */
void wmap_land_effect_23_sequence_12_step_04(void)
{
    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, D_80182D60.packed, 0x8, 0x2, 0);
    D_80182D60.point.y = D_80139260 / 0x10;
    if (D_80139260 < 0xA00)
    {
        D_80139260 += 0x8;
    }
    if (--g_wmap_land_effect_23_sequence_12_timer == 0)
    {
        g_wmap_land_effect_23_sequence_12_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_12_end, g_wmap_land_effect_23_sequence_12_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_13, D_800D68CC, 0x8, g_wmap_land_effect_23_sequence_13_step, g_wmap_land_effect_23_sequence_13_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_13_reset, g_wmap_land_effect_23_sequence_13_step, g_wmap_land_effect_23_sequence_13_timer)

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void wmap_land_effect_23_sequence_13_step_02(void)
{
    s32 remaining;

    func_8006B328(0xB4, 0xF0, 1, -1, -1, -4, 0, 0xB, -0xFA, 0xFA, -0xDC, 0x8C, 1, 0x81, 1, 8, 1);
    D_80182DF4 += 8;
    remaining = g_wmap_land_effect_23_sequence_13_timer - 1;
    g_wmap_land_effect_23_sequence_13_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_23_sequence_13_step += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_23_sequence_13_step_03(void)
{
    g_wmap_land_effect_23_sequence_13_timer = 0x28;
    g_wmap_land_effect_23_sequence_13_step += 1;
    wmap_land_effect_23_sequence_13_step_04();
}

/** @brief World-map step handler: spawn a scripted actor and expire the timer. */
void wmap_land_effect_23_sequence_13_step_04(void)
{
    func_8006B328(0xB4, 0xF0, 1, -1, -1, -4, 0, 0xB, -0xFA, 0xFA, -0xDC,
                  0x8C, 1, 0x81, 1, 8, 1);
    if (--g_wmap_land_effect_23_sequence_13_timer == 0)
    {
        g_wmap_land_effect_23_sequence_13_step += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_23_sequence_13_step_05(void)
{
    D_800DCEAC = 0;
    g_wmap_land_effect_23_sequence_13_timer = 0x40;
    g_wmap_land_effect_23_sequence_13_step += 1;
    wmap_land_effect_23_sequence_13_step_06();
}

/** @brief World-map step handler: spawn a scripted actor and expire the timer. */
void wmap_land_effect_23_sequence_13_step_06(void)
{
    func_8006B328(0xB4, 0xF0, 1, -1, -1, -4, 0, 0xB, -0xFA, 0xFA, -0xDC,
                  0x8C, 1, 0x81, 1, 8, 1);
    if (--g_wmap_land_effect_23_sequence_13_timer == 0)
    {
        g_wmap_land_effect_23_sequence_13_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_23_sequence_13_end, g_wmap_land_effect_23_sequence_13_step)

WMAP_STEP_RUNNER(wmap_land_effect_23_run_sequence_14, D_800D68EC, 0x6, g_wmap_land_effect_23_sequence_14_step, g_wmap_land_effect_23_sequence_14_timer)

WMAP_STEP_RESET(wmap_land_effect_23_sequence_14_reset, g_wmap_land_effect_23_sequence_14_step, g_wmap_land_effect_23_sequence_14_timer)

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_23_sequence_14_step_01(void)
{
    D_80139264 = 0;
    g_wmap_land_effect_23_sequence_14_timer = 0x8C;
    g_wmap_land_effect_23_sequence_14_step += 1;
    wmap_land_effect_23_sequence_14_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_23_sequence_14_step_03(void)
{
    g_wmap_land_effect_23_sequence_14_timer = 0x40;
    g_wmap_land_effect_23_sequence_14_step += 1;
    wmap_land_effect_23_sequence_14_step_04();
}

/**
 * @brief Clear the world-map translation and rotation offsets, then advance the transition counter.
 */
void wmap_land_effect_23_sequence_14_step_05(void)
{
    D_80182D48.vy = 0;
    D_80182D48.vx = 0;
    D_801398C8.vy = 0;
    D_801398C8.vx = 0;
    g_wmap_land_effect_23_sequence_14_step += 1;
}
