#include "wmap_land_effect_05.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

/** @brief Initialize the actor group and its animation resources, then advance. */
void func_8008C7CC(void)
{
/* Partial WMAP decompilation: 98.935486% (gcc280_g0). */

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

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 field_00;
    s16 angle;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    s16 field_10;
    s16 pad_12;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *data;
} WmapResource;

extern WmapConfigA D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern u8 D_80121538[];
extern s32 D_801B0FD0;
extern s32 D_801B2A28;
extern s32 D_801B2A2C;
extern void func_8008DFD0(void);

    s32 i;

    D_801B0FD0 = 5;
    for (i = 200; i < 205; i++)
    {
        D_80139988[i].data = D_80121538;
        D_800D9268[i].field_02 = 0;
        D_800D9268[i].field_06 = 15;
        D_800D9268[i].field_0E = 0;
        D_800D9268[i].field_10 = -1;
        D_800D9268[i].field_22 = 63;
        D_800D9268[i].field_24 = 2;
        D_800D9268[i].field_26 = 2;
        D_801AFBD0[i].field_00 = 1;
        D_801AFBD0[i].angle = i * 0x333;
        D_801AFBD0[i].field_04 = 0;
        D_801AFBD0[i].field_08 = 340000;
        D_801AFBD0[i].field_0C = 9999;
        D_801AFBD0[i].field_0E = 0;
        D_801AFBD0[i].field_10 = 80;
    }
    D_801B2A2C = 180;
    D_801B2A28++;
    func_8008DFD0();
}

/** @brief Draw two oscillating effect layers and update their intensity. */
void func_8008C8C4(void)

{
/* Partial WMAP decompilation: 87.588780% (gcc280_g0). */

extern s8 D_80051B4C[];
extern void *D_8011CF24;
extern VECTOR D_80182DC0;
extern s32 D_80182DE4;
extern s32 D_801B2468;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B24B4;
extern s32 D_801B2A30;
extern s32 D_801B2A34;

    s32 first_frame;
    SVECTOR *rotation;
    s32 second_frame;
    s32 intensity;
    s32 remaining;

    PushMatrix();
    rotation = &D_801B2490;
    first_frame = (s32) (D_80051B4C[D_801B24B4] + 0x80) >> 5;
    func_8006CFA8(&D_80182DC0, rotation);
    func_8006CD98(D_8011CF24, first_frame, 20, 0x35, 0x7800, 4096, D_801B2468);
    rotation->vz = (u16) (rotation->vz - 0x18);
    rotation = &D_801B2498;
    second_frame = (s32) (D_80051B4C[D_80182DE4] + 0x80) >> 5;
    func_8006CFA8(&D_80182DC0, rotation);
    func_8006CD98(D_8011CF24, second_frame, 20, 0x35, 0x7800, 4096, D_801B2468);
    rotation->vz = (u16) (rotation->vz + 0x30);
    PopMatrix();
    D_801B24B4 = (D_801B24B4 + 4) & 0xFF;
    D_80182DE4 = (D_80182DE4 + 2) & 0xFF;
    intensity = D_801B2468 + 1;
    D_801B2468 = intensity;
    if (intensity >= 0x81)
    {
        D_801B2468 = 0x80;
    }
    remaining = D_801B2A34 - 1;
    D_801B2A34 = remaining;
    if (remaining == 0)
    {
        D_801B2A30 += 1;
    }
}

/** @brief Animate and fade two counter-rotating effect layers. */
void func_8008CA70(void)
{
extern s8 D_80051B4C[];
extern void *D_8011CF24;
extern VECTOR D_80182DC0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_80182DE4;
extern s32 D_801B2468;
extern s32 D_801B24B4;
extern s32 D_801B2A30;
extern s32 D_801B2A34;

    s32 first_frame;
    
    s32 intensity;
    s32 remaining;

    PushMatrix();
    first_frame = (s32) (D_80051B4C[D_801B24B4] + 0x80) >> 5;
    func_8006CFA8(&D_80182DC0, &D_801B2490);
    func_8006CD98(D_8011CF24, first_frame, 0x14, 0x35, 0x7800, 0x1000, D_801B2468);
    first_frame = (s32) (D_80051B4C[D_80182DE4] + 0x80) >> 5;
    D_801B2490.vz = (u16) (D_801B2490.vz - 0x18);
    func_8006CFA8(&D_80182DC0, &D_801B2498);
    func_8006CD98(D_8011CF24, first_frame, 0x14, 0x35, 0x7800, 0x1000, D_801B2468);
    D_801B2498.vz = (u16) (D_801B2498.vz + 0x30);
    PopMatrix();
    D_801B24B4 = (D_801B24B4 + 4) & 0xFF;
    D_80182DE4 = (D_80182DE4 + 2) & 0xFF;
    intensity = D_801B2468 - 4;
    D_801B2468 = intensity;
    if (intensity < 0)
    {
        D_801B2468 = 0;
    }
    remaining = D_801B2A34 - 1;
    D_801B2A34 = remaining;
    if (remaining == 0)
    {
        D_801B2A30 += 1;
    }
}

