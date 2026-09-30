#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_land_effect_25.h"
#include "sdk/libgte.h"
#include "wmap_sequence_runtime.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
#include "wmap_effect_primitives.h"
#include "wmap_sprite_render.h"

void wmap_land_effect_25_sequence_12_step_02(void);
void wmap_land_effect_25_sequence_13_step_02(void);
void wmap_land_effect_25_sequence_14_step_02(void);
void wmap_land_effect_25_sequence_15_step_02(void);
void wmap_land_effect_25_sequence_17_step_02(void);
void wmap_land_effect_25_wait_idle_02(void);
void wmap_land_effect_25_step_03(void);
s32 wmap_land_effect_25_run_timeline(s32 arg0);
void wmap_land_effect_25_wait_idle_04(void);
void wmap_land_effect_25_step_05(void);
s32 wmap_land_effect_25_run_sequence_1(s32 arg0);
s32 wmap_land_effect_25_run_sequence_3(s32 arg0);
s32 wmap_land_effect_25_run_sequence_15(s32 arg0);
s32 wmap_land_effect_25_run_sequence_6(s32 arg0);
s32 wmap_land_effect_25_run_sequence_7(s32 arg0);
s32 wmap_land_effect_25_run_sequence_14(s32 arg0);
s32 wmap_land_effect_25_run_sequence_18(s32 arg0);
s32 wmap_land_effect_25_run_sequence_8(s32 arg0);
s32 wmap_land_effect_25_run_sequence_12(s32 arg0);
s32 wmap_land_effect_25_run_sequence_13(s32 arg0);
s32 wmap_land_effect_25_run_sequence_4(s32 arg0);
s32 wmap_land_effect_25_run_sequence_17(s32 arg0);
s32 wmap_land_effect_25_run_sequence_11(s32 arg0);
s32 wmap_land_effect_25_run_sequence_9(s32 arg0);
s32 wmap_land_effect_25_run_sequence_16(s32 arg0);
s32 wmap_land_effect_25_run_sequence_2(s32 arg0);
s32 wmap_land_effect_25_run_sequence_10(s32 arg0);
s32 wmap_land_effect_25_run_sequence_5(s32 arg0);
void wmap_land_effect_25_sequence_1_step_02(void);
void wmap_land_effect_25_sequence_1_step_04(void);
void wmap_land_effect_25_sequence_2_step_02(void);
void wmap_land_effect_25_sequence_10_step_02(void);
void wmap_land_effect_25_sequence_11_step_02(void);
void wmap_land_effect_25_sequence_11_step_04(void);
void wmap_land_effect_25_sequence_12_step_04(void);
void wmap_land_effect_25_sequence_13_step_04(void);
void wmap_land_effect_25_sequence_14_step_04(void);
void wmap_land_effect_25_sequence_15_step_04(void);
void wmap_land_effect_25_sequence_16_step_02(void);
void wmap_land_effect_25_sequence_16_step_04(void);
void wmap_land_effect_25_sequence_17_step_04(void);
void wmap_land_effect_25_sequence_17_step_06(void);
void wmap_land_effect_25_sequence_18_step_02(void);
void wmap_land_effect_25_sequence_18_step_04(void);

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

typedef struct
{
    u8 pad0[0x22];
    s16 f22;
    u8 pad1[0x2];
    s16 f26;
    u8 pad2[0x4];
} WmapEntry;

extern VECTOR D_8011CF60;
extern u8 D_800DCF18[];
extern s32 D_80182DE8;
extern s32 D_801B2F14;
extern s32 D_80182DEC;
extern u8* D_8011CF1C;
extern s32 D_801B2F1C;
extern s32 D_80182DF0;
extern u8* D_8011CF24;
extern s32 D_801B2F24;
extern s32* D_8011CF28;
extern s32 D_80182DE4;
extern s32 D_801B2F2C;
extern s32* D_8011CF2C;
extern s32 D_80182DF4;
extern s32 D_801B2F34;
extern u8* D_8011CF30;
extern s32 D_80139234;
extern s32 D_801B25D8;
extern s32 D_801B2F3C;
extern u8* D_8011CF34;
extern s32 D_8013923C;
extern s32 D_801B25DC;
extern s32 D_801B2F44;
extern s32 D_801B0FD0;
extern s32 D_801B2F5C;
extern s32 rand(void);
extern s32 D_801B2F64;
extern s32 D_801B2F6C;
extern s32 D_801B2F74;
extern s32 D_800D9154;
extern s32 D_800DCEAC;
extern s32 D_801B25E0;
extern s32 D_801B2F84;
extern s32 D_801B2EF4;
extern void (*D_800D6FDC[])(void);
extern s32 D_800D923C;
extern s32 D_8013B20C;
extern void wmap_land_effect_25_step_03(void);
extern s32 D_801B2EFC;
extern void (*D_800D6FF4[])(void);
extern s32 D_8013B208;
extern s32 D_80139244;
extern s32 D_801ADAE0;
extern SVECTOR D_800DCEB8;
extern VECTOR D_80139200;
extern SVECTOR D_80139210;
extern VECTOR D_80139968;
extern s32 D_8013B29C;
extern CVECTOR D_80182D74;
extern CVECTOR D_80182D80;
extern CVECTOR D_80182D8C;
extern CVECTOR D_80182D94;
extern s32 D_8011D4FC;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern WmapValueRecord D_80139290[][6];
extern s32 D_801B2F04;
extern void (*D_800D7074[])(void);
extern u8* D_801399AC;
extern void wmap_land_effect_25_sequence_1_step_04(void);
extern s32 D_801B2F0C;
extern void (*D_800D708C[])(void);
extern u8* D_801399B4;
extern void wmap_land_effect_25_sequence_2_step_02(void);
extern void (*D_800D709C[])(void);
extern void (*D_800D70AC[])(void);
extern void (*D_800D70BC[])(void);
extern void (*D_800D70CC[])(void);
extern void (*D_800D70E4[])(void);
extern void (*D_800D70FC[])(void);
extern void (*D_800D7114[])(void);
extern s32 D_801B2F4C;
extern void (*D_800D712C[])(void);
extern u8* D_801399C4;
extern void wmap_land_effect_25_sequence_10_step_02(void);
extern s32 D_801B2F54;
extern void (*D_800D713C[])(void);
extern void *D_801399BC;
extern void wmap_land_effect_25_sequence_11_step_04(void);
extern void (*D_800D7154[])(void);
extern WmapAnimationSlot D_80139A28[];
extern void (*D_800D716C[])(void);
extern u8 D_80139B18[];
extern void (*D_800D7184[])(void);
extern u8 D_800DB4C8;
extern void (*D_800D719C[])(void);
extern u8 D_800DAFA0[];
extern u8 D_80139ED8[];
extern s32 D_801B2F7C;
extern void (*D_800D71B4[])(void);
extern u8 D_80121538[];
extern u8* D_801399D4;
extern void wmap_land_effect_25_sequence_16_step_02(void);
extern void wmap_land_effect_25_sequence_16_step_04(void);
extern void (*D_800D71CC[])(void);
extern s32 D_801B2F8C;
extern void (*D_800D71EC[])(void);
extern u8 D_8011F538[];
extern u8* D_801399CC;
extern void wmap_land_effect_25_sequence_18_step_02(void);
extern void wmap_land_effect_25_sequence_18_step_04(void);

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

