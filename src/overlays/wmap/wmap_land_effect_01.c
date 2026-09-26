#include "wmap_land_effect_01.h"
#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_main.h"

/** @brief Four-word world-map transform vector. */
typedef struct
{
    s32 x, y, z, pad;
} WmapVector;

/** @brief Packed transform settings copied when the effect starts. */
typedef struct
{
    u8 bytes[8];
} WmapConfigBytes;

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

/** @brief First word of a 40-byte world-map cell. */
typedef struct
{
    s32 value;
    u8 unknown_04[36];
} WmapValueRecord;

/**
 * @brief Project active particles and initialize the first available slot.
 * @param actors Actor configurations for the particle slots.
 * @param resources Animation resources corresponding to the actor slots.
 * @param count Number of slots to process.
 */
void func_80072644(WmapConfigA* actors, WmapResource* resources, s32 count)
{
    extern WmapMotion D_801AFBD0[];
    extern s32 D_800D922C;
    extern s32 D_801B0FD0;
    extern s32 rand(void);

    SVECTOR position;
    s32 screen_position;
    s32 active_count;
    s32 i;

    active_count = 0;
    for (i = 0; i < count; i++)
    {
        if (D_801AFBD0[i].state != 0)
        {
            position.vx = ((D_801AFBD0[i].z >> 3) * (ccos(D_801AFBD0[i].angle) >> 6)) >> 12;
            position.vy = ((D_801AFBD0[i].z >> 3) * (csin(D_801AFBD0[i].angle) >> 6)) >> 12;
            position.vz = D_801AFBD0[i].field_0E;
            gte_ldv0(&position);
            gte_rtps();
            D_801AFBD0[i].field_0E += D_801AFBD0[i].x;
            D_801AFBD0[i].z += 1000;
            gte_stsxy(&screen_position);
            wmap_step_actor_animation(&actors[i], &resources[i]);
            wmap_draw_actor_sprite(&actors[i], screen_position, 13, 10, 0);
            D_801AFBD0[i].scale--;
            if (D_801AFBD0[i].scale == 0)
            {
                D_801AFBD0[i].state = 0;
            }
            active_count++;
        }
    }
    D_800D922C = active_count;
    for (i = 0; i < count; i++)
    {
        if (D_801AFBD0[i].state == 0)
        {
            if (D_801B0FD0 >= active_count)
            {
                actors[i].field_06 = 15;
                actors[i].field_10 = -1;
                actors[i].field_02 = 0;
                actors[i].field_0E = 1;
                actors[i].field_22 = 129;
                actors[i].field_24 = 129;
                D_801AFBD0[i].state = 1;
                D_801AFBD0[i].angle = rand();
                D_801AFBD0[i].z = 10000;
                D_801AFBD0[i].x = ((rand() * 5) >> 15) + 1;
                D_801AFBD0[i].scale = ((rand() * 2) >> 15) + 24;
                D_801AFBD0[i].field_0E = 0;
            }
            break;
        }
    }
}

/** @brief Project and draw the map effect, advancing when its timer expires. */
void func_8007287C(void)
{
typedef struct
{
    s32 x, y, scale, pad;
} WmapTransform;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern WmapTransform g_wmap_view;

extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_801B2530;
extern s32 D_801B2534;

    SVECTOR position;
    s32 screen_position;
    s32 remaining;
    u8 *actor = D_800D9344;

    position.vz = 0;
    position.vx = (((D_8011D510 - 1) * 160 -
                   g_wmap_view.x * 0x14000 / g_wmap_view.scale) * 0x6000) /
                  g_wmap_view.scale;
    position.vy = (((D_8011D530 - 1) * 160 -
                   g_wmap_view.y * 0x14000 / g_wmap_view.scale) * 0x6000) /
                  g_wmap_view.scale;
    gte_ldv0(&position);
    gte_rtps();
    wmap_step_actor_animation(actor, D_801399B0);
    gte_stsxy(&screen_position);
    wmap_draw_actor_sprite(actor, screen_position, 12, 10, 0);
    remaining = D_801B2534 - 1;
    D_801B2534 = remaining;
    if (remaining == 0)
    {
        D_801B2530++;
    }
}

