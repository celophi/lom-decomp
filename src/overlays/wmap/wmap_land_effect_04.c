#include "wmap_main.h"
#include "wmap_land_effect_04.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_sprite_render.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_effect_primitives.h"
#include "wmap_step_sequence.h"
#include "wmap_cells.h"

void wmap_land_effect_04_sequence_5_step_02(void);
void wmap_land_effect_04_wait_idle_02(void);
void wmap_land_effect_04_step_03(void);
s32 wmap_land_effect_04_run_timeline(s32 arg0);
void wmap_land_effect_04_wait_idle_04(void);
void wmap_land_effect_04_end(void);
s32 wmap_land_effect_04_run_sequence_3(s32 arg0);
s32 wmap_land_effect_04_run_sequence_5(s32 arg0);
s32 wmap_land_effect_04_run_sequence_2(s32 arg0);
s32 wmap_land_effect_04_run_sequence_7(s32 arg0);
s32 wmap_land_effect_04_run_sequence_6(s32 arg0);
s32 wmap_land_effect_04_run_sequence_1(s32 arg0);
s32 wmap_land_effect_04_run_sequence_9(s32 arg0);
s32 wmap_land_effect_04_run_sequence_4(s32 arg0);
s32 wmap_land_effect_04_run_sequence_8(s32 arg0);
void wmap_land_effect_04_sequence_1_step_02(void);
void wmap_land_effect_04_sequence_2_step_02(void);
void wmap_land_effect_04_sequence_3_step_02(void);
void wmap_land_effect_04_sequence_4_step_02(void);

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

extern u8 g_wmap_animation_bank_2[];
extern s32 g_wmap_land_effect_04_sequence_5_timer;
extern u8 g_wmap_load_buffer[];
extern s32 D_801B2468;
extern s32 g_wmap_land_effect_04_sequence_6_timer;
extern u8* g_wmap_effect_model_pack_1;
extern s32 g_wmap_land_effect_04_sequence_7_timer;
extern u8* g_wmap_effect_model_pack_2;
extern s32 g_wmap_land_effect_04_sequence_8_timer;
extern s32 g_wmap_land_effect_04_sequence_9_timer;
extern s32 rand(void);
extern s32 ccos(s32);
extern void wmap_land_effect_04_sequence_9_step_02(void);
extern s32 g_wmap_land_effect_04_timer;
extern void (*D_800D4DD0[])(void);
extern void wmap_land_effect_04_step_03(void);
extern void wmap_land_effect_04_end(void);
extern s32 g_wmap_land_effect_04_timeline_timer;
extern void (*D_800D4DE8[])(void);
extern s32 g_wmap_selected_artifact;
extern s32 g_wmap_land_effect_04_sequence_1_timer;
extern void (*D_800D4E38[])(void);
extern u8 g_wmap_animation_bank_1[];
extern void wmap_land_effect_04_sequence_1_step_02(void);
extern s32 g_wmap_land_effect_04_sequence_2_timer;
extern void (*D_800D4E48[])(void);
extern void wmap_land_effect_04_sequence_2_step_02(void);
extern s32 g_wmap_land_effect_04_sequence_3_timer;
extern void (*D_800D4E58[])(void);
extern s32 g_wmap_land_effect_04_sequence_4_timer;
extern void (*D_800D4E68[])(void);
extern u8 g_wmap_animation_bank_0[];
extern void wmap_land_effect_04_sequence_4_step_02(void);
extern void (*D_800D4E78[])(void);
extern void (*D_800D4E88[])(void);
extern s32 D_801B24B0;
extern void (*D_800D4EA0[])(void);
extern void (*D_800D4EB0[])(void);
extern void (*D_800D4EC0[])(void);
extern u32 g_wmap_land_effect_04_sequence_5_step;

extern u32 g_wmap_land_effect_04_sequence_6_step;
extern u32 g_wmap_land_effect_04_sequence_7_step;
extern u32 g_wmap_land_effect_04_sequence_8_step;
extern u32 g_wmap_land_effect_04_sequence_9_step;
extern u32 g_wmap_land_effect_04_step;
extern u32 g_wmap_land_effect_04_timeline_step;
extern u32 g_wmap_land_effect_04_sequence_1_step;
extern u32 g_wmap_land_effect_04_sequence_2_step;
extern u32 g_wmap_land_effect_04_sequence_3_step;
extern u32 g_wmap_land_effect_04_sequence_4_step;

extern VECTOR g_wmap_camera_translation;

extern SVECTOR D_801B2498;
extern SVECTOR D_801B2490;


extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* g_wmap_effect_params;

