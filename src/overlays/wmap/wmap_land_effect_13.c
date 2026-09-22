#include "wmap_land_effect_13.h"
#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "vector.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"

/** @brief Project active particles and initialize the first available slot. */
void func_80074368(WmapConfigA *actors, WmapResource *resources, s32 count)
{
/* Partial WMAP decompilation: 94.930070% (gcc280_g0). */

/** @brief World-map actor configuration. */


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



extern WmapMotion D_801AFBD0[];
extern s32 D_800D922C;
extern s32 D_801B0FD0;
extern s32 rand(void);

    SVECTOR position;
    s32 screen_position;
    s32 active_count;
    s32 i;
    WmapMotion *motion;
    WmapConfigA *actor;

    active_count = 0;
    for (i = 0; i < count; i++)
    {
        motion = &D_801AFBD0[i];
        actor = &actors[i];
        if (motion->state != 0)
        {
            position.vx = ((motion->z >> 3) * (ccos(motion->angle) >> 6)) >> 12;
            position.vy = ((motion->z >> 3) * (csin(motion->angle) >> 6)) >> 12;
            position.vz = motion->field_0E;
            gte_ldv0(&position);
            gte_rtps();
            motion->field_0E += motion->x;
            motion->z += 1000;
            gte_stsxy(&screen_position);
            func_8006CC4C(actor, &resources[i]);
            func_80066F9C(actor, screen_position, 15, 7, 0);
            motion->scale--;
            if (motion->scale == 0)
            {
                motion->state = 0;
            }
            active_count++;
        }
    }
    D_800D922C = active_count;
    for (i = 0; i < count; i++)
    {
        motion = &D_801AFBD0[i];
        actor = &actors[i];
        if (motion->state == 0)
        {
            if (D_801B0FD0 >= active_count)
            {
                actor->field_06 = 15;
                actor->field_0E = 3;
                actor->field_10 = -1;
                actor->field_22 = 129;
                actor->field_24 = 129;
                actor->field_02 = 0;
                motion->state = 1;
                motion->angle = rand();
                motion->z = 10000;
                motion->x = ((rand() * 5) >> 15) + 1;
                motion->scale = ((rand() * 2) >> 15) + 24;
                motion->field_0E = 0;
            }
            break;
        }
    }
}

/**
 * @brief Draw a world-map actor, seeding its position from a wrapping timer.
 * @note Copies the shared timer low half into two object fields, renders the
 *       actor at a cursor-relative offset, advances the timer and wraps it.
 * @note The dead Vec2s reproduces the original -0x30 frame reservation (FRAME-01).
 * @note Best match ~77% (gcc280_g0); residual is a callee-saved register tie
 *       (obj vs pos in s0/s1) plus a scheduling order difference on the timer read.
 */
void func_800745A4(void)
{
/* Partial WMAP decompilation: 77.309090% (gcc280_g0). */

extern u8 D_800D9528[];
extern u8 D_80139A08[];
extern s32 D_8011CF4C;
extern s32 D_80182DEC;
extern s32 D_801B25A0;
extern s32 D_801B25A4;

    u8* obj = D_800D9528;
    Vec2s scratch;
    s32 pos;

    pos = ((*(u16*)&D_8011CF4C - 0x6) & 0xFFFF) | ((*(u16*)((u8*)&D_8011CF4C + 0x2) + 0xC) << 16);
    *(s16*)(&obj[0x24]) = *(u16*)&D_80182DEC;
    *(s16*)(&obj[0x22]) = *(u16*)&D_80182DEC;
    func_8006CC4C(obj, D_80139A08);
    func_80066F9C(obj, pos, 0x4, 0xB, 0);
    D_80182DEC += 0x8;
    if (D_80182DEC >= 0x82)
    {
        D_80182DEC = 0x81;
    }
    if (--D_801B25A4 == 0)
    {
        D_801B25A0 += 1;
    }
}

/**
 * @brief Draw a world-map actor at a nudged copy of the cursor, then decay a timer.
 * @note Packs a per-actor coordinate offset from the cursor halves, renders the
 *       actor, copies a shared timer into two object fields, and steps counters.
 * @note Best match ~94.3% (gcc280_g0); residual is a callee-saved register tie
 *       (obj base vs packed pos land in s0/s1 swapped). The dead Vec2s reproduces
 *       the original -0x30 frame reservation (FRAME-01).
 */
void func_80074680(void)
{
/* Partial WMAP decompilation: 94.260000% (gcc280_g0). */

extern u8 D_800D9528[];
extern u8 D_80139A08[];
extern s32 D_8011CF4C;
extern s32 D_80182DEC;
extern s32 D_801B25A0;
extern s32 D_801B25A4;

    Vec2s scratch;
    s32 pos;

    pos = ((*(u16*)&D_8011CF4C - 0x6) & 0xFFFF) | ((*(u16*)((u8*)&D_8011CF4C + 0x2) + 0xC) << 16);
    func_8006CC4C(D_800D9528, D_80139A08);
    func_80066F9C(D_800D9528, pos, 0x4, 0xB, 0);
    *(s16*)(&D_800D9528[0x24]) = *(u16*)&D_80182DEC;
    *(s16*)(&D_800D9528[0x22]) = *(u16*)&D_80182DEC;
    D_80182DEC -= 0x8;
    if (D_80182DEC < 0)
    {
        D_80182DEC = 0;
    }
    if (--D_801B25A4 == 0)
    {
        D_801B25A0 += 1;
    }
}

/** @brief Draw the actor, increase its scale, and advance when its timer expires. */
void func_80074748(void)
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

extern WmapConfigA D_800D9554;
extern u8 *D_80139A10;
extern u16 D_8011CF4C[2];
extern s32 D_801B25DC;
extern s32 D_801B25A8;
extern s32 D_801B25AC;

    union
    {
        u16 half[2];
        s32 word;
    } position;

    position.half[0] = D_8011CF4C[0] + 16;
    position.half[1] = D_8011CF4C[1];
    D_800D9554.field_24 = *(u16 *)&D_801B25DC;
    D_800D9554.field_22 = *(u16 *)&D_801B25DC;
    func_8006CC4C(&D_800D9554, &D_80139A10);
    func_80066F9C(&D_800D9554, position.word, 4, 11, 0);
    D_801B25DC += 8;
    if (D_801B25DC >= 0x82)
    {
        D_801B25DC = 0x81;
    }
    if (--D_801B25AC == 0)
    {
        D_801B25A8++;
    }
}

