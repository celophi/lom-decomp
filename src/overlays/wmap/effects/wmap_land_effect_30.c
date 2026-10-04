#include "../internal/wmap_main.h"
#include "../internal/wmap_land_effect_30.h"
#include "../internal/wmap_sequence_runtime.h"
#include <libgte.h>
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_view_effects.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_cells.h"

void wmap_land_effect_30_sequence_12_step_02(void);
void wmap_land_effect_30_sequence_13_step_02(void);
void wmap_land_effect_30_sequence_14_step_02(void);
void wmap_land_effect_30_wait_idle_02(void);
void wmap_land_effect_30_step_03(void);
s32 wmap_land_effect_30_run_timeline(s32 arg0);
void wmap_land_effect_30_wait_idle_04(void);
void wmap_land_effect_30_end(void);
s32 wmap_land_effect_30_run_sequence_1(s32 arg0);
s32 wmap_land_effect_30_run_sequence_3(s32 arg0);
s32 wmap_land_effect_30_run_sequence_6(s32 arg0);
s32 wmap_land_effect_30_run_sequence_7(s32 arg0);
s32 wmap_land_effect_30_run_sequence_8(s32 arg0);
s32 wmap_land_effect_30_run_sequence_9(s32 arg0);
s32 wmap_land_effect_30_run_sequence_4(s32 arg0);
s32 wmap_land_effect_30_run_sequence_12(s32 arg0);
s32 wmap_land_effect_30_run_sequence_14(s32 arg0);
s32 wmap_land_effect_30_run_sequence_13(s32 arg0);
s32 wmap_land_effect_30_run_sequence_10(s32 arg0);
s32 wmap_land_effect_30_run_sequence_2(s32 arg0);
s32 wmap_land_effect_30_run_sequence_5(s32 arg0);
s32 wmap_land_effect_30_run_sequence_11(s32 arg0);
void wmap_land_effect_30_sequence_1_step_02(void);
void wmap_land_effect_30_sequence_2_step_02(void);
void wmap_land_effect_30_sequence_5_step_02(void);
void wmap_land_effect_30_sequence_6_step_02(void);
void wmap_land_effect_30_sequence_6_step_04(void);
void wmap_land_effect_30_sequence_7_step_02(void);
void wmap_land_effect_30_sequence_7_step_04(void);
void wmap_land_effect_30_sequence_8_step_02(void);
void wmap_land_effect_30_sequence_8_step_04(void);
void wmap_land_effect_30_sequence_9_step_02(void);
void wmap_land_effect_30_sequence_9_step_04(void);
void wmap_land_effect_30_sequence_10_step_02(void);
void wmap_land_effect_30_sequence_11_step_02(void);
void wmap_land_effect_30_sequence_13_step_04(void);
void wmap_land_effect_30_sequence_13_step_06(void);

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

extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_30_sequence_3_timer;
extern u8* g_wmap_effect_model_pack_1;
extern s32 g_wmap_land_effect_30_sequence_4_timer;
extern s32 D_80139980;
extern s32 g_wmap_land_effect_30_sequence_10_timer;
extern s32 g_wmap_land_effect_30_sequence_11_timer;
extern s32 D_80139240;
extern s32 D_80139250;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern s32 D_8013B264;
extern s32 D_8013B270;
extern s32 D_8013B278;
extern s32 g_wmap_land_effect_30_sequence_12_timer;
extern WmapMotion D_801B0530[];
extern s32 rand(void);
extern s32 D_800D9150;
extern s32 D_800DCEA8;
extern s32 D_801B25D8;
extern s32 g_wmap_land_effect_30_sequence_13_timer;
extern WmapMotion D_801AFCFC[];
extern s32 g_wmap_land_effect_30_sequence_14_timer;
extern s32 g_wmap_land_effect_30_timer;
extern void (*D_800D56F0[])(void);
extern void wmap_land_effect_30_step_03(void);
extern void wmap_land_effect_30_end(void);
extern s32 g_wmap_land_effect_30_timeline_timer;
extern void (*D_800D5708[])(void);
extern u32 g_wmap_selected_artifact;
extern s32 g_wmap_land_effect_30_sequence_1_timer;
extern void (*D_800D5768[])(void);
extern u8 g_wmap_animation_bank_0[];
extern void wmap_land_effect_30_sequence_1_step_02(void);
extern s32 g_wmap_land_effect_30_sequence_2_timer;
extern void (*D_800D5778[])(void);
extern u8 g_wmap_animation_bank_2[];
extern void wmap_land_effect_30_sequence_2_step_02(void);
extern void (*D_800D5788[])(void);
extern void (*D_800D5798[])(void);
extern s32 g_wmap_land_effect_30_sequence_5_timer;
extern void (*D_800D57A8[])(void);
extern s32 g_wmap_land_effect_30_sequence_6_timer;
extern void (*D_800D57B8[])(void);
extern u8 g_wmap_animation_bank_1[];
extern void wmap_land_effect_30_sequence_6_step_02(void);
extern void wmap_land_effect_30_sequence_6_step_04(void);
extern s32 g_wmap_land_effect_30_sequence_7_timer;
extern void (*D_800D57D0[])(void);
extern void wmap_land_effect_30_sequence_7_step_02(void);
extern void wmap_land_effect_30_sequence_7_step_04(void);
extern s32 g_wmap_land_effect_30_sequence_8_timer;
extern void (*D_800D57E8[])(void);
extern void wmap_land_effect_30_sequence_8_step_04(void);
extern s32 g_wmap_land_effect_30_sequence_9_timer;
extern void (*D_800D5800[])(void);
extern void wmap_land_effect_30_sequence_9_step_04(void);
extern void (*D_800D5818[])(void);
extern void (*D_800D5830[])(void);
extern void (*D_800D5848[])(void);
extern void (*D_800D5858[])(void);
extern void (*D_800D5878[])(void);

