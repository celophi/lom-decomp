#include "../internal/wmap_main.h"
#include "../internal/wmap_land_effect_09.h"
#include "../internal/wmap_sequence_runtime.h"
#include <libgte.h>
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_view_effects.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_cells.h"

void wmap_land_effect_09_sequence_5_step_02(void);
void wmap_land_effect_09_sequence_6_step_02(void);
void wmap_land_effect_09_sequence_7_step_02(void);
void wmap_land_effect_09_sequence_8_step_02(void);
void wmap_land_effect_09_sequence_9_step_02(void);
void wmap_land_effect_09_wait_idle_02(void);
void wmap_land_effect_09_step_03(void);
s32 wmap_land_effect_09_run_timeline(s32 arg0);
void wmap_land_effect_09_wait_idle_04(void);
void wmap_land_effect_09_end(void);
s32 wmap_land_effect_09_run_sequence_7(s32 arg0);
s32 wmap_land_effect_09_run_sequence_1(s32 arg0);
s32 wmap_land_effect_09_run_sequence_3(s32 arg0);
s32 wmap_land_effect_09_run_sequence_5(s32 arg0);
s32 wmap_land_effect_09_run_sequence_8(s32 arg0);
s32 wmap_land_effect_09_run_sequence_6(s32 arg0);
s32 wmap_land_effect_09_run_sequence_9(s32 arg0);
s32 wmap_land_effect_09_run_sequence_4(s32 arg0);
s32 wmap_land_effect_09_run_sequence_2(s32 arg0);
void wmap_land_effect_09_sequence_1_step_02(void);
void wmap_land_effect_09_sequence_2_step_02(void);
void wmap_land_effect_09_sequence_5_step_04(void);
void wmap_land_effect_09_sequence_6_step_04(void);
void wmap_land_effect_09_sequence_6_step_06(void);
void wmap_land_effect_09_sequence_7_step_04(void);
void wmap_land_effect_09_sequence_8_step_04(void);
void wmap_land_effect_09_sequence_9_step_04(void);

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 field_00;
    s16 angle;
    s32 field_04;
    s32 field_08;
    s16 field_0C;
    s16 field_0E;
    s16 field_10;
    s16 pad_12;
} WmapMotion;

extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_09_sequence_3_timer;
extern u8* g_wmap_effect_model_pack_1;
extern s32 g_wmap_land_effect_09_sequence_4_timer;
extern s32 g_wmap_land_effect_09_sequence_5_timer;
extern s32 D_800D9150;
extern s32 D_800DCEA8;
extern s32 g_wmap_land_effect_09_sequence_6_timer;
extern u8 g_wmap_animation_bank_3[];
extern s32 g_wmap_land_effect_09_sequence_7_timer;
extern s32 g_wmap_land_effect_09_sequence_8_timer;
extern s32 g_wmap_land_effect_09_sequence_9_timer;
extern s32 g_wmap_land_effect_09_timer;
extern void (*D_800D62A0[])(void);
extern void wmap_land_effect_09_step_03(void);
extern void wmap_land_effect_09_end(void);
extern s32 g_wmap_land_effect_09_timeline_timer;
extern void (*D_800D62B8[])(void);
extern u32 g_wmap_selected_artifact;
extern s32 g_wmap_land_effect_09_sequence_1_timer;
extern void (*D_800D62F8[])(void);
extern u8 g_wmap_animation_bank_1[];
extern s32 g_wmap_land_effect_09_sequence_2_timer;
extern void (*D_800D6308[])(void);
extern u8 g_wmap_animation_bank_0[];
extern void wmap_land_effect_09_sequence_2_step_02(void);
extern void (*D_800D6318[])(void);
extern void (*D_800D6328[])(void);
extern void (*D_800D6338[])(void);
extern void (*D_800D6350[])(void);
extern void (*D_800D6370[])(void);
extern void (*D_800D6388[])(void);
extern void wmap_land_effect_09_sequence_8_step_04(void);
extern void (*D_800D63A0[])(void);
extern u32 g_wmap_land_effect_09_sequence_3_step;
extern u32 g_wmap_land_effect_09_sequence_4_step;
extern u32 g_wmap_land_effect_09_sequence_5_step;
extern u8 g_wmap_animation_bank_2[];
extern u32 g_wmap_land_effect_09_sequence_6_step;
extern u32 g_wmap_land_effect_09_sequence_7_step;
extern u32 g_wmap_land_effect_09_sequence_8_step;
extern u32 g_wmap_land_effect_09_sequence_9_step;
extern u32 g_wmap_land_effect_09_step;
extern u32 g_wmap_land_effect_09_timeline_step;
extern u32 g_wmap_land_effect_09_sequence_1_step;
extern u32 g_wmap_land_effect_09_sequence_2_step;

