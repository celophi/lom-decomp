#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_land_effect_27.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

/** @brief World-map cell record. */
typedef struct
{
    u32 value;
    u8 pad_04[36];
} WmapEffectCell;

void func_8009C880(void);
void func_8009CCF4(void);
void func_8009CEF4(void);
void func_8009D0FC(void);
void func_8009D304(void);
void func_8009B9B8(void);
void func_8009B9F4(void);
s32 func_8009BA8C(s32 arg0);
void func_8009BA38(void);
void func_8009BA74(void);
s32 func_8009C7F0(s32 arg0);
s32 func_8009CE64(s32 arg0);
s32 func_8009C3E4(s32 arg0);
s32 func_8009CC64(s32 arg0);
s32 func_8009C694(s32 arg0);
s32 func_8009C09C(s32 arg0);
s32 func_8009C9F8(s32 arg0);
s32 func_8009D47C(s32 arg0);
s32 func_8009C53C(s32 arg0);
s32 func_8009C240(s32 arg0);
s32 func_8009D274(s32 arg0);
s32 func_8009D06C(s32 arg0);
void func_8009C1AC(void);
void func_8009C350(void);
void func_8009C954(void);
void func_8009CB08(void);
void func_8009CBD0(void);
void func_8009CDC4(void);
void func_8009CFC8(void);
void func_8009D1D0(void);
void func_8009D3D8(void);
void func_8009D58C(void);

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern u8* D_8011CF1C;
extern s32 D_801B2C8C;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2C84;
extern s32 D_800D665C[];
extern void *D_8011CF24;
extern void *D_8011CF28;
extern void *D_8011CF2C;
extern s32 D_80139234;
extern s32 D_80182DF0;
extern s32 D_801B2C94;
extern s32 D_801B0FD0;
extern s32 D_801B2C9C;
extern u8 D_80121538[];
extern s32 D_801B2CAC;
extern s32 D_801B2CB4;
extern s32 D_801B2CBC;
extern s32 D_801B2CC4;
extern s32 D_801B2C64;
extern void (*D_800D65A4[])(void);
extern s32 D_8013B20C;
extern void func_8009B9F4(void);
extern void func_8009BA74(void);
extern s32 D_801B2C6C;
extern void (*D_800D65BC[])(void);
extern s32 D_8013B208;
extern s32 D_80139244;
extern s32 D_801ADAE0;
extern WmapEffectCell D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2C74;
extern void (*D_800D661C[])(void);
extern void *D_801399AC;
extern s32 D_801B2C7C;
extern void (*D_800D662C[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern void func_8009C350(void);
extern void (*D_800D663C[])(void);
extern void (*D_800D664C[])(void);
extern void (*D_800D6674[])(void);
extern void (*D_800D668C[])(void);
extern u8 D_800DAB28[];
extern u8 D_80139E08[];
extern void func_8009C954(void);
extern s32 D_801B2CA4;
extern void (*D_800D66A4[])(void);
extern u8* D_801399BC;
extern void func_8009CB08(void);
extern void func_8009CBD0(void);
extern void (*D_800D66BC[])(void);
extern u8 D_800D95D8[];
extern WmapAnimationSlot D_80139A28[];
extern void (*D_800D66D4[])(void);
extern u8 D_800D9CB8[];
extern u8 D_80139B68[];
extern void func_8009CFC8(void);
extern void (*D_800D66EC[])(void);
extern u8 D_800DA398[];
extern u8 D_80139CA8[];
extern void func_8009D1D0(void);
extern void (*D_800D6704[])(void);
extern u8 D_800DB158[];
extern u8 D_80139F28[];
extern void func_8009D3D8(void);
extern s32 D_801B2CCC;
extern void (*D_800D671C[])(void);
extern u8* D_801399CC;
extern void func_8009D58C(void);
extern u32 D_801B2C88;
extern u32 D_801B2C80;

extern u32 D_801B2C90;
extern u32 D_801B2C98;
extern u32 D_801B2CA8;
extern u32 D_801B2CB0;
extern u32 D_801B2CB8;
extern u8 D_8011F538[];
extern u32 D_801B2CC0;
extern u32 D_801B2C60;
extern u32 D_801B2C68;
extern u32 D_801B2C70;
extern u32 D_801B2C78;
extern u32 D_801B2CA0;
extern u32 D_801B2CC8;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;

extern SVECTOR D_80139258;
extern SVECTOR D_801B24A0;
extern SVECTOR D_8013B238;
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

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8009AEBC(void)
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
        D_80182DE8 -= 0x4;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--D_801B2C84 == 0)
    {
        D_801B2C80 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8009AFBC(void)
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
        D_80182DEC -= 0x4;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    if (--D_801B2C8C == 0)
    {
        D_801B2C88 += 1;
    }
}

/** @brief Draw three rotating effect layers and update their fade and animation. */
void func_8009B0BC(void)
{
    s32 intensity;
    s32 frame;
    s32 remaining;
    u16 angle;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF24, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -10, -1);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B24A8);
    wmap_draw_model(D_8011CF28, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -40, -1);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF2C, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -70, -1);
    PopMatrix();
    intensity = D_80182DF0 + 2;
    D_80182DF0 = intensity;
    if (intensity >= 130)
    {
        D_80182DF0 = 129;
    }
    angle = D_8013B238.vz;
    D_8013B238.vz = angle + 330;
    D_801B24A8.vz += 10;
    D_8013B238.vz = angle + 340;
    frame = D_80139234 + 1;
    D_80139234 = frame;
    if (frame >= 6)
    {
        D_80139234 = 0;
    }
    remaining = D_801B2C94 - 1;
    D_801B2C94 = remaining;
    if (remaining == 0)
    {
        D_801B2C90++;
    }
}

