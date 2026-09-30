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

void func_800BFEEC(void);
void func_800BFF28(void);
void func_800BFF98(void);
void func_800BFFD8(void);
s32 func_800C0034(s32 reset);
void func_800C06DC(void);
void func_800C071C(void);
void func_800C075C(void);
void func_800C079C(void);
s32 func_800C0474(s32 reset);
s32 func_800C064C(s32 reset);
void func_800C0150(void);
void func_800C018C(void);
void func_800C0254(void);
void func_800C0294(void);
void func_800C02C0(void);
void func_800C07DC(void);
void func_800C02FC(void);
s32 func_800C07F8(s32 reset);
s32 func_800C09F4(s32 reset);
void func_800C0960(void);
void func_800C0B00(void);
void func_800C0BC8(void);

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
extern s32 D_8013B20C;
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
extern s32 D_801B3218;
extern s32 D_801B321C;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern void func_800BFEEC(void);
extern void func_800BFF28(void);
extern void func_800BFF98(void);
extern void func_800BFFD8(void);
extern s32 func_800C0034(s32);
extern void func_800C0150(void);
extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern s32 D_801B3234;
extern s32 D_801B3240;
extern s32 D_801B3244;
extern void func_800C02FC(void);
extern void func_800C07DC(void);
extern s32 func_800C07F8(s32);
extern void func_800C0960(void);
extern s32 func_800C09F4(s32);
extern void func_800C0B00(void);
extern void func_800C0BC8(void);
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

void func_800BF4C0(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

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
    if (--D_801B322C == 0)
    {
        D_801B3228++;
    }
}

void func_800BF5D4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

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
    if (--D_801B322C == 0)
    {
        D_801B3228++;
    }
}

void func_800BF6F8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

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
    if (--D_801B322C == 0)
    {
        D_801B3228++;
    }
}

void func_800BF81C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    g_wmap_view_scroll_mode = 0;
    if (wmap_find_land_cell(D_80054A18[(D_80182DD8 - 3) % 5], &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y) != 0)
    {
        g_wmap_view_scroll_mode = 2;
        g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 0x30) - g_wmap_view.x;
        g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 0x30) - g_wmap_view.y;
    }
    D_801B3230++;
    func_800C06DC();
}

void func_800BF920(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    g_wmap_view_scroll_mode = 0;
    if (wmap_find_land_cell(D_80054A18[(D_80182DD8 - 2) % 5], &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y) != 0)
    {
        g_wmap_view_scroll_mode = 2;
        g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 0x30) - g_wmap_view.x;
        g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 0x30) - g_wmap_view.y;
    }
    D_801B3230++;
    func_800C071C();
}

void func_800BFA24(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    g_wmap_view_scroll_mode = 0;
    if (wmap_find_land_cell(D_80054A18[(D_80182DD8 - 1) % 5], &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y) != 0)
    {
        g_wmap_view_scroll_mode = 2;
        g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 0x30) - g_wmap_view.x;
        g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 0x30) - g_wmap_view.y;
    }
    D_801B3230++;
    func_800C075C();
}

void func_800BFB28(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    g_wmap_view_scroll_mode = 0;
    if (wmap_find_land_cell(D_80054A18[D_80182DD8 % 5], &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y) != 0)
    {
        g_wmap_view_scroll_mode = 2;
        g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 0x30) - g_wmap_view.x;
        g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 0x30) - g_wmap_view.y;
    }
    D_801B3230++;
    func_800C079C();
}

void func_800BFC28(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

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
    if (--D_801B323C == 0)
    {
        D_801B3238++;
    }
}

s32 func_800BFD18(s32 reset)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (reset != 0)
    {
        D_801B3218 = 1;
        D_801B321C = 1;
        return 1;
    }

    if ((u32)D_801B3218 >= 8)
    {
        return 0;
    }

    D_800D7A0C[D_801B3218]();
    return 1;
}

void func_800BFD90(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3218 = 1;
    D_801B321C = 1;
}

/**
 * @brief Effect 34 step: save the view and cursor, move the focus, load
 *        effect resource 0x22 and advance the step.
 * @note JP keeps the cursor and calls func_8005FF88(-1) instead.
 */
