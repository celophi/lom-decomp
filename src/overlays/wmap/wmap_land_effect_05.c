#include "wmap_main.h"
#include "wmap_land_effect_05.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

void wmap_land_effect_05_sequence_3_step_02(void);
void wmap_land_effect_05_sequence_10_step_02(void);
void wmap_land_effect_05_wait_idle_02(void);
void wmap_land_effect_05_step_03(void);
s32 wmap_land_effect_05_run_timeline(s32 arg0);
void wmap_land_effect_05_wait_idle_04(void);
void wmap_land_effect_05_end(void);
s32 wmap_land_effect_05_run_sequence_3(s32 arg0);
s32 wmap_land_effect_05_run_sequence_1(s32 arg0);
s32 wmap_land_effect_05_run_sequence_8(s32 arg0);
s32 wmap_land_effect_05_run_sequence_4(s32 arg0);
s32 wmap_land_effect_05_run_sequence_5(s32 arg0);
s32 wmap_land_effect_05_run_sequence_6(s32 arg0);
s32 wmap_land_effect_05_run_sequence_7(s32 arg0);
s32 wmap_land_effect_05_run_sequence_10(s32 arg0);
s32 wmap_land_effect_05_run_sequence_9(s32 arg0);
s32 wmap_land_effect_05_run_sequence_2(s32 arg0);
void wmap_land_effect_05_sequence_1_step_02(void);
void wmap_land_effect_05_sequence_2_step_02(void);
void wmap_land_effect_05_sequence_3_step_04(void);
void wmap_land_effect_05_sequence_10_step_04(void);
void wmap_land_effect_05_sequence_10_step_06(void);

/** @brief World-map actor configuration. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 field_00;
    s16 angle;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    s16 field_10;
    s16 pad_12;
} WmapMotion;

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

/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

extern s32 D_801B0FD0;
extern s32 g_wmap_land_effect_05_sequence_3_timer;
extern s8 D_80051B4C[];
extern void *D_8011CF24;
extern s32 D_80182DE4;
extern s32 D_801B2468;
extern s32 D_801B24B4;
extern s32 g_wmap_land_effect_05_sequence_4_timer;
extern u8* D_8011CF28;
extern s32 D_80182DF4;
extern s32 g_wmap_land_effect_05_sequence_5_timer;
extern u8* D_8011CF2C;
extern s32 D_801B25D8;
extern s32 g_wmap_land_effect_05_sequence_6_timer;
extern s32 D_801B25DC;
extern s32 g_wmap_land_effect_05_sequence_7_timer;
extern u8 D_800DCF18[];
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern s32 g_wmap_land_effect_05_sequence_8_timer;
extern s32 D_80182DEC;
extern u8* D_8011CF1C;
extern s32 g_wmap_land_effect_05_sequence_9_timer;
extern s32 D_800D9150;
extern s32 D_800DCEA8;
extern s32 D_80182DF0;
extern s32 g_wmap_land_effect_05_sequence_10_timer;
extern s32 g_wmap_land_effect_05_timer;
extern void (*D_800D5D98[])(void);
extern void wmap_land_effect_05_step_03(void);
extern void wmap_land_effect_05_end(void);
extern s32 g_wmap_land_effect_05_timeline_timer;
extern void (*D_800D5DB0[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern s32 D_80139244;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 g_wmap_land_effect_05_sequence_1_timer;
extern void (*D_800D5E08[])(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern void wmap_land_effect_05_sequence_1_step_02(void);
extern s32 g_wmap_land_effect_05_sequence_2_timer;
extern void (*D_800D5E18[])(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern void wmap_land_effect_05_sequence_2_step_02(void);
extern void (*D_800D5E28[])(void);
extern void (*D_800D5E40[])(void);
extern void (*D_800D5E58[])(void);
extern void (*D_800D5E70[])(void);
extern void (*D_800D5E88[])(void);
extern void (*D_800D5EA0[])(void);
extern void (*D_800D5EB0[])(void);
extern void (*D_800D5EC0[])(void);
extern u8 D_80121538[];
extern u32 g_wmap_land_effect_05_sequence_3_step;
extern u32 g_wmap_land_effect_05_sequence_4_step;
extern u32 g_wmap_land_effect_05_sequence_5_step;
extern u32 g_wmap_land_effect_05_sequence_6_step;
extern u8* D_8011CF30;
extern u32 g_wmap_land_effect_05_sequence_7_step;
extern u32 g_wmap_land_effect_05_sequence_8_step;
extern u32 g_wmap_land_effect_05_sequence_9_step;
extern u32 g_wmap_land_effect_05_sequence_10_step;
extern u32 g_wmap_land_effect_05_step;
extern u32 g_wmap_land_effect_05_timeline_step;
extern u32 g_wmap_land_effect_05_sequence_1_step;
extern u32 g_wmap_land_effect_05_sequence_2_step;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;
extern VECTOR D_80139888;
extern VECTOR D_80139898;
extern VECTOR D_801B2660;

extern SVECTOR D_80139258;
extern SVECTOR D_801B2498;
extern SVECTOR D_8013B240;
extern SVECTOR D_801B24A0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2670;
extern SVECTOR D_801B24A8;
extern SVECTOR D_801B2678;

extern WmapSpriteActor D_800D9268[];
extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern WmapMotion D_801AFBD0[];

/** @brief Initialize the actor group and its animation resources, then advance. */
void wmap_land_effect_05_sequence_3_step_01(void)
{
    s32 i;
    WmapConfigA* config;

    D_801B0FD0 = 5;
    for (i = 200; i < 205; i++)
    {
        config = &D_800D9268[i];
        D_80139988[i].data = D_80121538;
        config->field_02 = 0;
        config->field_06 = 15;
        config->field_0E = 0;
        config->field_10 = -1;
        config->field_22 = 63;
        config->field_24 = 2;
        config->field_26 = 2;
        D_801AFBD0[i].field_00 = 1;
        D_801AFBD0[i].angle = i * 0x333;
        D_801AFBD0[i].field_08 = 340000;
        D_801AFBD0[i].field_04 = 0;
        D_801AFBD0[i].field_0C = 9999;
        D_801AFBD0[i].field_0E = 0;
        D_801AFBD0[i].field_10 = 80;
    }
    g_wmap_land_effect_05_sequence_3_timer = 180;
    g_wmap_land_effect_05_sequence_3_step++;
    wmap_land_effect_05_sequence_3_step_02();
}