/** @brief Draw the actor, reduce its scale, and advance when its timer expires. */
void func_80074820(void)
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

extern WmapConfigA D_800D9554;
extern u8 *D_80139A10;
extern u16 D_8011CF4C[2];
extern s32 D_801B25DC;
extern s32 D_801B25A8;
extern s32 D_801B25AC;

    union
    {
        u16 half[2];
        s32 word;
    } position;

    position.half[0] = D_8011CF4C[0] + 16;
    position.half[1] = D_8011CF4C[1];
    func_8006CC4C(&D_800D9554, &D_80139A10);
    func_80066F9C(&D_800D9554, position.word, 4, 11, 0);
    D_800D9554.field_24 = *(u16 *)&D_801B25DC;
    D_800D9554.field_22 = *(u16 *)&D_801B25DC;
    D_801B25DC -= 8;
    if (D_801B25DC < 0)
    {
        D_801B25DC = 0;
    }
    if (--D_801B25AC == 0)
    {
        D_801B25A8++;
    }
}

/** @brief Draw the actor, increase its scale, and advance when its timer expires. */
void func_800748E4(void)
{
/* Partial WMAP decompilation: 88.090910% (gcc280_g0). */

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

extern WmapConfigA D_800D9580;
extern u8 *D_80139A18;
extern u16 D_8011CF4C[2];
extern s32 D_801B25E0;
extern s32 D_801B25B0;
extern s32 D_801B25B4;

    union
    {
        u16 half[2];
        s32 word;
    } position;

    position.half[0] = D_8011CF4C[0] + 24;
    position.half[1] = D_8011CF4C[1] + 4;
    D_800D9580.field_24 = *(u16 *)&D_801B25E0;
    D_800D9580.field_22 = *(u16 *)&D_801B25E0;
    func_8006CC4C(&D_800D9580, &D_80139A18);
    func_80066F9C(&D_800D9580, position.word, 4, 11, 0);
    D_801B25E0 += 8;
    if (D_801B25E0 >= 0x82)
    {
        D_801B25E0 = 0x81;
    }
    if (--D_801B25B4 == 0)
    {
        D_801B25B0++;
    }
}

/**
 * @brief Draw a world-map actor at a nudged copy of the cursor, then decay a timer.
 * @note Packs a per-actor coordinate offset from the cursor halves, renders the
 *       actor, copies a shared timer into two object fields, and steps counters.
 * @note Best match ~94.3% (gcc280_g0); residual is a callee-saved register tie
 *       (obj base vs packed pos land in s0/s1 swapped). The dead Vec2s reproduces
 *       the original -0x30 frame reservation (FRAME-01).
 */
void func_800749C0(void)
{
/* Partial WMAP decompilation: 94.260000% (gcc280_g0). */

extern u8 D_800D9580[];
extern u8 D_80139A18[];
extern s32 D_8011CF4C;
extern s32 D_801B25E0;
extern s32 D_801B25B0;
extern s32 D_801B25B4;

    Vec2s scratch;
    s32 pos;

    pos = ((*(u16*)&D_8011CF4C + 0x18) & 0xFFFF) | ((*(u16*)((u8*)&D_8011CF4C + 0x2) + 0x4) << 16);
    func_8006CC4C(D_800D9580, D_80139A18);
    func_80066F9C(D_800D9580, pos, 0x4, 0xB, 0);
    *(s16*)(&D_800D9580[0x24]) = *(u16*)&D_801B25E0;
    *(s16*)(&D_800D9580[0x22]) = *(u16*)&D_801B25E0;
    D_801B25E0 -= 0x8;
    if (D_801B25E0 < 0)
    {
        D_801B25E0 = 0;
    }
    if (--D_801B25B4 == 0)
    {
        D_801B25B0 += 1;
    }
}

/**
 * @brief Draw a world-map actor, seeding its position from a wrapping timer.
 * @note Copies the shared timer low half into two object fields, renders the
 *       actor at a cursor-relative offset, advances the timer and wraps it.
 * @note The dead Vec2s reproduces the original -0x30 frame reservation (FRAME-01).
 * @note Best match ~77% (gcc280_g0); residual is a callee-saved register tie
 *       (obj vs pos in s0/s1) plus a scheduling order difference on the timer read.
 */
void func_80074A88(void)
{
/* Partial WMAP decompilation: 77.309090% (gcc280_g0). */

extern u8 D_800D95AC[];
extern u8 D_80139A20[];
extern s32 D_8011CF4C;
extern s32 D_801B25E4;
extern s32 D_801B25B8;
extern s32 D_801B25BC;

    u8* obj = D_800D95AC;
    Vec2s scratch;
    s32 pos;

    pos = ((*(u16*)&D_8011CF4C - 0x18) & 0xFFFF) | ((*(u16*)((u8*)&D_8011CF4C + 0x2) - 0xA) << 16);
    *(s16*)(&obj[0x24]) = *(u16*)&D_801B25E4;
    *(s16*)(&obj[0x22]) = *(u16*)&D_801B25E4;
    func_8006CC4C(obj, D_80139A20);
    func_80066F9C(obj, pos, 0x4, 0xB, 0);
    D_801B25E4 += 0x8;
    if (D_801B25E4 >= 0x82)
    {
        D_801B25E4 = 0x81;
    }
    if (--D_801B25BC == 0)
    {
        D_801B25B8 += 1;
    }
}

/**
 * @brief Draw a world-map actor at a nudged copy of the cursor, then decay a timer.
 * @note Packs a per-actor coordinate offset from the cursor halves, renders the
 *       actor, copies a shared timer into two object fields, and steps counters.
 * @note Best match ~94.3% (gcc280_g0); residual is a callee-saved register tie
 *       (obj base vs packed pos land in s0/s1 swapped). The dead Vec2s reproduces
 *       the original -0x30 frame reservation (FRAME-01).
 */
