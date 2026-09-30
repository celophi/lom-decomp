#include "wmap_main.h"
#include "wmap_land_effect_21.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_sprite_render.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_effect_primitives.h"
#include "wmap_step_sequence.h"

void wmap_land_effect_21_sequence_7_step_02(void);
void wmap_land_effect_21_sequence_8_step_02(void);
void wmap_land_effect_21_wait_idle_02(void);
void wmap_land_effect_21_step_03(void);
void wmap_land_effect_21_wait_idle_04(void);
s32 wmap_land_effect_21_run_timeline(s32 arg0);
void wmap_land_effect_21_end(void);
s32 wmap_land_effect_21_run_sequence_3(s32 arg0);
s32 wmap_land_effect_21_run_sequence_1(s32 arg0);
s32 wmap_land_effect_21_run_sequence_6(s32 arg0);
s32 wmap_land_effect_21_run_sequence_5(s32 arg0);
s32 wmap_land_effect_21_run_sequence_8(s32 arg0);
s32 wmap_land_effect_21_run_sequence_4(s32 arg0);
s32 wmap_land_effect_21_run_sequence_7(s32 arg0);
s32 wmap_land_effect_21_run_sequence_2(s32 arg0);
void wmap_land_effect_21_sequence_1_step_02(void);
void wmap_land_effect_21_sequence_1_step_04(void);
void wmap_land_effect_21_sequence_1_step_06(void);
void wmap_land_effect_21_sequence_1_step_08(void);
void wmap_land_effect_21_sequence_2_step_02(void);
void wmap_land_effect_21_sequence_5_step_04(void);
void wmap_land_effect_21_sequence_6_step_02(void);
void wmap_land_effect_21_sequence_6_step_04(void);
void wmap_land_effect_21_sequence_6_step_06(void);
void wmap_land_effect_21_sequence_7_step_04(void);

/** @brief Drawing position with the original 0x14-byte stride. */
typedef struct { s32 position; u8 pad[0x10]; } WmapPosition;

/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 g_wmap_land_effect_21_sequence_3_timer;
extern s32 D_80182DEC;
extern u8* D_8011CF1C;
extern s32 g_wmap_land_effect_21_sequence_4_timer;
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 g_wmap_land_effect_21_sequence_5_timer;
extern WmapPosition D_801AFBE0[];
extern s32 D_801B0FD0;
extern u8 D_8011D538[];
extern s32 g_wmap_land_effect_21_sequence_7_timer;
extern u8 D_80121538[];
extern s32 g_wmap_land_effect_21_sequence_8_timer;
extern s32 g_wmap_land_effect_21_timer;
extern void (*D_800D5B60[])(void);
extern void wmap_land_effect_21_end(void);
extern s32 g_wmap_land_effect_21_timeline_timer;
extern void (*D_800D5B78[])(void);
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 g_wmap_land_effect_21_sequence_1_timer;
extern void (*D_800D5BB8[])(void);
extern u8* D_801399AC;
extern void wmap_land_effect_21_sequence_1_step_02(void);
extern s16 D_800D9326;
extern void wmap_land_effect_21_sequence_1_step_08(void);
extern s32 g_wmap_land_effect_21_sequence_2_timer;
extern void (*D_800D5BE0[])(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern void wmap_land_effect_21_sequence_2_step_02(void);
extern void (*D_800D5BF0[])(void);
extern void (*D_800D5C00[])(void);
extern void (*D_800D5C10[])(void);
extern u8 *D_80139B6C;
extern s32 g_wmap_land_effect_21_sequence_6_timer;
extern void (*D_800D5C30[])(void);
extern u8* D_801399BC;
extern s16 D_800D937E;
extern void wmap_land_effect_21_sequence_6_step_06(void);
extern void (*D_800D5C50[])(void);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern void wmap_land_effect_21_sequence_7_step_04(void);
extern void (*D_800D5C68[])(void);
extern WmapAnimationSlot D_80139A28[];

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
} __attribute__((aligned(4))) WmapConfigA;

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    union { s32 packed; struct { s16 x, y; } point; } screen;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

