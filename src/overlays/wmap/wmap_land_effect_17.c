#include "wmap_main.h"
#include "wmap_land_effect_17.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_resource_support.h"
#include "wmap_view_effects.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"
#include "wmap_step_sequence.h"
#include "wmap_cells.h"

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
extern u32 g_wmap_selected_artifact;
extern s32 g_wmap_land_effect_17_sequence_1_timer;
extern void (*D_800D5CE8[])(void);
extern s32 g_wmap_land_effect_17_sequence_2_timer;
extern void (*D_800D5CF8[])(void);
extern u8 g_wmap_animation_bank_1[];
extern void wmap_land_effect_17_sequence_2_step_02(void);
extern void (*D_800D5D08[])(void);
extern void (*D_800D5D18[])(void);
extern void (*D_800D5D28[])(void);
extern void (*D_800D5D38[])(void);
extern void (*D_800D5D48[])(void);
extern void (*D_800D5D60[])(void);
extern void (*D_800D5D70[])(void);
extern void (*D_800D5D80[])(void);
extern u32 g_wmap_land_effect_17_sequence_3_step;
extern u32 g_wmap_land_effect_17_sequence_4_step;
extern u32 g_wmap_land_effect_17_sequence_5_step;
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



extern WmapAnimationSlot g_wmap_actor_animations[];

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

extern WmapMotion g_wmap_actor_motions[];
extern WmapMotion D_801AFD60[];
extern WmapMotion D_801AFFB8[];

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_17_sequence_3_step_02,
    g_wmap_land_effect_17_sequence_3_step, g_wmap_land_effect_17_sequence_3_timer,
    g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position,
    g_wmap_effect_fade_a, g_wmap_load_buffer, -3500, 4)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_17_sequence_4_step_02,
    g_wmap_land_effect_17_sequence_4_step, g_wmap_land_effect_17_sequence_4_timer,
    g_wmap_effect_model_b_rotation, g_wmap_effect_model_b_position,
    g_wmap_effect_fade_b, g_wmap_effect_model_pack_1, -3500, 1)

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
        WMAP_ACTOR_BLOCK(20)[i].scale_index = 15;
        WMAP_ACTOR_BLOCK(20)[i].previous_sequence = -1;
        WMAP_ACTOR_BLOCK(20)[i].shade_step = 2;
        WMAP_ACTOR_BLOCK(20)[i].resource_index = 0;
        WMAP_ACTOR_BLOCK(20)[i].sequence = 1;
        WMAP_ACTOR_BLOCK(20)[i].target_shade = 0;
        WMAP_ACTOR_BLOCK(20)[i].shade = 127;
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
        WMAP_ACTOR_BLOCK(50)[i].scale_index = 15;
        WMAP_ACTOR_BLOCK(50)[i].previous_sequence = -1;
        WMAP_ACTOR_BLOCK(50)[i].resource_index = 0;
        WMAP_ACTOR_BLOCK(50)[i].sequence = 2;
        WMAP_ACTOR_BLOCK(50)[i].shade_step = 2;
        WMAP_ACTOR_BLOCK(50)[i].target_shade = 0;
        WMAP_ACTOR_BLOCK(50)[i].shade = 127;
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
    WmapSpriteActor *actor;

    g_wmap_particle_intensity = 8;
    angle = (s16)0x27FD8;
    for (i = 80; i < 88; i++)
    {
        actor = &g_wmap_sprite_actors[i];
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_2;
        actor->resource_index = 0;
        actor->scale_index = 15;
        actor->sequence = 2;
        actor->previous_sequence = -1;
        actor->target_shade = 129;
        actor->shade = 1;
        actor->shade_step = 8;
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
    WmapSpriteActor *actor;
    s32 actor_offset;
    s32 resource_offset;
    WmapAnimationSlot *resource;
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
        WmapAnimationSlot *resources;
        u8 *resource_data;

        motion = &D_801AFFB8[i];
        motion->state = 1;
        angle = rand();
        resource_offset = (i + 50) * 8;
        actor_offset = i * 44;
        resources = g_wmap_actor_animations;
        resource_data = g_wmap_animation_bank_2;
        actor = (WmapSpriteActor *)((u8 *)&g_wmap_sprite_actors[50] + actor_offset);
        resource = (WmapAnimationSlot *)((u8 *)resources + resource_offset);
        motion->angle = angle & 4095;
        motion->scale = 128;
        motion->z = 10;
        motion->x = 0;
        motion->field_0E = 0;
        resource->data = resource_data;
        actor->scale_index = 15;
        actor->previous_sequence = -1;
        actor->shade_step = 2;
        actor->shade = 127;
        actor->resource_index = 0;
        actor->sequence = 1;
        actor->target_shade = 0;
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
    WmapSpriteActor *actor;
    s32 actor_offset;
    s32 resource_offset;
    WmapAnimationSlot *resource;
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
        WmapAnimationSlot *resources;
        u8 *resource_data;

        motion = &D_801AFD60[i];
        motion->state = 1;
        angle = rand();
        resource_offset = (i + 20) * 8;
        actor_offset = i * 44;
        resources = g_wmap_actor_animations;
        resource_data = g_wmap_animation_bank_2;
        actor = (WmapSpriteActor *)((u8 *)&g_wmap_sprite_actors[20] + actor_offset);
        resource = (WmapAnimationSlot *)((u8 *)resources + resource_offset);
        motion->angle = angle & 4095;
        motion->scale = 128;
        motion->z = 10;
        motion->x = 0;
        motion->field_0E = 0;
        resource->data = resource_data;
        actor->scale_index = 15;
        actor->sequence = 2;
        actor->previous_sequence = -1;
        actor->shade_step = 4;
        actor->shade = 127;
        actor->resource_index = 0;
        actor->target_shade = 0;
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
 * @brief Hide the placement overlay and set the wait timer.
 */
WMAP_STEP_HIDE_AND_WAIT(wmap_land_effect_17_timeline_step_07,
    g_wmap_land_effect_17_timeline_step, g_wmap_land_effect_17_timeline_timer,
    g_wmap_placement_overlay_hidden, 0xC)

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
    g_wmap_cells[g_wmap_focus_cell_x][g_wmap_focus_cell_y].land_id = g_wmap_selected_artifact | 0x100;
    g_wmap_land_effect_17_timeline_step += 1;
}

