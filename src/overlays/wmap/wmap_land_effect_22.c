#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_land_effect_22.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

void wmap_land_effect_22_sequence_12_step_02(void);
void wmap_land_effect_22_sequence_13_step_02(void);
void wmap_land_effect_22_sequence_14_step_02(void);
s32 wmap_land_effect_22_run_timeline(s32 arg0);
void wmap_land_effect_22_wait_idle(void);
void wmap_land_effect_22_step_03(void);
s32 wmap_land_effect_22_run_sequence_13(s32 arg0);
s32 wmap_land_effect_22_run_sequence_6(s32 arg0);
s32 wmap_land_effect_22_run_sequence_12(s32 arg0);
s32 wmap_land_effect_22_run_sequence_2(s32 arg0);
s32 wmap_land_effect_22_run_sequence_10(s32 arg0);
s32 wmap_land_effect_22_run_sequence_7(s32 arg0);
s32 wmap_land_effect_22_run_sequence_1(s32 arg0);
s32 wmap_land_effect_22_run_sequence_9(s32 arg0);
s32 wmap_land_effect_22_run_sequence_11(s32 arg0);
s32 wmap_land_effect_22_run_sequence_3(s32 arg0);
s32 wmap_land_effect_22_run_sequence_14(s32 arg0);
s32 wmap_land_effect_22_run_sequence_4(s32 arg0);
s32 wmap_land_effect_22_run_sequence_5(s32 arg0);
s32 wmap_land_effect_22_run_sequence_8(s32 arg0);
void wmap_land_effect_22_sequence_1_step_02(void);
void wmap_land_effect_22_sequence_5_step_02(void);
void wmap_land_effect_22_sequence_7_step_02(void);
void wmap_land_effect_22_sequence_8_step_02(void);
void wmap_land_effect_22_sequence_12_step_04(void);
void wmap_land_effect_22_sequence_13_step_04(void);
void wmap_land_effect_22_sequence_14_step_04(void);

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

/** @brief Record with a leading value and a 40-byte stride. */
typedef struct
{
    s32 value;
    u8 unknown_4[36];
} WmapValueRecord;

extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern u8* D_8011CF1C;
extern s32 D_801B2D74;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2D6C;
extern s32 D_80182DF0;
extern s32 D_801B2D7C;
extern int rand(void);
extern s16 D_801398C8[];
extern s32 D_80182D48[];
extern s32 D_80139268;
extern s32 D_801B2D8C;
extern s32* D_8011CF2C;
extern s32 D_8013923C;
extern s32 D_80182DE4;
extern s32 D_801B2DA4;
extern s32 D_80139264;
extern s32 D_801B2DAC;
extern s32 D_801B25D8;
extern s32 D_80139234;
extern s32 D_80139260;
extern s32 D_80182DF4;
extern s32 D_801B2DB4;
extern s32 D_801B0FD0;
extern u8 D_8011D538[];
extern s32 D_801B2DBC;
extern s32 D_801B2DC4;
extern s32 D_801B2DCC;
extern s32 D_801B2D54;
extern void (*D_800D6904[])(void);
extern s32 D_80139978;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern s32 D_80139228;
extern s32 D_801B2D5C;
extern void (*D_800D6914[])(void);
extern s32 D_8013B208;
extern s32 D_80139244;
extern s32 D_800DBE70;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern WmapValueRecord D_80139290[][6];
extern s32 D_801B2D64;
extern void (*D_800D696C[])(void);
extern u8* D_801399B4;
extern u8 D_8011F538[];
extern void (*D_800D697C[])(void);
extern void (*D_800D698C[])(void);
extern void (*D_800D699C[])(void);
extern s32 D_801B2D84;
extern void (*D_800D69AC[])(void);
extern u8 D_80121538[];
extern u8* D_801399BC;
extern void wmap_land_effect_22_sequence_5_step_02(void);
extern void (*D_800D69BC[])(void);
extern s32 D_801B2D94;
extern void (*D_800D69D4[])(void);
extern u8 D_80123538[];
extern u8* D_801399C4;
extern s32 D_801B2D9C;
extern void (*D_800D69E4[])(void);
extern u8* D_801399CC;
extern void (*D_800D69F4[])(void);
extern void (*D_800D6A0C[])(void);
extern void (*D_800D6A24[])(void);
extern void (*D_800D6A3C[])(void);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern void (*D_800D6A54[])(void);
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern void wmap_land_effect_22_sequence_13_step_04(void);
extern void (*D_800D6A6C[])(void);
extern u8 D_800DAB28[];
extern u8 D_80139E08[];
extern void wmap_land_effect_22_sequence_14_step_04(void);

