#include "wmap_main.h"
#include "wmap_land_effect_03.h"
#include "wmap_sequence_runtime.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"
#include "wmap_step_sequence.h"
#include "wmap_cells.h"

void wmap_land_effect_03_sequence_5_step_02(void);
void wmap_land_effect_03_sequence_6_step_02(void);
void wmap_land_effect_03_sequence_7_step_02(void);
void wmap_land_effect_03_sequence_8_step_02(void);
void wmap_land_effect_03_wait_idle_02(void);
void wmap_land_effect_03_step_03(void);
s32 wmap_land_effect_03_run_timeline(s32 arg0);
void wmap_land_effect_03_wait_idle_04(void);
void wmap_land_effect_03_end(void);
s32 wmap_land_effect_03_run_sequence_8(s32 arg0);
s32 wmap_land_effect_03_run_sequence_7(s32 arg0);
s32 wmap_land_effect_03_run_sequence_1(s32 arg0);
s32 wmap_land_effect_03_run_sequence_6(s32 arg0);
s32 wmap_land_effect_03_run_sequence_5(s32 arg0);
s32 wmap_land_effect_03_run_sequence_3(s32 arg0);
s32 wmap_land_effect_03_run_sequence_4(s32 arg0);
s32 wmap_land_effect_03_run_sequence_2(s32 arg0);
void wmap_land_effect_03_sequence_1_step_02(void);
void wmap_land_effect_03_sequence_2_step_02(void);
void wmap_land_effect_03_sequence_5_step_04(void);
void wmap_land_effect_03_sequence_6_step_04(void);
void wmap_land_effect_03_sequence_7_step_04(void);
void wmap_land_effect_03_sequence_8_step_04(void);

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

extern u8* g_wmap_effect_model_pack_1;
extern s32 g_wmap_land_effect_03_sequence_3_timer;
extern u8 g_wmap_load_buffer[];
extern s32 g_wmap_land_effect_03_sequence_4_timer;
extern s32 g_wmap_land_effect_03_sequence_5_timer;
extern s32 g_wmap_land_effect_03_sequence_6_timer;
extern s32 g_wmap_land_effect_03_sequence_7_timer;
extern s32 g_wmap_land_effect_03_sequence_8_timer;
extern s32 g_wmap_land_effect_03_timer;
extern void (*D_800D5A60[])(void);
extern void wmap_land_effect_03_step_03(void);
extern void wmap_land_effect_03_end(void);
extern s32 g_wmap_land_effect_03_timeline_timer;
extern void (*D_800D5A78[])(void);
extern u32 g_wmap_selected_artifact;
extern s32 g_wmap_land_effect_03_sequence_1_timer;
extern void (*D_800D5AB8[])(void);
extern u8 g_wmap_animation_bank_0[];
extern void wmap_land_effect_03_sequence_1_step_02(void);
extern s32 g_wmap_land_effect_03_sequence_2_timer;
extern void (*D_800D5AC8[])(void);
extern u8 g_wmap_animation_bank_1[];
extern void wmap_land_effect_03_sequence_2_step_02(void);
extern void (*D_800D5AD8[])(void);
extern void (*D_800D5AF0[])(void);
extern void (*D_800D5B00[])(void);
extern void (*D_800D5B18[])(void);
extern void wmap_land_effect_03_sequence_6_step_04(void);
extern void (*D_800D5B30[])(void);
extern void wmap_land_effect_03_sequence_7_step_04(void);
extern void (*D_800D5B48[])(void);
extern void wmap_land_effect_03_sequence_8_step_04(void);

extern u32 g_wmap_land_effect_03_sequence_3_step;
extern u32 g_wmap_land_effect_03_sequence_4_step;
extern u32 g_wmap_land_effect_03_sequence_5_step;
extern u8 g_wmap_animation_bank_2[];
extern u32 g_wmap_land_effect_03_sequence_6_step;
extern u32 g_wmap_land_effect_03_sequence_7_step;
extern u32 g_wmap_land_effect_03_sequence_8_step;
extern u32 g_wmap_land_effect_03_step;
extern u32 g_wmap_land_effect_03_timeline_step;
extern u32 g_wmap_land_effect_03_sequence_1_step;
extern u32 g_wmap_land_effect_03_sequence_2_step;