extern u32 g_wmap_land_effect_30_sequence_3_step;
extern u32 g_wmap_land_effect_30_sequence_4_step;
extern u32 g_wmap_land_effect_30_sequence_10_step;
extern u32 g_wmap_land_effect_30_sequence_11_step;
extern u32 g_wmap_land_effect_30_sequence_12_step;
extern u8 g_wmap_animation_bank_3[];
extern u32 g_wmap_land_effect_30_sequence_13_step;
extern u8 g_wmap_animation_bank_4[];
extern u32 g_wmap_land_effect_30_sequence_14_step;
extern u32 g_wmap_land_effect_30_step;
extern u32 g_wmap_land_effect_30_timeline_step;
extern u32 g_wmap_land_effect_30_sequence_1_step;
extern u32 g_wmap_land_effect_30_sequence_2_step;
extern u32 g_wmap_land_effect_30_sequence_5_step;
extern u32 g_wmap_land_effect_30_sequence_6_step;
extern u32 g_wmap_land_effect_30_sequence_7_step;
extern u8 g_wmap_animation_bank_5[];
extern u32 g_wmap_land_effect_30_sequence_8_step;
extern u32 g_wmap_land_effect_30_sequence_9_step;

extern VECTOR g_wmap_camera_translation;



extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;
extern WmapScreenPosition D_80182D58;

extern s32* g_wmap_effect_params;

extern WmapMotion g_wmap_actor_motions[];

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_30_sequence_3_step_02,
    g_wmap_land_effect_30_sequence_3_step, g_wmap_land_effect_30_sequence_3_timer,
    g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position,
    g_wmap_effect_fade_a, g_wmap_load_buffer, -3500, 5)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_30_sequence_4_step_02,
    g_wmap_land_effect_30_sequence_4_step, g_wmap_land_effect_30_sequence_4_timer,
    g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position,
    g_wmap_effect_fade_a, g_wmap_effect_model_pack_1, -3500, 2)

/** @brief Reduce the effect parameters, draw it, and advance its countdown. */
void wmap_land_effect_30_sequence_10_step_04(void)
{
    s32 value;
    s32 remaining;

    value = D_80139980 - 1;
    D_80139980 = value;
    if (value < 0)
    {
        D_80139980 = 0;
    }
    if (!(g_wmap_frame_count & 3))
    {
        g_wmap_particle_intensity -= 1;
    }
    if (g_wmap_particle_intensity < 0)
    {
        g_wmap_particle_intensity = 0;
    }
    func_8006AFAC(0x13, 0x45, 0x13, 0x20, 0xFD0, 4, 0x20, 1);
    remaining = g_wmap_land_effect_30_sequence_10_timer - 1;
    g_wmap_land_effect_30_sequence_10_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_30_sequence_10_step += 1;
    }
}

/** @brief Reduce the effect parameters, draw it, and advance its countdown. */
void wmap_land_effect_30_sequence_11_step_04(void)
{
    s32 value;
    s32 remaining;

    value = D_80139980 - 1;
    D_80139980 = value;
    if (value < 0)
    {
        D_80139980 = 0;
    }
    if (!(g_wmap_frame_count & 3))
    {
        g_wmap_particle_intensity -= 1;
    }
    if (g_wmap_particle_intensity < 0)
    {
        g_wmap_particle_intensity = 0;
    }
    func_8006AFAC(0x54, 0x5C, 0x13, 0x20, 0x1000, 0x20, 8, 2);
    remaining = g_wmap_land_effect_30_sequence_11_timer - 1;
    g_wmap_land_effect_30_sequence_11_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_30_sequence_11_step += 1;
    }
}

