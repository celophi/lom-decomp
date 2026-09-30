#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_land_effect_19.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"
#include "wmap_step_sequence.h"

void wmap_land_effect_19_sequence_7_step_02(void);
void wmap_land_effect_19_sequence_8_step_02(void);
void wmap_land_effect_19_sequence_9_step_02(void);
void wmap_land_effect_19_sequence_10_step_02(void);
void wmap_land_effect_19_sequence_11_step_02(void);
void wmap_land_effect_19_wait_idle_02(void);
void wmap_land_effect_19_step_03(void);
s32 wmap_land_effect_19_run_timeline(s32 arg0);
void wmap_land_effect_19_wait_idle_04(void);
void wmap_land_effect_19_end(void);
s32 wmap_land_effect_19_run_sequence_7(s32 arg0);
s32 wmap_land_effect_19_run_sequence_1(s32 arg0);
s32 wmap_land_effect_19_run_sequence_3(s32 arg0);
s32 wmap_land_effect_19_run_sequence_5(s32 arg0);
s32 wmap_land_effect_19_run_sequence_6(s32 arg0);
s32 wmap_land_effect_19_run_sequence_10(s32 arg0);
s32 wmap_land_effect_19_run_sequence_11(s32 arg0);
s32 wmap_land_effect_19_run_sequence_4(s32 arg0);
s32 wmap_land_effect_19_run_sequence_9(s32 arg0);
s32 wmap_land_effect_19_run_sequence_8(s32 arg0);
s32 wmap_land_effect_19_run_sequence_2(s32 arg0);
s32 wmap_land_effect_19_run_sequence_12(s32 arg0);
void wmap_land_effect_19_sequence_1_step_02(void);
void wmap_land_effect_19_sequence_2_step_02(void);
void wmap_land_effect_19_sequence_7_step_04(void);
void wmap_land_effect_19_sequence_7_step_06(void);
void wmap_land_effect_19_sequence_8_step_04(void);
void wmap_land_effect_19_sequence_9_step_04(void);
void wmap_land_effect_19_sequence_9_step_06(void);
void wmap_land_effect_19_sequence_10_step_04(void);
void wmap_land_effect_19_sequence_10_step_06(void);
void wmap_land_effect_19_sequence_11_step_04(void);
void wmap_land_effect_19_sequence_11_step_06(void);
void wmap_land_effect_19_sequence_12_step_02(void);

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

