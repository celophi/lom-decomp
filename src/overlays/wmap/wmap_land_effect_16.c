#include "wmap_main.h"
#include "wmap_land_effect_16.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"
#include "wmap_step_sequence.h"
#include "wmap_cells.h"

void wmap_land_effect_16_sequence_6_step_02(void);
void wmap_land_effect_16_sequence_11_step_02(void);
void wmap_land_effect_16_wait_idle_02(void);
void wmap_land_effect_16_step_03(void);
s32 wmap_land_effect_16_run_timeline(s32 arg0);
void wmap_land_effect_16_wait_idle_04(void);
void wmap_land_effect_16_end(void);
s32 wmap_land_effect_16_run_sequence_1(s32 arg0);
s32 wmap_land_effect_16_run_sequence_6(s32 arg0);
s32 wmap_land_effect_16_run_sequence_4(s32 arg0);
s32 wmap_land_effect_16_run_sequence_9(s32 arg0);
s32 wmap_land_effect_16_run_sequence_10(s32 arg0);
s32 wmap_land_effect_16_run_sequence_2(s32 arg0);
s32 wmap_land_effect_16_run_sequence_8(s32 arg0);
s32 wmap_land_effect_16_run_sequence_11(s32 arg0);
s32 wmap_land_effect_16_run_sequence_3(s32 arg0);
s32 wmap_land_effect_16_run_sequence_5(s32 arg0);
s32 wmap_land_effect_16_run_sequence_7(s32 arg0);
void wmap_land_effect_16_sequence_1_step_02(void);
void wmap_land_effect_16_sequence_2_step_02(void);
void wmap_land_effect_16_sequence_2_step_04(void);
void wmap_land_effect_16_sequence_3_step_02(void);
void wmap_land_effect_16_sequence_6_step_04(void);
void wmap_land_effect_16_sequence_7_step_02(void);
void wmap_land_effect_16_sequence_9_step_02(void);
void wmap_land_effect_16_sequence_10_step_02(void);
void wmap_land_effect_16_sequence_10_step_04(void);
void wmap_land_effect_16_sequence_11_step_04(void);

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
extern s32 g_wmap_land_effect_16_sequence_4_timer;
extern u8* g_wmap_effect_model_pack_1;
extern s32 g_wmap_land_effect_16_sequence_5_timer;
extern s32 g_wmap_land_effect_16_sequence_6_timer;
extern u8* g_wmap_effect_model_pack_2;
extern s32 D_8013923C;
extern s32 g_wmap_land_effect_16_sequence_8_timer;
extern s32 g_wmap_land_effect_16_sequence_11_timer;
extern s32 g_wmap_land_effect_16_timer;
extern void (*D_800D63B8[])(void);
extern void wmap_land_effect_16_step_03(void);
extern void wmap_land_effect_16_end(void);
extern s32 g_wmap_land_effect_16_timeline_timer;
extern void (*D_800D63D0[])(void);
extern u32 g_wmap_selected_artifact;
extern s32 g_wmap_land_effect_16_sequence_1_timer;
extern void (*D_800D6430[])(void);
extern void wmap_land_effect_16_sequence_1_step_02(void);
extern s32 g_wmap_land_effect_16_sequence_2_timer;
extern void (*D_800D6440[])(void);
extern void wmap_land_effect_16_sequence_2_step_04(void);
extern s32 g_wmap_land_effect_16_sequence_3_timer;
extern void (*D_800D6458[])(void);
extern u8 g_wmap_animation_bank_0[];
extern void wmap_land_effect_16_sequence_3_step_02(void);
extern void (*D_800D6468[])(void);
extern void (*D_800D6478[])(void);
extern void (*D_800D6488[])(void);
extern s32 g_wmap_land_effect_16_sequence_7_timer;
extern void (*D_800D64A0[])(void);
extern void (*D_800D64B0[])(void);
extern s32 g_wmap_land_effect_16_sequence_9_timer;
extern void (*D_800D64C8[])(void);
extern u8 g_wmap_animation_bank_2[];
extern void wmap_land_effect_16_sequence_9_step_02(void);
extern s32 g_wmap_land_effect_16_sequence_10_timer;
extern void (*D_800D64D8[])(void);
extern void wmap_land_effect_16_sequence_10_step_04(void);
extern void (*D_800D64F0[])(void);
extern void wmap_land_effect_16_sequence_11_step_04(void);