/** @brief Initialize spaced actors with randomized animation choices. */
void wmap_land_effect_30_sequence_12_step_01(void)
{
    s32 i;
    WmapSpriteActor *actor;

    D_80139264 = 120;
    D_80139268 = 19;
    D_8013926C = 0;
    WMAP_RESET_PARTICLE_SLOTS(i, 60,
                              g_wmap_actor_motions[i + 120].state,
                              120, g_wmap_animation_bank_3);
    D_80139240 = 0;
    for (i = 0; i < 60; i += 5)
    {
        actor = &g_wmap_sprite_actors[120 + i];
        actor->resource_index = 0;
        actor->scale_index = 15;
        actor->sequence = rand() % 3;
        actor->previous_sequence = -1;
        actor->shade_step = 2;
        actor->target_shade = 129;
        actor->shade = 8;
        D_801B0530[i].state = 1;
        D_801B0530[i].z = 0;
        D_801B0530[i].scale = 60;
        D_801B0530[i].angle = D_80139240;
        D_80139240 += 341;
        D_801B0530[i].field_0E = 0;
    }
    D_8013B264 = 60;
    D_8013B270 = 4;
    D_8013B278 = 5000;
    D_80139284 = 0;
    D_80139250 = -1;
    g_wmap_land_effect_30_sequence_12_timer = 60;
    g_wmap_land_effect_30_sequence_12_step++;
    wmap_land_effect_30_sequence_12_step_02();
}

/** @brief Advance spaced effect actors and copy their trailing samples. */
void wmap_land_effect_30_sequence_12_step_02(void)
{
    s32 next;
    s32 i;
    s32 destination;
    s32 remaining;

    func_8006D014(&g_wmap_sprite_actors[120], &g_wmap_actor_animations[120], 60, 128, 128, 8, 3);
    for (i = 120; i < 180; i += 5)
    {
        if (g_wmap_actor_motions[i].scale != 0)
        {
            g_wmap_actor_motions[i].scale--;
        }
        g_wmap_actor_motions[i].z += 5000;
    }
    if ((g_wmap_frame_count & 1) == 0)
    {
        for (i = 120; i < 180; i += 5)
        {
            next = i + 1;
            destination = D_80139284 + next;
            g_wmap_actor_motions[destination] = g_wmap_actor_motions[i];
            g_wmap_sprite_actors[destination] = g_wmap_sprite_actors[i];
            g_wmap_actor_animations[destination] = g_wmap_actor_animations[i];
            g_wmap_sprite_actors[destination].target_shade = 0;
        }
        D_80139284 = (D_80139284 + 1) % 4;
    }
    remaining = g_wmap_land_effect_30_sequence_12_timer - 1;
    g_wmap_land_effect_30_sequence_12_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_30_sequence_12_step++;
    }
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void wmap_land_effect_30_sequence_13_step_01(void)
{
    s32 i;

    i = 200;
    D_801B25D8 = 1;
    D_800DCEA8 = 1;

    do
    {
        g_wmap_actor_motions[i].state = 0;
        WMAP_INIT_PARTICLE_ACTOR(i, g_wmap_animation_bank_3, 3);
        i++;
    } while (i < 240);

    D_800D9150 = 2;
    g_wmap_land_effect_30_sequence_13_timer = 0x10;
    g_wmap_land_effect_30_sequence_13_step += 1;
    wmap_land_effect_30_sequence_13_step_02();
}

/** @brief Initialize effect actors at regular angular intervals. */
void wmap_land_effect_30_sequence_14_step_01(void)
{
    s32 i;
    s16 actor_field_value;

    g_wmap_effect_params[0x25] = 15;
    g_wmap_effect_params[0x26] = 19;
    g_wmap_effect_params[0x27] = 0;
    WMAP_RESET_PARTICLE_SLOTS(i, 60,
                              g_wmap_actor_motions[i + 15].state,
                              15, g_wmap_animation_bank_4);
    g_wmap_effect_params[0x21] = 256;
    for (i = 0; i < 60; i += 5)
    {
        actor_field_value = 2;
        WMAP_ACTOR_BLOCK(15)[i].shade_step = actor_field_value;
        actor_field_value = 129;
        WMAP_ACTOR_BLOCK(15)[i].target_shade = actor_field_value;
        actor_field_value = 8;
        WMAP_ACTOR_BLOCK(15)[i].shade = actor_field_value;
        WMAP_ACTOR_BLOCK(15)[i].resource_index = 0;
        WMAP_ACTOR_BLOCK(15)[i].scale_index = 15;
        WMAP_ACTOR_BLOCK(15)[i].sequence = 0;
        WMAP_ACTOR_BLOCK(15)[i].previous_sequence = -1;
        D_801AFCFC[i].state = 1;
        D_801AFCFC[i].z = 0;
        D_801AFCFC[i].scale = 60;
        D_801AFCFC[i].angle = g_wmap_effect_params[0x21];
        g_wmap_effect_params[0x21] += 341;
        D_801AFCFC[i].field_0E = 0;
    }
    D_8013B264 = 60;
    D_8013B270 = 4;
    D_8013B278 = 5000;
    g_wmap_effect_params[0x23] = -1;
    g_wmap_effect_params[0x28] = 0;
    g_wmap_land_effect_30_sequence_14_timer = 60;
    g_wmap_land_effect_30_sequence_14_step++;
    wmap_land_effect_30_sequence_14_step_02();
}

