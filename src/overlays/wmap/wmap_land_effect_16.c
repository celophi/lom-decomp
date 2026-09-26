#include "wmap_main.h"
#include "wmap_land_effect_16.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"

/** @brief World-map tile and its cached neighboring layout data. */
typedef struct
{
    s32 tile;
    s16 field_04;
    s16 field_06;
    u8 neighbors[32];
} WmapTile;

void func_80098CD8(void);
void func_800995DC(void);
void func_80097D24(void);
void func_80097D60(void);
s32 func_80097DF8(s32 arg0);
void func_80097DA4(void);
void func_80097DE0(void);
s32 func_800983E8(s32 arg0);
s32 func_80098C48(s32 arg0);
s32 func_80098998(s32 arg0);
s32 func_80099120(s32 arg0);
s32 func_800992C4(s32 arg0);
s32 func_8009858C(s32 arg0);
s32 func_80098FC8(s32 arg0);
s32 func_8009954C(s32 arg0);
s32 func_800987F8(s32 arg0);
s32 func_80098AF0(s32 arg0);
s32 func_80098E28(s32 arg0);
void func_800984F8(void);
void func_8009869C(void);
void func_80098764(void);
void func_80098904(void);
void func_80098DB4(void);
void func_80098F34(void);
void func_80099230(void);
void func_800993F0(void);
void func_800994B8(void);
void func_800996B0(void);

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80097624(void)
{
extern s32 D_801B2BD8;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2C0C;
extern s32 D_801B2C08;

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
        wmap_draw_model_default((s32)D_800DCF18, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE8);
        D_80182DE8 -= 0x2;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--D_801B2C0C == 0)
    {
        D_801B2C08 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80097724(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B2C14;
extern s32 D_801B2C10;

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
        wmap_draw_model_default(D_8011CF1C, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DEC);
        D_80182DEC -= 0x2;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    if (--D_801B2C14 == 0)
    {
        D_801B2C10 += 1;
    }
}

/** @brief Initialize four effect actors and their angular spacing. */
void func_80097824(void)
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
extern u8 D_8011F538[];
extern s32 D_801B2C18;
extern s32 D_801B2C1C;

    s32 i;
    s32 *descriptor;
    WmapConfigA *actor;

    D_801B2490 = D_80139258;
    descriptor = D_80139280;
    descriptor[0] = 1;
    descriptor[1] = 1;
    descriptor[2] = 1;
    D_80139280[3] = 0;
    D_80139280[4] = 1000;
    D_80139280[5] = 96;
    D_80139280[6] = 8;
    D_80139280[7] = 255;
    D_80139280[8] = 10;
    D_80139280[9] = 19;
    for (i = 20; i < 80; i++)
    {
        D_801AFBD0[i].state = 0;
    }
    for (i = 20; i < 80; i += 15)
    {
        actor = &D_800D9268[i];
        D_80139988[i].resource = D_8011F538;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_02 = 0;
        actor->field_0E = 0;
        actor->field_26 = 2;
        actor->field_22 = 128;
        actor->field_24 = 128;
        D_801AFBD0[i].state = 1;
        D_801AFBD0[i].z = 0;
        D_801AFBD0[i].angle = D_80139280[3];
        D_801AFBD0[i].field_0E = 0;
        D_80139280[3] += 1024;
    }
    D_801B2C1C = 48;
    D_801B2C18++;
    func_80098CD8();
}

