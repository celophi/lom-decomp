#include "wmap_main.h"
#include "wmap_land_effect_27.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8009AEBC(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B2C8C;
extern s32 D_801B2C88;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2C84;
extern s32 D_801B2C80;

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
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B2C8C;
extern s32 D_801B2C88;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2C84;
extern s32 D_801B2C80;

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
extern s32 D_800D665C[];
extern void *D_8011CF24;
extern void *D_8011CF28;
extern void *D_8011CF2C;
extern s32 D_80139234;
extern SVECTOR D_8013B238;
extern VECTOR D_80182DC0;
extern s32 D_80182DF0;
extern SVECTOR D_801B24A8;
extern s32 D_801B2C90;
extern s32 D_801B2C94;
extern void func_800675F0(void *, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    s32 intensity;
    s32 frame;
    s32 remaining;
    u16 angle;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_8013B238);
    func_800675F0(D_8011CF24, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -10, -1);
    func_8006CFA8(&D_80182DC0, &D_801B24A8);
    func_800675F0(D_8011CF28, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -40, -1);
    func_8006CFA8(&D_80182DC0, &D_8013B238);
    func_800675F0(D_8011CF2C, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -70, -1);
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
extern s32 D_800D665C[];
extern void *D_8011CF24;
extern void *D_8011CF28;
extern void *D_8011CF2C;
extern s32 D_80139234;
extern SVECTOR D_8013B238;
extern VECTOR D_80182DC0;
extern s32 D_80182DF0;
extern SVECTOR D_801B24A8;
extern s32 D_801B2C90;
extern s32 D_801B2C94;
extern void func_800675F0(void *, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    s32 intensity;
    s32 frame;
    s32 remaining;
    u16 angle;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_8013B238);
    func_800675F0(D_8011CF24, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -10, -1);
    func_8006CFA8(&D_80182DC0, &D_801B24A8);
    func_800675F0(D_8011CF28, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -40, -1);
    func_8006CFA8(&D_80182DC0, &D_8013B238);
    func_800675F0(D_8011CF2C, D_800D665C[D_80139234], 10, 54, 0x78C0, 0x1001, D_80182DF0, 0, -70, -1);
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
extern s32 D_801B2C98;
extern s32 D_801B2C9C;
extern u8 D_80121538[];
extern void func_8009C880__for_func_8009B4D4(void) __asm__("func_8009C880");

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
        D_80139988[i + 144].field_04 = D_80121538;
    }
    D_801B2C9C = 80;
    D_801B2C98++;
    func_8009C880__for_func_8009B4D4();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_8009B5B0(void)
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
extern u8 D_80121538[];
extern s32 D_801B2CA8;
extern s32 D_801B2CAC;
extern void func_8009CCF4__for_func_8009B5B0(void) __asm__("func_8009CCF4");

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
        D_80139988[i + 20].field_04 = D_80121538;
    }
    D_801B2CAC = 80;
    D_801B2CA8++;
    func_8009CCF4__for_func_8009B5B0();
}

/** @brief World-map step: init hero struct fields, clear tables, advance step. */
void func_8009B67C(void)
{
/* Partial WMAP decompilation: 96.078430% (gcc280_g0). */

typedef struct
{
    s16 field0;
    s16 field2;
    void *field4;
} WmapB;

typedef struct
{
    s16 field0;
    s16 pad[9];
} WmapA;

extern WmapA D_801AFBD0[];
extern WmapB D_80139988[];
extern u8 D_80121538[];
extern void *D_80139280;
extern s32 D_801B2CB0;
extern s32 D_801B2CB4;
extern void func_8009CEF4__for_func_8009B67C(void) __asm__("func_8009CEF4");

    s32 i;
    void *base = D_80139280;

    *(s32 *)((u8 *)base + 0x34) = 0x80;
    *(s32 *)((u8 *)base + 0x3C) = 2;
    *(s32 *)((u8 *)base + 0x40) = 0x64;
    *(s32 *)((u8 *)base + 0x44) = 0x3C;
    *(s32 *)((u8 *)base + 0x48) = 8;
    *(s32 *)((u8 *)base + 0x4C) = 1;
    *(s32 *)((u8 *)base + 0x2C) = 0;
    *(s32 *)((u8 *)base + 0x30) = 0;
    *(s32 *)((u8 *)base + 0x38) = 0;
    *(s32 *)((u8 *)base + 0x50) = 0x5DC0;

    for (i = 0; i < 0x18; i++)
    {
        D_801AFBD0[60 + i].field0 = 0;
        D_80139988[60 + i].field4 = D_80121538;
    }

    D_801B2CB4 = 0x38;
    D_801B2CB0 += 1;
    func_8009CEF4__for_func_8009B67C();
}

