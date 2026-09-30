#include "wmap_model_render.h"
#include "wmap_land_effect_26.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_sprite_render.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_effect_primitives.h"

/** @brief Screen coordinates and their packed renderer argument. */
typedef union
{
    s32 packed;
    struct
    {
        s16 x, y;
    } point;
} WmapScreenPoint;

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s16 field_10;
    s16 field_12;
} WmapMotion;

/** @brief World-map sprite configuration. */
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

/** @brief Land identifier and remaining storage of a map cell. */
typedef struct
{
    u32 land_id;
    u8 unknown_4[36];
} WmapValueRecord;

void wmap_land_effect_26_sequence_11_step_02(void);
void wmap_land_effect_26_wait_idle_02(void);
void wmap_land_effect_26_step_03(void);
s32 wmap_land_effect_26_run_timeline(s32 arg0);
void wmap_land_effect_26_wait_idle_04(void);
void wmap_land_effect_26_end(void);
s32 wmap_land_effect_26_run_sequence_8(s32 arg0);
s32 wmap_land_effect_26_run_sequence_2(s32 arg0);
s32 wmap_land_effect_26_run_sequence_1(s32 arg0);
s32 wmap_land_effect_26_run_sequence_7(s32 arg0);
s32 wmap_land_effect_26_run_sequence_9(s32 arg0);
s32 wmap_land_effect_26_run_sequence_4(s32 arg0);
s32 wmap_land_effect_26_run_sequence_5(s32 arg0);
s32 wmap_land_effect_26_run_sequence_6(s32 arg0);
s32 wmap_land_effect_26_run_sequence_3(s32 arg0);
s32 wmap_land_effect_26_run_sequence_11(s32 arg0);
s32 wmap_land_effect_26_run_sequence_10(s32 arg0);
void wmap_land_effect_26_sequence_7_step_02(void);
void wmap_land_effect_26_sequence_9_step_02(void);
void wmap_land_effect_26_sequence_9_step_04(void);
void wmap_land_effect_26_sequence_8_step_02(void);
void wmap_land_effect_26_sequence_8_step_04(void);
void wmap_land_effect_26_sequence_10_step_02(void);
void wmap_land_effect_26_sequence_11_step_04(void);
void wmap_land_effect_26_sequence_11_step_06(void);

