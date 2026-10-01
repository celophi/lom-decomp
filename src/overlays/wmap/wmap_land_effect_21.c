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
#include "wmap_step_sequence.h"
#include "wmap_cells.h"

void wmap_land_effect_21_sequence_7_step_02(void);
void wmap_land_effect_21_sequence_8_step_02(void);
void wmap_land_effect_21_wait_idle_02(void);
void wmap_land_effect_21_step_03(void);
void wmap_land_effect_21_wait_idle_04(void);
s32 wmap_land_effect_21_run_timeline(s32 arg0);
void wmap_land_effect_21_end(void);
s32 wmap_land_effect_21_run_sequence_3(s32 arg0);
s32 wmap_land_effect_21_run_sequence_1(s32 arg0);
s32 wmap_land_effect_21_run_sequence_6(s32 arg0);
s32 wmap_land_effect_21_run_sequence_5(s32 arg0);
s32 wmap_land_effect_21_run_sequence_8(s32 arg0);
s32 wmap_land_effect_21_run_sequence_4(s32 arg0);
s32 wmap_land_effect_21_run_sequence_7(s32 arg0);
s32 wmap_land_effect_21_run_sequence_2(s32 arg0);
void wmap_land_effect_21_sequence_1_step_02(void);
void wmap_land_effect_21_sequence_1_step_04(void);
void wmap_land_effect_21_sequence_1_step_06(void);
void wmap_land_effect_21_sequence_1_step_08(void);
void wmap_land_effect_21_sequence_2_step_02(void);
void wmap_land_effect_21_sequence_5_step_04(void);
void wmap_land_effect_21_sequence_6_step_02(void);
void wmap_land_effect_21_sequence_6_step_04(void);
void wmap_land_effect_21_sequence_6_step_06(void);
void wmap_land_effect_21_sequence_7_step_04(void);

/** @brief Drawing position with the original 0x14-byte stride. */
typedef struct { s32 position; u8 pad[0x10]; } WmapPosition;

extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_21_sequence_3_timer;
extern u8* g_wmap_effect_model_pack_1;
extern s32 g_wmap_land_effect_21_sequence_4_timer;
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 g_wmap_land_effect_21_sequence_5_timer;
extern WmapPosition D_801AFBE0[];
extern u8 g_wmap_animation_bank_0[];
extern s32 g_wmap_land_effect_21_sequence_7_timer;
extern u8 g_wmap_animation_bank_2[];
extern s32 g_wmap_land_effect_21_sequence_8_timer;
extern s32 g_wmap_land_effect_21_timer;
extern void (*D_800D5B60[])(void);
extern void wmap_land_effect_21_end(void);
extern s32 g_wmap_land_effect_21_timeline_timer;
extern void (*D_800D5B78[])(void);
extern u32 g_wmap_selected_artifact;
extern s32 g_wmap_land_effect_21_sequence_1_timer;
extern void (*D_800D5BB8[])(void);
extern void wmap_land_effect_21_sequence_1_step_02(void);
extern void wmap_land_effect_21_sequence_1_step_08(void);
extern s32 g_wmap_land_effect_21_sequence_2_timer;
extern void (*D_800D5BE0[])(void);
extern u8 g_wmap_animation_bank_1[];
extern void wmap_land_effect_21_sequence_2_step_02(void);
extern void (*D_800D5BF0[])(void);
extern void (*D_800D5C00[])(void);
extern void (*D_800D5C10[])(void);
extern s32 g_wmap_land_effect_21_sequence_6_timer;
extern void (*D_800D5C30[])(void);
extern void wmap_land_effect_21_sequence_6_step_06(void);
extern void (*D_800D5C50[])(void);
extern void wmap_land_effect_21_sequence_7_step_04(void);
extern void (*D_800D5C68[])(void);

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