/** @brief World-map step handler: configure a model descriptor and clear two entry tables, then advance.
 *  @note Best match ~85.6% (gcc280_g0); residual is loop induction-variable register coloring. */
void func_8009B748(void)
{
/* Partial WMAP decompilation: 88.137250% (gcc280_g0). */

extern void func_8009D0FC__for_func_8009B748(void) __asm__("func_8009D0FC");
extern u8 *D_80139280;
extern u8 D_80139988;
extern u8 D_801AFBD0;
extern u8 D_80121538;
extern s32 D_801B2CB8;
extern s32 D_801B2CBC;

    u8 *pa;
    u8 *pb;
    u8 *p;
    s32 i;
    s32 off_a;
    s32 off_b;

    p = D_80139280;
    *(s32 *)(p + 0x5C) = 0x20;
    *(s32 *)(p + 0x68) = 0x3E8;
    *(s32 *)(p + 0x6C) = 0x64;
    *(s32 *)(p + 0x70) = 8;
    *(s32 *)(p + 0x74) = 2;
    *(s32 *)(p + 0x54) = 1;
    *(s32 *)(p + 0x58) = 1;
    *(s32 *)(p + 0x60) = 0;
    *(s32 *)(p + 0x64) = 1;
    *(s32 *)(p + 0x78) = 0x6D60;

    pa = &D_80139988;
    pb = &D_801AFBD0;
    i = 0;
    off_a = 0x320;
    off_b = 0x7D0;
    do
    {
        *(s16 *)(pb + off_b) = 0;
        *(s32 *)(pa + off_a + 4) = (s32)&D_80121538;
        off_a += 8;
        off_b += 0x14;
        i += 1;
    } while (i < 0x14);

    D_801B2CBC = 0x14;
    D_801B2CB8 += 1;
    func_8009D0FC__for_func_8009B748();
}

/** @brief World-map step: init hero struct fields, clear tables, advance step.
 *  @note Best match ~94% (gcc280_g0); residual is a whole-function register
 *        renumbering caused by the held constant 1 (a0) plus a store-order tie. */