extern WmapMotion g_wmap_actor_motions[];

/** @brief Initialize four effect actors and their angular spacing. */
void wmap_land_effect_04_sequence_5_step_01(void)
{
    s32 i;
    s32 *descriptor;
    WmapSpriteActor *actor;

    D_801B2490 = g_wmap_zero_rotation;
    descriptor = g_wmap_effect_params;
    descriptor[0] = 2;
    descriptor[1] = 1;
    descriptor[2] = 2;
    g_wmap_effect_params[3] = 0;
    g_wmap_effect_params[4] = 3500;
    g_wmap_effect_params[5] = 96;
    g_wmap_effect_params[6] = 4;
    g_wmap_effect_params[7] = 0;
    g_wmap_effect_params[8] = 10;
    g_wmap_effect_params[9] = 8;
    for (i = 20; i < 80; i++)
    {
        g_wmap_actor_motions[i].state = 0;
    }
    for (i = 20; i < 80; i += 15)
    {
        actor = &g_wmap_sprite_actors[i];
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_2;
        actor->scale_index = 15;
        actor->previous_sequence = -1;
        actor->resource_index = 0;
        actor->sequence = 0;
        actor->shade_step = 4;
        actor->target_shade = 1;
        actor->shade = 129;
        g_wmap_actor_motions[i].state = 1;
        g_wmap_actor_motions[i].z = 150000;
        g_wmap_actor_motions[i].angle = g_wmap_effect_params[3];
        g_wmap_actor_motions[i].field_0E = 0;
        g_wmap_effect_params[3] += 1024;
    }
    g_wmap_land_effect_04_sequence_5_timer = 40;
    g_wmap_land_effect_04_sequence_5_step++;
    wmap_land_effect_04_sequence_5_step_02();
}

/** @brief Draw and brighten two rotating layers, then advance their shared countdown. */
void wmap_land_effect_04_sequence_6_step_02(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(g_wmap_load_buffer, 0, 16, 53, 0x7800, 1, D_801B2468);
    D_801B2490.vz = (u16)(D_801B2490.vz + 12);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(g_wmap_load_buffer, 0, 16, 53, 0x7800, 1, D_801B2468);
    D_801B2498.vz = (u16)(D_801B2498.vz - 4);
    PopMatrix();
    WMAP_MODEL_FADE_IN(D_801B2468, 8, 129, intensity);
    remaining = g_wmap_land_effect_04_sequence_6_timer - 1;
    g_wmap_land_effect_04_sequence_6_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_04_sequence_6_step++;
    }
}

/** @brief Draw and fade two rotating layers, then advance their shared countdown. */
void wmap_land_effect_04_sequence_6_step_04(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(g_wmap_load_buffer, 0, 16, 53, 0x7800, 1, D_801B2468);
    D_801B2490.vz = (u16)(D_801B2490.vz + 12);
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(g_wmap_load_buffer, 0, 16, 53, 0x7800, 1, D_801B2468);
    D_801B2498.vz = (u16)(D_801B2498.vz - 4);
    PopMatrix();
    WMAP_MODEL_FADE_OUT(D_801B2468, 4, intensity);
    remaining = g_wmap_land_effect_04_sequence_6_timer - 1;
    g_wmap_land_effect_04_sequence_6_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_04_sequence_6_step++;
    }
}

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_04_sequence_7_step_02,
    g_wmap_land_effect_04_sequence_7_step, g_wmap_land_effect_04_sequence_7_timer,
    g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position,
    g_wmap_effect_fade_a, g_wmap_effect_model_pack_1, -3500, 1)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_04_sequence_8_step_02,
    g_wmap_land_effect_04_sequence_8_step, g_wmap_land_effect_04_sequence_8_timer,
    g_wmap_effect_model_b_rotation, g_wmap_effect_model_b_position,
    g_wmap_effect_fade_b, g_wmap_effect_model_pack_2, -3500, 2)

/** @brief Initialize randomized actors along a cosine depth curve. */
void wmap_land_effect_04_sequence_9_step_01(void)
{
    s32 i;
    WmapSpriteActor *actor;
    WmapMotion *motion;

    i = 100;
    do
    {
        actor = &g_wmap_sprite_actors[i];
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_2;
        actor->scale_index = 15;
        actor->sequence = 2;
        actor->previous_sequence = -1;
        actor->resource_index = 0;
        actor->shade_step = 1;
        actor->target_shade = 129;
        actor->shade = 1;
        motion = (WmapMotion *)((u8 *)g_wmap_actor_motions + i * sizeof(WmapMotion));
        motion->angle = rand() & 4095;
        motion->field_0E = ((i - 100) * 140) / 30;
        motion->z = ccos(2048 - (((i - 100) * 1024) / 30)) * 140;
        motion->x = ((rand() * 100) >> 15) + 40;
        i++;
    } while (i < 130);
    g_wmap_land_effect_04_sequence_9_timer = 64;
    g_wmap_land_effect_04_sequence_9_step++;
    wmap_land_effect_04_sequence_9_step_02();
}

