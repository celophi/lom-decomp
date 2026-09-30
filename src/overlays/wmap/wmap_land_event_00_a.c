#include "wmap_model_render.h"
#include "wmap_land_event_00_a.h"
#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "cdrom.h"
#include "sdk/libgte.h"
#include "wmap_view_effects.h"
#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"
#include "wmap_main.h"
#include "wmap_effect_resources.h"
#include "wmap_step_sequence.h"

void wmap_land_event_00_a_step_02(void);
void wmap_land_event_00_a_step_03(void);
void wmap_land_event_00_a_wait_idle(void);
s32 wmap_land_event_00_a_run_timeline(s32 arg0);
void wmap_land_event_00_a_step_05(void);
s32 wmap_land_event_00_a_run_sequence_6(s32 arg0);
s32 wmap_land_event_00_a_run_sequence_1(s32 arg0);
s32 wmap_land_event_00_a_run_sequence_5(s32 arg0);
s32 wmap_land_event_00_a_run_sequence_4(s32 arg0);
s32 wmap_land_event_00_a_run_sequence_7(s32 arg0);
s32 wmap_land_event_00_a_run_sequence_2(s32 arg0);
s32 wmap_land_event_00_a_run_sequence_8(s32 arg0);
s32 wmap_land_event_00_a_run_sequence_3(s32 arg0);
void wmap_land_event_00_a_sequence_1_step_02(void);
void wmap_land_event_00_a_sequence_3_step_02(void);
void wmap_land_event_00_a_sequence_4_step_04(void);

/** @brief Event actor state at the start of its parameter block. */
typedef struct
{
    u8 pad[0x78];
    s32 state;
} WmapEventActorState;

/** @brief Three color channels. */
typedef struct
{
    u8 r, g, b;
} WmapColor;

extern s16 D_800D926A;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern u8 D_800DEF18[];
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 g_wmap_land_event_00_a_sequence_4_timer;
extern void wmap_land_event_00_a_sequence_4_step_02(void);
extern VECTOR D_8011CF60;
extern s32 D_80182DF4;
extern s32 g_wmap_land_event_00_a_sequence_5_timer;
extern s32 D_8013923C;
extern s32 D_80182DE4;
extern s32 g_wmap_land_event_00_a_sequence_6_timer;
extern s32 D_80139240;
extern s32 D_80182DE8;
extern s32 g_wmap_land_event_00_a_sequence_7_timer;
extern s32 D_8013924C;
extern s32 D_80182DEC;
extern s32 g_wmap_land_event_00_a_sequence_8_timer;
extern s32 g_wmap_land_event_00_a_timer;
extern void (*D_800D7204[])(void);
extern s32 D_800D9228;
extern s32 D_8011D500;
extern s32 D_8013B258;
extern s32 D_80182E38;
extern s32 D_8013B294;
extern s32 D_80139228;
extern s32 g_wmap_land_event_00_a_timeline_timer;
extern void (*D_800D721C[])(void);
extern s32 D_8013B208;
extern s32 g_wmap_land_event_00_a_sequence_1_timer;
extern void (*D_800D7274[])(void);
extern void wmap_land_event_00_a_sequence_1_step_02(void);
extern s32 g_wmap_land_event_00_a_sequence_2_timer;
extern void (*D_800D7284[])(void);
extern void wmap_land_event_00_a_sequence_2_step_02(void);
extern s32 g_wmap_land_event_00_a_sequence_3_timer;
extern void (*D_800D7294[])(void);
extern void (*D_800D72A4[])(void);
extern void (*D_800D72BC[])(void);
extern void (*D_800D72CC[])(void);
extern void (*D_800D72E4[])(void);
extern void (*D_800D72FC[])(void);
extern u8 D_8011D538[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern s32* D_8011CF28;
extern s32* D_8011CF2C;
extern u32 g_wmap_land_event_00_a_step;
extern u8 D_80121538;
extern u8 D_801AFBD0[];
extern u32 g_wmap_land_event_00_a_sequence_4_step;
extern s32 D_800DCF18[];
extern u32 g_wmap_land_event_00_a_sequence_5_step;
extern u32 g_wmap_land_event_00_a_sequence_6_step;
extern u32 g_wmap_land_event_00_a_sequence_7_step;
extern u32 g_wmap_land_event_00_a_sequence_8_step;
extern u32 g_wmap_land_event_00_a_timeline_step;
extern WmapColor D_80182D74;
extern WmapColor D_80182D80;
extern WmapColor D_80182D8C;
extern WmapColor D_80182D94;
extern u32 g_wmap_land_event_00_a_sequence_1_step;
extern u8* D_801399AC;
extern u32 g_wmap_land_event_00_a_sequence_2_step;
extern void* D_801399B4;
extern u32 g_wmap_land_event_00_a_sequence_3_step;
extern void* D_801399BC;
extern void func_8008ECF8(s32, s32, s32*, void*);

extern VECTOR g_wmap_camera_translation;
extern VECTOR D_80139888;

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
extern SVECTOR D_801B24A8;

extern WmapSpriteActor D_800D9318;
extern WmapSpriteActor D_800D9344;
extern WmapSpriteActor D_800D9370;

extern WmapAnimationSlot D_80139988[];
extern WmapAnimationSlot D_801399A8;
extern WmapAnimationSlot D_801399B0;
extern WmapAnimationSlot D_801399B8;

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* D_80139280;

/** @brief Queue effect resources and establish the map-relative effect position. */
void wmap_land_event_00_a_step_01(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x7000;
    D_8011CF28 = D_8011CF24 + 0x3000;
    D_8011CF2C = D_8011CF24 + 0x7000;
    func_80064F64(0x1218);
    func_80064F64(0x1219);
    func_80064F64(0x121A);
    cdrom_queue_read(0x121B, D_8011D538);
    cdrom_queue_read(0x121C, D_800DEF18 - 0x2000);
    cdrom_queue_read(0x121D, D_8011CF1C);
    cdrom_queue_read(0x121E, D_8011CF24);
    cdrom_queue_read(0x121F, D_8011CF28);
    D_800D926A = -1;
    wmap_find_land_cell(0, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 48) - g_wmap_view.x;
    g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 48) - g_wmap_view.y;
    g_wmap_land_event_00_a_step++;
    wmap_land_event_00_a_step_02();
}