extern u32 g_wmap_land_effect_21_sequence_3_step;
extern u32 g_wmap_land_effect_21_sequence_4_step;
extern u32 g_wmap_land_effect_21_sequence_5_step;
extern u32 g_wmap_land_effect_21_sequence_7_step;
extern u32 g_wmap_land_effect_21_sequence_8_step;
extern u32 g_wmap_land_effect_21_step;
extern u32 g_wmap_land_effect_21_timeline_step;
extern u32 g_wmap_land_effect_21_sequence_1_step;
extern u32 g_wmap_land_effect_21_sequence_2_step;
extern u32 g_wmap_land_effect_21_sequence_6_step;

extern VECTOR g_wmap_camera_translation;



extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;
extern WmapScreenPosition D_80182D58;

extern s32* g_wmap_effect_params;

extern WmapMotion g_wmap_actor_motions[];
extern WmapMotion D_801AFD60[];
extern WmapMotion D_801B0080;

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_21_sequence_3_step_02,
    g_wmap_land_effect_21_sequence_3_step, g_wmap_land_effect_21_sequence_3_timer,
    g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position,
    g_wmap_effect_fade_a, g_wmap_load_buffer, -3500, 5)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_21_sequence_4_step_02,
    g_wmap_land_effect_21_sequence_4_step, g_wmap_land_effect_21_sequence_4_timer,
    g_wmap_effect_model_b_rotation, g_wmap_effect_model_b_position,
    g_wmap_effect_fade_b, g_wmap_effect_model_pack_1, -3500, 2)

/** @brief Project a spiral emitter and append animated copies along its trail. */
void wmap_land_effect_21_sequence_5_step_02(void)
{
    SVECTOR position;
    s32 previous;
    s32 next;
    s32 i;
    s32 remaining;
    WmapMotion *motion;
    WmapAnimationSlot *resource;
    WmapSpriteActor *actor;

    position.vx = ((D_801B0080.z >> 3) * (ccos(D_801B0080.angle) >> 6)) >> 12;
    position.vy = ((D_801B0080.z >> 3) * (csin(D_801B0080.angle) >> 6)) >> 12;
    position.vz = D_801B0080.field_0E;
    gte_ldv0(&position);
    gte_rtps();
    D_801B0080.z += 1500;
    D_801B0080.angle += 192;
    D_8013923C--;
    gte_stsxy(&D_80182D58);
    D_801B0080.screen.point.x = D_80182D58.point.x;
    D_801B0080.screen.point.y = D_80182D58.point.y;
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
            g_wmap_actor_animations[D_80139234 + 60] = g_wmap_actor_animations[60];
            g_wmap_sprite_actors[D_80139234 + 60] = g_wmap_sprite_actors[60];
            g_wmap_sprite_actors[D_80139234 + 60].sequence = 0;
            g_wmap_sprite_actors[D_80139234 + 60].previous_sequence = -1;
        }
    }
    for (i = 61; i < D_80139234 + 60; i++)
    {
        actor = &g_wmap_sprite_actors[i];
        motion = &g_wmap_actor_motions[i];
        resource = &g_wmap_actor_animations[i];
        wmap_step_actor_animation(actor, resource);
        wmap_draw_actor_sprite(actor, motion->screen.packed, 8, 10, 0);
    }
    remaining = g_wmap_land_effect_21_sequence_5_timer - 1;
    g_wmap_land_effect_21_sequence_5_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_21_sequence_5_step++;
    }
}

/** @brief Animate and draw the active actor range, then advance its countdown. */
void wmap_land_effect_21_sequence_5_step_06(void)
{
    s32 i;
    s32 remaining;

    for (i = 61; i < D_80139234 + 60; i++)
    {
        g_wmap_sprite_actors[i].target_shade = 0;
        g_wmap_sprite_actors[i].shade_step = 2;
        wmap_step_actor_animation(&g_wmap_sprite_actors[i], &g_wmap_actor_animations[i]);
        wmap_draw_actor_sprite(&g_wmap_sprite_actors[i], D_801AFBE0[i].position, 8, 10, 0);
    }
    remaining = g_wmap_land_effect_21_sequence_5_timer - 1;
    g_wmap_land_effect_21_sequence_5_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_21_sequence_5_step++;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_21_sequence_7_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 24;
    g_wmap_effect_params[0x1F] = 1;
    g_wmap_effect_params[0x20] = 1;
    g_wmap_effect_params[0x21] = 20;
    g_wmap_effect_params[0x22] = 30;
    g_wmap_effect_params[0x23] = 2;
    g_wmap_effect_params[0x24] = 1500;
    g_wmap_effect_params[0x25] = 20;
    g_wmap_effect_params[0x26] = 15;
    g_wmap_effect_params[0x27] = 5;
    g_wmap_effect_params[0x28] = 10000;
    WMAP_RESET_PARTICLE_SLOTS(i, 24,
                              g_wmap_actor_motions[i + g_wmap_effect_params[0x25]].state,
                              24, g_wmap_animation_bank_0);
    g_wmap_land_effect_21_sequence_7_timer = 48;
    g_wmap_land_effect_21_sequence_7_step++;
    wmap_land_effect_21_sequence_7_step_02();
}

