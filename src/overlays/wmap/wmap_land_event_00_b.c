#include "wmap_model_render.h"
#include "wmap_land_event_00_b.h"
#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "cdrom.h"
#include "wmap_sprite_render.h"
#include "wmap_view_effects.h"
#include "wmap_main.h"
#include "wmap_effect_primitives.h"
#include "wmap_effect_resources.h"
#include "sdk/libgte.h"

void wmap_land_event_00_b_sequence_1_step_02(void);
void wmap_land_event_00_b_sequence_2_step_02(void);
void wmap_land_event_00_b_sequence_3_step_02(void);
void wmap_land_event_00_b_sequence_12_step_04(void);
s32 wmap_land_event_00_b_run_sequence_4(s32 reset);
s32 wmap_land_event_00_b_run_sequence_5(s32 reset);
s32 wmap_land_event_00_b_run_sequence_3(s32 reset);
s32 wmap_land_event_00_b_run_sequence_6(s32 reset);
s32 wmap_land_event_00_b_run_sequence_2(s32 reset);
s32 wmap_land_event_00_b_run_sequence_9(s32 reset);
s32 wmap_land_event_00_b_run_sequence_8(s32 reset);
s32 wmap_land_event_00_b_run_sequence_10(s32 reset);
s32 wmap_land_event_00_b_run_sequence_12(s32 reset);
s32 wmap_land_event_00_b_run_timeline(s32 reset);
void wmap_land_event_00_b_sequence_1_step_03(void);
void wmap_land_event_00_b_sequence_1_step_04(void);
void wmap_land_event_00_b_sequence_1_step_05(void);

typedef struct
{
    s16 field_00;
    s16 field_02;
    u16 field_04;
    s16 field_06;
} WmapShort4;

typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
    s32 field_0C;
    s32 field_10;
    s32 field_14;
    s32 field_18;
    s32 field_1C;
    s32 field_20;
    s32 state_24;
    s32 field_28;
    s32 field_2C;
    s32 field_30;
    s32 field_34;
    s32 field_38;
    s32 field_3C;
    s32 field_40;
    s32 field_44;
    s32 field_48;
    s32 tail_state;
} WmapState;

typedef void (*WmapHandler)(void);

/** @brief Event actor state at the start of its parameter block. */
typedef struct
{
    u8 pad[0x78];
    s32 state;
} WmapEventActorState;

extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern u8 D_800DEF18;
extern s32* D_8011CF1C;
extern s32* D_8011CF24;
extern s32* D_8011CF28;
extern s32* D_8011CF2C;
extern s32* D_8011CF30;
extern s32* D_8011CF34;
extern s32* D_8011CF38;
extern u8 D_8011D538;
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_801B2FE0;
extern WmapHandler D_800D7314[];
extern WmapHandler D_800D732C[];
extern WmapHandler D_800D7384[];
extern WmapHandler D_800D7394[];
extern WmapHandler D_800D73A4[];
extern WmapHandler D_800D73BC[];
extern WmapHandler D_800D73D4[];
extern WmapHandler D_800D73EC[];
extern WmapHandler D_800D7404[];
extern WmapHandler D_800D741C[];
extern WmapHandler D_800D7434[];
extern WmapHandler D_800D744C[];
extern WmapHandler D_800D745C[];
extern s32 D_800DCF18[];
extern s32 D_800D9228;
extern s32 D_8011D500;
extern s32 D_80139228;
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern void* D_801399AC;
extern void* D_801399B4;
extern s32 D_8013B20C;
extern s32 D_8013B208;
extern s32 D_8013B294;
extern s32 D_80182DE4;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern s32 D_80182E38;
extern VECTOR D_8011CF60;
extern s32 D_801B24B4;
extern s32 D_801B25D8;
extern s32 D_801B2FE4;
extern s32 D_801B2FE8;
extern s32 D_801B2FEC;
extern s32 D_801B2FF0;
extern s32 D_801B2FF4;
extern s32 D_801B2FF8;
extern s32 D_801B2FFC;
extern s32 D_801B3000;
extern s32 D_801B3004;
extern s32 D_801B3008;
extern s32 D_801B300C;
extern s32 D_801B3010;
extern s32 D_801B3014;
extern s32 D_801B3018;
extern s32 D_801B301C;
extern s32 D_801B3020;
extern s32 D_801B3024;
extern s32 D_801B3028;
extern s32 D_801B302C;
extern s32 D_801B3030;
extern s32 D_801B3034;
extern s32 D_801B3038;
extern s32 D_801B303C;
extern s32 D_801B3040;
extern s32 D_801B3044;
extern void func_8008ECF8(s32, s32, s32*, void*);
extern void wmap_land_event_00_b_sequence_4_step_02(void);
extern void wmap_land_event_00_b_sequence_4_step_04(void);
extern void wmap_land_event_00_b_sequence_5_step_02(void);
extern void wmap_land_event_00_b_sequence_5_step_04(void);
extern void wmap_land_event_00_b_sequence_6_step_02(void);
extern void wmap_land_event_00_b_sequence_6_step_04(void);
extern void wmap_land_event_00_b_sequence_7_step_02(void);
extern void wmap_land_event_00_b_sequence_7_step_04(void);
extern void wmap_land_event_00_b_sequence_8_step_02(void);
extern void wmap_land_event_00_b_sequence_8_step_04(void);
extern void wmap_land_event_00_b_sequence_9_step_02(void);
extern void wmap_land_event_00_b_sequence_9_step_04(void);
extern void wmap_land_event_00_b_sequence_10_step_02(void);
extern void wmap_land_event_00_b_sequence_10_step_04(void);
extern void wmap_land_event_00_b_sequence_11_step_02(void);
extern void wmap_land_event_00_b_sequence_12_step_02(void);
extern void wmap_land_event_00_b_sequence_1_step_03(void);
extern void wmap_land_event_00_b_sequence_1_step_04(void);
extern void wmap_land_event_00_b_sequence_1_step_05(void);
extern s32 wmap_land_event_00_b_run_timeline(s32);
extern s32 wmap_land_event_00_b_run_sequence_2(s32);
extern void wmap_land_event_00_b_sequence_2_step_02(void);
extern s32 wmap_land_event_00_b_run_sequence_3(s32);
extern void wmap_land_event_00_b_sequence_3_step_02(void);
extern s32 wmap_land_event_00_b_run_sequence_4(s32);
extern s32 wmap_land_event_00_b_run_sequence_5(s32);
extern s32 wmap_land_event_00_b_run_sequence_6(s32);
extern s32 wmap_land_event_00_b_run_sequence_8(s32);
extern s32 wmap_land_event_00_b_run_sequence_9(s32);
extern s32 wmap_land_event_00_b_run_sequence_10(s32);
extern s32 wmap_land_event_00_b_run_sequence_12(s32);
extern void wmap_land_event_00_b_sequence_12_step_04(void);
extern u8 D_80121538;
extern u8 D_801AFBD0[];

