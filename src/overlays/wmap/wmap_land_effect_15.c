#include "wmap_model_render.h"
#include "wmap_main.h"
#include "wmap_land_effect_15.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_sprite_render.h"
#include "wmap_spark_effect.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "sdk/rand.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_step_sequence.h"
#include "wmap_cells.h"

void wmap_land_effect_15_sequence_7_step_02(void);
void wmap_land_effect_15_wait_idle_02(void);
void wmap_land_effect_15_step_03(void);
s32 wmap_land_effect_15_run_timeline(s32 arg0);
void wmap_land_effect_15_wait_idle_04(void);
void wmap_land_effect_15_end(void);
s32 wmap_land_effect_15_run_sequence_4(s32 arg0);
s32 wmap_land_effect_15_run_sequence_6(s32 arg0);
s32 wmap_land_effect_15_run_sequence_3(s32 arg0);
s32 wmap_land_effect_15_run_sequence_7(s32 arg0);
s32 wmap_land_effect_15_run_sequence_8(s32 arg0);
s32 wmap_land_effect_15_run_sequence_10(s32 arg0);
s32 wmap_land_effect_15_run_sequence_1(s32 arg0);
s32 wmap_land_effect_15_run_sequence_9(s32 arg0);
s32 wmap_land_effect_15_run_sequence_5(s32 arg0);
s32 wmap_land_effect_15_run_sequence_2(s32 arg0);
void wmap_land_effect_15_sequence_1_step_02(void);
void wmap_land_effect_15_sequence_1_step_04(void);
void wmap_land_effect_15_sequence_1_step_06(void);
void wmap_land_effect_15_sequence_2_step_02(void);
void wmap_land_effect_15_sequence_3_step_02(void);
void wmap_land_effect_15_sequence_3_step_04(void);
void wmap_land_effect_15_sequence_6_step_02(void);
void wmap_land_effect_15_sequence_6_step_04(void);
void wmap_land_effect_15_sequence_7_step_04(void);
void wmap_land_effect_15_sequence_8_step_02(void);
void wmap_land_effect_15_sequence_8_step_04(void);
void wmap_land_effect_15_sequence_9_step_02(void);
void wmap_land_effect_15_sequence_9_step_04(void);
void wmap_land_effect_15_sequence_10_step_02(void);
void wmap_land_effect_15_sequence_10_step_04(void);

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
extern u8 *g_wmap_effect_model_pack_1;
extern u8 *g_wmap_effect_model_pack_2;
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_80182DE4;
extern u8* g_wmap_effect_model_pack_3;
extern s32 g_wmap_land_effect_15_sequence_4_timer;
extern u8* D_8011CF2C;
extern s32 g_wmap_land_effect_15_sequence_5_timer;
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern s32 D_8013B264;
extern s32 D_8013B270;
extern s32 D_8013B278;
extern s32 D_8013B280;
extern s32 g_wmap_land_effect_15_sequence_7_timer;
extern WmapMotion D_801B0B70[];
extern s32 D_80139980;
extern s32 g_wmap_land_effect_15_timer;
extern void (*D_800D53B0[])(void);
extern void wmap_land_effect_15_step_03(void);
extern void wmap_land_effect_15_end(void);
extern s32 g_wmap_land_effect_15_timeline_timer;
extern void (*D_800D53C8[])(void);
extern u32 g_wmap_selected_artifact;
extern s32 g_wmap_land_effect_15_sequence_1_timer;
extern void (*D_800D5410[])(void);
extern s32 g_wmap_land_effect_15_sequence_2_timer;
extern void (*D_800D5430[])(void);
extern u8 g_wmap_animation_bank_1[];
extern s32 g_wmap_land_effect_15_sequence_3_timer;
extern void (*D_800D5440[])(void);
extern void wmap_land_effect_15_sequence_3_step_02(void);
extern void wmap_land_effect_15_sequence_3_step_04(void);
extern void (*D_800D5458[])(void);
extern void (*D_800D5468[])(void);
extern s32 g_wmap_land_effect_15_sequence_6_timer;
extern void (*D_800D5478[])(void);
extern void wmap_land_effect_15_sequence_6_step_02(void);
extern void wmap_land_effect_15_sequence_6_step_04(void);
extern void (*D_800D5490[])(void);
extern s32 g_wmap_land_effect_15_sequence_8_timer;
extern void (*D_800D54A8[])(void);
extern void wmap_land_effect_15_sequence_8_step_02(void);
extern void wmap_land_effect_15_sequence_8_step_04(void);
extern s32 g_wmap_land_effect_15_sequence_9_timer;
extern void (*D_800D54C0[])(void);
extern u8 g_wmap_animation_bank_2[];
extern s32 g_wmap_land_effect_15_sequence_10_timer;
extern void (*D_800D54D8[])(void);
extern void wmap_land_effect_15_sequence_10_step_04(void);
extern u32 g_wmap_land_effect_15_sequence_4_step;
extern u32 g_wmap_land_effect_15_sequence_5_step;
extern u32 g_wmap_land_effect_15_sequence_7_step;
extern u8 g_wmap_animation_bank_0[];
extern u32 g_wmap_land_effect_15_step;
extern u32 g_wmap_land_effect_15_timeline_step;
extern u32 g_wmap_land_effect_15_sequence_1_step;
extern u32 g_wmap_land_effect_15_sequence_2_step;
extern u32 g_wmap_land_effect_15_sequence_3_step;
extern u32 g_wmap_land_effect_15_sequence_6_step;
extern u32 g_wmap_land_effect_15_sequence_8_step;
extern u32 g_wmap_land_effect_15_sequence_9_step;
extern u32 g_wmap_land_effect_15_sequence_10_step;

