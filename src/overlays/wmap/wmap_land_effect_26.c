#include "wmap_land_effect_26.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_sprite_render.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_effect_primitives.h"

/** @brief Approach the effect depth and draw its alternating-brightness fade. */
void func_80076C6C(void)
{
extern u8 D_800DCF18[];
extern s32 D_8011CF74;
extern s32 D_801B2470;
extern SVECTOR D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B25F8;
extern s32 D_801B25FC;
extern void func_800675F0(void *, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = D_801B2650.vz - 10000;
    D_801B2650.vz = depth;
    if (depth < 1000)
    {
        D_801B2650.vz = 1000;
    }
    PushMatrix();
    func_8006CFA8(&D_801B2650, &D_801B24A0);
    if (D_801B2470 != 0)
    {
        if (D_8011CF74 & 1)
        {
            func_800675F0(D_800DCF18, 0, 4, -1, -1, 1, D_801B2470, 5, -20, -1);
        }
        else
        {
            func_800675F0(D_800DCF18, 0, 4, -1, -1, 1, D_801B2470 / 2, 5, -20, -1);
        }
        intensity = D_801B2470 - 2;
        D_801B2470 = intensity;
        if (intensity < 0)
        {
            D_801B2470 = 0;
        }
    }
    PopMatrix();
    remaining = D_801B25FC - 1;
    D_801B25FC = remaining;
    if (remaining == 0)
    {
        D_801B25F8++;
    }
}

/** @brief Move the effect toward the camera while fading it and advancing its countdown. */
void func_80076DBC(void)
{
extern void *D_8011CF1C;
extern s32 D_801B2474;
extern VECTOR D_801B2478;
extern SVECTOR D_801B24A8;
extern s32 D_801B2600;
extern s32 D_801B2604;
extern void func_800675F0(void *, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = D_801B2478.vz - 2500;
    D_801B2478.vz = depth;
    if (depth < 100)
    {
        D_801B2478.vz = 100;
    }
    PushMatrix();
    func_8006CFA8(&D_801B2478, &D_801B24A8);
    if (D_801B2474 != 0)
    {
        func_800675F0(D_8011CF1C, 0, 4, -1, -1, 1, D_801B2474, 5, -20, -1);
    }
    PopMatrix();
    intensity = D_801B2474 - 8;
    D_801B2474 = intensity;
    if (intensity < 0)
    {
        D_801B2474 = 0;
    }
    remaining = D_801B2604 - 1;
    D_801B2604 = remaining;
    if (remaining == 0)
    {
        D_801B2600++;
    }
}

/** @brief Draw and fade the transformed effect, then advance its countdown. */
void func_80076EB4(void)
{
extern void *D_8011CF24;
extern VECTOR D_80139870;
extern VECTOR D_8011CF60;
extern SVECTOR D_8013B238;
extern s32 D_801B2608;
extern s32 D_801B260C;
extern s32 D_801B24B4;

    MATRIX matrix;
    s32 intensity;
    s32 remaining;

    if (D_80139870.vz < 10)
    {
        D_80139870.vz = 10;
    }
    PushMatrix();
    RotMatrix(&D_8013B238, &matrix);
    TransMatrix(&matrix, &D_8011CF60);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    if (D_801B24B4 != 0)
    {
        func_8006CD98(D_8011CF24, 0, 4, -1, -1, 1, D_801B24B4);
        intensity = D_801B24B4 - 8;
        D_801B24B4 = intensity;
        if (intensity < 0)
        {
            D_801B24B4 = 0;
        }
    }
    PopMatrix();
    remaining = D_801B260C - 1;
    D_801B260C = remaining;
    if (remaining == 0)
    {
        D_801B2608++;
    }
}

/** @brief World-map step handler: decay a value, draw the model while active, then countdown-advance. */
void func_80076FAC(void)
{
extern s32 D_80139888[];
extern s32 D_8013B240;
extern s32 D_80182DE4;
extern s32 D_8011CF28;
extern s32 D_801B2614;
extern s32 D_801B2610;

    s32 v1;

    v1 = D_80139888[2] - 0x5DC;
    D_80139888[2] = v1;
    if (v1 < 0x9C40)
    {
        D_80139888[2] = 0x9C40;
    }

    PushMatrix();
    func_8006CFA8(D_80139888, &D_8013B240);

    if (D_80182DE4 != 0)
    {
        func_8006CD98(D_8011CF28, 0, 0xC, 0x35, 0x7800, 1, D_80182DE4);
        if (D_80182DE4 < 0)
        {
            D_80182DE4 = 0;
        }
    }

    PopMatrix();
    if (--D_801B2614 == 0)
    {
        D_801B2610 += 1;
    }
}

/** @brief World-map step handler: decay a value, draw the model while active, then countdown-advance. */
void func_8007708C(void)
{
extern s32 D_80139898[];
extern s32 D_801B2670;
extern s32 D_80182DE8;
extern s32 D_8011CF2C;
extern s32 D_801B261C;
extern s32 D_801B2618;

    s32 v1;

    v1 = D_80139898[2] - 0x5DC;
    D_80139898[2] = v1;
    if (v1 < 0x9C40)
    {
        D_80139898[2] = 0x9C40;
    }

    PushMatrix();
    func_8006CFA8(D_80139898, &D_801B2670);

    if (D_80182DE8 != 0)
    {
        func_8006CD98(D_8011CF2C, 0, 0xC, 0x35, 0x7800, 1, D_80182DE8);
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--D_801B261C == 0)
    {
        D_801B2618 += 1;
    }
}

/** @brief World-map step handler: decay a value, draw the model while active, then countdown-advance. */
void func_8007716C(void)
{
extern s32 D_801B2660[];
extern s32 D_801B2678;
extern s32 D_80182DEC;
extern s32 D_8011CF30;
extern s32 D_801B2624;
extern s32 D_801B2620;

    s32 v1;

    v1 = D_801B2660[2] - 0x5DC;
    D_801B2660[2] = v1;
    if (v1 < 0x9C40)
    {
        D_801B2660[2] = 0x9C40;
    }

    PushMatrix();
    func_8006CFA8(D_801B2660, &D_801B2678);

    if (D_80182DEC != 0)
    {
        func_8006CD98(D_8011CF30, 0, 0xC, 0x35, 0x7800, 1, D_80182DEC);
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    if (--D_801B2624 == 0)
    {
        D_801B2620 += 1;
    }
}

/** @brief Initialize randomized motion for the effect actors. */
void func_8007724C(void)
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
    s16 field_10;
    s16 field_12;
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
extern s32 D_80182DF4;
extern s32 D_801B2648;
extern s32 D_801B264C;
extern u32 rand(void);
extern void func_80078CB4__for_func_8007724C(void) __asm__("func_80078CB4");

    s32 i;
    WmapConfigA *actor;

    D_80182DF4 = 1;
    for (i = 124; i < 170; i++)
    {
        actor = &D_800D9268[i];
        D_801AFBD0[i].state = 1;
        D_801AFBD0[i].angle = (rand() * 155) >> 10;
        D_801AFBD0[i].field_0E = (rand() * 7) >> 6;
        D_801AFBD0[i].x = ((s32)(rand() << 6) >> 15) + 16;
        D_801AFBD0[i].scale = rand() >> 9;
        D_801AFBD0[i].field_10 = ((s32)(rand() << 6) >> 15) + 4;
        D_801AFBD0[i].field_12 = 0;
        D_80139988[i].resource = D_80121538;
        actor->field_06 = 15;
        actor->field_02 = 0;
        actor->field_10 = -1;
        actor->field_26 = 0;
        actor->field_22 = D_80182DF4;
        actor->field_24 = D_80182DF4;
        actor->field_0E = i % 3;
    }
    D_801B264C = 16;
    D_801B2648++;
    func_80078CB4__for_func_8007724C();
}

/** @brief Advance oscillating actors and draw their sprites.
 * @param start First actor index.
 * @param end Exclusive end index.
 * @param depth Rendering depth.
 */
void func_800773B8(s32 start, s32 end, s32 depth)
{
/* Partial WMAP decompilation: 86.000000% (gcc280_g0). */

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
    s16 field_10;
    s16 field_12;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

/** @brief Packed screen coordinates passed to the sprite renderer. */
typedef union
{
    s32 packed;
    struct
    {
        s16 x, y;
    } point;
} WmapScreenPoint;
extern WmapConfigA D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern s8 D_80051B4C[];
extern u16 D_80182DF4;

    s32 i;
    s32 motion_offset;
    s32 x;
    s32 angle;
    WmapMotion *motion;
    WmapConfigA *actor;
    WmapScreenPoint screen;

    motion_offset = start * 20;
    for (i = start; i < end; i++)
    {
        motion = (WmapMotion *)((u8 *)D_801AFBD0 + motion_offset);
        motion_offset += 20;
        actor = &D_800D9268[i];
        motion->field_0E += motion->x;
        if (motion->field_0E >= 3841)
        {
            motion->field_0E = 0;
        }
        motion->field_12 = (motion->field_12 + motion->field_10) & 4095;
        angle = motion->angle;
        x = D_80051B4C[motion->field_12 / 16] * motion->scale;
        screen.point.x = (angle + x / 16) / 16;
        screen.point.y = motion->field_0E / 16;
        actor->field_22 = D_80182DF4;
        actor->field_24 = D_80182DF4;
        func_8006CC4C(actor, &D_80139988[i]);
        func_80066F9C(actor, screen.packed, depth, 4, 0);
    }
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80077560(s32 arg0)
{
extern u32 D_801B25E8;
extern s32 D_801B25EC;
extern void (*D_800D5148[])(void);
extern void func_80077634__for_func_80077560(void) __asm__("func_80077634");
extern s32 D_8013B20C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B25E8 = 1;
        D_801B25EC = 1;
        return 1;
    }

    if (D_801B25E8 < 0x6)
    {
        D_800D5148[D_801B25E8]();
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
void func_800775D8(void)
{
extern u32 D_801B25E8;
extern s32 D_801B25EC;
extern void (*D_800D5148[])(void);
extern void func_80077634__for_func_800775D8(void) __asm__("func_80077634");
extern s32 D_8013B20C;

    D_801B25E8 = 1;
    D_801B25EC = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_800775F0(void)
{
extern u32 D_801B25E8;
extern s32 D_801B25EC;
extern void (*D_800D5148[])(void);
extern void func_80077634__for_func_800775F0(void) __asm__("func_80077634");
extern s32 D_8013B20C;

    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B25E8 += 1;
    func_80077634__for_func_800775F0();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80077634(void)
{
extern s32 D_8013B20C;
extern s32 D_801B25E8;
extern void func_80077670__for_func_80077634(void) __asm__("func_80077670");
extern void func_80077708__for_func_80077634(void) __asm__("func_80077708");
extern void func_800776B4__for_func_80077634(void) __asm__("func_800776B4");

    if (D_8013B20C == 0)
    {
        D_801B25E8 += 1;
        func_80077670__for_func_80077634();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80077670(void)
{
extern s32 D_8013B20C;
extern s32 D_801B25E8;
extern void func_80077670(void);
extern void func_80077708__for_func_80077670(void) __asm__("func_80077708");
extern void func_800776B4__for_func_80077670(void) __asm__("func_800776B4");

    func_8006CAC0(func_80077708__for_func_80077670);
    D_8013B20C = 1;
    D_801B25E8 += 1;
    func_800776B4__for_func_80077670();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800776B4(void)
{
extern s32 D_801B25E8;
extern s32 D_8013B20C;
extern void func_800776F0__for_func_800776B4(void) __asm__("func_800776F0");

    if (D_8013B20C == 0)
    {
        D_801B25E8 += 1;
        func_800776F0__for_func_800776B4();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800776F0(void)
{
extern s32 D_801B25E8;
extern s32 D_8013B20C;
extern void func_800776F0(void);

    D_801B25E8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80077708(s32 arg0)
{
extern u32 D_801B25F0;
extern s32 D_801B25F4;
extern void (*D_800D5160[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B25F0 = 1;
        D_801B25F4 = 1;
        return 1;
    }

    if (D_801B25F0 < 0x14)
    {
        D_800D5160[D_801B25F0]();
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
void func_80077780(void)
{
extern u32 D_801B25F0;
extern s32 D_801B25F4;
extern void (*D_800D5160[])(void);

    D_801B25F0 = 1;
    D_801B25F4 = 1;
}

/** @brief World-map step handler: set up the actor, register callbacks, advance step. */
void func_80077798(void)
{
extern void func_8007882C__for_func_80077798(void) __asm__("func_8007882C");
extern s32 D_8013B208;
extern s32 D_801B25F4;
extern s32 D_801B25F0;

    D_8013B208 = 1;
    func_8006683C(0x804020);
    func_8006CAC0(func_8007882C__for_func_80077798);
    func_800652A8(0x19, 0x80);
    D_801B25F4 = 2;
    D_801B25F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800777F8(void)
{
extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_80077D60__for_func_800777F8(void) __asm__("func_80077D60");

    if (--D_801B25F4 == 0)
    {
        D_801B25F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007782C(void)
{
extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_80077D60__for_func_8007782C(void) __asm__("func_80077D60");

    func_8006CAC0(func_80077D60__for_func_8007782C);
    D_801B25F4 = 0x4;
    D_801B25F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80077868(void)
{
extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_80077C34__for_func_80077868(void) __asm__("func_80077C34");

    if (--D_801B25F4 == 0)
    {
        D_801B25F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007789C(void)
{
extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_80077C34__for_func_8007789C(void) __asm__("func_80077C34");

    func_8006CAC0(func_80077C34__for_func_8007789C);
    D_801B25F4 = 0x8;
    D_801B25F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800778D8(void)
{
extern s32 D_801B25F4;
extern s32 D_801B25F0;

    if (--D_801B25F4 == 0)
    {
        D_801B25F0 += 1;
    }
}

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void func_8007790C(void)
{
extern s32 D_801ADAE0;
extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_800783C8__for_func_8007790C(void) __asm__("func_800783C8");

    D_801ADAE0 = 1;
    func_8006CAC0(func_800783C8__for_func_8007790C);
    D_801B25F4 = 0x8;
    D_801B25F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80077954(void)
{
extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_8007855C__for_func_80077954(void) __asm__("func_8007855C");

    if (--D_801B25F4 == 0)
    {
        D_801B25F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80077988(void)
{
extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_8007855C__for_func_80077988(void) __asm__("func_8007855C");

    func_8006CAC0(func_8007855C__for_func_80077988);
    D_801B25F4 = 0x14;
    D_801B25F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800779C4(void)
{
extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_80077FD8__for_func_800779C4(void) __asm__("func_80077FD8");

    if (--D_801B25F4 == 0)
    {
        D_801B25F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800779F8(void)
{
extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_80077FD8__for_func_800779F8(void) __asm__("func_80077FD8");

    func_8006CAC0(func_80077FD8__for_func_800779F8);
    D_801B25F4 = 0xC;
    D_801B25F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80077A34(void)
{
extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_80078128__for_func_80077A34(void) __asm__("func_80078128");

    if (--D_801B25F4 == 0)
    {
        D_801B25F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80077A68(void)
{
extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_80078128__for_func_80077A68(void) __asm__("func_80078128");

    func_8006CAC0(func_80078128__for_func_80077A68);
    D_801B25F4 = 0xC;
    D_801B25F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80077AA4(void)
{
extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_80078278__for_func_80077AA4(void) __asm__("func_80078278");

    if (--D_801B25F4 == 0)
    {
        D_801B25F0 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80077AD8(void)
{
extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_80078278__for_func_80077AD8(void) __asm__("func_80078278");

    func_8006CAC0(func_80078278__for_func_80077AD8);
    D_801B25F4 = 0x10;
    D_801B25F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80077B14(void)
{
extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_80077E8C__for_func_80077B14(void) __asm__("func_80077E8C");
extern void func_80078C2C__for_func_80077B14(void) __asm__("func_80078C2C");
extern void func_80078A90__for_func_80077B14(void) __asm__("func_80078A90");

    if (--D_801B25F4 == 0)
    {
        D_801B25F0 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_80077B48(void)
{
extern s32 D_801B25F4;
extern s32 D_801B25F0;
extern void func_80077E8C__for_func_80077B48(void) __asm__("func_80077E8C");
extern void func_80078C2C__for_func_80077B48(void) __asm__("func_80078C2C");
extern void func_80078A90__for_func_80077B48(void) __asm__("func_80078A90");

    func_8006CAC0(func_80077E8C__for_func_80077B48);
    func_8006CAC0(func_80078C2C__for_func_80077B48);
    func_8006CAC0(func_80078A90__for_func_80077B48);
    D_801B25F4 = 0xAC;
    D_801B25F0 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80077B9C(void)
{
extern s32 D_801B25F4;
extern s32 D_801B25F0;

    if (--D_801B25F4 == 0)
    {
        D_801B25F0 += 1;
    }
}

void func_80077BD0(void)
{
/* Partial WMAP decompilation: 95.600000% (gcc280_g0). */

extern s32 D_8013B20C;
extern u32 D_80139290[];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B25F0;

    D_8013B20C = 0;
    D_80139290[D_8011D530 * 10 + D_8011D510 * 60] = D_8011D4FC | 0x100;
    D_801B25F0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80077C34(s32 arg0)
{
extern u32 D_801B25F8;
extern s32 D_801B25FC;
extern void (*D_800D51B0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B25F8 = 1;
        D_801B25FC = 1;
    }

    if (D_801B25F8 < 0x4)
    {
        D_800D51B0[D_801B25F8]();
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
void func_80077CA4(void)
{
extern u32 D_801B25F8;
extern s32 D_801B25FC;
extern void (*D_800D51B0[])(void);

    D_801B25F8 = 1;
    D_801B25FC = 1;
}

/** @brief Restore effect data, reset its vector, and begin a 48-tick sequence step. */
void func_80077CBC(void)
{
/** @brief Eight bytes of effect state copied together. */
typedef struct
{
    u8 bytes[8];
} WmapBlk8;

extern void func_80076C6C__for_func_80077CBC(void) __asm__("func_80076C6C");
extern WmapBlk8 D_80139258;
extern s32 D_801B2470;
extern WmapBlk8 D_801B24A0;
extern s32 D_801B25F8;
extern s32 D_801B25FC;
extern VECTOR D_801B2650;

    D_801B24A0 = D_80139258;
    D_801B2470 = 0x80;
    D_801B2650.vz = 0x2710;
    D_801B2650.vy = 0;
    D_801B2650.vx = 0;
    D_801B25FC = 0x30;
    D_801B25F8 += 1;
    func_80076C6C__for_func_80077CBC();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80077D48(void)
{
extern s32 D_801B25F8;

    D_801B25F8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80077D60(s32 arg0)
{
extern u32 D_801B2600;
extern s32 D_801B2604;
extern void (*D_800D51C0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2600 = 1;
        D_801B2604 = 1;
    }

    if (D_801B2600 < 0x4)
    {
        D_800D51C0[D_801B2600]();
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
void func_80077DD0(void)
{
extern u32 D_801B2600;
extern s32 D_801B2604;
extern void (*D_800D51C0[])(void);

    D_801B2600 = 1;
    D_801B2604 = 1;
}

/** @brief Restore effect data, reset its vector, and begin a 16-tick sequence step. */
void func_80077DE8(void)
{
/** @brief Eight bytes of effect state copied together. */
typedef struct
{
    u8 bytes[8];
} WmapBlk8;

extern void func_80076DBC__for_func_80077DE8(void) __asm__("func_80076DBC");
extern WmapBlk8 D_80139258;
extern s32 D_801B2474;
extern WmapBlk8 D_801B24A8;
extern s32 D_801B2600;
extern s32 D_801B2604;
extern VECTOR D_801B2478;

    D_801B24A8 = D_80139258;
    D_801B2474 = 0x80;
    D_801B2478.vz = 0xC350;
    D_801B2478.vy = 0;
    D_801B2478.vx = 0;
    D_801B2604 = 0x10;
    D_801B2600 += 1;
    func_80076DBC__for_func_80077DE8();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80077E74(void)
{
extern s32 D_801B2600;

    D_801B2600 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80077E8C(s32 arg0)
{
extern u32 D_801B2608;
extern s32 D_801B260C;
extern void (*D_800D51D0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2608 = 1;
        D_801B260C = 1;
    }

    if (D_801B2608 < 0x4)
    {
        D_800D51D0[D_801B2608]();
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
void func_80077EFC(void)
{
extern u32 D_801B2608;
extern s32 D_801B260C;
extern void (*D_800D51D0[])(void);

    D_801B2608 = 1;
    D_801B260C = 1;
}

/** @brief Restore the default transform and advance the sequence. */
void func_80077F14(void)
{
/** @brief Eight-byte orientation data with byte alignment. */
typedef struct
{
    u8 bytes[8];
} WmapOrientation;

/** @brief Four-word world-map transform state. */
typedef struct
{
    s32 words[4];
} WmapTransform;

extern WmapOrientation D_80139258;
extern WmapOrientation D_8013B238;
extern WmapTransform D_80182DC0;
extern WmapTransform D_80139870;
extern s32 D_801B24B4;
extern s32 D_801B2608;
extern s32 D_801B260C;
extern void func_80076EB4__for_func_80077F14(void) __asm__("func_80076EB4");

    D_8013B238 = D_80139258;
    D_80139870 = D_80182DC0;
    D_801B24B4 = 0x80;
    D_80139870.words[2] = 1000;
    D_801B260C = 0x80;
    D_801B2608++;
    func_80076EB4__for_func_80077F14();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80077FC0(void)
{
extern s32 D_801B2608;

    D_801B2608 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80077FD8(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2610;
extern s32 D_801B2614;
extern void (*D_800D51E0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139888;
extern s32 D_80182DE4;
extern void func_80076FAC__for_func_80077FD8(void) __asm__("func_80076FAC");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2610 = 1;
        D_801B2614 = 1;
    }

    if (D_801B2610 < 0x4)
    {
        D_800D51E0[D_801B2610]();
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
void func_80078048(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2610;
extern s32 D_801B2614;
extern void (*D_800D51E0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139888;
extern s32 D_80182DE4;
extern void func_80076FAC__for_func_80078048(void) __asm__("func_80076FAC");

    D_801B2610 = 1;
    D_801B2614 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80078060(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2610;
extern s32 D_801B2614;
extern void (*D_800D51E0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139888;
extern s32 D_80182DE4;
extern void func_80076FAC__for_func_80078060(void) __asm__("func_80076FAC");

    D_8013B240 = D_80139258;
    D_80139888 = D_80182DC0;
    D_80182DE4 = 0x80;
    D_80139888.w[2] = 0xAFC8;
    D_801B2614 = 0x2A;
    D_801B2610 += 1;
    func_80076FAC__for_func_80078060();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80078110(void)
{
extern s32 D_801B2610;

    D_801B2610 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80078128(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2618;
extern s32 D_801B261C;
extern void (*D_800D51F0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2670;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139898;
extern s32 D_80182DE8;
extern void func_8007708C__for_func_80078128(void) __asm__("func_8007708C");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2618 = 1;
        D_801B261C = 1;
    }

    if (D_801B2618 < 0x4)
    {
        D_800D51F0[D_801B2618]();
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
void func_80078198(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2618;
extern s32 D_801B261C;
extern void (*D_800D51F0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2670;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139898;
extern s32 D_80182DE8;
extern void func_8007708C__for_func_80078198(void) __asm__("func_8007708C");

    D_801B2618 = 1;
    D_801B261C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_800781B0(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2618;
extern s32 D_801B261C;
extern void (*D_800D51F0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2670;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139898;
extern s32 D_80182DE8;
extern void func_8007708C__for_func_800781B0(void) __asm__("func_8007708C");

    D_801B2670 = D_80139258;
    D_80139898 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_80139898.w[2] = 0xAFC8;
    D_801B261C = 0x1E;
    D_801B2618 += 1;
    func_8007708C__for_func_800781B0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80078260(void)
{
extern s32 D_801B2618;

    D_801B2618 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80078278(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2620;
extern s32 D_801B2624;
extern void (*D_800D5200[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2678;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2660;
extern s32 D_80182DEC;
extern void func_8007716C__for_func_80078278(void) __asm__("func_8007716C");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2620 = 1;
        D_801B2624 = 1;
    }

    if (D_801B2620 < 0x4)
    {
        D_800D5200[D_801B2620]();
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
void func_800782E8(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2620;
extern s32 D_801B2624;
extern void (*D_800D5200[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2678;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2660;
extern s32 D_80182DEC;
extern void func_8007716C__for_func_800782E8(void) __asm__("func_8007716C");

    D_801B2620 = 1;
    D_801B2624 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80078300(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2620;
extern s32 D_801B2624;
extern void (*D_800D5200[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2678;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2660;
extern s32 D_80182DEC;
extern void func_8007716C__for_func_80078300(void) __asm__("func_8007716C");

    D_801B2678 = D_80139258;
    D_801B2660 = D_80182DC0;
    D_80182DEC = 0x80;
    D_801B2660.w[2] = 0xAFC8;
    D_801B2624 = 0x12;
    D_801B2620 += 1;
    func_8007716C__for_func_80078300();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800783B0(void)
{
extern s32 D_801B2620;

    D_801B2620 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_800783C8(s32 arg0)
{
extern u32 D_801B2628;
extern s32 D_801B262C;
extern void (*D_800D5210[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2628 = 1;
        D_801B262C = 1;
    }

    if (D_801B2628 < 0x4)
    {
        D_800D5210[D_801B2628]();
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
void func_80078438(void)
{
extern u32 D_801B2628;
extern s32 D_801B262C;
extern void (*D_800D5210[])(void);

    D_801B2628 = 1;
    D_801B262C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80078450(void)
{
extern u8* D_801399AC;
extern u8 D_8011D538[];
extern u8 D_800D9318[];
extern s32 D_801B2628;
extern s32 D_801B262C;
extern void func_800784C8__for_func_80078450(void) __asm__("func_800784C8");

    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x26] = 0;
    *(s16*)&D_800D9318[0x22] = 0x80;
    *(s16*)&D_800D9318[0x24] = 0x80;
    D_801B262C = 0x4A;
    D_801B2628 += 1;
    func_800784C8__for_func_80078450();
}

/** @brief World-map step: init a sub-object then count down a timer. */
void func_800784C8(void)
{
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B262C;
extern s32 D_801B2628;

    s32 n = 0xB;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, n, n, 0);
    if (--D_801B262C == 0)
    {
        D_801B2628 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80078544(void)
{
extern s32 D_801B2628;

    D_801B2628 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007855C(s32 arg0)
{
extern u32 D_801B2638;
extern s32 D_801B263C;
extern void (*D_800D5238[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2638 = 1;
        D_801B263C = 1;
    }

    if (D_801B2638 < 0x6)
    {
        D_800D5238[D_801B2638]();
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
void func_800785CC(void)
{
extern u32 D_801B2638;
extern s32 D_801B263C;
extern void (*D_800D5238[])(void);

    D_801B2638 = 1;
    D_801B263C = 1;
}

/** @brief Initialize twenty effect records and begin a 32-tick sequence step. */
void func_800785E4(void)
{
/** @brief Eight-byte record linking an index to its image data. */
typedef struct
{
    s32 index;
    u8 *image;
} WmapImageLink;

extern u8 D_8011D538[];
extern WmapImageLink D_80139988[];
extern u8 D_801AFBD0[];
extern s32 D_80182DF0;
extern s32 D_801B0FD0;
extern s32 D_801B2638;
extern s32 D_801B263C;
extern void func_80078678__for_func_800785E4(void) __asm__("func_80078678");

    s32 index;

    D_801B0FD0 = 20;
    D_80182DF0 = 255;
    for (index = 14; index < 34; index++)
    {
        *(s16 *)(D_801AFBD0 + index * 20) = 0;
        D_80139988[index + 14].image = D_8011D538;
    }
    D_801B263C = 32;
    D_801B2638++;
    func_80078678__for_func_800785E4();
}

/** @brief Update the actor effect and advance the sequence after its countdown. */
void func_80078678(void)
{
extern u8 D_800D94D0[];
extern u8 D_801399F8[];
extern s32 D_80182DF0;
extern s32 D_801B2638;
extern s32 D_801B263C;

    func_8006A9C4(D_800D94D0, D_801399F8, 0xE, 0x22, D_80182DF0, 0x1FF6, 0x200, 0x30, 2, 0x320, 0xB, 4);
    if (--D_801B263C == 0)
    {
        D_801B2638++;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007871C(void)
{
extern void func_80078754__for_func_8007871C(void) __asm__("func_80078754");
extern s32 D_801B263C;
extern s32 D_801B2638;

    D_801B263C = 0x40;
    D_801B2638 += 1;
    func_80078754__for_func_8007871C();
}

/** @brief Update the actor effect and advance the sequence after its countdown. */
void func_80078754(void)
{
extern u8 D_800D94D0[];
extern u8 D_801399F8[];
extern s32 D_80182DF0;
extern s32 D_801B2638;
extern s32 D_801B263C;

    func_8006A9C4(D_800D94D0, D_801399F8, 0xE, 0x22, D_80182DF0, 0x1FF6, 0x200, 0x30, 2, 0x320, 0xB, 4);
    D_80182DF0 -= 4;
    if (D_80182DF0 < 0)
    {
        D_80182DF0 = 0;
    }
    if (--D_801B263C == 0)
    {
        D_801B2638++;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80078814(void)
{
extern s32 D_801B2638;

    D_801B2638 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007882C(s32 arg0)
{
extern u32 D_801B2630;
extern s32 D_801B2634;
extern void (*D_800D5220[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2630 = 1;
        D_801B2634 = 1;
    }

    if (D_801B2630 < 0x6)
    {
        D_800D5220[D_801B2630]();
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
void func_8007889C(void)
{
extern u32 D_801B2630;
extern s32 D_801B2634;
extern void (*D_800D5220[])(void);

    D_801B2630 = 1;
    D_801B2634 = 1;
}

/** @brief Initialize actor configuration and begin a 20-tick sequence step. */
void func_800788B4(void)
{
/** @brief World-map actor configuration with its original field layout. */
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

extern void func_80078934__for_func_800788B4(void) __asm__("func_80078934");
extern WmapConfigA D_800D9370;
extern u8 D_8011D538[];
extern u8 *D_801399BC;
extern s32 D_801B2630;
extern s32 D_801B2634;

    D_801399BC = D_8011D538;
    D_800D9370.field_06 = 0xF;
    D_800D9370.field_0E = 2;
    D_800D9370.field_10 = -1;
    D_800D9370.field_26 = -1;
    D_800D9370.field_22 = 0x81;
    D_800D9370.field_02 = 0;
    D_800D9370.field_24 = 0x90;
    D_801B2634 = 0x14;
    D_801B2630 += 1;
    func_80078934__for_func_800788B4();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80078934(void)
{
extern void func_800789FC__for_func_80078934(void) __asm__("func_800789FC");
extern s16 D_800D9370[];
extern s32 D_801B2634;
extern s32 D_801B2630;
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0xB, 0x4, 0);
    if (--D_801B2634 == 0)
    {
        D_801B2630 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800789B0(void)
{
extern void func_800789FC__for_func_800789B0(void) __asm__("func_800789FC");
extern s16 D_800D9370[];
extern s32 D_801B2634;
extern s32 D_801B2630;
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    D_800D9370[19] = 8;
    D_800D9370[17] = 0;
    D_801B2634 = 0x10;
    D_801B2630 += 1;
    func_800789FC__for_func_800789B0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800789FC(void)
{
extern void func_800789FC(void);
extern s16 D_800D9370[];
extern s32 D_801B2634;
extern s32 D_801B2630;
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0xB, 0x4, 0);
    if (--D_801B2634 == 0)
    {
        D_801B2630 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80078A78(void)
{
extern s32 D_801B2630;

    D_801B2630 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80078A90(s32 arg0)
{
extern u32 D_801B2640;
extern s32 D_801B2644;
extern void (*D_800D5250[])(void);
extern void func_80078B98__for_func_80078A90(void) __asm__("func_80078B98");
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2640 = 1;
        D_801B2644 = 1;
    }

    if (D_801B2640 < 0x4)
    {
        D_800D5250[D_801B2640]();
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
void func_80078B00(void)
{
extern u32 D_801B2640;
extern s32 D_801B2644;
extern void (*D_800D5250[])(void);
extern void func_80078B98__for_func_80078B00(void) __asm__("func_80078B98");
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801B2640 = 1;
    D_801B2644 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80078B18(void)
{
extern u32 D_801B2640;
extern s32 D_801B2644;
extern void (*D_800D5250[])(void);
extern void func_80078B98__for_func_80078B18(void) __asm__("func_80078B98");
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801399B4 = D_8011F538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 4;
    *(s16*)&D_800D9344[0x22] = 0x80;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x24] = 0xFC;
    D_801B2644 = 0xAF;
    D_801B2640 += 1;
    func_80078B98__for_func_80078B18();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80078B98(void)
{
extern u32 D_801B2640;
extern s32 D_801B2644;
extern void (*D_800D5250[])(void);
extern void func_80078B98(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x12, 0xB, 0);
    if (--D_801B2644 == 0)
    {
        D_801B2640 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80078C14(void)
{
extern s32 D_801B2640;

    D_801B2640 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80078C2C(s32 arg0)
{
extern u32 D_801B2648;
extern s32 D_801B264C;
extern void (*D_800D5260[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2648 = 1;
        D_801B264C = 1;
    }

    if (D_801B2648 < 0x8)
    {
        D_800D5260[D_801B2648]();
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
void func_80078C9C(void)
{
extern u32 D_801B2648;
extern s32 D_801B264C;
extern void (*D_800D5260[])(void);

    D_801B2648 = 1;
    D_801B264C = 1;
}

/** @brief Draw the effect, raise its value to at most 129, and update the countdown. */
void func_80078CB4(void)
{
extern void func_800773B8__for_func_80078CB4(s32, s32, s32) __asm__("func_800773B8");
extern s32 D_80182DF4;
extern s32 D_801B2648;
extern s32 D_801B264C;

    s32 value;
    s32 remaining_ticks;

    func_800773B8__for_func_80078CB4(0x7C, 0xAA, 0xD);
    value = D_80182DF4 + 8;
    D_80182DF4 = value;
    if (value >= 0x82)
    {
        D_80182DF4 = 0x81;
    }
    remaining_ticks = D_801B264C - 1;
    D_801B264C = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2648 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80078D2C(void)
{
extern void func_80078D64__for_func_80078D2C(void) __asm__("func_80078D64");
extern s32 D_801B264C;
extern s32 D_801B2648;

    D_801B264C = 0x64;
    D_801B2648 += 1;
    func_80078D64__for_func_80078D2C();
}

/** @brief Update the sequence effect and advance when its countdown expires. */
void func_80078D64(void)
{
extern void func_800773B8__for_func_80078D64(s32, s32, s32) __asm__("func_800773B8");
extern s32 D_801B2648;
extern s32 D_801B264C;

    s32 remaining_ticks;

    func_800773B8__for_func_80078D64(0x7C, 0xAA, 0xD);
    remaining_ticks = D_801B264C - 1;
    D_801B264C = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2648 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80078DB8(void)
{
extern void func_80078DF0__for_func_80078DB8(void) __asm__("func_80078DF0");
extern s32 D_801B264C;
extern s32 D_801B2648;

    D_801B264C = 0x10;
    D_801B2648 += 1;
    func_80078DF0__for_func_80078DB8();
}

/** @brief Draw the effect, reduce its value toward zero, and update the countdown. */
void func_80078DF0(void)
{
extern void func_800773B8__for_func_80078DF0(s32, s32, s32) __asm__("func_800773B8");
extern s32 D_80182DF4;
extern s32 D_801B2648;
extern s32 D_801B264C;

    s32 value;
    s32 remaining_ticks;

    func_800773B8__for_func_80078DF0(0x7C, 0xAA, 0xD);
    value = D_80182DF4 - 8;
    D_80182DF4 = value;
    if (value < 0)
    {
        D_80182DF4 = 0;
    }
    remaining_ticks = D_801B264C - 1;
    D_801B264C = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2648 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80078E60(void)
{
extern s32 D_801B2648;

    D_801B2648 += 1;
}