extern u32 g_wmap_land_effect_16_sequence_4_step;
extern u32 g_wmap_land_effect_16_sequence_5_step;
extern u8 g_wmap_animation_bank_1[];
extern u32 g_wmap_land_effect_16_sequence_6_step;
extern u32 g_wmap_land_effect_16_sequence_8_step;
extern u32 g_wmap_land_effect_16_sequence_11_step;
extern u32 g_wmap_land_effect_16_step;
extern u32 g_wmap_land_effect_16_timeline_step;
extern u32 g_wmap_land_effect_16_sequence_1_step;
extern u32 g_wmap_land_effect_16_sequence_2_step;
extern u32 g_wmap_land_effect_16_sequence_3_step;
extern u32 g_wmap_land_effect_16_sequence_7_step;
extern u32 g_wmap_land_effect_16_sequence_9_step;
extern u32 g_wmap_land_effect_16_sequence_10_step;

extern VECTOR g_wmap_camera_translation;

extern SVECTOR D_801B2490;


extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;
extern WmapScreenPosition D_80182D60;

extern s32* g_wmap_effect_params;

extern WmapMotion g_wmap_actor_motions[];

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_16_sequence_4_step_02,
    g_wmap_land_effect_16_sequence_4_step, g_wmap_land_effect_16_sequence_4_timer,
    g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position,
    g_wmap_effect_fade_a, g_wmap_load_buffer, -3500, 2)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_16_sequence_5_step_02,
    g_wmap_land_effect_16_sequence_5_step, g_wmap_land_effect_16_sequence_5_timer,
    g_wmap_effect_model_b_rotation, g_wmap_effect_model_b_position,
    g_wmap_effect_fade_b, g_wmap_effect_model_pack_1, -3500, 2)

/** @brief Initialize four effect actors and their angular spacing. */
void wmap_land_effect_16_sequence_6_step_01(void)
{
    s32 i;
    s32 *descriptor;
    WmapSpriteActor *actor;

    D_801B2490 = g_wmap_zero_rotation;
    descriptor = g_wmap_effect_params;
    descriptor[0] = 1;
    descriptor[1] = 1;
    descriptor[2] = 1;
    g_wmap_effect_params[3] = 0;
    g_wmap_effect_params[4] = 1000;
    g_wmap_effect_params[5] = 96;
    g_wmap_effect_params[6] = 8;
    g_wmap_effect_params[7] = 255;
    g_wmap_effect_params[8] = 10;
    g_wmap_effect_params[9] = 19;
    for (i = 20; i < 80; i++)
    {
        g_wmap_actor_motions[i].state = 0;
    }
    for (i = 20; i < 80; i += 15)
    {
        actor = &g_wmap_sprite_actors[i];
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_1;
        actor->scale_index = 15;
        actor->previous_sequence = -1;
        actor->resource_index = 0;
        actor->sequence = 0;
        actor->shade_step = 2;
        actor->target_shade = 128;
        actor->shade = 128;
        g_wmap_actor_motions[i].state = 1;
        g_wmap_actor_motions[i].z = 0;
        g_wmap_actor_motions[i].angle = g_wmap_effect_params[3];
        g_wmap_actor_motions[i].field_0E = 0;
        g_wmap_effect_params[3] += 1024;
    }
    g_wmap_land_effect_16_sequence_6_timer = 48;
    g_wmap_land_effect_16_sequence_6_step++;
    wmap_land_effect_16_sequence_6_step_02();
}

/** @brief Draw and brighten the rotating model, then advance its countdown. */
void wmap_land_effect_16_sequence_8_step_02(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_c_rotation);
    wmap_draw_model_default(g_wmap_effect_model_pack_2, D_8013923C, 4, 0x35, 0x7800, 0x1001, g_wmap_effect_fade_c);
    g_wmap_effect_model_c_rotation.vz = (u16) (g_wmap_effect_model_c_rotation.vz + 0x10);
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_c, 2, 0x81, intensity);
    D_8013923C = (D_8013923C + 1) & 7;
    PopMatrix();
    remaining = g_wmap_land_effect_16_sequence_8_timer - 1;
    g_wmap_land_effect_16_sequence_8_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_16_sequence_8_step += 1;
    }
}