extern u32 D_801B2F10;
extern u32 D_801B2F18;
extern u32 D_801B2F20;
extern u32 D_801B2F28;
extern u32 D_801B2F30;
extern u32 D_801B2F38;
extern u32 D_801B2F40;
extern WmapConfigA D_800D95D8[];
extern u8 D_80123538[];
extern u32 D_801B2F58;
extern WmapConfigA D_800D9B00[];
extern u32 D_801B2F60;
extern u32 D_801B2F68;
extern u32 D_801B2F70;
extern u32 D_801B2F80;
extern u32 D_801B2EF0;
extern u32 D_801B2EF8;
extern u32 D_801B2F00;
extern u8 D_8011D538[];
extern u32 D_801B2F08;
extern u32 D_801B2F48;
extern u32 D_801B2F50;
extern u32 D_801B2F78;
extern u32 D_801B2F88;

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
extern SVECTOR D_801B2678;

extern WmapSpriteActor D_800D9268[];
extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370;
extern WmapSpriteActor D_800D939C;
extern WmapSpriteActor D_800D93C8;
extern WmapSpriteActor D_800D93F4;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8;
extern WmapAnimationSlot D_801399C0;
extern WmapAnimationSlot D_801399C8;
extern WmapAnimationSlot D_801399D0;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* D_80139280;

extern WmapMotion D_801AFBD0[];
extern WmapMotion D_801AFD60[];
extern WmapMotion D_801AFFB8[];

/**
 * @brief World-map step handler: advance the model's spin toward a floor, draw it
 *        while active, then countdown-advance the step.
 */
void wmap_land_effect_25_sequence_3_step_02(void)
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
        wmap_draw_model(D_800DCF18, 0, 0x4, 0x35, 0x7800, 1, D_80182DE8, 0, 0, -1);
        D_80182DE8 -= 2;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }
    PopMatrix();
    if (--D_801B2F14 == 0)
    {
        D_801B2F10 += 1;
    }
}

/** @brief World-map step: spin the model matrix, draw the highlight, then tick the sub-counter. */
void wmap_land_effect_25_sequence_4_step_02(void)
{
    MATRIX m;

    D_801B2478.vz -= 0xDAC;
    if (D_801B2478.vz < 0x2710)
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
        wmap_draw_model(D_8011CF1C, 0, 4, 0x35, 0x7800, 1, D_80182DEC, 0, 0, -1);
        D_80182DEC -= 2;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }
    PopMatrix();
    if (--D_801B2F1C == 0)
    {
        D_801B2F18 += 1;
    }
}

/** @brief World-map step: spin the model matrix, draw the highlight, then tick the sub-counter. */
void wmap_land_effect_25_sequence_5_step_02(void)
{
    MATRIX m;

    D_80139870.vz -= 0xDAC;
    if (D_80139870.vz < 0x2710)
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
        wmap_draw_model(D_8011CF24, 0, 4, 0x35, 0x7800, 1, D_80182DF0, 0, 0, -1);
        D_80182DF0 -= 4;
        if (D_80182DF0 < 0)
        {
            D_80182DF0 = 0;
        }
    }
    PopMatrix();
    if (--D_801B2F24 == 0)
    {
        D_801B2F20 += 1;
    }
}

/**
 * @brief World-map step handler: draw the animated actor, ramp its size up to a
 *        cap, scroll the sprite field, then advance when the frame counter expires.
 */
void wmap_land_effect_25_sequence_6_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model(D_8011CF28, 0, 0x7, 0x35, 0x7840, 0x1001, D_80182DE4, 0, 0x32, -1);
    value = D_80182DE4 + 2;
    D_80182DE4 = value;
    if (value >= 0x82)
    {
        D_80182DE4 = 0x81;
    }
    timer = D_801B2F2C;
    D_801B2498.vz += 0x8;
    next_timer = timer - 1;
    D_801B2F2C = next_timer;
    if (next_timer == 0)
    {
        D_801B2F28++;
    }
}

/**
 * @brief World-map step handler: render the actor, ramp its size down to a floor,
 *        scroll the shadow field, then advance when the frame counter expires.
 */
void wmap_land_effect_25_sequence_6_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model(D_8011CF28, 0, 0x7, 0x35, 0x7840, 0x1001, D_80182DE4, 0, 0x32, -1);
    value = D_80182DE4 - 2;
    D_80182DE4 = value;
    if (value < 0)
    {
        D_80182DE4 = 0;
    }
    timer = D_801B2F2C;
    D_801B2498.vz += 0x8;
    next_timer = timer - 1;
    D_801B2F2C = next_timer;
    if (next_timer == 0)
    {
        D_801B2F28++;
    }
}

/**
 * @brief World-map step handler: draw the animated actor, ramp its size up to a
 *        cap, scroll the sprite field, then advance when the frame counter expires.
 */