/** @brief Advance spaced effect actors and copy their trailing samples. */
void wmap_land_effect_30_sequence_14_step_02(void)
{
    s32 next;
    s32 i;
    s32 destination;
    s32 remaining;

    func_8006A2FC(&g_wmap_sprite_actors[15], &g_wmap_actor_animations[15], 60, 128, 128, 8, 3, &g_wmap_effect_params[30]);
    for (i = 15; i < 75; i += 5)
    {
        if (g_wmap_actor_motions[i].scale != 0)
        {
            g_wmap_actor_motions[i].scale--;
        }
        g_wmap_actor_motions[i].z += 5000;
        g_wmap_actor_motions[i].angle += 128;
    }
    if ((g_wmap_frame_count & 1) == 0)
    {
        for (i = 15; i < 75; i += 5)
        {
            next = i + 1;
            destination = g_wmap_effect_params[40] + next;
            g_wmap_actor_motions[destination] = g_wmap_actor_motions[i];
            g_wmap_sprite_actors[destination] = g_wmap_sprite_actors[i];
            g_wmap_actor_animations[destination] = g_wmap_actor_animations[i];
            g_wmap_sprite_actors[destination].target_shade = 0;
        }
        g_wmap_effect_params[40] = (g_wmap_effect_params[40] + 1) % 4;
    }
    remaining = g_wmap_land_effect_30_sequence_14_timer - 1;
    g_wmap_land_effect_30_sequence_14_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_30_sequence_14_step++;
    }
}

WMAP_STEP_RUNNER(wmap_land_effect_30_run, D_800D56F0, 0x6, g_wmap_land_effect_30_step, g_wmap_land_effect_30_timer)

WMAP_STEP_RESET(wmap_land_effect_30_reset, g_wmap_land_effect_30_step, g_wmap_land_effect_30_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_30_step_01, g_wmap_land_effect_30_step, wmap_run_land_focus, wmap_land_effect_30_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_30_wait_idle_02, g_wmap_land_effect_30_step, wmap_land_effect_30_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_30_step_03, g_wmap_land_effect_30_step, wmap_land_effect_30_run_timeline, wmap_land_effect_30_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_30_wait_idle_04, g_wmap_land_effect_30_step, wmap_land_effect_30_end)

WMAP_STEP_ADVANCE(wmap_land_effect_30_end, g_wmap_land_effect_30_step)

WMAP_STEP_RUNNER(wmap_land_effect_30_run_timeline, D_800D5708, 0x18, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_30_timeline_reset, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer)

/** @brief Set world-map color, play sound 30, register two callbacks, and begin an eight-tick delay. */
void wmap_land_effect_30_timeline_step_01(void)
{
    wmap_start_map_tint(0x122840);
    g_wmap_backdrop_target_level = 4;
    wmap_play_sound(0x1E, 0x80);
    wmap_start_sequence(&wmap_land_effect_30_run_sequence_1);
    wmap_start_sequence(&wmap_land_effect_30_run_sequence_3);
    g_wmap_land_effect_30_timeline_timer = 8;
    g_wmap_land_effect_30_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_30_timeline_wait_02, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_30_timeline_step_03, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer,
                         wmap_land_effect_30_run_sequence_6, 0x4)

WMAP_STEP_WAIT(wmap_land_effect_30_timeline_wait_04, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer)

/** @brief World-map step handler: register the next callback and advance the counter. */
void wmap_land_effect_30_timeline_step_05(void)
{
    wmap_start_sequence(wmap_land_effect_30_run_sequence_7);
    g_wmap_placement_overlay_hidden = 1;
    g_wmap_land_effect_30_timeline_timer = 4;
    g_wmap_land_effect_30_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_30_timeline_wait_06, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_30_timeline_step_07, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer,
                         wmap_land_effect_30_run_sequence_8, 0x4)