extern VECTOR g_wmap_camera_translation;



extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* g_wmap_effect_params;

extern WmapMotion g_wmap_actor_motions[];

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_09_sequence_3_step_02,
    g_wmap_land_effect_09_sequence_3_step, g_wmap_land_effect_09_sequence_3_timer,
    g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position,
    g_wmap_effect_fade_a, g_wmap_load_buffer, -3500, 2)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_09_sequence_4_step_02,
    g_wmap_land_effect_09_sequence_4_step, g_wmap_land_effect_09_sequence_4_timer,
    g_wmap_effect_model_b_rotation, g_wmap_effect_model_b_position,
    g_wmap_effect_fade_b, g_wmap_effect_model_pack_1, -3500, 2)

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_09_sequence_5_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 50;
    g_wmap_effect_params[0x1] = 0;
    g_wmap_effect_params[0x2] = 0;
    g_wmap_effect_params[0x3] = 255;
    g_wmap_effect_params[0x4] = 0;
    g_wmap_effect_params[0x5] = 2;
    g_wmap_effect_params[0x6] = 1000;
    g_wmap_effect_params[0x7] = 200;
    g_wmap_effect_params[0x8] = 8;
    g_wmap_effect_params[0x9] = 0;
    g_wmap_effect_params[0xA] = 18200;
    WMAP_RESET_PARTICLE_SLOTS(i, 50,
                              g_wmap_actor_motions[i + g_wmap_effect_params[0x7]].field_00,
                              204, g_wmap_animation_bank_2);
    g_wmap_land_effect_09_sequence_5_timer = 100;
    g_wmap_land_effect_09_sequence_5_step++;
    wmap_land_effect_09_sequence_5_step_02();
}

/**
 * @brief Initialize a range of world-map per-entry records and schedule the next step.
 */
void wmap_land_effect_09_sequence_6_step_01(void)
{
    s32 i;

    i = 150;
    g_wmap_effect_fade_c = 1;
    D_800DCEA8 = 1;

    do
    {
        g_wmap_actor_motions[i].field_00 = 0;
        WMAP_INIT_PARTICLE_ACTOR(i, g_wmap_animation_bank_2, 1);
        i++;
    } while (i < 158);

    D_800D9150 = 3;
    g_wmap_land_effect_09_sequence_6_timer = 0x10;
    g_wmap_land_effect_09_sequence_6_step += 1;
    wmap_land_effect_09_sequence_6_step_02();
}

/** @brief Initialize the actor group and its animation resources, then advance. */
void wmap_land_effect_09_sequence_7_step_01(void)
{
    s32 i;
    WmapSpriteActor* config;

    g_wmap_particle_intensity = 4;
    for (i = 140; i < 144; i++)
    {
        config = &g_wmap_sprite_actors[i];
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_3;
        config->resource_index = 0;
        config->scale_index = 15;
        config->sequence = 1;
        config->previous_sequence = -1;
        config->target_shade = 129;
        config->shade = 129;
        config->shade_step = 8;
        g_wmap_actor_motions[i].field_00 = 1;
        g_wmap_actor_motions[i].angle = i << 10;
        g_wmap_actor_motions[i].field_04 = -20000;
        g_wmap_actor_motions[i].field_08 = 990000;
        g_wmap_actor_motions[i].field_0C = 9999;
        g_wmap_actor_motions[i].field_0E = 240;
        g_wmap_actor_motions[i].field_10 = 160;
    }
    g_wmap_land_effect_09_sequence_7_timer = 28;
    g_wmap_land_effect_09_sequence_7_step++;
    wmap_land_effect_09_sequence_7_step_02();
}

