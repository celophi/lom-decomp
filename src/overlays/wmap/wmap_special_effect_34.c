#include "wmap_model_render.h"
#include "wmap_special_effect_34.h"
#include "wmap_sprite_render.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_main.h"
#include "wmap_map_labels.h"
#include "wmap_effect_primitives.h"
#include "wmap_sequence_runtime.h"
#include "wmap_effect_resources.h"
#include "cdrom.h"
#include "sdk/libgte.h"
#include "wmap_step_sequence.h"

void wmap_special_effect_34_step_04(void);
void wmap_special_effect_34_step_05(void);
void wmap_special_effect_34_step_06(void);
void wmap_special_effect_34_step_07(void);
s32 wmap_special_effect_34_run_timeline(s32 reset);
void wmap_special_effect_34_sequence_2_step_02(void);
void wmap_special_effect_34_sequence_2_step_04(void);
void wmap_special_effect_34_sequence_2_step_06(void);
void wmap_special_effect_34_sequence_2_step_08(void);
s32 wmap_special_effect_34_run_sequence_1(s32 reset);
s32 wmap_special_effect_34_run_sequence_2(s32 reset);
void wmap_special_effect_34_timeline_step_02(void);
void wmap_special_effect_34_timeline_step_03(void);
void wmap_special_effect_34_timeline_step_04(void);
void wmap_special_effect_34_timeline_step_05(void);
void wmap_special_effect_34_timeline_step_06(void);
void wmap_special_effect_34_sequence_2_step_09(void);
void wmap_special_effect_34_timeline_step_07(void);
s32 wmap_special_effect_34_run_sequence_3(s32 reset);
s32 wmap_special_effect_34_run_sequence_4(s32 reset);
void wmap_special_effect_34_sequence_3_step_04(void);
void wmap_special_effect_34_sequence_4_step_02(void);
void wmap_special_effect_34_sequence_4_step_04(void);

typedef void (*WmapHandler)(void);

extern WmapAnimationSlot g_wmap_actor_animations[];
extern WmapHandler D_800D7A0C[];
extern WmapHandler D_800D7A2C[];
extern s32 g_wmap_cursor_column;
extern s32 g_wmap_cursor_row;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 g_wmap_load_buffer[];
extern u8 g_wmap_animation_bank_1;
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 g_wmap_view_scroll_mode;
extern s32 D_8013B294;
extern u8 D_80182E40;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_80182DD8;
extern u8 D_8018B240;
extern s32 D_801B3210;
extern s32 D_801B3214;
extern u32 g_wmap_special_effect_34_step;
extern s32 g_wmap_special_effect_34_timer;
extern u32 g_wmap_special_effect_34_timeline_step;
extern s32 g_wmap_special_effect_34_timeline_timer;
extern u32 g_wmap_special_effect_34_sequence_1_step;
extern s32 g_wmap_special_effect_34_sequence_1_timer;
extern u32 g_wmap_special_effect_34_sequence_2_step;
extern u32 g_wmap_special_effect_34_sequence_3_step;
extern s32 g_wmap_special_effect_34_sequence_3_timer;
extern void wmap_special_effect_34_step_04(void);
extern void wmap_special_effect_34_step_05(void);
extern void wmap_special_effect_34_step_06(void);
extern void wmap_special_effect_34_step_07(void);
extern s32 wmap_special_effect_34_run_timeline(s32);
extern void wmap_special_effect_34_timeline_step_02(void);
extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern s32 g_wmap_special_effect_34_sequence_2_timer;
extern u32 g_wmap_special_effect_34_sequence_4_step;
extern s32 g_wmap_special_effect_34_sequence_4_timer;
extern void wmap_special_effect_34_timeline_step_07(void);
extern void wmap_special_effect_34_sequence_2_step_09(void);
extern s32 wmap_special_effect_34_run_sequence_3(s32);
extern void wmap_special_effect_34_sequence_3_step_04(void);
extern s32 wmap_special_effect_34_run_sequence_4(s32);
extern void wmap_special_effect_34_sequence_4_step_02(void);
extern void wmap_special_effect_34_sequence_4_step_04(void);
extern const s32 D_80054A18[];


