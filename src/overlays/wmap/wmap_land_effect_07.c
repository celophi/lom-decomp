#include "wmap_land_effect_07.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_effect_primitives.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"

/** @brief Draw two oscillating effect layers and update their intensity. */
void func_8007EC70(void)

{
/* Partial WMAP decompilation: 87.198110% (gcc280_g0). */

extern s8 D_80051B4C[];
extern u8 D_800DCF18[];
extern VECTOR D_80182DC0;
extern s32 D_80182DE4;
extern s32 D_801B2468;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B24B4;
extern s32 D_801B2790;
extern s32 D_801B2794;

    s32 first_frame;
    SVECTOR *rotation;
    s32 second_frame;
    s32 intensity;
    s32 remaining;

    PushMatrix();
    rotation = &D_801B2490;
    first_frame = (s32) (D_80051B4C[D_801B24B4] + 0x80) >> 5;
    func_8006CFA8(&D_80182DC0, rotation);
    func_8006CD98(D_800DCF18, first_frame, 4, 0x35, 0x7800, 0, D_801B2468);
    rotation->vz = (u16) (rotation->vz + 0x14);
    rotation = &D_801B2498;
    second_frame = (s32) (D_80051B4C[D_80182DE4] + 0x80) >> 5;
    func_8006CFA8(&D_80182DC0, rotation);
    func_8006CD98(D_800DCF18, second_frame, 4, 0x35, 0x7800, 0, D_801B2468);
    rotation->vz = (u16) (rotation->vz + 0x30);
    PopMatrix();
    D_801B24B4 = (D_801B24B4 + 8) & 0xFF;
    D_80182DE4 = (D_80182DE4 + 4) & 0xFF;
    intensity = D_801B2468 + 1;
    D_801B2468 = intensity;
    if (intensity >= 0x81)
    {
        D_801B2468 = 0x80;
    }
    remaining = D_801B2794 - 1;
    D_801B2794 = remaining;
    if (remaining == 0)
    {
        D_801B2790 += 1;
    }
}

/** @brief Draw two oscillating effect layers and update their intensity. */
void func_8007EE18(void)