/** @brief Initialize the effect actors, resources, and evenly spaced angles. */
void wmap_land_effect_21_sequence_8_step_01(void)
{
    s32 i;
    WmapSpriteActor* actors = &g_wmap_sprite_actors[20];

    i = 0;
    g_wmap_particle_intensity = 12;
    g_wmap_effect_params[0x15] = 128;
    g_wmap_effect_params[0x16] = 1;
    g_wmap_effect_params[0x17] = 128;
    g_wmap_effect_params[0x18] = 0;
    g_wmap_effect_params[0x19] = -1;
    g_wmap_effect_params[0x1A] = 1500;
    g_wmap_effect_params[0x1B] = 20;
    g_wmap_effect_params[0x1C] = 8;
    g_wmap_effect_params[0x1D] = 1;
    g_wmap_effect_params[0x1E] = 10;
    do
    {
        D_801AFD60[i].state = 1;
        D_801AFD60[i].angle = i * 341;
        D_801AFD60[i].scale = 128;
        D_801AFD60[i].z = 10;
        D_801AFD60[i].x = 0;
        D_801AFD60[i].field_0E = 0;
        g_wmap_actor_animations[i + 20].data = g_wmap_animation_bank_2;
        actors[i].scale_index = 15;
        actors[i].previous_sequence = -1;
        actors[i].shade_step = 2;
        actors[i].resource_index = 0;
        actors[i].sequence = 1;
        actors[i].target_shade = 0;
        actors[i].shade = 127;
        i++;
    } while (i < 12);
    g_wmap_land_effect_21_sequence_8_timer = 32;
    g_wmap_land_effect_21_sequence_8_step++;
    wmap_land_effect_21_sequence_8_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_21_run, D_800D5B60, 0x6, g_wmap_land_effect_21_step, g_wmap_land_effect_21_timer)

WMAP_STEP_RESET(wmap_land_effect_21_reset, g_wmap_land_effect_21_step, g_wmap_land_effect_21_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_21_step_01, g_wmap_land_effect_21_step, wmap_run_land_focus, wmap_land_effect_21_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_21_wait_idle_02, g_wmap_land_effect_21_step, wmap_land_effect_21_step_03)