void func_80074B64(void)
{
/* Partial WMAP decompilation: 94.260000% (gcc280_g0). */

extern u8 D_800D95AC[];
extern u8 D_80139A20[];
extern s32 D_8011CF4C;
extern s32 D_801B25E4;
extern s32 D_801B25B8;
extern s32 D_801B25BC;

    Vec2s scratch;
    s32 pos;

    pos = ((*(u16*)&D_8011CF4C - 0x18) & 0xFFFF) | ((*(u16*)((u8*)&D_8011CF4C + 0x2) - 0xA) << 16);
    func_8006CC4C(D_800D95AC, D_80139A20);
    func_80066F9C(D_800D95AC, pos, 0x4, 0xB, 0);
    *(s16*)(&D_800D95AC[0x24]) = *(u16*)&D_801B25E4;
    *(s16*)(&D_800D95AC[0x22]) = *(u16*)&D_801B25E4;
    D_801B25E4 -= 0x8;
    if (D_801B25E4 < 0)
    {
        D_801B25E4 = 0;
    }
    if (--D_801B25BC == 0)
    {
        D_801B25B8 += 1;
    }
}

/** @brief Draw the rotating effect, raise its intensity, and advance its countdown. */
void func_80074C2C(void)
{
/* Partial WMAP decompilation: 99.365080% (gcc280_g0). */

extern u8 D_800DCF18[];
extern VECTOR D_80139888;
extern SVECTOR D_801B24A0;
extern s32 D_801B25D0;
extern s32 D_801B25D4;
extern s32 D_801B25D8;

    MATRIX matrix;
    s32 intensity;
    s32 remaining;

    PushMatrix();
    RotMatrix(&D_801B24A0, &matrix);
    TransMatrix(&matrix, &D_80139888);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    func_8006CD98(D_800DCF18, 0, 36, 183, 0x7A40, 1, -1);
    PopMatrix();
    intensity = D_801B25D8 + 15;
    D_801B25D8 = intensity;
    if (intensity >= 129)
    {
        D_801B25D8 = 128;
    }
    D_801B24A0.vz = (u16)(D_801B24A0.vz + 40);
    remaining = D_801B25D4 - 1;
    D_801B25D4 = remaining;
    D_80139888.vz -= 7000;
    if (remaining == 0)
    {
        D_801B25D0++;
    }
}

