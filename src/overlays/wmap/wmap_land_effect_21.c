#include "wmap_main.h"
#include "wmap_land_effect_21.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_sprite_render.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_effect_primitives.h"

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80088938(void)
{
extern s32 D_801B2950;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B297C;
extern s32 D_801B2978;

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
    if (--D_801B297C == 0)
    {
        D_801B2978 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80088A38(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B2984;
extern s32 D_801B2980;

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
    if (--D_801B2984 == 0)
    {
        D_801B2980 += 1;
    }
}

/** @brief Project a spiral emitter and append animated copies along its trail. */
void func_80088B38(void)
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
} __attribute__((aligned(4))) WmapConfigA;

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    union { s32 packed; struct { s16 x, y; } point; } screen;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

extern WmapConfigA D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern WmapMotion D_801B0080;
extern WmapResource D_80139988[];
extern DVECTOR D_80182D58;
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_801B2988;
extern s32 D_801B298C;

    SVECTOR position;
    s32 previous;
    s32 next;
    s32 i;
    s32 remaining;
    WmapMotion *motion;
    WmapResource *resource;
    WmapConfigA *actor;

    position.vx = ((D_801B0080.z >> 3) * (ccos(D_801B0080.angle) >> 6)) >> 12;
    position.vy = ((D_801B0080.z >> 3) * (csin(D_801B0080.angle) >> 6)) >> 12;
    position.vz = D_801B0080.field_0E;
    gte_ldv0(&position);
    gte_rtps();
    D_801B0080.z += 1500;
    D_801B0080.angle += 192;
    D_8013923C--;
    gte_stsxy(&D_80182D58);
    D_801B0080.screen.point.x = D_80182D58.vx;
    D_801B0080.screen.point.y = D_80182D58.vy;
    if (D_8013923C == 0)
    {
        previous = D_80139234;
        D_80139234 = previous + 1;
        next = previous + 61;
        if (next < 250)
        {
            D_8013923C = 2;
            motion = &D_801B0080 - 60;
            motion[next] = D_801B0080;
            D_80139988[D_80139234 + 60] = D_80139988[60];
            D_800D9268[D_80139234 + 60] = D_800D9268[60];
            D_800D9268[D_80139234 + 60].field_0E = 0;
            D_800D9268[D_80139234 + 60].field_10 = -1;
        }
    }
    for (i = 61; i < D_80139234 + 60; i++)
    {
        actor = &D_800D9268[i];
        motion = &D_801AFBD0[i];
        resource = &D_80139988[i];
        func_8006CC4C(actor, resource);
        func_80066F9C(actor, motion->screen.packed, 8, 10, 0);
    }
    remaining = D_801B298C - 1;
    D_801B298C = remaining;
    if (remaining == 0)
    {
        D_801B2988++;
    }
}

