#include "../internal/wmap_main.h"
#include "../internal/wmap_land_effect_08.h"
#include "../internal/wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "../internal/wmap_sprite_render.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "../internal/wmap_view_effects.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_star_effect.h"
#include "../internal/wmap_cells.h"

void wmap_land_effect_08_sequence_6_step_02(void);
void wmap_land_effect_08_sequence_8_step_02(void);
void wmap_land_effect_08_sequence_9_step_02(void);
void wmap_land_effect_08_wait_idle_02(void);
void wmap_land_effect_08_step_03(void);
s32 wmap_land_effect_08_run_timeline(s32 arg0);
void wmap_land_effect_08_wait_idle_04(void);
void wmap_land_effect_08_end(void);
s32 wmap_land_effect_08_run_sequence_7(s32 arg0);
s32 wmap_land_effect_08_run_sequence_8(s32 arg0);
s32 wmap_land_effect_08_run_sequence_3(s32 arg0);
s32 wmap_land_effect_08_run_sequence_1(s32 arg0);
s32 wmap_land_effect_08_run_sequence_5(s32 arg0);
s32 wmap_land_effect_08_run_sequence_6(s32 arg0);
s32 wmap_land_effect_08_run_sequence_9(s32 arg0);
s32 wmap_land_effect_08_run_sequence_4(s32 arg0);
s32 wmap_land_effect_08_run_sequence_2(s32 arg0);
s32 wmap_land_effect_08_run_sequence_10(s32 arg0);
void wmap_land_effect_08_sequence_1_step_02(void);
void wmap_land_effect_08_sequence_1_step_04(void);
void wmap_land_effect_08_sequence_2_step_02(void);
void wmap_land_effect_08_sequence_6_step_04(void);
void wmap_land_effect_08_sequence_6_step_06(void);
void wmap_land_effect_08_sequence_7_step_02(void);
void wmap_land_effect_08_sequence_7_step_04(void);
void wmap_land_effect_08_sequence_9_step_04(void);
void wmap_land_effect_08_sequence_10_step_02(void);

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
extern s32 g_wmap_land_effect_08_sequence_3_timer;
extern u8* g_wmap_effect_model_pack_1;
extern s32 g_wmap_land_effect_08_sequence_4_timer;
extern u8* g_wmap_effect_model_pack_2;
extern s32 D_801B2468;
extern s32 g_wmap_land_effect_08_sequence_5_timer;
extern s32 D_800DCEA8;
extern s32 D_800D9150;
extern s32 g_wmap_land_effect_08_sequence_6_timer;
extern u8 g_wmap_animation_bank_0[];
extern s32 g_wmap_land_effect_08_sequence_8_timer;
extern s32 rand(void);
extern s32 ccos(s32);
extern s32 g_wmap_land_effect_08_sequence_9_timer;
extern s32 g_wmap_land_effect_08_timer;
extern void (*D_800D6150[])(void);
extern void wmap_land_effect_08_step_03(void);
extern void wmap_land_effect_08_end(void);
extern s32 g_wmap_land_effect_08_timeline_timer;
extern void (*D_800D6168[])(void);
extern u32 g_wmap_selected_artifact;
extern s32 g_wmap_land_effect_08_sequence_1_timer;
extern void (*D_800D61C8[])(void);
extern void wmap_land_effect_08_sequence_1_step_02(void);
extern void wmap_land_effect_08_sequence_1_step_04(void);
extern s32 g_wmap_land_effect_08_sequence_2_timer;
extern void (*D_800D61E0[])(void);
extern u8 g_wmap_animation_bank_1[];
extern void wmap_land_effect_08_sequence_2_step_02(void);
extern void (*D_800D61F0[])(void);
extern void (*D_800D6200[])(void);
extern void (*D_800D6210[])(void);
extern s32 D_801B24B0;
extern void (*D_800D6228[])(void);
extern s32 g_wmap_land_effect_08_sequence_7_timer;
extern void (*D_800D6248[])(void);
extern void (*D_800D6260[])(void);
extern void (*D_800D6278[])(void);
extern void wmap_land_effect_08_sequence_9_step_04(void);
extern s32 g_wmap_land_effect_08_sequence_10_timer;
extern void (*D_800D6290[])(void);
extern void wmap_land_effect_08_sequence_10_step_02(void);