extern VECTOR g_wmap_camera_translation;



extern WmapScreenPosition g_wmap_focus_screen_position;


/** @brief Draw nine map effect sprites with fixed positions and ordering depths. */


void func_8007B0D8(s32 color_mask)
{
    wmap_draw_model(g_wmap_load_buffer, 0, 0x24, 0x35, 0x7800, 1, D_8013923C | color_mask, 0x46, -5, D_80139234);
    wmap_draw_model(g_wmap_effect_model_pack_1, 0, 0x26, 0x35, 0x7800, 1, D_8013923C | color_mask, 0x64, -0x19, D_80139234);
    wmap_draw_model(g_wmap_effect_model_pack_2, 0, 0x22, 0x35, 0x7800, 1, D_8013923C | color_mask, 0x50, 0x1E, D_80139234);
    wmap_draw_model(g_wmap_load_buffer, 0, 0x20, 0x35, 0x7800, 1, D_8013923C | color_mask, 0x14, 0x28, D_80139234);
    wmap_draw_model(g_wmap_effect_model_pack_1, 0, 0x1F, 0x35, 0x7800, 1, D_8013923C | color_mask, 0x32, 0x41, D_80139234);
    wmap_draw_model(g_wmap_effect_model_pack_2, 0, 0x21, 0x35, 0x7800, 1, D_8013923C | color_mask, -0x28, 0x23, D_80139234);
    wmap_draw_model(g_wmap_load_buffer, 0, 0x25, 0x35, 0x7800, 1, D_8013923C | color_mask, -0x28, -0x14, D_80139234);
    wmap_draw_model(g_wmap_effect_model_pack_1, 0, 0x23, 0x35, 0x7800, 1, D_8013923C | color_mask, -0x44, 0xA, D_80139234);
    wmap_draw_model(g_wmap_effect_model_pack_2, 0, 0x27, 0x35, 0x7800, 1, D_8013923C | color_mask, -5, -0x32, D_80139234);
}

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_15_sequence_4_step_02,
    g_wmap_land_effect_15_sequence_4_step, g_wmap_land_effect_15_sequence_4_timer,
    g_wmap_effect_model_c_rotation, g_wmap_effect_model_c_position,
    D_80182DE4, g_wmap_effect_model_pack_3, -3500, 4)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_15_sequence_5_step_02,
    g_wmap_land_effect_15_sequence_5_step, g_wmap_land_effect_15_sequence_5_timer,
    g_wmap_effect_model_d_rotation, g_wmap_effect_model_d_position,
    g_wmap_effect_fade_a, D_8011CF2C, -3500, 4)

