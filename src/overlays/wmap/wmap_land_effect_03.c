#include "wmap_main.h"
#include "wmap_land_effect_03.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

void func_800881F0(void);
void func_800883E0(void);
void func_800885D8(void);
void func_800887D0(void);
void func_80087670(void);
void func_800876AC(void);
s32 func_80087744(s32 arg0);
void func_800876F0(void);
void func_8008772C(void);
s32 func_80088740(s32 arg0);
s32 func_80088548(s32 arg0);
s32 func_80087B90(s32 arg0);
s32 func_80088350(s32 arg0);
s32 func_80088160(s32 arg0);
s32 func_80087ED0(s32 arg0);
s32 func_80088008(s32 arg0);
s32 func_80087D30(s32 arg0);
void func_80087C9C(void);
void func_80087E3C(void);
void func_800882B8(void);
void func_800884AC(void);
void func_800886A4(void);
void func_8008889C(void);

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

extern s32 D_8011CF1C;
extern s32 D_80182DF0;
extern s32 D_801B292C;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2934;
extern s32 D_801B0FD0;
extern s32 D_801B293C;
extern s32 D_801B2944;
extern s32 D_801B294C;
extern s32 D_801B2954;
extern s32 D_801B290C;
extern void (*D_800D5A60[])(void);
extern s32 D_8013B20C;
extern void func_800876AC(void);
extern void func_8008772C(void);
extern s32 D_801B2914;
extern void (*D_800D5A78[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B291C;
extern void (*D_800D5AB8[])(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern void func_80087C9C(void);
extern s32 D_801B2924;
extern void (*D_800D5AC8[])(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern void func_80087E3C(void);
extern void (*D_800D5AD8[])(void);
extern void (*D_800D5AF0[])(void);
extern void (*D_800D5B00[])(void);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern void (*D_800D5B18[])(void);
extern u8 D_800D9F20[];
extern u8 D_80139BD8[];
extern void func_800884AC(void);
extern void (*D_800D5B30[])(void);
extern u8 D_800DA7B8[];
extern u8 D_80139D68[];
extern void func_800886A4(void);
extern void (*D_800D5B48[])(void);
extern u8 D_800DB050[];
extern u8 D_80139EF8[];
extern void func_8008889C(void);

extern u32 D_801B2928;
extern u32 D_801B2930;
extern u32 D_801B2938;
extern u8 D_80121538[];
extern u32 D_801B2940;
extern u32 D_801B2948;
extern u32 D_801B2950;
extern u32 D_801B2908;
extern u32 D_801B2910;
extern u32 D_801B2918;
extern u32 D_801B2920;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;

extern SVECTOR D_80139258;
extern SVECTOR D_801B24A0;
extern SVECTOR D_801B2490;

extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* D_80139280;

extern WmapSlot14 D_801AFBD0[];

/** @brief Draw the rotating effect, increase its scale, and update the sequence timer. */
void func_80086F48(void)
{
    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(D_8011CF1C, 0, 4, 0x35, 0x7800, 1, D_80182DF0);
    D_801B2490.vz += 100;
    PopMatrix();
    D_80182DF0 += 2;
    if (D_80182DF0 >= 0x82)
    {
        D_80182DF0 = 0x81;
    }
    if (--D_801B292C == 0)
    {
        D_801B2928++;
    }
}

/** @brief Draw the rotating effect, reduce its scale, and update the sequence timer. */
void func_8008701C(void)
{
    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(D_8011CF1C, 0, 4, 0x35, 0x7800, 1, D_80182DF0);
    D_801B2490.vz += 100;
    PopMatrix();
    D_80182DF0 -= 8;
    if (D_80182DF0 < 0)
    {
        D_80182DF0 = 0;
    }
    if (--D_801B292C == 0)
    {
        D_801B2928++;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_800870E8(void)
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
    if (--D_801B2934 == 0)
    {
        D_801B2930 += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800871E8(void)
{
    s32 i;

    D_801B0FD0 = 50;
    D_80139280[0x1] = 4;
    D_80139280[0x2] = 1;
    D_80139280[0x3] = 20;
    D_80139280[0x4] = 30;
    D_80139280[0x5] = 16;
    D_80139280[0x6] = 1500;
    D_80139280[0x7] = 20;
    D_80139280[0x8] = 19;
    D_80139280[0x9] = 0;
    D_80139280[0xA] = 10000;
    for (i = 0; i < 50; i++)
    {
        D_801AFBD0[i + D_80139280[0x7]].field_00 = 0;
        D_80139988[i + 24].data = D_80121538;
    }
    D_801B293C = 144;
    D_801B2938++;
    func_800881F0();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800872D4(void)
{
    s32 i;

    D_801B0FD0 = 50;
    D_80139280[0xB] = 4;
    D_80139280[0xC] = 1;
    D_80139280[0xD] = 20;
    D_80139280[0xE] = 30;
    D_80139280[0xF] = 4;
    D_80139280[0x10] = 1500;
    D_80139280[0x11] = 70;
    D_80139280[0x12] = 19;
    D_80139280[0x13] = 1;
    D_80139280[0x14] = 10000;
    for (i = 0; i < 50; i++)
    {
        D_801AFBD0[i + D_80139280[0x11]].field_00 = 0;
        D_80139988[i + 74].data = D_80121538;
    }
    D_801B2944 = 144;
    D_801B2940++;
    func_800883E0();
}

/** @brief World-map step: fill spawn descriptor slot 1, clear its slot run, then advance. */
void func_800873C0(void)
{
    s32 i;

    D_801B0FD0 = 0x32;
    D_80139280[21] = 2;
    D_80139280[22] = 1;
    D_80139280[23] = 0x14;
    D_80139280[24] = 0x1E;
    D_80139280[25] = 2;
    D_80139280[26] = 0x5DC;
    D_80139280[27] = 0x78;
    D_80139280[28] = 0x13;
    D_80139280[29] = 2;
    D_80139280[30] = 0x2710;
    for (i = 0; i < 0x32; i++)
    {
        D_801AFBD0[i + D_80139280[27]].field_00 = 0;
        D_80139988[i + 0x7C].data = &D_80121538;
    }
    D_801B294C = 0x90;
    D_801B2948 += 1;
    func_800885D8();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800874AC(void)
{
    s32 i;

    D_801B0FD0 = 50;
    D_80139280[0x1F] = 1;
    D_80139280[0x20] = 1;
    D_80139280[0x21] = 20;
    D_80139280[0x22] = 30;
    D_80139280[0x23] = 2;
    D_80139280[0x24] = 1500;
    D_80139280[0x25] = 170;
    D_80139280[0x26] = 19;
    D_80139280[0x27] = 3;
    D_80139280[0x28] = 10000;
    for (i = 0; i < 50; i++)
    {
        D_801AFBD0[i + D_80139280[0x25]].field_00 = 0;
        D_80139988[i + 174].data = D_80121538;
    }
    D_801B2954 = 144;
    D_801B2950++;
    func_800887D0();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008759C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2908 = 1;
        D_801B290C = 1;
        return 1;
    }

    if (D_801B2908 < 0x6)
    {
        D_800D5A60[D_801B2908]();
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
void func_80087614(void)
{
    D_801B2908 = 1;
    D_801B290C = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8008762C(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    D_8013B20C = 1;
    D_801B2908 += 1;
    func_80087670();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80087670(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2908 += 1;
        func_800876AC();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_800876AC(void)
{
    wmap_start_sequence(func_80087744);
    D_8013B20C = 1;
    D_801B2908 += 1;
    func_800876F0();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800876F0(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2908 += 1;
        func_8008772C();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008772C(void)
{
    D_801B2908 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80087744(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2910 = 1;
        D_801B2914 = 1;
        return 1;
    }

    if (D_801B2910 < 0x10)
    {
        D_800D5A78[D_801B2910]();
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
void func_800877BC(void)
{
    D_801B2910 = 1;
    D_801B2914 = 1;
}

/** @brief World-map step handler: kick two jobs and advance the step. */
void func_800877D4(void)
{
    D_8013B208 = 1;
    wmap_start_map_tint(0x802028);
    g_wmap_backdrop_target_level = 4;
    wmap_play_sound(0x21, 0x80);
    D_801B2914 = 0x1E;
    D_801B2910 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80087834(void)
{
    if (--D_801B2914 == 0)
    {
        D_801B2910 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_80087868(void)
{
    wmap_start_sequence(func_80088740);
    wmap_start_sequence(func_80088548);
    wmap_start_sequence(func_80087B90);
    D_801B2914 = 0x18;
    D_801B2910 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800878BC(void)
{
    if (--D_801B2914 == 0)
    {
        D_801B2910 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800878F0(void)
{
    wmap_start_sequence(func_80088350);
    D_801B2914 = 0x10;
    D_801B2910 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008792C(void)
{
    if (--D_801B2914 == 0)
    {
        D_801B2910 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80087960(void)
{
    wmap_start_sequence(func_80088160);
    D_801B2914 = 0x8;
    D_801B2910 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008799C(void)
{
    if (--D_801B2914 == 0)
    {
        D_801B2910 += 1;
    }
}

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void func_800879D0(void)
{
    D_801ADAE0 = 1;
    wmap_start_sequence(func_80087ED0);
    D_801B2914 = 0x3C;
    D_801B2910 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80087A18(void)
{
    if (--D_801B2914 == 0)
    {
        D_801B2910 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80087A4C(void)
{
    wmap_start_sequence(func_80088008);
    D_801B2914 = 0x8;
    D_801B2910 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80087A88(void)
{
    if (--D_801B2914 == 0)
    {
        D_801B2910 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80087ABC(void)
{
    wmap_start_sequence(func_80087D30);
    D_801B2914 = 0x8A;
    D_801B2910 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80087AF8(void)
{
    if (--D_801B2914 == 0)
    {
        D_801B2910 += 1;
    }
}

void func_80087B2C(void)
{
    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B2910 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80087B90(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2918 = 1;
        D_801B291C = 1;
        return 1;
    }

    if (D_801B2918 < 0x4)
    {
        D_800D5AB8[D_801B2918]();
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
void func_80087C08(void)
{
    D_801B2918 = 1;
    D_801B291C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80087C20(void)
{
    D_801399AC = D_8011D538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 2;
    D_800D9318.unknown_02 = 0;
    D_800D9318.sequence = 0;
    D_800D9318.target_shade = 0x80;
    D_800D9318.shade = 0;
    D_801B291C = 0x7C;
    D_801B2918 += 1;
    func_80087C9C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80087C9C(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0x13, 0xA, 0);
    if (--D_801B291C == 0)
    {
        D_801B2918 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80087D18(void)
{
    D_801B2918 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80087D30(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2920 = 1;
        D_801B2924 = 1;
        return 1;
    }

    if (D_801B2920 < 0x4)
    {
        D_800D5AC8[D_801B2920]();
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
void func_80087DA8(void)
{
    D_801B2920 = 1;
    D_801B2924 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80087DC0(void)
{
    D_801399B4 = D_8011F538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 8;
    D_800D9344.unknown_02 = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0;
    D_801B2924 = 0x8C;
    D_801B2920 += 1;
    func_80087E3C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80087E3C(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x1A, 0xA, 0);
    if (--D_801B2924 == 0)
    {
        D_801B2920 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80087EB8(void)
{
    D_801B2920 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80087ED0(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2928 = 1;
        D_801B292C = 1;
        return 1;
    }

    if (D_801B2928 < 0x6)
    {
        D_800D5AD8[D_801B2928]();
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
void func_80087F48(void)
{
    D_801B2928 = 1;
    D_801B292C = 1;
}

/** @brief Clear the rotation vector, set the flag, and start a 128-tick sequence step. */
void func_80087F60(void)
{
    D_80182DF0 = 1;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    D_801B292C = 0x80;
    D_801B2928 += 1;
    func_80086F48();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80087FB8(void)
{
    D_801B292C = 0x10;
    D_801B2928 += 1;
    func_8008701C();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80087FF0(void)
{
    D_801B2928 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80088008(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2930 = 1;
        D_801B2934 = 1;
        return 1;
    }

    if (D_801B2930 < 0x4)
    {
        D_800D5AF0[D_801B2930]();
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
void func_80088080(void)
{
    D_801B2930 = 1;
    D_801B2934 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80088098(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B2934 = 0x40;
    D_801B2930 += 1;
    func_800870E8();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80088148(void)
{
    D_801B2930 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80088160(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2938 = 1;
        D_801B293C = 1;
        return 1;
    }

    if (D_801B2938 < 0x6)
    {
        D_800D5B00[D_801B2938]();
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
void func_800881D8(void)
{
    D_801B2938 = 1;
    D_801B293C = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800881F0(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x32, 0, 0x7F, 0x2, 0, (s32)D_80139280);
    if (--D_801B293C == 0)
    {
        D_801B2938 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80088270(void)
{
    D_801B293C = 0x20;
    D_80139280[5] = -1;
    D_801B2938 += 1;
    func_800882B8();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800882B8(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0x32, 0, 0x7F, 0x2, 0, (s32)D_80139280);
    if (--D_801B293C == 0)
    {
        D_801B2938 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80088338(void)
{
    D_801B2938 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80088350(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2940 = 1;
        D_801B2944 = 1;
        return 1;
    }

    if (D_801B2940 < 0x6)
    {
        D_800D5B18[D_801B2940]();
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
void func_800883C8(void)
{
    D_801B2940 = 1;
    D_801B2944 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800883E0(void)
{
    func_8006A2FC(D_800D9F20, D_80139BD8, 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2944 == 0)
    {
        D_801B2940 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80088464(void)
{
    D_801B2944 = 0x20;
    D_80139280[15] = -1;
    D_801B2940 += 1;
    func_800884AC();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800884AC(void)
{
    func_8006A2FC(D_800D9F20, D_80139BD8, 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2944 == 0)
    {
        D_801B2940 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80088530(void)
{
    D_801B2940 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80088548(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2948 = 1;
        D_801B294C = 1;
        return 1;
    }

    if (D_801B2948 < 0x6)
    {
        D_800D5B30[D_801B2948]();
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
void func_800885C0(void)
{
    D_801B2948 = 1;
    D_801B294C = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800885D8(void)
{
    func_8006A2FC(D_800DA7B8, D_80139D68, 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B294C == 0)
    {
        D_801B2948 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8008865C(void)
{
    D_801B294C = 0x20;
    D_80139280[25] = -1;
    D_801B2948 += 1;
    func_800886A4();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800886A4(void)
{
    func_8006A2FC(D_800DA7B8, D_80139D68, 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B294C == 0)
    {
        D_801B2948 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80088728(void)
{
    D_801B2948 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80088740(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2950 = 1;
        D_801B2954 = 1;
        return 1;
    }

    if (D_801B2950 < 0x6)
    {
        D_800D5B48[D_801B2950]();
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
void func_800887B8(void)
{
    D_801B2950 = 1;
    D_801B2954 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800887D0(void)
{
    func_8006A2FC(D_800DB050, D_80139EF8, 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2954 == 0)
    {
        D_801B2950 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80088854(void)
{
    D_801B2954 = 0x20;
    D_80139280[35] = -1;
    D_801B2950 += 1;
    func_8008889C();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_8008889C(void)
{
    func_8006A2FC(D_800DB050, D_80139EF8, 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2954 == 0)
    {
        D_801B2950 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80088920(void)
{
    D_801B2950 += 1;
}