/** @brief Advance the world-map effect and its sequence state. */
void wmap_land_effect_16_sequence_8_step_04(void)
{
    s32 remaining;
    s32 intensity;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_c_rotation);
    wmap_draw_model_default(g_wmap_effect_model_pack_2, D_8013923C, 4, 0x35, 0x7800, 0x1001, g_wmap_effect_fade_c);
    intensity = g_wmap_effect_fade_c - 2;
    g_wmap_effect_model_c_rotation.vz = (u16) (g_wmap_effect_model_c_rotation.vz + 0x10);
    g_wmap_effect_fade_c = intensity;
    if (intensity < 0)
    {
        g_wmap_effect_fade_c = 0;
    }
    D_8013923C = (D_8013923C + 1) & 7;
    PopMatrix();
    remaining = g_wmap_land_effect_16_sequence_8_timer - 1;
    g_wmap_land_effect_16_sequence_8_timer = remaining;
    if (remaining == 0)
    {
        g_wmap_land_effect_16_sequence_8_step += 1;
    }
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_16_sequence_11_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 40;
    g_wmap_effect_params[0x1F] = -1;
    g_wmap_effect_params[0x20] = -4;
    g_wmap_effect_params[0x21] = 0x20;
    g_wmap_effect_params[0x22] = 0;
    g_wmap_effect_params[0x23] = 2;
    g_wmap_effect_params[0x24] = 0;
    g_wmap_effect_params[0x25] = 0xC8;
    g_wmap_effect_params[0x26] = 0x13;
    g_wmap_effect_params[0x27] = 2;
    g_wmap_effect_params[0x28] = 0x1F40;
    for (i = 0; i < 40; i++)
    {
        g_wmap_actor_motions[i + 200].state = 0;
        g_wmap_actor_animations[i + 204].data = g_wmap_animation_bank_1;
    }
    g_wmap_land_effect_16_sequence_11_timer = 80;
    g_wmap_land_effect_16_sequence_11_step++;
    wmap_land_effect_16_sequence_11_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_16_run, D_800D63B8, 0x6, g_wmap_land_effect_16_step, g_wmap_land_effect_16_timer)

WMAP_STEP_RESET(wmap_land_effect_16_reset, g_wmap_land_effect_16_step, g_wmap_land_effect_16_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_16_step_01, g_wmap_land_effect_16_step, wmap_run_land_focus, wmap_land_effect_16_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_16_wait_idle_02, g_wmap_land_effect_16_step, wmap_land_effect_16_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_16_step_03, g_wmap_land_effect_16_step, wmap_land_effect_16_run_timeline, wmap_land_effect_16_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_16_wait_idle_04, g_wmap_land_effect_16_step, wmap_land_effect_16_end)

WMAP_STEP_ADVANCE(wmap_land_effect_16_end, g_wmap_land_effect_16_step)

WMAP_STEP_RUNNER(wmap_land_effect_16_run_timeline, D_800D63D0, 0x18, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_16_timeline_reset, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer)

/** @brief World-map step handler: kick a sub-task and advance the step counter. */
void wmap_land_effect_16_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 4;
    g_wmap_land_effect_16_timeline_timer = 0x1E;
    g_wmap_land_effect_16_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_16_timeline_wait_02, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer)

/** @brief World-map step handler: register a callback and advance the step counter. */
void wmap_land_effect_16_timeline_step_03(void)
{
    wmap_play_sound(0x27, 0x80);
    wmap_start_sequence(wmap_land_effect_16_run_sequence_1);
    g_wmap_land_effect_16_timeline_timer = 0x10;
    g_wmap_land_effect_16_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_16_timeline_wait_04, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_16_timeline_step_05, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer,
                         wmap_land_effect_16_run_sequence_6, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_16_timeline_wait_06, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer)

/**
 * @brief Hide the placement overlay and set the wait timer.
 */
WMAP_STEP_HIDE_AND_WAIT(wmap_land_effect_16_timeline_step_07,
    g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer,
    g_wmap_placement_overlay_hidden, 0x28)

WMAP_STEP_WAIT(wmap_land_effect_16_timeline_wait_08, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_16_timeline_step_09, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer,
                         wmap_land_effect_16_run_sequence_4, 0x4)

WMAP_STEP_WAIT(wmap_land_effect_16_timeline_wait_10, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_16_timeline_step_11, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer,
                         wmap_land_effect_16_run_sequence_9, 0x14)

WMAP_STEP_WAIT(wmap_land_effect_16_timeline_wait_12, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_16_timeline_step_13, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer,
                         wmap_land_effect_16_run_sequence_10, 0x1E)

WMAP_STEP_WAIT(wmap_land_effect_16_timeline_wait_14, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_16_timeline_step_15, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer,
                             wmap_land_effect_16_run_sequence_2, wmap_land_effect_16_run_sequence_8, 0x8)