/**
 * @brief World-map step handler: seed a 100-entry table, init an actor, advance.
 */
void wmap_land_event_00_a_sequence_4_step_01(void)
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
    g_wmap_land_event_00_a_sequence_4_timer = 0xE1;
    actor = (u8*)D_80139280;
    *(s32 *)(actor + 0x7C) = setting = 5;
    *(s32 *)(actor + 0x8C) = setting;
    ((WmapEventActorState*)actor)->state = 1;
    *(s32 *)(actor + 0x7C) = setting;
    current = g_wmap_land_event_00_a_sequence_4_step;
    *(s32 *)(actor + 0x80) = -0x3E8;
    *(s32 *)(actor + 0x84) = 0x1770;
    *(s32 *)(actor + 0x88) = 0x1388;
    *(s32 *)(actor + 0x90) = -0x64;
    *(s32 *)(actor + 0x98) = -0x12C;
    *(s32 *)(actor + 0x94) = 0;
    *(s32 *)(actor + 0xA0) = -1;
    *(s32 *)(actor + 0xA4) = 0;
    g_wmap_land_event_00_a_sequence_4_step = current + 1;
    wmap_land_event_00_a_sequence_4_step_02();
}

/**
 * @brief World-map step handler: advance the model's spin toward a floor, draw it
 *        while active, then countdown-advance the step.
 */
void wmap_land_event_00_a_sequence_5_step_02(void)
{
    MATRIX m;
    s32 x;

    x = D_80139888.vz - 0xDAC;
    D_80139888.vz = x;
    if (x < 0x2710)
    {
        D_80139888.vz = 0x2710;
    }
    PushMatrix();
    RotMatrix(&D_8013B240, &m);
    TransMatrix(&m, &D_8011CF60);
    SetRotMatrix(&m);
    SetTransMatrix(&m);
    if (D_80182DF4 != 0)
    {
        wmap_draw_model(D_800DCF18, 0, 0x4, 0x35, 0x7800, 1, D_80182DF4, 0, 0, -1);
        D_80182DF4 -= 0x20;
        if (D_80182DF4 < 0)
        {
            D_80182DF4 = 0;
        }
    }
    PopMatrix();
    if (--g_wmap_land_event_00_a_sequence_5_timer == 0)
    {
        g_wmap_land_event_00_a_sequence_5_step += 1;
    }
}

