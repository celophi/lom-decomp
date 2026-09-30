#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_land_effect_25.h"
#include "sdk/libgte.h"
#include "wmap_sequence_runtime.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
#include "wmap_effect_primitives.h"
#include "wmap_sprite_render.h"
#include "wmap_step_sequence.h"

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

extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_25_sequence_3_timer;
extern u8* g_wmap_effect_model_pack_1;
extern s32 g_wmap_land_effect_25_sequence_4_timer;
extern u8* g_wmap_effect_model_pack_2;
extern s32 g_wmap_land_effect_25_sequence_5_timer;
extern s32* g_wmap_effect_model_pack_3;
extern s32 D_80182DE4;
extern s32 g_wmap_land_effect_25_sequence_6_timer;
extern s32* D_8011CF2C;
extern s32 g_wmap_land_effect_25_sequence_7_timer;
extern u8* D_8011CF30;
extern s32 D_80139234;
extern s32 D_801B25D8;
extern s32 g_wmap_land_effect_25_sequence_8_timer;
extern u8* D_8011CF34;
extern s32 D_8013923C;
extern s32 D_801B25DC;
extern s32 g_wmap_land_effect_25_sequence_9_timer;
extern s32 g_wmap_land_effect_25_sequence_12_timer;
extern s32 rand(void);
extern s32 g_wmap_land_effect_25_sequence_13_timer;
extern s32 g_wmap_land_effect_25_sequence_14_timer;
extern s32 g_wmap_land_effect_25_sequence_15_timer;
extern s32 D_800D9154;
extern s32 D_800DCEAC;
extern s32 D_801B25E0;
extern s32 g_wmap_land_effect_25_sequence_17_timer;
extern s32 g_wmap_land_effect_25_timer;
extern void (*D_800D6FDC[])(void);
extern s32 D_800D923C;
extern void wmap_land_effect_25_step_03(void);
extern s32 g_wmap_land_effect_25_timeline_timer;
extern void (*D_800D6FF4[])(void);
extern SVECTOR D_800DCEB8;
extern VECTOR D_80139200;
extern SVECTOR D_80139210;
extern VECTOR D_80139968;
extern s32 D_8013B29C;
extern CVECTOR D_80182D74;
extern CVECTOR D_80182D80;
extern CVECTOR D_80182D8C;
extern CVECTOR D_80182D94;
extern s32 g_wmap_selected_artifact;
extern WmapValueRecord g_wmap_cells[][6];
extern s32 g_wmap_land_effect_25_sequence_1_timer;
extern void (*D_800D7074[])(void);
extern void wmap_land_effect_25_sequence_1_step_04(void);
extern s32 g_wmap_land_effect_25_sequence_2_timer;
extern void (*D_800D708C[])(void);
extern void wmap_land_effect_25_sequence_2_step_02(void);
extern void (*D_800D709C[])(void);
extern void (*D_800D70AC[])(void);
extern void (*D_800D70BC[])(void);
extern void (*D_800D70CC[])(void);
extern void (*D_800D70E4[])(void);
extern void (*D_800D70FC[])(void);
extern void (*D_800D7114[])(void);
extern s32 g_wmap_land_effect_25_sequence_10_timer;
extern void (*D_800D712C[])(void);
extern void wmap_land_effect_25_sequence_10_step_02(void);
extern s32 g_wmap_land_effect_25_sequence_11_timer;
extern void (*D_800D713C[])(void);
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
extern s32 g_wmap_land_effect_25_sequence_16_timer;
extern void (*D_800D71B4[])(void);
extern u8 g_wmap_animation_bank_2[];
extern void wmap_land_effect_25_sequence_16_step_02(void);
extern void wmap_land_effect_25_sequence_16_step_04(void);
extern void (*D_800D71CC[])(void);
extern s32 g_wmap_land_effect_25_sequence_18_timer;
extern void (*D_800D71EC[])(void);
extern u8 g_wmap_animation_bank_1[];
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

extern u32 g_wmap_land_effect_25_sequence_3_step;
extern u32 g_wmap_land_effect_25_sequence_4_step;
extern u32 g_wmap_land_effect_25_sequence_5_step;
extern u32 g_wmap_land_effect_25_sequence_6_step;
extern u32 g_wmap_land_effect_25_sequence_7_step;
extern u32 g_wmap_land_effect_25_sequence_8_step;
extern u32 g_wmap_land_effect_25_sequence_9_step;
extern WmapConfigA D_800D95D8[];
extern u8 g_wmap_animation_bank_3[];
extern u32 g_wmap_land_effect_25_sequence_12_step;
extern WmapConfigA D_800D9B00[];
extern u32 g_wmap_land_effect_25_sequence_13_step;
extern u32 g_wmap_land_effect_25_sequence_14_step;
extern u32 g_wmap_land_effect_25_sequence_15_step;
extern u32 g_wmap_land_effect_25_sequence_17_step;
extern u32 g_wmap_land_effect_25_step;
extern u32 g_wmap_land_effect_25_timeline_step;
extern u32 g_wmap_land_effect_25_sequence_1_step;
extern u8 g_wmap_animation_bank_0[];
extern u32 g_wmap_land_effect_25_sequence_2_step;
extern u32 g_wmap_land_effect_25_sequence_10_step;
extern u32 g_wmap_land_effect_25_sequence_11_step;
extern u32 g_wmap_land_effect_25_sequence_16_step;
extern u32 g_wmap_land_effect_25_sequence_18_step;

extern VECTOR g_wmap_camera_translation;

