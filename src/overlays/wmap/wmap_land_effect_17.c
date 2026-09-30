#include "wmap_main.h"
#include "wmap_land_effect_17.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
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

void wmap_land_effect_17_sequence_5_step_02(void);
void wmap_land_effect_17_sequence_6_step_02(void);
void wmap_land_effect_17_sequence_7_step_02(void);
void wmap_land_effect_17_sequence_8_step_02(void);
void wmap_land_effect_17_sequence_9_step_02(void);
void wmap_land_effect_17_sequence_10_step_02(void);
void wmap_land_effect_17_wait_idle_02(void);
void wmap_land_effect_17_step_03(void);
s32 wmap_land_effect_17_run_timeline(s32 arg0);
void wmap_land_effect_17_wait_idle_04(void);
void wmap_land_effect_17_end(void);
s32 wmap_land_effect_17_run_sequence_3(s32 arg0);
s32 wmap_land_effect_17_run_sequence_8(s32 arg0);
s32 wmap_land_effect_17_run_sequence_9(s32 arg0);
s32 wmap_land_effect_17_run_sequence_1(s32 arg0);
s32 wmap_land_effect_17_run_sequence_7(s32 arg0);
s32 wmap_land_effect_17_run_sequence_10(s32 arg0);
s32 wmap_land_effect_17_run_sequence_4(s32 arg0);
s32 wmap_land_effect_17_run_sequence_2(s32 arg0);
s32 wmap_land_effect_17_run_sequence_5(s32 arg0);
s32 wmap_land_effect_17_run_sequence_6(s32 arg0);
void wmap_land_effect_17_sequence_1_step_02(void);
void wmap_land_effect_17_sequence_2_step_02(void);
void wmap_land_effect_17_sequence_7_step_04(void);
void wmap_land_effect_17_sequence_10_step_04(void);

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

/** @brief Position and velocity halfwords for an effect particle. */
typedef struct
{
    s16 x, y, z, pad_06;
    s16 vx, vy, vz, pad_0E;
} WmapParticle;

