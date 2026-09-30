#include "wmap_land_effect_32.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_step_sequence.h"

void wmap_land_effect_32_sequence_2_step_02(void);
void wmap_land_effect_32_sequence_10_step_02(void);
void wmap_land_effect_32_wait_idle_02(void);
void wmap_land_effect_32_step_03(void);
s32 wmap_land_effect_32_run_timeline(s32 arg0);
void wmap_land_effect_32_wait_idle_04(void);
void wmap_land_effect_32_end(void);
s32 wmap_land_effect_32_run_sequence_1(s32 arg0);
s32 wmap_land_effect_32_run_sequence_7(s32 arg0);
s32 wmap_land_effect_32_run_sequence_8(s32 arg0);
s32 wmap_land_effect_32_run_sequence_3(s32 arg0);
s32 wmap_land_effect_32_run_sequence_4(s32 arg0);
s32 wmap_land_effect_32_run_sequence_5(s32 arg0);
s32 wmap_land_effect_32_run_sequence_2(s32 arg0);
s32 wmap_land_effect_32_run_sequence_6(s32 arg0);
s32 wmap_land_effect_32_run_sequence_9(s32 arg0);
s32 wmap_land_effect_32_run_sequence_10(s32 arg0);
void wmap_land_effect_32_sequence_1_step_02(void);
void wmap_land_effect_32_sequence_1_step_04(void);
void wmap_land_effect_32_sequence_1_step_06(void);
void wmap_land_effect_32_sequence_2_step_04(void);
void wmap_land_effect_32_sequence_8_step_02(void);
void wmap_land_effect_32_sequence_9_step_02(void);
void wmap_land_effect_32_sequence_10_step_04(void);

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

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
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s32 field_10;
} WmapMotion;