/** @brief Initialize eight effect actors with evenly spaced angles. */
void wmap_land_effect_15_sequence_7_step_01(void)
{
    s32 i;
    WmapSpriteActor *actor;
    g_wmap_particle_intensity = 8;
    g_wmap_effect_fade_b = 0x7F;
    D_80139240 = 0x50;
    D_8013924C = 2;
    D_80139250 = -1;
    D_80139260 = 0x44C;
    D_80139264 = 0xC8;
    D_80139268 = 0x15;
    D_8013926C = 2;
    D_80139284 = 0;
    D_8013B264 = 0x28;
    D_8013B270 = 0x50;
    D_8013B278 = 0xA;
    D_8013B280 = 0x50;
    for (i = 0; i < 8; i++)
    {
        actor = &g_wmap_sprite_actors[204 + i];
        g_wmap_actor_animations[i + 204].data = g_wmap_animation_bank_0;
        actor->scale_index = 15;
        actor->previous_sequence = -1;
        actor->shade = 255;
        actor->shade_step = 3;
        actor->resource_index = 0;
        actor->target_shade = 0;
        actor->sequence = D_8013926C;
        D_801B0B70[i].state = 1;
        D_801B0B70[i].z = D_80139284;
        D_801B0B70[i].scale = 80;
        D_801B0B70[i].angle = i << 9;
        D_801B0B70[i].x = 0;
        D_801B0B70[i].field_0E = 0;
    }
    g_wmap_land_effect_15_sequence_7_timer = 120;
    g_wmap_land_effect_15_sequence_7_step++;
    wmap_land_effect_15_sequence_7_step_02();
}

/** @brief Draw and respawn the radial sparks for land effect 15. */
WMAP_DEFINE_RADIAL_SPARK_UPDATE(wmap_land_effect_15_update_sparks,
    8,   /* Motion slots. */
    104,   /* First sprite actor. */
    13,   /* Texture index. */
    32,   /* Draw order. */
    32,   /* Minimum lifetime in frames. */
    D_80139980)

WMAP_STEP_RUNNER(wmap_land_effect_15_run, D_800D53B0, 0x6, g_wmap_land_effect_15_step, g_wmap_land_effect_15_timer)

WMAP_STEP_RESET(wmap_land_effect_15_reset, g_wmap_land_effect_15_step, g_wmap_land_effect_15_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_15_step_01, g_wmap_land_effect_15_step, wmap_run_land_focus, wmap_land_effect_15_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_15_wait_idle_02, g_wmap_land_effect_15_step, wmap_land_effect_15_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_15_step_03, g_wmap_land_effect_15_step, wmap_land_effect_15_run_timeline, wmap_land_effect_15_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_15_wait_idle_04, g_wmap_land_effect_15_step, wmap_land_effect_15_end)

WMAP_STEP_ADVANCE(wmap_land_effect_15_end, g_wmap_land_effect_15_step)

WMAP_STEP_RUNNER(wmap_land_effect_15_run_timeline, D_800D53C8, 0x12, g_wmap_land_effect_15_timeline_step, g_wmap_land_effect_15_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_15_timeline_reset, g_wmap_land_effect_15_timeline_step, g_wmap_land_effect_15_timeline_timer)

/** @brief World-map step handler: kick a sub-task and advance the step counter. */
void wmap_land_effect_15_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x503030);
    g_wmap_backdrop_target_level = 4;
    g_wmap_land_effect_15_timeline_timer = 0x10;
    g_wmap_land_effect_15_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_15_timeline_wait_02, g_wmap_land_effect_15_timeline_step, g_wmap_land_effect_15_timeline_timer)

/**
 * @brief Set up a world-map sub-scene: request assets and register its handler.
 */
void wmap_land_effect_15_timeline_step_03(void)
{
    g_wmap_transition_mesh_hidden = 1;
    wmap_play_sound(0x1B, 0x80);
    wmap_start_sequence(wmap_land_effect_15_run_sequence_4);
    g_wmap_land_effect_15_timeline_timer = 8;
    g_wmap_land_effect_15_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_15_timeline_wait_04, g_wmap_land_effect_15_timeline_step, g_wmap_land_effect_15_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_15_timeline_step_05, g_wmap_land_effect_15_timeline_step, g_wmap_land_effect_15_timeline_timer,
                             wmap_land_effect_15_run_sequence_6, wmap_land_effect_15_run_sequence_3, 0x4)

