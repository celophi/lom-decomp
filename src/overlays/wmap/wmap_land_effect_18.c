#include "wmap_main.h"
#include "wmap_land_effect_18.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_sprite_render.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "sdk/rand.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"

/** @brief Compose the effect transform, draw it, and advance its rotation and countdown. */
void func_8006D918(void)
{
extern void *D_8011CF24;
extern VECTOR D_8011CF60;
extern SVECTOR D_80139278;
extern VECTOR D_80182DC0;
extern SVECTOR D_801B24A0;
extern s32 D_801B2410;
extern s32 D_801B2414;

    MATRIX base;
    MATRIX effect;
    s32 remaining;

    PushMatrix();
    RotMatrix(&D_80139278, &base);
    TransMatrix(&base, &D_80182DC0);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&D_801B24A0, &effect);
    TransMatrix(&effect, &D_8011CF60);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    func_8006CD98(D_8011CF24, 0, 36, 183, 0x7A40, 1, -1);
    PopMatrix();
    remaining = D_801B2414 - 1;
    D_801B24A0.vz = (u16)(D_801B24A0.vz + 320);
    D_801B2414 = remaining;
    if (remaining == 0)
    {
        D_801B2410++;
    }
}

/** @brief Draw and fade the transformed effect, then advance its countdown. */
void func_8006DA24(void)
{
extern void *D_8011CF24;
extern VECTOR D_8011CF60;
extern SVECTOR D_80139278;
extern VECTOR D_80182DC0;
extern s32 D_801B2410;
extern s32 D_801B2414;
extern SVECTOR D_801B24A0;
extern s32 D_801B24B4;

    MATRIX base;
    MATRIX effect;
    s32 intensity;
    s32 remaining;

    if (D_801B24B4 != 0)
    {
        PushMatrix();
        RotMatrix(&D_80139278, &base);
        TransMatrix(&base, &D_80182DC0);
        SetRotMatrix(&base);
        SetTransMatrix(&base);
        RotMatrix(&D_801B24A0, &effect);
        TransMatrix(&effect, &D_8011CF60);
        CompMatrix(&base, &effect, &effect);
        SetRotMatrix(&effect);
        SetTransMatrix(&effect);
        func_8006CD98(D_8011CF24, 0, 0x24, 0xB7, 0x7A40, 1, D_801B24B4);
        intensity = D_801B24B4 - 2;
        D_801B24B4 = intensity;
        if (intensity < 0)
        {
            D_801B24B4 = 0;
        }
        PopMatrix();
        D_801B24A0.vz = (u16) (D_801B24A0.vz + 0x12C);
    }
    remaining = D_801B2414 - 1;
    D_801B2414 = remaining;
    if (remaining == 0)
    {
        D_801B2410 += 1;
    }
}

