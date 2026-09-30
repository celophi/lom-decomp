#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_land_effect_00.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

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
extern s32 D_801B2AFC;
extern s32 D_80182DEC;
extern u8* D_8011CF24;
extern s32 D_801B2B04;
extern u8 D_800DCF18[];
extern s32 D_801B2B0C;
extern s32 D_80182DF4;
extern s32 D_80139234;
extern s32 D_80182DF0;
extern s32 D_800DCEA8;
extern s32 D_800D9150;
extern s32 D_801B2B14;
extern s32 D_801B0FD0;
extern s32 D_801B2B1C;
extern s32 D_801B2AD4;
extern void (*D_800D6028[])(void);
extern s32 D_8013B20C;
extern void wmap_land_effect_00_step_03(void);
extern s32 D_801B2ADC;
extern void (*D_800D6048[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2AE4;
extern void (*D_800D6090[])(void);
extern u8* D_801399AC;
extern u8 D_8011F538[];
extern s32 D_801B2AEC;
extern void (*D_800D60A0[])(void);
extern u8* D_801399CC;
extern void wmap_land_effect_00_sequence_2_step_02(void);
extern void wmap_land_effect_00_sequence_2_step_04(void);
extern s32 D_801B2AF4;
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
extern s32 D_801B2B24;
extern void (*D_800D6138[])(void);
extern u8 D_80123538[];
extern u8* D_801399D4;
extern void wmap_land_effect_00_sequence_9_step_02(void);
extern void wmap_land_effect_00_sequence_9_step_04(void);

extern u32 D_801B2AF8;
extern u32 D_801B2B00;

extern u32 D_801B2B08;
extern u8 D_80121538[];
extern u32 D_801B2B10;
extern u32 D_801B2B18;
extern u32 D_801B2AD0;
extern u32 D_801B2AD8;
extern u32 D_801B2AE0;
extern u32 D_801B2AE8;
extern u32 D_801B2AF0;
extern u32 D_801B2B20;

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
    if (--D_801B2AFC == 0)
    {
        D_801B2AF8 += 1;
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
    if (--D_801B2B04 == 0)
    {
        D_801B2B00 += 1;
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
    remaining = D_801B2B0C - 1;
    D_8013B240.vz = (u16)(D_8013B240.vz + 220);
    D_801B2B0C = remaining;
    if (remaining == 0)
    {
        D_801B2B08++;
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
    remaining = D_801B2B0C - 1;
    D_801B2B0C = remaining;
    if (remaining == 0)
    {
        D_801B2B08++;
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
    D_801B2B14 = 0x10;
    D_801B2B10 += 1;
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
    D_801B2B1C = 100;
    D_801B2B18++;
    wmap_land_effect_00_sequence_8_step_02();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_00_run(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2AD0 = 1;
        D_801B2AD4 = 1;
        return 1;
    }

    if (D_801B2AD0 < 0x8)
    {
        D_800D6028[D_801B2AD0]();
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
void wmap_land_effect_00_reset(void)
{
    D_801B2AD0 = 1;
    D_801B2AD4 = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_00_step_01(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    D_8013B20C = 1;
    D_801B2AD0 += 1;
    wmap_land_effect_00_wait_idle_02();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_00_wait_idle_02(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2AD0 += 1;
        wmap_land_effect_00_step_03();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_00_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_00_run_timeline);
    D_8013B20C = 1;
    D_801B2AD0 += 1;
    wmap_land_effect_00_wait_idle_04();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_00_wait_idle_04(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2AD0 += 1;
        wmap_land_effect_00_step_05();
    }
}

/**
 * @brief Reset a world-map step slot: arm its wait and advance the step index.
 */
void wmap_land_effect_00_step_05(void)
{
    D_801B2AD4 = 5;
    D_801B2AD0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_00_wait_06(void)
{
    if (--D_801B2AD4 == 0)
    {
        D_801B2AD0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_00_end(void)
{
    D_801B2AD0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_00_run_timeline(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2AD8 = 1;
        D_801B2ADC = 1;
        return 1;
    }

    if (D_801B2AD8 < 0x12)
    {
        D_800D6048[D_801B2AD8]();
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
void wmap_land_effect_00_timeline_reset(void)
{
    D_801B2AD8 = 1;
    D_801B2ADC = 1;
}

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
    D_801B2ADC = 0x1E;
    D_801B2AD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_00_timeline_wait_02(void)
{
    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_00_timeline_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_00_run_sequence_4);
    wmap_start_sequence(wmap_land_effect_00_run_sequence_1);
    D_801B2ADC = 0x4;
    D_801B2AD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_00_timeline_wait_04(void)
{
    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/** @brief World-map step: register a callback, set flags, advance the step. */
void wmap_land_effect_00_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_00_run_sequence_7);
    D_801ADAE0 = 1;
    wmap_start_map_tint(0x202540);
    g_wmap_backdrop_target_level = 3;
    D_801B2ADC = 0x14;
    D_801B2AD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_00_timeline_wait_06(void)
{
    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_00_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_00_run_sequence_8);
    D_801B2ADC = 0x2;
    D_801B2AD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_00_timeline_wait_08(void)
{
    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_00_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_00_run_sequence_6);
    D_801B2ADC = 0x11;
    D_801B2AD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_00_timeline_wait_10(void)
{
    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_00_timeline_step_11(void)
{
    wmap_start_sequence(wmap_land_effect_00_run_sequence_2);
    D_801B2ADC = 0x26;
    D_801B2AD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_00_timeline_wait_12(void)
{
    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/** @brief World-map step handler: register a callback, kick a job, advance the step. */
void wmap_land_effect_00_timeline_step_13(void)
{
    wmap_start_sequence(wmap_land_effect_00_run_sequence_5);
    wmap_start_map_tint(0x404050);
    g_wmap_backdrop_target_level = 8;
    D_801B2ADC = 4;
    D_801B2AD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_00_timeline_wait_14(void)
{
    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_00_timeline_step_15(void)
{
    wmap_start_sequence(wmap_land_effect_00_run_sequence_3);
    D_801B2ADC = 0x9C;
    D_801B2AD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_00_timeline_wait_16(void)
{
    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/** @brief Update the selected world-map cell value, clear the gate flag, and advance the sequence. */
void wmap_land_effect_00_timeline_finish(void)
{
    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B2AD8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_00_run_sequence_1(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2AE0 = 1;
        D_801B2AE4 = 1;
        return 1;
    }

    if (D_801B2AE0 < 0x4)
    {
        D_800D6090[D_801B2AE0]();
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
void wmap_land_effect_00_sequence_1_reset(void)
{
    D_801B2AE0 = 1;
    D_801B2AE4 = 1;
}

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
    D_801B2AE4 = 0x30;
    D_801B2AE0 += 1;
    wmap_land_effect_00_sequence_1_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_00_sequence_1_step_02(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0x13, 0x14, 0);
    if (--D_801B2AE4 == 0)
    {
        D_801B2AE0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_00_sequence_1_end(void)
{
    D_801B2AE0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_00_run_sequence_2(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2AE8 = 1;
        D_801B2AEC = 1;
        return 1;
    }

    if (D_801B2AE8 < 0x6)
    {
        D_800D60A0[D_801B2AE8]();
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
void wmap_land_effect_00_sequence_2_reset(void)
{
    D_801B2AE8 = 1;
    D_801B2AEC = 1;
}

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
    D_801B2AEC = 0x60;
    D_801B2AE8 += 1;
    wmap_land_effect_00_sequence_2_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_00_sequence_2_step_02(void)
{
    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, g_wmap_focus_screen_position.packed, 0x13, 0x14, 0);
    if (--D_801B2AEC == 0)
    {
        D_801B2AE8 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_00_sequence_2_step_03(void)
{
    D_800D93C8.shade_step = 4;
    D_800D93C8.target_shade = 0;
    D_801B2AEC = 0x20;
    D_801B2AE8 += 1;
    wmap_land_effect_00_sequence_2_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_00_sequence_2_step_04(void)
{
    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, g_wmap_focus_screen_position.packed, 0x13, 0x14, 0);
    if (--D_801B2AEC == 0)
    {
        D_801B2AE8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_00_sequence_2_end(void)
{
    D_801B2AE8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_00_run_sequence_3(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2AF0 = 1;
        D_801B2AF4 = 1;
        return 1;
    }

    if (D_801B2AF0 < 0x4)
    {
        D_800D60B8[D_801B2AF0]();
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
void wmap_land_effect_00_sequence_3_reset(void)
{
    D_801B2AF0 = 1;
    D_801B2AF4 = 1;
}

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
    D_801B2AF4 = 0x9D;
    D_801B2AF0 += 1;
    wmap_land_effect_00_sequence_3_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_00_sequence_3_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x14, 0x1F, 0);
    if (--D_801B2AF4 == 0)
    {
        D_801B2AF0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_00_sequence_3_end(void)
{
    D_801B2AF0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_00_run_sequence_4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2AF8 = 1;
        D_801B2AFC = 1;
        return 1;
    }

    if (D_801B2AF8 < 0x4)
    {
        D_800D60C8[D_801B2AF8]();
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
void wmap_land_effect_00_sequence_4_reset(void)
{
    D_801B2AF8 = 1;
    D_801B2AFC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_00_sequence_4_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B2AFC = 0x20;
    D_801B2AF8 += 1;
    wmap_land_effect_00_sequence_4_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_00_sequence_4_end(void)
{
    D_801B2AF8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_00_run_sequence_5(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2B00 = 1;
        D_801B2B04 = 1;
        return 1;
    }

    if (D_801B2B00 < 0x4)
    {
        D_800D60D8[D_801B2B00]();
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
void wmap_land_effect_00_sequence_5_reset(void)
{
    D_801B2B00 = 1;
    D_801B2B04 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_00_sequence_5_step_01(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    D_801B2B04 = 0x40;
    D_801B2B00 += 1;
    wmap_land_effect_00_sequence_5_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_00_sequence_5_end(void)
{
    D_801B2B00 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_00_run_sequence_6(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2B08 = 1;
        D_801B2B0C = 1;
        return 1;
    }

    if (D_801B2B08 < 0x6)
    {
        D_800D60E8[D_801B2B08]();
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
void wmap_land_effect_00_sequence_6_reset(void)
{
    D_801B2B08 = 1;
    D_801B2B0C = 1;
}

/** @brief Restore effect state and begin a 66-tick sequence step. */
void wmap_land_effect_00_sequence_6_step_01(void)
{
    D_80182DF4 = 1;
    D_8013B240 = D_80139258;
    D_80139234 = 0x200;
    D_801B2B0C = 0x42;
    D_801B2B08 += 1;
    wmap_land_effect_00_sequence_6_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_00_sequence_6_step_03(void)
{
    D_801B2B0C = 0x20;
    D_801B2B08 += 1;
    wmap_land_effect_00_sequence_6_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_00_sequence_6_end(void)
{
    D_801B2B08 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_00_run_sequence_7(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2B10 = 1;
        D_801B2B14 = 1;
        return 1;
    }

    if (D_801B2B10 < 0x8)
    {
        D_800D6100[D_801B2B10]();
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
void wmap_land_effect_00_sequence_7_reset(void)
{
    D_801B2B10 = 1;
    D_801B2B14 = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void wmap_land_effect_00_sequence_7_step_02(void)
{
    s32 remaining;

    func_8006B328(0x64, 0x7C, 4, -1, -3, -4, 0, 0x13, -0xA0, 0x140, -0xA0, 0x140, 0x32, 1, 0x81, 4, 0);
    D_80182DF0 += 8;
    remaining = D_801B2B14 - 1;
    D_801B2B14 = remaining;
    if (remaining == 0)
    {
        D_801B2B10 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_00_sequence_7_step_03(void)
{
    D_801B2B14 = 0x60;
    D_801B2B10 += 1;
    wmap_land_effect_00_sequence_7_step_04();
}

/** @brief World-map effect spawn: submit a request and tick the refcount. */
void wmap_land_effect_00_sequence_7_step_04(void)
{
    s32 c;

    func_8006B328(0x64, 0x7C, 4, -1, -3, -4, 0, 0x13, -0xA0, 0x140, -0xA0,
                  0x140, 0x32, 1, 0x81, 4, 0);
    c = D_801B2B14 - 1;
    D_801B2B14 = c;
    if (c == 0)
    {
        D_801B2B10 += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_00_sequence_7_step_05(void)
{
    D_800DCEA8 = 0;
    D_801B2B14 = 0x40;
    D_801B2B10 += 1;
    wmap_land_effect_00_sequence_7_step_06();
}

/** @brief World-map effect spawn: submit a request and tick the refcount. */
void wmap_land_effect_00_sequence_7_step_06(void)
{
    s32 c;

    func_8006B328(0x64, 0x7C, 4, -1, -3, -4, 0, 0x13, -0xA0, 0x140, -0xA0,
                  0x140, 0x32, 1, 0x81, 4, 0);
    c = D_801B2B14 - 1;
    D_801B2B14 = c;
    if (c == 0)
    {
        D_801B2B10 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_00_sequence_7_end(void)
{
    D_801B2B10 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_00_run_sequence_8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2B18 = 1;
        D_801B2B1C = 1;
        return 1;
    }

    if (D_801B2B18 < 0x6)
    {
        D_800D6120[D_801B2B18]();
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
void wmap_land_effect_00_sequence_8_reset(void)
{
    D_801B2B18 = 1;
    D_801B2B1C = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_00_sequence_8_step_02(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x19, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2B1C == 0)
    {
        D_801B2B18 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_00_sequence_8_step_03(void)
{
    D_801B2B1C = 0x20;
    D_80139280[35] = -1;
    D_801B2B18 += 1;
    wmap_land_effect_00_sequence_8_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_00_sequence_8_step_04(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x19, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2B1C == 0)
    {
        D_801B2B18 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_00_sequence_8_end(void)
{
    D_801B2B18 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_00_run_sequence_9(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2B20 = 1;
        D_801B2B24 = 1;
        return 1;
    }

    if (D_801B2B20 < 0x6)
    {
        D_800D6138[D_801B2B20]();
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
void wmap_land_effect_00_sequence_9_reset(void)
{
    D_801B2B20 = 1;
    D_801B2B24 = 1;
}

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
    D_801B2B24 = 0x88;
    D_801B2B20 += 1;
    wmap_land_effect_00_sequence_9_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_00_sequence_9_step_02(void)
{
    wmap_step_actor_animation(&D_800D93F4, &D_801399D0);
    wmap_draw_actor_sprite(&D_800D93F4, g_wmap_focus_screen_position.packed, 0x8, 0x1E, 0);
    if (--D_801B2B24 == 0)
    {
        D_801B2B20 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_00_sequence_9_step_03(void)
{
    D_800D93F4.shade_step = 4;
    D_800D93F4.target_shade = 0;
    D_801B2B24 = 0x20;
    D_801B2B20 += 1;
    wmap_land_effect_00_sequence_9_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_00_sequence_9_step_04(void)
{
    wmap_step_actor_animation(&D_800D93F4, &D_801399D0);
    wmap_draw_actor_sprite(&D_800D93F4, g_wmap_focus_screen_position.packed, 0x8, 0x1E, 0);
    if (--D_801B2B24 == 0)
    {
        D_801B2B20 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_00_sequence_9_end(void)
{
    D_801B2B20 += 1;
}
