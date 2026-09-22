#include "wmap_land_effect_22.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_800A0838(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B2D74;
extern s32 D_801B2D70;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2D6C;
extern s32 D_801B2D68;

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
        func_8006CD98((s32)D_800DCF18, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE8);
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
void func_800A0938(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B2D74;
extern s32 D_801B2D70;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2D6C;
extern s32 D_801B2D68;

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
        func_8006CD98(D_8011CF1C, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DEC);
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
void func_800A0A38(void)
{
extern s32 D_80139870[];
extern SVECTOR D_8013B238;
extern VECTOR D_8011CF60;
extern s32 D_80182DF0;
extern s32 D_8011CF1C;
extern s32 D_801B2D7C;
extern s32 D_801B2D78;

    MATRIX m;
    s32 x;

    x = D_80139870[2] - 0xDAC;
    D_80139870[2] = x;
    if (x < 0x2710)
    {
        D_80139870[2] = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_8013B238, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DF0 != 0)
    {
        func_8006CD98(D_8011CF1C, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DF0);
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
void func_800A0B38(void)
{
extern int rand(void);
extern s16 D_801398C8[];
extern s32 D_80182D48[];
extern s32 D_80139268;
extern s32 D_801B2D8C;
extern s32 D_801B2D88;

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
void func_800A0C44(void)
{
extern int rand(void);
extern s16 D_801398C8[];
extern s32 D_80182D48[];
extern s32 D_80139268;
extern s32 D_801B2D88;
extern s32 D_801B2D8C;

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
void func_800A0D80(void)
{
extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern u8 D_80182DC0[];
extern u8 D_801B2498[];
extern s32* D_8011CF2C;
extern s32 D_8013923C;
extern s32 D_80182DE4;
extern s32 D_801B2DA0;
extern s32 D_801B2DA4;

    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(D_80182DC0, D_801B2498);
    func_800675F0(D_8011CF2C, (D_8013923C / 0x10) & 3, 0xA, 0x35, 0x7800, 0x1001, D_80182DE4, 0, 0x14, -1);
    D_8013923C += 0x10;
    value = D_80182DE4 + 2;
    D_80182DE4 = value;
    if (value >= 0x82)
    {
        D_80182DE4 = 0x81;
    }
    timer = D_801B2DA4;
    ((u16*)D_801B2498)[2] += 0x38;
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
void func_800A0E84(void)
{
extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern u8 D_80182DC0[];
extern u8 D_801B2498[];
extern s32* D_8011CF2C;
extern s32 D_8013923C;
extern s32 D_80182DE4;
extern s32 D_801B2DA0;
extern s32 D_801B2DA4;

    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(D_80182DC0, D_801B2498);
    func_800675F0(D_8011CF2C, (D_8013923C / 0x10) & 3, 0xA, 0x35, 0x7800, 0x1001, D_80182DE4, 0, 0x14, -1);
    value = D_80182DE4 - 2;
    D_80182DE4 = value;
    if (value < 0)
    {
        D_80182DE4 = 0;
    }
    D_8013923C += 0x10;
    timer = D_801B2DA4;
    ((u16*)D_801B2498)[2] += 0x38;
    next_timer = timer - 1;
    D_801B2DA4 = next_timer;
    if (next_timer == 0)
    {
        D_801B2DA0++;
    }
}

/** @brief Draw and brighten the rotating effect while reducing its scale. */
void func_800A0F84(void)
{
/* Partial WMAP decompilation: 96.750000% (gcc280_g0). */

extern void *D_8011CF28;
extern s32 D_80139264;
extern VECTOR D_80182DC0;
extern SVECTOR D_801B2670;
extern s32 D_801B2DA8;
extern s32 D_801B2DAC;
extern s32 D_801B25D8;
extern s32 D_80139234;
extern void func_800675F0(void *, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    s32 scale;
    s32 intensity;
    s32 remaining;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_801B2670);
    func_800675F0(D_8011CF28, D_80139264 & 3, 4, 54, 0x78C0, 0x1001, D_801B25D8, 0, 0, D_80139234 / 16);
    scale = D_80139234 - 32;
    D_80139264++;
    D_80139234 = scale;
    intensity = D_801B25D8 + 8;
    D_801B25D8 = intensity;
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
void func_800A10B4(void)
{
extern void *D_8011CF28;
extern s32 D_80139264;
extern VECTOR D_80182DC0;
extern SVECTOR D_801B2670;
extern s32 D_801B2DA8;
extern s32 D_801B2DAC;
extern s32 D_801B25D8;
extern s32 D_80139234;
extern void func_800675F0(void *, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    s32 intensity;
    s32 remaining;

    if (D_801B25D8 != 0)
    {
        PushMatrix();
        func_8006CFA8(&D_80182DC0, &D_801B2670);
        func_800675F0(D_8011CF28, D_80139264 & 3, 4, 54, 0x78C0, 0x1001, D_801B25D8, 0, 0, D_80139234 / 16);
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
void func_800A11C4(void)
{
/** @brief World-map vector; only the third halfword is changed here. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    s16 field_04;
    s16 field_06;
} WmapVector;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 D_8011CF28;
extern s32 D_80139260;
extern WmapVector D_8013B240;
extern s32 D_80182DC0[];
extern s32 D_80182DF4;
extern s32 D_801B2DB0;
extern s32 D_801B2DB4;

    s32 remaining;
    s32 intensity;

    func_8006CFA8(D_80182DC0, &D_8013B240);
    func_800675F0(D_8011CF28, D_80139260 & 3, 4, 0x36, 0x7900, 0x1001, D_80182DF4, 0, 0, -1);
    D_80139260 += 1;
    intensity = D_80182DF4 + 2;
    D_80182DF4 = intensity;
    if (intensity >= 0x62)
    {
        D_80182DF4 = 0x61;
    }
    remaining = D_801B2DB4 - 1;
    D_8013B240.field_04 = (u16) (D_8013B240.field_04 + 0x14);
    D_801B2DB4 = remaining;
    if (remaining == 0)
    {
        D_801B2DB0 += 1;
    }
}

/** @brief Advance the world-map effect and its sequence state. */
void func_800A12B0(void)
{
/** @brief World-map vector; only the third halfword is changed here. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    s16 field_04;
    s16 field_06;
} WmapVector;

extern void func_800675F0(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 D_8011CF28;
extern s32 D_80139260;
extern WmapVector D_8013B240;
extern s32 D_80182DC0[];
extern s32 D_80182DF4;
extern s32 D_801B2DB0;
extern s32 D_801B2DB4;

    s32 remaining;
    s32 intensity;

    func_8006CFA8(D_80182DC0, &D_8013B240);
    func_800675F0(D_8011CF28, D_80139260 & 3, 4, 0x36, 0x7900, 0x1001, D_80182DF4, 0, 0, -1);
    intensity = D_80182DF4 - 4;
    D_80139260 += 1;
    D_80182DF4 = intensity;
    if (intensity < 0)
    {
        D_80182DF4 = 0;
    }
    remaining = D_801B2DB4 - 1;
    D_8013B240.field_04 = (u16) (D_8013B240.field_04 + 0x14);
    D_801B2DB4 = remaining;
    if (remaining == 0)
    {
        D_801B2DB0 += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800A1394(void)
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
extern s32 D_801B0FD0;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern u8 D_8011D538[];
extern s32 D_801B2DB8;
extern s32 D_801B2DBC;
extern void func_800A2DD0(void);

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
        D_80139988[i + 24].field_04 = D_8011D538;
    }
    D_801B2DBC = 276;
    D_801B2DB8++;
    func_800A2DD0();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800A146C(void)
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
extern u8 D_8011D538[];
extern s32 D_801B2DC0;
extern s32 D_801B2DC4;
extern void func_800A2FC8(void);

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
        D_80139988[i + 104].field_04 = D_8011D538;
    }
    D_801B2DC4 = 80;
    D_801B2DC0++;
    func_800A2FC8();
}

/** @brief World-map step handler: configure a model descriptor and clear two entry tables, then advance.
 *  @note Best match ~88% (gcc280_g0); residual is loop induction-variable register coloring. */
void func_800A1538(void)
{
/* Partial WMAP decompilation: 88.215680% (gcc280_g0). */

extern void func_800A31D0(void);
extern u8 *D_80139280;
extern u8 D_80139988;
extern u8 D_801AFBD0;
extern u8 D_8011D538;
extern s32 D_801B2DC8;
extern s32 D_801B2DCC;

    u8 *pa;
    u8 *pb;
    u8 *p;
    s32 i;
    s32 off_a;
    s32 off_b;

    p = D_80139280;
    *(s32 *)(p + 0x80) = 5;
    *(s32 *)(p + 0x84) = 0x20;
    *(s32 *)(p + 0x8C) = 8;
    *(s32 *)(p + 0x94) = 0x8C;
    *(s32 *)(p + 0x98) = 0x15;
    *(s32 *)(p + 0x7C) = 1;
    *(s32 *)(p + 0x88) = 0;
    *(s32 *)(p + 0x90) = 0;
    *(s32 *)(p + 0x9C) = 1;
    *(s32 *)(p + 0xA0) = 0x1F40;

    pa = &D_80139988;
    pb = &D_801AFBD0;
    i = 0;
    off_a = 0x480;
    off_b = 0xAF0;
    do
    {
        *(s16 *)(pb + off_b) = 0;
        *(s32 *)(pa + off_a + 4) = (s32)&D_8011D538;
        off_a += 8;
        off_b += 0x14;
        i += 1;
    } while (i < 0xA);

    D_801B2DCC = 0x50;
    D_801B2DC8 += 1;
    func_800A31D0();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A1604(s32 arg0)
{
extern u32 D_801B2D50;
extern s32 D_801B2D54;
extern void (*D_800D6904[])(void);

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
void func_800A167C(void)
{
extern u32 D_801B2D50;
extern s32 D_801B2D54;
extern void (*D_800D6904[])(void);

    D_801B2D50 = 1;
    D_801B2D54 = 1;
}

/** @brief World-map step: arm a timed callback, flag it active, then tick the sub-counter. */
void func_800A1694(void)
{
extern s32 D_80139978;
extern s16 D_8011CF4C[];
extern void func_800A1760(void);
extern s32 D_8013B20C;
extern s32 D_801B2D50;
extern void func_800A16FC(void);

    D_80139978 = 0x17;
    D_8011CF4C[0] = 0xA4;
    D_8011CF4C[1] = 0x69;
    func_8006CAC0(func_800A1760);
    D_8013B20C = 1;
    D_801B2D50 += 1;
    func_800A16FC();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A16FC(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2D50;
extern void func_800A1738(void);

    if (D_8013B20C == 0)
    {
        D_801B2D50 += 1;
        func_800A1738();
    }
}

/** @brief World-map trigger: set two flags and bump a counter. */
void func_800A1738(void)
{
extern s32 D_8013B294;
extern s32 D_80139228;
extern s32 D_801B2D50;

    D_8013B294 = 1;
    D_80139228 = 1;
    D_801B2D50 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A1760(s32 arg0)
{
extern u32 D_801B2D58;
extern s32 D_801B2D5C;
extern void (*D_800D6914[])(void);

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
void func_800A17D8(void)
{
extern u32 D_801B2D58;
extern s32 D_801B2D5C;
extern void (*D_800D6914[])(void);

    D_801B2D58 = 1;
    D_801B2D5C = 1;
}

/** @brief World-map step handler: kick two jobs and advance the step. */
void func_800A17F0(void)
{
extern s32 D_8013B208;
extern s32 D_801ADAF4;
extern s32 D_801B2D58;
extern s32 D_801B2D5C;

    D_8013B208 = 1;
    func_8006683C(0x601040);
    D_801ADAF4 = 8;
    func_800652A8(0x2C, 0x80);
    D_801B2D5C = 0xF;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A1850(void)
{
extern s32 D_801B2D5C;
extern s32 D_801B2D58;
extern void func_800A2F38(void);

    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800A1884(void)
{
extern s32 D_801B2D5C;
extern s32 D_801B2D58;
extern void func_800A2F38(void);

    func_8006CAC0(func_800A2F38);
    D_801B2D5C = 0x14;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A18C0(void)
{
extern s32 D_801B2D5C;
extern s32 D_801B2D58;
extern void func_800A24A8(void);
extern void func_800A2D40(void);

    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800A18F4(void)
{
extern s32 D_801B2D5C;
extern s32 D_801B2D58;
extern void func_800A24A8(void);
extern void func_800A2D40(void);

    func_8006CAC0(func_800A24A8);
    func_8006CAC0(func_800A2D40);
    D_801B2D5C = 0x78;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A193C(void)
{
extern s32 D_801B2D5C;
extern s32 D_801B2D58;

    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/** @brief World-map step: register a callback, set flags, advance the step. */
void func_800A1970(void)
{
extern s32 D_80139244;
extern s32 D_801ADAF4;
extern s32 D_801B2D58;
extern s32 D_801B2D5C;
extern void func_800A1F00(void);

    func_8006CAC0(func_800A1F00);
    D_80139244 = 1;
    func_8006683C(0x351040);
    D_801ADAF4 = 3;
    D_801B2D5C = 2;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A19D0(void)
{
extern s32 D_801B2D5C;
extern s32 D_801B2D58;

    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/** @brief World-map step handler: register three callbacks, reset state, advance the step. */
void func_800A1A04(void)
{
extern void func_800A2A80(void);
extern void func_800A25E4(void);
extern void func_800A1D64(void);
extern s32 D_800DBE70;
extern s32 D_80139978;
extern s32 D_801B2D58;
extern s32 D_801B2D5C;

    func_8006CAC0(func_800A2A80);
    func_8006CAC0(func_800A25E4);
    func_8006CAC0(func_800A1D64);
    D_800DBE70 = 0;
    D_80139978 = -1;
    D_801B2D5C = 0xF;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A1A6C(void)
{
extern s32 D_801B2D5C;
extern s32 D_801B2D58;
extern void func_800A2924(void);

    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800A1AA0(void)
{
extern s32 D_801B2D5C;
extern s32 D_801B2D58;
extern void func_800A2924(void);

    func_8006CAC0(func_800A2924);
    D_801B2D5C = 0x2D;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A1ADC(void)
{
extern s32 D_801B2D5C;
extern s32 D_801B2D58;
extern void func_800A2BE4(void);

    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800A1B10(void)
{
extern s32 D_801B2D5C;
extern s32 D_801B2D58;
extern void func_800A2BE4(void);

    func_8006CAC0(func_800A2BE4);
    D_801B2D5C = 0x78;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A1B4C(void)
{
extern s32 D_801B2D5C;
extern s32 D_801B2D58;

    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/** @brief Register two callbacks around a color update and begin a 136-tick delay. */
void func_800A1B80(void)
{
extern s32 D_80139244;
extern s32 D_801ADAF4;
extern s32 D_801B2D58;
extern s32 D_801B2D5C;
extern void func_800A2058(void);
extern void func_800A3140(void);

    func_8006CAC0(&func_800A2058);
    D_80139244 = 0;
    func_8006683C(0x601550);
    D_801ADAF4 = 8;
    func_8006CAC0(&func_800A3140);
    D_801B2D5C = 0x88;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A1BE8(void)
{
extern s32 D_801B2D5C;
extern s32 D_801B2D58;
extern void func_800A21B0(void);
extern void func_800A2308(void);

    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800A1C1C(void)
{
extern s32 D_801B2D5C;
extern s32 D_801B2D58;
extern void func_800A21B0(void);
extern void func_800A2308(void);

    func_8006CAC0(func_800A21B0);
    func_8006CAC0(func_800A2308);
    D_801B2D5C = 0x5;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A1C64(void)
{
extern s32 D_801B2D5C;
extern s32 D_801B2D58;
extern void func_800A2784(void);

    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800A1C98(void)
{
extern s32 D_801B2D5C;
extern s32 D_801B2D58;
extern void func_800A2784(void);

    func_8006CAC0(func_800A2784);
    D_801B2D5C = 0x84;
    D_801B2D58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A1CD4(void)
{
extern s32 D_801B2D5C;
extern s32 D_801B2D58;

    if (--D_801B2D5C == 0)
    {
        D_801B2D58 += 1;
    }
}

/** @brief Set the selected record value, clear the flag, and advance the sequence. */
void func_800A1D08(void)
{
extern s32 D_8011D510;
extern s32 D_8011D530;
/** @brief Record with a leading value and a 40-byte stride. */
typedef struct
{
    s32 value;
    u8 unknown_4[36];
} WmapValueRecord;

extern WmapValueRecord D_80139290[][6];
extern s32 D_8013B20C;
extern s32 D_801B2D58;

    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = 0x110;
    D_801B2D58 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A1D64(s32 arg0)
{
extern u32 D_801B2D60;
extern s32 D_801B2D64;
extern void (*D_800D696C[])(void);

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
void func_800A1DDC(void)
{
extern u32 D_801B2D60;
extern s32 D_801B2D64;
extern void (*D_800D696C[])(void);

    D_801B2D60 = 1;
    D_801B2D64 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void func_800A1DF4(void)
{
extern u8* D_801399B4;
extern u8 D_8011F538[];
extern u8 D_800D9344[];
extern s32 D_801B2D60;
extern s32 D_801B2D64;
extern void func_800A1E6C(void);

    D_801399B4 = D_8011F538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x26] = 0;
    *(s16*)&D_800D9344[0x22] = 0x7F;
    *(s16*)&D_800D9344[0x24] = 0x7F;
    D_801B2D64 = 0x159;
    D_801B2D60 += 1;
    func_800A1E6C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800A1E6C(void)
{
extern s32 D_801B2D60;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;
extern s32 D_801B2D64;

    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x16, 0xB, 0);
    if (--D_801B2D64 == 0)
    {
        D_801B2D60 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A1EE8(void)
{
extern s32 D_801B2D60;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;
extern s32 D_801B2D64;

    D_801B2D60 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A1F00(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2D68;
extern s32 D_801B2D6C;
extern void (*D_800D697C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_800A0838(void);

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
void func_800A1F78(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2D68;
extern s32 D_801B2D6C;
extern void (*D_800D697C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_800A0838(void);

    D_801B2D68 = 1;
    D_801B2D6C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_800A1F90(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2D68;
extern s32 D_801B2D6C;
extern void (*D_800D697C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_800A0838(void);

    D_801B24A0 = D_80139258;
    D_801B2650 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B2D6C = 0x40;
    D_801B2D68 += 1;
    func_800A0838();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A2040(void)
{
extern s32 D_801B2D68;

    D_801B2D68 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A2058(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2D70;
extern s32 D_801B2D74;
extern void (*D_800D698C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_800A0938(void);

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
void func_800A20D0(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2D70;
extern s32 D_801B2D74;
extern void (*D_800D698C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_800A0938(void);

    D_801B2D70 = 1;
    D_801B2D74 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_800A20E8(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2D70;
extern s32 D_801B2D74;
extern void (*D_800D698C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_800A0938(void);

    D_801B24A8 = D_80139258;
    D_801B2478 = D_80182DC0;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B2D74 = 0x40;
    D_801B2D70 += 1;
    func_800A0938();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A2198(void)
{
extern s32 D_801B2D70;

    D_801B2D70 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A21B0(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2D78;
extern s32 D_801B2D7C;
extern void (*D_800D699C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139870;
extern s32 D_80182DF0;
extern void func_800A0A38(void);

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
void func_800A2228(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2D78;
extern s32 D_801B2D7C;
extern void (*D_800D699C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139870;
extern s32 D_80182DF0;
extern void func_800A0A38(void);

    D_801B2D78 = 1;
    D_801B2D7C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_800A2240(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2D78;
extern s32 D_801B2D7C;
extern void (*D_800D699C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139870;
extern s32 D_80182DF0;
extern void func_800A0A38(void);

    D_8013B238 = D_80139258;
    D_80139870 = D_80182DC0;
    D_80182DF0 = 0x80;
    D_80139870.w[2] = 0xAFC8;
    D_801B2D7C = 0x40;
    D_801B2D78 += 1;
    func_800A0A38();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A22F0(void)
{
extern s32 D_801B2D78;

    D_801B2D78 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A2308(s32 arg0)
{
extern u32 D_801B2D80;
extern s32 D_801B2D84;
extern void (*D_800D69AC[])(void);
extern void func_800A2414(void);
extern u8 D_80121538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

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
void func_800A2380(void)
{
extern u32 D_801B2D80;
extern s32 D_801B2D84;
extern void (*D_800D69AC[])(void);
extern void func_800A2414(void);
extern u8 D_80121538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    D_801B2D80 = 1;
    D_801B2D84 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800A2398(void)
{
extern u32 D_801B2D80;
extern s32 D_801B2D84;
extern void (*D_800D69AC[])(void);
extern void func_800A2414(void);
extern u8 D_80121538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    D_801399BC = D_80121538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0x10] = -1;
    *(s16*)&D_800D9370[0x26] = 0x10;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0xE] = 0;
    *(s16*)&D_800D9370[0x22] = 0x80;
    *(s16*)&D_800D9370[0x24] = 0;
    D_801B2D84 = 0x8C;
    D_801B2D80 += 1;
    func_800A2414();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800A2414(void)
{
extern u32 D_801B2D80;
extern s32 D_801B2D84;
extern void (*D_800D69AC[])(void);
extern void func_800A2414(void);
extern u8 D_80121538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0x16, 0xA, 0);
    if (--D_801B2D84 == 0)
    {
        D_801B2D80 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A2490(void)
{
extern s32 D_801B2D80;

    D_801B2D80 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A24A8(s32 arg0)
{
extern u32 D_801B2D88;
extern s32 D_801B2D8C;
extern void (*D_800D69BC[])(void);
extern void func_800A0B38(void);
extern s32 D_80139268;

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
void func_800A2520(void)
{
extern u32 D_801B2D88;
extern s32 D_801B2D8C;
extern void (*D_800D69BC[])(void);
extern void func_800A0B38(void);
extern s32 D_80139268;

    D_801B2D88 = 1;
    D_801B2D8C = 1;
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A2538(void)
{
extern u32 D_801B2D88;
extern s32 D_801B2D8C;
extern void (*D_800D69BC[])(void);
extern void func_800A0B38(void);
extern s32 D_80139268;

    D_80139268 = 0;
    D_801B2D8C = 0xFA;
    D_801B2D88 += 1;
    func_800A0B38();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A2578(void)
{
extern void func_800A0C44(void);
extern s32 D_801B2D8C;
extern s32 D_801B2D88;

    D_801B2D8C = 0x40;
    D_801B2D88 += 1;
    func_800A0C44();
}

/**
 * @brief Reset the world-map cursor state and bump the transition counter.
 * @note Best match ~68.46% (gcc280_g0); residual is a sched2 lui-ordering tie
 *       (permuter territory).
 */
void func_800A25B0(void)
{
/* Partial WMAP decompilation: 68.461540% (gcc280_g0). */

extern s32 D_80182D48;
extern s16 D_801398C8;
extern s32 D_801B2D88;

    s32 *a;
    s16 *b;

    a = &D_80182D48;
    a[1] = 0;
    a[0] = 0;
    b = &D_801398C8;
    b[1] = 0;
    b[0] = 0;
    D_801B2D88 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A25E4(s32 arg0)
{
extern u32 D_801B2D90;
extern s32 D_801B2D94;
extern void (*D_800D69D4[])(void);
extern void func_800A26F0(void);
extern u8 D_80123538[];
extern u8* D_801399C4;
extern u8 D_800D939C[];

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
void func_800A265C(void)
{
extern u32 D_801B2D90;
extern s32 D_801B2D94;
extern void (*D_800D69D4[])(void);
extern void func_800A26F0(void);
extern u8 D_80123538[];
extern u8* D_801399C4;
extern u8 D_800D939C[];

    D_801B2D90 = 1;
    D_801B2D94 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800A2674(void)
{
extern u32 D_801B2D90;
extern s32 D_801B2D94;
extern void (*D_800D69D4[])(void);
extern void func_800A26F0(void);
extern u8 D_80123538[];
extern u8* D_801399C4;
extern u8 D_800D939C[];

    D_801399C4 = D_80123538;
    D_800D939C[0x6] = 0xF;
    *(s16*)&D_800D939C[0xE] = 1;
    *(s16*)&D_800D939C[0x10] = -1;
    *(s16*)&D_800D939C[0x2] = 0;
    *(s16*)&D_800D939C[0x26] = 0;
    *(s16*)&D_800D939C[0x22] = 0x7F;
    *(s16*)&D_800D939C[0x24] = 0x7F;
    D_801B2D94 = 0xBE;
    D_801B2D90 += 1;
    func_800A26F0();
}

/** @brief World-map step: init a sub-object then count down a timer. */
void func_800A26F0(void)
{
extern u8 D_800D939C[];
extern u8 D_801399C0[];
extern s32 D_8011CF4C;
extern s32 D_801B2D94;
extern s32 D_801B2D90;

    s32 n = 0x8;

    func_8006CC4C(D_800D939C, D_801399C0);
    func_80066F9C(D_800D939C, D_8011CF4C, n, n, 0);
    if (--D_801B2D94 == 0)
    {
        D_801B2D90 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A276C(void)
{
extern s32 D_801B2D90;

    D_801B2D90 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A2784(s32 arg0)
{
extern u32 D_801B2D98;
extern s32 D_801B2D9C;
extern void (*D_800D69E4[])(void);
extern void func_800A2890(void);
extern u8 D_80123538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];

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
void func_800A27FC(void)
{
extern u32 D_801B2D98;
extern s32 D_801B2D9C;
extern void (*D_800D69E4[])(void);
extern void func_800A2890(void);
extern u8 D_80123538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];

    D_801B2D98 = 1;
    D_801B2D9C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800A2814(void)
{
extern u32 D_801B2D98;
extern s32 D_801B2D9C;
extern void (*D_800D69E4[])(void);
extern void func_800A2890(void);
extern u8 D_80123538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];

    D_801399CC = D_80123538;
    D_800D93C8[0x6] = 0xF;
    *(s16*)&D_800D93C8[0x10] = -1;
    *(s16*)&D_800D93C8[0x26] = 8;
    *(s16*)&D_800D93C8[0x2] = 0;
    *(s16*)&D_800D93C8[0xE] = 0;
    *(s16*)&D_800D93C8[0x22] = 0;
    *(s16*)&D_800D93C8[0x24] = 0x81;
    D_801B2D9C = 0x10;
    D_801B2D98 += 1;
    func_800A2890();
}

/** @brief World-map step: init a sub-object then count down a timer. */
void func_800A2890(void)
{
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;
extern s32 D_801B2D9C;
extern s32 D_801B2D98;

    s32 n = 0x8;

    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_8011CF4C, n, n, 0);
    if (--D_801B2D9C == 0)
    {
        D_801B2D98 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A290C(void)
{
extern s32 D_801B2D98;

    D_801B2D98 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A2924(s32 arg0)
{
extern u32 D_801B2DA0;
extern s32 D_801B2DA4;
extern void (*D_800D69F4[])(void);

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
void func_800A299C(void)
{
extern u32 D_801B2DA0;
extern s32 D_801B2DA4;
extern void (*D_800D69F4[])(void);

    D_801B2DA0 = 1;
    D_801B2DA4 = 1;
}

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_800A29B4(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DE4;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2498;
extern s32 D_8013923C;
extern s32 D_801B2DA0;
extern s32 D_801B2DA4;
extern void func_800A0D80(void);

    D_80182DE4 = 1;
    D_801B2498 = D_80139258;
    D_8013923C = 0;
    D_801B2DA4 = 0x7C;
    D_801B2DA0 += 1;
    func_800A0D80();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A2A30(void)
{
extern s32 D_801B2DA0;
extern void func_800A0E84(void);
extern s32 D_801B2DA4;

    D_801B2DA4 = 0x40;
    D_801B2DA0 += 1;
    func_800A0E84();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A2A68(void)
{
extern s32 D_801B2DA0;
extern void func_800A0E84(void);
extern s32 D_801B2DA4;

    D_801B2DA0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A2A80(s32 arg0)
{
extern u32 D_801B2DA8;
extern s32 D_801B2DAC;
extern void (*D_800D6A0C[])(void);

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
void func_800A2AF8(void)
{
extern u32 D_801B2DA8;
extern s32 D_801B2DAC;
extern void (*D_800D6A0C[])(void);

    D_801B2DA8 = 1;
    D_801B2DAC = 1;
}

/** @brief Reset effect state and begin a 96-tick sequence step. */
void func_800A2B10(void)
{
/** @brief Eight bytes of world-map effect state copied together. */
typedef struct
{
    u8 bytes[8];
} WmapBlk8;

extern void func_800A0F84(void);
extern s32 D_80139234;
extern WmapBlk8 D_80139258;
extern s32 D_80139264;
extern s32 D_801B25D8;
extern WmapBlk8 D_801B2670;
extern s32 D_801B2DA8;
extern s32 D_801B2DAC;

    D_801B25D8 = 1;
    D_801B2670 = D_80139258;
    D_80139234 = 0;
    D_80139264 = 0;
    D_801B2DAC = 0x60;
    D_801B2DA8 += 1;
    func_800A0F84();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A2B94(void)
{
extern s32 D_801B2DA8;
extern void func_800A10B4(void);
extern s32 D_801B2DAC;

    D_801B2DAC = 0x10;
    D_801B2DA8 += 1;
    func_800A10B4();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A2BCC(void)
{
extern s32 D_801B2DA8;
extern void func_800A10B4(void);
extern s32 D_801B2DAC;

    D_801B2DA8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A2BE4(s32 arg0)
{
extern u32 D_801B2DB0;
extern s32 D_801B2DB4;
extern void (*D_800D6A24[])(void);

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
void func_800A2C5C(void)
{
extern u32 D_801B2DB0;
extern s32 D_801B2DB4;
extern void (*D_800D6A24[])(void);

    D_801B2DB0 = 1;
    D_801B2DB4 = 1;
}

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_800A2C74(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DF4;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern s32 D_80139260;
extern s32 D_801B2DB0;
extern s32 D_801B2DB4;
extern void func_800A11C4(void);

    D_80182DF4 = 1;
    D_8013B240 = D_80139258;
    D_80139260 = 0;
    D_801B2DB4 = 0x6C;
    D_801B2DB0 += 1;
    func_800A11C4();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A2CF0(void)
{
extern s32 D_801B2DB0;
extern void func_800A12B0(void);
extern s32 D_801B2DB4;

    D_801B2DB4 = 0x20;
    D_801B2DB0 += 1;
    func_800A12B0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A2D28(void)
{
extern s32 D_801B2DB0;
extern void func_800A12B0(void);
extern s32 D_801B2DB4;

    D_801B2DB0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A2D40(s32 arg0)
{
extern u32 D_801B2DB8;
extern s32 D_801B2DBC;
extern void (*D_800D6A3C[])(void);

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
void func_800A2DB8(void)
{
extern u32 D_801B2DB8;
extern s32 D_801B2DBC;
extern void (*D_800D6A3C[])(void);

    D_801B2DB8 = 1;
    D_801B2DBC = 1;
}

/**
 * @brief World-map step handler: submit a batched sprite draw, then advance the
 *        sequence once its frame counter expires.
 */
void func_800A2DD0(void)
{
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32* D_80139280;
extern s32 D_801B2DB8;
extern s32 D_801B2DBC;

    func_8006A2FC(D_800D9688, D_80139A48, 0x2E, 0x1, 0xFF, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2DBC == 0)
    {
        D_801B2DB8 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_800A2E54(void)
{
extern void func_800A2E9C(void);
extern s32* D_80139280;
extern s32 D_801B2DBC;
extern s32 D_801B2DB8;

    D_801B2DBC = 0x40;
    D_80139280[15] = -1;
    D_801B2DB8 += 1;
    func_800A2E9C();
}

/**
 * @brief World-map step handler: submit a batched sprite draw, then advance the
 *        sequence once its frame counter expires.
 */
void func_800A2E9C(void)
{
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32* D_80139280;
extern s32 D_801B2DB8;
extern s32 D_801B2DBC;

    func_8006A2FC(D_800D9688, D_80139A48, 0x2E, 0x1, 0xFF, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2DBC == 0)
    {
        D_801B2DB8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A2F20(void)
{
extern s32 D_801B2DB8;

    D_801B2DB8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A2F38(s32 arg0)
{
extern u32 D_801B2DC0;
extern s32 D_801B2DC4;
extern void (*D_800D6A54[])(void);
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32 D_80139280;

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
void func_800A2FB0(void)
{
extern u32 D_801B2DC0;
extern s32 D_801B2DC4;
extern void (*D_800D6A54[])(void);
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32 D_80139280;

    D_801B2DC0 = 1;
    D_801B2DC4 = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_800A2FC8(void)
{
extern u32 D_801B2DC0;
extern s32 D_801B2DC4;
extern void (*D_800D6A54[])(void);
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32 D_80139280;

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
void func_800A3054(void)
{
extern void func_800A309C(void);
extern s32* D_80139280;
extern s32 D_801B2DC4;
extern s32 D_801B2DC0;
extern u8 D_800DA448[];
extern u8 D_80139CC8[];

    D_801B2DC4 = 0x20;
    D_80139280[25] = -1;
    D_801B2DC0 += 1;
    func_800A309C();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_800A309C(void)
{
extern void func_800A309C(void);
extern s32* D_80139280;
extern s32 D_801B2DC4;
extern s32 D_801B2DC0;
extern u8 D_800DA448[];
extern u8 D_80139CC8[];

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
void func_800A3128(void)
{
extern s32 D_801B2DC0;

    D_801B2DC0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A3140(s32 arg0)
{
extern u32 D_801B2DC8;
extern s32 D_801B2DCC;
extern void (*D_800D6A6C[])(void);
extern u8 D_800DAB28[];
extern u8 D_80139E08[];
extern s32 D_80139280;

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
void func_800A31B8(void)
{
extern u32 D_801B2DC8;
extern s32 D_801B2DCC;
extern void (*D_800D6A6C[])(void);
extern u8 D_800DAB28[];
extern u8 D_80139E08[];
extern s32 D_80139280;

    D_801B2DC8 = 1;
    D_801B2DCC = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_800A31D0(void)
{
extern u32 D_801B2DC8;
extern s32 D_801B2DCC;
extern void (*D_800D6A6C[])(void);
extern u8 D_800DAB28[];
extern u8 D_80139E08[];
extern s32 D_80139280;

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
void func_800A325C(void)
{
extern void func_800A32A4(void);
extern s32* D_80139280;
extern s32 D_801B2DCC;
extern s32 D_801B2DC8;
extern u8 D_800DAB28[];
extern u8 D_80139E08[];

    D_801B2DCC = 0x20;
    D_80139280[35] = -1;
    D_801B2DC8 += 1;
    func_800A32A4();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_800A32A4(void)
{
extern void func_800A32A4(void);
extern s32* D_80139280;
extern s32 D_801B2DCC;
extern s32 D_801B2DC8;
extern u8 D_800DAB28[];
extern u8 D_80139E08[];

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
void func_800A3330(void)
{
extern s32 D_801B2DC8;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2DF4;
extern s32 D_801B2DF0;

    D_801B2DC8 += 1;
}