extern u32 g_wmap_land_effect_08_sequence_3_step;
extern u32 g_wmap_land_effect_08_sequence_4_step;
extern u32 g_wmap_land_effect_08_sequence_5_step;
extern s32 g_wmap_animation_bank_2;
extern u32 g_wmap_land_effect_08_sequence_6_step;
extern u32 g_wmap_land_effect_08_sequence_8_step;
extern u32 g_wmap_land_effect_08_sequence_9_step;
extern u32 g_wmap_land_effect_08_step;
extern u32 g_wmap_land_effect_08_timeline_step;
extern u32 g_wmap_land_effect_08_sequence_1_step;
extern u32 g_wmap_land_effect_08_sequence_2_step;
extern u32 g_wmap_land_effect_08_sequence_7_step;
extern u32 g_wmap_land_effect_08_sequence_10_step;

extern VECTOR g_wmap_camera_translation;

extern SVECTOR D_801B2498;
extern SVECTOR D_801B2490;


extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* g_wmap_effect_params;

extern WmapOrbitingStar g_wmap_actor_motions[];

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_08_sequence_3_step_02,
    g_wmap_land_effect_08_sequence_3_step, g_wmap_land_effect_08_sequence_3_timer,
    g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position,
    g_wmap_effect_fade_a, g_wmap_load_buffer, -3500, 2)

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_08_sequence_4_step_02,
    g_wmap_land_effect_08_sequence_4_step, g_wmap_land_effect_08_sequence_4_timer,
    g_wmap_effect_model_b_rotation, g_wmap_effect_model_b_position,
    g_wmap_effect_fade_b, g_wmap_effect_model_pack_1, -3500, 1)

/**
 * @brief Draw and brighten two rotating model layers, then advance their countdown.
 */
