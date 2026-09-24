#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_land_effect_00.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80091650(void)
{
extern s32 D_801B2AC8;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern s32 D_8011CF1C;
extern s32 D_801B2AFC;
extern s32 D_801B2AF8;

    MATRIX m;
    s32 x;

    x = D_801B2650[2] - 0xDAC;
    D_801B2650[2] = x;
    if (x < 0x2710)
    {
        D_801B2650[2] = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_801B24A0, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DE8 != 0)
    {
        func_8006CD98(D_8011CF1C, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE8);
        D_80182DE8 -= 0x4;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--D_801B2AFC == 0)
    {
        D_801B2AF8 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80091750(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF24;
extern s32 D_801B2B04;
extern s32 D_801B2B00;

    MATRIX m;
    s32 x;

    x = D_801B2478[2] - 0xDAC;
    D_801B2478[2] = x;
    if (x < 0x2710)
    {
        D_801B2478[2] = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_801B24A8, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DEC != 0)
    {
        func_8006CD98(D_8011CF24, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DEC);
        D_80182DEC -= 0x2;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    if (--D_801B2B04 == 0)
    {
        D_801B2B00 += 1;
    }
}

/** @brief Draw the expanding effect and advance its rotation and countdown. */
void func_80091850(void)
{
extern u8 D_800DCF18[];
extern VECTOR D_80182DC0;
extern SVECTOR D_8013B240;
extern s32 D_801B2B08;
extern s32 D_801B2B0C;
extern s32 D_80182DF4;
extern s32 D_80139234;


    s32 scale;
    s32 intensity;
    s32 remaining;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_8013B240);
    wmap_draw_model(D_800DCF18, 0, 10, 183, 0x7A40, 0x1001, D_80182DF4, 0, 5, D_80139234 / 16);
    scale = D_80139234 - 128;
    D_80139234 = scale;
    intensity = D_80182DF4 + 2;
    D_80182DF4 = intensity;
    if (intensity >= 130)
    {
        D_80182DF4 = 129;
    }
    if (scale < 16)
    {
        D_80139234 = 16;
    }
    PopMatrix();
    remaining = D_801B2B0C - 1;
    D_8013B240.vz = (u16)(D_8013B240.vz + 220);
    D_801B2B0C = remaining;
    if (remaining == 0)
    {
        D_801B2B08++;
    }
}

/** @brief Draw and fade the rotating effect, then advance its countdown. */
void func_80091964(void)
{
extern u8 D_800DCF18[];
extern VECTOR D_80182DC0;
extern SVECTOR D_8013B240;
extern s32 D_801B2B08;
extern s32 D_801B2B0C;
extern s32 D_80182DF4;
extern s32 D_80139234;


    s32 intensity;
    s32 remaining;

    if (D_80182DF4 != 0)
    {
        PushMatrix();
        func_8006CFA8(&D_80182DC0, &D_8013B240);
        wmap_draw_model(D_800DCF18, 0, 10, 183, 0x7A40, 0x1001, D_80182DF4, 0, 5, D_80139234 / 16);
        intensity = D_80182DF4 - 4;
        D_80182DF4 = intensity;
        if (intensity < 0)
        {
            D_80182DF4 = 0;
        }
        PopMatrix();
        D_8013B240.vz = (u16)(D_8013B240.vz + 220);
    }
    remaining = D_801B2B0C - 1;
    D_801B2B0C = remaining;
    if (remaining == 0)
    {
        D_801B2B08++;
    }
}

/**
 * @brief Reset a range of world-map actor slots and kick the next handler.
 * @note Clears three parallel slot arrays for indices [0x64, 0x7C), seeds each
 *       actor's default fields, then advances the shared step counters.
 */
void func_80091A64(void)
{
extern u8 D_800D9268[];
extern u8 D_80139988[];
extern u8 D_801AFBD0[];
extern s32 D_80121538;
extern s32 D_80182DF0;
extern s32 D_800DCEA8;
extern s32 D_800D9150;
extern s32 D_801B2B10;
extern s32 D_801B2B14;
extern void func_80092D38__for_func_80091A64(void) __asm__("func_80092D38");

    s32 i;
    u8* pa;
    u8* pb;

    D_80182DF0 = 1;
    D_800DCEA8 = 1;
    for (i = 0x64; i < 0x7C; i++)
    {
        *(s16*)(D_801AFBD0 + i * 0x14) = 0;
        pb = D_80139988 + i * 0x8;
        *(s32*)(pb + 0x4) = (s32)&D_80121538;
        pa = D_800D9268 + i * 0x2C;
        *(s16*)(pa + 0x2) = 0;
        *(s8*)(pa + 0x6) = 0xF;
        *(s16*)(pa + 0xE) = 0;
        *(s16*)(pa + 0x10) = -1;
    }
    D_800D9150 = 4;
    D_801B2B14 = 0x10;
    D_801B2B10 += 1;
    func_80092D38__for_func_80091A64();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_80091B24(void)
{
/** @brief World-map 8-byte slot: only the +4 pointer field is written here. */
typedef struct
{
    s32 field_00;
    void *field_04;
} WmapSlot8;

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

extern s32 *D_80139280;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern s32 D_801B0FD0;
extern s32 D_801B2B18;
extern s32 D_801B2B1C;
extern u8 D_80121538[];
extern void func_80093074__for_func_80091B24(void) __asm__("func_80093074");

    s32 i;

    D_801B0FD0 = 25;
    D_80139280[0x1F] = 4;
    D_80139280[0x20] = 4;
    D_80139280[0x21] = 72;
    D_80139280[0x22] = 0;
    D_80139280[0x23] = 4;
    D_80139280[0x24] = 1500;
    D_80139280[0x25] = 20;
    D_80139280[0x26] = 19;
    D_80139280[0x27] = 1;
    D_80139280[0x28] = 10000;
    for (i = 0; i < 25; i++)
    {
        D_801AFBD0[i + D_80139280[0x25]].field_00 = 0;
        D_80139988[i + 24].field_04 = D_80121538;
    }
    D_801B2B1C = 100;
    D_801B2B18++;
    func_80093074__for_func_80091B24();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80091C0C(s32 arg0)
{
extern u32 D_801B2AD0;
extern s32 D_801B2AD4;
extern void (*D_800D6028[])(void);
extern void func_80091CE0__for_func_80091C0C(void) __asm__("func_80091CE0");
extern s32 D_8013B20C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2AD0 = 1;
        D_801B2AD4 = 1;
        return 1;
    }

    if (D_801B2AD0 < 0x8)
    {
        D_800D6028[D_801B2AD0]();
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
void func_80091C84(void)
{
extern u32 D_801B2AD0;
extern s32 D_801B2AD4;
extern void (*D_800D6028[])(void);
extern void func_80091CE0__for_func_80091C84(void) __asm__("func_80091CE0");
extern s32 D_8013B20C;

    D_801B2AD0 = 1;
    D_801B2AD4 = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80091C9C(void)
{
extern u32 D_801B2AD0;
extern s32 D_801B2AD4;
extern void (*D_800D6028[])(void);
extern void func_80091CE0__for_func_80091C9C(void) __asm__("func_80091CE0");
extern s32 D_8013B20C;

    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2AD0 += 1;
    func_80091CE0__for_func_80091C9C();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80091CE0(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2AD0;
extern void func_80091D1C__for_func_80091CE0(void) __asm__("func_80091D1C");
extern void func_80091E08__for_func_80091CE0(void) __asm__("func_80091E08");
extern void func_80091D60__for_func_80091CE0(void) __asm__("func_80091D60");

    if (D_8013B20C == 0)
    {
        D_801B2AD0 += 1;
        func_80091D1C__for_func_80091CE0();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80091D1C(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2AD0;
extern void func_80091D1C(void);
extern void func_80091E08__for_func_80091D1C(void) __asm__("func_80091E08");
extern void func_80091D60__for_func_80091D1C(void) __asm__("func_80091D60");

    func_8006CAC0(func_80091E08__for_func_80091D1C);
    D_8013B20C = 1;
    D_801B2AD0 += 1;
    func_80091D60__for_func_80091D1C();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80091D60(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2AD0;
extern void func_80091D9C__for_func_80091D60(void) __asm__("func_80091D9C");

    if (D_8013B20C == 0)
    {
        D_801B2AD0 += 1;
        func_80091D9C__for_func_80091D60();
    }
}

/**
 * @brief Reset a world-map step slot: arm its wait and advance the step index.
 */
void func_80091D9C(void)
{
extern s32 D_801B2AD0;
extern s32 D_801B2AD4;

    D_801B2AD4 = 5;
    D_801B2AD0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80091DBC(void)
{
extern s32 D_801B2AD0;
extern s32 D_801B2AD4;

    if (--D_801B2AD4 == 0)
    {
        D_801B2AD0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80091DF0(void)
{
extern s32 D_801B2AD0;
extern s32 D_801B2AD4;

    D_801B2AD0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80091E08(s32 arg0)
{
extern u32 D_801B2AD8;
extern s32 D_801B2ADC;
extern void (*D_800D6048[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2AD8 = 1;
        D_801B2ADC = 1;
        return 1;
    }

    if (D_801B2AD8 < 0x12)
    {
        D_800D6048[D_801B2AD8]();
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
void func_80091E80(void)
{
extern u32 D_801B2AD8;
extern s32 D_801B2ADC;
extern void (*D_800D6048[])(void);

    D_801B2AD8 = 1;
    D_801B2ADC = 1;
}

/**
 * @brief Set up a tinted world-map sub-scene and register its handler.
 */
void func_80091E98(void)
{
extern s32 D_8013B208;
extern s32 D_801B2AD8;
extern s32 D_801B2ADC;
extern void func_800931DC__for_func_80091E98(void) __asm__("func_800931DC");

    D_8013B208 = 1;
    func_8006683C(0x404045);
    g_wmap_backdrop_target_level = 8;
    func_800652A8(0x28, 0x80);
    func_8006CAC0(func_800931DC__for_func_80091E98);
    D_801B2ADC = 0x1E;
    D_801B2AD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80091F04(void)
{
extern s32 D_801B2ADC;
extern s32 D_801B2AD8;
extern void func_80092898__for_func_80091F04(void) __asm__("func_80092898");
extern void func_800922F4__for_func_80091F04(void) __asm__("func_800922F4");

    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_80091F38(void)
{
extern s32 D_801B2ADC;
extern s32 D_801B2AD8;
extern void func_80092898__for_func_80091F38(void) __asm__("func_80092898");
extern void func_800922F4__for_func_80091F38(void) __asm__("func_800922F4");

    func_8006CAC0(func_80092898__for_func_80091F38);
    func_8006CAC0(func_800922F4__for_func_80091F38);
    D_801B2ADC = 0x4;
    D_801B2AD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80091F80(void)
{
extern s32 D_801B2ADC;
extern s32 D_801B2AD8;

    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/** @brief World-map step: register a callback, set flags, advance the step. */
void func_80091FB4(void)
{
extern s32 D_801ADAE0;
extern s32 D_801B2AD8;
extern s32 D_801B2ADC;
extern void func_80092CA8__for_func_80091FB4(void) __asm__("func_80092CA8");

    func_8006CAC0(func_80092CA8__for_func_80091FB4);
    D_801ADAE0 = 1;
    func_8006683C(0x202540);
    g_wmap_backdrop_target_level = 3;
    D_801B2ADC = 0x14;
    D_801B2AD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80092014(void)
{
extern s32 D_801B2ADC;
extern s32 D_801B2AD8;
extern void func_80092FE4__for_func_80092014(void) __asm__("func_80092FE4");

    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80092048(void)
{
extern s32 D_801B2ADC;
extern s32 D_801B2AD8;
extern void func_80092FE4__for_func_80092048(void) __asm__("func_80092FE4");

    func_8006CAC0(func_80092FE4__for_func_80092048);
    D_801B2ADC = 0x2;
    D_801B2AD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80092084(void)
{
extern s32 D_801B2ADC;
extern s32 D_801B2AD8;
extern void func_80092B48__for_func_80092084(void) __asm__("func_80092B48");

    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800920B8(void)
{
extern s32 D_801B2ADC;
extern s32 D_801B2AD8;
extern void func_80092B48__for_func_800920B8(void) __asm__("func_80092B48");

    func_8006CAC0(func_80092B48__for_func_800920B8);
    D_801B2ADC = 0x11;
    D_801B2AD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800920F4(void)
{
extern s32 D_801B2ADC;
extern s32 D_801B2AD8;
extern void func_80092490__for_func_800920F4(void) __asm__("func_80092490");

    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80092128(void)
{
extern s32 D_801B2ADC;
extern s32 D_801B2AD8;
extern void func_80092490__for_func_80092128(void) __asm__("func_80092490");

    func_8006CAC0(func_80092490__for_func_80092128);
    D_801B2ADC = 0x26;
    D_801B2AD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80092164(void)
{
extern s32 D_801B2ADC;
extern s32 D_801B2AD8;

    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/** @brief World-map step handler: register a callback, kick a job, advance the step. */
void func_80092198(void)
{
extern s32 D_801B2AD8;
extern s32 D_801B2ADC;
extern void func_800929F0__for_func_80092198(void) __asm__("func_800929F0");

    func_8006CAC0(func_800929F0__for_func_80092198);
    func_8006683C(0x404050);
    g_wmap_backdrop_target_level = 8;
    D_801B2ADC = 4;
    D_801B2AD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800921EC(void)
{
extern s32 D_801B2ADC;
extern s32 D_801B2AD8;
extern void func_800926F8__for_func_800921EC(void) __asm__("func_800926F8");

    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80092220(void)
{
extern s32 D_801B2ADC;
extern s32 D_801B2AD8;
extern void func_800926F8__for_func_80092220(void) __asm__("func_800926F8");

    func_8006CAC0(func_800926F8__for_func_80092220);
    D_801B2ADC = 0x9C;
    D_801B2AD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009225C(void)
{
extern s32 D_801B2ADC;
extern s32 D_801B2AD8;

    if (--D_801B2ADC == 0)
    {
        D_801B2AD8 += 1;
    }
}

/** @brief Update the selected world-map cell value, clear the gate flag, and advance the sequence. */
void func_80092290(void)
{
extern s32 D_8013B20C;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2AD8;

    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B2AD8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800922F4(s32 arg0)
{
extern u32 D_801B2AE0;
extern s32 D_801B2AE4;
extern void (*D_800D6090[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2AE0 = 1;
        D_801B2AE4 = 1;
        return 1;
    }

    if (D_801B2AE0 < 0x4)
    {
        D_800D6090[D_801B2AE0]();
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
void func_8009236C(void)
{
extern u32 D_801B2AE0;
extern s32 D_801B2AE4;
extern void (*D_800D6090[])(void);

    D_801B2AE0 = 1;
    D_801B2AE4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void func_80092384(void)
{
extern u8* D_801399AC;
extern u8 D_8011F538[];
extern u8 D_800D9318[];
extern s32 D_801B2AE0;
extern s32 D_801B2AE4;
extern void func_800923FC__for_func_80092384(void) __asm__("func_800923FC");

    D_801399AC = D_8011F538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x26] = 0;
    *(s16*)&D_800D9318[0x22] = 0x81;
    *(s16*)&D_800D9318[0x24] = 0x81;
    D_801B2AE4 = 0x30;
    D_801B2AE0 += 1;
    func_800923FC__for_func_80092384();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800923FC(void)
{
extern s32 D_801B2AE0;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B2AE4;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0x13, 0x14, 0);
    if (--D_801B2AE4 == 0)
    {
        D_801B2AE0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80092478(void)
{
extern s32 D_801B2AE0;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B2AE4;

    D_801B2AE0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80092490(s32 arg0)
{
extern u32 D_801B2AE8;
extern s32 D_801B2AEC;
extern void (*D_800D60A0[])(void);
extern void func_8009259C__for_func_80092490(void) __asm__("func_8009259C");
extern u8 D_8011F538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2AE8 = 1;
        D_801B2AEC = 1;
        return 1;
    }

    if (D_801B2AE8 < 0x6)
    {
        D_800D60A0[D_801B2AE8]();
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
void func_80092508(void)
{
extern u32 D_801B2AE8;
extern s32 D_801B2AEC;
extern void (*D_800D60A0[])(void);
extern void func_8009259C__for_func_80092508(void) __asm__("func_8009259C");
extern u8 D_8011F538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    D_801B2AE8 = 1;
    D_801B2AEC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80092520(void)
{
extern u32 D_801B2AE8;
extern s32 D_801B2AEC;
extern void (*D_800D60A0[])(void);
extern void func_8009259C__for_func_80092520(void) __asm__("func_8009259C");
extern u8 D_8011F538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    D_801399CC = D_8011F538;
    D_800D93C8[0x6] = 0xF;
    *(s16*)&D_800D93C8[0xE] = 1;
    *(s16*)&D_800D93C8[0x10] = -1;
    *(s16*)&D_800D93C8[0x2] = 0;
    *(s16*)&D_800D93C8[0x26] = 0;
    *(s16*)&D_800D93C8[0x22] = 0x81;
    *(s16*)&D_800D93C8[0x24] = 0x81;
    D_801B2AEC = 0x60;
    D_801B2AE8 += 1;
    func_8009259C__for_func_80092520();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009259C(void)
{
extern u32 D_801B2AE8;
extern s32 D_801B2AEC;
extern void (*D_800D60A0[])(void);
extern void func_8009259C(void);
extern u8 D_8011F538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_8011CF4C, 0x13, 0x14, 0);
    if (--D_801B2AEC == 0)
    {
        D_801B2AE8 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80092618(void)
{
extern void func_80092664__for_func_80092618(void) __asm__("func_80092664");
extern s16 D_800D93C8[];
extern s32 D_801B2AEC;
extern s32 D_801B2AE8;
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    D_800D93C8[19] = 4;
    D_800D93C8[17] = 0;
    D_801B2AEC = 0x20;
    D_801B2AE8 += 1;
    func_80092664__for_func_80092618();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80092664(void)
{
extern void func_80092664(void);
extern s16 D_800D93C8[];
extern s32 D_801B2AEC;
extern s32 D_801B2AE8;
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_8011CF4C, 0x13, 0x14, 0);
    if (--D_801B2AEC == 0)
    {
        D_801B2AE8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800926E0(void)
{
extern s32 D_801B2AE8;

    D_801B2AE8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800926F8(s32 arg0)
{
extern u32 D_801B2AF0;
extern s32 D_801B2AF4;
extern void (*D_800D60B8[])(void);
extern void func_80092804__for_func_800926F8(void) __asm__("func_80092804");
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2AF0 = 1;
        D_801B2AF4 = 1;
        return 1;
    }

    if (D_801B2AF0 < 0x4)
    {
        D_800D60B8[D_801B2AF0]();
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
void func_80092770(void)
{
extern u32 D_801B2AF0;
extern s32 D_801B2AF4;
extern void (*D_800D60B8[])(void);
extern void func_80092804__for_func_80092770(void) __asm__("func_80092804");
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801B2AF0 = 1;
    D_801B2AF4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80092788(void)
{
extern u32 D_801B2AF0;
extern s32 D_801B2AF4;
extern void (*D_800D60B8[])(void);
extern void func_80092804__for_func_80092788(void) __asm__("func_80092804");
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801399B4 = D_8011D538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 8;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x22] = 0x80;
    *(s16*)&D_800D9344[0x24] = 0;
    D_801B2AF4 = 0x9D;
    D_801B2AF0 += 1;
    func_80092804__for_func_80092788();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80092804(void)
{
extern u32 D_801B2AF0;
extern s32 D_801B2AF4;
extern void (*D_800D60B8[])(void);
extern void func_80092804(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x14, 0x1F, 0);
    if (--D_801B2AF4 == 0)
    {
        D_801B2AF0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80092880(void)
{
extern s32 D_801B2AF0;

    D_801B2AF0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80092898(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2AF8;
extern s32 D_801B2AFC;
extern void (*D_800D60C8[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80091650__for_func_80092898(void) __asm__("func_80091650");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2AF8 = 1;
        D_801B2AFC = 1;
        return 1;
    }

    if (D_801B2AF8 < 0x4)
    {
        D_800D60C8[D_801B2AF8]();
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
void func_80092910(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2AF8;
extern s32 D_801B2AFC;
extern void (*D_800D60C8[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80091650__for_func_80092910(void) __asm__("func_80091650");

    D_801B2AF8 = 1;
    D_801B2AFC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80092928(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2AF8;
extern s32 D_801B2AFC;
extern void (*D_800D60C8[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80091650__for_func_80092928(void) __asm__("func_80091650");

    D_801B24A0 = D_80139258;
    D_801B2650 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B2AFC = 0x20;
    D_801B2AF8 += 1;
    func_80091650__for_func_80092928();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800929D8(void)
{
extern s32 D_801B2AF8;

    D_801B2AF8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800929F0(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2B00;
extern s32 D_801B2B04;
extern void (*D_800D60D8[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_80091750__for_func_800929F0(void) __asm__("func_80091750");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B00 = 1;
        D_801B2B04 = 1;
        return 1;
    }

    if (D_801B2B00 < 0x4)
    {
        D_800D60D8[D_801B2B00]();
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
void func_80092A68(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2B00;
extern s32 D_801B2B04;
extern void (*D_800D60D8[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_80091750__for_func_80092A68(void) __asm__("func_80091750");

    D_801B2B00 = 1;
    D_801B2B04 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80092A80(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2B00;
extern s32 D_801B2B04;
extern void (*D_800D60D8[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_80091750__for_func_80092A80(void) __asm__("func_80091750");

    D_801B24A8 = D_80139258;
    D_801B2478 = D_80182DC0;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B2B04 = 0x40;
    D_801B2B00 += 1;
    func_80091750__for_func_80092A80();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80092B30(void)
{
extern s32 D_801B2B00;

    D_801B2B00 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80092B48(s32 arg0)
{
extern u32 D_801B2B08;
extern s32 D_801B2B0C;
extern void (*D_800D60E8[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B08 = 1;
        D_801B2B0C = 1;
        return 1;
    }

    if (D_801B2B08 < 0x6)
    {
        D_800D60E8[D_801B2B08]();
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
void func_80092BC0(void)
{
extern u32 D_801B2B08;
extern s32 D_801B2B0C;
extern void (*D_800D60E8[])(void);

    D_801B2B08 = 1;
    D_801B2B0C = 1;
}

/** @brief Restore effect state and begin a 66-tick sequence step. */
void func_80092BD8(void)
{
/** @brief Eight bytes of world-map effect state copied together. */
typedef struct
{
    u8 bytes[8];
} WmapBlk8;

extern void func_80091850__for_func_80092BD8(void) __asm__("func_80091850");
extern s32 D_80139234;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern s32 D_80182DF4;
extern s32 D_801B2B08;
extern s32 D_801B2B0C;

    D_80182DF4 = 1;
    D_8013B240 = D_80139258;
    D_80139234 = 0x200;
    D_801B2B0C = 0x42;
    D_801B2B08 += 1;
    func_80091850__for_func_80092BD8();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80092C58(void)
{
extern s32 D_801B2B08;
extern void func_80091964__for_func_80092C58(void) __asm__("func_80091964");
extern s32 D_801B2B0C;

    D_801B2B0C = 0x20;
    D_801B2B08 += 1;
    func_80091964__for_func_80092C58();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80092C90(void)
{
extern s32 D_801B2B08;
extern void func_80091964__for_func_80092C90(void) __asm__("func_80091964");
extern s32 D_801B2B0C;

    D_801B2B08 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80092CA8(s32 arg0)
{
extern u32 D_801B2B10;
extern s32 D_801B2B14;
extern void (*D_800D6100[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B10 = 1;
        D_801B2B14 = 1;
        return 1;
    }

    if (D_801B2B10 < 0x8)
    {
        D_800D6100[D_801B2B10]();
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
void func_80092D20(void)
{
extern u32 D_801B2B10;
extern s32 D_801B2B14;
extern void (*D_800D6100[])(void);

    D_801B2B10 = 1;
    D_801B2B14 = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_80092D38(void)
{
extern s32 D_80182DF0;
extern s32 D_801B2B10;
extern s32 D_801B2B14;

    s32 remaining;

    func_8006B328(0x64, 0x7C, 4, -1, -3, -4, 0, 0x13, -0xA0, 0x140, -0xA0, 0x140, 0x32, 1, 0x81, 4, 0);
    D_80182DF0 += 8;
    remaining = D_801B2B14 - 1;
    D_801B2B14 = remaining;
    if (remaining == 0)
    {
        D_801B2B10 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80092DF4(void)
{
extern void func_80092E2C__for_func_80092DF4(void) __asm__("func_80092E2C");
extern s32 D_801B2B14;
extern s32 D_801B2B10;

    D_801B2B14 = 0x60;
    D_801B2B10 += 1;
    func_80092E2C__for_func_80092DF4();
}

/** @brief World-map effect spawn: submit a request and tick the refcount. */
void func_80092E2C(void)
{
extern s32 D_801B2B10;
extern s32 D_801B2B14;

    s32 c;

    func_8006B328(0x64, 0x7C, 4, -1, -3, -4, 0, 0x13, -0xA0, 0x140, -0xA0,
                  0x140, 0x32, 1, 0x81, 4, 0);
    c = D_801B2B14 - 1;
    D_801B2B14 = c;
    if (c == 0)
    {
        D_801B2B10 += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_80092EDC(void)
{
extern void func_80092F1C__for_func_80092EDC(void) __asm__("func_80092F1C");
extern s32 D_800DCEA8;
extern s32 D_801B2B14;
extern s32 D_801B2B10;

    D_800DCEA8 = 0;
    D_801B2B14 = 0x40;
    D_801B2B10 += 1;
    func_80092F1C__for_func_80092EDC();
}

/** @brief World-map effect spawn: submit a request and tick the refcount. */
void func_80092F1C(void)
{
extern s32 D_801B2B10;
extern s32 D_801B2B14;

    s32 c;

    func_8006B328(0x64, 0x7C, 4, -1, -3, -4, 0, 0x13, -0xA0, 0x140, -0xA0,
                  0x140, 0x32, 1, 0x81, 4, 0);
    c = D_801B2B14 - 1;
    D_801B2B14 = c;
    if (c == 0)
    {
        D_801B2B10 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80092FCC(void)
{
extern s32 D_801B2B10;

    D_801B2B10 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80092FE4(s32 arg0)
{
extern u32 D_801B2B18;
extern s32 D_801B2B1C;
extern void (*D_800D6120[])(void);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32 D_80139280;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B18 = 1;
        D_801B2B1C = 1;
        return 1;
    }

    if (D_801B2B18 < 0x6)
    {
        D_800D6120[D_801B2B18]();
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
void func_8009305C(void)
{
extern u32 D_801B2B18;
extern s32 D_801B2B1C;
extern void (*D_800D6120[])(void);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32 D_80139280;

    D_801B2B18 = 1;
    D_801B2B1C = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80093074(void)
{
extern u32 D_801B2B18;
extern s32 D_801B2B1C;
extern void (*D_800D6120[])(void);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32 D_80139280;

    func_8006A2FC(D_800D9688, D_80139A48, 0x19, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2B1C == 0)
    {
        D_801B2B18 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_800930F8(void)
{
extern void func_80093140__for_func_800930F8(void) __asm__("func_80093140");
extern s32* D_80139280;
extern s32 D_801B2B1C;
extern s32 D_801B2B18;
extern u8 D_800D9688[];
extern u8 D_80139A48[];

    D_801B2B1C = 0x20;
    D_80139280[35] = -1;
    D_801B2B18 += 1;
    func_80093140__for_func_800930F8();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80093140(void)
{
extern void func_80093140(void);
extern s32* D_80139280;
extern s32 D_801B2B1C;
extern s32 D_801B2B18;
extern u8 D_800D9688[];
extern u8 D_80139A48[];

    func_8006A2FC(D_800D9688, D_80139A48, 0x19, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2B1C == 0)
    {
        D_801B2B18 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800931C4(void)
{
extern s32 D_801B2B18;

    D_801B2B18 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800931DC(s32 arg0)
{
extern u32 D_801B2B20;
extern s32 D_801B2B24;
extern void (*D_800D6138[])(void);
extern void func_800932EC__for_func_800931DC(void) __asm__("func_800932EC");
extern u8 D_80123538[];
extern u8* D_801399D4;
extern u8 D_800D93F4[];
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B20 = 1;
        D_801B2B24 = 1;
        return 1;
    }

    if (D_801B2B20 < 0x6)
    {
        D_800D6138[D_801B2B20]();
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
void func_80093254(void)
{
extern u32 D_801B2B20;
extern s32 D_801B2B24;
extern void (*D_800D6138[])(void);
extern void func_800932EC__for_func_80093254(void) __asm__("func_800932EC");
extern u8 D_80123538[];
extern u8* D_801399D4;
extern u8 D_800D93F4[];
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

    D_801B2B20 = 1;
    D_801B2B24 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8009326C(void)
{
extern u32 D_801B2B20;
extern s32 D_801B2B24;
extern void (*D_800D6138[])(void);
extern void func_800932EC__for_func_8009326C(void) __asm__("func_800932EC");
extern u8 D_80123538[];
extern u8* D_801399D4;
extern u8 D_800D93F4[];
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

    D_801399D4 = D_80123538;
    D_800D93F4[0x6] = 0xF;
    *(s16*)&D_800D93F4[0x10] = -1;
    *(s16*)&D_800D93F4[0x26] = 2;
    *(s16*)&D_800D93F4[0x22] = 0x81;
    *(s16*)&D_800D93F4[0x2] = 0;
    *(s16*)&D_800D93F4[0xE] = 0;
    *(s16*)&D_800D93F4[0x24] = 1;
    D_801B2B24 = 0x88;
    D_801B2B20 += 1;
    func_800932EC__for_func_8009326C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800932EC(void)
{
extern u32 D_801B2B20;
extern s32 D_801B2B24;
extern void (*D_800D6138[])(void);
extern void func_800932EC(void);
extern u8 D_80123538[];
extern u8* D_801399D4;
extern u8 D_800D93F4[];
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D93F4, D_801399D0);
    func_80066F9C(D_800D93F4, D_8011CF4C, 0x8, 0x1E, 0);
    if (--D_801B2B24 == 0)
    {
        D_801B2B20 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80093368(void)
{
extern void func_800933B4__for_func_80093368(void) __asm__("func_800933B4");
extern s16 D_800D93F4[];
extern s32 D_801B2B24;
extern s32 D_801B2B20;
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

    D_800D93F4[19] = 4;
    D_800D93F4[17] = 0;
    D_801B2B24 = 0x20;
    D_801B2B20 += 1;
    func_800933B4__for_func_80093368();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800933B4(void)
{
extern void func_800933B4(void);
extern s16 D_800D93F4[];
extern s32 D_801B2B24;
extern s32 D_801B2B20;
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D93F4, D_801399D0);
    func_80066F9C(D_800D93F4, D_8011CF4C, 0x8, 0x1E, 0);
    if (--D_801B2B24 == 0)
    {
        D_801B2B20 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80093430(void)
{
extern s32 D_801B2B20;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2B4C;
extern s32 D_801B2B48;

    D_801B2B20 += 1;
}