extern SVECTOR D_801B2498;
extern SVECTOR D_801B2670;
extern SVECTOR D_801B2678;

extern WmapSpriteActor D_800D939C;
extern WmapSpriteActor D_800D93C8;
extern WmapSpriteActor D_800D93F4;

extern WmapAnimationSlot g_wmap_actor_animations[];
extern WmapAnimationSlot D_801399C0;
extern WmapAnimationSlot D_801399C8;
extern WmapAnimationSlot D_801399D0;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* g_wmap_effect_params;

extern WmapMotion g_wmap_actor_motions[];
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

    x = g_wmap_effect_model_a_position.vz - 0xDAC;
    g_wmap_effect_model_a_position.vz = x;
    if (x < 0x2710)
    {
        g_wmap_effect_model_a_position.vz = 0x2710;
    }
    PushMatrix();
    RotMatrix(&g_wmap_effect_model_a_rotation, &m);
    TransMatrix(&m, &g_wmap_zero_translation);
    SetRotMatrix(&m);
    SetTransMatrix(&m);
    if (g_wmap_effect_fade_a != 0)
    {
        wmap_draw_model(g_wmap_load_buffer, 0, 0x4, 0x35, 0x7800, 1, g_wmap_effect_fade_a, 0, 0, -1);
        g_wmap_effect_fade_a -= 2;
        if (g_wmap_effect_fade_a < 0)
        {
            g_wmap_effect_fade_a = 0;
        }
    }
    PopMatrix();
    if (--g_wmap_land_effect_25_sequence_3_timer == 0)
    {
        g_wmap_land_effect_25_sequence_3_step += 1;
    }
}

/** @brief World-map step: spin the model matrix, draw the highlight, then tick the sub-counter. */
void wmap_land_effect_25_sequence_4_step_02(void)
{
    MATRIX m;

    g_wmap_effect_model_b_position.vz -= 0xDAC;
    if (g_wmap_effect_model_b_position.vz < 0x2710)
    {
        g_wmap_effect_model_b_position.vz = 0x2710;
    }
    PushMatrix();
    RotMatrix(&g_wmap_effect_model_b_rotation, &m);
    TransMatrix(&m, &g_wmap_zero_translation);
    SetRotMatrix(&m);
    SetTransMatrix(&m);
    if (g_wmap_effect_fade_b != 0)
    {
        wmap_draw_model(g_wmap_effect_model_pack_1, 0, 4, 0x35, 0x7800, 1, g_wmap_effect_fade_b, 0, 0, -1);
        g_wmap_effect_fade_b -= 2;
        if (g_wmap_effect_fade_b < 0)
        {
            g_wmap_effect_fade_b = 0;
        }
    }
    PopMatrix();
    if (--g_wmap_land_effect_25_sequence_4_timer == 0)
    {
        g_wmap_land_effect_25_sequence_4_step += 1;
    }
}

/** @brief World-map step: spin the model matrix, draw the highlight, then tick the sub-counter. */
void wmap_land_effect_25_sequence_5_step_02(void)
{
    MATRIX m;

    g_wmap_effect_model_c_position.vz -= 0xDAC;
    if (g_wmap_effect_model_c_position.vz < 0x2710)
    {
        g_wmap_effect_model_c_position.vz = 0x2710;
    }
    PushMatrix();
    RotMatrix(&g_wmap_effect_model_c_rotation, &m);
    TransMatrix(&m, &g_wmap_zero_translation);
    SetRotMatrix(&m);
    SetTransMatrix(&m);
    if (g_wmap_effect_fade_c != 0)
    {
        wmap_draw_model(g_wmap_effect_model_pack_2, 0, 4, 0x35, 0x7800, 1, g_wmap_effect_fade_c, 0, 0, -1);
        g_wmap_effect_fade_c -= 4;
        if (g_wmap_effect_fade_c < 0)
        {
            g_wmap_effect_fade_c = 0;
        }
    }
    PopMatrix();
    if (--g_wmap_land_effect_25_sequence_5_timer == 0)
    {
        g_wmap_land_effect_25_sequence_5_step += 1;
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
    wmap_draw_model(g_wmap_effect_model_pack_3, 0, 0x7, 0x35, 0x7840, 0x1001, D_80182DE4, 0, 0x32, -1);
    value = D_80182DE4 + 2;
    D_80182DE4 = value;
    if (value >= 0x82)
    {
        D_80182DE4 = 0x81;
    }
    timer = g_wmap_land_effect_25_sequence_6_timer;
    D_801B2498.vz += 0x8;
    next_timer = timer - 1;
    g_wmap_land_effect_25_sequence_6_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_effect_25_sequence_6_step++;
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
    wmap_draw_model(g_wmap_effect_model_pack_3, 0, 0x7, 0x35, 0x7840, 0x1001, D_80182DE4, 0, 0x32, -1);
    value = D_80182DE4 - 2;
    D_80182DE4 = value;
    if (value < 0)
    {
        D_80182DE4 = 0;
    }
    timer = g_wmap_land_effect_25_sequence_6_timer;
    D_801B2498.vz += 0x8;
    next_timer = timer - 1;
    g_wmap_land_effect_25_sequence_6_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_effect_25_sequence_6_step++;
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

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_d_rotation);
    wmap_draw_model(D_8011CF2C, 0, 0x7, 0x35, 0x7840, 0x1, g_wmap_effect_fade_d, 0, 0x1E, -1);
    value = g_wmap_effect_fade_d + 2;
    g_wmap_effect_fade_d = value;
    if (value >= 0x82)
    {
        g_wmap_effect_fade_d = 0x81;
    }
    timer = g_wmap_land_effect_25_sequence_7_timer;
    g_wmap_effect_model_d_rotation.vz -= 0x8;
    next_timer = timer - 1;
    g_wmap_land_effect_25_sequence_7_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_effect_25_sequence_7_step++;
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

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_d_rotation);
    wmap_draw_model(D_8011CF2C, 0, 0x7, 0x35, 0x7840, 0x1, g_wmap_effect_fade_d, 0, 0x1E, -1);
    value = g_wmap_effect_fade_d - 2;
    g_wmap_effect_fade_d = value;
    if (value < 0)
    {
        g_wmap_effect_fade_d = 0;
    }
    timer = g_wmap_land_effect_25_sequence_7_timer;
    g_wmap_effect_model_d_rotation.vz += -0x8;
    next_timer = timer - 1;
    g_wmap_land_effect_25_sequence_7_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_effect_25_sequence_7_step++;
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
    remaining = g_wmap_land_effect_25_sequence_8_timer - 1;
    D_801B2670.vz = (u16) (D_801B2670.vz + 0x18);
    g_wmap_land_effect_25_sequence_8_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_25_sequence_8_step += 1;
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
    remaining = g_wmap_land_effect_25_sequence_8_timer - 1;
    D_801B2670.vz = (u16) (D_801B2670.vz + 0x18);
    g_wmap_land_effect_25_sequence_8_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_25_sequence_8_step += 1;
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
    remaining = g_wmap_land_effect_25_sequence_9_timer - 1;
    D_801B2678.vz = (u16) (D_801B2678.vz + 0xC);
    g_wmap_land_effect_25_sequence_9_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_25_sequence_9_step += 1;
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
    remaining = g_wmap_land_effect_25_sequence_9_timer - 1;
    D_801B2678.vz = (u16) (D_801B2678.vz + 0xC);
    g_wmap_land_effect_25_sequence_9_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_25_sequence_9_step += 1;
    }
}