extern VECTOR g_wmap_camera_translation;
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

extern SVECTOR D_80139258;
extern SVECTOR D_801B2498;
extern SVECTOR D_8013B240;
extern SVECTOR D_801B24A0;
extern SVECTOR D_8013B238;
extern SVECTOR D_801B2490;
extern SVECTOR D_801B2670;
extern SVECTOR D_801B24A8;

extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* D_80139280;

/** @brief Queue world-map effect resources and initialize their map position. */
void wmap_land_event_00_b_sequence_1_step_01(void)
{
    D_8011CF1C = (s32*)&D_800DEF18;
    D_8011CF24 = (s32*)((u8*)D_8011CF1C + 0x3000);
    D_8011CF28 = (s32*)((u8*)D_8011CF24 + 0x3000);
    D_8011CF2C = (s32*)((u8*)D_8011CF28 + 0x3000);
    D_8011CF30 = (s32*)((u8*)D_8011CF2C + 0x3000);
    D_8011CF34 = (s32*)((u8*)D_8011CF30 + 0x3000);
    D_8011CF38 = (s32*)((u8*)D_8011CF34 + 0x3000);
    func_80064F64(0x1220);
    func_80064F64(0x1221);
    func_80064F64(0x1222);
    cdrom_queue_read(0x1223, &D_8011D538);
    cdrom_queue_read(0x1224, &D_800DEF18 - 0x2000);
    cdrom_queue_read(0x1225, D_8011CF1C);
    cdrom_queue_read(0x1226, D_8011CF24);
    cdrom_queue_read(0x1227, D_8011CF28);
    cdrom_queue_read(0x1228, D_8011CF2C);
    cdrom_queue_read(0x1229, D_8011CF38);
    cdrom_queue_read(0x122A, D_8011CF30);
    cdrom_queue_read(0x122B, D_8011CF34);
    wmap_find_land_cell(0, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 0x30) - g_wmap_view.x;
    g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 0x30) - g_wmap_view.y;
    D_801B2FE0++;
    wmap_land_event_00_b_sequence_1_step_02();
}

void wmap_land_event_00_b_sequence_4_step_02(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model(D_8011CF1C, (D_80139234 / 0x10) & 3, 5, 0x37, 0x7800, 1, D_801B24B4, -0x5A, -0xA, -1);
    D_80139234 += 4;
    value = D_801B24B4 + 2;
    D_801B24B4 = value;
    if (value >= 0x82)
    {
        D_801B24B4 = 0x81;
    }
    timer = D_801B3004;
    ((WmapShort4*)&D_801B2490)->field_04 += 0;
    next_timer = timer - 1;
    D_801B3004 = next_timer;
    if (next_timer == 0)
    {
        D_801B3000++;
    }
}