WMAP_STEP_WAIT(wmap_land_effect_16_timeline_wait_16, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_16_timeline_step_17, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer,
                         wmap_land_effect_16_run_sequence_11, 0xF)

WMAP_STEP_WAIT(wmap_land_effect_16_timeline_wait_18, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_16_timeline_step_19, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer,
                         wmap_land_effect_16_run_sequence_3, 0x80)

WMAP_STEP_WAIT(wmap_land_effect_16_timeline_wait_20, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_16_timeline_step_21, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer,
                             wmap_land_effect_16_run_sequence_5, wmap_land_effect_16_run_sequence_7, 0x80)

WMAP_STEP_WAIT(wmap_land_effect_16_timeline_wait_22, g_wmap_land_effect_16_timeline_step, g_wmap_land_effect_16_timeline_timer)

/** @brief Mark the current world-map tile state and advance the sequence. */
void wmap_land_effect_16_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    g_wmap_cells[g_wmap_focus_cell_x][g_wmap_focus_cell_y].land_id = g_wmap_selected_artifact | 0x100;
    g_wmap_land_effect_16_timeline_step += 1;
}

WMAP_STEP_RUNNER(wmap_land_effect_16_run_sequence_1, D_800D6430, 0x4, g_wmap_land_effect_16_sequence_1_step, g_wmap_land_effect_16_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_16_sequence_1_reset, g_wmap_land_effect_16_sequence_1_step, g_wmap_land_effect_16_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_16_sequence_1_step_01,
    g_wmap_land_effect_16_sequence_1_step, g_wmap_land_effect_16_sequence_1_timer,
    4, g_wmap_animation_bank_1, 0,
    1, 0x81, 0x10,
    0x40, wmap_land_effect_16_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_16_sequence_1_step_02, g_wmap_land_effect_16_sequence_1_step, g_wmap_land_effect_16_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0x13, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_16_sequence_1_end, g_wmap_land_effect_16_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_16_run_sequence_2, D_800D6440, 0x6, g_wmap_land_effect_16_sequence_2_step, g_wmap_land_effect_16_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_16_sequence_2_reset, g_wmap_land_effect_16_sequence_2_step, g_wmap_land_effect_16_sequence_2_timer)

/** @brief Start the sprite animation and run its first update. */
WMAP_STEP_START_ACTOR(wmap_land_effect_16_sequence_2_step_01,
    g_wmap_land_effect_16_sequence_2_step, g_wmap_land_effect_16_sequence_2_timer,
    9, g_wmap_animation_bank_1, 1,
    1, 0x81, 2,
    0x64, wmap_land_effect_16_sequence_2_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_16_sequence_2_step_02, g_wmap_land_effect_16_sequence_2_step, g_wmap_land_effect_16_sequence_2_timer, g_wmap_sprite_actors[9],
                              g_wmap_actor_animations[9], g_wmap_focus_screen_position, 0x13, 0x2, 0)

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_16_sequence_2_step_03(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[9];

    actor->target_shade = 0;
    actor->shade_step = 2;
    g_wmap_land_effect_16_sequence_2_timer = 0x40;
    g_wmap_land_effect_16_sequence_2_step += 1;
    wmap_land_effect_16_sequence_2_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_16_sequence_2_step_04, g_wmap_land_effect_16_sequence_2_step, g_wmap_land_effect_16_sequence_2_timer, g_wmap_sprite_actors[9],
                              g_wmap_actor_animations[9], g_wmap_focus_screen_position, 0x13, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_16_sequence_2_end, g_wmap_land_effect_16_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_16_run_sequence_3, D_800D6458, 0x4, g_wmap_land_effect_16_sequence_3_step, g_wmap_land_effect_16_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_16_sequence_3_reset, g_wmap_land_effect_16_sequence_3_step, g_wmap_land_effect_16_sequence_3_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_16_sequence_3_step_01,
    g_wmap_land_effect_16_sequence_3_step, g_wmap_land_effect_16_sequence_3_timer,
    5, g_wmap_animation_bank_0, 0,
    0, 0x80, 1,
    0x81, wmap_land_effect_16_sequence_3_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_16_sequence_3_step_02, g_wmap_land_effect_16_sequence_3_step, g_wmap_land_effect_16_sequence_3_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x18, 0x8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_16_sequence_3_end, g_wmap_land_effect_16_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_16_run_sequence_4, D_800D6468, 0x4, g_wmap_land_effect_16_sequence_4_step, g_wmap_land_effect_16_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_16_sequence_4_reset, g_wmap_land_effect_16_sequence_4_step, g_wmap_land_effect_16_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_16_sequence_4_step_01, g_wmap_land_effect_16_sequence_4_step, g_wmap_land_effect_16_sequence_4_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40, wmap_land_effect_16_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_16_sequence_4_end, g_wmap_land_effect_16_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_16_run_sequence_5, D_800D6478, 0x4, g_wmap_land_effect_16_sequence_5_step, g_wmap_land_effect_16_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_16_sequence_5_reset, g_wmap_land_effect_16_sequence_5_step, g_wmap_land_effect_16_sequence_5_timer)