/** @brief Draw and fade the rotating effect, then advance its countdown. */
void func_80074D28(void)
{
extern u8 D_800DCF18[];
extern VECTOR D_80139888;
extern SVECTOR D_801B24A0;
extern s32 D_801B25D0;
extern s32 D_801B25D4;
extern s32 D_801B25D8;

    MATRIX matrix;
    s32 intensity;
    s32 remaining;

    if (D_801B25D8 != 0)
    {
        PushMatrix();
        RotMatrix(&D_801B24A0, &matrix);
        TransMatrix(&matrix, &D_80139888);
        SetRotMatrix(&matrix);
        SetTransMatrix(&matrix);
        func_8006CD98(D_800DCF18, 0, 36, 183, 0x7A40, 1, D_801B25D8);
        intensity = D_801B25D8 - 4;
        D_801B25D8 = intensity;
        if (intensity < 0)
        {
            D_801B25D8 = 0;
        }
        PopMatrix();
        D_801B24A0.vz = (u16)(D_801B24A0.vz + 40);
    }
    remaining = D_801B25D4 - 1;
    D_801B25D4 = remaining;
    if (remaining == 0)
    {
        D_801B25D0++;
    }
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80074E20(s32 arg0)
{
extern u32 D_801B2580;
extern s32 D_801B2584;
extern void (*D_800D5050[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2580 = 1;
        D_801B2584 = 1;
    }

    if (D_801B2580 < 0x4)
    {
        D_800D5050[D_801B2580]();
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
void func_80074E90(void)
{
extern u32 D_801B2580;
extern s32 D_801B2584;
extern void (*D_800D5050[])(void);

    D_801B2580 = 1;
    D_801B2584 = 1;
}

/** @brief World-map step handler: init a UI descriptor block, set the timer, advance the step. */
void func_80074EA8(void)
{
extern void *D_801399B4;
extern s32 D_8011D538;
extern u8 D_800D9268;
extern s32 D_801B2580;
extern s32 D_801B2584;

    u8 *base = &D_800D9268;

    D_801399B4 = &D_8011D538;
    *(u8 *)(base + 0xE2) = 0xF;
    *(s16 *)(base + 0xEA) = 1;
    *(s16 *)(base + 0xEC) = -1;
    *(s16 *)(base + 0xDE) = 0;
    *(s16 *)(base + 0xFE) = 0x80;
    *(s16 *)(base + 0x100) = 0x80;
    D_801B2584 = 0x36;
    D_801B2580 += 1;
    func_80074F20();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80074F20(void)
{
extern s32 D_801B2580;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;
extern s32 D_801B2584;

    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0xF, 0xB, 0);
    if (--D_801B2584 == 0)
    {
        D_801B2580 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80074F9C(void)
{
extern s32 D_801B2580;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;
extern s32 D_801B2584;

    D_801B2580 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80074FB4(s32 arg0)
{
extern u32 D_801B2568;
extern s32 D_801B256C;
extern void (*D_800D4FC8[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2568 = 1;
        D_801B256C = 1;
        return 1;
    }

    if (D_801B2568 < 0x6)
    {
        D_800D4FC8[D_801B2568]();
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
void func_8007502C(void)
{
extern u32 D_801B2568;
extern s32 D_801B256C;
extern void (*D_800D4FC8[])(void);

    D_801B2568 = 1;
    D_801B256C = 1;
}

/**
 * @brief Register a world-map callback and advance to the next step.
 */
void func_80075044(void)
{
extern s32 D_8011CF50;
extern s32 D_8013B20C;
extern s32 D_801B2568;

    D_8011CF50 = 1;
    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2568 += 1;
    func_80075094();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80075094(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2568;

    if (D_8013B20C == 0)
    {
        D_801B2568 += 1;
        func_800750D0();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_800750D0(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2568;

    func_8006CAC0(func_80075168);
    D_8013B20C = 1;
    D_801B2568 += 1;
    func_80075114();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80075114(void)
{
extern s32 D_801B2568;
extern s32 D_8013B20C;

    if (D_8013B20C == 0)
    {
        D_801B2568 += 1;
        func_80075150();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80075150(void)
{
extern s32 D_801B2568;
extern s32 D_8013B20C;

    D_801B2568 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80075168(s32 arg0)
{
extern u32 D_801B2570;
extern s32 D_801B2574;
extern void (*D_800D4FE0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2570 = 1;
        D_801B2574 = 1;
        return 1;
    }

    if (D_801B2570 < 0x16)
    {
        D_800D4FE0[D_801B2570]();
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
void func_800751E0(void)
{
extern u32 D_801B2570;
extern s32 D_801B2574;
extern void (*D_800D4FE0[])(void);

    D_801B2570 = 1;
    D_801B2574 = 1;
}

/** @brief Enable the world-map flag, set the drawing color, and start a two-tick delay. */
void func_800751F8(void)
{
extern s32 D_8013B208;
extern s32 D_801B2570;
extern s32 D_801B2574;

    D_8013B208 = 1;
    func_8006683C(0x404040);
    D_801B2574 = 2;
    D_801B2570 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80075240(void)
{
extern s32 D_801B2574;
extern s32 D_801B2570;

    if (--D_801B2574 == 0)
    {
        D_801B2570 += 1;
    }
}

/** @brief World-map step handler: register a callback and advance the step counter. */
void func_80075274(void)
{
extern s32 D_801B2570;
extern s32 D_801B2574;

    func_800652A8(0x10, 0x80);
    func_8006CAC0(func_80075704);
    D_801B2574 = 8;
    D_801B2570 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800752BC(void)
{
extern s32 D_801B2574;
extern s32 D_801B2570;

    if (--D_801B2574 == 0)
    {
        D_801B2570 += 1;
    }
}

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void func_800752F0(void)
{
extern s32 D_801ADAE0;
extern s32 D_801B2574;
extern s32 D_801B2570;

    D_801ADAE0 = 1;
    func_8006CAC0(func_80074E20);
    D_801B2574 = 0x24;
    D_801B2570 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80075338(void)
{
extern s32 D_801B2574;
extern s32 D_801B2570;

    if (--D_801B2574 == 0)
    {
        D_801B2570 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8007536C(void)
{
extern s32 D_801B2574;
extern s32 D_801B2570;

    func_8006CAC0(func_80075B34);
    func_8006CAC0(func_800759B4);
    D_801B2574 = 0x20;
    D_801B2570 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800753B4(void)
{
extern s32 D_801B2574;
extern s32 D_801B2570;

    if (--D_801B2574 == 0)
    {
        D_801B2570 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800753E8(void)
{
extern s32 D_801B2574;
extern s32 D_801B2570;

    func_8006CAC0(func_80076AE8);
    D_801B2574 = 0xE;
    D_801B2570 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80075424(void)
{
extern s32 D_801B2574;
extern s32 D_801B2570;

    if (--D_801B2574 == 0)
    {
        D_801B2570 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80075458(void)
{
extern s32 D_801B2574;
extern s32 D_801B2570;

    func_8006CAC0(func_800765A4);
    D_801B2574 = 0x4;
    D_801B2570 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80075494(void)
{
extern s32 D_801B2574;
extern s32 D_801B2570;

    if (--D_801B2574 == 0)
    {
        D_801B2570 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800754C8(void)
{
extern s32 D_801B2574;
extern s32 D_801B2570;

    func_8006CAC0(func_80076848);
    D_801B2574 = 0x26;
    D_801B2570 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80075504(void)
{
extern s32 D_801B2574;
extern s32 D_801B2570;

    if (--D_801B2574 == 0)
    {
        D_801B2570 += 1;
    }
}

/** @brief World-map step handler: register a callback and advance the step. */
void func_80075538(void)
{
extern s32 D_801B2570;
extern s32 D_801B2574;

    func_8006CAC0(func_80075DD8);
    D_801B2574 = 1;
    D_801B2570 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80075574(void)
{
extern s32 D_801B2574;
extern s32 D_801B2570;

    if (--D_801B2574 == 0)
    {
        D_801B2570 += 1;
    }
}

/** @brief Register two sequence callbacks and advance to a one-tick delay. */
void func_800755A8(void)
{
extern s32 D_801B2570;
extern s32 D_801B2574;

    func_8006CAC0(&func_80076074);
    func_8006CAC0(&func_800761C0);
    D_801B2574 = 1;
    D_801B2570 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800755F0(void)
{
extern s32 D_801B2574;
extern s32 D_801B2570;

    if (--D_801B2574 == 0)
    {
        D_801B2570 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_80075624(void)
{
extern s32 D_801B2574;
extern s32 D_801B2570;

    func_8006CAC0(func_8007630C);
    func_8006CAC0(func_80076458);
    D_801B2574 = 0x3B;
    D_801B2570 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8007566C(void)
{
extern s32 D_801B2574;
extern s32 D_801B2570;

    if (--D_801B2574 == 0)
    {
        D_801B2570 += 1;
    }
}

void func_800756A0(void)
{
/* Partial WMAP decompilation: 95.600000% (gcc280_g0). */

extern s32 D_8013B20C;
extern u32 D_80139290[];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2570;

    D_8013B20C = 0;
    D_80139290[D_8011D530 * 10 + D_8011D510 * 60] = D_8011D4FC | 0x100;
    D_801B2570 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80075704(s32 arg0)
{
extern u32 D_801B2578;
extern s32 D_801B257C;
extern void (*D_800D5038[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2578 = 1;
        D_801B257C = 1;
    }

    if (D_801B2578 < 0x6)
    {
        D_800D5038[D_801B2578]();
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
void func_80075774(void)
{
extern u32 D_801B2578;
extern s32 D_801B257C;
extern void (*D_800D5038[])(void);

    D_801B2578 = 1;
    D_801B257C = 1;
}

/** @brief World-map step handler: prime a sub-object and advance the step counter. */
void func_8007578C(void)
{
extern void *D_801399AC;
extern s32 D_8011D538;
extern u8 D_800D9268[];
extern s32 D_801B24B4;
extern s32 D_801B2578;
extern s32 D_801B257C;

    u8 *base;

    D_801399AC = &D_8011D538;
    base = D_800D9268;
    *(u8 *)(base + 0xB6) = 0xF;
    *(s16 *)(base + 0xB2) = 0;
    *(s16 *)(base + 0xBE) = 0;
    *(s16 *)(base + 0xC0) = -1;
    D_801B24B4 = 0;
    D_801B257C = 8;
    D_801B2578 += 1;
    func_800757FC();
}

/** @brief Draw the actor, increase its scale, and advance when its timer expires. */
void func_800757FC(void)
{
/* Partial WMAP decompilation: 92.652176% (gcc280_g0). */

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

extern WmapConfigA D_800D9318;
extern u8 *D_801399A8;
extern s32 D_8011CF4C;
extern s32 D_801B24B4;
extern s32 D_801B2578;
extern s32 D_801B257C;


    D_800D9318.field_24 = *(u16 *)&D_801B24B4;
    D_800D9318.field_22 = *(u16 *)&D_801B24B4;
    func_8006CC4C(&D_800D9318, &D_801399A8);
    func_80066F9C(&D_800D9318, D_8011CF4C, 15, 4, 0);
    D_801B24B4 += 8;
    if (D_801B24B4 >= 0x82)
    {
        D_801B24B4 = 0x81;
    }
    if (--D_801B257C == 0)
    {
        D_801B2578++;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800758B4(void)
{
extern s32 D_801B257C;
extern s32 D_801B2578;

    D_801B257C = 0x20;
    D_801B2578 += 1;
    func_800758EC();
}

/** @brief Draw the actor, reduce its scale, and advance when its timer expires. */
void func_800758EC(void)
{
/* Partial WMAP decompilation: 92.318184% (gcc280_g0). */

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

extern WmapConfigA D_800D9318;
extern u8 *D_801399A8;
extern s32 D_8011CF4C;
extern s32 D_801B24B4;
extern s32 D_801B2578;
extern s32 D_801B257C;


    D_800D9318.field_24 = *(u16 *)&D_801B24B4;
    D_800D9318.field_22 = *(u16 *)&D_801B24B4;
    func_8006CC4C(&D_800D9318, &D_801399A8);
    func_80066F9C(&D_800D9318, D_8011CF4C, 15, 4, 0);
    D_801B24B4 -= 2;
    if (D_801B24B4 < 0)
    {
        D_801B24B4 = 0;
    }
    if (--D_801B257C == 0)
    {
        D_801B2578++;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007599C(void)
{
extern s32 D_801B2578;

    D_801B2578 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_800759B4(s32 arg0)
{
extern u32 D_801B2588;
extern s32 D_801B258C;
extern void (*D_800D5060[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2588 = 1;
        D_801B258C = 1;
    }

    if (D_801B2588 < 0x4)
    {
        D_800D5060[D_801B2588]();
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
void func_80075A24(void)
{
extern u32 D_801B2588;
extern s32 D_801B258C;
extern void (*D_800D5060[])(void);

    D_801B2588 = 1;
    D_801B258C = 1;
}

/** @brief World-map step: reset a run of slot tables then advance the sub-counter. */
void func_80075A3C(void)
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

extern s32 D_801B0FD0;
extern WmapSlot8 D_80139988[];
extern s32 D_8011D538;
extern WmapSlot14 D_801AFBD0[];
extern s32 D_801B258C;
extern s32 D_801B2588;

    s32 i;

    D_801B0FD0 = 0x18;
    for (i = 0; i < 0x18; i++)
    {
        D_801AFBD0[i].field_00 = 0;
        D_80139988[i + 0xCC].field_04 = &D_8011D538;
    }
    D_801B258C = 0x30;
    D_801B2588 += 1;
    func_80075AC0();
}

/** @brief World-map step handler: run the sub-step, then advance after the timer. */
void func_80075AC0(void)
{
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_801B2588;
extern s32 D_801B258C;

    func_80074368(D_800DB578, D_80139FE8, 0x18);
    if (--D_801B258C == 0)
    {
        D_801B2588 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80075B1C(void)
{
extern s32 D_801B2588;

    D_801B2588 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80075B34(s32 arg0)
{
extern u32 D_801B2590;
extern s32 D_801B2594;
extern void (*D_800D5070[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2590 = 1;
        D_801B2594 = 1;
    }

    if (D_801B2590 < 0x6)
    {
        D_800D5070[D_801B2590]();
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
void func_80075BA4(void)
{
extern u32 D_801B2590;
extern s32 D_801B2594;
extern void (*D_800D5070[])(void);

    D_801B2590 = 1;
    D_801B2594 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80075BBC(void)
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

extern u8* D_801399BC;
extern u8 D_8011D538[];
extern WmapConfigA D_800D9268[];
extern s32 D_80182DE4;
extern s32 D_801B2590;
extern s32 D_801B2594;

    D_801399BC = D_8011D538;
    D_800D9268[6].field_06 = 0xF;
    D_800D9268[6].field_0E = 2;
    D_800D9268[6].field_10 = -1;
    D_80182DE4 = 0;
    D_800D9268[6].field_02 = 0;
    D_801B2594 = 0x28;
    D_801B2590 += 1;
    func_80075C30();
}

/**
 * @brief Draw a scrolling world-map element and advance its slide/hold state.
 */
void func_80075C30(void)
{
extern u8 D_800D9370[];
extern u8 D_80182DE4[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B2594;
extern s32 D_801B2590;

    u8* obj = D_800D9370;
    u16 pos = *(u16*)&D_80182DE4[0];

    *(s16*)&obj[0x24] = pos;
    *(s16*)&obj[0x22] = pos;
    func_8006CC4C(obj, D_801399B8);
    func_80066F9C(obj, D_8011CF4C, 0xF, 0xB, 0);
    *(s32*)&D_80182DE4[0] += 4;
    if (*(s32*)&D_80182DE4[0] >= 0x82)
    {
        *(s32*)&D_80182DE4[0] = 0x81;
    }
    if (--D_801B2594 == 0)
    {
        D_801B2590 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80075CE4(void)
{
extern s32 D_801B2594;
extern s32 D_801B2590;

    D_801B2594 = 0x20;
    D_801B2590 += 1;
    func_80075D1C();
}

/** @brief Draw the actor, reduce its scale, and advance when its timer expires. */
void func_80075D1C(void)
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

extern WmapConfigA D_800D9370;
extern u8 *D_801399B8;
extern s32 D_8011CF4C;
extern s32 D_80182DE4;
extern s32 D_801B2590;
extern s32 D_801B2594;

    WmapConfigA *actors;

    func_8006CC4C(&D_800D9370, &D_801399B8);
    func_80066F9C(&D_800D9370, D_8011CF4C, 15, 11, 0);
    actors = &D_800D9370 - 6;
    actors[6].field_24 = *(u16 *)&D_80182DE4;
    actors[6].field_22 = *(u16 *)&D_80182DE4;
    D_80182DE4 -= 8;
    if (D_80182DE4 < 0)
    {
        D_80182DE4 = 0;
    }
    if (--D_801B2594 == 0)
    {
        D_801B2590++;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80075DC0(void)
{
extern s32 D_801B2590;

    D_801B2590 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80075DD8(s32 arg0)
{
extern u32 D_801B2598;
extern s32 D_801B259C;
extern void (*D_800D5088[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2598 = 1;
        D_801B259C = 1;
    }

    if (D_801B2598 < 0x6)
    {
        D_800D5088[D_801B2598]();
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
void func_80075E48(void)
{
extern u32 D_801B2598;
extern s32 D_801B259C;
extern void (*D_800D5088[])(void);

    D_801B2598 = 1;
    D_801B259C = 1;
}

/** @brief World-map step handler: prime a sub-object and advance the step counter. */
void func_80075E60(void)
{
extern void *D_801399C4;
extern void *D_8011F538;
extern u8 D_800D939C[];
extern s32 D_80182DE8;
extern s32 D_801B2598;
extern s32 D_801B259C;

    u8 *base;

    D_801399C4 = &D_8011F538;
    base = D_800D939C;
    *(u8 *)(base + 0x6) = 0xF;
    *(s16 *)(base + 0x2) = 0;
    *(s16 *)(base + 0xE) = 0;
    *(s16 *)(base + 0x10) = -1;
    D_80182DE8 = 0;
    D_801B259C = 0xC;
    D_801B2598 += 1;
    func_80075ED0();
}

/**
 * @brief Draw a scrolling world-map element and advance its slide/hold state.
 */
void func_80075ED0(void)
{
extern u8 D_800D939C[];
extern u8 D_80182DE8[];
extern u8 D_801399C0[];
extern s32 D_8011CF4C;
extern s32 D_801B259C;
extern s32 D_801B2598;

    u8* obj = D_800D939C;
    u16 pos = *(u16*)&D_80182DE8[0];

    *(s16*)&obj[0x24] = pos;
    *(s16*)&obj[0x22] = pos;
    func_8006CC4C(obj, D_801399C0);
    func_80066F9C(obj, D_8011CF4C, 0x4, 0xB, 0);
    *(s32*)&D_80182DE8[0] += 0x18;
    if (*(s32*)&D_80182DE8[0] >= 0x82)
    {
        *(s32*)&D_80182DE8[0] = 0x81;
    }
    if (--D_801B259C == 0)
    {
        D_801B2598 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80075F84(void)
{
extern s32 D_801B259C;
extern s32 D_801B2598;

    D_801B259C = 0x24;
    D_801B2598 += 1;
    func_80075FBC();
}

/** @brief World-map step: build a sprite, decrement a shared budget, expire the timer. */
void func_80075FBC(void)
{
extern u8 D_800D939C[];
extern s32 D_801399C0;
extern s32 D_8011CF4C;
extern s32 D_80182DE8;
extern s32 D_801B2598;
extern s32 D_801B259C;

    func_8006CC4C(D_800D939C, &D_801399C0);
    func_80066F9C(D_800D939C, D_8011CF4C, 4, 0xB, 0);
    *(s16 *)(D_800D939C + 0x24) = (u16)D_80182DE8;
    *(s16 *)(D_800D939C + 0x22) = (u16)D_80182DE8;
    D_80182DE8 -= 4;
    if (D_80182DE8 < 0)
    {
        D_80182DE8 = 0;
    }
    if (--D_801B259C == 0)
    {
        D_801B2598 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007605C(void)
{
extern s32 D_801B2598;

    D_801B2598 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80076074(s32 arg0)
{
extern u32 D_801B25A0;
extern s32 D_801B25A4;
extern void (*D_800D50A0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B25A0 = 1;
        D_801B25A4 = 1;
    }

    if (D_801B25A0 < 0x6)
    {
        D_800D50A0[D_801B25A0]();
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
void func_800760E4(void)
{
extern u32 D_801B25A0;
extern s32 D_801B25A4;
extern void (*D_800D50A0[])(void);

    D_801B25A0 = 1;
    D_801B25A4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800760FC(void)
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

extern u8* D_80139A0C;
extern u8 D_8011F538[];
extern WmapConfigA D_800D9528;
extern s32 D_80182DEC;
extern s32 D_801B25A0;
extern s32 D_801B25A4;

    D_80139A0C = D_8011F538;
    D_800D9528.field_06 = 0xF;
    D_800D9528.field_0E = 1;
    D_800D9528.field_10 = -1;
    D_80182DEC = 0;
    D_800D9528.field_02 = 0;
    D_801B25A4 = 0x18;
    D_801B25A0 += 1;
    func_800745A4();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80076170(void)
{
extern s32 D_801B25A0;
extern s32 D_801B25A4;

    D_801B25A4 = 0x28;
    D_801B25A0 += 1;
    func_80074680();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800761A8(void)
{
extern s32 D_801B25A0;
extern s32 D_801B25A4;

    D_801B25A0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_800761C0(s32 arg0)
{
extern u32 D_801B25A8;
extern s32 D_801B25AC;
extern void (*D_800D50B8[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B25A8 = 1;
        D_801B25AC = 1;
    }

    if (D_801B25A8 < 0x6)
    {
        D_800D50B8[D_801B25A8]();
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
void func_80076230(void)
{
extern u32 D_801B25A8;
extern s32 D_801B25AC;
extern void (*D_800D50B8[])(void);

    D_801B25A8 = 1;
    D_801B25AC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80076248(void)
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

extern u8* D_80139A14;
extern u8 D_8011F538[];
extern WmapConfigA D_800D9554;
extern s32 D_801B25DC;
extern s32 D_801B25A8;
extern s32 D_801B25AC;

    D_80139A14 = D_8011F538;
    D_800D9554.field_06 = 0xF;
    D_800D9554.field_0E = 1;
    D_800D9554.field_10 = -1;
    D_801B25DC = 0;
    D_800D9554.field_02 = 0;
    D_801B25AC = 0x18;
    D_801B25A8 += 1;
    func_80074748();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800762BC(void)
{
extern s32 D_801B25A8;
extern s32 D_801B25AC;

    D_801B25AC = 0x20;
    D_801B25A8 += 1;
    func_80074820();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800762F4(void)
{
extern s32 D_801B25A8;
extern s32 D_801B25AC;

    D_801B25A8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_8007630C(s32 arg0)
{
extern u32 D_801B25B0;
extern s32 D_801B25B4;
extern void (*D_800D50D0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B25B0 = 1;
        D_801B25B4 = 1;
    }

    if (D_801B25B0 < 0x6)
    {
        D_800D50D0[D_801B25B0]();
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
void func_8007637C(void)
{
extern u32 D_801B25B0;
extern s32 D_801B25B4;
extern void (*D_800D50D0[])(void);

    D_801B25B0 = 1;
    D_801B25B4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80076394(void)
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

extern u8* D_80139A1C;
extern u8 D_8011F538[];
extern WmapConfigA D_800D9580;
extern s32 D_801B25E0;
extern s32 D_801B25B0;
extern s32 D_801B25B4;

    D_80139A1C = D_8011F538;
    D_800D9580.field_06 = 0xF;
    D_800D9580.field_0E = 1;
    D_800D9580.field_10 = -1;
    D_801B25E0 = 0;
    D_800D9580.field_02 = 0;
    D_801B25B4 = 0x20;
    D_801B25B0 += 1;
    func_800748E4();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80076408(void)
{
extern s32 D_801B25B0;
extern s32 D_801B25B4;

    D_801B25B4 = 0x20;
    D_801B25B0 += 1;
    func_800749C0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80076440(void)
{
extern s32 D_801B25B0;
extern s32 D_801B25B4;

    D_801B25B0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80076458(s32 arg0)
{
extern u32 D_801B25B8;
extern s32 D_801B25BC;
extern void (*D_800D50E8[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B25B8 = 1;
        D_801B25BC = 1;
    }

    if (D_801B25B8 < 0x6)
    {
        D_800D50E8[D_801B25B8]();
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
void func_800764C8(void)
{
extern u32 D_801B25B8;
extern s32 D_801B25BC;
extern void (*D_800D50E8[])(void);

    D_801B25B8 = 1;
    D_801B25BC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800764E0(void)
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

extern u8* D_80139A24;
extern u8 D_8011F538[];
extern WmapConfigA D_800D95AC;
extern s32 D_801B25E4;
extern s32 D_801B25B8;
extern s32 D_801B25BC;

    D_80139A24 = D_8011F538;
    D_800D95AC.field_06 = 0xF;
    D_800D95AC.field_0E = 1;
    D_800D95AC.field_10 = -1;
    D_801B25E4 = 0;
    D_800D95AC.field_02 = 0;
    D_801B25BC = 0xC;
    D_801B25B8 += 1;
    func_80074A88();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80076554(void)
{
extern s32 D_801B25B8;
extern s32 D_801B25BC;

    D_801B25BC = 0x28;
    D_801B25B8 += 1;
    func_80074B64();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007658C(void)
{
extern s32 D_801B25B8;
extern s32 D_801B25BC;

    D_801B25B8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800765A4(s32 arg0)
{
extern u32 D_801B25C0;
extern s32 D_801B25C4;
extern void (*D_800D5100[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B25C0 = 1;
        D_801B25C4 = 1;
        return 1;
    }

    if (D_801B25C0 < 0x6)
    {
        D_800D5100[D_801B25C0]();
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
void func_8007661C(void)
{
extern u32 D_801B25C0;
extern s32 D_801B25C4;
extern void (*D_800D5100[])(void);

    D_801B25C0 = 1;
    D_801B25C4 = 1;
}

/** @brief World-map step handler: prime a sub-object and advance the step counter. */
void func_80076634(void)
{
extern void *D_801399D4;
extern void *D_80121538;
extern u8 D_800D93F4[];
extern s32 D_80182DF0;
extern s32 D_801B25C0;
extern s32 D_801B25C4;

    u8 *base;

    D_801399D4 = &D_80121538;
    base = D_800D93F4;
    *(u8 *)(base + 0x6) = 0xF;
    *(s16 *)(base + 0x2) = 0;
    *(s16 *)(base + 0xE) = 0;
    *(s16 *)(base + 0x10) = -1;
    D_80182DF0 = 0;
    D_801B25C4 = 0x10;
    D_801B25C0 += 1;
    func_800766A4();
}

/**
 * @brief Draw a scrolling world-map element and advance its slide/hold state.
 */
void func_800766A4(void)
{
extern u8 D_800D93F4[];
extern u8 D_80182DF0[];
extern u8 D_801399D0[];
extern s32 D_8011CF4C;
extern s32 D_801B25C4;
extern s32 D_801B25C0;

    u8* obj = D_800D93F4;
    u16 pos = *(u16*)&D_80182DF0[0];

    *(s16*)&obj[0x24] = pos;
    *(s16*)&obj[0x22] = pos;
    func_8006CC4C(obj, D_801399D0);
    func_80066F9C(obj, D_8011CF4C, 0x10, 0xB, 0);
    *(s32*)&D_80182DF0[0] += 0x20;
    if (*(s32*)&D_80182DF0[0] >= 0x82)
    {
        *(s32*)&D_80182DF0[0] = 0x81;
    }
    if (--D_801B25C4 == 0)
    {
        D_801B25C0 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80076758(void)
{
extern s32 D_801B25C4;
extern s32 D_801B25C0;

    D_801B25C4 = 0x10;
    D_801B25C0 += 1;
    func_80076790();
}

/** @brief World-map step: build a sprite, decrement a shared budget, expire the timer. */
void func_80076790(void)
{
extern u8 D_800D93F4[];
extern s32 D_801399D0;
extern s32 D_8011CF4C;
extern s32 D_80182DF0;
extern s32 D_801B25C0;
extern s32 D_801B25C4;

    func_8006CC4C(D_800D93F4, &D_801399D0);
    func_80066F9C(D_800D93F4, D_8011CF4C, 0x10, 0xB, 0);
    *(s16 *)(D_800D93F4 + 0x24) = (u16)D_80182DF0;
    *(s16 *)(D_800D93F4 + 0x22) = (u16)D_80182DF0;
    D_80182DF0 -= 0x10;
    if (D_80182DF0 < 0)
    {
        D_80182DF0 = 0;
    }
    if (--D_801B25C4 == 0)
    {
        D_801B25C0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80076830(void)
{
extern s32 D_801B25C0;

    D_801B25C0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80076848(s32 arg0)
{
extern u32 D_801B25C8;
extern s32 D_801B25CC;
extern void (*D_800D5118[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B25C8 = 1;
        D_801B25CC = 1;
        return 1;
    }

    if (D_801B25C8 < 0x6)
    {
        D_800D5118[D_801B25C8]();
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
void func_800768C0(void)
{
extern u32 D_801B25C8;
extern s32 D_801B25CC;
extern void (*D_800D5118[])(void);

    D_801B25C8 = 1;
    D_801B25CC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800768D8(void)
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

extern u8* D_801399DC;
extern u8 D_80121538[];
extern WmapConfigA D_800D9420;
extern s32 D_80182DF4;
extern s32 D_801B25C8;
extern s32 D_801B25CC;

    D_801399DC = D_80121538;
    D_800D9420.field_06 = 0xF;
    D_800D9420.field_0E = 1;
    D_800D9420.field_10 = -1;
    D_80182DF4 = 0;
    D_800D9420.field_02 = 0;
    D_801B25CC = 0x59;
    D_801B25C8 += 1;
    func_8007694C();
}

/**
 * @brief Draw a scrolling world-map element and advance its slide/hold state.
 */
void func_8007694C(void)
{
extern u8 D_800D9420[];
extern u8 D_80182DF4[];
extern u8 D_801399D8[];
extern s32 D_8011CF4C;
extern s32 D_801B25CC;
extern s32 D_801B25C8;

    u8* obj = D_800D9420;
    u16 pos = *(u16*)&D_80182DF4[0];

    *(s16*)&obj[0x24] = pos;
    *(s16*)&obj[0x22] = pos;
    func_8006CC4C(obj, D_801399D8);
    func_80066F9C(obj, D_8011CF4C, 0x10, 0xB, 0);
    *(s32*)&D_80182DF4[0] += 0x8;
    if (*(s32*)&D_80182DF4[0] >= 0x81)
    {
        *(s32*)&D_80182DF4[0] = 0x80;
    }
    if (--D_801B25CC == 0)
    {
        D_801B25C8 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80076A00(void)
{
extern s32 D_801B25CC;
extern s32 D_801B25C8;

    D_801B25CC = 0x10;
    D_801B25C8 += 1;
    func_80076A38();
}

/** @brief Draw the actor, update two effect fields, and advance when the countdown expires. */
void func_80076A38(void)
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

extern WmapConfigA D_800D9420;
extern s32 D_8011CF4C;
extern u8 D_801399D8[];
extern s32 D_80182DF4;
extern s32 D_801B25C8;
extern s32 D_801B25CC;

    s32 remaining_ticks;

    func_8006CC4C(&D_800D9420, &D_801399D8);
    func_80066F9C(&D_800D9420, D_8011CF4C, 0x10, 0xB, 0);
    D_800D9420.field_24 = (u16) D_80182DF4;
    D_800D9420.field_22 = (u16) D_80182DF4;
    if ((s32) D_80182DF4 < 0)
    {
        D_80182DF4 = 0;
    }
    remaining_ticks = D_801B25CC - 1;
    D_801B25CC = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B25C8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80076AD0(void)
{
extern s32 D_801B25C8;

    D_801B25C8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80076AE8(s32 arg0)
{
extern u32 D_801B25D0;
extern s32 D_801B25D4;
extern void (*D_800D5130[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B25D0 = 1;
        D_801B25D4 = 1;
    }

    if (D_801B25D0 < 0x6)
    {
        D_800D5130[D_801B25D0]();
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
void func_80076B58(void)
{
extern u32 D_801B25D0;
extern s32 D_801B25D4;
extern void (*D_800D5130[])(void);

    D_801B25D0 = 1;
    D_801B25D4 = 1;
}

/** @brief Restore the default transform and advance the sequence. */
void func_80076B70(void)
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
extern WmapOrientation D_801B24A0;
extern WmapTransform D_80182DC0;
extern WmapTransform D_80139888;
extern s32 D_801B25D8;
extern s32 D_801B25D0;
extern s32 D_801B25D4;

    D_801B25D8 = 0;
    D_801B24A0 = D_80139258;
    D_80139888 = D_80182DC0;
    D_80139888.words[2] = 0xC350;
    D_801B25D4 = 10;
    D_801B25D0++;
    func_80074C2C();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80076C1C(void)
{
extern s32 D_801B25D0;
extern s32 D_801B25D4;

    D_801B25D4 = 0x60;
    D_801B25D0 += 1;
    func_80074D28();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80076C54(void)
{
extern s32 D_801B25D0;
extern s32 D_801B25D4;

    D_801B25D0 += 1;
}