/** @brief World-map step: fill a spawn descriptor, clear its slot run, then advance. */
void wmap_land_effect_09_sequence_8_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 12;
    g_wmap_effect_params[0x1F] = 1;
    g_wmap_effect_params[0x20] = 3;
    g_wmap_effect_params[0x21] = 64;
    g_wmap_effect_params[0x22] = 30;
    g_wmap_effect_params[0x23] = -2;
    g_wmap_effect_params[0x24] = 1000;
    g_wmap_effect_params[0x25] = 20;
    g_wmap_effect_params[0x26] = 15;
    g_wmap_effect_params[0x27] = 0;
    g_wmap_effect_params[0x28] = 10000;
    WMAP_RESET_PARTICLE_SLOTS(i, 12,
                              g_wmap_actor_motions[i + g_wmap_effect_params[0x25]].field_00,
                              24, g_wmap_animation_bank_3);
    g_wmap_land_effect_09_sequence_8_timer = 64;
    g_wmap_land_effect_09_sequence_8_step++;
    wmap_land_effect_09_sequence_8_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_09_sequence_9_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 20;
    g_wmap_effect_params[0xB] = 0;
    g_wmap_effect_params[0xC] = 0;
    g_wmap_effect_params[0xF] = 1;
    g_wmap_effect_params[0x10] = 120;
    g_wmap_effect_params[0x11] = 100;
    g_wmap_effect_params[0x12] = 15;
    g_wmap_effect_params[0x13] = 2;
    g_wmap_effect_params[0x14] = 18500;
    WMAP_RESET_PARTICLE_SLOTS(i, 20,
                              g_wmap_actor_motions[i + g_wmap_effect_params[0x11]].field_00,
                              104, g_wmap_animation_bank_3);
    g_wmap_land_effect_09_sequence_9_timer = 20;
    g_wmap_land_effect_09_sequence_9_step++;
    wmap_land_effect_09_sequence_9_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_09_run, D_800D62A0, 0x6, g_wmap_land_effect_09_step, g_wmap_land_effect_09_timer)

WMAP_STEP_RESET(wmap_land_effect_09_reset, g_wmap_land_effect_09_step, g_wmap_land_effect_09_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_09_step_01, g_wmap_land_effect_09_step, wmap_run_land_focus, wmap_land_effect_09_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_09_wait_idle_02, g_wmap_land_effect_09_step, wmap_land_effect_09_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_09_step_03, g_wmap_land_effect_09_step, wmap_land_effect_09_run_timeline, wmap_land_effect_09_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_09_wait_idle_04, g_wmap_land_effect_09_step, wmap_land_effect_09_end)

WMAP_STEP_ADVANCE(wmap_land_effect_09_end, g_wmap_land_effect_09_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_timeline, D_800D62B8, 0x10, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_09_timeline_reset, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

/**
 * @brief Set up a world-map sub-scene: request assets and register its handler.
 */
void wmap_land_effect_09_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_play_sound(0x30, 0x80);
    wmap_start_sequence(wmap_land_effect_09_run_sequence_7);
    g_wmap_land_effect_09_timeline_timer = 0x28;
    g_wmap_land_effect_09_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_02, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

/** @brief Register four callbacks, set effect color and flags, and begin a one-tick delay. */
void wmap_land_effect_09_timeline_step_03(void)
{
    wmap_start_sequence(&wmap_land_effect_09_run_sequence_3);
    wmap_start_sequence(&wmap_land_effect_09_run_sequence_5);
    wmap_start_sequence(&wmap_land_effect_09_run_sequence_8);
    g_wmap_transition_mesh_hidden = 1;
    wmap_start_sequence(&wmap_land_effect_09_run_sequence_1);
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 4;
    g_wmap_land_effect_09_timeline_timer = 1;
    g_wmap_land_effect_09_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_04, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

/**
 * @brief Hide the placement overlay and set the wait timer.
 */
WMAP_STEP_HIDE_AND_WAIT(wmap_land_effect_09_timeline_step_05,
    g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer,
    g_wmap_placement_overlay_hidden, 0x23)

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_06, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_09_timeline_step_07, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer,
                         wmap_land_effect_09_run_sequence_6, 0x8)

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_08, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_09_timeline_step_09, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer,
                         wmap_land_effect_09_run_sequence_9, 0x32)

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_10, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

/** @brief Register a callback, set world-map state and color, and begin a two-tick delay. */
void wmap_land_effect_09_timeline_step_11(void)
{
    wmap_start_sequence(&wmap_land_effect_09_run_sequence_4);
    g_wmap_transition_mesh_hidden = 0;
    g_wmap_backdrop_target_level = 0x10;
    wmap_start_map_tint(0x808080);
    g_wmap_land_effect_09_timeline_timer = 2;
    g_wmap_land_effect_09_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_12, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_09_timeline_step_13, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer,
                         wmap_land_effect_09_run_sequence_2, 0xAB)