extern u32 D_801B2D70;
extern u32 D_801B2D68;
extern u32 D_801B2D78;
extern u32 D_801B2D88;
extern u32 D_801B2DA0;
extern void *D_8011CF28;
extern u32 D_801B2DA8;
extern u32 D_801B2DB0;
extern u32 D_801B2DB8;
extern u32 D_801B2DC0;
extern u32 D_801B2DC8;
extern u32 D_801B2D50;
extern u32 D_801B2D58;
extern u32 D_801B2D60;
extern u32 D_801B2D80;
extern u32 D_801B2D90;
extern u32 D_801B2D98;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;
extern VECTOR D_80139870;

extern SVECTOR D_80139258;
extern SVECTOR D_801B2498;
extern SVECTOR D_8013B240;
extern SVECTOR D_801B24A0;
extern SVECTOR D_8013B238;
extern SVECTOR D_801B2670;
extern SVECTOR D_801B24A8;

extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370;
extern WmapSpriteActor D_800D939C;
extern WmapSpriteActor D_800D93C8;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8;
extern WmapAnimationSlot D_801399C0;
extern WmapAnimationSlot D_801399C8;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* D_80139280;

extern WmapSlot14 D_801AFBD0[];

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_22_sequence_2_step_02(void)
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
    if (--D_801B2D6C == 0)
    {
        D_801B2D68 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_22_sequence_3_step_02(void)
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
    if (--D_801B2D74 == 0)
    {
        D_801B2D70 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_22_sequence_4_step_02(void)
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
        wmap_draw_model_default(D_8011CF1C, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DF0);
        D_80182DF0 -= 0x2;
        if (D_80182DF0 < 0)
        {
            D_80182DF0 = 0;
        }
    }

    PopMatrix();
    if (--D_801B2D7C == 0)
    {
        D_801B2D78 += 1;
    }
}

/**
 * @brief World-map step handler: jitter the actor position with a ramping random
 *        amplitude, then countdown-advance the step.
 */
void wmap_land_effect_22_sequence_6_step_02(void)
{
    D_801398C8[0] = rand() * D_80139268 / 16 >> 15;
    D_801398C8[1] = rand() * D_80139268 / 16 >> 15;
    D_80182D48[0] = rand() * D_80139268 / 16 >> 15;
    D_80182D48[1] = rand() * D_80139268 / 16 >> 15;
    if (D_80139268 < 0x80)
    {
        D_80139268 += 4;
    }
    if (--D_801B2D8C == 0)
    {
        D_801B2D88 += 1;
    }
}

/**
 * @brief World-map step handler: jitter the actor with a random amplitude, and when
 *        the amplitude ramp expires, zero the offsets; otherwise countdown-advance.
 */
void wmap_land_effect_22_sequence_6_step_04(void)
{
    D_801398C8[0] = rand() * D_80139268 / 16 >> 15;
    D_801398C8[1] = rand() * D_80139268 / 16 >> 15;
    D_80182D48[0] = rand() * D_80139268 / 16 >> 15;
    D_80182D48[1] = rand() * D_80139268 / 16 >> 15;
    if (--D_80139268 < 0)
    {
        D_80182D48[1] = 0;
        D_80182D48[0] = 0;
        D_801398C8[1] = 0;
        D_801398C8[0] = 0;
        D_801B2D88 += 1;
    }
    else if (--D_801B2D8C == 0)
    {
        D_801B2D88 += 1;
    }
}

/**
 * @brief World-map step handler: render the animated actor, ramp its size up to a
 *        cap, scroll the shadow field, then advance when the frame counter expires.
 */