/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 g_wmap_land_effect_19_sequence_3_timer;
extern s32 D_80182DEC;
extern u8* D_8011CF1C;
extern s32 g_wmap_land_effect_19_sequence_4_timer;
extern s32* D_8011CF24;
extern s32 D_80182DF0;
extern s32 g_wmap_land_effect_19_sequence_5_timer;
extern s32 D_80182DF4;
extern s32 g_wmap_land_effect_19_sequence_6_timer;
extern s32 D_80182DE4;
extern s32 D_800DCEA8;
extern s32 D_800D9150;
extern s32 g_wmap_land_effect_19_sequence_7_timer;
extern s32 g_wmap_land_effect_19_sequence_8_timer;
extern s32 D_801B25D8;
extern s32 D_800DCEAC;
extern s32 D_800D9154;
extern s32 g_wmap_land_effect_19_sequence_9_timer;
extern s32 rand(void);
extern u8 D_80121538[];
extern s32 D_801B25DC;
extern s32 D_800DCEB0;
extern s32 D_800D9158;
extern s32 g_wmap_land_effect_19_sequence_10_timer;
extern s32 D_800D915C;
extern s32 D_800DCEB4;
extern s32 g_wmap_land_effect_19_sequence_11_timer;
extern s32 g_wmap_land_effect_19_timer;
extern void (*D_800D6A84[])(void);
extern void wmap_land_effect_19_step_03(void);
extern void wmap_land_effect_19_end(void);
extern s32 g_wmap_land_effect_19_timeline_timer;
extern void (*D_800D6A9C[])(void);
extern s32 D_8013B208;
extern s32 D_80139244;
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 g_wmap_land_effect_19_sequence_1_timer;
extern void (*D_800D6AFC[])(void);
extern void *D_801399AC;
extern s32 g_wmap_land_effect_19_sequence_2_timer;
extern void (*D_800D6B0C[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern void wmap_land_effect_19_sequence_2_step_02(void);
extern void (*D_800D6B1C[])(void);
extern void (*D_800D6B2C[])(void);
extern void (*D_800D6B3C[])(void);
extern void (*D_800D6B54[])(void);
extern void (*D_800D6B6C[])(void);
extern void (*D_800D6B8C[])(void);
extern u8 D_800D95D8[];
extern WmapAnimationSlot D_80139A28[];
extern void (*D_800D6BA4[])(void);
extern void (*D_800D6BC4[])(void);
extern void (*D_800D6BE4[])(void);
extern s32 g_wmap_land_effect_19_sequence_12_timer;
extern void (*D_800D6C04[])(void);
extern u8* D_801399BC;
extern u8 D_80125538[];

extern u32 g_wmap_land_effect_19_sequence_3_step;
extern u32 g_wmap_land_effect_19_sequence_4_step;
extern u32 g_wmap_land_effect_19_sequence_5_step;
extern u32 g_wmap_land_effect_19_sequence_6_step;
extern u8 D_8011F538[];
extern u32 g_wmap_land_effect_19_sequence_7_step;
extern u8 D_80123538[];
extern u32 g_wmap_land_effect_19_sequence_8_step;
extern u32 g_wmap_land_effect_19_sequence_9_step;
extern u32 g_wmap_land_effect_19_sequence_10_step;
extern u32 g_wmap_land_effect_19_sequence_11_step;
extern u32 g_wmap_land_effect_19_step;
extern u32 g_wmap_land_effect_19_timeline_step;
extern u32 g_wmap_land_effect_19_sequence_1_step;
extern u32 g_wmap_land_effect_19_sequence_2_step;
extern u32 g_wmap_land_effect_19_sequence_12_step;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;

extern SVECTOR D_80139258;
extern SVECTOR D_8013B240;
extern SVECTOR D_801B24A0;
extern SVECTOR D_8013B238;
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

extern WmapSlot14 D_801AFBD0[];

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_19_sequence_3_step_02(void)
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
    if (--g_wmap_land_effect_19_sequence_3_timer == 0)
    {
        g_wmap_land_effect_19_sequence_3_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_19_sequence_4_step_02(void)
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
    if (--g_wmap_land_effect_19_sequence_4_timer == 0)
    {
        g_wmap_land_effect_19_sequence_4_step += 1;
    }
}

/**
 * @brief World-map step handler: draw the animated actor, ramp its size up to a
 *        cap, scroll the sprite field, then advance when the frame counter expires.
 */
void wmap_land_effect_19_sequence_5_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF24, 0, 0xA, 0x36, 0x7880, 0x1001, D_80182DF0, 0, 0xA, -1);
    value = D_80182DF0 + 4;
    D_80182DF0 = value;
    if (value >= 0x82)
    {
        D_80182DF0 = 0x81;
    }
    timer = g_wmap_land_effect_19_sequence_5_timer;
    D_8013B238.vz += 0x16;
    next_timer = timer - 1;
    g_wmap_land_effect_19_sequence_5_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_effect_19_sequence_5_step++;
    }
}

/**
 * @brief World-map step handler: render the actor, ramp its size down to a floor,
 *        scroll the shadow field, then advance when the frame counter expires.
 */
void wmap_land_effect_19_sequence_5_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF24, 0, 0xA, 0x36, 0x7880, 0x1001, D_80182DF0, 0, 0xA, -1);
    value = D_80182DF0 - 2;
    D_80182DF0 = value;
    if (value < 0)
    {
        D_80182DF0 = 0;
    }
    timer = g_wmap_land_effect_19_sequence_5_timer;
    D_8013B238.vz += 0x16;
    next_timer = timer - 1;
    g_wmap_land_effect_19_sequence_5_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_effect_19_sequence_5_step++;
    }
}

/**
 * @brief World-map step handler: draw the animated actor, ramp its size up to a
 *        cap, scroll the sprite field, then advance when the frame counter expires.
 */