/** @brief Map scroll position (map units) and projection scale. */
typedef struct
{
    s32 x;
    s32 y;
    s32 projection_scale;
    s32 unknown_0c;
} WmapView;

extern WmapView g_wmap_view;
extern WmapView g_wmap_saved_view;




extern WmapScreenPosition g_wmap_focus_screen_position;
extern WmapScreenPosition D_80182D58;
extern WmapScreenPosition D_80182D60;

void wmap_special_effect_34_sequence_1_step_02(void)
{
    MATRIX matrix;
    s32 value;

    value = g_wmap_effect_model_a_position.vz + 0x7D0;
    g_wmap_effect_model_a_position.vz = value;
    if (value > 0xBB80)
    {
        g_wmap_effect_model_a_position.vz = 0xBB80;
    }
    PushMatrix();
    RotMatrix(&g_wmap_effect_model_a_rotation, &matrix);
    TransMatrix(&matrix, &g_wmap_effect_model_a_position);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    wmap_draw_model(g_wmap_load_buffer, 0, 4, 0xB6, 0x7880, 1, g_wmap_effect_fade_a, 0, 0, -1);
    value = g_wmap_effect_fade_a + 2;
    g_wmap_effect_fade_a = value;
    if (value >= 0x41)
    {
        g_wmap_effect_fade_a = 0x10080;
    }
    PopMatrix();
    if (--g_wmap_special_effect_34_sequence_1_timer == 0)
    {
        g_wmap_special_effect_34_sequence_1_step++;
    }
}

void wmap_special_effect_34_sequence_1_step_04(void)
{
    MATRIX matrix;
    SVECTOR* rotation = &g_wmap_effect_model_a_rotation;
    VECTOR* translation;

    rotation->vy += D_80139240;
    D_80139240 += 4;
    if (D_80139240 >= 0x101)
    {
        D_80139240 = 0x100;
    }
    D_8013923C -= 2;
    PushMatrix();
    translation = &g_wmap_effect_model_a_position;
    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    wmap_draw_model(g_wmap_load_buffer, 0, 4, 0xB6, 0x7880, 1, 0x10080, 0, D_8013923C >> 4, -1);
    PopMatrix();
    if (--g_wmap_special_effect_34_sequence_1_timer == 0)
    {
        g_wmap_special_effect_34_sequence_1_step++;
    }
}

void wmap_special_effect_34_sequence_1_step_06(void)
{
    MATRIX matrix;
    SVECTOR* rotation = &g_wmap_effect_model_a_rotation;
    VECTOR* translation;

    rotation->vy += D_80139240;
    D_80139240++;
    if (D_80139240 >= 0x81)
    {
        D_80139240 = 0x80;
    }
    D_8013923C -= 0x12C;
    PushMatrix();
    translation = &g_wmap_effect_model_a_position;
    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    wmap_draw_model(g_wmap_load_buffer, 0, 4, 0xB6, 0x7880, 1, 0x10080, 0, D_8013923C >> 4, -1);
    PopMatrix();
    if (--g_wmap_special_effect_34_sequence_1_timer == 0)
    {
        g_wmap_special_effect_34_sequence_1_step++;
    }
}

void wmap_special_effect_34_sequence_2_step_01(void)
{
    g_wmap_view_scroll_mode = 0;
    if (wmap_find_land_cell(D_80054A18[(D_80182DD8 - 3) % 5], &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y) != 0)
    {
        g_wmap_view_scroll_mode = 2;
        g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 0x30) - g_wmap_view.x;
        g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 0x30) - g_wmap_view.y;
    }
    g_wmap_special_effect_34_sequence_2_step++;
    wmap_special_effect_34_sequence_2_step_02();
}

