#include "wmap_main.h"
#include "wmap_land_effect_03.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

void wmap_land_effect_03_sequence_5_step_02(void);
void wmap_land_effect_03_sequence_6_step_02(void);
void wmap_land_effect_03_sequence_7_step_02(void);
void wmap_land_effect_03_sequence_8_step_02(void);
void wmap_land_effect_03_wait_idle_02(void);
void wmap_land_effect_03_step_03(void);
s32 wmap_land_effect_03_run_timeline(s32 arg0);
void wmap_land_effect_03_wait_idle_04(void);
void wmap_land_effect_03_end(void);
s32 wmap_land_effect_03_run_sequence_8(s32 arg0);
s32 wmap_land_effect_03_run_sequence_7(s32 arg0);
s32 wmap_land_effect_03_run_sequence_1(s32 arg0);
s32 wmap_land_effect_03_run_sequence_6(s32 arg0);
s32 wmap_land_effect_03_run_sequence_5(s32 arg0);
s32 wmap_land_effect_03_run_sequence_3(s32 arg0);
s32 wmap_land_effect_03_run_sequence_4(s32 arg0);
s32 wmap_land_effect_03_run_sequence_2(s32 arg0);
void wmap_land_effect_03_sequence_1_step_02(void);
void wmap_land_effect_03_sequence_2_step_02(void);
void wmap_land_effect_03_sequence_5_step_04(void);
void wmap_land_effect_03_sequence_6_step_04(void);
void wmap_land_effect_03_sequence_7_step_04(void);
void wmap_land_effect_03_sequence_8_step_04(void);

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