/** @brief Initialize effect actors with randomized angles and speeds. */
void wmap_land_effect_25_sequence_12_step_01(void)
{
    s32 i;
    WmapMotion *motion;
    WmapConfigA *actor;

    g_wmap_particle_intensity = 10;
    g_wmap_effect_params[11] = 0;
    g_wmap_effect_params[12] = -2;
    g_wmap_effect_params[13] = 220;
    g_wmap_effect_params[14] = 32;
    g_wmap_effect_params[15] = -1;
    g_wmap_effect_params[16] = -900;
    g_wmap_effect_params[17] = 20;
    g_wmap_effect_params[18] = 8;
    g_wmap_effect_params[19] = 0;
    g_wmap_effect_params[20] = 102000;
    i = 0;
    do
    {
        g_wmap_actor_animations[i + 20].data = g_wmap_animation_bank_3;
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
    g_wmap_land_effect_25_sequence_12_timer = 32;
    g_wmap_land_effect_25_sequence_12_step++;
    wmap_land_effect_25_sequence_12_step_02();
}

/** @brief Initialize effect actors with randomized angles and speeds. */
void wmap_land_effect_25_sequence_13_step_01(void)
{
    s32 i;
    WmapMotion *motion;
    WmapConfigA *actor;

    g_wmap_particle_intensity = 30;
    g_wmap_effect_params[21] = -1;
    g_wmap_effect_params[22] = -2;
    g_wmap_effect_params[23] = 180;
    g_wmap_effect_params[24] = 32;
    g_wmap_effect_params[25] = -1;
    g_wmap_effect_params[26] = -1600;
    g_wmap_effect_params[27] = 50;
    g_wmap_effect_params[28] = 8;
    g_wmap_effect_params[29] = 1;
    g_wmap_effect_params[30] = 102000;
    i = 0;
    do
    {
        g_wmap_actor_animations[i + 50].data = g_wmap_animation_bank_3;
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
    g_wmap_land_effect_25_sequence_13_timer = 32;
    g_wmap_land_effect_25_sequence_13_step++;
    wmap_land_effect_25_sequence_13_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_25_sequence_14_step_01(void)
{
    s32 i;

    g_wmap_effect_params[0x28] = 40;
    g_wmap_effect_params[0x29] = 200;
    g_wmap_effect_params[0x2A] = -280;
    g_wmap_effect_params[0x2B] = 560;
    g_wmap_effect_params[0x2C] = -280;
    g_wmap_effect_params[0x2D] = 560;
    g_wmap_effect_params[0x2E] = 200;
    g_wmap_effect_params[0x2F] = 80;
    g_wmap_effect_params[0x30] = 129;
    g_wmap_effect_params[0x31] = 1;
    g_wmap_effect_params[0x32] = 2;
    g_wmap_effect_params[0x33] = 40;
    g_wmap_effect_params[0x34] = 8;
    g_wmap_effect_params[0x35] = 2;
    for (i = 0; i < 40; i++)
    {
        g_wmap_actor_motions[i + 200].state = 0;
        g_wmap_actor_animations[i + 200].data = g_wmap_animation_bank_3;
    }
    g_wmap_land_effect_25_sequence_14_timer = 160;
    g_wmap_land_effect_25_sequence_14_step++;
    wmap_land_effect_25_sequence_14_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_25_sequence_15_step_01(void)
{
    s32 i;

    g_wmap_effect_params[0x1] = 1;
    g_wmap_effect_params[0x2] = 8;
    g_wmap_effect_params[0x3] = 0x40;
    g_wmap_effect_params[0x4] = 0;
    g_wmap_effect_params[0x5] = 1;
    g_wmap_effect_params[0x6] = 0x5DC;
    g_wmap_effect_params[0x7] = 0xAA;
    g_wmap_effect_params[0x8] = 8;
    g_wmap_effect_params[0x9] = 3;
    g_wmap_effect_params[0xA] = 0x32C8;
    for (i = 0; i < 24; i++)
    {
        g_wmap_actor_motions[i + 170].state = 0;
        g_wmap_actor_animations[i + 170].data = g_wmap_animation_bank_3;
    }
    g_wmap_land_effect_25_sequence_15_timer = 24;
    g_wmap_land_effect_25_sequence_15_step++;
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
        g_wmap_actor_motions[i].state = 0;
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_3;
        entry = &g_wmap_sprite_actors[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 1;
        entry->unk10 = -1;
        i++;
    } while (i < 155);

    D_800D9154 = 2;
    g_wmap_land_effect_25_sequence_17_timer = 0x10;
    g_wmap_land_effect_25_sequence_17_step += 1;
    wmap_land_effect_25_sequence_17_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_25_run, D_800D6FDC, 0x6, g_wmap_land_effect_25_step, g_wmap_land_effect_25_timer)

WMAP_STEP_RESET(wmap_land_effect_25_reset, g_wmap_land_effect_25_step, g_wmap_land_effect_25_timer)

/**
 * @brief Register a world-map callback and advance to the next step.
 */
void wmap_land_effect_25_step_01(void)
{
    D_800D923C = 1;
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_25_step += 1;
    wmap_land_effect_25_wait_idle_02();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_25_wait_idle_02, g_wmap_land_effect_25_step, wmap_land_effect_25_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_25_step_03, g_wmap_land_effect_25_step, wmap_land_effect_25_run_timeline, wmap_land_effect_25_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_25_wait_idle_04, g_wmap_land_effect_25_step, wmap_land_effect_25_step_05)

/**
 * @brief World-map step handler: clear the shared flag and advance the step counter.
 */
void wmap_land_effect_25_step_05(void)
{
    D_800D923C = 0;
    g_wmap_land_effect_25_step += 1;
}

WMAP_STEP_RUNNER(wmap_land_effect_25_run_timeline, D_800D6FF4, 0x20, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_25_timeline_reset, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

/** @brief World-map step: mark active, request a resource, seed the sub-counter, tick. */
void wmap_land_effect_25_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_play_sound(0x2F, 0x80);
    g_wmap_land_effect_25_timeline_timer = 2;
    g_wmap_land_effect_25_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_02, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_25_timeline_step_03, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer,
                         wmap_land_effect_25_run_sequence_1, 0x10)

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_04, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

/** @brief Register callbacks, set effect flags and color, and begin an eight-tick delay. */
void wmap_land_effect_25_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_25_run_sequence_15);
    wmap_start_sequence(wmap_land_effect_25_run_sequence_3);
    g_wmap_transition_mesh_hidden = 1;
    g_wmap_backdrop_target_level = 1;
    wmap_start_map_tint(0x701020);
    g_wmap_placement_overlay_hidden = 1;
    g_wmap_land_effect_25_timeline_timer = 8;
    g_wmap_land_effect_25_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_06, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

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
    g_wmap_land_effect_25_timeline_timer = 0x28;
    g_wmap_land_effect_25_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_08, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_25_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_25_run_sequence_6);
    wmap_start_sequence(wmap_land_effect_25_run_sequence_7);
    wmap_start_sequence(wmap_land_effect_25_run_sequence_14);
    g_wmap_land_effect_25_timeline_timer = 0x7;
    g_wmap_land_effect_25_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_10, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

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
    g_wmap_land_effect_25_timeline_timer = 0x23;
    g_wmap_land_effect_25_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_12, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_25_timeline_step_13, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer,
                             wmap_land_effect_25_run_sequence_18, wmap_land_effect_25_run_sequence_8, 0x20)

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_14, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_25_timeline_step_15, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer,
                         wmap_land_effect_25_run_sequence_12, 0x20)

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_16, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_25_timeline_step_17, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer,
                         wmap_land_effect_25_run_sequence_13, 0x5E)

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_18, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

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
    g_wmap_land_effect_25_timeline_timer = 1;
    g_wmap_land_effect_25_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_20, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