void wmap_special_effect_34_sequence_2_step_03(void)
{
    g_wmap_view_scroll_mode = 0;
    if (wmap_find_land_cell(D_80054A18[(D_80182DD8 - 2) % 5], &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y) != 0)
    {
        g_wmap_view_scroll_mode = 2;
        g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 0x30) - g_wmap_view.x;
        g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 0x30) - g_wmap_view.y;
    }
    g_wmap_special_effect_34_sequence_2_step++;
    wmap_special_effect_34_sequence_2_step_04();
}

void wmap_special_effect_34_sequence_2_step_05(void)
{
    g_wmap_view_scroll_mode = 0;
    if (wmap_find_land_cell(D_80054A18[(D_80182DD8 - 1) % 5], &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y) != 0)
    {
        g_wmap_view_scroll_mode = 2;
        g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 0x30) - g_wmap_view.x;
        g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 0x30) - g_wmap_view.y;
    }
    g_wmap_special_effect_34_sequence_2_step++;
    wmap_special_effect_34_sequence_2_step_06();
}

void wmap_special_effect_34_sequence_2_step_07(void)
{
    g_wmap_view_scroll_mode = 0;
    if (wmap_find_land_cell(D_80054A18[D_80182DD8 % 5], &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y) != 0)
    {
        g_wmap_view_scroll_mode = 2;
        g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 0x30) - g_wmap_view.x;
        g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 0x30) - g_wmap_view.y;
    }
    g_wmap_special_effect_34_sequence_2_step++;
    wmap_special_effect_34_sequence_2_step_08();
}

void wmap_special_effect_34_sequence_3_step_02(void)
{
    s32 x;
    s32 y;
    WmapSpriteActor* config = &g_wmap_sprite_actors[5];

    if (D_80182D58.point.y < 0x78)
    {
        D_80182D60.point.x -= 0x80;
        D_80182D60.point.y += 0x168;
    }

    x = D_80182D60.point.x;
    if (x < 0)
    {
        x += 0xF;
    }
    y = D_80182D60.point.y;
    D_80182D58.point.x = (x >> 4) + 0xE6;
    if (y < 0)
    {
        y += 0xF;
    }
    D_80182D58.point.y = y >> 4;
    wmap_step_actor_animation(config, &g_wmap_actor_animations[5]);
    wmap_draw_actor_sprite(config, D_80182D58.packed, 8, 8, 0);
    if (--g_wmap_special_effect_34_sequence_3_timer == 0)
    {
        g_wmap_special_effect_34_sequence_3_step++;
    }
}

WMAP_STEP_RUNNER(wmap_special_effect_34_run, D_800D7A0C, 8, g_wmap_special_effect_34_step, g_wmap_special_effect_34_timer)

WMAP_STEP_RESET(wmap_special_effect_34_step_00, g_wmap_special_effect_34_step, g_wmap_special_effect_34_timer)

/**
 * @brief Effect 34 step: save the view and cursor, move the focus, load
 *        effect resource 0x22 and advance the step.
 * @note JP keeps the cursor and calls func_8005FF88(-1) instead.
 */
void wmap_special_effect_34_step_01(void)
{
#if !defined(VERSION_JP)
    s32 field_00;
    s32 field_04;
#endif

    g_wmap_focus_screen_position.point.x = 0xA4;
    g_wmap_focus_screen_position.point.y = 0x69;
    g_wmap_saved_view = g_wmap_view;
#if defined(VERSION_JP)
    func_8005FF88(-1);
#else
    field_00 = g_wmap_cursor_column;
    field_04 = g_wmap_cursor_row;
    g_wmap_cursor_row = 1;
    g_wmap_cursor_column = 1;
    D_801B3210 = field_00;
    D_801B3214 = field_04;
#endif
    func_800A89DC(0x22);
    g_wmap_special_effect_34_timer = 0x1E;
    g_wmap_special_effect_34_step++;
}

WMAP_STEP_WAIT(wmap_special_effect_34_step_02, g_wmap_special_effect_34_step, g_wmap_special_effect_34_timer)