/** @brief Draw three rotating effect layers and update their fade and animation. */
void func_8009B2CC(void)
{
    s32 intensity;
    s32 frame;
    s32 remaining;
    u16 angle;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF24, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -10, -1);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B24A8);
    wmap_draw_model(D_8011CF28, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -40, -1);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF2C, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -70, -1);
    PopMatrix();
    intensity = D_80182DF0 - 4;
    D_80182DF0 = intensity;
    if (intensity < 0)
    {
        D_80182DF0 = 0;
    }
    angle = D_8013B238.vz;
    D_8013B238.vz = angle + 330;
    D_801B24A8.vz += 10;
    D_8013B238.vz = angle + 340;
    frame = D_80139234 + 1;
    D_80139234 = frame;
    if (frame >= 6)
    {
        D_80139234 = 0;
    }
    remaining = D_801B2C94 - 1;
    D_801B2C94 = remaining;
    if (remaining == 0)
    {
        D_801B2C90++;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_8009B4D4(void)
{
    s32 i;

    D_801B0FD0 = 40;
    D_80139280[0x1F] = 1;
    D_80139280[0x20] = 4;
    D_80139280[0x21] = 32;
    D_80139280[0x22] = 0;
    D_80139280[0x23] = 2;
    D_80139280[0x24] = 0;
    D_80139280[0x25] = 140;
    D_80139280[0x26] = 8;
    D_80139280[0x27] = 3;
    D_80139280[0x28] = 8000;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + 140].field_00 = 0;
        D_80139988[i + 144].data = D_80121538;
    }
    D_801B2C9C = 80;
    D_801B2C98++;
    func_8009C880();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_8009B5B0(void)
{
    s32 i;

    D_80139280[0x1] = 1;
    D_80139280[0x2] = 4;
    D_80139280[0x3] = 0x20;
    D_80139280[0x4] = 0;
    D_80139280[0x5] = 2;
    D_80139280[0x6] = 0;
    D_80139280[0x7] = 0x14;
    D_80139280[0x8] = 8;
    D_80139280[0x9] = 0;
    D_80139280[0xA] = 0x2EE0;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + 20].field_00 = 0;
        D_80139988[i + 20].data = D_80121538;
    }
    D_801B2CAC = 80;
    D_801B2CA8++;
    func_8009CCF4();
}