/** @brief Draw two oscillating effect layers and update their intensity. */
void wmap_land_effect_05_sequence_4_step_02(void)
{
    s32 first_frame;
    s32 intensity;
    s32 remaining;

    PushMatrix();
    first_frame = (s32) (D_80051B4C[D_801B24B4] + 0x80) >> 5;
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(D_8011CF24, first_frame, 0x14, 0x35, 0x7800, 0x1000, D_801B2468);
    first_frame = (s32) (D_80051B4C[D_80182DE4] + 0x80) >> 5;
    D_801B2490.vz = (u16) (D_801B2490.vz - 0x18);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(D_8011CF24, first_frame, 0x14, 0x35, 0x7800, 0x1000, D_801B2468);
    D_801B2498.vz = (u16) (D_801B2498.vz + 0x30);
    PopMatrix();
    D_801B24B4 = (D_801B24B4 + 4) & 0xFF;
    D_80182DE4 = (D_80182DE4 + 2) & 0xFF;
    intensity = D_801B2468 + 1;
    D_801B2468 = intensity;
    if (intensity >= 0x81)
    {
        D_801B2468 = 0x80;
    }
    remaining = g_wmap_land_effect_05_sequence_4_timer - 1;
    g_wmap_land_effect_05_sequence_4_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_05_sequence_4_step += 1;
    }
}

/** @brief Animate and fade two counter-rotating effect layers. */
void wmap_land_effect_05_sequence_4_step_04(void)
{
    s32 first_frame;
    
    s32 intensity;
    s32 remaining;

    PushMatrix();
    first_frame = (s32) (D_80051B4C[D_801B24B4] + 0x80) >> 5;
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(D_8011CF24, first_frame, 0x14, 0x35, 0x7800, 0x1000, D_801B2468);
    first_frame = (s32) (D_80051B4C[D_80182DE4] + 0x80) >> 5;
    D_801B2490.vz = (u16) (D_801B2490.vz - 0x18);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(D_8011CF24, first_frame, 0x14, 0x35, 0x7800, 0x1000, D_801B2468);
    D_801B2498.vz = (u16) (D_801B2498.vz + 0x30);
    PopMatrix();
    D_801B24B4 = (D_801B24B4 + 4) & 0xFF;
    D_80182DE4 = (D_80182DE4 + 2) & 0xFF;
    intensity = D_801B2468 - 4;
    D_801B2468 = intensity;
    if (intensity < 0)
    {
        D_801B2468 = 0;
    }
    remaining = g_wmap_land_effect_05_sequence_4_timer - 1;
    g_wmap_land_effect_05_sequence_4_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_05_sequence_4_step += 1;
    }
}