extern u8 D_800DCF18[];
extern s32 D_8011CF74;
extern s32 D_801B2470;
extern s32 g_wmap_land_effect_26_sequence_1_timer;
extern void *D_8011CF1C;
extern s32 D_801B2474;
extern s32 g_wmap_land_effect_26_sequence_2_timer;
extern void *D_8011CF24;
extern VECTOR D_8011CF60;
extern s32 g_wmap_land_effect_26_sequence_3_timer;
extern s32 D_801B24B4;
extern s32 D_80182DE4;
extern u8* D_8011CF28;
extern s32 g_wmap_land_effect_26_sequence_4_timer;
extern s32 D_80182DE8;
extern u8* D_8011CF2C;
extern s32 g_wmap_land_effect_26_sequence_5_timer;
extern s32 D_80182DEC;
extern u8* D_8011CF30;
extern s32 g_wmap_land_effect_26_sequence_6_timer;
extern u8 D_80121538[];
extern s32 g_wmap_land_effect_26_sequence_11_timer;
extern u32 rand(void);
extern s8 D_80051B4C[];
extern s32 g_wmap_land_effect_26_timer;
extern void (*D_800D5148[])(void);
extern void wmap_land_effect_26_step_03(void);
extern void wmap_land_effect_26_end(void);
extern s32 g_wmap_land_effect_26_timeline_timer;
extern void (*D_800D5160[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern void (*D_800D51B0[])(void);
extern void (*D_800D51C0[])(void);
extern void (*D_800D51D0[])(void);
extern void (*D_800D51E0[])(void);
extern void (*D_800D51F0[])(void);
extern void (*D_800D5200[])(void);
extern s32 g_wmap_land_effect_26_sequence_7_timer;
extern void (*D_800D5210[])(void);
extern u8* D_801399AC;
extern u8 D_8011D538[];
extern s32 g_wmap_land_effect_26_sequence_9_timer;
extern void (*D_800D5238[])(void);
extern s32 D_80182DF0;
extern s32 D_801B0FD0;
extern u8 D_800D94D0[];
extern u8 D_801399F8[];
extern s32 g_wmap_land_effect_26_sequence_8_timer;
extern void (*D_800D5220[])(void);
extern u8 *D_801399BC;
extern void wmap_land_effect_26_sequence_8_step_04(void);
extern s32 g_wmap_land_effect_26_sequence_10_timer;
extern void (*D_800D5250[])(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern void wmap_land_effect_26_sequence_10_step_02(void);
extern void (*D_800D5260[])(void);

extern u32 g_wmap_land_effect_26_sequence_1_step;
extern u32 g_wmap_land_effect_26_sequence_2_step;
extern u32 g_wmap_land_effect_26_sequence_3_step;
extern u32 g_wmap_land_effect_26_sequence_4_step;
extern u32 g_wmap_land_effect_26_sequence_5_step;
extern u32 g_wmap_land_effect_26_sequence_6_step;
extern s32 D_80182DF4;
extern u32 g_wmap_land_effect_26_sequence_11_step;
extern u32 g_wmap_land_effect_26_step;
extern u32 g_wmap_land_effect_26_timeline_step;
extern u32 g_wmap_land_effect_26_sequence_7_step;
extern u32 g_wmap_land_effect_26_sequence_9_step;
extern u32 g_wmap_land_effect_26_sequence_8_step;
extern u32 g_wmap_land_effect_26_sequence_10_step;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;
extern VECTOR D_80139870;
extern VECTOR D_80139888;
extern VECTOR D_80139898;
extern VECTOR D_801B2660;

extern SVECTOR D_80139258;
extern SVECTOR D_8013B240;
extern SVECTOR D_801B24A0;
extern SVECTOR D_8013B238;
extern SVECTOR D_801B2670;
extern SVECTOR D_801B24A8;
extern SVECTOR D_801B2678;

extern WmapSpriteActor D_800D9268[];
extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern WmapMotion D_801AFBD0[];

/** @brief Approach the effect depth and draw its alternating-brightness fade. */
void wmap_land_effect_26_sequence_1_step_02(void)
{
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = D_801B2650.vz - 10000;
    D_801B2650.vz = depth;
    if (depth < 1000)
    {
        D_801B2650.vz = 1000;
    }
    PushMatrix();
    wmap_set_model_transform(&D_801B2650, &D_801B24A0);
    if (D_801B2470 != 0)
    {
        if (D_8011CF74 & 1)
        {
            wmap_draw_model(D_800DCF18, 0, 4, -1, -1, 1, D_801B2470, 5, -20, -1);
        }
        else
        {
            wmap_draw_model(D_800DCF18, 0, 4, -1, -1, 1, D_801B2470 / 2, 5, -20, -1);
        }
        intensity = D_801B2470 - 2;
        D_801B2470 = intensity;
        if (intensity < 0)
        {
            D_801B2470 = 0;
        }
    }
    PopMatrix();
    remaining = g_wmap_land_effect_26_sequence_1_timer - 1;
    g_wmap_land_effect_26_sequence_1_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_26_sequence_1_step++;
    }
}

/** @brief Move the effect toward the camera while fading it and advancing its countdown. */
void wmap_land_effect_26_sequence_2_step_02(void)
{
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = D_801B2478.vz - 2500;
    D_801B2478.vz = depth;
    if (depth < 100)
    {
        D_801B2478.vz = 100;
    }
    PushMatrix();
    wmap_set_model_transform(&D_801B2478, &D_801B24A8);
    if (D_801B2474 != 0)
    {
        wmap_draw_model(D_8011CF1C, 0, 4, -1, -1, 1, D_801B2474, 5, -20, -1);
    }
    PopMatrix();
    intensity = D_801B2474 - 8;
    D_801B2474 = intensity;
    if (intensity < 0)
    {
        D_801B2474 = 0;
    }
    remaining = g_wmap_land_effect_26_sequence_2_timer - 1;
    g_wmap_land_effect_26_sequence_2_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_26_sequence_2_step++;
    }
}

/** @brief Draw and fade the transformed effect, then advance its countdown. */
void wmap_land_effect_26_sequence_3_step_02(void)
{
    MATRIX matrix;
    s32 intensity;
    s32 remaining;

    if (D_80139870.vz < 10)
    {
        D_80139870.vz = 10;
    }
    PushMatrix();
    RotMatrix(&D_8013B238, &matrix);
    TransMatrix(&matrix, &D_8011CF60);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    if (D_801B24B4 != 0)
    {
        wmap_draw_model_default(D_8011CF24, 0, 4, -1, -1, 1, D_801B24B4);
        intensity = D_801B24B4 - 8;
        D_801B24B4 = intensity;
        if (intensity < 0)
        {
            D_801B24B4 = 0;
        }
    }
    PopMatrix();
    remaining = g_wmap_land_effect_26_sequence_3_timer - 1;
    g_wmap_land_effect_26_sequence_3_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_26_sequence_3_step++;
    }
}