void wmap_land_event_00_b_sequence_4_step_04(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 value;
    s32 timer;
    s32 next_timer;
    s32* timer_ptr;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2490);
    wmap_draw_model(D_8011CF1C, (D_80139234 / 0x10) & 3, 5, 0x37, 0x7800, 1, D_801B24B4, -0x5A, -0xA, -1);
    value = D_801B24B4 - 4;
    D_801B24B4 = value;
    if (value < 0)
    {
        D_801B24B4 = 0;
    }
    timer_ptr = &D_801B3004;
    D_80139234 += 4;
    ((WmapShort4*)&D_801B2490)->field_04 += 0;
    timer = *timer_ptr;
    next_timer = timer - 1;
    *timer_ptr = next_timer;
    if (next_timer == 0)
    {
        D_801B3000++;
    }
}

void wmap_land_event_00_b_sequence_5_step_02(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model(D_8011CF24, (D_8013923C / 0x10) & 3, 5, 0x37, 0x7800, 1, D_80182DE4, 0x5F, -0x14, -1);
    D_8013923C += 4;
    value = D_80182DE4 + 4;
    D_80182DE4 = value;
    if (value >= 0x72)
    {
        D_80182DE4 = 0x71;
    }
    timer = D_801B300C;
    ((WmapShort4*)&D_801B2498)->field_04 += 0;
    next_timer = timer - 1;
    D_801B300C = next_timer;
    if (next_timer == 0)
    {
        D_801B3008++;
    }
}

void wmap_land_event_00_b_sequence_5_step_04(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 value;
    s32 timer;
    s32 next_timer;
    s32* timer_ptr;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model(D_8011CF24, (D_8013923C / 0x10) & 3, 5, 0x37, 0x7800, 1, D_80182DE4, 0x5F, -0x14, -1);
    value = D_80182DE4 - 4;
    D_80182DE4 = value;
    if (value < 0)
    {
        D_80182DE4 = 0;
    }
    timer_ptr = &D_801B300C;
    D_8013923C += 4;
    ((WmapShort4*)&D_801B2498)->field_04 += 0;
    timer = *timer_ptr;
    next_timer = timer - 1;
    *timer_ptr = next_timer;
    if (next_timer == 0)
    {
        D_801B3008++;
    }
}

void wmap_land_event_00_b_sequence_6_step_02(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B24A0);
    wmap_draw_model(D_8011CF28, (D_80139240 / 0x10) & 3, 5, 0x37, 0x7800, 1, D_80182DE8, 0x14, 0xA, -1);
    D_80139240 += 4;
    value = D_80182DE8 + 2;
    D_80182DE8 = value;
    if (value >= 0x82)
    {
        D_80182DE8 = 0x81;
    }
    timer = D_801B3014;
    ((WmapShort4*)&D_801B24A0)->field_04 += 0;
    next_timer = timer - 1;
    D_801B3014 = next_timer;
    if (next_timer == 0)
    {
        D_801B3010++;
    }
}

void wmap_land_event_00_b_sequence_6_step_04(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 value;
    s32 timer;
    s32 next_timer;
    s32* timer_ptr;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B24A0);
    wmap_draw_model(D_8011CF28, (D_80139240 / 0x10) & 3, 5, 0x37, 0x7800, 1, D_80182DE8, 0x14, 0xA, -1);
    value = D_80182DE8 - 4;
    D_80182DE8 = value;
    if (value < 0)
    {
        D_80182DE8 = 0;
    }
    timer_ptr = &D_801B3014;
    D_80139240 += 4;
    ((WmapShort4*)&D_801B24A0)->field_04 += 0;
    timer = *timer_ptr;
    next_timer = timer - 1;
    *timer_ptr = next_timer;
    if (next_timer == 0)
    {
        D_801B3010++;
    }
}

void wmap_land_event_00_b_sequence_7_step_02(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 timer;
    s32 next_timer;
    s32 value;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B24A8);
    wmap_draw_model(D_8011CF2C, (D_8013924C / 0x10) & 3, 0xA, 0x37, 0x7800, 1, D_80182DEC, 0, 0x14, -1);
    D_8013924C += 4;
    value = D_80182DEC + 8;
    D_80182DEC = value;
    if (value >= 0x82)
    {
        D_80182DEC = 0x81;
    }
    timer = D_801B301C;
    ((WmapShort4*)&D_801B24A8)->field_04 += 0;
    next_timer = timer - 1;
    D_801B301C = next_timer;
    if (next_timer == 0)
    {
        D_801B3018++;
    }
}