/** @brief Advance the world-map effect and its sequence state. */
void func_800979A8(void)
{
/** @brief World-map vector; only the third halfword is changed here. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    s16 field_04;
    s16 field_06;
} WmapVector;

extern s32 D_8011CF24;
extern s32 D_8013923C;
extern WmapVector D_8013B238;
extern s32 g_wmap_camera_translation[];
extern s32 D_80182DF0;
extern s32 D_801B2C28;
extern s32 D_801B2C2C;

    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model_default(D_8011CF24, D_8013923C, 4, 0x35, 0x7800, 0x1001, D_80182DF0);
    D_8013B238.field_04 = (u16) (D_8013B238.field_04 + 0x10);
    intensity = D_80182DF0 + 2;
    D_80182DF0 = intensity;
    if (intensity >= 0x82)
    {
        D_80182DF0 = 0x81;
    }
    D_8013923C = (D_8013923C + 1) & 7;
    PopMatrix();
    remaining = D_801B2C2C - 1;
    D_801B2C2C = remaining;
    if (remaining == 0)
    {
        D_801B2C28 += 1;
    }
}

/** @brief Advance the world-map effect and its sequence state. */
void func_80097A94(void)
{
/** @brief World-map vector; only the third halfword is changed here. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    s16 field_04;
    s16 field_06;
} WmapVector;

extern s32 D_8011CF24;
extern s32 D_8013923C;
extern WmapVector D_8013B238;
extern s32 g_wmap_camera_translation[];
extern s32 D_80182DF0;
extern s32 D_801B2C28;
extern s32 D_801B2C2C;

    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model_default(D_8011CF24, D_8013923C, 4, 0x35, 0x7800, 0x1001, D_80182DF0);
    intensity = D_80182DF0 - 2;
    D_8013B238.field_04 = (u16) (D_8013B238.field_04 + 0x10);
    D_80182DF0 = intensity;
    if (intensity < 0)
    {
        D_80182DF0 = 0;
    }
    D_8013923C = (D_8013923C + 1) & 7;
    PopMatrix();
    remaining = D_801B2C2C - 1;
    D_801B2C2C = remaining;
    if (remaining == 0)
    {
        D_801B2C28 += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_80097B78(void)
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
extern u8 D_8011F538[];
extern s32 D_801B2C40;
extern s32 D_801B2C44;

    s32 i;

    D_801B0FD0 = 40;
    D_80139280[0x1F] = -1;
    D_80139280[0x20] = -4;
    D_80139280[0x21] = 0x20;
    D_80139280[0x22] = 0;
    D_80139280[0x23] = 2;
    D_80139280[0x24] = 0;
    D_80139280[0x25] = 0xC8;
    D_80139280[0x26] = 0x13;
    D_80139280[0x27] = 2;
    D_80139280[0x28] = 0x1F40;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + 200].field_00 = 0;
        D_80139988[i + 204].field_04 = D_8011F538;
    }
    D_801B2C44 = 80;
    D_801B2C40++;
    func_800995DC();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80097C50(s32 arg0)
{
extern u32 D_801B2BE0;
extern s32 D_801B2BE4;
extern void (*D_800D63B8[])(void);
extern s32 D_8013B20C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2BE0 = 1;
        D_801B2BE4 = 1;
        return 1;
    }

    if (D_801B2BE0 < 0x6)
    {
        D_800D63B8[D_801B2BE0]();
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
void func_80097CC8(void)
{
extern u32 D_801B2BE0;
extern s32 D_801B2BE4;
extern void (*D_800D63B8[])(void);
extern s32 D_8013B20C;

    D_801B2BE0 = 1;
    D_801B2BE4 = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80097CE0(void)
{
extern u32 D_801B2BE0;
extern s32 D_801B2BE4;
extern void (*D_800D63B8[])(void);
extern s32 D_8013B20C;

    wmap_start_sequence(wmap_run_land_focus);
    D_8013B20C = 1;
    D_801B2BE0 += 1;
    func_80097D24();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80097D24(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2BE0;

    if (D_8013B20C == 0)
    {
        D_801B2BE0 += 1;
        func_80097D60();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80097D60(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2BE0;
extern void func_80097D60(void);

    wmap_start_sequence(func_80097DF8);
    D_8013B20C = 1;
    D_801B2BE0 += 1;
    func_80097DA4();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80097DA4(void)
{
extern s32 D_801B2BE0;
extern s32 D_8013B20C;

    if (D_8013B20C == 0)
    {
        D_801B2BE0 += 1;
        func_80097DE0();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80097DE0(void)
{
extern s32 D_801B2BE0;
extern s32 D_8013B20C;
extern void func_80097DE0(void);

    D_801B2BE0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80097DF8(s32 arg0)
{
extern u32 D_801B2BE8;
extern s32 D_801B2BEC;
extern void (*D_800D63D0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2BE8 = 1;
        D_801B2BEC = 1;
        return 1;
    }

    if (D_801B2BE8 < 0x18)
    {
        D_800D63D0[D_801B2BE8]();
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
void func_80097E70(void)
{
extern u32 D_801B2BE8;
extern s32 D_801B2BEC;
extern void (*D_800D63D0[])(void);

    D_801B2BE8 = 1;
    D_801B2BEC = 1;
}

/** @brief World-map step handler: kick a sub-task and advance the step counter. */
void func_80097E88(void)
{
extern s32 D_8013B208;
extern s32 D_801B2BE8;
extern s32 D_801B2BEC;

    D_8013B208 = 1;
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 4;
    D_801B2BEC = 0x1E;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80097EDC(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/** @brief World-map step handler: register a callback and advance the step counter. */
void func_80097F10(void)
{
extern s32 D_801B2BE8;
extern s32 D_801B2BEC;

    wmap_play_sound(0x27, 0x80);
    wmap_start_sequence(func_800983E8);
    D_801B2BEC = 0x10;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80097F58(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80097F8C(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    wmap_start_sequence(func_80098C48);
    D_801B2BEC = 0x2;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80097FC8(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void func_80097FFC(void)
{
extern s32 D_801ADAE0;
extern s32 D_801B2BE8;
extern s32 D_801B2BEC;

    D_801ADAE0 = 1;
    D_801B2BEC = 0x28;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80098028(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009805C(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    wmap_start_sequence(func_80098998);
    D_801B2BEC = 0x4;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80098098(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800980CC(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    wmap_start_sequence(func_80099120);
    D_801B2BEC = 0x14;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80098108(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009813C(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    wmap_start_sequence(func_800992C4);
    D_801B2BEC = 0x1E;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80098178(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800981AC(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    wmap_start_sequence(func_8009858C);
    wmap_start_sequence(func_80098FC8);
    D_801B2BEC = 0x8;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800981F4(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80098228(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    wmap_start_sequence(func_8009954C);
    D_801B2BEC = 0xF;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80098264(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80098298(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    wmap_start_sequence(func_800987F8);
    D_801B2BEC = 0x80;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800982D4(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_80098308(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    wmap_start_sequence(func_80098AF0);
    wmap_start_sequence(func_80098E28);
    D_801B2BEC = 0x80;
    D_801B2BE8 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80098350(void)
{
extern s32 D_801B2BEC;
extern s32 D_801B2BE8;

    if (--D_801B2BEC == 0)
    {
        D_801B2BE8 += 1;
    }
}

/** @brief Mark the current world-map tile state and advance the sequence. */
void func_80098384(void)
{
extern s32 D_8013B20C;
extern WmapTile D_80139290[6][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2BE8;

    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].tile = D_8011D4FC | 0x100;
    D_801B2BE8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800983E8(s32 arg0)
{
extern u32 D_801B2BF0;
extern s32 D_801B2BF4;
extern void (*D_800D6430[])(void);
extern u8 D_8011F538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 g_wmap_focus_screen_position;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2BF0 = 1;
        D_801B2BF4 = 1;
        return 1;
    }

    if (D_801B2BF0 < 0x4)
    {
        D_800D6430[D_801B2BF0]();
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
void func_80098460(void)
{
extern u32 D_801B2BF0;
extern s32 D_801B2BF4;
extern void (*D_800D6430[])(void);
extern u8 D_8011F538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 g_wmap_focus_screen_position;

    D_801B2BF0 = 1;
    D_801B2BF4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80098478(void)
{
extern u32 D_801B2BF0;
extern s32 D_801B2BF4;
extern void (*D_800D6430[])(void);
extern u8 D_8011F538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 g_wmap_focus_screen_position;

    D_801399AC = D_8011F538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 0x10;
    *(s16*)&D_800D9318[0x22] = 0x81;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x24] = 1;
    D_801B2BF4 = 0x40;
    D_801B2BF0 += 1;
    func_800984F8();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800984F8(void)
{
extern u32 D_801B2BF0;
extern s32 D_801B2BF4;
extern void (*D_800D6430[])(void);
extern void func_800984F8(void);
extern u8 D_8011F538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 g_wmap_focus_screen_position;

    wmap_step_actor_animation(D_800D9318, D_801399A8);
    wmap_draw_actor_sprite(D_800D9318, g_wmap_focus_screen_position, 0x13, 0x2, 0);
    if (--D_801B2BF4 == 0)
    {
        D_801B2BF0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80098574(void)
{
extern s32 D_801B2BF0;

    D_801B2BF0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009858C(s32 arg0)
{
extern u32 D_801B2BF8;
extern s32 D_801B2BFC;
extern void (*D_800D6440[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2BF8 = 1;
        D_801B2BFC = 1;
        return 1;
    }

    if (D_801B2BF8 < 0x6)
    {
        D_800D6440[D_801B2BF8]();
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
void func_80098604(void)
{
extern u32 D_801B2BF8;
extern s32 D_801B2BFC;
extern void (*D_800D6440[])(void);

    D_801B2BF8 = 1;
    D_801B2BFC = 1;
}

/** @brief Initialize the world-map actor and advance the timed sequence. */
void func_8009861C(void)
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

extern WmapConfigA D_800D93F4;
extern u8 D_8011F538;
extern void *D_801399D4;
extern s32 D_801B2BF8;
extern s32 D_801B2BFC;

    D_801399D4 = &D_8011F538;
    D_800D93F4.field_06 = 0xF;
    D_800D93F4.field_02 = 0;
    D_800D93F4.field_0E = 1;
    D_800D93F4.field_10 = -1;
    D_800D93F4.field_26 = 2;
    D_800D93F4.field_22 = 0x81;
    D_800D93F4.field_24 = 1;
    D_801B2BFC = 0x64;
    D_801B2BF8 += 1;
    func_8009869C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009869C(void)
{
extern s16 D_800D93F4[];
extern s32 D_801B2BFC;
extern s32 D_801B2BF8;
extern u8 D_801399D0[];
extern s32 g_wmap_focus_screen_position;

    wmap_step_actor_animation(D_800D93F4, D_801399D0);
    wmap_draw_actor_sprite(D_800D93F4, g_wmap_focus_screen_position, 0x13, 0x2, 0);
    if (--D_801B2BFC == 0)
    {
        D_801B2BF8 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80098718(void)
{
extern s16 D_800D93F4[];
extern s32 D_801B2BFC;
extern s32 D_801B2BF8;
extern u8 D_801399D0[];
extern s32 g_wmap_focus_screen_position;

    D_800D93F4[17] = 0;
    D_800D93F4[19] = 2;
    D_801B2BFC = 0x40;
    D_801B2BF8 += 1;
    func_80098764();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80098764(void)
{
extern void func_80098764(void);
extern s16 D_800D93F4[];
extern s32 D_801B2BFC;
extern s32 D_801B2BF8;
extern u8 D_801399D0[];
extern s32 g_wmap_focus_screen_position;

    wmap_step_actor_animation(D_800D93F4, D_801399D0);
    wmap_draw_actor_sprite(D_800D93F4, g_wmap_focus_screen_position, 0x13, 0x2, 0);
    if (--D_801B2BFC == 0)
    {
        D_801B2BF8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800987E0(void)
{
extern s32 D_801B2BF8;

    D_801B2BF8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800987F8(s32 arg0)
{
extern u32 D_801B2C00;
extern s32 D_801B2C04;
extern void (*D_800D6458[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2C00 = 1;
        D_801B2C04 = 1;
        return 1;
    }

    if (D_801B2C00 < 0x4)
    {
        D_800D6458[D_801B2C00]();
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
void func_80098870(void)
{
extern u32 D_801B2C00;
extern s32 D_801B2C04;
extern void (*D_800D6458[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;

    D_801B2C00 = 1;
    D_801B2C04 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80098888(void)
{
extern u32 D_801B2C00;
extern s32 D_801B2C04;
extern void (*D_800D6458[])(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;

    D_801399B4 = D_8011D538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 1;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x22] = 0x80;
    *(s16*)&D_800D9344[0x24] = 0;
    D_801B2C04 = 0x81;
    D_801B2C00 += 1;
    func_80098904();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80098904(void)
{
extern u32 D_801B2C00;
extern s32 D_801B2C04;
extern void (*D_800D6458[])(void);
extern void func_80098904(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;

    wmap_step_actor_animation(D_800D9344, D_801399B0);
    wmap_draw_actor_sprite(D_800D9344, g_wmap_focus_screen_position, 0x18, 0x8, 0);
    if (--D_801B2C04 == 0)
    {
        D_801B2C00 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80098980(void)
{
extern s32 D_801B2C00;

    D_801B2C00 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80098998(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2C08;
extern s32 D_801B2C0C;
extern void (*D_800D6468[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2C08 = 1;
        D_801B2C0C = 1;
        return 1;
    }

    if (D_801B2C08 < 0x4)
    {
        D_800D6468[D_801B2C08]();
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
void func_80098A10(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2C08;
extern s32 D_801B2C0C;
extern void (*D_800D6468[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;

    D_801B2C08 = 1;
    D_801B2C0C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80098A28(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2C08;
extern s32 D_801B2C0C;
extern void (*D_800D6468[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;

    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B2C0C = 0x40;
    D_801B2C08 += 1;
    func_80097624();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80098AD8(void)
{
extern s32 D_801B2C08;

    D_801B2C08 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80098AF0(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2C10;
extern s32 D_801B2C14;
extern void (*D_800D6478[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2C10 = 1;
        D_801B2C14 = 1;
        return 1;
    }

    if (D_801B2C10 < 0x4)
    {
        D_800D6478[D_801B2C10]();
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
void func_80098B68(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2C10;
extern s32 D_801B2C14;
extern void (*D_800D6478[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;

    D_801B2C10 = 1;
    D_801B2C14 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80098B80(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2C10;
extern s32 D_801B2C14;
extern void (*D_800D6478[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;

    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B2C14 = 0x40;
    D_801B2C10 += 1;
    func_80097724();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80098C30(void)
{
extern s32 D_801B2C10;

    D_801B2C10 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80098C48(s32 arg0)
{
extern u32 D_801B2C18;
extern s32 D_801B2C1C;
extern void (*D_800D6488[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2C18 = 1;
        D_801B2C1C = 1;
        return 1;
    }

    if (D_801B2C18 < 0x6)
    {
        D_800D6488[D_801B2C18]();
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
void func_80098CC0(void)
{
extern u32 D_801B2C18;
extern s32 D_801B2C1C;
extern void (*D_800D6488[])(void);

    D_801B2C18 = 1;
    D_801B2C1C = 1;
}

/** @brief World-map step: kick off a descriptor animation, then tick the sub-counter. */
void func_80098CD8(void)
{
extern void *D_80139280;
extern s32 D_801B2C1C;
extern s32 D_801B2C18;

    func_8006BC44(0x14, 0x3C, D_80139280, 1);
    if (--D_801B2C1C == 0)
    {
        D_801B2C18 += 1;
    }
}

/** @brief Reset four actor configurations and begin a 16-tick sequence step. */
void func_80098D34(void)
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

extern WmapConfigA D_800D9268[];
extern s32 *D_80139280;
extern s32 D_801B2C18;
extern s32 D_801B2C1C;

    s32 index;

    D_80139280[0] = -1;
    D_80139280[4] = 0;
    D_80139280[5] = 0;
    for (index = 20; index < 80; index += 15)
    {
        D_800D9268[index].field_22 = 0;
        D_800D9268[index].field_26 = 8;
    }
    D_801B2C1C = 0x10;
    D_801B2C18 += 1;
    func_80098DB4();
}

/** @brief World-map step: kick off a descriptor animation, then tick the sub-counter. */
void func_80098DB4(void)
{
extern void *D_80139280;
extern s32 D_801B2C1C;
extern s32 D_801B2C18;

    func_8006BC44(0x14, 0x3C, D_80139280, 1);
    if (--D_801B2C1C == 0)
    {
        D_801B2C18 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80098E10(void)
{
extern s32 D_801B2C18;

    D_801B2C18 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80098E28(s32 arg0)
{
extern u32 D_801B2C20;
extern s32 D_801B2C24;
extern void (*D_800D64A0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2C20 = 1;
        D_801B2C24 = 1;
        return 1;
    }

    if (D_801B2C20 < 0x4)
    {
        D_800D64A0[D_801B2C20]();
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
void func_80098EA0(void)
{
extern u32 D_801B2C20;
extern s32 D_801B2C24;
extern void (*D_800D64A0[])(void);

    D_801B2C20 = 1;
    D_801B2C24 = 1;
}

/** @brief Initialize the actor configuration and begin a 129-tick sequence step. */
void func_80098EB8(void)
{
/** @brief World-map actor configuration with the original field layout. */
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
extern u8 D_8011D538[];
extern u8 *D_801399BC;
extern s32 D_801B2C20;
extern s32 D_801B2C24;

    D_801399BC = D_8011D538;
    D_800D9370.field_06 = 0xF;
    D_800D9370.field_0E = 1;
    D_800D9370.field_10 = -1;
    D_800D9370.field_26 = 1;
    D_800D9370.field_02 = 0;
    D_800D9370.field_22 = 0x80;
    D_800D9370.field_24 = 0x80;
    D_801B2C24 = 0x81;
    D_801B2C20 += 1;
    func_80098F34();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80098F34(void)
{
extern s32 D_801B2C20;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 g_wmap_focus_screen_position;
extern s32 D_801B2C24;

    wmap_step_actor_animation(D_800D9370, D_801399B8);
    wmap_draw_actor_sprite(D_800D9370, g_wmap_focus_screen_position, 0x18, 0x8, 0);
    if (--D_801B2C24 == 0)
    {
        D_801B2C20 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80098FB0(void)
{
extern s32 D_801B2C20;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 g_wmap_focus_screen_position;
extern s32 D_801B2C24;

    D_801B2C20 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80098FC8(s32 arg0)
{
extern u32 D_801B2C28;
extern s32 D_801B2C2C;
extern void (*D_800D64B0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2C28 = 1;
        D_801B2C2C = 1;
        return 1;
    }

    if (D_801B2C28 < 0x6)
    {
        D_800D64B0[D_801B2C28]();
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
void func_80099040(void)
{
extern u32 D_801B2C28;
extern s32 D_801B2C2C;
extern void (*D_800D64B0[])(void);

    D_801B2C28 = 1;
    D_801B2C2C = 1;
}

/** @brief Restore effect data, clear two flags, and begin a 100-tick sequence step. */
void func_80099058(void)
{
/** @brief Eight bytes of world-map effect state copied together. */
typedef struct
{
    u8 bytes[8];
} WmapBlk8;

extern s32 D_8013923C;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B238;
extern s32 D_80182DF0;
extern s32 D_801B2C28;
extern s32 D_801B2C2C;

    D_8013B238 = D_80139258;
    D_80182DF0 = 0;
    D_8013923C = 0;
    D_801B2C2C = 0x64;
    D_801B2C28 += 1;
    func_800979A8();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800990D0(void)
{
extern s32 D_801B2C28;
extern s32 D_801B2C2C;

    D_801B2C2C = 0x40;
    D_801B2C28 += 1;
    func_80097A94();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80099108(void)
{
extern s32 D_801B2C28;
extern s32 D_801B2C2C;

    D_801B2C28 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80099120(s32 arg0)
{
extern u32 D_801B2C30;
extern s32 D_801B2C34;
extern void (*D_800D64C8[])(void);
extern u8 D_80121538[];
extern u8* D_801399DC;
extern u8 D_800D9420[];
extern u8 D_801399D8[];
extern s32 g_wmap_focus_screen_position;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2C30 = 1;
        D_801B2C34 = 1;
        return 1;
    }

    if (D_801B2C30 < 0x4)
    {
        D_800D64C8[D_801B2C30]();
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
void func_80099198(void)
{
extern u32 D_801B2C30;
extern s32 D_801B2C34;
extern void (*D_800D64C8[])(void);
extern u8 D_80121538[];
extern u8* D_801399DC;
extern u8 D_800D9420[];
extern u8 D_801399D8[];
extern s32 g_wmap_focus_screen_position;

    D_801B2C30 = 1;
    D_801B2C34 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800991B0(void)
{
extern u32 D_801B2C30;
extern s32 D_801B2C34;
extern void (*D_800D64C8[])(void);
extern u8 D_80121538[];
extern u8* D_801399DC;
extern u8 D_800D9420[];
extern u8 D_801399D8[];
extern s32 g_wmap_focus_screen_position;

    D_801399DC = D_80121538;
    D_800D9420[0x6] = 0xF;
    *(s16*)&D_800D9420[0x10] = -1;
    *(s16*)&D_800D9420[0x26] = 4;
    *(s16*)&D_800D9420[0x22] = 0x81;
    *(s16*)&D_800D9420[0x2] = 0;
    *(s16*)&D_800D9420[0xE] = 0;
    *(s16*)&D_800D9420[0x24] = 1;
    D_801B2C34 = 0x3C;
    D_801B2C30 += 1;
    func_80099230();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80099230(void)
{
extern u32 D_801B2C30;
extern s32 D_801B2C34;
extern void (*D_800D64C8[])(void);
extern void func_80099230(void);
extern u8 D_80121538[];
extern u8* D_801399DC;
extern u8 D_800D9420[];
extern u8 D_801399D8[];
extern s32 g_wmap_focus_screen_position;

    wmap_step_actor_animation(D_800D9420, D_801399D8);
    wmap_draw_actor_sprite(D_800D9420, g_wmap_focus_screen_position, 0x13, 0x2, 0);
    if (--D_801B2C34 == 0)
    {
        D_801B2C30 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800992AC(void)
{
extern s32 D_801B2C30;

    D_801B2C30 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800992C4(s32 arg0)
{
extern u32 D_801B2C38;
extern s32 D_801B2C3C;
extern void (*D_800D64D8[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2C38 = 1;
        D_801B2C3C = 1;
        return 1;
    }

    if (D_801B2C38 < 0x6)
    {
        D_800D64D8[D_801B2C38]();
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
void func_8009933C(void)
{
extern u32 D_801B2C38;
extern s32 D_801B2C3C;
extern void (*D_800D64D8[])(void);

    D_801B2C38 = 1;
    D_801B2C3C = 1;
}

/** @brief Initialize the actor and its screen coordinates, then advance the sequence. */
void func_80099354(void)
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

extern WmapConfigA D_800D944C;
extern s32 g_wmap_focus_screen_position;
extern u8 D_80121538[];
extern u8 *D_801399E4;
extern s16 D_80182D60[];
extern s32 D_801B2C38;
extern s32 D_801B2C3C;

    s16 screen_y;

    D_801399E4 = D_80121538;
    D_800D944C.field_06 = 0xF;
    D_800D944C.field_02 = 0;
    D_800D944C.field_0E = 1;
    D_800D944C.field_10 = -1;
    D_800D944C.field_26 = 0x20;
    D_800D944C.field_22 = 0x81;
    D_800D944C.field_24 = 1;
    D_801B2C3C = 0x80;
    D_80182D60[0] = (u16) g_wmap_focus_screen_position;
    screen_y = g_wmap_focus_screen_position - 0x38;
    D_80182D60[1] = screen_y;
    D_801B2C38 += 1;
    func_800993F0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800993F0(void)
{
extern s16 D_800D944C[];
extern s32 D_801B2C3C;
extern s32 D_801B2C38;
extern u8 D_801399E0[];
extern s32 D_80182D60;

    wmap_step_actor_animation(D_800D944C, D_801399E0);
    wmap_draw_actor_sprite(D_800D944C, D_80182D60, 0x13, 0x2, 0);
    if (--D_801B2C3C == 0)
    {
        D_801B2C38 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_8009946C(void)
{
extern s16 D_800D944C[];
extern s32 D_801B2C3C;
extern s32 D_801B2C38;
extern u8 D_801399E0[];
extern s32 D_80182D60;

    D_800D944C[17] = 0;
    D_800D944C[19] = 2;
    D_801B2C3C = 0x40;
    D_801B2C38 += 1;
    func_800994B8();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800994B8(void)
{
extern void func_800994B8(void);
extern s16 D_800D944C[];
extern s32 D_801B2C3C;
extern s32 D_801B2C38;
extern u8 D_801399E0[];
extern s32 D_80182D60;

    wmap_step_actor_animation(D_800D944C, D_801399E0);
    wmap_draw_actor_sprite(D_800D944C, D_80182D60, 0x13, 0x2, 0);
    if (--D_801B2C3C == 0)
    {
        D_801B2C38 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80099534(void)
{
extern s32 D_801B2C38;

    D_801B2C38 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009954C(s32 arg0)
{
extern u32 D_801B2C40;
extern s32 D_801B2C44;
extern void (*D_800D64F0[])(void);
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_80139280;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2C40 = 1;
        D_801B2C44 = 1;
        return 1;
    }

    if (D_801B2C40 < 0x6)
    {
        D_800D64F0[D_801B2C40]();
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
void func_800995C4(void)
{
extern u32 D_801B2C40;
extern s32 D_801B2C44;
extern void (*D_800D64F0[])(void);
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_80139280;

    D_801B2C40 = 1;
    D_801B2C44 = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_800995DC(void)
{
extern u32 D_801B2C40;
extern s32 D_801B2C44;
extern void (*D_800D64F0[])(void);
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_80139280;

    func_8006AEE0();
    func_8006A2FC(D_800DB578, D_80139FE8, 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2C44 == 0)
    {
        D_801B2C40 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80099668(void)
{
extern s32* D_80139280;
extern s32 D_801B2C44;
extern s32 D_801B2C40;
extern u8 D_800DB578[];
extern u8 D_80139FE8[];

    D_801B2C44 = 0x20;
    D_80139280[35] = -1;
    D_801B2C40 += 1;
    func_800996B0();
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_800996B0(void)
{
extern void func_800996B0(void);
extern s32* D_80139280;
extern s32 D_801B2C44;
extern s32 D_801B2C40;
extern u8 D_800DB578[];
extern u8 D_80139FE8[];

    func_8006AEE0();
    func_8006A2FC(D_800DB578, D_80139FE8, 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2C44 == 0)
    {
        D_801B2C40 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009973C(void)
{
extern s32 D_801B2C40;

    D_801B2C40 += 1;
}