/** @brief World-map step handler: decay a value, draw the model while active, then countdown-advance. */
void wmap_land_effect_26_sequence_4_step_02(void)
{
    s32 v1;

    v1 = D_80139888.vz - 0x5DC;
    D_80139888.vz = v1;
    if (v1 < 0x9C40)
    {
        D_80139888.vz = 0x9C40;
    }

    PushMatrix();
    wmap_set_model_transform(&D_80139888, &D_8013B240);

    if (D_80182DE4 != 0)
    {
        wmap_draw_model_default(D_8011CF28, 0, 0xC, 0x35, 0x7800, 1, D_80182DE4);
        if (D_80182DE4 < 0)
        {
            D_80182DE4 = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_26_sequence_4_timer == 0)
    {
        g_wmap_land_effect_26_sequence_4_step += 1;
    }
}

/** @brief World-map step handler: decay a value, draw the model while active, then countdown-advance. */
void wmap_land_effect_26_sequence_5_step_02(void)
{
    s32 v1;

    v1 = D_80139898.vz - 0x5DC;
    D_80139898.vz = v1;
    if (v1 < 0x9C40)
    {
        D_80139898.vz = 0x9C40;
    }

    PushMatrix();
    wmap_set_model_transform(&D_80139898, &D_801B2670);

    if (D_80182DE8 != 0)
    {
        wmap_draw_model_default(D_8011CF2C, 0, 0xC, 0x35, 0x7800, 1, D_80182DE8);
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_26_sequence_5_timer == 0)
    {
        g_wmap_land_effect_26_sequence_5_step += 1;
    }
}

/** @brief World-map step handler: decay a value, draw the model while active, then countdown-advance. */
void wmap_land_effect_26_sequence_6_step_02(void)
{
    s32 v1;

    v1 = D_801B2660.vz - 0x5DC;
    D_801B2660.vz = v1;
    if (v1 < 0x9C40)
    {
        D_801B2660.vz = 0x9C40;
    }

    PushMatrix();
    wmap_set_model_transform(&D_801B2660, &D_801B2678);

    if (D_80182DEC != 0)
    {
        wmap_draw_model_default(D_8011CF30, 0, 0xC, 0x35, 0x7800, 1, D_80182DEC);
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_26_sequence_6_timer == 0)
    {
        g_wmap_land_effect_26_sequence_6_step += 1;
    }
}

/** @brief Initialize randomized motion for the effect actors. */
void wmap_land_effect_26_sequence_11_step_01(void)
{
    s32 i;
    WmapConfigA *actor;

    D_80182DF4 = 1;
    for (i = 124; i < 170; i++)
    {
        actor = &D_800D9268[i];
        D_801AFBD0[i].state = 1;
        D_801AFBD0[i].angle = (rand() * 155) >> 10;
        D_801AFBD0[i].field_0E = (rand() * 7) >> 6;
        D_801AFBD0[i].x = ((s32)(rand() << 6) >> 15) + 16;
        D_801AFBD0[i].scale = rand() >> 9;
        D_801AFBD0[i].field_10 = ((s32)(rand() << 6) >> 15) + 4;
        D_801AFBD0[i].field_12 = 0;
        D_80139988[i].data = D_80121538;
        actor->field_06 = 15;
        actor->field_02 = 0;
        actor->field_10 = -1;
        actor->field_26 = 0;
        actor->field_22 = D_80182DF4;
        actor->field_24 = D_80182DF4;
        actor->field_0E = i % 3;
    }
    g_wmap_land_effect_26_sequence_11_timer = 16;
    g_wmap_land_effect_26_sequence_11_step++;
    wmap_land_effect_26_sequence_11_step_02();
}

/** @brief Advance oscillating actors and draw their sprites.
 * @param start First actor index.
 * @param end Exclusive end index.
 * @param depth Rendering depth.
 */
void func_800773B8(s32 start, s32 end, s32 depth)
{
    s32 i;
    WmapMotion *motion;
    WmapConfigA *actor;
    WmapScreenPoint screen;

    for (i = start; i < end; i++)
    {
        actor = &D_800D9268[i];
        motion = &D_801AFBD0[i];
        motion->field_0E += motion->x;
        if (motion->field_0E >= 3841)
        {
            motion->field_0E = 0;
        }
        motion->field_12 = (motion->field_12 + motion->field_10) & 4095;
        screen.point.x = (motion->angle + (D_80051B4C[motion->field_12 / 16] * motion->scale) / 16) / 16;
        screen.point.y = motion->field_0E / 16;
        actor->field_22 = D_80182DF4;
        actor->field_24 = D_80182DF4;
        wmap_step_actor_animation(actor, &D_80139988[i]);
        wmap_draw_actor_sprite(actor, screen.packed, depth, 4, 0);
    }
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_26_run(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_26_step = 1;
        g_wmap_land_effect_26_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_26_step < 0x6)
    {
        D_800D5148[g_wmap_land_effect_26_step]();
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
void wmap_land_effect_26_reset(void)
{
    g_wmap_land_effect_26_step = 1;
    g_wmap_land_effect_26_timer = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_26_step_01(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_26_step += 1;
    wmap_land_effect_26_wait_idle_02();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_26_wait_idle_02(void)
{
    if (g_wmap_sequence_busy == 0)
    {
        g_wmap_land_effect_26_step += 1;
        wmap_land_effect_26_step_03();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_26_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_26_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_26_step += 1;
    wmap_land_effect_26_wait_idle_04();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_26_wait_idle_04(void)
{
    if (g_wmap_sequence_busy == 0)
    {
        g_wmap_land_effect_26_step += 1;
        wmap_land_effect_26_end();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_26_end(void)
{
    g_wmap_land_effect_26_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_26_run_timeline(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_26_timeline_step = 1;
        g_wmap_land_effect_26_timeline_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_26_timeline_step < 0x14)
    {
        D_800D5160[g_wmap_land_effect_26_timeline_step]();
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
void wmap_land_effect_26_timeline_reset(void)
{
    g_wmap_land_effect_26_timeline_step = 1;
    g_wmap_land_effect_26_timeline_timer = 1;
}

/** @brief World-map step handler: set up the actor, register callbacks, advance step. */
void wmap_land_effect_26_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_start_map_tint(0x804020);
    wmap_start_sequence(wmap_land_effect_26_run_sequence_8);
    wmap_play_sound(0x19, 0x80);
    g_wmap_land_effect_26_timeline_timer = 2;
    g_wmap_land_effect_26_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_26_timeline_wait_02(void)
{
    if (--g_wmap_land_effect_26_timeline_timer == 0)
    {
        g_wmap_land_effect_26_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_26_timeline_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_26_run_sequence_2);
    g_wmap_land_effect_26_timeline_timer = 0x4;
    g_wmap_land_effect_26_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_26_timeline_wait_04(void)
{
    if (--g_wmap_land_effect_26_timeline_timer == 0)
    {
        g_wmap_land_effect_26_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_26_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_26_run_sequence_1);
    g_wmap_land_effect_26_timeline_timer = 0x8;
    g_wmap_land_effect_26_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_26_timeline_wait_06(void)
{
    if (--g_wmap_land_effect_26_timeline_timer == 0)
    {
        g_wmap_land_effect_26_timeline_step += 1;
    }
}

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void wmap_land_effect_26_timeline_step_07(void)
{
    D_801ADAE0 = 1;
    wmap_start_sequence(wmap_land_effect_26_run_sequence_7);
    g_wmap_land_effect_26_timeline_timer = 0x8;
    g_wmap_land_effect_26_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_26_timeline_wait_08(void)
{
    if (--g_wmap_land_effect_26_timeline_timer == 0)
    {
        g_wmap_land_effect_26_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_26_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_26_run_sequence_9);
    g_wmap_land_effect_26_timeline_timer = 0x14;
    g_wmap_land_effect_26_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_26_timeline_wait_10(void)
{
    if (--g_wmap_land_effect_26_timeline_timer == 0)
    {
        g_wmap_land_effect_26_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_26_timeline_step_11(void)
{
    wmap_start_sequence(wmap_land_effect_26_run_sequence_4);
    g_wmap_land_effect_26_timeline_timer = 0xC;
    g_wmap_land_effect_26_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_26_timeline_wait_12(void)
{
    if (--g_wmap_land_effect_26_timeline_timer == 0)
    {
        g_wmap_land_effect_26_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_26_timeline_step_13(void)
{
    wmap_start_sequence(wmap_land_effect_26_run_sequence_5);
    g_wmap_land_effect_26_timeline_timer = 0xC;
    g_wmap_land_effect_26_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_26_timeline_wait_14(void)
{
    if (--g_wmap_land_effect_26_timeline_timer == 0)
    {
        g_wmap_land_effect_26_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_26_timeline_step_15(void)
{
    wmap_start_sequence(wmap_land_effect_26_run_sequence_6);
    g_wmap_land_effect_26_timeline_timer = 0x10;
    g_wmap_land_effect_26_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_26_timeline_wait_16(void)
{
    if (--g_wmap_land_effect_26_timeline_timer == 0)
    {
        g_wmap_land_effect_26_timeline_step += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_26_timeline_step_17(void)
{
    wmap_start_sequence(wmap_land_effect_26_run_sequence_3);
    wmap_start_sequence(wmap_land_effect_26_run_sequence_11);
    wmap_start_sequence(wmap_land_effect_26_run_sequence_10);
    g_wmap_land_effect_26_timeline_timer = 0xAC;
    g_wmap_land_effect_26_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_26_timeline_wait_18(void)
{
    if (--g_wmap_land_effect_26_timeline_timer == 0)
    {
        g_wmap_land_effect_26_timeline_step += 1;
    }
}

/** @brief Mark the selected map cell with the active land and advance the effect. */
void wmap_land_effect_26_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    D_80139290[D_8011D510][D_8011D530].land_id = D_8011D4FC | 0x100;
    g_wmap_land_effect_26_timeline_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 wmap_land_effect_26_run_sequence_1(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_26_sequence_1_step = 1;
        g_wmap_land_effect_26_sequence_1_timer = 1;
    }

    if (g_wmap_land_effect_26_sequence_1_step < 0x4)
    {
        D_800D51B0[g_wmap_land_effect_26_sequence_1_step]();
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
void wmap_land_effect_26_sequence_1_reset(void)
{
    g_wmap_land_effect_26_sequence_1_step = 1;
    g_wmap_land_effect_26_sequence_1_timer = 1;
}

/** @brief Restore effect data, reset its vector, and begin a 48-tick sequence step. */
void wmap_land_effect_26_sequence_1_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2470 = 0x80;
    D_801B2650.vz = 0x2710;
    D_801B2650.vy = 0;
    D_801B2650.vx = 0;
    g_wmap_land_effect_26_sequence_1_timer = 0x30;
    g_wmap_land_effect_26_sequence_1_step += 1;
    wmap_land_effect_26_sequence_1_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_26_sequence_1_end(void)
{
    g_wmap_land_effect_26_sequence_1_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 wmap_land_effect_26_run_sequence_2(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_26_sequence_2_step = 1;
        g_wmap_land_effect_26_sequence_2_timer = 1;
    }

    if (g_wmap_land_effect_26_sequence_2_step < 0x4)
    {
        D_800D51C0[g_wmap_land_effect_26_sequence_2_step]();
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
void wmap_land_effect_26_sequence_2_reset(void)
{
    g_wmap_land_effect_26_sequence_2_step = 1;
    g_wmap_land_effect_26_sequence_2_timer = 1;
}

/** @brief Restore effect data, reset its vector, and begin a 16-tick sequence step. */
void wmap_land_effect_26_sequence_2_step_01(void)
{
    D_801B24A8 = D_80139258;
    D_801B2474 = 0x80;
    D_801B2478.vz = 0xC350;
    D_801B2478.vy = 0;
    D_801B2478.vx = 0;
    g_wmap_land_effect_26_sequence_2_timer = 0x10;
    g_wmap_land_effect_26_sequence_2_step += 1;
    wmap_land_effect_26_sequence_2_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_26_sequence_2_end(void)
{
    g_wmap_land_effect_26_sequence_2_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 wmap_land_effect_26_run_sequence_3(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_26_sequence_3_step = 1;
        g_wmap_land_effect_26_sequence_3_timer = 1;
    }

    if (g_wmap_land_effect_26_sequence_3_step < 0x4)
    {
        D_800D51D0[g_wmap_land_effect_26_sequence_3_step]();
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
void wmap_land_effect_26_sequence_3_reset(void)
{
    g_wmap_land_effect_26_sequence_3_step = 1;
    g_wmap_land_effect_26_sequence_3_timer = 1;
}

/** @brief Restore the default transform and advance the sequence. */
void wmap_land_effect_26_sequence_3_step_01(void)
{
    D_8013B238 = D_80139258;
    D_80139870 = g_wmap_camera_translation;
    D_801B24B4 = 0x80;
    D_80139870.vz = 1000;
    g_wmap_land_effect_26_sequence_3_timer = 0x80;
    g_wmap_land_effect_26_sequence_3_step++;
    wmap_land_effect_26_sequence_3_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_26_sequence_3_end(void)
{
    g_wmap_land_effect_26_sequence_3_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 wmap_land_effect_26_run_sequence_4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_26_sequence_4_step = 1;
        g_wmap_land_effect_26_sequence_4_timer = 1;
    }

    if (g_wmap_land_effect_26_sequence_4_step < 0x4)
    {
        D_800D51E0[g_wmap_land_effect_26_sequence_4_step]();
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
void wmap_land_effect_26_sequence_4_reset(void)
{
    g_wmap_land_effect_26_sequence_4_step = 1;
    g_wmap_land_effect_26_sequence_4_timer = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_26_sequence_4_step_01(void)
{
    D_8013B240 = D_80139258;
    D_80139888 = g_wmap_camera_translation;
    D_80182DE4 = 0x80;
    D_80139888.vz = 0xAFC8;
    g_wmap_land_effect_26_sequence_4_timer = 0x2A;
    g_wmap_land_effect_26_sequence_4_step += 1;
    wmap_land_effect_26_sequence_4_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_26_sequence_4_end(void)
{
    g_wmap_land_effect_26_sequence_4_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 wmap_land_effect_26_run_sequence_5(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_26_sequence_5_step = 1;
        g_wmap_land_effect_26_sequence_5_timer = 1;
    }

    if (g_wmap_land_effect_26_sequence_5_step < 0x4)
    {
        D_800D51F0[g_wmap_land_effect_26_sequence_5_step]();
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
void wmap_land_effect_26_sequence_5_reset(void)
{
    g_wmap_land_effect_26_sequence_5_step = 1;
    g_wmap_land_effect_26_sequence_5_timer = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_26_sequence_5_step_01(void)
{
    D_801B2670 = D_80139258;
    D_80139898 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_80139898.vz = 0xAFC8;
    g_wmap_land_effect_26_sequence_5_timer = 0x1E;
    g_wmap_land_effect_26_sequence_5_step += 1;
    wmap_land_effect_26_sequence_5_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_26_sequence_5_end(void)
{
    g_wmap_land_effect_26_sequence_5_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 wmap_land_effect_26_run_sequence_6(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_26_sequence_6_step = 1;
        g_wmap_land_effect_26_sequence_6_timer = 1;
    }

    if (g_wmap_land_effect_26_sequence_6_step < 0x4)
    {
        D_800D5200[g_wmap_land_effect_26_sequence_6_step]();
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
void wmap_land_effect_26_sequence_6_reset(void)
{
    g_wmap_land_effect_26_sequence_6_step = 1;
    g_wmap_land_effect_26_sequence_6_timer = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_26_sequence_6_step_01(void)
{
    D_801B2678 = D_80139258;
    D_801B2660 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2660.vz = 0xAFC8;
    g_wmap_land_effect_26_sequence_6_timer = 0x12;
    g_wmap_land_effect_26_sequence_6_step += 1;
    wmap_land_effect_26_sequence_6_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_26_sequence_6_end(void)
{
    g_wmap_land_effect_26_sequence_6_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 wmap_land_effect_26_run_sequence_7(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_26_sequence_7_step = 1;
        g_wmap_land_effect_26_sequence_7_timer = 1;
    }

    if (g_wmap_land_effect_26_sequence_7_step < 0x4)
    {
        D_800D5210[g_wmap_land_effect_26_sequence_7_step]();
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
void wmap_land_effect_26_sequence_7_reset(void)
{
    g_wmap_land_effect_26_sequence_7_step = 1;
    g_wmap_land_effect_26_sequence_7_timer = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_26_sequence_7_step_01(void)
{
    D_801399AC = D_8011D538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.shade_step = 0;
    D_800D9318.target_shade = 0x80;
    D_800D9318.shade = 0x80;
    g_wmap_land_effect_26_sequence_7_timer = 0x4A;
    g_wmap_land_effect_26_sequence_7_step += 1;
    wmap_land_effect_26_sequence_7_step_02();
}

/** @brief World-map step: init a sub-object then count down a timer. */
void wmap_land_effect_26_sequence_7_step_02(void)
{
    s32 n = 0xB;

    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, n, n, 0);
    if (--g_wmap_land_effect_26_sequence_7_timer == 0)
    {
        g_wmap_land_effect_26_sequence_7_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_26_sequence_7_end(void)
{
    g_wmap_land_effect_26_sequence_7_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 wmap_land_effect_26_run_sequence_9(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_26_sequence_9_step = 1;
        g_wmap_land_effect_26_sequence_9_timer = 1;
    }

    if (g_wmap_land_effect_26_sequence_9_step < 0x6)
    {
        D_800D5238[g_wmap_land_effect_26_sequence_9_step]();
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
void wmap_land_effect_26_sequence_9_reset(void)
{
    g_wmap_land_effect_26_sequence_9_step = 1;
    g_wmap_land_effect_26_sequence_9_timer = 1;
}

/** @brief Initialize twenty effect records and begin a 32-tick sequence step. */
void wmap_land_effect_26_sequence_9_step_01(void)
{
    s32 index;

    D_801B0FD0 = 20;
    D_80182DF0 = 255;
    for (index = 14; index < 34; index++)
    {
        *(s16 *)((u8*)D_801AFBD0 + index * 20) = 0;
        D_80139988[index + 14].data = D_8011D538;
    }
    g_wmap_land_effect_26_sequence_9_timer = 32;
    g_wmap_land_effect_26_sequence_9_step++;
    wmap_land_effect_26_sequence_9_step_02();
}

/** @brief Update the actor effect and advance the sequence after its countdown. */
void wmap_land_effect_26_sequence_9_step_02(void)
{
    func_8006A9C4(D_800D94D0, D_801399F8, 0xE, 0x22, D_80182DF0, 0x1FF6, 0x200, 0x30, 2, 0x320, 0xB, 4);
    if (--g_wmap_land_effect_26_sequence_9_timer == 0)
    {
        g_wmap_land_effect_26_sequence_9_step++;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_26_sequence_9_step_03(void)
{
    g_wmap_land_effect_26_sequence_9_timer = 0x40;
    g_wmap_land_effect_26_sequence_9_step += 1;
    wmap_land_effect_26_sequence_9_step_04();
}

/** @brief Update the actor effect and advance the sequence after its countdown. */
void wmap_land_effect_26_sequence_9_step_04(void)
{
    func_8006A9C4(D_800D94D0, D_801399F8, 0xE, 0x22, D_80182DF0, 0x1FF6, 0x200, 0x30, 2, 0x320, 0xB, 4);
    D_80182DF0 -= 4;
    if (D_80182DF0 < 0)
    {
        D_80182DF0 = 0;
    }
    if (--g_wmap_land_effect_26_sequence_9_timer == 0)
    {
        g_wmap_land_effect_26_sequence_9_step++;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_26_sequence_9_end(void)
{
    g_wmap_land_effect_26_sequence_9_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 wmap_land_effect_26_run_sequence_8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_26_sequence_8_step = 1;
        g_wmap_land_effect_26_sequence_8_timer = 1;
    }

    if (g_wmap_land_effect_26_sequence_8_step < 0x6)
    {
        D_800D5220[g_wmap_land_effect_26_sequence_8_step]();
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
void wmap_land_effect_26_sequence_8_reset(void)
{
    g_wmap_land_effect_26_sequence_8_step = 1;
    g_wmap_land_effect_26_sequence_8_timer = 1;
}

/** @brief Initialize actor configuration and begin a 20-tick sequence step. */
void wmap_land_effect_26_sequence_8_step_01(void)
{
    D_801399BC = D_8011D538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.sequence = 2;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = -1;
    D_800D9370.target_shade = 0x81;
    D_800D9370.resource_index = 0;
    D_800D9370.shade = 0x90;
    g_wmap_land_effect_26_sequence_8_timer = 0x14;
    g_wmap_land_effect_26_sequence_8_step += 1;
    wmap_land_effect_26_sequence_8_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_26_sequence_8_step_02(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0xB, 0x4, 0);
    if (--g_wmap_land_effect_26_sequence_8_timer == 0)
    {
        g_wmap_land_effect_26_sequence_8_step += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_26_sequence_8_step_03(void)
{
    D_800D9370.shade_step = 8;
    D_800D9370.target_shade = 0;
    g_wmap_land_effect_26_sequence_8_timer = 0x10;
    g_wmap_land_effect_26_sequence_8_step += 1;
    wmap_land_effect_26_sequence_8_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_26_sequence_8_step_04(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0xB, 0x4, 0);
    if (--g_wmap_land_effect_26_sequence_8_timer == 0)
    {
        g_wmap_land_effect_26_sequence_8_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_26_sequence_8_end(void)
{
    g_wmap_land_effect_26_sequence_8_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 wmap_land_effect_26_run_sequence_10(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_26_sequence_10_step = 1;
        g_wmap_land_effect_26_sequence_10_timer = 1;
    }

    if (g_wmap_land_effect_26_sequence_10_step < 0x4)
    {
        D_800D5250[g_wmap_land_effect_26_sequence_10_step]();
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
void wmap_land_effect_26_sequence_10_reset(void)
{
    g_wmap_land_effect_26_sequence_10_step = 1;
    g_wmap_land_effect_26_sequence_10_timer = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_26_sequence_10_step_01(void)
{
    D_801399B4 = D_8011F538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 4;
    D_800D9344.target_shade = 0x80;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.shade = 0xFC;
    g_wmap_land_effect_26_sequence_10_timer = 0xAF;
    g_wmap_land_effect_26_sequence_10_step += 1;
    wmap_land_effect_26_sequence_10_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_26_sequence_10_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x12, 0xB, 0);
    if (--g_wmap_land_effect_26_sequence_10_timer == 0)
    {
        g_wmap_land_effect_26_sequence_10_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_26_sequence_10_end(void)
{
    g_wmap_land_effect_26_sequence_10_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 wmap_land_effect_26_run_sequence_11(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_26_sequence_11_step = 1;
        g_wmap_land_effect_26_sequence_11_timer = 1;
    }

    if (g_wmap_land_effect_26_sequence_11_step < 0x8)
    {
        D_800D5260[g_wmap_land_effect_26_sequence_11_step]();
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
void wmap_land_effect_26_sequence_11_reset(void)
{
    g_wmap_land_effect_26_sequence_11_step = 1;
    g_wmap_land_effect_26_sequence_11_timer = 1;
}

/** @brief Draw the effect, raise its value to at most 129, and update the countdown. */
void wmap_land_effect_26_sequence_11_step_02(void)
{
    s32 value;
    s32 remaining_ticks;

    func_800773B8(0x7C, 0xAA, 0xD);
    value = D_80182DF4 + 8;
    D_80182DF4 = value;
    if (value >= 0x82)
    {
        D_80182DF4 = 0x81;
    }
    remaining_ticks = g_wmap_land_effect_26_sequence_11_timer - 1;
    g_wmap_land_effect_26_sequence_11_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_26_sequence_11_step += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_26_sequence_11_step_03(void)
{
    g_wmap_land_effect_26_sequence_11_timer = 0x64;
    g_wmap_land_effect_26_sequence_11_step += 1;
    wmap_land_effect_26_sequence_11_step_04();
}

/** @brief Update the sequence effect and advance when its countdown expires. */
void wmap_land_effect_26_sequence_11_step_04(void)
{
    s32 remaining_ticks;

    func_800773B8(0x7C, 0xAA, 0xD);
    remaining_ticks = g_wmap_land_effect_26_sequence_11_timer - 1;
    g_wmap_land_effect_26_sequence_11_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_26_sequence_11_step += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_26_sequence_11_step_05(void)
{
    g_wmap_land_effect_26_sequence_11_timer = 0x10;
    g_wmap_land_effect_26_sequence_11_step += 1;
    wmap_land_effect_26_sequence_11_step_06();
}

/** @brief Draw the effect, reduce its value toward zero, and update the countdown. */
void wmap_land_effect_26_sequence_11_step_06(void)
{
    s32 value;
    s32 remaining_ticks;

    func_800773B8(0x7C, 0xAA, 0xD);
    value = D_80182DF4 - 8;
    D_80182DF4 = value;
    if (value < 0)
    {
        D_80182DF4 = 0;
    }
    remaining_ticks = g_wmap_land_effect_26_sequence_11_timer - 1;
    g_wmap_land_effect_26_sequence_11_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_26_sequence_11_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_26_sequence_11_end(void)
{
    g_wmap_land_effect_26_sequence_11_step += 1;
}