void wmap_land_event_00_b_sequence_7_step_04(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 timer;
    s32 next_timer;
    s32 value;
    s32* timer_ptr;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B24A8);
    wmap_draw_model(D_8011CF2C, (D_8013924C / 0x10) & 3, 0xA, 0x37, 0x7800, 1, D_80182DEC, 0, 0x14, -1);
    value = D_80182DEC - 2;
    D_80182DEC = value;
    if (value < 0)
    {
        D_80182DEC = 0;
    }
    timer_ptr = &D_801B301C;
    D_8013924C += 4;
    ((WmapShort4*)&D_801B24A8)->field_04 += 0;
    timer = *timer_ptr;
    next_timer = timer - 1;
    *timer_ptr = next_timer;
    if (next_timer == 0)
    {
        D_801B3018++;
    }
}

void wmap_land_event_00_b_sequence_8_step_02(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 value;
    s32 timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF30, (D_80139250 / 0x10) & 3, 0xA, 0x35, 0x7800, 1, D_80182DF0, 0, -0xA, -1);
    D_80139250 += 0x10;
    value = D_80182DF0 + 8;
    D_80182DF0 = value;
    if (value >= 0x82)
    {
        D_80182DF0 = 0x81;
    }
    timer = D_801B3024 - 1;
    D_8013B238.vz += 0x20;
    D_801B3024 = timer;
    if (timer == 0)
    {
        D_801B3020++;
    }
}

void wmap_land_event_00_b_sequence_8_step_04(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 value;
    s32 timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B238);
    wmap_draw_model(D_8011CF30, (D_80139250 / 0x10) & 3, 0xA, 0x35, 0x7800, 1, D_80182DF0, 0, -0xA, -1);
    value = D_80182DF0 - 2;
    D_80182DF0 = value;
    if (value < 0)
    {
        D_80182DF0 = 0;
    }
    D_80139250 += 0x10;
    timer = D_801B3024 - 1;
    D_801B3024 = timer;
    D_8013B238.vz += 0x20;
    if (timer == 0)
    {
        D_801B3020++;
    }
}

void wmap_land_event_00_b_sequence_9_step_02(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 value;
    s32 timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B240);
    wmap_draw_model(D_8011CF34, (D_80139260 / 0x10) & 3, 0xA, 0x35, 0x7800, 1, D_80182DF4, 0, 0, -1);
    D_80139260 += 8;
    value = D_80182DF4 + 2;
    D_80182DF4 = value;
    if (value >= 0x82)
    {
        D_80182DF4 = 0x81;
    }
    timer = D_801B302C - 1;
    D_8013B240.vz += 4;
    D_801B302C = timer;
    if (timer == 0)
    {
        D_801B3028++;
    }
}

void wmap_land_event_00_b_sequence_9_step_04(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 value;
    s32 timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_8013B240);
    wmap_draw_model(D_8011CF34, (D_80139260 / 0x10) & 3, 0xA, 0x35, 0x7800, 1, D_80182DF4, 0, 0, -1);
    value = D_80182DF4 - 4;
    D_80182DF4 = value;
    if (value < 0)
    {
        D_80182DF4 = 0;
    }
    D_80139260 += 8;
    timer = D_801B302C - 1;
    D_801B302C = timer;
    D_8013B240.vz += 4;
    if (timer == 0)
    {
        D_801B3028++;
    }
}

void wmap_land_event_00_b_sequence_10_step_02(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 value;
    s32 timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2670);
    wmap_draw_model(D_8011CF38, (D_80139264 / 0x10) & 3, 0xA, 0x35, 0x7800, 1, D_801B25D8, 0, -0xA, -1);
    D_80139264 += 8;
    value = D_801B25D8 + 8;
    D_801B25D8 = value;
    if (value >= 0x82)
    {
        D_801B25D8 = 0x81;
    }
    timer = D_801B3034 - 1;
    D_801B2670.vz += 4;
    D_801B3034 = timer;
    if (timer == 0)
    {
        D_801B3030++;
    }
}

void wmap_land_event_00_b_sequence_10_step_04(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 value;
    s32 timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2670);
    wmap_draw_model(D_8011CF38, (D_80139264 / 0x10) & 3, 0xA, 0x35, 0x7800, 1, D_801B25D8, 0, -0xA, -1);
    value = D_801B25D8 - 2;
    D_801B25D8 = value;
    if (value < 0)
    {
        D_801B25D8 = 0;
    }
    D_80139264 += 8;
    timer = D_801B3034 - 1;
    D_801B3034 = timer;
    D_801B2670.vz += 4;
    if (timer == 0)
    {
        D_801B3030++;
    }
}