void wmap_land_effect_08_sequence_5_step_02(void)
{
    s32 value;

    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(g_wmap_effect_model_pack_2, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    D_801B2490.vz += 0x20;
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(g_wmap_effect_model_pack_2, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    D_801B2498.vz += 0xC;
    PopMatrix();
    WMAP_MODEL_FADE_IN(D_801B2468, 2, 0x40, value);
    if (--g_wmap_land_effect_08_sequence_5_timer == 0)
    {
        g_wmap_land_effect_08_sequence_5_step += 1;
    }
}

/**
 * @brief World-map step handler: draw two frames of the animated actor, scroll each
 *        sub-field, decay the shared frame index with a floor, then advance the step.
 */
void wmap_land_effect_08_sequence_5_step_04(void)
{
    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(g_wmap_effect_model_pack_2, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    D_801B2490.vz += 0x20;
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model_default(g_wmap_effect_model_pack_2, 0, 0x10, 0x36, 0x7880, 1, D_801B2468);
    D_801B2498.vz += 0xC;
    PopMatrix();
    D_801B2468 -= 4;
    if (D_801B2468 < 0)
    {
        D_801B2468 = 0;
    }
    if (--g_wmap_land_effect_08_sequence_5_timer == 0)
    {
        g_wmap_land_effect_08_sequence_5_step += 1;
    }
}

/**
 * @brief Reset a range of world-map actor slots and kick the next handler.
 * @note Clears three parallel slot arrays for indices [0x64, 0x6C), seeds each
 *       actor's default fields, then advances the shared step counters.
 */
void wmap_land_effect_08_sequence_6_step_01(void)
{
    s32 i;

    g_wmap_effect_fade_c = 1;
    D_800DCEA8 = 1;
    for (i = 0x64; i < 0x6C; i++)
    {
        *(s16*)((u8*)g_wmap_actor_motions + i * 0x14) = 0;
        WMAP_INIT_PARTICLE_ACTOR(i, (u8*)&g_wmap_animation_bank_2, 0);
    }
    D_800D9150 = 4;
    g_wmap_land_effect_08_sequence_6_timer = 0x10;
    g_wmap_land_effect_08_sequence_6_step += 1;
    wmap_land_effect_08_sequence_6_step_02();
}

/** @brief Initialize randomized actors along a cosine depth curve. */
void wmap_land_effect_08_sequence_8_step_01(void)
{
    s32 i;
    WmapSpriteActor *actor;
    WmapMotion *motion;
    s32 field_value;

    i = 150;
    for (; i < 180; i++)
    {
        actor = &g_wmap_sprite_actors[i];
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_0;
        field_value = 1;
        actor->scale_index = 15;
        actor->previous_sequence = -1;
        actor->shade_step = 2;
        actor->resource_index = 0;
        actor->sequence = field_value;
        actor->target_shade = 129;
        actor->shade = field_value;
        motion = &g_wmap_actor_motions[i];
        motion->angle = rand() & 4095;
        motion->field_0E = (i - 150) * 4;
        motion->z = ccos(2048 - (((i - 150) * 1024) / 30)) * 120;
        motion->x = ((rand() * 80) >> 15) + 40;
    }
    g_wmap_land_effect_08_sequence_8_timer = 64;
    g_wmap_land_effect_08_sequence_8_step++;
    wmap_land_effect_08_sequence_8_step_02();
}

/** @brief Project and draw the world-map star field, spinning each entry each frame. */
WMAP_DEFINE_STAR_UPDATE(wmap_land_effect_08_sequence_8_step_02,
                        g_wmap_land_effect_08_sequence_8_step, g_wmap_land_effect_08_sequence_8_timer,
                        150, 180, 15)

/** @brief Project and draw the world-map star field, spinning each entry each frame. */
WMAP_DEFINE_STAR_UPDATE(wmap_land_effect_08_sequence_8_step_04,
                        g_wmap_land_effect_08_sequence_8_step, g_wmap_land_effect_08_sequence_8_timer,
                        150, 180, 15)

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_08_sequence_9_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 10;
    g_wmap_effect_params[0xB] = 1;
    g_wmap_effect_params[0xC] = 4;
    g_wmap_effect_params[0xD] = 512;
    g_wmap_effect_params[0xE] = 2;
    g_wmap_effect_params[0xF] = 2;
    g_wmap_effect_params[0x10] = 96;
    g_wmap_effect_params[0x11] = 200;
    g_wmap_effect_params[0x12] = 15;
    g_wmap_effect_params[0x13] = 0;
    g_wmap_effect_params[0x14] = 12000;
    WMAP_RESET_PARTICLE_SLOTS(i, 10,
                              g_wmap_actor_motions[i + g_wmap_effect_params[0x11]].state,
                              204, g_wmap_animation_bank_0);
    g_wmap_land_effect_08_sequence_9_timer = 20;
    g_wmap_land_effect_08_sequence_9_step++;
    wmap_land_effect_08_sequence_9_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_08_run, D_800D6150, 0x6, g_wmap_land_effect_08_step, g_wmap_land_effect_08_timer)

WMAP_STEP_RESET(wmap_land_effect_08_reset, g_wmap_land_effect_08_step, g_wmap_land_effect_08_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_08_step_01, g_wmap_land_effect_08_step, wmap_run_land_focus, wmap_land_effect_08_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_08_wait_idle_02, g_wmap_land_effect_08_step, wmap_land_effect_08_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_08_step_03, g_wmap_land_effect_08_step, wmap_land_effect_08_run_timeline, wmap_land_effect_08_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_08_wait_idle_04, g_wmap_land_effect_08_step, wmap_land_effect_08_end)

WMAP_STEP_ADVANCE(wmap_land_effect_08_end, g_wmap_land_effect_08_step)

WMAP_STEP_RUNNER(wmap_land_effect_08_run_timeline, D_800D6168, 0x18, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_08_timeline_reset, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer)

/**
 * @brief Set up a tinted world-map sub-scene and register its handler.
 */
void wmap_land_effect_08_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x701040);
    g_wmap_backdrop_target_level = 9;
    wmap_play_sound(0x2A, 0x80);
    wmap_start_sequence(wmap_land_effect_08_run_sequence_7);
    g_wmap_land_effect_08_timeline_timer = 0x14;
    g_wmap_land_effect_08_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_08_timeline_wait_02, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_08_timeline_step_03, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer,
                         wmap_land_effect_08_run_sequence_8, 0x14)

WMAP_STEP_WAIT(wmap_land_effect_08_timeline_wait_04, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_08_timeline_step_05, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer,
                         wmap_land_effect_08_run_sequence_3, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_08_timeline_wait_06, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer)

/** @brief Set world-map color and flags, register a callback, and begin a 20-tick delay. */
void wmap_land_effect_08_timeline_step_07(void)
{
    wmap_start_map_tint(0x561030);
    wmap_start_sequence(&wmap_land_effect_08_run_sequence_1);
    g_wmap_transition_mesh_hidden = 1;
    g_wmap_backdrop_target_level = 4;
    g_wmap_placement_overlay_hidden = 1;
    g_wmap_land_effect_08_timeline_timer = 0x14;
    g_wmap_land_effect_08_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_08_timeline_wait_08, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_08_timeline_step_09, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer,
                         wmap_land_effect_08_run_sequence_5, 0x14)

