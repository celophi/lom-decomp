#include "wmap_main.h"
#include "wmap_land_effect_04.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_sprite_render.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_effect_primitives.h"

/** @brief Initialize four effect actors and their angular spacing. */
void func_8007085C(void)
{
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
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s32 field_10;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

/** @brief Byte-aligned configuration copied into the effect state. */
typedef struct
{
    u8 bytes[8];
} WmapConfigBytes;
extern WmapConfigBytes D_80139258;
extern WmapConfigBytes D_801B2490;
extern WmapConfigA D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern s32 *D_80139280;
extern u8 D_80121538[];
extern s32 D_801B24E8;
extern s32 D_801B24EC;
extern void func_800720C4__for_func_8007085C(void) __asm__("func_800720C4");

    s32 i;
    s32 *descriptor;
    WmapConfigA *actor;

    D_801B2490 = D_80139258;
    descriptor = D_80139280;
    descriptor[0] = 2;
    descriptor[1] = 1;
    descriptor[2] = 2;
    D_80139280[3] = 0;
    D_80139280[4] = 3500;
    D_80139280[5] = 96;
    D_80139280[6] = 4;
    D_80139280[7] = 0;
    D_80139280[8] = 10;
    D_80139280[9] = 8;
    for (i = 20; i < 80; i++)
    {
        D_801AFBD0[i].state = 0;
    }
    for (i = 20; i < 80; i += 15)
    {
        actor = &D_800D9268[i];
        D_80139988[i].resource = D_80121538;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_02 = 0;
        actor->field_0E = 0;
        actor->field_26 = 4;
        actor->field_22 = 1;
        actor->field_24 = 129;
        D_801AFBD0[i].state = 1;
        D_801AFBD0[i].z = 150000;
        D_801AFBD0[i].angle = D_80139280[3];
        D_801AFBD0[i].field_0E = 0;
        D_80139280[3] += 1024;
    }
    D_801B24EC = 40;
    D_801B24E8++;
    func_800720C4__for_func_8007085C();
}

/** @brief Draw and brighten two rotating layers, then advance their shared countdown. */
void func_800709E8(void)
{
extern u8 D_800DCF18[];
extern VECTOR D_80182DC0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B2468;
extern s32 D_801B24F0;
extern s32 D_801B24F4;

    s32 remaining;
    s32 intensity;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_801B2490);
    func_8006CD98(D_800DCF18, 0, 16, 53, 0x7800, 1, D_801B2468);
    D_801B2490.vz = (u16)(D_801B2490.vz + 12);
    func_8006CFA8(&D_80182DC0, &D_801B2498);
    func_8006CD98(D_800DCF18, 0, 16, 53, 0x7800, 1, D_801B2468);
    D_801B2498.vz = (u16)(D_801B2498.vz - 4);
    PopMatrix();
    intensity = D_801B2468 + 8;
    D_801B2468 = intensity;
    if (intensity >= 130)
    {
        D_801B2468 = 129;
    }
    remaining = D_801B24F4 - 1;
    D_801B24F4 = remaining;
    if (remaining == 0)
    {
        D_801B24F0++;
    }
}

