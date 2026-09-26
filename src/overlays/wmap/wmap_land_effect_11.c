#include "wmap_main.h"
#include "wmap_land_effect_11.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

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

/** @brief World-map step: fill a spawn descriptor, clear its slot run, then advance. */
void func_8008543C(void)
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

/** @brief World-map spawn descriptor; a 0x50-byte record addressed via D_80139280. */
typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
    s32 field_0C;
    s32 field_10;
    s32 field_14;
    s32 field_18;
    s32 field_1C;
    s32 field_20;
    s32 field_24;
    s32 field_28;
    u8 pad_2C[0x24];
} WmapConfig;

extern s32 D_801B0FD0;
extern WmapConfig *D_80139280;
extern WmapSlot14 D_801AFBD0[];
extern WmapSlot8 D_80139988[];
extern s32 D_8011D538;
extern s32 D_801B28D4;
extern s32 D_801B28D0;
extern void func_800863E4__for_func_8008543C(void) __asm__("func_800863E4");

    s32 i;

    D_801B0FD0 = 0x46;
    D_80139280[0].field_04 = 2;
    D_80139280[0].field_08 = 8;
    D_80139280[0].field_0C = 0x40;
    D_80139280[0].field_10 = 2;
    D_80139280[0].field_14 = 2;
    D_80139280[0].field_18 = 0x190;
    D_80139280[0].field_1C = 0x14;
    D_80139280[0].field_20 = 0xD;
    D_80139280[0].field_24 = 1;
    D_80139280[0].field_28 = 0x3E8;
    for (i = 0; i < 0x46; i++)
    {
        D_801AFBD0[i + D_80139280[0].field_1C].field_00 = 0;
        D_80139988[i + 0x18].field_04 = &D_8011D538;
    }
    D_801B28D4 = 0x8D;
    D_801B28D0 += 1;
    func_800863E4__for_func_8008543C();
}

/** @brief World-map step: fill a spawn descriptor, clear its slot run, then advance. */
void func_80085528(void)
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

/** @brief World-map spawn descriptor addressed via D_80139280. */
typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
    s32 field_0C;
    s32 field_10;
    s32 field_14;
    s32 field_18;
    s32 field_1C;
    s32 field_20;
    s32 field_24;
    s32 field_28;
    s32 field_2C;
    s32 field_30;
    s32 field_34;
    s32 field_38;
    s32 field_3C;
    s32 field_40;
    s32 field_44;
    s32 field_48;
    s32 field_4C;
    s32 field_50;
} WmapConfig;