WMAP_STEP_RUNNER(wmap_land_effect_17_run_sequence_1, D_800D5CE8, 0x4, g_wmap_land_effect_17_sequence_1_step, g_wmap_land_effect_17_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_17_sequence_1_reset, g_wmap_land_effect_17_sequence_1_step, g_wmap_land_effect_17_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_17_sequence_1_step_01,
    g_wmap_land_effect_17_sequence_1_step, g_wmap_land_effect_17_sequence_1_timer,
    4, g_wmap_animation_bank_0, 0,
    0x80, 0x80, 0,
    0x48, wmap_land_effect_17_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_17_sequence_1_step_02, g_wmap_land_effect_17_sequence_1_step, g_wmap_land_effect_17_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0x1B, 0x1E, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_17_sequence_1_end, g_wmap_land_effect_17_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_17_run_sequence_2, D_800D5CF8, 0x4, g_wmap_land_effect_17_sequence_2_step, g_wmap_land_effect_17_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_17_sequence_2_reset, g_wmap_land_effect_17_sequence_2_step, g_wmap_land_effect_17_sequence_2_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_17_sequence_2_step_01,
    g_wmap_land_effect_17_sequence_2_step, g_wmap_land_effect_17_sequence_2_timer,
    5, g_wmap_animation_bank_1, 0,
    0, 0x80, 8,
    0xF5, wmap_land_effect_17_sequence_2_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_17_sequence_2_step_02, g_wmap_land_effect_17_sequence_2_step, g_wmap_land_effect_17_sequence_2_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x1A, 0x1E, 0)

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
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_17_sequence_5_step_02,
    g_wmap_land_effect_17_sequence_5_step, g_wmap_land_effect_17_sequence_5_timer,
    func_8006A2FC(&g_wmap_sprite_actors[20], &g_wmap_actor_animations[20], 0x3, 0, 0x7F, 0x2, 0, (s32)((u8*)g_wmap_effect_params + 0x50)))