void wmap_land_effect_19_sequence_6_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B240);
    wmap_draw_model(D_8011CF24, 0, 0xA, 0x36, 0x78C0, 0x1001, D_80182DF4, 0, 0xA, -1);
    value = D_80182DF4 + 8;
    D_80182DF4 = value;
    if (value >= 0x82)
    {
        D_80182DF4 = 0x81;
    }
    timer = g_wmap_land_effect_19_sequence_6_timer;
    D_8013B240.vz += 0x16;
    next_timer = timer - 1;
    g_wmap_land_effect_19_sequence_6_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_effect_19_sequence_6_step++;
    }
}

/**
 * @brief World-map step handler: render the actor, ramp its size down to a floor,
 *        scroll the shadow field, then advance when the frame counter expires.
 */
void wmap_land_effect_19_sequence_6_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B240);
    wmap_draw_model(D_8011CF24, 0, 0xA, 0x36, 0x78C0, 0x1001, D_80182DF4, 0, 0xA, -1);
    value = D_80182DF4 - 2;
    D_80182DF4 = value;
    if (value < 0)
    {
        D_80182DF4 = 0;
    }
    timer = g_wmap_land_effect_19_sequence_6_timer;
    D_8013B240.vz += 0x16;
    next_timer = timer - 1;
    g_wmap_land_effect_19_sequence_6_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_effect_19_sequence_6_step++;
    }
}

/**
 * @brief Reset a range of world-map actor slots and kick the next handler.
 * @note Clears three parallel slot arrays for indices [0xA, 0x14), seeds each
 *       actor's default fields, then advances the shared step counters.
 */
void wmap_land_effect_19_sequence_7_step_01(void)
{
    s32 i;
    u8* pa;
    u8* pb;

    D_80182DE4 = 1;
    D_800DCEA8 = 1;
    for (i = 0xA; i < 0x14; i++)
    {
        *(s16*)((u8*)D_801AFBD0 + i * 0x14) = 0;
        pb = (u8*)D_80139988 + i * 0x8;
        *(s32*)(pb + 0x4) = (s32)&D_8011F538;
        pa = (u8*)D_800D9268 + i * 0x2C;
        *(s16*)(pa + 0x2) = 0;
        *(s8*)(pa + 0x6) = 0xF;
        *(s16*)(pa + 0xE) = 0;
        *(s16*)(pa + 0x10) = -1;
    }
    D_800D9150 = 6;
    g_wmap_land_effect_19_sequence_7_timer = 0x10;
    g_wmap_land_effect_19_sequence_7_step += 1;
    wmap_land_effect_19_sequence_7_step_02();
}

/** @brief Configure the effect block, reset its twelve resource slots, and advance the step. */
void wmap_land_effect_19_sequence_8_step_01(void)
{
    s32 i;

    D_80139280[1] = 0;
    D_80139280[2] = 0;
    D_80139280[3] = 0x20;
    D_80139280[4] = 0;
    D_80139280[5] = 2;
    D_80139280[6] = 0x384;
    D_80139280[7] = 0x14;
    D_80139280[8] = 8;
    D_80139280[9] = 1;
    D_80139280[10] = 0x32C8;

    for (i = 0; i < 0xC; i++)
    {
        D_801AFBD0[i + 20].field_00 = 0;
        D_80139988[i + 20].data = D_80123538;
    }

    g_wmap_land_effect_19_sequence_8_timer = 0x18;
    g_wmap_land_effect_19_sequence_8_step += 1;
    wmap_land_effect_19_sequence_8_step_02();
}

/** @brief Reset the effect actors with randomized animation variants. */
void wmap_land_effect_19_sequence_9_step_01(void)
{
    s32 i;

    D_801B25D8 = 1;
    D_800DCEAC = 1;
    for (i = 80; i < 140; i++)
    {
        D_801AFBD0[i].field_00 = 0;
        D_80139988[i].data = D_8011F538;
        D_800D9268[i].resource_index = 0;
        D_800D9268[i].scale_index = 15;
        D_800D9268[i].sequence = rand() % 3 + 2;
        D_800D9268[i].previous_sequence = -1;
    }
    D_800D9154 = 2;
    g_wmap_land_effect_19_sequence_9_timer = 16;
    g_wmap_land_effect_19_sequence_9_step++;
    wmap_land_effect_19_sequence_9_step_02();
}