/**
 * @brief World-map step handler: render the animated actor, ramp its size up to a
 *        cap, scroll the shadow field, then advance when the frame counter expires.
 */
void wmap_land_event_00_a_sequence_6_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model(D_8011CF1C, (D_8013923C / 0x10) & 3, 0xA, 0x35, 0x7800, 0x1, D_80182DE4, 0, -0xA, -1);
    D_8013923C += 0x8;
    value = D_80182DE4 + 0x2;
    D_80182DE4 = value;
    if (value >= 0x82)
    {
        D_80182DE4 = 0x81;
    }
    timer = g_wmap_land_event_00_a_sequence_6_timer;
    D_801B2498.vz += 0x4;
    next_timer = timer - 1;
    g_wmap_land_event_00_a_sequence_6_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_event_00_a_sequence_6_step++;
    }
}

/**
 * @brief World-map step handler: render the animated actor, ramp its size down to a
 *        floor, scroll the shadow field, then advance when the frame counter expires.
 */
void wmap_land_event_00_a_sequence_6_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model(D_8011CF1C, (D_8013923C / 0x10) & 3, 0xA, 0x35, 0x7800, 0x1, D_80182DE4, 0, -0xA, -1);
    value = D_80182DE4 - 0x10;
    D_80182DE4 = value;
    if (value < 0)
    {
        D_80182DE4 = 0;
    }
    D_8013923C += 0x8;
    timer = g_wmap_land_event_00_a_sequence_6_timer;
    D_801B2498.vz += 0x4;
    next_timer = timer - 1;
    g_wmap_land_event_00_a_sequence_6_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_event_00_a_sequence_6_step++;
    }
}

/**
 * @brief World-map step handler: render the animated actor, ramp its size up to a
 *        cap, scroll the shadow field, then advance when the frame counter expires.
 */
void wmap_land_event_00_a_sequence_7_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B24A0);
    wmap_draw_model(D_8011CF24, (D_80139240 / 0x10) & 3, 0xA, 0x35, 0x7840, 0x1, D_80182DE8, 0, -0xA, -1);
    D_80139240 += 0x10;
    value = D_80182DE8 + 0x8;
    D_80182DE8 = value;
    if (value >= 0x82)
    {
        D_80182DE8 = 0x81;
    }
    timer = g_wmap_land_event_00_a_sequence_7_timer;
    D_801B24A0.vz += 0x20;
    next_timer = timer - 1;
    g_wmap_land_event_00_a_sequence_7_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_event_00_a_sequence_7_step++;
    }
}

/**
 * @brief World-map step handler: render the animated actor, ramp its size down to a
 *        floor, scroll the shadow field, then advance when the frame counter expires.
 */
void wmap_land_event_00_a_sequence_7_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B24A0);
    wmap_draw_model(D_8011CF24, (D_80139240 / 0x10) & 3, 0xA, 0x35, 0x7840, 0x1, D_80182DE8, 0, -0xA, -1);
    value = D_80182DE8 - 0x10;
    D_80182DE8 = value;
    if (value < 0)
    {
        D_80182DE8 = 0;
    }
    D_80139240 += 0x10;
    timer = g_wmap_land_event_00_a_sequence_7_timer;
    D_801B24A0.vz += 0x20;
    next_timer = timer - 1;
    g_wmap_land_event_00_a_sequence_7_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_event_00_a_sequence_7_step++;
    }
}

/**
 * @brief World-map step handler: render the animated actor, ramp its size up to a
 *        cap, scroll the shadow field, then advance when the frame counter expires.
 */