extern VECTOR g_wmap_camera_translation;

extern SVECTOR D_801B2490;


extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* g_wmap_effect_params;

extern WmapSlot14 g_wmap_actor_motions[];

/** @brief Draw the rotating effect, increase its scale, and update the sequence timer. */
void wmap_land_effect_03_sequence_3_step_02(void)
{
    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 0, 4, 0x35, 0x7800, 1, g_wmap_effect_fade_c);
    D_801B2490.vz += 100;
    PopMatrix();
    g_wmap_effect_fade_c += 2;
    if (g_wmap_effect_fade_c >= 0x82)
    {
        g_wmap_effect_fade_c = 0x81;
    }
    if (--g_wmap_land_effect_03_sequence_3_timer == 0)
    {
        g_wmap_land_effect_03_sequence_3_step++;
    }
}

/** @brief Draw the rotating effect, reduce its scale, and update the sequence timer. */
void wmap_land_effect_03_sequence_3_step_04(void)
{
    PushMatrix();
    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model_default(g_wmap_effect_model_pack_1, 0, 4, 0x35, 0x7800, 1, g_wmap_effect_fade_c);
    D_801B2490.vz += 100;
    PopMatrix();
    g_wmap_effect_fade_c -= 8;
    if (g_wmap_effect_fade_c < 0)
    {
        g_wmap_effect_fade_c = 0;
    }
    if (--g_wmap_land_effect_03_sequence_3_timer == 0)
    {
        g_wmap_land_effect_03_sequence_3_step++;
    }
}

/** @brief Move the model closer and fade it until the timer expires. */
WMAP_STEP_DROP_UPDATE(wmap_land_effect_03_sequence_4_step_02,
    g_wmap_land_effect_03_sequence_4_step, g_wmap_land_effect_03_sequence_4_timer,
    g_wmap_effect_model_a_rotation, g_wmap_effect_model_a_position,
    g_wmap_effect_fade_a, g_wmap_load_buffer, -3500, 2)

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_03_sequence_5_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 50;
    g_wmap_effect_params[0x1] = 4;
    g_wmap_effect_params[0x2] = 1;
    g_wmap_effect_params[0x3] = 20;
    g_wmap_effect_params[0x4] = 30;
    g_wmap_effect_params[0x5] = 16;
    g_wmap_effect_params[0x6] = 1500;
    g_wmap_effect_params[0x7] = 20;
    g_wmap_effect_params[0x8] = 19;
    g_wmap_effect_params[0x9] = 0;
    g_wmap_effect_params[0xA] = 10000;
    for (i = 0; i < 50; i++)
    {
        g_wmap_actor_motions[i + g_wmap_effect_params[0x7]].field_00 = 0;
        g_wmap_actor_animations[i + 24].data = g_wmap_animation_bank_2;
    }
    g_wmap_land_effect_03_sequence_5_timer = 144;
    g_wmap_land_effect_03_sequence_5_step++;
    wmap_land_effect_03_sequence_5_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_03_sequence_6_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 50;
    g_wmap_effect_params[0xB] = 4;
    g_wmap_effect_params[0xC] = 1;
    g_wmap_effect_params[0xD] = 20;
    g_wmap_effect_params[0xE] = 30;
    g_wmap_effect_params[0xF] = 4;
    g_wmap_effect_params[0x10] = 1500;
    g_wmap_effect_params[0x11] = 70;
    g_wmap_effect_params[0x12] = 19;
    g_wmap_effect_params[0x13] = 1;
    g_wmap_effect_params[0x14] = 10000;
    for (i = 0; i < 50; i++)
    {
        g_wmap_actor_motions[i + g_wmap_effect_params[0x11]].field_00 = 0;
        g_wmap_actor_animations[i + 74].data = g_wmap_animation_bank_2;
    }
    g_wmap_land_effect_03_sequence_6_timer = 144;
    g_wmap_land_effect_03_sequence_6_step++;
    wmap_land_effect_03_sequence_6_step_02();
}