void wmap_land_event_00_b_sequence_11_step_02(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 value;
    s32 fade;
    s32 timer;
    MATRIX matrix;

    value = D_801B2650.vz - 0xDAC;
    D_801B2650.vz = value;
    if (value < 0x2710)
    {
        D_801B2650.vz = 0x2710;
    }
    PushMatrix();
    RotMatrix(&D_801B24A0, &matrix);
    TransMatrix(&matrix, &D_8011CF60);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    if (D_80182DE8 != 0)
    {
        wmap_draw_model(D_800DCF18, 0, 4, 0x35, 0x7800, 1, D_80182DE8, 0, 0, -1);
        fade = D_80182DE8 - 8;
        D_80182DE8 = fade;
        if (fade < 0)
        {
            D_80182DE8 = 0;
        }
    }
    PopMatrix();
    timer = D_801B303C - 1;
    D_801B303C = timer;
    if (timer == 0)
    {
        D_801B3038++;
    }
}

/**
 * @brief World-map step handler: seed a 100-entry table, init an actor, advance.
 */
void wmap_land_event_00_b_sequence_12_step_01(void)
{
    s16 *slot;
    u8 *resource_cursor;
    u8 *actor;
    void *resource;
    s32 index;
    s32 setting;
    s32 current;

    index = 0x14;
    resource = &D_80121538;
    resource_cursor = (u8 *)&D_80139988 + index * 8;
    slot = (s16 *)((u8 *)&D_801AFBD0 + index * 20);
    do
    {
        *slot = 0;
        *(void **)(resource_cursor + 4) = resource;
        resource_cursor += 8;
        index += 1;
        slot += 0xA;
    } while (index < 0x78);
    D_801B3044 = 0xB4;
    actor = (WmapState*)D_80139280;
    *(s32 *)(actor + 0x7C) = setting = 5;
    *(s32 *)(actor + 0x8C) = setting;
    ((WmapEventActorState*)actor)->state = 1;
    *(s32 *)(actor + 0x7C) = setting;
    current = D_801B3040;
    *(s32 *)(actor + 0x80) = -0x3E8;
    *(s32 *)(actor + 0x84) = 0x1770;
    *(s32 *)(actor + 0x88) = 0x1388;
    *(s32 *)(actor + 0x90) = -0x64;
    *(s32 *)(actor + 0x98) = -0x12C;
    *(s32 *)(actor + 0x94) = 0;
    *(s32 *)(actor + 0xA0) = -1;
    *(s32 *)(actor + 0xA4) = 0;
    D_801B3040 = current + 1;
    wmap_land_event_00_b_sequence_12_step_02();
}

s32 wmap_land_event_00_b_run_sequence_1(s32 reset)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    if (reset != 0)
    {
        D_801B2FE0 = 1;
        D_801B2FE4 = 1;
        return 1;
    }
    if ((u32)D_801B2FE0 >= 6)
    {
        return 0;
    }
    D_800D7314[D_801B2FE0]();
    return 1;
}

void wmap_land_event_00_b_sequence_1_step_00(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B2FE0 = 1;
    D_801B2FE4 = 1;
}

void wmap_land_event_00_b_sequence_1_step_02(void)
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
        D_801B2FE0++;
        wmap_land_event_00_b_sequence_1_step_03();
    }
}

void wmap_land_event_00_b_sequence_1_step_03(void)
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
    g_wmap_focus_screen_position.point.x = 0x94;
    g_wmap_focus_screen_position.point.y = 0x31;
    D_80182E38 = 4;
    D_800D9228 = 0xFF;
    D_8011D500 = 0xFF;
    wmap_start_sequence(wmap_land_event_00_b_run_timeline);
    D_8013B20C = 1;
    D_801B2FE0++;
    wmap_land_event_00_b_sequence_1_step_04();
}

void wmap_land_event_00_b_sequence_1_step_04(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    if (D_8013B20C == 0)
    {
        D_801B2FE0++;
        wmap_land_event_00_b_sequence_1_step_05();
    }
}

void wmap_land_event_00_b_sequence_1_step_05(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_8013B294 = 1;
    D_80139228 = 2;
    D_801B2FE0++;
}

s32 wmap_land_event_00_b_run_timeline(s32 reset)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    if (reset != 0)
    {
        D_801B2FE8 = 1;
        D_801B2FEC = 1;
        return 1;
    }
    if ((u32)D_801B2FE8 >= 0x16)
    {
        return 0;
    }
    D_800D732C[D_801B2FE8]();
    return 1;
}

void wmap_land_event_00_b_timeline_step_00(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B2FE8 = 1;
    D_801B2FEC = 1;
}

void wmap_land_event_00_b_timeline_step_01(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_8013B208 = 1;
    wmap_play_sound(0x38, 0x80);
    D_8011D500 = 0x7F;
    wmap_start_sequence(wmap_land_event_00_b_run_sequence_10);
    wmap_start_sequence(wmap_land_event_00_b_run_sequence_8);
    wmap_start_map_tint(0x904060);
    g_wmap_backdrop_target_level = 0xE;
    wmap_start_sequence(wmap_land_event_00_b_run_sequence_12);
    D_801B2FEC = 0x5A;
    D_801B2FE8++;
}