void wmap_land_event_00_a_sequence_8_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B24A8);
    wmap_draw_model(D_8011CF28, (D_8013924C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1, D_80182DEC, 0, -0xA, -1);
    D_8013924C += 0x10;
    value = D_80182DEC + 0x8;
    D_80182DEC = value;
    if (value >= 0x82)
    {
        D_80182DEC = 0x81;
    }
    timer = g_wmap_land_event_00_a_sequence_8_timer;
    D_801B24A8.vz += 0x8;
    next_timer = timer - 1;
    g_wmap_land_event_00_a_sequence_8_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_event_00_a_sequence_8_step++;
    }
}

/**
 * @brief World-map step handler: render the animated actor, ramp its size down to a
 *        floor, scroll the shadow field, then advance when the frame counter expires.
 */
void wmap_land_event_00_a_sequence_8_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B24A8);
    wmap_draw_model(D_8011CF28, (D_8013924C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1, D_80182DEC, 0, -0xA, -1);
    value = D_80182DEC - 0x10;
    D_80182DEC = value;
    if (value < 0)
    {
        D_80182DEC = 0;
    }
    D_8013924C += 0x10;
    timer = g_wmap_land_event_00_a_sequence_8_timer;
    D_801B24A8.vz += 0x8;
    next_timer = timer - 1;
    g_wmap_land_event_00_a_sequence_8_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_event_00_a_sequence_8_step++;
    }
}

WMAP_STEP_RUNNER(wmap_land_event_00_a_run, D_800D7204, 0x6, g_wmap_land_event_00_a_step, g_wmap_land_event_00_a_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_reset, g_wmap_land_event_00_a_step, g_wmap_land_event_00_a_timer)

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void wmap_land_event_00_a_step_02(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        g_wmap_land_event_00_a_step += 1;
        wmap_land_event_00_a_step_03();
    }
}

/** @brief Wait for CD work, initialize sequence state, and register its callback. */
void wmap_land_event_00_a_step_03(void)
{
    cdrom_wait_queue_empty();
    g_wmap_focus_screen_position.point.x = 0x94;
    g_wmap_focus_screen_position.point.y = 0x31;
    D_8013B258 = 1;
    D_80182E38 = 4;
    D_8011D500 = 0x35;
    D_800D9228 = 0x35;
    wmap_start_sequence(&wmap_land_event_00_a_run_timeline);
    g_wmap_sequence_busy = 1;
    g_wmap_land_event_00_a_step += 1;
    wmap_land_event_00_a_wait_idle();
}

WMAP_STEP_WAIT_IDLE(wmap_land_event_00_a_wait_idle, g_wmap_land_event_00_a_step, wmap_land_event_00_a_step_05)

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void wmap_land_event_00_a_step_05(void)
{
    D_8013B294 = 1;
    D_80139228 = 0x2;
    g_wmap_land_event_00_a_step += 1;
}

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_timeline, D_800D721C, 0x16, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_timeline_reset, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