WMAP_STEP_DROP_START(wmap_land_effect_16_sequence_5_step_01, g_wmap_land_effect_16_sequence_5_step, g_wmap_land_effect_16_sequence_5_timer, g_wmap_effect_model_b_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x40, wmap_land_effect_16_sequence_5_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_16_sequence_5_end, g_wmap_land_effect_16_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_16_run_sequence_6, D_800D6488, 0x6, g_wmap_land_effect_16_sequence_6_step, g_wmap_land_effect_16_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_16_sequence_6_reset, g_wmap_land_effect_16_sequence_6_step, g_wmap_land_effect_16_sequence_6_timer)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_16_sequence_6_step_02,
    g_wmap_land_effect_16_sequence_6_step, g_wmap_land_effect_16_sequence_6_timer,
    func_8006BC44(0x14, 0x3C, g_wmap_effect_params, 1))

/** @brief Reset four actor configurations and begin a 16-tick sequence step. */
void wmap_land_effect_16_sequence_6_step_03(void)
{
    s32 index;

    g_wmap_effect_params[0] = -1;
    g_wmap_effect_params[4] = 0;
    g_wmap_effect_params[5] = 0;
    for (index = 20; index < 80; index += 15)
    {
        g_wmap_sprite_actors[index].target_shade = 0;
        g_wmap_sprite_actors[index].shade_step = 8;
    }
    g_wmap_land_effect_16_sequence_6_timer = 0x10;
    g_wmap_land_effect_16_sequence_6_step += 1;
    wmap_land_effect_16_sequence_6_step_04();
}

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_16_sequence_6_step_04,
    g_wmap_land_effect_16_sequence_6_step, g_wmap_land_effect_16_sequence_6_timer,
    func_8006BC44(0x14, 0x3C, g_wmap_effect_params, 1))

WMAP_STEP_ADVANCE(wmap_land_effect_16_sequence_6_end, g_wmap_land_effect_16_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_16_run_sequence_7, D_800D64A0, 0x4, g_wmap_land_effect_16_sequence_7_step, g_wmap_land_effect_16_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_16_sequence_7_reset, g_wmap_land_effect_16_sequence_7_step, g_wmap_land_effect_16_sequence_7_timer)

/** @brief Start the sprite animation and run its first update. */
WMAP_STEP_START_ACTOR(wmap_land_effect_16_sequence_7_step_01,
    g_wmap_land_effect_16_sequence_7_step, g_wmap_land_effect_16_sequence_7_timer,
    6, g_wmap_animation_bank_0, 1,
    0x80, 0x80, 1,
    0x81, wmap_land_effect_16_sequence_7_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_16_sequence_7_step_02, g_wmap_land_effect_16_sequence_7_step, g_wmap_land_effect_16_sequence_7_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x18, 0x8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_16_sequence_7_end, g_wmap_land_effect_16_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_16_run_sequence_8, D_800D64B0, 0x6, g_wmap_land_effect_16_sequence_8_step, g_wmap_land_effect_16_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_16_sequence_8_reset, g_wmap_land_effect_16_sequence_8_step, g_wmap_land_effect_16_sequence_8_timer)