WMAP_STEP_WAIT(wmap_land_effect_15_timeline_wait_06, g_wmap_land_effect_15_timeline_step, g_wmap_land_effect_15_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_15_timeline_step_07, g_wmap_land_effect_15_timeline_step, g_wmap_land_effect_15_timeline_timer,
                             wmap_land_effect_15_run_sequence_7, wmap_land_effect_15_run_sequence_8, 0x4)

WMAP_STEP_WAIT(wmap_land_effect_15_timeline_wait_08, g_wmap_land_effect_15_timeline_step, g_wmap_land_effect_15_timeline_timer)

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void wmap_land_effect_15_timeline_step_09(void)
{
    g_wmap_placement_overlay_hidden = 1;
    g_wmap_land_effect_15_timeline_timer = 0x46;
    g_wmap_land_effect_15_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_15_timeline_wait_10, g_wmap_land_effect_15_timeline_step, g_wmap_land_effect_15_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_15_timeline_step_11, g_wmap_land_effect_15_timeline_step, g_wmap_land_effect_15_timeline_timer,
                         wmap_land_effect_15_run_sequence_10, 0xC)

WMAP_STEP_WAIT(wmap_land_effect_15_timeline_wait_12, g_wmap_land_effect_15_timeline_step, g_wmap_land_effect_15_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_15_timeline_step_13, g_wmap_land_effect_15_timeline_step, g_wmap_land_effect_15_timeline_timer,
                             wmap_land_effect_15_run_sequence_1, wmap_land_effect_15_run_sequence_9, 0x24)

WMAP_STEP_WAIT(wmap_land_effect_15_timeline_wait_14, g_wmap_land_effect_15_timeline_step, g_wmap_land_effect_15_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_effect_15_timeline_step_15, g_wmap_land_effect_15_timeline_step, g_wmap_land_effect_15_timeline_timer,
                             wmap_land_effect_15_run_sequence_5, wmap_land_effect_15_run_sequence_2, 0x59)

WMAP_STEP_WAIT(wmap_land_effect_15_timeline_wait_16, g_wmap_land_effect_15_timeline_step, g_wmap_land_effect_15_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_15_timeline_finish, g_wmap_land_effect_15_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y, g_wmap_selected_artifact)

WMAP_STEP_RUNNER(wmap_land_effect_15_run_sequence_1, D_800D5410, 0x8, g_wmap_land_effect_15_sequence_1_step, g_wmap_land_effect_15_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_15_sequence_1_reset, g_wmap_land_effect_15_sequence_1_step, g_wmap_land_effect_15_sequence_1_timer)

/** @brief Initialize effect values and an eight-tick countdown, then run its first update. */
void wmap_land_effect_15_sequence_1_step_01(void)
{
    D_80139234 = 0x10;
    D_8013923C = 0x80;
    g_wmap_land_effect_15_sequence_1_timer = 8;
    g_wmap_land_effect_15_sequence_1_step += 1;
    wmap_land_effect_15_sequence_1_step_02();
}

/** @brief Update the sequence effect and advance when its countdown reaches zero. */
void wmap_land_effect_15_sequence_1_step_02(void)
{
    s32 remaining_ticks;

    func_8007B0D8(0x10000);
    remaining_ticks = g_wmap_land_effect_15_sequence_1_timer - 1;
    g_wmap_land_effect_15_sequence_1_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_15_sequence_1_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_15_sequence_1_step_03, g_wmap_land_effect_15_sequence_1_step, g_wmap_land_effect_15_sequence_1_timer, 0x28,
                    wmap_land_effect_15_sequence_1_step_04)

/** @brief Reduce the effect value to a minimum of one and count down the sequence step. */
void wmap_land_effect_15_sequence_1_step_04(void)
{
    s32 value;
    s32 remaining_ticks;

    value = D_80139234 - 1;
    D_80139234 = value;
    if (value <= 0)
    {
        D_80139234 = 1;
    }
    func_8007B0D8(0x10000);
    remaining_ticks = g_wmap_land_effect_15_sequence_1_timer - 1;
    g_wmap_land_effect_15_sequence_1_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_15_sequence_1_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_15_sequence_1_step_05, g_wmap_land_effect_15_sequence_1_step, g_wmap_land_effect_15_sequence_1_timer, 0x8,
                    wmap_land_effect_15_sequence_1_step_06)