/**
 * @brief World-map step handler: decay a timer with a floor, draw the model, ramp a
 *        secondary counter to its cap, scroll the field, then countdown-advance the step.
 */
void func_8008CC14(void)
{
extern s32 D_80139888[];
extern u8 D_8013B240[];
extern s32 D_8011CF28;
extern s32 D_80182DF4;
extern s32 D_801B2A38;
extern s32 D_801B2A3C;

    s32 v;

    v = D_80139888[2] - 0x5DC;
    D_80139888[2] = v;
    if (v < 0x7530)
    {
        D_80139888[2] = 0x7530;
    }
    PushMatrix();
    func_8006CFA8(D_80139888, D_8013B240);
    func_8006CD98(D_8011CF28, 0, 0xC, 0x35, 0x7840, 1, D_80182DF4);
    D_80182DF4 += 1;
    if (D_80182DF4 >= 0x42)
    {
        D_80182DF4 = 0x41;
    }
    ((u16*)D_8013B240)[2] += 2;
    PopMatrix();
    if (--D_801B2A3C == 0)
    {
        D_801B2A38 += 1;
    }
}

/**
 * @brief World-map step handler: clamp a fade level, draw a sprite, and expire the step.
 */
void func_8008CD10(void)
{
extern u8 D_80139888[];
extern u8 D_8013B240[];
extern s32 D_80182DF4;
extern s32 D_8011CF28;
extern s32 D_801B2A38;
extern s32 D_801B2A3C;

    u8 *p;
    u8 *q;

    p = D_80139888;
    if (*(s32 *)(p + 8) < 0x7530)
    {
        *(s32 *)(p + 8) = 0x7530;
    }
    PushMatrix();
    q = D_8013B240;
    func_8006CFA8(p, q);
    if (D_80182DF4 != 0)
    {
        func_8006CD98(D_8011CF28, 0, 0xC, 0x35, 0x7840, 1, D_80182DF4);
        D_80182DF4 -= 1;
        if (D_80182DF4 < 0)
        {
            D_80182DF4 = 0;
        }
        *(s16 *)(q + 4) += 2;
    }
    PopMatrix();
    if (--D_801B2A3C == 0)
    {
        D_801B2A38 += 1;
    }
}

/**
 * @brief World-map step handler: decay a timer with a floor, draw the model, ramp a
 *        secondary counter to its cap, scroll the field, then countdown-advance the step.
 */
void func_8008CE08(void)
{
extern s32 D_80139898[];
extern u8 D_801B2670[];
extern s32 D_8011CF2C;
extern s32 D_801B25D8;
extern s32 D_801B2A40;
extern s32 D_801B2A44;

    s32 v;

    v = D_80139898[2] - 0x5DC;
    D_80139898[2] = v;
    if (v < 0x7530)
    {
        D_80139898[2] = 0x7530;
    }
    PushMatrix();
    func_8006CFA8(D_80139898, D_801B2670);
    func_8006CD98(D_8011CF2C, 0, 0xC, 0x35, 0x7840, 1, D_801B25D8);
    D_801B25D8 += 1;
    if (D_801B25D8 >= 0x42)
    {
        D_801B25D8 = 0x41;
    }
    ((u16*)D_801B2670)[2] -= 3;
    PopMatrix();
    if (--D_801B2A44 == 0)
    {
        D_801B2A40 += 1;
    }
}

/**
 * @brief World-map step handler: clamp a fade level, draw a sprite, and expire the step.
 */
void func_8008CF04(void)
{
extern u8 D_80139898[];
extern u8 D_801B2670[];
extern s32 D_801B25D8;
extern s32 D_8011CF2C;
extern s32 D_801B2A40;
extern s32 D_801B2A44;

    u8 *p;
    u8 *q;

    p = D_80139898;
    if (*(s32 *)(p + 8) < 0x7530)
    {
        *(s32 *)(p + 8) = 0x7530;
    }
    PushMatrix();
    q = D_801B2670;
    func_8006CFA8(p, q);
    if (D_801B25D8 != 0)
    {
        func_8006CD98(D_8011CF2C, 0, 0xC, 0x35, 0x7840, 1, D_801B25D8);
        D_801B25D8 -= 1;
        if (D_801B25D8 < 0)
        {
            D_801B25D8 = 0;
        }
        *(s16 *)(q + 4) += -3;
    }
    PopMatrix();
    if (--D_801B2A44 == 0)
    {
        D_801B2A40 += 1;
    }
}

/**
 * @brief World-map step handler: decay a timer with a floor, draw the model, ramp a
 *        secondary counter to its cap, scroll the field, then countdown-advance the step.
 */