/** @brief Animate and draw the active actor range, then advance its countdown. */
void func_80088E24(void)
{
/** @brief Actor storage consumed by the animation and drawing helpers. */
typedef struct
{
    u8 pad_00[0x22];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapActor;
/** @brief Animation resource slot. */
typedef struct { s32 word; void *data; } WmapResource;
/** @brief Drawing position with the original 0x14-byte stride. */
typedef struct { s32 position; u8 pad[0x10]; } WmapPosition;

extern WmapActor D_800D9268[];
extern WmapResource D_80139988[];
extern WmapPosition D_801AFBE0[];
extern s32 D_80139234;
extern s32 D_801B2988;
extern s32 D_801B298C;

    s32 i;
    s32 remaining;

    for (i = 61; i < D_80139234 + 60; i++)
    {
        D_800D9268[i].field_22 = 0;
        D_800D9268[i].field_26 = 2;
        func_8006CC4C(&D_800D9268[i], &D_80139988[i]);
        func_80066F9C(&D_800D9268[i], D_801AFBE0[i].position, 8, 10, 0);
    }
    remaining = D_801B298C - 1;
    D_801B298C = remaining;
    if (remaining == 0)
    {
        D_801B2988++;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_80088F18(void)
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
extern u8 D_8011D538[];
extern s32 D_801B2998;
extern s32 D_801B299C;
extern void func_8008A5D0__for_func_80088F18(void) __asm__("func_8008A5D0");

    s32 i;

    D_801B0FD0 = 24;
    D_80139280[0x1F] = 1;
    D_80139280[0x20] = 1;
    D_80139280[0x21] = 20;
    D_80139280[0x22] = 30;
    D_80139280[0x23] = 2;
    D_80139280[0x24] = 1500;
    D_80139280[0x25] = 20;
    D_80139280[0x26] = 15;
    D_80139280[0x27] = 5;
    D_80139280[0x28] = 10000;
    for (i = 0; i < 24; i++)
    {
        D_801AFBD0[i + D_80139280[0x25]].field_00 = 0;
        D_80139988[i + 24].field_04 = D_8011D538;
    }
    D_801B299C = 48;
    D_801B2998++;
    func_8008A5D0__for_func_80088F18();
}

/** @brief Initialize the effect actors, resources, and evenly spaced angles. */
void func_80089004(void)
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

extern WmapConfigA D_800D95D8[];
extern WmapMotion D_801AFD60[];
extern WmapResource D_80139988[];
extern s32 *D_80139280;
extern s32 D_801B0FD0;
extern u8 D_80121538[];
extern s32 D_801B29A0;
extern s32 D_801B29A4;
extern void func_8008A7C8__for_func_80089004(void) __asm__("func_8008A7C8");

    s32 i;

    i = 0;
    D_801B0FD0 = 12;
    D_80139280[0x15] = 128;
    D_80139280[0x16] = 1;
    D_80139280[0x17] = 128;
    D_80139280[0x18] = 0;
    D_80139280[0x19] = -1;
    D_80139280[0x1A] = 1500;
    D_80139280[0x1B] = 20;
    D_80139280[0x1C] = 8;
    D_80139280[0x1D] = 1;
    D_80139280[0x1E] = 10;
    do
    {
        D_801AFD60[i].state = 1;
        D_801AFD60[i].angle = i * 341;
        D_801AFD60[i].scale = 128;
        D_801AFD60[i].z = 10;
        D_801AFD60[i].x = 0;
        D_801AFD60[i].field_0E = 0;
        D_80139988[i + 20].resource = D_80121538;
        D_800D95D8[i].field_06 = 15;
        D_800D95D8[i].field_10 = -1;
        D_800D95D8[i].field_26 = 2;
        D_800D95D8[i].field_02 = 0;
        D_800D95D8[i].field_0E = 1;
        D_800D95D8[i].field_22 = 0;
        D_800D95D8[i].field_24 = 127;
        i++;
    } while (i < 12);
    D_801B29A4 = 32;
    D_801B29A0++;
    func_8008A7C8__for_func_80089004();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80089134(s32 arg0)
{
extern u32 D_801B2958;
extern s32 D_801B295C;
extern void (*D_800D5B60[])(void);
extern void func_80089208__for_func_80089134(void) __asm__("func_80089208");
extern s32 D_8013B20C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2958 = 1;
        D_801B295C = 1;
        return 1;
    }

    if (D_801B2958 < 0x6)
    {
        D_800D5B60[D_801B2958]();
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
void func_800891AC(void)
{
extern u32 D_801B2958;
extern s32 D_801B295C;
extern void (*D_800D5B60[])(void);
extern void func_80089208__for_func_800891AC(void) __asm__("func_80089208");
extern s32 D_8013B20C;

    D_801B2958 = 1;
    D_801B295C = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_800891C4(void)
{
extern u32 D_801B2958;
extern s32 D_801B295C;
extern void (*D_800D5B60[])(void);
extern void func_80089208__for_func_800891C4(void) __asm__("func_80089208");
extern s32 D_8013B20C;

    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2958 += 1;
    func_80089208__for_func_800891C4();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80089208(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2958;
extern void func_80089244__for_func_80089208(void) __asm__("func_80089244");

    if (D_8013B20C == 0)
    {
        D_801B2958 += 1;
        func_80089244__for_func_80089208();
    }
}

/** @brief Save the coordinate pair, register a callback, and run the next sequence step. */
void func_80089244(void)
{
extern void func_80089298__for_func_80089244(void) __asm__("func_80089298");
extern s32 D_8011CF4C;
extern s32 D_8013B20C;
extern s32 D_80182D58;
extern s32 D_801B2958;
extern void func_800892EC__for_func_80089244(void) __asm__("func_800892EC");

    D_80182D58 = D_8011CF4C;
    func_8006CAC0(&func_800892EC__for_func_80089244);
    D_8013B20C = 1;
    D_801B2958 += 1;
    func_80089298__for_func_80089244();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80089298(void)
{
extern s32 D_801B2958;
extern s32 D_8013B20C;
extern void func_800892D4__for_func_80089298(void) __asm__("func_800892D4");

    if (D_8013B20C == 0)
    {
        D_801B2958 += 1;
        func_800892D4__for_func_80089298();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800892D4(void)
{
extern s32 D_801B2958;
extern s32 D_8013B20C;
extern void func_800892D4(void);

    D_801B2958 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800892EC(s32 arg0)
{
extern u32 D_801B2960;
extern s32 D_801B2964;
extern void (*D_800D5B78[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2960 = 1;
        D_801B2964 = 1;
        return 1;
    }

    if (D_801B2960 < 0x10)
    {
        D_800D5B78[D_801B2960]();
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
void func_80089364(void)
{
extern u32 D_801B2960;
extern s32 D_801B2964;
extern void (*D_800D5B78[])(void);

    D_801B2960 = 1;
    D_801B2964 = 1;
}

/** @brief Play sound 34, set the world-map color and value, and begin an eight-tick delay. */
void func_8008937C(void)
{
extern s32 D_801B2960;
extern s32 D_801B2964;

    func_800652A8(0x22, 0x80);
    g_wmap_backdrop_target_level = 4;
    func_8006683C(0x304010);
    D_801B2964 = 8;
    D_801B2960 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800893D0(void)
{
extern s32 D_801B2964;
extern s32 D_801B2960;
extern void func_80089CB8__for_func_800893D0(void) __asm__("func_80089CB8");

    if (--D_801B2964 == 0)
    {
        D_801B2960 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80089404(void)
{
extern s32 D_801B2964;
extern s32 D_801B2960;
extern void func_80089CB8__for_func_80089404(void) __asm__("func_80089CB8");

    func_8006CAC0(func_80089CB8__for_func_80089404);
    D_801B2964 = 0x4;
    D_801B2960 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80089440(void)
{
extern s32 D_801B2964;
extern s32 D_801B2960;

    if (--D_801B2964 == 0)
    {
        D_801B2960 += 1;
    }
}

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void func_80089474(void)
{
extern s32 D_801ADAE0;
extern s32 D_801B2964;
extern s32 D_801B2960;
extern void func_8008972C__for_func_80089474(void) __asm__("func_8008972C");

    D_801ADAE0 = 1;
    func_8006CAC0(func_8008972C__for_func_80089474);
    D_801B2964 = 0x34;
    D_801B2960 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800894BC(void)
{
extern s32 D_801B2964;
extern s32 D_801B2960;
extern void func_8008A210__for_func_800894BC(void) __asm__("func_8008A210");

    if (--D_801B2964 == 0)
    {
        D_801B2960 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800894F0(void)
{
extern s32 D_801B2964;
extern s32 D_801B2960;
extern void func_8008A210__for_func_800894F0(void) __asm__("func_8008A210");

    func_8006CAC0(func_8008A210__for_func_800894F0);
    D_801B2964 = 0x38;
    D_801B2960 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8008952C(void)
{
extern s32 D_801B2964;
extern s32 D_801B2960;
extern void func_80089F64__for_func_8008952C(void) __asm__("func_80089F64");
extern void func_8008A738__for_func_8008952C(void) __asm__("func_8008A738");

    if (--D_801B2964 == 0)
    {
        D_801B2960 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_80089560(void)
{
extern s32 D_801B2964;
extern s32 D_801B2960;
extern void func_80089F64__for_func_80089560(void) __asm__("func_80089F64");
extern void func_8008A738__for_func_80089560(void) __asm__("func_8008A738");

    func_8006CAC0(func_80089F64__for_func_80089560);
    func_8006CAC0(func_8008A738__for_func_80089560);
    D_801B2964 = 0x68;
    D_801B2960 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800895A8(void)
{
extern s32 D_801B2964;
extern s32 D_801B2960;
extern void func_80089E10__for_func_800895A8(void) __asm__("func_80089E10");

    if (--D_801B2964 == 0)
    {
        D_801B2960 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800895DC(void)
{
extern s32 D_801B2964;
extern s32 D_801B2960;
extern void func_80089E10__for_func_800895DC(void) __asm__("func_80089E10");

    func_8006CAC0(func_80089E10__for_func_800895DC);
    D_801B2964 = 0x3C;
    D_801B2960 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80089618(void)
{
extern s32 D_801B2964;
extern s32 D_801B2960;
extern void func_8008A540__for_func_80089618(void) __asm__("func_8008A540");
extern void func_80089B18__for_func_80089618(void) __asm__("func_80089B18");

    if (--D_801B2964 == 0)
    {
        D_801B2960 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_8008964C(void)
{
extern s32 D_801B2964;
extern s32 D_801B2960;
extern void func_8008A540__for_func_8008964C(void) __asm__("func_8008A540");
extern void func_80089B18__for_func_8008964C(void) __asm__("func_80089B18");

    func_8006CAC0(func_8008A540__for_func_8008964C);
    func_8006CAC0(func_80089B18__for_func_8008964C);
    D_801B2964 = 0x8B;
    D_801B2960 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80089694(void)
{
extern s32 D_801B2964;
extern s32 D_801B2960;

    if (--D_801B2964 == 0)
    {
        D_801B2960 += 1;
    }
}

void func_800896C8(void)
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
extern s32 D_801B2960;

    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B2960 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008972C(s32 arg0)
{
extern u32 D_801B2968;
extern s32 D_801B296C;
extern void (*D_800D5BB8[])(void);
extern void func_8008983C__for_func_8008972C(void) __asm__("func_8008983C");
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_80182D58;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2968 = 1;
        D_801B296C = 1;
        return 1;
    }

    if (D_801B2968 < 0xA)
    {
        D_800D5BB8[D_801B2968]();
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
void func_800897A4(void)
{
extern u32 D_801B2968;
extern s32 D_801B296C;
extern void (*D_800D5BB8[])(void);
extern void func_8008983C__for_func_800897A4(void) __asm__("func_8008983C");
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_80182D58;

    D_801B2968 = 1;
    D_801B296C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800897BC(void)
{
extern u32 D_801B2968;
extern s32 D_801B296C;
extern void (*D_800D5BB8[])(void);
extern void func_8008983C__for_func_800897BC(void) __asm__("func_8008983C");
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_80182D58;

    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 8;
    *(s16*)&D_800D9318[0x22] = 0x81;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x24] = 1;
    D_801B296C = 0x14;
    D_801B2968 += 1;
    func_8008983C__for_func_800897BC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008983C(void)
{
extern u32 D_801B2968;
extern s32 D_801B296C;
extern void (*D_800D5BB8[])(void);
extern void func_8008983C(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_80182D58;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_80182D58, 0xF, 0x9, 0);
    if (--D_801B296C == 0)
    {
        D_801B2968 += 1;
    }
}

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void func_800898B8(void)
{
extern s16 D_800D9326;
extern s32 D_801B2968;
extern s32 D_801B296C;
extern void func_800898FC__for_func_800898B8(void) __asm__("func_800898FC");

    D_800D9326 = 1;
    D_801B296C = 0x20;
    D_801B2968 += 1;
    func_800898FC__for_func_800898B8();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800898FC(void)
{
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_80182D58;
extern s32 D_801B296C;
extern s32 D_801B2968;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_80182D58, 0xF, 0x9, 0);
    if (--D_801B296C == 0)
    {
        D_801B2968 += 1;
    }
}

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void func_80089978(void)
{
extern s16 D_800D9326;
extern s32 D_801B2968;
extern s32 D_801B296C;
extern void func_800899BC__for_func_80089978(void) __asm__("func_800899BC");

    D_800D9326 = 2;
    D_801B296C = 0x8C;
    D_801B2968 += 1;
    func_800899BC__for_func_80089978();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800899BC(void)
{
extern void func_80089A84__for_func_800899BC(void) __asm__("func_80089A84");
extern s16 D_800D9268[];
extern s32 D_801B296C;
extern s32 D_801B2968;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_80182D58;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_80182D58, 0xF, 0x9, 0);
    if (--D_801B296C == 0)
    {
        D_801B2968 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80089A38(void)
{
extern void func_80089A84__for_func_80089A38(void) __asm__("func_80089A84");
extern s16 D_800D9268[];
extern s32 D_801B296C;
extern s32 D_801B2968;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_80182D58;

    D_800D9268[107] = 8;
    D_800D9268[105] = 0;
    D_801B296C = 0x10;
    D_801B2968 += 1;
    func_80089A84__for_func_80089A38();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80089A84(void)
{
extern void func_80089A84(void);
extern s16 D_800D9268[];
extern s32 D_801B296C;
extern s32 D_801B2968;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_80182D58;

    func_8006CC4C(D_800D9318, D_801399A8);
    func_80066F9C(D_800D9318, D_80182D58, 0xF, 0x9, 0);
    if (--D_801B296C == 0)
    {
        D_801B2968 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80089B00(void)
{
extern s32 D_801B2968;

    D_801B2968 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80089B18(s32 arg0)
{
extern u32 D_801B2970;
extern s32 D_801B2974;
extern void (*D_800D5BE0[])(void);
extern void func_80089C24__for_func_80089B18(void) __asm__("func_80089C24");
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2970 = 1;
        D_801B2974 = 1;
        return 1;
    }

    if (D_801B2970 < 0x4)
    {
        D_800D5BE0[D_801B2970]();
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
void func_80089B90(void)
{
extern u32 D_801B2970;
extern s32 D_801B2974;
extern void (*D_800D5BE0[])(void);
extern void func_80089C24__for_func_80089B90(void) __asm__("func_80089C24");
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801B2970 = 1;
    D_801B2974 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80089BA8(void)
{
extern u32 D_801B2970;
extern s32 D_801B2974;
extern void (*D_800D5BE0[])(void);
extern void func_80089C24__for_func_80089BA8(void) __asm__("func_80089C24");
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801399B4 = D_8011F538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 8;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x22] = 0x80;
    *(s16*)&D_800D9344[0x24] = 0;
    D_801B2974 = 0x8C;
    D_801B2970 += 1;
    func_80089C24__for_func_80089BA8();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80089C24(void)
{
extern u32 D_801B2970;
extern s32 D_801B2974;
extern void (*D_800D5BE0[])(void);
extern void func_80089C24(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x18, 0x1E, 0);
    if (--D_801B2974 == 0)
    {
        D_801B2970 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80089CA0(void)
{
extern s32 D_801B2970;

    D_801B2970 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80089CB8(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2978;
extern s32 D_801B297C;
extern void (*D_800D5BF0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80088938__for_func_80089CB8(void) __asm__("func_80088938");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2978 = 1;
        D_801B297C = 1;
        return 1;
    }

    if (D_801B2978 < 0x4)
    {
        D_800D5BF0[D_801B2978]();
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
void func_80089D30(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2978;
extern s32 D_801B297C;
extern void (*D_800D5BF0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80088938__for_func_80089D30(void) __asm__("func_80088938");

    D_801B2978 = 1;
    D_801B297C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80089D48(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2978;
extern s32 D_801B297C;
extern void (*D_800D5BF0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80088938__for_func_80089D48(void) __asm__("func_80088938");

    D_801B24A0 = D_80139258;
    D_801B2650 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B297C = 0x28;
    D_801B2978 += 1;
    func_80088938__for_func_80089D48();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80089DF8(void)
{
extern s32 D_801B2978;

    D_801B2978 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80089E10(s32 arg0)
{
extern u32 D_801B2980;
extern s32 D_801B2984;
extern void (*D_800D5C00[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2980 = 1;
        D_801B2984 = 1;
        return 1;
    }

    if (D_801B2980 < 0x4)
    {
        D_800D5C00[D_801B2980]();
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
void func_80089E88(void)
{
extern u32 D_801B2980;
extern s32 D_801B2984;
extern void (*D_800D5C00[])(void);

    D_801B2980 = 1;
    D_801B2984 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80089EA0(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 D_80182DC0;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern s32 D_801B2984;
extern u32 D_801B2980;
extern void func_80088A38__for_func_80089EA0(void) __asm__("func_80088A38");

    D_801B24A8 = D_80139258;
    D_801B2478 = D_80182DC0;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B2984 = 0x80;
    D_801B2980 += 1;
    func_80088A38__for_func_80089EA0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80089F4C(void)
{
extern s32 D_801B2980;

    D_801B2980 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80089F64(s32 arg0)
{
extern u32 D_801B2988;
extern s32 D_801B298C;
extern void (*D_800D5C10[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2988 = 1;
        D_801B298C = 1;
        return 1;
    }

    if (D_801B2988 < 0x8)
    {
        D_800D5C10[D_801B2988]();
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
void func_80089FDC(void)
{
extern u32 D_801B2988;
extern s32 D_801B298C;
extern void (*D_800D5C10[])(void);

    D_801B2988 = 1;
    D_801B298C = 1;
}

/** @brief Initialize the actor and auxiliary state for the next timed step. */
void func_80089FF4(void)
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

/** @brief Auxiliary sequence state and counter fields. */
typedef struct
{
    s16 state;
    s16 counter;
    u8 unknown_04[4];
    s32 value;
    s16 unknown_0c;
    s16 flags;
} WmapAuxState;

extern void func_80088B38__for_func_80089FF4(void) __asm__("func_80088B38");
extern WmapConfigA D_800D9CB8;
extern u8 D_80121538[];
extern s32 D_80139234;
extern s32 D_8013923C;
extern u8 *D_80139B6C;
extern WmapAuxState D_801B0080;
extern s32 D_801B2988;
extern s32 D_801B298C;

    D_80139234 = 0;
    D_8013923C = 2;
    D_80139B6C = D_80121538;
    D_800D9CB8.field_06 = 0xF;
    D_800D9CB8.field_10 = -1;
    D_800D9CB8.field_26 = 8;
    D_800D9CB8.field_02 = 0;
    D_800D9CB8.field_0E = 0;
    D_800D9CB8.field_22 = 0x81;
    D_800D9CB8.field_24 = 0x81;
    D_801B0080.state = 1;
    D_801B0080.value = 0;
    D_801B0080.counter = 0;
    D_801B0080.flags = 0;
    D_801B298C = 0x70;
    D_801B2988 += 1;
    func_80088B38__for_func_80089FF4();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8008A0A0(void)
{
extern void func_8008A0D8__for_func_8008A0A0(void) __asm__("func_8008A0D8");
extern s32 D_801B298C;
extern s32 D_801B2988;

    D_801B298C = 0x28;
    D_801B2988 += 1;
    func_8008A0D8__for_func_8008A0A0();
}

/** @brief Animate and draw the active actor range, then advance its countdown. */
void func_8008A0D8(void)
{
/** @brief Actor storage consumed by the animation and drawing helpers. */
typedef struct { u8 bytes[0x2C]; } WmapActor;
/** @brief Animation resource slot. */
typedef struct { s32 word; void *data; } WmapResource;
/** @brief Drawing position with the original 0x14-byte stride. */
typedef struct { s32 position; u8 pad[0x10]; } WmapPosition;

extern WmapActor D_800D9268[];
extern WmapResource D_80139988[];
extern WmapPosition D_801AFBE0[];
extern s32 D_80139234;
extern s32 D_801B2988;
extern s32 D_801B298C;

    s32 i;
    s32 remaining;

    for (i = 61; i < D_80139234 + 60; i++)
    {
        func_8006CC4C(&D_800D9268[i], &D_80139988[i]);
        func_80066F9C(&D_800D9268[i], D_801AFBE0[i].position, 8, 10, 0);
    }
    remaining = D_801B298C - 1;
    D_801B298C = remaining;
    if (remaining == 0)
    {
        D_801B2988++;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8008A1C0(void)
{
extern s32 D_801B2988;
extern void func_80088E24__for_func_8008A1C0(void) __asm__("func_80088E24");
extern s32 D_801B298C;

    D_801B298C = 0x40;
    D_801B2988 += 1;
    func_80088E24__for_func_8008A1C0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008A1F8(void)
{
extern s32 D_801B2988;
extern void func_80088E24__for_func_8008A1F8(void) __asm__("func_80088E24");
extern s32 D_801B298C;

    D_801B2988 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008A210(s32 arg0)
{
extern u32 D_801B2990;
extern s32 D_801B2994;
extern void (*D_800D5C30[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2990 = 1;
        D_801B2994 = 1;
        return 1;
    }

    if (D_801B2990 < 0x8)
    {
        D_800D5C30[D_801B2990]();
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
void func_8008A288(void)
{
extern u32 D_801B2990;
extern s32 D_801B2994;
extern void (*D_800D5C30[])(void);

    D_801B2990 = 1;
    D_801B2994 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8008A2A0(void)
{
extern u8* D_801399BC;
extern u8 D_8011D538[];
extern u8 D_800D9370[];
extern s32 D_801B2990;
extern s32 D_801B2994;
extern void func_8008A324__for_func_8008A2A0(void) __asm__("func_8008A324");

    D_801399BC = D_8011D538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0xE] = 3;
    *(s16*)&D_800D9370[0x10] = -1;
    *(s16*)&D_800D9370[0x26] = 2;
    *(s16*)&D_800D9370[0x22] = 0x81;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0x24] = 1;
    D_801B2994 = 0x10;
    D_801B2990 += 1;
    func_8008A324__for_func_8008A2A0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008A324(void)
{
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_80182D58;
extern s32 D_801B2994;
extern s32 D_801B2990;

    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_80182D58, 0xF, 0x2, 0);
    if (--D_801B2994 == 0)
    {
        D_801B2990 += 1;
    }
}

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void func_8008A3A0(void)
{
extern s16 D_800D937E;
extern s32 D_801B2990;
extern s32 D_801B2994;
extern void func_8008A3E4__for_func_8008A3A0(void) __asm__("func_8008A3E4");

    D_800D937E = 4;
    D_801B2994 = 0x8C;
    D_801B2990 += 1;
    func_8008A3E4__for_func_8008A3A0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008A3E4(void)
{
extern void func_8008A4AC__for_func_8008A3E4(void) __asm__("func_8008A4AC");
extern s16 D_800D9268[];
extern s32 D_801B2994;
extern s32 D_801B2990;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_80182D58;

    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_80182D58, 0xF, 0x2, 0);
    if (--D_801B2994 == 0)
    {
        D_801B2990 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8008A460(void)
{
extern void func_8008A4AC__for_func_8008A460(void) __asm__("func_8008A4AC");
extern s16 D_800D9268[];
extern s32 D_801B2994;
extern s32 D_801B2990;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_80182D58;

    D_800D9268[151] = 8;
    D_800D9268[149] = 0;
    D_801B2994 = 0x10;
    D_801B2990 += 1;
    func_8008A4AC__for_func_8008A460();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8008A4AC(void)
{
extern void func_8008A4AC(void);
extern s16 D_800D9268[];
extern s32 D_801B2994;
extern s32 D_801B2990;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_80182D58;

    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_80182D58, 0xF, 0x2, 0);
    if (--D_801B2994 == 0)
    {
        D_801B2990 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008A528(void)
{
extern s32 D_801B2990;

    D_801B2990 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008A540(s32 arg0)
{
extern u32 D_801B2998;
extern s32 D_801B299C;
extern void (*D_800D5C50[])(void);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32 D_80139280;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2998 = 1;
        D_801B299C = 1;
        return 1;
    }

    if (D_801B2998 < 0x6)
    {
        D_800D5C50[D_801B2998]();
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
void func_8008A5B8(void)
{
extern u32 D_801B2998;
extern s32 D_801B299C;
extern void (*D_800D5C50[])(void);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32 D_80139280;

    D_801B2998 = 1;
    D_801B299C = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_8008A5D0(void)
{
extern u32 D_801B2998;
extern s32 D_801B299C;
extern void (*D_800D5C50[])(void);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32 D_80139280;

    func_8006A2FC(D_800D9688, D_80139A48, 0x18, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B299C == 0)
    {
        D_801B2998 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_8008A654(void)
{
extern void func_8008A69C__for_func_8008A654(void) __asm__("func_8008A69C");
extern s32* D_80139280;
extern s32 D_801B299C;
extern s32 D_801B2998;
extern u8 D_800D9688[];
extern u8 D_80139A48[];

    D_801B299C = 0x20;
    D_80139280[35] = -1;
    D_801B2998 += 1;
    func_8008A69C__for_func_8008A654();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_8008A69C(void)
{
extern void func_8008A69C(void);
extern s32* D_80139280;
extern s32 D_801B299C;
extern s32 D_801B2998;
extern u8 D_800D9688[];
extern u8 D_80139A48[];

    func_8006A2FC(D_800D9688, D_80139A48, 0x18, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B299C == 0)
    {
        D_801B2998 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008A720(void)
{
extern s32 D_801B2998;

    D_801B2998 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008A738(s32 arg0)
{
extern u32 D_801B29A0;
extern s32 D_801B29A4;
extern void (*D_800D5C68[])(void);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_80139280;

    s32 result;

    if (arg0 != 0)
    {
        D_801B29A0 = 1;
        D_801B29A4 = 1;
        return 1;
    }

    if (D_801B29A0 < 0x4)
    {
        D_800D5C68[D_801B29A0]();
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
void func_8008A7B0(void)
{
extern u32 D_801B29A0;
extern s32 D_801B29A4;
extern void (*D_800D5C68[])(void);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_80139280;

    D_801B29A0 = 1;
    D_801B29A4 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_8008A7C8(void)
{
extern u32 D_801B29A0;
extern s32 D_801B29A4;
extern void (*D_800D5C68[])(void);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_80139280;

    func_8006A2FC(D_800D95D8, D_80139A28, 0xC, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B29A4 == 0)
    {
        D_801B29A0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8008A84C(void)
{
extern s32 D_801B29A0;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B29CC;
extern s32 D_801B29C8;

    D_801B29A0 += 1;
}