extern u32 g_wmap_land_effect_21_sequence_3_step;
extern u32 g_wmap_land_effect_21_sequence_4_step;
extern u32 g_wmap_land_effect_21_sequence_5_step;
extern u32 g_wmap_land_effect_21_sequence_7_step;
extern WmapConfigA D_800D95D8[];
extern u32 g_wmap_land_effect_21_sequence_8_step;
extern u32 g_wmap_land_effect_21_step;
extern u32 g_wmap_land_effect_21_timeline_step;
extern u32 g_wmap_land_effect_21_sequence_1_step;
extern u32 g_wmap_land_effect_21_sequence_2_step;
extern WmapConfigA D_800D9CB8;
extern u32 g_wmap_land_effect_21_sequence_6_step;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;

extern SVECTOR D_80139258;
extern SVECTOR D_801B24A0;
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
extern WmapScreenPosition D_80182D58;

extern s32* D_80139280;

extern WmapMotion D_801AFBD0[];
extern WmapMotion D_801AFD60[];
extern WmapMotion D_801B0080;

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_21_sequence_3_step_02(void)
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
        D_80182DE8 -= 0x5;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_21_sequence_3_timer == 0)
    {
        g_wmap_land_effect_21_sequence_3_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_21_sequence_4_step_02(void)
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
    if (--g_wmap_land_effect_21_sequence_4_timer == 0)
    {
        g_wmap_land_effect_21_sequence_4_step += 1;
    }
}

/** @brief Project a spiral emitter and append animated copies along its trail. */
void wmap_land_effect_21_sequence_5_step_02(void)
{
    SVECTOR position;
    s32 previous;
    s32 next;
    s32 i;
    s32 remaining;
    WmapMotion *motion;
    WmapResource *resource;
    WmapConfigA *actor;

    position.vx = ((D_801B0080.z >> 3) * (ccos(D_801B0080.angle) >> 6)) >> 12;
    position.vy = ((D_801B0080.z >> 3) * (csin(D_801B0080.angle) >> 6)) >> 12;
    position.vz = D_801B0080.field_0E;
    gte_ldv0(&position);
    gte_rtps();
    D_801B0080.z += 1500;
    D_801B0080.angle += 192;
    D_8013923C--;
    gte_stsxy(&D_80182D58);
    D_801B0080.screen.point.x = D_80182D58.point.x;
    D_801B0080.screen.point.y = D_80182D58.point.y;
    if (D_8013923C == 0)
    {
        previous = D_80139234;
        D_80139234 = previous + 1;
        next = previous + 61;
        if (next < 250)
        {
            D_8013923C = 2;
            motion = &D_801B0080 - 60;
            motion[next] = D_801B0080;
            D_80139988[D_80139234 + 60] = D_80139988[60];
            D_800D9268[D_80139234 + 60] = D_800D9268[60];
            D_800D9268[D_80139234 + 60].sequence = 0;
            D_800D9268[D_80139234 + 60].previous_sequence = -1;
        }
    }
    for (i = 61; i < D_80139234 + 60; i++)
    {
        actor = &D_800D9268[i];
        motion = &D_801AFBD0[i];
        resource = &D_80139988[i];
        wmap_step_actor_animation(actor, resource);
        wmap_draw_actor_sprite(actor, motion->screen.packed, 8, 10, 0);
    }
    remaining = g_wmap_land_effect_21_sequence_5_timer - 1;
    g_wmap_land_effect_21_sequence_5_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_21_sequence_5_step++;
    }
}