void wmap_land_effect_22_sequence_9_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model(D_8011CF2C, (D_8013923C / 0x10) & 3, 0xA, 0x35, 0x7800, 0x1001, D_80182DE4, 0, 0x14, -1);
    D_8013923C += 0x10;
    value = D_80182DE4 + 2;
    D_80182DE4 = value;
    if (value >= 0x82)
    {
        D_80182DE4 = 0x81;
    }
    timer = D_801B2DA4;
    D_801B2498.vz += 0x38;
    next_timer = timer - 1;
    D_801B2DA4 = next_timer;
    if (next_timer == 0)
    {
        D_801B2DA0++;
    }
}

/**
 * @brief World-map step handler: render the animated actor, ramp its size down to a
 *        floor, scroll the shadow field, then advance when the frame counter expires.
 */
void wmap_land_effect_22_sequence_9_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model(D_8011CF2C, (D_8013923C / 0x10) & 3, 0xA, 0x35, 0x7800, 0x1001, D_80182DE4, 0, 0x14, -1);
    value = D_80182DE4 - 2;
    D_80182DE4 = value;
    if (value < 0)
    {
        D_80182DE4 = 0;
    }
    D_8013923C += 0x10;
    timer = D_801B2DA4;
    D_801B2498.vz += 0x38;
    next_timer = timer - 1;
    D_801B2DA4 = next_timer;
    if (next_timer == 0)
    {
        D_801B2DA0++;
    }
}

/** @brief Draw and brighten the rotating effect while reducing its scale. */
void wmap_land_effect_22_sequence_10_step_02(void)
{
    s32 scale;
    s32 intensity;
    s32 remaining;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2670);
    wmap_draw_model(D_8011CF28, D_80139264 & 3, 4, 54, 0x78C0, 0x1001, D_801B25D8, 0, 0, D_80139234 / 16);
    scale = D_80139234 - 32;
    D_80139234 = scale;
    intensity = D_801B25D8 + 8;
    D_801B25D8 = intensity;
    D_80139264++;
    if (intensity >= 98)
    {
        D_801B25D8 = 97;
    }
    if (scale < 16)
    {
        D_80139234 = 16;
    }
    PopMatrix();
    remaining = D_801B2DAC - 1;
    D_801B2670.vz = (u16)(D_801B2670.vz + 30);
    D_801B2DAC = remaining;
    if (remaining == 0)
    {
        D_801B2DA8++;
    }
}

/** @brief Draw and fade the rotating effect, then advance its countdown. */
void wmap_land_effect_22_sequence_10_step_04(void)
{
    s32 intensity;
    s32 remaining;

    if (D_801B25D8 != 0)
    {
        PushMatrix();
        wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2670);
        wmap_draw_model(D_8011CF28, D_80139264 & 3, 4, 54, 0x78C0, 0x1001, D_801B25D8, 0, 0, D_80139234 / 16);
        intensity = D_801B25D8 - 8;
        D_801B25D8 = intensity;
        D_80139264++;
        if (intensity < 0)
        {
            D_801B25D8 = 0;
        }
        PopMatrix();
        D_801B2670.vz = (u16)(D_801B2670.vz + 30);
    }
    remaining = D_801B2DAC - 1;
    D_801B2DAC = remaining;
    if (remaining == 0)
    {
        D_801B2DA8++;
    }
}

/** @brief Advance the world-map effect and its sequence state. */
void wmap_land_effect_22_sequence_11_step_02(void)
{
    s32 remaining;
    s32 intensity;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B240);
    wmap_draw_model(D_8011CF28, D_80139260 & 3, 4, 0x36, 0x7900, 0x1001, D_80182DF4, 0, 0, -1);
    D_80139260 += 1;
    intensity = D_80182DF4 + 2;
    D_80182DF4 = intensity;
    if (intensity >= 0x62)
    {
        D_80182DF4 = 0x61;
    }
    remaining = D_801B2DB4 - 1;
    D_8013B240.vz = (u16) (D_8013B240.vz + 0x14);
    D_801B2DB4 = remaining;
    if (remaining == 0)
    {
        D_801B2DB0 += 1;
    }
}