WMAP_STEP_WAIT(wmap_land_effect_30_timeline_wait_08, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_30_timeline_step_09, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer,
                         wmap_land_effect_30_run_sequence_9, 0x24)

WMAP_STEP_WAIT(wmap_land_effect_30_timeline_wait_10, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer)

/**
 * @brief Start three sequences and set the wait timer.
 */
WMAP_STEP_START_THREE_AND_WAIT(wmap_land_effect_30_timeline_step_11,
    g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer,
    wmap_land_effect_30_run_sequence_4, wmap_land_effect_30_run_sequence_12, wmap_land_effect_30_run_sequence_14, 0x38)

WMAP_STEP_WAIT(wmap_land_effect_30_timeline_wait_12, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_30_timeline_step_13, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer,
                         wmap_land_effect_30_run_sequence_13, 0xA)

WMAP_STEP_WAIT(wmap_land_effect_30_timeline_wait_14, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_30_timeline_step_15, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer,
                         wmap_land_effect_30_run_sequence_10, 0x10)

WMAP_STEP_WAIT(wmap_land_effect_30_timeline_wait_16, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_30_timeline_step_17, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer,
                         wmap_land_effect_30_run_sequence_2, 0x30)

WMAP_STEP_WAIT(wmap_land_effect_30_timeline_wait_18, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_30_timeline_step_19, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer,
                         wmap_land_effect_30_run_sequence_5, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_30_timeline_wait_20, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_30_timeline_step_21, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer,
                         wmap_land_effect_30_run_sequence_11, 0x28)

WMAP_STEP_WAIT(wmap_land_effect_30_timeline_wait_22, g_wmap_land_effect_30_timeline_step, g_wmap_land_effect_30_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_30_timeline_finish, g_wmap_land_effect_30_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y, g_wmap_selected_artifact)

WMAP_STEP_RUNNER(wmap_land_effect_30_run_sequence_1, D_800D5768, 0x4, g_wmap_land_effect_30_sequence_1_step, g_wmap_land_effect_30_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_30_sequence_1_reset, g_wmap_land_effect_30_sequence_1_step, g_wmap_land_effect_30_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_30_sequence_1_step_01,
    g_wmap_land_effect_30_sequence_1_step, g_wmap_land_effect_30_sequence_1_timer,
    4, g_wmap_animation_bank_0, 0,
    0, 0x80, 8,
    0x3C, wmap_land_effect_30_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_30_sequence_1_step_02, g_wmap_land_effect_30_sequence_1_step, g_wmap_land_effect_30_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0x13, 0x14, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_30_sequence_1_end, g_wmap_land_effect_30_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_30_run_sequence_2, D_800D5778, 0x4, g_wmap_land_effect_30_sequence_2_step, g_wmap_land_effect_30_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_30_sequence_2_reset, g_wmap_land_effect_30_sequence_2_step, g_wmap_land_effect_30_sequence_2_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_30_sequence_2_step_01,
    g_wmap_land_effect_30_sequence_2_step, g_wmap_land_effect_30_sequence_2_timer,
    5, g_wmap_animation_bank_2, 0,
    0x80, 0x80, 8,
    0x5D, wmap_land_effect_30_sequence_2_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_30_sequence_2_step_02, g_wmap_land_effect_30_sequence_2_step, g_wmap_land_effect_30_sequence_2_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x18, 0x33, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_30_sequence_2_end, g_wmap_land_effect_30_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_30_run_sequence_3, D_800D5788, 0x4, g_wmap_land_effect_30_sequence_3_step, g_wmap_land_effect_30_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_30_sequence_3_reset, g_wmap_land_effect_30_sequence_3_step, g_wmap_land_effect_30_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_30_sequence_3_step_01, g_wmap_land_effect_30_sequence_3_step, g_wmap_land_effect_30_sequence_3_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x14, wmap_land_effect_30_sequence_3_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_30_sequence_3_end, g_wmap_land_effect_30_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_30_run_sequence_4, D_800D5798, 0x4, g_wmap_land_effect_30_sequence_4_step, g_wmap_land_effect_30_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_30_sequence_4_reset, g_wmap_land_effect_30_sequence_4_step, g_wmap_land_effect_30_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_30_sequence_4_step_01, g_wmap_land_effect_30_sequence_4_step, g_wmap_land_effect_30_sequence_4_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40, wmap_land_effect_30_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_30_sequence_4_end, g_wmap_land_effect_30_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_30_run_sequence_5, D_800D57A8, 0x4, g_wmap_land_effect_30_sequence_5_step, g_wmap_land_effect_30_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_30_sequence_5_reset, g_wmap_land_effect_30_sequence_5_step, g_wmap_land_effect_30_sequence_5_timer)