/** @brief Save the coordinate pair, register a callback, and run the next sequence step. */
void wmap_land_effect_21_step_03(void)
{
    D_80182D58.packed = g_wmap_focus_screen_position.packed;
    wmap_start_sequence(&wmap_land_effect_21_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_effect_21_step += 1;
    wmap_land_effect_21_wait_idle_04();
}

WMAP_STEP_WAIT_IDLE(wmap_land_effect_21_wait_idle_04, g_wmap_land_effect_21_step, wmap_land_effect_21_end)

WMAP_STEP_ADVANCE(wmap_land_effect_21_end, g_wmap_land_effect_21_step)

WMAP_STEP_RUNNER(wmap_land_effect_21_run_timeline, D_800D5B78, 0x10, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_21_timeline_reset, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

/** @brief Play sound 34, set the world-map color and value, and begin an eight-tick delay. */
void wmap_land_effect_21_timeline_step_01(void)
{
    wmap_play_sound(0x22, 0x80);
    g_wmap_backdrop_target_level = 4;
    wmap_start_map_tint(0x304010);
    g_wmap_land_effect_21_timeline_timer = 8;
    g_wmap_land_effect_21_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_21_timeline_wait_02, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_21_timeline_step_03, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer,
                         wmap_land_effect_21_run_sequence_3, 0x4)

WMAP_STEP_WAIT(wmap_land_effect_21_timeline_wait_04, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

/**
 * @brief Hide the placement overlay, start the sequence, and set the wait timer.
 */
WMAP_STEP_HIDE_AND_START(wmap_land_effect_21_timeline_step_05,
    g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer,
    g_wmap_placement_overlay_hidden, wmap_land_effect_21_run_sequence_1, 0x34)

WMAP_STEP_WAIT(wmap_land_effect_21_timeline_wait_06, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_21_timeline_step_07, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer,
                         wmap_land_effect_21_run_sequence_6, 0x38)

WMAP_STEP_WAIT(wmap_land_effect_21_timeline_wait_08, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_21_timeline_step_09, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer,
                             wmap_land_effect_21_run_sequence_5, wmap_land_effect_21_run_sequence_8, 0x68)

WMAP_STEP_WAIT(wmap_land_effect_21_timeline_wait_10, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_21_timeline_step_11, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer,
                         wmap_land_effect_21_run_sequence_4, 0x3C)

WMAP_STEP_WAIT(wmap_land_effect_21_timeline_wait_12, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_21_timeline_step_13, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer,
                             wmap_land_effect_21_run_sequence_7, wmap_land_effect_21_run_sequence_2, 0x8B)

WMAP_STEP_WAIT(wmap_land_effect_21_timeline_wait_14, g_wmap_land_effect_21_timeline_step, g_wmap_land_effect_21_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_21_timeline_finish, g_wmap_land_effect_21_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y, g_wmap_selected_artifact)

WMAP_STEP_RUNNER(wmap_land_effect_21_run_sequence_1, D_800D5BB8, 0xA, g_wmap_land_effect_21_sequence_1_step, g_wmap_land_effect_21_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_21_sequence_1_reset, g_wmap_land_effect_21_sequence_1_step, g_wmap_land_effect_21_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_21_sequence_1_step_01,
    g_wmap_land_effect_21_sequence_1_step, g_wmap_land_effect_21_sequence_1_timer,
    4, g_wmap_animation_bank_0, 0,
    1, 0x81, 8,
    0x14, wmap_land_effect_21_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_21_sequence_1_step_02, g_wmap_land_effect_21_sequence_1_step, g_wmap_land_effect_21_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], D_80182D58, 0xF, 0x9, 0)

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void wmap_land_effect_21_sequence_1_step_03(void)
{
    g_wmap_sprite_actors[4].sequence = 1;
    g_wmap_land_effect_21_sequence_1_timer = 0x20;
    g_wmap_land_effect_21_sequence_1_step += 1;
    wmap_land_effect_21_sequence_1_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_21_sequence_1_step_04, g_wmap_land_effect_21_sequence_1_step, g_wmap_land_effect_21_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], D_80182D58, 0xF, 0x9, 0)

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void wmap_land_effect_21_sequence_1_step_05(void)
{
    g_wmap_sprite_actors[4].sequence = 2;
    g_wmap_land_effect_21_sequence_1_timer = 0x8C;
    g_wmap_land_effect_21_sequence_1_step += 1;
    wmap_land_effect_21_sequence_1_step_06();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_21_sequence_1_step_06, g_wmap_land_effect_21_sequence_1_step, g_wmap_land_effect_21_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], D_80182D58, 0xF, 0x9, 0)

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_21_sequence_1_step_07(void)
{
    g_wmap_sprite_actors[4].shade_step = 8;
    g_wmap_sprite_actors[4].target_shade = 0;
    g_wmap_land_effect_21_sequence_1_timer = 0x10;
    g_wmap_land_effect_21_sequence_1_step += 1;
    wmap_land_effect_21_sequence_1_step_08();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_21_sequence_1_step_08, g_wmap_land_effect_21_sequence_1_step, g_wmap_land_effect_21_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], D_80182D58, 0xF, 0x9, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_21_sequence_1_end, g_wmap_land_effect_21_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_21_run_sequence_2, D_800D5BE0, 0x4, g_wmap_land_effect_21_sequence_2_step, g_wmap_land_effect_21_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_21_sequence_2_reset, g_wmap_land_effect_21_sequence_2_step, g_wmap_land_effect_21_sequence_2_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_21_sequence_2_step_01,
    g_wmap_land_effect_21_sequence_2_step, g_wmap_land_effect_21_sequence_2_timer,
    5, g_wmap_animation_bank_1, 0,
    0, 0x80, 8,
    0x8C, wmap_land_effect_21_sequence_2_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_21_sequence_2_step_02, g_wmap_land_effect_21_sequence_2_step, g_wmap_land_effect_21_sequence_2_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x18, 0x1E, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_21_sequence_2_end, g_wmap_land_effect_21_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_21_run_sequence_3, D_800D5BF0, 0x4, g_wmap_land_effect_21_sequence_3_step, g_wmap_land_effect_21_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_21_sequence_3_reset, g_wmap_land_effect_21_sequence_3_step, g_wmap_land_effect_21_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_21_sequence_3_step_01, g_wmap_land_effect_21_sequence_3_step, g_wmap_land_effect_21_sequence_3_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x28, wmap_land_effect_21_sequence_3_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_21_sequence_3_end, g_wmap_land_effect_21_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_21_run_sequence_4, D_800D5C00, 0x4, g_wmap_land_effect_21_sequence_4_step, g_wmap_land_effect_21_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_21_sequence_4_reset, g_wmap_land_effect_21_sequence_4_step, g_wmap_land_effect_21_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_21_sequence_4_step_01, g_wmap_land_effect_21_sequence_4_step, g_wmap_land_effect_21_sequence_4_timer, g_wmap_effect_model_b_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x80, wmap_land_effect_21_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_21_sequence_4_end, g_wmap_land_effect_21_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_21_run_sequence_5, D_800D5C10, 0x8, g_wmap_land_effect_21_sequence_5_step, g_wmap_land_effect_21_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_21_sequence_5_reset, g_wmap_land_effect_21_sequence_5_step, g_wmap_land_effect_21_sequence_5_timer)

