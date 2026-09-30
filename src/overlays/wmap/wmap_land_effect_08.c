#include "wmap_main.h"
#include "wmap_land_effect_08.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_sprite_render.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_effect_primitives.h"

void wmap_land_effect_08_sequence_6_step_02(void);
void wmap_land_effect_08_sequence_8_step_02(void);
void wmap_land_effect_08_sequence_9_step_02(void);
void wmap_land_effect_08_wait_idle_02(void);
void wmap_land_effect_08_step_03(void);
s32 wmap_land_effect_08_run_timeline(s32 arg0);
void wmap_land_effect_08_wait_idle_04(void);
void wmap_land_effect_08_end(void);
s32 wmap_land_effect_08_run_sequence_7(s32 arg0);
s32 wmap_land_effect_08_run_sequence_8(s32 arg0);
s32 wmap_land_effect_08_run_sequence_3(s32 arg0);
s32 wmap_land_effect_08_run_sequence_1(s32 arg0);
s32 wmap_land_effect_08_run_sequence_5(s32 arg0);
s32 wmap_land_effect_08_run_sequence_6(s32 arg0);
s32 wmap_land_effect_08_run_sequence_9(s32 arg0);
s32 wmap_land_effect_08_run_sequence_4(s32 arg0);
s32 wmap_land_effect_08_run_sequence_2(s32 arg0);
s32 wmap_land_effect_08_run_sequence_10(s32 arg0);
void wmap_land_effect_08_sequence_1_step_02(void);
void wmap_land_effect_08_sequence_1_step_04(void);
void wmap_land_effect_08_sequence_2_step_02(void);
void wmap_land_effect_08_sequence_6_step_04(void);
void wmap_land_effect_08_sequence_6_step_06(void);
void wmap_land_effect_08_sequence_7_step_02(void);
void wmap_land_effect_08_sequence_7_step_04(void);
void wmap_land_effect_08_sequence_9_step_04(void);
void wmap_land_effect_08_sequence_10_step_02(void);

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

/** @brief World-map orbiting star: polar position, spin angle, and radius. */
typedef struct
{
    s16 unk00;
    s16 angle;
    u16 delta;
    s16 unk06;
    s32 radius;
    s16 unk0C;
    u16 unk0E;
    s16 unk10;
    s16 unk12;
} WmapStar;

typedef struct
{
    u32 value;
    u8 unknown_4[36];
} WmapValueRecord;

extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 g_wmap_land_effect_08_sequence_3_timer;
extern s32 D_80182DEC;
extern u8* D_8011CF1C;
extern s32 g_wmap_land_effect_08_sequence_4_timer;
extern u8* D_8011CF24;
extern s32 D_801B2468;
extern s32 g_wmap_land_effect_08_sequence_5_timer;
extern s32 D_80182DF0;
extern s32 D_800DCEA8;
extern s32 D_800D9150;
extern s32 g_wmap_land_effect_08_sequence_6_timer;
extern u8 D_8011D538[];
extern s32 g_wmap_land_effect_08_sequence_8_timer;
extern s32 rand(void);
extern s32 ccos(s32);
extern s32 D_801B0FD0;
extern s32 g_wmap_land_effect_08_sequence_9_timer;
extern s32 g_wmap_land_effect_08_timer;
extern void (*D_800D6150[])(void);
extern void wmap_land_effect_08_step_03(void);
extern void wmap_land_effect_08_end(void);
extern s32 g_wmap_land_effect_08_timeline_timer;
extern void (*D_800D6168[])(void);
extern s32 D_8013B208;
extern s32 D_80139244;
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 g_wmap_land_effect_08_sequence_1_timer;
extern void (*D_800D61C8[])(void);
extern u8* D_801399AC;
extern void wmap_land_effect_08_sequence_1_step_02(void);
extern void wmap_land_effect_08_sequence_1_step_04(void);
extern s32 g_wmap_land_effect_08_sequence_2_timer;
extern void (*D_800D61E0[])(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern void wmap_land_effect_08_sequence_2_step_02(void);
extern void (*D_800D61F0[])(void);
extern void (*D_800D6200[])(void);
extern void (*D_800D6210[])(void);
extern s32 D_801B24B0;
extern void (*D_800D6228[])(void);
extern s32 g_wmap_land_effect_08_sequence_7_timer;
extern void (*D_800D6248[])(void);
extern void (*D_800D6260[])(void);
extern void (*D_800D6278[])(void);
extern u8 D_800DB578[];
extern WmapAnimationSlot D_80139FE8[];
extern void wmap_land_effect_08_sequence_9_step_04(void);
extern s32 g_wmap_land_effect_08_sequence_10_timer;
extern void (*D_800D6290[])(void);
extern void wmap_land_effect_08_sequence_10_step_02(void);

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

extern u32 g_wmap_land_effect_08_sequence_3_step;
extern u32 g_wmap_land_effect_08_sequence_4_step;
extern u32 g_wmap_land_effect_08_sequence_5_step;
extern s32 D_80121538;
extern u32 g_wmap_land_effect_08_sequence_6_step;
extern u32 g_wmap_land_effect_08_sequence_8_step;
extern u32 g_wmap_land_effect_08_sequence_9_step;
extern u32 g_wmap_land_effect_08_step;
extern u32 g_wmap_land_effect_08_timeline_step;
extern u32 g_wmap_land_effect_08_sequence_1_step;
extern u32 g_wmap_land_effect_08_sequence_2_step;
extern u32 g_wmap_land_effect_08_sequence_7_step;
extern u8* D_801399BC;
extern u32 g_wmap_land_effect_08_sequence_10_step;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;

extern SVECTOR D_80139258;
extern SVECTOR D_801B2498;
extern SVECTOR D_801B24A0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B24A8;

extern WmapSpriteActor D_800D9268[];
extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* D_80139280;

extern WmapStar D_801AFBD0[];

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_08_sequence_3_step_02(void)
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
    if (--g_wmap_land_effect_08_sequence_3_timer == 0)
    {
        g_wmap_land_effect_08_sequence_3_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_08_sequence_4_step_02(void)
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
    if (--g_wmap_land_effect_08_sequence_4_timer == 0)
    {
        g_wmap_land_effect_08_sequence_4_step += 1;
    }
}

/**
 * @brief World-map step handler: draw two overlaid actor sprites within a matrix push,
 *        ramp the shared size up to a cap, then countdown-advance the step.
 */
void wmap_land_effect_08_sequence_5_step_02(void)
{
    s32 value;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(D_8011CF24, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    D_801B2490.vz += 0x20;
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(D_8011CF24, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    D_801B2498.vz += 0xC;
    PopMatrix();
    value = D_801B2468 + 2;
    D_801B2468 = value;
    if (value >= 0x41)
    {
        D_801B2468 = 0x40;
    }
    if (--g_wmap_land_effect_08_sequence_5_timer == 0)
    {
        g_wmap_land_effect_08_sequence_5_step += 1;
    }
}

/**
 * @brief World-map step handler: draw two frames of the animated actor, scroll each
 *        sub-field, decay the shared frame index with a floor, then advance the step.
 */
void wmap_land_effect_08_sequence_5_step_04(void)
{
    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(D_8011CF24, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    D_801B2490.vz += 0x20;
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(D_8011CF24, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    D_801B2498.vz += 0xC;
    PopMatrix();
    D_801B2468 -= 4;
    if (D_801B2468 < 0)
    {
        D_801B2468 = 0;
    }
    if (--g_wmap_land_effect_08_sequence_5_timer == 0)
    {
        g_wmap_land_effect_08_sequence_5_step += 1;
    }
}

/**
 * @brief Reset a range of world-map actor slots and kick the next handler.
 * @note Clears three parallel slot arrays for indices [0x64, 0x6C), seeds each
 *       actor's default fields, then advances the shared step counters.
 */
void wmap_land_effect_08_sequence_6_step_01(void)
{
    s32 i;
    u8* pa;
    u8* pb;

    D_80182DF0 = 1;
    D_800DCEA8 = 1;
    for (i = 0x64; i < 0x6C; i++)
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
    g_wmap_land_effect_08_sequence_6_timer = 0x10;
    g_wmap_land_effect_08_sequence_6_step += 1;
    wmap_land_effect_08_sequence_6_step_02();
}

/** @brief Initialize randomized actors along a cosine depth curve. */
void wmap_land_effect_08_sequence_8_step_01(void)
{
    s32 i;
    WmapConfigA *actor;
    WmapMotion *motion;
    s32 field_value;

    i = 150;
    for (; i < 180; i++)
    {
        actor = &D_800D9268[i];
        D_80139988[i].data = D_8011D538;
        field_value = 1;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_26 = 2;
        actor->field_02 = 0;
        actor->field_0E = field_value;
        actor->field_22 = 129;
        actor->field_24 = field_value;
        motion = &D_801AFBD0[i];
        motion->angle = rand() & 4095;
        motion->field_0E = (i - 150) * 4;
        motion->z = ccos(2048 - (((i - 150) * 1024) / 30)) * 120;
        motion->x = ((rand() * 80) >> 15) + 40;
    }
    g_wmap_land_effect_08_sequence_8_timer = 64;
    g_wmap_land_effect_08_sequence_8_step++;
    wmap_land_effect_08_sequence_8_step_02();
}

/** @brief Project and draw the world-map star field, spinning each entry each frame. */
void wmap_land_effect_08_sequence_8_step_02(void)
{
    SVECTOR position;
    s32 screen;
    s32 i;

    for (i = 0x96; i < 0xB4; i++)
    {
        position.vx = ((D_801AFBD0[i].radius >> 6) * (ccos(D_801AFBD0[i].angle) >> 6)) >> 0xC;
        position.vy = ((D_801AFBD0[i].radius >> 6) * (csin(D_801AFBD0[i].angle) >> 6)) >> 0xC;
        position.vz = D_801AFBD0[i].unk0E;
        gte_ldv0(&position);
        gte_rtps();
        wmap_step_actor_animation(&D_800D9268[i], &D_80139988[i]);
        gte_stsxy(&screen);
        if (D_801AFBD0[i].angle != 0)
        {
            wmap_draw_actor_sprite(&D_800D9268[i], screen, 0xF, 4, 0);
        }
        else
        {
            wmap_draw_actor_sprite(&D_800D9268[i], screen, 0xF, 0, 0);
        }
        D_801AFBD0[i].angle = ((u16)D_801AFBD0[i].angle + D_801AFBD0[i].delta) & 0xFFF;
    }
    if (--g_wmap_land_effect_08_sequence_8_timer == 0)
    {
        g_wmap_land_effect_08_sequence_8_step += 1;
    }
}

/** @brief Project and draw the world-map star field, spinning each entry each frame. */
void wmap_land_effect_08_sequence_8_step_04(void)
{
    SVECTOR position;
    s32 screen;
    s32 i;

    for (i = 0x96; i < 0xB4; i++)
    {
        position.vx = ((D_801AFBD0[i].radius >> 6) * (ccos(D_801AFBD0[i].angle) >> 6)) >> 0xC;
        position.vy = ((D_801AFBD0[i].radius >> 6) * (csin(D_801AFBD0[i].angle) >> 6)) >> 0xC;
        position.vz = D_801AFBD0[i].unk0E;
        gte_ldv0(&position);
        gte_rtps();
        wmap_step_actor_animation(&D_800D9268[i], &D_80139988[i]);
        gte_stsxy(&screen);
        if (D_801AFBD0[i].angle != 0)
        {
            wmap_draw_actor_sprite(&D_800D9268[i], screen, 0xF, 4, 0);
        }
        else
        {
            wmap_draw_actor_sprite(&D_800D9268[i], screen, 0xF, 0, 0);
        }
        D_801AFBD0[i].angle = ((u16)D_801AFBD0[i].angle + D_801AFBD0[i].delta) & 0xFFF;
    }
    if (--g_wmap_land_effect_08_sequence_8_timer == 0)
    {
        g_wmap_land_effect_08_sequence_8_step += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_08_sequence_9_step_01(void)
{
    s32 i;

    D_801B0FD0 = 10;
    D_80139280[0xB] = 1;
    D_80139280[0xC] = 4;
    D_80139280[0xD] = 512;
    D_80139280[0xE] = 2;
    D_80139280[0xF] = 2;
    D_80139280[0x10] = 96;
    D_80139280[0x11] = 200;
    D_80139280[0x12] = 15;
    D_80139280[0x13] = 0;
    D_80139280[0x14] = 12000;
    for (i = 0; i < 10; i++)
    {
        D_801AFBD0[i + D_80139280[0x11]].unk00 = 0;
        D_80139988[i + 204].data = D_8011D538;
    }
    g_wmap_land_effect_08_sequence_9_timer = 20;
    g_wmap_land_effect_08_sequence_9_step++;
    wmap_land_effect_08_sequence_9_step_02();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_08_run(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_08_step = 1;
        g_wmap_land_effect_08_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_08_step < 0x6)
    {
        D_800D6150[g_wmap_land_effect_08_step]();
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
void wmap_land_effect_08_reset(void)
{
    g_wmap_land_effect_08_step = 1;
    g_wmap_land_effect_08_timer = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_08_step_01(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_08_step += 1;
    wmap_land_effect_08_wait_idle_02();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_08_wait_idle_02(void)
{
    if (g_wmap_sequence_busy == 0)
    {
        g_wmap_land_effect_08_step += 1;
        wmap_land_effect_08_step_03();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_08_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_08_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_08_step += 1;
    wmap_land_effect_08_wait_idle_04();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_08_wait_idle_04(void)
{
    if (g_wmap_sequence_busy == 0)
    {
        g_wmap_land_effect_08_step += 1;
        wmap_land_effect_08_end();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_08_end(void)
{
    g_wmap_land_effect_08_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_08_run_timeline(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_08_timeline_step = 1;
        g_wmap_land_effect_08_timeline_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_08_timeline_step < 0x18)
    {
        D_800D6168[g_wmap_land_effect_08_timeline_step]();
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
void wmap_land_effect_08_timeline_reset(void)
{
    g_wmap_land_effect_08_timeline_step = 1;
    g_wmap_land_effect_08_timeline_timer = 1;
}

/**
 * @brief Set up a tinted world-map sub-scene and register its handler.
 */
void wmap_land_effect_08_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 9;
    wmap_play_sound(0x2A, 0x80);
    wmap_start_sequence(wmap_land_effect_08_run_sequence_7);
    g_wmap_land_effect_08_timeline_timer = 0x14;
    g_wmap_land_effect_08_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_08_timeline_wait_02(void)
{
    if (--g_wmap_land_effect_08_timeline_timer == 0)
    {
        g_wmap_land_effect_08_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_08_timeline_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_08_run_sequence_8);
    g_wmap_land_effect_08_timeline_timer = 0x14;
    g_wmap_land_effect_08_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_08_timeline_wait_04(void)
{
    if (--g_wmap_land_effect_08_timeline_timer == 0)
    {
        g_wmap_land_effect_08_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_08_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_08_run_sequence_3);
    g_wmap_land_effect_08_timeline_timer = 0x2;
    g_wmap_land_effect_08_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_08_timeline_wait_06(void)
{
    if (--g_wmap_land_effect_08_timeline_timer == 0)
    {
        g_wmap_land_effect_08_timeline_step += 1;
    }
}

/** @brief Set world-map color and flags, register a callback, and begin a 20-tick delay. */
void wmap_land_effect_08_timeline_step_07(void)
{
    wmap_start_map_tint(0x561030);
    wmap_start_sequence(&wmap_land_effect_08_run_sequence_1);
    D_80139244 = 1;
    g_wmap_backdrop_target_level = 4;
    D_801ADAE0 = 1;
    g_wmap_land_effect_08_timeline_timer = 0x14;
    g_wmap_land_effect_08_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_08_timeline_wait_08(void)
{
    if (--g_wmap_land_effect_08_timeline_timer == 0)
    {
        g_wmap_land_effect_08_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_08_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_08_run_sequence_5);
    g_wmap_land_effect_08_timeline_timer = 0x14;
    g_wmap_land_effect_08_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_08_timeline_wait_10(void)
{
    if (--g_wmap_land_effect_08_timeline_timer == 0)
    {
        g_wmap_land_effect_08_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_08_timeline_step_11(void)
{
    wmap_start_sequence(wmap_land_effect_08_run_sequence_6);
    g_wmap_land_effect_08_timeline_timer = 0x28;
    g_wmap_land_effect_08_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_08_timeline_wait_12(void)
{
    if (--g_wmap_land_effect_08_timeline_timer == 0)
    {
        g_wmap_land_effect_08_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_08_timeline_step_13(void)
{
    wmap_start_sequence(wmap_land_effect_08_run_sequence_9);
    g_wmap_land_effect_08_timeline_timer = 0x14;
    g_wmap_land_effect_08_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_08_timeline_wait_14(void)
{
    if (--g_wmap_land_effect_08_timeline_timer == 0)
    {
        g_wmap_land_effect_08_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_08_timeline_step_15(void)
{
    wmap_start_sequence(wmap_land_effect_08_run_sequence_4);
    g_wmap_land_effect_08_timeline_timer = 0x2;
    g_wmap_land_effect_08_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_08_timeline_wait_16(void)
{
    if (--g_wmap_land_effect_08_timeline_timer == 0)
    {
        g_wmap_land_effect_08_timeline_step += 1;
    }
}

/** @brief Set the drawing color and world-map values, then start a two-tick delay. */
void wmap_land_effect_08_timeline_step_17(void)
{
    g_wmap_backdrop_target_level = 8;
    wmap_start_map_tint(0x562056);
    D_80139244 = 0;
    g_wmap_land_effect_08_timeline_timer = 2;
    g_wmap_land_effect_08_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_08_timeline_wait_18(void)
{
    if (--g_wmap_land_effect_08_timeline_timer == 0)
    {
        g_wmap_land_effect_08_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_08_timeline_step_19(void)
{
    wmap_start_sequence(wmap_land_effect_08_run_sequence_2);
    g_wmap_land_effect_08_timeline_timer = 0x60;
    g_wmap_land_effect_08_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_08_timeline_wait_20(void)
{
    if (--g_wmap_land_effect_08_timeline_timer == 0)
    {
        g_wmap_land_effect_08_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_08_timeline_step_21(void)
{
    wmap_start_sequence(wmap_land_effect_08_run_sequence_10);
    g_wmap_land_effect_08_timeline_timer = 0x34;
    g_wmap_land_effect_08_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_08_timeline_wait_22(void)
{
    if (--g_wmap_land_effect_08_timeline_timer == 0)
    {
        g_wmap_land_effect_08_timeline_step += 1;
    }
}

void wmap_land_effect_08_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    g_wmap_land_effect_08_timeline_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_08_run_sequence_1(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_08_sequence_1_step = 1;
        g_wmap_land_effect_08_sequence_1_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_08_sequence_1_step < 0x6)
    {
        D_800D61C8[g_wmap_land_effect_08_sequence_1_step]();
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
void wmap_land_effect_08_sequence_1_reset(void)
{
    g_wmap_land_effect_08_sequence_1_step = 1;
    g_wmap_land_effect_08_sequence_1_timer = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_08_sequence_1_step_01(void)
{
    D_801399AC = D_8011D538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.sequence = 2;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 8;
    D_800D9318.resource_index = 0;
    D_800D9318.target_shade = 0x81;
    D_800D9318.shade = 0x81;
    g_wmap_land_effect_08_sequence_1_timer = 0x60;
    g_wmap_land_effect_08_sequence_1_step += 1;
    wmap_land_effect_08_sequence_1_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_08_sequence_1_step_02(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0xF, 0x2, 0);
    if (--g_wmap_land_effect_08_sequence_1_timer == 0)
    {
        g_wmap_land_effect_08_sequence_1_step += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_08_sequence_1_step_03(void)
{
    D_800D9318.target_shade = 0;
    D_800D9318.shade_step = 8;
    g_wmap_land_effect_08_sequence_1_timer = 0x10;
    g_wmap_land_effect_08_sequence_1_step += 1;
    wmap_land_effect_08_sequence_1_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_08_sequence_1_step_04(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0xF, 0x2, 0);
    if (--g_wmap_land_effect_08_sequence_1_timer == 0)
    {
        g_wmap_land_effect_08_sequence_1_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_08_sequence_1_end(void)
{
    g_wmap_land_effect_08_sequence_1_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_08_run_sequence_2(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_08_sequence_2_step = 1;
        g_wmap_land_effect_08_sequence_2_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_08_sequence_2_step < 0x4)
    {
        D_800D61E0[g_wmap_land_effect_08_sequence_2_step]();
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
void wmap_land_effect_08_sequence_2_reset(void)
{
    g_wmap_land_effect_08_sequence_2_step = 1;
    g_wmap_land_effect_08_sequence_2_timer = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_08_sequence_2_step_01(void)
{
    D_801399B4 = D_8011F538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 8;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0x80;
    g_wmap_land_effect_08_sequence_2_timer = 0x96;
    g_wmap_land_effect_08_sequence_2_step += 1;
    wmap_land_effect_08_sequence_2_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_08_sequence_2_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x17, 0x8, 0);
    if (--g_wmap_land_effect_08_sequence_2_timer == 0)
    {
        g_wmap_land_effect_08_sequence_2_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_08_sequence_2_end(void)
{
    g_wmap_land_effect_08_sequence_2_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_08_run_sequence_3(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_08_sequence_3_step = 1;
        g_wmap_land_effect_08_sequence_3_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_08_sequence_3_step < 0x4)
    {
        D_800D61F0[g_wmap_land_effect_08_sequence_3_step]();
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
void wmap_land_effect_08_sequence_3_reset(void)
{
    g_wmap_land_effect_08_sequence_3_step = 1;
    g_wmap_land_effect_08_sequence_3_timer = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_08_sequence_3_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    g_wmap_land_effect_08_sequence_3_timer = 0x40;
    g_wmap_land_effect_08_sequence_3_step += 1;
    wmap_land_effect_08_sequence_3_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_08_sequence_3_end(void)
{
    g_wmap_land_effect_08_sequence_3_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_08_run_sequence_4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_08_sequence_4_step = 1;
        g_wmap_land_effect_08_sequence_4_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_08_sequence_4_step < 0x4)
    {
        D_800D6200[g_wmap_land_effect_08_sequence_4_step]();
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
void wmap_land_effect_08_sequence_4_reset(void)
{
    g_wmap_land_effect_08_sequence_4_step = 1;
    g_wmap_land_effect_08_sequence_4_timer = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_08_sequence_4_step_01(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    g_wmap_land_effect_08_sequence_4_timer = 0x80;
    g_wmap_land_effect_08_sequence_4_step += 1;
    wmap_land_effect_08_sequence_4_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_08_sequence_4_end(void)
{
    g_wmap_land_effect_08_sequence_4_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_08_run_sequence_5(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_08_sequence_5_step = 1;
        g_wmap_land_effect_08_sequence_5_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_08_sequence_5_step < 0x6)
    {
        D_800D6210[g_wmap_land_effect_08_sequence_5_step]();
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
void wmap_land_effect_08_sequence_5_reset(void)
{
    g_wmap_land_effect_08_sequence_5_step = 1;
    g_wmap_land_effect_08_sequence_5_timer = 1;
}

/** @brief World-map step: reset counters and advance to the next handler. */
void wmap_land_effect_08_sequence_5_step_01(void)
{
    D_801B24B0 = 0;
    D_801B2468 = 1;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    D_801B2498.vx = 0;
    D_801B2498.vy = 0;
    D_801B2498.vz = 0;
    g_wmap_land_effect_08_sequence_5_timer = 0x40;
    g_wmap_land_effect_08_sequence_5_step += 1;
    wmap_land_effect_08_sequence_5_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_08_sequence_5_step_03(void)
{
    g_wmap_land_effect_08_sequence_5_timer = 0x20;
    g_wmap_land_effect_08_sequence_5_step += 1;
    wmap_land_effect_08_sequence_5_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_08_sequence_5_end(void)
{
    g_wmap_land_effect_08_sequence_5_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_08_run_sequence_6(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_08_sequence_6_step = 1;
        g_wmap_land_effect_08_sequence_6_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_08_sequence_6_step < 0x8)
    {
        D_800D6228[g_wmap_land_effect_08_sequence_6_step]();
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
void wmap_land_effect_08_sequence_6_reset(void)
{
    g_wmap_land_effect_08_sequence_6_step = 1;
    g_wmap_land_effect_08_sequence_6_timer = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void wmap_land_effect_08_sequence_6_step_02(void)
{
    s32 remaining;

    func_8006B328(0x64, 0x6C, 4, -1, -1, -4, 0, 8, -0x6E, 0xF0, -0x6E, 0xF0, 1, 0x7F, 1, 4, 0);
    D_80182DF0 += 8;
    remaining = g_wmap_land_effect_08_sequence_6_timer - 1;
    g_wmap_land_effect_08_sequence_6_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_08_sequence_6_step += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_08_sequence_6_step_03(void)
{
    g_wmap_land_effect_08_sequence_6_timer = 0x20;
    g_wmap_land_effect_08_sequence_6_step += 1;
    wmap_land_effect_08_sequence_6_step_04();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_08_sequence_6_step_04(void)
{
    func_8006B328(0x64, 0x6C, 4, -1, -1, -4, 0, 8, -0x6E, 0xF0, -0x6E, 0xF0, 1, 0x7F, 1, 4, 0);
    if (--g_wmap_land_effect_08_sequence_6_timer == 0)
    {
        g_wmap_land_effect_08_sequence_6_step += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_08_sequence_6_step_05(void)
{
    D_800DCEA8 = 0;
    g_wmap_land_effect_08_sequence_6_timer = 0x10;
    g_wmap_land_effect_08_sequence_6_step += 1;
    wmap_land_effect_08_sequence_6_step_06();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_08_sequence_6_step_06(void)
{
    func_8006B328(0x64, 0x6C, 4, -1, -1, -4, 0, 8, -0x6E, 0xF0, -0x6E, 0xF0, 1, 0x7F, 1, 4, 0);
    if (--g_wmap_land_effect_08_sequence_6_timer == 0)
    {
        g_wmap_land_effect_08_sequence_6_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_08_sequence_6_end(void)
{
    g_wmap_land_effect_08_sequence_6_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_08_run_sequence_7(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_08_sequence_7_step = 1;
        g_wmap_land_effect_08_sequence_7_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_08_sequence_7_step < 0x6)
    {
        D_800D6248[g_wmap_land_effect_08_sequence_7_step]();
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
void wmap_land_effect_08_sequence_7_reset(void)
{
    g_wmap_land_effect_08_sequence_7_step = 1;
    g_wmap_land_effect_08_sequence_7_timer = 1;
}

void wmap_land_effect_08_sequence_7_step_01(void)
{
    D_801399BC = &D_80121538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.sequence = 1;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 8;
    D_800D9370.resource_index = 0;
    D_800D9370.target_shade = 0x81;
    D_800D9370.shade = 1;
    g_wmap_land_effect_08_sequence_7_timer = 0x28;
    g_wmap_land_effect_08_sequence_7_step += 1;
    wmap_land_effect_08_sequence_7_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_08_sequence_7_step_02(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x8, 0x2, 0);
    if (--g_wmap_land_effect_08_sequence_7_timer == 0)
    {
        g_wmap_land_effect_08_sequence_7_step += 1;
    }
}

/** @brief Initialize resource fields, begin a 16-tick delay, and run the next step. */
void wmap_land_effect_08_sequence_7_step_03(void)
{
    D_800D9370.target_shade = 2;
    D_800D9370.shade_step = 8;
    g_wmap_land_effect_08_sequence_7_timer = 16;
    g_wmap_land_effect_08_sequence_7_step += 1;
    wmap_land_effect_08_sequence_7_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_08_sequence_7_step_04(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x8, 0x2, 0);
    if (--g_wmap_land_effect_08_sequence_7_timer == 0)
    {
        g_wmap_land_effect_08_sequence_7_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_08_sequence_7_end(void)
{
    g_wmap_land_effect_08_sequence_7_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_08_run_sequence_8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_08_sequence_8_step = 1;
        g_wmap_land_effect_08_sequence_8_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_08_sequence_8_step < 0x6)
    {
        D_800D6260[g_wmap_land_effect_08_sequence_8_step]();
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
void wmap_land_effect_08_sequence_8_reset(void)
{
    g_wmap_land_effect_08_sequence_8_step = 1;
    g_wmap_land_effect_08_sequence_8_timer = 1;
}

/**
 * @brief Reset a range of world-map actor configs, arm the timer, and advance the step.
 */
void wmap_land_effect_08_sequence_8_step_03(void)
{
    s32 i;

    for (i = 0x96; i < 0xB4; i++)
    {
        D_800D9268[i].target_shade = 0;
        D_800D9268[i].shade_step = 4;
    }
    g_wmap_land_effect_08_sequence_8_timer = 0x20;
    g_wmap_land_effect_08_sequence_8_step += 1;
    wmap_land_effect_08_sequence_8_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_08_sequence_8_end(void)
{
    g_wmap_land_effect_08_sequence_8_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_08_run_sequence_9(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_08_sequence_9_step = 1;
        g_wmap_land_effect_08_sequence_9_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_08_sequence_9_step < 0x6)
    {
        D_800D6278[g_wmap_land_effect_08_sequence_9_step]();
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
void wmap_land_effect_08_sequence_9_reset(void)
{
    g_wmap_land_effect_08_sequence_9_step = 1;
    g_wmap_land_effect_08_sequence_9_timer = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_08_sequence_9_step_02(void)
{
    func_8006A2FC(D_800DB578, D_80139FE8, 0xA, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--g_wmap_land_effect_08_sequence_9_timer == 0)
    {
        g_wmap_land_effect_08_sequence_9_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_08_sequence_9_step_03(void)
{
    g_wmap_land_effect_08_sequence_9_timer = 0x40;
    D_80139280[15] = -1;
    g_wmap_land_effect_08_sequence_9_step += 1;
    wmap_land_effect_08_sequence_9_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_08_sequence_9_step_04(void)
{
    func_8006A2FC(D_800DB578, D_80139FE8, 0xA, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--g_wmap_land_effect_08_sequence_9_timer == 0)
    {
        g_wmap_land_effect_08_sequence_9_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_08_sequence_9_end(void)
{
    g_wmap_land_effect_08_sequence_9_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_08_run_sequence_10(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_08_sequence_10_step = 1;
        g_wmap_land_effect_08_sequence_10_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_08_sequence_10_step < 0x4)
    {
        D_800D6290[g_wmap_land_effect_08_sequence_10_step]();
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
void wmap_land_effect_08_sequence_10_reset(void)
{
    g_wmap_land_effect_08_sequence_10_step = 1;
    g_wmap_land_effect_08_sequence_10_timer = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_08_sequence_10_step_01(void)
{
    D_801399BC = D_8011D538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.sequence = 3;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 1;
    D_800D9370.resource_index = 0;
    D_800D9370.target_shade = 0x81;
    D_800D9370.shade = 0x81;
    g_wmap_land_effect_08_sequence_10_timer = 0x24;
    g_wmap_land_effect_08_sequence_10_step += 1;
    wmap_land_effect_08_sequence_10_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_08_sequence_10_step_02(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0xF, 0x2, 0);
    if (--g_wmap_land_effect_08_sequence_10_timer == 0)
    {
        g_wmap_land_effect_08_sequence_10_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_08_sequence_10_end(void)
{
    g_wmap_land_effect_08_sequence_10_step += 1;
}
