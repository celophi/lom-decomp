#include "wmap_land_effect_19.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_800A3348(void)
{
extern s32 D_801B2DC8;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2DF4;
extern s32 D_801B2DF0;

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
    if (--D_801B2DF4 == 0)
    {
        D_801B2DF0 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_800A3448(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B2DFC;
extern s32 D_801B2DF8;

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
        D_80182DEC -= 1;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    if (--D_801B2DFC == 0)
    {
        D_801B2DF8 += 1;
    }
}

/**
 * @brief World-map step handler: draw the animated actor, ramp its size up to a
 *        cap, scroll the sprite field, then advance when the frame counter expires.
 */
void func_800A3548(void)
{
extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern u8 D_80182DC0[];
extern u8 D_8013B238[];
extern s32* D_8011CF24;
extern s32 D_80182DF0;
extern s32 D_801B2E04;
extern s32 D_801B2E00;

    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(D_80182DC0, D_8013B238);
    func_800675F0(D_8011CF24, 0, 0xA, 0x36, 0x7880, 0x1001, D_80182DF0, 0, 0xA, -1);
    value = D_80182DF0 + 4;
    D_80182DF0 = value;
    if (value >= 0x82)
    {
        D_80182DF0 = 0x81;
    }
    timer = D_801B2E04;
    ((u16*)D_8013B238)[2] += 0x16;
    next_timer = timer - 1;
    D_801B2E04 = next_timer;
    if (next_timer == 0)
    {
        D_801B2E00++;
    }
}

/**
 * @brief World-map step handler: render the actor, ramp its size down to a floor,
 *        scroll the shadow field, then advance when the frame counter expires.
 */
void func_800A3624(void)
{
extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern u8 D_80182DC0[];
extern u8 D_8013B238[];
extern s32* D_8011CF24;
extern s32 D_80182DF0;
extern s32 D_801B2E04;
extern s32 D_801B2E00;

    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(D_80182DC0, D_8013B238);
    func_800675F0(D_8011CF24, 0, 0xA, 0x36, 0x7880, 0x1001, D_80182DF0, 0, 0xA, -1);
    value = D_80182DF0 - 2;
    D_80182DF0 = value;
    if (value < 0)
    {
        D_80182DF0 = 0;
    }
    timer = D_801B2E04;
    ((u16*)D_8013B238)[2] += 0x16;
    next_timer = timer - 1;
    D_801B2E04 = next_timer;
    if (next_timer == 0)
    {
        D_801B2E00++;
    }
}

/**
 * @brief World-map step handler: draw the animated actor, ramp its size up to a
 *        cap, scroll the sprite field, then advance when the frame counter expires.
 */
void func_800A36F4(void)
{
extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern u8 D_80182DC0[];
extern u8 D_8013B240[];
extern s32* D_8011CF24;
extern s32 D_80182DF4;
extern s32 D_801B2E0C;
extern s32 D_801B2E08;

    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(D_80182DC0, D_8013B240);
    func_800675F0(D_8011CF24, 0, 0xA, 0x36, 0x78C0, 0x1001, D_80182DF4, 0, 0xA, -1);
    value = D_80182DF4 + 8;
    D_80182DF4 = value;
    if (value >= 0x82)
    {
        D_80182DF4 = 0x81;
    }
    timer = D_801B2E0C;
    ((u16*)D_8013B240)[2] += 0x16;
    next_timer = timer - 1;
    D_801B2E0C = next_timer;
    if (next_timer == 0)
    {
        D_801B2E08++;
    }
}

/**
 * @brief World-map step handler: render the actor, ramp its size down to a floor,
 *        scroll the shadow field, then advance when the frame counter expires.
 */
void func_800A37D0(void)
{
extern void func_800675F0(s32* a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9);
extern u8 D_80182DC0[];
extern u8 D_8013B240[];
extern s32* D_8011CF24;
extern s32 D_80182DF4;
extern s32 D_801B2E0C;
extern s32 D_801B2E08;

    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(D_80182DC0, D_8013B240);
    func_800675F0(D_8011CF24, 0, 0xA, 0x36, 0x78C0, 0x1001, D_80182DF4, 0, 0xA, -1);
    value = D_80182DF4 - 2;
    D_80182DF4 = value;
    if (value < 0)
    {
        D_80182DF4 = 0;
    }
    timer = D_801B2E0C;
    ((u16*)D_8013B240)[2] += 0x16;
    next_timer = timer - 1;
    D_801B2E0C = next_timer;
    if (next_timer == 0)
    {
        D_801B2E08++;
    }
}

/**
 * @brief Reset a range of world-map actor slots and kick the next handler.
 * @note Clears three parallel slot arrays for indices [0xA, 0x14), seeds each
 *       actor's default fields, then advances the shared step counters.
 */
void func_800A38A0(void)
{
extern u8 D_800D9268[];
extern u8 D_80139988[];
extern u8 D_801AFBD0[];
extern s32 D_8011F538;
extern s32 D_80182DE4;
extern s32 D_800DCEA8;
extern s32 D_800D9150;
extern s32 D_801B2E10;
extern s32 D_801B2E14;

    s32 i;
    u8* pa;
    u8* pb;

    D_80182DE4 = 1;
    D_800DCEA8 = 1;
    for (i = 0xA; i < 0x14; i++)
    {
        *(s16*)(D_801AFBD0 + i * 0x14) = 0;
        pb = D_80139988 + i * 0x8;
        *(s32*)(pb + 0x4) = (s32)&D_8011F538;
        pa = D_800D9268 + i * 0x2C;
        *(s16*)(pa + 0x2) = 0;
        *(s8*)(pa + 0x6) = 0xF;
        *(s16*)(pa + 0xE) = 0;
        *(s16*)(pa + 0x10) = -1;
    }
    D_800D9150 = 6;
    D_801B2E14 = 0x10;
    D_801B2E10 += 1;
    func_800A4DF4();
}

/** @brief World-map step: init hero struct fields, clear tables, advance step. */
void func_800A3960(void)
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
extern u8 D_80123538[];
extern void *D_80139280;
extern s32 D_801B2E18;
extern s32 D_801B2E1C;

    s32 i;
    void *base = D_80139280;

    *(s32 *)((u8 *)base + 0x0C) = 0x20;
    *(s32 *)((u8 *)base + 0x14) = 2;
    *(s32 *)((u8 *)base + 0x18) = 0x384;
    *(s32 *)((u8 *)base + 0x1C) = 0x14;
    *(s32 *)((u8 *)base + 0x20) = 8;
    *(s32 *)((u8 *)base + 0x24) = 1;
    *(s32 *)((u8 *)base + 0x04) = 0;
    *(s32 *)((u8 *)base + 0x08) = 0;
    *(s32 *)((u8 *)base + 0x10) = 0;
    *(s32 *)((u8 *)base + 0x28) = 0x32C8;

    for (i = 0; i < 0xC; i++)
    {
        D_801AFBD0[20 + i].field0 = 0;
        D_80139988[20 + i].field4 = D_80123538;
    }

    D_801B2E1C = 0x18;
    D_801B2E18 += 1;
    func_800A5118();
}

/** @brief Reset the effect actors with randomized animation variants. */
void func_800A3A2C(void)
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

extern WmapConfigA D_800D9268[];
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern u8 D_8011F538[];
extern s32 D_801B25D8;
extern s32 D_800DCEAC;
extern s32 D_800D9154;
extern s32 D_801B2E20;
extern s32 D_801B2E24;
extern s32 rand(void);

    s32 i;

    D_801B25D8 = 1;
    D_800DCEAC = 1;
    for (i = 80; i < 140; i++)
    {
        D_801AFBD0[i].field_00 = 0;
        D_80139988[i].field_04 = D_8011F538;
        D_800D9268[i].field_02 = 0;
        D_800D9268[i].field_06 = 15;
        D_800D9268[i].field_0E = rand() % 3 + 2;
        D_800D9268[i].field_10 = -1;
    }
    D_800D9154 = 2;
    D_801B2E24 = 16;
    D_801B2E20++;
    func_800A5318();
}