void wmap_land_event_00_b_timeline_step_02(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void wmap_land_event_00_b_timeline_step_03(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    wmap_start_sequence(wmap_land_event_00_b_run_sequence_2);
    D_801B2FEC = 4;
    D_801B2FE8++;
}

void wmap_land_event_00_b_timeline_step_04(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void wmap_land_event_00_b_timeline_step_05(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    wmap_start_sequence(wmap_land_event_00_b_run_sequence_2);
    g_wmap_backdrop_target_level = 9;
    D_801B2FEC = 0x77;
    D_801B2FE8++;
}

void wmap_land_event_00_b_timeline_step_06(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void wmap_land_event_00_b_timeline_step_07(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_8011D500 = 1;
    wmap_start_sequence(wmap_land_event_00_b_run_sequence_9);
    D_801B2FEC = 0x14;
    D_801B2FE8++;
}

void wmap_land_event_00_b_timeline_step_08(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void wmap_land_event_00_b_timeline_step_09(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    wmap_start_sequence(wmap_land_event_00_b_run_sequence_3);
    D_801B2FEC = 0x24;
    D_801B2FE8++;
}

void wmap_land_event_00_b_timeline_step_10(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void wmap_land_event_00_b_timeline_step_11(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    g_wmap_backdrop_target_level = 0x11;
    wmap_start_map_tint(0x755085);
    D_8011D500 = 0x23;
    wmap_start_sequence(wmap_land_event_00_b_run_sequence_6);
    D_801B2FEC = 0x14;
    D_801B2FE8++;
}

void wmap_land_event_00_b_timeline_step_12(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void wmap_land_event_00_b_timeline_step_13(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    g_wmap_backdrop_target_level = 0xE;
    D_801B2FEC = 4;
    D_801B2FE8++;
}

void wmap_land_event_00_b_timeline_step_14(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void wmap_land_event_00_b_timeline_step_15(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    wmap_start_map_tint(0x654080);
    g_wmap_backdrop_target_level = 0xA;
    wmap_start_sequence(wmap_land_event_00_b_run_sequence_4);
    D_801B2FEC = 0x1E;
    D_801B2FE8++;
}

void wmap_land_event_00_b_timeline_step_16(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void wmap_land_event_00_b_timeline_step_17(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    wmap_start_sequence(wmap_land_event_00_b_run_sequence_5);
    D_801B2FEC = 0x64;
    D_801B2FE8++;
}

void wmap_land_event_00_b_timeline_step_18(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void wmap_land_event_00_b_timeline_step_19(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_8011D500 = 0xFF;
    D_801B2FEC = 0x70;
    D_801B2FE8++;
}

void wmap_land_event_00_b_timeline_step_20(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void wmap_land_event_00_b_timeline_finish(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_8013B20C = 0;
    D_801B2FE8++;
}

s32 wmap_land_event_00_b_run_sequence_2(s32 reset)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    if (reset != 0)
    {
        D_801B2FF0 = 1;
        D_801B2FF4 = 1;
        return 1;
    }
    if ((u32)D_801B2FF0 >= 4)
    {
        return 0;
    }
    D_800D7384[D_801B2FF0]();
    return 1;
}

void wmap_land_event_00_b_sequence_2_step_00(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B2FF0 = 1;
    D_801B2FF4 = 1;
}

void wmap_land_event_00_b_sequence_2_step_01(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801399AC = &D_8011D538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 0x10;
    D_800D9318.target_shade = 1;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.shade = 0x81;
    D_801B2FF4 = 0x10;
    D_801B2FF0++;
    wmap_land_event_00_b_sequence_2_step_02();
}

void wmap_land_event_00_b_sequence_2_step_02(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0xF, 2, 0);
    timer = D_801B2FF4 - 1;
    D_801B2FF4 = timer;
    if (timer == 0)
    {
        D_801B2FF0++;
    }
}

void wmap_land_event_00_b_sequence_2_step_03(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B2FF0++;
}

s32 wmap_land_event_00_b_run_sequence_3(s32 reset)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    if (reset != 0)
    {
        D_801B2FF8 = 1;
        D_801B2FFC = 1;
        return 1;
    }
    if ((u32)D_801B2FF8 >= 4)
    {
        return 0;
    }
    D_800D7394[D_801B2FF8]();
    return 1;
}

void wmap_land_event_00_b_sequence_3_step_00(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B2FF8 = 1;
    D_801B2FFC = 1;
}

void wmap_land_event_00_b_sequence_3_step_01(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 value;

    D_801399B4 = &D_8011D538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.sequence = value = 1;
    D_800D9344.previous_sequence = -value;
    D_800D9344.shade_step = 2;
    D_800D9344.shade = value;
    D_800D9344.resource_index = 0;
    D_800D9344.target_shade = 0x81;
    D_801B2FFC = 0x118;
    D_801B2FF8++;
    wmap_land_event_00_b_sequence_3_step_02();
}

void wmap_land_event_00_b_sequence_3_step_02(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 timer;

    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0xF, 5, 0);
    timer = D_801B2FFC - 1;
    D_801B2FFC = timer;
    if (timer == 0)
    {
        D_801B2FF8++;
    }
}

void wmap_land_event_00_b_sequence_3_step_03(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B2FF8++;
}

s32 wmap_land_event_00_b_run_sequence_4(s32 reset)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    if (reset != 0)
    {
        D_801B3000 = 1;
        D_801B3004 = 1;
        return 1;
    }
    if ((u32)D_801B3000 >= 6)
    {
        return 0;
    }
    D_800D73A4[D_801B3000]();
    return 1;
}

void wmap_land_event_00_b_sequence_4_step_00(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3000 = 1;
    D_801B3004 = 1;
}

void wmap_land_event_00_b_sequence_4_step_01(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B24B4 = 1;
    D_801B2490 = D_80139258;
    D_80139234 = 0;
    D_801B3004 = 0xA9;
    D_801B3000++;
    wmap_land_event_00_b_sequence_4_step_02();
}

void wmap_land_event_00_b_sequence_4_step_03(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3004 = 0x20;
    D_801B3000++;
    wmap_land_event_00_b_sequence_4_step_04();
}

void wmap_land_event_00_b_sequence_4_step_05(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3000++;
}

s32 wmap_land_event_00_b_run_sequence_5(s32 reset)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    if (reset != 0)
    {
        D_801B3008 = 1;
        D_801B300C = 1;
        return 1;
    }
    if ((u32)D_801B3008 >= 6)
    {
        return 0;
    }
    D_800D73BC[D_801B3008]();
    return 1;
}

void wmap_land_event_00_b_sequence_5_step_00(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3008 = 1;
    D_801B300C = 1;
}

void wmap_land_event_00_b_sequence_5_step_01(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_80182DE4 = 1;
    D_801B2498 = D_80139258;
    D_8013923C = 0;
    D_801B300C = 0xA8;
    D_801B3008++;
    wmap_land_event_00_b_sequence_5_step_02();
}

void wmap_land_event_00_b_sequence_5_step_03(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B300C = 0x20;
    D_801B3008++;
    wmap_land_event_00_b_sequence_5_step_04();
}

void wmap_land_event_00_b_sequence_5_step_05(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3008++;
}

s32 wmap_land_event_00_b_run_sequence_6(s32 reset)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    if (reset != 0)
    {
        D_801B3010 = 1;
        D_801B3014 = 1;
        return 1;
    }
    if ((u32)D_801B3010 >= 6)
    {
        return 0;
    }
    D_800D73D4[D_801B3010]();
    return 1;
}

void wmap_land_event_00_b_sequence_6_step_00(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3010 = 1;
    D_801B3014 = 1;
}

void wmap_land_event_00_b_sequence_6_step_01(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_80182DE8 = 1;
    D_801B24A0 = D_80139258;
    D_80139240 = 0;
    D_801B3014 = 0xBE;
    D_801B3010++;
    wmap_land_event_00_b_sequence_6_step_02();
}

void wmap_land_event_00_b_sequence_6_step_03(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3014 = 0x20;
    D_801B3010++;
    wmap_land_event_00_b_sequence_6_step_04();
}

void wmap_land_event_00_b_sequence_6_step_05(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3010++;
}

s32 wmap_land_event_00_b_run_sequence_7(s32 reset)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    if (reset != 0)
    {
        D_801B3018 = 1;
        D_801B301C = 1;
        return 1;
    }
    if ((u32)D_801B3018 >= 6)
    {
        return 0;
    }
    D_800D73EC[D_801B3018]();
    return 1;
}

void wmap_land_event_00_b_sequence_7_step_00(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3018 = 1;
    D_801B301C = 1;
}

void wmap_land_event_00_b_sequence_7_step_01(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_80182DEC = 1;
    D_801B24A8 = D_80139258;
    D_8013924C = 0;
    D_801B301C = 0x1E;
    D_801B3018++;
    wmap_land_event_00_b_sequence_7_step_02();
}

void wmap_land_event_00_b_sequence_7_step_03(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B301C = 0x40;
    D_801B3018++;
    wmap_land_event_00_b_sequence_7_step_04();
}

void wmap_land_event_00_b_sequence_7_step_05(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3018++;
}

s32 wmap_land_event_00_b_run_sequence_8(s32 reset)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    if (reset != 0)
    {
        D_801B3020 = 1;
        D_801B3024 = 1;
        return 1;
    }
    if ((u32)D_801B3020 >= 6)
    {
        return 0;
    }
    D_800D7404[D_801B3020]();
    return 1;
}

void wmap_land_event_00_b_sequence_8_step_00(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3020 = 1;
    D_801B3024 = 1;
}

void wmap_land_event_00_b_sequence_8_step_01(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_80182DF0 = 1;
    D_8013B238 = D_80139258;
    D_80139250 = 0;
    D_801B3024 = 0xB4;
    D_801B3020++;
    wmap_land_event_00_b_sequence_8_step_02();
}

void wmap_land_event_00_b_sequence_8_step_03(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3024 = 0x40;
    D_801B3020++;
    wmap_land_event_00_b_sequence_8_step_04();
}

void wmap_land_event_00_b_sequence_8_step_05(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3020++;
}

s32 wmap_land_event_00_b_run_sequence_9(s32 reset)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    if (reset != 0)
    {
        D_801B3028 = 1;
        D_801B302C = 1;
        return 1;
    }
    if ((u32)D_801B3028 >= 6)
    {
        return 0;
    }
    D_800D741C[D_801B3028]();
    return 1;
}

