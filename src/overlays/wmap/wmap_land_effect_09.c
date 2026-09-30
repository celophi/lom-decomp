#include "wmap_main.h"
#include "wmap_land_effect_09.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"
#include "wmap_step_sequence.h"

void wmap_land_effect_09_sequence_5_step_02(void);
void wmap_land_effect_09_sequence_6_step_02(void);
void wmap_land_effect_09_sequence_7_step_02(void);
void wmap_land_effect_09_sequence_8_step_02(void);
void wmap_land_effect_09_sequence_9_step_02(void);
void wmap_land_effect_09_wait_idle_02(void);
void wmap_land_effect_09_step_03(void);
s32 wmap_land_effect_09_run_timeline(s32 arg0);
void wmap_land_effect_09_wait_idle_04(void);
void wmap_land_effect_09_end(void);
s32 wmap_land_effect_09_run_sequence_7(s32 arg0);
s32 wmap_land_effect_09_run_sequence_1(s32 arg0);
s32 wmap_land_effect_09_run_sequence_3(s32 arg0);
s32 wmap_land_effect_09_run_sequence_5(s32 arg0);
s32 wmap_land_effect_09_run_sequence_8(s32 arg0);
s32 wmap_land_effect_09_run_sequence_6(s32 arg0);
s32 wmap_land_effect_09_run_sequence_9(s32 arg0);
s32 wmap_land_effect_09_run_sequence_4(s32 arg0);
s32 wmap_land_effect_09_run_sequence_2(s32 arg0);
void wmap_land_effect_09_sequence_1_step_02(void);
void wmap_land_effect_09_sequence_2_step_02(void);
void wmap_land_effect_09_sequence_5_step_04(void);
void wmap_land_effect_09_sequence_6_step_04(void);
void wmap_land_effect_09_sequence_6_step_06(void);
void wmap_land_effect_09_sequence_7_step_04(void);
void wmap_land_effect_09_sequence_8_step_04(void);
void wmap_land_effect_09_sequence_9_step_04(void);

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

extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 g_wmap_land_effect_09_sequence_3_timer;
extern s32 D_80182DEC;
extern u8* D_8011CF1C;
extern s32 g_wmap_land_effect_09_sequence_4_timer;
extern s32 D_801B0FD0;
extern s32 g_wmap_land_effect_09_sequence_5_timer;
extern s32 D_800D9150;
extern s32 D_800DCEA8;
extern s32 D_80182DF0;
extern s32 g_wmap_land_effect_09_sequence_6_timer;
extern u8 D_80123538[];
extern s32 g_wmap_land_effect_09_sequence_7_timer;
extern s32 g_wmap_land_effect_09_sequence_8_timer;
extern s32 g_wmap_land_effect_09_sequence_9_timer;
extern s32 g_wmap_land_effect_09_timer;
extern void (*D_800D62A0[])(void);
extern void wmap_land_effect_09_step_03(void);
extern void wmap_land_effect_09_end(void);
extern s32 g_wmap_land_effect_09_timeline_timer;
extern void (*D_800D62B8[])(void);
extern s32 D_8013B208;
extern s32 D_80139244;
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 g_wmap_land_effect_09_sequence_1_timer;
extern void (*D_800D62F8[])(void);
extern u8* D_801399AC;
extern u8 D_8011F538[];
extern s32 g_wmap_land_effect_09_sequence_2_timer;
extern void (*D_800D6308[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern void wmap_land_effect_09_sequence_2_step_02(void);
extern void (*D_800D6318[])(void);
extern void (*D_800D6328[])(void);
extern void (*D_800D6338[])(void);
extern u8 D_800DB578[];
extern WmapAnimationSlot D_80139FE8[];
extern void (*D_800D6350[])(void);
extern void (*D_800D6370[])(void);
extern void (*D_800D6388[])(void);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern void wmap_land_effect_09_sequence_8_step_04(void);
extern void (*D_800D63A0[])(void);
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern u32 g_wmap_land_effect_09_sequence_3_step;
extern u32 g_wmap_land_effect_09_sequence_4_step;
extern u32 g_wmap_land_effect_09_sequence_5_step;
extern u8 D_80121538[];
extern u32 g_wmap_land_effect_09_sequence_6_step;
extern u32 g_wmap_land_effect_09_sequence_7_step;
extern u32 g_wmap_land_effect_09_sequence_8_step;
extern u32 g_wmap_land_effect_09_sequence_9_step;
extern u32 g_wmap_land_effect_09_step;
extern u32 g_wmap_land_effect_09_timeline_step;
extern u32 g_wmap_land_effect_09_sequence_1_step;
extern u32 g_wmap_land_effect_09_sequence_2_step;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;

extern SVECTOR D_80139258;
extern SVECTOR D_801B24A0;
extern SVECTOR D_801B24A8;

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
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_09_sequence_3_step_02(void)
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
    if (--g_wmap_land_effect_09_sequence_3_timer == 0)
    {
        g_wmap_land_effect_09_sequence_3_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_09_sequence_4_step_02(void)
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
    if (--g_wmap_land_effect_09_sequence_4_timer == 0)
    {
        g_wmap_land_effect_09_sequence_4_step += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_09_sequence_5_step_01(void)
{
    s32 i;

    D_801B0FD0 = 50;
    D_80139280[0x1] = 0;
    D_80139280[0x2] = 0;
    D_80139280[0x3] = 255;
    D_80139280[0x4] = 0;
    D_80139280[0x5] = 2;
    D_80139280[0x6] = 1000;
    D_80139280[0x7] = 200;
    D_80139280[0x8] = 8;
    D_80139280[0x9] = 0;
    D_80139280[0xA] = 18200;
    for (i = 0; i < 50; i++)
    {
        D_801AFBD0[i + D_80139280[0x7]].field_00 = 0;
        D_80139988[i + 204].data = D_80121538;
    }
    g_wmap_land_effect_09_sequence_5_timer = 100;
    g_wmap_land_effect_09_sequence_5_step++;
    wmap_land_effect_09_sequence_5_step_02();
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void wmap_land_effect_09_sequence_6_step_01(void)
{
    s32 i;
    WmapD94Entry *entry;

    i = 150;
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
    } while (i < 158);

    D_800D9150 = 3;
    g_wmap_land_effect_09_sequence_6_timer = 0x10;
    g_wmap_land_effect_09_sequence_6_step += 1;
    wmap_land_effect_09_sequence_6_step_02();
}

/** @brief Initialize the actor group and its animation resources, then advance. */
void wmap_land_effect_09_sequence_7_step_01(void)
{
    s32 i;
    WmapConfigA* config;

    D_801B0FD0 = 4;
    for (i = 140; i < 144; i++)
    {
        config = &D_800D9268[i];
        D_80139988[i].data = D_80123538;
        config->field_02 = 0;
        config->field_06 = 15;
        config->field_0E = 1;
        config->field_10 = -1;
        config->field_22 = 129;
        config->field_24 = 129;
        config->field_26 = 8;
        D_801AFBD0[i].field_00 = 1;
        D_801AFBD0[i].angle = i << 10;
        D_801AFBD0[i].field_04 = -20000;
        D_801AFBD0[i].field_08 = 990000;
        D_801AFBD0[i].field_0C = 9999;
        D_801AFBD0[i].field_0E = 240;
        D_801AFBD0[i].field_10 = 160;
    }
    g_wmap_land_effect_09_sequence_7_timer = 28;
    g_wmap_land_effect_09_sequence_7_step++;
    wmap_land_effect_09_sequence_7_step_02();
}

/** @brief World-map step: fill a spawn descriptor, clear its slot run, then advance. */
void wmap_land_effect_09_sequence_8_step_01(void)
{
    s32 i;

    D_801B0FD0 = 12;
    D_80139280[0x1F] = 1;
    D_80139280[0x20] = 3;
    D_80139280[0x21] = 64;
    D_80139280[0x22] = 30;
    D_80139280[0x23] = -2;
    D_80139280[0x24] = 1000;
    D_80139280[0x25] = 20;
    D_80139280[0x26] = 15;
    D_80139280[0x27] = 0;
    D_80139280[0x28] = 10000;
    for (i = 0; i < 12; i++)
    {
        D_801AFBD0[i + D_80139280[0x25]].field_00 = 0;
        D_80139988[i + 24].data = D_80123538;
    }
    g_wmap_land_effect_09_sequence_8_timer = 64;
    g_wmap_land_effect_09_sequence_8_step++;
    wmap_land_effect_09_sequence_8_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_09_sequence_9_step_01(void)
{
    s32 i;

    D_801B0FD0 = 20;
    D_80139280[0xB] = 0;
    D_80139280[0xC] = 0;
    D_80139280[0xF] = 1;
    D_80139280[0x10] = 120;
    D_80139280[0x11] = 100;
    D_80139280[0x12] = 15;
    D_80139280[0x13] = 2;
    D_80139280[0x14] = 18500;
    for (i = 0; i < 20; i++)
    {
        D_801AFBD0[i + D_80139280[0x11]].field_00 = 0;
        D_80139988[i + 104].data = D_80123538;
    }
    g_wmap_land_effect_09_sequence_9_timer = 20;
    g_wmap_land_effect_09_sequence_9_step++;
    wmap_land_effect_09_sequence_9_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_09_run, D_800D62A0, 0x6, g_wmap_land_effect_09_step, g_wmap_land_effect_09_timer)

WMAP_STEP_RESET(wmap_land_effect_09_reset, g_wmap_land_effect_09_step, g_wmap_land_effect_09_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_09_step_01, g_wmap_land_effect_09_step, wmap_run_land_focus, wmap_land_effect_09_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_09_wait_idle_02, g_wmap_land_effect_09_step, wmap_land_effect_09_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_09_step_03, g_wmap_land_effect_09_step, wmap_land_effect_09_run_timeline, wmap_land_effect_09_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_09_wait_idle_04, g_wmap_land_effect_09_step, wmap_land_effect_09_end)

WMAP_STEP_ADVANCE(wmap_land_effect_09_end, g_wmap_land_effect_09_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_timeline, D_800D62B8, 0x10, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_09_timeline_reset, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

/**
 * @brief Set up a world-map sub-scene: request assets and register its handler.
 */
void wmap_land_effect_09_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_play_sound(0x30, 0x80);
    wmap_start_sequence(wmap_land_effect_09_run_sequence_7);
    g_wmap_land_effect_09_timeline_timer = 0x28;
    g_wmap_land_effect_09_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_02, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

/** @brief Register four callbacks, set effect color and flags, and begin a one-tick delay. */
void wmap_land_effect_09_timeline_step_03(void)
{
    wmap_start_sequence(&wmap_land_effect_09_run_sequence_3);
    wmap_start_sequence(&wmap_land_effect_09_run_sequence_5);
    wmap_start_sequence(&wmap_land_effect_09_run_sequence_8);
    D_80139244 = 1;
    wmap_start_sequence(&wmap_land_effect_09_run_sequence_1);
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 4;
    g_wmap_land_effect_09_timeline_timer = 1;
    g_wmap_land_effect_09_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_04, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void wmap_land_effect_09_timeline_step_05(void)
{
    D_801ADAE0 = 1;
    g_wmap_land_effect_09_timeline_timer = 0x23;
    g_wmap_land_effect_09_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_06, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_09_timeline_step_07, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer,
                         wmap_land_effect_09_run_sequence_6, 0x8)

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_08, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_09_timeline_step_09, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer,
                         wmap_land_effect_09_run_sequence_9, 0x32)

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_10, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

/** @brief Register a callback, set world-map state and color, and begin a two-tick delay. */
void wmap_land_effect_09_timeline_step_11(void)
{
    wmap_start_sequence(&wmap_land_effect_09_run_sequence_4);
    D_80139244 = 0;
    g_wmap_backdrop_target_level = 0x10;
    wmap_start_map_tint(0x808080);
    g_wmap_land_effect_09_timeline_timer = 2;
    g_wmap_land_effect_09_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_12, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_09_timeline_step_13, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer,
                         wmap_land_effect_09_run_sequence_2, 0xAB)

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_14, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_09_timeline_finish, g_wmap_land_effect_09_timeline_step, D_80139290, D_8011D510, D_8011D530, D_8011D4FC)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_1, D_800D62F8, 0x4, g_wmap_land_effect_09_sequence_1_step, g_wmap_land_effect_09_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_1_reset, g_wmap_land_effect_09_sequence_1_step, g_wmap_land_effect_09_sequence_1_timer)

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void wmap_land_effect_09_sequence_1_step_01(void)
{
    D_801399AC = D_8011F538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.shade_step = 0;
    D_800D9318.target_shade = 0x81;
    D_800D9318.shade = 0x81;
    g_wmap_land_effect_09_sequence_1_timer = 0x4C;
    g_wmap_land_effect_09_sequence_1_step += 1;
    wmap_land_effect_09_sequence_1_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_09_sequence_1_step_02, g_wmap_land_effect_09_sequence_1_step, g_wmap_land_effect_09_sequence_1_timer, D_800D9318,
                              D_801399A8, g_wmap_focus_screen_position, 0xF, 0x4, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_1_end, g_wmap_land_effect_09_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_2, D_800D6308, 0x4, g_wmap_land_effect_09_sequence_2_step, g_wmap_land_effect_09_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_2_reset, g_wmap_land_effect_09_sequence_2_step, g_wmap_land_effect_09_sequence_2_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_09_sequence_2_step_01(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 8;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0;
    g_wmap_land_effect_09_sequence_2_timer = 0xAC;
    g_wmap_land_effect_09_sequence_2_step += 1;
    wmap_land_effect_09_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_09_sequence_2_step_02, g_wmap_land_effect_09_sequence_2_step, g_wmap_land_effect_09_sequence_2_timer, D_800D9344,
                              D_801399B0, g_wmap_focus_screen_position, 0x17, 0x8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_2_end, g_wmap_land_effect_09_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_3, D_800D6318, 0x4, g_wmap_land_effect_09_sequence_3_step, g_wmap_land_effect_09_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_3_reset, g_wmap_land_effect_09_sequence_3_step, g_wmap_land_effect_09_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_09_sequence_3_step_01, g_wmap_land_effect_09_sequence_3_step, g_wmap_land_effect_09_sequence_3_timer, D_801B24A0,
                     D_80139258, D_801B2650, D_80182DE8, 0x80, 0xAFC8, 0x40, wmap_land_effect_09_sequence_3_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_3_end, g_wmap_land_effect_09_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_4, D_800D6328, 0x4, g_wmap_land_effect_09_sequence_4_step, g_wmap_land_effect_09_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_4_reset, g_wmap_land_effect_09_sequence_4_step, g_wmap_land_effect_09_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_09_sequence_4_step_01, g_wmap_land_effect_09_sequence_4_step, g_wmap_land_effect_09_sequence_4_timer, D_801B24A8,
                     D_80139258, D_801B2478, D_80182DEC, 0x80, 0xAFC8, 0x40, wmap_land_effect_09_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_4_end, g_wmap_land_effect_09_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_5, D_800D6338, 0x6, g_wmap_land_effect_09_sequence_5_step, g_wmap_land_effect_09_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_5_reset, g_wmap_land_effect_09_sequence_5_step, g_wmap_land_effect_09_sequence_5_timer)

/** @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires. */
void wmap_land_effect_09_sequence_5_step_02(void)
{
    func_8006A2FC(D_800DB578, D_80139FE8, 0x32, 0x7F, 1, 4, 5, D_80139280);
    if (--g_wmap_land_effect_09_sequence_5_timer == 0)
    {
        g_wmap_land_effect_09_sequence_5_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_09_sequence_5_step_03(void)
{
    g_wmap_land_effect_09_sequence_5_timer = 0x40;
    D_80139280[5] = -1;
    g_wmap_land_effect_09_sequence_5_step += 1;
    wmap_land_effect_09_sequence_5_step_04();
}

/** @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires. */
void wmap_land_effect_09_sequence_5_step_04(void)
{
    func_8006A2FC(D_800DB578, D_80139FE8, 0x32, 0x7F, 1, 4, 5, D_80139280);
    if (--g_wmap_land_effect_09_sequence_5_timer == 0)
    {
        g_wmap_land_effect_09_sequence_5_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_5_end, g_wmap_land_effect_09_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_6, D_800D6350, 0x8, g_wmap_land_effect_09_sequence_6_step, g_wmap_land_effect_09_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_6_reset, g_wmap_land_effect_09_sequence_6_step, g_wmap_land_effect_09_sequence_6_timer)

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void wmap_land_effect_09_sequence_6_step_02(void)
{
    s32 remaining;

    func_8006B328(0x96, 0x9E, 3, -1, -1, -5, 0, 8, -0x50, 0xA0, -0x50, 0xA0, 1, 0x7F, 1, 4, 0);
    D_80182DF0 += 8;
    remaining = g_wmap_land_effect_09_sequence_6_timer - 1;
    g_wmap_land_effect_09_sequence_6_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_09_sequence_6_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_09_sequence_6_step_03, g_wmap_land_effect_09_sequence_6_step, g_wmap_land_effect_09_sequence_6_timer, 0x18,
                    wmap_land_effect_09_sequence_6_step_04)

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_09_sequence_6_step_04(void)
{
    func_8006B328(0x96, 0x9E, 3, -1, -1, -5, 0, 8, -0x50, 0xA0, -0x50, 0xA0, 1, 0x7F, 1, 4, 0);
    if (--g_wmap_land_effect_09_sequence_6_timer == 0)
    {
        g_wmap_land_effect_09_sequence_6_step += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_09_sequence_6_step_05(void)
{
    D_800DCEA8 = 0;
    g_wmap_land_effect_09_sequence_6_timer = 0x14;
    g_wmap_land_effect_09_sequence_6_step += 1;
    wmap_land_effect_09_sequence_6_step_06();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_09_sequence_6_step_06(void)
{
    func_8006B328(0x96, 0x9E, 3, -1, -1, -5, 0, 8, -0x50, 0xA0, -0x50, 0xA0, 1, 0x7F, 1, 4, 0);
    if (--g_wmap_land_effect_09_sequence_6_timer == 0)
    {
        g_wmap_land_effect_09_sequence_6_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_6_end, g_wmap_land_effect_09_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_7, D_800D6370, 0x6, g_wmap_land_effect_09_sequence_7_step, g_wmap_land_effect_09_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_7_reset, g_wmap_land_effect_09_sequence_7_step, g_wmap_land_effect_09_sequence_7_timer)

/** @brief World-map step handler: draw the sprite element, then countdown-advance the step. */
void wmap_land_effect_09_sequence_7_step_02(void)
{
    func_8006B6EC(0x8C, 0x90, 0xF, -6, 4);
    if (--g_wmap_land_effect_09_sequence_7_timer == 0)
    {
        g_wmap_land_effect_09_sequence_7_step += 1;
    }
}

/**
 * @brief Reset a range of world-map actor configs, arm the timer, and advance the step.
 */
void wmap_land_effect_09_sequence_7_step_03(void)
{
    s32 i;

    for (i = 0x8C; i < 0x90; i++)
    {
        D_800D9268[i].target_shade = 0;
        D_800D9268[i].shade_step = 8;
    }
    g_wmap_land_effect_09_sequence_7_timer = 0x10;
    g_wmap_land_effect_09_sequence_7_step += 1;
    wmap_land_effect_09_sequence_7_step_04();
}

/** @brief World-map step handler: draw the sprite element, then countdown-advance the step. */
void wmap_land_effect_09_sequence_7_step_04(void)
{
    func_8006B6EC(0x8C, 0x90, 0xF, -6, 4);
    if (--g_wmap_land_effect_09_sequence_7_timer == 0)
    {
        g_wmap_land_effect_09_sequence_7_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_7_end, g_wmap_land_effect_09_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_8, D_800D6388, 0x6, g_wmap_land_effect_09_sequence_8_step, g_wmap_land_effect_09_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_8_reset, g_wmap_land_effect_09_sequence_8_step, g_wmap_land_effect_09_sequence_8_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_09_sequence_8_step_02(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0xC, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--g_wmap_land_effect_09_sequence_8_timer == 0)
    {
        g_wmap_land_effect_09_sequence_8_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_09_sequence_8_step_03(void)
{
    g_wmap_land_effect_09_sequence_8_timer = 0x20;
    D_80139280[35] = -1;
    g_wmap_land_effect_09_sequence_8_step += 1;
    wmap_land_effect_09_sequence_8_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_09_sequence_8_step_04(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0xC, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--g_wmap_land_effect_09_sequence_8_timer == 0)
    {
        g_wmap_land_effect_09_sequence_8_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_8_end, g_wmap_land_effect_09_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_9, D_800D63A0, 0x6, g_wmap_land_effect_09_sequence_9_step, g_wmap_land_effect_09_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_9_reset, g_wmap_land_effect_09_sequence_9_step, g_wmap_land_effect_09_sequence_9_timer)

/**
 * @brief World-map step handler: submit a batched sprite draw, then advance the
 *        sequence once its frame counter expires.
 */
void wmap_land_effect_09_sequence_9_step_02(void)
{
    func_8006A2FC(D_800DA448, D_80139CC8, 0x14, 0x7F, 0x7F, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--g_wmap_land_effect_09_sequence_9_timer == 0)
    {
        g_wmap_land_effect_09_sequence_9_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_09_sequence_9_step_03(void)
{
    g_wmap_land_effect_09_sequence_9_timer = 0x20;
    D_80139280[15] = -1;
    g_wmap_land_effect_09_sequence_9_step += 1;
    wmap_land_effect_09_sequence_9_step_04();
}

/**
 * @brief World-map step handler: submit a batched sprite draw, then advance the
 *        sequence once its frame counter expires.
 */
void wmap_land_effect_09_sequence_9_step_04(void)
{
    func_8006A2FC(D_800DA448, D_80139CC8, 0x14, 0x7F, 0x7F, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--g_wmap_land_effect_09_sequence_9_timer == 0)
    {
        g_wmap_land_effect_09_sequence_9_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_9_end, g_wmap_land_effect_09_sequence_9_step)
