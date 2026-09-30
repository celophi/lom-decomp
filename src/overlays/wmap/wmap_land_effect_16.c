#include "wmap_main.h"
#include "wmap_land_effect_16.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

/** @brief World-map tile and its cached neighboring layout data. */
typedef struct
{
    s32 tile;
    s16 field_04;
    s16 field_06;
    u8 neighbors[32];
} WmapTile;

void func_80098CD8(void);
void func_800995DC(void);
void func_80097D24(void);
void func_80097D60(void);
s32 func_80097DF8(s32 arg0);
void func_80097DA4(void);
void func_80097DE0(void);
s32 func_800983E8(s32 arg0);
s32 func_80098C48(s32 arg0);
s32 func_80098998(s32 arg0);
s32 func_80099120(s32 arg0);
s32 func_800992C4(s32 arg0);
s32 func_8009858C(s32 arg0);
s32 func_80098FC8(s32 arg0);
s32 func_8009954C(s32 arg0);
s32 func_800987F8(s32 arg0);
s32 func_80098AF0(s32 arg0);
s32 func_80098E28(s32 arg0);
void func_800984F8(void);
void func_8009869C(void);
void func_80098764(void);
void func_80098904(void);
void func_80098DB4(void);
void func_80098F34(void);
void func_80099230(void);
void func_800993F0(void);
void func_800994B8(void);
void func_800996B0(void);

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

extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2C0C;
extern s32 D_80182DEC;
extern u8* D_8011CF1C;
extern s32 D_801B2C14;
extern s32 D_801B2C1C;
extern u8* D_8011CF24;
extern s32 D_8013923C;
extern s32 D_80182DF0;
extern s32 D_801B2C2C;
extern s32 D_801B0FD0;
extern s32 D_801B2C44;
extern s32 D_801B2BE4;
extern void (*D_800D63B8[])(void);
extern s32 D_8013B20C;
extern void func_80097D60(void);
extern void func_80097DE0(void);
extern s32 D_801B2BEC;
extern void (*D_800D63D0[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern WmapTile D_80139290[6][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2BF4;
extern void (*D_800D6430[])(void);
extern u8* D_801399AC;
extern void func_800984F8(void);
extern s32 D_801B2BFC;
extern void (*D_800D6440[])(void);
extern void *D_801399D4;
extern void func_80098764(void);
extern s32 D_801B2C04;
extern void (*D_800D6458[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern void func_80098904(void);
extern void (*D_800D6468[])(void);
extern void (*D_800D6478[])(void);
extern void (*D_800D6488[])(void);
extern s32 D_801B2C24;
extern void (*D_800D64A0[])(void);
extern u8 *D_801399BC;
extern void (*D_800D64B0[])(void);
extern s32 D_801B2C34;
extern void (*D_800D64C8[])(void);
extern u8 D_80121538[];
extern u8* D_801399DC;
extern void func_80099230(void);
extern s32 D_801B2C3C;
extern void (*D_800D64D8[])(void);
extern u8 *D_801399E4;
extern void func_800994B8(void);
extern void (*D_800D64F0[])(void);
extern u8 D_800DB578[];
extern WmapAnimationSlot D_80139FE8[];
extern void func_800996B0(void);

/** @brief World-map actor configuration with its original field layout. */
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

extern u32 D_801B2C08;
extern u32 D_801B2C10;
extern u8 D_8011F538[];
extern u32 D_801B2C18;
extern u32 D_801B2C28;
extern u32 D_801B2C40;
extern u32 D_801B2BE0;
extern u32 D_801B2BE8;
extern u32 D_801B2BF0;
extern u32 D_801B2BF8;
extern u32 D_801B2C00;
extern u32 D_801B2C20;
extern u32 D_801B2C30;
extern u32 D_801B2C38;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;

extern SVECTOR D_80139258;
extern SVECTOR D_801B24A0;
extern SVECTOR D_8013B238;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B24A8;

extern WmapSpriteActor D_800D9268[];
extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370;
extern WmapSpriteActor D_800D93F4;
extern WmapSpriteActor D_800D9420;
extern WmapSpriteActor D_800D944C;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8;
extern WmapAnimationSlot D_801399D0;
extern WmapAnimationSlot D_801399D8;
extern WmapAnimationSlot D_801399E0;

extern WmapScreenPosition g_wmap_focus_screen_position;
extern WmapScreenPosition D_80182D60;

extern s32* D_80139280;

extern WmapMotion D_801AFBD0[];

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80097624(void)
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
    if (--D_801B2C0C == 0)
    {
        D_801B2C08 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80097724(void)
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
    if (--D_801B2C14 == 0)
    {
        D_801B2C10 += 1;
    }
}

/** @brief Initialize four effect actors and their angular spacing. */
void func_80097824(void)
{
    s32 i;
    s32 *descriptor;
    WmapConfigA *actor;

    D_801B2490 = D_80139258;
    descriptor = D_80139280;
    descriptor[0] = 1;
    descriptor[1] = 1;
    descriptor[2] = 1;
    D_80139280[3] = 0;
    D_80139280[4] = 1000;
    D_80139280[5] = 96;
    D_80139280[6] = 8;
    D_80139280[7] = 255;
    D_80139280[8] = 10;
    D_80139280[9] = 19;
    for (i = 20; i < 80; i++)
    {
        D_801AFBD0[i].state = 0;
    }
    for (i = 20; i < 80; i += 15)
    {
        actor = &D_800D9268[i];
        D_80139988[i].data = D_8011F538;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_02 = 0;
        actor->field_0E = 0;
        actor->field_26 = 2;
        actor->field_22 = 128;
        actor->field_24 = 128;
        D_801AFBD0[i].state = 1;
        D_801AFBD0[i].z = 0;
        D_801AFBD0[i].angle = D_80139280[3];
        D_801AFBD0[i].field_0E = 0;
        D_80139280[3] += 1024;
    }
    D_801B2C1C = 48;
    D_801B2C18++;
    func_80098CD8();
}

/** @brief Advance the world-map effect and its sequence state. */
void func_800979A8(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model_default(D_8011CF24, D_8013923C, 4, 0x35, 0x7800, 0x1001, D_80182DF0);
    D_8013B238.vz = (u16) (D_8013B238.vz + 0x10);
    intensity = D_80182DF0 + 2;
    D_80182DF0 = intensity;
    if (intensity >= 0x82)
    {
        D_80182DF0 = 0x81;
    }
    D_8013923C = (D_8013923C + 1) & 7;
    PopMatrix();
    remaining = D_801B2C2C - 1;
    D_801B2C2C = remaining;
    if (remaining == 0)
    {
        D_801B2C28 += 1;
    }
}

/** @brief Advance the world-map effect and its sequence state. */
void func_80097A94(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model_default(D_8011CF24, D_8013923C, 4, 0x35, 0x7800, 0x1001, D_80182DF0);
    intensity = D_80182DF0 - 2;
    D_8013B238.vz = (u16) (D_8013B238.vz + 0x10);
    D_80182DF0 = intensity;
    if (intensity < 0)
    {
        D_80182DF0 = 0;
    }
    D_8013923C = (D_8013923C + 1) & 7;
    PopMatrix();
    remaining = D_801B2C2C - 1;
    D_801B2C2C = remaining;
    if (remaining == 0)
    {
        D_801B2C28 += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_80097B78(void)
{
    s32 i;

    D_801B0FD0 = 40;
    D_80139280[0x1F] = -1;
    D_80139280[0x20] = -4;
    D_80139280[0x21] = 0x20;
    D_80139280[0x22] = 0;
    D_80139280[0x23] = 2;
    D_80139280[0x24] = 0;
    D_80139280[0x25] = 0xC8;
    D_80139280[0x26] = 0x13;
    D_80139280[0x27] = 2;
    D_80139280[0x28] = 0x1F40;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + 200].state = 0;
        D_80139988[i + 204].data = D_8011F538;
    }
    D_801B2C44 = 80;
    D_801B2C40++;
    func_800995DC();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80097C50(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BE0 = 1;
        D_801B2BE4 = 1;
        return 1;
    }

    if (D_801B2BE0 < 0x6)
    {
        D_800D63B8[D_801B2BE0]();
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
void func_80097CC8(void)
{
    D_801B2BE0 = 1;
    D_801B2BE4 = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80097CE0(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    D_8013B20C = 1;
    D_801B2BE0 += 1;
    func_80097D24();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80097D24(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2BE0 += 1;
        func_80097D60();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80097D60(void)
{
    wmap_start_sequence(func_80097DF8);
    D_8013B20C = 1;
    D_801B2BE0 += 1;
    func_80097DA4();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80097DA4(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2BE0 += 1;
        func_80097DE0();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80097DE0(void)
{
    D_801B2BE0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80097DF8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BE8 = 1;
        D_801B2BEC = 1;
        return 1;
    }

    if (D_801B2BE8 < 0x18)
    {
        D_800D63D0[D_801B2BE8]();
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
void func_80097E70(void)
{
    D_801B2BE8 = 1;
    D_801B2BEC = 1;
}

/** @brief World-map step handler: kick a sub-task and advance the step counter. */
void func_80097E88(void)
{
    D_8013B208 = 1;
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 4;
    D_801B2BEC = 0x1E;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80097EDC(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/** @brief World-map step handler: register a callback and advance the step counter. */
void func_80097F10(void)
{
    wmap_play_sound(0x27, 0x80);
    wmap_start_sequence(func_800983E8);
    D_801B2BEC = 0x10;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80097F58(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80097F8C(void)
{
    wmap_start_sequence(func_80098C48);
    D_801B2BEC = 0x2;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80097FC8(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void func_80097FFC(void)
{
    D_801ADAE0 = 1;
    D_801B2BEC = 0x28;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80098028(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009805C(void)
{
    wmap_start_sequence(func_80098998);
    D_801B2BEC = 0x4;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80098098(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800980CC(void)
{
    wmap_start_sequence(func_80099120);
    D_801B2BEC = 0x14;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80098108(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009813C(void)
{
    wmap_start_sequence(func_800992C4);
    D_801B2BEC = 0x1E;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80098178(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800981AC(void)
{
    wmap_start_sequence(func_8009858C);
    wmap_start_sequence(func_80098FC8);
    D_801B2BEC = 0x8;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800981F4(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80098228(void)
{
    wmap_start_sequence(func_8009954C);
    D_801B2BEC = 0xF;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80098264(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80098298(void)
{
    wmap_start_sequence(func_800987F8);
    D_801B2BEC = 0x80;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800982D4(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_80098308(void)
{
    wmap_start_sequence(func_80098AF0);
    wmap_start_sequence(func_80098E28);
    D_801B2BEC = 0x80;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80098350(void)
{
    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/** @brief Mark the current world-map tile state and advance the sequence. */
void func_80098384(void)
{
    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].tile = D_8011D4FC | 0x100;
    D_801B2BE8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800983E8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BF0 = 1;
        D_801B2BF4 = 1;
        return 1;
    }

    if (D_801B2BF0 < 0x4)
    {
        D_800D6430[D_801B2BF0]();
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
void func_80098460(void)
{
    D_801B2BF0 = 1;
    D_801B2BF4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80098478(void)
{
    D_801399AC = D_8011F538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 0x10;
    D_800D9318.target_shade = 0x81;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.shade = 1;
    D_801B2BF4 = 0x40;
    D_801B2BF0 += 1;
    func_800984F8();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800984F8(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0x13, 0x2, 0);
    if (--D_801B2BF4 == 0)
    {
        D_801B2BF0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80098574(void)
{
    D_801B2BF0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009858C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BF8 = 1;
        D_801B2BFC = 1;
        return 1;
    }

    if (D_801B2BF8 < 0x6)
    {
        D_800D6440[D_801B2BF8]();
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
void func_80098604(void)
{
    D_801B2BF8 = 1;
    D_801B2BFC = 1;
}

/** @brief Initialize the world-map actor and advance the timed sequence. */
void func_8009861C(void)
{
    D_801399D4 = &D_8011F538;
    D_800D93F4.scale_index = 0xF;
    D_800D93F4.resource_index = 0;
    D_800D93F4.sequence = 1;
    D_800D93F4.previous_sequence = -1;
    D_800D93F4.shade_step = 2;
    D_800D93F4.target_shade = 0x81;
    D_800D93F4.shade = 1;
    D_801B2BFC = 0x64;
    D_801B2BF8 += 1;
    func_8009869C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009869C(void)
{
    wmap_step_actor_animation(&D_800D93F4, &D_801399D0);
    wmap_draw_actor_sprite(&D_800D93F4, g_wmap_focus_screen_position.packed, 0x13, 0x2, 0);
    if (--D_801B2BFC == 0)
    {
        D_801B2BF8 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80098718(void)
{
    D_800D93F4.target_shade = 0;
    D_800D93F4.shade_step = 2;
    D_801B2BFC = 0x40;
    D_801B2BF8 += 1;
    func_80098764();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80098764(void)
{
    wmap_step_actor_animation(&D_800D93F4, &D_801399D0);
    wmap_draw_actor_sprite(&D_800D93F4, g_wmap_focus_screen_position.packed, 0x13, 0x2, 0);
    if (--D_801B2BFC == 0)
    {
        D_801B2BF8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800987E0(void)
{
    D_801B2BF8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800987F8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C00 = 1;
        D_801B2C04 = 1;
        return 1;
    }

    if (D_801B2C00 < 0x4)
    {
        D_800D6458[D_801B2C00]();
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
void func_80098870(void)
{
    D_801B2C00 = 1;
    D_801B2C04 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80098888(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 1;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0;
    D_801B2C04 = 0x81;
    D_801B2C00 += 1;
    func_80098904();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80098904(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x18, 0x8, 0);
    if (--D_801B2C04 == 0)
    {
        D_801B2C00 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80098980(void)
{
    D_801B2C00 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80098998(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C08 = 1;
        D_801B2C0C = 1;
        return 1;
    }

    if (D_801B2C08 < 0x4)
    {
        D_800D6468[D_801B2C08]();
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
void func_80098A10(void)
{
    D_801B2C08 = 1;
    D_801B2C0C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80098A28(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B2C0C = 0x40;
    D_801B2C08 += 1;
    func_80097624();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80098AD8(void)
{
    D_801B2C08 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80098AF0(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C10 = 1;
        D_801B2C14 = 1;
        return 1;
    }

    if (D_801B2C10 < 0x4)
    {
        D_800D6478[D_801B2C10]();
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
void func_80098B68(void)
{
    D_801B2C10 = 1;
    D_801B2C14 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80098B80(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    D_801B2C14 = 0x40;
    D_801B2C10 += 1;
    func_80097724();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80098C30(void)
{
    D_801B2C10 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80098C48(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C18 = 1;
        D_801B2C1C = 1;
        return 1;
    }

    if (D_801B2C18 < 0x6)
    {
        D_800D6488[D_801B2C18]();
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
void func_80098CC0(void)
{
    D_801B2C18 = 1;
    D_801B2C1C = 1;
}

/** @brief World-map step: kick off a descriptor animation, then tick the sub-counter. */
void func_80098CD8(void)
{
    func_8006BC44(0x14, 0x3C, D_80139280, 1);
    if (--D_801B2C1C == 0)
    {
        D_801B2C18 += 1;
    }
}

/** @brief Reset four actor configurations and begin a 16-tick sequence step. */
void func_80098D34(void)
{
    s32 index;

    D_80139280[0] = -1;
    D_80139280[4] = 0;
    D_80139280[5] = 0;
    for (index = 20; index < 80; index += 15)
    {
        D_800D9268[index].target_shade = 0;
        D_800D9268[index].shade_step = 8;
    }
    D_801B2C1C = 0x10;
    D_801B2C18 += 1;
    func_80098DB4();
}

/** @brief World-map step: kick off a descriptor animation, then tick the sub-counter. */
void func_80098DB4(void)
{
    func_8006BC44(0x14, 0x3C, D_80139280, 1);
    if (--D_801B2C1C == 0)
    {
        D_801B2C18 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80098E10(void)
{
    D_801B2C18 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80098E28(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C20 = 1;
        D_801B2C24 = 1;
        return 1;
    }

    if (D_801B2C20 < 0x4)
    {
        D_800D64A0[D_801B2C20]();
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
void func_80098EA0(void)
{
    D_801B2C20 = 1;
    D_801B2C24 = 1;
}

/** @brief Initialize the actor configuration and begin a 129-tick sequence step. */
void func_80098EB8(void)
{
    D_801399BC = D_8011D538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.sequence = 1;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 1;
    D_800D9370.resource_index = 0;
    D_800D9370.target_shade = 0x80;
    D_800D9370.shade = 0x80;
    D_801B2C24 = 0x81;
    D_801B2C20 += 1;
    func_80098F34();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80098F34(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x18, 0x8, 0);
    if (--D_801B2C24 == 0)
    {
        D_801B2C20 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80098FB0(void)
{
    D_801B2C20 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80098FC8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C28 = 1;
        D_801B2C2C = 1;
        return 1;
    }

    if (D_801B2C28 < 0x6)
    {
        D_800D64B0[D_801B2C28]();
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
void func_80099040(void)
{
    D_801B2C28 = 1;
    D_801B2C2C = 1;
}

/** @brief Restore effect data, clear two flags, and begin a 100-tick sequence step. */
void func_80099058(void)
{
    D_8013B238 = D_80139258;
    D_80182DF0 = 0;
    D_8013923C = 0;
    D_801B2C2C = 0x64;
    D_801B2C28 += 1;
    func_800979A8();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800990D0(void)
{
    D_801B2C2C = 0x40;
    D_801B2C28 += 1;
    func_80097A94();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80099108(void)
{
    D_801B2C28 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80099120(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C30 = 1;
        D_801B2C34 = 1;
        return 1;
    }

    if (D_801B2C30 < 0x4)
    {
        D_800D64C8[D_801B2C30]();
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
void func_80099198(void)
{
    D_801B2C30 = 1;
    D_801B2C34 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800991B0(void)
{
    D_801399DC = D_80121538;
    D_800D9420.scale_index = 0xF;
    D_800D9420.previous_sequence = -1;
    D_800D9420.shade_step = 4;
    D_800D9420.target_shade = 0x81;
    D_800D9420.resource_index = 0;
    D_800D9420.sequence = 0;
    D_800D9420.shade = 1;
    D_801B2C34 = 0x3C;
    D_801B2C30 += 1;
    func_80099230();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80099230(void)
{
    wmap_step_actor_animation(&D_800D9420, &D_801399D8);
    wmap_draw_actor_sprite(&D_800D9420, g_wmap_focus_screen_position.packed, 0x13, 0x2, 0);
    if (--D_801B2C34 == 0)
    {
        D_801B2C30 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800992AC(void)
{
    D_801B2C30 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800992C4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C38 = 1;
        D_801B2C3C = 1;
        return 1;
    }

    if (D_801B2C38 < 0x6)
    {
        D_800D64D8[D_801B2C38]();
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
void func_8009933C(void)
{
    D_801B2C38 = 1;
    D_801B2C3C = 1;
}

/** @brief Initialize the actor and its screen coordinates, then advance the sequence. */
void func_80099354(void)
{
    s16 screen_y;

    D_801399E4 = D_80121538;
    D_800D944C.scale_index = 0xF;
    D_800D944C.resource_index = 0;
    D_800D944C.sequence = 1;
    D_800D944C.previous_sequence = -1;
    D_800D944C.shade_step = 0x20;
    D_800D944C.target_shade = 0x81;
    D_800D944C.shade = 1;
    D_801B2C3C = 0x80;
    D_80182D60.point.x = (u16) g_wmap_focus_screen_position.packed;
    screen_y = g_wmap_focus_screen_position.packed - 0x38;
    D_80182D60.point.y = screen_y;
    D_801B2C38 += 1;
    func_800993F0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800993F0(void)
{
    wmap_step_actor_animation(&D_800D944C, &D_801399E0);
    wmap_draw_actor_sprite(&D_800D944C, D_80182D60.packed, 0x13, 0x2, 0);
    if (--D_801B2C3C == 0)
    {
        D_801B2C38 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8009946C(void)
{
    D_800D944C.target_shade = 0;
    D_800D944C.shade_step = 2;
    D_801B2C3C = 0x40;
    D_801B2C38 += 1;
    func_800994B8();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800994B8(void)
{
    wmap_step_actor_animation(&D_800D944C, &D_801399E0);
    wmap_draw_actor_sprite(&D_800D944C, D_80182D60.packed, 0x13, 0x2, 0);
    if (--D_801B2C3C == 0)
    {
        D_801B2C38 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80099534(void)
{
    D_801B2C38 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009954C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C40 = 1;
        D_801B2C44 = 1;
        return 1;
    }

    if (D_801B2C40 < 0x6)
    {
        D_800D64F0[D_801B2C40]();
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
void func_800995C4(void)
{
    D_801B2C40 = 1;
    D_801B2C44 = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_800995DC(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB578, D_80139FE8, 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2C44 == 0)
    {
        D_801B2C40 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80099668(void)
{
    D_801B2C44 = 0x20;
    D_80139280[35] = -1;
    D_801B2C40 += 1;
    func_800996B0();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_800996B0(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB578, D_80139FE8, 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2C44 == 0)
    {
        D_801B2C40 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009973C(void)
{
    D_801B2C40 += 1;
}