extern s32 D_801B0FD0;
extern WmapConfig *D_80139280;
extern WmapSlot14 D_801AFBD0[];
extern WmapSlot8 D_80139988[];
extern s32 D_8011D538;
extern s32 D_801B28E4;
extern s32 D_801B28E0;
extern void func_80086778__for_func_80085528(void) __asm__("func_80086778");

    s32 i;

    D_801B0FD0 = 0xA;
    D_80139280[0].field_2C = 1;
    D_80139280[0].field_30 = 6;
    D_80139280[0].field_34 = 0x90;
    D_80139280[0].field_38 = 2;
    D_80139280[0].field_3C = 3;
    D_80139280[0].field_40 = 0x60;
    D_80139280[0].field_44 = 0x64;
    D_80139280[0].field_48 = 0xD;
    D_80139280[0].field_4C = 0;
    D_80139280[0].field_50 = 0x2EE0;
    for (i = 0; i < 0xA; i++)
    {
        D_801AFBD0[i + D_80139280[0].field_44].field_00 = 0;
        D_80139988[i + 0x68].field_04 = &D_8011D538;
    }
    D_801B28E4 = 0x29;
    D_801B28E0 += 1;
    func_80086778__for_func_80085528();
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80085618(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B28F4;
extern s32 D_801B28F0;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B28EC;
extern s32 D_801B28E8;

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
        D_80182DE8 -= 0x5;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--D_801B28EC == 0)
    {
        D_801B28E8 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80085718(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B28F4;
extern s32 D_801B28F0;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B28EC;
extern s32 D_801B28E8;

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
    if (--D_801B28F4 == 0)
    {
        D_801B28F0 += 1;
    }
}

/** @brief Draw and brighten two rotating effect layers and advance their shared countdown. */
void func_80085818(void)
{
extern void *D_8011CF24;
extern void *D_8011CF28;
extern VECTOR g_wmap_camera_translation;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_80182DF0;
extern s32 D_801B28F8;
extern s32 D_801B28FC;

    s32 remaining;
    s32 intensity;

    PushMatrix();
    func_8006CFA8(&g_wmap_camera_translation, &D_801B2490);
    func_8006CD98(D_8011CF24, 0, 4, 54, 0x7900, 1, D_80182DF0);
    D_801B2490.vz = (u16)(D_801B2490.vz + 24);
    func_8006CFA8(&g_wmap_camera_translation, &D_801B2498);
    func_8006CD98(D_8011CF28, 0, 4, 54, 0x7900, 1, D_80182DF0);
    D_801B2498.vz = (u16)(D_801B2498.vz - 16);
    PopMatrix();
    intensity = D_80182DF0 + 2;
    D_80182DF0 = intensity;
    if (intensity >= 130)
    {
        D_80182DF0 = 129;
    }
    remaining = D_801B28FC - 1;
    D_801B28FC = remaining;
    if (remaining == 0)
    {
        D_801B28F8++;
    }
}

/** @brief Draw two rotating effect layers and advance their shared countdown. */
void func_80085950(void)
{
extern void *D_8011CF24;
extern void *D_8011CF28;
extern VECTOR g_wmap_camera_translation;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_80182DF0;
extern s32 D_801B28F8;
extern s32 D_801B28FC;

    s32 remaining;

    PushMatrix();
    func_8006CFA8(&g_wmap_camera_translation, &D_801B2490);
    func_8006CD98(D_8011CF24, 0, 4, 54, 0x7900, 1, D_80182DF0);
    D_801B2490.vz = (u16)(D_801B2490.vz + 24);
    func_8006CFA8(&g_wmap_camera_translation, &D_801B2498);
    func_8006CD98(D_8011CF28, 0, 4, 54, 0x7900, 1, D_80182DF0);
    D_801B2498.vz = (u16)(D_801B2498.vz - 16);
    PopMatrix();
    remaining = D_801B28FC - 1;
    D_80182DF0 -= 2;
    D_801B28FC = remaining;
    if (remaining == 0)
    {
        D_801B28F8++;
    }
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80085A70(s32 arg0)
{
extern u32 D_801B28B0;
extern s32 D_801B28B4;
extern void (*D_800D5960[])(void);
extern void func_80085B44__for_func_80085A70(void) __asm__("func_80085B44");
extern s32 D_8013B20C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B28B0 = 1;
        D_801B28B4 = 1;
        return 1;
    }

    if (D_801B28B0 < 0x6)
    {
        D_800D5960[D_801B28B0]();
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
void func_80085AE8(void)
{
extern u32 D_801B28B0;
extern s32 D_801B28B4;
extern void (*D_800D5960[])(void);
extern void func_80085B44__for_func_80085AE8(void) __asm__("func_80085B44");
extern s32 D_8013B20C;

    D_801B28B0 = 1;
    D_801B28B4 = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80085B00(void)
{
extern u32 D_801B28B0;
extern s32 D_801B28B4;
extern void (*D_800D5960[])(void);
extern void func_80085B44__for_func_80085B00(void) __asm__("func_80085B44");
extern s32 D_8013B20C;

    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B28B0 += 1;
    func_80085B44__for_func_80085B00();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80085B44(void)
{
extern s32 D_8013B20C;
extern s32 D_801B28B0;
extern void func_80085B80__for_func_80085B44(void) __asm__("func_80085B80");
extern void func_80085C18__for_func_80085B44(void) __asm__("func_80085C18");
extern void func_80085BC4__for_func_80085B44(void) __asm__("func_80085BC4");

    if (D_8013B20C == 0)
    {
        D_801B28B0 += 1;
        func_80085B80__for_func_80085B44();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80085B80(void)
{
extern s32 D_8013B20C;
extern s32 D_801B28B0;
extern void func_80085B80(void);
extern void func_80085C18__for_func_80085B80(void) __asm__("func_80085C18");
extern void func_80085BC4__for_func_80085B80(void) __asm__("func_80085BC4");

    func_8006CAC0(func_80085C18__for_func_80085B80);
    D_8013B20C = 1;
    D_801B28B0 += 1;
    func_80085BC4__for_func_80085B80();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80085BC4(void)
{
extern s32 D_801B28B0;
extern s32 D_8013B20C;
extern void func_80085C00__for_func_80085BC4(void) __asm__("func_80085C00");

    if (D_8013B20C == 0)
    {
        D_801B28B0 += 1;
        func_80085C00__for_func_80085BC4();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80085C00(void)
{
extern s32 D_801B28B0;
extern s32 D_8013B20C;
extern void func_80085C00(void);

    D_801B28B0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80085C18(s32 arg0)
{
extern u32 D_801B28B8;
extern s32 D_801B28BC;
extern void (*D_800D5978[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B28B8 = 1;
        D_801B28BC = 1;
        return 1;
    }

    if (D_801B28B8 < 0xE)
    {
        D_800D5978[D_801B28B8]();
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
void func_80085C90(void)
{
extern u32 D_801B28B8;
extern s32 D_801B28BC;
extern void (*D_800D5978[])(void);

    D_801B28B8 = 1;
    D_801B28BC = 1;
}

/**
 * @brief Set up a world-map sub-scene: request assets and register its handler.
 */
void func_80085CA8(void)
{
extern s32 D_8013B208;
extern s32 D_801B28B8;
extern s32 D_801B28BC;
extern void func_80086354__for_func_80085CA8(void) __asm__("func_80086354");

    D_8013B208 = 1;
    wmap_play_sound(0x20, 0x80);
    func_8006CAC0(func_80086354__for_func_80085CA8);
    D_801B28BC = 8;
    D_801B28B8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80085CFC(void)
{
extern s32 D_801B28BC;
extern s32 D_801B28B8;
extern void func_80086544__for_func_80085CFC(void) __asm__("func_80086544");

    if (--D_801B28BC == 0)
    {
        D_801B28B8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80085D30(void)
{
extern s32 D_801B28BC;
extern s32 D_801B28B8;
extern void func_80086544__for_func_80085D30(void) __asm__("func_80086544");

    func_8006CAC0(func_80086544__for_func_80085D30);
    D_801B28BC = 0x1E;
    D_801B28B8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80085D6C(void)
{
extern s32 D_801B28BC;
extern s32 D_801B28B8;

    if (--D_801B28BC == 0)
    {
        D_801B28B8 += 1;
    }
}

/** @brief Register world-map callbacks, seed a mode value, and advance step counters. */
void func_80085DA0(void)
{
extern s32 D_801B28B8;
extern s32 D_801B28BC;
extern void func_800868E0__for_func_80085DA0(void) __asm__("func_800868E0");
extern void func_8008600C__for_func_80085DA0(void) __asm__("func_8008600C");

    func_8006CAC0(func_800868E0__for_func_80085DA0);
    wmap_start_map_tint(0x801045);
    g_wmap_backdrop_target_level = 4;
    func_8006CAC0(func_8008600C__for_func_80085DA0);
    D_801B28BC = 2;
    D_801B28B8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80085E00(void)
{
extern s32 D_801B28BC;
extern s32 D_801B28B8;

    if (--D_801B28BC == 0)
    {
        D_801B28B8 += 1;
    }
}

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void func_80085E34(void)
{
extern s32 D_801ADAE0;
extern s32 D_801B28BC;
extern s32 D_801B28B8;
extern void func_80086B90__for_func_80085E34(void) __asm__("func_80086B90");

    D_801ADAE0 = 1;
    func_8006CAC0(func_80086B90__for_func_80085E34);
    D_801B28BC = 0x3C;
    D_801B28B8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80085E7C(void)
{
extern s32 D_801B28BC;
extern s32 D_801B28B8;
extern void func_80086A38__for_func_80085E7C(void) __asm__("func_80086A38");
extern void func_800866E8__for_func_80085E7C(void) __asm__("func_800866E8");

    if (--D_801B28BC == 0)
    {
        D_801B28B8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_80085EB0(void)
{
extern s32 D_801B28BC;
extern s32 D_801B28B8;
extern void func_80086A38__for_func_80085EB0(void) __asm__("func_80086A38");
extern void func_800866E8__for_func_80085EB0(void) __asm__("func_800866E8");

    func_8006CAC0(func_80086A38__for_func_80085EB0);
    func_8006CAC0(func_800866E8__for_func_80085EB0);
    D_801B28BC = 0x2;
    D_801B28B8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80085EF8(void)
{
extern s32 D_801B28BC;
extern s32 D_801B28B8;
extern void func_800861B0__for_func_80085EF8(void) __asm__("func_800861B0");
extern void func_80086CDC__for_func_80085EF8(void) __asm__("func_80086CDC");

    if (--D_801B28BC == 0)
    {
        D_801B28B8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_80085F2C(void)
{
extern s32 D_801B28BC;
extern s32 D_801B28B8;
extern void func_800861B0__for_func_80085F2C(void) __asm__("func_800861B0");
extern void func_80086CDC__for_func_80085F2C(void) __asm__("func_80086CDC");

    func_8006CAC0(func_800861B0__for_func_80085F2C);
    func_8006CAC0(func_80086CDC__for_func_80085F2C);
    D_801B28BC = 0xBF;
    D_801B28B8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80085F74(void)
{
extern s32 D_801B28BC;
extern s32 D_801B28B8;

    if (--D_801B28BC == 0)
    {
        D_801B28B8 += 1;
    }
}

/** @brief Clear the sequence gate, update the active map cell, and advance the step. */
void func_80085FA8(void)
{
extern s32 D_8013B20C;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B28B8;

    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B28B8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008600C(s32 arg0)
{
extern u32 D_801B28C0;
extern s32 D_801B28C4;
extern void (*D_800D59B0[])(void);
extern void func_8008611C__for_func_8008600C(void) __asm__("func_8008611C");
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B28C0 = 1;
        D_801B28C4 = 1;
        return 1;
    }

    if (D_801B28C0 < 0x4)
    {
        D_800D59B0[D_801B28C0]();
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
void func_80086084(void)
{
extern u32 D_801B28C0;
extern s32 D_801B28C4;
extern void (*D_800D59B0[])(void);
extern void func_8008611C__for_func_80086084(void) __asm__("func_8008611C");
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    D_801B28C0 = 1;
    D_801B28C4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8008609C(void)
{
extern u32 D_801B28C0;
extern s32 D_801B28C4;
extern void (*D_800D59B0[])(void);
extern void func_8008611C__for_func_8008609C(void) __asm__("func_8008611C");
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0xE] = 3;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 8;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0x22] = 0x80;
    *(s16*)&D_800D9318[0x24] = 0;
    D_801B28C4 = 0x40;
    D_801B28C0 += 1;
    func_8008611C__for_func_8008609C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008611C(void)
{
extern u32 D_801B28C0;
extern s32 D_801B28C4;
extern void (*D_800D59B0[])(void);
extern void func_8008611C(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D9318, D_801399A8);
    wmap_draw_actor_sprite(D_800D9318, D_8011CF4C, 0xD, 0x1E, 0);
    if (--D_801B28C4 == 0)
    {
        D_801B28C0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80086198(void)
{
extern s32 D_801B28C0;

    D_801B28C0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800861B0(s32 arg0)
{
extern u32 D_801B28C8;
extern s32 D_801B28CC;
extern void (*D_800D59C0[])(void);
extern void func_800862C0__for_func_800861B0(void) __asm__("func_800862C0");
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B28C8 = 1;
        D_801B28CC = 1;
        return 1;
    }

    if (D_801B28C8 < 0x4)
    {
        D_800D59C0[D_801B28C8]();
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
void func_80086228(void)
{
extern u32 D_801B28C8;
extern s32 D_801B28CC;
extern void (*D_800D59C0[])(void);
extern void func_800862C0__for_func_80086228(void) __asm__("func_800862C0");
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801B28C8 = 1;
    D_801B28CC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80086240(void)
{
extern u32 D_801B28C8;
extern s32 D_801B28CC;
extern void (*D_800D59C0[])(void);
extern void func_800862C0__for_func_80086240(void) __asm__("func_800862C0");
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801399B4 = D_8011F538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 2;
    *(s16*)&D_800D9344[0x22] = 0x81;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x24] = 1;
    D_801B28CC = 0xC0;
    D_801B28C8 += 1;
    func_800862C0__for_func_80086240();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800862C0(void)
{
extern u32 D_801B28C8;
extern s32 D_801B28CC;
extern void (*D_800D59C0[])(void);
extern void func_800862C0(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D9344, D_801399B0);
    wmap_draw_actor_sprite(D_800D9344, D_8011CF4C, 0x18, 0xA, 0);
    if (--D_801B28CC == 0)
    {
        D_801B28C8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008633C(void)
{
extern s32 D_801B28C8;

    D_801B28C8 += 1;
}

/** @brief Reset or dispatch the current effect phase.
 * @param reset Nonzero to restart the effect.
 * @return One while active, otherwise zero.
 */
s32 func_80086354(s32 reset)
{
extern void (*D_800D59D0[])(void);
extern u32 D_801B28D0;
extern s32 D_801B28D4;

    s32 active;
    if (reset != 0)
    {
        D_801B28D0 = 1;
        D_801B28D4 = 1;
        return 1;
    }
    if (D_801B28D0 < 6U)
    {
        D_800D59D0[D_801B28D0]();
        active = 1;
    }
    else
    {
        active = 0;
    }
    return active;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800863CC(void)
{
extern s32 D_801B28D0;
extern s32 D_801B28D4;

    D_801B28D0 = 1;
    D_801B28D4 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800863E4(void)
{
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32 D_80139280;
extern s32 D_801B28D0;
extern s32 D_801B28D4;

    func_8006A2FC(D_800D9688, D_80139A48, 0x46, 0, 0x7F, 0x2, 0, D_80139280);
    if (--D_801B28D4 == 0)
    {
        D_801B28D0 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80086464(void)
{
extern void func_800864AC__for_func_80086464(void) __asm__("func_800864AC");
extern s32* D_80139280;
extern s32 D_801B28D4;
extern s32 D_801B28D0;

    D_801B28D4 = 0x40;
    D_80139280[5] = -1;
    D_801B28D0 += 1;
    func_800864AC__for_func_80086464();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800864AC(void)
{
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32 D_80139280;
extern s32 D_801B28D0;
extern s32 D_801B28D4;

    func_8006A2FC(D_800D9688, D_80139A48, 0x46, 0, 0x7F, 0x2, 0, D_80139280);
    if (--D_801B28D4 == 0)
    {
        D_801B28D0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008652C(void)
{
extern s32 D_801B28D0;

    D_801B28D0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80086544(s32 arg0)
{
extern u32 D_801B28D8;
extern s32 D_801B28DC;
extern void (*D_800D59E8[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B28D8 = 1;
        D_801B28DC = 1;
        return 1;
    }

    if (D_801B28D8 < 0x4)
    {
        D_800D59E8[D_801B28D8]();
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
void func_800865BC(void)
{
extern u32 D_801B28D8;
extern s32 D_801B28DC;
extern void (*D_800D59E8[])(void);

    D_801B28D8 = 1;
    D_801B28DC = 1;
}

/** @brief Configure the world-map actor and advance to its draw step. */
void func_800865D4(void)
{
extern WmapConfigA D_800D9370;
extern u8 D_8011D538;
extern void *D_801399BC;
extern s32 D_801B28D8;
extern s32 D_801B28DC;
extern void func_80086654__for_func_800865D4(void) __asm__("func_80086654");

    D_801399BC = &D_8011D538;
    D_800D9370.field_06 = 0xF;
    D_800D9370.field_0E = 2;
    D_800D9370.field_10 = -1;
    D_800D9370.field_26 = 2;
    D_800D9370.field_22 = 0x81;
    D_800D9370.field_02 = 0;
    D_800D9370.field_24 = 1;
    D_801B28DC = 0x62;
    D_801B28D8 += 1;
    func_80086654__for_func_800865D4();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80086654(void)
{
extern s32 D_801B28D8;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B28DC;

    wmap_step_actor_animation(D_800D9370, D_801399B8);
    wmap_draw_actor_sprite(D_800D9370, D_8011CF4C, 0xD, 0x3, 0);
    if (--D_801B28DC == 0)
    {
        D_801B28D8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800866D0(void)
{
extern s32 D_801B28D8;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B28DC;

    D_801B28D8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800866E8(s32 arg0)
{
extern u32 D_801B28E0;
extern s32 D_801B28E4;
extern void (*D_800D59F8[])(void);
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32 D_80139280;

    s32 result;

    if (arg0 != 0)
    {
        D_801B28E0 = 1;
        D_801B28E4 = 1;
        return 1;
    }

    if (D_801B28E0 < 0x6)
    {
        D_800D59F8[D_801B28E0]();
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
void func_80086760(void)
{
extern u32 D_801B28E0;
extern s32 D_801B28E4;
extern void (*D_800D59F8[])(void);
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32 D_80139280;

    D_801B28E0 = 1;
    D_801B28E4 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80086778(void)
{
extern u32 D_801B28E0;
extern s32 D_801B28E4;
extern void (*D_800D59F8[])(void);
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32 D_80139280;

    func_8006A2FC(D_800DA448, D_80139CC8, 0xA, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B28E4 == 0)
    {
        D_801B28E0 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_800867FC(void)
{
extern void func_80086844__for_func_800867FC(void) __asm__("func_80086844");
extern s32* D_80139280;
extern s32 D_801B28E4;
extern s32 D_801B28E0;
extern u8 D_800DA448[];
extern u8 D_80139CC8[];

    D_801B28E4 = 0x20;
    D_80139280[15] = -1;
    D_801B28E0 += 1;
    func_80086844__for_func_800867FC();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80086844(void)
{
extern void func_80086844(void);
extern s32* D_80139280;
extern s32 D_801B28E4;
extern s32 D_801B28E0;
extern u8 D_800DA448[];
extern u8 D_80139CC8[];

    func_8006A2FC(D_800DA448, D_80139CC8, 0xA, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B28E4 == 0)
    {
        D_801B28E0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800868C8(void)
{
extern s32 D_801B28E0;

    D_801B28E0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800868E0(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B28E8;
extern s32 D_801B28EC;
extern void (*D_800D5A10[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80085618__for_func_800868E0(void) __asm__("func_80085618");

    s32 result;

    if (arg0 != 0)
    {
        D_801B28E8 = 1;
        D_801B28EC = 1;
        return 1;
    }

    if (D_801B28E8 < 0x4)
    {
        D_800D5A10[D_801B28E8]();
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
void func_80086958(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B28E8;
extern s32 D_801B28EC;
extern void (*D_800D5A10[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80085618__for_func_80086958(void) __asm__("func_80085618");

    D_801B28E8 = 1;
    D_801B28EC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80086970(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B28E8;
extern s32 D_801B28EC;
extern void (*D_800D5A10[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80085618__for_func_80086970(void) __asm__("func_80085618");

    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B28EC = 0x28;
    D_801B28E8 += 1;
    func_80085618__for_func_80086970();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80086A20(void)
{
extern s32 D_801B28E8;

    D_801B28E8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80086A38(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B28F0;
extern s32 D_801B28F4;
extern void (*D_800D5A20[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_80085718__for_func_80086A38(void) __asm__("func_80085718");

    s32 result;

    if (arg0 != 0)
    {
        D_801B28F0 = 1;
        D_801B28F4 = 1;
        return 1;
    }

    if (D_801B28F0 < 0x4)
    {
        D_800D5A20[D_801B28F0]();
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
void func_80086AB0(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B28F0;
extern s32 D_801B28F4;
extern void (*D_800D5A20[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_80085718__for_func_80086AB0(void) __asm__("func_80085718");

    D_801B28F0 = 1;
    D_801B28F4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80086AC8(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B28F0;
extern s32 D_801B28F4;
extern void (*D_800D5A20[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern void func_80085718__for_func_80086AC8(void) __asm__("func_80085718");

    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B28F4 = 0x40;
    D_801B28F0 += 1;
    func_80085718__for_func_80086AC8();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80086B78(void)
{
extern s32 D_801B28F0;

    D_801B28F0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80086B90(s32 arg0)
{
extern u32 D_801B28F8;
extern s32 D_801B28FC;
extern void (*D_800D5A30[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B28F8 = 1;
        D_801B28FC = 1;
        return 1;
    }

    if (D_801B28F8 < 0x6)
    {
        D_800D5A30[D_801B28F8]();
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
void func_80086C08(void)
{
extern u32 D_801B28F8;
extern s32 D_801B28FC;
extern void (*D_800D5A30[])(void);

    D_801B28F8 = 1;
    D_801B28FC = 1;
}

/** @brief Clear two rotation vectors, set the flag, and start a 128-tick sequence step. */
void func_80086C20(void)
{
extern s32 D_80182DF0;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2498;
extern s32 D_801B28F8;
extern s32 D_801B28FC;
extern void func_80085818__for_func_80086C20(void) __asm__("func_80085818");

    D_80182DF0 = 1;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    D_801B2498.vx = 0;
    D_801B2498.vy = 0;
    D_801B2498.vz = 0;
    D_801B28FC = 0x80;
    D_801B28F8 += 1;
    func_80085818__for_func_80086C20();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80086C8C(void)
{
extern s32 D_801B28F8;
extern void func_80085950__for_func_80086C8C(void) __asm__("func_80085950");
extern s32 D_801B28FC;

    D_801B28FC = 0x40;
    D_801B28F8 += 1;
    func_80085950__for_func_80086C8C();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80086CC4(void)
{
extern s32 D_801B28F8;
extern void func_80085950__for_func_80086CC4(void) __asm__("func_80085950");
extern s32 D_801B28FC;

    D_801B28F8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80086CDC(s32 arg0)
{
extern u32 D_801B2900;
extern s32 D_801B2904;
extern void (*D_800D5A48[])(void);
extern void func_80086DEC__for_func_80086CDC(void) __asm__("func_80086DEC");
extern u8 D_8011F538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2900 = 1;
        D_801B2904 = 1;
        return 1;
    }

    if (D_801B2900 < 0x6)
    {
        D_800D5A48[D_801B2900]();
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
void func_80086D54(void)
{
extern u32 D_801B2900;
extern s32 D_801B2904;
extern void (*D_800D5A48[])(void);
extern void func_80086DEC__for_func_80086D54(void) __asm__("func_80086DEC");
extern u8 D_8011F538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    D_801B2900 = 1;
    D_801B2904 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80086D6C(void)
{
extern u32 D_801B2900;
extern s32 D_801B2904;
extern void (*D_800D5A48[])(void);
extern void func_80086DEC__for_func_80086D6C(void) __asm__("func_80086DEC");
extern u8 D_8011F538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    D_801399CC = D_8011F538;
    D_800D93C8[0x6] = 0xF;
    *(s16*)&D_800D93C8[0xE] = 1;
    *(s16*)&D_800D93C8[0x10] = -1;
    *(s16*)&D_800D93C8[0x26] = 2;
    *(s16*)&D_800D93C8[0x2] = 0;
    *(s16*)&D_800D93C8[0x22] = 0x81;
    *(s16*)&D_800D93C8[0x24] = 0x81;
    D_801B2904 = 0xBF;
    D_801B2900 += 1;
    func_80086DEC__for_func_80086D6C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80086DEC(void)
{
extern u32 D_801B2900;
extern s32 D_801B2904;
extern void (*D_800D5A48[])(void);
extern void func_80086DEC(void);
extern u8 D_8011F538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D93C8, D_801399C8);
    wmap_draw_actor_sprite(D_800D93C8, D_8011CF4C, 0x18, 0x9, 0);
    if (--D_801B2904 == 0)
    {
        D_801B2900 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80086E68(void)
{
extern void func_80086EB4__for_func_80086E68(void) __asm__("func_80086EB4");
extern s16 D_800D93C8[];
extern s32 D_801B2904;
extern s32 D_801B2900;
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    D_800D93C8[19] = 16;
    D_800D93C8[17] = 0;
    D_801B2904 = 0x8;
    D_801B2900 += 1;
    func_80086EB4__for_func_80086E68();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80086EB4(void)
{
extern void func_80086EB4(void);
extern s16 D_800D93C8[];
extern s32 D_801B2904;
extern s32 D_801B2900;
extern u8 D_801399C8[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D93C8, D_801399C8);
    wmap_draw_actor_sprite(D_800D93C8, D_8011CF4C, 0x18, 0x9, 0);
    if (--D_801B2904 == 0)
    {
        D_801B2900 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80086F30(void)
{
extern s32 D_801B2900;

    D_801B2900 += 1;
}
