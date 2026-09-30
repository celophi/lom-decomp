#include "wmap_main.h"
#include "wmap_land_effect_12.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

void wmap_land_effect_12_sequence_5_step_02(void);
void wmap_land_effect_12_sequence_6_step_02(void);
void wmap_land_effect_12_sequence_7_step_02(void);
void wmap_land_effect_12_wait_idle_02(void);
void wmap_land_effect_12_step_03(void);
s32 wmap_land_effect_12_run_timeline(s32 arg0);
void wmap_land_effect_12_wait_idle_04(void);
void wmap_land_effect_12_end(void);
s32 wmap_land_effect_12_run_sequence_1(s32 arg0);
s32 wmap_land_effect_12_run_sequence_4(s32 arg0);
s32 wmap_land_effect_12_run_sequence_5(s32 arg0);
s32 wmap_land_effect_12_run_sequence_3(s32 arg0);
s32 wmap_land_effect_12_run_sequence_6(s32 arg0);
s32 wmap_land_effect_12_run_sequence_2(s32 arg0);
s32 wmap_land_effect_12_run_sequence_7(s32 arg0);
void wmap_land_effect_12_sequence_1_step_02(void);
void wmap_land_effect_12_sequence_2_step_02(void);
void wmap_land_effect_12_sequence_5_step_04(void);
void wmap_land_effect_12_sequence_6_step_04(void);
void wmap_land_effect_12_sequence_7_step_04(void);

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

/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