/** @brief World-map step: fill spawn descriptor slot 1, clear its slot run, then advance. */
void wmap_land_effect_03_sequence_7_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 0x32;
    g_wmap_effect_params[21] = 2;
    g_wmap_effect_params[22] = 1;
    g_wmap_effect_params[23] = 0x14;
    g_wmap_effect_params[24] = 0x1E;
    g_wmap_effect_params[25] = 2;
    g_wmap_effect_params[26] = 0x5DC;
    g_wmap_effect_params[27] = 0x78;
    g_wmap_effect_params[28] = 0x13;
    g_wmap_effect_params[29] = 2;
    g_wmap_effect_params[30] = 0x2710;
    for (i = 0; i < 0x32; i++)
    {
        g_wmap_actor_motions[i + g_wmap_effect_params[27]].field_00 = 0;
        g_wmap_actor_animations[i + 0x7C].data = g_wmap_animation_bank_2;
    }
    g_wmap_land_effect_03_sequence_7_timer = 0x90;
    g_wmap_land_effect_03_sequence_7_step += 1;
    wmap_land_effect_03_sequence_7_step_02();
}

/** @brief Configure the effect, reset its resource slots, and advance the sequence. */
void wmap_land_effect_03_sequence_8_step_01(void)
{
    s32 i;

    g_wmap_particle_intensity = 50;
    g_wmap_effect_params[0x1F] = 1;
    g_wmap_effect_params[0x20] = 1;
    g_wmap_effect_params[0x21] = 20;
    g_wmap_effect_params[0x22] = 30;
    g_wmap_effect_params[0x23] = 2;
    g_wmap_effect_params[0x24] = 1500;
    g_wmap_effect_params[0x25] = 170;
    g_wmap_effect_params[0x26] = 19;
    g_wmap_effect_params[0x27] = 3;
    g_wmap_effect_params[0x28] = 10000;
    for (i = 0; i < 50; i++)
    {
        g_wmap_actor_motions[i + g_wmap_effect_params[0x25]].field_00 = 0;
        g_wmap_actor_animations[i + 174].data = g_wmap_animation_bank_2;
    }
    g_wmap_land_effect_03_sequence_8_timer = 144;
    g_wmap_land_effect_03_sequence_8_step++;
    wmap_land_effect_03_sequence_8_step_02();
}

WMAP_STEP_RUNNER(wmap_land_effect_03_run, D_800D5A60, 0x6, g_wmap_land_effect_03_step, g_wmap_land_effect_03_timer)

WMAP_STEP_RESET(wmap_land_effect_03_reset, g_wmap_land_effect_03_step, g_wmap_land_effect_03_timer)

WMAP_STEP_START_BLOCKING(wmap_land_effect_03_step_01, g_wmap_land_effect_03_step, wmap_run_land_focus, wmap_land_effect_03_wait_idle_02)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_03_wait_idle_02, g_wmap_land_effect_03_step, wmap_land_effect_03_step_03)

WMAP_STEP_START_BLOCKING(wmap_land_effect_03_step_03, g_wmap_land_effect_03_step, wmap_land_effect_03_run_timeline, wmap_land_effect_03_wait_idle_04)

WMAP_STEP_WAIT_IDLE(wmap_land_effect_03_wait_idle_04, g_wmap_land_effect_03_step, wmap_land_effect_03_end)

WMAP_STEP_ADVANCE(wmap_land_effect_03_end, g_wmap_land_effect_03_step)

WMAP_STEP_RUNNER(wmap_land_effect_03_run_timeline, D_800D5A78, 0x10, g_wmap_land_effect_03_timeline_step, g_wmap_land_effect_03_timeline_timer)

WMAP_STEP_RESET(wmap_land_effect_03_timeline_reset, g_wmap_land_effect_03_timeline_step, g_wmap_land_effect_03_timeline_timer)

/** @brief World-map step handler: kick two jobs and advance the step. */
void wmap_land_effect_03_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_start_map_tint(0x802028);
    g_wmap_backdrop_target_level = 4;
    wmap_play_sound(0x21, 0x80);
    g_wmap_land_effect_03_timeline_timer = 0x1E;
    g_wmap_land_effect_03_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_03_timeline_wait_02, g_wmap_land_effect_03_timeline_step, g_wmap_land_effect_03_timeline_timer)