WMAP_STEP_WAIT(wmap_land_effect_08_timeline_wait_10, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_08_timeline_step_11, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer,
                         wmap_land_effect_08_run_sequence_6, 0x28)

WMAP_STEP_WAIT(wmap_land_effect_08_timeline_wait_12, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_08_timeline_step_13, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer,
                         wmap_land_effect_08_run_sequence_9, 0x14)

WMAP_STEP_WAIT(wmap_land_effect_08_timeline_wait_14, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_08_timeline_step_15, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer,
                         wmap_land_effect_08_run_sequence_4, 0x2)

WMAP_STEP_WAIT(wmap_land_effect_08_timeline_wait_16, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer)

/** @brief Set the drawing color and world-map values, then start a two-tick delay. */
void wmap_land_effect_08_timeline_step_17(void)
{
    g_wmap_backdrop_target_level = 8;
    wmap_start_map_tint(0x562056);
    g_wmap_transition_mesh_hidden = 0;
    g_wmap_land_effect_08_timeline_timer = 2;
    g_wmap_land_effect_08_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_08_timeline_wait_18, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_08_timeline_step_19, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer,
                         wmap_land_effect_08_run_sequence_2, 0x60)

WMAP_STEP_WAIT(wmap_land_effect_08_timeline_wait_20, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_08_timeline_step_21, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer,
                         wmap_land_effect_08_run_sequence_10, 0x34)

WMAP_STEP_WAIT(wmap_land_effect_08_timeline_wait_22, g_wmap_land_effect_08_timeline_step, g_wmap_land_effect_08_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_08_timeline_finish, g_wmap_land_effect_08_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y, g_wmap_selected_artifact)

WMAP_STEP_RUNNER(wmap_land_effect_08_run_sequence_1, D_800D61C8, 0x6, g_wmap_land_effect_08_sequence_1_step, g_wmap_land_effect_08_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_08_sequence_1_reset, g_wmap_land_effect_08_sequence_1_step, g_wmap_land_effect_08_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_08_sequence_1_step_01,
    g_wmap_land_effect_08_sequence_1_step, g_wmap_land_effect_08_sequence_1_timer,
    4, g_wmap_animation_bank_0, 2,
    0x81, 0x81, 8,
    0x60, wmap_land_effect_08_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_08_sequence_1_step_02, g_wmap_land_effect_08_sequence_1_step, g_wmap_land_effect_08_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0xF, 0x2, 0)

/**
 * @brief Initialise two object half-word fields, arm the timer, advance, and run the handler.
 */
void wmap_land_effect_08_sequence_1_step_03(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[4];

    actor->target_shade = 0;
    actor->shade_step = 8;
    g_wmap_land_effect_08_sequence_1_timer = 0x10;
    g_wmap_land_effect_08_sequence_1_step += 1;
    wmap_land_effect_08_sequence_1_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_08_sequence_1_step_04, g_wmap_land_effect_08_sequence_1_step, g_wmap_land_effect_08_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0xF, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_08_sequence_1_end, g_wmap_land_effect_08_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_08_run_sequence_2, D_800D61E0, 0x4, g_wmap_land_effect_08_sequence_2_step, g_wmap_land_effect_08_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_08_sequence_2_reset, g_wmap_land_effect_08_sequence_2_step, g_wmap_land_effect_08_sequence_2_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_08_sequence_2_step_01,
    g_wmap_land_effect_08_sequence_2_step, g_wmap_land_effect_08_sequence_2_timer,
    5, g_wmap_animation_bank_1, 0,
    0x80, 0x80, 8,
    0x96, wmap_land_effect_08_sequence_2_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_08_sequence_2_step_02, g_wmap_land_effect_08_sequence_2_step, g_wmap_land_effect_08_sequence_2_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x17, 0x8, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_08_sequence_2_end, g_wmap_land_effect_08_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_08_run_sequence_3, D_800D61F0, 0x4, g_wmap_land_effect_08_sequence_3_step, g_wmap_land_effect_08_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_08_sequence_3_reset, g_wmap_land_effect_08_sequence_3_step, g_wmap_land_effect_08_sequence_3_timer)