void func_8009B814(void)
{
/* Partial WMAP decompilation: 94.076920% (gcc280_g0). */

typedef struct
{
    s16 field0;
    s16 field2;
    void *field4;
} WmapB;

typedef struct
{
    s16 field0;
    s16 pad[9];
} WmapA;

extern WmapA D_801AFBD0[];
extern WmapB D_80139988[];
extern u8 D_8011F538[];
extern void *D_80139280;
extern s32 D_801B2CC0;
extern s32 D_801B2CC4;
extern void func_8009D304__for_func_8009B814(void) __asm__("func_8009D304");

    s32 i;
    void *base = D_80139280;

    *(s32 *)((u8 *)base + 0xA8) = 4;
    *(s32 *)((u8 *)base + 0xAC) = 0x20;
    *(s32 *)((u8 *)base + 0xB8) = 0x3E8;
    *(s32 *)((u8 *)base + 0xBC) = 0xB4;
    *(s32 *)((u8 *)base + 0xC0) = 0x15;
    *(s32 *)((u8 *)base + 0xC4) = 2;
    *(s32 *)((u8 *)base + 0xA4) = 1;
    *(s32 *)((u8 *)base + 0xB0) = 0;
    *(s32 *)((u8 *)base + 0xB4) = 1;
    *(s32 *)((u8 *)base + 0xC8) = 0x1F40;

    for (i = 0; i < 0x14; i++)
    {
        D_801AFBD0[180 + i].field0 = 0;
        D_80139988[180 + i].field4 = D_8011F538;
    }

    D_801B2CC4 = 0x14;
    D_801B2CC0 += 1;
    func_8009D304__for_func_8009B814();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009B8E4(s32 arg0)
{
extern u32 D_801B2C60;
extern s32 D_801B2C64;
extern void (*D_800D65A4[])(void);
extern void func_8009B9B8__for_func_8009B8E4(void) __asm__("func_8009B9B8");
extern s32 D_8013B20C;

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
extern u32 D_801B2C60;
extern s32 D_801B2C64;
extern void (*D_800D65A4[])(void);
extern void func_8009B9B8__for_func_8009B95C(void) __asm__("func_8009B9B8");
extern s32 D_8013B20C;

    D_801B2C60 = 1;
    D_801B2C64 = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8009B974(void)
{
extern u32 D_801B2C60;
extern s32 D_801B2C64;
extern void (*D_800D65A4[])(void);
extern void func_8009B9B8__for_func_8009B974(void) __asm__("func_8009B9B8");
extern s32 D_8013B20C;

    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2C60 += 1;
    func_8009B9B8__for_func_8009B974();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009B9B8(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2C60;
extern void func_8009B9F4__for_func_8009B9B8(void) __asm__("func_8009B9F4");
extern void func_8009BA8C__for_func_8009B9B8(void) __asm__("func_8009BA8C");
extern void func_8009BA38__for_func_8009B9B8(void) __asm__("func_8009BA38");

    if (D_8013B20C == 0)
    {
        D_801B2C60 += 1;
        func_8009B9F4__for_func_8009B9B8();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8009B9F4(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2C60;
extern void func_8009B9F4(void);
extern void func_8009BA8C__for_func_8009B9F4(void) __asm__("func_8009BA8C");
extern void func_8009BA38__for_func_8009B9F4(void) __asm__("func_8009BA38");

    func_8006CAC0(func_8009BA8C__for_func_8009B9F4);
    D_8013B20C = 1;
    D_801B2C60 += 1;
    func_8009BA38__for_func_8009B9F4();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009BA38(void)
{
extern s32 D_801B2C60;
extern s32 D_8013B20C;
extern void func_8009BA74__for_func_8009BA38(void) __asm__("func_8009BA74");

    if (D_8013B20C == 0)
    {
        D_801B2C60 += 1;
        func_8009BA74__for_func_8009BA38();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009BA74(void)
{
extern s32 D_801B2C60;
extern s32 D_8013B20C;
extern void func_8009BA74(void);

    D_801B2C60 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009BA8C(s32 arg0)
{
extern u32 D_801B2C68;
extern s32 D_801B2C6C;
extern void (*D_800D65BC[])(void);

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
extern u32 D_801B2C68;
extern s32 D_801B2C6C;
extern void (*D_800D65BC[])(void);

    D_801B2C68 = 1;
    D_801B2C6C = 1;
}

/** @brief World-map step: mark active, request a resource, seed the sub-counter, tick. */
void func_8009BB1C(void)
{
extern s32 D_8013B208;
extern s32 D_801B2C6C;
extern s32 D_801B2C68;

    D_8013B208 = 1;
    func_800652A8(0x2E, 0x80);
    D_801B2C6C = 4;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BB64(void)
{
extern s32 D_801B2C6C;
extern s32 D_801B2C68;
extern void func_8009C7F0__for_func_8009BB64(void) __asm__("func_8009C7F0");

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
extern s32 D_801B2C6C;
extern s32 D_801B2C68;
extern void func_8009C7F0__for_func_8009BB98(void) __asm__("func_8009C7F0");

    func_8006CAC0(func_8009C7F0__for_func_8009BB98);
    D_801B2C6C = 0xF;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BBD4(void)
{
extern s32 D_801B2C6C;
extern s32 D_801B2C68;
extern void func_8009CE64__for_func_8009BBD4(void) __asm__("func_8009CE64");

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
extern s32 D_801B2C6C;
extern s32 D_801B2C68;
extern void func_8009CE64__for_func_8009BC08(void) __asm__("func_8009CE64");

    func_8006CAC0(func_8009CE64__for_func_8009BC08);
    D_801B2C6C = 0xC;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BC44(void)
{
extern s32 D_801B2C6C;
extern s32 D_801B2C68;

    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

/** @brief World-map step handler: register a callback, kick a job, advance the step. */
void func_8009BC78(void)
{
extern s32 D_801B2C68;
extern s32 D_801B2C6C;
extern void func_8009C3E4__for_func_8009BC78(void) __asm__("func_8009C3E4");

    func_8006CAC0(func_8009C3E4__for_func_8009BC78);
    func_8006683C(0x651035);
    g_wmap_backdrop_target_level = 4;
    D_801B2C6C = 2;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BCCC(void)
{
extern s32 D_801B2C6C;
extern s32 D_801B2C68;

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
extern s32 D_80139244;
extern s32 D_801B2C68;
extern s32 D_801B2C6C;

    D_80139244 = 1;
    D_801B2C6C = 0x14;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BD2C(void)
{
extern s32 D_801B2C6C;
extern s32 D_801B2C68;
extern void func_8009CC64__for_func_8009BD2C(void) __asm__("func_8009CC64");
extern void func_8009C694__for_func_8009BD2C(void) __asm__("func_8009C694");

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
extern s32 D_801B2C6C;
extern s32 D_801B2C68;
extern void func_8009CC64__for_func_8009BD60(void) __asm__("func_8009CC64");
extern void func_8009C694__for_func_8009BD60(void) __asm__("func_8009C694");

    func_8006CAC0(func_8009CC64__for_func_8009BD60);
    func_8006CAC0(func_8009C694__for_func_8009BD60);
    D_801B2C6C = 0x28;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BDA8(void)
{
extern s32 D_801B2C6C;
extern s32 D_801B2C68;

    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

/** @brief Register two callbacks around the sequence flag update and begin a 30-tick delay. */
void func_8009BDDC(void)
{
extern s32 D_801ADAE0;
extern s32 D_801B2C68;
extern s32 D_801B2C6C;
extern void func_8009C09C__for_func_8009BDDC(void) __asm__("func_8009C09C");
extern void func_8009C9F8__for_func_8009BDDC(void) __asm__("func_8009C9F8");

    func_8006CAC0(&func_8009C9F8__for_func_8009BDDC);
    D_801ADAE0 = 1;
    func_8006CAC0(&func_8009C09C__for_func_8009BDDC);
    D_801B2C6C = 0x1E;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BE30(void)
{
extern s32 D_801B2C6C;
extern s32 D_801B2C68;
extern void func_8009D47C__for_func_8009BE30(void) __asm__("func_8009D47C");

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
extern s32 D_801B2C6C;
extern s32 D_801B2C68;
extern void func_8009D47C__for_func_8009BE64(void) __asm__("func_8009D47C");

    func_8006CAC0(func_8009D47C__for_func_8009BE64);
    D_801B2C6C = 0x38;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BEA0(void)
{
extern s32 D_801B2C6C;
extern s32 D_801B2C68;
extern void func_8009C53C__for_func_8009BEA0(void) __asm__("func_8009C53C");

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
extern s32 D_801B2C6C;
extern s32 D_801B2C68;
extern void func_8009C53C__for_func_8009BED4(void) __asm__("func_8009C53C");

    func_8006CAC0(func_8009C53C__for_func_8009BED4);
    D_801B2C6C = 0x2;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BF10(void)
{
extern s32 D_801B2C6C;
extern s32 D_801B2C68;

    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

/** @brief Register a sequence callback, clear the world-map value, and start a 64-tick delay. */
void func_8009BF44(void)
{
extern s32 D_80139244;
extern s32 D_801B2C68;
extern s32 D_801B2C6C;
extern void func_8009C240__for_func_8009BF44(void) __asm__("func_8009C240");

    func_8006CAC0(&func_8009C240__for_func_8009BF44);
    D_80139244 = 0;
    D_801B2C6C = 0x40;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009BF88(void)
{
extern s32 D_801B2C6C;
extern s32 D_801B2C68;
extern void func_8009D274__for_func_8009BF88(void) __asm__("func_8009D274");
extern void func_8009D06C__for_func_8009BF88(void) __asm__("func_8009D06C");

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
extern s32 D_801B2C6C;
extern s32 D_801B2C68;
extern void func_8009D274__for_func_8009BFBC(void) __asm__("func_8009D274");
extern void func_8009D06C__for_func_8009BFBC(void) __asm__("func_8009D06C");

    func_8006CAC0(func_8009D274__for_func_8009BFBC);
    func_8006CAC0(func_8009D06C__for_func_8009BFBC);
    D_801B2C6C = 0xAC;
    D_801B2C68 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009C004(void)
{
extern s32 D_801B2C6C;
extern s32 D_801B2C68;

    if (--D_801B2C6C == 0)
    {
        D_801B2C68 += 1;
    }
}

void func_8009C038(void)
{
/* Partial WMAP decompilation: 95.600000% (gcc280_g0). */

extern s32 D_8013B20C;
extern u32 D_80139290[];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2C68;

    D_8013B20C = 0;
    D_80139290[D_8011D530 * 10 + D_8011D510 * 60] = D_8011D4FC | 0x100;
    D_801B2C68 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009C09C(s32 arg0)
{
extern u32 D_801B2C70;
extern s32 D_801B2C74;
extern void (*D_800D661C[])(void);

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
extern u32 D_801B2C70;
extern s32 D_801B2C74;
extern void (*D_800D661C[])(void);

    D_801B2C70 = 1;
    D_801B2C74 = 1;
}

void func_8009C12C(void)
{
/* Partial WMAP decompilation: 87.156250% (gcc280_g0). */

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
} __attribute__((aligned(4))) WmapConfigA;


extern WmapConfigA D_800D9318;
extern u8 D_8011F538;
extern void *D_801399AC;
extern s32 D_801B2C70;
extern s32 D_801B2C74;
extern void func_8009C1AC__for_func_8009C12C(void) __asm__("func_8009C1AC");

    D_801399AC = &D_8011F538;
    D_800D9318.field_06 = 0xF;
    D_800D9318.field_10 = -1;
    D_800D9318.field_26 = 8;
    D_800D9318.field_0E = 1;
    D_800D9318.field_24 = 1;
    D_800D9318.field_02 = 0;
    D_800D9318.field_22 = 0x81;
    D_801B2C74 = 0x24;
    D_801B2C70 += 1;
    func_8009C1AC__for_func_8009C12C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009C1AC(void)
{
extern s32 D_801B2C70;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B2C74;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0x15, 0x2, 0);
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
extern s32 D_801B2C70;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B2C74;

    D_801B2C70 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009C240(s32 arg0)
{
extern u32 D_801B2C78;
extern s32 D_801B2C7C;
extern void (*D_800D662C[])(void);
extern void func_8009C350__for_func_8009C240(void) __asm__("func_8009C350");
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

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
extern u32 D_801B2C78;
extern s32 D_801B2C7C;
extern void (*D_800D662C[])(void);
extern void func_8009C350__for_func_8009C2B8(void) __asm__("func_8009C350");
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801B2C78 = 1;
    D_801B2C7C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8009C2D0(void)
{
extern u32 D_801B2C78;
extern s32 D_801B2C7C;
extern void (*D_800D662C[])(void);
extern void func_8009C350__for_func_8009C2D0(void) __asm__("func_8009C350");
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801399B4 = D_8011D538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0xE] = 1;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 2;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0x22] = 0x80;
    *(s16*)&D_800D9344[0x24] = 0;
    D_801B2C7C = 0xEE;
    D_801B2C78 += 1;
    func_8009C350__for_func_8009C2D0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009C350(void)
{
extern u32 D_801B2C78;
extern s32 D_801B2C7C;
extern void (*D_800D662C[])(void);
extern void func_8009C350(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x12, 0x5, 0);
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
extern s32 D_801B2C78;

    D_801B2C78 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009C3E4(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2C80;
extern s32 D_801B2C84;
extern void (*D_800D663C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_8009AEBC__for_func_8009C3E4(void) __asm__("func_8009AEBC");

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
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2C80;
extern s32 D_801B2C84;
extern void (*D_800D663C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_8009AEBC__for_func_8009C45C(void) __asm__("func_8009AEBC");

    D_801B2C80 = 1;
    D_801B2C84 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8009C474(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2C80;
extern s32 D_801B2C84;
extern void (*D_800D663C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_8009AEBC__for_func_8009C474(void) __asm__("func_8009AEBC");

    D_801B24A0 = D_80139258;
    D_801B2650 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B2C84 = 0x20;
    D_801B2C80 += 1;
    func_8009AEBC__for_func_8009C474();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009C524(void)
{
extern s32 D_801B2C80;

    D_801B2C80 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009C53C(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2C88;
extern s32 D_801B2C8C;
extern void (*D_800D664C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_8009AFBC__for_func_8009C53C(void) __asm__("func_8009AFBC");

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
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2C88;
extern s32 D_801B2C8C;
extern void (*D_800D664C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_8009AFBC__for_func_8009C5B4(void) __asm__("func_8009AFBC");

    D_801B2C88 = 1;
    D_801B2C8C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8009C5CC(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2C88;
extern s32 D_801B2C8C;
extern void (*D_800D664C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_8009AFBC__for_func_8009C5CC(void) __asm__("func_8009AFBC");

    D_801B24A8 = D_80139258;
    D_801B2478 = D_80182DC0;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B2C8C = 0x20;
    D_801B2C88 += 1;
    func_8009AFBC__for_func_8009C5CC();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009C67C(void)
{
extern s32 D_801B2C88;

    D_801B2C88 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009C694(s32 arg0)
{
extern u32 D_801B2C90;
extern s32 D_801B2C94;
extern void (*D_800D6674[])(void);

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
extern u32 D_801B2C90;
extern s32 D_801B2C94;
extern void (*D_800D6674[])(void);

    D_801B2C90 = 1;
    D_801B2C94 = 1;
}

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_8009C724(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DF0;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern s32 D_80139234;
extern s32 D_801B2C90;
extern s32 D_801B2C94;
extern void func_8009B0BC__for_func_8009C724(void) __asm__("func_8009B0BC");

    D_80182DF0 = 1;
    D_8013B238 = D_80139258;
    D_80139234 = 0;
    D_801B2C94 = 0x60;
    D_801B2C90 += 1;
    func_8009B0BC__for_func_8009C724();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8009C7A0(void)
{
extern s32 D_801B2C90;
extern void func_8009B2CC__for_func_8009C7A0(void) __asm__("func_8009B2CC");
extern s32 D_801B2C94;

    D_801B2C94 = 0x20;
    D_801B2C90 += 1;
    func_8009B2CC__for_func_8009C7A0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009C7D8(void)
{
extern s32 D_801B2C90;
extern void func_8009B2CC__for_func_8009C7D8(void) __asm__("func_8009B2CC");
extern s32 D_801B2C94;

    D_801B2C90 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009C7F0(s32 arg0)
{
extern u32 D_801B2C98;
extern s32 D_801B2C9C;
extern void (*D_800D668C[])(void);
extern u8 D_800DAB28[];
extern u8 D_80139E08[];
extern s32 D_80139280;

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
extern u32 D_801B2C98;
extern s32 D_801B2C9C;
extern void (*D_800D668C[])(void);
extern u8 D_800DAB28[];
extern u8 D_80139E08[];
extern s32 D_80139280;

    D_801B2C98 = 1;
    D_801B2C9C = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009C880(void)
{
extern u32 D_801B2C98;
extern s32 D_801B2C9C;
extern void (*D_800D668C[])(void);
extern u8 D_800DAB28[];
extern u8 D_80139E08[];
extern s32 D_80139280;

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
extern void func_8009C954__for_func_8009C90C(void) __asm__("func_8009C954");
extern s32* D_80139280;
extern s32 D_801B2C9C;
extern s32 D_801B2C98;
extern u8 D_800DAB28[];
extern u8 D_80139E08[];

    D_801B2C9C = 0x20;
    D_80139280[35] = -1;
    D_801B2C98 += 1;
    func_8009C954__for_func_8009C90C();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009C954(void)
{
extern void func_8009C954(void);
extern s32* D_80139280;
extern s32 D_801B2C9C;
extern s32 D_801B2C98;
extern u8 D_800DAB28[];
extern u8 D_80139E08[];

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
extern s32 D_801B2C98;

    D_801B2C98 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009C9F8(s32 arg0)
{
extern u32 D_801B2CA0;
extern s32 D_801B2CA4;
extern void (*D_800D66A4[])(void);
extern void func_8009CB08__for_func_8009C9F8(void) __asm__("func_8009CB08");
extern u8 D_8011F538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

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
extern u32 D_801B2CA0;
extern s32 D_801B2CA4;
extern void (*D_800D66A4[])(void);
extern void func_8009CB08__for_func_8009CA70(void) __asm__("func_8009CB08");
extern u8 D_8011F538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    D_801B2CA0 = 1;
    D_801B2CA4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8009CA88(void)
{
extern u32 D_801B2CA0;
extern s32 D_801B2CA4;
extern void (*D_800D66A4[])(void);
extern void func_8009CB08__for_func_8009CA88(void) __asm__("func_8009CB08");
extern u8 D_8011F538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    D_801399BC = D_8011F538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0x10] = -1;
    *(s16*)&D_800D9370[0x26] = 4;
    *(s16*)&D_800D9370[0x22] = 0x81;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0xE] = 0;
    *(s16*)&D_800D9370[0x24] = 1;
    D_801B2CA4 = 0x80;
    D_801B2CA0 += 1;
    func_8009CB08__for_func_8009CA88();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009CB08(void)
{
extern u32 D_801B2CA0;
extern s32 D_801B2CA4;
extern void (*D_800D66A4[])(void);
extern void func_8009CB08(void);
extern u8 D_8011F538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0x15, 0x5, 0);
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
extern void func_8009CBD0__for_func_8009CB84(void) __asm__("func_8009CBD0");
extern s16 D_800D9370[];
extern s32 D_801B2CA4;
extern s32 D_801B2CA0;
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    D_800D9370[19] = 4;
    D_800D9370[17] = 0;
    D_801B2CA4 = 0x80;
    D_801B2CA0 += 1;
    func_8009CBD0__for_func_8009CB84();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009CBD0(void)
{
extern void func_8009CBD0(void);
extern s16 D_800D9370[];
extern s32 D_801B2CA4;
extern s32 D_801B2CA0;
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0x15, 0x5, 0);
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
extern s32 D_801B2CA0;

    D_801B2CA0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009CC64(s32 arg0)
{
extern u32 D_801B2CA8;
extern s32 D_801B2CAC;
extern void (*D_800D66BC[])(void);

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
extern u32 D_801B2CA8;
extern s32 D_801B2CAC;
extern void (*D_800D66BC[])(void);

    D_801B2CA8 = 1;
    D_801B2CAC = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009CCF4(void)
{
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32* D_80139280;
extern s32 D_801B2CAC;
extern s32 D_801B2CA8;

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
extern void func_8009CDC4__for_func_8009CD7C(void) __asm__("func_8009CDC4");
extern s32* D_80139280;
extern s32 D_801B2CAC;
extern s32 D_801B2CA8;

    D_801B2CAC = 0x20;
    D_80139280[5] = -1;
    D_801B2CA8 += 1;
    func_8009CDC4__for_func_8009CD7C();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009CDC4(void)
{
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32* D_80139280;
extern s32 D_801B2CAC;
extern s32 D_801B2CA8;

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
extern s32 D_801B2CA8;

    D_801B2CA8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009CE64(s32 arg0)
{
extern u32 D_801B2CB0;
extern s32 D_801B2CB4;
extern void (*D_800D66D4[])(void);
extern u8 D_800D9CB8[];
extern u8 D_80139B68[];
extern s32 D_80139280;

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
extern u32 D_801B2CB0;
extern s32 D_801B2CB4;
extern void (*D_800D66D4[])(void);
extern u8 D_800D9CB8[];
extern u8 D_80139B68[];
extern s32 D_80139280;

    D_801B2CB0 = 1;
    D_801B2CB4 = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009CEF4(void)
{
extern u32 D_801B2CB0;
extern s32 D_801B2CB4;
extern void (*D_800D66D4[])(void);
extern u8 D_800D9CB8[];
extern u8 D_80139B68[];
extern s32 D_80139280;

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
extern void func_8009CFC8__for_func_8009CF80(void) __asm__("func_8009CFC8");
extern s32* D_80139280;
extern s32 D_801B2CB4;
extern s32 D_801B2CB0;
extern u8 D_800D9CB8[];
extern u8 D_80139B68[];

    D_801B2CB4 = 0x80;
    D_80139280[15] = -1;
    D_801B2CB0 += 1;
    func_8009CFC8__for_func_8009CF80();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009CFC8(void)
{
extern void func_8009CFC8(void);
extern s32* D_80139280;
extern s32 D_801B2CB4;
extern s32 D_801B2CB0;
extern u8 D_800D9CB8[];
extern u8 D_80139B68[];

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
extern s32 D_801B2CB0;

    D_801B2CB0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009D06C(s32 arg0)
{
extern u32 D_801B2CB8;
extern s32 D_801B2CBC;
extern void (*D_800D66EC[])(void);
extern u8 D_800DA398[];
extern u8 D_80139CA8[];
extern s32 D_80139280;

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
extern u32 D_801B2CB8;
extern s32 D_801B2CBC;
extern void (*D_800D66EC[])(void);
extern u8 D_800DA398[];
extern u8 D_80139CA8[];
extern s32 D_80139280;

    D_801B2CB8 = 1;
    D_801B2CBC = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009D0FC(void)
{
extern u32 D_801B2CB8;
extern s32 D_801B2CBC;
extern void (*D_800D66EC[])(void);
extern u8 D_800DA398[];
extern u8 D_80139CA8[];
extern s32 D_80139280;

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
extern void func_8009D1D0__for_func_8009D188(void) __asm__("func_8009D1D0");
extern s32* D_80139280;
extern s32 D_801B2CBC;
extern s32 D_801B2CB8;
extern u8 D_800DA398[];
extern u8 D_80139CA8[];

    D_801B2CBC = 0x20;
    D_80139280[25] = -1;
    D_801B2CB8 += 1;
    func_8009D1D0__for_func_8009D188();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009D1D0(void)
{
extern void func_8009D1D0(void);
extern s32* D_80139280;
extern s32 D_801B2CBC;
extern s32 D_801B2CB8;
extern u8 D_800DA398[];
extern u8 D_80139CA8[];

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
extern s32 D_801B2CB8;

    D_801B2CB8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009D274(s32 arg0)
{
extern u32 D_801B2CC0;
extern s32 D_801B2CC4;
extern void (*D_800D6704[])(void);
extern u8 D_800DB158[];
extern u8 D_80139F28[];
extern s32 D_80139280;

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
extern u32 D_801B2CC0;
extern s32 D_801B2CC4;
extern void (*D_800D6704[])(void);
extern u8 D_800DB158[];
extern u8 D_80139F28[];
extern s32 D_80139280;

    D_801B2CC0 = 1;
    D_801B2CC4 = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009D304(void)
{
extern u32 D_801B2CC0;
extern s32 D_801B2CC4;
extern void (*D_800D6704[])(void);
extern u8 D_800DB158[];
extern u8 D_80139F28[];
extern s32 D_80139280;

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
extern void func_8009D3D8__for_func_8009D390(void) __asm__("func_8009D3D8");
extern s32* D_80139280;
extern s32 D_801B2CC4;
extern s32 D_801B2CC0;
extern u8 D_800DB158[];
extern u8 D_80139F28[];

    D_801B2CC4 = 0x20;
    D_80139280[45] = -1;
    D_801B2CC0 += 1;
    func_8009D3D8__for_func_8009D390();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009D3D8(void)
{
extern void func_8009D3D8(void);
extern s32* D_80139280;
extern s32 D_801B2CC4;
extern s32 D_801B2CC0;
extern u8 D_800DB158[];
extern u8 D_80139F28[];

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
extern s32 D_801B2CC0;

    D_801B2CC0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009D47C(s32 arg0)
{
extern u32 D_801B2CC8;
extern s32 D_801B2CCC;
extern void (*D_800D671C[])(void);
extern void func_8009D58C__for_func_8009D47C(void) __asm__("func_8009D58C");
extern u8 D_8011D538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

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
extern u32 D_801B2CC8;
extern s32 D_801B2CCC;
extern void (*D_800D671C[])(void);
extern void func_8009D58C__for_func_8009D4F4(void) __asm__("func_8009D58C");
extern u8 D_8011D538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    D_801B2CC8 = 1;
    D_801B2CCC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8009D50C(void)
{
extern u32 D_801B2CC8;
extern s32 D_801B2CCC;
extern void (*D_800D671C[])(void);
extern void func_8009D58C__for_func_8009D50C(void) __asm__("func_8009D58C");
extern u8 D_8011D538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    D_801399CC = D_8011D538;
    D_800D93C8[0x6] = 0xF;
    *(s16*)&D_800D93C8[0x10] = -1;
    *(s16*)&D_800D93C8[0x26] = 2;
    *(s16*)&D_800D93C8[0x22] = 0x81;
    *(s16*)&D_800D93C8[0x2] = 0;
    *(s16*)&D_800D93C8[0xE] = 0;
    *(s16*)&D_800D93C8[0x24] = 1;
    D_801B2CCC = 0x5A;
    D_801B2CC8 += 1;
    func_8009D58C__for_func_8009D50C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009D58C(void)
{
extern u32 D_801B2CC8;
extern s32 D_801B2CCC;
extern void (*D_800D671C[])(void);
extern void func_8009D58C(void);
extern u8 D_8011D538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_8011CF4C, 0x1E, 0x3, 0);
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
extern s32 D_801B2CC8;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2CEC;
extern s32 D_801B2CE8;

    D_801B2CC8 += 1;
}