/** @brief Draw and fade the transformed effect, then advance its countdown. */
void func_80072A58(void)
{
extern u8 D_800DCF18[];
extern VECTOR D_801B2478;
extern VECTOR D_8011CF60;
extern SVECTOR D_801B24A8;
extern s32 D_801B2538;
extern s32 D_801B253C;
extern s32 D_801B2470;

    MATRIX matrix;
    s32 depth;
    s32 intensity;
    s32 remaining;

    depth = D_801B2478.vz - 2400;
    D_801B2478.vz = depth;
    if (depth < 10)
    {
        D_801B2478.vz = 10;
    }
    PushMatrix();
    RotMatrix(&D_801B24A8, &matrix);
    TransMatrix(&matrix, &D_8011CF60);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    if (D_801B2470 != 0)
    {
        func_8006CD98(D_800DCF18, 0, 4, -1, -1, 1, D_801B2470);
    }
    PopMatrix();
    intensity = D_801B2470 - 9;
    D_801B2470 = intensity;
    if (intensity < 0)
    {
        D_801B2470 = 0;
    }
    remaining = D_801B253C - 1;
    D_801B253C = remaining;
    if (remaining == 0)
    {
        D_801B2538++;
    }
}

/** @brief Initialize twenty effect actors with alternating motion parameters. */
void func_80072B58(void)
{
    extern WmapConfigA D_800D9370[];
    extern u8 D_8011F538[];
    extern WmapConfigBytes D_80139258;
    extern WmapConfigBytes D_801B24A0;
    extern WmapVector D_80139870;
    extern WmapVector g_wmap_camera_translation;
    extern WmapResource D_80139988[];
    extern WmapMotion D_801AFC98[];
    extern WmapMotion* D_801B2560;
    extern s32 D_80139980;
    extern s32 D_801B2470;
    extern s32 D_801B2540;
    extern s32 D_801B2544;
    extern s32 rand(void);
    extern void func_80073CB0__for_func_80072B58(void) __asm__("func_80073CB0");

    s32 i = 0;
    WmapConfigA* actor;

    D_801B2560 = D_801AFC98;
    D_801B24A0 = D_80139258;
    D_80139870 = g_wmap_camera_translation;
    D_801B2470 = 128;
    for (; i < 20; i++)
    {
        actor = &D_800D9370[i];
        D_801B2560[i].state = 0;
        D_80139988[i + 6].resource = D_8011F538 + ((i % 3) << 13);
        actor->field_02 = 0;
        actor->field_06 = 15;
        actor->field_0E = rand() & 1;
        actor->field_10 = -1;
        actor->field_22 = 128;
        actor->field_24 = 128;
        D_801B2560[i].state = 1;
        D_801B2560[i].x = 30000;
        D_801B2560[i].angle = i * 204;
        D_801B2560[i].z = 0;
        if (i & 1)
        {
            D_801B2560[i].field_0E = 30;
        }
        else
        {
            D_801B2560[i].field_0E = 0;
        }
    }
    D_80139980 = 128;
    D_801B2544 = 36;
    D_801B2540++;
    func_80073CB0__for_func_80072B58();
}

/** @brief Compose the effect transform and project its twenty actors. */
void func_80072D30(void)
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


extern WmapConfigA D_800D9370[];
extern WmapResource D_801399B8[];
extern WmapMotion *D_801B2560;
extern VECTOR D_8011CF60;
extern VECTOR D_80139870;
extern SVECTOR g_wmap_camera_rotation;
extern SVECTOR D_801B24A0;
extern u16 D_80139980;

    MATRIX base_matrix;
    MATRIX effect_matrix;
    SVECTOR position;
    s32 screen_position;
    s32 i;
    s32 value;
    WmapMotion *motion;
    WmapConfigA *actor;
    WmapResource *resource;

    PushMatrix();
    RotMatrix(&g_wmap_camera_rotation, &base_matrix);
    TransMatrix(&base_matrix, &D_80139870);
    SetRotMatrix(&base_matrix);
    SetTransMatrix(&base_matrix);
    RotMatrix(&D_801B24A0, &effect_matrix);
    TransMatrix(&effect_matrix, &D_8011CF60);
    CompMatrix(&base_matrix, &effect_matrix, &effect_matrix);
    SetRotMatrix(&effect_matrix);
    SetTransMatrix(&effect_matrix);
    D_801B24A0.vz -= 80;
    for (i = 0; i < 20; i++)
    {
        actor = &D_800D9370[i];
        position.vx = ((D_801B2560[i].z >> 6) * (ccos(D_801B2560[i].angle) >> 6)) >> 12;
        position.vy = ((D_801B2560[i].z >> 6) * (csin(D_801B2560[i].angle) >> 6)) >> 12;
        motion = &D_801B2560[i];
        resource = &D_801399B8[i];
        position.vz = motion->field_0E;
        gte_ldv0(&position);
        gte_rtps();
        value = motion->z + motion->x;
        motion->z = value;
        if (value > 900000)
        {
            motion->z = 900000;
        }
        actor->field_22 = D_80139980;
        actor->field_24 = D_80139980;
        gte_stsxy(&screen_position);
        wmap_step_actor_animation(actor, resource);
        wmap_draw_actor_sprite(actor, screen_position, 13, 31, 0);
    }
    PopMatrix();
}