WMAP_STEP_DROP_START(wmap_land_effect_08_sequence_3_step_01, g_wmap_land_effect_08_sequence_3_step, g_wmap_land_effect_08_sequence_3_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40, wmap_land_effect_08_sequence_3_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_08_sequence_3_end, g_wmap_land_effect_08_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_08_run_sequence_4, D_800D6200, 0x4, g_wmap_land_effect_08_sequence_4_step, g_wmap_land_effect_08_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_08_sequence_4_reset, g_wmap_land_effect_08_sequence_4_step, g_wmap_land_effect_08_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_08_sequence_4_step_01, g_wmap_land_effect_08_sequence_4_step, g_wmap_land_effect_08_sequence_4_timer, g_wmap_effect_model_b_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_b_position, g_wmap_effect_fade_b, 0x80, 0xAFC8, 0x80, wmap_land_effect_08_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_08_sequence_4_end, g_wmap_land_effect_08_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_08_run_sequence_5, D_800D6210, 0x6, g_wmap_land_effect_08_sequence_5_step, g_wmap_land_effect_08_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_08_sequence_5_reset, g_wmap_land_effect_08_sequence_5_step, g_wmap_land_effect_08_sequence_5_timer)

/** @brief World-map step: reset counters and advance to the next handler. */
void wmap_land_effect_08_sequence_5_step_01(void)
{
    D_801B24B0 = 0;
    D_801B2468 = 1;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    D_801B2498.vx = 0;
    D_801B2498.vy = 0;
    D_801B2498.vz = 0;
    g_wmap_land_effect_08_sequence_5_timer = 0x40;
    g_wmap_land_effect_08_sequence_5_step += 1;
    wmap_land_effect_08_sequence_5_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_08_sequence_5_step_03, g_wmap_land_effect_08_sequence_5_step, g_wmap_land_effect_08_sequence_5_timer, 0x20,
                    wmap_land_effect_08_sequence_5_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_08_sequence_5_end, g_wmap_land_effect_08_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_08_run_sequence_6, D_800D6228, 0x8, g_wmap_land_effect_08_sequence_6_step, g_wmap_land_effect_08_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_08_sequence_6_reset, g_wmap_land_effect_08_sequence_6_step, g_wmap_land_effect_08_sequence_6_timer)

/**
 * @brief Update the particles and increase the shared fade value until the timer expires.
 */
WMAP_STEP_UPDATE_AND_RAMP(wmap_land_effect_08_sequence_6_step_02, g_wmap_land_effect_08_sequence_6_step, g_wmap_land_effect_08_sequence_6_timer,
                          g_wmap_effect_fade_c, 8,
                          func_8006B328(0x64, 0x6C, 4, -1, -1, -4, 0, 8, -0x6E, 0xF0, -0x6E, 0xF0, 1, 0x7F, 1, 4, 0))

WMAP_STEP_ARM_TIMER(wmap_land_effect_08_sequence_6_step_03, g_wmap_land_effect_08_sequence_6_step, g_wmap_land_effect_08_sequence_6_timer, 0x20,
                    wmap_land_effect_08_sequence_6_step_04)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_08_sequence_6_step_04,
    g_wmap_land_effect_08_sequence_6_step, g_wmap_land_effect_08_sequence_6_timer,
    func_8006B328(0x64, 0x6C, 4, -1, -1, -4, 0, 8, -0x6E, 0xF0, -0x6E, 0xF0, 1, 0x7F, 1, 4, 0))

/**
 * @brief Stop spawning particles and keep updating those already active.
 */
WMAP_STEP_STOP_PARTICLE_SPAWNS(wmap_land_effect_08_sequence_6_step_05, g_wmap_land_effect_08_sequence_6_step, g_wmap_land_effect_08_sequence_6_timer,
                              D_800DCEA8, 0x10, wmap_land_effect_08_sequence_6_step_06)

/** @brief Update the effect until the step timer expires. */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_08_sequence_6_step_06,
    g_wmap_land_effect_08_sequence_6_step, g_wmap_land_effect_08_sequence_6_timer,
    func_8006B328(0x64, 0x6C, 4, -1, -1, -4, 0, 8, -0x6E, 0xF0, -0x6E, 0xF0, 1, 0x7F, 1, 4, 0))

WMAP_STEP_ADVANCE(wmap_land_effect_08_sequence_6_end, g_wmap_land_effect_08_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_08_run_sequence_7, D_800D6248, 0x6, g_wmap_land_effect_08_sequence_7_step, g_wmap_land_effect_08_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_08_sequence_7_reset, g_wmap_land_effect_08_sequence_7_step, g_wmap_land_effect_08_sequence_7_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_08_sequence_7_step_01,
    g_wmap_land_effect_08_sequence_7_step, g_wmap_land_effect_08_sequence_7_timer,
    6, &g_wmap_animation_bank_2, 1,
    1, 0x81, 8,
    0x28, wmap_land_effect_08_sequence_7_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_08_sequence_7_step_02, g_wmap_land_effect_08_sequence_7_step, g_wmap_land_effect_08_sequence_7_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x8, 0x2, 0)

