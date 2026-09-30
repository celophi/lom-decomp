#include "wmap_main.h"
#include "wmap_land_effect_17.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"
#include "wmap_step_sequence.h"

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

extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_17_sequence_3_timer;
extern u8* g_wmap_effect_model_pack_1;
extern s32 g_wmap_land_effect_17_sequence_4_timer;
extern u8 g_wmap_animation_bank_0[];
extern s32 g_wmap_land_effect_17_sequence_5_timer;
extern s32 g_wmap_land_effect_17_sequence_6_timer;
extern u8 g_wmap_animation_bank_2[];
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
extern WmapTile g_wmap_cells[6][6];
extern u32 g_wmap_selected_artifact;
extern s32 g_wmap_land_effect_17_sequence_1_timer;
extern void (*D_800D5CE8[])(void);
extern u8* D_801399AC;
extern s32 g_wmap_land_effect_17_sequence_2_timer;
extern void (*D_800D5CF8[])(void);
extern u8 g_wmap_animation_bank_1[];
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


extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;

extern WmapAnimationSlot g_wmap_actor_animations[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* g_wmap_effect_params;

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

extern WmapMotion g_wmap_actor_motions[];
extern WmapMotion D_801AFD60[];
extern WmapMotion D_801AFFB8[];

/**
 * @brief Advance a world-map model's spin, draw it while active, then countdown-advance the step.
 */
void wmap_land_effect_17_sequence_3_step_02(void)
{
    MATRIX m;
    s32 x;

    x = g_wmap_effect_model_a_position.vz - 0xDAC;
    g_wmap_effect_model_a_position.vz = x;
    if (x < 0x2710)
    {
        g_wmap_effect_model_a_position.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&g_wmap_effect_model_a_rotation, &m);
    TransMatrix(&m, &g_wmap_zero_translation);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (g_wmap_effect_fade_a != 0)
    {
        wmap_draw_model_default((s32)g_wmap_load_buffer, 0, 0x4, 0x35, 0x7800, 0x1, g_wmap_effect_fade_a);
        g_wmap_effect_fade_a -= 0x4;
        if (g_wmap_effect_fade_a < 0)
        {
            g_wmap_effect_fade_a = 0;
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

    x = g_wmap_effect_model_b_position.vz - 0xDAC;
    g_wmap_effect_model_b_position.vz = x;
    if (x < 0x2710)
    {
        g_wmap_effect_model_b_position.vz = 0x2710;
    }

    PushMatrix();
    RotMatrix(&g_wmap_effect_model_b_rotation, &m);
    TransMatrix(&m, &g_wmap_zero_translation);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (g_wmap_effect_fade_b != 0)
    {
        wmap_draw_model_default(g_wmap_effect_model_pack_1, 0, 0x4, 0x35, 0x7800, 0x1, g_wmap_effect_fade_b);
        g_wmap_effect_fade_b -= 1;
        if (g_wmap_effect_fade_b < 0)
        {
            g_wmap_effect_fade_b = 0;
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
    g_wmap_particle_intensity = 3;
    g_wmap_effect_params[0x15] = 20;
    g_wmap_effect_params[0x16] = 20;
    g_wmap_effect_params[0x17] = 128;
    g_wmap_effect_params[0x18] = 0;
    g_wmap_effect_params[0x19] = -1;
    g_wmap_effect_params[0x1A] = 1000;
    g_wmap_effect_params[0x1B] = 20;
    g_wmap_effect_params[0x1C] = 15;
    g_wmap_effect_params[0x1D] = 1;
    g_wmap_effect_params[0x1E] = 12000;
    do
    {
        D_801AFD60[i].state = 1;
        D_801AFD60[i].angle = i * 1365;
        D_801AFD60[i].scale = 128;
        D_801AFD60[i].z = 12000;
        D_801AFD60[i].x = 0;
        D_801AFD60[i].field_0E = 0;
        g_wmap_actor_animations[i + 20].data = g_wmap_animation_bank_0;
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
    g_wmap_particle_intensity = 3;
    g_wmap_effect_params[0xB] = 4;
    g_wmap_effect_params[0xC] = 4;
    g_wmap_effect_params[0xD] = 128;
    g_wmap_effect_params[0xE] = 0;
    g_wmap_effect_params[0xF] = -1;
    g_wmap_effect_params[0x10] = 100;
    g_wmap_effect_params[0x11] = 50;
    g_wmap_effect_params[0x12] = 15;
    g_wmap_effect_params[0x13] = 2;
    g_wmap_effect_params[0x14] = 12000;
    do
    {
        D_801AFFB8[i].state = 1;
        D_801AFFB8[i].angle = i * 1365;
        D_801AFFB8[i].scale = 128;
        D_801AFFB8[i].z = 12000;
        D_801AFFB8[i].x = 0;
        D_801AFFB8[i].field_0E = 0;
        g_wmap_actor_animations[i + 50].data = g_wmap_animation_bank_0;
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

    g_wmap_particle_intensity = 8;
    angle = (s16)0x27FD8;
    for (i = 80; i < 88; i++)
    {
        actor = &g_wmap_sprite_actors[i];
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_2;
        actor->field_02 = 0;
        actor->field_06 = 15;
        actor->field_0E = 2;
        actor->field_10 = -1;
        actor->field_22 = 129;
        actor->field_24 = 1;
        actor->field_26 = 8;
        g_wmap_actor_motions[i].state = 1;
        g_wmap_actor_motions[i].angle = i << 9;
        g_wmap_actor_motions[i].x = -4000;
        g_wmap_actor_motions[i].z = 480000;
        g_wmap_actor_motions[i].scale = 9999;
        g_wmap_actor_motions[i].field_0E = 0;
        g_wmap_actor_motions[i].field_10 = 144;
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
    g_wmap_particle_intensity = 20;
    g_wmap_effect_params[0xB] = 64;
    g_wmap_effect_params[0xC] = 20;
    g_wmap_effect_params[0xD] = 128;
    g_wmap_effect_params[0xE] = 0;
    g_wmap_effect_params[0xF] = -1;
    g_wmap_effect_params[0x10] = 2000;
    g_wmap_effect_params[0x11] = 50;
    g_wmap_effect_params[0x12] = 8;
    g_wmap_effect_params[0x13] = 1;
    g_wmap_effect_params[0x14] = 10;
    do
    {
        WmapResource *resources;
        u8 *resource_data;

        motion = &D_801AFFB8[i];
        motion->state = 1;
        angle = rand();
        resource_offset = (i + 50) * 8;
        actor_offset = i * 44;
        resources = g_wmap_actor_animations;
        resource_data = g_wmap_animation_bank_2;
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
    g_wmap_particle_intensity = 30;
    g_wmap_effect_params[0x15] = 192;
    g_wmap_effect_params[0x16] = 16;
    g_wmap_effect_params[0x17] = 128;
    g_wmap_effect_params[0x18] = 0;
    g_wmap_effect_params[0x19] = -1;
    g_wmap_effect_params[0x1A] = 4000;
    g_wmap_effect_params[0x1B] = 20;
    g_wmap_effect_params[0x1C] = 8;
    g_wmap_effect_params[0x1D] = 2;
    g_wmap_effect_params[0x1E] = 10;
    do
    {
        WmapResource *resources;
        u8 *resource_data;

        motion = &D_801AFD60[i];
        motion->state = 1;
        angle = rand();
        resource_offset = (i + 20) * 8;
        actor_offset = i * 44;
        resources = g_wmap_actor_animations;
        resource_data = g_wmap_animation_bank_2;
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
        g_wmap_sprite_actors[i].resource_index = 0;
        g_wmap_sprite_actors[i].scale_index = 15;
        g_wmap_sprite_actors[i].sequence = (rand() * 3) >> 15;
        g_wmap_sprite_actors[i].previous_sequence = -1;
        g_wmap_sprite_actors[i].shade_step = 4;
        g_wmap_sprite_actors[i].target_shade = 129;
        g_wmap_sprite_actors[i].shade = 1;
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_2;
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

WMAP_STEP_RUNNER(wmap_land_effect_17_run, D_800D5C78, 0x6, g_wmap_land_effect_17_step, g_wmap_land_effect_17_timer)

WMAP_STEP_RESET(wmap_land_effect_17_reset, g_wmap_land_effect_17_step, g_wmap_land_effect_17_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_17_step_01, g_wmap_land_effect_17_step, wmap_run_land_focus, wmap_land_effect_17_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_17_wait_idle_02, g_wmap_land_effect_17_step, wmap_land_effect_17_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_17_step_03, g_wmap_land_effect_17_step, wmap_land_effect_17_run_timeline, wmap_land_effect_17_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_17_wait_idle_04, g_wmap_land_effect_17_step, wmap_land_effect_17_end)

WMAP_STEP_ADVANCE(wmap_land_effect_17_end, g_wmap_land_effect_17_step)

WMAP_STEP_RUNNER(wmap_land_effect_17_run_timeline, D_800D5C90, 0x16, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_17_timeline_reset, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer)

/** @brief Set two flags, play sound 35, and register a callback before a four-tick delay. */
void wmap_land_effect_17_timeline_step_01(void)
{
    g_wmap_sequence_busy = 1;
    g_wmap_event_active = 1;
    wmap_play_sound(0x23, 0x80);
    wmap_start_sequence(&wmap_land_effect_17_run_sequence_3);
    g_wmap_land_effect_17_timeline_timer = 4;
    g_wmap_land_effect_17_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_17_timeline_wait_02, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer)

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

WMAP_STEP_WAIT(wmap_land_effect_17_timeline_wait_04, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_17_timeline_step_05, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer,
                         wmap_land_effect_17_run_sequence_1, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_17_timeline_wait_06, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer)

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void wmap_land_effect_17_timeline_step_07(void)
{
    g_wmap_placement_overlay_hidden = 1;
    g_wmap_land_effect_17_timeline_timer = 0xC;
    g_wmap_land_effect_17_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_17_timeline_wait_08, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_17_timeline_step_09, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer,
                         wmap_land_effect_17_run_sequence_7, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_17_timeline_wait_10, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_17_timeline_step_11, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer,
                         wmap_land_effect_17_run_sequence_10, 0x32)

WMAP_STEP_WAIT(wmap_land_effect_17_timeline_wait_12, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_17_timeline_step_13, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer,
                         wmap_land_effect_17_run_sequence_4, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_17_timeline_wait_14, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_17_timeline_step_15, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer,
                         wmap_land_effect_17_run_sequence_2, 0x7A)

WMAP_STEP_WAIT(wmap_land_effect_17_timeline_wait_16, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_17_timeline_step_17, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer,
                         wmap_land_effect_17_run_sequence_5, 0x10)

WMAP_STEP_WAIT(wmap_land_effect_17_timeline_wait_18, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_17_timeline_step_19, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer,
                         wmap_land_effect_17_run_sequence_6, 0x68)

WMAP_STEP_WAIT(wmap_land_effect_17_timeline_wait_20, g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer)

/** @brief Mark the current world-map tile and advance the sequence step. */
void wmap_land_effect_17_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    g_wmap_cells[g_wmap_focus_cell_x][g_wmap_focus_cell_y].tile = g_wmap_selected_artifact | 0x100;
    g_wmap_land_effect_17_timeline_step += 1;
}

WMAP_STEP_RUNNER(wmap_land_effect_17_run_sequence_1, D_800D5CE8, 0x4, g_wmap_land_effect_17_sequence_1_step, g_wmap_land_effect_17_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_17_sequence_1_reset, g_wmap_land_effect_17_sequence_1_step, g_wmap_land_effect_17_sequence_1_timer)

/**
 * @brief Populate a world-map actor control block and schedule its next step.
 */
void wmap_land_effect_17_sequence_1_step_01(void)
{
    D_801399AC = g_wmap_animation_bank_0;
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

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_17_sequence_1_step_02, g_wmap_land_effect_17_sequence_1_step, g_wmap_land_effect_17_sequence_1_timer, D_800D9318,
                              D_801399A8, g_wmap_focus_screen_position, 0x1B, 0x1E, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_17_sequence_1_end, g_wmap_land_effect_17_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_17_run_sequence_2, D_800D5CF8, 0x4, g_wmap_land_effect_17_sequence_2_step, g_wmap_land_effect_17_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_17_sequence_2_reset, g_wmap_land_effect_17_sequence_2_step, g_wmap_land_effect_17_sequence_2_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_17_sequence_2_step_01(void)
{
    D_801399B4 = g_wmap_animation_bank_1;
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

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_17_sequence_2_step_02, g_wmap_land_effect_17_sequence_2_step, g_wmap_land_effect_17_sequence_2_timer, D_800D9344,
                              D_801399B0, g_wmap_focus_screen_position, 0x1A, 0x1E, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_17_sequence_2_end, g_wmap_land_effect_17_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_17_run_sequence_3, D_800D5D08, 0x4, g_wmap_land_effect_17_sequence_3_step, g_wmap_land_effect_17_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_17_sequence_3_reset, g_wmap_land_effect_17_sequence_3_step, g_wmap_land_effect_17_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_17_sequence_3_step_01, g_wmap_land_effect_17_sequence_3_step, g_wmap_land_effect_17_sequence_3_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40, wmap_land_effect_17_sequence_3_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_17_sequence_3_end, g_wmap_land_effect_17_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_17_run_sequence_4, D_800D5D18, 0x4, g_wmap_land_effect_17_sequence_4_step, g_wmap_land_effect_17_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_17_sequence_4_reset, g_wmap_land_effect_17_sequence_4_step, g_wmap_land_effect_17_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_17_sequence_4_step_01, g_wmap_land_effect_17_sequence_4_step, g_wmap_land_effect_17_sequence_4_timer, g_wmap_effect_model_b_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x100, wmap_land_effect_17_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_17_sequence_4_end, g_wmap_land_effect_17_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_17_run_sequence_5, D_800D5D28, 0x4, g_wmap_land_effect_17_sequence_5_step, g_wmap_land_effect_17_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_17_sequence_5_reset, g_wmap_land_effect_17_sequence_5_step, g_wmap_land_effect_17_sequence_5_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_17_sequence_5_step_02(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0x3, 0, 0x7F, 0x2, 0, (s32)((u8*)g_wmap_effect_params + 0x50));
    if (--g_wmap_land_effect_17_sequence_5_timer == 0)
    {
        g_wmap_land_effect_17_sequence_5_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_17_sequence_5_end, g_wmap_land_effect_17_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_17_run_sequence_6, D_800D5D38, 0x4, g_wmap_land_effect_17_sequence_6_step, g_wmap_land_effect_17_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_17_sequence_6_reset, g_wmap_land_effect_17_sequence_6_step, g_wmap_land_effect_17_sequence_6_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_17_sequence_6_step_02(void)
{
    func_8006A2FC(D_800D9B00, D_80139B18, 0x3, 0, 0x7F, 0x2, 0, (s32)((u8*)g_wmap_effect_params + 0x28));
    if (--g_wmap_land_effect_17_sequence_6_timer == 0)
    {
        g_wmap_land_effect_17_sequence_6_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_17_sequence_6_end, g_wmap_land_effect_17_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_17_run_sequence_7, D_800D5D48, 0x6, g_wmap_land_effect_17_sequence_7_step, g_wmap_land_effect_17_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_17_sequence_7_reset, g_wmap_land_effect_17_sequence_7_step, g_wmap_land_effect_17_sequence_7_timer)

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
        g_wmap_sprite_actors[i].target_shade = 0;
        g_wmap_sprite_actors[i].shade_step = 8;
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

WMAP_STEP_ADVANCE(wmap_land_effect_17_sequence_7_end, g_wmap_land_effect_17_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_17_run_sequence_8, D_800D5D60, 0x4, g_wmap_land_effect_17_sequence_8_step, g_wmap_land_effect_17_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_17_sequence_8_reset, g_wmap_land_effect_17_sequence_8_step, g_wmap_land_effect_17_sequence_8_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_17_sequence_8_step_02(void)
{
    func_8006A2FC(D_800D9B00, D_80139B18, 0x14, 0, 0x7F, 0x2, 0, (s32)((u8*)g_wmap_effect_params + 0x28));
    if (--g_wmap_land_effect_17_sequence_8_timer == 0)
    {
        g_wmap_land_effect_17_sequence_8_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_17_sequence_8_end, g_wmap_land_effect_17_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_17_run_sequence_9, D_800D5D70, 0x4, g_wmap_land_effect_17_sequence_9_step, g_wmap_land_effect_17_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_17_sequence_9_reset, g_wmap_land_effect_17_sequence_9_step, g_wmap_land_effect_17_sequence_9_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_17_sequence_9_step_02(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0x1E, 0, 0x7F, 0x4, 0, (s32)((u8*)g_wmap_effect_params + 0x50));
    if (--g_wmap_land_effect_17_sequence_9_timer == 0)
    {
        g_wmap_land_effect_17_sequence_9_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_17_sequence_9_end, g_wmap_land_effect_17_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_17_run_sequence_10, D_800D5D80, 0x6, g_wmap_land_effect_17_sequence_10_step, g_wmap_land_effect_17_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_17_sequence_10_reset, g_wmap_land_effect_17_sequence_10_step, g_wmap_land_effect_17_sequence_10_timer)

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
        g_wmap_sprite_actors[i].shade_step = 4;
        g_wmap_sprite_actors[i].target_shade = 1;
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

WMAP_STEP_ADVANCE(wmap_land_effect_17_sequence_10_end, g_wmap_land_effect_17_sequence_10_step)