void func_800BFDA8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

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
    D_801B321C = 0x1E;
    D_801B3218++;
}

void func_800BFE54(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (--D_801B321C == 0)
    {
        D_801B3218++;
    }
}

void func_800BFE88(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    cdrom_wait_queue_empty();
    func_800651B4(&D_80182E40);
    func_800651B4(&D_8018B240);
    wmap_start_sequence(func_800C0034);
    D_8013B20C = 1;
    D_801B3218++;
    func_800BFEEC();
}

void func_800BFEEC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (D_8013B20C == 0)
    {
        D_801B3218++;
        func_800BFF28();
    }
}

void func_800BFF28(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = g_wmap_saved_view.x - g_wmap_view.x;
    g_wmap_scroll_remaining_y = g_wmap_saved_view.y - g_wmap_view.y;
    D_801B3218++;
    func_800BFF98();
}

void func_800BFF98(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B3218++;
        func_800BFFD8();
    }
}

/**
 * @brief Effect 34 step: reset after the transition, restore the saved
 *        cursor and advance the step.
 * @note JP neither sets D_8013B294 nor restores the cursor.
 */
void func_800BFFD8(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_8013B208 = 0;
    wmap_reset_after_transition();
#if !defined(VERSION_JP)
    D_8013B294 = 1;
    g_wmap_cursor_column = D_801B3210;
    g_wmap_cursor_row = D_801B3214;
#endif
    D_801B3218++;
}

s32 func_800C0034(s32 reset)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    if (reset != 0)
    {
        D_801B3220 = 1;
        D_801B3224 = 1;
        return 1;
    }

    if ((u32)D_801B3220 >= 0xE)
    {
        return 0;
    }

    D_800D7A2C[D_801B3220]();
    return 1;
}

void func_800C00AC(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B3220 = 1;
    D_801B3224 = 1;
}

void func_800C00C4(void)
{
    D_8013B208 = 1;
    wmap_play_sound(0x3B, 0x80);
    wmap_start_map_tint(0x703040);
    g_wmap_backdrop_target_level = 0xA;
    D_80139234 = 1;
    D_8013924C = 1;
    wmap_start_sequence(func_800C0474);
    wmap_start_sequence(func_800C064C);
    D_801B3220++;
    func_800C0150();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800C0150(void)
{
    if (D_80139234 == 0)
    {
        D_801B3220 += 1;
        func_800C018C();
    }
}

/**
 * @brief Prepare sequence coordinates for the selected world-map entry and advance the step.
 */
void func_800C018C(void)
{
    wmap_find_land_cell(D_80054A18[D_80182DD8 - 4], &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 0x30) - g_wmap_view.x;
    g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 0x30) - g_wmap_view.y;
    D_801B3220++;
    func_800C0254();
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800C0254(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B3220 += 1;
        func_800C0294();
    }
}

/** @brief World-map step handler: bump the step counter and run the next step. */
void func_800C0294(void)
{
    D_801B3220 += 1;
    func_800C02C0();
}

void func_800C02C0(void)
{
    if (D_8013924C == 0)
    {
        D_801B3220++;
        func_800C02FC();
    }
}

void func_800C02FC(void)
{
    wmap_start_sequence(func_800C07F8);
    D_801B3224 = 0xC;
    D_801B3220++;
}

void func_800C0338(void)
{
    if (--D_801B3224 == 0)
    {
        D_801B3220++;
    }
}

void func_800C036C(void)
{
    wmap_start_sequence(func_800C09F4);
    D_801B3224 = 0x5A;
    D_801B3220++;
}

void func_800C03A8(void)
{
    if (--D_801B3224 == 0)
    {
        D_801B3220++;
    }
}

void func_800C03DC(void)
{
    wmap_start_map_tint(0x808080);
    g_wmap_backdrop_target_level = 0x10;
    D_801B3224 = 0x3C;
    D_801B3220++;
}

void func_800C0424(void)
{
    if (--D_801B3224 == 0)
    {
        D_801B3220++;
    }
}

void func_800C0458(void)
{
    D_8013B20C = 0;
    D_801B3220++;
}

s32 func_800C0474(s32 reset)
{
    if (reset != 0)
    {
        D_801B3228 = 1;
        D_801B322C = 1;
        return 1;
    }

    if ((u32)D_801B3228 >= 8)
    {
        return 0;
    }

    D_800D7A64[D_801B3228]();
    return 1;
}