/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

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
extern s32 D_801B24B4;
extern s32 D_801B0FD0;
extern s32 g_wmap_land_effect_32_sequence_2_timer;
extern u8 D_8011D538[];
extern u8* D_8011CF28;
extern s32 D_80182DE4;
extern s32 g_wmap_land_effect_32_sequence_3_timer;
extern void PushMatrix(void);
extern void PopMatrix(void);
extern u8* D_8011CF2C;
extern s32 D_80182DE8;
extern s32 g_wmap_land_effect_32_sequence_4_timer;
extern u8* D_8011CF30;
extern s32 D_80182DEC;
extern s32 g_wmap_land_effect_32_sequence_5_timer;
extern VECTOR D_8011CF60;
extern s32 D_80182DF0;
extern u8 D_800DCF18[];
extern s32 g_wmap_land_effect_32_sequence_6_timer;
extern void *D_8011CF1C;
extern SVECTOR g_wmap_camera_rotation;
extern s32 D_80182DF4;
extern s32 g_wmap_land_effect_32_sequence_7_timer;
extern s32 D_801B25D8;
extern s32 D_8013B264;
extern s32 D_8013B270;
extern s32 D_8013B278;
extern s32 D_8013B280;
extern s32 D_8013B284;
extern s32 g_wmap_land_effect_32_sequence_10_timer;
extern WmapMotion D_801B0B70[];
extern s32 rand(void);
extern s32 g_wmap_land_effect_32_timer;
extern void (*D_800D5280[])(void);
extern void wmap_land_effect_32_step_03(void);
extern void wmap_land_effect_32_end(void);
extern s32 g_wmap_land_effect_32_timeline_timer;
extern void (*D_800D5298[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 g_wmap_land_effect_32_sequence_1_timer;
extern void (*D_800D52D8[])(void);
extern u8* D_801399B4;
extern void wmap_land_effect_32_sequence_1_step_04(void);
extern s16 D_800D9366;
extern void (*D_800D52F8[])(void);
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern void (*D_800D5310[])(void);
extern void (*D_800D5328[])(void);
extern void (*D_800D5340[])(void);
extern void (*D_800D5358[])(void);
extern void (*D_800D5368[])(void);
extern s32 g_wmap_land_effect_32_sequence_8_timer;
extern void (*D_800D5378[])(void);
extern u8* D_801399BC;
extern void wmap_land_effect_32_sequence_8_step_02(void);
extern s32 g_wmap_land_effect_32_sequence_9_timer;
extern void (*D_800D5388[])(void);
extern u8* D_801399AC;
extern u8 D_8011F538[];
extern void (*D_800D5398[])(void);
extern WmapAnimationSlot D_80139FE8[];
extern u32 g_wmap_land_effect_32_sequence_2_step;

extern u32 g_wmap_land_effect_32_sequence_3_step;

extern u32 g_wmap_land_effect_32_sequence_4_step;

extern u32 g_wmap_land_effect_32_sequence_5_step;
extern u32 g_wmap_land_effect_32_sequence_6_step;
extern u32 g_wmap_land_effect_32_sequence_7_step;
extern u32 g_wmap_land_effect_32_sequence_10_step;
extern WmapConfigA D_800DB578[];
extern u32 g_wmap_land_effect_32_step;
extern u32 g_wmap_land_effect_32_timeline_step;
extern u32 g_wmap_land_effect_32_sequence_1_step;
extern u32 g_wmap_land_effect_32_sequence_8_step;
extern u32 g_wmap_land_effect_32_sequence_9_step;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2478;
extern VECTOR D_80139870;
extern VECTOR D_80139888;
extern VECTOR D_80139898;
extern VECTOR D_801B2660;

extern SVECTOR D_80139258;
extern SVECTOR D_8013B240;
extern SVECTOR D_8013B238;
extern SVECTOR D_801B2670;
extern SVECTOR D_801B24A8;
extern SVECTOR D_801B2678;

extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern WmapSlot14 D_801AFBD0[];

/** @brief Configure effect parameters and reset its animation resource slots. */
void wmap_land_effect_32_sequence_2_step_01(void)
{
    s32 i;

    D_801B0FD0 = 24;
    D_801B24B4 = 127;
    D_80139234 = 1;
    D_8013923C = 10;
    D_80139240 = 20;
    D_8013924C = 2;
    D_80139250 = 2;
    D_80139260 = 1500;
    D_80139264 = 100;
    D_80139268 = 19;
    D_8013926C = 0;
    D_80139284 = 1000;
    for (i = 0; i < 24; i++)
    {
        D_801AFBD0[i + D_80139264].field_00 = 0;
        D_80139988[i + 104].data = D_8011D538;
    }
    g_wmap_land_effect_32_sequence_2_timer = 40;
    g_wmap_land_effect_32_sequence_2_step++;
    wmap_land_effect_32_sequence_2_step_02();
}

/** @brief World-map animated element: advance phase, draw, and tick refcount. */
void wmap_land_effect_32_sequence_3_step_02(void)
{
    s32 *p = &D_80139888;
    u16 *q;
    s32 v;
    s32 t;
    s32 c;

    v = p[2] - 0x5DC;
    p[2] = v;
    if (v <= 0x9C3F)
    {
        p[2] = 0x9C40;
    }
    PushMatrix();
    q = &D_8013B240;
    wmap_set_model_transform(p, q);
    wmap_draw_model_default(D_8011CF28, 0, 0xC, 0x35, 0x7800, 1, D_80182DE4);
    t = D_80182DE4 + 0x10;
    D_80182DE4 = t;
    if (t >= 0x82)
    {
        D_80182DE4 = 0x81;
    }
    *(u16 *)((u8 *)q + 4) += 2;
    PopMatrix();
    c = g_wmap_land_effect_32_sequence_3_timer - 1;
    g_wmap_land_effect_32_sequence_3_timer = c;
    if (c == 0)
    {
        g_wmap_land_effect_32_sequence_3_step += 1;
    }
}

/** @brief World-map animated element: advance phase, draw while active, then tick refcount. */
void wmap_land_effect_32_sequence_3_step_04(void)
{
    s32 *p = &D_80139888;
    u16 *q;
    s32 v;
    s32 t;
    s32 c;

    v = p[2] - 0x5DC;
    p[2] = v;
    if (v <= 0x9C3F)
    {
        p[2] = 0x9C40;
    }
    PushMatrix();
    q = &D_8013B240;
    wmap_set_model_transform(p, q);
    if (D_80182DE4 != 0)
    {
        wmap_draw_model_default(D_8011CF28, 0, 0xC, 0x35, 0x7800, 1, D_80182DE4);
        t = D_80182DE4 - 2;
        D_80182DE4 = t;
        if (t < 0)
        {
            D_80182DE4 = 0;
        }
        *(u16 *)((u8 *)q + 4) += 2;
    }
    PopMatrix();
    c = g_wmap_land_effect_32_sequence_3_timer - 1;
    g_wmap_land_effect_32_sequence_3_timer = c;
    if (c == 0)
    {
        g_wmap_land_effect_32_sequence_3_step += 1;
    }
}

/** @brief World-map animated element: advance phase, draw, and tick refcount. */
void wmap_land_effect_32_sequence_4_step_02(void)
{
    s32 *p = &D_80139898;
    u16 *q;
    s32 v;
    s32 t;
    s32 c;

    v = p[2] - 0x5DC;
    p[2] = v;
    if (v <= 0x9C3F)
    {
        p[2] = 0x9C40;
    }
    PushMatrix();
    q = &D_801B2670;
    wmap_set_model_transform(p, q);
    wmap_draw_model_default(D_8011CF2C, 0, 0xC, 0x35, 0x7800, 1, D_80182DE8);
    t = D_80182DE8 + 0x10;
    D_80182DE8 = t;
    if (t >= 0x82)
    {
        D_80182DE8 = 0x81;
    }
    *(u16 *)((u8 *)q + 4) -= 3;
    PopMatrix();
    c = g_wmap_land_effect_32_sequence_4_timer - 1;
    g_wmap_land_effect_32_sequence_4_timer = c;
    if (c == 0)
    {
        g_wmap_land_effect_32_sequence_4_step += 1;
    }
}

/** @brief World-map animated element: advance phase, draw while active, then tick refcount. */
void wmap_land_effect_32_sequence_4_step_04(void)
{
    s32 *p = &D_80139898;
    u16 *q;
    s32 v;
    s32 t;
    s32 c;

    v = p[2] - 0x5DC;
    p[2] = v;
    if (v <= 0x9C3F)
    {
        p[2] = 0x9C40;
    }
    PushMatrix();
    q = &D_801B2670;
    wmap_set_model_transform(p, q);
    if (D_80182DE8 != 0)
    {
        wmap_draw_model_default(D_8011CF2C, 0, 0xC, 0x35, 0x7800, 1, D_80182DE8);
        t = D_80182DE8 - 2;
        D_80182DE8 = t;
        if (t < 0)
        {
            D_80182DE8 = 0;
        }
        *(u16 *)((u8 *)q + 4) -= 3;
    }
    PopMatrix();
    c = g_wmap_land_effect_32_sequence_4_timer - 1;
    g_wmap_land_effect_32_sequence_4_timer = c;
    if (c == 0)
    {
        g_wmap_land_effect_32_sequence_4_step += 1;
    }
}

/** @brief World-map animated element: advance phase, draw, and tick refcount. */
void wmap_land_effect_32_sequence_5_step_02(void)
{
    s32 *p = &D_801B2660;
    u16 *q;
    s32 v;
    s32 t;
    s32 c;

    v = p[2] - 0x5DC;
    p[2] = v;
    if (v <= 0x9C3F)
    {
        p[2] = 0x9C40;
    }
    PushMatrix();
    q = &D_801B2678;
    wmap_set_model_transform(p, q);
    wmap_draw_model_default(D_8011CF30, 0, 0xC, 0x35, 0x7800, 1, D_80182DEC);
    t = D_80182DEC + 0x10;
    D_80182DEC = t;
    if (t >= 0x82)
    {
        D_80182DEC = 0x81;
    }
    *(u16 *)((u8 *)q + 4) += 4;
    PopMatrix();
    c = g_wmap_land_effect_32_sequence_5_timer - 1;
    g_wmap_land_effect_32_sequence_5_timer = c;
    if (c == 0)
    {
        g_wmap_land_effect_32_sequence_5_step += 1;
    }
}

/** @brief World-map animated element: advance phase, draw while active, then tick refcount. */
void wmap_land_effect_32_sequence_5_step_04(void)
{
    s32 *p = &D_801B2660;
    u16 *q;
    s32 v;
    s32 t;
    s32 c;

    v = p[2] - 0x5DC;
    p[2] = v;
    if (v <= 0x9C3F)
    {
        p[2] = 0x9C40;
    }
    PushMatrix();
    q = &D_801B2678;
    wmap_set_model_transform(p, q);
    if (D_80182DEC != 0)
    {
        wmap_draw_model_default(D_8011CF30, 0, 0xC, 0x35, 0x7800, 1, D_80182DEC);
        t = D_80182DEC - 2;
        D_80182DEC = t;
        if (t < 0)
        {
            D_80182DEC = 0;
        }
        *(u16 *)((u8 *)q + 4) += 4;
    }
    PopMatrix();
    c = g_wmap_land_effect_32_sequence_5_timer - 1;
    g_wmap_land_effect_32_sequence_5_timer = c;
    if (c == 0)
    {
        g_wmap_land_effect_32_sequence_5_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_32_sequence_6_step_02(void)
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
        wmap_draw_model_default((s32)D_800DCF18, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DF0);
        D_80182DF0 -= 0x5;
        if (D_80182DF0 < 0)
        {
            D_80182DF0 = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_32_sequence_6_timer == 0)
    {
        g_wmap_land_effect_32_sequence_6_step += 1;
    }
}

/** @brief Draw and fade the transformed effect, then advance its countdown. */
void wmap_land_effect_32_sequence_7_step_02(void)
{
    MATRIX base;
    MATRIX effect;
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = D_801B2478.vz - 0x7D0;
    D_801B2478.vz = depth;
    if (depth < 0x1F40)
    {
        D_801B2478.vz = 0x1F40;
    }
    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base);
    TransMatrix(&base, &D_801B2478);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&D_801B24A8, &effect);
    TransMatrix(&effect, &D_8011CF60);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    if (D_80182DF4 != 0)
    {
        wmap_draw_model_default(D_8011CF1C, 0, 0xC, 0x35, 0x7800, 1, D_80182DF4);
        intensity = D_80182DF4 - 6;
        D_80182DF4 = intensity;
        if (intensity < 0)
        {
            D_80182DF4 = 0;
        }
    }
    PopMatrix();
    remaining = g_wmap_land_effect_32_sequence_7_timer - 1;
    g_wmap_land_effect_32_sequence_7_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_32_sequence_7_step += 1;
    }
}