/**
 * @brief World-map step handler: decay a timer with a floor, draw the model, ramp a
 *        secondary counter to its cap, scroll the field, then countdown-advance the step.
 */
void wmap_land_effect_05_sequence_5_step_02(void)
{
    s32 v;

    v = D_80139888.vz - 0x5DC;
    D_80139888.vz = v;
    if (v < 0x7530)
    {
        D_80139888.vz = 0x7530;
    }
    PushMatrix();
    wmap_set_model_transform(&D_80139888, &D_8013B240);
    wmap_draw_model_default(D_8011CF28, 0, 0xC, 0x35, 0x7840, 1, D_80182DF4);
    D_80182DF4 += 1;
    if (D_80182DF4 >= 0x42)
    {
        D_80182DF4 = 0x41;
    }
    D_8013B240.vz += 2;
    PopMatrix();
    if (--g_wmap_land_effect_05_sequence_5_timer == 0)
    {
        g_wmap_land_effect_05_sequence_5_step += 1;
    }
}

/**
 * @brief World-map step handler: clamp a fade level, draw a sprite, and expire the step.
 */
void wmap_land_effect_05_sequence_5_step_04(void)
{
    u8 *p;
    u8 *q;

    p = &D_80139888;
    if (*(s32 *)(p + 8) < 0x7530)
    {
        *(s32 *)(p + 8) = 0x7530;
    }
    PushMatrix();
    q = &D_8013B240;
    wmap_set_model_transform(p, q);
    if (D_80182DF4 != 0)
    {
        wmap_draw_model_default(D_8011CF28, 0, 0xC, 0x35, 0x7840, 1, D_80182DF4);
        D_80182DF4 -= 1;
        if (D_80182DF4 < 0)
        {
            D_80182DF4 = 0;
        }
        *(s16 *)(q + 4) += 2;
    }
    PopMatrix();
    if (--g_wmap_land_effect_05_sequence_5_timer == 0)
    {
        g_wmap_land_effect_05_sequence_5_step += 1;
    }
}

/**
 * @brief World-map step handler: decay a timer with a floor, draw the model, ramp a
 *        secondary counter to its cap, scroll the field, then countdown-advance the step.
 */
void wmap_land_effect_05_sequence_6_step_02(void)
{
    s32 v;

    v = D_80139898.vz - 0x5DC;
    D_80139898.vz = v;
    if (v < 0x7530)
    {
        D_80139898.vz = 0x7530;
    }
    PushMatrix();
    wmap_set_model_transform(&D_80139898, &D_801B2670);
    wmap_draw_model_default(D_8011CF2C, 0, 0xC, 0x35, 0x7840, 1, D_801B25D8);
    D_801B25D8 += 1;
    if (D_801B25D8 >= 0x42)
    {
        D_801B25D8 = 0x41;
    }
    D_801B2670.vz -= 3;
    PopMatrix();
    if (--g_wmap_land_effect_05_sequence_6_timer == 0)
    {
        g_wmap_land_effect_05_sequence_6_step += 1;
    }
}

/**
 * @brief World-map step handler: clamp a fade level, draw a sprite, and expire the step.
 */
void wmap_land_effect_05_sequence_6_step_04(void)
{
    u8 *p;
    u8 *q;

    p = &D_80139898;
    if (*(s32 *)(p + 8) < 0x7530)
    {
        *(s32 *)(p + 8) = 0x7530;
    }
    PushMatrix();
    q = &D_801B2670;
    wmap_set_model_transform(p, q);
    if (D_801B25D8 != 0)
    {
        wmap_draw_model_default(D_8011CF2C, 0, 0xC, 0x35, 0x7840, 1, D_801B25D8);
        D_801B25D8 -= 1;
        if (D_801B25D8 < 0)
        {
            D_801B25D8 = 0;
        }
        *(s16 *)(q + 4) += -3;
    }
    PopMatrix();
    if (--g_wmap_land_effect_05_sequence_6_timer == 0)
    {
        g_wmap_land_effect_05_sequence_6_step += 1;
    }
}

/**
 * @brief World-map step handler: decay a timer with a floor, draw the model, ramp a
 *        secondary counter to its cap, scroll the field, then countdown-advance the step.
 */