/** @brief Initialize the actor and auxiliary state for the next timed step. */
void wmap_land_effect_21_sequence_5_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[60];

    D_80139234 = 0;
    D_8013923C = 2;
    g_wmap_actor_animations[60].data = g_wmap_animation_bank_2;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 8;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->target_shade = 0x81;
    actor->shade = 0x81;
    D_801B0080.state = 1;
    D_801B0080.z = 0;
    D_801B0080.angle = 0;
    D_801B0080.field_0E = 0;
    g_wmap_land_effect_21_sequence_5_timer = 0x70;
    g_wmap_land_effect_21_sequence_5_step += 1;
    wmap_land_effect_21_sequence_5_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_21_sequence_5_step_03, g_wmap_land_effect_21_sequence_5_step, g_wmap_land_effect_21_sequence_5_timer, 0x28,
                    wmap_land_effect_21_sequence_5_step_04)

/** @brief Animate and draw the active actor range, then advance its countdown. */
void wmap_land_effect_21_sequence_5_step_04(void)
{
    s32 i;
    s32 remaining;

    for (i = 61; i < D_80139234 + 60; i++)
    {
        wmap_step_actor_animation(&g_wmap_sprite_actors[i], &g_wmap_actor_animations[i]);
        wmap_draw_actor_sprite(&g_wmap_sprite_actors[i], D_801AFBE0[i].position, 8, 10, 0);
    }
    remaining = g_wmap_land_effect_21_sequence_5_timer - 1;
    g_wmap_land_effect_21_sequence_5_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_21_sequence_5_step++;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_21_sequence_5_step_05, g_wmap_land_effect_21_sequence_5_step, g_wmap_land_effect_21_sequence_5_timer, 0x40,
                    wmap_land_effect_21_sequence_5_step_06)