/** @brief World-map step handler: register three callbacks, reset state, advance the step. */
void wmap_land_effect_25_timeline_step_21(void)
{
    wmap_start_sequence(wmap_land_effect_25_run_sequence_17);
    wmap_start_sequence(wmap_land_effect_25_run_sequence_11);
    wmap_start_sequence(wmap_land_effect_25_run_sequence_9);
    D_8013B29C = 0;
    g_wmap_backdrop_target_level = 3;
    g_wmap_land_effect_25_timeline_timer = 0x10;
    g_wmap_land_effect_25_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_22, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_25_timeline_step_23, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer,
                         wmap_land_effect_25_run_sequence_16, 0x40)

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_24, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_25_timeline_step_25, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer,
                         wmap_land_effect_25_run_sequence_2, 0x9C)

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_26, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_25_timeline_step_27, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer,
                         wmap_land_effect_25_run_sequence_10, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_28, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

/** @brief Register a callback, update the selected map cell, and begin a 80-tick delay. */
void wmap_land_effect_25_timeline_step_29(void)
{
    wmap_start_sequence(&wmap_land_effect_25_run_sequence_5);
    g_wmap_transition_mesh_hidden = 0;
    g_wmap_backdrop_target_level = 15;
    wmap_start_map_tint(0x606070);
    g_wmap_land_effect_25_timeline_timer = 0x50;
    g_wmap_cells[g_wmap_focus_cell_x][g_wmap_focus_cell_y].value = g_wmap_selected_artifact | 0x100;
    g_wmap_land_effect_25_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_25_timeline_wait_30, g_wmap_land_effect_25_timeline_step, g_wmap_land_effect_25_timeline_timer)