void wmap_land_effect_05_sequence_7_step_02(void)
{
    s32 v;

    v = D_801B2660.vz - 0x5DC;
    D_801B2660.vz = v;
    if (v < 0x7530)
    {
        D_801B2660.vz = 0x7530;
    }
    PushMatrix();
    wmap_set_model_transform(&D_801B2660, &D_801B2678);
    wmap_draw_model_default(D_8011CF30, 0, 0xC, 0x35, 0x7840, 1, D_801B25DC);
    D_801B25DC += 1;
    if (D_801B25DC >= 0x42)
    {
        D_801B25DC = 0x41;
    }
    D_801B2678.vz += 4;
    PopMatrix();
    if (--g_wmap_land_effect_05_sequence_7_timer == 0)
    {
        g_wmap_land_effect_05_sequence_7_step += 1;
    }
}

/** @brief Draw the fading effect and advance its countdown. */
void wmap_land_effect_05_sequence_7_step_04(void)
{
    s32 intensity;
    s32 remaining;

    if (D_801B2660.vz < 0x7530)
    {
        D_801B2660.vz = 0x7530;
    }
    PushMatrix();
    wmap_set_model_transform(&D_801B2660, &D_801B2678);
    if (D_801B25DC != 0)
    {
        wmap_draw_model_default(D_8011CF30, 0, 0xC, 0x35, 0x7840, 1, D_801B25DC);
        intensity = D_801B25DC - 1;
        D_801B25DC = intensity;
        if (intensity < 0)
        {
            D_801B25DC = 0;
        }
        D_801B2678.vz = (u16) (D_801B2678.vz + 4);
    }
    PopMatrix();
    remaining = g_wmap_land_effect_05_sequence_7_timer - 1;
    g_wmap_land_effect_05_sequence_7_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_05_sequence_7_step += 1;
    }
}