extern u8* D_8011CF1C;
extern s32 D_80182DF0;
extern s32 g_wmap_land_effect_03_sequence_3_timer;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 g_wmap_land_effect_03_sequence_4_timer;
extern s32 D_801B0FD0;
extern s32 g_wmap_land_effect_03_sequence_5_timer;
extern s32 g_wmap_land_effect_03_sequence_6_timer;
extern s32 g_wmap_land_effect_03_sequence_7_timer;
extern s32 g_wmap_land_effect_03_sequence_8_timer;
extern s32 g_wmap_land_effect_03_timer;
extern void (*D_800D5A60[])(void);
extern void wmap_land_effect_03_step_03(void);
extern void wmap_land_effect_03_end(void);
extern s32 g_wmap_land_effect_03_timeline_timer;
extern void (*D_800D5A78[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 g_wmap_land_effect_03_sequence_1_timer;
extern void (*D_800D5AB8[])(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern void wmap_land_effect_03_sequence_1_step_02(void);
extern s32 g_wmap_land_effect_03_sequence_2_timer;
extern void (*D_800D5AC8[])(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern void wmap_land_effect_03_sequence_2_step_02(void);
extern void (*D_800D5AD8[])(void);
extern void (*D_800D5AF0[])(void);
extern void (*D_800D5B00[])(void);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern void (*D_800D5B18[])(void);
extern u8 D_800D9F20[];
extern u8 D_80139BD8[];
extern void wmap_land_effect_03_sequence_6_step_04(void);
extern void (*D_800D5B30[])(void);
extern u8 D_800DA7B8[];
extern u8 D_80139D68[];
extern void wmap_land_effect_03_sequence_7_step_04(void);
extern void (*D_800D5B48[])(void);
extern u8 D_800DB050[];
extern u8 D_80139EF8[];
extern void wmap_land_effect_03_sequence_8_step_04(void);

extern u32 g_wmap_land_effect_03_sequence_3_step;
extern u32 g_wmap_land_effect_03_sequence_4_step;
extern u32 g_wmap_land_effect_03_sequence_5_step;
extern u8 D_80121538[];
extern u32 g_wmap_land_effect_03_sequence_6_step;
extern u32 g_wmap_land_effect_03_sequence_7_step;
extern u32 g_wmap_land_effect_03_sequence_8_step;
extern u32 g_wmap_land_effect_03_step;
extern u32 g_wmap_land_effect_03_timeline_step;
extern u32 g_wmap_land_effect_03_sequence_1_step;
extern u32 g_wmap_land_effect_03_sequence_2_step;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;

extern SVECTOR D_80139258;
extern SVECTOR D_801B24A0;
extern SVECTOR D_801B2490;

extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* D_80139280;

extern WmapSlot14 D_801AFBD0[];

/** @brief Draw the rotating effect, increase its scale, and update the sequence timer. */
void wmap_land_effect_03_sequence_3_step_02(void)
{
    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(D_8011CF1C, 0, 4, 0x35, 0x7800, 1, D_80182DF0);
    D_801B2490.vz += 100;
    PopMatrix();
    D_80182DF0 += 2;
    if (D_80182DF0 >= 0x82)
    {
        D_80182DF0 = 0x81;
    }
    if (--g_wmap_land_effect_03_sequence_3_timer == 0)
    {
        g_wmap_land_effect_03_sequence_3_step++;
    }
}

/** @brief Draw the rotating effect, reduce its scale, and update the sequence timer. */
void wmap_land_effect_03_sequence_3_step_04(void)
{
    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(D_8011CF1C, 0, 4, 0x35, 0x7800, 1, D_80182DF0);
    D_801B2490.vz += 100;
    PopMatrix();
    D_80182DF0 -= 8;
    if (D_80182DF0 < 0)
    {
        D_80182DF0 = 0;
    }
    if (--g_wmap_land_effect_03_sequence_3_timer == 0)
    {
        g_wmap_land_effect_03_sequence_3_step++;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_03_sequence_4_step_02(void)
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
        D_80182DE8 -= 0x2;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_03_sequence_4_timer == 0)
    {
        g_wmap_land_effect_03_sequence_4_step += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_03_sequence_5_step_01(void)
{
    s32 i;

    D_801B0FD0 = 50;
    D_80139280[0x1] = 4;
    D_80139280[0x2] = 1;
    D_80139280[0x3] = 20;
    D_80139280[0x4] = 30;
    D_80139280[0x5] = 16;
    D_80139280[0x6] = 1500;
    D_80139280[0x7] = 20;
    D_80139280[0x8] = 19;
    D_80139280[0x9] = 0;
    D_80139280[0xA] = 10000;
    for (i = 0; i < 50; i++)
    {
        D_801AFBD0[i + D_80139280[0x7]].field_00 = 0;
        D_80139988[i + 24].data = D_80121538;
    }
    g_wmap_land_effect_03_sequence_5_timer = 144;
    g_wmap_land_effect_03_sequence_5_step++;
    wmap_land_effect_03_sequence_5_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_03_sequence_6_step_01(void)
{
    s32 i;

    D_801B0FD0 = 50;
    D_80139280[0xB] = 4;
    D_80139280[0xC] = 1;
    D_80139280[0xD] = 20;
    D_80139280[0xE] = 30;
    D_80139280[0xF] = 4;
    D_80139280[0x10] = 1500;
    D_80139280[0x11] = 70;
    D_80139280[0x12] = 19;
    D_80139280[0x13] = 1;
    D_80139280[0x14] = 10000;
    for (i = 0; i < 50; i++)
    {
        D_801AFBD0[i + D_80139280[0x11]].field_00 = 0;
        D_80139988[i + 74].data = D_80121538;
    }
    g_wmap_land_effect_03_sequence_6_timer = 144;
    g_wmap_land_effect_03_sequence_6_step++;
    wmap_land_effect_03_sequence_6_step_02();
}

/** @brief World-map step: fill spawn descriptor slot 1, clear its slot run, then advance. */
void wmap_land_effect_03_sequence_7_step_01(void)
{
    s32 i;

    D_801B0FD0 = 0x32;
    D_80139280[21] = 2;
    D_80139280[22] = 1;
    D_80139280[23] = 0x14;
    D_80139280[24] = 0x1E;
    D_80139280[25] = 2;
    D_80139280[26] = 0x5DC;
    D_80139280[27] = 0x78;
    D_80139280[28] = 0x13;
    D_80139280[29] = 2;
    D_80139280[30] = 0x2710;
    for (i = 0; i < 0x32; i++)
    {
        D_801AFBD0[i + D_80139280[27]].field_00 = 0;
        D_80139988[i + 0x7C].data = &D_80121538;
    }
    g_wmap_land_effect_03_sequence_7_timer = 0x90;
    g_wmap_land_effect_03_sequence_7_step += 1;
    wmap_land_effect_03_sequence_7_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_03_sequence_8_step_01(void)
{
    s32 i;

    D_801B0FD0 = 50;
    D_80139280[0x1F] = 1;
    D_80139280[0x20] = 1;
    D_80139280[0x21] = 20;
    D_80139280[0x22] = 30;
    D_80139280[0x23] = 2;
    D_80139280[0x24] = 1500;
    D_80139280[0x25] = 170;
    D_80139280[0x26] = 19;
    D_80139280[0x27] = 3;
    D_80139280[0x28] = 10000;
    for (i = 0; i < 50; i++)
    {
        D_801AFBD0[i + D_80139280[0x25]].field_00 = 0;
        D_80139988[i + 174].data = D_80121538;
    }
    g_wmap_land_effect_03_sequence_8_timer = 144;
    g_wmap_land_effect_03_sequence_8_step++;
    wmap_land_effect_03_sequence_8_step_02();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_03_run(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_03_step = 1;
        g_wmap_land_effect_03_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_03_step < 0x6)
    {
        D_800D5A60[g_wmap_land_effect_03_step]();
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
void wmap_land_effect_03_reset(void)
{
    g_wmap_land_effect_03_step = 1;
    g_wmap_land_effect_03_timer = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_03_step_01(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_03_step += 1;
    wmap_land_effect_03_wait_idle_02();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_03_wait_idle_02(void)
{
    if (g_wmap_sequence_busy == 0)
    {
        g_wmap_land_effect_03_step += 1;
        wmap_land_effect_03_step_03();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_03_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_03_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_03_step += 1;
    wmap_land_effect_03_wait_idle_04();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_03_wait_idle_04(void)
{
    if (g_wmap_sequence_busy == 0)
    {
        g_wmap_land_effect_03_step += 1;
        wmap_land_effect_03_end();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_03_end(void)
{
    g_wmap_land_effect_03_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_03_run_timeline(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_03_timeline_step = 1;
        g_wmap_land_effect_03_timeline_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_03_timeline_step < 0x10)
    {
        D_800D5A78[g_wmap_land_effect_03_timeline_step]();
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
void wmap_land_effect_03_timeline_reset(void)
{
    g_wmap_land_effect_03_timeline_step = 1;
    g_wmap_land_effect_03_timeline_timer = 1;
}

/** @brief World-map step handler: kick two jobs and advance the step. */
void wmap_land_effect_03_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_start_map_tint(0x802028);
    g_wmap_backdrop_target_level = 4;
    wmap_play_sound(0x21, 0x80);
    g_wmap_land_effect_03_timeline_timer = 0x1E;
    g_wmap_land_effect_03_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_03_timeline_wait_02(void)
{
    if (--g_wmap_land_effect_03_timeline_timer == 0)
    {
        g_wmap_land_effect_03_timeline_step += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_03_timeline_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_03_run_sequence_8);
    wmap_start_sequence(wmap_land_effect_03_run_sequence_7);
    wmap_start_sequence(wmap_land_effect_03_run_sequence_1);
    g_wmap_land_effect_03_timeline_timer = 0x18;
    g_wmap_land_effect_03_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_03_timeline_wait_04(void)
{
    if (--g_wmap_land_effect_03_timeline_timer == 0)
    {
        g_wmap_land_effect_03_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_03_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_03_run_sequence_6);
    g_wmap_land_effect_03_timeline_timer = 0x10;
    g_wmap_land_effect_03_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_03_timeline_wait_06(void)
{
    if (--g_wmap_land_effect_03_timeline_timer == 0)
    {
        g_wmap_land_effect_03_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_03_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_03_run_sequence_5);
    g_wmap_land_effect_03_timeline_timer = 0x8;
    g_wmap_land_effect_03_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_03_timeline_wait_08(void)
{
    if (--g_wmap_land_effect_03_timeline_timer == 0)
    {
        g_wmap_land_effect_03_timeline_step += 1;
    }
}

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void wmap_land_effect_03_timeline_step_09(void)
{
    D_801ADAE0 = 1;
    wmap_start_sequence(wmap_land_effect_03_run_sequence_3);
    g_wmap_land_effect_03_timeline_timer = 0x3C;
    g_wmap_land_effect_03_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_03_timeline_wait_10(void)
{
    if (--g_wmap_land_effect_03_timeline_timer == 0)
    {
        g_wmap_land_effect_03_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_03_timeline_step_11(void)
{
    wmap_start_sequence(wmap_land_effect_03_run_sequence_4);
    g_wmap_land_effect_03_timeline_timer = 0x8;
    g_wmap_land_effect_03_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_03_timeline_wait_12(void)
{
    if (--g_wmap_land_effect_03_timeline_timer == 0)
    {
        g_wmap_land_effect_03_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_03_timeline_step_13(void)
{
    wmap_start_sequence(wmap_land_effect_03_run_sequence_2);
    g_wmap_land_effect_03_timeline_timer = 0x8A;
    g_wmap_land_effect_03_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_03_timeline_wait_14(void)
{
    if (--g_wmap_land_effect_03_timeline_timer == 0)
    {
        g_wmap_land_effect_03_timeline_step += 1;
    }
}

void wmap_land_effect_03_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    g_wmap_land_effect_03_timeline_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_03_run_sequence_1(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_03_sequence_1_step = 1;
        g_wmap_land_effect_03_sequence_1_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_03_sequence_1_step < 0x4)
    {
        D_800D5AB8[g_wmap_land_effect_03_sequence_1_step]();
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
void wmap_land_effect_03_sequence_1_reset(void)
{
    g_wmap_land_effect_03_sequence_1_step = 1;
    g_wmap_land_effect_03_sequence_1_timer = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_03_sequence_1_step_01(void)
{
    D_801399AC = D_8011D538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 2;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.target_shade = 0x80;
    D_800D9318.shade = 0;
    g_wmap_land_effect_03_sequence_1_timer = 0x7C;
    g_wmap_land_effect_03_sequence_1_step += 1;
    wmap_land_effect_03_sequence_1_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_1_step_02(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0x13, 0xA, 0);
    if (--g_wmap_land_effect_03_sequence_1_timer == 0)
    {
        g_wmap_land_effect_03_sequence_1_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_03_sequence_1_end(void)
{
    g_wmap_land_effect_03_sequence_1_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_03_run_sequence_2(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_03_sequence_2_step = 1;
        g_wmap_land_effect_03_sequence_2_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_03_sequence_2_step < 0x4)
    {
        D_800D5AC8[g_wmap_land_effect_03_sequence_2_step]();
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
void wmap_land_effect_03_sequence_2_reset(void)
{
    g_wmap_land_effect_03_sequence_2_step = 1;
    g_wmap_land_effect_03_sequence_2_timer = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_03_sequence_2_step_01(void)
{
    D_801399B4 = D_8011F538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 8;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0;
    g_wmap_land_effect_03_sequence_2_timer = 0x8C;
    g_wmap_land_effect_03_sequence_2_step += 1;
    wmap_land_effect_03_sequence_2_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_2_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x1A, 0xA, 0);
    if (--g_wmap_land_effect_03_sequence_2_timer == 0)
    {
        g_wmap_land_effect_03_sequence_2_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_03_sequence_2_end(void)
{
    g_wmap_land_effect_03_sequence_2_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_03_run_sequence_3(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_03_sequence_3_step = 1;
        g_wmap_land_effect_03_sequence_3_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_03_sequence_3_step < 0x6)
    {
        D_800D5AD8[g_wmap_land_effect_03_sequence_3_step]();
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
void wmap_land_effect_03_sequence_3_reset(void)
{
    g_wmap_land_effect_03_sequence_3_step = 1;
    g_wmap_land_effect_03_sequence_3_timer = 1;
}

/** @brief Clear the rotation vector, set the flag, and start a 128-tick sequence step. */
void wmap_land_effect_03_sequence_3_step_01(void)
{
    D_80182DF0 = 1;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    g_wmap_land_effect_03_sequence_3_timer = 0x80;
    g_wmap_land_effect_03_sequence_3_step += 1;
    wmap_land_effect_03_sequence_3_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_03_sequence_3_step_03(void)
{
    g_wmap_land_effect_03_sequence_3_timer = 0x10;
    g_wmap_land_effect_03_sequence_3_step += 1;
    wmap_land_effect_03_sequence_3_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_03_sequence_3_end(void)
{
    g_wmap_land_effect_03_sequence_3_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_03_run_sequence_4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_03_sequence_4_step = 1;
        g_wmap_land_effect_03_sequence_4_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_03_sequence_4_step < 0x4)
    {
        D_800D5AF0[g_wmap_land_effect_03_sequence_4_step]();
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
void wmap_land_effect_03_sequence_4_reset(void)
{
    g_wmap_land_effect_03_sequence_4_step = 1;
    g_wmap_land_effect_03_sequence_4_timer = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_03_sequence_4_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    g_wmap_land_effect_03_sequence_4_timer = 0x40;
    g_wmap_land_effect_03_sequence_4_step += 1;
    wmap_land_effect_03_sequence_4_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_03_sequence_4_end(void)
{
    g_wmap_land_effect_03_sequence_4_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_03_run_sequence_5(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_03_sequence_5_step = 1;
        g_wmap_land_effect_03_sequence_5_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_03_sequence_5_step < 0x6)
    {
        D_800D5B00[g_wmap_land_effect_03_sequence_5_step]();
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
void wmap_land_effect_03_sequence_5_reset(void)
{
    g_wmap_land_effect_03_sequence_5_step = 1;
    g_wmap_land_effect_03_sequence_5_timer = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_5_step_02(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x32, 0, 0x7F, 0x2, 0, (s32)D_80139280);
    if (--g_wmap_land_effect_03_sequence_5_timer == 0)
    {
        g_wmap_land_effect_03_sequence_5_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_03_sequence_5_step_03(void)
{
    g_wmap_land_effect_03_sequence_5_timer = 0x20;
    D_80139280[5] = -1;
    g_wmap_land_effect_03_sequence_5_step += 1;
    wmap_land_effect_03_sequence_5_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_5_step_04(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x32, 0, 0x7F, 0x2, 0, (s32)D_80139280);
    if (--g_wmap_land_effect_03_sequence_5_timer == 0)
    {
        g_wmap_land_effect_03_sequence_5_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_03_sequence_5_end(void)
{
    g_wmap_land_effect_03_sequence_5_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_03_run_sequence_6(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_03_sequence_6_step = 1;
        g_wmap_land_effect_03_sequence_6_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_03_sequence_6_step < 0x6)
    {
        D_800D5B18[g_wmap_land_effect_03_sequence_6_step]();
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
void wmap_land_effect_03_sequence_6_reset(void)
{
    g_wmap_land_effect_03_sequence_6_step = 1;
    g_wmap_land_effect_03_sequence_6_timer = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_6_step_02(void)
{
    func_8006A2FC(D_800D9F20, D_80139BD8, 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--g_wmap_land_effect_03_sequence_6_timer == 0)
    {
        g_wmap_land_effect_03_sequence_6_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_03_sequence_6_step_03(void)
{
    g_wmap_land_effect_03_sequence_6_timer = 0x20;
    D_80139280[15] = -1;
    g_wmap_land_effect_03_sequence_6_step += 1;
    wmap_land_effect_03_sequence_6_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_6_step_04(void)
{
    func_8006A2FC(D_800D9F20, D_80139BD8, 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--g_wmap_land_effect_03_sequence_6_timer == 0)
    {
        g_wmap_land_effect_03_sequence_6_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_03_sequence_6_end(void)
{
    g_wmap_land_effect_03_sequence_6_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_03_run_sequence_7(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_03_sequence_7_step = 1;
        g_wmap_land_effect_03_sequence_7_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_03_sequence_7_step < 0x6)
    {
        D_800D5B30[g_wmap_land_effect_03_sequence_7_step]();
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
void wmap_land_effect_03_sequence_7_reset(void)
{
    g_wmap_land_effect_03_sequence_7_step = 1;
    g_wmap_land_effect_03_sequence_7_timer = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_7_step_02(void)
{
    func_8006A2FC(D_800DA7B8, D_80139D68, 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--g_wmap_land_effect_03_sequence_7_timer == 0)
    {
        g_wmap_land_effect_03_sequence_7_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_03_sequence_7_step_03(void)
{
    g_wmap_land_effect_03_sequence_7_timer = 0x20;
    D_80139280[25] = -1;
    g_wmap_land_effect_03_sequence_7_step += 1;
    wmap_land_effect_03_sequence_7_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_7_step_04(void)
{
    func_8006A2FC(D_800DA7B8, D_80139D68, 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--g_wmap_land_effect_03_sequence_7_timer == 0)
    {
        g_wmap_land_effect_03_sequence_7_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_03_sequence_7_end(void)
{
    g_wmap_land_effect_03_sequence_7_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_03_run_sequence_8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_03_sequence_8_step = 1;
        g_wmap_land_effect_03_sequence_8_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_03_sequence_8_step < 0x6)
    {
        D_800D5B48[g_wmap_land_effect_03_sequence_8_step]();
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
void wmap_land_effect_03_sequence_8_reset(void)
{
    g_wmap_land_effect_03_sequence_8_step = 1;
    g_wmap_land_effect_03_sequence_8_timer = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_8_step_02(void)
{
    func_8006A2FC(D_800DB050, D_80139EF8, 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--g_wmap_land_effect_03_sequence_8_timer == 0)
    {
        g_wmap_land_effect_03_sequence_8_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_03_sequence_8_step_03(void)
{
    g_wmap_land_effect_03_sequence_8_timer = 0x20;
    D_80139280[35] = -1;
    g_wmap_land_effect_03_sequence_8_step += 1;
    wmap_land_effect_03_sequence_8_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_8_step_04(void)
{
    func_8006A2FC(D_800DB050, D_80139EF8, 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--g_wmap_land_effect_03_sequence_8_timer == 0)
    {
        g_wmap_land_effect_03_sequence_8_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_03_sequence_8_end(void)
{
    g_wmap_land_effect_03_sequence_8_step += 1;
}