/** @brief Reduce the effect value toward zero and advance when its countdown expires. */
void wmap_land_effect_15_sequence_1_step_06(void)
{
    s32 value;
    s32 remaining_ticks;

    value = D_8013923C - 8;
    D_8013923C = value;
    if (value < 0)
    {
        D_8013923C = 0;
    }
    func_8007B0D8(1);
    remaining_ticks = g_wmap_land_effect_15_sequence_1_timer - 1;
    g_wmap_land_effect_15_sequence_1_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_land_effect_15_sequence_1_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_15_sequence_1_end, g_wmap_land_effect_15_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_15_run_sequence_2, D_800D5430, 0x4, g_wmap_land_effect_15_sequence_2_step, g_wmap_land_effect_15_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_15_sequence_2_reset, g_wmap_land_effect_15_sequence_2_step, g_wmap_land_effect_15_sequence_2_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_15_sequence_2_step_01,
    g_wmap_land_effect_15_sequence_2_step, g_wmap_land_effect_15_sequence_2_timer,
    4, g_wmap_animation_bank_1, 0,
    0x80, 0x80, 0,
    0x5A, wmap_land_effect_15_sequence_2_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_15_sequence_2_step_02, g_wmap_land_effect_15_sequence_2_step, g_wmap_land_effect_15_sequence_2_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0x16, 0x22, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_15_sequence_2_end, g_wmap_land_effect_15_sequence_2_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_15_run_sequence_3, D_800D5440, 0x6, g_wmap_land_effect_15_sequence_3_step,
                               g_wmap_land_effect_15_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_15_sequence_3_reset, g_wmap_land_effect_15_sequence_3_step, g_wmap_land_effect_15_sequence_3_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_15_sequence_3_step_01,
    g_wmap_land_effect_15_sequence_3_step, g_wmap_land_effect_15_sequence_3_timer,
    6, g_wmap_animation_bank_0, 0,
    0x80, 0x80, 2,
    0x56, wmap_land_effect_15_sequence_3_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_15_sequence_3_step_02, g_wmap_land_effect_15_sequence_3_step, g_wmap_land_effect_15_sequence_3_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x15, 0xB, 0)

/**
 * @brief Start the sprite fade and run its first update.
 */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_15_sequence_3_step_03,
    g_wmap_land_effect_15_sequence_3_step, g_wmap_land_effect_15_sequence_3_timer,
    g_wmap_sprite_actors[6], 8, 0x40, wmap_land_effect_15_sequence_3_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_15_sequence_3_step_04, g_wmap_land_effect_15_sequence_3_step, g_wmap_land_effect_15_sequence_3_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x15, 0xB, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_15_sequence_3_end, g_wmap_land_effect_15_sequence_3_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_15_run_sequence_4, D_800D5458, 0x4, g_wmap_land_effect_15_sequence_4_step,
                               g_wmap_land_effect_15_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_15_sequence_4_reset, g_wmap_land_effect_15_sequence_4_step, g_wmap_land_effect_15_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_15_sequence_4_step_01, g_wmap_land_effect_15_sequence_4_step, g_wmap_land_effect_15_sequence_4_timer, g_wmap_effect_model_c_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_c_position, D_80182DE4, 0x80, 0xAFC8, 0x20, wmap_land_effect_15_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_15_sequence_4_end, g_wmap_land_effect_15_sequence_4_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_15_run_sequence_5, D_800D5468, 0x4, g_wmap_land_effect_15_sequence_5_step,
                               g_wmap_land_effect_15_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_15_sequence_5_reset, g_wmap_land_effect_15_sequence_5_step, g_wmap_land_effect_15_sequence_5_timer)

