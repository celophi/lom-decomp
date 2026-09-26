#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_land_effect_23.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8009D620(void)
{
extern s32 D_801B2CC8;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2CEC;
extern s32 D_801B2CE8;

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
    if (--D_801B2CEC == 0)
    {
        D_801B2CE8 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8009D720(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B2CF4;
extern s32 D_801B2CF0;

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
    if (--D_801B2CF4 == 0)
    {
        D_801B2CF0 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_8009D820(void)
{
extern s32 D_80139870[];
extern SVECTOR D_8013B238;
extern VECTOR D_8011CF60;
extern s32 D_80182DF0;
extern s32 D_8011CF1C;
extern s32 D_801B2CFC;
extern s32 D_801B2CF8;

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

    if (D_80182DF0 != 0)
    {
        func_8006CD98(D_8011CF1C, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DF0);
        D_80182DF0 -= 0x2;
        if (D_80182DF0 < 0)
        {
            D_80182DF0 = 0;
        }
    }

    PopMatrix();
    if (--D_801B2CFC == 0)
    {
        D_801B2CF8 += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_8009D920(void)
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
extern u8 D_8011F538[];
extern s32 D_801B2D00;
extern s32 D_801B2D04;
extern void func_8009F2B0__for_func_8009D920(void) __asm__("func_8009F2B0");

    s32 i;

    D_80139280[0x1] = 1;
    D_80139280[0x2] = 0;
    D_80139280[0x3] = 0x20;
    D_80139280[0x4] = 0;
    D_80139280[0x5] = 4;
    D_80139280[0x6] = 0x320;
    D_80139280[0x7] = 0xB4;
    D_80139280[0x8] = 8;
    D_80139280[0x9] = 1;
    D_80139280[0xA] = 0x124F8;
    for (i = 0; i < 20; i++)
    {
        D_801AFBD0[i + 180].field_00 = 0;
        D_80139988[i + 180].field_04 = D_8011F538;
    }
    D_801B2D04 = 80;
    D_801B2D00++;
    func_8009F2B0__for_func_8009D920();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_8009D9F0(void)
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
extern u8 D_8011F538[];
extern s32 D_801B2D08;
extern s32 D_801B2D0C;
extern void func_8009F4B0__for_func_8009D9F0(void) __asm__("func_8009F4B0");

    s32 i;

    D_80139280[0xB] = 0;
    D_80139280[0xC] = 0;
    D_80139280[0xD] = 0x40;
    D_80139280[0xE] = 0;
    D_80139280[0xF] = 5;
    D_80139280[0x10] = 0x258;
    D_80139280[0x11] = 0x1E;
    D_80139280[0x12] = 8;
    D_80139280[0x13] = 2;
    D_80139280[0x14] = 0x2710;
    for (i = 0; i < 90; i++)
    {
        D_801AFBD0[i + 30].field_00 = 0;
        D_80139988[i + 30].field_04 = D_8011F538;
    }
    D_801B2D0C = 450;
    D_801B2D08++;
    func_8009F4B0__for_func_8009D9F0();
}

/**
 * @brief Draw the first animated element, ramp its intensity up, rotate it, and advance after its timer expires.
 */
void func_8009DABC(void)
{

    extern VECTOR g_wmap_camera_translation;
    extern SVECTOR D_8013B238;
    extern s32* D_8011CF2C;
    extern s32 D_80139234;
    extern s32 D_801B25D8;
    extern s32 D_801B2D10;
    extern s32 D_801B2D14;

    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF2C, D_80139234 & 3, 0xA, 0x36, 0x7900, 0x1001, D_801B25D8, 0, 0xF, -1);
    value = D_801B25D8 + 2;
    D_801B25D8 = value;
    if (value >= 0x82)
    {
        D_801B25D8 = 0x81;
    }
    timer = D_801B2D14;
    D_8013B238.vz += 0x14;
    next_timer = timer - 1;
    D_801B2D14 = next_timer;
    D_80139234 += 1;
    if (next_timer == 0)
    {
        D_801B2D10 += 1;
    }
}

/**
 * @brief Draw an animated element, ramp its intensity down, rotate it, and advance after its timer expires.
 */
void func_8009DBB0(void)
{

    extern VECTOR g_wmap_camera_translation;
    extern SVECTOR D_8013B238;
    extern s32* D_8011CF2C;
    extern s32 D_80139234;
    extern s32 D_801B25D8;
    extern s32 D_801B2D10;
    extern s32 D_801B2D14;

    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF2C, D_80139234 & 3, 0xA, 0x36, 0x7900, 0x1001, D_801B25D8, 0, 0xF, -1);
    value = D_801B25D8 - 4;
    D_801B25D8 = value;
    if (value < 0)
    {
        D_801B25D8 = 0;
    }
    timer = D_801B2D14;
    D_8013B238.vz += 0x14;
    next_timer = timer - 1;
    D_801B2D14 = next_timer;
    D_80139234 += 1;
    if (next_timer == 0)
    {
        D_801B2D10 += 1;
    }
}

/**
 * @brief Draw the second animated element, ramp its intensity up, rotate it, and advance after its timer expires.
 */
void func_8009DC9C(void)
{

    extern VECTOR g_wmap_camera_translation;
    extern SVECTOR D_801B24A8;
    extern s32* D_8011CF28;
    extern s32 D_8013923C;
    extern s32 D_80182DEC;
    extern s32 D_801B2D18;
    extern s32 D_801B2D1C;

    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(&g_wmap_camera_translation, &D_801B24A8);
    wmap_draw_model(D_8011CF28, D_8013923C & 3, 0x4, 0x35, 0x7800, 0x1001, D_80182DEC, 0, 0xA, -1);
    value = D_80182DEC + 8;
    D_80182DEC = value;
    if (value >= 0x82)
    {
        D_80182DEC = 0x81;
    }
    timer = D_801B2D1C;
    D_801B24A8.vz += 0x38;
    next_timer = timer - 1;
    D_801B2D1C = next_timer;
    D_8013923C += 1;
    if (next_timer == 0)
    {
        D_801B2D18 += 1;
    }
}

/**
 * @brief Draw the second animated element, ramp its intensity down, rotate it, and advance after its timer expires.
 */
void func_8009DD90(void)
{

    extern VECTOR g_wmap_camera_translation;
    extern SVECTOR D_801B24A8;
    extern s32* D_8011CF28;
    extern s32 D_8013923C;
    extern s32 D_80182DEC;
    extern s32 D_801B2D18;
    extern s32 D_801B2D1C;

    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(&g_wmap_camera_translation, &D_801B24A8);
    wmap_draw_model(D_8011CF28, D_8013923C & 3, 0x4, 0x35, 0x7800, 0x1001, D_80182DEC, 0, 0xA, -1);
    value = D_80182DEC - 8;
    D_80182DEC = value;
    if (value < 0)
    {
        D_80182DEC = 0;
    }
    timer = D_801B2D1C;
    D_801B24A8.vz += 0x38;
    next_timer = timer - 1;
    D_801B2D1C = next_timer;
    D_8013923C += 1;
    if (next_timer == 0)
    {
        D_801B2D18 += 1;
    }
}

/** @brief Configure the effect and reset its forty-eight resource slots. */
void func_8009DE7C(void)
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

extern WmapConfig *D_80139280;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern u8 D_8011F538[];
extern s32 D_801B2D28;
extern s32 D_801B2D2C;
extern void func_8009FC60__for_func_8009DE7C(void) __asm__("func_8009FC60");

    s32 i;

    D_80139280[1].field_04 = 1;
    D_80139280[1].field_08 = 4;
    D_80139280[1].field_0C = 0x20;
    D_80139280[1].field_10 = 0;
    D_80139280[1].field_14 = 4;
    D_80139280[1].field_18 = 1;
    D_80139280[1].field_1C = 0x82;
    D_80139280[1].field_20 = 8;
    D_80139280[1].field_24 = 0;
    D_80139280[1].field_28 = 0x4650;
    for (i = 0; i < 48; i++)
    {
        D_801AFBD0[i + 130].field_00 = 0;
        D_80139988[i + 130].field_04 = D_8011F538;
    }
    D_801B2D2C = 0xC0;
    D_801B2D28++;
    func_8009FC60__for_func_8009DE7C();
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void func_8009DF44(void)
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
extern s32 D_80121538[];
extern WmapPair D_80139988[];
extern WmapAfcEntry D_801AFBD0[];
extern s32 D_800D9154;
extern s32 D_800DCEAC;
extern s32 D_80182DF4;
extern s32 D_801B2D40;
extern s32 D_801B2D44;

extern void func_800A0444__for_func_8009DF44(void) __asm__("func_800A0444");

    s32 i;
    WmapD94Entry *entry;

    i = 0xB4;
    D_80182DF4 = 1;
    D_800DCEAC = 8;

    do
    {
        D_801AFBD0[i].unk0 = 0;
        D_80139988[i].unk4 = D_80121538;
        entry = &D_800D9268[i];
        entry->unk2 = 0;
        entry->unk6 = 0xF;
        entry->unkE = 0;
        entry->unk10 = -1;
        i++;
    } while (i < 0xF0);

    D_800D9154 = 1;
    D_801B2D44 = 0x10;
    D_801B2D40 += 1;
    func_800A0444__for_func_8009DF44();
}

/**
 * @brief World-map step handler: jitter the actor position with a ramping random
 *        amplitude, then countdown-advance the step.
 */
void func_8009E008(void)
{
extern int rand(void);
extern s16 D_801398C8[];
extern s32 D_80182D48[];
extern s32 D_80139264;
extern s32 D_801B2D4C;
extern s32 D_801B2D48;

    D_801398C8[0] = rand() * D_80139264 / 16 >> 15;
    D_801398C8[1] = rand() * D_80139264 / 16 >> 15;
    D_80182D48[0] = rand() * D_80139264 / 16 >> 15;
    D_80182D48[1] = rand() * D_80139264 / 16 >> 15;
    if (D_80139264 < 0x80)
    {
        D_80139264 += 4;
    }
    if (--D_801B2D4C == 0)
    {
        D_801B2D48 += 1;
    }
}

/**
 * @brief World-map step handler: jitter the actor with a random amplitude, and when
 *        the amplitude ramp expires, zero the offsets; otherwise countdown-advance.
 */
void func_8009E114(void)
{
extern int rand(void);
extern s16 D_801398C8[];
extern s32 D_80182D48[];
extern s32 D_80139264;
extern s32 D_801B2D48;
extern s32 D_801B2D4C;

    D_801398C8[0] = rand() * D_80139264 / 16 >> 15;
    D_801398C8[1] = rand() * D_80139264 / 16 >> 15;
    D_80182D48[0] = rand() * D_80139264 / 16 >> 15;
    D_80182D48[1] = rand() * D_80139264 / 16 >> 15;
    if (--D_80139264 < 0)
    {
        D_80182D48[1] = 0;
        D_80182D48[0] = 0;
        D_801398C8[1] = 0;
        D_801398C8[0] = 0;
        D_801B2D48 += 1;
    }
    else if (--D_801B2D4C == 0)
    {
        D_801B2D48 += 1;
    }
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009E250(s32 arg0)
{
extern u32 D_801B2CD0;
extern s32 D_801B2CD4;
extern void (*D_800D672C[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2CD0 = 1;
        D_801B2CD4 = 1;
        return 1;
    }

    if (D_801B2CD0 < 0x4)
    {
        D_800D672C[D_801B2CD0]();
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
void func_8009E2C8(void)
{
extern u32 D_801B2CD0;
extern s32 D_801B2CD4;
extern void (*D_800D672C[])(void);

    D_801B2CD0 = 1;
    D_801B2CD4 = 1;
}

/** @brief World-map step: arm a timed callback, flag it active, then tick the sub-counter. */
void func_8009E2E0(void)
{
extern s32 D_80139978;
extern s16 D_8011CF4C[];
extern void func_8009E3AC__for_func_8009E2E0(void) __asm__("func_8009E3AC");
extern s32 D_8013B20C;
extern s32 D_801B2CD0;
extern void func_8009E348__for_func_8009E2E0(void) __asm__("func_8009E348");

    D_80139978 = 0x10;
    D_8011CF4C[0] = 0xA4;
    D_8011CF4C[1] = 0x69;
    func_8006CAC0(func_8009E3AC__for_func_8009E2E0);
    D_8013B20C = 1;
    D_801B2CD0 += 1;
    func_8009E348__for_func_8009E2E0();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8009E348(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2CD0;
extern void func_8009E384__for_func_8009E348(void) __asm__("func_8009E384");

    if (D_8013B20C == 0)
    {
        D_801B2CD0 += 1;
        func_8009E384__for_func_8009E348();
    }
}

/** @brief World-map trigger: set two flags and bump a counter. */
void func_8009E384(void)
{
extern s32 D_8013B294;
extern s32 D_80139228;
extern s32 D_801B2CD0;

    D_8013B294 = 1;
    D_80139228 = 1;
    D_801B2CD0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009E3AC(s32 arg0)
{
extern u32 D_801B2CD8;
extern s32 D_801B2CDC;
extern void (*D_800D673C[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2CD8 = 1;
        D_801B2CDC = 1;
        return 1;
    }

    if (D_801B2CD8 < 0x24)
    {
        D_800D673C[D_801B2CD8]();
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
void func_8009E424(void)
{
extern u32 D_801B2CD8;
extern s32 D_801B2CDC;
extern void (*D_800D673C[])(void);

    D_801B2CD8 = 1;
    D_801B2CDC = 1;
}

/** @brief Set effect flags and color, play sound 41, and begin a 24-tick delay. */
void func_8009E43C(void)
{
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern s32 D_801B2CD8;
extern s32 D_801B2CDC;

    D_8013B208 = 1;
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 8;
    D_801ADAE0 = 1;
    wmap_play_sound(0x29, 0x80);
    D_801B2CDC = 0x18;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E4AC(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_800A06FC__for_func_8009E4AC(void) __asm__("func_800A06FC");

    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009E4E0(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_800A06FC__for_func_8009E4E0(void) __asm__("func_800A06FC");

    func_8006CAC0(func_800A06FC__for_func_8009E4E0);
    D_801B2CDC = 0x18;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E51C(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009F220__for_func_8009E51C(void) __asm__("func_8009F220");

    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009E550(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009F220__for_func_8009E550(void) __asm__("func_8009F220");

    func_8006CAC0(func_8009F220__for_func_8009E550);
    D_801B2CDC = 0x65;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E58C(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009FBD0__for_func_8009E58C(void) __asm__("func_8009FBD0");
extern void func_8009F420__for_func_8009E58C(void) __asm__("func_8009F420");
extern void func_8009EE1C__for_func_8009E58C(void) __asm__("func_8009EE1C");

    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_8009E5C0(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009FBD0__for_func_8009E5C0(void) __asm__("func_8009FBD0");
extern void func_8009F420__for_func_8009E5C0(void) __asm__("func_8009F420");
extern void func_8009EE1C__for_func_8009E5C0(void) __asm__("func_8009EE1C");

    func_8006CAC0(func_8009FBD0__for_func_8009E5C0);
    func_8006CAC0(func_8009F420__for_func_8009E5C0);
    func_8006CAC0(func_8009EE1C__for_func_8009E5C0);
    D_801B2CDC = 0x2;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E614(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;

    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/** @brief World-map step handler: register a callback and advance the step. */
void func_8009E648(void)
{
extern s32 D_801B2CD8;
extern s32 D_801B2CDC;
extern void func_8009EC7C__for_func_8009E648(void) __asm__("func_8009EC7C");

    func_8006CAC0(func_8009EC7C__for_func_8009E648);
    D_801B2CDC = 1;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E684(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;

    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/** @brief World-map step handler: seed timers and advance the counter. */
void func_8009E6B8(void)
{
extern s32 D_800DBE70;
extern s32 D_80139978;
extern s32 D_801B2CD8;
extern s32 D_801B2CDC;

    D_800DBE70 = 0;
    D_80139978 = -1;
    D_801B2CDC = 0x2D;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E6EC(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009F628__for_func_8009E6EC(void) __asm__("func_8009F628");

    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009E720(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009F628__for_func_8009E720(void) __asm__("func_8009F628");

    func_8006CAC0(func_8009F628__for_func_8009E720);
    D_801B2CDC = 0x1C;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E75C(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009EF74__for_func_8009E75C(void) __asm__("func_8009EF74");

    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009E790(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009EF74__for_func_8009E790(void) __asm__("func_8009EF74");

    func_8006CAC0(func_8009EF74__for_func_8009E790);
    D_801B2CDC = 0x2;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E7CC(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;

    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/** @brief Set world-map flags and color, then begin a 48-tick delay. */
void func_8009E800(void)
{
extern s32 D_80139244;
extern s32 D_801B2CD8;
extern s32 D_801B2CDC;

    D_80139244 = 1;
    g_wmap_backdrop_target_level = 1;
    wmap_start_map_tint(0x201010);
    D_801B2CDC = 0x30;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E850(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009FDD8__for_func_8009E850(void) __asm__("func_8009FDD8");

    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009E884(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009FDD8__for_func_8009E884(void) __asm__("func_8009FDD8");

    func_8006CAC0(func_8009FDD8__for_func_8009E884);
    D_801B2CDC = 0x50;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E8C0(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;

    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/** @brief World-map step handler: register a callback, kick a job, advance the step. */
void func_8009E8F4(void)
{
extern s32 D_801B2CD8;
extern s32 D_801B2CDC;
extern void func_800A03B4__for_func_8009E8F4(void) __asm__("func_800A03B4");

    func_8006CAC0(func_800A03B4__for_func_8009E8F4);
    wmap_start_map_tint(0x302050);
    g_wmap_backdrop_target_level = 3;
    D_801B2CDC = 8;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E948(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;

    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/** @brief World-map step: set fade colour then advance to the next handler. */
void func_8009E97C(void)
{
extern s32 D_801B2CD8;
extern s32 D_801B2CDC;

    wmap_start_map_tint(0x252035);
    D_801B2CDC = 0x38;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009E9B8(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_800A00C4__for_func_8009E9B8(void) __asm__("func_800A00C4");

    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009E9EC(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_800A00C4__for_func_8009E9EC(void) __asm__("func_800A00C4");

    func_8006CAC0(func_800A00C4__for_func_8009E9EC);
    D_801B2CDC = 0x22;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009EA28(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009F8E0__for_func_8009EA28(void) __asm__("func_8009F8E0");
extern void func_8009F784__for_func_8009EA28(void) __asm__("func_8009F784");

    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8009EA5C(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009F8E0__for_func_8009EA5C(void) __asm__("func_8009F8E0");
extern void func_8009F784__for_func_8009EA5C(void) __asm__("func_8009F784");

    func_8006CAC0(func_8009F8E0__for_func_8009EA5C);
    func_8006CAC0(func_8009F784__for_func_8009EA5C);
    D_801B2CDC = 0x48;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009EAA4(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009F0C8__for_func_8009EAA4(void) __asm__("func_8009F0C8");

    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009EAD8(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;
extern void func_8009F0C8__for_func_8009EAD8(void) __asm__("func_8009F0C8");

    func_8006CAC0(func_8009F0C8__for_func_8009EAD8);
    D_801B2CDC = 0x2;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009EB14(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;

    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/** @brief World-map step: kick a sub-request, set the next state, and arm the timer. */
void func_8009EB48(void)
{
extern s32 D_80139244;
extern s32 D_801B2CD8;
extern s32 D_801B2CDC;

    D_80139244 = 0;
    wmap_start_map_tint(0x602050);
    g_wmap_backdrop_target_level = 7;
    D_801B2CDC = 0x75;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009EB98(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;

    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/** @brief Set the selected record value and advance to a 50-tick delay. */
void func_8009EBCC(void)
{
extern s32 D_8011D510;
extern s32 D_8011D530;
/** @brief Record with a leading value and a 40-byte stride. */
typedef struct
{
    s32 value;
    u8 unknown_4[36];
} WmapValueRecord;

extern WmapValueRecord D_80139290[][6];
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;

    D_801B2CDC = 50;
    D_80139290[D_8011D510][D_8011D530].value = 0x117;
    D_801B2CD8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009EC2C(void)
{
extern s32 D_801B2CDC;
extern s32 D_801B2CD8;

    if (--D_801B2CDC == 0)
    {
        D_801B2CD8 += 1;
    }
}

/**
 * @brief World-map step handler: clear the shared flag and advance the step counter.
 */
void func_8009EC60(void)
{
extern s32 D_801B2CD8;
extern s32 D_8013B20C;

    D_8013B20C = 0;
    D_801B2CD8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009EC7C(s32 arg0)
{
extern u32 D_801B2CE0;
extern s32 D_801B2CE4;
extern void (*D_800D67CC[])(void);
extern void func_8009ED88__for_func_8009EC7C(void) __asm__("func_8009ED88");
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2CE0 = 1;
        D_801B2CE4 = 1;
        return 1;
    }

    if (D_801B2CE0 < 0x4)
    {
        D_800D67CC[D_801B2CE0]();
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
void func_8009ECF4(void)
{
extern u32 D_801B2CE0;
extern s32 D_801B2CE4;
extern void (*D_800D67CC[])(void);
extern void func_8009ED88__for_func_8009ECF4(void) __asm__("func_8009ED88");
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801B2CE0 = 1;
    D_801B2CE4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8009ED0C(void)
{
extern u32 D_801B2CE0;
extern s32 D_801B2CE4;
extern void (*D_800D67CC[])(void);
extern void func_8009ED88__for_func_8009ED0C(void) __asm__("func_8009ED88");
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
    *(s16*)&D_800D9344[0x22] = 0x81;
    *(s16*)&D_800D9344[0x24] = 0x81;
    D_801B2CE4 = 0x206;
    D_801B2CE0 += 1;
    func_8009ED88__for_func_8009ED0C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009ED88(void)
{
extern u32 D_801B2CE0;
extern s32 D_801B2CE4;
extern void (*D_800D67CC[])(void);
extern void func_8009ED88(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D9344, D_801399B0);
    wmap_draw_actor_sprite(D_800D9344, D_8011CF4C, 0x17, 0x8, 0);
    if (--D_801B2CE4 == 0)
    {
        D_801B2CE0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009EE04(void)
{
extern s32 D_801B2CE0;

    D_801B2CE0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009EE1C(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2CE8;
extern s32 D_801B2CEC;
extern void (*D_800D67DC[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_8009D620__for_func_8009EE1C(void) __asm__("func_8009D620");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2CE8 = 1;
        D_801B2CEC = 1;
        return 1;
    }

    if (D_801B2CE8 < 0x4)
    {
        D_800D67DC[D_801B2CE8]();
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
void func_8009EE94(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2CE8;
extern s32 D_801B2CEC;
extern void (*D_800D67DC[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_8009D620__for_func_8009EE94(void) __asm__("func_8009D620");

    D_801B2CE8 = 1;
    D_801B2CEC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8009EEAC(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2CE8;
extern s32 D_801B2CEC;
extern void (*D_800D67DC[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_8009D620__for_func_8009EEAC(void) __asm__("func_8009D620");

    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B2CEC = 0x20;
    D_801B2CE8 += 1;
    func_8009D620__for_func_8009EEAC();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009EF5C(void)
{
extern s32 D_801B2CE8;

    D_801B2CE8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009EF74(s32 arg0)
{
extern u32 D_801B2CF0;
extern s32 D_801B2CF4;
extern void (*D_800D67EC[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2CF0 = 1;
        D_801B2CF4 = 1;
        return 1;
    }

    if (D_801B2CF0 < 0x4)
    {
        D_800D67EC[D_801B2CF0]();
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
void func_8009EFEC(void)
{
extern u32 D_801B2CF0;
extern s32 D_801B2CF4;
extern void (*D_800D67EC[])(void);

    D_801B2CF0 = 1;
    D_801B2CF4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8009F004(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern s32 D_801B2CF4;
extern u32 D_801B2CF0;
extern void func_8009D720__for_func_8009F004(void) __asm__("func_8009D720");

    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B2CF4 = 0x80;
    D_801B2CF0 += 1;
    func_8009D720__for_func_8009F004();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009F0B0(void)
{
extern s32 D_801B2CF0;

    D_801B2CF0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009F0C8(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2CF8;
extern s32 D_801B2CFC;
extern void (*D_800D67FC[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_80139870;
extern s32 D_80182DF0;
extern void func_8009D820__for_func_8009F0C8(void) __asm__("func_8009D820");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2CF8 = 1;
        D_801B2CFC = 1;
        return 1;
    }

    if (D_801B2CF8 < 0x4)
    {
        D_800D67FC[D_801B2CF8]();
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
void func_8009F140(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2CF8;
extern s32 D_801B2CFC;
extern void (*D_800D67FC[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_80139870;
extern s32 D_80182DF0;
extern void func_8009D820__for_func_8009F140(void) __asm__("func_8009D820");

    D_801B2CF8 = 1;
    D_801B2CFC = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_8009F158(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2CF8;
extern s32 D_801B2CFC;
extern void (*D_800D67FC[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_80139870;
extern s32 D_80182DF0;
extern void func_8009D820__for_func_8009F158(void) __asm__("func_8009D820");

    D_8013B238 = D_80139258;
    D_80139870 = g_wmap_camera_translation;
    D_80182DF0 = 0x80;
    D_80139870.w[2] = 0xAFC8;
    D_801B2CFC = 0x40;
    D_801B2CF8 += 1;
    func_8009D820__for_func_8009F158();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009F208(void)
{
extern s32 D_801B2CF8;

    D_801B2CF8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009F220(s32 arg0)
{
extern u32 D_801B2D00;
extern s32 D_801B2D04;
extern void (*D_800D680C[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2D00 = 1;
        D_801B2D04 = 1;
        return 1;
    }

    if (D_801B2D00 < 0x6)
    {
        D_800D680C[D_801B2D00]();
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
void func_8009F298(void)
{
extern u32 D_801B2D00;
extern s32 D_801B2D04;
extern void (*D_800D680C[])(void);

    D_801B2D00 = 1;
    D_801B2D04 = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009F2B0(void)
{
extern u8 D_800DB158[];
extern u8 D_80139F28[];
extern s32* D_80139280;
extern s32 D_801B2D04;
extern s32 D_801B2D00;

    func_8006AEE0();
    func_8006A2FC(D_800DB158, D_80139F28, 0x14, 0xFF, 0x1, 0x8, 0, (s32)D_80139280);
    if (--D_801B2D04 == 0)
    {
        D_801B2D00 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009F338(void)
{
extern void func_8009F380__for_func_8009F338(void) __asm__("func_8009F380");
extern s32* D_80139280;
extern s32 D_801B2D04;
extern s32 D_801B2D00;

    D_801B2D04 = 0x20;
    D_80139280[5] = -1;
    D_801B2D00 += 1;
    func_8009F380__for_func_8009F338();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009F380(void)
{
extern u8 D_800DB158[];
extern u8 D_80139F28[];
extern s32* D_80139280;
extern s32 D_801B2D04;
extern s32 D_801B2D00;

    func_8006AEE0();
    func_8006A2FC(D_800DB158, D_80139F28, 0x14, 0xFF, 0x1, 0x8, 0, (s32)D_80139280);
    if (--D_801B2D04 == 0)
    {
        D_801B2D00 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009F408(void)
{
extern s32 D_801B2D00;

    D_801B2D00 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009F420(s32 arg0)
{
extern u32 D_801B2D08;
extern s32 D_801B2D0C;
extern void (*D_800D6824[])(void);
extern u8 D_800D9790[];
extern u8 D_80139A78[];
extern s32 D_80139280;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2D08 = 1;
        D_801B2D0C = 1;
        return 1;
    }

    if (D_801B2D08 < 0x6)
    {
        D_800D6824[D_801B2D08]();
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
void func_8009F498(void)
{
extern u32 D_801B2D08;
extern s32 D_801B2D0C;
extern void (*D_800D6824[])(void);
extern u8 D_800D9790[];
extern u8 D_80139A78[];
extern s32 D_80139280;

    D_801B2D08 = 1;
    D_801B2D0C = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009F4B0(void)
{
extern u32 D_801B2D08;
extern s32 D_801B2D0C;
extern void (*D_800D6824[])(void);
extern u8 D_800D9790[];
extern u8 D_80139A78[];
extern s32 D_80139280;

    func_8006AEE0();
    func_8006A2FC(D_800D9790, D_80139A78, 0x5A, 0xFF, 0x1, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2D0C == 0)
    {
        D_801B2D08 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009F53C(void)
{
extern void func_8009F584__for_func_8009F53C(void) __asm__("func_8009F584");
extern s32* D_80139280;
extern s32 D_801B2D0C;
extern s32 D_801B2D08;
extern u8 D_800D9790[];
extern u8 D_80139A78[];

    D_801B2D0C = 0x40;
    D_80139280[15] = -1;
    D_801B2D08 += 1;
    func_8009F584__for_func_8009F53C();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009F584(void)
{
extern void func_8009F584(void);
extern s32* D_80139280;
extern s32 D_801B2D0C;
extern s32 D_801B2D08;
extern u8 D_800D9790[];
extern u8 D_80139A78[];

    func_8006AEE0();
    func_8006A2FC(D_800D9790, D_80139A78, 0x5A, 0xFF, 0x1, 0x4, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2D0C == 0)
    {
        D_801B2D08 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009F610(void)
{
extern s32 D_801B2D08;

    D_801B2D08 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009F628(s32 arg0)
{
extern u32 D_801B2D10;
extern s32 D_801B2D14;
extern void (*D_800D683C[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2D10 = 1;
        D_801B2D14 = 1;
        return 1;
    }

    if (D_801B2D10 < 0x6)
    {
        D_800D683C[D_801B2D10]();
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
void func_8009F6A0(void)
{
extern u32 D_801B2D10;
extern s32 D_801B2D14;
extern void (*D_800D683C[])(void);

    D_801B2D10 = 1;
    D_801B2D14 = 1;
}

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_8009F6B8(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_801B25D8;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern s32 D_80139234;
extern s32 D_801B2D10;
extern s32 D_801B2D14;
extern void func_8009DABC__for_func_8009F6B8(void) __asm__("func_8009DABC");

    D_801B25D8 = 1;
    D_8013B238 = D_80139258;
    D_80139234 = 0;
    D_801B2D14 = 0x12C;
    D_801B2D10 += 1;
    func_8009DABC__for_func_8009F6B8();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8009F734(void)
{
extern s32 D_801B2D10;
extern void func_8009DBB0__for_func_8009F734(void) __asm__("func_8009DBB0");
extern s32 D_801B2D14;

    D_801B2D14 = 0x20;
    D_801B2D10 += 1;
    func_8009DBB0__for_func_8009F734();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009F76C(void)
{
extern s32 D_801B2D10;
extern void func_8009DBB0__for_func_8009F76C(void) __asm__("func_8009DBB0");
extern s32 D_801B2D14;

    D_801B2D10 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009F784(s32 arg0)
{
extern u32 D_801B2D18;
extern s32 D_801B2D1C;
extern void (*D_800D6854[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2D18 = 1;
        D_801B2D1C = 1;
        return 1;
    }

    if (D_801B2D18 < 0x6)
    {
        D_800D6854[D_801B2D18]();
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
void func_8009F7FC(void)
{
extern u32 D_801B2D18;
extern s32 D_801B2D1C;
extern void (*D_800D6854[])(void);

    D_801B2D18 = 1;
    D_801B2D1C = 1;
}

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_8009F814(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DEC;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern s32 D_8013923C;
extern s32 D_801B2D18;
extern s32 D_801B2D1C;
extern void func_8009DC9C__for_func_8009F814(void) __asm__("func_8009DC9C");

    D_80182DEC = 1;
    D_801B24A8 = D_80139258;
    D_8013923C = 0;
    D_801B2D1C = 0x3C;
    D_801B2D18 += 1;
    func_8009DC9C__for_func_8009F814();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8009F890(void)
{
extern s32 D_801B2D18;
extern void func_8009DD90__for_func_8009F890(void) __asm__("func_8009DD90");
extern s32 D_801B2D1C;

    D_801B2D1C = 0x10;
    D_801B2D18 += 1;
    func_8009DD90__for_func_8009F890();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009F8C8(void)
{
extern s32 D_801B2D18;
extern void func_8009DD90__for_func_8009F8C8(void) __asm__("func_8009DD90");
extern s32 D_801B2D1C;

    D_801B2D18 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009F8E0(s32 arg0)
{
extern u32 D_801B2D20;
extern s32 D_801B2D24;
extern void (*D_800D686C[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2D20 = 1;
        D_801B2D24 = 1;
        return 1;
    }

    if (D_801B2D20 < 0x6)
    {
        D_800D686C[D_801B2D20]();
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
void func_8009F958(void)
{
extern u32 D_801B2D20;
extern s32 D_801B2D24;
extern void (*D_800D686C[])(void);

    D_801B2D20 = 1;
    D_801B2D24 = 1;
}

/** @brief Initialize the actor and its screen coordinates, then advance the sequence. */
void func_8009F970(void)
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

extern void func_8009FA0C__for_func_8009F970(void) __asm__("func_8009FA0C");
extern WmapConfigA D_800D9370;
extern s32 D_8011CF4C;
extern u8 D_80125538[];
extern s32 D_8013924C;
extern u8 *D_801399BC;
extern s32 D_80182D64;
extern s32 D_801B2D20;
extern s32 D_801B2D24;

    D_801399BC = D_80125538;
    D_8013924C = 0x280;
    D_800D9370.field_06 = 0xF;
    D_800D9370.field_10 = -1;
    D_800D9370.field_26 = 4;
    D_800D9370.field_22 = 0x81;
    D_800D9370.field_24 = 1;
    D_800D9370.field_02 = 0;
    D_800D9370.field_0E = 0;
    D_801B2D24 = 0x9C;
    D_80182D64 = D_8011CF4C;
    D_801B2D20 += 1;
    func_8009FA0C__for_func_8009F970();
}

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position downward and advance when the frame counter expires.
 */
void func_8009FA0C(void)
{
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s16 D_80182D64[];
extern s32 D_8013924C;
extern s32 D_801B2D20;
extern s32 D_801B2D24;

    wmap_step_actor_animation(D_800D9370, D_801399B8);
    wmap_draw_actor_sprite(D_800D9370, *(s32*)D_80182D64, 0x17, 0x2, 0);
    D_80182D64[1] = D_8013924C / 0x10;
    if (D_8013924C < 0x640)
    {
        D_8013924C += 0x10;
    }
    if (--D_801B2D24 == 0)
    {
        D_801B2D20 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8009FABC(void)
{
extern void func_8009FB08__for_func_8009FABC(void) __asm__("func_8009FB08");
extern s16 D_800D9370[];
extern s32 D_801B2D24;
extern s32 D_801B2D20;

    D_800D9370[19] = 2;
    D_800D9370[17] = 0;
    D_801B2D24 = 0x40;
    D_801B2D20 += 1;
    func_8009FB08__for_func_8009FABC();
}

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position and advance when the frame counter expires.
 */
void func_8009FB08(void)
{
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s16 D_80182D64[];
extern s32 D_8013924C;
extern s32 D_801B2D20;
extern s32 D_801B2D24;

    wmap_step_actor_animation(D_800D9370, D_801399B8);
    wmap_draw_actor_sprite(D_800D9370, *(s32*)D_80182D64, 0x17, 0x2, 0);
    D_80182D64[1] = D_8013924C / 0x10;
    if (D_8013924C < 0x640)
    {
        D_8013924C += 0x10;
    }
    if (--D_801B2D24 == 0)
    {
        D_801B2D20 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009FBB8(void)
{
extern s32 D_801B2D20;

    D_801B2D20 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009FBD0(s32 arg0)
{
extern u32 D_801B2D28;
extern s32 D_801B2D2C;
extern void (*D_800D6884[])(void);
extern u8 D_800DA8C0[];
extern u8 D_80139D98[];
extern s32 D_80139280;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2D28 = 1;
        D_801B2D2C = 1;
        return 1;
    }

    if (D_801B2D28 < 0x6)
    {
        D_800D6884[D_801B2D28]();
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
void func_8009FC48(void)
{
extern u32 D_801B2D28;
extern s32 D_801B2D2C;
extern void (*D_800D6884[])(void);
extern u8 D_800DA8C0[];
extern u8 D_80139D98[];
extern s32 D_80139280;

    D_801B2D28 = 1;
    D_801B2D2C = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009FC60(void)
{
extern u32 D_801B2D28;
extern s32 D_801B2D2C;
extern void (*D_800D6884[])(void);
extern u8 D_800DA8C0[];
extern u8 D_80139D98[];
extern s32 D_80139280;

    func_8006AEE0();
    func_8006A2FC(D_800DA8C0, D_80139D98, 0x30, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B2D2C == 0)
    {
        D_801B2D28 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8009FCEC(void)
{
extern void func_8009FD34__for_func_8009FCEC(void) __asm__("func_8009FD34");
extern s32* D_80139280;
extern s32 D_801B2D2C;
extern s32 D_801B2D28;
extern u8 D_800DA8C0[];
extern u8 D_80139D98[];

    D_801B2D2C = 0x20;
    D_80139280[25] = -1;
    D_801B2D28 += 1;
    func_8009FD34__for_func_8009FCEC();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009FD34(void)
{
extern void func_8009FD34(void);
extern s32* D_80139280;
extern s32 D_801B2D2C;
extern s32 D_801B2D28;
extern u8 D_800DA8C0[];
extern u8 D_80139D98[];

    func_8006AEE0();
    func_8006A2FC(D_800DA8C0, D_80139D98, 0x30, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B2D2C == 0)
    {
        D_801B2D28 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009FDC0(void)
{
extern s32 D_801B2D28;

    D_801B2D28 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009FDD8(s32 arg0)
{
extern u32 D_801B2D30;
extern s32 D_801B2D34;
extern void (*D_800D689C[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2D30 = 1;
        D_801B2D34 = 1;
        return 1;
    }

    if (D_801B2D30 < 0x6)
    {
        D_800D689C[D_801B2D30]();
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
void func_8009FE50(void)
{
extern u32 D_801B2D30;
extern s32 D_801B2D34;
extern void (*D_800D689C[])(void);

    D_801B2D30 = 1;
    D_801B2D34 = 1;
}

/** @brief Initialize the actor, save its screen position, and begin a 142-tick sequence step. */
void func_8009FE68(void)
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

extern void func_8009FF00__for_func_8009FE68(void) __asm__("func_8009FF00");
extern WmapConfigA D_800D939C;
extern s32 D_8011CF4C;
extern u8 D_80123538[];
extern s32 D_80139250;
extern u8 *D_801399C4;
extern s32 D_80182D58;
extern s32 D_801B2D30;
extern s32 D_801B2D34;

    D_801399C4 = D_80123538;
    D_80139250 = 0x780;
    D_800D939C.field_06 = 0xF;
    D_800D939C.field_10 = -1;
    D_800D939C.field_26 = 1;
    D_800D939C.field_22 = 0x7F;
    D_800D939C.field_02 = 0;
    D_800D939C.field_0E = 0;
    D_800D939C.field_24 = 0;
    D_801B2D34 = 0x8E;
    D_80182D58 = D_8011CF4C;
    D_801B2D30 += 1;
    func_8009FF00__for_func_8009FE68();
}

/** @brief Draw a world-map actor, then wind down a scrolling scalar. */
void func_8009FF00(void)
{
extern u8 D_800D939C[];
extern u8 D_801399C0[];
extern s32 D_80182D58;
extern s32 D_80139250;
extern s32 D_801B2D30;
extern s32 D_801B2D34;

    wmap_step_actor_animation(D_800D939C, D_801399C0);
    wmap_draw_actor_sprite(D_800D939C, D_80182D58, 0x8, 0x2, 0);
    *(s16*)((u8*)&D_80182D58 + 0x2) = D_80139250 / 16;
    if (D_80139250 >= 0x321)
    {
        D_80139250 -= 0x10;
    }
    if (--D_801B2D34 == 0)
    {
        D_801B2D30 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8009FFB0(void)
{
extern void func_8009FFFC__for_func_8009FFB0(void) __asm__("func_8009FFFC");
extern s16 D_800D939C[];
extern s32 D_801B2D34;
extern s32 D_801B2D30;

    D_800D939C[19] = 4;
    D_800D939C[17] = 0;
    D_801B2D34 = 0x20;
    D_801B2D30 += 1;
    func_8009FFFC__for_func_8009FFB0();
}

/** @brief Draw a world-map actor, then wind down a scrolling scalar. */
void func_8009FFFC(void)
{
extern u8 D_800D939C[];
extern u8 D_801399C0[];
extern s32 D_80182D58;
extern s32 D_80139250;
extern s32 D_801B2D30;
extern s32 D_801B2D34;

    wmap_step_actor_animation(D_800D939C, D_801399C0);
    wmap_draw_actor_sprite(D_800D939C, D_80182D58, 0x8, 0x2, 0);
    *(s16*)((u8*)&D_80182D58 + 0x2) = D_80139250 / 16;
    if (D_80139250 >= 0x321)
    {
        D_80139250 -= 0x10;
    }
    if (--D_801B2D34 == 0)
    {
        D_801B2D30 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A00AC(void)
{
extern s32 D_801B2D30;

    D_801B2D30 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A00C4(s32 arg0)
{
extern u32 D_801B2D38;
extern s32 D_801B2D3C;
extern void (*D_800D68B4[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2D38 = 1;
        D_801B2D3C = 1;
        return 1;
    }

    if (D_801B2D38 < 0x6)
    {
        D_800D68B4[D_801B2D38]();
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
void func_800A013C(void)
{
extern u32 D_801B2D38;
extern s32 D_801B2D3C;
extern void (*D_800D68B4[])(void);

    D_801B2D38 = 1;
    D_801B2D3C = 1;
}

/** @brief Initialize the actor and begin a 48-tick sequence step. */
void func_800A0154(void)
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

extern void func_800A01F0__for_func_800A0154(void) __asm__("func_800A01F0");
extern WmapConfigA D_800D93C8;
extern s32 D_8011CF4C;
extern u8 D_80123538[];
extern s32 D_80139260;
extern u8 *D_801399CC;
extern s32 D_80182D60;
extern s32 D_801B2D38;
extern s32 D_801B2D3C;

    D_801399CC = D_80123538;
    D_80139260 = 0x320;
    D_800D93C8.field_06 = 0xF;
    D_800D93C8.field_02 = 0;
    D_800D93C8.field_0E = 1;
    D_800D93C8.field_10 = -1;
    D_800D93C8.field_26 = 4;
    D_800D93C8.field_22 = 0x81;
    D_800D93C8.field_24 = 1;
    D_801B2D3C = 0x30;
    D_80182D60 = D_8011CF4C;
    D_801B2D38 += 1;
    func_800A01F0__for_func_800A0154();
}

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position and advance when the frame counter expires.
 */
void func_800A01F0(void)
{
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s16 D_80182D60[];
extern s32 D_80139260;
extern s32 D_801B2D38;
extern s32 D_801B2D3C;

    wmap_step_actor_animation(D_800D93C8, D_801399C8);
    wmap_draw_actor_sprite(D_800D93C8, *(s32*)D_80182D60, 0x8, 0x2, 0);
    D_80182D60[1] = D_80139260 / 0x10;
    if (D_80139260 < 0xA00)
    {
        D_80139260 += 0x8;
    }
    if (--D_801B2D3C == 0)
    {
        D_801B2D38 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_800A02A0(void)
{
extern void func_800A02EC__for_func_800A02A0(void) __asm__("func_800A02EC");
extern s16 D_800D93C8[];
extern s32 D_801B2D3C;
extern s32 D_801B2D38;

    D_800D93C8[19] = 4;
    D_800D93C8[17] = 0;
    D_801B2D3C = 0x20;
    D_801B2D38 += 1;
    func_800A02EC__for_func_800A02A0();
}

/**
 * @brief World-map step handler: draw the actor at the tracked position, then
 *        scroll the position and advance when the frame counter expires.
 */
void func_800A02EC(void)
{
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s16 D_80182D60[];
extern s32 D_80139260;
extern s32 D_801B2D38;
extern s32 D_801B2D3C;

    wmap_step_actor_animation(D_800D93C8, D_801399C8);
    wmap_draw_actor_sprite(D_800D93C8, *(s32*)D_80182D60, 0x8, 0x2, 0);
    D_80182D60[1] = D_80139260 / 0x10;
    if (D_80139260 < 0xA00)
    {
        D_80139260 += 0x8;
    }
    if (--D_801B2D3C == 0)
    {
        D_801B2D38 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A039C(void)
{
extern s32 D_801B2D38;

    D_801B2D38 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A03B4(s32 arg0)
{
extern u32 D_801B2D40;
extern s32 D_801B2D44;
extern void (*D_800D68CC[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2D40 = 1;
        D_801B2D44 = 1;
        return 1;
    }

    if (D_801B2D40 < 0x8)
    {
        D_800D68CC[D_801B2D40]();
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
void func_800A042C(void)
{
extern u32 D_801B2D40;
extern s32 D_801B2D44;
extern void (*D_800D68CC[])(void);

    D_801B2D40 = 1;
    D_801B2D44 = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_800A0444(void)
{
extern s32 D_80182DF4;
extern s32 D_801B2D40;
extern s32 D_801B2D44;

    s32 remaining;

    func_8006B328(0xB4, 0xF0, 1, -1, -1, -4, 0, 0xB, -0xFA, 0xFA, -0xDC, 0x8C, 1, 0x81, 1, 8, 1);
    D_80182DF4 += 8;
    remaining = D_801B2D44 - 1;
    D_801B2D44 = remaining;
    if (remaining == 0)
    {
        D_801B2D40 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A0504(void)
{
extern void func_800A053C__for_func_800A0504(void) __asm__("func_800A053C");
extern s32 D_801B2D44;
extern s32 D_801B2D40;

    D_801B2D44 = 0x28;
    D_801B2D40 += 1;
    func_800A053C__for_func_800A0504();
}

/** @brief World-map step handler: spawn a scripted actor and expire the timer. */
void func_800A053C(void)
{
extern s32 D_801B2D40;
extern s32 D_801B2D44;

    func_8006B328(0xB4, 0xF0, 1, -1, -1, -4, 0, 0xB, -0xFA, 0xFA, -0xDC,
                  0x8C, 1, 0x81, 1, 8, 1);
    if (--D_801B2D44 == 0)
    {
        D_801B2D40 += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A05F0(void)
{
extern void func_800A0630__for_func_800A05F0(void) __asm__("func_800A0630");
extern s32 D_800DCEAC;
extern s32 D_801B2D44;
extern s32 D_801B2D40;

    D_800DCEAC = 0;
    D_801B2D44 = 0x40;
    D_801B2D40 += 1;
    func_800A0630__for_func_800A05F0();
}

/** @brief World-map step handler: spawn a scripted actor and expire the timer. */
void func_800A0630(void)
{
extern s32 D_801B2D40;
extern s32 D_801B2D44;

    func_8006B328(0xB4, 0xF0, 1, -1, -1, -4, 0, 0xB, -0xFA, 0xFA, -0xDC,
                  0x8C, 1, 0x81, 1, 8, 1);
    if (--D_801B2D44 == 0)
    {
        D_801B2D40 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A06E4(void)
{
extern s32 D_801B2D40;

    D_801B2D40 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A06FC(s32 arg0)
{
extern u32 D_801B2D48;
extern s32 D_801B2D4C;
extern void (*D_800D68EC[])(void);
extern void func_8009E008__for_func_800A06FC(void) __asm__("func_8009E008");
extern s32 D_80139264;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2D48 = 1;
        D_801B2D4C = 1;
        return 1;
    }

    if (D_801B2D48 < 0x6)
    {
        D_800D68EC[D_801B2D48]();
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
void func_800A0774(void)
{
extern u32 D_801B2D48;
extern s32 D_801B2D4C;
extern void (*D_800D68EC[])(void);
extern void func_8009E008__for_func_800A0774(void) __asm__("func_8009E008");
extern s32 D_80139264;

    D_801B2D48 = 1;
    D_801B2D4C = 1;
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A078C(void)
{
extern u32 D_801B2D48;
extern s32 D_801B2D4C;
extern void (*D_800D68EC[])(void);
extern void func_8009E008__for_func_800A078C(void) __asm__("func_8009E008");
extern s32 D_80139264;

    D_80139264 = 0;
    D_801B2D4C = 0x8C;
    D_801B2D48 += 1;
    func_8009E008__for_func_800A078C();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A07CC(void)
{
extern void func_8009E114__for_func_800A07CC(void) __asm__("func_8009E114");
extern s32 D_801B2D4C;
extern s32 D_801B2D48;

    D_801B2D4C = 0x40;
    D_801B2D48 += 1;
    func_8009E114__for_func_800A07CC();
}

/**
 * @brief Clear the world-map translation and rotation offsets, then advance the transition counter.
 */
void func_800A0804(void)
{
    extern VECTOR D_80182D48;
    extern SVECTOR D_801398C8;
    extern s32 D_801B2D48;

    D_80182D48.vy = 0;
    D_80182D48.vx = 0;
    D_801398C8.vy = 0;
    D_801398C8.vx = 0;
    D_801B2D48 += 1;
}