/** @brief Advance the world-map effect and its sequence state. */
void wmap_land_effect_22_sequence_11_step_04(void)
{
    s32 remaining;
    s32 intensity;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B240);
    wmap_draw_model(D_8011CF28, D_80139260 & 3, 4, 0x36, 0x7900, 0x1001, D_80182DF4, 0, 0, -1);
    intensity = D_80182DF4 - 4;
    D_80139260 += 1;
    D_80182DF4 = intensity;
    if (intensity < 0)
    {
        D_80182DF4 = 0;
    }
    remaining = D_801B2DB4 - 1;
    D_8013B240.vz = (u16) (D_8013B240.vz + 0x14);
    D_801B2DB4 = remaining;
    if (remaining == 0)
    {
        D_801B2DB0 += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_22_sequence_12_step_01(void)
{
    s32 i;

    D_801B0FD0 = 46;
    D_80139280[0xB] = 0;
    D_80139280[0xC] = 0;
    D_80139280[0xF] = 6;
    D_80139280[0x10] = 400;
    D_80139280[0x11] = 20;
    D_80139280[0x12] = 21;
    D_80139280[0x13] = 0;
    D_80139280[0x14] = 12000;
    for (i = 0; i < 46; i++)
    {
        D_801AFBD0[i + D_80139280[0x11]].field_00 = 0;
        D_80139988[i + 24].data = D_8011D538;
    }
    D_801B2DBC = 276;
    D_801B2DB8++;
    wmap_land_effect_22_sequence_12_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_22_sequence_13_step_01(void)
{
    s32 i;

    D_80139280[0x15] = 1;
    D_80139280[0x16] = 4;
    D_80139280[0x17] = 0x20;
    D_80139280[0x18] = 0;
    D_80139280[0x19] = 2;
    D_80139280[0x1A] = 0;
    D_80139280[0x1B] = 0x64;
    D_80139280[0x1C] = 0x15;
    D_80139280[0x1D] = 2;
    D_80139280[0x1E] = 0x1F40;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + 100].field_00 = 0;
        D_80139988[i + 104].data = D_8011D538;
    }
    D_801B2DC4 = 80;
    D_801B2DC0++;
    wmap_land_effect_22_sequence_13_step_02();
}