WMAP_STEP_DROP_START(wmap_land_effect_15_sequence_5_step_01, g_wmap_land_effect_15_sequence_5_step, g_wmap_land_effect_15_sequence_5_timer, g_wmap_effect_model_d_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_d_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x20, wmap_land_effect_15_sequence_5_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_15_sequence_5_end, g_wmap_land_effect_15_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_15_run_sequence_6, D_800D5478, 0x6, g_wmap_land_effect_15_sequence_6_step, g_wmap_land_effect_15_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_15_sequence_6_reset, g_wmap_land_effect_15_sequence_6_step, g_wmap_land_effect_15_sequence_6_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_15_sequence_6_step_01,
    g_wmap_land_effect_15_sequence_6_step, g_wmap_land_effect_15_sequence_6_timer,
    8, g_wmap_animation_bank_0, 1,
    0x80, 0x80, 0,
    0x10, wmap_land_effect_15_sequence_6_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_15_sequence_6_step_02, g_wmap_land_effect_15_sequence_6_step, g_wmap_land_effect_15_sequence_6_timer, g_wmap_sprite_actors[8],
                              g_wmap_actor_animations[8], g_wmap_focus_screen_position, 0x15, 0x5, 0)

/**
 * @brief Start the sprite fade and run its first update.
 */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_15_sequence_6_step_03,
    g_wmap_land_effect_15_sequence_6_step, g_wmap_land_effect_15_sequence_6_timer,
    g_wmap_sprite_actors[8], 8, 0x10, wmap_land_effect_15_sequence_6_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_15_sequence_6_step_04, g_wmap_land_effect_15_sequence_6_step, g_wmap_land_effect_15_sequence_6_timer, g_wmap_sprite_actors[8],
                              g_wmap_actor_animations[8], g_wmap_focus_screen_position, 0x15, 0x5, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_15_sequence_6_end, g_wmap_land_effect_15_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_15_run_sequence_7, D_800D5490, 0x6, g_wmap_land_effect_15_sequence_7_step, g_wmap_land_effect_15_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_15_sequence_7_reset, g_wmap_land_effect_15_sequence_7_step, g_wmap_land_effect_15_sequence_7_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_15_sequence_7_step_02,
    g_wmap_land_effect_15_sequence_7_step, g_wmap_land_effect_15_sequence_7_timer,
    func_8006D014(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 8, 1, g_wmap_effect_fade_b, 8, 2))

WMAP_STEP_ARM_TIMER(wmap_land_effect_15_sequence_7_step_03, g_wmap_land_effect_15_sequence_7_step, g_wmap_land_effect_15_sequence_7_timer, 0x14,
                    wmap_land_effect_15_sequence_7_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_15_sequence_7_step_04,
    g_wmap_land_effect_15_sequence_7_step, g_wmap_land_effect_15_sequence_7_timer,
    func_8006D014(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 8, 1, g_wmap_effect_fade_b, 8, 2))

WMAP_STEP_ADVANCE(wmap_land_effect_15_sequence_7_end, g_wmap_land_effect_15_sequence_7_step)

WMAP_STEP_RUNNER_RESET_AND_RUN(wmap_land_effect_15_run_sequence_8, D_800D54A8, 0x6, g_wmap_land_effect_15_sequence_8_step,
                               g_wmap_land_effect_15_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_15_sequence_8_reset, g_wmap_land_effect_15_sequence_8_step, g_wmap_land_effect_15_sequence_8_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_15_sequence_8_step_01,
    g_wmap_land_effect_15_sequence_8_step, g_wmap_land_effect_15_sequence_8_timer,
    9, g_wmap_animation_bank_0, 3,
    0x80, 0x80, 0,
    0x50, wmap_land_effect_15_sequence_8_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_15_sequence_8_step_02, g_wmap_land_effect_15_sequence_8_step, g_wmap_land_effect_15_sequence_8_timer, g_wmap_sprite_actors[9],
                              g_wmap_actor_animations[9], g_wmap_focus_screen_position, 0x15, 0xB, 0)

/**
 * @brief Start the sprite fade and run its first update.
 */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_15_sequence_8_step_03,
    g_wmap_land_effect_15_sequence_8_step, g_wmap_land_effect_15_sequence_8_timer,
    g_wmap_sprite_actors[9], 8, 0x10, wmap_land_effect_15_sequence_8_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_15_sequence_8_step_04, g_wmap_land_effect_15_sequence_8_step, g_wmap_land_effect_15_sequence_8_timer, g_wmap_sprite_actors[9],
                              g_wmap_actor_animations[9], g_wmap_focus_screen_position, 0x15, 0xB, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_15_sequence_8_end, g_wmap_land_effect_15_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_15_run_sequence_9, D_800D54C0, 0x6, g_wmap_land_effect_15_sequence_9_step, g_wmap_land_effect_15_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_15_sequence_9_reset, g_wmap_land_effect_15_sequence_9_step, g_wmap_land_effect_15_sequence_9_timer)