extern u8* D_8011CF1C;
extern s32 D_801B2468;
extern s32 D_801B288C;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2894;
extern s32 D_801B0FD0;
extern u8 D_80121538[];
extern s32 D_801B289C;
extern s32 D_801B28A4;
extern s32 D_801B28AC;
extern s32 D_801B286C;
extern void (*D_800D5888[])(void);
extern s32 D_8013B20C;
extern void wmap_land_effect_12_step_03(void);
extern void wmap_land_effect_12_end(void);
extern s32 D_801B2874;
extern void (*D_800D58A0[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B287C;
extern void (*D_800D58D0[])(void);
extern u8 D_8011F538[];
extern u8* D_801399AC;
extern void wmap_land_effect_12_sequence_1_step_02(void);
extern s32 D_801B2884;
extern void (*D_800D58E0[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern void wmap_land_effect_12_sequence_2_step_02(void);
extern void (*D_800D58F0[])(void);
extern s32 D_801B24B0;
extern void (*D_800D5908[])(void);
extern void (*D_800D5918[])(void);
extern u8 D_800DA398[];
extern u8 D_80139CA8[];
extern void (*D_800D5930[])(void);
extern u8 D_800D95D8[];
extern WmapAnimationSlot D_80139A28[];
extern void wmap_land_effect_12_sequence_6_step_04(void);
extern void (*D_800D5948[])(void);
extern u32 D_801B2888;
extern u32 D_801B2890;
extern u32 D_801B2898;
extern u32 D_801B28A0;
extern u32 D_801B28A8;
extern u32 D_801B2868;
extern u32 D_801B2870;
extern u32 D_801B2878;
extern u32 D_801B2880;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;

extern SVECTOR D_80139258;
extern SVECTOR D_801B2498;
extern SVECTOR D_801B24A0;
extern SVECTOR D_801B2490;

extern WmapSpriteActor D_800D9268[];
extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* D_80139280;

extern WmapMotion D_801AFBD0[];

/**
 * @brief World-map step handler: draw two overlaid actor sprites within a matrix push,
 *        ramp the shared size up to a cap, then countdown-advance the step.
 */
void wmap_land_effect_12_sequence_3_step_02(void)
{
    s32 value;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(D_8011CF1C, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    D_801B2490.vz += 0xC;
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(D_8011CF1C, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    D_801B2498.vz += 0x4;
    PopMatrix();
    value = D_801B2468 + 2;
    D_801B2468 = value;
    if (value >= 0x82)
    {
        D_801B2468 = 0x81;
    }
    if (--D_801B288C == 0)
    {
        D_801B2888 += 1;
    }
}

/**
 * @brief World-map step handler: draw two frames of the animated actor, scroll each
 *        sub-field, decay the shared frame index with a floor, then advance the step.
 */
void wmap_land_effect_12_sequence_3_step_04(void)
{
    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(D_8011CF1C, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    D_801B2490.vz += 0xC;
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(D_8011CF1C, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    D_801B2498.vz += 4;
    PopMatrix();
    D_801B2468 -= 1;
    if (D_801B2468 < 0)
    {
        D_801B2468 = 0;
    }
    if (--D_801B288C == 0)
    {
        D_801B2888 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_12_sequence_4_step_02(void)
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
    if (--D_801B2894 == 0)
    {
        D_801B2890 += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_12_sequence_5_step_01(void)
{
    s32 i;

    D_801B0FD0 = 100;
    D_80139280[0x1] = 2;
    D_80139280[0x2] = 4;
    D_80139280[0x3] = 0x20;
    D_80139280[0x4] = 8;
    D_80139280[0x5] = 4;
    D_80139280[0x6] = 0x3E8;
    D_80139280[0x7] = 0x64;
    D_80139280[0x8] = 8;
    D_80139280[0x9] = 2;
    D_80139280[0xA] = 0xFA0;
    for (i = 0; i < 100; i++)
    {
        D_801AFBD0[i + 100].field_00 = 0;
        D_80139988[i + 100].data = D_80121538;
    }
    D_801B289C = 144;
    D_801B2898++;
    wmap_land_effect_12_sequence_5_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_12_sequence_6_step_01(void)
{
    s32 i;

    D_801B0FD0 = 5;
    D_80139280[0xB] = 4;
    D_80139280[0xC] = 2;
    D_80139280[0xD] = 32;
    D_80139280[0xE] = 1;
    D_80139280[0xF] = 2;
    D_80139280[0x10] = 2000;
    D_80139280[0x11] = 20;
    D_80139280[0x12] = 8;
    D_80139280[0x13] = 1;
    D_80139280[0x14] = 1000;
    for (i = 0; i < 5; i++)
    {
        D_801AFBD0[i + 20].field_00 = 0;
        D_80139988[i + 20].data = D_80121538;
    }
    D_801B28A4 = 10;
    D_801B28A0++;
    wmap_land_effect_12_sequence_6_step_02();
}

/** @brief Initialize the actor group and its animation resources, then advance. */
void wmap_land_effect_12_sequence_7_step_01(void)
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
        config->field_22 = 127;
        config->field_24 = 1;
        config->field_26 = 8;
        D_801AFBD0[i].field_00 = 1;
        D_801AFBD0[i].angle = i * 0x333;
        D_801AFBD0[i].field_08 = 380000;
        D_801AFBD0[i].field_04 = 0;
        D_801AFBD0[i].field_0C = 9999;
        D_801AFBD0[i].field_0E = 0;
        D_801AFBD0[i].field_10 = 80;
    }
    D_801B28AC = 64;
    D_801B28A8++;
    wmap_land_effect_12_sequence_7_step_02();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_12_run(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2868 = 1;
        D_801B286C = 1;
        return 1;
    }

    if (D_801B2868 < 0x6)
    {
        D_800D5888[D_801B2868]();
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
void wmap_land_effect_12_reset(void)
{
    D_801B2868 = 1;
    D_801B286C = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_12_step_01(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    D_8013B20C = 1;
    D_801B2868 += 1;
    wmap_land_effect_12_wait_idle_02();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_12_wait_idle_02(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2868 += 1;
        wmap_land_effect_12_step_03();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_12_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_12_run_timeline);
    D_8013B20C = 1;
    D_801B2868 += 1;
    wmap_land_effect_12_wait_idle_04();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_12_wait_idle_04(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2868 += 1;
        wmap_land_effect_12_end();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_12_end(void)
{
    D_801B2868 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_12_run_timeline(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2870 = 1;
        D_801B2874 = 1;
        return 1;
    }

    if (D_801B2870 < 0xC)
    {
        D_800D58A0[D_801B2870]();
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
void wmap_land_effect_12_timeline_reset(void)
{
    D_801B2870 = 1;
    D_801B2874 = 1;
}

/** @brief World-map state entry: load resources, register callbacks, advance. */
void wmap_land_effect_12_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_start_map_tint(0x304010);
    g_wmap_backdrop_target_level = 4;
    wmap_play_sound(0x1F, 0x80);
    wmap_start_sequence(wmap_land_effect_12_run_sequence_1);
    wmap_start_sequence(wmap_land_effect_12_run_sequence_4);
    D_801B2874 = 8;
    D_801B2870 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_12_timeline_wait_02(void)
{
    if (--D_801B2874 == 0)
    {
        D_801B2870 += 1;
    }
}

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void wmap_land_effect_12_timeline_step_03(void)
{
    D_801ADAE0 = 1;
    wmap_start_sequence(wmap_land_effect_12_run_sequence_5);
    D_801B2874 = 0x18;
    D_801B2870 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_12_timeline_wait_04(void)
{
    if (--D_801B2874 == 0)
    {
        D_801B2870 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_12_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_12_run_sequence_3);
    D_801B2874 = 0x3C;
    D_801B2870 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_12_timeline_wait_06(void)
{
    if (--D_801B2874 == 0)
    {
        D_801B2870 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_12_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_12_run_sequence_6);
    D_801B2874 = 0x1E;
    D_801B2870 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_12_timeline_wait_08(void)
{
    if (--D_801B2874 == 0)
    {
        D_801B2870 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_12_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_12_run_sequence_2);
    wmap_start_sequence(wmap_land_effect_12_run_sequence_7);
    D_801B2874 = 0x78;
    D_801B2870 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_12_timeline_wait_10(void)
{
    if (--D_801B2874 == 0)
    {
        D_801B2870 += 1;
    }
}

void wmap_land_effect_12_timeline_finish(void)
{
    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B2870 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_12_run_sequence_1(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2878 = 1;
        D_801B287C = 1;
        return 1;
    }

    if (D_801B2878 < 0x4)
    {
        D_800D58D0[D_801B2878]();
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
void wmap_land_effect_12_sequence_1_reset(void)
{
    D_801B2878 = 1;
    D_801B287C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_12_sequence_1_step_01(void)
{
    D_801399AC = D_8011F538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 8;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.target_shade = 0x80;
    D_800D9318.shade = 0;
    D_801B287C = 0x7E;
    D_801B2878 += 1;
    wmap_land_effect_12_sequence_1_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_12_sequence_1_step_02(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0x19, 0xE, 0);
    if (--D_801B287C == 0)
    {
        D_801B2878 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_12_sequence_1_end(void)
{
    D_801B2878 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_12_run_sequence_2(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2880 = 1;
        D_801B2884 = 1;
        return 1;
    }

    if (D_801B2880 < 0x4)
    {
        D_800D58E0[D_801B2880]();
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
void wmap_land_effect_12_sequence_2_reset(void)
{
    D_801B2880 = 1;
    D_801B2884 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_12_sequence_2_step_01(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 8;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0x80;
    D_801B2884 = 0x79;
    D_801B2880 += 1;
    wmap_land_effect_12_sequence_2_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_12_sequence_2_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x1A, 0xF, 0);
    if (--D_801B2884 == 0)
    {
        D_801B2880 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_12_sequence_2_end(void)
{
    D_801B2880 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_12_run_sequence_3(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2888 = 1;
        D_801B288C = 1;
        return 1;
    }

    if (D_801B2888 < 0x6)
    {
        D_800D58F0[D_801B2888]();
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
void wmap_land_effect_12_sequence_3_reset(void)
{
    D_801B2888 = 1;
    D_801B288C = 1;
}

/** @brief World-map step: reset counters and advance to the next handler. */
void wmap_land_effect_12_sequence_3_step_01(void)
{
    D_801B24B0 = 0;
    D_801B2468 = 1;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    D_801B2498.vx = 0;
    D_801B2498.vy = 0;
    D_801B2498.vz = 0;
    D_801B288C = 0x40;
    D_801B2888 += 1;
    wmap_land_effect_12_sequence_3_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_12_sequence_3_step_03(void)
{
    D_801B288C = 0x80;
    D_801B2888 += 1;
    wmap_land_effect_12_sequence_3_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_12_sequence_3_end(void)
{
    D_801B2888 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_12_run_sequence_4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2890 = 1;
        D_801B2894 = 1;
        return 1;
    }

    if (D_801B2890 < 0x4)
    {
        D_800D5908[D_801B2890]();
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
void wmap_land_effect_12_sequence_4_reset(void)
{
    D_801B2890 = 1;
    D_801B2894 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_12_sequence_4_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B2894 = 0x20;
    D_801B2890 += 1;
    wmap_land_effect_12_sequence_4_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_12_sequence_4_end(void)
{
    D_801B2890 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_12_run_sequence_5(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2898 = 1;
        D_801B289C = 1;
        return 1;
    }

    if (D_801B2898 < 0x6)
    {
        D_800D5918[D_801B2898]();
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
void wmap_land_effect_12_sequence_5_reset(void)
{
    D_801B2898 = 1;
    D_801B289C = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_12_sequence_5_step_02(void)
{
    func_8006A2FC(D_800DA398, D_80139CA8, 0x64, 0, 0x7F, 0x4, 0, (s32)D_80139280);
    if (--D_801B289C == 0)
    {
        D_801B2898 += 1;
    }
}

void wmap_land_effect_12_sequence_5_step_03(void)
{
    D_801B289C = 0x20;
    D_80139280[5] = -1;
    D_801B2898 += 1;
    wmap_land_effect_12_sequence_5_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_12_sequence_5_step_04(void)
{
    func_8006A2FC(D_800DA398, D_80139CA8, 0x64, 0, 0x7F, 0x4, 0, (s32)D_80139280);
    if (--D_801B289C == 0)
    {
        D_801B2898 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_12_sequence_5_end(void)
{
    D_801B2898 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_12_run_sequence_6(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B28A0 = 1;
        D_801B28A4 = 1;
        return 1;
    }

    if (D_801B28A0 < 0x6)
    {
        D_800D5930[D_801B28A0]();
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
void wmap_land_effect_12_sequence_6_reset(void)
{
    D_801B28A0 = 1;
    D_801B28A4 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_12_sequence_6_step_02(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0x5, 0, 0x7F, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B28A4 == 0)
    {
        D_801B28A0 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_12_sequence_6_step_03(void)
{
    D_801B28A4 = 0x20;
    D_80139280[15] = -1;
    D_801B28A0 += 1;
    wmap_land_effect_12_sequence_6_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_12_sequence_6_step_04(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0x5, 0, 0x7F, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B28A4 == 0)
    {
        D_801B28A0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_12_sequence_6_end(void)
{
    D_801B28A0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_12_run_sequence_7(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B28A8 = 1;
        D_801B28AC = 1;
        return 1;
    }

    if (D_801B28A8 < 0x6)
    {
        D_800D5948[D_801B28A8]();
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
void wmap_land_effect_12_sequence_7_reset(void)
{
    D_801B28A8 = 1;
    D_801B28AC = 1;
}

/**
 * @brief Draw the world-map cursor via func_8006B6EC, then advance after the wait expires.
 */
void wmap_land_effect_12_sequence_7_step_02(void)
{
    func_8006B6EC(0xC8, 0xCD, 0x8, 0, 0xE);
    if (--D_801B28AC == 0)
    {
        D_801B28A8 += 1;
    }
}

/**
 * @brief Reset a range of world-map actor configs, arm the timer, and advance the step.
 */
void wmap_land_effect_12_sequence_7_step_03(void)
{
    s32 i;

    for (i = 0xC8; i < 0xCD; i++)
    {
        D_800D9268[i].target_shade = 0;
        D_800D9268[i].shade_step = 8;
    }
    D_801B28AC = 0x10;
    D_801B28A8 += 1;
    wmap_land_effect_12_sequence_7_step_04();
}

/**
 * @brief Draw the world-map cursor via func_8006B6EC, then advance after the wait expires.
 */
void wmap_land_effect_12_sequence_7_step_04(void)
{
    func_8006B6EC(0xC8, 0xCD, 0x8, 0, 0xE);
    if (--D_801B28AC == 0)
    {
        D_801B28A8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_12_sequence_7_end(void)
{
    D_801B28A8 += 1;
}