/** @brief Configure the actor, save its screen position, and begin a 44-tick delay. */
void wmap_land_effect_30_sequence_5_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    g_wmap_actor_animations[6].data = g_wmap_animation_bank_2;
    actor->scale_index = 0xF;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    actor->shade_step = 8;
    actor->target_shade = 0x80;
    actor->shade = 0x80;
    actor->resource_index = 0;
    g_wmap_land_effect_30_sequence_5_timer = 0x2C;
    D_80182D58.packed = g_wmap_focus_screen_position.packed;
    g_wmap_land_effect_30_sequence_5_step += 1;
    wmap_land_effect_30_sequence_5_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_30_sequence_5_step_02, g_wmap_land_effect_30_sequence_5_step, g_wmap_land_effect_30_sequence_5_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x18, 0x22, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_30_sequence_5_end, g_wmap_land_effect_30_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_30_run_sequence_6, D_800D57B8, 0x6, g_wmap_land_effect_30_sequence_6_step, g_wmap_land_effect_30_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_30_sequence_6_reset, g_wmap_land_effect_30_sequence_6_step, g_wmap_land_effect_30_sequence_6_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_30_sequence_6_step_01,
    g_wmap_land_effect_30_sequence_6_step, g_wmap_land_effect_30_sequence_6_timer,
    7, g_wmap_animation_bank_1, 0,
    1, 0x81, 2,
    0x30, wmap_land_effect_30_sequence_6_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_30_sequence_6_step_02, g_wmap_land_effect_30_sequence_6_step, g_wmap_land_effect_30_sequence_6_timer, g_wmap_sprite_actors[7],
                              g_wmap_actor_animations[7], g_wmap_focus_screen_position, 0x8, 0xA, 0)

/**
 * @brief Start the sprite fade and run its first update.
 */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_30_sequence_6_step_03,
    g_wmap_land_effect_30_sequence_6_step, g_wmap_land_effect_30_sequence_6_timer,
    g_wmap_sprite_actors[7], 2, 0x8, wmap_land_effect_30_sequence_6_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_30_sequence_6_step_04, g_wmap_land_effect_30_sequence_6_step, g_wmap_land_effect_30_sequence_6_timer, g_wmap_sprite_actors[7],
                              g_wmap_actor_animations[7], g_wmap_focus_screen_position, 0x8, 0xA, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_30_sequence_6_end, g_wmap_land_effect_30_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_30_run_sequence_7, D_800D57D0, 0x6, g_wmap_land_effect_30_sequence_7_step, g_wmap_land_effect_30_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_30_sequence_7_reset, g_wmap_land_effect_30_sequence_7_step, g_wmap_land_effect_30_sequence_7_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_30_sequence_7_step_01,
    g_wmap_land_effect_30_sequence_7_step, g_wmap_land_effect_30_sequence_7_timer,
    8, g_wmap_animation_bank_5, 0,
    1, 0x81, 8,
    0x20, wmap_land_effect_30_sequence_7_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_30_sequence_7_step_02, g_wmap_land_effect_30_sequence_7_step, g_wmap_land_effect_30_sequence_7_timer, g_wmap_sprite_actors[8],
                              g_wmap_actor_animations[8], g_wmap_focus_screen_position, 0x8, 0xA, 0)

/**
 * @brief Start the sprite fade and run its first update.
 */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_30_sequence_7_step_03,
    g_wmap_land_effect_30_sequence_7_step, g_wmap_land_effect_30_sequence_7_timer,
    g_wmap_sprite_actors[8], 8, 0x8, wmap_land_effect_30_sequence_7_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_30_sequence_7_step_04, g_wmap_land_effect_30_sequence_7_step, g_wmap_land_effect_30_sequence_7_timer, g_wmap_sprite_actors[8],
                              g_wmap_actor_animations[8], g_wmap_focus_screen_position, 0x8, 0xA, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_30_sequence_7_end, g_wmap_land_effect_30_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_30_run_sequence_8, D_800D57E8, 0x6, g_wmap_land_effect_30_sequence_8_step, g_wmap_land_effect_30_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_30_sequence_8_reset, g_wmap_land_effect_30_sequence_8_step, g_wmap_land_effect_30_sequence_8_timer)

/** @brief Start the sprite animation and run its first update. */
WMAP_STEP_START_ACTOR(wmap_land_effect_30_sequence_8_step_01,
    g_wmap_land_effect_30_sequence_8_step, g_wmap_land_effect_30_sequence_8_timer,
    9, g_wmap_animation_bank_5, 1,
    1, 0x81, 8,
    0x1C, wmap_land_effect_30_sequence_8_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_30_sequence_8_step_02, g_wmap_land_effect_30_sequence_8_step, g_wmap_land_effect_30_sequence_8_timer, g_wmap_sprite_actors[9],
                              g_wmap_actor_animations[9], g_wmap_focus_screen_position, 0x8, 0xA, 0)