/** @brief World-map step: init hero struct fields, clear tables, advance step. */
void func_8009B67C(void)
{
    s32 i;
    void *base = D_80139280;

    *(s32 *)((u8 *)base + 0x2C) = 0;
    *(s32 *)((u8 *)base + 0x30) = 0;
    *(s32 *)((u8 *)base + 0x38) = 0;
    *(s32 *)((u8 *)base + 0x34) = 0x80;
    *(s32 *)((u8 *)base + 0x3C) = 2;
    *(s32 *)((u8 *)base + 0x40) = 0x64;
    *(s32 *)((u8 *)base + 0x44) = 0x3C;
    *(s32 *)((u8 *)base + 0x48) = 8;
    *(s32 *)((u8 *)base + 0x4C) = 1;
    *(s32 *)((u8 *)base + 0x50) = 0x5DC0;

    for (i = 0; i < 0x18; i++)
    {
        D_801AFBD0[60 + i].field_00 = 0;
        D_80139988[60 + i].data = D_80121538;
    }

    D_801B2CB4 = 0x38;
    D_801B2CB0 += 1;
    func_8009CEF4();
}

/** @brief World-map step handler: configure a model descriptor and clear two entry tables, then advance. */
void func_8009B748(void)
{
    s32 i;

    D_80139280[0x15] = 1;
    D_80139280[0x17] = 0x20;
    D_80139280[0x1A] = 0x3E8;
    D_80139280[0x1B] = 0x64;
    D_80139280[0x1C] = 8;
    D_80139280[0x1D] = 2;
    D_80139280[0x16] = 1;
    D_80139280[0x18] = 0;
    D_80139280[0x19] = 1;
    D_80139280[0x1E] = 0x6D60;

    for (i = 0; i < 20; i++)
    {
        D_801AFBD0[100 + i].field_00 = 0;
        D_80139988[100 + i].data = D_80121538;
    }

    D_801B2CBC = 20;
    D_801B2CB8 += 1;
    func_8009D0FC();
}