/**
 * @brief World-map step handler: seed an 8-entry table and advance the step.
 */
void wmap_land_effect_15_sequence_9_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 0;
    D_80139980 = 0x7F;
    for (i = 0; i < 8; i++)
    {
        g_wmap_actor_motions[i].active = 0;
        g_wmap_actor_animations[i + 104].data = g_wmap_animation_bank_2;
    }
    g_wmap_land_effect_15_sequence_9_timer = 0x20;
    g_wmap_land_effect_15_sequence_9_step += 1;
    wmap_land_effect_15_sequence_9_step_02();
}

/** @brief World-map step tick: bump a counter, run the sub-step, and expire the timer. */
void wmap_land_effect_15_sequence_9_step_02(void)
{
    if (g_wmap_frame_count & 1)
    {
        g_wmap_particle_intensity += 1;
    }
    wmap_land_effect_15_update_sparks();
    if (--g_wmap_land_effect_15_sequence_9_timer == 0)
    {
        g_wmap_land_effect_15_sequence_9_step += 1;
    }
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_15_sequence_9_step_03, g_wmap_land_effect_15_sequence_9_step, g_wmap_land_effect_15_sequence_9_timer, 0x40,
                    wmap_land_effect_15_sequence_9_step_04)

/**
 * @brief World-map step tick: age two timers with zero clamps, run the sub-step,
 *        and expire the step counter.
 */
void wmap_land_effect_15_sequence_9_step_04(void)
{
    D_80139980 -= 1;
    if (D_80139980 < 0)
    {
        D_80139980 = 0;
    }
    if ((g_wmap_frame_count & 3) == 0)
    {
        g_wmap_particle_intensity -= 1;
    }
    if (g_wmap_particle_intensity < 0)
    {
        g_wmap_particle_intensity = 0;
    }
    wmap_land_effect_15_update_sparks();
    if (--g_wmap_land_effect_15_sequence_9_timer == 0)
    {
        g_wmap_land_effect_15_sequence_9_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_15_sequence_9_end, g_wmap_land_effect_15_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_15_run_sequence_10, D_800D54D8, 0x6, g_wmap_land_effect_15_sequence_10_step, g_wmap_land_effect_15_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_15_sequence_10_reset, g_wmap_land_effect_15_sequence_10_step, g_wmap_land_effect_15_sequence_10_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_15_sequence_10_step_01,
    g_wmap_land_effect_15_sequence_10_step, g_wmap_land_effect_15_sequence_10_timer,
    54, g_wmap_animation_bank_0, 4,
    1, 0x81, 8,
    0x40, wmap_land_effect_15_sequence_10_step_02)

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_15_sequence_10_step_02(void)
{
    wmap_step_actor_animation(&g_wmap_sprite_actors[54], &g_wmap_actor_animations[54]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[54], g_wmap_focus_screen_position.packed, 0x15, 0x28, 0);
    if (--g_wmap_land_effect_15_sequence_10_timer == 0)
    {
        g_wmap_land_effect_15_sequence_10_step += 1;
    }
}

/**
 * @brief Start the sprite fade and run its first update.
 */
WMAP_STEP_FADE_ACTOR(wmap_land_effect_15_sequence_10_step_03,
    g_wmap_land_effect_15_sequence_10_step, g_wmap_land_effect_15_sequence_10_timer,
    g_wmap_sprite_actors[54], 8, 0x10, wmap_land_effect_15_sequence_10_step_04)

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_effect_15_sequence_10_step_04(void)
{
    wmap_step_actor_animation(&g_wmap_sprite_actors[54], &g_wmap_actor_animations[54]);
    wmap_draw_actor_sprite(&g_wmap_sprite_actors[54], g_wmap_focus_screen_position.packed, 0x15, 0x28, 0);
    if (--g_wmap_land_effect_15_sequence_10_timer == 0)
    {
        g_wmap_land_effect_15_sequence_10_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_15_sequence_10_end, g_wmap_land_effect_15_sequence_10_step)