/**
 * @brief Start the sprite fade and run its first update.
 */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_30_sequence_8_step_03,
    g_wmap_land_effect_30_sequence_8_step, g_wmap_land_effect_30_sequence_8_timer,
    g_wmap_sprite_actors[9], 8, 0x8, wmap_land_effect_30_sequence_8_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_30_sequence_8_step_04, g_wmap_land_effect_30_sequence_8_step, g_wmap_land_effect_30_sequence_8_timer, g_wmap_sprite_actors[9],
                              g_wmap_actor_animations[9], g_wmap_focus_screen_position, 0x8, 0xA, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_30_sequence_8_end, g_wmap_land_effect_30_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_30_run_sequence_9, D_800D5800, 0x6, g_wmap_land_effect_30_sequence_9_step, g_wmap_land_effect_30_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_30_sequence_9_reset, g_wmap_land_effect_30_sequence_9_step, g_wmap_land_effect_30_sequence_9_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_30_sequence_9_step_01,
    g_wmap_land_effect_30_sequence_9_step, g_wmap_land_effect_30_sequence_9_timer,
    10, g_wmap_animation_bank_5, 2,
    1, 0x81, 8,
    0x18, wmap_land_effect_30_sequence_9_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_30_sequence_9_step_02, g_wmap_land_effect_30_sequence_9_step, g_wmap_land_effect_30_sequence_9_timer, g_wmap_sprite_actors[10],
                              g_wmap_actor_animations[10], g_wmap_focus_screen_position, 0x8, 0xA, 0)

/**
 * @brief Start the sprite fade and run its first update.
 */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_30_sequence_9_step_03,
    g_wmap_land_effect_30_sequence_9_step, g_wmap_land_effect_30_sequence_9_timer,
    g_wmap_sprite_actors[10], 8, 0x8, wmap_land_effect_30_sequence_9_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_30_sequence_9_step_04, g_wmap_land_effect_30_sequence_9_step, g_wmap_land_effect_30_sequence_9_timer, g_wmap_sprite_actors[10],
                              g_wmap_actor_animations[10], g_wmap_focus_screen_position, 0x8, 0xA, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_30_sequence_9_end, g_wmap_land_effect_30_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_30_run_sequence_10, D_800D5818, 0x6, g_wmap_land_effect_30_sequence_10_step, g_wmap_land_effect_30_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_30_sequence_10_reset, g_wmap_land_effect_30_sequence_10_step, g_wmap_land_effect_30_sequence_10_timer)

/** @brief World-map step: clear a run of per-entry slots, then schedule the next step. */
void wmap_land_effect_30_sequence_10_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 0;
    D_80139980 = 0x7F;
    WMAP_RESET_PARTICLE_SLOTS(i, 0x32,
                              g_wmap_actor_motions[i + 0x13].state,
                              0x13, g_wmap_animation_bank_4);
    g_wmap_land_effect_30_sequence_10_timer = 0x32;
    g_wmap_land_effect_30_sequence_10_step += 1;
    wmap_land_effect_30_sequence_10_step_02();
}

/** @brief Advance the effect on odd frames, draw it, and update the countdown. */
void wmap_land_effect_30_sequence_10_step_02(void)
{
    s32 remaining_ticks;

    if (g_wmap_frame_count & 1)
    {
        g_wmap_particle_intensity += 1;
    }
    func_8006AFAC(0x13, 0x45, 0x13, 0x20, 0xFD0, 4, 0x20, 1);
    remaining_ticks = g_wmap_land_effect_30_sequence_10_timer - 1;
    g_wmap_land_effect_30_sequence_10_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_30_sequence_10_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_30_sequence_10_step_03, g_wmap_land_effect_30_sequence_10_step, g_wmap_land_effect_30_sequence_10_timer, 0x20,
                    wmap_land_effect_30_sequence_10_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_30_sequence_10_end, g_wmap_land_effect_30_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_30_run_sequence_11, D_800D5830, 0x6, g_wmap_land_effect_30_sequence_11_step, g_wmap_land_effect_30_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_30_sequence_11_reset, g_wmap_land_effect_30_sequence_11_step, g_wmap_land_effect_30_sequence_11_timer)

