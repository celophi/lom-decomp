#include "wmap_main.h"
#include "wmap_land_effect_12.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

/**
 * @brief World-map step handler: draw two overlaid actor sprites within a matrix push,
 *        ramp the shared size up to a cap, then countdown-advance the step.
 */
void func_80083D64(void)
{
extern u8 D_80182DC0[];
extern u8 D_801B2490[];
extern u8 D_801B2498[];
extern s32 D_8011CF1C;
extern s32 D_801B2468;
extern s32 D_801B288C;
extern s32 D_801B2888;

    s32 value;

    PushMatrix();
    func_8006CFA8(D_80182DC0, D_801B2490);
    func_8006CD98(D_8011CF1C, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    ((u16*)D_801B2490)[2] += 0xC;
    func_8006CFA8(D_80182DC0, D_801B2498);
    func_8006CD98(D_8011CF1C, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    ((u16*)D_801B2498)[2] += 0x4;
    PopMatrix();
    value = D_801B2468 + 2;
    D_801B2468 = value;
    if (value >= 0x82)
    {
        D_801B2468 = 0x81;
    }
    if (--D_801B288C == 0)
    {
        D_801B2888 += 1;
    }
}

/**
 * @brief World-map step handler: draw two frames of the animated actor, scroll each
 *        sub-field, decay the shared frame index with a floor, then advance the step.
 */
void func_80083EA0(void)
{
extern u8 D_80182DC0[];
extern u8 D_801B2490[];
extern u8 D_801B2498[];
extern s32 D_8011CF1C;
extern s32 D_801B2468;
extern s32 D_801B2888;
extern s32 D_801B288C;

    PushMatrix();
    func_8006CFA8(D_80182DC0, D_801B2490);
    func_8006CD98(D_8011CF1C, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    ((u16*)D_801B2490)[2] += 0xC;
    func_8006CFA8(D_80182DC0, D_801B2498);
    func_8006CD98(D_8011CF1C, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    ((u16*)D_801B2498)[2] += 4;
    PopMatrix();
    D_801B2468 -= 1;
    if (D_801B2468 < 0)
    {
        D_801B2468 = 0;
    }
    if (--D_801B288C == 0)
    {
        D_801B2888 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80083FD4(void)
{
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2894;
extern s32 D_801B2890;

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
    if (--D_801B2894 == 0)
    {
        D_801B2890 += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800840D4(void)
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
extern u8 D_80121538[];
extern s32 D_801B2898;
extern s32 D_801B289C;
extern void func_80084F18__for_func_800840D4(void) __asm__("func_80084F18");

    s32 i;

    D_801B0FD0 = 100;
    D_80139280[0x1] = 2;
    D_80139280[0x2] = 4;
    D_80139280[0x3] = 0x20;
    D_80139280[0x4] = 8;
    D_80139280[0x5] = 4;
    D_80139280[0x6] = 0x3E8;
    D_80139280[0x7] = 0x64;
    D_80139280[0x8] = 8;
    D_80139280[0x9] = 2;
    D_80139280[0xA] = 0xFA0;
    for (i = 0; i < 100; i++)
    {
        D_801AFBD0[i + 100].field_00 = 0;
        D_80139988[i + 100].field_04 = D_80121538;
    }
    D_801B289C = 144;
    D_801B2898++;
    func_80084F18__for_func_800840D4();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_800841A8(void)
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
extern s32 D_801B28A0;
extern s32 D_801B28A4;
extern u8 D_80121538[];
extern void func_80085108__for_func_800841A8(void) __asm__("func_80085108");

    s32 i;

    D_801B0FD0 = 5;
    D_80139280[0xB] = 4;
    D_80139280[0xC] = 2;
    D_80139280[0xD] = 32;
    D_80139280[0xE] = 1;
    D_80139280[0xF] = 2;
    D_80139280[0x10] = 2000;
    D_80139280[0x11] = 20;
    D_80139280[0x12] = 8;
    D_80139280[0x13] = 1;
    D_80139280[0x14] = 1000;
    for (i = 0; i < 5; i++)
    {
        D_801AFBD0[i + 20].field_00 = 0;
        D_80139988[i + 20].field_04 = D_80121538;
    }
    D_801B28A4 = 10;
    D_801B28A0++;
    func_80085108__for_func_800841A8();
}

/** @brief Initialize the actor group and its animation resources, then advance. */
void func_80084284(void)
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
extern s32 D_801B28A8;
extern s32 D_801B28AC;
extern void func_80085300__for_func_80084284(void) __asm__("func_80085300");

    s32 i;
    WmapConfigA* config;

    D_801B0FD0 = 5;
    for (i = 200; i < 205; i++)
    {
        config = &D_800D9268[i];
        D_80139988[i].data = D_80121538;
        config->field_02 = 0;
        config->field_06 = 15;
        config->field_0E = 0;
        config->field_10 = -1;
        config->field_22 = 127;
        config->field_24 = 1;
        config->field_26 = 8;
        D_801AFBD0[i].field_00 = 1;
        D_801AFBD0[i].angle = i * 0x333;
        D_801AFBD0[i].field_08 = 380000;
        D_801AFBD0[i].field_04 = 0;
        D_801AFBD0[i].field_0C = 9999;
        D_801AFBD0[i].field_0E = 0;
        D_801AFBD0[i].field_10 = 80;
    }
    D_801B28AC = 64;
    D_801B28A8++;
    func_80085300__for_func_80084284();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008437C(s32 arg0)
{
extern u32 D_801B2868;
extern s32 D_801B286C;
extern void (*D_800D5888[])(void);
extern void func_80084450__for_func_8008437C(void) __asm__("func_80084450");
extern s32 D_8013B20C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2868 = 1;
        D_801B286C = 1;
        return 1;
    }

    if (D_801B2868 < 0x6)
    {
        D_800D5888[D_801B2868]();
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
void func_800843F4(void)
{
extern u32 D_801B2868;
extern s32 D_801B286C;
extern void (*D_800D5888[])(void);
extern void func_80084450__for_func_800843F4(void) __asm__("func_80084450");
extern s32 D_8013B20C;

    D_801B2868 = 1;
    D_801B286C = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8008440C(void)
{
extern u32 D_801B2868;
extern s32 D_801B286C;
extern void (*D_800D5888[])(void);
extern void func_80084450__for_func_8008440C(void) __asm__("func_80084450");
extern s32 D_8013B20C;

    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2868 += 1;
    func_80084450__for_func_8008440C();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80084450(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2868;
extern void func_8008448C__for_func_80084450(void) __asm__("func_8008448C");
extern void func_80084524__for_func_80084450(void) __asm__("func_80084524");
extern void func_800844D0__for_func_80084450(void) __asm__("func_800844D0");

    if (D_8013B20C == 0)
    {
        D_801B2868 += 1;
        func_8008448C__for_func_80084450();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_8008448C(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2868;
extern void func_8008448C(void);
extern void func_80084524__for_func_8008448C(void) __asm__("func_80084524");
extern void func_800844D0__for_func_8008448C(void) __asm__("func_800844D0");

    func_8006CAC0(func_80084524__for_func_8008448C);
    D_8013B20C = 1;
    D_801B2868 += 1;
    func_800844D0__for_func_8008448C();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800844D0(void)
{
extern s32 D_801B2868;
extern s32 D_8013B20C;
extern void func_8008450C__for_func_800844D0(void) __asm__("func_8008450C");

    if (D_8013B20C == 0)
    {
        D_801B2868 += 1;
        func_8008450C__for_func_800844D0();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008450C(void)
{
extern s32 D_801B2868;
extern s32 D_8013B20C;
extern void func_8008450C(void);

    D_801B2868 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80084524(s32 arg0)
{
extern u32 D_801B2870;
extern s32 D_801B2874;
extern void (*D_800D58A0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2870 = 1;
        D_801B2874 = 1;
        return 1;
    }

    if (D_801B2870 < 0xC)
    {
        D_800D58A0[D_801B2870]();
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
void func_8008459C(void)
{
extern u32 D_801B2870;
extern s32 D_801B2874;
extern void (*D_800D58A0[])(void);

    D_801B2870 = 1;
    D_801B2874 = 1;
}

/** @brief World-map state entry: load resources, register callbacks, advance. */
void func_800845B4(void)
{
extern s32 D_8013B208;
extern s32 D_801B2870;
extern s32 D_801B2874;
extern void func_8008489C__for_func_800845B4(void) __asm__("func_8008489C");
extern void func_80084D30__for_func_800845B4(void) __asm__("func_80084D30");

    D_8013B208 = 1;
    func_8006683C(0x304010);
    g_wmap_backdrop_target_level = 4;
    func_800652A8(0x1F, 0x80);
    func_8006CAC0(func_8008489C__for_func_800845B4);
    func_8006CAC0(func_80084D30__for_func_800845B4);
    D_801B2874 = 8;
    D_801B2870 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008462C(void)
{
extern s32 D_801B2874;
extern s32 D_801B2870;

    if (--D_801B2874 == 0)
    {
        D_801B2870 += 1;
    }
}

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void func_80084660(void)
{
extern s32 D_801ADAE0;
extern s32 D_801B2874;
extern s32 D_801B2870;
extern void func_80084E88__for_func_80084660(void) __asm__("func_80084E88");

    D_801ADAE0 = 1;
    func_8006CAC0(func_80084E88__for_func_80084660);
    D_801B2874 = 0x18;
    D_801B2870 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800846A8(void)
{
extern s32 D_801B2874;
extern s32 D_801B2870;
extern void func_80084BDC__for_func_800846A8(void) __asm__("func_80084BDC");

    if (--D_801B2874 == 0)
    {
        D_801B2870 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800846DC(void)
{
extern s32 D_801B2874;
extern s32 D_801B2870;
extern void func_80084BDC__for_func_800846DC(void) __asm__("func_80084BDC");

    func_8006CAC0(func_80084BDC__for_func_800846DC);
    D_801B2874 = 0x3C;
    D_801B2870 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80084718(void)
{
extern s32 D_801B2874;
extern s32 D_801B2870;
extern void func_80085078__for_func_80084718(void) __asm__("func_80085078");

    if (--D_801B2874 == 0)
    {
        D_801B2870 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8008474C(void)
{
extern s32 D_801B2874;
extern s32 D_801B2870;
extern void func_80085078__for_func_8008474C(void) __asm__("func_80085078");

    func_8006CAC0(func_80085078__for_func_8008474C);
    D_801B2874 = 0x1E;
    D_801B2870 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80084788(void)
{
extern s32 D_801B2874;
extern s32 D_801B2870;
extern void func_80084A3C__for_func_80084788(void) __asm__("func_80084A3C");
extern void func_80085270__for_func_80084788(void) __asm__("func_80085270");

    if (--D_801B2874 == 0)
    {
        D_801B2870 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800847BC(void)
{
extern s32 D_801B2874;
extern s32 D_801B2870;
extern void func_80084A3C__for_func_800847BC(void) __asm__("func_80084A3C");
extern void func_80085270__for_func_800847BC(void) __asm__("func_80085270");

    func_8006CAC0(func_80084A3C__for_func_800847BC);
    func_8006CAC0(func_80085270__for_func_800847BC);
    D_801B2874 = 0x78;
    D_801B2870 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80084804(void)
{
extern s32 D_801B2874;
extern s32 D_801B2870;

    if (--D_801B2874 == 0)
    {
        D_801B2870 += 1;
    }
}

void func_80084838(void)
{
/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

extern s32 D_8013B20C;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2870;

    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B2870 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008489C(s32 arg0)
{
extern u32 D_801B2878;
extern s32 D_801B287C;
extern void (*D_800D58D0[])(void);
extern void func_800849A8__for_func_8008489C(void) __asm__("func_800849A8");
extern u8 D_8011F538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2878 = 1;
        D_801B287C = 1;
        return 1;
    }

    if (D_801B2878 < 0x4)
    {
        D_800D58D0[D_801B2878]();
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
void func_80084914(void)
{
extern u32 D_801B2878;
extern s32 D_801B287C;
extern void (*D_800D58D0[])(void);
extern void func_800849A8__for_func_80084914(void) __asm__("func_800849A8");
extern u8 D_8011F538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    D_801B2878 = 1;
    D_801B287C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8008492C(void)
{
extern u32 D_801B2878;
extern s32 D_801B287C;
extern void (*D_800D58D0[])(void);
extern void func_800849A8__for_func_8008492C(void) __asm__("func_800849A8");
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
    D_801B287C = 0x7E;
    D_801B2878 += 1;
    func_800849A8__for_func_8008492C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800849A8(void)
{
extern u32 D_801B2878;
extern s32 D_801B287C;
extern void (*D_800D58D0[])(void);
extern void func_800849A8(void);
extern u8 D_8011F538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_8011CF4C, 0x19, 0xE, 0);
    if (--D_801B287C == 0)
    {
        D_801B2878 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80084A24(void)
{
extern s32 D_801B2878;

    D_801B2878 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80084A3C(s32 arg0)
{
extern u32 D_801B2880;
extern s32 D_801B2884;
extern void (*D_800D58E0[])(void);
extern void func_80084B48__for_func_80084A3C(void) __asm__("func_80084B48");
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2880 = 1;
        D_801B2884 = 1;
        return 1;
    }

    if (D_801B2880 < 0x4)
    {
        D_800D58E0[D_801B2880]();
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
void func_80084AB4(void)
{
extern u32 D_801B2880;
extern s32 D_801B2884;
extern void (*D_800D58E0[])(void);
extern void func_80084B48__for_func_80084AB4(void) __asm__("func_80084B48");
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801B2880 = 1;
    D_801B2884 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80084ACC(void)
{
extern u32 D_801B2880;
extern s32 D_801B2884;
extern void (*D_800D58E0[])(void);
extern void func_80084B48__for_func_80084ACC(void) __asm__("func_80084B48");
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
    *(s16*)&D_800D9344[0x24] = 0x80;
    D_801B2884 = 0x79;
    D_801B2880 += 1;
    func_80084B48__for_func_80084ACC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80084B48(void)
{
extern u32 D_801B2880;
extern s32 D_801B2884;
extern void (*D_800D58E0[])(void);
extern void func_80084B48(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x1A, 0xF, 0);
    if (--D_801B2884 == 0)
    {
        D_801B2880 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80084BC4(void)
{
extern s32 D_801B2880;

    D_801B2880 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80084BDC(s32 arg0)
{
extern u32 D_801B2888;
extern s32 D_801B288C;
extern void (*D_800D58F0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2888 = 1;
        D_801B288C = 1;
        return 1;
    }

    if (D_801B2888 < 0x6)
    {
        D_800D58F0[D_801B2888]();
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
void func_80084C54(void)
{
extern u32 D_801B2888;
extern s32 D_801B288C;
extern void (*D_800D58F0[])(void);

    D_801B2888 = 1;
    D_801B288C = 1;
}

/** @brief World-map step: reset counters and advance to the next handler. */
void func_80084C6C(void)
{
extern s32 D_801B24B0;
extern s32 D_801B2468;
extern s16 D_801B2490[];
extern s16 D_801B2498[];
extern s32 D_801B2888;
extern s32 D_801B288C;
extern void func_80083D64__for_func_80084C6C(void) __asm__("func_80083D64");

    D_801B24B0 = 0;
    D_801B2468 = 1;
    D_801B2490[0] = 0;
    D_801B2490[1] = 0;
    D_801B2490[2] = 0;
    D_801B2498[0] = 0;
    D_801B2498[1] = 0;
    D_801B2498[2] = 0;
    D_801B288C = 0x40;
    D_801B2888 += 1;
    func_80083D64__for_func_80084C6C();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80084CE0(void)
{
extern s32 D_801B2888;
extern void func_80083EA0__for_func_80084CE0(void) __asm__("func_80083EA0");
extern s32 D_801B288C;

    D_801B288C = 0x80;
    D_801B2888 += 1;
    func_80083EA0__for_func_80084CE0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80084D18(void)
{
extern s32 D_801B2888;
extern void func_80083EA0__for_func_80084D18(void) __asm__("func_80083EA0");
extern s32 D_801B288C;

    D_801B2888 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80084D30(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2890;
extern s32 D_801B2894;
extern void (*D_800D5908[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80083FD4__for_func_80084D30(void) __asm__("func_80083FD4");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2890 = 1;
        D_801B2894 = 1;
        return 1;
    }

    if (D_801B2890 < 0x4)
    {
        D_800D5908[D_801B2890]();
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
void func_80084DA8(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2890;
extern s32 D_801B2894;
extern void (*D_800D5908[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80083FD4__for_func_80084DA8(void) __asm__("func_80083FD4");

    D_801B2890 = 1;
    D_801B2894 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80084DC0(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2890;
extern s32 D_801B2894;
extern void (*D_800D5908[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80083FD4__for_func_80084DC0(void) __asm__("func_80083FD4");

    D_801B24A0 = D_80139258;
    D_801B2650 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B2894 = 0x20;
    D_801B2890 += 1;
    func_80083FD4__for_func_80084DC0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80084E70(void)
{
extern s32 D_801B2890;

    D_801B2890 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80084E88(s32 arg0)
{
extern u32 D_801B2898;
extern s32 D_801B289C;
extern void (*D_800D5918[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2898 = 1;
        D_801B289C = 1;
        return 1;
    }

    if (D_801B2898 < 0x6)
    {
        D_800D5918[D_801B2898]();
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
void func_80084F00(void)
{
extern u32 D_801B2898;
extern s32 D_801B289C;
extern void (*D_800D5918[])(void);

    D_801B2898 = 1;
    D_801B289C = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80084F18(void)
{
extern u8 D_800DA398[];
extern u8 D_80139CA8[];
extern s32 D_80139280;
extern s32 D_801B2898;
extern s32 D_801B289C;

    func_8006A2FC(D_800DA398, D_80139CA8, 0x64, 0, 0x7F, 0x4, 0, D_80139280);
    if (--D_801B289C == 0)
    {
        D_801B2898 += 1;
    }
}

void func_80084F98(void)
{
typedef struct { unsigned char pad[0x14]; s32 unk14; } WmapObj;

extern void func_80084FE0__for_func_80084F98(void) __asm__("func_80084FE0");
extern WmapObj* D_80139280;
extern s32 D_801B2898;
extern s32 D_801B289C;

    D_801B289C = 0x20;
    D_80139280->unk14 = -1;
    D_801B2898 += 1;
    func_80084FE0__for_func_80084F98();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80084FE0(void)
{
extern u8 D_800DA398[];
extern u8 D_80139CA8[];
extern s32 D_80139280;
extern s32 D_801B2898;
extern s32 D_801B289C;

    func_8006A2FC(D_800DA398, D_80139CA8, 0x64, 0, 0x7F, 0x4, 0, D_80139280);
    if (--D_801B289C == 0)
    {
        D_801B2898 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80085060(void)
{
extern s32 D_801B2898;

    D_801B2898 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80085078(s32 arg0)
{
extern u32 D_801B28A0;
extern s32 D_801B28A4;
extern void (*D_800D5930[])(void);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_80139280;

    s32 result;

    if (arg0 != 0)
    {
        D_801B28A0 = 1;
        D_801B28A4 = 1;
        return 1;
    }

    if (D_801B28A0 < 0x6)
    {
        D_800D5930[D_801B28A0]();
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
void func_800850F0(void)
{
extern u32 D_801B28A0;
extern s32 D_801B28A4;
extern void (*D_800D5930[])(void);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_80139280;

    D_801B28A0 = 1;
    D_801B28A4 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80085108(void)
{
extern u32 D_801B28A0;
extern s32 D_801B28A4;
extern void (*D_800D5930[])(void);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_80139280;

    func_8006A2FC(D_800D95D8, D_80139A28, 0x5, 0, 0x7F, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B28A4 == 0)
    {
        D_801B28A0 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8008518C(void)
{
extern void func_800851D4__for_func_8008518C(void) __asm__("func_800851D4");
extern s32* D_80139280;
extern s32 D_801B28A4;
extern s32 D_801B28A0;
extern u8 D_800D95D8[];
extern u8 D_80139A28[];

    D_801B28A4 = 0x20;
    D_80139280[15] = -1;
    D_801B28A0 += 1;
    func_800851D4__for_func_8008518C();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800851D4(void)
{
extern void func_800851D4(void);
extern s32* D_80139280;
extern s32 D_801B28A4;
extern s32 D_801B28A0;
extern u8 D_800D95D8[];
extern u8 D_80139A28[];

    func_8006A2FC(D_800D95D8, D_80139A28, 0x5, 0, 0x7F, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B28A4 == 0)
    {
        D_801B28A0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80085258(void)
{
extern s32 D_801B28A0;

    D_801B28A0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80085270(s32 arg0)
{
extern u32 D_801B28A8;
extern s32 D_801B28AC;
extern void (*D_800D5948[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B28A8 = 1;
        D_801B28AC = 1;
        return 1;
    }

    if (D_801B28A8 < 0x6)
    {
        D_800D5948[D_801B28A8]();
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
void func_800852E8(void)
{
extern u32 D_801B28A8;
extern s32 D_801B28AC;
extern void (*D_800D5948[])(void);

    D_801B28A8 = 1;
    D_801B28AC = 1;
}

/**
 * @brief Draw the world-map cursor via func_8006B6EC, then advance after the wait expires.
 */
void func_80085300(void)
{
extern s32 D_801B28A8;
extern s32 D_801B28AC;

    func_8006B6EC(0xC8, 0xCD, 0x8, 0, 0xE);
    if (--D_801B28AC == 0)
    {
        D_801B28A8 += 1;
    }
}

/**
 * @brief Reset a range of world-map actor configs, arm the timer, and advance the step.
 */
void func_80085360(void)
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
extern s32 D_801B28A8;
extern s32 D_801B28AC;
extern void func_800853C4__for_func_80085360(void) __asm__("func_800853C4");

    s32 i;

    for (i = 0xC8; i < 0xCD; i++)
    {
        D_800D9268[i].field_22 = 0;
        D_800D9268[i].field_26 = 8;
    }
    D_801B28AC = 0x10;
    D_801B28A8 += 1;
    func_800853C4__for_func_80085360();
}

/**
 * @brief Draw the world-map cursor via func_8006B6EC, then advance after the wait expires.
 */
void func_800853C4(void)
{
extern s32 D_801B28A8;
extern s32 D_801B28AC;

    func_8006B6EC(0xC8, 0xCD, 0x8, 0, 0xE);
    if (--D_801B28AC == 0)
    {
        D_801B28A8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80085424(void)
{
extern s32 D_801B28A8;

    D_801B28A8 += 1;
}