/** @brief Animate and draw the active actor range, then advance its countdown. */
void wmap_land_effect_21_sequence_5_step_06(void)
{
    s32 i;
    s32 remaining;

    for (i = 61; i < D_80139234 + 60; i++)
    {
        D_800D9268[i].target_shade = 0;
        D_800D9268[i].shade_step = 2;
        wmap_step_actor_animation(&D_800D9268[i], &D_80139988[i]);
        wmap_draw_actor_sprite(&D_800D9268[i], D_801AFBE0[i].position, 8, 10, 0);
    }
    remaining = g_wmap_land_effect_21_sequence_5_timer - 1;
    g_wmap_land_effect_21_sequence_5_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_21_sequence_5_step++;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_21_sequence_7_step_01(void)
{
    s32 i;

    D_801B0FD0 = 24;
    D_80139280[0x1F] = 1;
    D_80139280[0x20] = 1;
    D_80139280[0x21] = 20;
    D_80139280[0x22] = 30;
    D_80139280[0x23] = 2;
    D_80139280[0x24] = 1500;
    D_80139280[0x25] = 20;
    D_80139280[0x26] = 15;
    D_80139280[0x27] = 5;
    D_80139280[0x28] = 10000;
    for (i = 0; i < 24; i++)
    {
        D_801AFBD0[i + D_80139280[0x25]].state = 0;
        D_80139988[i + 24].data = D_8011D538;
    }
    g_wmap_land_effect_21_sequence_7_timer = 48;
    g_wmap_land_effect_21_sequence_7_step++;
    wmap_land_effect_21_sequence_7_step_02();
}

/** @brief Initialize the effect actors, resources, and evenly spaced angles. */
void wmap_land_effect_21_sequence_8_step_01(void)
{
    s32 i;

    i = 0;
    D_801B0FD0 = 12;
    D_80139280[0x15] = 128;
    D_80139280[0x16] = 1;
    D_80139280[0x17] = 128;
    D_80139280[0x18] = 0;
    D_80139280[0x19] = -1;
    D_80139280[0x1A] = 1500;
    D_80139280[0x1B] = 20;
    D_80139280[0x1C] = 8;
    D_80139280[0x1D] = 1;
    D_80139280[0x1E] = 10;
    do
    {
        D_801AFD60[i].state = 1;
        D_801AFD60[i].angle = i * 341;
        D_801AFD60[i].scale = 128;
        D_801AFD60[i].z = 10;
        D_801AFD60[i].x = 0;
        D_801AFD60[i].field_0E = 0;
        D_80139988[i + 20].data = D_80121538;
        D_800D95D8[i].field_06 = 15;
        D_800D95D8[i].field_10 = -1;
        D_800D95D8[i].field_26 = 2;
        D_800D95D8[i].field_02 = 0;
        D_800D95D8[i].field_0E = 1;
        D_800D95D8[i].field_22 = 0;
        D_800D95D8[i].field_24 = 127;
        i++;
    } while (i < 12);
    g_wmap_land_effect_21_sequence_8_timer = 32;
    g_wmap_land_effect_21_sequence_8_step++;
    wmap_land_effect_21_sequence_8_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_21_run, D_800D5B60, 0x6, g_wmap_land_effect_21_step, g_wmap_land_effect_21_timer)

WMAP_STEP_RESET(wmap_land_effect_21_reset, g_wmap_land_effect_21_step, g_wmap_land_effect_21_timer)

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_21_step_01(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_21_step += 1;
    wmap_land_effect_21_wait_idle_02();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_21_wait_idle_02, g_wmap_land_effect_21_step, wmap_land_effect_21_step_03)

/** @brief Save the coordinate pair, register a callback, and run the next sequence step. */
void wmap_land_effect_21_step_03(void)
{
    D_80182D58.packed = g_wmap_focus_screen_position.packed;
    wmap_start_sequence(&wmap_land_effect_21_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_21_step += 1;
    wmap_land_effect_21_wait_idle_04();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_21_wait_idle_04, g_wmap_land_effect_21_step, wmap_land_effect_21_end)

WMAP_STEP_ADVANCE(wmap_land_effect_21_end, g_wmap_land_effect_21_step)

WMAP_STEP_RUNNER(wmap_land_effect_21_run_timeline, D_800D5B78, 0x10, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_21_timeline_reset, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

/** @brief Play sound 34, set the world-map color and value, and begin an eight-tick delay. */
void wmap_land_effect_21_timeline_step_01(void)
{
    wmap_play_sound(0x22, 0x80);
    g_wmap_backdrop_target_level = 4;
    wmap_start_map_tint(0x304010);
    g_wmap_land_effect_21_timeline_timer = 8;
    g_wmap_land_effect_21_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_21_timeline_wait_02, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_21_timeline_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_21_run_sequence_3);
    g_wmap_land_effect_21_timeline_timer = 0x4;
    g_wmap_land_effect_21_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_21_timeline_wait_04, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void wmap_land_effect_21_timeline_step_05(void)
{
    D_801ADAE0 = 1;
    wmap_start_sequence(wmap_land_effect_21_run_sequence_1);
    g_wmap_land_effect_21_timeline_timer = 0x34;
    g_wmap_land_effect_21_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_21_timeline_wait_06, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_21_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_21_run_sequence_6);
    g_wmap_land_effect_21_timeline_timer = 0x38;
    g_wmap_land_effect_21_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_21_timeline_wait_08, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_21_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_21_run_sequence_5);
    wmap_start_sequence(wmap_land_effect_21_run_sequence_8);
    g_wmap_land_effect_21_timeline_timer = 0x68;
    g_wmap_land_effect_21_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_21_timeline_wait_10, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_21_timeline_step_11(void)
{
    wmap_start_sequence(wmap_land_effect_21_run_sequence_4);
    g_wmap_land_effect_21_timeline_timer = 0x3C;
    g_wmap_land_effect_21_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_21_timeline_wait_12, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_21_timeline_step_13(void)
{
    wmap_start_sequence(wmap_land_effect_21_run_sequence_7);
    wmap_start_sequence(wmap_land_effect_21_run_sequence_2);
    g_wmap_land_effect_21_timeline_timer = 0x8B;
    g_wmap_land_effect_21_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_21_timeline_wait_14, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

void wmap_land_effect_21_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    g_wmap_land_effect_21_timeline_step += 1;
}

WMAP_STEP_RUNNER(wmap_land_effect_21_run_sequence_1, D_800D5BB8, 0xA, g_wmap_land_effect_21_sequence_1_step, g_wmap_land_effect_21_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_21_sequence_1_reset, g_wmap_land_effect_21_sequence_1_step, g_wmap_land_effect_21_sequence_1_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_21_sequence_1_step_01(void)
{
    D_801399AC = D_8011D538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 8;
    D_800D9318.target_shade = 0x81;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.shade = 1;
    g_wmap_land_effect_21_sequence_1_timer = 0x14;
    g_wmap_land_effect_21_sequence_1_step += 1;
    wmap_land_effect_21_sequence_1_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_21_sequence_1_step_02(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, D_80182D58.packed, 0xF, 0x9, 0);
    if (--g_wmap_land_effect_21_sequence_1_timer == 0)
    {
        g_wmap_land_effect_21_sequence_1_step += 1;
    }
}

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void wmap_land_effect_21_sequence_1_step_03(void)
{
    D_800D9326 = 1;
    g_wmap_land_effect_21_sequence_1_timer = 0x20;
    g_wmap_land_effect_21_sequence_1_step += 1;
    wmap_land_effect_21_sequence_1_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_21_sequence_1_step_04(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, D_80182D58.packed, 0xF, 0x9, 0);
    if (--g_wmap_land_effect_21_sequence_1_timer == 0)
    {
        g_wmap_land_effect_21_sequence_1_step += 1;
    }
}

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void wmap_land_effect_21_sequence_1_step_05(void)
{
    D_800D9326 = 2;
    g_wmap_land_effect_21_sequence_1_timer = 0x8C;
    g_wmap_land_effect_21_sequence_1_step += 1;
    wmap_land_effect_21_sequence_1_step_06();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_21_sequence_1_step_06(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, D_80182D58.packed, 0xF, 0x9, 0);
    if (--g_wmap_land_effect_21_sequence_1_timer == 0)
    {
        g_wmap_land_effect_21_sequence_1_step += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_21_sequence_1_step_07(void)
{
    D_800D9268[4].shade_step = 8;
    D_800D9268[4].target_shade = 0;
    g_wmap_land_effect_21_sequence_1_timer = 0x10;
    g_wmap_land_effect_21_sequence_1_step += 1;
    wmap_land_effect_21_sequence_1_step_08();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_21_sequence_1_step_08(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, D_80182D58.packed, 0xF, 0x9, 0);
    if (--g_wmap_land_effect_21_sequence_1_timer == 0)
    {
        g_wmap_land_effect_21_sequence_1_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_21_sequence_1_end, g_wmap_land_effect_21_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_21_run_sequence_2, D_800D5BE0, 0x4, g_wmap_land_effect_21_sequence_2_step, g_wmap_land_effect_21_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_21_sequence_2_reset, g_wmap_land_effect_21_sequence_2_step, g_wmap_land_effect_21_sequence_2_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_21_sequence_2_step_01(void)
{
    D_801399B4 = D_8011F538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 8;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0;
    g_wmap_land_effect_21_sequence_2_timer = 0x8C;
    g_wmap_land_effect_21_sequence_2_step += 1;
    wmap_land_effect_21_sequence_2_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_21_sequence_2_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x18, 0x1E, 0);
    if (--g_wmap_land_effect_21_sequence_2_timer == 0)
    {
        g_wmap_land_effect_21_sequence_2_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_21_sequence_2_end, g_wmap_land_effect_21_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_21_run_sequence_3, D_800D5BF0, 0x4, g_wmap_land_effect_21_sequence_3_step, g_wmap_land_effect_21_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_21_sequence_3_reset, g_wmap_land_effect_21_sequence_3_step, g_wmap_land_effect_21_sequence_3_timer)

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_21_sequence_3_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    g_wmap_land_effect_21_sequence_3_timer = 0x28;
    g_wmap_land_effect_21_sequence_3_step += 1;
    wmap_land_effect_21_sequence_3_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_effect_21_sequence_3_end, g_wmap_land_effect_21_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_21_run_sequence_4, D_800D5C00, 0x4, g_wmap_land_effect_21_sequence_4_step, g_wmap_land_effect_21_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_21_sequence_4_reset, g_wmap_land_effect_21_sequence_4_step, g_wmap_land_effect_21_sequence_4_timer)

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_21_sequence_4_step_01(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    g_wmap_land_effect_21_sequence_4_timer = 0x80;
    g_wmap_land_effect_21_sequence_4_step += 1;
    wmap_land_effect_21_sequence_4_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_effect_21_sequence_4_end, g_wmap_land_effect_21_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_21_run_sequence_5, D_800D5C10, 0x8, g_wmap_land_effect_21_sequence_5_step, g_wmap_land_effect_21_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_21_sequence_5_reset, g_wmap_land_effect_21_sequence_5_step, g_wmap_land_effect_21_sequence_5_timer)

/** @brief Initialize the actor and auxiliary state for the next timed step. */
void wmap_land_effect_21_sequence_5_step_01(void)
{
    D_80139234 = 0;
    D_8013923C = 2;
    D_80139B6C = D_80121538;
    D_800D9CB8.field_06 = 0xF;
    D_800D9CB8.field_10 = -1;
    D_800D9CB8.field_26 = 8;
    D_800D9CB8.field_02 = 0;
    D_800D9CB8.field_0E = 0;
    D_800D9CB8.field_22 = 0x81;
    D_800D9CB8.field_24 = 0x81;
    D_801B0080.state = 1;
    D_801B0080.z = 0;
    D_801B0080.angle = 0;
    D_801B0080.field_0E = 0;
    g_wmap_land_effect_21_sequence_5_timer = 0x70;
    g_wmap_land_effect_21_sequence_5_step += 1;
    wmap_land_effect_21_sequence_5_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_21_sequence_5_step_03(void)
{
    g_wmap_land_effect_21_sequence_5_timer = 0x28;
    g_wmap_land_effect_21_sequence_5_step += 1;
    wmap_land_effect_21_sequence_5_step_04();
}

/** @brief Animate and draw the active actor range, then advance its countdown. */
void wmap_land_effect_21_sequence_5_step_04(void)
{
    s32 i;
    s32 remaining;

    for (i = 61; i < D_80139234 + 60; i++)
    {
        wmap_step_actor_animation(&D_800D9268[i], &D_80139988[i]);
        wmap_draw_actor_sprite(&D_800D9268[i], D_801AFBE0[i].position, 8, 10, 0);
    }
    remaining = g_wmap_land_effect_21_sequence_5_timer - 1;
    g_wmap_land_effect_21_sequence_5_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_21_sequence_5_step++;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_21_sequence_5_step_05(void)
{
    g_wmap_land_effect_21_sequence_5_timer = 0x40;
    g_wmap_land_effect_21_sequence_5_step += 1;
    wmap_land_effect_21_sequence_5_step_06();
}

WMAP_STEP_ADVANCE(wmap_land_effect_21_sequence_5_end, g_wmap_land_effect_21_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_21_run_sequence_6, D_800D5C30, 0x8, g_wmap_land_effect_21_sequence_6_step, g_wmap_land_effect_21_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_21_sequence_6_reset, g_wmap_land_effect_21_sequence_6_step, g_wmap_land_effect_21_sequence_6_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_21_sequence_6_step_01(void)
{
    D_801399BC = D_8011D538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.sequence = 3;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 2;
    D_800D9370.target_shade = 0x81;
    D_800D9370.resource_index = 0;
    D_800D9370.shade = 1;
    g_wmap_land_effect_21_sequence_6_timer = 0x10;
    g_wmap_land_effect_21_sequence_6_step += 1;
    wmap_land_effect_21_sequence_6_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_21_sequence_6_step_02(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, D_80182D58.packed, 0xF, 0x2, 0);
    if (--g_wmap_land_effect_21_sequence_6_timer == 0)
    {
        g_wmap_land_effect_21_sequence_6_step += 1;
    }
}

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void wmap_land_effect_21_sequence_6_step_03(void)
{
    D_800D937E = 4;
    g_wmap_land_effect_21_sequence_6_timer = 0x8C;
    g_wmap_land_effect_21_sequence_6_step += 1;
    wmap_land_effect_21_sequence_6_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_21_sequence_6_step_04(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, D_80182D58.packed, 0xF, 0x2, 0);
    if (--g_wmap_land_effect_21_sequence_6_timer == 0)
    {
        g_wmap_land_effect_21_sequence_6_step += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_21_sequence_6_step_05(void)
{
    D_800D9268[6].shade_step = 8;
    D_800D9268[6].target_shade = 0;
    g_wmap_land_effect_21_sequence_6_timer = 0x10;
    g_wmap_land_effect_21_sequence_6_step += 1;
    wmap_land_effect_21_sequence_6_step_06();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_21_sequence_6_step_06(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, D_80182D58.packed, 0xF, 0x2, 0);
    if (--g_wmap_land_effect_21_sequence_6_timer == 0)
    {
        g_wmap_land_effect_21_sequence_6_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_21_sequence_6_end, g_wmap_land_effect_21_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_21_run_sequence_7, D_800D5C50, 0x6, g_wmap_land_effect_21_sequence_7_step, g_wmap_land_effect_21_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_21_sequence_7_reset, g_wmap_land_effect_21_sequence_7_step, g_wmap_land_effect_21_sequence_7_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_21_sequence_7_step_02(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x18, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--g_wmap_land_effect_21_sequence_7_timer == 0)
    {
        g_wmap_land_effect_21_sequence_7_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_21_sequence_7_step_03(void)
{
    g_wmap_land_effect_21_sequence_7_timer = 0x20;
    D_80139280[35] = -1;
    g_wmap_land_effect_21_sequence_7_step += 1;
    wmap_land_effect_21_sequence_7_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_21_sequence_7_step_04(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x18, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--g_wmap_land_effect_21_sequence_7_timer == 0)
    {
        g_wmap_land_effect_21_sequence_7_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_21_sequence_7_end, g_wmap_land_effect_21_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_21_run_sequence_8, D_800D5C68, 0x4, g_wmap_land_effect_21_sequence_8_step, g_wmap_land_effect_21_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_21_sequence_8_reset, g_wmap_land_effect_21_sequence_8_step, g_wmap_land_effect_21_sequence_8_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_21_sequence_8_step_02(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0xC, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--g_wmap_land_effect_21_sequence_8_timer == 0)
    {
        g_wmap_land_effect_21_sequence_8_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_21_sequence_8_end, g_wmap_land_effect_21_sequence_8_step)