void wmap_special_effect_34_step_03(void)
{
    cdrom_wait_queue_empty();
    func_800651B4(&D_80182E40);
    func_800651B4(&D_8018B240);
    wmap_start_sequence(wmap_special_effect_34_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_special_effect_34_step++;
    wmap_special_effect_34_step_04();
}

WMAP_STEP_WAIT_IDLE(wmap_special_effect_34_step_04, g_wmap_special_effect_34_step, wmap_special_effect_34_step_05)

void wmap_special_effect_34_step_05(void)
{
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = g_wmap_saved_view.x - g_wmap_view.x;
    g_wmap_scroll_remaining_y = g_wmap_saved_view.y - g_wmap_view.y;
    g_wmap_special_effect_34_step++;
    wmap_special_effect_34_step_06();
}

void wmap_special_effect_34_step_06(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        g_wmap_special_effect_34_step++;
        wmap_special_effect_34_step_07();
    }
}

/**
 * @brief Effect 34 step: reset after the transition, restore the saved
 *        cursor and advance the step.
 * @note JP neither sets D_8013B294 nor restores the cursor.
 */
void wmap_special_effect_34_step_07(void)
{
    g_wmap_event_active = 0;
    wmap_reset_after_transition();
#if !defined(VERSION_JP)
    D_8013B294 = 1;
    g_wmap_cursor_column = D_801B3210;
    g_wmap_cursor_row = D_801B3214;
#endif
    g_wmap_special_effect_34_step++;
}

WMAP_STEP_RUNNER(wmap_special_effect_34_run_timeline, D_800D7A2C, 0xE, g_wmap_special_effect_34_timeline_step, g_wmap_special_effect_34_timeline_timer)

WMAP_STEP_RESET(wmap_special_effect_34_timeline_step_00, g_wmap_special_effect_34_timeline_step, g_wmap_special_effect_34_timeline_timer)

void wmap_special_effect_34_timeline_step_01(void)
{
    g_wmap_event_active = 1;
    wmap_play_sound(0x3B, 0x80);
    wmap_start_map_tint(0x703040);
    g_wmap_backdrop_target_level = 0xA;
    D_80139234 = 1;
    D_8013924C = 1;
    wmap_start_sequence(wmap_special_effect_34_run_sequence_1);
    wmap_start_sequence(wmap_special_effect_34_run_sequence_2);
    g_wmap_special_effect_34_timeline_step++;
    wmap_special_effect_34_timeline_step_02();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void wmap_special_effect_34_timeline_step_02(void)
{
    if (D_80139234 == 0)
    {
        g_wmap_special_effect_34_timeline_step += 1;
        wmap_special_effect_34_timeline_step_03();
    }
}

/**
 * @brief Prepare sequence coordinates for the selected world-map entry and advance the step.
 */
void wmap_special_effect_34_timeline_step_03(void)
{
    wmap_find_land_cell(D_80054A18[D_80182DD8 - 4], &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 0x30) - g_wmap_view.x;
    g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 0x30) - g_wmap_view.y;
    g_wmap_special_effect_34_timeline_step++;
    wmap_special_effect_34_timeline_step_04();
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void wmap_special_effect_34_timeline_step_04(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        g_wmap_special_effect_34_timeline_step += 1;
        wmap_special_effect_34_timeline_step_05();
    }
}

/** @brief World-map step handler: bump the step counter and run the next step. */
void wmap_special_effect_34_timeline_step_05(void)
{
    g_wmap_special_effect_34_timeline_step += 1;
    wmap_special_effect_34_timeline_step_06();
}

void wmap_special_effect_34_timeline_step_06(void)
{
    if (D_8013924C == 0)
    {
        g_wmap_special_effect_34_timeline_step++;
        wmap_special_effect_34_timeline_step_07();
    }
}

WMAP_STEP_START_AND_WAIT(wmap_special_effect_34_timeline_step_07, g_wmap_special_effect_34_timeline_step, g_wmap_special_effect_34_timeline_timer,
                         wmap_special_effect_34_run_sequence_3, 0xC)