/** @brief Initialize resource fields, begin a 16-tick delay, and run the next step. */
void wmap_land_effect_08_sequence_7_step_03(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[6];

    actor->target_shade = 2;
    actor->shade_step = 8;
    g_wmap_land_effect_08_sequence_7_timer = 16;
    g_wmap_land_effect_08_sequence_7_step += 1;
    wmap_land_effect_08_sequence_7_step_04();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_08_sequence_7_step_04, g_wmap_land_effect_08_sequence_7_step, g_wmap_land_effect_08_sequence_7_timer, g_wmap_sprite_actors[6],
                              g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x8, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_08_sequence_7_end, g_wmap_land_effect_08_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_08_run_sequence_8, D_800D6260, 0x6, g_wmap_land_effect_08_sequence_8_step, g_wmap_land_effect_08_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_08_sequence_8_reset, g_wmap_land_effect_08_sequence_8_step, g_wmap_land_effect_08_sequence_8_timer)

/**
 * @brief Start fading the sprite range and run its first update.
 */
WMAP_STEP_FADE_ACTOR_RANGE(wmap_land_effect_08_sequence_8_step_03,
    g_wmap_land_effect_08_sequence_8_step, g_wmap_land_effect_08_sequence_8_timer,
    0x96, 0xB4, 4, 0x20, wmap_land_effect_08_sequence_8_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_08_sequence_8_end, g_wmap_land_effect_08_sequence_8_step)

WMAP_STEP_RUNNER(wmap_land_effect_08_run_sequence_9, D_800D6278, 0x6, g_wmap_land_effect_08_sequence_9_step, g_wmap_land_effect_08_sequence_9_timer)

WMAP_STEP_RESET(wmap_land_effect_08_sequence_9_reset, g_wmap_land_effect_08_sequence_9_step, g_wmap_land_effect_08_sequence_9_timer)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_08_sequence_9_step_02,
    g_wmap_land_effect_08_sequence_9_step, g_wmap_land_effect_08_sequence_9_timer,
    func_8006A2FC(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 0xA, 0, 0x7F, 0x2, 0, &g_wmap_effect_params[10]))

/**
 * @brief Stop spawning particles and run the next update.
 */
WMAP_STEP_STOP_EMITTER(wmap_land_effect_08_sequence_9_step_03,
    g_wmap_land_effect_08_sequence_9_step, g_wmap_land_effect_08_sequence_9_timer,
    g_wmap_effect_params[15], 0x40, wmap_land_effect_08_sequence_9_step_04)

/**
 * @brief Update the effect until the step timer expires.
 */
WMAP_STEP_UPDATE_AND_WAIT(wmap_land_effect_08_sequence_9_step_04,
    g_wmap_land_effect_08_sequence_9_step, g_wmap_land_effect_08_sequence_9_timer,
    func_8006A2FC(&g_wmap_sprite_actors[204], &g_wmap_actor_animations[204], 0xA, 0, 0x7F, 0x2, 0, &g_wmap_effect_params[10]))

WMAP_STEP_ADVANCE(wmap_land_effect_08_sequence_9_end, g_wmap_land_effect_08_sequence_9_step)

WMAP_STEP_RUNNER(wmap_land_effect_08_run_sequence_10, D_800D6290, 0x4, g_wmap_land_effect_08_sequence_10_step, g_wmap_land_effect_08_sequence_10_timer)

WMAP_STEP_RESET(wmap_land_effect_08_sequence_10_reset, g_wmap_land_effect_08_sequence_10_step, g_wmap_land_effect_08_sequence_10_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_effect_08_sequence_10_step_01,
    g_wmap_land_effect_08_sequence_10_step, g_wmap_land_effect_08_sequence_10_timer,
    6, g_wmap_animation_bank_0, 3,
    0x81, 0x81, 1,
    0x24, wmap_land_effect_08_sequence_10_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_08_sequence_10_step_02, g_wmap_land_effect_08_sequence_10_step, g_wmap_land_effect_08_sequence_10_timer,
                              g_wmap_sprite_actors[6], g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0xF, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_08_sequence_10_end, g_wmap_land_effect_08_sequence_10_step)