/** @brief World-map step: clear a run of per-entry slots, then schedule the next step. */
void wmap_land_effect_30_sequence_11_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 0;
    D_80139980 = 0x7F;
    WMAP_RESET_PARTICLE_SLOTS(i, 0x8,
                              g_wmap_actor_motions[i + 0x54].state,
                              0x54, g_wmap_animation_bank_4);
    g_wmap_land_effect_30_sequence_11_timer = 2;
    g_wmap_land_effect_30_sequence_11_step += 1;
    wmap_land_effect_30_sequence_11_step_02();
}

/** @brief Advance the effect on odd frames, draw it, and update the countdown. */
void wmap_land_effect_30_sequence_11_step_02(void)
{
    s32 remaining_ticks;

    if (g_wmap_frame_count & 1)
    {
        g_wmap_particle_intensity += 1;
    }
    func_8006AFAC(0x54, 0x5C, 0x13, 0x20, 0x1000, 0x20, 8, 2);
    remaining_ticks = g_wmap_land_effect_30_sequence_11_timer - 1;
    g_wmap_land_effect_30_sequence_11_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_30_sequence_11_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_30_sequence_11_step_03, g_wmap_land_effect_30_sequence_11_step, g_wmap_land_effect_30_sequence_11_timer, 0x18,
                    wmap_land_effect_30_sequence_11_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_30_sequence_11_end, g_wmap_land_effect_30_sequence_11_step)

WMAP_STEP_RUNNER(wmap_land_effect_30_run_sequence_12, D_800D5848, 0x4, g_wmap_land_effect_30_sequence_12_step, g_wmap_land_effect_30_sequence_12_timer)

WMAP_STEP_RESET(wmap_land_effect_30_sequence_12_reset, g_wmap_land_effect_30_sequence_12_step, g_wmap_land_effect_30_sequence_12_timer)

WMAP_STEP_ADVANCE(wmap_land_effect_30_sequence_12_end, g_wmap_land_effect_30_sequence_12_step)

WMAP_STEP_RUNNER(wmap_land_effect_30_run_sequence_13, D_800D5858, 0x8, g_wmap_land_effect_30_sequence_13_step, g_wmap_land_effect_30_sequence_13_timer)

WMAP_STEP_RESET(wmap_land_effect_30_sequence_13_reset, g_wmap_land_effect_30_sequence_13_step, g_wmap_land_effect_30_sequence_13_timer)

/**
 * @brief Update the particles and increase the shared fade value until the timer expires.
 */
WMAP_STEP_UPDATE_AND_RAMP(wmap_land_effect_30_sequence_13_step_02, g_wmap_land_effect_30_sequence_13_step, g_wmap_land_effect_30_sequence_13_timer,
                          D_801B25D8, 8,
                          func_8006B328(0xC8, 0xF0, 2, -1, 0x12, 4, 0x168, 0x13, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 0))

WMAP_STEP_ARM_TIMER(wmap_land_effect_30_sequence_13_step_03, g_wmap_land_effect_30_sequence_13_step, g_wmap_land_effect_30_sequence_13_timer, 0x18,
                    wmap_land_effect_30_sequence_13_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_30_sequence_13_step_04,
    g_wmap_land_effect_30_sequence_13_step, g_wmap_land_effect_30_sequence_13_timer,
    func_8006B328(0xC8, 0xF0, 2, -1, 0x12, 4, 0x168, 0x13, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 0))

/**
 * @brief Stop spawning particles and keep updating those already active.
 */
WMAP_STEP_STOP_PARTICLE_SPAWNS(wmap_land_effect_30_sequence_13_step_05, g_wmap_land_effect_30_sequence_13_step, g_wmap_land_effect_30_sequence_13_timer,
                              D_800DCEA8, 0x18, wmap_land_effect_30_sequence_13_step_06)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_30_sequence_13_step_06,
    g_wmap_land_effect_30_sequence_13_step, g_wmap_land_effect_30_sequence_13_timer,
    func_8006B328(0xC8, 0xF0, 2, -1, 0x12, 4, 0x168, 0x13, -0x32, 0x64, -0x32, 0x64, 0xB4, 0x81, 0x81, 8, 0))

WMAP_STEP_ADVANCE(wmap_land_effect_30_sequence_13_end, g_wmap_land_effect_30_sequence_13_step)

WMAP_STEP_RUNNER(wmap_land_effect_30_run_sequence_14, D_800D5878, 0x4, g_wmap_land_effect_30_sequence_14_step, g_wmap_land_effect_30_sequence_14_timer)

WMAP_STEP_RESET(wmap_land_effect_30_sequence_14_reset, g_wmap_land_effect_30_sequence_14_step, g_wmap_land_effect_30_sequence_14_timer)

WMAP_STEP_ADVANCE(wmap_land_effect_30_sequence_14_end, g_wmap_land_effect_30_sequence_14_step)