WMAP_STEP_ADVANCE(wmap_land_effect_17_sequence_5_end, g_wmap_land_effect_17_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_17_run_sequence_6, D_800D5D38, 0x4, g_wmap_land_effect_17_sequence_6_step, g_wmap_land_effect_17_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_17_sequence_6_reset, g_wmap_land_effect_17_sequence_6_step, g_wmap_land_effect_17_sequence_6_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_17_sequence_6_step_02,
    g_wmap_land_effect_17_sequence_6_step, g_wmap_land_effect_17_sequence_6_timer,
    func_8006A2FC(&g_wmap_sprite_actors[50], &g_wmap_actor_animations[50], 0x3, 0, 0x7F, 0x2, 0, (s32)((u8*)g_wmap_effect_params + 0x28)))

WMAP_STEP_ADVANCE(wmap_land_effect_17_sequence_6_end, g_wmap_land_effect_17_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_17_run_sequence_7, D_800D5D48, 0x6, g_wmap_land_effect_17_sequence_7_step, g_wmap_land_effect_17_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_17_sequence_7_reset, g_wmap_land_effect_17_sequence_7_step, g_wmap_land_effect_17_sequence_7_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_17_sequence_7_step_02,
    g_wmap_land_effect_17_sequence_7_step, g_wmap_land_effect_17_sequence_7_timer,
    func_8006B6EC(0x50, 0x58, 0x8, 0, 0x1C))

/**
 * @brief Start fading the sprite range and run its first update.
 */
WMAP_STEP_FADE_ACTOR_RANGE(wmap_land_effect_17_sequence_7_step_03,
    g_wmap_land_effect_17_sequence_7_step, g_wmap_land_effect_17_sequence_7_timer,
    0x50, 0x58, 8, 0x10, wmap_land_effect_17_sequence_7_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_17_sequence_7_step_04,
    g_wmap_land_effect_17_sequence_7_step, g_wmap_land_effect_17_sequence_7_timer,
    func_8006B6EC(0x50, 0x58, 0x8, 0, 0x1C))

WMAP_STEP_ADVANCE(wmap_land_effect_17_sequence_7_end, g_wmap_land_effect_17_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_17_run_sequence_8, D_800D5D60, 0x4, g_wmap_land_effect_17_sequence_8_step, g_wmap_land_effect_17_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_17_sequence_8_reset, g_wmap_land_effect_17_sequence_8_step, g_wmap_land_effect_17_sequence_8_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_17_sequence_8_step_02,
    g_wmap_land_effect_17_sequence_8_step, g_wmap_land_effect_17_sequence_8_timer,
    func_8006A2FC(&g_wmap_sprite_actors[50], &g_wmap_actor_animations[50], 0x14, 0, 0x7F, 0x2, 0, (s32)((u8*)g_wmap_effect_params + 0x28)))

WMAP_STEP_ADVANCE(wmap_land_effect_17_sequence_8_end, g_wmap_land_effect_17_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_17_run_sequence_9, D_800D5D70, 0x4, g_wmap_land_effect_17_sequence_9_step, g_wmap_land_effect_17_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_17_sequence_9_reset, g_wmap_land_effect_17_sequence_9_step, g_wmap_land_effect_17_sequence_9_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_17_sequence_9_step_02,
    g_wmap_land_effect_17_sequence_9_step, g_wmap_land_effect_17_sequence_9_timer,
    func_8006A2FC(&g_wmap_sprite_actors[20], &g_wmap_actor_animations[20], 0x1E, 0, 0x7F, 0x4, 0, (s32)((u8*)g_wmap_effect_params + 0x50)))

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