extern VECTOR D_8011CF60;
extern s32 D_80182DE8;
extern u8 D_800DCF18[];
extern s32 g_wmap_land_effect_17_sequence_3_timer;
extern s32 D_80182DEC;
extern u8* D_8011CF1C;
extern s32 g_wmap_land_effect_17_sequence_4_timer;
extern s32 D_801B0FD0;
extern u8 D_8011D538[];
extern s32 g_wmap_land_effect_17_sequence_5_timer;
extern s32 g_wmap_land_effect_17_sequence_6_timer;
extern u8 D_80121538[];
extern s32 g_wmap_land_effect_17_sequence_7_timer;
extern s32 g_wmap_land_effect_17_sequence_8_timer;
extern s32 rand(void);
extern s32 g_wmap_land_effect_17_sequence_9_timer;
extern s32 g_wmap_land_effect_17_sequence_10_timer;
extern s32 g_wmap_land_effect_17_timer;
extern void (*D_800D5C78[])(void);
extern void wmap_land_effect_17_step_03(void);
extern void wmap_land_effect_17_end(void);
extern s32 g_wmap_land_effect_17_timeline_timer;
extern void (*D_800D5C90[])(void);
extern s32 D_8013B208;
extern s32 D_801ADAE0;
extern WmapTile D_80139290[6][6];
extern s32 D_8011D530;
extern s32 D_8011D510;
extern u32 D_8011D4FC;
extern s32 g_wmap_land_effect_17_sequence_1_timer;
extern void (*D_800D5CE8[])(void);
extern u8* D_801399AC;
extern s32 g_wmap_land_effect_17_sequence_2_timer;
extern void (*D_800D5CF8[])(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern void wmap_land_effect_17_sequence_2_step_02(void);
extern void (*D_800D5D08[])(void);
extern void (*D_800D5D18[])(void);
extern void (*D_800D5D28[])(void);
extern WmapAnimationSlot D_80139A28[];
extern void (*D_800D5D38[])(void);
extern u8 D_80139B18[];
extern void (*D_800D5D48[])(void);
extern void (*D_800D5D60[])(void);
extern void (*D_800D5D70[])(void);
extern void (*D_800D5D80[])(void);
extern u32 g_wmap_land_effect_17_sequence_3_step;
extern u32 g_wmap_land_effect_17_sequence_4_step;
extern WmapConfigA D_800D95D8[];
extern u32 g_wmap_land_effect_17_sequence_5_step;
extern WmapConfigA D_800D9B00[];
extern u32 g_wmap_land_effect_17_sequence_6_step;
extern u32 g_wmap_land_effect_17_sequence_7_step;
extern u32 g_wmap_land_effect_17_sequence_8_step;
extern u32 g_wmap_land_effect_17_sequence_9_step;
extern u8 D_800E4F18[];
extern u32 g_wmap_land_effect_17_sequence_10_step;
extern u32 g_wmap_land_effect_17_step;
extern u32 g_wmap_land_effect_17_timeline_step;
extern u32 g_wmap_land_effect_17_sequence_1_step;
extern u32 g_wmap_land_effect_17_sequence_2_step;

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_801B2650;
extern VECTOR D_801B2478;

extern SVECTOR D_80139258;
extern SVECTOR D_801B24A0;
extern SVECTOR D_801B24A8;

extern WmapSpriteActor D_800D9268[];
extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* D_80139280;

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s16 field_10;
    s16 unknown_12;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

extern WmapMotion D_801AFBD0[];
extern WmapMotion D_801AFD60[];
extern WmapMotion D_801AFFB8[];

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_17_sequence_3_step_02(void)
{
    MATRIX m;
    s32 x;

    x = D_801B2650.vz - 0xDAC;
    D_801B2650.vz = x;
    if (x < 0x2710)
    {
        D_801B2650.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_801B24A0, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DE8 != 0)
    {
        wmap_draw_model_default((s32)D_800DCF18, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DE8);
        D_80182DE8 -= 0x4;
        if (D_80182DE8 < 0)
        {
            D_80182DE8 = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_17_sequence_3_timer == 0)
    {
        g_wmap_land_effect_17_sequence_3_step += 1;
    }
}

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_17_sequence_4_step_02(void)
{
    MATRIX m;
    s32 x;

    x = D_801B2478.vz - 0xDAC;
    D_801B2478.vz = x;
    if (x < 0x2710)
    {
        D_801B2478.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&D_801B24A8, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (D_80182DEC != 0)
    {
        wmap_draw_model_default(D_8011CF1C, 0, 0x4, 0x35, 0x7800, 0x1, D_80182DEC);
        D_80182DEC -= 1;
        if (D_80182DEC < 0)
        {
            D_80182DEC = 0;
        }
    }

    PopMatrix();
    if (--g_wmap_land_effect_17_sequence_4_timer == 0)
    {
        g_wmap_land_effect_17_sequence_4_step += 1;
    }
}

/** @brief Initialize the effect actors, resources, and evenly spaced angles. */
void wmap_land_effect_17_sequence_5_step_01(void)
{
    s32 i;

    i = 0;
    D_801B0FD0 = 3;
    D_80139280[0x15] = 20;
    D_80139280[0x16] = 20;
    D_80139280[0x17] = 128;
    D_80139280[0x18] = 0;
    D_80139280[0x19] = -1;
    D_80139280[0x1A] = 1000;
    D_80139280[0x1B] = 20;
    D_80139280[0x1C] = 15;
    D_80139280[0x1D] = 1;
    D_80139280[0x1E] = 12000;
    do
    {
        D_801AFD60[i].state = 1;
        D_801AFD60[i].angle = i * 1365;
        D_801AFD60[i].scale = 128;
        D_801AFD60[i].z = 12000;
        D_801AFD60[i].x = 0;
        D_801AFD60[i].field_0E = 0;
        D_80139988[i + 20].data = D_8011D538;
        D_800D95D8[i].field_06 = 15;
        D_800D95D8[i].field_10 = -1;
        D_800D95D8[i].field_26 = 2;
        D_800D95D8[i].field_02 = 0;
        D_800D95D8[i].field_0E = 1;
        D_800D95D8[i].field_22 = 0;
        D_800D95D8[i].field_24 = 127;
        i++;
    } while (i < 3);
    g_wmap_land_effect_17_sequence_5_timer = 32;
    g_wmap_land_effect_17_sequence_5_step++;
    wmap_land_effect_17_sequence_5_step_02();
}

/** @brief Initialize the effect actors, resources, and evenly spaced angles. */
void wmap_land_effect_17_sequence_6_step_01(void)
{
    s32 i;

    i = 0;
    D_801B0FD0 = 3;
    D_80139280[0xB] = 4;
    D_80139280[0xC] = 4;
    D_80139280[0xD] = 128;
    D_80139280[0xE] = 0;
    D_80139280[0xF] = -1;
    D_80139280[0x10] = 100;
    D_80139280[0x11] = 50;
    D_80139280[0x12] = 15;
    D_80139280[0x13] = 2;
    D_80139280[0x14] = 12000;
    do
    {
        D_801AFFB8[i].state = 1;
        D_801AFFB8[i].angle = i * 1365;
        D_801AFFB8[i].scale = 128;
        D_801AFFB8[i].z = 12000;
        D_801AFFB8[i].x = 0;
        D_801AFFB8[i].field_0E = 0;
        D_80139988[i + 50].data = D_8011D538;
        D_800D9B00[i].field_06 = 15;
        D_800D9B00[i].field_10 = -1;
        D_800D9B00[i].field_02 = 0;
        D_800D9B00[i].field_0E = 2;
        D_800D9B00[i].field_26 = 2;
        D_800D9B00[i].field_22 = 0;
        D_800D9B00[i].field_24 = 127;
        i++;
    } while (i < 3);
    g_wmap_land_effect_17_sequence_6_timer = 64;
    g_wmap_land_effect_17_sequence_6_step++;
    wmap_land_effect_17_sequence_6_step_02();
}

/** @brief Initialize the actor group and its animation resources, then advance. */
void wmap_land_effect_17_sequence_7_step_01(void)
{
    s32 i;
    s16 angle;
    WmapConfigA *actor;

    D_801B0FD0 = 8;
    angle = (s16)0x27FD8;
    for (i = 80; i < 88; i++)
    {
        actor = &D_800D9268[i];
        D_80139988[i].data = D_80121538;
        actor->field_02 = 0;
        actor->field_06 = 15;
        actor->field_0E = 2;
        actor->field_10 = -1;
        actor->field_22 = 129;
        actor->field_24 = 1;
        actor->field_26 = 8;
        D_801AFBD0[i].state = 1;
        D_801AFBD0[i].angle = i << 9;
        D_801AFBD0[i].x = -4000;
        D_801AFBD0[i].z = 480000;
        D_801AFBD0[i].scale = 9999;
        D_801AFBD0[i].field_0E = 0;
        D_801AFBD0[i].field_10 = 144;
    }
    g_wmap_land_effect_17_sequence_7_timer = 42;
    g_wmap_land_effect_17_sequence_7_step++;
    wmap_land_effect_17_sequence_7_step_02();
}

/** @brief Initialize the effect descriptor and actors with randomized angles. */
void wmap_land_effect_17_sequence_8_step_01(void)
{
    s32 i;
    s32 angle;
    WmapConfigA *actor;
    s32 actor_offset;
    s32 resource_offset;
    WmapResource *resource;
    WmapMotion *motion;

    i = 0;
    D_801B0FD0 = 20;
    D_80139280[0xB] = 64;
    D_80139280[0xC] = 20;
    D_80139280[0xD] = 128;
    D_80139280[0xE] = 0;
    D_80139280[0xF] = -1;
    D_80139280[0x10] = 2000;
    D_80139280[0x11] = 50;
    D_80139280[0x12] = 8;
    D_80139280[0x13] = 1;
    D_80139280[0x14] = 10;
    do
    {
        WmapResource *resources;
        u8 *resource_data;

        motion = &D_801AFFB8[i];
        motion->state = 1;
        angle = rand();
        resource_offset = (i + 50) * 8;
        actor_offset = i * 44;
        resources = D_80139988;
        resource_data = D_80121538;
        actor = (WmapConfigA *)((u8 *)D_800D9B00 + actor_offset);
        resource = (WmapResource *)((u8 *)resources + resource_offset);
        motion->angle = angle & 4095;
        motion->scale = 128;
        motion->z = 10;
        motion->x = 0;
        motion->field_0E = 0;
        resource->resource = resource_data;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_26 = 2;
        actor->field_24 = 127;
        actor->field_02 = 0;
        actor->field_0E = 1;
        actor->field_22 = 0;
        i++;
    } while (i < 20);
    g_wmap_land_effect_17_sequence_8_timer = 64;
    g_wmap_land_effect_17_sequence_8_step++;
    wmap_land_effect_17_sequence_8_step_02();
}

/** @brief Initialize the effect descriptor and actors with randomized angles. */
void wmap_land_effect_17_sequence_9_step_01(void)
{
    s32 i;
    s32 angle;
    WmapConfigA *actor;
    s32 actor_offset;
    s32 resource_offset;
    WmapResource *resource;
    WmapMotion *motion;

    i = 0;
    D_801B0FD0 = 30;
    D_80139280[0x15] = 192;
    D_80139280[0x16] = 16;
    D_80139280[0x17] = 128;
    D_80139280[0x18] = 0;
    D_80139280[0x19] = -1;
    D_80139280[0x1A] = 4000;
    D_80139280[0x1B] = 20;
    D_80139280[0x1C] = 8;
    D_80139280[0x1D] = 2;
    D_80139280[0x1E] = 10;
    do
    {
        WmapResource *resources;
        u8 *resource_data;

        motion = &D_801AFD60[i];
        motion->state = 1;
        angle = rand();
        resource_offset = (i + 20) * 8;
        actor_offset = i * 44;
        resources = D_80139988;
        resource_data = D_80121538;
        actor = (WmapConfigA *)((u8 *)D_800D95D8 + actor_offset);
        resource = (WmapResource *)((u8 *)resources + resource_offset);
        motion->angle = angle & 4095;
        motion->scale = 128;
        motion->z = 10;
        motion->x = 0;
        motion->field_0E = 0;
        resource->resource = resource_data;
        actor->field_06 = 15;
        actor->field_0E = 2;
        actor->field_10 = -1;
        actor->field_26 = 4;
        actor->field_24 = 127;
        actor->field_02 = 0;
        actor->field_22 = 0;
        i++;
    } while (i < 30);
    g_wmap_land_effect_17_sequence_9_timer = 64;
    g_wmap_land_effect_17_sequence_9_step++;
    wmap_land_effect_17_sequence_9_step_02();
}

/** @brief Initialize particle positions, velocities and animation resources. */
void wmap_land_effect_17_sequence_10_step_01(void)
{
    s32 i;
    WmapParticle *particles = D_800E4F18;

    for (i = 100; i < 124; i++)
    {
        particles[i].x = ((rand() * 200) >> 15) - 100;
        particles[i].y = ((rand() * 200) >> 15) - 100;
        particles[i].z = ((rand() * 150) >> 15) - 150;
        particles[i].vx = ((rand() << 6) >> 15) - 20;
        particles[i].vy = ((rand() << 6) >> 15) - 20;
        particles[i].vz = ((rand() << 6) >> 15) - 20;
        D_800D9268[i].resource_index = 0;
        D_800D9268[i].scale_index = 15;
        D_800D9268[i].sequence = (rand() * 3) >> 15;
        D_800D9268[i].previous_sequence = -1;
        D_800D9268[i].shade_step = 4;
        D_800D9268[i].target_shade = 129;
        D_800D9268[i].shade = 1;
        D_80139988[i].data = D_80121538;
    }
    particles[i].x = -100;
    particles[i].y = -100;
    particles[i].z = -150;
    particles[i].vx = 300;
    particles[i].vy = 300;
    particles[i].vz = 300;
    g_wmap_land_effect_17_sequence_10_timer = 112;
    g_wmap_land_effect_17_sequence_10_step++;
    wmap_land_effect_17_sequence_10_step_02();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_17_run(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_17_step = 1;
        g_wmap_land_effect_17_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_17_step < 0x6)
    {
        D_800D5C78[g_wmap_land_effect_17_step]();
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
void wmap_land_effect_17_reset(void)
{
    g_wmap_land_effect_17_step = 1;
    g_wmap_land_effect_17_timer = 1;
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_17_step_01(void)
{
    wmap_start_sequence(wmap_run_land_focus);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_17_step += 1;
    wmap_land_effect_17_wait_idle_02();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_17_wait_idle_02(void)
{
    if (g_wmap_sequence_busy == 0)
    {
        g_wmap_land_effect_17_step += 1;
        wmap_land_effect_17_step_03();
    }
}

/**
 * @brief Register the dispatch step, raise the run flag, advance the counter, and continue.
 */
void wmap_land_effect_17_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_17_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_17_step += 1;
    wmap_land_effect_17_wait_idle_04();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_land_effect_17_wait_idle_04(void)
{
    if (g_wmap_sequence_busy == 0)
    {
        g_wmap_land_effect_17_step += 1;
        wmap_land_effect_17_end();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_17_end(void)
{
    g_wmap_land_effect_17_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_17_run_timeline(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_17_timeline_step = 1;
        g_wmap_land_effect_17_timeline_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_17_timeline_step < 0x16)
    {
        D_800D5C90[g_wmap_land_effect_17_timeline_step]();
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
void wmap_land_effect_17_timeline_reset(void)
{
    g_wmap_land_effect_17_timeline_step = 1;
    g_wmap_land_effect_17_timeline_timer = 1;
}

/** @brief Set two flags, play sound 35, and register a callback before a four-tick delay. */
void wmap_land_effect_17_timeline_step_01(void)
{
    g_wmap_sequence_busy = 1;
    D_8013B208 = 1;
    wmap_play_sound(0x23, 0x80);
    wmap_start_sequence(&wmap_land_effect_17_run_sequence_3);
    g_wmap_land_effect_17_timeline_timer = 4;
    g_wmap_land_effect_17_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_17_timeline_wait_02(void)
{
    if (--g_wmap_land_effect_17_timeline_timer == 0)
    {
        g_wmap_land_effect_17_timeline_step += 1;
    }
}

/** @brief Register two callbacks, set world-map color and state, and begin an eight-tick delay. */
void wmap_land_effect_17_timeline_step_03(void)
{
    wmap_start_sequence(&wmap_land_effect_17_run_sequence_9);
    wmap_start_sequence(&wmap_land_effect_17_run_sequence_8);
    wmap_start_map_tint(0x103056);
    g_wmap_backdrop_target_level = 4;
    g_wmap_land_effect_17_timeline_timer = 8;
    g_wmap_land_effect_17_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_17_timeline_wait_04(void)
{
    if (--g_wmap_land_effect_17_timeline_timer == 0)
    {
        g_wmap_land_effect_17_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_17_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_17_run_sequence_1);
    g_wmap_land_effect_17_timeline_timer = 0x2;
    g_wmap_land_effect_17_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_17_timeline_wait_06(void)
{
    if (--g_wmap_land_effect_17_timeline_timer == 0)
    {
        g_wmap_land_effect_17_timeline_step += 1;
    }
}

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void wmap_land_effect_17_timeline_step_07(void)
{
    D_801ADAE0 = 1;
    g_wmap_land_effect_17_timeline_timer = 0xC;
    g_wmap_land_effect_17_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_17_timeline_wait_08(void)
{
    if (--g_wmap_land_effect_17_timeline_timer == 0)
    {
        g_wmap_land_effect_17_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_17_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_effect_17_run_sequence_7);
    g_wmap_land_effect_17_timeline_timer = 0x2;
    g_wmap_land_effect_17_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_17_timeline_wait_10(void)
{
    if (--g_wmap_land_effect_17_timeline_timer == 0)
    {
        g_wmap_land_effect_17_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_17_timeline_step_11(void)
{
    wmap_start_sequence(wmap_land_effect_17_run_sequence_10);
    g_wmap_land_effect_17_timeline_timer = 0x32;
    g_wmap_land_effect_17_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_17_timeline_wait_12(void)
{
    if (--g_wmap_land_effect_17_timeline_timer == 0)
    {
        g_wmap_land_effect_17_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_17_timeline_step_13(void)
{
    wmap_start_sequence(wmap_land_effect_17_run_sequence_4);
    g_wmap_land_effect_17_timeline_timer = 0x2;
    g_wmap_land_effect_17_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_17_timeline_wait_14(void)
{
    if (--g_wmap_land_effect_17_timeline_timer == 0)
    {
        g_wmap_land_effect_17_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_17_timeline_step_15(void)
{
    wmap_start_sequence(wmap_land_effect_17_run_sequence_2);
    g_wmap_land_effect_17_timeline_timer = 0x7A;
    g_wmap_land_effect_17_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_17_timeline_wait_16(void)
{
    if (--g_wmap_land_effect_17_timeline_timer == 0)
    {
        g_wmap_land_effect_17_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_17_timeline_step_17(void)
{
    wmap_start_sequence(wmap_land_effect_17_run_sequence_5);
    g_wmap_land_effect_17_timeline_timer = 0x10;
    g_wmap_land_effect_17_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_17_timeline_wait_18(void)
{
    if (--g_wmap_land_effect_17_timeline_timer == 0)
    {
        g_wmap_land_effect_17_timeline_step += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_effect_17_timeline_step_19(void)
{
    wmap_start_sequence(wmap_land_effect_17_run_sequence_6);
    g_wmap_land_effect_17_timeline_timer = 0x68;
    g_wmap_land_effect_17_timeline_step += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void wmap_land_effect_17_timeline_wait_20(void)
{
    if (--g_wmap_land_effect_17_timeline_timer == 0)
    {
        g_wmap_land_effect_17_timeline_step += 1;
    }
}

/** @brief Mark the current world-map tile and advance the sequence step. */
void wmap_land_effect_17_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    D_80139290[D_8011D510][D_8011D530].tile = D_8011D4FC | 0x100;
    g_wmap_land_effect_17_timeline_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_17_run_sequence_1(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_17_sequence_1_step = 1;
        g_wmap_land_effect_17_sequence_1_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_17_sequence_1_step < 0x4)
    {
        D_800D5CE8[g_wmap_land_effect_17_sequence_1_step]();
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
void wmap_land_effect_17_sequence_1_reset(void)
{
    g_wmap_land_effect_17_sequence_1_step = 1;
    g_wmap_land_effect_17_sequence_1_timer = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void wmap_land_effect_17_sequence_1_step_01(void)
{
    D_801399AC = D_8011D538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.shade_step = 0;
    D_800D9318.target_shade = 0x80;
    D_800D9318.shade = 0x80;
    g_wmap_land_effect_17_sequence_1_timer = 0x48;
    g_wmap_land_effect_17_sequence_1_step += 1;
    wmap_land_effect_17_sequence_1_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_17_sequence_1_step_02(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0x1B, 0x1E, 0);
    if (--g_wmap_land_effect_17_sequence_1_timer == 0)
    {
        g_wmap_land_effect_17_sequence_1_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_17_sequence_1_end(void)
{
    g_wmap_land_effect_17_sequence_1_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_17_run_sequence_2(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_17_sequence_2_step = 1;
        g_wmap_land_effect_17_sequence_2_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_17_sequence_2_step < 0x4)
    {
        D_800D5CF8[g_wmap_land_effect_17_sequence_2_step]();
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
void wmap_land_effect_17_sequence_2_reset(void)
{
    g_wmap_land_effect_17_sequence_2_step = 1;
    g_wmap_land_effect_17_sequence_2_timer = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_17_sequence_2_step_01(void)
{
    D_801399B4 = D_8011F538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.previous_sequence = -1;
    D_800D9344.shade_step = 8;
    D_800D9344.resource_index = 0;
    D_800D9344.sequence = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0;
    g_wmap_land_effect_17_sequence_2_timer = 0xF5;
    g_wmap_land_effect_17_sequence_2_step += 1;
    wmap_land_effect_17_sequence_2_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_17_sequence_2_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x1A, 0x1E, 0);
    if (--g_wmap_land_effect_17_sequence_2_timer == 0)
    {
        g_wmap_land_effect_17_sequence_2_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_17_sequence_2_end(void)
{
    g_wmap_land_effect_17_sequence_2_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_17_run_sequence_3(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_17_sequence_3_step = 1;
        g_wmap_land_effect_17_sequence_3_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_17_sequence_3_step < 0x4)
    {
        D_800D5D08[g_wmap_land_effect_17_sequence_3_step]();
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
void wmap_land_effect_17_sequence_3_reset(void)
{
    g_wmap_land_effect_17_sequence_3_step = 1;
    g_wmap_land_effect_17_sequence_3_timer = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_17_sequence_3_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    g_wmap_land_effect_17_sequence_3_timer = 0x40;
    g_wmap_land_effect_17_sequence_3_step += 1;
    wmap_land_effect_17_sequence_3_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_17_sequence_3_end(void)
{
    g_wmap_land_effect_17_sequence_3_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_17_run_sequence_4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_17_sequence_4_step = 1;
        g_wmap_land_effect_17_sequence_4_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_17_sequence_4_step < 0x4)
    {
        D_800D5D18[g_wmap_land_effect_17_sequence_4_step]();
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
void wmap_land_effect_17_sequence_4_reset(void)
{
    g_wmap_land_effect_17_sequence_4_step = 1;
    g_wmap_land_effect_17_sequence_4_timer = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_17_sequence_4_step_01(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = g_wmap_camera_translation;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    g_wmap_land_effect_17_sequence_4_timer = 0x100;
    g_wmap_land_effect_17_sequence_4_step += 1;
    wmap_land_effect_17_sequence_4_step_02();
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_17_sequence_4_end(void)
{
    g_wmap_land_effect_17_sequence_4_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_17_run_sequence_5(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_17_sequence_5_step = 1;
        g_wmap_land_effect_17_sequence_5_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_17_sequence_5_step < 0x4)
    {
        D_800D5D28[g_wmap_land_effect_17_sequence_5_step]();
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
void wmap_land_effect_17_sequence_5_reset(void)
{
    g_wmap_land_effect_17_sequence_5_step = 1;
    g_wmap_land_effect_17_sequence_5_timer = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_17_sequence_5_step_02(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0x3, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--g_wmap_land_effect_17_sequence_5_timer == 0)
    {
        g_wmap_land_effect_17_sequence_5_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_17_sequence_5_end(void)
{
    g_wmap_land_effect_17_sequence_5_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_17_run_sequence_6(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_17_sequence_6_step = 1;
        g_wmap_land_effect_17_sequence_6_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_17_sequence_6_step < 0x4)
    {
        D_800D5D38[g_wmap_land_effect_17_sequence_6_step]();
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
void wmap_land_effect_17_sequence_6_reset(void)
{
    g_wmap_land_effect_17_sequence_6_step = 1;
    g_wmap_land_effect_17_sequence_6_timer = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_17_sequence_6_step_02(void)
{
    func_8006A2FC(D_800D9B00, D_80139B18, 0x3, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--g_wmap_land_effect_17_sequence_6_timer == 0)
    {
        g_wmap_land_effect_17_sequence_6_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_17_sequence_6_end(void)
{
    g_wmap_land_effect_17_sequence_6_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_17_run_sequence_7(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_17_sequence_7_step = 1;
        g_wmap_land_effect_17_sequence_7_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_17_sequence_7_step < 0x6)
    {
        D_800D5D48[g_wmap_land_effect_17_sequence_7_step]();
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
void wmap_land_effect_17_sequence_7_reset(void)
{
    g_wmap_land_effect_17_sequence_7_step = 1;
    g_wmap_land_effect_17_sequence_7_timer = 1;
}

/**
 * @brief Draw the world-map cursor via func_8006B6EC, then advance after the wait expires.
 */
void wmap_land_effect_17_sequence_7_step_02(void)
{
    func_8006B6EC(0x50, 0x58, 0x8, 0, 0x1C);
    if (--g_wmap_land_effect_17_sequence_7_timer == 0)
    {
        g_wmap_land_effect_17_sequence_7_step += 1;
    }
}

/**
 * @brief Reset a range of world-map actor configs, arm the timer, and advance the step.
 */
void wmap_land_effect_17_sequence_7_step_03(void)
{
    s32 i;

    for (i = 0x50; i < 0x58; i++)
    {
        D_800D9268[i].target_shade = 0;
        D_800D9268[i].shade_step = 8;
    }
    g_wmap_land_effect_17_sequence_7_timer = 0x10;
    g_wmap_land_effect_17_sequence_7_step += 1;
    wmap_land_effect_17_sequence_7_step_04();
}

/**
 * @brief Draw the world-map cursor via func_8006B6EC, then advance after the wait expires.
 */
void wmap_land_effect_17_sequence_7_step_04(void)
{
    func_8006B6EC(0x50, 0x58, 0x8, 0, 0x1C);
    if (--g_wmap_land_effect_17_sequence_7_timer == 0)
    {
        g_wmap_land_effect_17_sequence_7_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_17_sequence_7_end(void)
{
    g_wmap_land_effect_17_sequence_7_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_17_run_sequence_8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_17_sequence_8_step = 1;
        g_wmap_land_effect_17_sequence_8_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_17_sequence_8_step < 0x4)
    {
        D_800D5D60[g_wmap_land_effect_17_sequence_8_step]();
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
void wmap_land_effect_17_sequence_8_reset(void)
{
    g_wmap_land_effect_17_sequence_8_step = 1;
    g_wmap_land_effect_17_sequence_8_timer = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_17_sequence_8_step_02(void)
{
    func_8006A2FC(D_800D9B00, D_80139B18, 0x14, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--g_wmap_land_effect_17_sequence_8_timer == 0)
    {
        g_wmap_land_effect_17_sequence_8_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_17_sequence_8_end(void)
{
    g_wmap_land_effect_17_sequence_8_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_17_run_sequence_9(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_17_sequence_9_step = 1;
        g_wmap_land_effect_17_sequence_9_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_17_sequence_9_step < 0x4)
    {
        D_800D5D70[g_wmap_land_effect_17_sequence_9_step]();
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
void wmap_land_effect_17_sequence_9_reset(void)
{
    g_wmap_land_effect_17_sequence_9_step = 1;
    g_wmap_land_effect_17_sequence_9_timer = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_17_sequence_9_step_02(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0x1E, 0, 0x7F, 0x4, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--g_wmap_land_effect_17_sequence_9_timer == 0)
    {
        g_wmap_land_effect_17_sequence_9_step += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_17_sequence_9_end(void)
{
    g_wmap_land_effect_17_sequence_9_step += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 wmap_land_effect_17_run_sequence_10(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        g_wmap_land_effect_17_sequence_10_step = 1;
        g_wmap_land_effect_17_sequence_10_timer = 1;
        return 1;
    }

    if (g_wmap_land_effect_17_sequence_10_step < 0x6)
    {
        D_800D5D80[g_wmap_land_effect_17_sequence_10_step]();
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
void wmap_land_effect_17_sequence_10_reset(void)
{
    g_wmap_land_effect_17_sequence_10_step = 1;
    g_wmap_land_effect_17_sequence_10_timer = 1;
}

/** @brief Draw the sequence effect and advance when its countdown expires. */
void wmap_land_effect_17_sequence_10_step_02(void)
{
    s32 remaining_ticks;

    func_8006B998(0x64, 0x7C, D_800E4F18, 8, 0xA);
    remaining_ticks = g_wmap_land_effect_17_sequence_10_timer - 1;
    g_wmap_land_effect_17_sequence_10_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_17_sequence_10_step += 1;
    }
}

/** @brief Set particle display parameters and begin their countdown. */
void wmap_land_effect_17_sequence_10_step_03(void)
{
    s32 i;

    for (i = 100; i < 124; i++)
    {
        D_800D9268[i].shade_step = 4;
        D_800D9268[i].target_shade = 1;
    }
    g_wmap_land_effect_17_sequence_10_timer = 64;
    g_wmap_land_effect_17_sequence_10_step++;
    wmap_land_effect_17_sequence_10_step_04();
}

/** @brief Draw the particle range and advance when its countdown expires. */
void wmap_land_effect_17_sequence_10_step_04(void)
{
    s32 remaining;

    func_8006B998(100, 124, D_800E4F18, 8, 10);
    remaining = g_wmap_land_effect_17_sequence_10_timer - 1;
    g_wmap_land_effect_17_sequence_10_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_17_sequence_10_step++;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void wmap_land_effect_17_sequence_10_end(void)
{
    g_wmap_land_effect_17_sequence_10_step += 1;
}