/** @brief Draw and brighten two rotating layers, then advance their shared countdown. */
void func_8006DB68(void)
{
extern void *D_8011CF1C;
extern VECTOR D_80182DC0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B2468;
extern s32 D_801B2418;
extern s32 D_801B241C;

    s32 remaining;
    s32 intensity;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_801B2490);
    func_8006CD98(D_8011CF1C, 0, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2490.vz = (u16)(D_801B2490.vz + 32);
    func_8006CFA8(&D_80182DC0, &D_801B2498);
    func_8006CD98(D_8011CF1C, 0, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2498.vz = (u16)(D_801B2498.vz + 16);
    PopMatrix();
    intensity = D_801B2468 + 2;
    D_801B2468 = intensity;
    if (intensity >= 129)
    {
        D_801B2468 = 128;
    }
    remaining = D_801B241C - 1;
    D_801B241C = remaining;
    if (remaining == 0)
    {
        D_801B2418++;
    }
}

/** @brief Draw two rotating effect layers and advance their shared countdown. */
void func_8006DC98(void)
{
extern void *D_8011CF1C;
extern VECTOR D_80182DC0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B2468;
extern s32 D_801B24B0;
extern s32 D_801B2418;
extern s32 D_801B241C;

    s32 remaining;
    s32 frame;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_801B2490);
    frame = D_801B24B0 + 1;
    D_801B24B0 = frame;
    if (frame >= 8)
    {
        D_801B24B0 = 7;
    }
    if (D_801B2468 >= 5)
    {
        D_801B2468 -= 4;
    }
    func_8006CD98(D_8011CF1C, D_801B24B0, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2490.vz = (u16)(D_801B2490.vz + 32);
    func_8006CFA8(&D_80182DC0, &D_801B2498);
    func_8006CD98(D_8011CF1C, D_801B24B0, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2498.vz = (u16)(D_801B2498.vz + 16);
    PopMatrix();
    remaining = D_801B241C - 1;
    D_801B241C = remaining;
    if (remaining == 0)
    {
        D_801B2418++;
    }
}

/** @brief Draw two rotating effect layers and advance their shared countdown. */
void func_8006DDEC(void)
{
extern void *D_8011CF1C;
extern VECTOR D_80182DC0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B2468;
extern s32 D_801B2418;
extern s32 D_801B241C;

    s32 remaining;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_801B2490);
    func_8006CD98(D_8011CF1C, 7, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2490.vz = (u16)(D_801B2490.vz + 32);
    func_8006CFA8(&D_80182DC0, &D_801B2498);
    func_8006CD98(D_8011CF1C, 7, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2498.vz = (u16)(D_801B2498.vz + 16);
    PopMatrix();
    remaining = D_801B241C - 1;
    D_801B241C = remaining;
    if (remaining == 0)
    {
        D_801B2418++;
    }
}

/** @brief Draw and fade two rotating layers, then advance their shared countdown. */
void func_8006DEFC(void)
{
extern void *D_8011CF1C;
extern VECTOR D_80182DC0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B2468;
extern s32 D_801B2418;
extern s32 D_801B241C;

    s32 remaining;
    s32 intensity;

    PushMatrix();
    func_8006CFA8(&D_80182DC0, &D_801B2490);
    func_8006CD98(D_8011CF1C, 7, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2490.vz = (u16)(D_801B2490.vz + 32);
    func_8006CFA8(&D_80182DC0, &D_801B2498);
    func_8006CD98(D_8011CF1C, 7, 4, 183, 0x7A80, 0, D_801B2468);
    D_801B2498.vz = (u16)(D_801B2498.vz + 16);
    PopMatrix();
    intensity = D_801B2468 - 2;
    D_801B2468 = intensity;
    if (intensity < 0)
    {
        D_801B2468 = 0;
    }
    remaining = D_801B241C - 1;
    D_801B241C = remaining;
    if (remaining == 0)
    {
        D_801B2418++;
    }
}

/** @brief Draw the effect at successive depths and advance its countdown. */
void func_8006E024(void)
{
extern u8 *D_8011CF24;
extern VECTOR D_8011CF60;
extern SVECTOR D_80139278;
extern VECTOR D_80139870;
extern s32 D_80139878;
extern SVECTOR D_8013B238;
extern s32 D_801B2420;
extern s32 D_801B2424;
extern s32 D_801B2488;

    MATRIX base;
    MATRIX effect;
    s32 remaining;

    switch (D_801B2488)
   
  {
    case 0:
        D_80139878 = 8000;
        break;
    case 1:
        D_80139878 = 18000;
        break;
    case 2:
        D_80139878 = 10000;
        break;
    }
    D_801B2488 += 1;
    PushMatrix();
    RotMatrix(&D_80139278, &base);
    TransMatrix(&base, &D_80139870);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&D_8013B238, &effect);
    TransMatrix(&effect, &D_8011CF60);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    func_8006CD98(D_8011CF24 + 0x4000, 0, 4, -1, -1, 1, -1);
    PopMatrix();
    remaining = D_801B2424 - 1;
    D_801B2424 = remaining;
    if (remaining == 0)
    {
        D_801B2420 += 1;
    }
}

/** @brief Compose the effect transform, draw it at alternating depths, and advance its countdown. */
void func_8006E190(void)
{
extern u8 *D_8011CF24;
extern s32 D_8011CF74;
extern s32 D_80139878;
extern VECTOR D_8011CF60;
extern SVECTOR D_80139278;
extern VECTOR D_80139870;
extern SVECTOR D_8013B238;
extern s32 D_801B2420;
extern s32 D_801B2424;

    MATRIX base;
    MATRIX effect;
    s32 remaining;

    if (D_8011CF74 & 1)
    {
        D_80139878 = 28000;
    }
    else
    {
        D_80139878 = 18000;
    }
    PushMatrix();
    RotMatrix(&D_80139278, &base);
    TransMatrix(&base, &D_80139870);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&D_8013B238, &effect);
    TransMatrix(&effect, &D_8011CF60);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    func_8006CD98(D_8011CF24 + 0x4000, 0, 4, -1, -1, 1, -1);
    PopMatrix();
    remaining = D_801B2424 - 1;
    D_801B2424 = remaining;
    if (remaining == 0)
    {
        D_801B2420++;
    }
}

/** @brief Compose the effect transform, draw it at alternating depths, and advance its countdown. */
void func_8006E2B0(void)
{
extern u8 *D_8011CF24;
extern s32 D_8011CF74;
extern s32 D_80139878;
extern VECTOR D_8011CF60;
extern SVECTOR D_80139278;
extern VECTOR D_80139870;
extern SVECTOR D_8013B238;
extern s32 D_801B2420;
extern s32 D_801B246C;
extern s32 D_801B2424;

    MATRIX base;
    MATRIX effect;
    s32 remaining;

    if (D_8011CF74 & 1)
    {
        D_80139878 = 28000;
    }
    else
    {
        D_80139878 = 18000;
    }
    PushMatrix();
    RotMatrix(&D_80139278, &base);
    TransMatrix(&base, &D_80139870);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&D_8013B238, &effect);
    TransMatrix(&effect, &D_8011CF60);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    func_8006CD98(D_8011CF24 + 0x4000, 0, 4, -1, -1, 1, D_801B246C);
    D_801B246C--;
    PopMatrix();
    remaining = D_801B2424 - 1;
    D_801B2424 = remaining;
    if (remaining == 0)
    {
        D_801B2420++;
    }
}

/** @brief Advance the effect depth, fade its intensity, and update the countdown. */
void func_8006E3E4(void)
{
extern u8 *D_8011CF24;
extern VECTOR D_8011CF60;
extern SVECTOR D_80139278;
extern s32 D_80182DC8;
extern s32 D_801B2428;
extern s32 D_801B242C;
extern s32 D_801B2470;
extern s32 D_801B2474;
extern VECTOR D_801B2478;
extern SVECTOR D_801B24A8;

    MATRIX base;
    MATRIX effect;
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = D_801B2478.vz - 0x960;
    D_801B2478.vz = depth;
    if (depth < 0x3E8)
    {
        D_801B2478.vz = (s32) D_80182DC8;
        D_801B2474 = D_801B2470;
    }
    PushMatrix();
    RotMatrix(&D_80139278, &base);
    TransMatrix(&base, &D_801B2478);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&D_801B24A8, &effect);
    TransMatrix(&effect, &D_8011CF60);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    if (D_801B2474 != 0)
    {
        func_8006CD98(D_8011CF24 + 0x5000, 0, 4, -1, -1, 1, D_801B2474);
    }
    PopMatrix();
    intensity = D_801B2474 - 9;
    D_801B2474 = intensity;
    if (intensity < 0)
    {
        D_801B2474 = 0;
    }
    remaining = D_801B242C - 1;
    D_801B242C = remaining;
    if (remaining == 0)
    {
        D_801B2428 += 1;
    }
}

/** @brief Advance the effect depth, fade its intensity, and update the countdown. */
void func_8006E544(void)
{
extern u8 *D_8011CF24;
extern VECTOR D_8011CF60;
extern SVECTOR D_80139278;
extern s32 D_80182DC8;
extern s32 D_801B2428;
extern s32 D_801B242C;
extern s32 D_801B2470;
extern s32 D_801B2474;
extern VECTOR D_801B2478;
extern SVECTOR D_801B24A8;

    MATRIX base;
    MATRIX effect;
    s32 depth;
    s32 intensity;
    s32 remaining;
    s32 peak;

    depth = D_801B2478.vz - 0x960;
    D_801B2478.vz = depth;
    if (depth < 0x3E8)
    {
        D_801B2478.vz = (s32) D_80182DC8;
        D_801B2474 = D_801B2470;
    }
    PushMatrix();
    RotMatrix(&D_80139278, &base);
    TransMatrix(&base, &D_801B2478);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&D_801B24A8, &effect);
    TransMatrix(&effect, &D_8011CF60);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    if (D_801B2474 != 0)
    {
        func_8006CD98(D_8011CF24 + 0x5000, 0, 4, -1, -1, 1, D_801B2474);
    }
    PopMatrix();
    peak = D_801B2470 - 1;
    intensity = D_801B2474 - 9;
    D_801B2470 = peak;
    D_801B2474 = intensity;
    if (intensity < 0)
    {
        D_801B2474 = 0;
    }
    if (peak < 0)
    {
        D_801B2470 = 0;
    }
    remaining = D_801B242C - 1;
    D_801B242C = remaining;
    if (remaining == 0)
    {
        D_801B2428 += 1;
    }
}

/**
 * @brief Project and draw active world-map sparks, then respawn empty slots.
 * @note First pass projects each live spark through the GTE and advances its
 *       radius; second pass seeds fresh sparks up to the shared population cap.
 * @note GTE-tagged: best-effort structural match (mirrors func_8007115C); the
 *       gcc280_g0 diff harness cannot assemble the GTE mnemonics for a percent.
 */