void wmap_land_effect_25_sequence_7_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B240);
    wmap_draw_model(D_8011CF2C, 0, 0x7, 0x35, 0x7840, 0x1, D_80182DF4, 0, 0x1E, -1);
    value = D_80182DF4 + 2;
    D_80182DF4 = value;
    if (value >= 0x82)
    {
        D_80182DF4 = 0x81;
    }
    timer = D_801B2F34;
    D_8013B240.vz -= 0x8;
    next_timer = timer - 1;
    D_801B2F34 = next_timer;
    if (next_timer == 0)
    {
        D_801B2F30++;
    }
}

/**
 * @brief World-map step handler: render the actor, ramp its size down to a floor,
 *        scroll the shadow field, then advance when the frame counter expires.
 */
void wmap_land_effect_25_sequence_7_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B240);
    wmap_draw_model(D_8011CF2C, 0, 0x7, 0x35, 0x7840, 0x1, D_80182DF4, 0, 0x1E, -1);
    value = D_80182DF4 - 2;
    D_80182DF4 = value;
    if (value < 0)
    {
        D_80182DF4 = 0;
    }
    timer = D_801B2F34;
    D_8013B240.vz += -0x8;
    next_timer = timer - 1;
    D_801B2F34 = next_timer;
    if (next_timer == 0)
    {
        D_801B2F30++;
    }
}

/** @brief Advance the world-map effect and its sequence state. */
void wmap_land_effect_25_sequence_8_step_02(void)
{
    s32 remaining;
    s32 intensity;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2670);
    wmap_draw_model(D_8011CF30, (D_80139234 >> 4) & 3, 7, 0x36, 0x7980, 0x1001, D_801B25D8, 0, -10, -1);
    D_80139234 += 16;
    intensity = D_801B25D8 + 2;
    D_801B25D8 = intensity;
    if (intensity >= 0x82)
    {
        D_801B25D8 = 0x81;
    }
    remaining = D_801B2F3C - 1;
    D_801B2670.vz = (u16) (D_801B2670.vz + 0x18);
    D_801B2F3C = remaining;
    if (remaining == 0)
    {
        D_801B2F38 += 1;
    }
}

/** @brief Advance the world-map effect and its sequence state. */
void wmap_land_effect_25_sequence_8_step_04(void)
{
    s32 remaining;
    s32 intensity;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2670);
    wmap_draw_model(D_8011CF30, (D_80139234 >> 4) & 3, 7, 0x36, 0x7980, 0x1001, D_801B25D8, 0, -10, -1);
    intensity = D_801B25D8 - 4;
    D_80139234 += 16;
    D_801B25D8 = intensity;
    if (intensity < 0)
    {
        D_801B25D8 = 0;
    }
    remaining = D_801B2F3C - 1;
    D_801B2670.vz = (u16) (D_801B2670.vz + 0x18);
    D_801B2F3C = remaining;
    if (remaining == 0)
    {
        D_801B2F38 += 1;
    }
}

/** @brief Advance the world-map effect and its sequence state. */
void wmap_land_effect_25_sequence_9_step_02(void)
{
    s32 remaining;
    s32 intensity;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2678);
    wmap_draw_model(D_8011CF34, (D_8013923C >> 4) & 7, 10, 0x35, 0x7800, 0x1001, D_801B25DC, -1, 7, -1);
    D_8013923C += 16;
    intensity = D_801B25DC + 2;
    D_801B25DC = intensity;
    if (intensity >= 0x82)
    {
        D_801B25DC = 0x81;
    }
    remaining = D_801B2F44 - 1;
    D_801B2678.vz = (u16) (D_801B2678.vz + 0xC);
    D_801B2F44 = remaining;
    if (remaining == 0)
    {
        D_801B2F40 += 1;
    }
}

/** @brief Advance the world-map effect and its sequence state. */
void wmap_land_effect_25_sequence_9_step_04(void)
{
    s32 remaining;
    s32 intensity;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2678);
    wmap_draw_model(D_8011CF34, (D_8013923C >> 4) & 7, 10, 0x35, 0x7800, 0x1001, D_801B25DC, -1, 7, -1);
    intensity = D_801B25DC - 2;
    D_8013923C += 16;
    D_801B25DC = intensity;
    if (intensity < 0)
    {
        D_801B25DC = 0;
    }
    remaining = D_801B2F44 - 1;
    D_801B2678.vz = (u16) (D_801B2678.vz + 0xC);
    D_801B2F44 = remaining;
    if (remaining == 0)
    {
        D_801B2F40 += 1;
    }
}

/** @brief Initialize effect actors with randomized angles and speeds. */
void wmap_land_effect_25_sequence_12_step_01(void)
{
    s32 i;
    WmapMotion *motion;
    WmapConfigA *actor;

    D_801B0FD0 = 10;
    D_80139280[11] = 0;
    D_80139280[12] = -2;
    D_80139280[13] = 220;
    D_80139280[14] = 32;
    D_80139280[15] = -1;
    D_80139280[16] = -900;
    D_80139280[17] = 20;
    D_80139280[18] = 8;
    D_80139280[19] = 0;
    D_80139280[20] = 102000;
    i = 0;
    do
    {
        D_80139988[i + 20].data = D_80123538;
        actor = &D_800D95D8[i];
        motion = &D_801AFD60[i];
        motion->state = 1;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_02 = 0;
        actor->field_0E = 0;
        actor->field_22 = 255;
        actor->field_24 = 1;
        actor->field_26 = 0;
        motion->state = 1;
        motion->angle = rand() & 4095;
        motion->z = 102000;
        motion->x = (-(rand() * 2) >> 15);
        motion->field_0E = ((rand() * 50) >> 15) + 220;
        i++;
    } while (i < 10);
    D_801B2F5C = 32;
    D_801B2F58++;
    wmap_land_effect_25_sequence_12_step_02();
}