/**
 * @brief World-map step handler: clear the shared flag and advance the step counter.
 */
void wmap_land_effect_25_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    g_wmap_land_effect_25_timeline_step += 1;
}

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_1, D_800D7074, 0x6, g_wmap_land_effect_25_sequence_1_step, g_wmap_land_effect_25_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_1_reset, g_wmap_land_effect_25_sequence_1_step, g_wmap_land_effect_25_sequence_1_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_25_sequence_1_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[4];

    g_wmap_actor_animations[4].data = g_wmap_animation_bank_0;
    actor->scale_index = 0xF;
    actor->sequence = 3;
    actor->previous_sequence = -1;
    actor->shade_step = 8;
    actor->target_shade = 0x81;
    actor->resource_index = 0;
    actor->shade = 1;
    g_wmap_land_effect_25_sequence_1_timer = 0x80;
    g_wmap_land_effect_25_sequence_1_step += 1;
    wmap_land_effect_25_sequence_1_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_1_step_02, g_wmap_land_effect_25_sequence_1_step, g_wmap_land_effect_25_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0x10, 0x5, 0)

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_25_sequence_1_step_03(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[4];

    actor->shade_step = 4;
    actor->target_shade = 0;
    g_wmap_land_effect_25_sequence_1_timer = 0x20;
    g_wmap_land_effect_25_sequence_1_step += 1;
    wmap_land_effect_25_sequence_1_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_1_step_04, g_wmap_land_effect_25_sequence_1_step, g_wmap_land_effect_25_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0x10, 0x5, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_1_end, g_wmap_land_effect_25_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_2, D_800D708C, 0x4, g_wmap_land_effect_25_sequence_2_step, g_wmap_land_effect_25_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_2_reset, g_wmap_land_effect_25_sequence_2_step, g_wmap_land_effect_25_sequence_2_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_25_sequence_2_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    g_wmap_actor_animations[5].data = g_wmap_animation_bank_0;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 2;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->target_shade = 0x80;
    actor->shade = 0x80;
    g_wmap_land_effect_25_sequence_2_timer = 0xA0;
    g_wmap_land_effect_25_sequence_2_step += 1;
    wmap_land_effect_25_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_2_step_02, g_wmap_land_effect_25_sequence_2_step, g_wmap_land_effect_25_sequence_2_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x10, 0x8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_2_end, g_wmap_land_effect_25_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_3, D_800D709C, 0x4, g_wmap_land_effect_25_sequence_3_step, g_wmap_land_effect_25_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_3_reset, g_wmap_land_effect_25_sequence_3_step, g_wmap_land_effect_25_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_25_sequence_3_step_01, g_wmap_land_effect_25_sequence_3_step, g_wmap_land_effect_25_sequence_3_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40, wmap_land_effect_25_sequence_3_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_3_end, g_wmap_land_effect_25_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_4, D_800D70AC, 0x4, g_wmap_land_effect_25_sequence_4_step, g_wmap_land_effect_25_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_4_reset, g_wmap_land_effect_25_sequence_4_step, g_wmap_land_effect_25_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_25_sequence_4_step_01, g_wmap_land_effect_25_sequence_4_step, g_wmap_land_effect_25_sequence_4_timer, g_wmap_effect_model_b_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x40, wmap_land_effect_25_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_4_end, g_wmap_land_effect_25_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_5, D_800D70BC, 0x4, g_wmap_land_effect_25_sequence_5_step, g_wmap_land_effect_25_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_5_reset, g_wmap_land_effect_25_sequence_5_step, g_wmap_land_effect_25_sequence_5_timer)