/**
 * @brief Project and draw the world-map star field, spinning each entry each frame.
 */
void wmap_land_effect_04_sequence_9_step_02(void)
{
    SVECTOR position;
    s32 screen;
    WmapStar* star;
    WmapSpriteActor* draw;
    s32 i;

    for (i = 0x64; i < 0x82; i++)
    {
        draw = &g_wmap_sprite_actors[i];
        star = &g_wmap_actor_motions[i];
        position.vx = ((star->radius >> 6) * (ccos(star->angle) >> 6)) >> 0xC;
        position.vy = ((star->radius >> 6) * (csin(star->angle) >> 6)) >> 0xC;
        position.vz = star->unk0E;
        gte_ldv0(&position);
        gte_rtps();
        wmap_step_actor_animation(draw, &g_wmap_actor_animations[i]);
        gte_stsxy(&screen);
        if (star->angle != 0)
        {
            wmap_draw_actor_sprite(draw, screen, 8, 4, 0);
        }
        else
        {
            wmap_draw_actor_sprite(draw, screen, 8, 0, 0);
        }
        star->angle = ((u16)star->angle + star->delta) & 0xFFF;
    }
    if (--g_wmap_land_effect_04_sequence_9_timer == 0)
    {
        g_wmap_land_effect_04_sequence_9_step += 1;
    }
}

/**
 * @brief Project and draw the world-map star field, spinning each entry each frame.
 */
void wmap_land_effect_04_sequence_9_step_04(void)
{
    SVECTOR position;
    s32 screen;
    WmapStar* star;
    WmapSpriteActor* draw;
    s32 i;

    for (i = 0x64; i < 0x82; i++)
    {
        draw = &g_wmap_sprite_actors[i];
        star = &g_wmap_actor_motions[i];
        position.vx = ((star->radius >> 6) * (ccos(star->angle) >> 6)) >> 0xC;
        position.vy = ((star->radius >> 6) * (csin(star->angle) >> 6)) >> 0xC;
        position.vz = star->unk0E;
        gte_ldv0(&position);
        gte_rtps();
        wmap_step_actor_animation(draw, &g_wmap_actor_animations[i]);
        gte_stsxy(&screen);
        if (star->angle != 0)
        {
            wmap_draw_actor_sprite(draw, screen, 8, 4, 0);
        }
        else
        {
            wmap_draw_actor_sprite(draw, screen, 8, 0, 0);
        }
        star->angle = ((u16)star->angle + star->delta) & 0xFFF;
    }
    if (--g_wmap_land_effect_04_sequence_9_timer == 0)
    {
        g_wmap_land_effect_04_sequence_9_step += 1;
    }
}

WMAP_STEP_RUNNER(wmap_land_effect_04_run, D_800D4DD0, 0x6, g_wmap_land_effect_04_step, g_wmap_land_effect_04_timer)

WMAP_STEP_RESET(wmap_land_effect_04_reset, g_wmap_land_effect_04_step, g_wmap_land_effect_04_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_04_step_01, g_wmap_land_effect_04_step, wmap_run_land_focus, wmap_land_effect_04_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_04_wait_idle_02, g_wmap_land_effect_04_step, wmap_land_effect_04_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_04_step_03, g_wmap_land_effect_04_step, wmap_land_effect_04_run_timeline, wmap_land_effect_04_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_04_wait_idle_04, g_wmap_land_effect_04_step, wmap_land_effect_04_end)

