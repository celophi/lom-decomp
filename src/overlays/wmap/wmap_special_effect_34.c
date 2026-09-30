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

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A0C[];
extern WmapHandler D_800D7A2C[];
extern s32 g_wmap_cursor_column;
extern s32 g_wmap_cursor_row;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 D_800DCF18[];
extern u8 D_8011F538;
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 g_wmap_view_scroll_mode;
extern void* D_801399B4;
extern void* D_801399BC;
extern s32 D_8013B208;
extern s32 D_8013B294;
extern u8 D_80182E40;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_80182DD8;
extern s32 D_80182DE8;
extern VECTOR D_8011CF60;
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

extern VECTOR D_801B2650;

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

extern SVECTOR D_80139258;
extern SVECTOR D_801B24A0;

extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370;

extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8;

extern WmapScreenPosition g_wmap_focus_screen_position;
extern WmapScreenPosition D_80182D58;
extern WmapScreenPosition D_80182D60;

void wmap_special_effect_34_sequence_1_step_02(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    MATRIX matrix;
    s32 value;

    value = D_801B2650.vz + 0x7D0;
    D_801B2650.vz = value;
    if (value > 0xBB80)
    {
        D_801B2650.vz = 0xBB80;
    }
    PushMatrix();
    RotMatrix(&D_801B24A0, &matrix);
    TransMatrix(&matrix, &D_801B2650);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    wmap_draw_model(D_800DCF18, 0, 4, 0xB6, 0x7880, 1, D_80182DE8, 0, 0, -1);
    value = D_80182DE8 + 2;
    D_80182DE8 = value;
    if (value >= 0x41)
    {
        D_80182DE8 = 0x10080;
    }
    PopMatrix();
    if (--g_wmap_special_effect_34_sequence_1_timer == 0)
    {
        g_wmap_special_effect_34_sequence_1_step++;
    }
}

void wmap_special_effect_34_sequence_1_step_04(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    MATRIX matrix;
    SVECTOR* rotation = &D_801B24A0;
    VECTOR* translation;

    rotation->vy += D_80139240;
    D_80139240 += 4;
    if (D_80139240 >= 0x101)
    {
        D_80139240 = 0x100;
    }
    D_8013923C -= 2;
    PushMatrix();
    translation = &D_801B2650;
    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    wmap_draw_model(D_800DCF18, 0, 4, 0xB6, 0x7880, 1, 0x10080, 0, D_8013923C >> 4, -1);
    PopMatrix();
    if (--g_wmap_special_effect_34_sequence_1_timer == 0)
    {
        g_wmap_special_effect_34_sequence_1_step++;
    }
}

void wmap_special_effect_34_sequence_1_step_06(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    MATRIX matrix;
    SVECTOR* rotation = &D_801B24A0;
    VECTOR* translation;

    rotation->vy += D_80139240;
    D_80139240++;
    if (D_80139240 >= 0x81)
    {
        D_80139240 = 0x80;
    }
    D_8013923C -= 0x12C;
    PushMatrix();
    translation = &D_801B2650;
    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    wmap_draw_model(D_800DCF18, 0, 4, 0xB6, 0x7880, 1, 0x10080, 0, D_8013923C >> 4, -1);
    PopMatrix();
    if (--g_wmap_special_effect_34_sequence_1_timer == 0)
    {
        g_wmap_special_effect_34_sequence_1_step++;
    }
}

void wmap_special_effect_34_sequence_2_step_01(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

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
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

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
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

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
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

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
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 x;
    s32 y;
    WmapConfigA* config = &D_800D9344;

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
    wmap_step_actor_animation(config, &D_801399B0);
    wmap_draw_actor_sprite(config, D_80182D58.packed, 8, 8, 0);
    if (--g_wmap_special_effect_34_sequence_3_timer == 0)
    {
        g_wmap_special_effect_34_sequence_3_step++;
    }
}

WMAP_STEP_RUNNER(wmap_special_effect_34_run, D_800D7A0C, 8, g_wmap_special_effect_34_step, g_wmap_special_effect_34_timer)

void wmap_special_effect_34_step_00(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    g_wmap_special_effect_34_step = 1;
    g_wmap_special_effect_34_timer = 1;
}