/** @brief World-map step handler: configure a model descriptor and clear two entry tables, then advance. */
void wmap_land_effect_22_sequence_14_step_01(void)
{
    s32 i;

    D_80139280[0x1F] = 1;
    D_80139280[0x20] = 5;
    D_80139280[0x21] = 0x20;
    D_80139280[0x22] = 0;
    D_80139280[0x23] = 8;
    D_80139280[0x24] = 0;
    D_80139280[0x25] = 0x8C;
    D_80139280[0x26] = 0x15;
    D_80139280[0x27] = 1;
    D_80139280[0x28] = 0x1F40;
    for (i = 0; i < 10; i++)
    {
        D_801AFBD0[i + 140].field_00 = 0;
        D_80139988[i + 144].data = D_8011D538;
    }
    D_801B2DCC = 80;
    D_801B2DC8++;
    wmap_land_effect_22_sequence_14_step_02();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_22_run(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D50 = 1;
        D_801B2D54 = 1;
        return 1;
    }

    if (D_801B2D50 < 0x4)
    {
        D_800D6904[D_801B2D50]();
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
void wmap_land_effect_22_reset(void)
{
    D_801B2D50 = 1;
    D_801B2D54 = 1;
}

/** @brief World-map step: arm a timed callback, flag it active, then tick the sub-counter. */
void wmap_land_effect_22_step_01(void)
{
    D_80139978 = 0x17;
    g_wmap_focus_screen_position.point.x = 0xA4;
    g_wmap_focus_screen_position.point.y = 0x69;
    wmap_start_sequence(wmap_land_effect_22_run_timeline);
    D_8013B20C = 1;
    D_801B2D50 += 1;
    wmap_land_effect_22_wait_idle();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_22_wait_idle(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2D50 += 1;
        wmap_land_effect_22_step_03();
    }
}

/** @brief World-map trigger: set two flags and bump a counter. */
void wmap_land_effect_22_step_03(void)
{
    D_8013B294 = 1;
    D_80139228 = 1;
    D_801B2D50 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_22_run_timeline(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D58 = 1;
        D_801B2D5C = 1;
        return 1;
    }

    if (D_801B2D58 < 0x16)
    {
        D_800D6914[D_801B2D58]();
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
void wmap_land_effect_22_timeline_reset(void)
{
    D_801B2D58 = 1;
    D_801B2D5C = 1;
}

/** @brief World-map step handler: kick two jobs and advance the step. */
void wmap_land_effect_22_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_start_map_tint(0x601040);
    g_wmap_backdrop_target_level = 8;
    wmap_play_sound(0x2C, 0x80);
    D_801B2D5C = 0xF;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_22_timeline_wait_02(void)
{
    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_22_timeline_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_22_run_sequence_13);
    D_801B2D5C = 0x14;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_22_timeline_wait_04(void)
{
    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_22_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_22_run_sequence_6);
    wmap_start_sequence(wmap_land_effect_22_run_sequence_12);
    D_801B2D5C = 0x78;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_22_timeline_wait_06(void)
{
    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/** @brief World-map step: register a callback, set flags, advance the step. */
void wmap_land_effect_22_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_effect_22_run_sequence_2);
    D_80139244 = 1;
    wmap_start_map_tint(0x351040);
    g_wmap_backdrop_target_level = 3;
    D_801B2D5C = 2;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_22_timeline_wait_08(void)
{
    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/** @brief World-map step handler: register three callbacks, reset state, advance the step. */
void wmap_land_effect_22_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_22_run_sequence_10);
    wmap_start_sequence(wmap_land_effect_22_run_sequence_7);
    wmap_start_sequence(wmap_land_effect_22_run_sequence_1);
    D_800DBE70 = 0;
    D_80139978 = -1;
    D_801B2D5C = 0xF;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_22_timeline_wait_10(void)
{
    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_22_timeline_step_11(void)
{
    wmap_start_sequence(wmap_land_effect_22_run_sequence_9);
    D_801B2D5C = 0x2D;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_22_timeline_wait_12(void)
{
    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_22_timeline_step_13(void)
{
    wmap_start_sequence(wmap_land_effect_22_run_sequence_11);
    D_801B2D5C = 0x78;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_22_timeline_wait_14(void)
{
    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/** @brief Register two callbacks around a color update and begin a 136-tick delay. */
void wmap_land_effect_22_timeline_step_15(void)
{
    wmap_start_sequence(&wmap_land_effect_22_run_sequence_3);
    D_80139244 = 0;
    wmap_start_map_tint(0x601550);
    g_wmap_backdrop_target_level = 8;
    wmap_start_sequence(&wmap_land_effect_22_run_sequence_14);
    D_801B2D5C = 0x88;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_22_timeline_wait_16(void)
{
    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_22_timeline_step_17(void)
{
    wmap_start_sequence(wmap_land_effect_22_run_sequence_4);
    wmap_start_sequence(wmap_land_effect_22_run_sequence_5);
    D_801B2D5C = 0x5;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_22_timeline_wait_18(void)
{
    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_22_timeline_step_19(void)
{
    wmap_start_sequence(wmap_land_effect_22_run_sequence_8);
    D_801B2D5C = 0x84;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_22_timeline_wait_20(void)
{
    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/** @brief Set the selected record value, clear the flag, and advance the sequence. */
void wmap_land_effect_22_timeline_finish(void)
{
    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = 0x110;
    D_801B2D58 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_22_run_sequence_1(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D60 = 1;
        D_801B2D64 = 1;
        return 1;
    }

    if (D_801B2D60 < 0x4)
    {
        D_800D696C[D_801B2D60]();
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
void wmap_land_effect_22_sequence_1_reset(void)
{
    D_801B2D60 = 1;
    D_801B2D64 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void wmap_land_effect_22_sequence_1_step_01(void)
{
    D_801399B4 = D_8011F538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.shade_step = 0;
    D_800D9344.target_shade = 0x7F;
    D_800D9344.shade = 0x7F;
    D_801B2D64 = 0x159;
    D_801B2D60 += 1;
    wmap_land_effect_22_sequence_1_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_22_sequence_1_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x16, 0xB, 0);
    if (--D_801B2D64 == 0)
    {
        D_801B2D60 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_22_sequence_1_end(void)
{
    D_801B2D60 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_22_run_sequence_2(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D68 = 1;
        D_801B2D6C = 1;
        return 1;
    }

    if (D_801B2D68 < 0x4)
    {
        D_800D697C[D_801B2D68]();
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
void wmap_land_effect_22_sequence_2_reset(void)
{
    D_801B2D68 = 1;
    D_801B2D6C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_22_sequence_2_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B2D6C = 0x40;
    D_801B2D68 += 1;
    wmap_land_effect_22_sequence_2_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_22_sequence_2_end(void)
{
    D_801B2D68 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_22_run_sequence_3(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D70 = 1;
        D_801B2D74 = 1;
        return 1;
    }

    if (D_801B2D70 < 0x4)
    {
        D_800D698C[D_801B2D70]();
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
void wmap_land_effect_22_sequence_3_reset(void)
{
    D_801B2D70 = 1;
    D_801B2D74 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_22_sequence_3_step_01(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    D_801B2D74 = 0x40;
    D_801B2D70 += 1;
    wmap_land_effect_22_sequence_3_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_22_sequence_3_end(void)
{
    D_801B2D70 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_22_run_sequence_4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D78 = 1;
        D_801B2D7C = 1;
        return 1;
    }

    if (D_801B2D78 < 0x4)
    {
        D_800D699C[D_801B2D78]();
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
void wmap_land_effect_22_sequence_4_reset(void)
{
    D_801B2D78 = 1;
    D_801B2D7C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_22_sequence_4_step_01(void)
{
    D_8013B238 = D_80139258;
    D_80139870 = g_wmap_camera_translation;
    D_80182DF0 = 0x80;
    D_80139870.vz = 0xAFC8;
    D_801B2D7C = 0x40;
    D_801B2D78 += 1;
    wmap_land_effect_22_sequence_4_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_22_sequence_4_end(void)
{
    D_801B2D78 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_22_run_sequence_5(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D80 = 1;
        D_801B2D84 = 1;
        return 1;
    }

    if (D_801B2D80 < 0x4)
    {
        D_800D69AC[D_801B2D80]();
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
void wmap_land_effect_22_sequence_5_reset(void)
{
    D_801B2D80 = 1;
    D_801B2D84 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_22_sequence_5_step_01(void)
{
    D_801399BC = D_80121538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 0x10;
    D_800D9370.resource_index = 0;
    D_800D9370.sequence = 0;
    D_800D9370.target_shade = 0x80;
    D_800D9370.shade = 0;
    D_801B2D84 = 0x8C;
    D_801B2D80 += 1;
    wmap_land_effect_22_sequence_5_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_22_sequence_5_step_02(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x16, 0xA, 0);
    if (--D_801B2D84 == 0)
    {
        D_801B2D80 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_22_sequence_5_end(void)
{
    D_801B2D80 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_22_run_sequence_6(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D88 = 1;
        D_801B2D8C = 1;
        return 1;
    }

    if (D_801B2D88 < 0x6)
    {
        D_800D69BC[D_801B2D88]();
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
void wmap_land_effect_22_sequence_6_reset(void)
{
    D_801B2D88 = 1;
    D_801B2D8C = 1;
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_22_sequence_6_step_01(void)
{
    D_80139268 = 0;
    D_801B2D8C = 0xFA;
    D_801B2D88 += 1;
    wmap_land_effect_22_sequence_6_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_22_sequence_6_step_03(void)
{
    D_801B2D8C = 0x40;
    D_801B2D88 += 1;
    wmap_land_effect_22_sequence_6_step_04();
}

/**
 * @brief Reset the world-map cursor state and bump the transition counter.
 */
void wmap_land_effect_22_sequence_6_step_05(void)
{
    D_80182D48[1] = 0;
    D_80182D48[0] = 0;
    D_801398C8[1] = 0;
    D_801398C8[0] = 0;
    D_801B2D88 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_22_run_sequence_7(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D90 = 1;
        D_801B2D94 = 1;
        return 1;
    }

    if (D_801B2D90 < 0x4)
    {
        D_800D69D4[D_801B2D90]();
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
void wmap_land_effect_22_sequence_7_reset(void)
{
    D_801B2D90 = 1;
    D_801B2D94 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_22_sequence_7_step_01(void)
{
    D_801399C4 = D_80123538;
    D_800D939C.scale_index = 0xF;
    D_800D939C.sequence = 1;
    D_800D939C.previous_sequence = -1;
    D_800D939C.resource_index = 0;
    D_800D939C.shade_step = 0;
    D_800D939C.target_shade = 0x7F;
    D_800D939C.shade = 0x7F;
    D_801B2D94 = 0xBE;
    D_801B2D90 += 1;
    wmap_land_effect_22_sequence_7_step_02();
}

/** @brief World-map step: init a sub-object then count down a timer. */
void wmap_land_effect_22_sequence_7_step_02(void)
{
    s32 n = 0x8;

    wmap_step_actor_animation(&D_800D939C, &D_801399C0);
    wmap_draw_actor_sprite(&D_800D939C, g_wmap_focus_screen_position.packed, n, n, 0);
    if (--D_801B2D94 == 0)
    {
        D_801B2D90 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_22_sequence_7_end(void)
{
    D_801B2D90 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_22_run_sequence_8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D98 = 1;
        D_801B2D9C = 1;
        return 1;
    }

    if (D_801B2D98 < 0x4)
    {
        D_800D69E4[D_801B2D98]();
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
void wmap_land_effect_22_sequence_8_reset(void)
{
    D_801B2D98 = 1;
    D_801B2D9C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_22_sequence_8_step_01(void)
{
    D_801399CC = D_80123538;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.previous_sequence = -1;
    D_800D93C8.shade_step = 8;
    D_800D93C8.resource_index = 0;
    D_800D93C8.sequence = 0;
    D_800D93C8.target_shade = 0;
    D_800D93C8.shade = 0x81;
    D_801B2D9C = 0x10;
    D_801B2D98 += 1;
    wmap_land_effect_22_sequence_8_step_02();
}

/** @brief World-map step: init a sub-object then count down a timer. */
void wmap_land_effect_22_sequence_8_step_02(void)
{
    s32 n = 0x8;

    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, g_wmap_focus_screen_position.packed, n, n, 0);
    if (--D_801B2D9C == 0)
    {
        D_801B2D98 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_22_sequence_8_end(void)
{
    D_801B2D98 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_22_run_sequence_9(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2DA0 = 1;
        D_801B2DA4 = 1;
        return 1;
    }

    if (D_801B2DA0 < 0x6)
    {
        D_800D69F4[D_801B2DA0]();
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
void wmap_land_effect_22_sequence_9_reset(void)
{
    D_801B2DA0 = 1;
    D_801B2DA4 = 1;
}

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void wmap_land_effect_22_sequence_9_step_01(void)
{
    D_80182DE4 = 1;
    D_801B2498 = D_80139258;
    D_8013923C = 0;
    D_801B2DA4 = 0x7C;
    D_801B2DA0 += 1;
    wmap_land_effect_22_sequence_9_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_22_sequence_9_step_03(void)
{
    D_801B2DA4 = 0x40;
    D_801B2DA0 += 1;
    wmap_land_effect_22_sequence_9_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_22_sequence_9_end(void)
{
    D_801B2DA0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_22_run_sequence_10(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2DA8 = 1;
        D_801B2DAC = 1;
        return 1;
    }

    if (D_801B2DA8 < 0x6)
    {
        D_800D6A0C[D_801B2DA8]();
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
void wmap_land_effect_22_sequence_10_reset(void)
{
    D_801B2DA8 = 1;
    D_801B2DAC = 1;
}

/** @brief Reset effect state and begin a 96-tick sequence step. */
void wmap_land_effect_22_sequence_10_step_01(void)
{
    D_801B25D8 = 1;
    D_801B2670 = D_80139258;
    D_80139234 = 0;
    D_80139264 = 0;
    D_801B2DAC = 0x60;
    D_801B2DA8 += 1;
    wmap_land_effect_22_sequence_10_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_22_sequence_10_step_03(void)
{
    D_801B2DAC = 0x10;
    D_801B2DA8 += 1;
    wmap_land_effect_22_sequence_10_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_22_sequence_10_end(void)
{
    D_801B2DA8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_22_run_sequence_11(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2DB0 = 1;
        D_801B2DB4 = 1;
        return 1;
    }

    if (D_801B2DB0 < 0x6)
    {
        D_800D6A24[D_801B2DB0]();
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
void wmap_land_effect_22_sequence_11_reset(void)
{
    D_801B2DB0 = 1;
    D_801B2DB4 = 1;
}

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void wmap_land_effect_22_sequence_11_step_01(void)
{
    D_80182DF4 = 1;
    D_8013B240 = D_80139258;
    D_80139260 = 0;
    D_801B2DB4 = 0x6C;
    D_801B2DB0 += 1;
    wmap_land_effect_22_sequence_11_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_22_sequence_11_step_03(void)
{
    D_801B2DB4 = 0x20;
    D_801B2DB0 += 1;
    wmap_land_effect_22_sequence_11_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_22_sequence_11_end(void)
{
    D_801B2DB0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_22_run_sequence_12(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2DB8 = 1;
        D_801B2DBC = 1;
        return 1;
    }

    if (D_801B2DB8 < 0x6)
    {
        D_800D6A3C[D_801B2DB8]();
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
void wmap_land_effect_22_sequence_12_reset(void)
{
    D_801B2DB8 = 1;
    D_801B2DBC = 1;
}

/**
 * @brief World-map step handler: submit a batched sprite draw, then advance the
 *        sequence once its frame counter expires.
 */
void wmap_land_effect_22_sequence_12_step_02(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x2E, 0x1, 0xFF, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2DBC == 0)
    {
        D_801B2DB8 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_22_sequence_12_step_03(void)
{
    D_801B2DBC = 0x40;
    D_80139280[15] = -1;
    D_801B2DB8 += 1;
    wmap_land_effect_22_sequence_12_step_04();
}

/**
 * @brief World-map step handler: submit a batched sprite draw, then advance the
 *        sequence once its frame counter expires.
 */
void wmap_land_effect_22_sequence_12_step_04(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x2E, 0x1, 0xFF, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2DBC == 0)
    {
        D_801B2DB8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_22_sequence_12_end(void)
{
    D_801B2DB8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_22_run_sequence_13(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2DC0 = 1;
        D_801B2DC4 = 1;
        return 1;
    }

    if (D_801B2DC0 < 0x6)
    {
        D_800D6A54[D_801B2DC0]();
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
void wmap_land_effect_22_sequence_13_reset(void)
{
    D_801B2DC0 = 1;
    D_801B2DC4 = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_22_sequence_13_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA448, D_80139CC8, 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B2DC4 == 0)
    {
        D_801B2DC0 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_22_sequence_13_step_03(void)
{
    D_801B2DC4 = 0x20;
    D_80139280[25] = -1;
    D_801B2DC0 += 1;
    wmap_land_effect_22_sequence_13_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_22_sequence_13_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA448, D_80139CC8, 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B2DC4 == 0)
    {
        D_801B2DC0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_22_sequence_13_end(void)
{
    D_801B2DC0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_22_run_sequence_14(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2DC8 = 1;
        D_801B2DCC = 1;
        return 1;
    }

    if (D_801B2DC8 < 0x6)
    {
        D_800D6A6C[D_801B2DC8]();
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
void wmap_land_effect_22_sequence_14_reset(void)
{
    D_801B2DC8 = 1;
    D_801B2DCC = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_22_sequence_14_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DAB28, D_80139E08, 0xA, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2DCC == 0)
    {
        D_801B2DC8 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_22_sequence_14_step_03(void)
{
    D_801B2DCC = 0x20;
    D_80139280[35] = -1;
    D_801B2DC8 += 1;
    wmap_land_effect_22_sequence_14_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_22_sequence_14_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DAB28, D_80139E08, 0xA, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2DCC == 0)
    {
        D_801B2DC8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_22_sequence_14_end(void)
{
    D_801B2DC8 += 1;
}