WMAP_STEP_ADVANCE(wmap_land_effect_04_end, g_wmap_land_effect_04_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_timeline, D_800D4DE8, 0x14, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_04_timeline_reset, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

/** @brief World-map step handler: kick a sub-task and advance the step counter. */
void wmap_land_effect_04_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 4;
    g_wmap_land_effect_04_timeline_timer = 0x14;
    g_wmap_land_effect_04_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_02, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

/** @brief World-map step handler: register a callback and advance the step counter. */
void wmap_land_effect_04_timeline_step_03(void)
{
    wmap_play_sound(0x24, 0x80);
    wmap_start_sequence(wmap_land_effect_04_run_sequence_3);
    g_wmap_land_effect_04_timeline_timer = 4;
    g_wmap_land_effect_04_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_04, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_04_timeline_step_05, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer,
                         wmap_land_effect_04_run_sequence_5, 0x1E)

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_06, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

/** @brief Register two callbacks, set sequence flags, and begin a one-tick delay. */
void wmap_land_effect_04_timeline_step_07(void)
{
    wmap_start_sequence(&wmap_land_effect_04_run_sequence_7);
    g_wmap_backdrop_target_level = 2;
    g_wmap_transition_mesh_hidden = 1;
    wmap_start_sequence(&wmap_land_effect_04_run_sequence_2);
    g_wmap_placement_overlay_hidden = 1;
    g_wmap_land_effect_04_timeline_timer = 1;
    g_wmap_land_effect_04_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_08, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_04_timeline_step_09, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer,
                         wmap_land_effect_04_run_sequence_6, 0x3)

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_10, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_04_timeline_step_11, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer,
                         wmap_land_effect_04_run_sequence_1, 0x18)

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_12, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_04_timeline_step_13, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer,
                         wmap_land_effect_04_run_sequence_9, 0x1E)

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_14, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_04_timeline_step_15, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer,
                         wmap_land_effect_04_run_sequence_4, 0x3C)

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_16, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

/** @brief Register a callback, update the selected map cell, and begin a 68-tick delay. */
void wmap_land_effect_04_timeline_step_17(void)
{
    wmap_start_sequence(&wmap_land_effect_04_run_sequence_8);
    g_wmap_transition_mesh_hidden = 0;
    g_wmap_land_effect_04_timeline_timer = 0x44;
    g_wmap_cells[g_wmap_focus_cell_x][g_wmap_focus_cell_y].land_id = g_wmap_selected_artifact | 0x100;
    g_wmap_land_effect_04_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_04_timeline_wait_18, g_wmap_land_effect_04_timeline_step, g_wmap_land_effect_04_timeline_timer)

/**
 * @brief Clear the blocking flag and finish the timeline.
 */