/** @brief Configure effect parameters and reset its animation resource slots. */
void func_80072F38(void)
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
extern s32 D_801B2558;
extern s32 D_801B255C;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern u8 D_80125538[];
extern void func_8007421C__for_func_80072F38(void) __asm__("func_8007421C");

    s32 i;

    D_801B0FD0 = 24;
    D_80182DEC = 256;
    D_80139234 = 12;
    D_8013923C = 10;
    D_80139240 = 24;
    D_8013924C = 2;
    D_80139250 = 2;
    D_80139260 = 1500;
    D_80139264 = 128;
    D_80139268 = 13;
    D_8013926C = 1;
    D_80139284 = 0;
    for (i = 0; i < 24; i++)
    {
        D_801AFBD0[i + D_80139264].field_00 = 0;
        D_80139988[i + 204].field_04 = D_80125538;
    }
    D_801B255C = 32;
    D_801B2558++;
    func_8007421C__for_func_80072F38();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80073048(s32 arg0)
{
extern u32 D_801B2510;
extern s32 D_801B2514;
extern void (*D_800D4ED8[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2510 = 1;
        D_801B2514 = 1;
        return 1;
    }

    if (D_801B2510 < 0xE)
    {
        D_800D4ED8[D_801B2510]();
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
void func_800730C0(void)
{
extern u32 D_801B2510;
extern s32 D_801B2514;
extern void (*D_800D4ED8[])(void);

    D_801B2510 = 1;
    D_801B2514 = 1;
}

/** @brief World-map step handler: set up the actor, register callbacks, advance step. */
void func_800730D8(void)
{
extern void func_80073430__for_func_800730D8(void) __asm__("func_80073430");
extern s32 D_8013B208;
extern s32 D_801B2514;
extern s32 D_801B2510;

    D_8013B208 = 1;
    wmap_start_map_tint(0x404040);
    func_8006CAC0(func_80073430__for_func_800730D8);
    wmap_play_sound(0xF, 0x80);
    D_801B2514 = 8;
    D_801B2510 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80073138(void)
{
extern s32 D_801B2514;
extern s32 D_801B2510;

    if (--D_801B2514 == 0)
    {
        D_801B2510 += 1;
    }
}

/** @brief Set the sequence flag, register two callbacks, and begin a 48-tick delay. */
void func_8007316C(void)
{
extern s32 D_801ADAE0;
extern s32 D_801B2510;
extern s32 D_801B2514;
extern void func_80073848__for_func_8007316C(void) __asm__("func_80073848");
extern void func_800739C8__for_func_8007316C(void) __asm__("func_800739C8");

    D_801ADAE0 = 1;
    func_8006CAC0(&func_80073848__for_func_8007316C);
    func_8006CAC0(&func_800739C8__for_func_8007316C);
    D_801B2514 = 0x30;
    D_801B2510 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800731C0(void)
{
extern s32 D_801B2514;
extern s32 D_801B2510;
extern void func_80073ADC__for_func_800731C0(void) __asm__("func_80073ADC");

    if (--D_801B2514 == 0)
    {
        D_801B2510 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800731F4(void)
{
extern s32 D_801B2514;
extern s32 D_801B2510;
extern void func_80073ADC__for_func_800731F4(void) __asm__("func_80073ADC");

    func_8006CAC0(func_80073ADC__for_func_800731F4);
    D_801B2514 = 0x2;
    D_801B2510 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80073230(void)
{
extern s32 D_801B2514;
extern s32 D_801B2510;
extern void func_80073C20__for_func_80073230(void) __asm__("func_80073C20");

    if (--D_801B2514 == 0)
    {
        D_801B2510 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_80073264(void)
{
extern s32 D_801B2514;
extern s32 D_801B2510;
extern void func_80073C20__for_func_80073264(void) __asm__("func_80073C20");

    func_8006CAC0(func_80073C20__for_func_80073264);
    D_801B2514 = 0x2;
    D_801B2510 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800732A0(void)
{
extern s32 D_801B2514;
extern s32 D_801B2510;
extern void func_80073F48__for_func_800732A0(void) __asm__("func_80073F48");
extern void func_80074194__for_func_800732A0(void) __asm__("func_80074194");
extern void func_80073DB0__for_func_800732A0(void) __asm__("func_80073DB0");

    if (--D_801B2514 == 0)
    {
        D_801B2510 += 1;
    }
}

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void func_800732D4(void)
{
extern s32 D_801B2514;
extern s32 D_801B2510;
extern void func_80073F48__for_func_800732D4(void) __asm__("func_80073F48");
extern void func_80074194__for_func_800732D4(void) __asm__("func_80074194");
extern void func_80073DB0__for_func_800732D4(void) __asm__("func_80073DB0");

    func_8006CAC0(func_80073F48__for_func_800732D4);
    func_8006CAC0(func_80074194__for_func_800732D4);
    func_8006CAC0(func_80073DB0__for_func_800732D4);
    D_801B2514 = 0x60;
    D_801B2510 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80073328(void)
{
extern s32 D_801B2514;
extern s32 D_801B2510;

    if (--D_801B2514 == 0)
    {
        D_801B2510 += 1;
    }
}

/** @brief Play sound 19, start a 54-tick delay, and advance the state. */
void func_8007335C(void)
{
extern s32 D_801B2510;
extern s32 D_801B2514;

    wmap_play_sound(0x13, 0x80);
    D_801B2514 = 54;
    D_801B2510 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_80073398(void)
{
extern s32 D_801B2514;
extern s32 D_801B2510;

    if (--D_801B2514 == 0)
    {
        D_801B2510 += 1;
    }
}

/** @brief Update the selected world-map cell value, clear the gate flag, and advance the sequence. */
void func_800733CC(void)
{
extern s32 D_8013B20C;
extern WmapValueRecord D_80139290[][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 D_801B2510;

    D_8013B20C = 0;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B2510 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80073430(s32 arg0)
{
extern u32 D_801B2528;
extern s32 D_801B252C;
extern void (*D_800D4F38[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2528 = 1;
        D_801B252C = 1;
    }

    if (D_801B2528 < 0x6)
    {
        D_800D4F38[D_801B2528]();
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
void func_800734A0(void)
{
extern u32 D_801B2528;
extern s32 D_801B252C;
extern void (*D_800D4F38[])(void);

    D_801B2528 = 1;
    D_801B252C = 1;
}

/** @brief Initialize the actor configuration and begin an eight-tick sequence step. */
void func_800734B8(void)
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

extern void func_80073530__for_func_800734B8(void) __asm__("func_80073530");
extern WmapConfigA D_800D9268[];
extern u8 D_8011D538[];
extern u8 *D_801399AC;
extern s32 D_801B2528;
extern s32 D_801B252C;

    D_801399AC = D_8011D538;
    D_800D9268[4].field_06 = 0xF;
    D_800D9268[4].field_0E = 1;
    D_800D9268[4].field_10 = -1;
    D_800D9268[4].field_24 = 1;
    D_800D9268[4].field_02 = 0;
    D_800D9268[4].field_22 = 0xF1;
    D_801B252C = 8;
    D_801B2528 += 1;
    func_80073530__for_func_800734B8();
}

/**
 * @brief World-map step handler: draw the sprite, bump its animation field, then
 *        countdown-advance the step.
 */
void func_80073530(void)
{
    extern u8 D_800D9318[];
    extern u8 D_801399A8[];
    extern s32 D_8011CF4C;
    extern s32 D_801B2528;
    extern s32 D_801B252C;

    u8* obj;
    WmapConfigA* actors;

    obj = D_800D9318;
    wmap_step_actor_animation(obj, D_801399A8);
    wmap_draw_actor_sprite(obj, D_8011CF4C, 0xC, 0xA, 0);
    /* This sprite occupies the fifth actor slot. */
    actors = (WmapConfigA*)obj - 4;
    actors[4].field_24 += 2;
    if (--D_801B252C == 0)
    {
        D_801B2528 += 1;
    }
}

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void func_800735B8(void)
{
extern s16 D_800D933A;
extern s32 D_801B2528;
extern s32 D_801B252C;
extern void func_800735FC__for_func_800735B8(void) __asm__("func_800735FC");

    D_800D933A = 1;
    D_801B252C = 0x10;
    D_801B2528 += 1;
    func_800735FC__for_func_800735B8();
}

/**
 * @brief World-map step handler: draw the sprite, bump its animation field, then
 *        countdown-advance the step.
 */
void func_800735FC(void)
{
    extern u8 D_800D9318[];
    extern u8 D_801399A8[];
    extern s32 D_8011CF4C;
    extern s32 D_801B2528;
    extern s32 D_801B252C;

    u8* obj;
    WmapConfigA* actors;

    obj = D_800D9318;
    wmap_step_actor_animation(obj, D_801399A8);
    wmap_draw_actor_sprite(obj, D_8011CF4C, 0xC, 0xA, 0);
    /* This sprite occupies the fifth actor slot. */
    actors = (WmapConfigA*)obj - 4;
    actors[4].field_24 += 2;
    if (--D_801B252C == 0)
    {
        D_801B2528 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80073684(void)
{
extern s32 D_801B2528;

    D_801B2528 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8007369C(s32 arg0)
{
extern u32 D_801B2518;
extern s32 D_801B251C;
extern void (*D_800D4F10[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2518 = 1;
        D_801B251C = 1;
        return 1;
    }

    if (D_801B2518 < 0x6)
    {
        D_800D4F10[D_801B2518]();
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
void func_80073714(void)
{
extern u32 D_801B2518;
extern s32 D_801B251C;
extern void (*D_800D4F10[])(void);

    D_801B2518 = 1;
    D_801B251C = 1;
}

/**
 * @brief Register a world-map callback and advance to the next step.
 */
void func_8007372C(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2518;
extern void func_8007377C__for_func_8007372C(void) __asm__("func_8007377C");

    g_wmap_input_locked = 1;
    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B2518 += 1;
    func_8007377C__for_func_8007372C();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8007377C(void)
{
extern s32 D_8013B20C;
extern s32 D_801B2518;
extern void func_800737B8__for_func_8007377C(void) __asm__("func_800737B8");

    if (D_8013B20C == 0)
    {
        D_801B2518 += 1;
        func_800737B8__for_func_8007377C();
    }
}

/** @brief World-map step: register the next draw callback and advance to the next handler. */
void func_800737B8(void)
{
extern void func_80073048__for_func_800737B8(void) __asm__("func_80073048");
extern void func_800737F4__for_func_800737B8(void) __asm__("func_800737F4");
extern s32 D_801B2518;

    func_8006CAC0(func_80073048__for_func_800737B8);
    D_801B2518 += 1;
    func_800737F4__for_func_800737B8();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800737F4(void)
{
extern s32 D_801B2518;
extern s32 D_8013B20C;
extern void func_80073830__for_func_800737F4(void) __asm__("func_80073830");

    if (D_8013B20C == 0)
    {
        D_801B2518 += 1;
        func_80073830__for_func_800737F4();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80073830(void)
{
extern s32 D_801B2518;
extern s32 D_8013B20C;
extern void func_80073830(void);

    D_801B2518 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80073848(s32 arg0)
{
extern u32 D_801B2520;
extern s32 D_801B2524;
extern void (*D_800D4F28[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2520 = 1;
        D_801B2524 = 1;
    }

    if (D_801B2520 < 0x4)
    {
        D_800D4F28[D_801B2520]();
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
void func_800738B8(void)
{
extern u32 D_801B2520;
extern s32 D_801B2524;
extern void (*D_800D4F28[])(void);

    D_801B2520 = 1;
    D_801B2524 = 1;
}

/** @brief World-map step: reset a run of slot tables then advance the sub-counter. */
void func_800738D0(void)
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
extern s32 D_80125538;
extern WmapSlot14 D_801AFBD0[];
extern s32 D_801B2524;
extern s32 D_801B2520;
extern void func_80073954__for_func_800738D0(void) __asm__("func_80073954");

    s32 i;

    D_801B0FD0 = 0x18;
    for (i = 0; i < 0x18; i++)
    {
        D_801AFBD0[i].field_00 = 0;
        D_80139988[i + 0xCC].field_04 = &D_80125538;
    }
    D_801B2524 = 0x30;
    D_801B2520 += 1;
    func_80073954__for_func_800738D0();
}

/** @brief World-map step handler: run the sub-step, then advance after the timer. */
void func_80073954(void)
{
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_801B2520;
extern s32 D_801B2524;
extern void func_80072644__for_func_80073954(u8 *a0, u8 *a1, s32 a2) __asm__("func_80072644");

    func_80072644__for_func_80073954(D_800DB578, D_80139FE8, 0x18);
    if (--D_801B2524 == 0)
    {
        D_801B2520 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800739B0(void)
{
extern s32 D_801B2520;

    D_801B2520 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_800739C8(s32 arg0)
{
extern u32 D_801B2530;
extern s32 D_801B2534;
extern void (*D_800D4F50[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2530 = 1;
        D_801B2534 = 1;
    }

    if (D_801B2530 < 0x4)
    {
        D_800D4F50[D_801B2530]();
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
void func_80073A38(void)
{
extern u32 D_801B2530;
extern s32 D_801B2534;
extern void (*D_800D4F50[])(void);

    D_801B2530 = 1;
    D_801B2534 = 1;
}

/** @brief World-map step handler: init substate block and advance. */
void func_80073A50(void)
{
extern void *D_801399B4;
extern s32 D_8011D538;
extern u8 D_800D9268;
extern s32 D_801B2530;
extern s32 D_801B2534;
extern void func_8007287C__for_func_80073A50(void) __asm__("func_8007287C");

    u8 *base = &D_800D9268;

    D_801399B4 = &D_8011D538;
    *(u8 *)(base + 0xE2) = 0xF;
    *(s16 *)(base + 0xEC) = -1;
    *(s16 *)(base + 0xDE) = 0;
    *(s16 *)(base + 0xEA) = 0;
    *(s16 *)(base + 0xFE) = 0x80;
    *(s16 *)(base + 0x100) = 0x80;
    D_801B2534 = 0x32;
    D_801B2530 += 1;
    func_8007287C__for_func_80073A50();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80073AC4(void)
{
extern s32 D_801B2530;

    D_801B2530 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80073ADC(s32 arg0)
{
extern u32 D_801B2538;
extern s32 D_801B253C;
extern void (*D_800D4F60[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2538 = 1;
        D_801B253C = 1;
    }

    if (D_801B2538 < 0x4)
    {
        D_800D4F60[D_801B2538]();
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
void func_80073B4C(void)
{
extern u32 D_801B2538;
extern s32 D_801B253C;
extern void (*D_800D4F60[])(void);

    D_801B2538 = 1;
    D_801B253C = 1;
}

/** @brief Restore the default transform and advance the sequence. */
void func_80073B64(void)
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
extern WmapTransform g_wmap_camera_translation;
extern WmapTransform D_801B2478;
extern s32 D_801B2470;
extern s32 D_801B2538;
extern s32 D_801B253C;
extern void func_80072A58__for_func_80073B64(void) __asm__("func_80072A58");

    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_801B2470 = 0x80;
    D_801B253C = 0x20;
    D_801B2538++;
    func_80072A58__for_func_80073B64();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80073C08(void)
{
extern s32 D_801B2538;

    D_801B2538 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80073C20(s32 arg0)
{
extern u32 D_801B2540;
extern s32 D_801B2544;
extern void (*D_800D4F70[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2540 = 1;
        D_801B2544 = 1;
        return 1;
    }

    if (D_801B2540 < 0x6)
    {
        D_800D4F70[D_801B2540]();
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
void func_80073C98(void)
{
extern u32 D_801B2540;
extern s32 D_801B2544;
extern void (*D_800D4F70[])(void);

    D_801B2540 = 1;
    D_801B2544 = 1;
}

/** @brief Update the sequence effect and advance when its countdown reaches zero. */
void func_80073CB0(void)
{
extern void func_80072D30__for_func_80073CB0(void) __asm__("func_80072D30");
extern s32 D_801B2540;
extern s32 D_801B2544;

    s32 remaining_ticks;

    func_80072D30__for_func_80073CB0();
    remaining_ticks = D_801B2544 - 1;
    D_801B2544 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2540 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_80073CFC(void)
{
extern void func_80073D34__for_func_80073CFC(void) __asm__("func_80073D34");
extern s32 D_801B2544;
extern s32 D_801B2540;

    D_801B2544 = 0x20;
    D_801B2540 += 1;
    func_80073D34__for_func_80073CFC();
}

/** @brief Reduce the effect value toward zero, update it, and advance when the countdown expires. */
void func_80073D34(void)
{
extern void func_80072D30__for_func_80073D34(void) __asm__("func_80072D30");
extern s32 D_80139980;
extern s32 D_801B2540;
extern s32 D_801B2544;

    s32 value;
    s32 remaining_ticks;

    value = D_80139980 - 4;
    D_80139980 = value;
    if (value < 0)
    {
        D_80139980 = 0;
    }
    func_80072D30__for_func_80073D34();
    remaining_ticks = D_801B2544 - 1;
    D_801B2544 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2540 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80073D98(void)
{
extern s32 D_801B2540;

    D_801B2540 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80073DB0(s32 arg0)
{
extern u32 D_801B2548;
extern s32 D_801B254C;
extern void (*D_800D4F88[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2548 = 1;
        D_801B254C = 1;
        return 1;
    }

    if (D_801B2548 < 0x4)
    {
        D_800D4F88[D_801B2548]();
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
void func_80073E28(void)
{
extern u32 D_801B2548;
extern s32 D_801B254C;
extern void (*D_800D4F88[])(void);

    D_801B2548 = 1;
    D_801B254C = 1;
}

/** @brief World-map step handler: init substate block and advance. */
void func_80073E40(void)
{
extern void *D_8013A17C;
extern s32 D_80127538;
extern u8 D_800D9268;
extern s32 D_801B2548;
extern s32 D_801B254C;
extern void func_80073EB4__for_func_80073E40(void) __asm__("func_80073EB4");

    u8 *base = &D_800D9268;

    D_8013A17C = &D_80127538;
    *(u8 *)(base + 0x2BAE) = 0xF;
    *(s16 *)(base + 0x2BB8) = -1;
    *(s16 *)(base + 0x2BAA) = 0;
    *(s16 *)(base + 0x2BB6) = 0;
    *(s16 *)(base + 0x2BCA) = 0x80;
    *(s16 *)(base + 0x2BCC) = 0;
    D_801B254C = 0x98;
    D_801B2548 += 1;
    func_80073EB4__for_func_80073E40();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80073EB4(void)
{
extern s32 D_801B2548;
extern u8 D_800DBE10[];
extern u8 D_8013A178[];
extern s32 D_8011CF4C;
extern s32 D_801B254C;

    wmap_step_actor_animation(D_800DBE10, D_8013A178);
    wmap_draw_actor_sprite(D_800DBE10, D_8011CF4C, 0xE, 0xA, 0);
    if (--D_801B254C == 0)
    {
        D_801B2548 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80073F30(void)
{
extern s32 D_801B2548;
extern u8 D_800DBE10[];
extern u8 D_8013A178[];
extern s32 D_8011CF4C;
extern s32 D_801B254C;

    D_801B2548 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80073F48(s32 arg0)
{
extern u32 D_801B2550;
extern s32 D_801B2554;
extern void (*D_800D4F98[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2550 = 1;
        D_801B2554 = 1;
    }

    if (D_801B2550 < 0x6)
    {
        D_800D4F98[D_801B2550]();
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
void func_80073FB8(void)
{
extern u32 D_801B2550;
extern s32 D_801B2554;
extern void (*D_800D4F98[])(void);

    D_801B2550 = 1;
    D_801B2554 = 1;
}

/** @brief World-map step handler: init substate block and advance. */
void func_80073FD0(void)
{
extern void *D_8013A184;
extern s32 D_80125538;
extern u8 D_800D9268;
extern s32 D_801B2550;
extern s32 D_801B2554;
extern void func_80074044__for_func_80073FD0(void) __asm__("func_80074044");

    u8 *base = &D_800D9268;

    D_8013A184 = &D_80125538;
    *(u8 *)(base + 0x2BDA) = 0xF;
    *(s16 *)(base + 0x2BE4) = -1;
    *(s16 *)(base + 0x2BD6) = 0;
    *(s16 *)(base + 0x2BE2) = 0;
    *(s16 *)(base + 0x2BF6) = 0x81;
    *(s16 *)(base + 0x2BF8) = 0x81;
    D_801B2554 = 0x40;
    D_801B2550 += 1;
    func_80074044__for_func_80073FD0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80074044(void)
{
extern u8 g_wmap_vehicle_actor[];
extern u8 g_wmap_vehicle_animation[];
extern s32 D_8011CF4C;
extern s32 D_801B2554;
extern s32 D_801B2550;

    wmap_step_actor_animation(g_wmap_vehicle_actor, g_wmap_vehicle_animation);
    wmap_draw_actor_sprite(g_wmap_vehicle_actor, D_8011CF4C, 0xD, 0xA, 0);
    if (--D_801B2554 == 0)
    {
        D_801B2550 += 1;
    }
}

/**
 * @brief Reset a world-map animation flag and schedule the next step.
 */
void func_800740C0(void)
{
extern u16 D_800DBE5E;
extern s32 D_801B2550;
extern s32 D_801B2554;
extern void func_80074100__for_func_800740C0(void) __asm__("func_80074100");

    D_800DBE5E = 0;
    D_801B2554 = 0x10;
    D_801B2550 += 1;
    func_80074100__for_func_800740C0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80074100(void)
{
extern s32 D_801B2550;
extern u8 g_wmap_vehicle_actor[];
extern u8 g_wmap_vehicle_animation[];
extern s32 D_8011CF4C;
extern s32 D_801B2554;

    wmap_step_actor_animation(g_wmap_vehicle_actor, g_wmap_vehicle_animation);
    wmap_draw_actor_sprite(g_wmap_vehicle_actor, D_8011CF4C, 0xD, 0xA, 0);
    if (--D_801B2554 == 0)
    {
        D_801B2550 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8007417C(void)
{
extern s32 D_801B2550;
extern u8 g_wmap_vehicle_actor[];
extern u8 g_wmap_vehicle_animation[];
extern s32 D_8011CF4C;
extern s32 D_801B2554;

    D_801B2550 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80074194(s32 arg0)
{
extern u32 D_801B2558;
extern s32 D_801B255C;
extern void (*D_800D4FB0[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2558 = 1;
        D_801B255C = 1;
    }

    if (D_801B2558 < 0x6)
    {
        D_800D4FB0[D_801B2558]();
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
void func_80074204(void)
{
extern u32 D_801B2558;
extern s32 D_801B255C;
extern void (*D_800D4FB0[])(void);

    D_801B2558 = 1;
    D_801B255C = 1;
}

/** @brief Draw the sequence effect and advance when the countdown expires. */
void func_8007421C(void)
{
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_801B2558;
extern s32 D_801B255C;

    s32 value;

    func_8006CFE4((s32)D_800DB578, (s32)D_80139FE8, 0x18, 0x81, 0x81, 0x10);
    value = D_801B255C - 1;
    D_801B255C = value;
    if (value == 0)
    {
        D_801B2558 += 1;
    }
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007428C(void)
{
extern void func_800742C4__for_func_8007428C(void) __asm__("func_800742C4");
extern s32 D_801B255C;
extern s32 D_801B2558;

    D_801B255C = 0x20;
    D_801B2558 += 1;
    func_800742C4__for_func_8007428C();
}

/** @brief Draw the effect, reduce its value toward zero, and update the countdown. */
void func_800742C4(void)
{
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_80182DEC;
extern s32 D_801B2558;
extern s32 D_801B255C;

    s32 value;
    s32 remaining_ticks;

    func_8006CFE4((s32)D_800DB578, (s32)D_80139FE8, 0x18, 0x81, 0x81, 0x10);
    value = D_80182DEC - 0xA;
    D_80182DEC = value;
    if (value < 0)
    {
        D_80182DEC = 0;
    }
    remaining_ticks = D_801B255C - 1;
    D_801B255C = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2558 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_80074350(void)
{
extern s32 D_801B2558;

    D_801B2558 += 1;
}
