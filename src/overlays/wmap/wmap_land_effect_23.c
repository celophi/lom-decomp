#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_land_effect_23.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

void func_8009F2B0(void);
void func_8009F4B0(void);
void func_8009FC60(void);
void func_800A0444(void);
s32 func_8009E3AC(s32 arg0);
void func_8009E348(void);
void func_8009E384(void);
s32 func_800A06FC(s32 arg0);
s32 func_8009F220(s32 arg0);
s32 func_8009FBD0(s32 arg0);
s32 func_8009F420(s32 arg0);
s32 func_8009EE1C(s32 arg0);
s32 func_8009EC7C(s32 arg0);
s32 func_8009F628(s32 arg0);
s32 func_8009EF74(s32 arg0);
s32 func_8009FDD8(s32 arg0);
s32 func_800A03B4(s32 arg0);
s32 func_800A00C4(s32 arg0);
s32 func_8009F8E0(s32 arg0);
s32 func_8009F784(s32 arg0);
s32 func_8009F0C8(s32 arg0);
void func_8009ED88(void);
void func_8009F380(void);
void func_8009F584(void);
void func_8009FA0C(void);
void func_8009FB08(void);
void func_8009FD34(void);
void func_8009FF00(void);
void func_8009FFFC(void);
void func_800A01F0(void);
void func_800A02EC(void);
void func_800A053C(void);
void func_800A0630(void);

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

/** @brief Record with a leading value and a 40-byte stride. */
typedef struct
{
    s32 value;
    u8 unknown_4[36];
} WmapValueRecord;

extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2CEC;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B2CF4;
extern s32 D_80182DF0;
extern s32 D_801B2CFC;
extern u8 D_8011F538[];
extern s32 D_801B2D04;
extern s32 D_801B2D0C;
extern s32* D_8011CF2C;
extern s32 D_80139234;
extern s32 D_801B25D8;
extern s32 D_801B2D14;
extern s32* D_8011CF28;
extern s32 D_8013923C;
extern s32 D_801B2D1C;
extern s32 D_801B2D2C;
extern s32 D_80121538[];
extern s32 D_800D9154;
extern s32 D_800DCEAC;
extern s32 D_80182DF4;
extern s32 D_801B2D44;
extern int rand(void);
extern s32 D_80139264;
extern s32 D_801B2D4C;
extern s32 D_801B2CD4;
extern void (*D_800D672C[])(void);
extern s32 D_80139978;
extern s32 D_8013B20C;
extern s32 D_8013B294;
extern s32 D_80139228;
extern s32 D_801B2CDC;
extern void (*D_800D673C[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern s32 D_800DBE70;
extern s32 D_80139244;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern WmapValueRecord D_80139290[][6];
extern s32 D_801B2CE4;
extern void (*D_800D67CC[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern void func_8009ED88(void);
extern void (*D_800D67DC[])(void);
extern void (*D_800D67EC[])(void);
extern void (*D_800D67FC[])(void);
extern void (*D_800D680C[])(void);
extern u8 D_800DB158[];
extern u8 D_80139F28[];
extern void (*D_800D6824[])(void);
extern u8 D_800D9790[];
extern u8 D_80139A78[];
extern void func_8009F584(void);
extern void (*D_800D683C[])(void);
extern void (*D_800D6854[])(void);
extern s32 D_801B2D24;
extern void (*D_800D686C[])(void);
extern u8 D_80125538[];
extern s32 D_8013924C;
extern u8 *D_801399BC;
extern void (*D_800D6884[])(void);
extern u8 D_800DA8C0[];
extern u8 D_80139D98[];
extern void func_8009FD34(void);
extern s32 D_801B2D34;
extern void (*D_800D689C[])(void);
extern u8 D_80123538[];
extern s32 D_80139250;
extern u8 *D_801399C4;
extern s32 D_801B2D3C;
extern void (*D_800D68B4[])(void);
extern s32 D_80139260;
extern u8 *D_801399CC;
extern void (*D_800D68CC[])(void);
extern void (*D_800D68EC[])(void);
extern u32 D_801B2CE8;
extern u32 D_801B2CF0;
extern u32 D_801B2CF8;
extern u32 D_801B2D00;
extern u32 D_801B2D08;

extern u32 D_801B2D10;
extern u32 D_801B2D18;
extern u32 D_801B2D28;
extern u32 D_801B2D40;
extern u32 D_801B2D48;
extern u32 D_801B2CD0;
extern u32 D_801B2CD8;
extern u32 D_801B2CE0;
extern u32 D_801B2D20;
extern u32 D_801B2D30;
extern u32 D_801B2D38;

extern WmapSpriteActor D_800D9268[];
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
extern WmapScreenPosition D_80182D58;
extern WmapScreenPosition D_80182D60;
extern WmapScreenPosition D_80182D64;

extern s32* D_80139280;

extern WmapSlot14 D_801AFBD0[];

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;
extern VECTOR D_80139870;
extern VECTOR D_80182D48;

extern SVECTOR D_80139258;
extern SVECTOR D_801B24A0;
extern SVECTOR D_8013B238;
extern SVECTOR D_801B24A8;
extern SVECTOR D_801398C8;

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8009D620(void)
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
    if (--D_801B2CEC == 0)
    {
        D_801B2CE8 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8009D720(void)
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
    if (--D_801B2CF4 == 0)
    {
        D_801B2CF0 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8009D820(void)
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
    if (--D_801B2CFC == 0)
    {
        D_801B2CF8 += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_8009D920(void)
{
    s32 i;

    D_80139280[0x1] = 1;
    D_80139280[0x2] = 0;
    D_80139280[0x3] = 0x20;
    D_80139280[0x4] = 0;
    D_80139280[0x5] = 4;
    D_80139280[0x6] = 0x320;
    D_80139280[0x7] = 0xB4;
    D_80139280[0x8] = 8;
    D_80139280[0x9] = 1;
    D_80139280[0xA] = 0x124F8;
    for (i = 0; i < 20; i++)
    {
        D_801AFBD0[i + 180].field_00 = 0;
        D_80139988[i + 180].data = D_8011F538;
    }
    D_801B2D04 = 80;
    D_801B2D00++;
    func_8009F2B0();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_8009D9F0(void)
{
    s32 i;

    D_80139280[0xB] = 0;
    D_80139280[0xC] = 0;
    D_80139280[0xD] = 0x40;
    D_80139280[0xE] = 0;
    D_80139280[0xF] = 5;
    D_80139280[0x10] = 0x258;
    D_80139280[0x11] = 0x1E;
    D_80139280[0x12] = 8;
    D_80139280[0x13] = 2;
    D_80139280[0x14] = 0x2710;
    for (i = 0; i < 90; i++)
    {
        D_801AFBD0[i + 30].field_00 = 0;
        D_80139988[i + 30].data = D_8011F538;
    }
    D_801B2D0C = 450;
    D_801B2D08++;
    func_8009F4B0();
}

/**
 * @brief Draw the first animated element, ramp its intensity up, rotate it, and advance after its timer expires.
 */
void func_8009DABC(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF2C, D_80139234 & 3, 0xA, 0x36, 0x7900, 0x1001, D_801B25D8, 0, 0xF, -1);
    value = D_801B25D8 + 2;
    D_801B25D8 = value;
    if (value >= 0x82)
    {
        D_801B25D8 = 0x81;
    }
    timer = D_801B2D14;
    D_8013B238.vz += 0x14;
    next_timer = timer - 1;
    D_801B2D14 = next_timer;
    D_80139234 += 1;
    if (next_timer == 0)
    {
        D_801B2D10 += 1;
    }
}

/**
 * @brief Draw an animated element, ramp its intensity down, rotate it, and advance after its timer expires.
 */
void func_8009DBB0(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF2C, D_80139234 & 3, 0xA, 0x36, 0x7900, 0x1001, D_801B25D8, 0, 0xF, -1);
    value = D_801B25D8 - 4;
    D_801B25D8 = value;
    if (value < 0)
    {
        D_801B25D8 = 0;
    }
    timer = D_801B2D14;
    D_8013B238.vz += 0x14;
    next_timer = timer - 1;
    D_801B2D14 = next_timer;
    D_80139234 += 1;
    if (next_timer == 0)
    {
        D_801B2D10 += 1;
    }
}

/**
 * @brief Draw the second animated element, ramp its intensity up, rotate it, and advance after its timer expires.
 */
void func_8009DC9C(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B24A8);
    wmap_draw_model(D_8011CF28, D_8013923C & 3, 0x4, 0x35, 0x7800, 0x1001, D_80182DEC, 0, 0xA, -1);
    value = D_80182DEC + 8;
    D_80182DEC = value;
    if (value >= 0x82)
    {
        D_80182DEC = 0x81;
    }
    timer = D_801B2D1C;
    D_801B24A8.vz += 0x38;
    next_timer = timer - 1;
    D_801B2D1C = next_timer;
    D_8013923C += 1;
    if (next_timer == 0)
    {
        D_801B2D18 += 1;
    }
}

/**
 * @brief Draw the second animated element, ramp its intensity down, rotate it, and advance after its timer expires.
 */
void func_8009DD90(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B24A8);
    wmap_draw_model(D_8011CF28, D_8013923C & 3, 0x4, 0x35, 0x7800, 0x1001, D_80182DEC, 0, 0xA, -1);
    value = D_80182DEC - 8;
    D_80182DEC = value;
    if (value < 0)
    {
        D_80182DEC = 0;
    }
    timer = D_801B2D1C;
    D_801B24A8.vz += 0x38;
    next_timer = timer - 1;
    D_801B2D1C = next_timer;
    D_8013923C += 1;
    if (next_timer == 0)
    {
        D_801B2D18 += 1;
    }
}

/** @brief Configure the effect and reset its forty-eight resource slots. */
void func_8009DE7C(void)
{
    s32 i;

    D_80139280[21] = 1;
    D_80139280[22] = 4;
    D_80139280[23] = 0x20;
    D_80139280[24] = 0;
    D_80139280[25] = 4;
    D_80139280[26] = 1;
    D_80139280[27] = 0x82;
    D_80139280[28] = 8;
    D_80139280[29] = 0;
    D_80139280[30] = 0x4650;
    for (i = 0; i < 48; i++)
    {
        D_801AFBD0[i + 130].field_00 = 0;
        D_80139988[i + 130].data = D_8011F538;
    }
    D_801B2D2C = 0xC0;
    D_801B2D28++;
    func_8009FC60();
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void func_8009DF44(void)
{
    s32 i;
    WmapD94Entry *entry;

    i = 0xB4;
    D_80182DF4 = 1;
    D_800DCEAC = 8;

    do
    {
        D_801AFBD0[i].field_00 = 0;
        D_80139988[i].data = D_80121538;
        entry = &D_800D9268[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 0;
        entry->unk10 = -1;
        i++;
    } while (i < 0xF0);

    D_800D9154 = 1;
    D_801B2D44 = 0x10;
    D_801B2D40 += 1;
    func_800A0444();
}

/**
 * @brief World-map step handler: jitter the actor position with a ramping random
 *        amplitude, then countdown-advance the step.
 */
void func_8009E008(void)
{
    D_801398C8.vx = rand() * D_80139264 / 16 >> 15;
    D_801398C8.vy = rand() * D_80139264 / 16 >> 15;
    D_80182D48.vx = rand() * D_80139264 / 16 >> 15;
    D_80182D48.vy = rand() * D_80139264 / 16 >> 15;
    if (D_80139264 < 0x80)
    {
        D_80139264 += 4;
    }
    if (--D_801B2D4C == 0)
    {
        D_801B2D48 += 1;
    }
}

/**
 * @brief World-map step handler: jitter the actor with a random amplitude, and when
 *        the amplitude ramp expires, zero the offsets; otherwise countdown-advance.
 */
void func_8009E114(void)
{
    D_801398C8.vx = rand() * D_80139264 / 16 >> 15;
    D_801398C8.vy = rand() * D_80139264 / 16 >> 15;
    D_80182D48.vx = rand() * D_80139264 / 16 >> 15;
    D_80182D48.vy = rand() * D_80139264 / 16 >> 15;
    if (--D_80139264 < 0)
    {
        D_80182D48.vy = 0;
        D_80182D48.vx = 0;
        D_801398C8.vy = 0;
        D_801398C8.vx = 0;
        D_801B2D48 += 1;
    }
    else if (--D_801B2D4C == 0)
    {
        D_801B2D48 += 1;
    }
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009E250(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2CD0 = 1;
        D_801B2CD4 = 1;
        return 1;
    }

    if (D_801B2CD0 < 0x4)
    {
        D_800D672C[D_801B2CD0]();
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
void func_8009E2C8(void)
{
    D_801B2CD0 = 1;
    D_801B2CD4 = 1;
}

/** @brief World-map step: arm a timed callback, flag it active, then tick the sub-counter. */
void func_8009E2E0(void)
{
    D_80139978 = 0x10;
    g_wmap_focus_screen_position.point.x = 0xA4;
    g_wmap_focus_screen_position.point.y = 0x69;
    wmap_start_sequence(func_8009E3AC);
    D_8013B20C = 1;
    D_801B2CD0 += 1;
    func_8009E348();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009E348(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2CD0 += 1;
        func_8009E384();
    }
}

/** @brief World-map trigger: set two flags and bump a counter. */
void func_8009E384(void)
{
    D_8013B294 = 1;
    D_80139228 = 1;
    D_801B2CD0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009E3AC(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2CD8 = 1;
        D_801B2CDC = 1;
        return 1;
    }

    if (D_801B2CD8 < 0x24)
    {
        D_800D673C[D_801B2CD8]();
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
void func_8009E424(void)
{
    D_801B2CD8 = 1;
    D_801B2CDC = 1;
}

/** @brief Set effect flags and color, play sound 41, and begin a 24-tick delay. */
void func_8009E43C(void)
{
    D_8013B208 = 1;
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 8;
    D_801ADAE0 = 1;
    wmap_play_sound(0x29, 0x80);
    D_801B2CDC = 0x18;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E4AC(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009E4E0(void)
{
    wmap_start_sequence(func_800A06FC);
    D_801B2CDC = 0x18;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E51C(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009E550(void)
{
    wmap_start_sequence(func_8009F220);
    D_801B2CDC = 0x65;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E58C(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_8009E5C0(void)
{
    wmap_start_sequence(func_8009FBD0);
    wmap_start_sequence(func_8009F420);
    wmap_start_sequence(func_8009EE1C);
    D_801B2CDC = 0x2;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E614(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/** @brief World-map step handler: register a callback and advance the step. */
void func_8009E648(void)
{
    wmap_start_sequence(func_8009EC7C);
    D_801B2CDC = 1;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E684(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/** @brief World-map step handler: seed timers and advance the counter. */
void func_8009E6B8(void)
{
    D_800DBE70 = 0;
    D_80139978 = -1;
    D_801B2CDC = 0x2D;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E6EC(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009E720(void)
{
    wmap_start_sequence(func_8009F628);
    D_801B2CDC = 0x1C;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E75C(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009E790(void)
{
    wmap_start_sequence(func_8009EF74);
    D_801B2CDC = 0x2;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E7CC(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/** @brief Set world-map flags and color, then begin a 48-tick delay. */
void func_8009E800(void)
{
    D_80139244 = 1;
    g_wmap_backdrop_target_level = 1;
    wmap_start_map_tint(0x201010);
    D_801B2CDC = 0x30;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E850(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009E884(void)
{
    wmap_start_sequence(func_8009FDD8);
    D_801B2CDC = 0x50;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E8C0(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/** @brief World-map step handler: register a callback, kick a job, advance the step. */
void func_8009E8F4(void)
{
    wmap_start_sequence(func_800A03B4);
    wmap_start_map_tint(0x302050);
    g_wmap_backdrop_target_level = 3;
    D_801B2CDC = 8;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E948(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/** @brief World-map step: set fade colour then advance to the next handler. */
void func_8009E97C(void)
{
    wmap_start_map_tint(0x252035);
    D_801B2CDC = 0x38;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E9B8(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009E9EC(void)
{
    wmap_start_sequence(func_800A00C4);
    D_801B2CDC = 0x22;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009EA28(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8009EA5C(void)
{
    wmap_start_sequence(func_8009F8E0);
    wmap_start_sequence(func_8009F784);
    D_801B2CDC = 0x48;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009EAA4(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009EAD8(void)
{
    wmap_start_sequence(func_8009F0C8);
    D_801B2CDC = 0x2;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009EB14(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/** @brief World-map step: kick a sub-request, set the next state, and arm the timer. */
void func_8009EB48(void)
{
    D_80139244 = 0;
    wmap_start_map_tint(0x602050);
    g_wmap_backdrop_target_level = 7;
    D_801B2CDC = 0x75;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009EB98(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/** @brief Set the selected record value and advance to a 50-tick delay. */
void func_8009EBCC(void)
{
    D_801B2CDC = 50;
    D_80139290[D_8011D510][D_8011D530].value = 0x117;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009EC2C(void)
{
    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief World-map step handler: clear the shared flag and advance the step counter.
 */
void func_8009EC60(void)
{
    D_8013B20C = 0;
    D_801B2CD8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009EC7C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2CE0 = 1;
        D_801B2CE4 = 1;
        return 1;
    }

    if (D_801B2CE0 < 0x4)
    {
        D_800D67CC[D_801B2CE0]();
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
void func_8009ECF4(void)
{
    D_801B2CE0 = 1;
    D_801B2CE4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8009ED0C(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 8;
    D_800D9344.unknown_02 = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x81;
    D_800D9344.shade = 0x81;
    D_801B2CE4 = 0x206;
    D_801B2CE0 += 1;
    func_8009ED88();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009ED88(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x17, 0x8, 0);
    if (--D_801B2CE4 == 0)
    {
        D_801B2CE0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009EE04(void)
{
    D_801B2CE0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009EE1C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2CE8 = 1;
        D_801B2CEC = 1;
        return 1;
    }

    if (D_801B2CE8 < 0x4)
    {
        D_800D67DC[D_801B2CE8]();
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
void func_8009EE94(void)
{
    D_801B2CE8 = 1;
    D_801B2CEC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8009EEAC(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B2CEC = 0x20;
    D_801B2CE8 += 1;
    func_8009D620();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009EF5C(void)
{
    D_801B2CE8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009EF74(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2CF0 = 1;
        D_801B2CF4 = 1;
        return 1;
    }

    if (D_801B2CF0 < 0x4)
    {
        D_800D67EC[D_801B2CF0]();
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
void func_8009EFEC(void)
{
    D_801B2CF0 = 1;
    D_801B2CF4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8009F004(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    D_801B2CF4 = 0x80;
    D_801B2CF0 += 1;
    func_8009D720();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009F0B0(void)
{
    D_801B2CF0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009F0C8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2CF8 = 1;
        D_801B2CFC = 1;
        return 1;
    }

    if (D_801B2CF8 < 0x4)
    {
        D_800D67FC[D_801B2CF8]();
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
void func_8009F140(void)
{
    D_801B2CF8 = 1;
    D_801B2CFC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8009F158(void)
{
    D_8013B238 = D_80139258;
    D_80139870 = g_wmap_camera_translation;
    D_80182DF0 = 0x80;
    D_80139870.vz = 0xAFC8;
    D_801B2CFC = 0x40;
    D_801B2CF8 += 1;
    func_8009D820();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009F208(void)
{
    D_801B2CF8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009F220(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D00 = 1;
        D_801B2D04 = 1;
        return 1;
    }

    if (D_801B2D00 < 0x6)
    {
        D_800D680C[D_801B2D00]();
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
void func_8009F298(void)
{
    D_801B2D00 = 1;
    D_801B2D04 = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009F2B0(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB158, D_80139F28, 0x14, 0xFF, 0x1, 0x8, 0, (s32)D_80139280);
    if (--D_801B2D04 == 0)
    {
        D_801B2D00 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009F338(void)
{
    D_801B2D04 = 0x20;
    D_80139280[5] = -1;
    D_801B2D00 += 1;
    func_8009F380();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009F380(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB158, D_80139F28, 0x14, 0xFF, 0x1, 0x8, 0, (s32)D_80139280);
    if (--D_801B2D04 == 0)
    {
        D_801B2D00 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009F408(void)
{
    D_801B2D00 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009F420(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D08 = 1;
        D_801B2D0C = 1;
        return 1;
    }

    if (D_801B2D08 < 0x6)
    {
        D_800D6824[D_801B2D08]();
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
void func_8009F498(void)
{
    D_801B2D08 = 1;
    D_801B2D0C = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009F4B0(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D9790, D_80139A78, 0x5A, 0xFF, 0x1, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2D0C == 0)
    {
        D_801B2D08 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009F53C(void)
{
    D_801B2D0C = 0x40;
    D_80139280[15] = -1;
    D_801B2D08 += 1;
    func_8009F584();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009F584(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D9790, D_80139A78, 0x5A, 0xFF, 0x1, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2D0C == 0)
    {
        D_801B2D08 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009F610(void)
{
    D_801B2D08 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009F628(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D10 = 1;
        D_801B2D14 = 1;
        return 1;
    }

    if (D_801B2D10 < 0x6)
    {
        D_800D683C[D_801B2D10]();
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
void func_8009F6A0(void)
{
    D_801B2D10 = 1;
    D_801B2D14 = 1;
}

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_8009F6B8(void)
{
    D_801B25D8 = 1;
    D_8013B238 = D_80139258;
    D_80139234 = 0;
    D_801B2D14 = 0x12C;
    D_801B2D10 += 1;
    func_8009DABC();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8009F734(void)
{
    D_801B2D14 = 0x20;
    D_801B2D10 += 1;
    func_8009DBB0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009F76C(void)
{
    D_801B2D10 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009F784(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D18 = 1;
        D_801B2D1C = 1;
        return 1;
    }

    if (D_801B2D18 < 0x6)
    {
        D_800D6854[D_801B2D18]();
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
void func_8009F7FC(void)
{
    D_801B2D18 = 1;
    D_801B2D1C = 1;
}

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_8009F814(void)
{
    D_80182DEC = 1;
    D_801B24A8 = D_80139258;
    D_8013923C = 0;
    D_801B2D1C = 0x3C;
    D_801B2D18 += 1;
    func_8009DC9C();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8009F890(void)
{
    D_801B2D1C = 0x10;
    D_801B2D18 += 1;
    func_8009DD90();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009F8C8(void)
{
    D_801B2D18 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009F8E0(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D20 = 1;
        D_801B2D24 = 1;
        return 1;
    }

    if (D_801B2D20 < 0x6)
    {
        D_800D686C[D_801B2D20]();
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
void func_8009F958(void)
{
    D_801B2D20 = 1;
    D_801B2D24 = 1;
}

/** @brief Initialize the actor and its screen coordinates, then advance the sequence. */
void func_8009F970(void)
{
    D_801399BC = D_80125538;
    D_8013924C = 0x280;
    D_800D9370.scale_index = 0xF;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 4;
    D_800D9370.target_shade = 0x81;
    D_800D9370.shade = 1;
    D_800D9370.unknown_02 = 0;
    D_800D9370.sequence = 0;
    D_801B2D24 = 0x9C;
    D_80182D64.packed = g_wmap_focus_screen_position.packed;
    D_801B2D20 += 1;
    func_8009FA0C();
}

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position downward and advance when the frame counter expires.
 */
void func_8009FA0C(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, D_80182D64.packed, 0x17, 0x2, 0);
    D_80182D64.point.y = D_8013924C / 0x10;
    if (D_8013924C < 0x640)
    {
        D_8013924C += 0x10;
    }
    if (--D_801B2D24 == 0)
    {
        D_801B2D20 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8009FABC(void)
{
    D_800D9370.shade_step = 2;
    D_800D9370.target_shade = 0;
    D_801B2D24 = 0x40;
    D_801B2D20 += 1;
    func_8009FB08();
}

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position and advance when the frame counter expires.
 */
void func_8009FB08(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, D_80182D64.packed, 0x17, 0x2, 0);
    D_80182D64.point.y = D_8013924C / 0x10;
    if (D_8013924C < 0x640)
    {
        D_8013924C += 0x10;
    }
    if (--D_801B2D24 == 0)
    {
        D_801B2D20 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009FBB8(void)
{
    D_801B2D20 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009FBD0(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D28 = 1;
        D_801B2D2C = 1;
        return 1;
    }

    if (D_801B2D28 < 0x6)
    {
        D_800D6884[D_801B2D28]();
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
void func_8009FC48(void)
{
    D_801B2D28 = 1;
    D_801B2D2C = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009FC60(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA8C0, D_80139D98, 0x30, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B2D2C == 0)
    {
        D_801B2D28 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009FCEC(void)
{
    D_801B2D2C = 0x20;
    D_80139280[25] = -1;
    D_801B2D28 += 1;
    func_8009FD34();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009FD34(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA8C0, D_80139D98, 0x30, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B2D2C == 0)
    {
        D_801B2D28 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009FDC0(void)
{
    D_801B2D28 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009FDD8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D30 = 1;
        D_801B2D34 = 1;
        return 1;
    }

    if (D_801B2D30 < 0x6)
    {
        D_800D689C[D_801B2D30]();
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
void func_8009FE50(void)
{
    D_801B2D30 = 1;
    D_801B2D34 = 1;
}

/** @brief Initialize the actor, save its screen position, and begin a 142-tick sequence step. */
void func_8009FE68(void)
{
    D_801399C4 = D_80123538;
    D_80139250 = 0x780;
    D_800D939C.scale_index = 0xF;
    D_800D939C.previous_sequence = -1;
    D_800D939C.shade_step = 1;
    D_800D939C.target_shade = 0x7F;
    D_800D939C.unknown_02 = 0;
    D_800D939C.sequence = 0;
    D_800D939C.shade = 0;
    D_801B2D34 = 0x8E;
    D_80182D58.packed = g_wmap_focus_screen_position.packed;
    D_801B2D30 += 1;
    func_8009FF00();
}

/** @brief Draw a world-map actor, then wind down a scrolling scalar. */
void func_8009FF00(void)
{
    wmap_step_actor_animation(&D_800D939C, &D_801399C0);
    wmap_draw_actor_sprite(&D_800D939C, D_80182D58.packed, 0x8, 0x2, 0);
    D_80182D58.point.y = D_80139250 / 16;
    if (D_80139250 >= 0x321)
    {
        D_80139250 -= 0x10;
    }
    if (--D_801B2D34 == 0)
    {
        D_801B2D30 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8009FFB0(void)
{
    D_800D939C.shade_step = 4;
    D_800D939C.target_shade = 0;
    D_801B2D34 = 0x20;
    D_801B2D30 += 1;
    func_8009FFFC();
}

/** @brief Draw a world-map actor, then wind down a scrolling scalar. */
void func_8009FFFC(void)
{
    wmap_step_actor_animation(&D_800D939C, &D_801399C0);
    wmap_draw_actor_sprite(&D_800D939C, D_80182D58.packed, 0x8, 0x2, 0);
    D_80182D58.point.y = D_80139250 / 16;
    if (D_80139250 >= 0x321)
    {
        D_80139250 -= 0x10;
    }
    if (--D_801B2D34 == 0)
    {
        D_801B2D30 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A00AC(void)
{
    D_801B2D30 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A00C4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D38 = 1;
        D_801B2D3C = 1;
        return 1;
    }

    if (D_801B2D38 < 0x6)
    {
        D_800D68B4[D_801B2D38]();
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
void func_800A013C(void)
{
    D_801B2D38 = 1;
    D_801B2D3C = 1;
}

/** @brief Initialize the actor and begin a 48-tick sequence step. */
void func_800A0154(void)
{
    D_801399CC = D_80123538;
    D_80139260 = 0x320;
    D_800D93C8.scale_index = 0xF;
    D_800D93C8.unknown_02 = 0;
    D_800D93C8.sequence = 1;
    D_800D93C8.previous_sequence = -1;
    D_800D93C8.shade_step = 4;
    D_800D93C8.target_shade = 0x81;
    D_800D93C8.shade = 1;
    D_801B2D3C = 0x30;
    D_80182D60.packed = g_wmap_focus_screen_position.packed;
    D_801B2D38 += 1;
    func_800A01F0();
}

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position and advance when the frame counter expires.
 */
void func_800A01F0(void)
{
    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, D_80182D60.packed, 0x8, 0x2, 0);
    D_80182D60.point.y = D_80139260 / 0x10;
    if (D_80139260 < 0xA00)
    {
        D_80139260 += 0x8;
    }
    if (--D_801B2D3C == 0)
    {
        D_801B2D38 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800A02A0(void)
{
    D_800D93C8.shade_step = 4;
    D_800D93C8.target_shade = 0;
    D_801B2D3C = 0x20;
    D_801B2D38 += 1;
    func_800A02EC();
}

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position and advance when the frame counter expires.
 */
void func_800A02EC(void)
{
    wmap_step_actor_animation(&D_800D93C8, &D_801399C8);
    wmap_draw_actor_sprite(&D_800D93C8, D_80182D60.packed, 0x8, 0x2, 0);
    D_80182D60.point.y = D_80139260 / 0x10;
    if (D_80139260 < 0xA00)
    {
        D_80139260 += 0x8;
    }
    if (--D_801B2D3C == 0)
    {
        D_801B2D38 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A039C(void)
{
    D_801B2D38 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A03B4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D40 = 1;
        D_801B2D44 = 1;
        return 1;
    }

    if (D_801B2D40 < 0x8)
    {
        D_800D68CC[D_801B2D40]();
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
void func_800A042C(void)
{
    D_801B2D40 = 1;
    D_801B2D44 = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_800A0444(void)
{
    s32 remaining;

    func_8006B328(0xB4, 0xF0, 1, -1, -1, -4, 0, 0xB, -0xFA, 0xFA, -0xDC, 0x8C, 1, 0x81, 1, 8, 1);
    D_80182DF4 += 8;
    remaining = D_801B2D44 - 1;
    D_801B2D44 = remaining;
    if (remaining == 0)
    {
        D_801B2D40 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A0504(void)
{
    D_801B2D44 = 0x28;
    D_801B2D40 += 1;
    func_800A053C();
}

/** @brief World-map step handler: spawn a scripted actor and expire the timer. */
void func_800A053C(void)
{
    func_8006B328(0xB4, 0xF0, 1, -1, -1, -4, 0, 0xB, -0xFA, 0xFA, -0xDC,
                  0x8C, 1, 0x81, 1, 8, 1);
    if (--D_801B2D44 == 0)
    {
        D_801B2D40 += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A05F0(void)
{
    D_800DCEAC = 0;
    D_801B2D44 = 0x40;
    D_801B2D40 += 1;
    func_800A0630();
}

/** @brief World-map step handler: spawn a scripted actor and expire the timer. */
void func_800A0630(void)
{
    func_8006B328(0xB4, 0xF0, 1, -1, -1, -4, 0, 0xB, -0xFA, 0xFA, -0xDC,
                  0x8C, 1, 0x81, 1, 8, 1);
    if (--D_801B2D44 == 0)
    {
        D_801B2D40 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A06E4(void)
{
    D_801B2D40 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A06FC(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D48 = 1;
        D_801B2D4C = 1;
        return 1;
    }

    if (D_801B2D48 < 0x6)
    {
        D_800D68EC[D_801B2D48]();
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
void func_800A0774(void)
{
    D_801B2D48 = 1;
    D_801B2D4C = 1;
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A078C(void)
{
    D_80139264 = 0;
    D_801B2D4C = 0x8C;
    D_801B2D48 += 1;
    func_8009E008();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A07CC(void)
{
    D_801B2D4C = 0x40;
    D_801B2D48 += 1;
    func_8009E114();
}

/**
 * @brief Clear the world-map translation and rotation offsets, then advance the transition counter.
 */
void func_800A0804(void)
{
    D_80182D48.vy = 0;
    D_80182D48.vx = 0;
    D_801398C8.vy = 0;
    D_801398C8.vx = 0;
    D_801B2D48 += 1;
}