void func_8006E6B8(void)
{
/* Partial WMAP decompilation: 79.604650% (gcc280_g0). */

/** @brief World-map spark particle: spin angle, radial velocity, and lifetime. */
typedef struct
{
    s16 active;   /* 0x0 */
    s16 angle;    /* 0x2 */
    s32 delta;    /* 0x4 */
    s32 radius;   /* 0x8 */
    u16 timer;    /* 0xC */
    s16 unk0E;    /* 0xE */
    s16 unk10;    /* 0x10 */
    s16 unk12;    /* 0x12 */
} WmapSpark;

/** @brief World-map draw record; helpers use it opaquely, this handler seeds fields. */
typedef struct
{
    u8 pad00[0x2];
    s16 unk02;    /* 0x2 */
    u8 pad04[0x2];
    s8 unk06;     /* 0x6 */
    u8 pad07[0x7];
    s16 unk0E;    /* 0xE */
    s16 unk10;    /* 0x10 */
    u8 pad12[0x10];
    s16 unk22;    /* 0x22 */
    s16 unk24;    /* 0x24 */
    u8 pad26[0x6];
} WmapDraw;

extern WmapSpark D_801AFBD0[];
extern WmapDraw D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_80139980;
extern s32 D_801B0FD0;

    SVECTOR position;
    s32 screen;
    WmapSpark* spark;
    WmapDraw* draw;
    s32 count;
    s32 i;

    count = 0;
    spark = D_801AFBD0;
    draw = D_800D9370;
    for (i = 0; i < 0x1E; i++)
    {
        if (spark->active != 0)
        {
            position.vx = ((spark->radius >> 6) * (ccos(spark->angle) >> 6)) >> 0xC;
            position.vy = ((spark->radius >> 6) * (csin(spark->angle) >> 6)) >> 0xC;
            position.vz = 0;
            gte_ldv0(&position);
            gte_rtps();
            spark->radius += spark->delta;
            draw->unk22 = *(u16*)&D_80139980;
            draw->unk24 = *(u16*)&D_80139980;
            gte_stsxy(&screen);
            func_8006CC4C(draw, &D_801399B8[i * 8]);
            func_80066F9C(draw, screen, 9, 0x1F, 0);
            if (--spark->timer == 0)
            {
                spark->active = 0;
            }
            count++;
        }
        spark++;
        draw++;
    }
    spark = D_801AFBD0;
    draw = D_800D9370;
    for (i = 0; i < 0x1E; i++)
    {
        if (spark->active == 0)
        {
            if (D_801B0FD0 < count)
            {
                break;
            }
            count++;
            draw->unk06 = 0xF;
            draw->unk10 = -1;
            draw->unk02 = 0;
            draw->unk0E = 0;
            spark->active = 1;
            spark->angle = rand() >> 3;
            spark->radius = 0;
            spark->delta = rand() / 2 + 0x1000;
            spark->timer = (rand() & 0x3C) + 0x4B;
        }
        spark++;
        draw++;
    }
}

/** @brief Initialize and project the map effect before its first draw. */
void func_8006E8BC(void)
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



typedef struct
{
    s32 x, y, scale, pad;
} WmapTransform;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern WmapTransform D_80139950;

extern WmapConfigA D_800D9268[];
extern s32 D_8011CF4C;
extern u8 D_8011D538[];
extern void *D_801399AC;
extern s32 D_80182DF0;
extern s32 D_801B2438;
extern s32 D_801B243C;
extern void func_8006F864__for_func_8006E8BC(void) __asm__("func_8006F864");

    SVECTOR position;

    D_801399AC = D_8011D538;
    D_800D9268[4].field_06 = 15;
    D_800D9268[4].field_10 = -1;
    D_800D9268[4].field_02 = 0;
    D_800D9268[4].field_0E = 0;
    D_800D9268[4].field_22 = 128;
    D_800D9268[4].field_24 = 128;
    position.vz = 0;
    position.vx = (((D_8011D510 - 1) * 160 -
                   D_80139950.x * 0x14000 / D_80139950.scale) * 0x6000) /
                  D_80139950.scale;
    position.vy = (((D_8011D530 - 1) * 160 -
                   D_80139950.y * 0x14000 / D_80139950.scale) * 0x6000) /
                  D_80139950.scale;
    gte_ldv0(&position);
    gte_rtps();
    gte_stsxy(&D_8011CF4C);
    D_80182DF0 = 128;
    D_801B243C = 64;
    D_801B2438++;
    func_8006F864__for_func_8006E8BC();
}

/** @brief Configure effect parameters and reset its animation resource slots. */
void func_8006EAA4(void)
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
extern s32 D_80182DEC;
extern s32 D_801B0FD0;
extern s32 D_801B2450;
extern s32 D_801B2454;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern u8 D_8011F538[];
extern void func_80070368__for_func_8006EAA4(void) __asm__("func_80070368");

    s32 i;

    D_801B0FD0 = 16;
    D_80139234 = 3;
    D_8013923C = 1;
    D_80139240 = 56;
    D_8013924C = 0;
    D_80139250 = 1;
    D_80139260 = 5000;
    D_80139264 = 60;
    D_80139268 = 17;
    D_8013926C = 1;
    D_80139284 = 0;
    D_80182DEC = 127;
    for (i = 0; i < 16; i++)
    {
        D_801AFBD0[i + D_80139264].field_00 = 0;
        D_80139988[i + 104].field_04 = D_8011F538;
    }
    D_801B2454 = 32;
    D_801B2450++;
    func_80070368__for_func_8006EAA4();
}

/** @brief Configure effect parameters and reset its animation resource slots. */
void func_8006EBB0(void)
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
extern s32 D_80182DF4;
extern s32 D_801B0FD0;
extern s32 D_801B2458;
extern s32 D_801B245C;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern u8 D_8011F538[];
extern void func_80070490__for_func_8006EBB0(void) __asm__("func_80070490");

    s32 i;

    D_801B0FD0 = 40;
    D_80139234 = 6;
    D_8013923C = 2;
    D_80139240 = 32;
    D_8013924C = 2;
    D_80139250 = 1;
    D_80139260 = 5000;
    D_80139264 = 120;
    D_80139268 = 17;
    D_8013926C = 2;
    D_80139284 = 0;
    D_80182DF4 = 127;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + D_80139264].field_00 = 0;
        D_80139988[i + 204].field_04 = D_8011F538;
    }
    D_801B245C = 48;
    D_801B2458++;
    func_80070490__for_func_8006EBB0();
}

