#include "wmap_main.h"
#include "wmap_land_effect_09.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

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
extern s32 D_801B2BAC;
extern s32 D_80182DEC;
extern u8* D_8011CF1C;
extern s32 D_801B2BB4;
extern s32 D_801B0FD0;
extern s32 D_801B2BBC;
extern s32 D_800D9150;
extern s32 D_800DCEA8;
extern s32 D_80182DF0;
extern s32 D_801B2BC4;
extern u8 D_80123538[];
extern s32 D_801B2BCC;
extern s32 D_801B2BD4;
extern s32 D_801B2BDC;
extern s32 D_801B2B8C;
extern void (*D_800D62A0[])(void);
extern s32 D_8013B20C;
extern void wmap_land_effect_09_step_03(void);
extern void wmap_land_effect_09_end(void);
extern s32 D_801B2B94;
extern void (*D_800D62B8[])(void);
extern s32 D_8013B208;
extern s32 D_80139244;
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2B9C;
extern void (*D_800D62F8[])(void);
extern u8* D_801399AC;
extern u8 D_8011F538[];
extern s32 D_801B2BA4;
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
extern u32 D_801B2BA8;
extern u32 D_801B2BB0;
extern u32 D_801B2BB8;
extern u8 D_80121538[];
extern u32 D_801B2BC0;
extern u32 D_801B2BC8;
extern u32 D_801B2BD0;
extern u32 D_801B2BD8;
extern u32 D_801B2B88;
extern u32 D_801B2B90;
extern u32 D_801B2B98;
extern u32 D_801B2BA0;

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
    if (--D_801B2BAC == 0)
    {
        D_801B2BA8 += 1;
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
    if (--D_801B2BB4 == 0)
    {
        D_801B2BB0 += 1;
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
    D_801B2BBC = 100;
    D_801B2BB8++;
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
    D_801B2BC4 = 0x10;
    D_801B2BC0 += 1;
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
    D_801B2BCC = 28;
    D_801B2BC8++;
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
    D_801B2BD4 = 64;
    D_801B2BD0++;
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
    D_801B2BDC = 20;
    D_801B2BD8++;
    wmap_land_effect_09_sequence_9_step_02();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_09_run(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2B88 = 1;
        D_801B2B8C = 1;
        return 1;
    }

    if (D_801B2B88 < 0x6)
    {
        D_800D62A0[D_801B2B88]();
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
void wmap_land_effect_09_reset(void)
{
    D_801B2B88 = 1;
    D_801B2B8C = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_09_step_01(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    D_8013B20C = 1;
    D_801B2B88 += 1;
    wmap_land_effect_09_wait_idle_02();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_09_wait_idle_02(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2B88 += 1;
        wmap_land_effect_09_step_03();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_09_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_09_run_timeline);
    D_8013B20C = 1;
    D_801B2B88 += 1;
    wmap_land_effect_09_wait_idle_04();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_09_wait_idle_04(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2B88 += 1;
        wmap_land_effect_09_end();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_09_end(void)
{
    D_801B2B88 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_09_run_timeline(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2B90 = 1;
        D_801B2B94 = 1;
        return 1;
    }

    if (D_801B2B90 < 0x10)
    {
        D_800D62B8[D_801B2B90]();
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
void wmap_land_effect_09_timeline_reset(void)
{
    D_801B2B90 = 1;
    D_801B2B94 = 1;
}

/**
 * @brief Set up a world-map sub-scene: request assets and register its handler.
 */
void wmap_land_effect_09_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_play_sound(0x30, 0x80);
    wmap_start_sequence(wmap_land_effect_09_run_sequence_7);
    D_801B2B94 = 0x28;
    D_801B2B90 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_09_timeline_wait_02(void)
{
    if (--D_801B2B94 == 0)
    {
        D_801B2B90 += 1;
    }
}

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
    D_801B2B94 = 1;
    D_801B2B90 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_09_timeline_wait_04(void)
{
    if (--D_801B2B94 == 0)
    {
        D_801B2B90 += 1;
    }
}

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void wmap_land_effect_09_timeline_step_05(void)
{
    D_801ADAE0 = 1;
    D_801B2B94 = 0x23;
    D_801B2B90 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_09_timeline_wait_06(void)
{
    if (--D_801B2B94 == 0)
    {
        D_801B2B90 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_09_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_09_run_sequence_6);
    D_801B2B94 = 0x8;
    D_801B2B90 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_09_timeline_wait_08(void)
{
    if (--D_801B2B94 == 0)
    {
        D_801B2B90 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_09_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_09_run_sequence_9);
    D_801B2B94 = 0x32;
    D_801B2B90 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_09_timeline_wait_10(void)
{
    if (--D_801B2B94 == 0)
    {
        D_801B2B90 += 1;
    }
}

/** @brief Register a callback, set world-map state and color, and begin a two-tick delay. */
void wmap_land_effect_09_timeline_step_11(void)
{
    wmap_start_sequence(&wmap_land_effect_09_run_sequence_4);
    D_80139244 = 0;
    g_wmap_backdrop_target_level = 0x10;
    wmap_start_map_tint(0x808080);
    D_801B2B94 = 2;
    D_801B2B90 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_09_timeline_wait_12(void)
{
    if (--D_801B2B94 == 0)
    {
        D_801B2B90 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_09_timeline_step_13(void)
{
    wmap_start_sequence(wmap_land_effect_09_run_sequence_2);
    D_801B2B94 = 0xAB;
    D_801B2B90 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_09_timeline_wait_14(void)
{
    if (--D_801B2B94 == 0)
    {
        D_801B2B90 += 1;
    }
}

void wmap_land_effect_09_timeline_finish(void)
{
    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B2B90 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_09_run_sequence_1(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2B98 = 1;
        D_801B2B9C = 1;
        return 1;
    }

    if (D_801B2B98 < 0x4)
    {
        D_800D62F8[D_801B2B98]();
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
void wmap_land_effect_09_sequence_1_reset(void)
{
    D_801B2B98 = 1;
    D_801B2B9C = 1;
}

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
    D_801B2B9C = 0x4C;
    D_801B2B98 += 1;
    wmap_land_effect_09_sequence_1_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_09_sequence_1_step_02(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0xF, 0x4, 0);
    if (--D_801B2B9C == 0)
    {
        D_801B2B98 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_09_sequence_1_end(void)
{
    D_801B2B98 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_09_run_sequence_2(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BA0 = 1;
        D_801B2BA4 = 1;
        return 1;
    }

    if (D_801B2BA0 < 0x4)
    {
        D_800D6308[D_801B2BA0]();
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
void wmap_land_effect_09_sequence_2_reset(void)
{
    D_801B2BA0 = 1;
    D_801B2BA4 = 1;
}

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
    D_801B2BA4 = 0xAC;
    D_801B2BA0 += 1;
    wmap_land_effect_09_sequence_2_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_09_sequence_2_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x17, 0x8, 0);
    if (--D_801B2BA4 == 0)
    {
        D_801B2BA0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_09_sequence_2_end(void)
{
    D_801B2BA0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_09_run_sequence_3(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BA8 = 1;
        D_801B2BAC = 1;
        return 1;
    }

    if (D_801B2BA8 < 0x4)
    {
        D_800D6318[D_801B2BA8]();
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
void wmap_land_effect_09_sequence_3_reset(void)
{
    D_801B2BA8 = 1;
    D_801B2BAC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_09_sequence_3_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B2BAC = 0x40;
    D_801B2BA8 += 1;
    wmap_land_effect_09_sequence_3_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_09_sequence_3_end(void)
{
    D_801B2BA8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_09_run_sequence_4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BB0 = 1;
        D_801B2BB4 = 1;
        return 1;
    }

    if (D_801B2BB0 < 0x4)
    {
        D_800D6328[D_801B2BB0]();
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
void wmap_land_effect_09_sequence_4_reset(void)
{
    D_801B2BB0 = 1;
    D_801B2BB4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_09_sequence_4_step_01(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    D_801B2BB4 = 0x40;
    D_801B2BB0 += 1;
    wmap_land_effect_09_sequence_4_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_09_sequence_4_end(void)
{
    D_801B2BB0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_09_run_sequence_5(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BB8 = 1;
        D_801B2BBC = 1;
        return 1;
    }

    if (D_801B2BB8 < 0x6)
    {
        D_800D6338[D_801B2BB8]();
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
void wmap_land_effect_09_sequence_5_reset(void)
{
    D_801B2BB8 = 1;
    D_801B2BBC = 1;
}

/** @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires. */
void wmap_land_effect_09_sequence_5_step_02(void)
{
    func_8006A2FC(D_800DB578, D_80139FE8, 0x32, 0x7F, 1, 4, 5, D_80139280);
    if (--D_801B2BBC == 0)
    {
        D_801B2BB8 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_09_sequence_5_step_03(void)
{
    D_801B2BBC = 0x40;
    D_80139280[5] = -1;
    D_801B2BB8 += 1;
    wmap_land_effect_09_sequence_5_step_04();
}

/** @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires. */
void wmap_land_effect_09_sequence_5_step_04(void)
{
    func_8006A2FC(D_800DB578, D_80139FE8, 0x32, 0x7F, 1, 4, 5, D_80139280);
    if (--D_801B2BBC == 0)
    {
        D_801B2BB8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_09_sequence_5_end(void)
{
    D_801B2BB8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_09_run_sequence_6(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BC0 = 1;
        D_801B2BC4 = 1;
        return 1;
    }

    if (D_801B2BC0 < 0x8)
    {
        D_800D6350[D_801B2BC0]();
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
void wmap_land_effect_09_sequence_6_reset(void)
{
    D_801B2BC0 = 1;
    D_801B2BC4 = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void wmap_land_effect_09_sequence_6_step_02(void)
{
    s32 remaining;

    func_8006B328(0x96, 0x9E, 3, -1, -1, -5, 0, 8, -0x50, 0xA0, -0x50, 0xA0, 1, 0x7F, 1, 4, 0);
    D_80182DF0 += 8;
    remaining = D_801B2BC4 - 1;
    D_801B2BC4 = remaining;
    if (remaining == 0)
    {
        D_801B2BC0 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_09_sequence_6_step_03(void)
{
    D_801B2BC4 = 0x18;
    D_801B2BC0 += 1;
    wmap_land_effect_09_sequence_6_step_04();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_09_sequence_6_step_04(void)
{
    func_8006B328(0x96, 0x9E, 3, -1, -1, -5, 0, 8, -0x50, 0xA0, -0x50, 0xA0, 1, 0x7F, 1, 4, 0);
    if (--D_801B2BC4 == 0)
    {
        D_801B2BC0 += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_09_sequence_6_step_05(void)
{
    D_800DCEA8 = 0;
    D_801B2BC4 = 0x14;
    D_801B2BC0 += 1;
    wmap_land_effect_09_sequence_6_step_06();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_09_sequence_6_step_06(void)
{
    func_8006B328(0x96, 0x9E, 3, -1, -1, -5, 0, 8, -0x50, 0xA0, -0x50, 0xA0, 1, 0x7F, 1, 4, 0);
    if (--D_801B2BC4 == 0)
    {
        D_801B2BC0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_09_sequence_6_end(void)
{
    D_801B2BC0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_09_run_sequence_7(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BC8 = 1;
        D_801B2BCC = 1;
        return 1;
    }

    if (D_801B2BC8 < 0x6)
    {
        D_800D6370[D_801B2BC8]();
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
void wmap_land_effect_09_sequence_7_reset(void)
{
    D_801B2BC8 = 1;
    D_801B2BCC = 1;
}

/** @brief World-map step handler: draw the sprite element, then countdown-advance the step. */
void wmap_land_effect_09_sequence_7_step_02(void)
{
    func_8006B6EC(0x8C, 0x90, 0xF, -6, 4);
    if (--D_801B2BCC == 0)
    {
        D_801B2BC8 += 1;
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
    D_801B2BCC = 0x10;
    D_801B2BC8 += 1;
    wmap_land_effect_09_sequence_7_step_04();
}

/** @brief World-map step handler: draw the sprite element, then countdown-advance the step. */
void wmap_land_effect_09_sequence_7_step_04(void)
{
    func_8006B6EC(0x8C, 0x90, 0xF, -6, 4);
    if (--D_801B2BCC == 0)
    {
        D_801B2BC8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_09_sequence_7_end(void)
{
    D_801B2BC8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_09_run_sequence_8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BD0 = 1;
        D_801B2BD4 = 1;
        return 1;
    }

    if (D_801B2BD0 < 0x6)
    {
        D_800D6388[D_801B2BD0]();
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
void wmap_land_effect_09_sequence_8_reset(void)
{
    D_801B2BD0 = 1;
    D_801B2BD4 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_09_sequence_8_step_02(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0xC, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2BD4 == 0)
    {
        D_801B2BD0 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_09_sequence_8_step_03(void)
{
    D_801B2BD4 = 0x20;
    D_80139280[35] = -1;
    D_801B2BD0 += 1;
    wmap_land_effect_09_sequence_8_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_09_sequence_8_step_04(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0xC, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2BD4 == 0)
    {
        D_801B2BD0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_09_sequence_8_end(void)
{
    D_801B2BD0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_09_run_sequence_9(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BD8 = 1;
        D_801B2BDC = 1;
        return 1;
    }

    if (D_801B2BD8 < 0x6)
    {
        D_800D63A0[D_801B2BD8]();
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
void wmap_land_effect_09_sequence_9_reset(void)
{
    D_801B2BD8 = 1;
    D_801B2BDC = 1;
}

/**
 * @brief World-map step handler: submit a batched sprite draw, then advance the
 *        sequence once its frame counter expires.
 */
void wmap_land_effect_09_sequence_9_step_02(void)
{
    func_8006A2FC(D_800DA448, D_80139CC8, 0x14, 0x7F, 0x7F, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2BDC == 0)
    {
        D_801B2BD8 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_09_sequence_9_step_03(void)
{
    D_801B2BDC = 0x20;
    D_80139280[15] = -1;
    D_801B2BD8 += 1;
    wmap_land_effect_09_sequence_9_step_04();
}

/**
 * @brief World-map step handler: submit a batched sprite draw, then advance the
 *        sequence once its frame counter expires.
 */
void wmap_land_effect_09_sequence_9_step_04(void)
{
    func_8006A2FC(D_800DA448, D_80139CC8, 0x14, 0x7F, 0x7F, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2BDC == 0)
    {
        D_801B2BD8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_09_sequence_9_end(void)
{
    D_801B2BD8 += 1;
}