/** @brief Draw and fade two rotating layers, then advance their shared countdown. */
void func_80070B28(void)
{
extern u8 D_800DCF18[];
extern VECTOR D_80182DC0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B2468;
extern s32 D_801B24F0;
extern s32 D_801B24F4;

    s32 remaining;
    s32 intensity;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_801B2490);
    func_8006CD98(D_800DCF18, 0, 16, 53, 0x7800, 1, D_801B2468);
    D_801B2490.vz = (u16)(D_801B2490.vz + 12);
    func_8006CFA8(&D_80182DC0, &D_801B2498);
    func_8006CD98(D_800DCF18, 0, 16, 53, 0x7800, 1, D_801B2468);
    D_801B2498.vz = (u16)(D_801B2498.vz - 4);
    PopMatrix();
    intensity = D_801B2468 - 4;
    D_801B2468 = intensity;
    if (intensity < 0)
    {
        D_801B2468 = 0;
    }
    remaining = D_801B24F4 - 1;
    D_801B24F4 = remaining;
    if (remaining == 0)
    {
        D_801B24F0++;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80070C60(void)
{
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern s32 D_8011CF1C;
extern s32 D_801B24FC;
extern s32 D_801B24F8;

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
        D_80182DE8 -= 1;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--D_801B24FC == 0)
    {
        D_801B24F8 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80070D60(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF24;
extern s32 D_801B2504;
extern s32 D_801B2500;

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
    if (--D_801B2504 == 0)
    {
        D_801B2500 += 1;
    }
}

/** @brief Initialize randomized actors along a cosine depth curve. */
void func_80070E60(void)
{
/* Partial WMAP decompilation: 88.176470% (gcc280_g0). */

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
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s32 field_10;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

extern WmapConfigA D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern u8 D_80121538[];
extern s32 D_801B2508;
extern s32 D_801B250C;
extern s32 rand(void);
extern s32 ccos(s32);
extern void func_80070FF8__for_func_80070E60(void) __asm__("func_80070FF8");

    s32 i;
    WmapConfigA *actor;
    s32 phase;
    s32 spread;
    s32 motion_offset;
    WmapMotion *motion;

    i = 100;
    phase = 0;
    spread = 0;
    motion_offset = 100 * sizeof(WmapMotion);
    do
    {
        actor = &D_800D9268[i];
        D_80139988[i].resource = D_80121538;
        actor->field_06 = 15;
        actor->field_0E = 2;
        actor->field_10 = -1;
        actor->field_02 = 0;
        actor->field_26 = 1;
        actor->field_22 = 129;
        actor->field_24 = 1;
        motion = (WmapMotion *)((u8 *)D_801AFBD0 + motion_offset);
        motion_offset += sizeof(WmapMotion);
        motion->angle = rand() & 4095;
        motion->field_0E = spread / 30;
        motion->z = ccos(2048 - (phase / 30)) * 140;
        i++;
        motion->x = ((rand() * 100) >> 15) + 40;
        phase += 1024;
        spread += 140;
    } while (i < 130);
    D_801B250C = 64;
    D_801B2508++;
    func_80070FF8__for_func_80070E60();
}

/**
 * @brief Project and draw the world-map star field, spinning each entry each frame.
 * @see decomp.me (100%)
 */
void func_80070FF8(void)
{
/** @brief World-map orbiting star: polar position, spin angle, and radius. */
typedef struct
{
    s16 unk00;
    s16 angle;
    u16 delta;
    s16 unk06;
    s32 radius;
    s16 unk0C;
    u16 unk0E;
    s16 unk10;
    s16 unk12;
} WmapStar;

/** @brief World-map draw record; only used opaquely by the primitive helpers. */
typedef struct
{
    u8 pad[0x2C];
} WmapDraw;

extern WmapStar D_801AFBD0[];
extern WmapDraw D_800D9268[];
extern u8 D_80139988[];
extern s32 D_801B2508;
extern s32 D_801B250C;

    SVECTOR position;
    s32 screen;
    WmapStar* star;
    WmapDraw* draw;
    s32 i;

    for (i = 0x64; i < 0x82; i++)
    {
        draw = &D_800D9268[i];
        star = &D_801AFBD0[i];
        position.vx = ((star->radius >> 6) * (ccos(star->angle) >> 6)) >> 0xC;
        position.vy = ((star->radius >> 6) * (csin(star->angle) >> 6)) >> 0xC;
        position.vz = star->unk0E;
        gte_ldv0(&position);
        gte_rtps();
        func_8006CC4C(draw, &D_80139988[i * 8]);
        gte_stsxy(&screen);
        if (star->angle != 0)
        {
            func_80066F9C(draw, screen, 8, 4, 0);
        }
        else
        {
            func_80066F9C(draw, screen, 8, 0, 0);
        }
        star->angle = ((u16)star->angle + star->delta) & 0xFFF;
    }
    if (--D_801B250C == 0)
    {
        D_801B2508 += 1;
    }
}

/**
 * @brief Project and draw the world-map star field, spinning each entry each frame.
 * @see decomp.me (100%)
 */
void func_8007115C(void)
{
/** @brief World-map orbiting star: polar position, spin angle, and radius. */
typedef struct
{
    s16 unk00;
    s16 angle;
    u16 delta;
    s16 unk06;
    s32 radius;
    s16 unk0C;
    u16 unk0E;
    s16 unk10;
    s16 unk12;
} WmapStar;

/** @brief World-map draw record; only used opaquely by the primitive helpers. */
typedef struct
{
    u8 pad[0x2C];
} WmapDraw;

extern WmapStar D_801AFBD0[];
extern WmapDraw D_800D9268[];
extern u8 D_80139988[];
extern s32 D_801B2508;
extern s32 D_801B250C;

    SVECTOR position;
    s32 screen;
    WmapStar* star;
    WmapDraw* draw;
    s32 i;

    for (i = 0x64; i < 0x82; i++)
    {
        draw = &D_800D9268[i];
        star = &D_801AFBD0[i];
        position.vx = ((star->radius >> 6) * (ccos(star->angle) >> 6)) >> 0xC;
        position.vy = ((star->radius >> 6) * (csin(star->angle) >> 6)) >> 0xC;
        position.vz = star->unk0E;
        gte_ldv0(&position);
        gte_rtps();
        func_8006CC4C(draw, &D_80139988[i * 8]);
        gte_stsxy(&screen);
        if (star->angle != 0)
        {
            func_80066F9C(draw, screen, 8, 4, 0);
        }
        else
        {
            func_80066F9C(draw, screen, 8, 0, 0);
        }
        star->angle = ((u16)star->angle + star->delta) & 0xFFF;
    }
    if (--D_801B250C == 0)
    {
        D_801B2508 += 1;
    }
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800712C0(s32 arg0)
{
extern u32 D_801B24B8;
extern s32 D_801B24BC;
extern void (*D_800D4DD0[])(void);
extern void func_80071394__for_func_800712C0(void) __asm__("func_80071394");
extern s32 D_8013B20C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B24B8 = 1;
        D_801B24BC = 1;
        return 1;
    }

    if (D_801B24B8 < 0x6)
    {
        D_800D4DD0[D_801B24B8]();
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
void func_80071338(void)
{
extern u32 D_801B24B8;
extern s32 D_801B24BC;
extern void (*D_800D4DD0[])(void);
extern void func_80071394__for_func_80071338(void) __asm__("func_80071394");
extern s32 D_8013B20C;

    D_801B24B8 = 1;
    D_801B24BC = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80071350(void)
{
extern u32 D_801B24B8;
extern s32 D_801B24BC;
extern void (*D_800D4DD0[])(void);
extern void func_80071394__for_func_80071350(void) __asm__("func_80071394");
extern s32 D_8013B20C;

    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B24B8 += 1;
    func_80071394__for_func_80071350();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80071394(void)
{
extern s32 D_8013B20C;
extern s32 D_801B24B8;
extern void func_800713D0__for_func_80071394(void) __asm__("func_800713D0");
extern void func_80071468__for_func_80071394(void) __asm__("func_80071468");
extern void func_80071414__for_func_80071394(void) __asm__("func_80071414");

    if (D_8013B20C == 0)
    {
        D_801B24B8 += 1;
        func_800713D0__for_func_80071394();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_800713D0(void)
{
extern s32 D_8013B20C;
extern s32 D_801B24B8;
extern void func_800713D0(void);
extern void func_80071468__for_func_800713D0(void) __asm__("func_80071468");
extern void func_80071414__for_func_800713D0(void) __asm__("func_80071414");

    func_8006CAC0(func_80071468__for_func_800713D0);
    D_8013B20C = 1;
    D_801B24B8 += 1;
    func_80071414__for_func_800713D0();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80071414(void)
{
extern s32 D_801B24B8;
extern s32 D_8013B20C;
extern void func_80071450__for_func_80071414(void) __asm__("func_80071450");

    if (D_8013B20C == 0)
    {
        D_801B24B8 += 1;
        func_80071450__for_func_80071414();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80071450(void)
{
extern s32 D_801B24B8;
extern s32 D_8013B20C;
extern void func_80071450(void);

    D_801B24B8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80071468(s32 arg0)
{
extern u32 D_801B24C0;
extern s32 D_801B24C4;
extern void (*D_800D4DE8[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B24C0 = 1;
        D_801B24C4 = 1;
        return 1;
    }

    if (D_801B24C0 < 0x14)
    {
        D_800D4DE8[D_801B24C0]();
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
void func_800714E0(void)
{
extern u32 D_801B24C0;
extern s32 D_801B24C4;
extern void (*D_800D4DE8[])(void);

    D_801B24C0 = 1;
    D_801B24C4 = 1;
}

/** @brief World-map step handler: kick a sub-task and advance the step counter. */
void func_800714F8(void)
{
extern s32 D_8013B208;
extern s32 D_801B24C0;
extern s32 D_801B24C4;

    D_8013B208 = 1;
    func_8006683C(0x701040);
    g_wmap_backdrop_target_level = 4;
    D_801B24C4 = 0x14;
    D_801B24C0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007154C(void)
{
extern s32 D_801B24C4;
extern s32 D_801B24C0;

    if (--D_801B24C4 == 0)
    {
        D_801B24C0 += 1;
    }
}

/** @brief World-map step handler: register a callback and advance the step counter. */
void func_80071580(void)
{
extern void func_80071CEC__for_func_80071580(void) __asm__("func_80071CEC");
extern s32 D_801B24C0;
extern s32 D_801B24C4;

    func_800652A8(0x24, 0x80);
    func_8006CAC0(func_80071CEC__for_func_80071580);
    D_801B24C4 = 4;
    D_801B24C0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800715C8(void)
{
extern s32 D_801B24C4;
extern s32 D_801B24C0;
extern void func_80072034__for_func_800715C8(void) __asm__("func_80072034");

    if (--D_801B24C4 == 0)
    {
        D_801B24C0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800715FC(void)
{
extern s32 D_801B24C4;
extern s32 D_801B24C0;
extern void func_80072034__for_func_800715FC(void) __asm__("func_80072034");

    func_8006CAC0(func_80072034__for_func_800715FC);
    D_801B24C4 = 0x1E;
    D_801B24C0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80071638(void)
{
extern s32 D_801B24C4;
extern s32 D_801B24C0;

    if (--D_801B24C4 == 0)
    {
        D_801B24C0 += 1;
    }
}

/** @brief Register two callbacks, set sequence flags, and begin a one-tick delay. */
void func_8007166C(void)
{
extern s32 D_80139244;
extern s32 D_801ADAE0;
extern s32 D_801B24C0;
extern s32 D_801B24C4;
extern void func_80071B48__for_func_8007166C(void) __asm__("func_80071B48");
extern void func_8007228C__for_func_8007166C(void) __asm__("func_8007228C");

    func_8006CAC0(&func_8007228C__for_func_8007166C);
    g_wmap_backdrop_target_level = 2;
    D_80139244 = 1;
    func_8006CAC0(&func_80071B48__for_func_8007166C);
    D_801ADAE0 = 1;
    D_801B24C4 = 1;
    D_801B24C0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800716D8(void)
{
extern s32 D_801B24C4;
extern s32 D_801B24C0;
extern void func_80072138__for_func_800716D8(void) __asm__("func_80072138");

    if (--D_801B24C4 == 0)
    {
        D_801B24C0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007170C(void)
{
extern s32 D_801B24C4;
extern s32 D_801B24C0;
extern void func_80072138__for_func_8007170C(void) __asm__("func_80072138");

    func_8006CAC0(func_80072138__for_func_8007170C);
    D_801B24C4 = 0x3;
    D_801B24C0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80071748(void)
{
extern s32 D_801B24C4;
extern s32 D_801B24C0;
extern void func_800719A8__for_func_80071748(void) __asm__("func_800719A8");

    if (--D_801B24C4 == 0)
    {
        D_801B24C0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007177C(void)
{
extern s32 D_801B24C4;
extern s32 D_801B24C0;
extern void func_800719A8__for_func_8007177C(void) __asm__("func_800719A8");

    func_8006CAC0(func_800719A8__for_func_8007177C);
    D_801B24C4 = 0x18;
    D_801B24C0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800717B8(void)
{
extern s32 D_801B24C4;
extern s32 D_801B24C0;
extern void func_80072538__for_func_800717B8(void) __asm__("func_80072538");

    if (--D_801B24C4 == 0)
    {
        D_801B24C0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800717EC(void)
{
extern s32 D_801B24C4;
extern s32 D_801B24C0;
extern void func_80072538__for_func_800717EC(void) __asm__("func_80072538");

    func_8006CAC0(func_80072538__for_func_800717EC);
    D_801B24C4 = 0x1E;
    D_801B24C0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80071828(void)
{
extern s32 D_801B24C4;
extern s32 D_801B24C0;
extern void func_80071E94__for_func_80071828(void) __asm__("func_80071E94");

    if (--D_801B24C4 == 0)
    {
        D_801B24C0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007185C(void)
{
extern s32 D_801B24C4;
extern s32 D_801B24C0;
extern void func_80071E94__for_func_8007185C(void) __asm__("func_80071E94");

    func_8006CAC0(func_80071E94__for_func_8007185C);
    D_801B24C4 = 0x3C;
    D_801B24C0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80071898(void)
{
extern s32 D_801B24C4;
extern s32 D_801B24C0;

    if (--D_801B24C4 == 0)
    {
        D_801B24C0 += 1;
    }
}

/** @brief Register a callback, update the selected map cell, and begin a 68-tick delay. */
void func_800718CC(void)
{
/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

extern s32 D_8011D4FC;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern s32 D_80139244;
extern WmapValueRecord D_80139290[][6];
extern s32 D_801B24C0;
extern s32 D_801B24C4;
extern void func_800723E0__for_func_800718CC(void) __asm__("func_800723E0");

    func_8006CAC0(&func_800723E0__for_func_800718CC);
    D_80139244 = 0;
    D_801B24C4 = 0x44;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B24C0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80071958(void)
{
extern s32 D_801B24C4;
extern s32 D_801B24C0;

    if (--D_801B24C4 == 0)
    {
        D_801B24C0 += 1;
    }
}

/**
 * @brief World-map step handler: clear the shared flag and advance the step counter.
 */
void func_8007198C(void)
{
extern s32 D_801B24C0;
extern s32 D_8013B20C;

    D_8013B20C = 0;
    D_801B24C0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800719A8(s32 arg0)
{
extern u32 D_801B24C8;
extern s32 D_801B24CC;
extern void (*D_800D4E38[])(void);
extern void func_80071AB4__for_func_800719A8(void) __asm__("func_80071AB4");
extern u8 D_8011F538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B24C8 = 1;
        D_801B24CC = 1;
        return 1;
    }

    if (D_801B24C8 < 0x4)
    {
        D_800D4E38[D_801B24C8]();
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
void func_80071A20(void)
{
extern u32 D_801B24C8;
extern s32 D_801B24CC;
extern void (*D_800D4E38[])(void);
extern void func_80071AB4__for_func_80071A20(void) __asm__("func_80071AB4");
extern u8 D_8011F538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    D_801B24C8 = 1;
    D_801B24CC = 1;
}

void func_80071A38(void)
{
extern u32 D_801B24C8;
extern s32 D_801B24CC;
extern void (*D_800D4E38[])(void);
extern void func_80071AB4__for_func_80071A38(void) __asm__("func_80071AB4");
extern u8 D_8011F538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    D_801399AC = D_8011F538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 8;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x22] = 0x80;
    *(s16*)&D_800D9318[0x24] = 0;
    D_801B24CC = 0x7C;
    D_801B24C8 += 1;
    func_80071AB4__for_func_80071A38();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80071AB4(void)
{
extern u32 D_801B24C8;
extern s32 D_801B24CC;
extern void (*D_800D4E38[])(void);
extern void func_80071AB4(void);
extern u8 D_8011F538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0xB, 0x9, 0);
    if (--D_801B24CC == 0)
    {
        D_801B24C8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80071B30(void)
{
extern s32 D_801B24C8;

    D_801B24C8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80071B48(s32 arg0)
{
extern u32 D_801B24D0;
extern s32 D_801B24D4;
extern void (*D_800D4E48[])(void);
extern void func_80071C58__for_func_80071B48(void) __asm__("func_80071C58");
extern u8 D_8011F538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B24D0 = 1;
        D_801B24D4 = 1;
        return 1;
    }

    if (D_801B24D0 < 0x4)
    {
        D_800D4E48[D_801B24D0]();
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
void func_80071BC0(void)
{
extern u32 D_801B24D0;
extern s32 D_801B24D4;
extern void (*D_800D4E48[])(void);
extern void func_80071C58__for_func_80071BC0(void) __asm__("func_80071C58");
extern u8 D_8011F538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    D_801B24D0 = 1;
    D_801B24D4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80071BD8(void)
{
extern u32 D_801B24D0;
extern s32 D_801B24D4;
extern void (*D_800D4E48[])(void);
extern void func_80071C58__for_func_80071BD8(void) __asm__("func_80071C58");
extern u8 D_8011F538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    s32 one = 1;

    D_801399BC = D_8011F538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0xE] = 1;
    *(s16*)&D_800D9370[0x10] = -one;
    *(s16*)&D_800D9370[0x26] = 2;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0x22] = 0x81;
    *(s16*)&D_800D9370[0x24] = 1;
    D_801B24D4 = 0x7C;
    D_801B24D0 += one;
    func_80071C58__for_func_80071BD8();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80071C58(void)
{
extern u32 D_801B24D0;
extern s32 D_801B24D4;
extern void (*D_800D4E48[])(void);
extern void func_80071C58(void);
extern u8 D_8011F538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0xB, 0xA, 0);
    if (--D_801B24D4 == 0)
    {
        D_801B24D0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80071CD4(void)
{
extern s32 D_801B24D0;

    D_801B24D0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80071CEC(s32 arg0)
{
extern u32 D_801B24D8;
extern s32 D_801B24DC;
extern void (*D_800D4E58[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B24D8 = 1;
        D_801B24DC = 1;
        return 1;
    }

    if (D_801B24D8 < 0x4)
    {
        D_800D4E58[D_801B24D8]();
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
void func_80071D64(void)
{
extern u32 D_801B24D8;
extern s32 D_801B24DC;
extern void (*D_800D4E58[])(void);

    D_801B24D8 = 1;
    D_801B24DC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80071D7C(void)
{
extern u8* D_801399C4;
extern u8 D_8011F538[];
extern u8 D_800D939C[];
extern s32 D_801B24D8;
extern s32 D_801B24DC;
extern void func_80071E00__for_func_80071D7C(void) __asm__("func_80071E00");

    D_801399C4 = D_8011F538;
    D_800D939C[0x6] = 0xF;
    *(s16*)&D_800D939C[0xE] = 2;
    *(s16*)&D_800D939C[0x10] = -1;
    *(s16*)&D_800D939C[0x26] = 0x10;
    *(s16*)&D_800D939C[0x22] = 0x81;
    *(s16*)&D_800D939C[0x2] = 0;
    *(s16*)&D_800D939C[0x24] = 1;
    D_801B24DC = 0x28;
    D_801B24D8 += 1;
    func_80071E00__for_func_80071D7C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80071E00(void)
{
extern s32 D_801B24D8;
extern u8 D_800D939C[];
extern u8 D_801399C0[];
extern s32 D_8011CF4C;
extern s32 D_801B24DC;

    func_8006CC4C(D_800D939C, D_801399C0);
    func_80066F9C(D_800D939C, D_8011CF4C, 0xB, 0x2, 0);
    if (--D_801B24DC == 0)
    {
        D_801B24D8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80071E7C(void)
{
extern s32 D_801B24D8;
extern u8 D_800D939C[];
extern u8 D_801399C0[];
extern s32 D_8011CF4C;
extern s32 D_801B24DC;

    D_801B24D8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80071E94(s32 arg0)
{
extern u32 D_801B24E0;
extern s32 D_801B24E4;
extern void (*D_800D4E68[])(void);
extern void func_80071FA0__for_func_80071E94(void) __asm__("func_80071FA0");
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B24E0 = 1;
        D_801B24E4 = 1;
        return 1;
    }

    if (D_801B24E0 < 0x4)
    {
        D_800D4E68[D_801B24E0]();
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
void func_80071F0C(void)
{
extern u32 D_801B24E0;
extern s32 D_801B24E4;
extern void (*D_800D4E68[])(void);
extern void func_80071FA0__for_func_80071F0C(void) __asm__("func_80071FA0");
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801B24E0 = 1;
    D_801B24E4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80071F24(void)
{
extern u32 D_801B24E0;
extern s32 D_801B24E4;
extern void (*D_800D4E68[])(void);
extern void func_80071FA0__for_func_80071F24(void) __asm__("func_80071FA0");
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
    D_801B24E4 = 0x3D;
    D_801B24E0 += 1;
    func_80071FA0__for_func_80071F24();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80071FA0(void)
{
extern u32 D_801B24E0;
extern s32 D_801B24E4;
extern void (*D_800D4E68[])(void);
extern void func_80071FA0(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x1C, 0x8, 0);
    if (--D_801B24E4 == 0)
    {
        D_801B24E0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007201C(void)
{
extern s32 D_801B24E0;

    D_801B24E0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80072034(s32 arg0)
{
extern u32 D_801B24E8;
extern s32 D_801B24EC;
extern void (*D_800D4E78[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B24E8 = 1;
        D_801B24EC = 1;
        return 1;
    }

    if (D_801B24E8 < 0x4)
    {
        D_800D4E78[D_801B24E8]();
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
void func_800720AC(void)
{
extern u32 D_801B24E8;
extern s32 D_801B24EC;
extern void (*D_800D4E78[])(void);

    D_801B24E8 = 1;
    D_801B24EC = 1;
}

/** @brief Draw the sequence effect and advance when its countdown expires. */
void func_800720C4(void)
{
extern s32 D_80139280;
extern s32 D_801B24E8;
extern s32 D_801B24EC;

    s32 remaining_ticks;

    func_8006BC44(0x14, 0x3C, D_80139280, 0);
    remaining_ticks = D_801B24EC - 1;
    D_801B24EC = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B24E8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80072120(void)
{
extern s32 D_801B24E8;

    D_801B24E8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80072138(s32 arg0)
{
extern u32 D_801B24F0;
extern s32 D_801B24F4;
extern void (*D_800D4E88[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B24F0 = 1;
        D_801B24F4 = 1;
        return 1;
    }

    if (D_801B24F0 < 0x6)
    {
        D_800D4E88[D_801B24F0]();
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
void func_800721B0(void)
{
extern u32 D_801B24F0;
extern s32 D_801B24F4;
extern void (*D_800D4E88[])(void);

    D_801B24F0 = 1;
    D_801B24F4 = 1;
}

/** @brief World-map step: reset counters and advance to the next handler. */
void func_800721C8(void)
{
extern s32 D_801B24B0;
extern s32 D_801B2468;
extern s16 D_801B2490[];
extern s16 D_801B2498[];
extern s32 D_801B24F0;
extern s32 D_801B24F4;
extern void func_800709E8__for_func_800721C8(void) __asm__("func_800709E8");

    D_801B24B0 = 0;
    D_801B2468 = 1;
    D_801B2490[0] = 0;
    D_801B2490[1] = 0;
    D_801B2490[2] = 0;
    D_801B2498[0] = 0;
    D_801B2498[1] = 0;
    D_801B2498[2] = 0;
    D_801B24F4 = 0x54;
    D_801B24F0 += 1;
    func_800709E8__for_func_800721C8();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007223C(void)
{
extern s32 D_801B24F0;
extern void func_80070B28__for_func_8007223C(void) __asm__("func_80070B28");
extern s32 D_801B24F4;

    D_801B24F4 = 0x20;
    D_801B24F0 += 1;
    func_80070B28__for_func_8007223C();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80072274(void)
{
extern s32 D_801B24F0;
extern void func_80070B28__for_func_80072274(void) __asm__("func_80070B28");
extern s32 D_801B24F4;

    D_801B24F0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007228C(s32 arg0)
{
extern u32 D_801B24F8;
extern s32 D_801B24FC;
extern void (*D_800D4EA0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B24F8 = 1;
        D_801B24FC = 1;
        return 1;
    }

    if (D_801B24F8 < 0x4)
    {
        D_800D4EA0[D_801B24F8]();
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
void func_80072304(void)
{
extern u32 D_801B24F8;
extern s32 D_801B24FC;
extern void (*D_800D4EA0[])(void);

    D_801B24F8 = 1;
    D_801B24FC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8007231C(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern s32 D_801B24FC;
extern u32 D_801B24F8;
extern void func_80070C60__for_func_8007231C(void) __asm__("func_80070C60");

    D_801B24A0 = D_80139258;
    D_801B2650 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B24FC = 0x80;
    D_801B24F8 += 1;
    func_80070C60__for_func_8007231C();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800723C8(void)
{
extern s32 D_801B24F8;

    D_801B24F8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800723E0(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2500;
extern s32 D_801B2504;
extern void (*D_800D4EB0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_80070D60__for_func_800723E0(void) __asm__("func_80070D60");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2500 = 1;
        D_801B2504 = 1;
        return 1;
    }

    if (D_801B2500 < 0x4)
    {
        D_800D4EB0[D_801B2500]();
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
void func_80072458(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2500;
extern s32 D_801B2504;
extern void (*D_800D4EB0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_80070D60__for_func_80072458(void) __asm__("func_80070D60");

    D_801B2500 = 1;
    D_801B2504 = 1;
}

void func_80072470(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2500;
extern s32 D_801B2504;
extern void (*D_800D4EB0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_80070D60__for_func_80072470(void) __asm__("func_80070D60");

    D_801B24A8 = D_80139258;
    D_801B2478 = D_80182DC0;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B2504 = 0x40;
    D_801B2500 += 1;
    func_80070D60__for_func_80072470();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80072520(void)
{
extern s32 D_801B2500;

    D_801B2500 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80072538(s32 arg0)
{
extern u32 D_801B2508;
extern s32 D_801B250C;
extern void (*D_800D4EC0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2508 = 1;
        D_801B250C = 1;
        return 1;
    }

    if (D_801B2508 < 0x6)
    {
        D_800D4EC0[D_801B2508]();
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
void func_800725B0(void)
{
extern u32 D_801B2508;
extern s32 D_801B250C;
extern void (*D_800D4EC0[])(void);

    D_801B2508 = 1;
    D_801B250C = 1;
}

/**
 * @brief Reset a range of world-map actor configs, arm the timer, and advance the step.
 */
void func_800725C8(void)
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
extern s32 D_801B2508;
extern s32 D_801B250C;
extern void func_8007115C__for_func_800725C8(void) __asm__("func_8007115C");

    s32 i;

    for (i = 0x64; i < 0x82; i++)
    {
        D_800D9268[i].field_22 = 0;
        D_800D9268[i].field_26 = 2;
    }
    D_801B250C = 0x40;
    D_801B2508 += 1;
    func_8007115C__for_func_800725C8();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007262C(void)
{
extern s32 D_801B2508;

    D_801B2508 += 1;
}