/** @brief Compose the effect transform, draw and brighten it, and advance its countdown. */
void func_8006ECC0(void)
{
extern u8 *D_8011CF24;
extern s32 D_80182DE4;
extern VECTOR D_8011CF60;
extern SVECTOR D_80139278;
/** @brief Translation whose depth read is retained by the original draw routine. */
typedef struct
{
    s32 vx;
    s32 vy;
    volatile s32 vz;
    s32 pad;
} WmapEffectTranslation;

extern WmapEffectTranslation D_80139888;
extern SVECTOR D_8013B240;
extern s32 D_801B2460;
extern s32 D_801B2464;

    MATRIX base;
    MATRIX effect;
    s32 remaining;
    s32 intensity;

    /* Preserve the original depth read before installing the transform. */
    (void)D_80139888.vz;
    PushMatrix();
    RotMatrix(&D_80139278, &base);
    TransMatrix(&base, (VECTOR *)&D_80139888);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&D_8013B240, &effect);
    TransMatrix(&effect, &D_8011CF60);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    if (D_80182DE4 != 0)
    {
        func_8006CD98(D_8011CF24 + 0x6000, 0, 4, -1, -1, 1, D_80182DE4);
    }
    PopMatrix();
    intensity = D_80182DE4 + 10;
    D_80182DE4 = intensity;
    if (intensity >= 256)
    {
        D_80182DE4 = 255;
    }
    remaining = D_801B2464 - 1;
    D_801B2464 = remaining;
    if (remaining == 0)
    {
        D_801B2460++;
    }
}