WMAP_STEP_WAIT(wmap_special_effect_34_timeline_step_08, g_wmap_special_effect_34_timeline_step, g_wmap_special_effect_34_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_special_effect_34_timeline_step_09, g_wmap_special_effect_34_timeline_step, g_wmap_special_effect_34_timeline_timer,
                         wmap_special_effect_34_run_sequence_4, 0x5A)

WMAP_STEP_WAIT(wmap_special_effect_34_timeline_step_10, g_wmap_special_effect_34_timeline_step, g_wmap_special_effect_34_timeline_timer)

void wmap_special_effect_34_timeline_step_11(void)
{
    wmap_start_map_tint(0x808080);
    g_wmap_backdrop_target_level = 0x10;
    g_wmap_special_effect_34_timeline_timer = 0x3C;
    g_wmap_special_effect_34_timeline_step++;
}

WMAP_STEP_WAIT(wmap_special_effect_34_timeline_step_12, g_wmap_special_effect_34_timeline_step, g_wmap_special_effect_34_timeline_timer)

void wmap_special_effect_34_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    g_wmap_special_effect_34_timeline_step++;
}

WMAP_STEP_RUNNER(wmap_special_effect_34_run_sequence_1, D_800D7A64, 8, g_wmap_special_effect_34_sequence_1_step, g_wmap_special_effect_34_sequence_1_timer)

WMAP_STEP_RESET(wmap_special_effect_34_sequence_1_reset, g_wmap_special_effect_34_sequence_1_step, g_wmap_special_effect_34_sequence_1_timer)

void wmap_special_effect_34_sequence_1_step_01(void)
{
    g_wmap_effect_model_a_rotation = g_wmap_zero_rotation;
    g_wmap_effect_model_a_position = g_wmap_zero_translation;
    g_wmap_effect_fade_a = 0;
    g_wmap_effect_model_a_position.vz = 0x1388;
    D_80139240 = 0;
    D_8013923C = 0;
    g_wmap_special_effect_34_sequence_1_timer = 0x3C;
    g_wmap_special_effect_34_sequence_1_step++;
    wmap_special_effect_34_sequence_1_step_02();
}

WMAP_STEP_ARM_TIMER(wmap_special_effect_34_sequence_1_step_03, g_wmap_special_effect_34_sequence_1_step, g_wmap_special_effect_34_sequence_1_timer, 0xB4,
                    wmap_special_effect_34_sequence_1_step_04)

WMAP_STEP_ARM_TIMER(wmap_special_effect_34_sequence_1_step_05, g_wmap_special_effect_34_sequence_1_step, g_wmap_special_effect_34_sequence_1_timer, 0x1E,
                    wmap_special_effect_34_sequence_1_step_06)

void wmap_special_effect_34_sequence_1_step_07(void)
{
    D_8013924C = 0;
    g_wmap_special_effect_34_sequence_1_step++;
}

WMAP_STEP_RUNNER(wmap_special_effect_34_run_sequence_2, D_800D7A84, 0xA, g_wmap_special_effect_34_sequence_2_step, g_wmap_special_effect_34_sequence_2_timer)

WMAP_STEP_RESET(wmap_special_effect_34_sequence_2_reset, g_wmap_special_effect_34_sequence_2_step, g_wmap_special_effect_34_sequence_2_timer)

void wmap_special_effect_34_sequence_2_step_02(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        g_wmap_special_effect_34_sequence_2_step++;
        wmap_special_effect_34_sequence_2_step_03();
    }
}

void wmap_special_effect_34_sequence_2_step_04(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        g_wmap_special_effect_34_sequence_2_step++;
        wmap_special_effect_34_sequence_2_step_05();
    }
}

void wmap_special_effect_34_sequence_2_step_06(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        g_wmap_special_effect_34_sequence_2_step++;
        wmap_special_effect_34_sequence_2_step_07();
    }
}

void wmap_special_effect_34_sequence_2_step_08(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        g_wmap_special_effect_34_sequence_2_step++;
        wmap_special_effect_34_sequence_2_step_09();
    }
}

