#include "wmap_main.h"
#include "wmap_land_effect_08.h"
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
void func_80093448(void)
{
extern s32 D_801B2B20;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2B4C;
extern s32 D_801B2B48;

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
    if (--D_801B2B4C == 0)
    {
        D_801B2B48 += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void func_80093548(void)
{
extern s32 D_801B2478[];
extern SVECTOR D_801B24A8;
extern VECTOR D_8011CF60;
extern s32 D_80182DEC;
extern s32 D_8011CF1C;
extern s32 D_801B2B54;
extern s32 D_801B2B50;

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
    if (--D_801B2B54 == 0)
    {
        D_801B2B50 += 1;
    }
}

/**
 * @brief World-map step handler: draw two overlaid actor sprites within a matrix push,
 *        ramp the shared size up to a cap, then countdown-advance the step.
 */
void func_80093648(void)
{
extern u8 g_wmap_camera_translation[];
extern u8 D_801B2490[];
extern u8 D_801B2498[];
extern s32 D_8011CF24;
extern s32 D_801B2468;
extern s32 D_801B2B5C;
extern s32 D_801B2B58;

    s32 value;

    PushMatrix();
    func_8006CFA8(g_wmap_camera_translation, D_801B2490);
    func_8006CD98(D_8011CF24, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    ((u16*)D_801B2490)[2] += 0x20;
    func_8006CFA8(g_wmap_camera_translation, D_801B2498);
    func_8006CD98(D_8011CF24, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    ((u16*)D_801B2498)[2] += 0xC;
    PopMatrix();
    value = D_801B2468 + 2;
    D_801B2468 = value;
    if (value >= 0x41)
    {
        D_801B2468 = 0x40;
    }
    if (--D_801B2B5C == 0)
    {
        D_801B2B58 += 1;
    }
}

/**
 * @brief World-map step handler: draw two frames of the animated actor, scroll each
 *        sub-field, decay the shared frame index with a floor, then advance the step.
 */
void func_80093784(void)
{
extern u8 g_wmap_camera_translation[];
extern u8 D_801B2490[];
extern u8 D_801B2498[];
extern s32 D_8011CF24;
extern s32 D_801B2468;
extern s32 D_801B2B58;
extern s32 D_801B2B5C;

    PushMatrix();
    func_8006CFA8(g_wmap_camera_translation, D_801B2490);
    func_8006CD98(D_8011CF24, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    ((u16*)D_801B2490)[2] += 0x20;
    func_8006CFA8(g_wmap_camera_translation, D_801B2498);
    func_8006CD98(D_8011CF24, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    ((u16*)D_801B2498)[2] += 0xC;
    PopMatrix();
    D_801B2468 -= 4;
    if (D_801B2468 < 0)
    {
        D_801B2468 = 0;
    }
    if (--D_801B2B5C == 0)
    {
        D_801B2B58 += 1;
    }
}

/**
 * @brief Reset a range of world-map actor slots and kick the next handler.
 * @note Clears three parallel slot arrays for indices [0x64, 0x6C), seeds each
 *       actor's default fields, then advances the shared step counters.
 */
void func_800938B8(void)
{
extern u8 D_800D9268[];
extern u8 D_80139988[];
extern u8 D_801AFBD0[];
extern s32 D_80121538;
extern s32 D_80182DF0;
extern s32 D_800DCEA8;
extern s32 D_800D9150;
extern s32 D_801B2B60;
extern s32 D_801B2B64;
extern void func_80094F10__for_func_800938B8(void) __asm__("func_80094F10");

    s32 i;
    u8* pa;
    u8* pb;

    D_80182DF0 = 1;
    D_800DCEA8 = 1;
    for (i = 0x64; i < 0x6C; i++)
    {
        *(s16*)(D_801AFBD0 + i * 0x14) = 0;
        pb = D_80139988 + i * 0x8;
        *(s32*)(pb + 0x4) = (s32)&D_80121538;
        pa = D_800D9268 + i * 0x2C;
        *(s16*)(pa + 0x2) = 0;
        *(s8*)(pa + 0x6) = 0xF;
        *(s16*)(pa + 0xE) = 0;
        *(s16*)(pa + 0x10) = -1;
    }
    D_800D9150 = 4;
    D_801B2B64 = 0x10;
    D_801B2B60 += 1;
    func_80094F10__for_func_800938B8();
}

/** @brief Initialize randomized actors along a cosine depth curve. */
void func_80093978(void)
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

extern WmapConfigA D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern u8 D_8011D538[];
extern s32 D_801B2B70;
extern s32 D_801B2B74;
extern s32 rand(void);
extern s32 ccos(s32);
extern void func_80093AE4__for_func_80093978(void) __asm__("func_80093AE4");

    s32 i;
    WmapConfigA *actor;
    WmapMotion *motion;
    s32 field_value;

    i = 150;
    for (; i < 180; i++)
    {
        actor = &D_800D9268[i];
        D_80139988[i].resource = D_8011D538;
        field_value = 1;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_26 = 2;
        actor->field_02 = 0;
        actor->field_0E = field_value;
        actor->field_22 = 129;
        actor->field_24 = field_value;
        motion = &D_801AFBD0[i];
        motion->angle = rand() & 4095;
        motion->field_0E = (i - 150) * 4;
        motion->z = ccos(2048 - (((i - 150) * 1024) / 30)) * 120;
        motion->x = ((rand() * 80) >> 15) + 40;
    }
    D_801B2B74 = 64;
    D_801B2B70++;
    func_80093AE4__for_func_80093978();
}

/** @brief Project and draw the world-map star field, spinning each entry each frame. */
void func_80093AE4(void)
{
/** @brief World-map orbiting star: polar position, spin angle, and radius. */
typedef struct
{
    s16 unk00;
    s16 angle;
    u16 delta;
    s16 unk06;
    s32 radius;
    s16 unk0C;
    u16 unk0E;
    s16 unk10;
    s16 unk12;
} WmapStar;

/** @brief World-map draw record; only used opaquely by the primitive helpers. */
typedef struct
{
    u8 pad[0x2C];
} WmapDraw;

extern WmapStar D_801AFBD0[];
extern WmapDraw D_800D9268[];
extern u8 D_80139988[];
extern s32 D_801B2B70;
extern s32 D_801B2B74;

    SVECTOR position;
    s32 screen;
    s32 i;

    for (i = 0x96; i < 0xB4; i++)
    {
        position.vx = ((D_801AFBD0[i].radius >> 6) * (ccos(D_801AFBD0[i].angle) >> 6)) >> 0xC;
        position.vy = ((D_801AFBD0[i].radius >> 6) * (csin(D_801AFBD0[i].angle) >> 6)) >> 0xC;
        position.vz = D_801AFBD0[i].unk0E;
        gte_ldv0(&position);
        gte_rtps();
        wmap_step_actor_animation(&D_800D9268[i], &D_80139988[i * 8]);
        gte_stsxy(&screen);
        if (D_801AFBD0[i].angle != 0)
        {
            wmap_draw_actor_sprite(&D_800D9268[i], screen, 0xF, 4, 0);
        }
        else
        {
            wmap_draw_actor_sprite(&D_800D9268[i], screen, 0xF, 0, 0);
        }
        D_801AFBD0[i].angle = ((u16)D_801AFBD0[i].angle + D_801AFBD0[i].delta) & 0xFFF;
    }
    if (--D_801B2B74 == 0)
    {
        D_801B2B70 += 1;
    }
}

/** @brief Project and draw the world-map star field, spinning each entry each frame. */
void func_80093C48(void)
{
/** @brief World-map orbiting star: polar position, spin angle, and radius. */
typedef struct
{
    s16 unk00;
    s16 angle;
    u16 delta;
    s16 unk06;
    s32 radius;
    s16 unk0C;
    u16 unk0E;
    s16 unk10;
    s16 unk12;
} WmapStar;

/** @brief World-map draw record; only used opaquely by the primitive helpers. */
typedef struct
{
    u8 pad[0x2C];
} WmapDraw;

extern WmapStar D_801AFBD0[];
extern WmapDraw D_800D9268[];
extern u8 D_80139988[];
extern s32 D_801B2B70;
extern s32 D_801B2B74;

    SVECTOR position;
    s32 screen;
    s32 i;

    for (i = 0x96; i < 0xB4; i++)
    {
        position.vx = ((D_801AFBD0[i].radius >> 6) * (ccos(D_801AFBD0[i].angle) >> 6)) >> 0xC;
        position.vy = ((D_801AFBD0[i].radius >> 6) * (csin(D_801AFBD0[i].angle) >> 6)) >> 0xC;
        position.vz = D_801AFBD0[i].unk0E;
        gte_ldv0(&position);
        gte_rtps();
        wmap_step_actor_animation(&D_800D9268[i], &D_80139988[i * 8]);
        gte_stsxy(&screen);
        if (D_801AFBD0[i].angle != 0)
        {
            wmap_draw_actor_sprite(&D_800D9268[i], screen, 0xF, 4, 0);
        }
        else
        {
            wmap_draw_actor_sprite(&D_800D9268[i], screen, 0xF, 0, 0);
        }
        D_801AFBD0[i].angle = ((u16)D_801AFBD0[i].angle + D_801AFBD0[i].delta) & 0xFFF;
    }
    if (--D_801B2B74 == 0)
    {
        D_801B2B70 += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void func_80093DAC(void)
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
extern s32 D_801B2B78;
extern s32 D_801B2B7C;
extern void func_800955BC__for_func_80093DAC(void) __asm__("func_800955BC");

    s32 i;

    D_801B0FD0 = 10;
    D_80139280[0xB] = 1;
    D_80139280[0xC] = 4;
    D_80139280[0xD] = 512;
    D_80139280[0xE] = 2;
    D_80139280[0xF] = 2;
    D_80139280[0x10] = 96;
    D_80139280[0x11] = 200;
    D_80139280[0x12] = 15;
    D_80139280[0x13] = 0;
    D_80139280[0x14] = 12000;
    for (i = 0; i < 10; i++)
    {
        D_801AFBD0[i + D_80139280[0x11]].field_00 = 0;
        D_80139988[i + 204].field_04 = D_8011D538;
    }
    D_801B2B7C = 20;
    D_801B2B78++;
    func_800955BC__for_func_80093DAC();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80093E98(s32 arg0)
{
extern u32 D_801B2B28;
extern s32 D_801B2B2C;
extern void (*D_800D6150[])(void);
extern void func_80093F6C__for_func_80093E98(void) __asm__("func_80093F6C");
extern s32 D_8013B20C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B28 = 1;
        D_801B2B2C = 1;
        return 1;
    }

    if (D_801B2B28 < 0x6)
    {
        D_800D6150[D_801B2B28]();
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
void func_80093F10(void)
{
extern u32 D_801B2B28;
extern s32 D_801B2B2C;
extern void (*D_800D6150[])(void);
extern void func_80093F6C__for_func_80093F10(void) __asm__("func_80093F6C");
extern s32 D_8013B20C;

    D_801B2B28 = 1;
    D_801B2B2C = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80093F28(void)
{
extern u32 D_801B2B28;
extern s32 D_801B2B2C;
extern void (*D_800D6150[])(void);
extern void func_80093F6C__for_func_80093F28(void) __asm__("func_80093F6C");
extern s32 D_8013B20C;

    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2B28 += 1;
    func_80093F6C__for_func_80093F28();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80093F6C(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2B28;
extern void func_80093FA8__for_func_80093F6C(void) __asm__("func_80093FA8");
extern void func_80094040__for_func_80093F6C(void) __asm__("func_80094040");
extern void func_80093FEC__for_func_80093F6C(void) __asm__("func_80093FEC");

    if (D_8013B20C == 0)
    {
        D_801B2B28 += 1;
        func_80093FA8__for_func_80093F6C();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void func_80093FA8(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2B28;
extern void func_80093FA8(void);
extern void func_80094040__for_func_80093FA8(void) __asm__("func_80094040");
extern void func_80093FEC__for_func_80093FA8(void) __asm__("func_80093FEC");

    func_8006CAC0(func_80094040__for_func_80093FA8);
    D_8013B20C = 1;
    D_801B2B28 += 1;
    func_80093FEC__for_func_80093FA8();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_80093FEC(void)
{
extern s32 D_801B2B28;
extern s32 D_8013B20C;
extern void func_80094028__for_func_80093FEC(void) __asm__("func_80094028");

    if (D_8013B20C == 0)
    {
        D_801B2B28 += 1;
        func_80094028__for_func_80093FEC();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80094028(void)
{
extern s32 D_801B2B28;
extern s32 D_8013B20C;
extern void func_80094028(void);

    D_801B2B28 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80094040(s32 arg0)
{
extern u32 D_801B2B30;
extern s32 D_801B2B34;
extern void (*D_800D6168[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B30 = 1;
        D_801B2B34 = 1;
        return 1;
    }

    if (D_801B2B30 < 0x18)
    {
        D_800D6168[D_801B2B30]();
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
void func_800940B8(void)
{
extern u32 D_801B2B30;
extern s32 D_801B2B34;
extern void (*D_800D6168[])(void);

    D_801B2B30 = 1;
    D_801B2B34 = 1;
}

/**
 * @brief Set up a tinted world-map sub-scene and register its handler.
 */
void func_800940D0(void)
{
extern s32 D_8013B208;
extern s32 D_801B2B30;
extern s32 D_801B2B34;
extern void func_800951B0__for_func_800940D0(void) __asm__("func_800951B0");

    D_8013B208 = 1;
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 9;
    wmap_play_sound(0x2A, 0x80);
    func_8006CAC0(func_800951B0__for_func_800940D0);
    D_801B2B34 = 0x14;
    D_801B2B30 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009413C(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_80095420__for_func_8009413C(void) __asm__("func_80095420");

    if (--D_801B2B34 == 0)
    {
        D_801B2B30 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80094170(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_80095420__for_func_80094170(void) __asm__("func_80095420");

    func_8006CAC0(func_80095420__for_func_80094170);
    D_801B2B34 = 0x14;
    D_801B2B30 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800941AC(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_80094A80__for_func_800941AC(void) __asm__("func_80094A80");

    if (--D_801B2B34 == 0)
    {
        D_801B2B30 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800941E0(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_80094A80__for_func_800941E0(void) __asm__("func_80094A80");

    func_8006CAC0(func_80094A80__for_func_800941E0);
    D_801B2B34 = 0x2;
    D_801B2B30 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009421C(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;

    if (--D_801B2B34 == 0)
    {
        D_801B2B30 += 1;
    }
}

/** @brief Set world-map color and flags, register a callback, and begin a 20-tick delay. */
void func_80094250(void)
{
extern s32 D_80139244;
extern s32 D_801ADAE0;
extern s32 D_801B2B30;
extern s32 D_801B2B34;
extern void func_80094674__for_func_80094250(void) __asm__("func_80094674");

    wmap_start_map_tint(0x561030);
    func_8006CAC0(&func_80094674__for_func_80094250);
    D_80139244 = 1;
    g_wmap_backdrop_target_level = 4;
    D_801ADAE0 = 1;
    D_801B2B34 = 0x14;
    D_801B2B30 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800942B8(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_80094D2C__for_func_800942B8(void) __asm__("func_80094D2C");

    if (--D_801B2B34 == 0)
    {
        D_801B2B30 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800942EC(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_80094D2C__for_func_800942EC(void) __asm__("func_80094D2C");

    func_8006CAC0(func_80094D2C__for_func_800942EC);
    D_801B2B34 = 0x14;
    D_801B2B30 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80094328(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_80094E80__for_func_80094328(void) __asm__("func_80094E80");

    if (--D_801B2B34 == 0)
    {
        D_801B2B30 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009435C(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_80094E80__for_func_8009435C(void) __asm__("func_80094E80");

    func_8006CAC0(func_80094E80__for_func_8009435C);
    D_801B2B34 = 0x28;
    D_801B2B30 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80094398(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_8009552C__for_func_80094398(void) __asm__("func_8009552C");

    if (--D_801B2B34 == 0)
    {
        D_801B2B30 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800943CC(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_8009552C__for_func_800943CC(void) __asm__("func_8009552C");

    func_8006CAC0(func_8009552C__for_func_800943CC);
    D_801B2B34 = 0x14;
    D_801B2B30 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80094408(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_80094BD8__for_func_80094408(void) __asm__("func_80094BD8");

    if (--D_801B2B34 == 0)
    {
        D_801B2B30 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_8009443C(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_80094BD8__for_func_8009443C(void) __asm__("func_80094BD8");

    func_8006CAC0(func_80094BD8__for_func_8009443C);
    D_801B2B34 = 0x2;
    D_801B2B30 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80094478(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;

    if (--D_801B2B34 == 0)
    {
        D_801B2B30 += 1;
    }
}

/** @brief Set the drawing color and world-map values, then start a two-tick delay. */
void func_800944AC(void)
{
extern s32 D_80139244;
extern s32 D_801B2B30;
extern s32 D_801B2B34;

    g_wmap_backdrop_target_level = 8;
    wmap_start_map_tint(0x562056);
    D_80139244 = 0;
    D_801B2B34 = 2;
    D_801B2B30 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800944FC(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_800948E0__for_func_800944FC(void) __asm__("func_800948E0");

    if (--D_801B2B34 == 0)
    {
        D_801B2B30 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80094530(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_800948E0__for_func_80094530(void) __asm__("func_800948E0");

    func_8006CAC0(func_800948E0__for_func_80094530);
    D_801B2B34 = 0x60;
    D_801B2B30 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8009456C(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_80095724__for_func_8009456C(void) __asm__("func_80095724");

    if (--D_801B2B34 == 0)
    {
        D_801B2B30 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800945A0(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;
extern void func_80095724__for_func_800945A0(void) __asm__("func_80095724");

    func_8006CAC0(func_80095724__for_func_800945A0);
    D_801B2B34 = 0x34;
    D_801B2B30 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800945DC(void)
{
extern s32 D_801B2B34;
extern s32 D_801B2B30;

    if (--D_801B2B34 == 0)
    {
        D_801B2B30 += 1;
    }
}

void func_80094610(void)
{
typedef struct
{
    u32 value;
    u8 unknown_4[36];
} WmapValueRecord;

extern s32 D_8013B20C;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2B30;

    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B2B30 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80094674(s32 arg0)
{
extern u32 D_801B2B38;
extern s32 D_801B2B3C;
extern void (*D_800D61C8[])(void);
extern void func_80094784__for_func_80094674(void) __asm__("func_80094784");
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B38 = 1;
        D_801B2B3C = 1;
        return 1;
    }

    if (D_801B2B38 < 0x6)
    {
        D_800D61C8[D_801B2B38]();
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
void func_800946EC(void)
{
extern u32 D_801B2B38;
extern s32 D_801B2B3C;
extern void (*D_800D61C8[])(void);
extern void func_80094784__for_func_800946EC(void) __asm__("func_80094784");
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    D_801B2B38 = 1;
    D_801B2B3C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80094704(void)
{
extern u32 D_801B2B38;
extern s32 D_801B2B3C;
extern void (*D_800D61C8[])(void);
extern void func_80094784__for_func_80094704(void) __asm__("func_80094784");
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0xE] = 2;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 8;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0x22] = 0x81;
    *(s16*)&D_800D9318[0x24] = 0x81;
    D_801B2B3C = 0x60;
    D_801B2B38 += 1;
    func_80094784__for_func_80094704();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80094784(void)
{
extern u32 D_801B2B38;
extern s32 D_801B2B3C;
extern void (*D_800D61C8[])(void);
extern void func_80094784(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D9318, D_801399A8);
    wmap_draw_actor_sprite(D_800D9318, D_8011CF4C, 0xF, 0x2, 0);
    if (--D_801B2B3C == 0)
    {
        D_801B2B38 += 1;
    }
}

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void func_80094800(void)
{
extern void func_8009484C__for_func_80094800(void) __asm__("func_8009484C");
extern s16 D_800D9318[];
extern s32 D_801B2B3C;
extern s32 D_801B2B38;
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    D_800D9318[17] = 0;
    D_800D9318[19] = 8;
    D_801B2B3C = 0x10;
    D_801B2B38 += 1;
    func_8009484C__for_func_80094800();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009484C(void)
{
extern void func_8009484C(void);
extern s16 D_800D9318[];
extern s32 D_801B2B3C;
extern s32 D_801B2B38;
extern u8 D_801399A8[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D9318, D_801399A8);
    wmap_draw_actor_sprite(D_800D9318, D_8011CF4C, 0xF, 0x2, 0);
    if (--D_801B2B3C == 0)
    {
        D_801B2B38 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800948C8(void)
{
extern s32 D_801B2B38;

    D_801B2B38 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800948E0(s32 arg0)
{
extern u32 D_801B2B40;
extern s32 D_801B2B44;
extern void (*D_800D61E0[])(void);
extern void func_800949EC__for_func_800948E0(void) __asm__("func_800949EC");
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B40 = 1;
        D_801B2B44 = 1;
        return 1;
    }

    if (D_801B2B40 < 0x4)
    {
        D_800D61E0[D_801B2B40]();
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
void func_80094958(void)
{
extern u32 D_801B2B40;
extern s32 D_801B2B44;
extern void (*D_800D61E0[])(void);
extern void func_800949EC__for_func_80094958(void) __asm__("func_800949EC");
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    D_801B2B40 = 1;
    D_801B2B44 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80094970(void)
{
extern u32 D_801B2B40;
extern s32 D_801B2B44;
extern void (*D_800D61E0[])(void);
extern void func_800949EC__for_func_80094970(void) __asm__("func_800949EC");
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
    *(s16*)&D_800D9344[0x24] = 0x80;
    D_801B2B44 = 0x96;
    D_801B2B40 += 1;
    func_800949EC__for_func_80094970();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800949EC(void)
{
extern u32 D_801B2B40;
extern s32 D_801B2B44;
extern void (*D_800D61E0[])(void);
extern void func_800949EC(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D9344, D_801399B0);
    wmap_draw_actor_sprite(D_800D9344, D_8011CF4C, 0x17, 0x8, 0);
    if (--D_801B2B44 == 0)
    {
        D_801B2B40 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80094A68(void)
{
extern s32 D_801B2B40;

    D_801B2B40 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80094A80(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2B48;
extern s32 D_801B2B4C;
extern void (*D_800D61F0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80093448__for_func_80094A80(void) __asm__("func_80093448");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B48 = 1;
        D_801B2B4C = 1;
        return 1;
    }

    if (D_801B2B48 < 0x4)
    {
        D_800D61F0[D_801B2B48]();
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
void func_80094AF8(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2B48;
extern s32 D_801B2B4C;
extern void (*D_800D61F0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80093448__for_func_80094AF8(void) __asm__("func_80093448");

    D_801B2B48 = 1;
    D_801B2B4C = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80094B10(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2B48;
extern s32 D_801B2B4C;
extern void (*D_800D61F0[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2650;
extern s32 D_80182DE8;
extern void func_80093448__for_func_80094B10(void) __asm__("func_80093448");

    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.w[2] = 0xAFC8;
    D_801B2B4C = 0x40;
    D_801B2B48 += 1;
    func_80093448__for_func_80094B10();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80094BC0(void)
{
extern s32 D_801B2B48;

    D_801B2B48 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80094BD8(s32 arg0)
{
extern u32 D_801B2B50;
extern s32 D_801B2B54;
extern void (*D_800D6200[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B50 = 1;
        D_801B2B54 = 1;
        return 1;
    }

    if (D_801B2B50 < 0x4)
    {
        D_800D6200[D_801B2B50]();
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
void func_80094C50(void)
{
extern u32 D_801B2B50;
extern s32 D_801B2B54;
extern void (*D_800D6200[])(void);

    D_801B2B50 = 1;
    D_801B2B54 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_80094C68(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A8;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_801B2478;
extern s32 D_80182DEC;
extern s32 D_801B2B54;
extern u32 D_801B2B50;
extern void func_80093548__for_func_80094C68(void) __asm__("func_80093548");

    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.w[2] = 0xAFC8;
    D_801B2B54 = 0x80;
    D_801B2B50 += 1;
    func_80093548__for_func_80094C68();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80094D14(void)
{
extern s32 D_801B2B50;

    D_801B2B50 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80094D2C(s32 arg0)
{
extern u32 D_801B2B58;
extern s32 D_801B2B5C;
extern void (*D_800D6210[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B58 = 1;
        D_801B2B5C = 1;
        return 1;
    }

    if (D_801B2B58 < 0x6)
    {
        D_800D6210[D_801B2B58]();
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
void func_80094DA4(void)
{
extern u32 D_801B2B58;
extern s32 D_801B2B5C;
extern void (*D_800D6210[])(void);

    D_801B2B58 = 1;
    D_801B2B5C = 1;
}

/** @brief World-map step: reset counters and advance to the next handler. */
void func_80094DBC(void)
{
extern s32 D_801B24B0;
extern s32 D_801B2468;
extern s16 D_801B2490[];
extern s16 D_801B2498[];
extern s32 D_801B2B58;
extern s32 D_801B2B5C;
extern void func_80093648__for_func_80094DBC(void) __asm__("func_80093648");

    D_801B24B0 = 0;
    D_801B2468 = 1;
    D_801B2490[0] = 0;
    D_801B2490[1] = 0;
    D_801B2490[2] = 0;
    D_801B2498[0] = 0;
    D_801B2498[1] = 0;
    D_801B2498[2] = 0;
    D_801B2B5C = 0x40;
    D_801B2B58 += 1;
    func_80093648__for_func_80094DBC();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80094E30(void)
{
extern s32 D_801B2B58;
extern void func_80093784__for_func_80094E30(void) __asm__("func_80093784");
extern s32 D_801B2B5C;

    D_801B2B5C = 0x20;
    D_801B2B58 += 1;
    func_80093784__for_func_80094E30();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80094E68(void)
{
extern s32 D_801B2B58;
extern void func_80093784__for_func_80094E68(void) __asm__("func_80093784");
extern s32 D_801B2B5C;

    D_801B2B58 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80094E80(s32 arg0)
{
extern u32 D_801B2B60;
extern s32 D_801B2B64;
extern void (*D_800D6228[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B60 = 1;
        D_801B2B64 = 1;
        return 1;
    }

    if (D_801B2B60 < 0x8)
    {
        D_800D6228[D_801B2B60]();
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
void func_80094EF8(void)
{
extern u32 D_801B2B60;
extern s32 D_801B2B64;
extern void (*D_800D6228[])(void);

    D_801B2B60 = 1;
    D_801B2B64 = 1;
}

/** @brief Draw the sequence effect, grow its scale, and update the countdown. */
void func_80094F10(void)
{
extern s32 D_80182DF0;
extern s32 D_801B2B60;
extern s32 D_801B2B64;

    s32 remaining;

    func_8006B328(0x64, 0x6C, 4, -1, -1, -4, 0, 8, -0x6E, 0xF0, -0x6E, 0xF0, 1, 0x7F, 1, 4, 0);
    D_80182DF0 += 8;
    remaining = D_801B2B64 - 1;
    D_801B2B64 = remaining;
    if (remaining == 0)
    {
        D_801B2B60 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80094FC8(void)
{
extern void func_80095000__for_func_80094FC8(void) __asm__("func_80095000");
extern s32 D_801B2B64;
extern s32 D_801B2B60;

    D_801B2B64 = 0x20;
    D_801B2B60 += 1;
    func_80095000__for_func_80094FC8();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_80095000(void)
{
extern s32 D_801B2B60;
extern s32 D_801B2B64;

    func_8006B328(0x64, 0x6C, 4, -1, -1, -4, 0, 8, -0x6E, 0xF0, -0x6E, 0xF0, 1, 0x7F, 1, 4, 0);
    if (--D_801B2B64 == 0)
    {
        D_801B2B60 += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800950AC(void)
{
extern void func_800950EC__for_func_800950AC(void) __asm__("func_800950EC");
extern s32 D_800DCEA8;
extern s32 D_801B2B64;
extern s32 D_801B2B60;

    D_800DCEA8 = 0;
    D_801B2B64 = 0x10;
    D_801B2B60 += 1;
    func_800950EC__for_func_800950AC();
}

/** @brief World-map step: emit a UI primitive then tick the shared frame counter. */
void func_800950EC(void)
{
extern s32 D_801B2B60;
extern s32 D_801B2B64;

    func_8006B328(0x64, 0x6C, 4, -1, -1, -4, 0, 8, -0x6E, 0xF0, -0x6E, 0xF0, 1, 0x7F, 1, 4, 0);
    if (--D_801B2B64 == 0)
    {
        D_801B2B60 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80095198(void)
{
extern s32 D_801B2B60;

    D_801B2B60 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800951B0(s32 arg0)
{
extern u32 D_801B2B68;
extern s32 D_801B2B6C;
extern void (*D_800D6248[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B68 = 1;
        D_801B2B6C = 1;
        return 1;
    }

    if (D_801B2B68 < 0x6)
    {
        D_800D6248[D_801B2B68]();
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
void func_80095228(void)
{
extern u32 D_801B2B68;
extern s32 D_801B2B6C;
extern void (*D_800D6248[])(void);

    D_801B2B68 = 1;
    D_801B2B6C = 1;
}

void func_80095240(void)
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


extern WmapConfigA D_800D9370;
extern u8 D_80121538;
extern void *D_801399BC;
extern s32 D_801B2B68;
extern s32 D_801B2B6C;
extern void func_800952C0__for_func_80095240(void) __asm__("func_800952C0");

    D_801399BC = &D_80121538;
    D_800D9370.field_06 = 0xF;
    D_800D9370.field_0E = 1;
    D_800D9370.field_10 = -1;
    D_800D9370.field_26 = 8;
    D_800D9370.field_02 = 0;
    D_800D9370.field_22 = 0x81;
    D_800D9370.field_24 = 1;
    D_801B2B6C = 0x28;
    D_801B2B68 += 1;
    func_800952C0__for_func_80095240();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800952C0(void)
{
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B2B6C;
extern s32 D_801B2B68;

    wmap_step_actor_animation(D_800D9370, D_801399B8);
    wmap_draw_actor_sprite(D_800D9370, D_8011CF4C, 0x8, 0x2, 0);
    if (--D_801B2B6C == 0)
    {
        D_801B2B68 += 1;
    }
}

/** @brief Initialize resource fields, begin a 16-tick delay, and run the next step. */
void func_8009533C(void)
{
/** @brief Partially identified world-map resource fields. */
typedef struct
{
    u8 unknown_0[0x22];
    s16 value_22;
    u8 unknown_24[2];
    s16 value_26;
} WmapResourceFields;

extern WmapResourceFields D_800D9370;
extern s32 D_801B2B68;
extern s32 D_801B2B6C;
extern void func_8009538C__for_func_8009533C(void) __asm__("func_8009538C");

    D_800D9370.value_22 = 2;
    D_800D9370.value_26 = 8;
    D_801B2B6C = 16;
    D_801B2B68 += 1;
    func_8009538C__for_func_8009533C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_8009538C(void)
{
extern s32 D_801B2B68;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B2B6C;

    wmap_step_actor_animation(D_800D9370, D_801399B8);
    wmap_draw_actor_sprite(D_800D9370, D_8011CF4C, 0x8, 0x2, 0);
    if (--D_801B2B6C == 0)
    {
        D_801B2B68 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80095408(void)
{
extern s32 D_801B2B68;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;
extern s32 D_801B2B6C;

    D_801B2B68 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80095420(s32 arg0)
{
extern u32 D_801B2B70;
extern s32 D_801B2B74;
extern void (*D_800D6260[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B70 = 1;
        D_801B2B74 = 1;
        return 1;
    }

    if (D_801B2B70 < 0x6)
    {
        D_800D6260[D_801B2B70]();
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
void func_80095498(void)
{
extern u32 D_801B2B70;
extern s32 D_801B2B74;
extern void (*D_800D6260[])(void);

    D_801B2B70 = 1;
    D_801B2B74 = 1;
}

/**
 * @brief Reset a range of world-map actor configs, arm the timer, and advance the step.
 */
void func_800954B0(void)
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
extern s32 D_801B2B70;
extern s32 D_801B2B74;
extern void func_80093C48__for_func_800954B0(void) __asm__("func_80093C48");

    s32 i;

    for (i = 0x96; i < 0xB4; i++)
    {
        D_800D9268[i].field_22 = 0;
        D_800D9268[i].field_26 = 4;
    }
    D_801B2B74 = 0x20;
    D_801B2B70 += 1;
    func_80093C48__for_func_800954B0();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80095514(void)
{
extern s32 D_801B2B70;

    D_801B2B70 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009552C(s32 arg0)
{
extern u32 D_801B2B78;
extern s32 D_801B2B7C;
extern void (*D_800D6278[])(void);
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_80139280;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B78 = 1;
        D_801B2B7C = 1;
        return 1;
    }

    if (D_801B2B78 < 0x6)
    {
        D_800D6278[D_801B2B78]();
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
void func_800955A4(void)
{
extern u32 D_801B2B78;
extern s32 D_801B2B7C;
extern void (*D_800D6278[])(void);
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_80139280;

    D_801B2B78 = 1;
    D_801B2B7C = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800955BC(void)
{
extern u32 D_801B2B78;
extern s32 D_801B2B7C;
extern void (*D_800D6278[])(void);
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_80139280;

    func_8006A2FC(D_800DB578, D_80139FE8, 0xA, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2B7C == 0)
    {
        D_801B2B78 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_80095640(void)
{
extern void func_80095688__for_func_80095640(void) __asm__("func_80095688");
extern s32* D_80139280;
extern s32 D_801B2B7C;
extern s32 D_801B2B78;
extern u8 D_800DB578[];
extern u8 D_80139FE8[];

    D_801B2B7C = 0x40;
    D_80139280[15] = -1;
    D_801B2B78 += 1;
    func_80095688__for_func_80095640();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80095688(void)
{
extern void func_80095688(void);
extern s32* D_80139280;
extern s32 D_801B2B7C;
extern s32 D_801B2B78;
extern u8 D_800DB578[];
extern u8 D_80139FE8[];

    func_8006A2FC(D_800DB578, D_80139FE8, 0xA, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2B7C == 0)
    {
        D_801B2B78 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8009570C(void)
{
extern s32 D_801B2B78;

    D_801B2B78 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80095724(s32 arg0)
{
extern u32 D_801B2B80;
extern s32 D_801B2B84;
extern void (*D_800D6290[])(void);
extern void func_80095834__for_func_80095724(void) __asm__("func_80095834");
extern u8 D_8011D538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2B80 = 1;
        D_801B2B84 = 1;
        return 1;
    }

    if (D_801B2B80 < 0x4)
    {
        D_800D6290[D_801B2B80]();
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
void func_8009579C(void)
{
extern u32 D_801B2B80;
extern s32 D_801B2B84;
extern void (*D_800D6290[])(void);
extern void func_80095834__for_func_8009579C(void) __asm__("func_80095834");
extern u8 D_8011D538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    D_801B2B80 = 1;
    D_801B2B84 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800957B4(void)
{
extern u32 D_801B2B80;
extern s32 D_801B2B84;
extern void (*D_800D6290[])(void);
extern void func_80095834__for_func_800957B4(void) __asm__("func_80095834");
extern u8 D_8011D538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    D_801399BC = D_8011D538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0xE] = 3;
    *(s16*)&D_800D9370[0x10] = -1;
    *(s16*)&D_800D9370[0x26] = 1;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0x22] = 0x81;
    *(s16*)&D_800D9370[0x24] = 0x81;
    D_801B2B84 = 0x24;
    D_801B2B80 += 1;
    func_80095834__for_func_800957B4();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80095834(void)
{
extern u32 D_801B2B80;
extern s32 D_801B2B84;
extern void (*D_800D6290[])(void);
extern void func_80095834(void);
extern u8 D_8011D538[];
extern u8* D_801399BC;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_8011CF4C;

    wmap_step_actor_animation(D_800D9370, D_801399B8);
    wmap_draw_actor_sprite(D_800D9370, D_8011CF4C, 0xF, 0x2, 0);
    if (--D_801B2B84 == 0)
    {
        D_801B2B80 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800958B0(void)
{
extern s32 D_801B2B80;
extern s32 D_801B2650[];
extern SVECTOR D_801B24A0;
extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 D_801B2BAC;
extern s32 D_801B2BA8;

    D_801B2B80 += 1;
}