/**
 * @brief Register three sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_effect_03_timeline_step_03(void)
{
    wmap_start_sequence(wmap_land_effect_03_run_sequence_8);
    wmap_start_sequence(wmap_land_effect_03_run_sequence_7);
    wmap_start_sequence(wmap_land_effect_03_run_sequence_1);
    g_wmap_land_effect_03_timeline_timer = 0x18;
    g_wmap_land_effect_03_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_03_timeline_wait_04, g_wmap_land_effect_03_timeline_step, g_wmap_land_effect_03_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_03_timeline_step_05, g_wmap_land_effect_03_timeline_step, g_wmap_land_effect_03_timeline_timer,
                         wmap_land_effect_03_run_sequence_6, 0x10)

WMAP_STEP_WAIT(wmap_land_effect_03_timeline_wait_06, g_wmap_land_effect_03_timeline_step, g_wmap_land_effect_03_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_03_timeline_step_07, g_wmap_land_effect_03_timeline_step, g_wmap_land_effect_03_timeline_timer,
                         wmap_land_effect_03_run_sequence_5, 0x8)

WMAP_STEP_WAIT(wmap_land_effect_03_timeline_wait_08, g_wmap_land_effect_03_timeline_step, g_wmap_land_effect_03_timeline_timer)

/**
 * @brief Register a world-map step callback and schedule its wait timer.
 */
void wmap_land_effect_03_timeline_step_09(void)
{
    g_wmap_placement_overlay_hidden = 1;
    wmap_start_sequence(wmap_land_effect_03_run_sequence_3);
    g_wmap_land_effect_03_timeline_timer = 0x3C;
    g_wmap_land_effect_03_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_effect_03_timeline_wait_10, g_wmap_land_effect_03_timeline_step, g_wmap_land_effect_03_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_03_timeline_step_11, g_wmap_land_effect_03_timeline_step, g_wmap_land_effect_03_timeline_timer,
                         wmap_land_effect_03_run_sequence_4, 0x8)

WMAP_STEP_WAIT(wmap_land_effect_03_timeline_wait_12, g_wmap_land_effect_03_timeline_step, g_wmap_land_effect_03_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_effect_03_timeline_step_13, g_wmap_land_effect_03_timeline_step, g_wmap_land_effect_03_timeline_timer,
                         wmap_land_effect_03_run_sequence_2, 0x8A)

WMAP_STEP_WAIT(wmap_land_effect_03_timeline_wait_14, g_wmap_land_effect_03_timeline_step, g_wmap_land_effect_03_timeline_timer)

WMAP_STEP_FINISH_MARK_CELL(wmap_land_effect_03_timeline_finish, g_wmap_land_effect_03_timeline_step, g_wmap_cells, g_wmap_focus_cell_x, g_wmap_focus_cell_y, g_wmap_selected_artifact)

WMAP_STEP_RUNNER(wmap_land_effect_03_run_sequence_1, D_800D5AB8, 0x4, g_wmap_land_effect_03_sequence_1_step, g_wmap_land_effect_03_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_effect_03_sequence_1_reset, g_wmap_land_effect_03_sequence_1_step, g_wmap_land_effect_03_sequence_1_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_03_sequence_1_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[4];

    g_wmap_actor_animations[4].data = g_wmap_animation_bank_0;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 2;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->target_shade = 0x80;
    actor->shade = 0;
    g_wmap_land_effect_03_sequence_1_timer = 0x7C;
    g_wmap_land_effect_03_sequence_1_step += 1;
    wmap_land_effect_03_sequence_1_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_03_sequence_1_step_02, g_wmap_land_effect_03_sequence_1_step, g_wmap_land_effect_03_sequence_1_timer, g_wmap_sprite_actors[4],
                              g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0x13, 0xA, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_03_sequence_1_end, g_wmap_land_effect_03_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_effect_03_run_sequence_2, D_800D5AC8, 0x4, g_wmap_land_effect_03_sequence_2_step, g_wmap_land_effect_03_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_effect_03_sequence_2_reset, g_wmap_land_effect_03_sequence_2_step, g_wmap_land_effect_03_sequence_2_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_effect_03_sequence_2_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    g_wmap_actor_animations[5].data = g_wmap_animation_bank_1;
    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->shade_step = 8;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->target_shade = 0x80;
    actor->shade = 0;
    g_wmap_land_effect_03_sequence_2_timer = 0x8C;
    g_wmap_land_effect_03_sequence_2_step += 1;
    wmap_land_effect_03_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_effect_03_sequence_2_step_02, g_wmap_land_effect_03_sequence_2_step, g_wmap_land_effect_03_sequence_2_timer, g_wmap_sprite_actors[5],
                              g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x1A, 0xA, 0)