/** @brief Restore effect data, clear two flags, and begin a 100-tick sequence step. */
void wmap_land_effect_16_sequence_8_step_01(void)
{
    g_wmap_effect_model_c_rotation = g_wmap_zero_rotation;
    g_wmap_effect_fade_c = 0;
    D_8013923C = 0;
    g_wmap_land_effect_16_sequence_8_timer = 0x64;
    g_wmap_land_effect_16_sequence_8_step += 1;
    wmap_land_effect_16_sequence_8_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_16_sequence_8_step_03, g_wmap_land_effect_16_sequence_8_step, g_wmap_land_effect_16_sequence_8_timer, 0x40,
                    wmap_land_effect_16_sequence_8_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_16_sequence_8_end, g_wmap_land_effect_16_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_16_run_sequence_9, D_800D64C8, 0x4, g_wmap_land_effect_16_sequence_9_step, g_wmap_land_effect_16_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_16_sequence_9_reset, g_wmap_land_effect_16_sequence_9_step, g_wmap_land_effect_16_sequence_9_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_16_sequence_9_step_01,
    g_wmap_land_effect_16_sequence_9_step, g_wmap_land_effect_16_sequence_9_timer,
    10, g_wmap_animation_bank_2, 0,
    1, 0x81, 4,
    0x3C, wmap_land_effect_16_sequence_9_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_16_sequence_9_step_02, g_wmap_land_effect_16_sequence_9_step, g_wmap_land_effect_16_sequence_9_timer, g_wmap_sprite_actors[10],
                              g_wmap_actor_animations[10], g_wmap_focus_screen_position, 0x13, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_16_sequence_9_end, g_wmap_land_effect_16_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_16_run_sequence_10, D_800D64D8, 0x6, g_wmap_land_effect_16_sequence_10_step, g_wmap_land_effect_16_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_16_sequence_10_reset, g_wmap_land_effect_16_sequence_10_step, g_wmap_land_effect_16_sequence_10_timer)

/** @brief Initialize the actor and its screen coordinates, then advance the sequence. */
void wmap_land_effect_16_sequence_10_step_01(void)
{
    s16 screen_y;
    WmapSpriteActor* actor = &g_wmap_sprite_actors[11];

    g_wmap_actor_animations[11].data = g_wmap_animation_bank_2;
    actor->scale_index = 0xF;
    actor->resource_index = 0;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    actor->shade_step = 0x20;
    actor->target_shade = 0x81;
    actor->shade = 1;
    g_wmap_land_effect_16_sequence_10_timer = 0x80;
    D_80182D60.point.x = (u16) g_wmap_focus_screen_position.packed;
    screen_y = g_wmap_focus_screen_position.packed - 0x38;
    D_80182D60.point.y = screen_y;
    g_wmap_land_effect_16_sequence_10_step += 1;
    wmap_land_effect_16_sequence_10_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_16_sequence_10_step_02, g_wmap_land_effect_16_sequence_10_step, g_wmap_land_effect_16_sequence_10_timer,
                              g_wmap_sprite_actors[11], g_wmap_actor_animations[11], D_80182D60, 0x13, 0x2, 0)

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_16_sequence_10_step_03(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[11];

    actor->target_shade = 0;
    actor->shade_step = 2;
    g_wmap_land_effect_16_sequence_10_timer = 0x40;
    g_wmap_land_effect_16_sequence_10_step += 1;
    wmap_land_effect_16_sequence_10_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_16_sequence_10_step_04, g_wmap_land_effect_16_sequence_10_step, g_wmap_land_effect_16_sequence_10_timer,
                              g_wmap_sprite_actors[11], g_wmap_actor_animations[11], D_80182D60, 0x13, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_16_sequence_10_end, g_wmap_land_effect_16_sequence_10_step)

WMAP_STEP_RUNNER(wmap_land_effect_16_run_sequence_11, D_800D64F0, 0x6, g_wmap_land_effect_16_sequence_11_step, g_wmap_land_effect_16_sequence_11_timer)

WMAP_STEP_RESET(wmap_land_effect_16_sequence_11_reset, g_wmap_land_effect_16_sequence_11_step, g_wmap_land_effect_16_sequence_11_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_16_sequence_11_step_02,
    g_wmap_land_effect_16_sequence_11_step, g_wmap_land_effect_16_sequence_11_timer,
    func_8006AEE0(),
    func_8006A2FC(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)g_wmap_effect_params + 0x78)))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_16_sequence_11_step_03,
    g_wmap_land_effect_16_sequence_11_step, g_wmap_land_effect_16_sequence_11_timer,
    g_wmap_effect_params[35], 0x20, wmap_land_effect_16_sequence_11_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_land_effect_16_sequence_11_step_04,
    g_wmap_land_effect_16_sequence_11_step, g_wmap_land_effect_16_sequence_11_timer,
    func_8006AEE0(),
    func_8006A2FC(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)g_wmap_effect_params + 0x78)))

WMAP_STEP_ADVANCE(wmap_land_effect_16_sequence_11_end, g_wmap_land_effect_16_sequence_11_step)