void func_800C04EC(void)
{
    D_801B3228 = 1;
    D_801B322C = 1;
}

void func_800C0504(void)
{
    D_801B24A0 = D_80139258;
    D_801B2650 = D_8011CF60;
    D_80182DE8 = 0;
    D_801B2650.vz = 0x1388;
    D_80139240 = 0;
    D_8013923C = 0;
    D_801B322C = 0x3C;
    D_801B3228++;
    func_800BF4C0();
}

void func_800C05C0(void)
{
    D_801B322C = 0xB4;
    D_801B3228++;
    func_800BF5D4();
}

void func_800C05F8(void)
{
    D_801B322C = 0x1E;
    D_801B3228++;
    func_800BF6F8();
}

void func_800C0630(void)
{
    D_8013924C = 0;
    D_801B3228++;
}

s32 func_800C064C(s32 reset)
{
    if (reset != 0)
    {
        D_801B3230 = 1;
        D_801B3234 = 1;
        return 1;
    }

    if ((u32)D_801B3230 >= 0xA)
    {
        return 0;
    }

    D_800D7A84[D_801B3230]();
    return 1;
}

void func_800C06C4(void)
{
    D_801B3230 = 1;
    D_801B3234 = 1;
}

void func_800C06DC(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B3230++;
        func_800BF920();
    }
}

void func_800C071C(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B3230++;
        func_800BFA24();
    }
}

void func_800C075C(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B3230++;
        func_800BFB28();
    }
}

void func_800C079C(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B3230++;
        func_800C07DC();
    }
}

void func_800C07DC(void)
{
    D_80139234 = 0;
    D_801B3230++;
}

s32 func_800C07F8(s32 reset)
{
    if (reset != 0)
    {
        D_801B3238 = 1;
        D_801B323C = 1;
        return 1;
    }

    if ((u32)D_801B3238 >= 6)
    {
        return 0;
    }

    D_800D7AAC[D_801B3238]();
    return 1;
}

void func_800C0870(void)
{
    D_801B3238 = 1;
    D_801B323C = 1;
}

void func_800C0888(void)
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
    D_801B323C = 0x64;
    D_801B3238++;
    func_800BFC28();
}

void func_800C0914(void)
{
    D_800D9344.shade_step = 4;
    D_800D9344.target_shade = 0;
    D_801B323C = 0x20;
    D_801B3238++;
    func_800C0960();
}

void func_800C0960(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, D_80182D58.packed, 8, 8, 0);
    if (--D_801B323C == 0)
    {
        D_801B3238++;
    }
}

void func_800C09DC(void)
{
    D_801B3238++;
}

s32 func_800C09F4(s32 reset)
{
    if (reset != 0)
    {
        D_801B3240 = 1;
        D_801B3244 = 1;
        return 1;
    }

    if ((u32)D_801B3240 >= 6)
    {
        return 0;
    }

    D_800D7AC4[D_801B3240]();
    return 1;
}

void func_800C0A6C(void)
{
    D_801B3240 = 1;
    D_801B3244 = 1;
}

void func_800C0A84(void)
{
    D_801399BC = &D_8011F538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 0x10;
    D_800D9370.resource_index = 0;
    D_800D9370.sequence = 0;
    D_800D9370.target_shade = 0x80;
    D_800D9370.shade = 0;
    D_801B3244 = 0x3E;
    D_801B3240++;
    func_800C0B00();
}

void func_800C0B00(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 8, 8, 0);
    if (--D_801B3244 == 0)
    {
        D_801B3240++;
    }
}

void func_800C0B7C(void)
{
    D_800D9370.shade_step = 2;
    D_800D9370.target_shade = 0;
    D_801B3244 = 0x40;
    D_801B3240++;
    func_800C0BC8();
}

void func_800C0BC8(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 8, 8, 0);
    if (--D_801B3244 == 0)
    {
        D_801B3240++;
    }
}

void func_800C0C44(void)
{
    D_801B3240++;
}

void func_800C0C5C(VECTOR* translation, SVECTOR* rotation)
{
    MATRIX matrix;

    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
}