WMAP_STEP_ADVANCE(wmap_land_effect_03_sequence_2_end, g_wmap_land_effect_03_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_effect_03_run_sequence_3, D_800D5AD8, 0x6, g_wmap_land_effect_03_sequence_3_step, g_wmap_land_effect_03_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_effect_03_sequence_3_reset, g_wmap_land_effect_03_sequence_3_step, g_wmap_land_effect_03_sequence_3_timer)

/** @brief Clear the rotation vector, set the flag, and start a 128-tick sequence step. */
void wmap_land_effect_03_sequence_3_step_01(void)
{
    g_wmap_effect_fade_c = 1;
    D_801B2490.vx = 0;
    D_801B2490.vy = 0;
    D_801B2490.vz = 0;
    g_wmap_land_effect_03_sequence_3_timer = 0x80;
    g_wmap_land_effect_03_sequence_3_step += 1;
    wmap_land_effect_03_sequence_3_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_land_effect_03_sequence_3_step_03, g_wmap_land_effect_03_sequence_3_step, g_wmap_land_effect_03_sequence_3_timer, 0x10,
                    wmap_land_effect_03_sequence_3_step_04)

WMAP_STEP_ADVANCE(wmap_land_effect_03_sequence_3_end, g_wmap_land_effect_03_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_effect_03_run_sequence_4, D_800D5AF0, 0x4, g_wmap_land_effect_03_sequence_4_step, g_wmap_land_effect_03_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_effect_03_sequence_4_reset, g_wmap_land_effect_03_sequence_4_step, g_wmap_land_effect_03_sequence_4_timer)

WMAP_STEP_DROP_START(wmap_land_effect_03_sequence_4_step_01, g_wmap_land_effect_03_sequence_4_step, g_wmap_land_effect_03_sequence_4_timer, g_wmap_effect_model_a_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_a_position, g_wmap_effect_fade_a, 0x80, 0xAFC8, 0x40, wmap_land_effect_03_sequence_4_step_02)