void wmap_land_event_00_b_sequence_9_step_00(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3028 = 1;
    D_801B302C = 1;
}

void wmap_land_event_00_b_sequence_9_step_01(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_80182DF4 = 1;
    D_8013B240 = D_80139258;
    D_80139260 = 0;
    D_801B302C = 0xF8;
    D_801B3028++;
    wmap_land_event_00_b_sequence_9_step_02();
}

void wmap_land_event_00_b_sequence_9_step_03(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B302C = 0x20;
    D_801B3028++;
    wmap_land_event_00_b_sequence_9_step_04();
}

void wmap_land_event_00_b_sequence_9_step_05(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3028++;
}

s32 wmap_land_event_00_b_run_sequence_10(s32 reset)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    if (reset != 0)
    {
        D_801B3030 = 1;
        D_801B3034 = 1;
        return 1;
    }
    if ((u32)D_801B3030 >= 6)
    {
        return 0;
    }
    D_800D7434[D_801B3030]();
    return 1;
}

void wmap_land_event_00_b_sequence_10_step_00(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3030 = 1;
    D_801B3034 = 1;
}

void wmap_land_event_00_b_sequence_10_step_01(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B25D8 = 1;
    D_801B2670 = D_80139258;
    D_80139264 = 0;
    D_801B3034 = 0xB4;
    D_801B3030++;
    wmap_land_event_00_b_sequence_10_step_02();
}