WMAP_STEP_WAIT(wmap_land_effect_09_timeline_wait_14, g_wmap_land_effect_09_timeline_step, g_wmap_land_effect_09_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_09_timeline_finish, g_wmap_land_effect_09_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y, g_wmap_selected_artifact)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_1, D_800D62F8, 0x4, g_wmap_land_effect_09_sequence_1_step, g_wmap_land_effect_09_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_1_reset, g_wmap_land_effect_09_sequence_1_step, g_wmap_land_effect_09_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_09_sequence_1_step_01,
    g_wmap_land_effect_09_sequence_1_step, g_wmap_land_effect_09_sequence_1_timer,
    4, g_wmap_animation_bank_1, 0,
    0x81, 0x81, 0,
    0x4C, wmap_land_effect_09_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_09_sequence_1_step_02, g_wmap_land_effect_09_sequence_1_step, g_wmap_land_effect_09_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0xF, 0x4, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_1_end, g_wmap_land_effect_09_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_2, D_800D6308, 0x4, g_wmap_land_effect_09_sequence_2_step, g_wmap_land_effect_09_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_2_reset, g_wmap_land_effect_09_sequence_2_step, g_wmap_land_effect_09_sequence_2_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_09_sequence_2_step_01,
    g_wmap_land_effect_09_sequence_2_step, g_wmap_land_effect_09_sequence_2_timer,
    5, g_wmap_animation_bank_0, 0,
    0, 0x80, 8,
    0xAC, wmap_land_effect_09_sequence_2_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_09_sequence_2_step_02, g_wmap_land_effect_09_sequence_2_step, g_wmap_land_effect_09_sequence_2_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x17, 0x8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_2_end, g_wmap_land_effect_09_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_3, D_800D6318, 0x4, g_wmap_land_effect_09_sequence_3_step, g_wmap_land_effect_09_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_3_reset, g_wmap_land_effect_09_sequence_3_step, g_wmap_land_effect_09_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_09_sequence_3_step_01, g_wmap_land_effect_09_sequence_3_step, g_wmap_land_effect_09_sequence_3_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40, wmap_land_effect_09_sequence_3_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_3_end, g_wmap_land_effect_09_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_4, D_800D6328, 0x4, g_wmap_land_effect_09_sequence_4_step, g_wmap_land_effect_09_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_4_reset, g_wmap_land_effect_09_sequence_4_step, g_wmap_land_effect_09_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_09_sequence_4_step_01, g_wmap_land_effect_09_sequence_4_step, g_wmap_land_effect_09_sequence_4_timer, g_wmap_effect_model_b_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x40, wmap_land_effect_09_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_4_end, g_wmap_land_effect_09_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_5, D_800D6338, 0x6, g_wmap_land_effect_09_sequence_5_step, g_wmap_land_effect_09_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_5_reset, g_wmap_land_effect_09_sequence_5_step, g_wmap_land_effect_09_sequence_5_timer)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_5_step_02,
    g_wmap_land_effect_09_sequence_5_step, g_wmap_land_effect_09_sequence_5_timer,
    func_8006A2FC(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 0x32, 0x7F, 1, 4, 5, g_wmap_effect_params))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_09_sequence_5_step_03,
    g_wmap_land_effect_09_sequence_5_step, g_wmap_land_effect_09_sequence_5_timer,
    g_wmap_effect_params[5], 0x40, wmap_land_effect_09_sequence_5_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_5_step_04,
    g_wmap_land_effect_09_sequence_5_step, g_wmap_land_effect_09_sequence_5_timer,
    func_8006A2FC(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 0x32, 0x7F, 1, 4, 5, g_wmap_effect_params))

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_5_end, g_wmap_land_effect_09_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_6, D_800D6350, 0x8, g_wmap_land_effect_09_sequence_6_step, g_wmap_land_effect_09_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_6_reset, g_wmap_land_effect_09_sequence_6_step, g_wmap_land_effect_09_sequence_6_timer)

/**
 * @brief Update the particles and increase the shared fade value until the timer expires.
 */
WMAP_STEP_UPDATE_AND_RAMP(wmap_land_effect_09_sequence_6_step_02, g_wmap_land_effect_09_sequence_6_step, g_wmap_land_effect_09_sequence_6_timer,
                          g_wmap_effect_fade_c, 8,
                          func_8006B328(0x96, 0x9E, 3, -1, -1, -5, 0, 8, -0x50, 0xA0, -0x50, 0xA0, 1, 0x7F, 1, 4, 0))

WMAP_STEP_ARM_TIMER(wmap_land_effect_09_sequence_6_step_03, g_wmap_land_effect_09_sequence_6_step, g_wmap_land_effect_09_sequence_6_timer, 0x18,
                    wmap_land_effect_09_sequence_6_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_6_step_04,
    g_wmap_land_effect_09_sequence_6_step, g_wmap_land_effect_09_sequence_6_timer,
    func_8006B328(0x96, 0x9E, 3, -1, -1, -5, 0, 8, -0x50, 0xA0, -0x50, 0xA0, 1, 0x7F, 1, 4, 0))