WMAP_STEP_ADVANCE(wmap_land_effect_21_sequence_5_end, g_wmap_land_effect_21_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_21_run_sequence_6, D_800D5C30, 0x8, g_wmap_land_effect_21_sequence_6_step, g_wmap_land_effect_21_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_21_sequence_6_reset, g_wmap_land_effect_21_sequence_6_step, g_wmap_land_effect_21_sequence_6_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_21_sequence_6_step_01,
    g_wmap_land_effect_21_sequence_6_step, g_wmap_land_effect_21_sequence_6_timer,
    6, g_wmap_animation_bank_0, 3,
    1, 0x81, 2,
    0x10, wmap_land_effect_21_sequence_6_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_21_sequence_6_step_02, g_wmap_land_effect_21_sequence_6_step, g_wmap_land_effect_21_sequence_6_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], D_80182D58, 0xF, 0x2, 0)

/**
 * @brief Kick off a world-map step: arm a flag and wait, bump the index, run it.
 */
void wmap_land_effect_21_sequence_6_step_03(void)
{
    g_wmap_sprite_actors[6].sequence = 4;
    g_wmap_land_effect_21_sequence_6_timer = 0x8C;
    g_wmap_land_effect_21_sequence_6_step += 1;
    wmap_land_effect_21_sequence_6_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_21_sequence_6_step_04, g_wmap_land_effect_21_sequence_6_step, g_wmap_land_effect_21_sequence_6_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], D_80182D58, 0xF, 0x2, 0)

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_21_sequence_6_step_05(void)
{
    g_wmap_sprite_actors[6].shade_step = 8;
    g_wmap_sprite_actors[6].target_shade = 0;
    g_wmap_land_effect_21_sequence_6_timer = 0x10;
    g_wmap_land_effect_21_sequence_6_step += 1;
    wmap_land_effect_21_sequence_6_step_06();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_21_sequence_6_step_06, g_wmap_land_effect_21_sequence_6_step, g_wmap_land_effect_21_sequence_6_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], D_80182D58, 0xF, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_21_sequence_6_end, g_wmap_land_effect_21_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_21_run_sequence_7, D_800D5C50, 0x6, g_wmap_land_effect_21_sequence_7_step, g_wmap_land_effect_21_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_21_sequence_7_reset, g_wmap_land_effect_21_sequence_7_step, g_wmap_land_effect_21_sequence_7_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_21_sequence_7_step_02,
    g_wmap_land_effect_21_sequence_7_step, g_wmap_land_effect_21_sequence_7_timer,
    func_8006A2FC(&g_wmap_sprite_actors[24], &g_wmap_actor_animations[24], 0x18, 0, 0x7F, 0x2, 0, (s32)((u8*)g_wmap_effect_params + 0x78)))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_21_sequence_7_step_03,
    g_wmap_land_effect_21_sequence_7_step, g_wmap_land_effect_21_sequence_7_timer,
    g_wmap_effect_params[35], 0x20, wmap_land_effect_21_sequence_7_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_21_sequence_7_step_04,
    g_wmap_land_effect_21_sequence_7_step, g_wmap_land_effect_21_sequence_7_timer,
    func_8006A2FC(&g_wmap_sprite_actors[24], &g_wmap_actor_animations[24], 0x18, 0, 0x7F, 0x2, 0, (s32)((u8*)g_wmap_effect_params + 0x78)))

WMAP_STEP_ADVANCE(wmap_land_effect_21_sequence_7_end, g_wmap_land_effect_21_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_21_run_sequence_8, D_800D5C68, 0x4, g_wmap_land_effect_21_sequence_8_step, g_wmap_land_effect_21_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_21_sequence_8_reset, g_wmap_land_effect_21_sequence_8_step, g_wmap_land_effect_21_sequence_8_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_21_sequence_8_step_02,
    g_wmap_land_effect_21_sequence_8_step, g_wmap_land_effect_21_sequence_8_timer,
    func_8006A2FC(&g_wmap_sprite_actors[20], &g_wmap_actor_animations[20], 0xC, 0, 0x7F, 0x2, 0, (s32)((u8*)g_wmap_effect_params + 0x50)))

WMAP_STEP_ADVANCE(wmap_land_effect_21_sequence_8_end, g_wmap_land_effect_21_sequence_8_step)