{
/* Partial WMAP decompilation: 86.951920% (gcc280_g0). */

extern s8 D_80051B4C[];
extern u8 D_800DCF18[];
extern VECTOR D_80182DC0;
extern s32 D_80182DE4;
extern s32 D_801B2468;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B24B4;
extern s32 D_801B2790;
extern s32 D_801B2794;

    s32 first_frame;
    SVECTOR *rotation;
    s32 second_frame;
    s32 intensity;
    s32 remaining;

    PushMatrix();
    rotation = &D_801B2490;
    first_frame = (s32) (D_80051B4C[D_801B24B4] + 0x80) >> 5;
    func_8006CFA8(&D_80182DC0, rotation);
    func_8006CD98(D_800DCF18, first_frame, 4, 0x35, 0x7800, 0, D_801B2468);
    rotation->vz = (u16) (rotation->vz + 0x14);
    rotation = &D_801B2498;
    second_frame = (s32) (D_80051B4C[D_80182DE4] + 0x80) >> 5;
    func_8006CFA8(&D_80182DC0, rotation);
    func_8006CD98(D_800DCF18, second_frame, 4, 0x35, 0x7800, 0, D_801B2468);
    rotation->vz = (u16) (rotation->vz + 0x30);
    PopMatrix();
    D_801B24B4 = (D_801B24B4 + 8) & 0xFF;
    D_80182DE4 = (D_80182DE4 + 4) & 0xFF;
    intensity = D_801B2468 - 2;
    D_801B2468 = intensity;
    if (intensity < 0)
    {
        D_801B2468 = 0;
    }
    remaining = D_801B2794 - 1;
    D_801B2794 = remaining;
    if (remaining == 0)
    {
        D_801B2790 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8007EFB8(void)
{
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern s32 D_8011CF1C;
extern s32 D_801B279C;
extern s32 D_801B2798;

    MATRIX m;
    s32 x;

    x = D_801B2650[2] + 0xDAC;
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
        D_80182DE8 -= 0x8;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--D_801B279C == 0)
    {
        D_801B2798 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8007F0B8(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF24;
extern s32 D_801B27A4;
extern s32 D_801B27A0;

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
        D_80182DEC -= 0x8;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    if (--D_801B27A4 == 0)
    {
        D_801B27A0 += 1;
    }
}

/** @brief Configure effect parameters and reset its animation resource slots. */
void func_8007F1B8(void)
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

extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern s32 D_801B24B4;
extern s32 D_801B0FD0;
extern s32 D_801B27D8;
extern s32 D_801B27DC;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern u8 D_8011F538[];
extern void func_80080C88(void);

    s32 i;

    D_801B0FD0 = 12;
    D_801B24B4 = 127;
    D_80139234 = 4;
    D_8013923C = 4;
    D_80139240 = 16;
    D_8013924C = 0;
    D_80139250 = 2;
    D_80139260 = 1500;
    D_80139264 = 100;
    D_80139268 = 19;
    D_8013926C = 1;
    D_80139284 = 0x938801F4;
    for (i = 0; i < 12; i++)
    {
        D_801AFBD0[i + D_80139264].field_00 = 0;
        D_80139988[i + 104].field_04 = D_8011F538;
    }
    D_801B27DC = 24;
    D_801B27D8++;
    func_80080C88();
}

/** @brief Reduce the effect parameters, draw it, and advance its countdown. */
void func_8007F2D0(void)
{
extern s32 D_8011CF74;
extern s32 D_80139980;
extern s32 D_801B0FD0;
extern s32 D_801B27E0;
extern s32 D_801B27E4;

    s32 value;
    s32 remaining;

    value = D_80139980 - 2;
    D_80139980 = value;
    if (value < 0)
    {
        D_80139980 = 0;
    }
    if (!(D_8011CF74 & 3))
    {
        D_801B0FD0 -= 1;
    }
    if (D_801B0FD0 < 0)
    {
        D_801B0FD0 = 0;
    }
    func_8006AFAC(0xCC, 0xD2, 0x13, 0x20, 0x650, 8, 0x60, 0);
    remaining = D_801B27E4 - 1;
    D_801B27E4 = remaining;
    if (remaining == 0)
    {
        D_801B27E0 += 1;
    }
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007F398(s32 arg0)
{
extern u32 D_801B2780;
extern s32 D_801B2784;
extern void (*D_800D55B8[])(void);
extern void func_8007F46C(void);
extern s32 D_8013B20C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2780 = 1;
        D_801B2784 = 1;
        return 1;
    }

    if (D_801B2780 < 0x6)
    {
        D_800D55B8[D_801B2780]();
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
void func_8007F410(void)
{
extern u32 D_801B2780;
extern s32 D_801B2784;
extern void (*D_800D55B8[])(void);
extern void func_8007F46C(void);
extern s32 D_8013B20C;

    D_801B2780 = 1;
    D_801B2784 = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8007F428(void)
{
extern u32 D_801B2780;
extern s32 D_801B2784;
extern void (*D_800D55B8[])(void);
extern void func_8007F46C(void);
extern s32 D_8013B20C;

    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2780 += 1;
    func_8007F46C();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8007F46C(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2780;
extern void func_8007F4A8(void);
extern void func_8007F540(void);
extern void func_8007F4EC(void);

    if (D_8013B20C == 0)
    {
        D_801B2780 += 1;
        func_8007F4A8();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8007F4A8(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2780;
extern void func_8007F4A8(void);
extern void func_8007F540(void);
extern void func_8007F4EC(void);

    func_8006CAC0(func_8007F540);
    D_8013B20C = 1;
    D_801B2780 += 1;
    func_8007F4EC();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8007F4EC(void)
{
extern s32 D_801B2780;
extern s32 D_8013B20C;
extern void func_8007F528(void);

    if (D_8013B20C == 0)
    {
        D_801B2780 += 1;
        func_8007F528();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007F528(void)
{
extern s32 D_801B2780;
extern s32 D_8013B20C;
extern void func_8007F528(void);

    D_801B2780 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007F540(s32 arg0)
{
extern u32 D_801B2788;
extern s32 D_801B278C;
extern void (*D_800D55D0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2788 = 1;
        D_801B278C = 1;
        return 1;
    }

    if (D_801B2788 < 0x16)
    {
        D_800D55D0[D_801B2788]();
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
void func_8007F5B8(void)
{
extern u32 D_801B2788;
extern s32 D_801B278C;
extern void (*D_800D55D0[])(void);

    D_801B2788 = 1;
    D_801B278C = 1;
}

/** @brief World-map state entry: load resources, register callbacks, advance. */
void func_8007F5D0(void)
{
extern s32 D_8013B208;
extern s32 D_801ADAF4;
extern s32 D_801B2788;
extern s32 D_801B278C;
extern void func_8007FC38(void);
extern void func_8007FADC(void);

    D_8013B208 = 1;
    func_8006683C(0x803030);
    D_801ADAF4 = 4;
    func_800652A8(0x1D, 0x80);
    func_8006CAC0(func_8007FC38);
    func_8006CAC0(func_8007FADC);
    D_801B278C = 8;
    D_801B2788 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007F648(void)
{
extern s32 D_801B278C;
extern s32 D_801B2788;
extern void func_8007FEE8(void);

    if (--D_801B278C == 0)
    {
        D_801B2788 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007F67C(void)
{
extern s32 D_801B278C;
extern s32 D_801B2788;
extern void func_8007FEE8(void);

    func_8006CAC0(func_8007FEE8);
    D_801B278C = 0x8;
    D_801B2788 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007F6B8(void)
{
extern s32 D_801B278C;
extern s32 D_801B2788;
extern void func_80080228(void);

    if (--D_801B278C == 0)
    {
        D_801B2788 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007F6EC(void)
{
extern s32 D_801B278C;
extern s32 D_801B2788;
extern void func_80080228(void);

    func_8006CAC0(func_80080228);
    D_801B278C = 0x14;
    D_801B2788 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007F728(void)
{
extern s32 D_801B278C;
extern s32 D_801B2788;

    if (--D_801B278C == 0)
    {
        D_801B2788 += 1;
    }
}

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void func_8007F75C(void)
{
extern s32 D_801ADAE0;
extern s32 D_801B278C;
extern s32 D_801B2788;
extern void func_8008049C(void);

    D_801ADAE0 = 1;
    func_8006CAC0(func_8008049C);
    D_801B278C = 0x14;
    D_801B2788 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007F7A4(void)
{
extern s32 D_801B278C;
extern s32 D_801B2788;
extern void func_80080710(void);

    if (--D_801B278C == 0)
    {
        D_801B2788 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007F7D8(void)
{
extern s32 D_801B278C;
extern s32 D_801B2788;
extern void func_80080710(void);

    func_8006CAC0(func_80080710);
    D_801B278C = 0x14;
    D_801B2788 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007F814(void)
{
extern s32 D_801B278C;
extern s32 D_801B2788;
extern void func_80080984(void);

    if (--D_801B278C == 0)
    {
        D_801B2788 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007F848(void)
{
extern s32 D_801B278C;
extern s32 D_801B2788;
extern void func_80080984(void);

    func_8006CAC0(func_80080984);
    D_801B278C = 0x28;
    D_801B2788 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007F884(void)
{
extern s32 D_801B278C;
extern s32 D_801B2788;

    if (--D_801B278C == 0)
    {
        D_801B2788 += 1;
    }
}

/** @brief World-map step handler: register a callback and advance the step. */
void func_8007F8B8(void)
{
extern s32 D_801B2788;
extern s32 D_801B278C;
extern void func_8007FD90(void);

    func_8006CAC0(func_8007FD90);
    D_801B278C = 1;
    D_801B2788 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007F8F4(void)
{
extern s32 D_801B278C;
extern s32 D_801B2788;
extern void func_80080088(void);

    if (--D_801B278C == 0)
    {
        D_801B2788 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007F928(void)
{
extern s32 D_801B278C;
extern s32 D_801B2788;
extern void func_80080088(void);

    func_8006CAC0(func_80080088);
    D_801B278C = 0x12;
    D_801B2788 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007F964(void)
{
extern s32 D_801B278C;
extern s32 D_801B2788;
extern void func_80080DC4(void);

    if (--D_801B278C == 0)
    {
        D_801B2788 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007F998(void)
{
extern s32 D_801B278C;
extern s32 D_801B2788;
extern void func_80080DC4(void);

    func_8006CAC0(func_80080DC4);
    D_801B278C = 0x10;
    D_801B2788 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007F9D4(void)
{
extern s32 D_801B278C;
extern s32 D_801B2788;
extern void func_80080BF8(void);

    if (--D_801B278C == 0)
    {
        D_801B2788 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007FA08(void)
{
extern s32 D_801B278C;
extern s32 D_801B2788;
extern void func_80080BF8(void);

    func_8006CAC0(func_80080BF8);
    D_801B278C = 0x63;
    D_801B2788 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007FA44(void)
{
extern s32 D_801B278C;
extern s32 D_801B2788;

    if (--D_801B278C == 0)
    {
        D_801B2788 += 1;
    }
}

void func_8007FA78(void)
{
/* Partial WMAP decompilation: 95.600000% (gcc280_g0). */

extern s32 D_8013B20C;
extern u32 D_80139290[];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2788;

    D_8013B20C = 0;
    D_80139290[D_8011D530 * 10 + D_8011D510 * 60] = D_8011D4FC | 0x100;
    D_801B2788 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007FADC(s32 arg0)
{
extern u32 D_801B2790;
extern s32 D_801B2794;
extern void (*D_800D5628[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2790 = 1;
        D_801B2794 = 1;
        return 1;
    }

    if (D_801B2790 < 0x6)
    {
        D_800D5628[D_801B2790]();
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
void func_8007FB54(void)
{
extern u32 D_801B2790;
extern s32 D_801B2794;
extern void (*D_800D5628[])(void);

    D_801B2790 = 1;
    D_801B2794 = 1;
}

/** @brief World-map step: reset the sprite counters and advance to the next handler. */
void func_8007FB6C(void)
{
extern s32 D_801B2468;
extern s32 D_801B24B4;
extern s32 D_80182DE4;
extern s16 D_801B2490[];
extern s16 D_801B2498[];
extern s32 D_801B2794;
extern s32 D_801B2790;
extern void func_8007EC70(void);

    D_801B2468 = 1;
    D_801B24B4 = 0;
    D_80182DE4 = 0;
    D_801B2490[0] = 0;
    D_801B2490[1] = 0;
    D_801B2490[2] = 0;
    D_801B2498[0] = 0;
    D_801B2498[1] = 0;
    D_801B2498[2] = 0;
    D_801B2794 = 0x80;
    D_801B2790 += 1;
    func_8007EC70();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007FBE8(void)
{
extern s32 D_801B2790;
extern void func_8007EE18(void);
extern s32 D_801B2794;

    D_801B2794 = 0x40;
    D_801B2790 += 1;
    func_8007EE18();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007FC20(void)
{
extern s32 D_801B2790;
extern void func_8007EE18(void);
extern s32 D_801B2794;

    D_801B2790 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007FC38(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2798;
extern s32 D_801B279C;
extern void (*D_800D5640[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_8007EFB8(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2798 = 1;
        D_801B279C = 1;
        return 1;
    }

    if (D_801B2798 < 0x4)
    {
        D_800D5640[D_801B2798]();
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
void func_8007FCB0(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2798;
extern s32 D_801B279C;
extern void (*D_800D5640[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_8007EFB8(void);

    D_801B2798 = 1;
    D_801B279C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8007FCC8(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2798;
extern s32 D_801B279C;
extern void (*D_800D5640[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_8007EFB8(void);

    D_801B24A0 = D_80139258;
    D_801B2650 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B279C = 0x10;
    D_801B2798 += 1;
    func_8007EFB8();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007FD78(void)
{
extern s32 D_801B2798;

    D_801B2798 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007FD90(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B27A0;
extern s32 D_801B27A4;
extern void (*D_800D5650[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_8007F0B8(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B27A0 = 1;
        D_801B27A4 = 1;
        return 1;
    }

    if (D_801B27A0 < 0x4)
    {
        D_800D5650[D_801B27A0]();
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
void func_8007FE08(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B27A0;
extern s32 D_801B27A4;
extern void (*D_800D5650[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_8007F0B8(void);

    D_801B27A0 = 1;
    D_801B27A4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8007FE20(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B27A0;
extern s32 D_801B27A4;
extern void (*D_800D5650[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_8007F0B8(void);

    D_801B24A8 = D_80139258;
    D_801B2478 = D_80182DC0;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B27A4 = 0x10;
    D_801B27A0 += 1;
    func_8007F0B8();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007FED0(void)
{
extern s32 D_801B27A0;

    D_801B27A0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007FEE8(s32 arg0)
{
extern u32 D_801B27A8;
extern s32 D_801B27AC;
extern void (*D_800D5660[])(void);
extern void func_8007FFF4(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B27A8 = 1;
        D_801B27AC = 1;
        return 1;
    }

    if (D_801B27A8 < 0x4)
    {
        D_800D5660[D_801B27A8]();
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
void func_8007FF60(void)
{
extern u32 D_801B27A8;
extern s32 D_801B27AC;
extern void (*D_800D5660[])(void);
extern void func_8007FFF4(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    D_801B27A8 = 1;
    D_801B27AC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8007FF78(void)
{
extern u32 D_801B27A8;
extern s32 D_801B27AC;
extern void (*D_800D5660[])(void);
extern void func_8007FFF4(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 8;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x22] = 0x80;
    *(s16*)&D_800D9318[0x24] = 0;
    D_801B27AC = 0x70;
    D_801B27A8 += 1;
    func_8007FFF4();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007FFF4(void)
{
extern u32 D_801B27A8;
extern s32 D_801B27AC;
extern void (*D_800D5660[])(void);
extern void func_8007FFF4(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0x13, 0x2, 0);
    if (--D_801B27AC == 0)
    {
        D_801B27A8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80080070(void)
{
extern s32 D_801B27A8;

    D_801B27A8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80080088(s32 arg0)
{
extern u32 D_801B27B0;
extern s32 D_801B27B4;
extern void (*D_800D5670[])(void);
extern void func_80080194(void);
extern u8 D_80121538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B27B0 = 1;
        D_801B27B4 = 1;
        return 1;
    }

    if (D_801B27B0 < 0x4)
    {
        D_800D5670[D_801B27B0]();
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
void func_80080100(void)
{
extern u32 D_801B27B0;
extern s32 D_801B27B4;
extern void (*D_800D5670[])(void);
extern void func_80080194(void);
extern u8 D_80121538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801B27B0 = 1;
    D_801B27B4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80080118(void)
{
extern u32 D_801B27B0;
extern s32 D_801B27B4;
extern void (*D_800D5670[])(void);
extern void func_80080194(void);
extern u8 D_80121538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801399B4 = D_80121538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 8;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x22] = 0x80;
    *(s16*)&D_800D9344[0x24] = 0;
    D_801B27B4 = 0x88;
    D_801B27B0 += 1;
    func_80080194();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80080194(void)
{
extern u32 D_801B27B0;
extern s32 D_801B27B4;
extern void (*D_800D5670[])(void);
extern void func_80080194(void);
extern u8 D_80121538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x17, 0x70, 0);
    if (--D_801B27B4 == 0)
    {
        D_801B27B0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80080210(void)
{
extern s32 D_801B27B0;

    D_801B27B0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80080228(s32 arg0)
{
extern u32 D_801B27B8;
extern s32 D_801B27BC;
extern void (*D_800D5680[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B27B8 = 1;
        D_801B27BC = 1;
        return 1;
    }

    if (D_801B27B8 < 0x4)
    {
        D_800D5680[D_801B27B8]();
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
void func_800802A0(void)
{
extern u32 D_801B27B8;
extern s32 D_801B27BC;
extern void (*D_800D5680[])(void);

    D_801B27B8 = 1;
    D_801B27BC = 1;
}

/**
 * @brief Populate a world-map actor pair and schedule its animation step.
 */
void func_800802B8(void)
{
extern u8 D_800D9370[];
extern u8 D_80139988[];
extern u8 D_8011F538[];
extern s32 D_801B27B8;
extern s32 D_801B27BC;
extern void func_800803A0(void);

typedef struct
{
    s32 w[11];
} WmapBlk2C;

    u8* src = D_800D9370;
    u8* dst = D_800D9370 + 0x2C;
    u8* tbl = D_80139988;

    *(u8**)&tbl[0x34] = D_8011F538;
    *(u8**)&tbl[0x3C] = D_8011F538;
    src[0x6] = 0xF;
    *(s16*)&src[0xE] = 2;
    *(s16*)&src[0x10] = -1;
    *(s16*)&src[0x26] = 8;
    *(s16*)&src[0x22] = 0x81;
    *(s16*)&src[0x2] = 0;
    *(s16*)&src[0x24] = 1;
    *(WmapBlk2C*)dst = *(WmapBlk2C*)src;
    *(s16*)&dst[0xE] = 3;
    D_801B27BC = 0x68;
    D_801B27B8 += 1;
    func_800803A0();
}

/**
 * @brief Draw a world-map actor pair at an offset copy of the cursor position.
 * @note Repacks the cursor coordinate word, nudging each 16-bit half, then
 *       renders the primary actor and its shadow twin.
 */
void func_800803A0(void)
{
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B27B8;
extern s32 D_801B27BC;

    s32 pos;

    pos = (D_8011CF4C & 0xFFFF0000) | ((D_8011CF4C - 0xA) & 0xFFFF);
    pos = (pos & 0xFFFF) | (((pos >> 16) - 0xA) << 16);
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, pos, 0x13, 0x1, 0);
    func_8006CC4C(&D_800D9370[0x2C], &D_801399B8[0x8]);
    func_80066F9C(&D_800D9370[0x2C], pos, 0x13, 0x1, 0);
    if (--D_801B27BC == 0)
    {
        D_801B27B8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80080484(void)
{
extern s32 D_801B27B8;

    D_801B27B8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008049C(s32 arg0)
{
extern u32 D_801B27C0;
extern s32 D_801B27C4;
extern void (*D_800D5690[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B27C0 = 1;
        D_801B27C4 = 1;
        return 1;
    }

    if (D_801B27C0 < 0x4)
    {
        D_800D5690[D_801B27C0]();
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
void func_80080514(void)
{
extern u32 D_801B27C0;
extern s32 D_801B27C4;
extern void (*D_800D5690[])(void);

    D_801B27C0 = 1;
    D_801B27C4 = 1;
}

/**
 * @brief Populate a world-map actor pair and schedule its animation step.
 */
void func_8008052C(void)
{
extern u8 D_800D93C8[];
extern u8 D_80139988[];
extern u8 D_8011F538[];
extern s32 D_801B27C0;
extern s32 D_801B27C4;
extern void func_80080614(void);

typedef struct
{
    s32 w[11];
} WmapBlk2C;

    u8* src = D_800D93C8;
    u8* dst = D_800D93C8 + 0x2C;
    u8* tbl = D_80139988;

    *(u8**)&tbl[0x44] = D_8011F538;
    *(u8**)&tbl[0x4C] = D_8011F538;
    src[0x6] = 0xF;
    *(s16*)&src[0xE] = 2;
    *(s16*)&src[0x10] = -1;
    *(s16*)&src[0x26] = 8;
    *(s16*)&src[0x22] = 0x81;
    *(s16*)&src[0x2] = 0;
    *(s16*)&src[0x24] = 1;
    *(WmapBlk2C*)dst = *(WmapBlk2C*)src;
    *(s16*)&dst[0xE] = 3;
    D_801B27C4 = 0x54;
    D_801B27C0 += 1;
    func_80080614();
}

/**
 * @brief Draw a world-map actor pair at an offset copy of the cursor position.
 * @note Repacks the cursor coordinate word, nudging each 16-bit half, then
 *       renders the primary actor and its shadow twin.
 * @note Best match ~88.6% (gcc280_g0); residual is a callee-saved register
 *       allocation tie (pos vs base pointers) shared with func_800803A0.
 */
void func_80080614(void)
{
/* Partial WMAP decompilation: 88.561400% (gcc280_g0). */

extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;
extern s32 D_801B27C0;
extern s32 D_801B27C4;

    s32 pos;

    pos = (D_8011CF4C & 0xFFFF0000) | ((D_8011CF4C + 0x2) & 0xFFFF);
    pos = (pos & 0xFFFF) | (((pos >> 16) - 0x1C) << 16);
    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, pos, 0x13, 0x1, 0);
    func_8006CC4C(&D_800D93C8[0x2C], &D_801399C8[0x8]);
    func_80066F9C(&D_800D93C8[0x2C], pos, 0x13, 0x1, 0);
    if (--D_801B27C4 == 0)
    {
        D_801B27C0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800806F8(void)
{
extern s32 D_801B27C0;

    D_801B27C0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80080710(s32 arg0)
{
extern u32 D_801B27C8;
extern s32 D_801B27CC;
extern void (*D_800D56A0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B27C8 = 1;
        D_801B27CC = 1;
        return 1;
    }

    if (D_801B27C8 < 0x4)
    {
        D_800D56A0[D_801B27C8]();
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
void func_80080788(void)
{
extern u32 D_801B27C8;
extern s32 D_801B27CC;
extern void (*D_800D56A0[])(void);

    D_801B27C8 = 1;
    D_801B27CC = 1;
}

/**
 * @brief Populate a world-map actor pair and schedule its animation step.
 */
void func_800807A0(void)
{
extern u8 D_800D9420[];
extern u8 D_80139988[];
extern u8 D_8011F538[];
extern s32 D_801B27C8;
extern s32 D_801B27CC;
extern void func_80080888(void);

typedef struct
{
    s32 w[11];
} WmapBlk2C;

    u8* src = D_800D9420;
    u8* dst = D_800D9420 + 0x2C;
    u8* tbl = D_80139988;

    *(u8**)&tbl[0x54] = D_8011F538;
    *(u8**)&tbl[0x5C] = D_8011F538;
    src[0x6] = 0xF;
    *(s16*)&src[0xE] = 2;
    *(s16*)&src[0x10] = -1;
    *(s16*)&src[0x26] = 8;
    *(s16*)&src[0x22] = 0x81;
    *(s16*)&src[0x2] = 0;
    *(s16*)&src[0x24] = 1;
    *(WmapBlk2C*)dst = *(WmapBlk2C*)src;
    *(s16*)&dst[0xE] = 3;
    D_801B27CC = 0x40;
    D_801B27C8 += 1;
    func_80080888();
}

/**
 * @brief Draw a world-map actor pair at an offset copy of the cursor position.
 * @note Repacks the cursor coordinate word, nudging each 16-bit half, then
 *       renders the primary actor and its shadow twin.
 * @note Best match ~88.6% (gcc280_g0); residual is a callee-saved register
 *       allocation tie (pos vs base pointers) shared with func_800803A0.
 */
void func_80080888(void)
{
/* Partial WMAP decompilation: 88.561400% (gcc280_g0). */

extern u8 D_800D9420[];
extern u8 D_801399D8[];
extern s32 D_8011CF4C;
extern s32 D_801B27C8;
extern s32 D_801B27CC;

    s32 pos;

    pos = (D_8011CF4C & 0xFFFF0000) | ((D_8011CF4C + 0x14) & 0xFFFF);
    pos = (pos & 0xFFFF) | (((pos >> 16) - 0xF) << 16);
    func_8006CC4C(D_800D9420, D_801399D8);
    func_80066F9C(D_800D9420, pos, 0x13, 0x1, 0);
    func_8006CC4C(&D_800D9420[0x2C], &D_801399D8[0x8]);
    func_80066F9C(&D_800D9420[0x2C], pos, 0x13, 0x1, 0);
    if (--D_801B27CC == 0)
    {
        D_801B27C8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008096C(void)
{
extern s32 D_801B27C8;

    D_801B27C8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80080984(s32 arg0)
{
extern u32 D_801B27D0;
extern s32 D_801B27D4;
extern void (*D_800D56B0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B27D0 = 1;
        D_801B27D4 = 1;
        return 1;
    }

    if (D_801B27D0 < 0x4)
    {
        D_800D56B0[D_801B27D0]();
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
void func_800809FC(void)
{
extern u32 D_801B27D0;
extern s32 D_801B27D4;
extern void (*D_800D56B0[])(void);

    D_801B27D0 = 1;
    D_801B27D4 = 1;
}

/**
 * @brief Populate a world-map actor pair and schedule its animation step.
 */
void func_80080A14(void)
{
extern u8 D_800D9478[];
extern u8 D_80139988[];
extern u8 D_8011F538[];
extern s32 D_801B27D0;
extern s32 D_801B27D4;
extern void func_80080AFC(void);

typedef struct
{
    s32 w[11];
} WmapBlk2C;

    u8* src = D_800D9478;
    u8* dst = D_800D9478 + 0x2C;
    u8* tbl = D_80139988;

    *(u8**)&tbl[0x64] = D_8011F538;
    *(u8**)&tbl[0x6C] = D_8011F538;
    src[0x6] = 0xF;
    *(s16*)&src[0xE] = 2;
    *(s16*)&src[0x10] = -1;
    *(s16*)&src[0x26] = 8;
    *(s16*)&src[0x22] = 0x81;
    *(s16*)&src[0x2] = 0;
    *(s16*)&src[0x24] = 1;
    *(WmapBlk2C*)dst = *(WmapBlk2C*)src;
    *(s16*)&dst[0xE] = 3;
    D_801B27D4 = 0x2C;
    D_801B27D0 += 1;
    func_80080AFC();
}

/**
 * @brief Draw a world-map actor pair at an offset copy of the cursor position.
 * @note Repacks the cursor coordinate word, nudging each 16-bit half, then
 *       renders the primary actor and its shadow twin.
 * @note Best match ~88.6% (gcc280_g0); residual is a callee-saved register
 *       allocation tie (pos vs base pointers) shared with func_800803A0.
 */
void func_80080AFC(void)
{
/* Partial WMAP decompilation: 88.561400% (gcc280_g0). */

extern u8 D_800D9478[];
extern u8 D_801399E8[];
extern s32 D_8011CF4C;
extern s32 D_801B27D0;
extern s32 D_801B27D4;

    s32 pos;

    pos = (D_8011CF4C & 0xFFFF0000) | ((D_8011CF4C + 0x8) & 0xFFFF);
    pos = (pos & 0xFFFF) | (((pos >> 16) - 0x16) << 16);
    func_8006CC4C(D_800D9478, D_801399E8);
    func_80066F9C(D_800D9478, pos, 0x13, 0x1, 0);
    func_8006CC4C(&D_800D9478[0x2C], &D_801399E8[0x8]);
    func_80066F9C(&D_800D9478[0x2C], pos, 0x13, 0x1, 0);
    if (--D_801B27D4 == 0)
    {
        D_801B27D0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80080BE0(void)
{
extern s32 D_801B27D0;

    D_801B27D0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80080BF8(s32 arg0)
{
extern u32 D_801B27D8;
extern s32 D_801B27DC;
extern void (*D_800D56C0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B27D8 = 1;
        D_801B27DC = 1;
        return 1;
    }

    if (D_801B27D8 < 0x6)
    {
        D_800D56C0[D_801B27D8]();
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
void func_80080C70(void)
{
extern u32 D_801B27D8;
extern s32 D_801B27DC;
extern void (*D_800D56C0[])(void);

    D_801B27D8 = 1;
    D_801B27DC = 1;
}

/** @brief World-map step handler: spawn a sub-object and expire the step counter. */
void func_80080C88(void)
{
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32 D_801B24B4;
extern s32 D_801B27D8;
extern s32 D_801B27DC;

    func_8006CFE4(D_800DA448, D_80139CC8, 0xC, D_801B24B4, D_801B24B4, 0);
    if (--D_801B27DC == 0)
    {
        D_801B27D8 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80080CF4(void)
{
extern void func_80080D2C(void);
extern s32 D_801B27DC;
extern s32 D_801B27D8;

    D_801B27DC = 0x30;
    D_801B27D8 += 1;
    func_80080D2C();
}

/** @brief Draw the active effect and advance when the countdown expires. */
void func_80080D2C(void)
{
extern u8 D_800DA448[];
extern s32 *D_80139280;
extern u8 D_80139CC8[];
extern s32 D_801B24B4;
extern s32 D_801B27D8;
extern s32 D_801B27DC;

    s32 remaining_ticks;

    D_80139280[5] = -1;
    if (D_801B24B4 != 0)
    {
        func_8006CFE4((s32)D_800DA448, (s32)D_80139CC8, 0xC, D_801B24B4, D_801B24B4, 0);
    }
    remaining_ticks = D_801B27DC - 1;
    D_801B27DC = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B27D8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80080DAC(void)
{
extern s32 D_801B27D8;

    D_801B27D8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80080DC4(s32 arg0)
{
extern u32 D_801B27E0;
extern s32 D_801B27E4;
extern void (*D_800D56D8[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B27E0 = 1;
        D_801B27E4 = 1;
        return 1;
    }

    if (D_801B27E0 < 0x6)
    {
        D_800D56D8[D_801B27E0]();
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
void func_80080E3C(void)
{
extern u32 D_801B27E0;
extern s32 D_801B27E4;
extern void (*D_800D56D8[])(void);

    D_801B27E0 = 1;
    D_801B27E4 = 1;
}

/**
 * @brief World-map step handler: seed a 6-entry table and advance the step.
 * @note Best match ~84.86% (gcc280_g0); residual is loop induction-variable
 *       register allocation (permuter territory).
 */
void func_80080E54(void)
{
/* Partial WMAP decompilation: 84.857140% (gcc280_g0). */

extern u8 D_80139988[];
extern void *D_8011F538;
extern s16 D_801AFBD0;
extern s32 D_801B0FD0;
extern s32 D_80139980;
extern s32 D_801B27E0;
extern s32 D_801B27E4;
extern void func_80080EE0(void);

    s16 *p;
    s32 off;
    s32 i;

    D_801B0FD0 = 0;
    D_80139980 = 0x7F;
    p = &D_801AFBD0;
    i = 0;
    for (off = 0x660; i < 6; off += 8)
    {
        *p = 0;
        i += 1;
        *(void **)((u8 *)&D_80139988 + off + 4) = &D_8011F538;
        p += 0xA;
    }
    D_801B27E4 = 0x20;
    D_801B27E0 += 1;
    func_80080EE0();
}

/** @brief Advance the effect on odd frames, draw it, and update the countdown. */
void func_80080EE0(void)
{
extern s32 D_8011CF74;
extern s32 D_801B0FD0;
extern s32 D_801B27E0;
extern s32 D_801B27E4;

    s32 remaining_ticks;

    if (D_8011CF74 & 1)
    {
        D_801B0FD0 += 1;
    }
    func_8006AFAC(0xCC, 0xD2, 0x13, 0x20, 0x650, 8, 0x60, 0);
    remaining_ticks = D_801B27E4 - 1;
    D_801B27E4 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B27E0 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80080F78(void)
{
extern s32 D_801B27E0;
extern void func_8007F2D0(void);
extern s32 D_801B27E4;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B280C;
extern s32 D_801B2808;

    D_801B27E4 = 0x40;
    D_801B27E0 += 1;
    func_8007F2D0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80080FB0(void)
{
extern s32 D_801B27E0;
extern void func_8007F2D0(void);
extern s32 D_801B27E4;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B280C;
extern s32 D_801B2808;

    D_801B27E0 += 1;
}