void func_8008CFFC(void)
{
extern s32 D_801B2660[];
extern u8 D_801B2678[];
extern s32 D_8011CF30;
extern s32 D_801B25DC;
extern s32 D_801B2A48;
extern s32 D_801B2A4C;

    s32 v;

    v = D_801B2660[2] - 0x5DC;
    D_801B2660[2] = v;
    if (v < 0x7530)
    {
        D_801B2660[2] = 0x7530;
    }
    PushMatrix();
    func_8006CFA8(D_801B2660, D_801B2678);
    func_8006CD98(D_8011CF30, 0, 0xC, 0x35, 0x7840, 1, D_801B25DC);
    D_801B25DC += 1;
    if (D_801B25DC >= 0x42)
    {
        D_801B25DC = 0x41;
    }
    ((u16*)D_801B2678)[2] += 4;
    PopMatrix();
    if (--D_801B2A4C == 0)
    {
        D_801B2A48 += 1;
    }
}

/** @brief Draw the fading effect and advance its countdown. */
void func_8008D0F8(void)
{
extern void *D_8011CF30;
extern VECTOR D_801B2660;
extern SVECTOR D_801B2678;
extern s32 D_801B25DC;
extern s32 D_801B2A48;
extern s32 D_801B2A4C;

    s32 intensity;
    s32 remaining;

    if (D_801B2660.vz < 0x7530)
    {
        D_801B2660.vz = 0x7530;
    }
    PushMatrix();
    func_8006CFA8(&D_801B2660, &D_801B2678);
    if (D_801B25DC != 0)
    {
        func_8006CD98(D_8011CF30, 0, 0xC, 0x35, 0x7840, 1, D_801B25DC);
        intensity = D_801B25DC - 1;
        D_801B25DC = intensity;
        if (intensity < 0)
        {
            D_801B25DC = 0;
        }
        D_801B2678.vz = (u16) (D_801B2678.vz + 4);
    }
    PopMatrix();
    remaining = D_801B2A4C - 1;
    D_801B2A4C = remaining;
    if (remaining == 0)
    {
        D_801B2A48 += 1;
    }
}