WMAP_STEP_FINISH_BLOCKING(wmap_land_effect_04_timeline_finish,
    g_wmap_land_effect_04_timeline_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_1, D_800D4E38, 0x4, g_wmap_land_effect_04_sequence_1_step, g_wmap_land_effect_04_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_1_reset, g_wmap_land_effect_04_sequence_1_step, g_wmap_land_effect_04_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_04_sequence_1_step_01,
    g_wmap_land_effect_04_sequence_1_step, g_wmap_land_effect_04_sequence_1_timer,
    4, g_wmap_animation_bank_1, 0,
    0, 0x80, 8,
    0x7C, wmap_land_effect_04_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_04_sequence_1_step_02, g_wmap_land_effect_04_sequence_1_step, g_wmap_land_effect_04_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0xB, 0x9, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_1_end, g_wmap_land_effect_04_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_2, D_800D4E48, 0x4, g_wmap_land_effect_04_sequence_2_step, g_wmap_land_effect_04_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_2_reset, g_wmap_land_effect_04_sequence_2_step, g_wmap_land_effect_04_sequence_2_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_04_sequence_2_step_01(void)
{
    s32 one = 1;
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    g_wmap_actor_animations[6].data = g_wmap_animation_bank_1;
    actor->scale_index = 0xF;
    actor->sequence = 1;
    actor->previous_sequence = -one;
    actor->shade_step = 2;
    actor->resource_index = 0;
    actor->target_shade = 0x81;
    actor->shade = 1;
    g_wmap_land_effect_04_sequence_2_timer = 0x7C;
    g_wmap_land_effect_04_sequence_2_step += one;
    wmap_land_effect_04_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_04_sequence_2_step_02, g_wmap_land_effect_04_sequence_2_step, g_wmap_land_effect_04_sequence_2_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0xB, 0xA, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_2_end, g_wmap_land_effect_04_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_3, D_800D4E58, 0x4, g_wmap_land_effect_04_sequence_3_step, g_wmap_land_effect_04_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_3_reset, g_wmap_land_effect_04_sequence_3_step, g_wmap_land_effect_04_sequence_3_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_04_sequence_3_step_01,
    g_wmap_land_effect_04_sequence_3_step, g_wmap_land_effect_04_sequence_3_timer,
    7, g_wmap_animation_bank_1, 2,
    1, 0x81, 0x10,
    0x28, wmap_land_effect_04_sequence_3_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_04_sequence_3_step_02, g_wmap_land_effect_04_sequence_3_step, g_wmap_land_effect_04_sequence_3_timer, g_wmap_sprite_actors[7],
                              g_wmap_actor_animations[7], g_wmap_focus_screen_position, 0xB, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_3_end, g_wmap_land_effect_04_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_4, D_800D4E68, 0x4, g_wmap_land_effect_04_sequence_4_step, g_wmap_land_effect_04_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_4_reset, g_wmap_land_effect_04_sequence_4_step, g_wmap_land_effect_04_sequence_4_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_04_sequence_4_step_01,
    g_wmap_land_effect_04_sequence_4_step, g_wmap_land_effect_04_sequence_4_timer,
    5, g_wmap_animation_bank_0, 0,
    0, 0x80, 8,
    0x3D, wmap_land_effect_04_sequence_4_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_04_sequence_4_step_02, g_wmap_land_effect_04_sequence_4_step, g_wmap_land_effect_04_sequence_4_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x1C, 0x8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_4_end, g_wmap_land_effect_04_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_5, D_800D4E78, 0x4, g_wmap_land_effect_04_sequence_5_step, g_wmap_land_effect_04_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_5_reset, g_wmap_land_effect_04_sequence_5_step, g_wmap_land_effect_04_sequence_5_timer)

/** @brief Draw the sequence effect and advance when its countdown expires. */
void wmap_land_effect_04_sequence_5_step_02(void)
{
    s32 remaining_ticks;

    func_8006BC44(0x14, 0x3C, g_wmap_effect_params, 0);
    remaining_ticks = g_wmap_land_effect_04_sequence_5_timer - 1;
    g_wmap_land_effect_04_sequence_5_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_04_sequence_5_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_5_end, g_wmap_land_effect_04_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_6, D_800D4E88, 0x6, g_wmap_land_effect_04_sequence_6_step, g_wmap_land_effect_04_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_6_reset, g_wmap_land_effect_04_sequence_6_step, g_wmap_land_effect_04_sequence_6_timer)

/** @brief World-map step: reset counters and advance to the next handler. */
void wmap_land_effect_04_sequence_6_step_01(void)
{
    D_801B24B0 = 0;
    D_801B2468 = 1;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    D_801B2498.vx = 0;
    D_801B2498.vy = 0;
    D_801B2498.vz = 0;
    g_wmap_land_effect_04_sequence_6_timer = 0x54;
    g_wmap_land_effect_04_sequence_6_step += 1;
    wmap_land_effect_04_sequence_6_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_04_sequence_6_step_03, g_wmap_land_effect_04_sequence_6_step, g_wmap_land_effect_04_sequence_6_timer, 0x20,
                    wmap_land_effect_04_sequence_6_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_6_end, g_wmap_land_effect_04_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_7, D_800D4EA0, 0x4, g_wmap_land_effect_04_sequence_7_step, g_wmap_land_effect_04_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_7_reset, g_wmap_land_effect_04_sequence_7_step, g_wmap_land_effect_04_sequence_7_timer)

WMAP_STEP_DROP_START(wmap_land_effect_04_sequence_7_step_01, g_wmap_land_effect_04_sequence_7_step, g_wmap_land_effect_04_sequence_7_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x80, wmap_land_effect_04_sequence_7_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_7_end, g_wmap_land_effect_04_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_8, D_800D4EB0, 0x4, g_wmap_land_effect_04_sequence_8_step, g_wmap_land_effect_04_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_8_reset, g_wmap_land_effect_04_sequence_8_step, g_wmap_land_effect_04_sequence_8_timer)

WMAP_STEP_DROP_START(wmap_land_effect_04_sequence_8_step_01, g_wmap_land_effect_04_sequence_8_step, g_wmap_land_effect_04_sequence_8_timer, g_wmap_effect_model_b_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x40, wmap_land_effect_04_sequence_8_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_8_end, g_wmap_land_effect_04_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_04_run_sequence_9, D_800D4EC0, 0x6, g_wmap_land_effect_04_sequence_9_step, g_wmap_land_effect_04_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_04_sequence_9_reset, g_wmap_land_effect_04_sequence_9_step, g_wmap_land_effect_04_sequence_9_timer)

/**
 * @brief Start fading the sprite range and run its first update.
 */
WMAP_STEP_FADE_ACTOR_RANGE(wmap_land_effect_04_sequence_9_step_03,
    g_wmap_land_effect_04_sequence_9_step, g_wmap_land_effect_04_sequence_9_timer,
    0x64, 0x82, 2, 0x40, wmap_land_effect_04_sequence_9_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_04_sequence_9_end, g_wmap_land_effect_04_sequence_9_step)