/** @brief Compose the effect transform, draw and fade it, and advance its countdown. */
void func_8006EDF4(void)
{
extern u8 *D_8011CF24;
extern s32 D_80182DE4;
extern VECTOR D_8011CF60;
extern SVECTOR D_80139278;
/** @brief Translation whose depth read is retained by the original draw routine. */
typedef struct
{
    s32 vx;
    s32 vy;
    volatile s32 vz;
    s32 pad;
} WmapEffectTranslation;

extern WmapEffectTranslation D_80139888;
extern SVECTOR D_8013B240;
extern s32 D_801B2460;
extern s32 D_801B2464;

    MATRIX base;
    MATRIX effect;
    s32 remaining;
    s32 intensity;

    /* Preserve the original depth read before installing the transform. */
    (void)D_80139888.vz;
    PushMatrix();
    RotMatrix(&D_80139278, &base);
    TransMatrix(&base, (VECTOR *)&D_80139888);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(&D_8013B240, &effect);
    TransMatrix(&effect, &D_8011CF60);
    CompMatrix(&base, &effect, &effect);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
    if (D_80182DE4 != 0)
    {
        func_8006CD98(D_8011CF24 + 0x6000, 0, 4, -1, -1, 1, D_80182DE4);
    }
    PopMatrix();
    intensity = D_80182DE4 - 10;
    D_80182DE4 = intensity;
    if (intensity < 0)
    {
        D_80182DE4 = 0;
    }
    remaining = D_801B2464 - 1;
    D_801B2464 = remaining;
    if (remaining == 0)
    {
        D_801B2460++;
    }
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8006EF20(s32 arg0)
{
extern u32 D_801B2410;
extern s32 D_801B2414;
extern void (*D_800D4CC4[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2410 = 1;
        D_801B2414 = 1;
    }

    if (D_801B2410 < 0x6)
    {
        D_800D4CC4[D_801B2410]();
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
void func_8006EF90(void)
{
extern u32 D_801B2410;
extern s32 D_801B2414;
extern void (*D_800D4CC4[])(void);

    D_801B2410 = 1;
    D_801B2414 = 1;
}

/** @brief Restore the effect state and begin a one-tick sequence step. */
void func_8006EFA8(void)
{
/** @brief Eight bytes copied together as world-map effect state. */
typedef struct
{
    u8 bytes[8];
} WmapBlk8;

extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern s32 D_801B24B4;
extern s32 D_801B2410;
extern s32 D_801B2414;
extern void func_8006D918__for_func_8006EFA8(void) __asm__("func_8006D918");

    D_801B24B4 = 128;
    D_801B24A0 = D_80139258;
    D_801B2414 = 1;
    D_801B2410 += 1;
    func_8006D918__for_func_8006EFA8();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8006F01C(void)
{
extern s32 D_801B2410;
extern void func_8006DA24__for_func_8006F01C(void) __asm__("func_8006DA24");
extern s32 D_801B2414;

    D_801B2414 = 0x40;
    D_801B2410 += 1;
    func_8006DA24__for_func_8006F01C();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8006F054(void)
{
extern s32 D_801B2410;
extern void func_8006DA24__for_func_8006F054(void) __asm__("func_8006DA24");
extern s32 D_801B2414;

    D_801B2410 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8006F06C(s32 arg0)
{
extern u32 D_801B2418;
extern s32 D_801B241C;
extern void (*D_800D4CDC[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2418 = 1;
        D_801B241C = 1;
        return 1;
    }

    if (D_801B2418 < 0xA)
    {
        D_800D4CDC[D_801B2418]();
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
void func_8006F0E4(void)
{
extern u32 D_801B2418;
extern s32 D_801B241C;
extern void (*D_800D4CDC[])(void);

    D_801B2418 = 1;
    D_801B241C = 1;
}

/** @brief Clear effect state and two vectors, then begin a 60-tick sequence step. */
void func_8006F0FC(void)
{
extern void func_8006DB68__for_func_8006F0FC(void) __asm__("func_8006DB68");
extern s32 D_801B2418;
extern s32 D_801B241C;
extern s32 D_801B2468;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B24B0;

    D_801B2468 = 0;
    D_801B24B0 = 0;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    D_801B2498.vx = 0;
    D_801B2498.vy = 0;
    D_801B2498.vz = 0;
    D_801B241C = 0x3C;
    D_801B2418 += 1;
    func_8006DB68__for_func_8006F0FC();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8006F16C(void)
{
extern void func_8006DC98__for_func_8006F16C(void) __asm__("func_8006DC98");
extern s32 D_801B241C;
extern s32 D_801B2418;

    D_801B241C = 0x9;
    D_801B2418 += 1;
    func_8006DC98__for_func_8006F16C();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8006F1A4(void)
{
extern void func_8006DDEC__for_func_8006F1A4(void) __asm__("func_8006DDEC");
extern s32 D_801B241C;
extern s32 D_801B2418;

    D_801B241C = 0x30;
    D_801B2418 += 1;
    func_8006DDEC__for_func_8006F1A4();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8006F1DC(void)
{
extern s32 D_801B2418;
extern void func_8006DEFC__for_func_8006F1DC(void) __asm__("func_8006DEFC");
extern s32 D_801B241C;

    D_801B241C = 0x40;
    D_801B2418 += 1;
    func_8006DEFC__for_func_8006F1DC();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8006F214(void)
{
extern s32 D_801B2418;
extern void func_8006DEFC__for_func_8006F214(void) __asm__("func_8006DEFC");
extern s32 D_801B241C;

    D_801B2418 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8006F22C(s32 arg0)
{
extern u32 D_801B2420;
extern s32 D_801B2424;
extern void (*D_800D4D04[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2420 = 1;
        D_801B2424 = 1;
    }

    if (D_801B2420 < 0x8)
    {
        D_800D4D04[D_801B2420]();
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
void func_8006F29C(void)
{
extern u32 D_801B2420;
extern s32 D_801B2424;
extern void (*D_800D4D04[])(void);

    D_801B2420 = 1;
    D_801B2424 = 1;
}

/** @brief Restore the default transform and advance the sequence. */
void func_8006F2B4(void)
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
extern s32 D_801B246C;
extern s32 D_801B2488;
extern s32 D_801B2420;
extern s32 D_801B2424;
extern void func_8006E024__for_func_8006F2B4(void) __asm__("func_8006E024");

    D_8013B238 = D_80139258;
    D_80139870 = D_80182DC0;
    D_801B2488 = 0;
    D_801B246C = 0x80;
    D_801B2424 = 4;
    D_801B2420++;
    func_8006E024__for_func_8006F2B4();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8006F360(void)
{
extern void func_8006E190__for_func_8006F360(void) __asm__("func_8006E190");
extern s32 D_801B2424;
extern s32 D_801B2420;

    D_801B2424 = 0x20;
    D_801B2420 += 1;
    func_8006E190__for_func_8006F360();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8006F398(void)
{
extern s32 D_801B2420;
extern void func_8006E2B0__for_func_8006F398(void) __asm__("func_8006E2B0");
extern s32 D_801B2424;

    D_801B2424 = 0x80;
    D_801B2420 += 1;
    func_8006E2B0__for_func_8006F398();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8006F3D0(void)
{
extern s32 D_801B2420;
extern void func_8006E2B0__for_func_8006F3D0(void) __asm__("func_8006E2B0");
extern s32 D_801B2424;

    D_801B2420 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8006F3E8(s32 arg0)
{
extern u32 D_801B2428;
extern s32 D_801B242C;
extern void (*D_800D4D24[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2428 = 1;
        D_801B242C = 1;
    }

    if (D_801B2428 < 0x6)
    {
        D_800D4D24[D_801B2428]();
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
void func_8006F458(void)
{
extern u32 D_801B2428;
extern s32 D_801B242C;
extern void (*D_800D4D24[])(void);

    D_801B2428 = 1;
    D_801B242C = 1;
}

/** @brief Restore the default transform and advance the sequence. */
void func_8006F470(void)
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
extern WmapOrientation D_801B24A8;
extern WmapTransform D_80182DC0;
extern WmapTransform D_801B2478;
extern s32 D_801B2470;
extern s32 D_801B2428;
extern s32 D_801B242C;
extern void func_8006E3E4__for_func_8006F470(void) __asm__("func_8006E3E4");

    D_801B24A8 = D_80139258;
    D_801B2478 = D_80182DC0;
    D_801B2470 = 0x80;
    D_801B242C = 1;
    D_801B2428++;
    func_8006E3E4__for_func_8006F470();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8006F514(void)
{
extern s32 D_801B2428;
extern void func_8006E544__for_func_8006F514(void) __asm__("func_8006E544");
extern s32 D_801B242C;

    D_801B242C = 0x20;
    D_801B2428 += 1;
    func_8006E544__for_func_8006F514();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8006F54C(void)
{
extern s32 D_801B2428;
extern void func_8006E544__for_func_8006F54C(void) __asm__("func_8006E544");
extern s32 D_801B242C;

    D_801B2428 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8006F564(s32 arg0)
{
extern u32 D_801B2430;
extern s32 D_801B2434;
extern void (*D_800D4D3C[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2430 = 1;
        D_801B2434 = 1;
    }

    if (D_801B2430 < 0x6)
    {
        D_800D4D3C[D_801B2430]();
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
void func_8006F5D4(void)
{
extern u32 D_801B2430;
extern s32 D_801B2434;
extern void (*D_800D4D3C[])(void);

    D_801B2430 = 1;
    D_801B2434 = 1;
}

/**
 * @brief World-map step handler: seed a 30-entry table and advance the step.
 * @note Best match ~84.86% (gcc280_g0); residual is loop induction-variable
 *       register allocation (permuter territory).
 */
void func_8006F5EC(void)
{
/* Partial WMAP decompilation: 84.857140% (gcc280_g0). */

extern u8 D_80139988[];
extern void *D_8011F538;
extern s16 D_801AFBD0;
extern s32 D_801B0FD0;
extern s32 D_80139980;
extern s32 D_801B2430;
extern s32 D_801B2434;
extern void func_8006F678__for_func_8006F5EC(void) __asm__("func_8006F678");

    s16 *p;
    s32 off;
    s32 i;

    D_801B0FD0 = 0;
    D_80139980 = 0x80;
    p = &D_801AFBD0;
    i = 0;
    for (off = 0x30; i < 30; off += 8)
    {
        *p = 0;
        i += 1;
        *(void **)((u8 *)&D_80139988 + off + 4) = &D_8011F538;
        p += 0xA;
    }
    D_801B2434 = 0xC;
    D_801B2430 += 1;
    func_8006F678__for_func_8006F5EC();
}

/** @brief World-map step tick: bump a counter, run the sub-step, and expire the timer. */
void func_8006F678(void)
{
extern s32 D_8011CF74;
extern s32 D_801B0FD0;
extern s32 D_801B2430;
extern s32 D_801B2434;
extern void func_8006E6B8__for_func_8006F678(void) __asm__("func_8006E6B8");

    if (D_8011CF74 & 1)
    {
        D_801B0FD0 += 1;
    }
    func_8006E6B8__for_func_8006F678();
    if (--D_801B2434 == 0)
    {
        D_801B2430 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8006F6E8(void)
{
extern void func_8006F720__for_func_8006F6E8(void) __asm__("func_8006F720");
extern s32 D_801B2434;
extern s32 D_801B2430;

    D_801B2434 = 0x40;
    D_801B2430 += 1;
    func_8006F720__for_func_8006F6E8();
}

/**
 * @brief World-map step tick: age two timers with zero clamps, run the sub-step,
 *        and expire the step counter.
 */
void func_8006F720(void)
{
extern s32 D_80139980;
extern s32 D_8011CF74;
extern s32 D_801B0FD0;
extern s32 D_801B2430;
extern s32 D_801B2434;
extern void func_8006E6B8__for_func_8006F720(void) __asm__("func_8006E6B8");

    D_80139980 -= 1;
    if (D_80139980 < 0)
    {
        D_80139980 = 0;
    }
    if ((D_8011CF74 & 3) == 0)
    {
        D_801B0FD0 -= 1;
    }
    if (D_801B0FD0 < 0)
    {
        D_801B0FD0 = 0;
    }
    func_8006E6B8__for_func_8006F720();
    if (--D_801B2434 == 0)
    {
        D_801B2430 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8006F7C4(void)
{
extern s32 D_801B2430;

    D_801B2430 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8006F7DC(s32 arg0)
{
extern u32 D_801B2438;
extern s32 D_801B243C;
extern void (*D_800D4D54[])(void);
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2438 = 1;
        D_801B243C = 1;
    }

    if (D_801B2438 < 0x6)
    {
        D_800D4D54[D_801B2438]();
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
void func_8006F84C(void)
{
extern u32 D_801B2438;
extern s32 D_801B243C;
extern void (*D_800D4D54[])(void);
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    D_801B2438 = 1;
    D_801B243C = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8006F864(void)
{
extern u32 D_801B2438;
extern s32 D_801B243C;
extern void (*D_800D4D54[])(void);
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 8, 0x2E, 0);
    if (--D_801B243C == 0)
    {
        D_801B2438 += 1;
    }
}

void func_8006F8E0(void)
{
extern void func_8006F92C__for_func_8006F8E0(void) __asm__("func_8006F92C");
extern s16 D_800D9268[];
extern s32 D_801B2438;
extern s32 D_801B243C;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    D_800D9268[0xD2 / 2] = 0;
    D_800D9268[0xD6 / 2] = 2;
    D_801B243C = 0x5D;
    D_801B2438 += 1;
    func_8006F92C__for_func_8006F8E0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8006F92C(void)
{
extern void func_8006F92C(void);
extern s16 D_800D9268[];
extern s32 D_801B2438;
extern s32 D_801B243C;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0x8, 0x2E, 0);
    if (--D_801B243C == 0)
    {
        D_801B2438 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8006F9A8(void)
{
extern s32 D_801B2438;

    D_801B2438 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8006F9C0(s32 arg0)
{
extern u32 D_801B2440;
extern s32 D_801B2444;
extern void (*D_800D4D6C[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2440 = 1;
        D_801B2444 = 1;
        return 1;
    }

    if (D_801B2440 < 0x4)
    {
        D_800D4D6C[D_801B2440]();
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
void func_8006FA38(void)
{
extern u32 D_801B2440;
extern s32 D_801B2444;
extern void (*D_800D4D6C[])(void);

    D_801B2440 = 1;
    D_801B2444 = 1;
}

/** @brief World-map step handler: init a UI descriptor block, set the timer, advance the step. */
void func_8006FA50(void)
{
extern void *D_801399B4;
extern s32 D_8011D538;
extern u8 D_800D9268;
extern s32 D_801B2440;
extern s32 D_801B2444;
extern void func_8006FAC8__for_func_8006FA50(void) __asm__("func_8006FAC8");

    u8 *base = &D_800D9268;

    D_801399B4 = &D_8011D538;
    *(u8 *)(base + 0xE2) = 0xF;
    *(s16 *)(base + 0xEA) = 1;
    *(s16 *)(base + 0xEC) = -1;
    *(s16 *)(base + 0xDE) = 0;
    *(s16 *)(base + 0xFE) = 0x80;
    *(s16 *)(base + 0x100) = 0x80;
    D_801B2444 = 0xC7;
    D_801B2440 += 1;
    func_8006FAC8__for_func_8006FA50();
}

/** @brief Draw the projected effect and advance after its countdown. */
void func_8006FAC8(void)
{
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_801B2440;
extern s32 D_801B2444;

    s32 screen_position;
    s32 remaining;
    u8 *actor = D_800D9344;

    func_8006CDDC();
    gte_stsxy(&screen_position);
    func_8006CC4C(actor, D_801399B0);
    func_80066F9C(actor, screen_position, 8, 0x2E, 0);
    remaining = D_801B2444 - 1;
    D_801B2444 = remaining;
    if (remaining == 0)
    {
        D_801B2440++;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8006FB4C(void)
{
extern s32 D_801B2440;

    D_801B2440 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8006FB64(s32 arg0)
{
extern u32 D_801B2400;
extern s32 D_801B2404;
extern void (*D_800D4C54[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2400 = 1;
        D_801B2404 = 1;
        return 1;
    }

    if (D_801B2400 < 0x16)
    {
        D_800D4C54[D_801B2400]();
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
void func_8006FBDC(void)
{
extern u32 D_801B2400;
extern s32 D_801B2404;
extern void (*D_800D4C54[])(void);

    D_801B2400 = 1;
    D_801B2404 = 1;
}

/** @brief Set world-map flags and color, then begin an eight-tick delay. */
void func_8006FBF4(void)
{
extern s32 D_80139244;
extern s32 D_801B2400;
extern s32 D_801B2404;

    D_80139244 = 1;
    g_wmap_backdrop_target_level = 8;
    func_8006683C(0x262726);
    D_801B2404 = 8;
    D_801B2400 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006FC4C(void)
{
extern s32 D_801B2404;
extern s32 D_801B2400;

    if (--D_801B2404 == 0)
    {
        D_801B2400 += 1;
    }
}

/** @brief Play sound 18, register two callbacks, and begin a seven-tick delay. */
void func_8006FC80(void)
{
extern s32 D_801ADAE0;
extern s32 D_801B2400;
extern s32 D_801B2404;
extern void func_8006F7DC__for_func_8006FC80(void) __asm__("func_8006F7DC");
extern void func_80070530__for_func_8006FC80(void) __asm__("func_80070530");

    func_800652A8(0x12, 0x80);
    func_8006CAC0(&func_80070530__for_func_8006FC80);
    D_801ADAE0 = 1;
    func_8006CAC0(&func_8006F7DC__for_func_8006FC80);
    D_801B2404 = 7;
    D_801B2400 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006FCE0(void)
{
extern s32 D_801B2404;
extern s32 D_801B2400;
extern void func_8006F564__for_func_8006FCE0(void) __asm__("func_8006F564");

    if (--D_801B2404 == 0)
    {
        D_801B2400 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8006FD14(void)
{
extern s32 D_801B2404;
extern s32 D_801B2400;
extern void func_8006F564__for_func_8006FD14(void) __asm__("func_8006F564");

    func_8006CAC0(func_8006F564__for_func_8006FD14);
    D_801B2404 = 0x34;
    D_801B2400 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006FD50(void)
{
extern s32 D_801B2404;
extern s32 D_801B2400;
extern void func_8006F06C__for_func_8006FD50(void) __asm__("func_8006F06C");

    if (--D_801B2404 == 0)
    {
        D_801B2400 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8006FD84(void)
{
extern s32 D_801B2404;
extern s32 D_801B2400;
extern void func_8006F06C__for_func_8006FD84(void) __asm__("func_8006F06C");

    func_8006CAC0(func_8006F06C__for_func_8006FD84);
    D_801B2404 = 0x3A;
    D_801B2400 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006FDC0(void)
{
extern s32 D_801B2404;
extern s32 D_801B2400;
extern void func_8006F22C__for_func_8006FDC0(void) __asm__("func_8006F22C");
extern void func_8006EF20__for_func_8006FDC0(void) __asm__("func_8006EF20");
extern void func_80070408__for_func_8006FDC0(void) __asm__("func_80070408");

    if (--D_801B2404 == 0)
    {
        D_801B2400 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_8006FDF4(void)
{
extern s32 D_801B2404;
extern s32 D_801B2400;
extern void func_8006F22C__for_func_8006FDF4(void) __asm__("func_8006F22C");
extern void func_8006EF20__for_func_8006FDF4(void) __asm__("func_8006EF20");
extern void func_80070408__for_func_8006FDF4(void) __asm__("func_80070408");

    func_8006CAC0(func_8006F22C__for_func_8006FDF4);
    func_8006CAC0(func_8006EF20__for_func_8006FDF4);
    func_8006CAC0(func_80070408__for_func_8006FDF4);
    D_801B2404 = 0x10;
    D_801B2400 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006FE48(void)
{
extern s32 D_801B2404;
extern s32 D_801B2400;
extern void func_8006F3E8__for_func_8006FE48(void) __asm__("func_8006F3E8");
extern void func_80070114__for_func_8006FE48(void) __asm__("func_80070114");

    if (--D_801B2404 == 0)
    {
        D_801B2400 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8006FE7C(void)
{
extern s32 D_801B2404;
extern s32 D_801B2400;
extern void func_8006F3E8__for_func_8006FE7C(void) __asm__("func_8006F3E8");
extern void func_80070114__for_func_8006FE7C(void) __asm__("func_80070114");

    func_8006CAC0(func_8006F3E8__for_func_8006FE7C);
    func_8006CAC0(func_80070114__for_func_8006FE7C);
    D_801B2404 = 0x18;
    D_801B2400 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006FEC4(void)
{
extern s32 D_801B2404;
extern s32 D_801B2400;
extern void func_8006F9C0__for_func_8006FEC4(void) __asm__("func_8006F9C0");

    if (--D_801B2404 == 0)
    {
        D_801B2400 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8006FEF8(void)
{
extern s32 D_801B2404;
extern s32 D_801B2400;
extern void func_8006F9C0__for_func_8006FEF8(void) __asm__("func_8006F9C0");

    func_8006CAC0(func_8006F9C0__for_func_8006FEF8);
    D_801B2404 = 0x2;
    D_801B2400 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006FF34(void)
{
extern s32 D_801B2404;
extern s32 D_801B2400;

    if (--D_801B2404 == 0)
    {
        D_801B2400 += 1;
    }
}

/** @brief World-map step: set fade colour then advance to the next handler. */
void func_8006FF68(void)
{
extern s32 D_801B2400;
extern s32 D_801B2404;

    func_8006683C(0x808080);
    D_801B2404 = 0x54;
    D_801B2400 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006FFA4(void)
{
extern s32 D_801B2404;
extern s32 D_801B2400;

    if (--D_801B2404 == 0)
    {
        D_801B2400 += 1;
    }
}

/** @brief World-map step handler: seed timers and advance the counter. */
void func_8006FFD8(void)
{
extern s32 D_80139244;
extern s32 D_801B2400;
extern s32 D_801B2404;

    D_80139244 = 0;
    g_wmap_backdrop_target_level = 0x10;
    D_801B2404 = 0x28;
    D_801B2400 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007000C(void)
{
extern s32 D_801B2404;
extern s32 D_801B2400;
extern void func_800702E0__for_func_8007000C(void) __asm__("func_800702E0");

    if (--D_801B2404 == 0)
    {
        D_801B2400 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80070040(void)
{
extern s32 D_801B2404;
extern s32 D_801B2400;
extern void func_800702E0__for_func_80070040(void) __asm__("func_800702E0");

    func_8006CAC0(func_800702E0__for_func_80070040);
    D_801B2404 = 0x46;
    D_801B2400 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007007C(void)
{
extern s32 D_801B2404;
extern s32 D_801B2400;

    if (--D_801B2404 == 0)
    {
        D_801B2400 += 1;
    }
}

void func_800700B0(void)
{
/* Partial WMAP decompilation: 95.600000% (gcc280_g0). */

/* Best so far: 95.6%. The target computes the packet slot address as
 * base + D_8011D530*40 + D_8011D510*240 with DIRECT byte scaling
 * (a*5<<3, b*15<<4) AND schedules the D_801B2400 counter load across the
 * packet store. A byte-pointer cast gives the direct scaling but creates an
 * alias barrier (counter load can't cross the cast store) -> 35%. The u32[]
 * index form below clears aliasing (correct schedule) but factors the *4 into
 * one extra shift -> 95.6% (1 insn off). Needs a form that yields both. */
extern s32 D_8013B20C;
extern u32 D_80139290[];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2400;

    D_8013B20C = 0;
    D_80139290[D_8011D530 * 10 + D_8011D510 * 60] = D_8011D4FC | 0x100;
    D_801B2400 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80070114(s32 arg0)
{
extern u32 D_801B2448;
extern s32 D_801B244C;
extern void (*D_800D4D7C[])(void);
extern void func_800701DC__for_func_80070114(void) __asm__("func_800701DC");
extern s32 D_801B248C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2448 = 1;
        D_801B244C = 1;
    }

    if (D_801B2448 < 0x6)
    {
        D_800D4D7C[D_801B2448]();
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
void func_80070184(void)
{
extern u32 D_801B2448;
extern s32 D_801B244C;
extern void (*D_800D4D7C[])(void);
extern void func_800701DC__for_func_80070184(void) __asm__("func_800701DC");
extern s32 D_801B248C;

    D_801B2448 = 1;
    D_801B244C = 1;
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007019C(void)
{
extern u32 D_801B2448;
extern s32 D_801B244C;
extern void (*D_800D4D7C[])(void);
extern void func_800701DC__for_func_8007019C(void) __asm__("func_800701DC");
extern s32 D_801B248C;

    D_801B248C = 0;
    D_801B244C = 0x3D;
    D_801B2448 += 1;
    func_800701DC__for_func_8007019C();
}

/** @brief Adjust the Z angle and advance when the countdown reaches zero. */
void func_800701DC(void)
{
extern SVECTOR D_801398C8;
extern s32 D_801B2448;
extern s32 D_801B244C;
extern u16 D_801B248C;

    s32 remaining_ticks;
    D_801398C8.vz = (u16)(D_801398C8.vz + D_801B248C);
    remaining_ticks = D_801B244C - 1;
    D_801B244C = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2448 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80070228(void)
{
extern void func_80070260__for_func_80070228(void) __asm__("func_80070260");
extern s32 D_801B244C;
extern s32 D_801B2448;

    D_801B244C = 0x80;
    D_801B2448 += 1;
    func_80070260__for_func_80070228();
}

/** @brief Reduce angular speed toward zero, update the Z angle, and count down the step. */
void func_80070260(void)
{
extern SVECTOR D_801398C8;
extern s32 D_801B2448;
extern s32 D_801B244C;
extern s32 D_801B248C;

    s32 speed;
    s32 remaining_ticks;
    speed = D_801B248C - 5;
    D_801B248C = speed;
    if (speed < 0)
    {
        D_801B248C = 0;
    }
    D_801398C8.vz = (u16)(D_801398C8.vz + (u16)D_801B248C);
    remaining_ticks = D_801B244C - 1;
    D_801B244C = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2448 += 1;
    }
}

/**
 * @brief Clear the sequence flag and advance the step counter.
 */
void func_800702C4(void)
{
extern s16 D_801398CC;
extern s32 D_801B2448;

    D_801398CC = 0;
    D_801B2448 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_800702E0(s32 arg0)
{
extern u32 D_801B2450;
extern s32 D_801B2454;
extern void (*D_800D4D94[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2450 = 1;
        D_801B2454 = 1;
    }

    if (D_801B2450 < 0x4)
    {
        D_800D4D94[D_801B2450]();
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
void func_80070350(void)
{
extern u32 D_801B2450;
extern s32 D_801B2454;
extern void (*D_800D4D94[])(void);

    D_801B2450 = 1;
    D_801B2454 = 1;
}

/** @brief World-map step: draw the active overlay while its timer runs, then tick down. */
void func_80070368(void)
{
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32 D_80182DEC;
extern s32 D_801B2450;
extern s32 D_801B2454;

    s32 c;

    if (D_80182DEC > 0)
    {
        func_8006CFE4(D_800DA448, D_80139CC8, 0x10, 0, D_80182DEC, 3);
    }
    D_80182DEC -= 4;
    c = D_801B2454 - 1;
    D_801B2454 = c;
    if (c == 0)
    {
        D_801B2450 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800703F0(void)
{
extern s32 D_801B2450;

    D_801B2450 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80070408(s32 arg0)
{
extern u32 D_801B2458;
extern s32 D_801B245C;
extern void (*D_800D4DA4[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2458 = 1;
        D_801B245C = 1;
    }

    if (D_801B2458 < 0x4)
    {
        D_800D4DA4[D_801B2458]();
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
void func_80070478(void)
{
extern u32 D_801B2458;
extern s32 D_801B245C;
extern void (*D_800D4DA4[])(void);

    D_801B2458 = 1;
    D_801B245C = 1;
}

/** @brief World-map step: draw the active overlay while its timer runs, then tick down. */
void func_80070490(void)
{
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_80182DF4;
extern s32 D_801B2458;
extern s32 D_801B245C;

    s32 c;

    if (D_80182DF4 > 0)
    {
        func_8006CFE4(D_800DB578, D_80139FE8, 0x28, 0, D_80182DF4, 3);
    }
    D_80182DF4 -= 4;
    c = D_801B245C - 1;
    D_801B245C = c;
    if (c == 0)
    {
        D_801B2458 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80070518(void)
{
extern s32 D_801B2458;

    D_801B2458 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80070530(s32 arg0)
{
extern u32 D_801B2460;
extern s32 D_801B2464;
extern void (*D_800D4DB4[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2460 = 1;
        D_801B2464 = 1;
    }

    if (D_801B2460 < 0x6)
    {
        D_800D4DB4[D_801B2460]();
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
void func_800705A0(void)
{
extern u32 D_801B2460;
extern s32 D_801B2464;
extern void (*D_800D4DB4[])(void);

    D_801B2460 = 1;
    D_801B2464 = 1;
}

/** @brief Restore the default transform and advance the sequence. */
void func_800705B8(void)
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
extern WmapOrientation D_8013B240;
extern WmapTransform D_80182DC0;
extern WmapTransform D_80139888;
extern s32 D_80182DE4;
extern s32 D_801B2460;
extern s32 D_801B2464;
extern void func_8006ECC0__for_func_800705B8(void) __asm__("func_8006ECC0");

    D_8013B240 = D_80139258;
    D_80139888 = D_80182DC0;
    D_80182DE4 = 0;
    D_80139888.words[2] = 0;
    D_801B2464 = 12;
    D_801B2460++;
    func_8006ECC0__for_func_800705B8();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80070660(void)
{
extern s32 D_801B2460;
extern void func_8006EDF4__for_func_80070660(void) __asm__("func_8006EDF4");
extern s32 D_801B2464;

    D_801B2464 = 0x28;
    D_801B2460 += 1;
    func_8006EDF4__for_func_80070660();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80070698(void)
{
extern s32 D_801B2460;
extern void func_8006EDF4__for_func_80070698(void) __asm__("func_8006EDF4");
extern s32 D_801B2464;

    D_801B2460 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800706B0(s32 arg0)
{
extern u32 D_801B2408;
extern s32 D_801B240C;
extern void (*D_800D4CAC[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2408 = 1;
        D_801B240C = 1;
        return 1;
    }

    if (D_801B2408 < 0x6)
    {
        D_800D4CAC[D_801B2408]();
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
void func_80070728(void)
{
extern u32 D_801B2408;
extern s32 D_801B240C;
extern void (*D_800D4CAC[])(void);

    D_801B2408 = 1;
    D_801B240C = 1;
}

/**
 * @brief Register a world-map callback and advance to the next step.
 */
void func_80070740(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2408;
extern void func_80070790__for_func_80070740(void) __asm__("func_80070790");

    g_wmap_input_locked = 1;
    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2408 += 1;
    func_80070790__for_func_80070740();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80070790(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2408;
extern void func_800707CC__for_func_80070790(void) __asm__("func_800707CC");

    if (D_8013B20C == 0)
    {
        D_801B2408 += 1;
        func_800707CC__for_func_80070790();
    }
}

/** @brief World-map step: register the next draw callback and advance to the next handler. */
void func_800707CC(void)
{
extern void func_8006FB64__for_func_800707CC(void) __asm__("func_8006FB64");
extern void func_80070808__for_func_800707CC(void) __asm__("func_80070808");
extern s32 D_801B2408;

    func_8006CAC0(func_8006FB64__for_func_800707CC);
    D_801B2408 += 1;
    func_80070808__for_func_800707CC();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80070808(void)
{
extern s32 D_801B2408;
extern s32 D_8013B20C;
extern void func_80070844__for_func_80070808(void) __asm__("func_80070844");

    if (D_8013B20C == 0)
    {
        D_801B2408 += 1;
        func_80070844__for_func_80070808();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80070844(void)
{
extern s32 D_801B2408;
extern s32 D_8013B20C;
extern void func_80070844(void);

    D_801B2408 += 1;
}