void wmap_special_effect_34_sequence_2_step_09(void)
{
    D_80139234 = 0;
    g_wmap_special_effect_34_sequence_2_step++;
}

WMAP_STEP_RUNNER(wmap_special_effect_34_run_sequence_3, D_800D7AAC, 6, g_wmap_special_effect_34_sequence_3_step, g_wmap_special_effect_34_sequence_3_timer)

WMAP_STEP_RESET(wmap_special_effect_34_sequence_3_reset, g_wmap_special_effect_34_sequence_3_step, g_wmap_special_effect_34_sequence_3_timer)

void wmap_special_effect_34_sequence_3_step_01(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    g_wmap_actor_animations[5].data = &g_wmap_animation_bank_1;
    actor->scale_index = 0xF;
    actor->sequence = 1;
    actor->previous_sequence = -1;
    actor->resource_index = 0;
    actor->shade_step = 0;
    actor->target_shade = 0x80;
    actor->shade = 0x80;
    D_80182D60.point.x = 0;
    D_80182D60.point.y = 0;
    g_wmap_special_effect_34_sequence_3_timer = 0x64;
    g_wmap_special_effect_34_sequence_3_step++;
    wmap_special_effect_34_sequence_3_step_02();
}

/** @brief Start the sprite fade and run its first update. */
WMAP_STEP_FADE_ACTOR(wmap_special_effect_34_sequence_3_step_03,
    g_wmap_special_effect_34_sequence_3_step, g_wmap_special_effect_34_sequence_3_timer,
    g_wmap_sprite_actors[5], 4, 0x20, wmap_special_effect_34_sequence_3_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_special_effect_34_sequence_3_step_04, g_wmap_special_effect_34_sequence_3_step, g_wmap_special_effect_34_sequence_3_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], D_80182D58, 8, 8, 0)

WMAP_STEP_ADVANCE(wmap_special_effect_34_sequence_3_end, g_wmap_special_effect_34_sequence_3_step)

WMAP_STEP_RUNNER(wmap_special_effect_34_run_sequence_4, D_800D7AC4, 6, g_wmap_special_effect_34_sequence_4_step, g_wmap_special_effect_34_sequence_4_timer)

WMAP_STEP_RESET(wmap_special_effect_34_sequence_4_reset, g_wmap_special_effect_34_sequence_4_step, g_wmap_special_effect_34_sequence_4_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_special_effect_34_sequence_4_step_01,
    g_wmap_special_effect_34_sequence_4_step, g_wmap_special_effect_34_sequence_4_timer,
    6, &g_wmap_animation_bank_1, 0,
    0, 0x80, 0x10,
    0x3E, wmap_special_effect_34_sequence_4_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_special_effect_34_sequence_4_step_02, g_wmap_special_effect_34_sequence_4_step, g_wmap_special_effect_34_sequence_4_timer,
                              g_wmap_sprite_actors[6], g_wmap_actor_animations[6], g_wmap_focus_screen_position, 8, 8, 0)

/** @brief Start the sprite fade and run its first update. */
WMAP_STEP_FADE_ACTOR(wmap_special_effect_34_sequence_4_step_03,
    g_wmap_special_effect_34_sequence_4_step, g_wmap_special_effect_34_sequence_4_timer,
    g_wmap_sprite_actors[6], 2, 0x40, wmap_special_effect_34_sequence_4_step_04)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_special_effect_34_sequence_4_step_04, g_wmap_special_effect_34_sequence_4_step, g_wmap_special_effect_34_sequence_4_timer,
                              g_wmap_sprite_actors[6], g_wmap_actor_animations[6], g_wmap_focus_screen_position, 8, 8, 0)

WMAP_STEP_ADVANCE(wmap_special_effect_34_sequence_4_end, g_wmap_special_effect_34_sequence_4_step)

void func_800C0C5C(VECTOR* translation, SVECTOR* rotation)
{
    MATRIX matrix;

    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
}