/** @brief Initialize the effect actors and randomized motion angles. */
void wmap_land_effect_32_sequence_10_step_01(void)
{
    s32 i;
    WmapConfigA *actor;
    WmapMotion *motion;

    D_801B0FD0 = 40;
    D_801B25D8 = 256;
    D_80139234 = 48;
    D_8013923C = 10;
    D_80139240 = 60;
    D_8013924C = 2;
    D_80139250 = -1;
    D_80139260 = 8000;
    D_80139264 = 200;
    D_80139268 = 19;
    D_8013926C = 1;
    D_80139284 = 200;
    D_8013B264 = 54;
    D_8013B270 = 1;
    D_8013B278 = 30;
    D_8013B280 = 1;
    D_8013B284 = 3;
    for (i = 0; i < 40; i++)
    {
        actor = &D_800DB578[i];
        motion = &D_801B0B70[i];
        D_80139988[i + 204].data = D_8011D538;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_22 = 128;
        actor->field_24 = 128;
        actor->field_02 = 0;
        actor->field_26 = 0;
        actor->field_0E = D_8013926C;
        motion->state = 1;
        motion->angle = rand() & 0xFFF;
        motion->scale = 60;
        motion->field_0E = 50;
        motion->x = 0;
        motion->z = D_80139284;
    }
    g_wmap_land_effect_32_sequence_10_timer = 90;
    g_wmap_land_effect_32_sequence_10_step++;
    wmap_land_effect_32_sequence_10_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_32_run, D_800D5280, 0x6, g_wmap_land_effect_32_step, g_wmap_land_effect_32_timer)