/**
 * @brief Effect 34 step: save the view and cursor, move the focus, load
 *        effect resource 0x22 and advance the step.
 * @note JP keeps the cursor and calls func_8005FF88(-1) instead.
 */
void wmap_special_effect_34_step_01(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

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

void wmap_special_effect_34_step_02(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    if (--g_wmap_special_effect_34_timer == 0)
    {
        g_wmap_special_effect_34_step++;
    }
}

void wmap_special_effect_34_step_03(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    cdrom_wait_queue_empty();
    func_800651B4(&D_80182E40);
    func_800651B4(&D_8018B240);
    wmap_start_sequence(wmap_special_effect_34_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_special_effect_34_step++;
    wmap_special_effect_34_step_04();
}

void wmap_special_effect_34_step_04(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    if (g_wmap_sequence_busy == 0)
    {
        g_wmap_special_effect_34_step++;
        wmap_special_effect_34_step_05();
    }
}

void wmap_special_effect_34_step_05(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = g_wmap_saved_view.x - g_wmap_view.x;
    g_wmap_scroll_remaining_y = g_wmap_saved_view.y - g_wmap_view.y;
    g_wmap_special_effect_34_step++;
    wmap_special_effect_34_step_06();
}

void wmap_special_effect_34_step_06(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

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
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_8013B208 = 0;
    wmap_reset_after_transition();
#if !defined(VERSION_JP)
    D_8013B294 = 1;
    g_wmap_cursor_column = D_801B3210;
    g_wmap_cursor_row = D_801B3214;
#endif
    g_wmap_special_effect_34_step++;
}

WMAP_STEP_RUNNER(wmap_special_effect_34_run_timeline, D_800D7A2C, 0xE, g_wmap_special_effect_34_timeline_step, g_wmap_special_effect_34_timeline_timer)

void wmap_special_effect_34_timeline_step_00(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    g_wmap_special_effect_34_timeline_step = 1;
    g_wmap_special_effect_34_timeline_timer = 1;
}

void wmap_special_effect_34_timeline_step_01(void)
{
    D_8013B208 = 1;
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

void wmap_special_effect_34_timeline_step_07(void)
{
    wmap_start_sequence(wmap_special_effect_34_run_sequence_3);
    g_wmap_special_effect_34_timeline_timer = 0xC;
    g_wmap_special_effect_34_timeline_step++;
}

void wmap_special_effect_34_timeline_step_08(void)
{
    if (--g_wmap_special_effect_34_timeline_timer == 0)
    {
        g_wmap_special_effect_34_timeline_step++;
    }
}

void wmap_special_effect_34_timeline_step_09(void)
{
    wmap_start_sequence(wmap_special_effect_34_run_sequence_4);
    g_wmap_special_effect_34_timeline_timer = 0x5A;
    g_wmap_special_effect_34_timeline_step++;
}

void wmap_special_effect_34_timeline_step_10(void)
{
    if (--g_wmap_special_effect_34_timeline_timer == 0)
    {
        g_wmap_special_effect_34_timeline_step++;
    }
}

void wmap_special_effect_34_timeline_step_11(void)
{
    wmap_start_map_tint(0x808080);
    g_wmap_backdrop_target_level = 0x10;
    g_wmap_special_effect_34_timeline_timer = 0x3C;
    g_wmap_special_effect_34_timeline_step++;
}

void wmap_special_effect_34_timeline_step_12(void)
{
    if (--g_wmap_special_effect_34_timeline_timer == 0)
    {
        g_wmap_special_effect_34_timeline_step++;
    }
}

void wmap_special_effect_34_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    g_wmap_special_effect_34_timeline_step++;
}

WMAP_STEP_RUNNER(wmap_special_effect_34_run_sequence_1, D_800D7A64, 8, g_wmap_special_effect_34_sequence_1_step, g_wmap_special_effect_34_sequence_1_timer)

WMAP_STEP_RESET(wmap_special_effect_34_sequence_1_reset, g_wmap_special_effect_34_sequence_1_step, g_wmap_special_effect_34_sequence_1_timer)

void wmap_special_effect_34_sequence_1_step_01(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = D_8011CF60;
    D_80182DE8 = 0;
    D_801B2650.vz = 0x1388;
    D_80139240 = 0;
    D_8013923C = 0;
    g_wmap_special_effect_34_sequence_1_timer = 0x3C;
    g_wmap_special_effect_34_sequence_1_step++;
    wmap_special_effect_34_sequence_1_step_02();
}

void wmap_special_effect_34_sequence_1_step_03(void)
{
    g_wmap_special_effect_34_sequence_1_timer = 0xB4;
    g_wmap_special_effect_34_sequence_1_step++;
    wmap_special_effect_34_sequence_1_step_04();
}

void wmap_special_effect_34_sequence_1_step_05(void)
{
    g_wmap_special_effect_34_sequence_1_timer = 0x1E;
    g_wmap_special_effect_34_sequence_1_step++;
    wmap_special_effect_34_sequence_1_step_06();
}

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
    D_801399B4 = &D_8011F538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.sequence = 1;
    D_800D9344.previous_sequence = -1;
    D_800D9344.resource_index = 0;
    D_800D9344.shade_step = 0;
    D_800D9344.target_shade = 0x80;
    D_800D9344.shade = 0x80;
    D_80182D60.point.x = 0;
    D_80182D60.point.y = 0;
    g_wmap_special_effect_34_sequence_3_timer = 0x64;
    g_wmap_special_effect_34_sequence_3_step++;
    wmap_special_effect_34_sequence_3_step_02();
}

void wmap_special_effect_34_sequence_3_step_03(void)
{
    D_800D9344.shade_step = 4;
    D_800D9344.target_shade = 0;
    g_wmap_special_effect_34_sequence_3_timer = 0x20;
    g_wmap_special_effect_34_sequence_3_step++;
    wmap_special_effect_34_sequence_3_step_04();
}

void wmap_special_effect_34_sequence_3_step_04(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, D_80182D58.packed, 8, 8, 0);
    if (--g_wmap_special_effect_34_sequence_3_timer == 0)
    {
        g_wmap_special_effect_34_sequence_3_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_special_effect_34_sequence_3_end, g_wmap_special_effect_34_sequence_3_step)

WMAP_STEP_RUNNER(wmap_special_effect_34_run_sequence_4, D_800D7AC4, 6, g_wmap_special_effect_34_sequence_4_step, g_wmap_special_effect_34_sequence_4_timer)

WMAP_STEP_RESET(wmap_special_effect_34_sequence_4_reset, g_wmap_special_effect_34_sequence_4_step, g_wmap_special_effect_34_sequence_4_timer)

void wmap_special_effect_34_sequence_4_step_01(void)
{
    D_801399BC = &D_8011F538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 0x10;
    D_800D9370.resource_index = 0;
    D_800D9370.sequence = 0;
    D_800D9370.target_shade = 0x80;
    D_800D9370.shade = 0;
    g_wmap_special_effect_34_sequence_4_timer = 0x3E;
    g_wmap_special_effect_34_sequence_4_step++;
    wmap_special_effect_34_sequence_4_step_02();
}

void wmap_special_effect_34_sequence_4_step_02(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 8, 8, 0);
    if (--g_wmap_special_effect_34_sequence_4_timer == 0)
    {
        g_wmap_special_effect_34_sequence_4_step++;
    }
}

void wmap_special_effect_34_sequence_4_step_03(void)
{
    D_800D9370.shade_step = 2;
    D_800D9370.target_shade = 0;
    g_wmap_special_effect_34_sequence_4_timer = 0x40;
    g_wmap_special_effect_34_sequence_4_step++;
    wmap_special_effect_34_sequence_4_step_04();
}

void wmap_special_effect_34_sequence_4_step_04(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 8, 8, 0);
    if (--g_wmap_special_effect_34_sequence_4_timer == 0)
    {
        g_wmap_special_effect_34_sequence_4_step++;
    }
}

WMAP_STEP_ADVANCE(wmap_special_effect_34_sequence_4_end, g_wmap_special_effect_34_sequence_4_step)

void func_800C0C5C(VECTOR* translation, SVECTOR* rotation)
{
    MATRIX matrix;

    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
}