WMAP_STEP_ADVANCE(wmap_land_effect_03_sequence_4_end, g_wmap_land_effect_03_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_effect_03_run_sequence_5, D_800D5B00, 0x6, g_wmap_land_effect_03_sequence_5_step, g_wmap_land_effect_03_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_effect_03_sequence_5_reset, g_wmap_land_effect_03_sequence_5_step, g_wmap_land_effect_03_sequence_5_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_5_step_02(void)
{
    func_8006A2FC(&g_wmap_sprite_actors[24], &g_wmap_actor_animations[24], 0x32, 0, 0x7F, 0x2, 0, (s32)g_wmap_effect_params);
    if (--g_wmap_land_effect_03_sequence_5_timer == 0)
    {
        g_wmap_land_effect_03_sequence_5_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_03_sequence_5_step_03(void)
{
    g_wmap_land_effect_03_sequence_5_timer = 0x20;
    g_wmap_effect_params[5] = -1;
    g_wmap_land_effect_03_sequence_5_step += 1;
    wmap_land_effect_03_sequence_5_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_5_step_04(void)
{
    func_8006A2FC(&g_wmap_sprite_actors[24], &g_wmap_actor_animations[24], 0x32, 0, 0x7F, 0x2, 0, (s32)g_wmap_effect_params);
    if (--g_wmap_land_effect_03_sequence_5_timer == 0)
    {
        g_wmap_land_effect_03_sequence_5_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_03_sequence_5_end, g_wmap_land_effect_03_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_effect_03_run_sequence_6, D_800D5B18, 0x6, g_wmap_land_effect_03_sequence_6_step, g_wmap_land_effect_03_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_effect_03_sequence_6_reset, g_wmap_land_effect_03_sequence_6_step, g_wmap_land_effect_03_sequence_6_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_6_step_02(void)
{
    func_8006A2FC(&g_wmap_sprite_actors[74], &g_wmap_actor_animations[74], 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)g_wmap_effect_params + 0x28));
    if (--g_wmap_land_effect_03_sequence_6_timer == 0)
    {
        g_wmap_land_effect_03_sequence_6_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_03_sequence_6_step_03(void)
{
    g_wmap_land_effect_03_sequence_6_timer = 0x20;
    g_wmap_effect_params[15] = -1;
    g_wmap_land_effect_03_sequence_6_step += 1;
    wmap_land_effect_03_sequence_6_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_6_step_04(void)
{
    func_8006A2FC(&g_wmap_sprite_actors[74], &g_wmap_actor_animations[74], 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)g_wmap_effect_params + 0x28));
    if (--g_wmap_land_effect_03_sequence_6_timer == 0)
    {
        g_wmap_land_effect_03_sequence_6_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_03_sequence_6_end, g_wmap_land_effect_03_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_effect_03_run_sequence_7, D_800D5B30, 0x6, g_wmap_land_effect_03_sequence_7_step, g_wmap_land_effect_03_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_effect_03_sequence_7_reset, g_wmap_land_effect_03_sequence_7_step, g_wmap_land_effect_03_sequence_7_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_7_step_02(void)
{
    func_8006A2FC(&g_wmap_sprite_actors[124], &g_wmap_actor_animations[124], 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)g_wmap_effect_params + 0x50));
    if (--g_wmap_land_effect_03_sequence_7_timer == 0)
    {
        g_wmap_land_effect_03_sequence_7_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_03_sequence_7_step_03(void)
{
    g_wmap_land_effect_03_sequence_7_timer = 0x20;
    g_wmap_effect_params[25] = -1;
    g_wmap_land_effect_03_sequence_7_step += 1;
    wmap_land_effect_03_sequence_7_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_7_step_04(void)
{
    func_8006A2FC(&g_wmap_sprite_actors[124], &g_wmap_actor_animations[124], 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)g_wmap_effect_params + 0x50));
    if (--g_wmap_land_effect_03_sequence_7_timer == 0)
    {
        g_wmap_land_effect_03_sequence_7_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_03_sequence_7_end, g_wmap_land_effect_03_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_effect_03_run_sequence_8, D_800D5B48, 0x6, g_wmap_land_effect_03_sequence_8_step, g_wmap_land_effect_03_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_effect_03_sequence_8_reset, g_wmap_land_effect_03_sequence_8_step, g_wmap_land_effect_03_sequence_8_timer)

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_8_step_02(void)
{
    func_8006A2FC(&g_wmap_sprite_actors[174], &g_wmap_actor_animations[174], 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)g_wmap_effect_params + 0x78));
    if (--g_wmap_land_effect_03_sequence_8_timer == 0)
    {
        g_wmap_land_effect_03_sequence_8_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_effect_03_sequence_8_step_03(void)
{
    g_wmap_land_effect_03_sequence_8_timer = 0x20;
    g_wmap_effect_params[35] = -1;
    g_wmap_land_effect_03_sequence_8_step += 1;
    wmap_land_effect_03_sequence_8_step_04();
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void wmap_land_effect_03_sequence_8_step_04(void)
{
    func_8006A2FC(&g_wmap_sprite_actors[174], &g_wmap_actor_animations[174], 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)g_wmap_effect_params + 0x78));
    if (--g_wmap_land_effect_03_sequence_8_timer == 0)
    {
        g_wmap_land_effect_03_sequence_8_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_effect_03_sequence_8_end, g_wmap_land_effect_03_sequence_8_step)