/**
 * @brief Stop spawning particles and keep updating those already active.
 */
WMAP_STEP_STOP_PARTICLE_SPAWNS(wmap_land_effect_09_sequence_6_step_05, g_wmap_land_effect_09_sequence_6_step, g_wmap_land_effect_09_sequence_6_timer,
                              D_800DCEA8, 0x14, wmap_land_effect_09_sequence_6_step_06)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_6_step_06,
    g_wmap_land_effect_09_sequence_6_step, g_wmap_land_effect_09_sequence_6_timer,
    func_8006B328(0x96, 0x9E, 3, -1, -1, -5, 0, 8, -0x50, 0xA0, -0x50, 0xA0, 1, 0x7F, 1, 4, 0))

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_6_end, g_wmap_land_effect_09_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_7, D_800D6370, 0x6, g_wmap_land_effect_09_sequence_7_step, g_wmap_land_effect_09_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_7_reset, g_wmap_land_effect_09_sequence_7_step, g_wmap_land_effect_09_sequence_7_timer)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_7_step_02,
    g_wmap_land_effect_09_sequence_7_step, g_wmap_land_effect_09_sequence_7_timer,
    func_8006B6EC(0x8C, 0x90, 0xF, -6, 4))

/**
 * @brief Start fading the sprite range and run its first update.
 */
WMAP_STEP_FADE_ACTOR_RANGE(wmap_land_effect_09_sequence_7_step_03,
    g_wmap_land_effect_09_sequence_7_step, g_wmap_land_effect_09_sequence_7_timer,
    0x8C, 0x90, 8, 0x10, wmap_land_effect_09_sequence_7_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_7_step_04,
    g_wmap_land_effect_09_sequence_7_step, g_wmap_land_effect_09_sequence_7_timer,
    func_8006B6EC(0x8C, 0x90, 0xF, -6, 4))

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_7_end, g_wmap_land_effect_09_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_8, D_800D6388, 0x6, g_wmap_land_effect_09_sequence_8_step, g_wmap_land_effect_09_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_8_reset, g_wmap_land_effect_09_sequence_8_step, g_wmap_land_effect_09_sequence_8_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_8_step_02,
    g_wmap_land_effect_09_sequence_8_step, g_wmap_land_effect_09_sequence_8_timer,
    func_8006A2FC(&g_wmap_sprite_actors[24], &g_wmap_actor_animations[24], 0xC, 0, 0x7F, 0x2, 0, &g_wmap_effect_params[30]))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_09_sequence_8_step_03,
    g_wmap_land_effect_09_sequence_8_step, g_wmap_land_effect_09_sequence_8_timer,
    g_wmap_effect_params[35], 0x20, wmap_land_effect_09_sequence_8_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_8_step_04,
    g_wmap_land_effect_09_sequence_8_step, g_wmap_land_effect_09_sequence_8_timer,
    func_8006A2FC(&g_wmap_sprite_actors[24], &g_wmap_actor_animations[24], 0xC, 0, 0x7F, 0x2, 0, &g_wmap_effect_params[30]))

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_8_end, g_wmap_land_effect_09_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_09_run_sequence_9, D_800D63A0, 0x6, g_wmap_land_effect_09_sequence_9_step, g_wmap_land_effect_09_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_09_sequence_9_reset, g_wmap_land_effect_09_sequence_9_step, g_wmap_land_effect_09_sequence_9_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_9_step_02,
    g_wmap_land_effect_09_sequence_9_step, g_wmap_land_effect_09_sequence_9_timer,
    func_8006A2FC(&g_wmap_sprite_actors[104], &g_wmap_actor_animations[104], 0x14, 0x7F, 0x7F, 0x4, 0, &g_wmap_effect_params[10]))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_09_sequence_9_step_03,
    g_wmap_land_effect_09_sequence_9_step, g_wmap_land_effect_09_sequence_9_timer,
    g_wmap_effect_params[15], 0x20, wmap_land_effect_09_sequence_9_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_09_sequence_9_step_04,
    g_wmap_land_effect_09_sequence_9_step, g_wmap_land_effect_09_sequence_9_timer,
    func_8006A2FC(&g_wmap_sprite_actors[104], &g_wmap_actor_animations[104], 0x14, 0x7F, 0x7F, 0x4, 0, &g_wmap_effect_params[10]))

WMAP_STEP_ADVANCE(wmap_land_effect_09_sequence_9_end, g_wmap_land_effect_09_sequence_9_step)