void wmap_land_event_00_b_sequence_10_step_03(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3034 = 0x40;
    D_801B3030++;
    wmap_land_event_00_b_sequence_10_step_04();
}

void wmap_land_event_00_b_sequence_10_step_05(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3030++;
}

s32 wmap_land_event_00_b_run_sequence_11(s32 reset)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    if (reset != 0)
    {
        D_801B3038 = 1;
        D_801B303C = 1;
        return 1;
    }
    if ((u32)D_801B3038 >= 4)
    {
        return 0;
    }
    D_800D744C[D_801B3038]();
    return 1;
}

void wmap_land_event_00_b_sequence_11_step_00(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3038 = 1;
    D_801B303C = 1;
}

void wmap_land_event_00_b_sequence_11_step_01(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B24A0 = D_80139258;
    D_801B2650 = g_wmap_camera_translation;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B303C = 0x10;
    D_801B3038++;
    wmap_land_event_00_b_sequence_11_step_02();
}

void wmap_land_event_00_b_sequence_11_step_03(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3038++;
}

s32 wmap_land_event_00_b_run_sequence_12(s32 reset)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    if (reset != 0)
    {
        D_801B3040 = 1;
        D_801B3044 = 1;
        return 1;
    }
    if ((u32)D_801B3040 >= 6)
    {
        return 0;
    }
    D_800D745C[D_801B3040]();
    return 1;
}

void wmap_land_event_00_b_sequence_12_step_00(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3040 = 1;
    D_801B3044 = 1;
}

void wmap_land_event_00_b_sequence_12_step_02(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 timer;

    func_8006AEE0();
    func_8008ECF8(0x14, 0x78, D_8011CF2C, (u8*)D_80139280 + 0x78);
    func_8006534C(0x25, 5);
    timer = D_801B3044 - 1;
    D_801B3044 = timer;
    if (timer == 0)
    {
        D_801B3040++;
    }
}

void wmap_land_event_00_b_sequence_12_step_03(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3044 = 0x20;
    D_80139280[30] = 0x270F;
    D_801B3040++;
    wmap_land_event_00_b_sequence_12_step_04();
}

void wmap_land_event_00_b_sequence_12_step_04(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    s32 timer;

    func_8006AEE0();
    func_8008ECF8(0x14, 0x78, D_8011CF2C, (u8*)D_80139280 + 0x78);
    func_8006534C(0x25, 5);
    timer = D_801B3044 - 1;
    D_801B3044 = timer;
    if (timer == 0)
    {
        D_801B3040++;
    }
}

void wmap_land_event_00_b_sequence_12_step_05(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    D_801B3040++;
}