WMAP_STEP_DROP_START(wmap_land_effect_25_sequence_5_step_01, g_wmap_land_effect_25_sequence_5_step, g_wmap_land_effect_25_sequence_5_timer, g_wmap_effect_model_c_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_c_position, g_wmap_effect_fade_c, 0x80, 0xAFC8, 0x20, wmap_land_effect_25_sequence_5_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_5_end, g_wmap_land_effect_25_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_6, D_800D70CC, 0x6, g_wmap_land_effect_25_sequence_6_step, g_wmap_land_effect_25_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_6_reset, g_wmap_land_effect_25_sequence_6_step, g_wmap_land_effect_25_sequence_6_timer)

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void wmap_land_effect_25_sequence_6_step_01(void)
{
    D_80182DE4 = 1;
    D_801B2498 = g_wmap_zero_rotation;
    g_wmap_land_effect_25_sequence_6_timer = 0xB4;
    g_wmap_land_effect_25_sequence_6_step += 1;
    wmap_land_effect_25_sequence_6_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_25_sequence_6_step_03, g_wmap_land_effect_25_sequence_6_step, g_wmap_land_effect_25_sequence_6_timer, 0x40,
                    wmap_land_effect_25_sequence_6_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_6_end, g_wmap_land_effect_25_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_7, D_800D70E4, 0x6, g_wmap_land_effect_25_sequence_7_step, g_wmap_land_effect_25_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_7_reset, g_wmap_land_effect_25_sequence_7_step, g_wmap_land_effect_25_sequence_7_timer)

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void wmap_land_effect_25_sequence_7_step_01(void)
{
    g_wmap_effect_fade_d = 1;
    g_wmap_effect_model_d_rotation = g_wmap_zero_rotation;
    g_wmap_land_effect_25_sequence_7_timer = 0xAE;
    g_wmap_land_effect_25_sequence_7_step += 1;
    wmap_land_effect_25_sequence_7_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_25_sequence_7_step_03, g_wmap_land_effect_25_sequence_7_step, g_wmap_land_effect_25_sequence_7_timer, 0x40,
                    wmap_land_effect_25_sequence_7_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_7_end, g_wmap_land_effect_25_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_8, D_800D70FC, 0x6, g_wmap_land_effect_25_sequence_8_step, g_wmap_land_effect_25_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_8_reset, g_wmap_land_effect_25_sequence_8_step, g_wmap_land_effect_25_sequence_8_timer)

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void wmap_land_effect_25_sequence_8_step_01(void)
{
    D_801B25D8 = 1;
    D_801B2670 = g_wmap_zero_rotation;
    D_80139234 = 0;
    g_wmap_land_effect_25_sequence_8_timer = 0x84;
    g_wmap_land_effect_25_sequence_8_step += 1;
    wmap_land_effect_25_sequence_8_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_25_sequence_8_step_03, g_wmap_land_effect_25_sequence_8_step, g_wmap_land_effect_25_sequence_8_timer, 0x20,
                    wmap_land_effect_25_sequence_8_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_8_end, g_wmap_land_effect_25_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_9, D_800D7114, 0x6, g_wmap_land_effect_25_sequence_9_step, g_wmap_land_effect_25_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_9_reset, g_wmap_land_effect_25_sequence_9_step, g_wmap_land_effect_25_sequence_9_timer)

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void wmap_land_effect_25_sequence_9_step_01(void)
{
    D_801B25DC = 1;
    D_801B2678 = g_wmap_zero_rotation;
    D_8013923C = 0;
    g_wmap_land_effect_25_sequence_9_timer = 0x84;
    g_wmap_land_effect_25_sequence_9_step += 1;
    wmap_land_effect_25_sequence_9_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_25_sequence_9_step_03, g_wmap_land_effect_25_sequence_9_step, g_wmap_land_effect_25_sequence_9_timer, 0x40,
                    wmap_land_effect_25_sequence_9_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_9_end, g_wmap_land_effect_25_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_10, D_800D712C, 0x4, g_wmap_land_effect_25_sequence_10_step, g_wmap_land_effect_25_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_10_reset, g_wmap_land_effect_25_sequence_10_step, g_wmap_land_effect_25_sequence_10_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_25_sequence_10_step_01(void)
{
    g_wmap_actor_animations[7].data = g_wmap_animation_bank_0;
    D_800D939C.scale_index = 0xF;
    D_800D939C.sequence = 2;
    D_800D939C.previous_sequence = -1;
    D_800D939C.shade_step = 8;
    D_800D939C.resource_index = 0;
    D_800D939C.target_shade = 0x81;
    D_800D939C.shade = 0x81;
    g_wmap_land_effect_25_sequence_10_timer = 0x24;
    g_wmap_land_effect_25_sequence_10_step += 1;
    wmap_land_effect_25_sequence_10_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_10_step_02, g_wmap_land_effect_25_sequence_10_step, g_wmap_land_effect_25_sequence_10_timer,
                              D_800D939C, D_801399C0, g_wmap_focus_screen_position, 0x10, 0x4, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_10_end, g_wmap_land_effect_25_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_11, D_800D713C, 0x6, g_wmap_land_effect_25_sequence_11_step, g_wmap_land_effect_25_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_11_reset, g_wmap_land_effect_25_sequence_11_step, g_wmap_land_effect_25_sequence_11_timer)

/** @brief Initialize the world-map actor and advance the timed sequence. */
void wmap_land_effect_25_sequence_11_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    g_wmap_actor_animations[6].data = g_wmap_animation_bank_0;
    actor->scale_index = 0xF;
    actor->resource_index = 0;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    actor->shade_step = 4;
    actor->target_shade = 0x81;
    actor->shade = 1;
    g_wmap_land_effect_25_sequence_11_timer = 0x50;
    g_wmap_land_effect_25_sequence_11_step += 1;
    wmap_land_effect_25_sequence_11_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_11_step_02, g_wmap_land_effect_25_sequence_11_step, g_wmap_land_effect_25_sequence_11_timer,
                              g_wmap_sprite_actors[6], g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x10, 0x7, 0)

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_25_sequence_11_step_03(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    actor->shade_step = 2;
    actor->target_shade = 0;
    g_wmap_land_effect_25_sequence_11_timer = 0x40;
    g_wmap_land_effect_25_sequence_11_step += 1;
    wmap_land_effect_25_sequence_11_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_11_step_04, g_wmap_land_effect_25_sequence_11_step, g_wmap_land_effect_25_sequence_11_timer,
                              g_wmap_sprite_actors[6], g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x10, 0x7, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_11_end, g_wmap_land_effect_25_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_12, D_800D7154, 0x6, g_wmap_land_effect_25_sequence_12_step, g_wmap_land_effect_25_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_12_reset, g_wmap_land_effect_25_sequence_12_step, g_wmap_land_effect_25_sequence_12_timer)

/** @brief World-map step handler: spawn a sprite and count down the shared timer. */
void wmap_land_effect_25_sequence_12_step_02(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0xA, 1, 0xFF, 2, 6, (s32)g_wmap_effect_params + 0x28);
    if (--g_wmap_land_effect_25_sequence_12_timer == 0)
    {
        g_wmap_land_effect_25_sequence_12_step += 1;
    }
}