WMAP_STEP_RESET(wmap_land_effect_32_reset, g_wmap_land_effect_32_step, g_wmap_land_effect_32_timer)

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_32_step_01(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_32_step += 1;
    wmap_land_effect_32_wait_idle_02();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_32_wait_idle_02, g_wmap_land_effect_32_step, wmap_land_effect_32_step_03)

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_32_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_32_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_32_step += 1;
    wmap_land_effect_32_wait_idle_04();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_32_wait_idle_04, g_wmap_land_effect_32_step, wmap_land_effect_32_end)

WMAP_STEP_ADVANCE(wmap_land_effect_32_end, g_wmap_land_effect_32_step)

WMAP_STEP_RUNNER(wmap_land_effect_32_run_timeline, D_800D5298, 0x10, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_32_timeline_reset, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer)

/** @brief Set color and flags, play sound 26, and register three sequence callbacks. */
void wmap_land_effect_32_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_start_map_tint(0x202020);
    wmap_play_sound(0x1A, 0x80);
    wmap_start_sequence(&wmap_land_effect_32_run_sequence_1);
    D_801ADAE0 = 1;
    wmap_start_sequence(&wmap_land_effect_32_run_sequence_8);
    wmap_start_sequence(&wmap_land_effect_32_run_sequence_7);
    g_wmap_land_effect_32_timeline_timer = 8;
    g_wmap_land_effect_32_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_32_timeline_wait_02, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_32_timeline_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_32_run_sequence_3);
    g_wmap_land_effect_32_timeline_timer = 0x4;
    g_wmap_land_effect_32_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_32_timeline_wait_04, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_32_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_32_run_sequence_4);
    g_wmap_land_effect_32_timeline_timer = 0x4;
    g_wmap_land_effect_32_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_32_timeline_wait_06, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_32_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_32_run_sequence_5);
    g_wmap_land_effect_32_timeline_timer = 0xF;
    g_wmap_land_effect_32_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_32_timeline_wait_08, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_32_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_32_run_sequence_2);
    g_wmap_land_effect_32_timeline_timer = 0x30;
    g_wmap_land_effect_32_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_32_timeline_wait_10, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer)

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_32_timeline_step_11(void)
{
    wmap_start_sequence(wmap_land_effect_32_run_sequence_6);
    wmap_start_sequence(wmap_land_effect_32_run_sequence_9);
    g_wmap_land_effect_32_timeline_timer = 0x42;
    g_wmap_land_effect_32_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_32_timeline_wait_12, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_32_timeline_step_13(void)
{
    wmap_start_sequence(wmap_land_effect_32_run_sequence_10);
    g_wmap_land_effect_32_timeline_timer = 0x83;
    g_wmap_land_effect_32_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_32_timeline_wait_14, g_wmap_land_effect_32_timeline_step, g_wmap_land_effect_32_timeline_timer)

void wmap_land_effect_32_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    g_wmap_land_effect_32_timeline_step += 1;
}

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_1, D_800D52D8, 0x8, g_wmap_land_effect_32_sequence_1_step,
                               g_wmap_land_effect_32_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_1_reset, g_wmap_land_effect_32_sequence_1_step, g_wmap_land_effect_32_sequence_1_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_32_sequence_1_step_01(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.sequence = 2;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 0x10;
    D_800D9344.target_shade = 0x81;
    D_800D9344.resource_index = 0;
    D_800D9344.shade = 1;
    g_wmap_land_effect_32_sequence_1_timer = 0x10;
    g_wmap_land_effect_32_sequence_1_step += 1;
    wmap_land_effect_32_sequence_1_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_32_sequence_1_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x13, 0xB, 0);
    if (--g_wmap_land_effect_32_sequence_1_timer == 0)
    {
        g_wmap_land_effect_32_sequence_1_step += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_32_sequence_1_step_03(void)
{
    g_wmap_land_effect_32_sequence_1_timer = 0x8;
    g_wmap_land_effect_32_sequence_1_step += 1;
    wmap_land_effect_32_sequence_1_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_32_sequence_1_step_04(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x13, 0xB, 0);
    if (--g_wmap_land_effect_32_sequence_1_timer == 0)
    {
        g_wmap_land_effect_32_sequence_1_step += 1;
    }
}

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void wmap_land_effect_32_sequence_1_step_05(void)
{
    D_800D9366 = 1;
    g_wmap_land_effect_32_sequence_1_timer = 0x10;
    g_wmap_land_effect_32_sequence_1_step += 1;
    wmap_land_effect_32_sequence_1_step_06();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_32_sequence_1_step_06(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x13, 0xB, 0);
    if (--g_wmap_land_effect_32_sequence_1_timer == 0)
    {
        g_wmap_land_effect_32_sequence_1_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_1_end, g_wmap_land_effect_32_sequence_1_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_2, D_800D52F8, 0x6, g_wmap_land_effect_32_sequence_2_step,
                               g_wmap_land_effect_32_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_2_reset, g_wmap_land_effect_32_sequence_2_step, g_wmap_land_effect_32_sequence_2_timer)

/** @brief World-map step handler: spawn a sub-object and expire the step counter. */
void wmap_land_effect_32_sequence_2_step_02(void)
{
    func_8006CFE4(D_800DA448, D_80139CC8, 0x18, D_801B24B4, D_801B24B4, 0);
    if (--g_wmap_land_effect_32_sequence_2_timer == 0)
    {
        g_wmap_land_effect_32_sequence_2_step += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_32_sequence_2_step_03(void)
{
    g_wmap_land_effect_32_sequence_2_timer = 0x10;
    g_wmap_land_effect_32_sequence_2_step += 1;
    wmap_land_effect_32_sequence_2_step_04();
}

/** @brief Draw and fade the effect, then advance when its countdown expires. */
void wmap_land_effect_32_sequence_2_step_04(void)
{
    s32 value;
    s32 remaining_ticks;

    if (D_801B24B4 != 0)
    {
        func_8006CFE4((s32)D_800DA448, (s32)D_80139CC8, 0x18, D_801B24B4, D_801B24B4, 0);
        value = D_801B24B4 - 8;
        D_801B24B4 = value;
        if (value < 0)
        {
            D_801B24B4 = 0;
        }
    }
    remaining_ticks = g_wmap_land_effect_32_sequence_2_timer - 1;
    g_wmap_land_effect_32_sequence_2_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_32_sequence_2_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_2_end, g_wmap_land_effect_32_sequence_2_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_3, D_800D5310, 0x6, g_wmap_land_effect_32_sequence_3_step,
                               g_wmap_land_effect_32_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_3_reset, g_wmap_land_effect_32_sequence_3_step, g_wmap_land_effect_32_sequence_3_timer)

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_32_sequence_3_step_01(void)
{
    D_8013B240 = D_80139258;
    D_80139888 = g_wmap_camera_translation;
    D_80182DE4 = 0x1;
    D_80139888.vz = 0xA410;
    g_wmap_land_effect_32_sequence_3_timer = 0x42;
    g_wmap_land_effect_32_sequence_3_step += 1;
    wmap_land_effect_32_sequence_3_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_32_sequence_3_step_03(void)
{
    g_wmap_land_effect_32_sequence_3_timer = 0x40;
    g_wmap_land_effect_32_sequence_3_step += 1;
    wmap_land_effect_32_sequence_3_step_04();
}

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_3_end, g_wmap_land_effect_32_sequence_3_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_4, D_800D5328, 0x6, g_wmap_land_effect_32_sequence_4_step,
                               g_wmap_land_effect_32_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_4_reset, g_wmap_land_effect_32_sequence_4_step, g_wmap_land_effect_32_sequence_4_timer)

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_32_sequence_4_step_01(void)
{
    D_801B2670 = D_80139258;
    D_80139898 = g_wmap_camera_translation;
    D_80182DE8 = 0x1;
    D_80139898.vz = 0xA410;
    g_wmap_land_effect_32_sequence_4_timer = 0x3E;
    g_wmap_land_effect_32_sequence_4_step += 1;
    wmap_land_effect_32_sequence_4_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_32_sequence_4_step_03(void)
{
    g_wmap_land_effect_32_sequence_4_timer = 0x40;
    g_wmap_land_effect_32_sequence_4_step += 1;
    wmap_land_effect_32_sequence_4_step_04();
}

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_4_end, g_wmap_land_effect_32_sequence_4_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_5, D_800D5340, 0x6, g_wmap_land_effect_32_sequence_5_step,
                               g_wmap_land_effect_32_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_5_reset, g_wmap_land_effect_32_sequence_5_step, g_wmap_land_effect_32_sequence_5_timer)

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_32_sequence_5_step_01(void)
{
    D_801B2678 = D_80139258;
    D_801B2660 = g_wmap_camera_translation;
    D_80182DEC = 0x1;
    D_801B2660.vz = 0xA410;
    g_wmap_land_effect_32_sequence_5_timer = 0x3A;
    g_wmap_land_effect_32_sequence_5_step += 1;
    wmap_land_effect_32_sequence_5_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_32_sequence_5_step_03(void)
{
    g_wmap_land_effect_32_sequence_5_timer = 0x40;
    g_wmap_land_effect_32_sequence_5_step += 1;
    wmap_land_effect_32_sequence_5_step_04();
}

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_5_end, g_wmap_land_effect_32_sequence_5_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_6, D_800D5358, 0x4, g_wmap_land_effect_32_sequence_6_step,
                               g_wmap_land_effect_32_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_6_reset, g_wmap_land_effect_32_sequence_6_step, g_wmap_land_effect_32_sequence_6_timer)

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_32_sequence_6_step_01(void)
{
    D_8013B238 = D_80139258;
    D_80139870 = g_wmap_camera_translation;
    D_80182DF0 = 0x80;
    D_80139870.vz = 0xAFC8;
    g_wmap_land_effect_32_sequence_6_timer = 0x28;
    g_wmap_land_effect_32_sequence_6_step += 1;
    wmap_land_effect_32_sequence_6_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_6_end, g_wmap_land_effect_32_sequence_6_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_7, D_800D5368, 0x4, g_wmap_land_effect_32_sequence_7_step,
                               g_wmap_land_effect_32_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_7_reset, g_wmap_land_effect_32_sequence_7_step, g_wmap_land_effect_32_sequence_7_timer)

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_32_sequence_7_step_01(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DF4 = 0x80;
    D_801B2478.vz = 0xAFC8;
    g_wmap_land_effect_32_sequence_7_timer = 0x28;
    g_wmap_land_effect_32_sequence_7_step += 1;
    wmap_land_effect_32_sequence_7_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_7_end, g_wmap_land_effect_32_sequence_7_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_8, D_800D5378, 0x4, g_wmap_land_effect_32_sequence_8_step,
                               g_wmap_land_effect_32_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_8_reset, g_wmap_land_effect_32_sequence_8_step, g_wmap_land_effect_32_sequence_8_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_32_sequence_8_step_01(void)
{
    D_801399BC = D_8011D538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.sequence = 3;
    D_800D9370.previous_sequence = -1;
    D_800D9370.resource_index = 0;
    D_800D9370.shade_step = 0;
    D_800D9370.target_shade = 0x81;
    D_800D9370.shade = 0x81;
    g_wmap_land_effect_32_sequence_8_timer = 0x4D;
    g_wmap_land_effect_32_sequence_8_step += 1;
    wmap_land_effect_32_sequence_8_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_32_sequence_8_step_02(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x13, 0xB, 0);
    if (--g_wmap_land_effect_32_sequence_8_timer == 0)
    {
        g_wmap_land_effect_32_sequence_8_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_8_end, g_wmap_land_effect_32_sequence_8_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_32_run_sequence_9, D_800D5388, 0x4, g_wmap_land_effect_32_sequence_9_step,
                               g_wmap_land_effect_32_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_9_reset, g_wmap_land_effect_32_sequence_9_step, g_wmap_land_effect_32_sequence_9_timer)

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void wmap_land_effect_32_sequence_9_step_01(void)
{
    D_801399AC = D_8011F538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.shade_step = 0;
    D_800D9318.target_shade = 0x80;
    D_800D9318.shade = 0x80;
    g_wmap_land_effect_32_sequence_9_timer = 0xC8;
    g_wmap_land_effect_32_sequence_9_step += 1;
    wmap_land_effect_32_sequence_9_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_32_sequence_9_step_02(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0x14, 0xB, 0);
    if (--g_wmap_land_effect_32_sequence_9_timer == 0)
    {
        g_wmap_land_effect_32_sequence_9_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_9_end, g_wmap_land_effect_32_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_32_run_sequence_10, D_800D5398, 0x6, g_wmap_land_effect_32_sequence_10_step, g_wmap_land_effect_32_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_32_sequence_10_reset, g_wmap_land_effect_32_sequence_10_step, g_wmap_land_effect_32_sequence_10_timer)

/** @brief Draw the sequence effect and advance when its countdown expires. */
void wmap_land_effect_32_sequence_10_step_02(void)
{
    s32 value;

    func_8006D014((s32)D_800DB578, (s32)D_80139FE8, 0x28, 0, D_801B25D8, 8, 1);
    value = g_wmap_land_effect_32_sequence_10_timer - 1;
    g_wmap_land_effect_32_sequence_10_timer = value;
    if (value == 0)
    {
        g_wmap_land_effect_32_sequence_10_step += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_32_sequence_10_step_03(void)
{
    g_wmap_land_effect_32_sequence_10_timer = 0x10;
    g_wmap_land_effect_32_sequence_10_step += 1;
    wmap_land_effect_32_sequence_10_step_04();
}

/** @brief Draw and fade the effect, then advance when its countdown expires. */
void wmap_land_effect_32_sequence_10_step_04(void)
{
    s32 value;
    s32 remaining_ticks;

    if (D_801B25D8 != 0)
    {
        func_8006D014((s32)D_800DB578, (s32)D_80139FE8, 0x28, 1, D_801B25D8, 8, 1);
        value = D_801B25D8 - 8;
        D_801B25D8 = value;
        if (value < 0)
        {
            D_801B25D8 = 0;
        }
    }
    remaining_ticks = g_wmap_land_effect_32_sequence_10_timer - 1;
    g_wmap_land_effect_32_sequence_10_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_32_sequence_10_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_32_sequence_10_end, g_wmap_land_effect_32_sequence_10_step)