/** @brief Draw the fading effect and advance its countdown. */
void wmap_land_effect_05_sequence_8_step_02(void)
{
    MATRIX transform;
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = D_801B2650.vz - 0xDAC;
    D_801B2650.vz = depth;
    if (depth < 0x2710)
    {
        D_801B2650.vz = 0x2710;
    }
    PushMatrix();
    RotMatrix(&D_801B24A0, &transform);
    TransMatrix(&transform, &D_8011CF60);
    SetRotMatrix(&transform);
    SetTransMatrix(&transform);
    if (D_80182DE8 != 0)
    {
        wmap_draw_model_default(D_800DCF18, 0, 4, 0x35, 0x7800, 1, D_80182DE8);
        intensity = D_80182DE8 - 2;
        D_80182DE8 = intensity;
        if (intensity < 0)
        {
            D_80182DE8 = 0;
        }
    }
    PopMatrix();
    remaining = g_wmap_land_effect_05_sequence_8_timer - 1;
    g_wmap_land_effect_05_sequence_8_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_05_sequence_8_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_05_sequence_9_step_02(void)
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
    if (--g_wmap_land_effect_05_sequence_9_timer == 0)
    {
        g_wmap_land_effect_05_sequence_9_step += 1;
    }
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void wmap_land_effect_05_sequence_10_step_01(void)
{
    s32 i;
    WmapD94Entry *entry;

    i = 100;
    D_80182DF0 = 1;
    D_800DCEA8 = 1;

    do
    {
        D_801AFBD0[i].field_00 = 0;
        D_80139988[i].data = D_80121538;
        entry = &D_800D9268[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 1;
        entry->unk10 = -1;
        i++;
    } while (i < 200);

    D_800D9150 = 1;
    g_wmap_land_effect_05_sequence_10_timer = 0x10;
    g_wmap_land_effect_05_sequence_10_step += 1;
    wmap_land_effect_05_sequence_10_step_02();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_05_run(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_05_step = 1;
        g_wmap_land_effect_05_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_05_step < 0x6)
    {
        D_800D5D98[g_wmap_land_effect_05_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void wmap_land_effect_05_reset(void)
{
    g_wmap_land_effect_05_step = 1;
    g_wmap_land_effect_05_timer = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_05_step_01(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_05_step += 1;
    wmap_land_effect_05_wait_idle_02();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_05_wait_idle_02(void)
{
    if (g_wmap_sequence_busy == 0)
    {
        g_wmap_land_effect_05_step += 1;
        wmap_land_effect_05_step_03();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_05_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_05_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_05_step += 1;
    wmap_land_effect_05_wait_idle_04();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_05_wait_idle_04(void)
{
    if (g_wmap_sequence_busy == 0)
    {
        g_wmap_land_effect_05_step += 1;
        wmap_land_effect_05_end();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_05_end(void)
{
    g_wmap_land_effect_05_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_05_run_timeline(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_05_timeline_step = 1;
        g_wmap_land_effect_05_timeline_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_05_timeline_step < 0x16)
    {
        D_800D5DB0[g_wmap_land_effect_05_timeline_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void wmap_land_effect_05_timeline_reset(void)
{
    g_wmap_land_effect_05_timeline_step = 1;
    g_wmap_land_effect_05_timeline_timer = 1;
}

/** @brief World-map step: mark active, request a resource, seed the sub-counter, tick. */
void wmap_land_effect_05_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_play_sound(0x25, 0x80);
    g_wmap_land_effect_05_timeline_timer = 8;
    g_wmap_land_effect_05_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_05_timeline_wait_02(void)
{
    if (--g_wmap_land_effect_05_timeline_timer == 0)
    {
        g_wmap_land_effect_05_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_05_timeline_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_05_run_sequence_3);
    g_wmap_land_effect_05_timeline_timer = 0x1E;
    g_wmap_land_effect_05_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_05_timeline_wait_04(void)
{
    if (--g_wmap_land_effect_05_timeline_timer == 0)
    {
        g_wmap_land_effect_05_timeline_step += 1;
    }
}

/** @brief Register two callbacks around a color and state update and begin a two-tick delay. */
void wmap_land_effect_05_timeline_step_05(void)
{
    wmap_start_sequence(&wmap_land_effect_05_run_sequence_8);
    wmap_start_map_tint(0x501040);
    g_wmap_backdrop_target_level = 4;
    D_801ADAE0 = 1;
    wmap_start_sequence(&wmap_land_effect_05_run_sequence_1);
    g_wmap_land_effect_05_timeline_timer = 2;
    g_wmap_land_effect_05_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_05_timeline_wait_06(void)
{
    if (--g_wmap_land_effect_05_timeline_timer == 0)
    {
        g_wmap_land_effect_05_timeline_step += 1;
    }
}

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void wmap_land_effect_05_timeline_step_07(void)
{
    D_80139244 = 1;
    g_wmap_land_effect_05_timeline_timer = 0x16;
    g_wmap_land_effect_05_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_05_timeline_wait_08(void)
{
    if (--g_wmap_land_effect_05_timeline_timer == 0)
    {
        g_wmap_land_effect_05_timeline_step += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_05_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_05_run_sequence_4);
    wmap_start_sequence(wmap_land_effect_05_run_sequence_5);
    g_wmap_land_effect_05_timeline_timer = 0x4;
    g_wmap_land_effect_05_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_05_timeline_wait_10(void)
{
    if (--g_wmap_land_effect_05_timeline_timer == 0)
    {
        g_wmap_land_effect_05_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_05_timeline_step_11(void)
{
    wmap_start_sequence(wmap_land_effect_05_run_sequence_6);
    g_wmap_land_effect_05_timeline_timer = 0x2;
    g_wmap_land_effect_05_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_05_timeline_wait_12(void)
{
    if (--g_wmap_land_effect_05_timeline_timer == 0)
    {
        g_wmap_land_effect_05_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_05_timeline_step_13(void)
{
    wmap_start_sequence(wmap_land_effect_05_run_sequence_7);
    g_wmap_land_effect_05_timeline_timer = 0x2;
    g_wmap_land_effect_05_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_05_timeline_wait_14(void)
{
    if (--g_wmap_land_effect_05_timeline_timer == 0)
    {
        g_wmap_land_effect_05_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_05_timeline_step_15(void)
{
    wmap_start_sequence(wmap_land_effect_05_run_sequence_10);
    g_wmap_land_effect_05_timeline_timer = 0x74;
    g_wmap_land_effect_05_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_05_timeline_wait_16(void)
{
    if (--g_wmap_land_effect_05_timeline_timer == 0)
    {
        g_wmap_land_effect_05_timeline_step += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_05_timeline_step_17(void)
{
    wmap_start_sequence(wmap_land_effect_05_run_sequence_9);
    wmap_start_sequence(wmap_land_effect_05_run_sequence_2);
    g_wmap_land_effect_05_timeline_timer = 0x2;
    g_wmap_land_effect_05_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_05_timeline_wait_18(void)
{
    if (--g_wmap_land_effect_05_timeline_timer == 0)
    {
        g_wmap_land_effect_05_timeline_step += 1;
    }
}

/** @brief Clear the world-map value, set the drawing color, and start a 140-tick delay. */
void wmap_land_effect_05_timeline_step_19(void)
{
    D_80139244 = 0;
    wmap_start_map_tint(0x403060);
    g_wmap_land_effect_05_timeline_timer = 0x8C;
    g_wmap_land_effect_05_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_05_timeline_wait_20(void)
{
    if (--g_wmap_land_effect_05_timeline_timer == 0)
    {
        g_wmap_land_effect_05_timeline_step += 1;
    }
}

void wmap_land_effect_05_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    g_wmap_land_effect_05_timeline_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_05_run_sequence_1(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_05_sequence_1_step = 1;
        g_wmap_land_effect_05_sequence_1_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_05_sequence_1_step < 0x4)
    {
        D_800D5E08[g_wmap_land_effect_05_sequence_1_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void wmap_land_effect_05_sequence_1_reset(void)
{
    g_wmap_land_effect_05_sequence_1_step = 1;
    g_wmap_land_effect_05_sequence_1_timer = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_05_sequence_1_step_01(void)
{
    D_801399AC = D_8011D538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 8;
    D_800D9318.target_shade = 0x81;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.shade = 1;
    g_wmap_land_effect_05_sequence_1_timer = 0x96;
    g_wmap_land_effect_05_sequence_1_step += 1;
    wmap_land_effect_05_sequence_1_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_05_sequence_1_step_02(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0xF, 0x6, 0);
    if (--g_wmap_land_effect_05_sequence_1_timer == 0)
    {
        g_wmap_land_effect_05_sequence_1_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_05_sequence_1_end(void)
{
    g_wmap_land_effect_05_sequence_1_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_05_run_sequence_2(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_05_sequence_2_step = 1;
        g_wmap_land_effect_05_sequence_2_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_05_sequence_2_step < 0x4)
    {
        D_800D5E18[g_wmap_land_effect_05_sequence_2_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void wmap_land_effect_05_sequence_2_reset(void)
{
    g_wmap_land_effect_05_sequence_2_step = 1;
    g_wmap_land_effect_05_sequence_2_timer = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_05_sequence_2_step_01(void)
{
    D_801399B4 = D_8011F538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 8;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0;
    g_wmap_land_effect_05_sequence_2_timer = 0x90;
    g_wmap_land_effect_05_sequence_2_step += 1;
    wmap_land_effect_05_sequence_2_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_05_sequence_2_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x18, 0xA, 0);
    if (--g_wmap_land_effect_05_sequence_2_timer == 0)
    {
        g_wmap_land_effect_05_sequence_2_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_05_sequence_2_end(void)
{
    g_wmap_land_effect_05_sequence_2_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_05_run_sequence_3(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_05_sequence_3_step = 1;
        g_wmap_land_effect_05_sequence_3_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_05_sequence_3_step < 0x6)
    {
        D_800D5E28[g_wmap_land_effect_05_sequence_3_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void wmap_land_effect_05_sequence_3_reset(void)
{
    g_wmap_land_effect_05_sequence_3_step = 1;
    g_wmap_land_effect_05_sequence_3_timer = 1;
}

/**
 * @brief Draw the world-map cursor via func_8006B6EC, then advance after the wait expires.
 */
void wmap_land_effect_05_sequence_3_step_02(void)
{
    func_8006B6EC(0xC8, 0xCD, 0xF, 0, 0x6);
    if (--g_wmap_land_effect_05_sequence_3_timer == 0)
    {
        g_wmap_land_effect_05_sequence_3_step += 1;
    }
}

/**
 * @brief Reset a range of world-map actor configs, arm the timer, and advance the step.
 */
void wmap_land_effect_05_sequence_3_step_03(void)
{
    s32 i;

    for (i = 0xC8; i < 0xCD; i++)
    {
        D_800D9268[i].target_shade = 0;
        D_800D9268[i].shade_step = 8;
    }
    g_wmap_land_effect_05_sequence_3_timer = 0x10;
    g_wmap_land_effect_05_sequence_3_step += 1;
    wmap_land_effect_05_sequence_3_step_04();
}

/**
 * @brief Draw the world-map cursor via func_8006B6EC, then advance after the wait expires.
 */
void wmap_land_effect_05_sequence_3_step_04(void)
{
    func_8006B6EC(0xC8, 0xCD, 0xF, 0, 0x6);
    if (--g_wmap_land_effect_05_sequence_3_timer == 0)
    {
        g_wmap_land_effect_05_sequence_3_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_05_sequence_3_end(void)
{
    g_wmap_land_effect_05_sequence_3_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_05_run_sequence_4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_05_sequence_4_step = 1;
        g_wmap_land_effect_05_sequence_4_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_05_sequence_4_step < 0x6)
    {
        D_800D5E40[g_wmap_land_effect_05_sequence_4_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void wmap_land_effect_05_sequence_4_reset(void)
{
    g_wmap_land_effect_05_sequence_4_step = 1;
    g_wmap_land_effect_05_sequence_4_timer = 1;
}

/** @brief World-map step: reset the sprite counters and advance to the next handler. */
void wmap_land_effect_05_sequence_4_step_01(void)
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
    g_wmap_land_effect_05_sequence_4_timer = 0x60;
    g_wmap_land_effect_05_sequence_4_step += 1;
    wmap_land_effect_05_sequence_4_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_05_sequence_4_step_03(void)
{
    g_wmap_land_effect_05_sequence_4_timer = 0x20;
    g_wmap_land_effect_05_sequence_4_step += 1;
    wmap_land_effect_05_sequence_4_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_05_sequence_4_end(void)
{
    g_wmap_land_effect_05_sequence_4_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_05_run_sequence_5(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_05_sequence_5_step = 1;
        g_wmap_land_effect_05_sequence_5_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_05_sequence_5_step < 0x6)
    {
        D_800D5E58[g_wmap_land_effect_05_sequence_5_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void wmap_land_effect_05_sequence_5_reset(void)
{
    g_wmap_land_effect_05_sequence_5_step = 1;
    g_wmap_land_effect_05_sequence_5_timer = 1;
}

/** @brief World-map step handler: seed the actor block, arm the timer, and advance. */
void wmap_land_effect_05_sequence_5_step_01(void)
{
    D_8013B240 = D_80139258;
    D_80139888 = g_wmap_camera_translation;
    D_80182DF4 = 1;
    D_80139888.vz = 0xBB8;
    g_wmap_land_effect_05_sequence_5_timer = 0x48;
    g_wmap_land_effect_05_sequence_5_step += 1;
    wmap_land_effect_05_sequence_5_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_05_sequence_5_step_03(void)
{
    g_wmap_land_effect_05_sequence_5_timer = 0x40;
    g_wmap_land_effect_05_sequence_5_step += 1;
    wmap_land_effect_05_sequence_5_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_05_sequence_5_end(void)
{
    g_wmap_land_effect_05_sequence_5_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_05_run_sequence_6(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_05_sequence_6_step = 1;
        g_wmap_land_effect_05_sequence_6_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_05_sequence_6_step < 0x6)
    {
        D_800D5E70[g_wmap_land_effect_05_sequence_6_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void wmap_land_effect_05_sequence_6_reset(void)
{
    g_wmap_land_effect_05_sequence_6_step = 1;
    g_wmap_land_effect_05_sequence_6_timer = 1;
}

/** @brief World-map step handler: seed the actor block, arm the timer, and advance. */
void wmap_land_effect_05_sequence_6_step_01(void)
{
    D_801B2670 = D_80139258;
    D_80139898 = g_wmap_camera_translation;
    D_801B25D8 = 1;
    D_80139898.vz = 0x7530;
    g_wmap_land_effect_05_sequence_6_timer = 0x7C;
    g_wmap_land_effect_05_sequence_6_step += 1;
    wmap_land_effect_05_sequence_6_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_05_sequence_6_step_03(void)
{
    g_wmap_land_effect_05_sequence_6_timer = 0x40;
    g_wmap_land_effect_05_sequence_6_step += 1;
    wmap_land_effect_05_sequence_6_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_05_sequence_6_end(void)
{
    g_wmap_land_effect_05_sequence_6_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_05_run_sequence_7(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_05_sequence_7_step = 1;
        g_wmap_land_effect_05_sequence_7_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_05_sequence_7_step < 0x6)
    {
        D_800D5E88[g_wmap_land_effect_05_sequence_7_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void wmap_land_effect_05_sequence_7_reset(void)
{
    g_wmap_land_effect_05_sequence_7_step = 1;
    g_wmap_land_effect_05_sequence_7_timer = 1;
}

/** @brief World-map step handler: seed the actor block, arm the timer, and advance. */
void wmap_land_effect_05_sequence_7_step_01(void)
{
    D_801B2678 = D_80139258;
    D_801B2660 = g_wmap_camera_translation;
    D_801B25DC = 1;
    D_801B2660.vz = 0x7530;
    g_wmap_land_effect_05_sequence_7_timer = 0x7C;
    g_wmap_land_effect_05_sequence_7_step += 1;
    wmap_land_effect_05_sequence_7_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_05_sequence_7_step_03(void)
{
    g_wmap_land_effect_05_sequence_7_timer = 0x40;
    g_wmap_land_effect_05_sequence_7_step += 1;
    wmap_land_effect_05_sequence_7_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_05_sequence_7_end(void)
{
    g_wmap_land_effect_05_sequence_7_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_05_run_sequence_8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_05_sequence_8_step = 1;
        g_wmap_land_effect_05_sequence_8_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_05_sequence_8_step < 0x4)
    {
        D_800D5EA0[g_wmap_land_effect_05_sequence_8_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void wmap_land_effect_05_sequence_8_reset(void)
{
    g_wmap_land_effect_05_sequence_8_step = 1;
    g_wmap_land_effect_05_sequence_8_timer = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_05_sequence_8_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    g_wmap_land_effect_05_sequence_8_timer = 0x40;
    g_wmap_land_effect_05_sequence_8_step += 1;
    wmap_land_effect_05_sequence_8_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_05_sequence_8_end(void)
{
    g_wmap_land_effect_05_sequence_8_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_05_run_sequence_9(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_05_sequence_9_step = 1;
        g_wmap_land_effect_05_sequence_9_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_05_sequence_9_step < 0x4)
    {
        D_800D5EB0[g_wmap_land_effect_05_sequence_9_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void wmap_land_effect_05_sequence_9_reset(void)
{
    g_wmap_land_effect_05_sequence_9_step = 1;
    g_wmap_land_effect_05_sequence_9_timer = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_05_sequence_9_step_01(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    g_wmap_land_effect_05_sequence_9_timer = 0x40;
    g_wmap_land_effect_05_sequence_9_step += 1;
    wmap_land_effect_05_sequence_9_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_05_sequence_9_end(void)
{
    g_wmap_land_effect_05_sequence_9_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_05_run_sequence_10(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_05_sequence_10_step = 1;
        g_wmap_land_effect_05_sequence_10_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_05_sequence_10_step < 0x8)
    {
        D_800D5EC0[g_wmap_land_effect_05_sequence_10_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void wmap_land_effect_05_sequence_10_reset(void)
{
    g_wmap_land_effect_05_sequence_10_step = 1;
    g_wmap_land_effect_05_sequence_10_timer = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void wmap_land_effect_05_sequence_10_step_02(void)
{
    s32 remaining;

    func_8006B328(0x64, 0xC8, 1, -1, -2, -2, 0, 0xF, -0xAF, 0x15E, -0xAF, 0x15E, 0x32, 1, 0x81, 2, 0);
    D_80182DF0 += 8;
    remaining = g_wmap_land_effect_05_sequence_10_timer - 1;
    g_wmap_land_effect_05_sequence_10_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_05_sequence_10_step += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_05_sequence_10_step_03(void)
{
    g_wmap_land_effect_05_sequence_10_timer = 0x40;
    g_wmap_land_effect_05_sequence_10_step += 1;
    wmap_land_effect_05_sequence_10_step_04();
}

/** @brief World-map step handler: spawn a sprite via a long argument list, then countdown-advance. */
void wmap_land_effect_05_sequence_10_step_04(void)
{
    func_8006B328(0x64, 0xC8, 1, -1, -2, -2, 0, 0xF, -0xAF, 0x15E, -0xAF, 0x15E,
                  0x32, 1, 0x81, 2, 0);
    if (--g_wmap_land_effect_05_sequence_10_timer == 0)
    {
        g_wmap_land_effect_05_sequence_10_step += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_05_sequence_10_step_05(void)
{
    D_800DCEA8 = 0;
    g_wmap_land_effect_05_sequence_10_timer = 0x40;
    g_wmap_land_effect_05_sequence_10_step += 1;
    wmap_land_effect_05_sequence_10_step_06();
}

/** @brief World-map step handler: spawn a sprite via a long argument list, then countdown-advance. */
void wmap_land_effect_05_sequence_10_step_06(void)
{
    func_8006B328(0x64, 0xC8, 1, -1, -2, -2, 0, 0xF, -0xAF, 0x15E, -0xAF, 0x15E,
                  0x32, 1, 0x81, 2, 0);
    if (--g_wmap_land_effect_05_sequence_10_timer == 0)
    {
        g_wmap_land_effect_05_sequence_10_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_05_sequence_10_end(void)
{
    g_wmap_land_effect_05_sequence_10_step += 1;
}
