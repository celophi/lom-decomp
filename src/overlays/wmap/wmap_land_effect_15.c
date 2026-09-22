#include "wmap_land_effect_15.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_sprite_render.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "sdk/rand.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"

/** @brief Draw nine map effect sprites with fixed positions and ordering depths. */
void func_8007B0D8(s32 color_mask)
{
extern u8 D_800DCF18[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern s32 D_80139234;
extern s32 D_8013923C;
extern void func_800675F0(u8 *, s32, s32, s32, s32, s32, s32, s32, s32, s32);

    func_800675F0(D_800DCF18, 0, 0x24, 0x35, 0x7800, 1, D_8013923C | color_mask, 0x46, -5, D_80139234);
    func_800675F0(D_8011CF1C, 0, 0x26, 0x35, 0x7800, 1, D_8013923C | color_mask, 0x64, -0x19, D_80139234);
    func_800675F0(D_8011CF24, 0, 0x22, 0x35, 0x7800, 1, D_8013923C | color_mask, 0x50, 0x1E, D_80139234);
    func_800675F0(D_800DCF18, 0, 0x20, 0x35, 0x7800, 1, D_8013923C | color_mask, 0x14, 0x28, D_80139234);
    func_800675F0(D_8011CF1C, 0, 0x1F, 0x35, 0x7800, 1, D_8013923C | color_mask, 0x32, 0x41, D_80139234);
    func_800675F0(D_8011CF24, 0, 0x21, 0x35, 0x7800, 1, D_8013923C | color_mask, -0x28, 0x23, D_80139234);
    func_800675F0(D_800DCF18, 0, 0x25, 0x35, 0x7800, 1, D_8013923C | color_mask, -0x28, -0x14, D_80139234);
    func_800675F0(D_8011CF1C, 0, 0x23, 0x35, 0x7800, 1, D_8013923C | color_mask, -0x44, 0xA, D_80139234);
    func_800675F0(D_8011CF24, 0, 0x27, 0x35, 0x7800, 1, D_8013923C | color_mask, -5, -0x32, D_80139234);
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8007B394(void)
{
extern s32 D_80139870[];
extern SVECTOR D_8013B238;
extern VECTOR D_8011CF60;
extern s32 D_80182DE4;
extern s32 D_8011CF28;
extern s32 D_801B270C;
extern s32 D_801B2708;

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

    if (D_80182DE4 != 0)
    {
        func_8006CD98(D_8011CF28, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE4);
        D_80182DE4 -= 0x4;
        if (D_80182DE4 < 0)
        {
            D_80182DE4 = 0;
        }
    }

    PopMatrix();
    if (--D_801B270C == 0)
    {
        D_801B2708 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8007B494(void)
{
extern s32 D_80139888[];
extern SVECTOR D_8013B240;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern s32 D_8011CF2C;
extern s32 D_801B2714;
extern s32 D_801B2710;

    MATRIX m;
    s32 x;

    x = D_80139888[2] - 0xDAC;
    D_80139888[2] = x;
    if (x < 0x2710)
    {
        D_80139888[2] = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_8013B240, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DE8 != 0)
    {
        func_8006CD98(D_8011CF2C, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE8);
        D_80182DE8 -= 0x4;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--D_801B2714 == 0)
    {
        D_801B2710 += 1;
    }
}

/** @brief Initialize eight effect actors with evenly spaced angles. */
void func_8007B594(void)
{
/* Partial WMAP decompilation: 94.914894% (gcc280_g0). */

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

extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern s32 D_8013B264;
extern s32 D_8013B270;
extern s32 D_8013B278;
extern s32 D_8013B280;
extern s32 D_80182DEC;
extern s32 D_801B0FD0;
extern s32 D_801B2720;
extern s32 D_801B2724;
extern WmapConfigA D_800DB578[];
extern WmapMotion D_801B0B70[];
extern WmapResource D_80139988[];
extern u8 D_8011D538[];
extern void func_8007CB90(void);

    s32 i;
    WmapConfigA *actor;
    D_801B0FD0 = 8;
    D_80182DEC = 0x7F;
    D_80139240 = 0x50;
    D_8013924C = 2;
    D_80139250 = -1;
    D_80139260 = 0x44C;
    D_80139264 = 0xC8;
    D_80139268 = 0x15;
    D_8013926C = 2;
    D_80139284 = 0;
    D_8013B264 = 0x28;
    D_8013B270 = 0x50;
    D_8013B278 = 0xA;
    D_8013B280 = 0x50;
    for (i = 0; i < 8; i++)
    {
        actor = &D_800DB578[i];
        D_80139988[i + 204].resource = D_8011D538;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_24 = 255;
        actor->field_26 = 3;
        actor->field_02 = 0;
        actor->field_22 = 0;
        actor->field_0E = D_8013926C;
        D_801B0B70[i].state = 1;
        D_801B0B70[i].scale = 80;
        D_801B0B70[i].angle = i << 9;
        D_801B0B70[i].x = 0;
        D_801B0B70[i].field_0E = 0;
        D_801B0B70[i].z = D_80139284;
    }
    D_801B2724 = 120;
    D_801B2720++;
    func_8007CB90();
}

/**
 * @brief Project and draw active world-map sparks, then respawn empty slots.
 * @note First pass projects each live spark through the GTE and advances its
 *       radius; second pass seeds fresh sparks up to the shared population cap.
 * @note GTE-tagged: best-effort structural match (mirrors func_8007115C); the
 *       gcc280_g0 diff harness cannot assemble the GTE mnemonics for a percent.
 */
void func_8007B70C(void)
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
extern WmapDraw D_800DA448[];
extern u8 D_80139CC8[];
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
    draw = D_800DA448;
    for (i = 0; i < 0x8; i++)
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
            func_8006CC4C(draw, &D_80139CC8[i * 8]);
            func_80066F9C(draw, screen, 0xD, 0x20, 0);
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
    draw = D_800DA448;
    for (i = 0; i < 0x8; i++)
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
            spark->timer = (rand() & 0x3C) + 0x20;
        }
        spark++;
        draw++;
    }
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007B910(s32 arg0)
{
extern u32 D_801B26E0;
extern s32 D_801B26E4;
extern void (*D_800D53B0[])(void);
extern void func_8007B9E4(void);
extern s32 D_8013B20C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B26E0 = 1;
        D_801B26E4 = 1;
        return 1;
    }

    if (D_801B26E0 < 0x6)
    {
        D_800D53B0[D_801B26E0]();
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
void func_8007B988(void)
{
extern u32 D_801B26E0;
extern s32 D_801B26E4;
extern void (*D_800D53B0[])(void);
extern void func_8007B9E4(void);
extern s32 D_8013B20C;

    D_801B26E0 = 1;
    D_801B26E4 = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8007B9A0(void)
{
extern u32 D_801B26E0;
extern s32 D_801B26E4;
extern void (*D_800D53B0[])(void);
extern void func_8007B9E4(void);
extern s32 D_8013B20C;

    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B26E0 += 1;
    func_8007B9E4();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8007B9E4(void)
{
extern s32 D_8013B20C;
extern s32 D_801B26E0;
extern void func_8007BA20(void);
extern void func_8007BAB8(void);
extern void func_8007BA64(void);

    if (D_8013B20C == 0)
    {
        D_801B26E0 += 1;
        func_8007BA20();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8007BA20(void)
{
extern s32 D_8013B20C;
extern s32 D_801B26E0;
extern void func_8007BA20(void);
extern void func_8007BAB8(void);
extern void func_8007BA64(void);

    func_8006CAC0(func_8007BAB8);
    D_8013B20C = 1;
    D_801B26E0 += 1;
    func_8007BA64();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8007BA64(void)
{
extern s32 D_801B26E0;
extern s32 D_8013B20C;
extern void func_8007BAA0(void);

    if (D_8013B20C == 0)
    {
        D_801B26E0 += 1;
        func_8007BAA0();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007BAA0(void)
{
extern s32 D_801B26E0;
extern s32 D_8013B20C;
extern void func_8007BAA0(void);

    D_801B26E0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007BAB8(s32 arg0)
{
extern u32 D_801B26E8;
extern s32 D_801B26EC;
extern void (*D_800D53C8[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B26E8 = 1;
        D_801B26EC = 1;
        return 1;
    }

    if (D_801B26E8 < 0x12)
    {
        D_800D53C8[D_801B26E8]();
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
void func_8007BB30(void)
{
extern u32 D_801B26E8;
extern s32 D_801B26EC;
extern void (*D_800D53C8[])(void);

    D_801B26E8 = 1;
    D_801B26EC = 1;
}

/** @brief World-map step handler: kick a sub-task and advance the step counter. */
void func_8007BB48(void)
{
extern s32 D_8013B208;
extern s32 D_801ADAF4;
extern s32 D_801B26E8;
extern s32 D_801B26EC;

    D_8013B208 = 1;
    func_8006683C(0x503030);
    D_801ADAF4 = 4;
    D_801B26EC = 0x10;
    D_801B26E8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007BB9C(void)
{
extern s32 D_801B26EC;
extern s32 D_801B26E8;

    if (--D_801B26EC == 0)
    {
        D_801B26E8 += 1;
    }
}

/**
 * @brief Set up a world-map sub-scene: request assets and register its handler.
 */
void func_8007BBD0(void)
{
extern s32 D_80139244;
extern s32 D_801B26E8;
extern s32 D_801B26EC;
extern void func_8007C5F8(void);

    D_80139244 = 1;
    func_800652A8(0x1B, 0x80);
    func_8006CAC0(func_8007C5F8);
    D_801B26EC = 8;
    D_801B26E8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007BC24(void)
{
extern s32 D_801B26EC;
extern s32 D_801B26E8;
extern void func_8007C898(void);
extern void func_8007C398(void);

    if (--D_801B26EC == 0)
    {
        D_801B26E8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8007BC58(void)
{
extern s32 D_801B26EC;
extern s32 D_801B26E8;
extern void func_8007C898(void);
extern void func_8007C398(void);

    func_8006CAC0(func_8007C898);
    func_8006CAC0(func_8007C398);
    D_801B26EC = 0x4;
    D_801B26E8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007BCA0(void)
{
extern s32 D_801B26EC;
extern s32 D_801B26E8;
extern void func_8007CB00(void);
extern void func_8007CCD8(void);

    if (--D_801B26EC == 0)
    {
        D_801B26E8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8007BCD4(void)
{
extern s32 D_801B26EC;
extern s32 D_801B26E8;
extern void func_8007CB00(void);
extern void func_8007CCD8(void);

    func_8006CAC0(func_8007CB00);
    func_8006CAC0(func_8007CCD8);
    D_801B26EC = 0x4;
    D_801B26E8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007BD1C(void)
{
extern s32 D_801B26EC;
extern s32 D_801B26E8;

    if (--D_801B26EC == 0)
    {
        D_801B26E8 += 1;
    }
}

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void func_8007BD50(void)
{
extern s32 D_801ADAE0;
extern s32 D_801B26E8;
extern s32 D_801B26EC;

    D_801ADAE0 = 1;
    D_801B26EC = 0x46;
    D_801B26E8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007BD7C(void)
{
extern s32 D_801B26EC;
extern s32 D_801B26E8;
extern void func_8007D1B8(void);

    if (--D_801B26EC == 0)
    {
        D_801B26E8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8007BDB0(void)
{
extern s32 D_801B26EC;
extern s32 D_801B26E8;
extern void func_8007D1B8(void);

    func_8006CAC0(func_8007D1B8);
    D_801B26EC = 0xC;
    D_801B26E8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007BDEC(void)
{
extern s32 D_801B26EC;
extern s32 D_801B26E8;
extern void func_8007BF7C(void);
extern void func_8007CF38(void);

    if (--D_801B26EC == 0)
    {
        D_801B26E8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8007BE20(void)
{
extern s32 D_801B26EC;
extern s32 D_801B26E8;
extern void func_8007BF7C(void);
extern void func_8007CF38(void);

    func_8006CAC0(func_8007BF7C);
    func_8006CAC0(func_8007CF38);
    D_801B26EC = 0x24;
    D_801B26E8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007BE68(void)
{
extern s32 D_801B26EC;
extern s32 D_801B26E8;
extern void func_8007C748(void);
extern void func_8007C1FC(void);

    if (--D_801B26EC == 0)
    {
        D_801B26E8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8007BE9C(void)
{
extern s32 D_801B26EC;
extern s32 D_801B26E8;
extern void func_8007C748(void);
extern void func_8007C1FC(void);

    func_8006CAC0(func_8007C748);
    func_8006CAC0(func_8007C1FC);
    D_801B26EC = 0x59;
    D_801B26E8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007BEE4(void)
{
extern s32 D_801B26EC;
extern s32 D_801B26E8;

    if (--D_801B26EC == 0)
    {
        D_801B26E8 += 1;
    }
}

void func_8007BF18(void)
{
/* Partial WMAP decompilation: 95.600000% (gcc280_g0). */

extern s32 D_8013B20C;
extern u32 D_80139290[];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B26E8;

    D_8013B20C = 0;
    D_80139290[D_8011D530 * 10 + D_8011D510 * 60] = D_8011D4FC | 0x100;
    D_801B26E8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007BF7C(s32 arg0)
{
extern u32 D_801B26F0;
extern s32 D_801B26F4;
extern void (*D_800D5410[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B26F0 = 1;
        D_801B26F4 = 1;
        return 1;
    }

    if (D_801B26F0 < 0x8)
    {
        D_800D5410[D_801B26F0]();
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
void func_8007BFF4(void)
{
extern u32 D_801B26F0;
extern s32 D_801B26F4;
extern void (*D_800D5410[])(void);

    D_801B26F0 = 1;
    D_801B26F4 = 1;
}

/** @brief Initialize effect values and an eight-tick countdown, then run its first update. */
void func_8007C00C(void)
{
extern void func_8007C05C(void);
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_801B26F0;
extern s32 D_801B26F4;

    D_80139234 = 0x10;
    D_8013923C = 0x80;
    D_801B26F4 = 8;
    D_801B26F0 += 1;
    func_8007C05C();
}

/** @brief Update the sequence effect and advance when its countdown reaches zero. */
void func_8007C05C(void)
{
extern void func_8007B0D8(s32);
extern s32 D_801B26F0;
extern s32 D_801B26F4;

    s32 remaining_ticks;

    func_8007B0D8(0x10000);
    remaining_ticks = D_801B26F4 - 1;
    D_801B26F4 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B26F0 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007C0A8(void)
{
extern void func_8007C0E0(void);
extern s32 D_801B26F4;
extern s32 D_801B26F0;

    D_801B26F4 = 0x28;
    D_801B26F0 += 1;
    func_8007C0E0();
}

/** @brief Reduce the effect value to a minimum of one and count down the sequence step. */
void func_8007C0E0(void)
{
extern void func_8007B0D8(s32);
extern s32 D_80139234;
extern s32 D_801B26F0;
extern s32 D_801B26F4;

    s32 value;
    s32 remaining_ticks;

    value = D_80139234 - 1;
    D_80139234 = value;
    if (value <= 0)
    {
        D_80139234 = 1;
    }
    func_8007B0D8(0x10000);
    remaining_ticks = D_801B26F4 - 1;
    D_801B26F4 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B26F0 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007C148(void)
{
extern void func_8007C180(void);
extern s32 D_801B26F4;
extern s32 D_801B26F0;

    D_801B26F4 = 0x8;
    D_801B26F0 += 1;
    func_8007C180();
}

/** @brief Reduce the effect value toward zero and advance when its countdown expires. */
void func_8007C180(void)
{
extern void func_8007B0D8(s32);
extern s32 D_8013923C;
extern s32 D_801B26F0;
extern s32 D_801B26F4;

    s32 value;
    s32 remaining_ticks;

    value = D_8013923C - 8;
    D_8013923C = value;
    if (value < 0)
    {
        D_8013923C = 0;
    }
    func_8007B0D8(1);
    remaining_ticks = D_801B26F4 - 1;
    D_801B26F4 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B26F0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007C1E4(void)
{
extern s32 D_801B26F0;

    D_801B26F0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007C1FC(s32 arg0)
{
extern u32 D_801B26F8;
extern s32 D_801B26FC;
extern void (*D_800D5430[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B26F8 = 1;
        D_801B26FC = 1;
        return 1;
    }

    if (D_801B26F8 < 0x4)
    {
        D_800D5430[D_801B26F8]();
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
void func_8007C274(void)
{
extern u32 D_801B26F8;
extern s32 D_801B26FC;
extern void (*D_800D5430[])(void);

    D_801B26F8 = 1;
    D_801B26FC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void func_8007C28C(void)
{
extern u8* D_801399AC;
extern u8 D_8011F538[];
extern u8 D_800D9318[];
extern s32 D_801B26F8;
extern s32 D_801B26FC;
extern void func_8007C304(void);

    D_801399AC = D_8011F538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x26] = 0;
    *(s16*)&D_800D9318[0x22] = 0x80;
    *(s16*)&D_800D9318[0x24] = 0x80;
    D_801B26FC = 0x5A;
    D_801B26F8 += 1;
    func_8007C304();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007C304(void)
{
extern s32 D_801B26F8;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B26FC;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0x16, 0x22, 0);
    if (--D_801B26FC == 0)
    {
        D_801B26F8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007C380(void)
{
extern s32 D_801B26F8;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B26FC;

    D_801B26F8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007C398(s32 arg0)
{
extern u32 D_801B2700;
extern s32 D_801B2704;
extern void (*D_800D5440[])(void);
extern void func_8007C49C(void);
extern u8 D_8011D538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2700 = 1;
        D_801B2704 = 1;
    }

    if (D_801B2700 < 0x6)
    {
        D_800D5440[D_801B2700]();
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
void func_8007C408(void)
{
extern u32 D_801B2700;
extern s32 D_801B2704;
extern void (*D_800D5440[])(void);
extern void func_8007C49C(void);
extern u8 D_8011D538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    D_801B2700 = 1;
    D_801B2704 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8007C420(void)
{
extern u32 D_801B2700;
extern s32 D_801B2704;
extern void (*D_800D5440[])(void);
extern void func_8007C49C(void);
extern u8 D_8011D538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    D_801399BC = D_8011D538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0x10] = -1;
    *(s16*)&D_800D9370[0x26] = 2;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0xE] = 0;
    *(s16*)&D_800D9370[0x22] = 0x80;
    *(s16*)&D_800D9370[0x24] = 0x80;
    D_801B2704 = 0x56;
    D_801B2700 += 1;
    func_8007C49C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007C49C(void)
{
extern u32 D_801B2700;
extern s32 D_801B2704;
extern void (*D_800D5440[])(void);
extern void func_8007C49C(void);
extern u8 D_8011D538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0x15, 0xB, 0);
    if (--D_801B2704 == 0)
    {
        D_801B2700 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8007C518(void)
{
extern void func_8007C564(void);
extern s16 D_800D9370[];
extern s32 D_801B2704;
extern s32 D_801B2700;
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    D_800D9370[19] = 8;
    D_800D9370[17] = 0;
    D_801B2704 = 0x40;
    D_801B2700 += 1;
    func_8007C564();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007C564(void)
{
extern void func_8007C564(void);
extern s16 D_800D9370[];
extern s32 D_801B2704;
extern s32 D_801B2700;
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0x15, 0xB, 0);
    if (--D_801B2704 == 0)
    {
        D_801B2700 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007C5E0(void)
{
extern s32 D_801B2700;

    D_801B2700 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007C5F8(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2708;
extern s32 D_801B270C;
extern void (*D_800D5458[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139870;
extern s32 D_80182DE4;
extern void func_8007B394(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2708 = 1;
        D_801B270C = 1;
    }

    if (D_801B2708 < 0x4)
    {
        D_800D5458[D_801B2708]();
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
void func_8007C668(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2708;
extern s32 D_801B270C;
extern void (*D_800D5458[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139870;
extern s32 D_80182DE4;
extern void func_8007B394(void);

    D_801B2708 = 1;
    D_801B270C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8007C680(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2708;
extern s32 D_801B270C;
extern void (*D_800D5458[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139870;
extern s32 D_80182DE4;
extern void func_8007B394(void);

    D_8013B238 = D_80139258;
    D_80139870 = D_80182DC0;
    D_80182DE4 = 0x80;
    D_80139870.w[2] = 0xAFC8;
    D_801B270C = 0x20;
    D_801B2708 += 1;
    func_8007B394();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007C730(void)
{
extern s32 D_801B2708;

    D_801B2708 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007C748(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2710;
extern s32 D_801B2714;
extern void (*D_800D5468[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139888;
extern s32 D_80182DE8;
extern void func_8007B494(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2710 = 1;
        D_801B2714 = 1;
    }

    if (D_801B2710 < 0x4)
    {
        D_800D5468[D_801B2710]();
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
void func_8007C7B8(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2710;
extern s32 D_801B2714;
extern void (*D_800D5468[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139888;
extern s32 D_80182DE8;
extern void func_8007B494(void);

    D_801B2710 = 1;
    D_801B2714 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8007C7D0(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2710;
extern s32 D_801B2714;
extern void (*D_800D5468[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_80139888;
extern s32 D_80182DE8;
extern void func_8007B494(void);

    D_8013B240 = D_80139258;
    D_80139888 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_80139888.w[2] = 0xAFC8;
    D_801B2714 = 0x20;
    D_801B2710 += 1;
    func_8007B494();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007C880(void)
{
extern s32 D_801B2710;

    D_801B2710 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007C898(s32 arg0)
{
extern u32 D_801B2718;
extern s32 D_801B271C;
extern void (*D_800D5478[])(void);
extern void func_8007C9A4(void);
extern u8 D_8011D538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2718 = 1;
        D_801B271C = 1;
        return 1;
    }

    if (D_801B2718 < 0x6)
    {
        D_800D5478[D_801B2718]();
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
void func_8007C910(void)
{
extern u32 D_801B2718;
extern s32 D_801B271C;
extern void (*D_800D5478[])(void);
extern void func_8007C9A4(void);
extern u8 D_8011D538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    D_801B2718 = 1;
    D_801B271C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8007C928(void)
{
extern u32 D_801B2718;
extern s32 D_801B271C;
extern void (*D_800D5478[])(void);
extern void func_8007C9A4(void);
extern u8 D_8011D538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    D_801399CC = D_8011D538;
    D_800D93C8[0x6] = 0xF;
    *(s16*)&D_800D93C8[0xE] = 1;
    *(s16*)&D_800D93C8[0x10] = -1;
    *(s16*)&D_800D93C8[0x2] = 0;
    *(s16*)&D_800D93C8[0x26] = 0;
    *(s16*)&D_800D93C8[0x22] = 0x80;
    *(s16*)&D_800D93C8[0x24] = 0x80;
    D_801B271C = 0x10;
    D_801B2718 += 1;
    func_8007C9A4();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007C9A4(void)
{
extern u32 D_801B2718;
extern s32 D_801B271C;
extern void (*D_800D5478[])(void);
extern void func_8007C9A4(void);
extern u8 D_8011D538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_8011CF4C, 0x15, 0x5, 0);
    if (--D_801B271C == 0)
    {
        D_801B2718 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8007CA20(void)
{
extern void func_8007CA6C(void);
extern s16 D_800D93C8[];
extern s32 D_801B271C;
extern s32 D_801B2718;
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    D_800D93C8[19] = 8;
    D_800D93C8[17] = 0;
    D_801B271C = 0x10;
    D_801B2718 += 1;
    func_8007CA6C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007CA6C(void)
{
extern void func_8007CA6C(void);
extern s16 D_800D93C8[];
extern s32 D_801B271C;
extern s32 D_801B2718;
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_8011CF4C, 0x15, 0x5, 0);
    if (--D_801B271C == 0)
    {
        D_801B2718 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007CAE8(void)
{
extern s32 D_801B2718;

    D_801B2718 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007CB00(s32 arg0)
{
extern u32 D_801B2720;
extern s32 D_801B2724;
extern void (*D_800D5490[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2720 = 1;
        D_801B2724 = 1;
        return 1;
    }

    if (D_801B2720 < 0x6)
    {
        D_800D5490[D_801B2720]();
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
void func_8007CB78(void)
{
extern u32 D_801B2720;
extern s32 D_801B2724;
extern void (*D_800D5490[])(void);

    D_801B2720 = 1;
    D_801B2724 = 1;
}

/**
 * @brief Draw a world-map element, then advance the step when its wait expires.
 */
void func_8007CB90(void)
{
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_80182DEC;
extern s32 D_801B2720;
extern s32 D_801B2724;

    func_8006D014(D_800DB578, D_80139FE8, 8, 1, D_80182DEC, 8, 2);
    if (--D_801B2724 == 0)
    {
        D_801B2720 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007CC0C(void)
{
extern void func_8007CC44(void);
extern s32 D_801B2724;
extern s32 D_801B2720;

    D_801B2724 = 0x14;
    D_801B2720 += 1;
    func_8007CC44();
}

/**
 * @brief Draw a world-map element, then advance the step when its wait expires.
 */
void func_8007CC44(void)
{
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_80182DEC;
extern s32 D_801B2720;
extern s32 D_801B2724;

    func_8006D014(D_800DB578, D_80139FE8, 8, 1, D_80182DEC, 8, 2);
    if (--D_801B2724 == 0)
    {
        D_801B2720 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007CCC0(void)
{
extern s32 D_801B2720;

    D_801B2720 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007CCD8(s32 arg0)
{
extern u32 D_801B2728;
extern s32 D_801B272C;
extern void (*D_800D54A8[])(void);
extern void func_8007CDDC(void);
extern u8 D_8011D538[];
extern u8* D_801399D4;
extern u8 D_800D93F4[];
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2728 = 1;
        D_801B272C = 1;
    }

    if (D_801B2728 < 0x6)
    {
        D_800D54A8[D_801B2728]();
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
void func_8007CD48(void)
{
extern u32 D_801B2728;
extern s32 D_801B272C;
extern void (*D_800D54A8[])(void);
extern void func_8007CDDC(void);
extern u8 D_8011D538[];
extern u8* D_801399D4;
extern u8 D_800D93F4[];
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

    D_801B2728 = 1;
    D_801B272C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8007CD60(void)
{
extern u32 D_801B2728;
extern s32 D_801B272C;
extern void (*D_800D54A8[])(void);
extern void func_8007CDDC(void);
extern u8 D_8011D538[];
extern u8* D_801399D4;
extern u8 D_800D93F4[];
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

    D_801399D4 = D_8011D538;
    D_800D93F4[0x6] = 0xF;
    *(s16*)&D_800D93F4[0xE] = 3;
    *(s16*)&D_800D93F4[0x10] = -1;
    *(s16*)&D_800D93F4[0x2] = 0;
    *(s16*)&D_800D93F4[0x26] = 0;
    *(s16*)&D_800D93F4[0x22] = 0x80;
    *(s16*)&D_800D93F4[0x24] = 0x80;
    D_801B272C = 0x50;
    D_801B2728 += 1;
    func_8007CDDC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007CDDC(void)
{
extern u32 D_801B2728;
extern s32 D_801B272C;
extern void (*D_800D54A8[])(void);
extern void func_8007CDDC(void);
extern u8 D_8011D538[];
extern u8* D_801399D4;
extern u8 D_800D93F4[];
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D93F4, D_801399D0);
    func_80066F9C(D_800D93F4, D_8011CF4C, 0x15, 0xB, 0);
    if (--D_801B272C == 0)
    {
        D_801B2728 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8007CE58(void)
{
extern void func_8007CEA4(void);
extern s16 D_800D93F4[];
extern s32 D_801B272C;
extern s32 D_801B2728;
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

    D_800D93F4[19] = 8;
    D_800D93F4[17] = 0;
    D_801B272C = 0x10;
    D_801B2728 += 1;
    func_8007CEA4();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007CEA4(void)
{
extern void func_8007CEA4(void);
extern s16 D_800D93F4[];
extern s32 D_801B272C;
extern s32 D_801B2728;
extern u8 D_801399D0[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D93F4, D_801399D0);
    func_80066F9C(D_800D93F4, D_8011CF4C, 0x15, 0xB, 0);
    if (--D_801B272C == 0)
    {
        D_801B2728 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007CF20(void)
{
extern s32 D_801B2728;

    D_801B2728 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007CF38(s32 arg0)
{
extern u32 D_801B2730;
extern s32 D_801B2734;
extern void (*D_800D54C0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2730 = 1;
        D_801B2734 = 1;
        return 1;
    }

    if (D_801B2730 < 0x6)
    {
        D_800D54C0[D_801B2730]();
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
void func_8007CFB0(void)
{
extern u32 D_801B2730;
extern s32 D_801B2734;
extern void (*D_800D54C0[])(void);

    D_801B2730 = 1;
    D_801B2734 = 1;
}

/**
 * @brief World-map step handler: seed an 8-entry table and advance the step.
 * @note Best match ~84.86% (gcc280_g0); residual is loop induction-variable
 *       register allocation (permuter territory).
 */
void func_8007CFC8(void)
{
/* Partial WMAP decompilation: 84.857140% (gcc280_g0). */

extern u8 D_80139988[];
extern void *D_80121538;
extern s16 D_801AFBD0;
extern s32 D_801B0FD0;
extern s32 D_80139980;
extern s32 D_801B2730;
extern s32 D_801B2734;
extern void func_8007D054(void);

    s16 *p;
    s32 off;
    s32 i;

    D_801B0FD0 = 0;
    D_80139980 = 0x7F;
    p = &D_801AFBD0;
    i = 0;
    for (off = 0x340; i < 8; off += 8)
    {
        *p = 0;
        i += 1;
        *(void **)((u8 *)&D_80139988 + off + 4) = &D_80121538;
        p += 0xA;
    }
    D_801B2734 = 0x20;
    D_801B2730 += 1;
    func_8007D054();
}

/** @brief World-map step tick: bump a counter, run the sub-step, and expire the timer. */
void func_8007D054(void)
{
extern s32 D_8011CF74;
extern s32 D_801B0FD0;
extern s32 D_801B2730;
extern s32 D_801B2734;
extern void func_8007B70C(void);

    if (D_8011CF74 & 1)
    {
        D_801B0FD0 += 1;
    }
    func_8007B70C();
    if (--D_801B2734 == 0)
    {
        D_801B2730 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007D0C4(void)
{
extern void func_8007D0FC(void);
extern s32 D_801B2734;
extern s32 D_801B2730;

    D_801B2734 = 0x40;
    D_801B2730 += 1;
    func_8007D0FC();
}

/**
 * @brief World-map step tick: age two timers with zero clamps, run the sub-step,
 *        and expire the step counter.
 */
void func_8007D0FC(void)
{
extern s32 D_80139980;
extern s32 D_8011CF74;
extern s32 D_801B0FD0;
extern s32 D_801B2730;
extern s32 D_801B2734;
extern void func_8007B70C(void);

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
    func_8007B70C();
    if (--D_801B2734 == 0)
    {
        D_801B2730 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007D1A0(void)
{
extern s32 D_801B2730;

    D_801B2730 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007D1B8(s32 arg0)
{
extern u32 D_801B2738;
extern s32 D_801B273C;
extern void (*D_800D54D8[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2738 = 1;
        D_801B273C = 1;
        return 1;
    }

    if (D_801B2738 < 0x6)
    {
        D_800D54D8[D_801B2738]();
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
void func_8007D230(void)
{
extern u32 D_801B2738;
extern s32 D_801B273C;
extern void (*D_800D54D8[])(void);

    D_801B2738 = 1;
    D_801B273C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8007D248(void)
{
extern u8* D_80139B3C;
extern u8 D_8011D538[];
extern u8 D_800D9BB0[];
extern s32 D_801B2738;
extern s32 D_801B273C;
extern void func_8007D2CC(void);

    D_80139B3C = D_8011D538;
    D_800D9BB0[0x6] = 0xF;
    *(s16*)&D_800D9BB0[0xE] = 4;
    *(s16*)&D_800D9BB0[0x10] = -1;
    *(s16*)&D_800D9BB0[0x26] = 8;
    *(s16*)&D_800D9BB0[0x22] = 0x81;
    *(s16*)&D_800D9BB0[0x2] = 0;
    *(s16*)&D_800D9BB0[0x24] = 1;
    D_801B273C = 0x40;
    D_801B2738 += 1;
    func_8007D2CC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007D2CC(void)
{
extern void func_8007D394(void);
extern s16 D_800D9BB0[];
extern s32 D_801B273C;
extern s32 D_801B2738;
extern u8 D_80139B38[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9BB0, D_80139B38);
    func_80066F9C(D_800D9BB0, D_8011CF4C, 0x15, 0x28, 0);
    if (--D_801B273C == 0)
    {
        D_801B2738 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8007D348(void)
{
extern void func_8007D394(void);
extern s16 D_800D9BB0[];
extern s32 D_801B273C;
extern s32 D_801B2738;
extern u8 D_80139B38[];
extern s32 D_8011CF4C;

    D_800D9BB0[19] = 8;
    D_800D9BB0[17] = 0;
    D_801B273C = 0x10;
    D_801B2738 += 1;
    func_8007D394();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8007D394(void)
{
extern void func_8007D394(void);
extern s16 D_800D9BB0[];
extern s32 D_801B273C;
extern s32 D_801B2738;
extern u8 D_80139B38[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9BB0, D_80139B38);
    func_80066F9C(D_800D9BB0, D_8011CF4C, 0x15, 0x28, 0);
    if (--D_801B273C == 0)
    {
        D_801B2738 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007D410(void)
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
    s16 unk0;
    s16 unk2;
    s32 unk4;
    s32 unk8;
    s16 unkC;
    s16 unkE;
    s32 unk10;
} WmapAfcEntry;

typedef struct
{
    s32 unk0;
    void *unk4;
} WmapPair;

extern WmapD94Entry D_800D94D0[];
extern s32 D_8011D538[];
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern WmapPair D_80139988[];
extern s32 D_8013B264;
extern s32 D_8013B270;
extern s32 D_8013B278;
extern s32 D_8013B280;
extern s32 D_80182DEC;
extern WmapAfcEntry D_801AFC98[];
extern s32 D_801B0FD0;
extern s32 D_801B2760;
extern s32 D_801B2764;

extern void func_8007E510(void);
extern s32 D_801B2738;
extern s32 D_80139888[];
extern SVECTOR D_8013B240;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B276C;
extern s32 D_801B2768;

    D_801B2738 += 1;
}