/** @brief Draw the fading effect and advance its countdown. */
void func_8008D1F0(void)
{
extern u8 D_800DCF18[];
extern VECTOR D_8011CF60;
extern VECTOR D_801B2650;
extern SVECTOR D_801B24A0;
extern s32 D_80182DE8;
extern s32 D_801B2A50;
extern s32 D_801B2A54;

    MATRIX transform;
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = D_801B2650.vz - 0xDAC;
    D_801B2650.vz = depth;
    if (depth < 0x2710)
    {
        D_801B2650.vz = 0x2710;
    }
    PushMatrix();
    RotMatrix(&D_801B24A0, &transform);
    TransMatrix(&transform, &D_8011CF60);
    SetRotMatrix(&transform);
    SetTransMatrix(&transform);
    if (D_80182DE8 != 0)
    {
        func_8006CD98(D_800DCF18, 0, 4, 0x35, 0x7800, 1, D_80182DE8);
        intensity = D_80182DE8 - 2;
        D_80182DE8 = intensity;
        if (intensity < 0)
        {
            D_80182DE8 = 0;
        }
    }
    PopMatrix();
    remaining = D_801B2A54 - 1;
    D_801B2A54 = remaining;
    if (remaining == 0)
    {
        D_801B2A50 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8008D2F0(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B2A5C;
extern s32 D_801B2A58;

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
    if (--D_801B2A5C == 0)
    {
        D_801B2A58 += 1;
    }
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void func_8008D3F0(void)
{
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

typedef struct
{
    s32 unk0;
    void *unk4;
} WmapPair;

typedef struct
{
    s16 unk0;
    s16 unk2;
    s32 unk4;
    s32 unk8;
    s16 unkC;
    s16 unkE;
    s32 unk10;
} WmapAfcEntry;

extern WmapD94Entry D_800D9268[];
extern s32 D_80121538[];
extern WmapPair D_80139988[];
extern WmapAfcEntry D_801AFBD0[];
extern s32 D_800D9150;
extern s32 D_800DCEA8;
extern s32 D_80182DF0;
extern s32 D_801B2A60;
extern s32 D_801B2A64;

extern void func_8008EA58(void);

    s32 i;
    WmapD94Entry *entry;

    i = 100;
    D_80182DF0 = 1;
    D_800DCEA8 = 1;

    do
    {
        D_801AFBD0[i].unk0 = 0;
        D_80139988[i].unk4 = D_80121538;
        entry = &D_800D9268[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 1;
        entry->unk10 = -1;
        i++;
    } while (i < 200);

    D_800D9150 = 1;
    D_801B2A64 = 0x10;
    D_801B2A60 += 1;
    func_8008EA58();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008D4B4(s32 arg0)
{
extern u32 D_801B2A08;
extern s32 D_801B2A0C;
extern void (*D_800D5D98[])(void);
extern void func_8008D588(void);
extern s32 D_8013B20C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A08 = 1;
        D_801B2A0C = 1;
        return 1;
    }

    if (D_801B2A08 < 0x6)
    {
        D_800D5D98[D_801B2A08]();
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
void func_8008D52C(void)
{
extern u32 D_801B2A08;
extern s32 D_801B2A0C;
extern void (*D_800D5D98[])(void);
extern void func_8008D588(void);
extern s32 D_8013B20C;

    D_801B2A08 = 1;
    D_801B2A0C = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8008D544(void)
{
extern u32 D_801B2A08;
extern s32 D_801B2A0C;
extern void (*D_800D5D98[])(void);
extern void func_8008D588(void);
extern s32 D_8013B20C;

    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2A08 += 1;
    func_8008D588();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8008D588(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2A08;
extern void func_8008D5C4(void);
extern void func_8008D65C(void);
extern void func_8008D608(void);

    if (D_8013B20C == 0)
    {
        D_801B2A08 += 1;
        func_8008D5C4();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8008D5C4(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2A08;
extern void func_8008D5C4(void);
extern void func_8008D65C(void);
extern void func_8008D608(void);

    func_8006CAC0(func_8008D65C);
    D_8013B20C = 1;
    D_801B2A08 += 1;
    func_8008D608();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8008D608(void)
{
extern s32 D_801B2A08;
extern s32 D_8013B20C;
extern void func_8008D644(void);

    if (D_8013B20C == 0)
    {
        D_801B2A08 += 1;
        func_8008D644();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008D644(void)
{
extern s32 D_801B2A08;
extern s32 D_8013B20C;
extern void func_8008D644(void);

    D_801B2A08 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008D65C(s32 arg0)
{
extern u32 D_801B2A10;
extern s32 D_801B2A14;
extern void (*D_800D5DB0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A10 = 1;
        D_801B2A14 = 1;
        return 1;
    }

    if (D_801B2A10 < 0x16)
    {
        D_800D5DB0[D_801B2A10]();
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
void func_8008D6D4(void)
{
extern u32 D_801B2A10;
extern s32 D_801B2A14;
extern void (*D_800D5DB0[])(void);

    D_801B2A10 = 1;
    D_801B2A14 = 1;
}

/** @brief World-map step: mark active, request a resource, seed the sub-counter, tick. */
void func_8008D6EC(void)
{
extern s32 D_8013B208;
extern s32 D_801B2A14;
extern s32 D_801B2A10;

    D_8013B208 = 1;
    func_800652A8(0x25, 0x80);
    D_801B2A14 = 8;
    D_801B2A10 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008D734(void)
{
extern s32 D_801B2A14;
extern s32 D_801B2A10;
extern void func_8008DF40(void);

    if (--D_801B2A14 == 0)
    {
        D_801B2A10 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008D768(void)
{
extern s32 D_801B2A14;
extern s32 D_801B2A10;
extern void func_8008DF40(void);

    func_8006CAC0(func_8008DF40);
    D_801B2A14 = 0x1E;
    D_801B2A10 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008D7A4(void)
{
extern s32 D_801B2A14;
extern s32 D_801B2A10;

    if (--D_801B2A14 == 0)
    {
        D_801B2A10 += 1;
    }
}

/** @brief Register two callbacks around a color and state update and begin a two-tick delay. */
void func_8008D7D8(void)
{
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern s32 D_801B2A10;
extern s32 D_801B2A14;
extern void func_8008DBFC(void);
extern void func_8008E718(void);

    func_8006CAC0(&func_8008E718);
    func_8006683C(0x501040);
    D_801ADAF4 = 4;
    D_801ADAE0 = 1;
    func_8006CAC0(&func_8008DBFC);
    D_801B2A14 = 2;
    D_801B2A10 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008D844(void)
{
extern s32 D_801B2A14;
extern s32 D_801B2A10;

    if (--D_801B2A14 == 0)
    {
        D_801B2A10 += 1;
    }
}

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void func_8008D878(void)
{
extern s32 D_80139244;
extern s32 D_801B2A10;
extern s32 D_801B2A14;

    D_80139244 = 1;
    D_801B2A14 = 0x16;
    D_801B2A10 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008D8A4(void)
{
extern s32 D_801B2A14;
extern s32 D_801B2A10;
extern void func_8008E10C(void);
extern void func_8008E268(void);

    if (--D_801B2A14 == 0)
    {
        D_801B2A10 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8008D8D8(void)
{
extern s32 D_801B2A14;
extern s32 D_801B2A10;
extern void func_8008E10C(void);
extern void func_8008E268(void);

    func_8006CAC0(func_8008E10C);
    func_8006CAC0(func_8008E268);
    D_801B2A14 = 0x4;
    D_801B2A10 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008D920(void)
{
extern s32 D_801B2A14;
extern s32 D_801B2A10;
extern void func_8008E3F8(void);

    if (--D_801B2A14 == 0)
    {
        D_801B2A10 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008D954(void)
{
extern s32 D_801B2A14;
extern s32 D_801B2A10;
extern void func_8008E3F8(void);

    func_8006CAC0(func_8008E3F8);
    D_801B2A14 = 0x2;
    D_801B2A10 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008D990(void)
{
extern s32 D_801B2A14;
extern s32 D_801B2A10;
extern void func_8008E588(void);

    if (--D_801B2A14 == 0)
    {
        D_801B2A10 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008D9C4(void)
{
extern s32 D_801B2A14;
extern s32 D_801B2A10;
extern void func_8008E588(void);

    func_8006CAC0(func_8008E588);
    D_801B2A14 = 0x2;
    D_801B2A10 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008DA00(void)
{
extern s32 D_801B2A14;
extern s32 D_801B2A10;
extern void func_8008E9C8(void);

    if (--D_801B2A14 == 0)
    {
        D_801B2A10 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008DA34(void)
{
extern s32 D_801B2A14;
extern s32 D_801B2A10;
extern void func_8008E9C8(void);

    func_8006CAC0(func_8008E9C8);
    D_801B2A14 = 0x74;
    D_801B2A10 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008DA70(void)
{
extern s32 D_801B2A14;
extern s32 D_801B2A10;
extern void func_8008E870(void);
extern void func_8008DDA0(void);

    if (--D_801B2A14 == 0)
    {
        D_801B2A10 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8008DAA4(void)
{
extern s32 D_801B2A14;
extern s32 D_801B2A10;
extern void func_8008E870(void);
extern void func_8008DDA0(void);

    func_8006CAC0(func_8008E870);
    func_8006CAC0(func_8008DDA0);
    D_801B2A14 = 0x2;
    D_801B2A10 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008DAEC(void)
{
extern s32 D_801B2A14;
extern s32 D_801B2A10;

    if (--D_801B2A14 == 0)
    {
        D_801B2A10 += 1;
    }
}

/** @brief Clear the world-map value, set the drawing color, and start a 140-tick delay. */
void func_8008DB20(void)
{
extern s32 D_80139244;
extern s32 D_801B2A10;
extern s32 D_801B2A14;

    D_80139244 = 0;
    func_8006683C(0x403060);
    D_801B2A14 = 0x8C;
    D_801B2A10 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008DB64(void)
{
extern s32 D_801B2A14;
extern s32 D_801B2A10;

    if (--D_801B2A14 == 0)
    {
        D_801B2A10 += 1;
    }
}

void func_8008DB98(void)
{
/* Partial WMAP decompilation: 95.600000% (gcc280_g0). */

extern s32 D_8013B20C;
extern u32 D_80139290[];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2A10;

    D_8013B20C = 0;
    D_80139290[D_8011D530 * 10 + D_8011D510 * 60] = D_8011D4FC | 0x100;
    D_801B2A10 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008DBFC(s32 arg0)
{
extern u32 D_801B2A18;
extern s32 D_801B2A1C;
extern void (*D_800D5E08[])(void);
extern void func_8008DD0C(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A18 = 1;
        D_801B2A1C = 1;
        return 1;
    }

    if (D_801B2A18 < 0x4)
    {
        D_800D5E08[D_801B2A18]();
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
void func_8008DC74(void)
{
extern u32 D_801B2A18;
extern s32 D_801B2A1C;
extern void (*D_800D5E08[])(void);
extern void func_8008DD0C(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    D_801B2A18 = 1;
    D_801B2A1C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8008DC8C(void)
{
extern u32 D_801B2A18;
extern s32 D_801B2A1C;
extern void (*D_800D5E08[])(void);
extern void func_8008DD0C(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 8;
    *(s16*)&D_800D9318[0x22] = 0x81;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x24] = 1;
    D_801B2A1C = 0x96;
    D_801B2A18 += 1;
    func_8008DD0C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008DD0C(void)
{
extern u32 D_801B2A18;
extern s32 D_801B2A1C;
extern void (*D_800D5E08[])(void);
extern void func_8008DD0C(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0xF, 0x6, 0);
    if (--D_801B2A1C == 0)
    {
        D_801B2A18 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008DD88(void)
{
extern s32 D_801B2A18;

    D_801B2A18 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008DDA0(s32 arg0)
{
extern u32 D_801B2A20;
extern s32 D_801B2A24;
extern void (*D_800D5E18[])(void);
extern void func_8008DEAC(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A20 = 1;
        D_801B2A24 = 1;
        return 1;
    }

    if (D_801B2A20 < 0x4)
    {
        D_800D5E18[D_801B2A20]();
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
void func_8008DE18(void)
{
extern u32 D_801B2A20;
extern s32 D_801B2A24;
extern void (*D_800D5E18[])(void);
extern void func_8008DEAC(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801B2A20 = 1;
    D_801B2A24 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8008DE30(void)
{
extern u32 D_801B2A20;
extern s32 D_801B2A24;
extern void (*D_800D5E18[])(void);
extern void func_8008DEAC(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801399B4 = D_8011F538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 8;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x22] = 0x80;
    *(s16*)&D_800D9344[0x24] = 0;
    D_801B2A24 = 0x90;
    D_801B2A20 += 1;
    func_8008DEAC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008DEAC(void)
{
extern u32 D_801B2A20;
extern s32 D_801B2A24;
extern void (*D_800D5E18[])(void);
extern void func_8008DEAC(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x18, 0xA, 0);
    if (--D_801B2A24 == 0)
    {
        D_801B2A20 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008DF28(void)
{
extern s32 D_801B2A20;

    D_801B2A20 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008DF40(s32 arg0)
{
extern u32 D_801B2A28;
extern s32 D_801B2A2C;
extern void (*D_800D5E28[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A28 = 1;
        D_801B2A2C = 1;
        return 1;
    }

    if (D_801B2A28 < 0x6)
    {
        D_800D5E28[D_801B2A28]();
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
void func_8008DFB8(void)
{
extern u32 D_801B2A28;
extern s32 D_801B2A2C;
extern void (*D_800D5E28[])(void);

    D_801B2A28 = 1;
    D_801B2A2C = 1;
}

/**
 * @brief Draw the world-map cursor via func_8006B6EC, then advance after the wait expires.
 */
void func_8008DFD0(void)
{
extern s32 D_801B2A28;
extern s32 D_801B2A2C;

    func_8006B6EC(0xC8, 0xCD, 0xF, 0, 0x6);
    if (--D_801B2A2C == 0)
    {
        D_801B2A28 += 1;
    }
}

/**
 * @brief Reset a range of world-map actor configs, arm the timer, and advance the step.
 */
void func_8008E030(void)
{
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

extern WmapConfigA D_800D9268[];
extern s32 D_801B2A28;
extern s32 D_801B2A2C;
extern void func_8008E094(void);

    s32 i;

    for (i = 0xC8; i < 0xCD; i++)
    {
        D_800D9268[i].field_22 = 0;
        D_800D9268[i].field_26 = 8;
    }
    D_801B2A2C = 0x10;
    D_801B2A28 += 1;
    func_8008E094();
}

/**
 * @brief Draw the world-map cursor via func_8006B6EC, then advance after the wait expires.
 */
void func_8008E094(void)
{
extern s32 D_801B2A28;
extern s32 D_801B2A2C;

    func_8006B6EC(0xC8, 0xCD, 0xF, 0, 0x6);
    if (--D_801B2A2C == 0)
    {
        D_801B2A28 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008E0F4(void)
{
extern s32 D_801B2A28;

    D_801B2A28 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008E10C(s32 arg0)
{
extern u32 D_801B2A30;
extern s32 D_801B2A34;
extern void (*D_800D5E40[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A30 = 1;
        D_801B2A34 = 1;
        return 1;
    }

    if (D_801B2A30 < 0x6)
    {
        D_800D5E40[D_801B2A30]();
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
void func_8008E184(void)
{
extern u32 D_801B2A30;
extern s32 D_801B2A34;
extern void (*D_800D5E40[])(void);

    D_801B2A30 = 1;
    D_801B2A34 = 1;
}

/** @brief World-map step: reset the sprite counters and advance to the next handler. */
void func_8008E19C(void)
{
extern s32 D_801B2468;
extern s32 D_801B24B4;
extern s32 D_80182DE4;
extern s16 D_801B2490[];
extern s16 D_801B2498[];
extern s32 D_801B2A34;
extern s32 D_801B2A30;
extern void func_8008C8C4(void);

    D_801B2468 = 1;
    D_801B24B4 = 0;
    D_80182DE4 = 0;
    D_801B2490[0] = 0;
    D_801B2490[1] = 0;
    D_801B2490[2] = 0;
    D_801B2498[0] = 0;
    D_801B2498[1] = 0;
    D_801B2498[2] = 0;
    D_801B2A34 = 0x60;
    D_801B2A30 += 1;
    func_8008C8C4();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8008E218(void)
{
extern s32 D_801B2A30;
extern void func_8008CA70(void);
extern s32 D_801B2A34;

    D_801B2A34 = 0x20;
    D_801B2A30 += 1;
    func_8008CA70();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008E250(void)
{
extern s32 D_801B2A30;
extern void func_8008CA70(void);
extern s32 D_801B2A34;

    D_801B2A30 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008E268(s32 arg0)
{
extern u32 D_801B2A38;
extern s32 D_801B2A3C;
extern void (*D_800D5E58[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A38 = 1;
        D_801B2A3C = 1;
        return 1;
    }

    if (D_801B2A38 < 0x6)
    {
        D_800D5E58[D_801B2A38]();
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
void func_8008E2E0(void)
{
extern u32 D_801B2A38;
extern s32 D_801B2A3C;
extern void (*D_800D5E58[])(void);

    D_801B2A38 = 1;
    D_801B2A3C = 1;
}

/** @brief World-map step handler: seed the actor block, arm the timer, and advance. */
void func_8008E2F8(void)
{
typedef struct
{
    u8 b[8];
} Blk8;

typedef struct
{
    s32 w[4];
} Blk16;

extern Blk8 D_80139258;
extern Blk8 D_8013B240;
extern Blk16 D_80182DC0;
extern Blk16 D_80139888;
extern s32 D_80182DF4;
extern s32 D_801B2A38;
extern s32 D_801B2A3C;
extern void func_8008CC14(void);

    D_8013B240 = D_80139258;
    D_80139888 = D_80182DC0;
    D_80182DF4 = 1;
    *(s32 *)((u8 *)&D_80139888 + 0x8) = 0xBB8;
    D_801B2A3C = 0x48;
    D_801B2A38 += 1;
    func_8008CC14();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8008E3A8(void)
{
extern s32 D_801B2A38;
extern void func_8008CD10(void);
extern s32 D_801B2A3C;

    D_801B2A3C = 0x40;
    D_801B2A38 += 1;
    func_8008CD10();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008E3E0(void)
{
extern s32 D_801B2A38;
extern void func_8008CD10(void);
extern s32 D_801B2A3C;

    D_801B2A38 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008E3F8(s32 arg0)
{
extern u32 D_801B2A40;
extern s32 D_801B2A44;
extern void (*D_800D5E70[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A40 = 1;
        D_801B2A44 = 1;
        return 1;
    }

    if (D_801B2A40 < 0x6)
    {
        D_800D5E70[D_801B2A40]();
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
void func_8008E470(void)
{
extern u32 D_801B2A40;
extern s32 D_801B2A44;
extern void (*D_800D5E70[])(void);

    D_801B2A40 = 1;
    D_801B2A44 = 1;
}

/** @brief World-map step handler: seed the actor block, arm the timer, and advance. */
void func_8008E488(void)
{
typedef struct
{
    u8 b[8];
} Blk8;

typedef struct
{
    s32 w[4];
} Blk16;

extern Blk8 D_80139258;
extern Blk8 D_801B2670;
extern Blk16 D_80182DC0;
extern Blk16 D_80139898;
extern s32 D_801B25D8;
extern s32 D_801B2A40;
extern s32 D_801B2A44;
extern void func_8008CE08(void);

    D_801B2670 = D_80139258;
    D_80139898 = D_80182DC0;
    D_801B25D8 = 1;
    *(s32 *)((u8 *)&D_80139898 + 0x8) = 0x7530;
    D_801B2A44 = 0x7C;
    D_801B2A40 += 1;
    func_8008CE08();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8008E538(void)
{
extern s32 D_801B2A40;
extern void func_8008CF04(void);
extern s32 D_801B2A44;

    D_801B2A44 = 0x40;
    D_801B2A40 += 1;
    func_8008CF04();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008E570(void)
{
extern s32 D_801B2A40;
extern void func_8008CF04(void);
extern s32 D_801B2A44;

    D_801B2A40 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008E588(s32 arg0)
{
extern u32 D_801B2A48;
extern s32 D_801B2A4C;
extern void (*D_800D5E88[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A48 = 1;
        D_801B2A4C = 1;
        return 1;
    }

    if (D_801B2A48 < 0x6)
    {
        D_800D5E88[D_801B2A48]();
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
void func_8008E600(void)
{
extern u32 D_801B2A48;
extern s32 D_801B2A4C;
extern void (*D_800D5E88[])(void);

    D_801B2A48 = 1;
    D_801B2A4C = 1;
}

/** @brief World-map step handler: seed the actor block, arm the timer, and advance. */
void func_8008E618(void)
{
typedef struct
{
    u8 b[8];
} Blk8;

typedef struct
{
    s32 w[4];
} Blk16;

extern Blk8 D_80139258;
extern Blk8 D_801B2678;
extern Blk16 D_80182DC0;
extern Blk16 D_801B2660;
extern s32 D_801B25DC;
extern s32 D_801B2A48;
extern s32 D_801B2A4C;
extern void func_8008CFFC(void);

    D_801B2678 = D_80139258;
    D_801B2660 = D_80182DC0;
    D_801B25DC = 1;
    *(s32 *)((u8 *)&D_801B2660 + 0x8) = 0x7530;
    D_801B2A4C = 0x7C;
    D_801B2A48 += 1;
    func_8008CFFC();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8008E6C8(void)
{
extern s32 D_801B2A48;
extern void func_8008D0F8(void);
extern s32 D_801B2A4C;

    D_801B2A4C = 0x40;
    D_801B2A48 += 1;
    func_8008D0F8();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008E700(void)
{
extern s32 D_801B2A48;
extern void func_8008D0F8(void);
extern s32 D_801B2A4C;

    D_801B2A48 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008E718(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2A50;
extern s32 D_801B2A54;
extern void (*D_800D5EA0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_8008D1F0(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A50 = 1;
        D_801B2A54 = 1;
        return 1;
    }

    if (D_801B2A50 < 0x4)
    {
        D_800D5EA0[D_801B2A50]();
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
void func_8008E790(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2A50;
extern s32 D_801B2A54;
extern void (*D_800D5EA0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_8008D1F0(void);

    D_801B2A50 = 1;
    D_801B2A54 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8008E7A8(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2A50;
extern s32 D_801B2A54;
extern void (*D_800D5EA0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_8008D1F0(void);

    D_801B24A0 = D_80139258;
    D_801B2650 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B2A54 = 0x40;
    D_801B2A50 += 1;
    func_8008D1F0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008E858(void)
{
extern s32 D_801B2A50;

    D_801B2A50 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008E870(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2A58;
extern s32 D_801B2A5C;
extern void (*D_800D5EB0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_8008D2F0(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A58 = 1;
        D_801B2A5C = 1;
        return 1;
    }

    if (D_801B2A58 < 0x4)
    {
        D_800D5EB0[D_801B2A58]();
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
void func_8008E8E8(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2A58;
extern s32 D_801B2A5C;
extern void (*D_800D5EB0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_8008D2F0(void);

    D_801B2A58 = 1;
    D_801B2A5C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8008E900(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2A58;
extern s32 D_801B2A5C;
extern void (*D_800D5EB0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_8008D2F0(void);

    D_801B24A8 = D_80139258;
    D_801B2478 = D_80182DC0;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B2A5C = 0x40;
    D_801B2A58 += 1;
    func_8008D2F0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008E9B0(void)
{
extern s32 D_801B2A58;

    D_801B2A58 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008E9C8(s32 arg0)
{
extern u32 D_801B2A60;
extern s32 D_801B2A64;
extern void (*D_800D5EC0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2A60 = 1;
        D_801B2A64 = 1;
        return 1;
    }

    if (D_801B2A60 < 0x8)
    {
        D_800D5EC0[D_801B2A60]();
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
void func_8008EA40(void)
{
extern u32 D_801B2A60;
extern s32 D_801B2A64;
extern void (*D_800D5EC0[])(void);

    D_801B2A60 = 1;
    D_801B2A64 = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_8008EA58(void)
{
extern s32 D_80182DF0;
extern s32 D_801B2A60;
extern s32 D_801B2A64;

    s32 remaining;

    func_8006B328(0x64, 0xC8, 1, -1, -2, -2, 0, 0xF, -0xAF, 0x15E, -0xAF, 0x15E, 0x32, 1, 0x81, 2, 0);
    D_80182DF0 += 8;
    remaining = D_801B2A64 - 1;
    D_801B2A64 = remaining;
    if (remaining == 0)
    {
        D_801B2A60 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8008EB10(void)
{
extern void func_8008EB48(void);
extern s32 D_801B2A64;
extern s32 D_801B2A60;

    D_801B2A64 = 0x40;
    D_801B2A60 += 1;
    func_8008EB48();
}

/** @brief World-map step handler: spawn a sprite via a long argument list, then countdown-advance. */
void func_8008EB48(void)
{
extern s32 D_801B2A64;
extern s32 D_801B2A60;

    func_8006B328(0x64, 0xC8, 1, -1, -2, -2, 0, 0xF, -0xAF, 0x15E, -0xAF, 0x15E,
                  0x32, 1, 0x81, 2, 0);
    if (--D_801B2A64 == 0)
    {
        D_801B2A60 += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_8008EBF4(void)
{
extern void func_8008EC34(void);
extern s32 D_800DCEA8;
extern s32 D_801B2A64;
extern s32 D_801B2A60;

    D_800DCEA8 = 0;
    D_801B2A64 = 0x40;
    D_801B2A60 += 1;
    func_8008EC34();
}

/** @brief World-map step handler: spawn a sprite via a long argument list, then countdown-advance. */
void func_8008EC34(void)
{
extern s32 D_801B2A64;
extern s32 D_801B2A60;

    func_8006B328(0x64, 0xC8, 1, -1, -2, -2, 0, 0xF, -0xAF, 0x15E, -0xAF, 0x15E,
                  0x32, 1, 0x81, 2, 0);
    if (--D_801B2A64 == 0)
    {
        D_801B2A60 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008ECE0(void)
{
extern s32 D_801B2A60;

    D_801B2A60 += 1;
}