/** @brief Set the sequence colors and start its fifteen-frame countdown. */
void wmap_land_event_00_a_timeline_step_01(void)
{
    D_8013B208 = 1;
    wmap_play_sound(0x37, 0x80);
    D_80182D74.r = 0x32;
    D_80182D74.g = 0;
    D_80182D74.b = 0xA0;
    D_80182D80.r = 0x32;
    D_80182D80.g = 0;
    D_80182D80.b = 0xA0;
    D_80182D8C.r = 0;
    D_80182D8C.g = 0;
    D_80182D8C.b = 0;
    D_80182D94.r = 0;
    D_80182D94.g = 0;
    D_80182D94.b = 0;
    g_wmap_land_event_00_a_timeline_timer = 0xF;
    g_wmap_land_event_00_a_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_00_a_timeline_wait_02, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

/** @brief World-map step handler: register the next callback and advance the counter. */
void wmap_land_event_00_a_timeline_step_03(void)
{
    wmap_start_sequence(wmap_land_event_00_a_run_sequence_6);
    D_8011D500 = 0x71;
    g_wmap_land_event_00_a_timeline_timer = 0x5A;
    g_wmap_land_event_00_a_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_00_a_timeline_wait_04, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

/** @brief Set world-map color and state, register two callbacks, and begin a four-tick delay. */
void wmap_land_event_00_a_timeline_step_05(void)
{
    wmap_start_map_tint(0x903065);
    g_wmap_backdrop_target_level = 0xD;
    wmap_start_sequence(&wmap_land_event_00_a_run_sequence_5);
    wmap_start_sequence(&wmap_land_event_00_a_run_sequence_1);
    g_wmap_land_event_00_a_timeline_timer = 4;
    g_wmap_land_event_00_a_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_00_a_timeline_wait_06, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

/** @brief World-map step handler: register the next callback and advance the counter. */
void wmap_land_event_00_a_timeline_step_07(void)
{
    wmap_start_sequence(wmap_land_event_00_a_run_sequence_1);
    D_8011D500 = 0x87;
    g_wmap_land_event_00_a_timeline_timer = 0xF;
    g_wmap_land_event_00_a_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_00_a_timeline_wait_08, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_event_00_a_timeline_step_09(void)
{
    wmap_start_sequence(wmap_land_event_00_a_run_sequence_4);
    wmap_start_sequence(wmap_land_event_00_a_run_sequence_7);
    g_wmap_land_event_00_a_timeline_timer = 0x3C;
    g_wmap_land_event_00_a_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_00_a_timeline_wait_10, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_event_00_a_timeline_step_11(void)
{
    wmap_start_sequence(wmap_land_event_00_a_run_sequence_2);
    g_wmap_land_event_00_a_timeline_timer = 0x4;
    g_wmap_land_event_00_a_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_00_a_timeline_wait_12, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void wmap_land_event_00_a_timeline_step_13(void)
{
    wmap_start_sequence(wmap_land_event_00_a_run_sequence_2);
    wmap_start_sequence(wmap_land_event_00_a_run_sequence_8);
    g_wmap_land_event_00_a_timeline_timer = 0x5A;
    g_wmap_land_event_00_a_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_00_a_timeline_wait_14, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_event_00_a_timeline_step_15(void)
{
    wmap_start_sequence(wmap_land_event_00_a_run_sequence_3);
    g_wmap_land_event_00_a_timeline_timer = 0x4;
    g_wmap_land_event_00_a_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_00_a_timeline_wait_16, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void wmap_land_event_00_a_timeline_step_17(void)
{
    wmap_start_sequence(wmap_land_event_00_a_run_sequence_3);
    g_wmap_land_event_00_a_timeline_timer = 0x10;
    g_wmap_land_event_00_a_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_00_a_timeline_wait_18, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void wmap_land_event_00_a_timeline_step_19(void)
{
    D_8011D500 = 0xFF;
    g_wmap_land_event_00_a_timeline_timer = 0x40;
    g_wmap_land_event_00_a_timeline_step += 1;
}

WMAP_STEP_WAIT(wmap_land_event_00_a_timeline_wait_20, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

/**
 * @brief World-map step handler: clear the shared flag and advance the step counter.
 */
void wmap_land_event_00_a_timeline_finish(void)
{
    g_wmap_sequence_busy = 0;
    g_wmap_land_event_00_a_timeline_step += 1;
}

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_sequence_1, D_800D7274, 0x4, g_wmap_land_event_00_a_sequence_1_step, g_wmap_land_event_00_a_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_sequence_1_reset, g_wmap_land_event_00_a_sequence_1_step, g_wmap_land_event_00_a_sequence_1_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_event_00_a_sequence_1_step_01(void)
{
    D_801399AC = D_8011D538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 0x10;
    D_800D9318.target_shade = 1;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.shade = 0x81;
    g_wmap_land_event_00_a_sequence_1_timer = 0x10;
    g_wmap_land_event_00_a_sequence_1_step += 1;
    wmap_land_event_00_a_sequence_1_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_event_00_a_sequence_1_step_02(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0x2A, 0x2, 0);
    if (--g_wmap_land_event_00_a_sequence_1_timer == 0)
    {
        g_wmap_land_event_00_a_sequence_1_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_event_00_a_sequence_1_end, g_wmap_land_event_00_a_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_sequence_2, D_800D7284, 0x4, g_wmap_land_event_00_a_sequence_2_step, g_wmap_land_event_00_a_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_sequence_2_reset, g_wmap_land_event_00_a_sequence_2_step, g_wmap_land_event_00_a_sequence_2_timer)

void wmap_land_event_00_a_sequence_2_step_01(void)
{
    s32 one = 1;

    D_801399B4 = D_8011D538;
    D_800D9344.scale_index = 0xF;
    D_800D9344.sequence = one;
    D_800D9344.previous_sequence = -one;
    D_800D9344.shade_step = 4;
    D_800D9344.resource_index = 0;
    D_800D9344.target_shade = one;
    D_800D9344.shade = 0x81;
    g_wmap_land_event_00_a_sequence_2_timer = 0x10;
    g_wmap_land_event_00_a_sequence_2_step += one;
    wmap_land_event_00_a_sequence_2_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_event_00_a_sequence_2_step_02(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x2A, 0x2, 0);
    if (--g_wmap_land_event_00_a_sequence_2_timer == 0)
    {
        g_wmap_land_event_00_a_sequence_2_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_event_00_a_sequence_2_end, g_wmap_land_event_00_a_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_sequence_3, D_800D7294, 0x4, g_wmap_land_event_00_a_sequence_3_step, g_wmap_land_event_00_a_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_sequence_3_reset, g_wmap_land_event_00_a_sequence_3_step, g_wmap_land_event_00_a_sequence_3_timer)

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void wmap_land_event_00_a_sequence_3_step_01(void)
{
    D_801399BC = D_8011D538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.sequence = 2;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 0x10;
    D_800D9370.target_shade = 1;
    D_800D9370.resource_index = 0;
    D_800D9370.shade = 0x81;
    g_wmap_land_event_00_a_sequence_3_timer = 0x10;
    g_wmap_land_event_00_a_sequence_3_step += 1;
    wmap_land_event_00_a_sequence_3_step_02();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void wmap_land_event_00_a_sequence_3_step_02(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x2A, 0x2, 0);
    if (--g_wmap_land_event_00_a_sequence_3_timer == 0)
    {
        g_wmap_land_event_00_a_sequence_3_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_event_00_a_sequence_3_end, g_wmap_land_event_00_a_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_sequence_4, D_800D72A4, 0x6, g_wmap_land_event_00_a_sequence_4_step, g_wmap_land_event_00_a_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_sequence_4_reset, g_wmap_land_event_00_a_sequence_4_step, g_wmap_land_event_00_a_sequence_4_timer)

/** @brief World-map step handler: draw a framed panel and expire the step counter. */
void wmap_land_event_00_a_sequence_4_step_02(void)
{
    func_8006AEE0();
    func_8008ECF8(0x14, 0x78, D_8011CF2C, (s32)D_80139280 + 0x78);
    func_8006534C(0x25, 5);
    if (--g_wmap_land_event_00_a_sequence_4_timer == 0)
    {
        g_wmap_land_event_00_a_sequence_4_step += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void wmap_land_event_00_a_sequence_4_step_03(void)
{
    g_wmap_land_event_00_a_sequence_4_timer = 0x20;
    D_80139280[30] = 9999;
    g_wmap_land_event_00_a_sequence_4_step += 1;
    wmap_land_event_00_a_sequence_4_step_04();
}

/** @brief World-map step handler: draw a framed panel and expire the step counter. */
void wmap_land_event_00_a_sequence_4_step_04(void)
{
    func_8006AEE0();
    func_8008ECF8(0x14, 0x78, D_8011CF2C, (s32)D_80139280 + 0x78);
    func_8006534C(0x25, 5);
    if (--g_wmap_land_event_00_a_sequence_4_timer == 0)
    {
        g_wmap_land_event_00_a_sequence_4_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_event_00_a_sequence_4_end, g_wmap_land_event_00_a_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_sequence_5, D_800D72BC, 0x4, g_wmap_land_event_00_a_sequence_5_step, g_wmap_land_event_00_a_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_sequence_5_reset, g_wmap_land_event_00_a_sequence_5_step, g_wmap_land_event_00_a_sequence_5_timer)

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void wmap_land_event_00_a_sequence_5_step_01(void)
{
    D_8013B240 = D_80139258;
    D_80139888 = g_wmap_camera_translation;
    D_80182DF4 = 0x80;
    D_80139888.vz = 0xAFC8;
    g_wmap_land_event_00_a_sequence_5_timer = 0x8;
    g_wmap_land_event_00_a_sequence_5_step += 1;
    wmap_land_event_00_a_sequence_5_step_02();
}

WMAP_STEP_ADVANCE(wmap_land_event_00_a_sequence_5_end, g_wmap_land_event_00_a_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_sequence_6, D_800D72CC, 0x6, g_wmap_land_event_00_a_sequence_6_step, g_wmap_land_event_00_a_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_sequence_6_reset, g_wmap_land_event_00_a_sequence_6_step, g_wmap_land_event_00_a_sequence_6_timer)

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void wmap_land_event_00_a_sequence_6_step_01(void)
{
    D_80182DE4 = 1;
    D_801B2498 = D_80139258;
    D_8013923C = 0;
    g_wmap_land_event_00_a_sequence_6_timer = 0x159;
    g_wmap_land_event_00_a_sequence_6_step += 1;
    wmap_land_event_00_a_sequence_6_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_event_00_a_sequence_6_step_03(void)
{
    g_wmap_land_event_00_a_sequence_6_timer = 0x8;
    g_wmap_land_event_00_a_sequence_6_step += 1;
    wmap_land_event_00_a_sequence_6_step_04();
}

WMAP_STEP_ADVANCE(wmap_land_event_00_a_sequence_6_end, g_wmap_land_event_00_a_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_sequence_7, D_800D72E4, 0x6, g_wmap_land_event_00_a_sequence_7_step, g_wmap_land_event_00_a_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_sequence_7_reset, g_wmap_land_event_00_a_sequence_7_step, g_wmap_land_event_00_a_sequence_7_timer)

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void wmap_land_event_00_a_sequence_7_step_01(void)
{
    D_80182DE8 = 1;
    D_801B24A0 = D_80139258;
    D_80139240 = 0;
    g_wmap_land_event_00_a_sequence_7_timer = 0xE9;
    g_wmap_land_event_00_a_sequence_7_step += 1;
    wmap_land_event_00_a_sequence_7_step_02();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void wmap_land_event_00_a_sequence_7_step_03(void)
{
    g_wmap_land_event_00_a_sequence_7_timer = 0x8;
    g_wmap_land_event_00_a_sequence_7_step += 1;
    wmap_land_event_00_a_sequence_7_step_04();
}

WMAP_STEP_ADVANCE(wmap_land_event_00_a_sequence_7_end, g_wmap_land_event_00_a_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_sequence_8, D_800D72FC, 0x6, g_wmap_land_event_00_a_sequence_8_step, g_wmap_land_event_00_a_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_sequence_8_reset, g_wmap_land_event_00_a_sequence_8_step, g_wmap_land_event_00_a_sequence_8_timer)

void wmap_land_event_00_a_sequence_8_step_01(void)
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
    g_wmap_land_event_00_a_sequence_8_timer = 0xA7;
    g_wmap_land_event_00_a_sequence_8_step++;
    wmap_land_event_00_a_sequence_8_step_02();
}

void wmap_land_event_00_a_sequence_8_step_03(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    g_wmap_land_event_00_a_sequence_8_timer = 8;
    g_wmap_land_event_00_a_sequence_8_step++;
    wmap_land_event_00_a_sequence_8_step_04();
}

void wmap_land_event_00_a_sequence_8_step_05(void)
{
void wmap_land_effect_31_sequence_1_step_04(void);
void wmap_land_effect_31_sequence_2_step_02(void);
void wmap_land_effect_31_sequence_3_step_02(void);
void wmap_land_effect_31_sequence_3_step_04(void);
void wmap_land_effect_31_step_02(void);
void wmap_land_effect_31_step_03(void);
void wmap_land_effect_31_step_04(void);
void wmap_land_effect_31_step_05(void);

    g_wmap_land_event_00_a_sequence_8_step++;
}