/** @brief World-map step: init hero struct fields, clear tables, advance step. */
void func_8009B814(void)
{
    s32 i;

    D_80139280[0x29] = 1;
    D_80139280[0x2A] = 4;
    D_80139280[0x2B] = 0x20;
    D_80139280[0x2E] = 0x3E8;
    D_80139280[0x2F] = 0xB4;
    D_80139280[0x30] = 0x15;
    D_80139280[0x31] = 2;
    D_80139280[0x2C] = 0;
    D_80139280[0x2D] = 1;
    D_80139280[0x32] = 0x1F40;

    for (i = 0; i < 20; i++)
    {
        D_801AFBD0[180 + i].field_00 = 0;
        D_80139988[180 + i].data = D_8011F538;
    }

    D_801B2CC4 = 20;
    D_801B2CC0 += 1;
    func_8009D304();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009B8E4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C60 = 1;
        D_801B2C64 = 1;
        return 1;
    }

    if (D_801B2C60 < 0x6)
    {
        D_800D65A4[D_801B2C60]();
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
void func_8009B95C(void)
{
    D_801B2C60 = 1;
    D_801B2C64 = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8009B974(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    D_8013B20C = 1;
    D_801B2C60 += 1;
    func_8009B9B8();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009B9B8(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2C60 += 1;
        func_8009B9F4();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8009B9F4(void)
{
    wmap_start_sequence(func_8009BA8C);
    D_8013B20C = 1;
    D_801B2C60 += 1;
    func_8009BA38();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009BA38(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2C60 += 1;
        func_8009BA74();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009BA74(void)
{
    D_801B2C60 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009BA8C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C68 = 1;
        D_801B2C6C = 1;
        return 1;
    }

    if (D_801B2C68 < 0x18)
    {
        D_800D65BC[D_801B2C68]();
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
void func_8009BB04(void)
{
    D_801B2C68 = 1;
    D_801B2C6C = 1;
}

/** @brief World-map step: mark active, request a resource, seed the sub-counter, tick. */
void func_8009BB1C(void)
{
    D_8013B208 = 1;
    wmap_play_sound(0x2E, 0x80);
    D_801B2C6C = 4;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BB64(void)
{
    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009BB98(void)
{
    wmap_start_sequence(func_8009C7F0);
    D_801B2C6C = 0xF;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BBD4(void)
{
    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009BC08(void)
{
    wmap_start_sequence(func_8009CE64);
    D_801B2C6C = 0xC;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BC44(void)
{
    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

/** @brief World-map step handler: register a callback, kick a job, advance the step. */
void func_8009BC78(void)
{
    wmap_start_sequence(func_8009C3E4);
    wmap_start_map_tint(0x651035);
    g_wmap_backdrop_target_level = 4;
    D_801B2C6C = 2;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BCCC(void)
{
    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void func_8009BD00(void)
{
    D_80139244 = 1;
    D_801B2C6C = 0x14;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BD2C(void)
{
    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8009BD60(void)
{
    wmap_start_sequence(func_8009CC64);
    wmap_start_sequence(func_8009C694);
    D_801B2C6C = 0x28;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BDA8(void)
{
    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

/** @brief Register two callbacks around the sequence flag update and begin a 30-tick delay. */
void func_8009BDDC(void)
{
    wmap_start_sequence(&func_8009C9F8);
    D_801ADAE0 = 1;
    wmap_start_sequence(&func_8009C09C);
    D_801B2C6C = 0x1E;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BE30(void)
{
    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009BE64(void)
{
    wmap_start_sequence(func_8009D47C);
    D_801B2C6C = 0x38;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BEA0(void)
{
    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009BED4(void)
{
    wmap_start_sequence(func_8009C53C);
    D_801B2C6C = 0x2;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BF10(void)
{
    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

/** @brief Register a sequence callback, clear the world-map value, and start a 64-tick delay. */
void func_8009BF44(void)
{
    wmap_start_sequence(&func_8009C240);
    D_80139244 = 0;
    D_801B2C6C = 0x40;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BF88(void)
{
    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8009BFBC(void)
{
    wmap_start_sequence(func_8009D274);
    wmap_start_sequence(func_8009D06C);
    D_801B2C6C = 0xAC;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009C004(void)
{
    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

void func_8009C038(void)
{
    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B2C68 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009C09C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C70 = 1;
        D_801B2C74 = 1;
        return 1;
    }

    if (D_801B2C70 < 0x4)
    {
        D_800D661C[D_801B2C70]();
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
void func_8009C114(void)
{
    D_801B2C70 = 1;
    D_801B2C74 = 1;
}

void func_8009C12C(void)
{
    D_801399AC = &D_8011F538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.sequence = 1;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 8;
    D_800D9318.resource_index = 0;
    D_800D9318.target_shade = 0x81;
    D_800D9318.shade = 1;
    D_801B2C74 = 0x24;
    D_801B2C70 += 1;
    func_8009C1AC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009C1AC(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0x15, 0x2, 0);
    if (--D_801B2C74 == 0)
    {
        D_801B2C70 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009C228(void)
{
    D_801B2C70 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009C240(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C78 = 1;
        D_801B2C7C = 1;
        return 1;
    }

    if (D_801B2C78 < 0x4)
    {
        D_800D662C[D_801B2C78]();
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
void func_8009C2B8(void)
{
    D_801B2C78 = 1;
    D_801B2C7C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8009C2D0(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.sequence = 1;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 2;
    D_800D9344.resource_index = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0;
    D_801B2C7C = 0xEE;
    D_801B2C78 += 1;
    func_8009C350();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009C350(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x12, 0x5, 0);
    if (--D_801B2C7C == 0)
    {
        D_801B2C78 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009C3CC(void)
{
    D_801B2C78 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009C3E4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C80 = 1;
        D_801B2C84 = 1;
        return 1;
    }

    if (D_801B2C80 < 0x4)
    {
        D_800D663C[D_801B2C80]();
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
void func_8009C45C(void)
{
    D_801B2C80 = 1;
    D_801B2C84 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8009C474(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B2C84 = 0x20;
    D_801B2C80 += 1;
    func_8009AEBC();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009C524(void)
{
    D_801B2C80 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009C53C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C88 = 1;
        D_801B2C8C = 1;
        return 1;
    }

    if (D_801B2C88 < 0x4)
    {
        D_800D664C[D_801B2C88]();
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
void func_8009C5B4(void)
{
    D_801B2C88 = 1;
    D_801B2C8C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8009C5CC(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    D_801B2C8C = 0x20;
    D_801B2C88 += 1;
    func_8009AFBC();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009C67C(void)
{
    D_801B2C88 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009C694(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C90 = 1;
        D_801B2C94 = 1;
        return 1;
    }

    if (D_801B2C90 < 0x6)
    {
        D_800D6674[D_801B2C90]();
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
void func_8009C70C(void)
{
    D_801B2C90 = 1;
    D_801B2C94 = 1;
}

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_8009C724(void)
{
    D_80182DF0 = 1;
    D_8013B238 = D_80139258;
    D_80139234 = 0;
    D_801B2C94 = 0x60;
    D_801B2C90 += 1;
    func_8009B0BC();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8009C7A0(void)
{
    D_801B2C94 = 0x20;
    D_801B2C90 += 1;
    func_8009B2CC();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009C7D8(void)
{
    D_801B2C90 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009C7F0(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C98 = 1;
        D_801B2C9C = 1;
        return 1;
    }

    if (D_801B2C98 < 0x6)
    {
        D_800D668C[D_801B2C98]();
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
void func_8009C868(void)
{
    D_801B2C98 = 1;
    D_801B2C9C = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009C880(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DAB28, D_80139E08, 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2C9C == 0)
    {
        D_801B2C98 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009C90C(void)
{
    D_801B2C9C = 0x20;
    D_80139280[35] = -1;
    D_801B2C98 += 1;
    func_8009C954();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009C954(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DAB28, D_80139E08, 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2C9C == 0)
    {
        D_801B2C98 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009C9E0(void)
{
    D_801B2C98 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009C9F8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2CA0 = 1;
        D_801B2CA4 = 1;
        return 1;
    }

    if (D_801B2CA0 < 0x6)
    {
        D_800D66A4[D_801B2CA0]();
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
void func_8009CA70(void)
{
    D_801B2CA0 = 1;
    D_801B2CA4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8009CA88(void)
{
    D_801399BC = D_8011F538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 4;
    D_800D9370.target_shade = 0x81;
    D_800D9370.resource_index = 0;
    D_800D9370.sequence = 0;
    D_800D9370.shade = 1;
    D_801B2CA4 = 0x80;
    D_801B2CA0 += 1;
    func_8009CB08();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009CB08(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x15, 0x5, 0);
    if (--D_801B2CA4 == 0)
    {
        D_801B2CA0 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8009CB84(void)
{
    D_800D9370.shade_step = 4;
    D_800D9370.target_shade = 0;
    D_801B2CA4 = 0x80;
    D_801B2CA0 += 1;
    func_8009CBD0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009CBD0(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x15, 0x5, 0);
    if (--D_801B2CA4 == 0)
    {
        D_801B2CA0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009CC4C(void)
{
    D_801B2CA0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009CC64(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2CA8 = 1;
        D_801B2CAC = 1;
        return 1;
    }

    if (D_801B2CA8 < 0x6)
    {
        D_800D66BC[D_801B2CA8]();
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
void func_8009CCDC(void)
{
    D_801B2CA8 = 1;
    D_801B2CAC = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009CCF4(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0x28, 0xFF, 0x1, 0x8, 0, (s32)D_80139280);
    if (--D_801B2CAC == 0)
    {
        D_801B2CA8 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009CD7C(void)
{
    D_801B2CAC = 0x20;
    D_80139280[5] = -1;
    D_801B2CA8 += 1;
    func_8009CDC4();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009CDC4(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0x28, 0xFF, 0x1, 0x8, 0, (s32)D_80139280);
    if (--D_801B2CAC == 0)
    {
        D_801B2CA8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009CE4C(void)
{
    D_801B2CA8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009CE64(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2CB0 = 1;
        D_801B2CB4 = 1;
        return 1;
    }

    if (D_801B2CB0 < 0x6)
    {
        D_800D66D4[D_801B2CB0]();
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
void func_8009CEDC(void)
{
    D_801B2CB0 = 1;
    D_801B2CB4 = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009CEF4(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D9CB8, D_80139B68, 0x18, 0xFF, 0x1, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2CB4 == 0)
    {
        D_801B2CB0 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009CF80(void)
{
    D_801B2CB4 = 0x80;
    D_80139280[15] = -1;
    D_801B2CB0 += 1;
    func_8009CFC8();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009CFC8(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D9CB8, D_80139B68, 0x18, 0xFF, 0x1, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2CB4 == 0)
    {
        D_801B2CB0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009D054(void)
{
    D_801B2CB0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009D06C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2CB8 = 1;
        D_801B2CBC = 1;
        return 1;
    }

    if (D_801B2CB8 < 0x6)
    {
        D_800D66EC[D_801B2CB8]();
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
void func_8009D0E4(void)
{
    D_801B2CB8 = 1;
    D_801B2CBC = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009D0FC(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA398, D_80139CA8, 0x14, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B2CBC == 0)
    {
        D_801B2CB8 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009D188(void)
{
    D_801B2CBC = 0x20;
    D_80139280[25] = -1;
    D_801B2CB8 += 1;
    func_8009D1D0();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009D1D0(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA398, D_80139CA8, 0x14, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B2CBC == 0)
    {
        D_801B2CB8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009D25C(void)
{
    D_801B2CB8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009D274(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2CC0 = 1;
        D_801B2CC4 = 1;
        return 1;
    }

    if (D_801B2CC0 < 0x6)
    {
        D_800D6704[D_801B2CC0]();
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
void func_8009D2EC(void)
{
    D_801B2CC0 = 1;
    D_801B2CC4 = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009D304(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB158, D_80139F28, 0x14, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0xA0));
    if (--D_801B2CC4 == 0)
    {
        D_801B2CC0 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009D390(void)
{
    D_801B2CC4 = 0x20;
    D_80139280[45] = -1;
    D_801B2CC0 += 1;
    func_8009D3D8();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009D3D8(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB158, D_80139F28, 0x14, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0xA0));
    if (--D_801B2CC4 == 0)
    {
        D_801B2CC0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009D464(void)
{
    D_801B2CC0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009D47C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2CC8 = 1;
        D_801B2CCC = 1;
        return 1;
    }

    if (D_801B2CC8 < 0x4)
    {
        D_800D671C[D_801B2CC8]();
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
void func_8009D4F4(void)
{
    D_801B2CC8 = 1;
    D_801B2CCC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8009D50C(void)
{
    D_801399CC = D_8011D538;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.previous_sequence = -1;
    D_800D93C8.shade_step = 2;
    D_800D93C8.target_shade = 0x81;
    D_800D93C8.resource_index = 0;
    D_800D93C8.sequence = 0;
    D_800D93C8.shade = 1;
    D_801B2CCC = 0x5A;
    D_801B2CC8 += 1;
    func_8009D58C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009D58C(void)
{
    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, g_wmap_focus_screen_position.packed, 0x1E, 0x3, 0);
    if (--D_801B2CCC == 0)
    {
        D_801B2CC8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009D608(void)
{
    D_801B2CC8 += 1;
}
