#include "wmap_main.h"
#include "wmap_land_effect_11.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

void func_800863E4(void);
void func_80086778(void);
void func_80085B44(void);
void func_80085B80(void);
s32 func_80085C18(s32 arg0);
void func_80085BC4(void);
void func_80085C00(void);
s32 func_80086354(s32 reset);
s32 func_80086544(s32 arg0);
s32 func_800868E0(s32 arg0);
s32 func_8008600C(s32 arg0);
s32 func_80086B90(s32 arg0);
s32 func_80086A38(s32 arg0);
s32 func_800866E8(s32 arg0);
s32 func_800861B0(s32 arg0);
s32 func_80086CDC(s32 arg0);
void func_8008611C(void);
void func_800862C0(void);
void func_800864AC(void);
void func_80086654(void);
void func_80086844(void);
void func_80086DEC(void);
void func_80086EB4(void);

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

extern s32 D_801B0FD0;
extern s32 D_801B28D4;
extern s32 D_801B28E4;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B28F4;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B28EC;
extern void *D_8011CF24;
extern void *D_8011CF28;
extern s32 D_80182DF0;
extern s32 D_801B28FC;
extern s32 D_801B28B4;
extern void (*D_800D5960[])(void);
extern s32 D_8013B20C;
extern void func_80085B80(void);
extern void func_80085C00(void);
extern s32 D_801B28BC;
extern void (*D_800D5978[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B28C4;
extern void (*D_800D59B0[])(void);
extern u8* D_801399AC;
extern void func_8008611C(void);
extern s32 D_801B28CC;
extern void (*D_800D59C0[])(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern void func_800862C0(void);
extern void (*D_800D59D0[])(void);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32 D_801B28DC;
extern void (*D_800D59E8[])(void);
extern void *D_801399BC;
extern void (*D_800D59F8[])(void);
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern void func_80086844(void);
extern void (*D_800D5A10[])(void);
extern void (*D_800D5A20[])(void);
extern void (*D_800D5A30[])(void);
extern s32 D_801B2904;
extern void (*D_800D5A48[])(void);
extern u8* D_801399CC;
extern void func_80086DEC(void);
extern void func_80086EB4(void);

extern u8 D_8011D538[];
extern s32 D_801B28D0;
extern u32 D_801B28E0;
extern u32 D_801B28F0;
extern u32 D_801B28E8;

extern u32 D_801B28F8;
extern u32 D_801B28B0;
extern u32 D_801B28B8;
extern u32 D_801B28C0;
extern u32 D_801B28C8;
extern u32 D_801B28D8;
extern u32 D_801B2900;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;

extern SVECTOR D_80139258;
extern SVECTOR D_801B2498;
extern SVECTOR D_801B24A0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B24A8;

extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370;
extern WmapSpriteActor D_800D93C8;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8;
extern WmapAnimationSlot D_801399C8;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* D_80139280;

extern WmapSlot14 D_801AFBD0[];

/** @brief World-map step: fill a spawn descriptor, clear its slot run, then advance. */
void func_8008543C(void)
{
    s32 i;

    D_801B0FD0 = 0x46;
    D_80139280[1] = 2;
    D_80139280[2] = 8;
    D_80139280[3] = 0x40;
    D_80139280[4] = 2;
    D_80139280[5] = 2;
    D_80139280[6] = 0x190;
    D_80139280[7] = 0x14;
    D_80139280[8] = 0xD;
    D_80139280[9] = 1;
    D_80139280[10] = 0x3E8;
    for (i = 0; i < 0x46; i++)
    {
        D_801AFBD0[i + D_80139280[7]].field_00 = 0;
        D_80139988[i + 0x18].data = &D_8011D538;
    }
    D_801B28D4 = 0x8D;
    D_801B28D0 += 1;
    func_800863E4();
}

/** @brief World-map step: fill a spawn descriptor, clear its slot run, then advance. */
void func_80085528(void)
{
    s32 i;

    D_801B0FD0 = 0xA;
    D_80139280[11] = 1;
    D_80139280[12] = 6;
    D_80139280[13] = 0x90;
    D_80139280[14] = 2;
    D_80139280[15] = 3;
    D_80139280[16] = 0x60;
    D_80139280[17] = 0x64;
    D_80139280[18] = 0xD;
    D_80139280[19] = 0;
    D_80139280[20] = 0x2EE0;
    for (i = 0; i < 0xA; i++)
    {
        D_801AFBD0[i + D_80139280[17]].field_00 = 0;
        D_80139988[i + 0x68].data = &D_8011D538;
    }
    D_801B28E4 = 0x29;
    D_801B28E0 += 1;
    func_80086778();
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80085618(void)
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
    if (--D_801B28EC == 0)
    {
        D_801B28E8 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80085718(void)
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
    if (--D_801B28F4 == 0)
    {
        D_801B28F0 += 1;
    }
}

/** @brief Draw and brighten two rotating effect layers and advance their shared countdown. */
void func_80085818(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(D_8011CF24, 0, 4, 54, 0x7900, 1, D_80182DF0);
    D_801B2490.vz = (u16)(D_801B2490.vz + 24);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(D_8011CF28, 0, 4, 54, 0x7900, 1, D_80182DF0);
    D_801B2498.vz = (u16)(D_801B2498.vz - 16);
    PopMatrix();
    intensity = D_80182DF0 + 2;
    D_80182DF0 = intensity;
    if (intensity >= 130)
    {
        D_80182DF0 = 129;
    }
    remaining = D_801B28FC - 1;
    D_801B28FC = remaining;
    if (remaining == 0)
    {
        D_801B28F8++;
    }
}

/** @brief Draw two rotating effect layers and advance their shared countdown. */
void func_80085950(void)
{
    s32 remaining;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(D_8011CF24, 0, 4, 54, 0x7900, 1, D_80182DF0);
    D_801B2490.vz = (u16)(D_801B2490.vz + 24);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(D_8011CF28, 0, 4, 54, 0x7900, 1, D_80182DF0);
    D_801B2498.vz = (u16)(D_801B2498.vz - 16);
    PopMatrix();
    remaining = D_801B28FC - 1;
    D_80182DF0 -= 2;
    D_801B28FC = remaining;
    if (remaining == 0)
    {
        D_801B28F8++;
    }
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80085A70(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B28B0 = 1;
        D_801B28B4 = 1;
        return 1;
    }

    if (D_801B28B0 < 0x6)
    {
        D_800D5960[D_801B28B0]();
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
void func_80085AE8(void)
{
    D_801B28B0 = 1;
    D_801B28B4 = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80085B00(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    D_8013B20C = 1;
    D_801B28B0 += 1;
    func_80085B44();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80085B44(void)
{
    if (D_8013B20C == 0)
    {
        D_801B28B0 += 1;
        func_80085B80();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80085B80(void)
{
    wmap_start_sequence(func_80085C18);
    D_8013B20C = 1;
    D_801B28B0 += 1;
    func_80085BC4();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80085BC4(void)
{
    if (D_8013B20C == 0)
    {
        D_801B28B0 += 1;
        func_80085C00();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80085C00(void)
{
    D_801B28B0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80085C18(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B28B8 = 1;
        D_801B28BC = 1;
        return 1;
    }

    if (D_801B28B8 < 0xE)
    {
        D_800D5978[D_801B28B8]();
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
void func_80085C90(void)
{
    D_801B28B8 = 1;
    D_801B28BC = 1;
}

/**
 * @brief Set up a world-map sub-scene: request assets and register its handler.
 */
void func_80085CA8(void)
{
    D_8013B208 = 1;
    wmap_play_sound(0x20, 0x80);
    wmap_start_sequence(func_80086354);
    D_801B28BC = 8;
    D_801B28B8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80085CFC(void)
{
    if (--D_801B28BC == 0)
    {
        D_801B28B8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80085D30(void)
{
    wmap_start_sequence(func_80086544);
    D_801B28BC = 0x1E;
    D_801B28B8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80085D6C(void)
{
    if (--D_801B28BC == 0)
    {
        D_801B28B8 += 1;
    }
}

/** @brief Register world-map callbacks, seed a mode value, and advance step counters. */
void func_80085DA0(void)
{
    wmap_start_sequence(func_800868E0);
    wmap_start_map_tint(0x801045);
    g_wmap_backdrop_target_level = 4;
    wmap_start_sequence(func_8008600C);
    D_801B28BC = 2;
    D_801B28B8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80085E00(void)
{
    if (--D_801B28BC == 0)
    {
        D_801B28B8 += 1;
    }
}

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void func_80085E34(void)
{
    D_801ADAE0 = 1;
    wmap_start_sequence(func_80086B90);
    D_801B28BC = 0x3C;
    D_801B28B8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80085E7C(void)
{
    if (--D_801B28BC == 0)
    {
        D_801B28B8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_80085EB0(void)
{
    wmap_start_sequence(func_80086A38);
    wmap_start_sequence(func_800866E8);
    D_801B28BC = 0x2;
    D_801B28B8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80085EF8(void)
{
    if (--D_801B28BC == 0)
    {
        D_801B28B8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_80085F2C(void)
{
    wmap_start_sequence(func_800861B0);
    wmap_start_sequence(func_80086CDC);
    D_801B28BC = 0xBF;
    D_801B28B8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80085F74(void)
{
    if (--D_801B28BC == 0)
    {
        D_801B28B8 += 1;
    }
}

/** @brief Clear the sequence gate, update the active map cell, and advance the step. */
void func_80085FA8(void)
{
    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B28B8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008600C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B28C0 = 1;
        D_801B28C4 = 1;
        return 1;
    }

    if (D_801B28C0 < 0x4)
    {
        D_800D59B0[D_801B28C0]();
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
void func_80086084(void)
{
    D_801B28C0 = 1;
    D_801B28C4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8008609C(void)
{
    D_801399AC = D_8011D538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.sequence = 3;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 8;
    D_800D9318.unknown_02 = 0;
    D_800D9318.target_shade = 0x80;
    D_800D9318.shade = 0;
    D_801B28C4 = 0x40;
    D_801B28C0 += 1;
    func_8008611C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008611C(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0xD, 0x1E, 0);
    if (--D_801B28C4 == 0)
    {
        D_801B28C0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80086198(void)
{
    D_801B28C0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800861B0(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B28C8 = 1;
        D_801B28CC = 1;
        return 1;
    }

    if (D_801B28C8 < 0x4)
    {
        D_800D59C0[D_801B28C8]();
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
void func_80086228(void)
{
    D_801B28C8 = 1;
    D_801B28CC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80086240(void)
{
    D_801399B4 = D_8011F538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 2;
    D_800D9344.target_shade = 0x81;
    D_800D9344.unknown_02 = 0;
    D_800D9344.sequence = 0;
    D_800D9344.shade = 1;
    D_801B28CC = 0xC0;
    D_801B28C8 += 1;
    func_800862C0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800862C0(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x18, 0xA, 0);
    if (--D_801B28CC == 0)
    {
        D_801B28C8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008633C(void)
{
    D_801B28C8 += 1;
}

/** @brief Reset or dispatch the current effect phase.
 * @param reset Nonzero to restart the effect.
 * @return One while active, otherwise zero.
 */
s32 func_80086354(s32 reset)
{
    s32 active;
    if (reset != 0)
    {
        D_801B28D0 = 1;
        D_801B28D4 = 1;
        return 1;
    }
    if (D_801B28D0 < 6U)
    {
        D_800D59D0[D_801B28D0]();
        active = 1;
    }
    else
    {
        active = 0;
    }
    return active;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800863CC(void)
{
    D_801B28D0 = 1;
    D_801B28D4 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800863E4(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x46, 0, 0x7F, 0x2, 0, (s32)D_80139280);
    if (--D_801B28D4 == 0)
    {
        D_801B28D0 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80086464(void)
{
    D_801B28D4 = 0x40;
    D_80139280[5] = -1;
    D_801B28D0 += 1;
    func_800864AC();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800864AC(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x46, 0, 0x7F, 0x2, 0, (s32)D_80139280);
    if (--D_801B28D4 == 0)
    {
        D_801B28D0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008652C(void)
{
    D_801B28D0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80086544(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B28D8 = 1;
        D_801B28DC = 1;
        return 1;
    }

    if (D_801B28D8 < 0x4)
    {
        D_800D59E8[D_801B28D8]();
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
void func_800865BC(void)
{
    D_801B28D8 = 1;
    D_801B28DC = 1;
}

/** @brief Configure the world-map actor and advance to its draw step. */
void func_800865D4(void)
{
    D_801399BC = &D_8011D538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.sequence = 2;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 2;
    D_800D9370.target_shade = 0x81;
    D_800D9370.unknown_02 = 0;
    D_800D9370.shade = 1;
    D_801B28DC = 0x62;
    D_801B28D8 += 1;
    func_80086654();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80086654(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0xD, 0x3, 0);
    if (--D_801B28DC == 0)
    {
        D_801B28D8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800866D0(void)
{
    D_801B28D8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800866E8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B28E0 = 1;
        D_801B28E4 = 1;
        return 1;
    }

    if (D_801B28E0 < 0x6)
    {
        D_800D59F8[D_801B28E0]();
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
void func_80086760(void)
{
    D_801B28E0 = 1;
    D_801B28E4 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80086778(void)
{
    func_8006A2FC(D_800DA448, D_80139CC8, 0xA, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B28E4 == 0)
    {
        D_801B28E0 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_800867FC(void)
{
    D_801B28E4 = 0x20;
    D_80139280[15] = -1;
    D_801B28E0 += 1;
    func_80086844();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80086844(void)
{
    func_8006A2FC(D_800DA448, D_80139CC8, 0xA, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B28E4 == 0)
    {
        D_801B28E0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800868C8(void)
{
    D_801B28E0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800868E0(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B28E8 = 1;
        D_801B28EC = 1;
        return 1;
    }

    if (D_801B28E8 < 0x4)
    {
        D_800D5A10[D_801B28E8]();
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
void func_80086958(void)
{
    D_801B28E8 = 1;
    D_801B28EC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80086970(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B28EC = 0x28;
    D_801B28E8 += 1;
    func_80085618();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80086A20(void)
{
    D_801B28E8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80086A38(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B28F0 = 1;
        D_801B28F4 = 1;
        return 1;
    }

    if (D_801B28F0 < 0x4)
    {
        D_800D5A20[D_801B28F0]();
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
void func_80086AB0(void)
{
    D_801B28F0 = 1;
    D_801B28F4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80086AC8(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    D_801B28F4 = 0x40;
    D_801B28F0 += 1;
    func_80085718();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80086B78(void)
{
    D_801B28F0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80086B90(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B28F8 = 1;
        D_801B28FC = 1;
        return 1;
    }

    if (D_801B28F8 < 0x6)
    {
        D_800D5A30[D_801B28F8]();
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
void func_80086C08(void)
{
    D_801B28F8 = 1;
    D_801B28FC = 1;
}

/** @brief Clear two rotation vectors, set the flag, and start a 128-tick sequence step. */
void func_80086C20(void)
{
    D_80182DF0 = 1;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    D_801B2498.vx = 0;
    D_801B2498.vy = 0;
    D_801B2498.vz = 0;
    D_801B28FC = 0x80;
    D_801B28F8 += 1;
    func_80085818();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80086C8C(void)
{
    D_801B28FC = 0x40;
    D_801B28F8 += 1;
    func_80085950();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80086CC4(void)
{
    D_801B28F8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80086CDC(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2900 = 1;
        D_801B2904 = 1;
        return 1;
    }

    if (D_801B2900 < 0x6)
    {
        D_800D5A48[D_801B2900]();
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
void func_80086D54(void)
{
    D_801B2900 = 1;
    D_801B2904 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80086D6C(void)
{
    D_801399CC = D_8011F538;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.sequence = 1;
    D_800D93C8.previous_sequence = -1;
    D_800D93C8.shade_step = 2;
    D_800D93C8.unknown_02 = 0;
    D_800D93C8.target_shade = 0x81;
    D_800D93C8.shade = 0x81;
    D_801B2904 = 0xBF;
    D_801B2900 += 1;
    func_80086DEC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80086DEC(void)
{
    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, g_wmap_focus_screen_position.packed, 0x18, 0x9, 0);
    if (--D_801B2904 == 0)
    {
        D_801B2900 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80086E68(void)
{
    D_800D93C8.shade_step = 16;
    D_800D93C8.target_shade = 0;
    D_801B2904 = 0x8;
    D_801B2900 += 1;
    func_80086EB4();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80086EB4(void)
{
    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, g_wmap_focus_screen_position.packed, 0x18, 0x9, 0);
    if (--D_801B2904 == 0)
    {
        D_801B2900 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80086F30(void)
{
    D_801B2900 += 1;
}