/** @brief Initialize effect actors with randomized angles and speeds. */
void wmap_land_effect_25_sequence_13_step_01(void)
{
    s32 i;
    WmapMotion *motion;
    WmapConfigA *actor;

    D_801B0FD0 = 30;
    D_80139280[21] = -1;
    D_80139280[22] = -2;
    D_80139280[23] = 180;
    D_80139280[24] = 32;
    D_80139280[25] = -1;
    D_80139280[26] = -1600;
    D_80139280[27] = 50;
    D_80139280[28] = 8;
    D_80139280[29] = 1;
    D_80139280[30] = 102000;
    i = 0;
    do
    {
        D_80139988[i + 50].data = D_80123538;
        actor = &D_800D9B00[i];
        motion = &D_801AFFB8[i];
        motion->state = 1;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_02 = 0;
        actor->field_0E = 1;
        actor->field_22 = 255;
        actor->field_24 = 1;
        actor->field_26 = 0;
        motion->state = 1;
        motion->angle = rand() & 4095;
        motion->z = 102000;
        motion->x = (-(rand() * 2) >> 15) - 1;
        motion->field_0E = ((rand() * 100) >> 15) + 180;
        i++;
    } while (i < 30);
    D_801B2F64 = 32;
    D_801B2F60++;
    wmap_land_effect_25_sequence_13_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_25_sequence_14_step_01(void)
{
    s32 i;

    D_80139280[0x28] = 40;
    D_80139280[0x29] = 200;
    D_80139280[0x2A] = -280;
    D_80139280[0x2B] = 560;
    D_80139280[0x2C] = -280;
    D_80139280[0x2D] = 560;
    D_80139280[0x2E] = 200;
    D_80139280[0x2F] = 80;
    D_80139280[0x30] = 129;
    D_80139280[0x31] = 1;
    D_80139280[0x32] = 2;
    D_80139280[0x33] = 40;
    D_80139280[0x34] = 8;
    D_80139280[0x35] = 2;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + 200].state = 0;
        D_80139988[i + 200].data = D_80123538;
    }
    D_801B2F6C = 160;
    D_801B2F68++;
    wmap_land_effect_25_sequence_14_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_25_sequence_15_step_01(void)
{
    s32 i;

    D_80139280[0x1] = 1;
    D_80139280[0x2] = 8;
    D_80139280[0x3] = 0x40;
    D_80139280[0x4] = 0;
    D_80139280[0x5] = 1;
    D_80139280[0x6] = 0x5DC;
    D_80139280[0x7] = 0xAA;
    D_80139280[0x8] = 8;
    D_80139280[0x9] = 3;
    D_80139280[0xA] = 0x32C8;
    for (i = 0; i < 24; i++)
    {
        D_801AFBD0[i + 170].state = 0;
        D_80139988[i + 170].data = D_80123538;
    }
    D_801B2F74 = 24;
    D_801B2F70++;
    wmap_land_effect_25_sequence_15_step_02();
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void wmap_land_effect_25_sequence_17_step_01(void)
{
    s32 i;
    WmapD94Entry *entry;

    i = 110;
    D_801B25E0 = 1;
    D_800DCEAC = 1;

    do
    {
        D_801AFBD0[i].state = 0;
        D_80139988[i].data = D_80123538;
        entry = &D_800D9268[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 1;
        entry->unk10 = -1;
        i++;
    } while (i < 155);

    D_800D9154 = 2;
    D_801B2F84 = 0x10;
    D_801B2F80 += 1;
    wmap_land_effect_25_sequence_17_step_02();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2EF0 = 1;
        D_801B2EF4 = 1;
        return 1;
    }

    if (D_801B2EF0 < 0x6)
    {
        D_800D6FDC[D_801B2EF0]();
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
void wmap_land_effect_25_reset(void)
{
    D_801B2EF0 = 1;
    D_801B2EF4 = 1;
}

/**
 * @brief Register a world-map callback and advance to the next step.
 */
void wmap_land_effect_25_step_01(void)
{
    D_800D923C = 1;
    wmap_start_sequence(wmap_run_land_focus);
    D_8013B20C = 1;
    D_801B2EF0 += 1;
    wmap_land_effect_25_wait_idle_02();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_25_wait_idle_02(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2EF0 += 1;
        wmap_land_effect_25_step_03();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_25_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_25_run_timeline);
    D_8013B20C = 1;
    D_801B2EF0 += 1;
    wmap_land_effect_25_wait_idle_04();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_25_wait_idle_04(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2EF0 += 1;
        wmap_land_effect_25_step_05();
    }
}

/**
 * @brief World-map step handler: clear the shared flag and advance the step counter.
 */
void wmap_land_effect_25_step_05(void)
{
    D_800D923C = 0;
    D_801B2EF0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_timeline(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2EF8 = 1;
        D_801B2EFC = 1;
        return 1;
    }

    if (D_801B2EF8 < 0x20)
    {
        D_800D6FF4[D_801B2EF8]();
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
void wmap_land_effect_25_timeline_reset(void)
{
    D_801B2EF8 = 1;
    D_801B2EFC = 1;
}

/** @brief World-map step: mark active, request a resource, seed the sub-counter, tick. */
void wmap_land_effect_25_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_play_sound(0x2F, 0x80);
    D_801B2EFC = 2;
    D_801B2EF8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_25_timeline_wait_02(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_25_timeline_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_25_run_sequence_1);
    D_801B2EFC = 0x10;
    D_801B2EF8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_25_timeline_wait_04(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/** @brief Register callbacks, set effect flags and color, and begin an eight-tick delay. */
void wmap_land_effect_25_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_25_run_sequence_15);
    wmap_start_sequence(wmap_land_effect_25_run_sequence_3);
    D_80139244 = 1;
    g_wmap_backdrop_target_level = 1;
    wmap_start_map_tint(0x701020);
    D_801ADAE0 = 1;
    D_801B2EFC = 8;
    D_801B2EF8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_25_timeline_wait_06(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/** @brief Initialize the sequence transforms and start a forty-tick countdown. */
void wmap_land_effect_25_timeline_step_07(void)
{
    D_8013B29C = 1;
    wmap_install_callback(&func_8006C0EC);
    D_80139210.vx = 5;
    D_80139210.vy = 0;
    D_80139210.vz = 0;
    D_80139968.vx = 0;
    D_80139968.vy = 2;
    D_80139968.vz = 0;
    D_800DCEB8.vx = 0xFA;
    D_800DCEB8.vy = 0;
    D_800DCEB8.vz = 0;
    D_80139200.vx = 0;
    D_80139200.vy = 0x64;
    D_80139200.vz = 0;
    D_801B2EFC = 0x28;
    D_801B2EF8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_25_timeline_wait_08(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_25_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_25_run_sequence_6);
    wmap_start_sequence(wmap_land_effect_25_run_sequence_7);
    wmap_start_sequence(wmap_land_effect_25_run_sequence_14);
    D_801B2EFC = 0x7;
    D_801B2EF8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_25_timeline_wait_10(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/** @brief Set two effect colors, clear two colors, and begin a 35-tick sequence step. */
void wmap_land_effect_25_timeline_step_11(void)
{
    g_wmap_backdrop_target_level = 0;
    D_80182D74.r = 0x32;
    D_80182D74.g = 0;
    D_80182D74.b = 0xA0;
    D_80182D80.r = 0x32;
    D_80182D80.g = 0;
    D_80182D80.b = 0xA0;
    D_80182D8C.r = 0;
    D_80182D8C.g = 0;
    D_80182D8C.b = 0;
    D_80182D94.r = 0;
    D_80182D94.g = 0;
    D_80182D94.b = 0;
    D_801B2EFC = 0x23;
    D_801B2EF8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_25_timeline_wait_12(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_25_timeline_step_13(void)
{
    wmap_start_sequence(wmap_land_effect_25_run_sequence_18);
    wmap_start_sequence(wmap_land_effect_25_run_sequence_8);
    D_801B2EFC = 0x20;
    D_801B2EF8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_25_timeline_wait_14(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_25_timeline_step_15(void)
{
    wmap_start_sequence(wmap_land_effect_25_run_sequence_12);
    D_801B2EFC = 0x20;
    D_801B2EF8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_25_timeline_wait_16(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_25_timeline_step_17(void)
{
    wmap_start_sequence(wmap_land_effect_25_run_sequence_13);
    D_801B2EFC = 0x5E;
    D_801B2EF8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_25_timeline_wait_18(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/** @brief Register an effect callback, initialize four vectors, and advance the sequence. */
void wmap_land_effect_25_timeline_step_19(void)
{
    wmap_start_sequence(&wmap_land_effect_25_run_sequence_4);
    D_800DCEB8.vx = 0;
    D_800DCEB8.vy = 0;
    D_800DCEB8.vz = 0;
    D_80139200.vx = 0;
    D_80139200.vy = 0;
    D_80139200.vz = 0;
    D_80139210.vx = 0xFA;
    D_80139210.vy = 0;
    D_80139210.vz = 0;
    D_80139968.vx = 0;
    D_80139968.vy = 0x64;
    D_80139968.vz = 0;
    D_801B2EFC = 1;
    D_801B2EF8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_25_timeline_wait_20(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/** @brief World-map step handler: register three callbacks, reset state, advance the step. */
void wmap_land_effect_25_timeline_step_21(void)
{
    wmap_start_sequence(wmap_land_effect_25_run_sequence_17);
    wmap_start_sequence(wmap_land_effect_25_run_sequence_11);
    wmap_start_sequence(wmap_land_effect_25_run_sequence_9);
    D_8013B29C = 0;
    g_wmap_backdrop_target_level = 3;
    D_801B2EFC = 0x10;
    D_801B2EF8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_25_timeline_wait_22(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_25_timeline_step_23(void)
{
    wmap_start_sequence(wmap_land_effect_25_run_sequence_16);
    D_801B2EFC = 0x40;
    D_801B2EF8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_25_timeline_wait_24(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_25_timeline_step_25(void)
{
    wmap_start_sequence(wmap_land_effect_25_run_sequence_2);
    D_801B2EFC = 0x9C;
    D_801B2EF8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_25_timeline_wait_26(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_25_timeline_step_27(void)
{
    wmap_start_sequence(wmap_land_effect_25_run_sequence_10);
    D_801B2EFC = 0x2;
    D_801B2EF8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_25_timeline_wait_28(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/** @brief Register a callback, update the selected map cell, and begin a 80-tick delay. */
void wmap_land_effect_25_timeline_step_29(void)
{
    wmap_start_sequence(&wmap_land_effect_25_run_sequence_5);
    D_80139244 = 0;
    g_wmap_backdrop_target_level = 15;
    wmap_start_map_tint(0x606070);
    D_801B2EFC = 0x50;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B2EF8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_25_timeline_wait_30(void)
{
    if (--D_801B2EFC == 0)
    {
        D_801B2EF8 += 1;
    }
}

/**
 * @brief World-map step handler: clear the shared flag and advance the step counter.
 */
void wmap_land_effect_25_timeline_finish(void)
{
    D_8013B20C = 0;
    D_801B2EF8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_1(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F00 = 1;
        D_801B2F04 = 1;
        return 1;
    }

    if (D_801B2F00 < 0x6)
    {
        D_800D7074[D_801B2F00]();
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
void wmap_land_effect_25_sequence_1_reset(void)
{
    D_801B2F00 = 1;
    D_801B2F04 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_25_sequence_1_step_01(void)
{
    D_801399AC = D_8011D538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.sequence = 3;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 8;
    D_800D9318.target_shade = 0x81;
    D_800D9318.resource_index = 0;
    D_800D9318.shade = 1;
    D_801B2F04 = 0x80;
    D_801B2F00 += 1;
    wmap_land_effect_25_sequence_1_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_25_sequence_1_step_02(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0x10, 0x5, 0);
    if (--D_801B2F04 == 0)
    {
        D_801B2F00 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_25_sequence_1_step_03(void)
{
    D_800D9318.shade_step = 4;
    D_800D9318.target_shade = 0;
    D_801B2F04 = 0x20;
    D_801B2F00 += 1;
    wmap_land_effect_25_sequence_1_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_25_sequence_1_step_04(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0x10, 0x5, 0);
    if (--D_801B2F04 == 0)
    {
        D_801B2F00 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_1_end(void)
{
    D_801B2F00 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_2(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F08 = 1;
        D_801B2F0C = 1;
        return 1;
    }

    if (D_801B2F08 < 0x4)
    {
        D_800D708C[D_801B2F08]();
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
void wmap_land_effect_25_sequence_2_reset(void)
{
    D_801B2F08 = 1;
    D_801B2F0C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_25_sequence_2_step_01(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 2;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0x80;
    D_801B2F0C = 0xA0;
    D_801B2F08 += 1;
    wmap_land_effect_25_sequence_2_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_25_sequence_2_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x10, 0x8, 0);
    if (--D_801B2F0C == 0)
    {
        D_801B2F08 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_2_end(void)
{
    D_801B2F08 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_3(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F10 = 1;
        D_801B2F14 = 1;
        return 1;
    }

    if (D_801B2F10 < 0x4)
    {
        D_800D709C[D_801B2F10]();
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
void wmap_land_effect_25_sequence_3_reset(void)
{
    D_801B2F10 = 1;
    D_801B2F14 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_25_sequence_3_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B2F14 = 0x40;
    D_801B2F10 += 1;
    wmap_land_effect_25_sequence_3_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_3_end(void)
{
    D_801B2F10 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F18 = 1;
        D_801B2F1C = 1;
        return 1;
    }

    if (D_801B2F18 < 0x4)
    {
        D_800D70AC[D_801B2F18]();
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
void wmap_land_effect_25_sequence_4_reset(void)
{
    D_801B2F18 = 1;
    D_801B2F1C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_25_sequence_4_step_01(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    D_801B2F1C = 0x40;
    D_801B2F18 += 1;
    wmap_land_effect_25_sequence_4_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_4_end(void)
{
    D_801B2F18 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_5(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F20 = 1;
        D_801B2F24 = 1;
        return 1;
    }

    if (D_801B2F20 < 0x4)
    {
        D_800D70BC[D_801B2F20]();
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
void wmap_land_effect_25_sequence_5_reset(void)
{
    D_801B2F20 = 1;
    D_801B2F24 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_25_sequence_5_step_01(void)
{
    D_8013B238 = D_80139258;
    D_80139870 = g_wmap_camera_translation;
    D_80182DF0 = 0x80;
    D_80139870.vz = 0xAFC8;
    D_801B2F24 = 0x20;
    D_801B2F20 += 1;
    wmap_land_effect_25_sequence_5_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_5_end(void)
{
    D_801B2F20 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_6(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F28 = 1;
        D_801B2F2C = 1;
        return 1;
    }

    if (D_801B2F28 < 0x6)
    {
        D_800D70CC[D_801B2F28]();
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
void wmap_land_effect_25_sequence_6_reset(void)
{
    D_801B2F28 = 1;
    D_801B2F2C = 1;
}

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void wmap_land_effect_25_sequence_6_step_01(void)
{
    D_80182DE4 = 1;
    D_801B2498 = D_80139258;
    D_801B2F2C = 0xB4;
    D_801B2F28 += 1;
    wmap_land_effect_25_sequence_6_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_25_sequence_6_step_03(void)
{
    D_801B2F2C = 0x40;
    D_801B2F28 += 1;
    wmap_land_effect_25_sequence_6_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_6_end(void)
{
    D_801B2F28 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_7(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F30 = 1;
        D_801B2F34 = 1;
        return 1;
    }

    if (D_801B2F30 < 0x6)
    {
        D_800D70E4[D_801B2F30]();
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
void wmap_land_effect_25_sequence_7_reset(void)
{
    D_801B2F30 = 1;
    D_801B2F34 = 1;
}

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void wmap_land_effect_25_sequence_7_step_01(void)
{
    D_80182DF4 = 1;
    D_8013B240 = D_80139258;
    D_801B2F34 = 0xAE;
    D_801B2F30 += 1;
    wmap_land_effect_25_sequence_7_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_25_sequence_7_step_03(void)
{
    D_801B2F34 = 0x40;
    D_801B2F30 += 1;
    wmap_land_effect_25_sequence_7_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_7_end(void)
{
    D_801B2F30 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F38 = 1;
        D_801B2F3C = 1;
        return 1;
    }

    if (D_801B2F38 < 0x6)
    {
        D_800D70FC[D_801B2F38]();
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
void wmap_land_effect_25_sequence_8_reset(void)
{
    D_801B2F38 = 1;
    D_801B2F3C = 1;
}

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void wmap_land_effect_25_sequence_8_step_01(void)
{
    D_801B25D8 = 1;
    D_801B2670 = D_80139258;
    D_80139234 = 0;
    D_801B2F3C = 0x84;
    D_801B2F38 += 1;
    wmap_land_effect_25_sequence_8_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_25_sequence_8_step_03(void)
{
    D_801B2F3C = 0x20;
    D_801B2F38 += 1;
    wmap_land_effect_25_sequence_8_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_8_end(void)
{
    D_801B2F38 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_9(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F40 = 1;
        D_801B2F44 = 1;
        return 1;
    }

    if (D_801B2F40 < 0x6)
    {
        D_800D7114[D_801B2F40]();
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
void wmap_land_effect_25_sequence_9_reset(void)
{
    D_801B2F40 = 1;
    D_801B2F44 = 1;
}

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void wmap_land_effect_25_sequence_9_step_01(void)
{
    D_801B25DC = 1;
    D_801B2678 = D_80139258;
    D_8013923C = 0;
    D_801B2F44 = 0x84;
    D_801B2F40 += 1;
    wmap_land_effect_25_sequence_9_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_25_sequence_9_step_03(void)
{
    D_801B2F44 = 0x40;
    D_801B2F40 += 1;
    wmap_land_effect_25_sequence_9_step_04();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_9_end(void)
{
    D_801B2F40 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_10(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F48 = 1;
        D_801B2F4C = 1;
        return 1;
    }

    if (D_801B2F48 < 0x4)
    {
        D_800D712C[D_801B2F48]();
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
void wmap_land_effect_25_sequence_10_reset(void)
{
    D_801B2F48 = 1;
    D_801B2F4C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_25_sequence_10_step_01(void)
{
    D_801399C4 = D_8011D538;
    D_800D939C.scale_index = 0xF;
    D_800D939C.sequence = 2;
    D_800D939C.previous_sequence = -1;
    D_800D939C.shade_step = 8;
    D_800D939C.resource_index = 0;
    D_800D939C.target_shade = 0x81;
    D_800D939C.shade = 0x81;
    D_801B2F4C = 0x24;
    D_801B2F48 += 1;
    wmap_land_effect_25_sequence_10_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_25_sequence_10_step_02(void)
{
    wmap_step_actor_animation(&D_800D939C, &D_801399C0);
    wmap_draw_actor_sprite(&D_800D939C, g_wmap_focus_screen_position.packed, 0x10, 0x4, 0);
    if (--D_801B2F4C == 0)
    {
        D_801B2F48 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_10_end(void)
{
    D_801B2F48 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_11(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F50 = 1;
        D_801B2F54 = 1;
        return 1;
    }

    if (D_801B2F50 < 0x6)
    {
        D_800D713C[D_801B2F50]();
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
void wmap_land_effect_25_sequence_11_reset(void)
{
    D_801B2F50 = 1;
    D_801B2F54 = 1;
}

/** @brief Initialize the world-map actor and advance the timed sequence. */
void wmap_land_effect_25_sequence_11_step_01(void)
{
    D_801399BC = &D_8011D538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.resource_index = 0;
    D_800D9370.sequence = 1;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 4;
    D_800D9370.target_shade = 0x81;
    D_800D9370.shade = 1;
    D_801B2F54 = 0x50;
    D_801B2F50 += 1;
    wmap_land_effect_25_sequence_11_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_25_sequence_11_step_02(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x10, 0x7, 0);
    if (--D_801B2F54 == 0)
    {
        D_801B2F50 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_25_sequence_11_step_03(void)
{
    D_800D9370.shade_step = 2;
    D_800D9370.target_shade = 0;
    D_801B2F54 = 0x40;
    D_801B2F50 += 1;
    wmap_land_effect_25_sequence_11_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_25_sequence_11_step_04(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x10, 0x7, 0);
    if (--D_801B2F54 == 0)
    {
        D_801B2F50 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_11_end(void)
{
    D_801B2F50 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_12(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F58 = 1;
        D_801B2F5C = 1;
        return 1;
    }

    if (D_801B2F58 < 0x6)
    {
        D_800D7154[D_801B2F58]();
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
void wmap_land_effect_25_sequence_12_reset(void)
{
    D_801B2F58 = 1;
    D_801B2F5C = 1;
}

/** @brief World-map step handler: spawn a sprite and count down the shared timer. */
void wmap_land_effect_25_sequence_12_step_02(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0xA, 1, 0xFF, 2, 6, (s32)D_80139280 + 0x28);
    if (--D_801B2F5C == 0)
    {
        D_801B2F58 += 1;
    }
}

/** @brief World-map step handler: arm a config range, set the timer, and advance. */
void wmap_land_effect_25_sequence_12_step_03(void)
{
    s32 i;

    for (i = 0; i < 0xA; i++)
    {
        D_800D9268[i + 0x14].shade_step = 2;
    }
    D_801B2F5C = 0x64;
    D_801B2F58 += 1;
    wmap_land_effect_25_sequence_12_step_04();
}

/** @brief World-map step handler: spawn a sprite and count down the shared timer. */
void wmap_land_effect_25_sequence_12_step_04(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0xA, 1, 0xFF, 2, 7, (s32)D_80139280 + 0x28);
    if (--D_801B2F5C == 0)
    {
        D_801B2F58 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_12_end(void)
{
    D_801B2F58 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_13(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F60 = 1;
        D_801B2F64 = 1;
        return 1;
    }

    if (D_801B2F60 < 0x6)
    {
        D_800D716C[D_801B2F60]();
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
void wmap_land_effect_25_sequence_13_reset(void)
{
    D_801B2F60 = 1;
    D_801B2F64 = 1;
}

/** @brief World-map step handler: spawn a sprite and count down the shared timer. */
void wmap_land_effect_25_sequence_13_step_02(void)
{
    func_8006A2FC(D_800D9B00, D_80139B18, 0x1E, 1, 0xFF, 4, 6, (s32)D_80139280 + 0x50);
    if (--D_801B2F64 == 0)
    {
        D_801B2F60 += 1;
    }
}

/** @brief World-map step handler: arm a config range, set the timer, and advance. */
void wmap_land_effect_25_sequence_13_step_03(void)
{
    s32 i;

    for (i = 0; i < 0x1E; i++)
    {
        D_800D9268[i + 0x32].shade_step = 4;
    }
    D_801B2F64 = 0x40;
    D_801B2F60 += 1;
    wmap_land_effect_25_sequence_13_step_04();
}

/** @brief World-map step handler: spawn a sprite and count down the shared timer. */
void wmap_land_effect_25_sequence_13_step_04(void)
{
    func_8006A2FC(D_800D9B00, D_80139B18, 0x1E, 1, 0xFF, 4, 7, (s32)D_80139280 + 0x50);
    if (--D_801B2F64 == 0)
    {
        D_801B2F60 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_13_end(void)
{
    D_801B2F60 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_14(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F68 = 1;
        D_801B2F6C = 1;
        return 1;
    }

    if (D_801B2F68 < 0x6)
    {
        D_800D7184[D_801B2F68]();
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
void wmap_land_effect_25_sequence_14_reset(void)
{
    D_801B2F68 = 1;
    D_801B2F6C = 1;
}

/** @brief Kick a world-map sub-handler off the shared context, then step counters. */
void wmap_land_effect_25_sequence_14_step_02(void)
{
    func_8006C448((s32)D_80139280 + 0xA0);
    if (--D_801B2F6C == 0)
    {
        D_801B2F68 += 1;
    }
}

/** @brief World-map step handler: clear a small entry table, set the timer, advance the step. */
void wmap_land_effect_25_sequence_14_step_03(void)
{
    WmapEntry *entries;
    s32 i;

    entries = (WmapEntry *)&D_800DB4C8;
    for (i = 0; i < 0x28; i++)
    {
        entries[i].f22 = 0;
        entries[i].f26 = 2;
    }

    D_801B2F6C = 0x40;
    D_801B2F68 += 1;
    wmap_land_effect_25_sequence_14_step_04();
}

/** @brief Kick a world-map sub-handler off the shared context, then step counters. */
void wmap_land_effect_25_sequence_14_step_04(void)
{
    func_8006C448((s32)D_80139280 + 0xA0);
    if (--D_801B2F6C == 0)
    {
        D_801B2F68 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_14_end(void)
{
    D_801B2F68 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_15(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F70 = 1;
        D_801B2F74 = 1;
        return 1;
    }

    if (D_801B2F70 < 0x6)
    {
        D_800D719C[D_801B2F70]();
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
void wmap_land_effect_25_sequence_15_reset(void)
{
    D_801B2F70 = 1;
    D_801B2F74 = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_25_sequence_15_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DAFA0, D_80139ED8, 0x18, 0xFF, 0x1, 0x4, 0, (s32)D_80139280);
    if (--D_801B2F74 == 0)
    {
        D_801B2F70 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_25_sequence_15_step_03(void)
{
    D_801B2F74 = 0x40;
    D_80139280[5] = -1;
    D_801B2F70 += 1;
    wmap_land_effect_25_sequence_15_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_25_sequence_15_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DAFA0, D_80139ED8, 0x18, 0xFF, 0x1, 0x4, 0, (s32)D_80139280);
    if (--D_801B2F74 == 0)
    {
        D_801B2F70 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_15_end(void)
{
    D_801B2F70 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_16(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F78 = 1;
        D_801B2F7C = 1;
        return 1;
    }

    if (D_801B2F78 < 0x6)
    {
        D_800D71B4[D_801B2F78]();
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
void wmap_land_effect_25_sequence_16_reset(void)
{
    D_801B2F78 = 1;
    D_801B2F7C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_25_sequence_16_step_01(void)
{
    D_801399D4 = D_80121538;
    D_800D93F4.scale_index = 0xF;
    D_800D93F4.previous_sequence = -1;
    D_800D93F4.shade_step = 4;
    D_800D93F4.target_shade = 0x81;
    D_800D93F4.resource_index = 0;
    D_800D93F4.sequence = 0;
    D_800D93F4.shade = 1;
    D_801B2F7C = 0x78;
    D_801B2F78 += 1;
    wmap_land_effect_25_sequence_16_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_25_sequence_16_step_02(void)
{
    wmap_step_actor_animation(&D_800D93F4, &D_801399D0);
    wmap_draw_actor_sprite(&D_800D93F4, g_wmap_focus_screen_position.packed, 0x8, 0x5, 0);
    if (--D_801B2F7C == 0)
    {
        D_801B2F78 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_25_sequence_16_step_03(void)
{
    D_800D93F4.shade_step = 2;
    D_800D93F4.target_shade = 0;
    D_801B2F7C = 0x40;
    D_801B2F78 += 1;
    wmap_land_effect_25_sequence_16_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_25_sequence_16_step_04(void)
{
    wmap_step_actor_animation(&D_800D93F4, &D_801399D0);
    wmap_draw_actor_sprite(&D_800D93F4, g_wmap_focus_screen_position.packed, 0x8, 0x5, 0);
    if (--D_801B2F7C == 0)
    {
        D_801B2F78 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_16_end(void)
{
    D_801B2F78 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_17(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F80 = 1;
        D_801B2F84 = 1;
        return 1;
    }

    if (D_801B2F80 < 0x8)
    {
        D_800D71CC[D_801B2F80]();
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
void wmap_land_effect_25_sequence_17_reset(void)
{
    D_801B2F80 = 1;
    D_801B2F84 = 1;
}

/** @brief World-map effect spawn: submit a request and tick a refcount. */
void wmap_land_effect_25_sequence_17_step_02(void)
{
    s32 c;

    func_8006B328(0x6E, 0x9B, 2, -1, 3, 2, 0x168, 8, -0x78, 0xF0, -0x78,
                  0xF0, 0x64, 0x81, 0x81, 4, 1);
    D_801B25E0 += 8;
    c = D_801B2F84 - 1;
    D_801B2F84 = c;
    if (c == 0)
    {
        D_801B2F80 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_25_sequence_17_step_03(void)
{
    D_801B2F84 = 0x5A;
    D_801B2F80 += 1;
    wmap_land_effect_25_sequence_17_step_04();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_25_sequence_17_step_04(void)
{
    func_8006B328(0x6E, 0x9B, 2, -1, 3, 2, 0x168, 8, -0x78, 0xF0, -0x78, 0xF0, 0x64, 0x81, 0x81, 4, 1);
    if (--D_801B2F84 == 0)
    {
        D_801B2F80 += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_25_sequence_17_step_05(void)
{
    D_800DCEAC = 0;
    D_801B2F84 = 0x64;
    D_801B2F80 += 1;
    wmap_land_effect_25_sequence_17_step_06();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_25_sequence_17_step_06(void)
{
    func_8006B328(0x6E, 0x9B, 2, -1, 3, 2, 0x168, 8, -0x78, 0xF0, -0x78, 0xF0, 0x64, 0x81, 0x81, 4, 1);
    if (--D_801B2F84 == 0)
    {
        D_801B2F80 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_17_end(void)
{
    D_801B2F80 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_25_run_sequence_18(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F88 = 1;
        D_801B2F8C = 1;
        return 1;
    }

    if (D_801B2F88 < 0x6)
    {
        D_800D71EC[D_801B2F88]();
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
void wmap_land_effect_25_sequence_18_reset(void)
{
    D_801B2F88 = 1;
    D_801B2F8C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_25_sequence_18_step_01(void)
{
    D_801399CC = D_8011F538;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.previous_sequence = -1;
    D_800D93C8.shade_step = 4;
    D_800D93C8.target_shade = 0x81;
    D_800D93C8.resource_index = 0;
    D_800D93C8.sequence = 0;
    D_800D93C8.shade = 1;
    D_801B2F8C = 0x96;
    D_801B2F88 += 1;
    wmap_land_effect_25_sequence_18_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_25_sequence_18_step_02(void)
{
    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, g_wmap_focus_screen_position.packed, 0x11, 0x4, 0);
    if (--D_801B2F8C == 0)
    {
        D_801B2F88 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_25_sequence_18_step_03(void)
{
    D_800D93C8.shade_step = 4;
    D_800D93C8.target_shade = 0;
    D_801B2F8C = 0x20;
    D_801B2F88 += 1;
    wmap_land_effect_25_sequence_18_step_04();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_25_sequence_18_step_04(void)
{
    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, g_wmap_focus_screen_position.packed, 0x11, 0x4, 0);
    if (--D_801B2F8C == 0)
    {
        D_801B2F88 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_25_sequence_18_end(void)
{
    D_801B2F88 += 1;
}