/** @brief World-map step handler: arm a config range, set the timer, and advance. */
void wmap_land_effect_25_sequence_12_step_03(void)
{
    s32 i;

    for (i = 0; i < 0xA; i++)
    {
        g_wmap_sprite_actors[i + 0x14].shade_step = 2;
    }
    g_wmap_land_effect_25_sequence_12_timer = 0x64;
    g_wmap_land_effect_25_sequence_12_step += 1;
    wmap_land_effect_25_sequence_12_step_04();
}

/** @brief World-map step handler: spawn a sprite and count down the shared timer. */
void wmap_land_effect_25_sequence_12_step_04(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0xA, 1, 0xFF, 2, 7, (s32)g_wmap_effect_params + 0x28);
    if (--g_wmap_land_effect_25_sequence_12_timer == 0)
    {
        g_wmap_land_effect_25_sequence_12_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_12_end, g_wmap_land_effect_25_sequence_12_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_13, D_800D716C, 0x6, g_wmap_land_effect_25_sequence_13_step, g_wmap_land_effect_25_sequence_13_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_13_reset, g_wmap_land_effect_25_sequence_13_step, g_wmap_land_effect_25_sequence_13_timer)

/** @brief World-map step handler: spawn a sprite and count down the shared timer. */
void wmap_land_effect_25_sequence_13_step_02(void)
{
    func_8006A2FC(D_800D9B00, D_80139B18, 0x1E, 1, 0xFF, 4, 6, (s32)g_wmap_effect_params + 0x50);
    if (--g_wmap_land_effect_25_sequence_13_timer == 0)
    {
        g_wmap_land_effect_25_sequence_13_step += 1;
    }
}

/** @brief World-map step handler: arm a config range, set the timer, and advance. */
void wmap_land_effect_25_sequence_13_step_03(void)
{
    s32 i;

    for (i = 0; i < 0x1E; i++)
    {
        g_wmap_sprite_actors[i + 0x32].shade_step = 4;
    }
    g_wmap_land_effect_25_sequence_13_timer = 0x40;
    g_wmap_land_effect_25_sequence_13_step += 1;
    wmap_land_effect_25_sequence_13_step_04();
}