/** @brief Initialize sixty alternating actors and begin their timed effect. */
void wmap_land_effect_19_sequence_10_step_01(void)
{
    s32 i;

    D_801B25DC = 1;
    D_800DCEB0 = 12;
    for (i = 150; i < 210; i++)
    {
        D_801AFBD0[i].field_00 = 0;
        D_80139988[i].data = D_80121538;
        D_800D9268[i].sequence = i & 1;
        D_800D9268[i].resource_index = 0;
        D_800D9268[i].scale_index = 15;
        D_800D9268[i].previous_sequence = -1;
    }
    D_800D9158 = 1;
    g_wmap_land_effect_19_sequence_10_timer = 16;
    g_wmap_land_effect_19_sequence_10_step++;
    wmap_land_effect_19_sequence_10_step_02();
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void wmap_land_effect_19_sequence_11_step_01(void)
{
    s32 i;
    WmapD94Entry *entry;

    i = 0xD2;
    D_801B25D8 = 1;
    D_800DCEB4 = 8;

    do
    {
        D_801AFBD0[i].field_00 = 0;
        D_80139988[i].data = D_80123538;
        entry = &D_800D9268[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 0;
        entry->unk10 = -1;
        i++;
    } while (i < 0xE6);

    D_800D915C = 2;
    g_wmap_land_effect_19_sequence_11_timer = 0x10;
    g_wmap_land_effect_19_sequence_11_step += 1;
    wmap_land_effect_19_sequence_11_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_19_run, D_800D6A84, 0x6, g_wmap_land_effect_19_step, g_wmap_land_effect_19_timer)

WMAP_STEP_RESET(wmap_land_effect_19_reset, g_wmap_land_effect_19_step, g_wmap_land_effect_19_timer)

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_19_step_01(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_19_step += 1;
    wmap_land_effect_19_wait_idle_02();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_19_wait_idle_02, g_wmap_land_effect_19_step, wmap_land_effect_19_step_03)

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_19_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_19_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_19_step += 1;
    wmap_land_effect_19_wait_idle_04();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_19_wait_idle_04, g_wmap_land_effect_19_step, wmap_land_effect_19_end)

WMAP_STEP_ADVANCE(wmap_land_effect_19_end, g_wmap_land_effect_19_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_timeline, D_800D6A9C, 0x18, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_19_timeline_reset, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

/**
 * @brief Set up a tinted world-map sub-scene and register its handler.
 */
void wmap_land_effect_19_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_start_map_tint(0x702540);
    g_wmap_backdrop_target_level = 8;
    wmap_play_sound(0x2B, 0x80);
    wmap_start_sequence(wmap_land_effect_19_run_sequence_7);
    g_wmap_land_effect_19_timeline_timer = 0x20;
    g_wmap_land_effect_19_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_02, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

/** @brief Register callbacks, set effect flags and color, and begin a ten-tick delay. */
void wmap_land_effect_19_timeline_step_03(void)
{
    wmap_start_sequence(&wmap_land_effect_19_run_sequence_3);
    D_80139244 = 1;
    D_801ADAE0 = 1;
    wmap_start_sequence(&wmap_land_effect_19_run_sequence_1);
    g_wmap_backdrop_target_level = 1;
    wmap_start_map_tint(0x201010);
    g_wmap_land_effect_19_timeline_timer = 0xA;
    g_wmap_land_effect_19_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_04, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_19_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_19_run_sequence_5);
    g_wmap_land_effect_19_timeline_timer = 0x1E;
    g_wmap_land_effect_19_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_06, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_19_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_19_run_sequence_6);
    g_wmap_land_effect_19_timeline_timer = 0xC;
    g_wmap_land_effect_19_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_08, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_19_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_19_run_sequence_10);
    wmap_start_sequence(wmap_land_effect_19_run_sequence_11);
    g_wmap_land_effect_19_timeline_timer = 0x44;
    g_wmap_land_effect_19_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_10, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_19_timeline_step_11(void)
{
    wmap_start_sequence(wmap_land_effect_19_run_sequence_4);
    g_wmap_land_effect_19_timeline_timer = 0x2;
    g_wmap_land_effect_19_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_12, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

/** @brief World-map step: kick a sub-request, set the next state, and arm the timer. */
void wmap_land_effect_19_timeline_step_13(void)
{
    D_80139244 = 0;
    wmap_start_map_tint(0x504060);
    g_wmap_backdrop_target_level = 8;
    g_wmap_land_effect_19_timeline_timer = 0x1E;
    g_wmap_land_effect_19_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_14, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_19_timeline_step_15(void)
{
    wmap_start_sequence(wmap_land_effect_19_run_sequence_9);
    g_wmap_land_effect_19_timeline_timer = 0x10;
    g_wmap_land_effect_19_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_16, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_19_timeline_step_17(void)
{
    wmap_start_sequence(wmap_land_effect_19_run_sequence_8);
    wmap_start_sequence(wmap_land_effect_19_run_sequence_2);
    g_wmap_land_effect_19_timeline_timer = 0x64;
    g_wmap_land_effect_19_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_18, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_19_timeline_step_19(void)
{
    wmap_start_sequence(wmap_land_effect_19_run_sequence_12);
    g_wmap_land_effect_19_timeline_timer = 0x4;
    g_wmap_land_effect_19_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_20, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_19_timeline_step_21(void)
{
    wmap_start_sequence(wmap_land_effect_19_run_sequence_8);
    g_wmap_land_effect_19_timeline_timer = 0x7C;
    g_wmap_land_effect_19_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_19_timeline_wait_22, g_wmap_land_effect_19_timeline_step, g_wmap_land_effect_19_timeline_timer)

void wmap_land_effect_19_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    g_wmap_land_effect_19_timeline_step += 1;
}

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_1, D_800D6AFC, 0x4, g_wmap_land_effect_19_sequence_1_step, g_wmap_land_effect_19_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_1_reset, g_wmap_land_effect_19_sequence_1_step, g_wmap_land_effect_19_sequence_1_timer)

/**
 * @brief Arm the world-map sprite actor, set its wait, advance the step, and run the draw handler.
 */
void wmap_land_effect_19_sequence_1_step_01(void)
{
    D_801399AC = &D_8011F538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.sequence = 1;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 8;
    D_800D9318.resource_index = 0;
    D_800D9318.target_shade = 0x81;
    D_800D9318.shade = 1;
    g_wmap_land_effect_19_sequence_1_timer = 0x80;
    g_wmap_land_effect_19_sequence_1_step += 1;
    wmap_land_effect_19_sequence_1_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_19_sequence_1_step_02(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0x19, 0x8, 0);
    if (--g_wmap_land_effect_19_sequence_1_timer == 0)
    {
        g_wmap_land_effect_19_sequence_1_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_1_end, g_wmap_land_effect_19_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_2, D_800D6B0C, 0x4, g_wmap_land_effect_19_sequence_2_step, g_wmap_land_effect_19_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_2_reset, g_wmap_land_effect_19_sequence_2_step, g_wmap_land_effect_19_sequence_2_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_19_sequence_2_step_01(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 4;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0;
    g_wmap_land_effect_19_sequence_2_timer = 0x88;
    g_wmap_land_effect_19_sequence_2_step += 1;
    wmap_land_effect_19_sequence_2_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_19_sequence_2_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x1F, 0x8, 0);
    if (--g_wmap_land_effect_19_sequence_2_timer == 0)
    {
        g_wmap_land_effect_19_sequence_2_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_2_end, g_wmap_land_effect_19_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_3, D_800D6B1C, 0x4, g_wmap_land_effect_19_sequence_3_step, g_wmap_land_effect_19_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_3_reset, g_wmap_land_effect_19_sequence_3_step, g_wmap_land_effect_19_sequence_3_timer)

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_19_sequence_3_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    g_wmap_land_effect_19_sequence_3_timer = 0x40;
    g_wmap_land_effect_19_sequence_3_step += 1;
    wmap_land_effect_19_sequence_3_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_3_end, g_wmap_land_effect_19_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_4, D_800D6B2C, 0x4, g_wmap_land_effect_19_sequence_4_step, g_wmap_land_effect_19_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_4_reset, g_wmap_land_effect_19_sequence_4_step, g_wmap_land_effect_19_sequence_4_timer)

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_19_sequence_4_step_01(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    g_wmap_land_effect_19_sequence_4_timer = 0x80;
    g_wmap_land_effect_19_sequence_4_step += 1;
    wmap_land_effect_19_sequence_4_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_4_end, g_wmap_land_effect_19_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_5, D_800D6B3C, 0x6, g_wmap_land_effect_19_sequence_5_step, g_wmap_land_effect_19_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_5_reset, g_wmap_land_effect_19_sequence_5_step, g_wmap_land_effect_19_sequence_5_timer)

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void wmap_land_effect_19_sequence_5_step_01(void)
{
    D_80182DF0 = 1;
    D_8013B238 = D_80139258;
    g_wmap_land_effect_19_sequence_5_timer = 0x40;
    g_wmap_land_effect_19_sequence_5_step += 1;
    wmap_land_effect_19_sequence_5_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_19_sequence_5_step_03(void)
{
    g_wmap_land_effect_19_sequence_5_timer = 0x40;
    g_wmap_land_effect_19_sequence_5_step += 1;
    wmap_land_effect_19_sequence_5_step_04();
}

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_5_end, g_wmap_land_effect_19_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_6, D_800D6B54, 0x6, g_wmap_land_effect_19_sequence_6_step, g_wmap_land_effect_19_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_6_reset, g_wmap_land_effect_19_sequence_6_step, g_wmap_land_effect_19_sequence_6_timer)

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void wmap_land_effect_19_sequence_6_step_01(void)
{
    D_80182DF4 = 1;
    D_8013B240 = D_80139258;
    g_wmap_land_effect_19_sequence_6_timer = 0x14;
    g_wmap_land_effect_19_sequence_6_step += 1;
    wmap_land_effect_19_sequence_6_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_19_sequence_6_step_03(void)
{
    g_wmap_land_effect_19_sequence_6_timer = 0x40;
    g_wmap_land_effect_19_sequence_6_step += 1;
    wmap_land_effect_19_sequence_6_step_04();
}

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_6_end, g_wmap_land_effect_19_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_7, D_800D6B6C, 0x8, g_wmap_land_effect_19_sequence_7_step, g_wmap_land_effect_19_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_7_reset, g_wmap_land_effect_19_sequence_7_step, g_wmap_land_effect_19_sequence_7_timer)

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void wmap_land_effect_19_sequence_7_step_02(void)
{
    s32 remaining;

    func_8006B328(0xA, 0x14, 6, -1, -1, -6, 0, 0x19, -0x32, 0x64, -0x32, 0x64, 1, 0x7F, 0x7F, 0, 0);
    D_80182DE4 += 8;
    remaining = g_wmap_land_effect_19_sequence_7_timer - 1;
    g_wmap_land_effect_19_sequence_7_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_19_sequence_7_step += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_19_sequence_7_step_03(void)
{
    g_wmap_land_effect_19_sequence_7_timer = 0x3C;
    g_wmap_land_effect_19_sequence_7_step += 1;
    wmap_land_effect_19_sequence_7_step_04();
}

/** @brief World-map step: spawn an effect object then count down a timer. */
void wmap_land_effect_19_sequence_7_step_04(void)
{
    func_8006B328(0xA, 0x14, 6, -1, -1, -6, 0, 0x19, -0x32, 0x64, -0x32,
                  0x64, 1, 0x7F, 0x7F, 0, 0);
    if (--g_wmap_land_effect_19_sequence_7_timer == 0)
    {
        g_wmap_land_effect_19_sequence_7_step += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_19_sequence_7_step_05(void)
{
    D_800DCEA8 = 0;
    g_wmap_land_effect_19_sequence_7_timer = 0x14;
    g_wmap_land_effect_19_sequence_7_step += 1;
    wmap_land_effect_19_sequence_7_step_06();
}

/** @brief World-map step: spawn an effect object then count down a timer. */
void wmap_land_effect_19_sequence_7_step_06(void)
{
    func_8006B328(0xA, 0x14, 6, -1, -1, -6, 0, 0x19, -0x32, 0x64, -0x32,
                  0x64, 1, 0x7F, 0x7F, 0, 0);
    if (--g_wmap_land_effect_19_sequence_7_timer == 0)
    {
        g_wmap_land_effect_19_sequence_7_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_7_end, g_wmap_land_effect_19_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_8, D_800D6B8C, 0x6, g_wmap_land_effect_19_sequence_8_step, g_wmap_land_effect_19_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_8_reset, g_wmap_land_effect_19_sequence_8_step, g_wmap_land_effect_19_sequence_8_timer)

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_19_sequence_8_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0xC, 0xFF, 0x1, 0x8, 0, (s32)D_80139280);
    if (--g_wmap_land_effect_19_sequence_8_timer == 0)
    {
        g_wmap_land_effect_19_sequence_8_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_19_sequence_8_step_03(void)
{
    g_wmap_land_effect_19_sequence_8_timer = 0x20;
    D_80139280[5] = -1;
    g_wmap_land_effect_19_sequence_8_step += 1;
    wmap_land_effect_19_sequence_8_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_19_sequence_8_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0xC, 0xFF, 0x1, 0x8, 0, (s32)D_80139280);
    if (--g_wmap_land_effect_19_sequence_8_timer == 0)
    {
        g_wmap_land_effect_19_sequence_8_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_8_end, g_wmap_land_effect_19_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_9, D_800D6BA4, 0x8, g_wmap_land_effect_19_sequence_9_step, g_wmap_land_effect_19_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_9_reset, g_wmap_land_effect_19_sequence_9_step, g_wmap_land_effect_19_sequence_9_timer)

/** @brief World-map effect spawn: submit a request and tick a refcount. */
void wmap_land_effect_19_sequence_9_step_02(void)
{
    s32 c;

    func_8006B328(0x50, 0x8C, 2, -1, 0x14, 4, 0x168, 0x19, -0x32, 0x64, -0x32,
                  0x64, 0xB4, 0x81, 0x81, 8, 1);
    D_801B25D8 += 8;
    c = g_wmap_land_effect_19_sequence_9_timer - 1;
    g_wmap_land_effect_19_sequence_9_timer = c;
    if (c == 0)
    {
        g_wmap_land_effect_19_sequence_9_step += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_19_sequence_9_step_03(void)
{
    g_wmap_land_effect_19_sequence_9_timer = 0x18;
    g_wmap_land_effect_19_sequence_9_step += 1;
    wmap_land_effect_19_sequence_9_step_04();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_19_sequence_9_step_04(void)
{
    func_8006B328(0x50, 0x8C, 2, -1, 0x14, 4, 0x168, 0x19, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 1);
    if (--g_wmap_land_effect_19_sequence_9_timer == 0)
    {
        g_wmap_land_effect_19_sequence_9_step += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_19_sequence_9_step_05(void)
{
    D_800DCEAC = 0;
    g_wmap_land_effect_19_sequence_9_timer = 0x18;
    g_wmap_land_effect_19_sequence_9_step += 1;
    wmap_land_effect_19_sequence_9_step_06();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_19_sequence_9_step_06(void)
{
    func_8006B328(0x50, 0x8C, 2, -1, 0x14, 4, 0x168, 0x19, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 1);
    if (--g_wmap_land_effect_19_sequence_9_timer == 0)
    {
        g_wmap_land_effect_19_sequence_9_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_9_end, g_wmap_land_effect_19_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_10, D_800D6BC4, 0x8, g_wmap_land_effect_19_sequence_10_step, g_wmap_land_effect_19_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_10_reset, g_wmap_land_effect_19_sequence_10_step, g_wmap_land_effect_19_sequence_10_timer)

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void wmap_land_effect_19_sequence_10_step_02(void)
{
    s32 remaining;

    func_8006B328(0x96, 0xD2, 1, -1, -1, -3, 0, 8, -0xB4, 0x190, -0xA0, 0x190, 1, 1, 0x81, 4, 2);
    D_801B25DC += 8;
    remaining = g_wmap_land_effect_19_sequence_10_timer - 1;
    g_wmap_land_effect_19_sequence_10_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_19_sequence_10_step += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_19_sequence_10_step_03(void)
{
    g_wmap_land_effect_19_sequence_10_timer = 0x30;
    g_wmap_land_effect_19_sequence_10_step += 1;
    wmap_land_effect_19_sequence_10_step_04();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_19_sequence_10_step_04(void)
{
    func_8006B328(0x96, 0xD2, 1, -1, -1, -3, 0, 8, -0xB4, 0x190, -0xA0, 0x190, 1, 1, 0x81, 4, 2);
    if (--g_wmap_land_effect_19_sequence_10_timer == 0)
    {
        g_wmap_land_effect_19_sequence_10_step += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_19_sequence_10_step_05(void)
{
    D_800DCEB0 = 0;
    g_wmap_land_effect_19_sequence_10_timer = 0xA0;
    g_wmap_land_effect_19_sequence_10_step += 1;
    wmap_land_effect_19_sequence_10_step_06();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_19_sequence_10_step_06(void)
{
    func_8006B328(0x96, 0xD2, 1, -1, -1, -3, 0, 8, -0xB4, 0x190, -0xA0, 0x190, 1, 1, 0x81, 4, 2);
    if (--g_wmap_land_effect_19_sequence_10_timer == 0)
    {
        g_wmap_land_effect_19_sequence_10_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_10_end, g_wmap_land_effect_19_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_11, D_800D6BE4, 0x8, g_wmap_land_effect_19_sequence_11_step, g_wmap_land_effect_19_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_11_reset, g_wmap_land_effect_19_sequence_11_step, g_wmap_land_effect_19_sequence_11_timer)

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void wmap_land_effect_19_sequence_11_step_02(void)
{
    s32 remaining;

    func_8006B328(0xD2, 0xE6, 2, -1, -1, -8, 0x28, 8, -0xB4, 0x168, -0xB4, 0x168, 1, 1, 0x81, 4, 3);
    D_801B25D8 += 8;
    remaining = g_wmap_land_effect_19_sequence_11_timer - 1;
    g_wmap_land_effect_19_sequence_11_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_19_sequence_11_step += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_19_sequence_11_step_03(void)
{
    g_wmap_land_effect_19_sequence_11_timer = 0x28;
    g_wmap_land_effect_19_sequence_11_step += 1;
    wmap_land_effect_19_sequence_11_step_04();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_19_sequence_11_step_04(void)
{
    func_8006B328(0xD2, 0xE6, 2, -1, -1, -8, 0x28, 8, -0xB4, 0x168, -0xB4, 0x168, 1, 1, 0x81, 4, 3);
    if (--g_wmap_land_effect_19_sequence_11_timer == 0)
    {
        g_wmap_land_effect_19_sequence_11_step += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_19_sequence_11_step_05(void)
{
    D_800DCEB4 = 0;
    g_wmap_land_effect_19_sequence_11_timer = 0xA0;
    g_wmap_land_effect_19_sequence_11_step += 1;
    wmap_land_effect_19_sequence_11_step_06();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_19_sequence_11_step_06(void)
{
    func_8006B328(0xD2, 0xE6, 2, -1, -1, -8, 0x28, 8, -0xB4, 0x168, -0xB4, 0x168, 1, 1, 0x81, 4, 3);
    if (--g_wmap_land_effect_19_sequence_11_timer == 0)
    {
        g_wmap_land_effect_19_sequence_11_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_11_end, g_wmap_land_effect_19_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_effect_19_run_sequence_12, D_800D6C04, 0x4, g_wmap_land_effect_19_sequence_12_step, g_wmap_land_effect_19_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_effect_19_sequence_12_reset, g_wmap_land_effect_19_sequence_12_step, g_wmap_land_effect_19_sequence_12_timer)

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void wmap_land_effect_19_sequence_12_step_01(void)
{
    D_801399BC = D_80125538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.previous_sequence = -1;
    D_800D9370.resource_index = 0;
    D_800D9370.sequence = 0;
    D_800D9370.shade_step = 0;
    D_800D9370.target_shade = 0x80;
    D_800D9370.shade = 0x80;
    g_wmap_land_effect_19_sequence_12_timer = 0x82;
    g_wmap_land_effect_19_sequence_12_step += 1;
    wmap_land_effect_19_sequence_12_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_19_sequence_12_step_02(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x1F, 0x7, 0);
    if (--g_wmap_land_effect_19_sequence_12_timer == 0)
    {
        g_wmap_land_effect_19_sequence_12_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_19_sequence_12_end, g_wmap_land_effect_19_sequence_12_step)
