#include "wmap_main.h"
#include "wmap_land_effect_16.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

/** @brief World-map tile and its cached neighboring layout data. */
typedef struct
{
    s32 tile;
    s16 field_04;
    s16 field_06;
    u8 neighbors[32];
} WmapTile;

void wmap_land_effect_16_sequence_6_step_02(void);
void wmap_land_effect_16_sequence_11_step_02(void);
void wmap_land_effect_16_wait_idle_02(void);
void wmap_land_effect_16_step_03(void);
s32 wmap_land_effect_16_run_timeline(s32 arg0);
void wmap_land_effect_16_wait_idle_04(void);
void wmap_land_effect_16_end(void);
s32 wmap_land_effect_16_run_sequence_1(s32 arg0);
s32 wmap_land_effect_16_run_sequence_6(s32 arg0);
s32 wmap_land_effect_16_run_sequence_4(s32 arg0);
s32 wmap_land_effect_16_run_sequence_9(s32 arg0);
s32 wmap_land_effect_16_run_sequence_10(s32 arg0);
s32 wmap_land_effect_16_run_sequence_2(s32 arg0);
s32 wmap_land_effect_16_run_sequence_8(s32 arg0);
s32 wmap_land_effect_16_run_sequence_11(s32 arg0);
s32 wmap_land_effect_16_run_sequence_3(s32 arg0);
s32 wmap_land_effect_16_run_sequence_5(s32 arg0);
s32 wmap_land_effect_16_run_sequence_7(s32 arg0);
void wmap_land_effect_16_sequence_1_step_02(void);
void wmap_land_effect_16_sequence_2_step_02(void);
void wmap_land_effect_16_sequence_2_step_04(void);
void wmap_land_effect_16_sequence_3_step_02(void);
void wmap_land_effect_16_sequence_6_step_04(void);
void wmap_land_effect_16_sequence_7_step_02(void);
void wmap_land_effect_16_sequence_9_step_02(void);
void wmap_land_effect_16_sequence_10_step_02(void);
void wmap_land_effect_16_sequence_10_step_04(void);
void wmap_land_effect_16_sequence_11_step_04(void);

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s32 field_10;
} WmapMotion;

extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2C0C;
extern s32 D_80182DEC;
extern u8* D_8011CF1C;
extern s32 D_801B2C14;
extern s32 D_801B2C1C;
extern u8* D_8011CF24;
extern s32 D_8013923C;
extern s32 D_80182DF0;
extern s32 D_801B2C2C;
extern s32 D_801B0FD0;
extern s32 D_801B2C44;
extern s32 D_801B2BE4;
extern void (*D_800D63B8[])(void);
extern s32 D_8013B20C;
extern void wmap_land_effect_16_step_03(void);
extern void wmap_land_effect_16_end(void);
extern s32 D_801B2BEC;
extern void (*D_800D63D0[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern WmapTile D_80139290[6][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2BF4;
extern void (*D_800D6430[])(void);
extern u8* D_801399AC;
extern void wmap_land_effect_16_sequence_1_step_02(void);
extern s32 D_801B2BFC;
extern void (*D_800D6440[])(void);
extern void *D_801399D4;
extern void wmap_land_effect_16_sequence_2_step_04(void);
extern s32 D_801B2C04;
extern void (*D_800D6458[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern void wmap_land_effect_16_sequence_3_step_02(void);
extern void (*D_800D6468[])(void);
extern void (*D_800D6478[])(void);
extern void (*D_800D6488[])(void);
extern s32 D_801B2C24;
extern void (*D_800D64A0[])(void);
extern u8 *D_801399BC;
extern void (*D_800D64B0[])(void);
extern s32 D_801B2C34;
extern void (*D_800D64C8[])(void);
extern u8 D_80121538[];
extern u8* D_801399DC;
extern void wmap_land_effect_16_sequence_9_step_02(void);
extern s32 D_801B2C3C;
extern void (*D_800D64D8[])(void);
extern u8 *D_801399E4;
extern void wmap_land_effect_16_sequence_10_step_04(void);
extern void (*D_800D64F0[])(void);
extern u8 D_800DB578[];
extern WmapAnimationSlot D_80139FE8[];
extern void wmap_land_effect_16_sequence_11_step_04(void);

/** @brief World-map actor configuration with its original field layout. */
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

extern u32 D_801B2C08;
extern u32 D_801B2C10;
extern u8 D_8011F538[];
extern u32 D_801B2C18;
extern u32 D_801B2C28;
extern u32 D_801B2C40;
extern u32 D_801B2BE0;
extern u32 D_801B2BE8;
extern u32 D_801B2BF0;
extern u32 D_801B2BF8;
extern u32 D_801B2C00;
extern u32 D_801B2C20;
extern u32 D_801B2C30;
extern u32 D_801B2C38;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;

extern SVECTOR D_80139258;
extern SVECTOR D_801B24A0;
extern SVECTOR D_8013B238;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B24A8;

extern WmapSpriteActor D_800D9268[];
extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370;
extern WmapSpriteActor D_800D93F4;
extern WmapSpriteActor D_800D9420;
extern WmapSpriteActor D_800D944C;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8;
extern WmapAnimationSlot D_801399D0;
extern WmapAnimationSlot D_801399D8;
extern WmapAnimationSlot D_801399E0;

extern WmapScreenPosition g_wmap_focus_screen_position;
extern WmapScreenPosition D_80182D60;

extern s32* D_80139280;

extern WmapMotion D_801AFBD0[];

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_16_sequence_4_step_02(void)
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
    if (--D_801B2C0C == 0)
    {
        D_801B2C08 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_16_sequence_5_step_02(void)
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
    if (--D_801B2C14 == 0)
    {
        D_801B2C10 += 1;
    }
}

/** @brief Initialize four effect actors and their angular spacing. */
void wmap_land_effect_16_sequence_6_step_01(void)
{
    s32 i;
    s32 *descriptor;
    WmapConfigA *actor;

    D_801B2490 = D_80139258;
    descriptor = D_80139280;
    descriptor[0] = 1;
    descriptor[1] = 1;
    descriptor[2] = 1;
    D_80139280[3] = 0;
    D_80139280[4] = 1000;
    D_80139280[5] = 96;
    D_80139280[6] = 8;
    D_80139280[7] = 255;
    D_80139280[8] = 10;
    D_80139280[9] = 19;
    for (i = 20; i < 80; i++)
    {
        D_801AFBD0[i].state = 0;
    }
    for (i = 20; i < 80; i += 15)
    {
        actor = &D_800D9268[i];
        D_80139988[i].data = D_8011F538;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_02 = 0;
        actor->field_0E = 0;
        actor->field_26 = 2;
        actor->field_22 = 128;
        actor->field_24 = 128;
        D_801AFBD0[i].state = 1;
        D_801AFBD0[i].z = 0;
        D_801AFBD0[i].angle = D_80139280[3];
        D_801AFBD0[i].field_0E = 0;
        D_80139280[3] += 1024;
    }
    D_801B2C1C = 48;
    D_801B2C18++;
    wmap_land_effect_16_sequence_6_step_02();
}

/** @brief Advance the world-map effect and its sequence state. */
void wmap_land_effect_16_sequence_8_step_02(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model_default(D_8011CF24, D_8013923C, 4, 0x35, 0x7800, 0x1001, D_80182DF0);
    D_8013B238.vz = (u16) (D_8013B238.vz + 0x10);
    intensity = D_80182DF0 + 2;
    D_80182DF0 = intensity;
    if (intensity >= 0x82)
    {
        D_80182DF0 = 0x81;
    }
    D_8013923C = (D_8013923C + 1) & 7;
    PopMatrix();
    remaining = D_801B2C2C - 1;
    D_801B2C2C = remaining;
    if (remaining == 0)
    {
        D_801B2C28 += 1;
    }
}

/** @brief Advance the world-map effect and its sequence state. */
void wmap_land_effect_16_sequence_8_step_04(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model_default(D_8011CF24, D_8013923C, 4, 0x35, 0x7800, 0x1001, D_80182DF0);
    intensity = D_80182DF0 - 2;
    D_8013B238.vz = (u16) (D_8013B238.vz + 0x10);
    D_80182DF0 = intensity;
    if (intensity < 0)
    {
        D_80182DF0 = 0;
    }
    D_8013923C = (D_8013923C + 1) & 7;
    PopMatrix();
    remaining = D_801B2C2C - 1;
    D_801B2C2C = remaining;
    if (remaining == 0)
    {
        D_801B2C28 += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_16_sequence_11_step_01(void)
{
    s32 i;

    D_801B0FD0 = 40;
    D_80139280[0x1F] = -1;
    D_80139280[0x20] = -4;
    D_80139280[0x21] = 0x20;
    D_80139280[0x22] = 0;
    D_80139280[0x23] = 2;
    D_80139280[0x24] = 0;
    D_80139280[0x25] = 0xC8;
    D_80139280[0x26] = 0x13;
    D_80139280[0x27] = 2;
    D_80139280[0x28] = 0x1F40;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + 200].state = 0;
        D_80139988[i + 204].data = D_8011F538;
    }
    D_801B2C44 = 80;
    D_801B2C40++;
    wmap_land_effect_16_sequence_11_step_02();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_16_run(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BE0 = 1;
        D_801B2BE4 = 1;
        return 1;
    }

    if (D_801B2BE0 < 0x6)
    {
        D_800D63B8[D_801B2BE0]();
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
void wmap_land_effect_16_reset(void)
{
    D_801B2BE0 = 1;
    D_801B2BE4 = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_16_step_01(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    D_8013B20C = 1;
    D_801B2BE0 += 1;
    wmap_land_effect_16_wait_idle_02();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_16_wait_idle_02(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2BE0 += 1;
        wmap_land_effect_16_step_03();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_16_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_16_run_timeline);
    D_8013B20C = 1;
    D_801B2BE0 += 1;
    wmap_land_effect_16_wait_idle_04();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_16_wait_idle_04(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2BE0 += 1;
        wmap_land_effect_16_end();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_16_end(void)
{
    D_801B2BE0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_16_run_timeline(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BE8 = 1;
        D_801B2BEC = 1;
        return 1;
    }

    if (D_801B2BE8 < 0x18)
    {
        D_800D63D0[D_801B2BE8]();
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
void wmap_land_effect_16_timeline_reset(void)
{
    D_801B2BE8 = 1;
    D_801B2BEC = 1;
}

/** @brief World-map step handler: kick a sub-task and advance the step counter. */
void wmap_land_effect_16_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 4;
    D_801B2BEC = 0x1E;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_16_timeline_wait_02(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/** @brief World-map step handler: register a callback and advance the step counter. */
void wmap_land_effect_16_timeline_step_03(void)
{
    wmap_play_sound(0x27, 0x80);
    wmap_start_sequence(wmap_land_effect_16_run_sequence_1);
    D_801B2BEC = 0x10;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_16_timeline_wait_04(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_16_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_16_run_sequence_6);
    D_801B2BEC = 0x2;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_16_timeline_wait_06(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void wmap_land_effect_16_timeline_step_07(void)
{
    D_801ADAE0 = 1;
    D_801B2BEC = 0x28;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_16_timeline_wait_08(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_16_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_16_run_sequence_4);
    D_801B2BEC = 0x4;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_16_timeline_wait_10(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_16_timeline_step_11(void)
{
    wmap_start_sequence(wmap_land_effect_16_run_sequence_9);
    D_801B2BEC = 0x14;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_16_timeline_wait_12(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_16_timeline_step_13(void)
{
    wmap_start_sequence(wmap_land_effect_16_run_sequence_10);
    D_801B2BEC = 0x1E;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_16_timeline_wait_14(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_16_timeline_step_15(void)
{
    wmap_start_sequence(wmap_land_effect_16_run_sequence_2);
    wmap_start_sequence(wmap_land_effect_16_run_sequence_8);
    D_801B2BEC = 0x8;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_16_timeline_wait_16(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_16_timeline_step_17(void)
{
    wmap_start_sequence(wmap_land_effect_16_run_sequence_11);
    D_801B2BEC = 0xF;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_16_timeline_wait_18(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_16_timeline_step_19(void)
{
    wmap_start_sequence(wmap_land_effect_16_run_sequence_3);
    D_801B2BEC = 0x80;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_16_timeline_wait_20(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_16_timeline_step_21(void)
{
    wmap_start_sequence(wmap_land_effect_16_run_sequence_5);
    wmap_start_sequence(wmap_land_effect_16_run_sequence_7);
    D_801B2BEC = 0x80;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_16_timeline_wait_22(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/** @brief Mark the current world-map tile state and advance the sequence. */
void wmap_land_effect_16_timeline_finish(void)
{
    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].tile = D_8011D4FC | 0x100;
    D_801B2BE8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_16_run_sequence_1(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BF0 = 1;
        D_801B2BF4 = 1;
        return 1;
    }

    if (D_801B2BF0 < 0x4)
    {
        D_800D6430[D_801B2BF0]();
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
void wmap_land_effect_16_sequence_1_reset(void)
{
    D_801B2BF0 = 1;
    D_801B2BF4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_16_sequence_1_step_01(void)
{
    D_801399AC = D_8011F538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 0x10;
    D_800D9318.target_shade = 0x81;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.shade = 1;
    D_801B2BF4 = 0x40;
    D_801B2BF0 += 1;
    wmap_land_effect_16_sequence_1_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_16_sequence_1_step_02(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0x13, 0x2, 0);
    if (--D_801B2BF4 == 0)
    {
        D_801B2BF0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_16_sequence_1_end(void)
{
    D_801B2BF0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_16_run_sequence_2(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BF8 = 1;
        D_801B2BFC = 1;
        return 1;
    }

    if (D_801B2BF8 < 0x6)
    {
        D_800D6440[D_801B2BF8]();
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
void wmap_land_effect_16_sequence_2_reset(void)
{
    D_801B2BF8 = 1;
    D_801B2BFC = 1;
}

/** @brief Initialize the world-map actor and advance the timed sequence. */
void wmap_land_effect_16_sequence_2_step_01(void)
{
    D_801399D4 = &D_8011F538;
    D_800D93F4.scale_index = 0xF;
    D_800D93F4.resource_index = 0;
    D_800D93F4.sequence = 1;
    D_800D93F4.previous_sequence = -1;
    D_800D93F4.shade_step = 2;
    D_800D93F4.target_shade = 0x81;
    D_800D93F4.shade = 1;
    D_801B2BFC = 0x64;
    D_801B2BF8 += 1;
    wmap_land_effect_16_sequence_2_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_16_sequence_2_step_02(void)
{
    wmap_step_actor_animation(&D_800D93F4, &D_801399D0);
    wmap_draw_actor_sprite(&D_800D93F4, g_wmap_focus_screen_position.packed, 0x13, 0x2, 0);
    if (--D_801B2BFC == 0)
    {
        D_801B2BF8 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_16_sequence_2_step_03(void)
{
    D_800D93F4.target_shade = 0;
    D_800D93F4.shade_step = 2;
    D_801B2BFC = 0x40;
    D_801B2BF8 += 1;
    wmap_land_effect_16_sequence_2_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_16_sequence_2_step_04(void)
{
    wmap_step_actor_animation(&D_800D93F4, &D_801399D0);
    wmap_draw_actor_sprite(&D_800D93F4, g_wmap_focus_screen_position.packed, 0x13, 0x2, 0);
    if (--D_801B2BFC == 0)
    {
        D_801B2BF8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_16_sequence_2_end(void)
{
    D_801B2BF8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_16_run_sequence_3(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C00 = 1;
        D_801B2C04 = 1;
        return 1;
    }

    if (D_801B2C00 < 0x4)
    {
        D_800D6458[D_801B2C00]();
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
void wmap_land_effect_16_sequence_3_reset(void)
{
    D_801B2C00 = 1;
    D_801B2C04 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_16_sequence_3_step_01(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 1;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0;
    D_801B2C04 = 0x81;
    D_801B2C00 += 1;
    wmap_land_effect_16_sequence_3_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_16_sequence_3_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x18, 0x8, 0);
    if (--D_801B2C04 == 0)
    {
        D_801B2C00 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_16_sequence_3_end(void)
{
    D_801B2C00 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_16_run_sequence_4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C08 = 1;
        D_801B2C0C = 1;
        return 1;
    }

    if (D_801B2C08 < 0x4)
    {
        D_800D6468[D_801B2C08]();
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
void wmap_land_effect_16_sequence_4_reset(void)
{
    D_801B2C08 = 1;
    D_801B2C0C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_16_sequence_4_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B2C0C = 0x40;
    D_801B2C08 += 1;
    wmap_land_effect_16_sequence_4_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_16_sequence_4_end(void)
{
    D_801B2C08 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_16_run_sequence_5(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C10 = 1;
        D_801B2C14 = 1;
        return 1;
    }

    if (D_801B2C10 < 0x4)
    {
        D_800D6478[D_801B2C10]();
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
void wmap_land_effect_16_sequence_5_reset(void)
{
    D_801B2C10 = 1;
    D_801B2C14 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_16_sequence_5_step_01(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    D_801B2C14 = 0x40;
    D_801B2C10 += 1;
    wmap_land_effect_16_sequence_5_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_16_sequence_5_end(void)
{
    D_801B2C10 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_16_run_sequence_6(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C18 = 1;
        D_801B2C1C = 1;
        return 1;
    }

    if (D_801B2C18 < 0x6)
    {
        D_800D6488[D_801B2C18]();
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
void wmap_land_effect_16_sequence_6_reset(void)
{
    D_801B2C18 = 1;
    D_801B2C1C = 1;
}

/** @brief World-map step: kick off a descriptor animation, then tick the sub-counter. */
void wmap_land_effect_16_sequence_6_step_02(void)
{
    func_8006BC44(0x14, 0x3C, D_80139280, 1);
    if (--D_801B2C1C == 0)
    {
        D_801B2C18 += 1;
    }
}

/** @brief Reset four actor configurations and begin a 16-tick sequence step. */
void wmap_land_effect_16_sequence_6_step_03(void)
{
    s32 index;

    D_80139280[0] = -1;
    D_80139280[4] = 0;
    D_80139280[5] = 0;
    for (index = 20; index < 80; index += 15)
    {
        D_800D9268[index].target_shade = 0;
        D_800D9268[index].shade_step = 8;
    }
    D_801B2C1C = 0x10;
    D_801B2C18 += 1;
    wmap_land_effect_16_sequence_6_step_04();
}

/** @brief World-map step: kick off a descriptor animation, then tick the sub-counter. */
void wmap_land_effect_16_sequence_6_step_04(void)
{
    func_8006BC44(0x14, 0x3C, D_80139280, 1);
    if (--D_801B2C1C == 0)
    {
        D_801B2C18 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_16_sequence_6_end(void)
{
    D_801B2C18 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_16_run_sequence_7(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C20 = 1;
        D_801B2C24 = 1;
        return 1;
    }

    if (D_801B2C20 < 0x4)
    {
        D_800D64A0[D_801B2C20]();
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
void wmap_land_effect_16_sequence_7_reset(void)
{
    D_801B2C20 = 1;
    D_801B2C24 = 1;
}

/** @brief Initialize the actor configuration and begin a 129-tick sequence step. */
void wmap_land_effect_16_sequence_7_step_01(void)
{
    D_801399BC = D_8011D538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.sequence = 1;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 1;
    D_800D9370.resource_index = 0;
    D_800D9370.target_shade = 0x80;
    D_800D9370.shade = 0x80;
    D_801B2C24 = 0x81;
    D_801B2C20 += 1;
    wmap_land_effect_16_sequence_7_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_16_sequence_7_step_02(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x18, 0x8, 0);
    if (--D_801B2C24 == 0)
    {
        D_801B2C20 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_16_sequence_7_end(void)
{
    D_801B2C20 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_16_run_sequence_8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C28 = 1;
        D_801B2C2C = 1;
        return 1;
    }

    if (D_801B2C28 < 0x6)
    {
        D_800D64B0[D_801B2C28]();
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
void wmap_land_effect_16_sequence_8_reset(void)
{
    D_801B2C28 = 1;
    D_801B2C2C = 1;
}

/** @brief Restore effect data, clear two flags, and begin a 100-tick sequence step. */
void wmap_land_effect_16_sequence_8_step_01(void)
{
    D_8013B238 = D_80139258;
    D_80182DF0 = 0;
    D_8013923C = 0;
    D_801B2C2C = 0x64;
    D_801B2C28 += 1;
    wmap_land_effect_16_sequence_8_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_16_sequence_8_step_03(void)
{
    D_801B2C2C = 0x40;
    D_801B2C28 += 1;
    wmap_land_effect_16_sequence_8_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_16_sequence_8_end(void)
{
    D_801B2C28 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_16_run_sequence_9(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C30 = 1;
        D_801B2C34 = 1;
        return 1;
    }

    if (D_801B2C30 < 0x4)
    {
        D_800D64C8[D_801B2C30]();
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
void wmap_land_effect_16_sequence_9_reset(void)
{
    D_801B2C30 = 1;
    D_801B2C34 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_16_sequence_9_step_01(void)
{
    D_801399DC = D_80121538;
    D_800D9420.scale_index = 0xF;
    D_800D9420.previous_sequence = -1;
    D_800D9420.shade_step = 4;
    D_800D9420.target_shade = 0x81;
    D_800D9420.resource_index = 0;
    D_800D9420.sequence = 0;
    D_800D9420.shade = 1;
    D_801B2C34 = 0x3C;
    D_801B2C30 += 1;
    wmap_land_effect_16_sequence_9_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_16_sequence_9_step_02(void)
{
    wmap_step_actor_animation(&D_800D9420, &D_801399D8);
    wmap_draw_actor_sprite(&D_800D9420, g_wmap_focus_screen_position.packed, 0x13, 0x2, 0);
    if (--D_801B2C34 == 0)
    {
        D_801B2C30 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_16_sequence_9_end(void)
{
    D_801B2C30 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_16_run_sequence_10(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C38 = 1;
        D_801B2C3C = 1;
        return 1;
    }

    if (D_801B2C38 < 0x6)
    {
        D_800D64D8[D_801B2C38]();
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
void wmap_land_effect_16_sequence_10_reset(void)
{
    D_801B2C38 = 1;
    D_801B2C3C = 1;
}

/** @brief Initialize the actor and its screen coordinates, then advance the sequence. */
void wmap_land_effect_16_sequence_10_step_01(void)
{
    s16 screen_y;

    D_801399E4 = D_80121538;
    D_800D944C.scale_index = 0xF;
    D_800D944C.resource_index = 0;
    D_800D944C.sequence = 1;
    D_800D944C.previous_sequence = -1;
    D_800D944C.shade_step = 0x20;
    D_800D944C.target_shade = 0x81;
    D_800D944C.shade = 1;
    D_801B2C3C = 0x80;
    D_80182D60.point.x = (u16) g_wmap_focus_screen_position.packed;
    screen_y = g_wmap_focus_screen_position.packed - 0x38;
    D_80182D60.point.y = screen_y;
    D_801B2C38 += 1;
    wmap_land_effect_16_sequence_10_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_16_sequence_10_step_02(void)
{
    wmap_step_actor_animation(&D_800D944C, &D_801399E0);
    wmap_draw_actor_sprite(&D_800D944C, D_80182D60.packed, 0x13, 0x2, 0);
    if (--D_801B2C3C == 0)
    {
        D_801B2C38 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_16_sequence_10_step_03(void)
{
    D_800D944C.target_shade = 0;
    D_800D944C.shade_step = 2;
    D_801B2C3C = 0x40;
    D_801B2C38 += 1;
    wmap_land_effect_16_sequence_10_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_16_sequence_10_step_04(void)
{
    wmap_step_actor_animation(&D_800D944C, &D_801399E0);
    wmap_draw_actor_sprite(&D_800D944C, D_80182D60.packed, 0x13, 0x2, 0);
    if (--D_801B2C3C == 0)
    {
        D_801B2C38 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_16_sequence_10_end(void)
{
    D_801B2C38 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_16_run_sequence_11(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C40 = 1;
        D_801B2C44 = 1;
        return 1;
    }

    if (D_801B2C40 < 0x6)
    {
        D_800D64F0[D_801B2C40]();
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
void wmap_land_effect_16_sequence_11_reset(void)
{
    D_801B2C40 = 1;
    D_801B2C44 = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_16_sequence_11_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB578, D_80139FE8, 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2C44 == 0)
    {
        D_801B2C40 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_16_sequence_11_step_03(void)
{
    D_801B2C44 = 0x20;
    D_80139280[35] = -1;
    D_801B2C40 += 1;
    wmap_land_effect_16_sequence_11_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_16_sequence_11_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB578, D_80139FE8, 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2C44 == 0)
    {
        D_801B2C40 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_16_sequence_11_end(void)
{
    D_801B2C40 += 1;
}