/** @brief Initialize sixty alternating actors and begin their timed effect. */
void func_800A3B3C(void)
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

extern WmapConfigA D_800D9268[];
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern u8 D_80121538[];
extern s32 D_801B25DC;
extern s32 D_800DCEB0;
extern s32 D_800D9158;
extern s32 D_801B2E28;
extern s32 D_801B2E2C;

    s32 i;

    D_801B25DC = 1;
    D_800DCEB0 = 12;
    for (i = 150; i < 210; i++)
    {
        D_801AFBD0[i].field_00 = 0;
        D_80139988[i].field_04 = D_80121538;
        D_800D9268[i].field_0E = i & 1;
        D_800D9268[i].field_02 = 0;
        D_800D9268[i].field_06 = 15;
        D_800D9268[i].field_10 = -1;
    }
    D_800D9158 = 1;
    D_801B2E2C = 16;
    D_801B2E28++;
    func_800A5660();
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void func_800A3C04(void)
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
extern s32 D_80123538[];
extern WmapPair D_80139988[];
extern WmapAfcEntry D_801AFBD0[];
extern s32 D_800D915C;
extern s32 D_800DCEB4;
extern s32 D_801B25D8;
extern s32 D_801B2E30;
extern s32 D_801B2E34;

    s32 i;
    WmapD94Entry *entry;

    i = 0xD2;
    D_801B25D8 = 1;
    D_800DCEB4 = 8;

    do
    {
        D_801AFBD0[i].unk0 = 0;
        D_80139988[i].unk4 = D_80123538;
        entry = &D_800D9268[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 0;
        entry->unk10 = -1;
        i++;
    } while (i < 0xE6);

    D_800D915C = 2;
    D_801B2E34 = 0x10;
    D_801B2E30 += 1;
    func_800A59A8();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A3CC8(s32 arg0)
{
extern u32 D_801B2DD0;
extern s32 D_801B2DD4;
extern void (*D_800D6A84[])(void);
extern s32 D_8013B20C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2DD0 = 1;
        D_801B2DD4 = 1;
        return 1;
    }

    if (D_801B2DD0 < 0x6)
    {
        D_800D6A84[D_801B2DD0]();
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
void func_800A3D40(void)
{
extern u32 D_801B2DD0;
extern s32 D_801B2DD4;
extern void (*D_800D6A84[])(void);
extern s32 D_8013B20C;

    D_801B2DD0 = 1;
    D_801B2DD4 = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_800A3D58(void)
{
extern u32 D_801B2DD0;
extern s32 D_801B2DD4;
extern void (*D_800D6A84[])(void);
extern s32 D_8013B20C;

    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2DD0 += 1;
    func_800A3D9C();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A3D9C(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2DD0;

    if (D_8013B20C == 0)
    {
        D_801B2DD0 += 1;
        func_800A3DD8();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_800A3DD8(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2DD0;

    func_8006CAC0(func_800A3E70);
    D_8013B20C = 1;
    D_801B2DD0 += 1;
    func_800A3E1C();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A3E1C(void)
{
extern s32 D_801B2DD0;
extern s32 D_8013B20C;

    if (D_8013B20C == 0)
    {
        D_801B2DD0 += 1;
        func_800A3E58();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A3E58(void)
{
extern s32 D_801B2DD0;
extern s32 D_8013B20C;

    D_801B2DD0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A3E70(s32 arg0)
{
extern u32 D_801B2DD8;
extern s32 D_801B2DDC;
extern void (*D_800D6A9C[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2DD8 = 1;
        D_801B2DDC = 1;
        return 1;
    }

    if (D_801B2DD8 < 0x18)
    {
        D_800D6A9C[D_801B2DD8]();
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
void func_800A3EE8(void)
{
extern u32 D_801B2DD8;
extern s32 D_801B2DDC;
extern void (*D_800D6A9C[])(void);

    D_801B2DD8 = 1;
    D_801B2DDC = 1;
}

/**
 * @brief Set up a tinted world-map sub-scene and register its handler.
 */
void func_800A3F00(void)
{
extern s32 D_8013B208;
extern s32 D_801ADAF4;
extern s32 D_801B2DD8;
extern s32 D_801B2DDC;

    D_8013B208 = 1;
    func_8006683C(0x702540);
    D_801ADAF4 = 8;
    func_800652A8(0x2B, 0x80);
    func_8006CAC0(func_800A4D64);
    D_801B2DDC = 0x20;
    D_801B2DD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A3F6C(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    if (--D_801B2DDC == 0)
    {
        D_801B2DD8 += 1;
    }
}

/** @brief Register callbacks, set effect flags and color, and begin a ten-tick delay. */
void func_800A3FA0(void)
{
extern s32 D_80139244;
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern s32 D_801B2DD8;
extern s32 D_801B2DDC;

    func_8006CAC0(&func_800A4810);
    D_80139244 = 1;
    D_801ADAE0 = 1;
    func_8006CAC0(&func_800A44CC);
    D_801ADAF4 = 1;
    func_8006683C(0x201010);
    D_801B2DDC = 0xA;
    D_801B2DD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A4018(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    if (--D_801B2DDC == 0)
    {
        D_801B2DD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800A404C(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    func_8006CAC0(func_800A4ABC);
    D_801B2DDC = 0x1E;
    D_801B2DD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A4088(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    if (--D_801B2DDC == 0)
    {
        D_801B2DD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800A40BC(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    func_8006CAC0(func_800A4C10);
    D_801B2DDC = 0xC;
    D_801B2DD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A40F8(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    if (--D_801B2DDC == 0)
    {
        D_801B2DD8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800A412C(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    func_8006CAC0(func_800A55D0);
    func_8006CAC0(func_800A5918);
    D_801B2DDC = 0x44;
    D_801B2DD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A4174(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    if (--D_801B2DDC == 0)
    {
        D_801B2DD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800A41A8(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    func_8006CAC0(func_800A4968);
    D_801B2DDC = 0x2;
    D_801B2DD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A41E4(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    if (--D_801B2DDC == 0)
    {
        D_801B2DD8 += 1;
    }
}

/** @brief World-map step: kick a sub-request, set the next state, and arm the timer. */
void func_800A4218(void)
{
extern s32 D_80139244;
extern s32 D_801ADAF4;
extern s32 D_801B2DD8;
extern s32 D_801B2DDC;

    D_80139244 = 0;
    func_8006683C(0x504060);
    D_801ADAF4 = 8;
    D_801B2DDC = 0x1E;
    D_801B2DD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A4268(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    if (--D_801B2DDC == 0)
    {
        D_801B2DD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800A429C(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    func_8006CAC0(func_800A5288);
    D_801B2DDC = 0x10;
    D_801B2DD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A42D8(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    if (--D_801B2DDC == 0)
    {
        D_801B2DD8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800A430C(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    func_8006CAC0(func_800A5088);
    func_8006CAC0(func_800A4670);
    D_801B2DDC = 0x64;
    D_801B2DD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A4354(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    if (--D_801B2DDC == 0)
    {
        D_801B2DD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800A4388(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    func_8006CAC0(func_800A5C60);
    D_801B2DDC = 0x4;
    D_801B2DD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A43C4(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    if (--D_801B2DDC == 0)
    {
        D_801B2DD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800A43F8(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    func_8006CAC0(func_800A5088);
    D_801B2DDC = 0x7C;
    D_801B2DD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A4434(void)
{
extern s32 D_801B2DDC;
extern s32 D_801B2DD8;

    if (--D_801B2DDC == 0)
    {
        D_801B2DD8 += 1;
    }
}

void func_800A4468(void)
{
/* Partial WMAP decompilation: 95.600000% (gcc280_g0). */

extern s32 D_8013B20C;
extern u32 D_80139290[];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2DD8;

    D_8013B20C = 0;
    D_80139290[D_8011D530 * 10 + D_8011D510 * 60] = D_8011D4FC | 0x100;
    D_801B2DD8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A44CC(s32 arg0)
{
extern u32 D_801B2DE0;
extern s32 D_801B2DE4;
extern void (*D_800D6AFC[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2DE0 = 1;
        D_801B2DE4 = 1;
        return 1;
    }

    if (D_801B2DE0 < 0x4)
    {
        D_800D6AFC[D_801B2DE0]();
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
void func_800A4544(void)
{
extern u32 D_801B2DE0;
extern s32 D_801B2DE4;
extern void (*D_800D6AFC[])(void);

    D_801B2DE0 = 1;
    D_801B2DE4 = 1;
}

void func_800A455C(void)
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
extern s32 D_801B2DE0;
extern s32 D_801B2DE4;

    D_801399AC = &D_8011F538;
    D_800D9318.field_06 = 0xF;
    D_800D9318.field_10 = -1;
    D_800D9318.field_26 = 8;
    D_800D9318.field_0E = 1;
    D_800D9318.field_24 = 1;
    D_800D9318.field_02 = 0;
    D_800D9318.field_22 = 0x81;
    D_801B2DE4 = 0x80;
    D_801B2DE0 += 1;
    func_800A45DC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800A45DC(void)
{
extern s32 D_801B2DE0;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B2DE4;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0x19, 0x8, 0);
    if (--D_801B2DE4 == 0)
    {
        D_801B2DE0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A4658(void)
{
extern s32 D_801B2DE0;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;
extern s32 D_801B2DE4;

    D_801B2DE0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A4670(s32 arg0)
{
extern u32 D_801B2DE8;
extern s32 D_801B2DEC;
extern void (*D_800D6B0C[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2DE8 = 1;
        D_801B2DEC = 1;
        return 1;
    }

    if (D_801B2DE8 < 0x4)
    {
        D_800D6B0C[D_801B2DE8]();
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
void func_800A46E8(void)
{
extern u32 D_801B2DE8;
extern s32 D_801B2DEC;
extern void (*D_800D6B0C[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801B2DE8 = 1;
    D_801B2DEC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800A4700(void)
{
extern u32 D_801B2DE8;
extern s32 D_801B2DEC;
extern void (*D_800D6B0C[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801399B4 = D_8011D538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 4;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x22] = 0x80;
    *(s16*)&D_800D9344[0x24] = 0;
    D_801B2DEC = 0x88;
    D_801B2DE8 += 1;
    func_800A477C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800A477C(void)
{
extern u32 D_801B2DE8;
extern s32 D_801B2DEC;
extern void (*D_800D6B0C[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x1F, 0x8, 0);
    if (--D_801B2DEC == 0)
    {
        D_801B2DE8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A47F8(void)
{
extern s32 D_801B2DE8;

    D_801B2DE8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A4810(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2DF0;
extern s32 D_801B2DF4;
extern void (*D_800D6B1C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2DF0 = 1;
        D_801B2DF4 = 1;
        return 1;
    }

    if (D_801B2DF0 < 0x4)
    {
        D_800D6B1C[D_801B2DF0]();
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
void func_800A4888(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2DF0;
extern s32 D_801B2DF4;
extern void (*D_800D6B1C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;

    D_801B2DF0 = 1;
    D_801B2DF4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_800A48A0(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2DF0;
extern s32 D_801B2DF4;
extern void (*D_800D6B1C[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;

    D_801B24A0 = D_80139258;
    D_801B2650 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B2DF4 = 0x40;
    D_801B2DF0 += 1;
    func_800A3348();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A4950(void)
{
extern s32 D_801B2DF0;

    D_801B2DF0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A4968(s32 arg0)
{
extern u32 D_801B2DF8;
extern s32 D_801B2DFC;
extern void (*D_800D6B2C[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2DF8 = 1;
        D_801B2DFC = 1;
        return 1;
    }

    if (D_801B2DF8 < 0x4)
    {
        D_800D6B2C[D_801B2DF8]();
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
void func_800A49E0(void)
{
extern u32 D_801B2DF8;
extern s32 D_801B2DFC;
extern void (*D_800D6B2C[])(void);

    D_801B2DF8 = 1;
    D_801B2DFC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_800A49F8(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern s32 D_801B2DFC;
extern u32 D_801B2DF8;

    D_801B24A8 = D_80139258;
    D_801B2478 = D_80182DC0;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B2DFC = 0x80;
    D_801B2DF8 += 1;
    func_800A3448();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A4AA4(void)
{
extern s32 D_801B2DF8;

    D_801B2DF8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A4ABC(s32 arg0)
{
extern u32 D_801B2E00;
extern s32 D_801B2E04;
extern void (*D_800D6B3C[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2E00 = 1;
        D_801B2E04 = 1;
        return 1;
    }

    if (D_801B2E00 < 0x6)
    {
        D_800D6B3C[D_801B2E00]();
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
void func_800A4B34(void)
{
extern u32 D_801B2E00;
extern s32 D_801B2E04;
extern void (*D_800D6B3C[])(void);

    D_801B2E00 = 1;
    D_801B2E04 = 1;
}

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void func_800A4B4C(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DF0;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern s32 D_801B2E00;
extern s32 D_801B2E04;

    D_80182DF0 = 1;
    D_8013B238 = D_80139258;
    D_801B2E04 = 0x40;
    D_801B2E00 += 1;
    func_800A3548();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A4BC0(void)
{
extern s32 D_801B2E00;
extern s32 D_801B2E04;

    D_801B2E04 = 0x40;
    D_801B2E00 += 1;
    func_800A3624();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A4BF8(void)
{
extern s32 D_801B2E00;
extern s32 D_801B2E04;

    D_801B2E00 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A4C10(s32 arg0)
{
extern u32 D_801B2E08;
extern s32 D_801B2E0C;
extern void (*D_800D6B54[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2E08 = 1;
        D_801B2E0C = 1;
        return 1;
    }

    if (D_801B2E08 < 0x6)
    {
        D_800D6B54[D_801B2E08]();
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
void func_800A4C88(void)
{
extern u32 D_801B2E08;
extern s32 D_801B2E0C;
extern void (*D_800D6B54[])(void);

    D_801B2E08 = 1;
    D_801B2E0C = 1;
}

/** @brief World-map step handler: copy the source block, set the size cap, and advance. */
void func_800A4CA0(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DF4;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern s32 D_801B2E08;
extern s32 D_801B2E0C;

    D_80182DF4 = 1;
    D_8013B240 = D_80139258;
    D_801B2E0C = 0x14;
    D_801B2E08 += 1;
    func_800A36F4();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A4D14(void)
{
extern s32 D_801B2E08;
extern s32 D_801B2E0C;

    D_801B2E0C = 0x40;
    D_801B2E08 += 1;
    func_800A37D0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A4D4C(void)
{
extern s32 D_801B2E08;
extern s32 D_801B2E0C;

    D_801B2E08 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A4D64(s32 arg0)
{
extern u32 D_801B2E10;
extern s32 D_801B2E14;
extern void (*D_800D6B6C[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2E10 = 1;
        D_801B2E14 = 1;
        return 1;
    }

    if (D_801B2E10 < 0x8)
    {
        D_800D6B6C[D_801B2E10]();
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
void func_800A4DDC(void)
{
extern u32 D_801B2E10;
extern s32 D_801B2E14;
extern void (*D_800D6B6C[])(void);

    D_801B2E10 = 1;
    D_801B2E14 = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_800A4DF4(void)
{
extern s32 D_80182DE4;
extern s32 D_801B2E10;
extern s32 D_801B2E14;

    s32 remaining;

    func_8006B328(0xA, 0x14, 6, -1, -1, -6, 0, 0x19, -0x32, 0x64, -0x32, 0x64, 1, 0x7F, 0x7F, 0, 0);
    D_80182DE4 += 8;
    remaining = D_801B2E14 - 1;
    D_801B2E14 = remaining;
    if (remaining == 0)
    {
        D_801B2E10 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A4EA8(void)
{
extern s32 D_801B2E14;
extern s32 D_801B2E10;

    D_801B2E14 = 0x3C;
    D_801B2E10 += 1;
    func_800A4EE0();
}

/** @brief World-map step: spawn an effect object then count down a timer. */
void func_800A4EE0(void)
{
extern s32 D_801B2E10;
extern s32 D_801B2E14;

    func_8006B328(0xA, 0x14, 6, -1, -1, -6, 0, 0x19, -0x32, 0x64, -0x32,
                  0x64, 1, 0x7F, 0x7F, 0, 0);
    if (--D_801B2E14 == 0)
    {
        D_801B2E10 += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A4F88(void)
{
extern s32 D_800DCEA8;
extern s32 D_801B2E14;
extern s32 D_801B2E10;

    D_800DCEA8 = 0;
    D_801B2E14 = 0x14;
    D_801B2E10 += 1;
    func_800A4FC8();
}

/** @brief World-map step: spawn an effect object then count down a timer. */
void func_800A4FC8(void)
{
extern s32 D_801B2E10;
extern s32 D_801B2E14;

    func_8006B328(0xA, 0x14, 6, -1, -1, -6, 0, 0x19, -0x32, 0x64, -0x32,
                  0x64, 1, 0x7F, 0x7F, 0, 0);
    if (--D_801B2E14 == 0)
    {
        D_801B2E10 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A5070(void)
{
extern s32 D_801B2E10;

    D_801B2E10 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A5088(s32 arg0)
{
extern u32 D_801B2E18;
extern s32 D_801B2E1C;
extern void (*D_800D6B8C[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2E18 = 1;
        D_801B2E1C = 1;
        return 1;
    }

    if (D_801B2E18 < 0x6)
    {
        D_800D6B8C[D_801B2E18]();
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
void func_800A5100(void)
{
extern u32 D_801B2E18;
extern s32 D_801B2E1C;
extern void (*D_800D6B8C[])(void);

    D_801B2E18 = 1;
    D_801B2E1C = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_800A5118(void)
{
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32* D_80139280;
extern s32 D_801B2E1C;
extern s32 D_801B2E18;

    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0xC, 0xFF, 0x1, 0x8, 0, (s32)D_80139280);
    if (--D_801B2E1C == 0)
    {
        D_801B2E18 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_800A51A0(void)
{
extern s32* D_80139280;
extern s32 D_801B2E1C;
extern s32 D_801B2E18;

    D_801B2E1C = 0x20;
    D_80139280[5] = -1;
    D_801B2E18 += 1;
    func_800A51E8();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_800A51E8(void)
{
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32* D_80139280;
extern s32 D_801B2E1C;
extern s32 D_801B2E18;

    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0xC, 0xFF, 0x1, 0x8, 0, (s32)D_80139280);
    if (--D_801B2E1C == 0)
    {
        D_801B2E18 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A5270(void)
{
extern s32 D_801B2E18;

    D_801B2E18 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A5288(s32 arg0)
{
extern u32 D_801B2E20;
extern s32 D_801B2E24;
extern void (*D_800D6BA4[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2E20 = 1;
        D_801B2E24 = 1;
        return 1;
    }

    if (D_801B2E20 < 0x8)
    {
        D_800D6BA4[D_801B2E20]();
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
void func_800A5300(void)
{
extern u32 D_801B2E20;
extern s32 D_801B2E24;
extern void (*D_800D6BA4[])(void);

    D_801B2E20 = 1;
    D_801B2E24 = 1;
}

/** @brief World-map effect spawn: submit a request and tick a refcount. */
void func_800A5318(void)
{
extern s32 D_801B25D8;
extern s32 D_801B2E20;
extern s32 D_801B2E24;

    s32 c;

    func_8006B328(0x50, 0x8C, 2, -1, 0x14, 4, 0x168, 0x19, -0x32, 0x64, -0x32,
                  0x64, 0xB4, 0x81, 0x81, 8, 1);
    D_801B25D8 += 8;
    c = D_801B2E24 - 1;
    D_801B2E24 = c;
    if (c == 0)
    {
        D_801B2E20 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A53D8(void)
{
extern s32 D_801B2E24;
extern s32 D_801B2E20;

    D_801B2E24 = 0x18;
    D_801B2E20 += 1;
    func_800A5410();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_800A5410(void)
{
extern s32 D_801B2E24;
extern s32 D_801B2E20;

    func_8006B328(0x50, 0x8C, 2, -1, 0x14, 4, 0x168, 0x19, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 1);
    if (--D_801B2E24 == 0)
    {
        D_801B2E20 += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A54C4(void)
{
extern s32 D_800DCEAC;
extern s32 D_801B2E24;
extern s32 D_801B2E20;

    D_800DCEAC = 0;
    D_801B2E24 = 0x18;
    D_801B2E20 += 1;
    func_800A5504();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_800A5504(void)
{
extern s32 D_801B2E24;
extern s32 D_801B2E20;

    func_8006B328(0x50, 0x8C, 2, -1, 0x14, 4, 0x168, 0x19, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 1);
    if (--D_801B2E24 == 0)
    {
        D_801B2E20 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A55B8(void)
{
extern s32 D_801B2E20;

    D_801B2E20 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A55D0(s32 arg0)
{
extern u32 D_801B2E28;
extern s32 D_801B2E2C;
extern void (*D_800D6BC4[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2E28 = 1;
        D_801B2E2C = 1;
        return 1;
    }

    if (D_801B2E28 < 0x8)
    {
        D_800D6BC4[D_801B2E28]();
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
void func_800A5648(void)
{
extern u32 D_801B2E28;
extern s32 D_801B2E2C;
extern void (*D_800D6BC4[])(void);

    D_801B2E28 = 1;
    D_801B2E2C = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_800A5660(void)
{
extern s32 D_801B25DC;
extern s32 D_801B2E28;
extern s32 D_801B2E2C;

    s32 remaining;

    func_8006B328(0x96, 0xD2, 1, -1, -1, -3, 0, 8, -0xB4, 0x190, -0xA0, 0x190, 1, 1, 0x81, 4, 2);
    D_801B25DC += 8;
    remaining = D_801B2E2C - 1;
    D_801B2E2C = remaining;
    if (remaining == 0)
    {
        D_801B2E28 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A5720(void)
{
extern s32 D_801B2E2C;
extern s32 D_801B2E28;

    D_801B2E2C = 0x30;
    D_801B2E28 += 1;
    func_800A5758();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_800A5758(void)
{
extern s32 D_801B2E28;
extern s32 D_801B2E2C;

    func_8006B328(0x96, 0xD2, 1, -1, -1, -3, 0, 8, -0xB4, 0x190, -0xA0, 0x190, 1, 1, 0x81, 4, 2);
    if (--D_801B2E2C == 0)
    {
        D_801B2E28 += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A580C(void)
{
extern s32 D_800DCEB0;
extern s32 D_801B2E2C;
extern s32 D_801B2E28;

    D_800DCEB0 = 0;
    D_801B2E2C = 0xA0;
    D_801B2E28 += 1;
    func_800A584C();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_800A584C(void)
{
extern s32 D_801B2E28;
extern s32 D_801B2E2C;

    func_8006B328(0x96, 0xD2, 1, -1, -1, -3, 0, 8, -0xB4, 0x190, -0xA0, 0x190, 1, 1, 0x81, 4, 2);
    if (--D_801B2E2C == 0)
    {
        D_801B2E28 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A5900(void)
{
extern s32 D_801B2E28;

    D_801B2E28 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A5918(s32 arg0)
{
extern u32 D_801B2E30;
extern s32 D_801B2E34;
extern void (*D_800D6BE4[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2E30 = 1;
        D_801B2E34 = 1;
        return 1;
    }

    if (D_801B2E30 < 0x8)
    {
        D_800D6BE4[D_801B2E30]();
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
void func_800A5990(void)
{
extern u32 D_801B2E30;
extern s32 D_801B2E34;
extern void (*D_800D6BE4[])(void);

    D_801B2E30 = 1;
    D_801B2E34 = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_800A59A8(void)
{
extern s32 D_801B25D8;
extern s32 D_801B2E30;
extern s32 D_801B2E34;

    s32 remaining;

    func_8006B328(0xD2, 0xE6, 2, -1, -1, -8, 0x28, 8, -0xB4, 0x168, -0xB4, 0x168, 1, 1, 0x81, 4, 3);
    D_801B25D8 += 8;
    remaining = D_801B2E34 - 1;
    D_801B2E34 = remaining;
    if (remaining == 0)
    {
        D_801B2E30 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A5A68(void)
{
extern s32 D_801B2E34;
extern s32 D_801B2E30;

    D_801B2E34 = 0x28;
    D_801B2E30 += 1;
    func_800A5AA0();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_800A5AA0(void)
{
extern s32 D_801B2E34;
extern s32 D_801B2E30;

    func_8006B328(0xD2, 0xE6, 2, -1, -1, -8, 0x28, 8, -0xB4, 0x168, -0xB4, 0x168, 1, 1, 0x81, 4, 3);
    if (--D_801B2E34 == 0)
    {
        D_801B2E30 += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A5B54(void)
{
extern s32 D_800DCEB4;
extern s32 D_801B2E34;
extern s32 D_801B2E30;

    D_800DCEB4 = 0;
    D_801B2E34 = 0xA0;
    D_801B2E30 += 1;
    func_800A5B94();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_800A5B94(void)
{
extern s32 D_801B2E34;
extern s32 D_801B2E30;

    func_8006B328(0xD2, 0xE6, 2, -1, -1, -8, 0x28, 8, -0xB4, 0x168, -0xB4, 0x168, 1, 1, 0x81, 4, 3);
    if (--D_801B2E34 == 0)
    {
        D_801B2E30 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A5C48(void)
{
extern s32 D_801B2E30;

    D_801B2E30 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A5C60(s32 arg0)
{
extern u32 D_801B2E38;
extern s32 D_801B2E3C;
extern void (*D_800D6C04[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2E38 = 1;
        D_801B2E3C = 1;
        return 1;
    }

    if (D_801B2E38 < 0x4)
    {
        D_800D6C04[D_801B2E38]();
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
void func_800A5CD8(void)
{
extern u32 D_801B2E38;
extern s32 D_801B2E3C;
extern void (*D_800D6C04[])(void);

    D_801B2E38 = 1;
    D_801B2E3C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void func_800A5CF0(void)
{
extern u8* D_801399BC;
extern u8 D_80125538[];
extern u8 D_800D9370[];
extern s32 D_801B2E38;
extern s32 D_801B2E3C;

    D_801399BC = D_80125538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0x10] = -1;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0xE] = 0;
    *(s16*)&D_800D9370[0x26] = 0;
    *(s16*)&D_800D9370[0x22] = 0x80;
    *(s16*)&D_800D9370[0x24] = 0x80;
    D_801B2E3C = 0x82;
    D_801B2E38 += 1;
    func_800A5D68();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800A5D68(void)
{
extern s32 D_801B2E38;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B2E3C;

    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_8011CF4C, 0x1F, 0x7, 0);
    if (--D_801B2E3C == 0)
    {
        D_801B2E38 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A5DE4(void)
{
extern s32 D_801B2E38;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B2E3C;

    D_801B2E38 += 1;
}