/** @brief World-map step handler: spawn a sprite and count down the shared timer. */
void wmap_land_effect_25_sequence_13_step_04(void)
{
    func_8006A2FC(D_800D9B00, D_80139B18, 0x1E, 1, 0xFF, 4, 7, (s32)g_wmap_effect_params + 0x50);
    if (--g_wmap_land_effect_25_sequence_13_timer == 0)
    {
        g_wmap_land_effect_25_sequence_13_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_13_end, g_wmap_land_effect_25_sequence_13_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_14, D_800D7184, 0x6, g_wmap_land_effect_25_sequence_14_step, g_wmap_land_effect_25_sequence_14_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_14_reset, g_wmap_land_effect_25_sequence_14_step, g_wmap_land_effect_25_sequence_14_timer)

/** @brief Kick a world-map sub-handler off the shared context, then step counters. */
void wmap_land_effect_25_sequence_14_step_02(void)
{
    func_8006C448((s32)g_wmap_effect_params + 0xA0);
    if (--g_wmap_land_effect_25_sequence_14_timer == 0)
    {
        g_wmap_land_effect_25_sequence_14_step += 1;
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

    g_wmap_land_effect_25_sequence_14_timer = 0x40;
    g_wmap_land_effect_25_sequence_14_step += 1;
    wmap_land_effect_25_sequence_14_step_04();
}

/** @brief Kick a world-map sub-handler off the shared context, then step counters. */
void wmap_land_effect_25_sequence_14_step_04(void)
{
    func_8006C448((s32)g_wmap_effect_params + 0xA0);
    if (--g_wmap_land_effect_25_sequence_14_timer == 0)
    {
        g_wmap_land_effect_25_sequence_14_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_14_end, g_wmap_land_effect_25_sequence_14_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_15, D_800D719C, 0x6, g_wmap_land_effect_25_sequence_15_step, g_wmap_land_effect_25_sequence_15_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_15_reset, g_wmap_land_effect_25_sequence_15_step, g_wmap_land_effect_25_sequence_15_timer)

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_25_sequence_15_step_02(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DAFA0, D_80139ED8, 0x18, 0xFF, 0x1, 0x4, 0, (s32)g_wmap_effect_params);
    if (--g_wmap_land_effect_25_sequence_15_timer == 0)
    {
        g_wmap_land_effect_25_sequence_15_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_25_sequence_15_step_03(void)
{
    g_wmap_land_effect_25_sequence_15_timer = 0x40;
    g_wmap_effect_params[5] = -1;
    g_wmap_land_effect_25_sequence_15_step += 1;
    wmap_land_effect_25_sequence_15_step_04();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void wmap_land_effect_25_sequence_15_step_04(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DAFA0, D_80139ED8, 0x18, 0xFF, 0x1, 0x4, 0, (s32)g_wmap_effect_params);
    if (--g_wmap_land_effect_25_sequence_15_timer == 0)
    {
        g_wmap_land_effect_25_sequence_15_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_15_end, g_wmap_land_effect_25_sequence_15_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_16, D_800D71B4, 0x6, g_wmap_land_effect_25_sequence_16_step, g_wmap_land_effect_25_sequence_16_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_16_reset, g_wmap_land_effect_25_sequence_16_step, g_wmap_land_effect_25_sequence_16_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_25_sequence_16_step_01(void)
{
    g_wmap_actor_animations[9].data = g_wmap_animation_bank_2;
    D_800D93F4.scale_index = 0xF;
    D_800D93F4.previous_sequence = -1;
    D_800D93F4.shade_step = 4;
    D_800D93F4.target_shade = 0x81;
    D_800D93F4.resource_index = 0;
    D_800D93F4.sequence = 0;
    D_800D93F4.shade = 1;
    g_wmap_land_effect_25_sequence_16_timer = 0x78;
    g_wmap_land_effect_25_sequence_16_step += 1;
    wmap_land_effect_25_sequence_16_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_16_step_02, g_wmap_land_effect_25_sequence_16_step, g_wmap_land_effect_25_sequence_16_timer,
                              D_800D93F4, D_801399D0, g_wmap_focus_screen_position, 0x8, 0x5, 0)

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_25_sequence_16_step_03(void)
{
    D_800D93F4.shade_step = 2;
    D_800D93F4.target_shade = 0;
    g_wmap_land_effect_25_sequence_16_timer = 0x40;
    g_wmap_land_effect_25_sequence_16_step += 1;
    wmap_land_effect_25_sequence_16_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_16_step_04, g_wmap_land_effect_25_sequence_16_step, g_wmap_land_effect_25_sequence_16_timer,
                              D_800D93F4, D_801399D0, g_wmap_focus_screen_position, 0x8, 0x5, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_16_end, g_wmap_land_effect_25_sequence_16_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_17, D_800D71CC, 0x8, g_wmap_land_effect_25_sequence_17_step, g_wmap_land_effect_25_sequence_17_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_17_reset, g_wmap_land_effect_25_sequence_17_step, g_wmap_land_effect_25_sequence_17_timer)

/** @brief World-map effect spawn: submit a request and tick a refcount. */
void wmap_land_effect_25_sequence_17_step_02(void)
{
    s32 c;

    func_8006B328(0x6E, 0x9B, 2, -1, 3, 2, 0x168, 8, -0x78, 0xF0, -0x78,
                  0xF0, 0x64, 0x81, 0x81, 4, 1);
    D_801B25E0 += 8;
    c = g_wmap_land_effect_25_sequence_17_timer - 1;
    g_wmap_land_effect_25_sequence_17_timer = c;
    if (c == 0)
    {
        g_wmap_land_effect_25_sequence_17_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_25_sequence_17_step_03, g_wmap_land_effect_25_sequence_17_step, g_wmap_land_effect_25_sequence_17_timer, 0x5A,
                    wmap_land_effect_25_sequence_17_step_04)

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_25_sequence_17_step_04(void)
{
    func_8006B328(0x6E, 0x9B, 2, -1, 3, 2, 0x168, 8, -0x78, 0xF0, -0x78, 0xF0, 0x64, 0x81, 0x81, 4, 1);
    if (--g_wmap_land_effect_25_sequence_17_timer == 0)
    {
        g_wmap_land_effect_25_sequence_17_step += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_effect_25_sequence_17_step_05(void)
{
    D_800DCEAC = 0;
    g_wmap_land_effect_25_sequence_17_timer = 0x64;
    g_wmap_land_effect_25_sequence_17_step += 1;
    wmap_land_effect_25_sequence_17_step_06();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void wmap_land_effect_25_sequence_17_step_06(void)
{
    func_8006B328(0x6E, 0x9B, 2, -1, 3, 2, 0x168, 8, -0x78, 0xF0, -0x78, 0xF0, 0x64, 0x81, 0x81, 4, 1);
    if (--g_wmap_land_effect_25_sequence_17_timer == 0)
    {
        g_wmap_land_effect_25_sequence_17_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_17_end, g_wmap_land_effect_25_sequence_17_step)

WMAP_STEP_RUNNER(wmap_land_effect_25_run_sequence_18, D_800D71EC, 0x6, g_wmap_land_effect_25_sequence_18_step, g_wmap_land_effect_25_sequence_18_timer)

WMAP_STEP_RESET(wmap_land_effect_25_sequence_18_reset, g_wmap_land_effect_25_sequence_18_step, g_wmap_land_effect_25_sequence_18_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_25_sequence_18_step_01(void)
{
    g_wmap_actor_animations[8].data = g_wmap_animation_bank_1;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.previous_sequence = -1;
    D_800D93C8.shade_step = 4;
    D_800D93C8.target_shade = 0x81;
    D_800D93C8.resource_index = 0;
    D_800D93C8.sequence = 0;
    D_800D93C8.shade = 1;
    g_wmap_land_effect_25_sequence_18_timer = 0x96;
    g_wmap_land_effect_25_sequence_18_step += 1;
    wmap_land_effect_25_sequence_18_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_18_step_02, g_wmap_land_effect_25_sequence_18_step, g_wmap_land_effect_25_sequence_18_timer,
                              D_800D93C8, D_801399C8, g_wmap_focus_screen_position, 0x11, 0x4, 0)

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_25_sequence_18_step_03(void)
{
    D_800D93C8.shade_step = 4;
    D_800D93C8.target_shade = 0;
    g_wmap_land_effect_25_sequence_18_timer = 0x20;
    g_wmap_land_effect_25_sequence_18_step += 1;
    wmap_land_effect_25_sequence_18_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_25_sequence_18_step_04, g_wmap_land_effect_25_sequence_18_step, g_wmap_land_effect_25_sequence_18_timer,
                              D_800D93C8, D_801399C8, g_wmap_focus_screen_position, 0x11, 0x4, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_25_sequence_18_end, g_wmap_land_effect_25_sequence_18_step)
