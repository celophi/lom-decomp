#include "../internal/wmap_model_render.h"
#include "../internal/wmap_land_event_00_a.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "main/cdrom.h"
#include <libgte.h>
#include "../internal/wmap_view_effects.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_effect_resources.h"
#include "../internal/wmap_step_sequence.h"

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

extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern u8 D_800DEF18[];
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 g_wmap_land_event_00_a_sequence_4_timer;
extern void wmap_land_event_00_a_sequence_4_step_02(void);
extern s32 g_wmap_land_event_00_a_sequence_5_timer;
extern s32 D_8013923C;
extern s32 D_80182DE4;
extern s32 g_wmap_land_event_00_a_sequence_6_timer;
extern s32 D_80139240;
extern s32 g_wmap_land_event_00_a_sequence_7_timer;
extern s32 D_8013924C;
extern s32 g_wmap_land_event_00_a_sequence_8_timer;
extern s32 g_wmap_land_event_00_a_timer;
extern void (*D_800D7204[])(void);
extern s32 D_800D9228;
extern s32 D_8011D500;
extern s32 D_80182E38;
extern s32 D_8013B294;
extern s32 D_80139228;
extern s32 g_wmap_land_event_00_a_timeline_timer;
extern void (*D_800D721C[])(void);
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
extern u8 g_wmap_animation_bank_0[];
extern u8 *g_wmap_effect_model_pack_1;
extern u8 *g_wmap_effect_model_pack_2;
extern s32* g_wmap_effect_model_pack_3;
extern s32* D_8011CF2C;
extern u32 g_wmap_land_event_00_a_step;
extern u8 g_wmap_animation_bank_2;
extern u8 g_wmap_actor_motions[];
extern u32 g_wmap_land_event_00_a_sequence_4_step;
extern s32 g_wmap_load_buffer[];
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
extern u32 g_wmap_land_event_00_a_sequence_2_step;
extern u32 g_wmap_land_event_00_a_sequence_3_step;
extern void func_8008ECF8(s32, s32, s32*, void*);

extern VECTOR g_wmap_camera_translation;

/** @brief Map scroll position (map units) and projection scale. */
typedef struct
{
    s32 x;
    s32 y;
    s32 projection_scale;
    s32 unknown_0c;
} WmapView;

extern WmapView g_wmap_view;

extern SVECTOR D_801B2498;


extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapScreenPosition g_wmap_focus_screen_position;

extern s32* g_wmap_effect_params;

/** @brief Queue effect resources and establish the map-relative effect position. */
void wmap_land_event_00_a_step_01(void)
{
    g_wmap_effect_model_pack_1 = D_800DEF18;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x7000;
    g_wmap_effect_model_pack_3 = g_wmap_effect_model_pack_2 + 0x3000;
    D_8011CF2C = g_wmap_effect_model_pack_2 + 0x7000;
    func_80064F64(0x1218);
    func_80064F64(0x1219);
    func_80064F64(0x121A);
    cdrom_queue_read(0x121B, g_wmap_animation_bank_0);
    cdrom_queue_read(0x121C, D_800DEF18 - 0x2000);
    cdrom_queue_read(0x121D, g_wmap_effect_model_pack_1);
    cdrom_queue_read(0x121E, g_wmap_effect_model_pack_2);
    cdrom_queue_read(0x121F, g_wmap_effect_model_pack_3);
    g_wmap_sprite_actors[0].resource_index = -1;
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
    resource = &g_wmap_animation_bank_2;
    resource_cursor = (u8 *)&g_wmap_actor_animations + index * 8;
    slot = (s16 *)((u8 *)&g_wmap_actor_motions + index * 20);
    do
    {
        *slot = 0;
        *(void **)(resource_cursor + 4) = resource;
        resource_cursor += 8;
        index += 1;
        slot += 0xA;
    } while (index < 0x78);
    g_wmap_land_event_00_a_sequence_4_timer = 0xE1;
    actor = (u8*)g_wmap_effect_params;
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
 * @brief Move the model along Z, draw it, and fade it until the timer expires.
 */
WMAP_STEP_DROP_UPDATE_WITH_DRAW(wmap_land_event_00_a_sequence_5_step_02,
    g_wmap_land_event_00_a_sequence_5_step, g_wmap_land_event_00_a_sequence_5_timer,
    g_wmap_effect_model_d_rotation, g_wmap_effect_model_d_position, g_wmap_effect_fade_d,
    -0xDAC, 0x20,
    wmap_draw_model(g_wmap_load_buffer, 0, 0x4, 0x35, 0x7800, 1, g_wmap_effect_fade_d, 0, 0, -1))

/**
 * @brief Draw and spin the model while fading it in.
 */
void wmap_land_event_00_a_sequence_6_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model(g_wmap_effect_model_pack_1, (D_8013923C / 0x10) & 3, 0xA, 0x35, 0x7800, 0x1, D_80182DE4, 0, -0xA, -1);
    D_8013923C += 0x8;
    WMAP_MODEL_FADE_IN(D_80182DE4, 0x2, 0x81, value);
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
 * @brief Draw and spin the model while fading it out.
 */
void wmap_land_event_00_a_sequence_6_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &D_801B2498);
    wmap_draw_model(g_wmap_effect_model_pack_1, (D_8013923C / 0x10) & 3, 0xA, 0x35, 0x7800, 0x1, D_80182DE4, 0, -0xA, -1);
    WMAP_MODEL_FADE_OUT(D_80182DE4, 0x10, value);
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
 * @brief Draw and spin the model while fading it in.
 */
void wmap_land_event_00_a_sequence_7_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_a_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_2, (D_80139240 / 0x10) & 3, 0xA, 0x35, 0x7840, 0x1, g_wmap_effect_fade_a, 0, -0xA, -1);
    D_80139240 += 0x10;
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_a, 0x8, 0x81, value);
    timer = g_wmap_land_event_00_a_sequence_7_timer;
    g_wmap_effect_model_a_rotation.vz += 0x20;
    next_timer = timer - 1;
    g_wmap_land_event_00_a_sequence_7_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_event_00_a_sequence_7_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it out.
 */
void wmap_land_event_00_a_sequence_7_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_a_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_2, (D_80139240 / 0x10) & 3, 0xA, 0x35, 0x7840, 0x1, g_wmap_effect_fade_a, 0, -0xA, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_a, 0x10, value);
    D_80139240 += 0x10;
    timer = g_wmap_land_event_00_a_sequence_7_timer;
    g_wmap_effect_model_a_rotation.vz += 0x20;
    next_timer = timer - 1;
    g_wmap_land_event_00_a_sequence_7_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_event_00_a_sequence_7_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it in.
 */
void wmap_land_event_00_a_sequence_8_step_02(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_b_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, (D_8013924C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1, g_wmap_effect_fade_b, 0, -0xA, -1);
    D_8013924C += 0x10;
    WMAP_MODEL_FADE_IN(g_wmap_effect_fade_b, 0x8, 0x81, value);
    timer = g_wmap_land_event_00_a_sequence_8_timer;
    g_wmap_effect_model_b_rotation.vz += 0x8;
    next_timer = timer - 1;
    g_wmap_land_event_00_a_sequence_8_timer = next_timer;
    if (next_timer == 0)
    {
        g_wmap_land_event_00_a_sequence_8_step++;
    }
}

/**
 * @brief Draw and spin the model while fading it out.
 */
void wmap_land_event_00_a_sequence_8_step_04(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(&g_wmap_camera_translation, &g_wmap_effect_model_b_rotation);
    wmap_draw_model(g_wmap_effect_model_pack_3, (D_8013924C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1, g_wmap_effect_fade_b, 0, -0xA, -1);
    WMAP_MODEL_FADE_OUT(g_wmap_effect_fade_b, 0x10, value);
    D_8013924C += 0x10;
    timer = g_wmap_land_event_00_a_sequence_8_timer;
    g_wmap_effect_model_b_rotation.vz += 0x8;
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
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(wmap_land_event_00_a_step_02,
    g_wmap_land_event_00_a_step, wmap_land_event_00_a_step_03)

/** @brief Wait for CD work, initialize sequence state, and register its callback. */
void wmap_land_event_00_a_step_03(void)
{
    cdrom_wait_queue_empty();
    g_wmap_focus_screen_position.point.x = 0x94;
    g_wmap_focus_screen_position.point.y = 0x31;
    g_wmap_auxiliary_labels_hidden = 1;
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
 * @brief Start the world-map exit and advance the sequence.
 */
WMAP_STEP_BEGIN_EXIT(wmap_land_event_00_a_step_05, g_wmap_land_event_00_a_step, 0x2)

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_timeline, D_800D721C, 0x16, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_timeline_reset, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

/** @brief Set the sequence colors and start its fifteen-frame countdown. */
void wmap_land_event_00_a_timeline_step_01(void)
{
    g_wmap_event_active = 1;
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

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_event_00_a_timeline_step_09, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer,
                             wmap_land_event_00_a_run_sequence_4, wmap_land_event_00_a_run_sequence_7, 0x3C)

WMAP_STEP_WAIT(wmap_land_event_00_a_timeline_wait_10, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_event_00_a_timeline_step_11, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer,
                         wmap_land_event_00_a_run_sequence_2, 0x4)

WMAP_STEP_WAIT(wmap_land_event_00_a_timeline_wait_12, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

WMAP_STEP_START_TWO_AND_WAIT(wmap_land_event_00_a_timeline_step_13, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer,
                             wmap_land_event_00_a_run_sequence_2, wmap_land_event_00_a_run_sequence_8, 0x5A)

WMAP_STEP_WAIT(wmap_land_event_00_a_timeline_wait_14, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_event_00_a_timeline_step_15, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer,
                         wmap_land_event_00_a_run_sequence_3, 0x4)

WMAP_STEP_WAIT(wmap_land_event_00_a_timeline_wait_16, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer)

WMAP_STEP_START_AND_WAIT(wmap_land_event_00_a_timeline_step_17, g_wmap_land_event_00_a_timeline_step, g_wmap_land_event_00_a_timeline_timer,
                         wmap_land_event_00_a_run_sequence_3, 0x10)

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
 * @brief Clear the blocking flag and finish the timeline.
 */
WMAP_STEP_FINISH_BLOCKING(wmap_land_event_00_a_timeline_finish,
    g_wmap_land_event_00_a_timeline_step)

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_sequence_1, D_800D7274, 0x4, g_wmap_land_event_00_a_sequence_1_step, g_wmap_land_event_00_a_sequence_1_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_sequence_1_reset, g_wmap_land_event_00_a_sequence_1_step, g_wmap_land_event_00_a_sequence_1_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_event_00_a_sequence_1_step_01,
    g_wmap_land_event_00_a_sequence_1_step, g_wmap_land_event_00_a_sequence_1_timer,
    4, g_wmap_animation_bank_0, 0,
    0x81, 1, 0x10,
    0x10, wmap_land_event_00_a_sequence_1_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_event_00_a_sequence_1_step_02, g_wmap_land_event_00_a_sequence_1_step, g_wmap_land_event_00_a_sequence_1_timer,
                              g_wmap_sprite_actors[4], g_wmap_actor_animations[4], g_wmap_focus_screen_position, 0x2A, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_event_00_a_sequence_1_end, g_wmap_land_event_00_a_sequence_1_step)

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_sequence_2, D_800D7284, 0x4, g_wmap_land_event_00_a_sequence_2_step, g_wmap_land_event_00_a_sequence_2_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_sequence_2_reset, g_wmap_land_event_00_a_sequence_2_step, g_wmap_land_event_00_a_sequence_2_timer)

void wmap_land_event_00_a_sequence_2_step_01(void)
{
    s32 one = 1;
    WmapSpriteActor* actor = &g_wmap_sprite_actors[5];

    g_wmap_actor_animations[5].data = g_wmap_animation_bank_0;
    actor->scale_index = 0xF;
    actor->sequence = one;
    actor->previous_sequence = -one;
    actor->shade_step = 4;
    actor->resource_index = 0;
    actor->target_shade = one;
    actor->shade = 0x81;
    g_wmap_land_event_00_a_sequence_2_timer = 0x10;
    g_wmap_land_event_00_a_sequence_2_step += one;
    wmap_land_event_00_a_sequence_2_step_02();
}

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_event_00_a_sequence_2_step_02, g_wmap_land_event_00_a_sequence_2_step, g_wmap_land_event_00_a_sequence_2_timer,
                              g_wmap_sprite_actors[5], g_wmap_actor_animations[5], g_wmap_focus_screen_position, 0x2A, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_event_00_a_sequence_2_end, g_wmap_land_event_00_a_sequence_2_step)

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_sequence_3, D_800D7294, 0x4, g_wmap_land_event_00_a_sequence_3_step, g_wmap_land_event_00_a_sequence_3_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_sequence_3_reset, g_wmap_land_event_00_a_sequence_3_step, g_wmap_land_event_00_a_sequence_3_timer)

/**
 * @brief Start the sprite animation and run its first update.
 */
WMAP_STEP_START_ACTOR(wmap_land_event_00_a_sequence_3_step_01,
    g_wmap_land_event_00_a_sequence_3_step, g_wmap_land_event_00_a_sequence_3_timer,
    6, g_wmap_animation_bank_0, 2,
    0x81, 1, 0x10,
    0x10, wmap_land_event_00_a_sequence_3_step_02)

WMAP_STEP_DRAW_ACTOR_AND_WAIT(wmap_land_event_00_a_sequence_3_step_02, g_wmap_land_event_00_a_sequence_3_step, g_wmap_land_event_00_a_sequence_3_timer,
                              g_wmap_sprite_actors[6], g_wmap_actor_animations[6], g_wmap_focus_screen_position, 0x2A, 0x2, 0)

WMAP_STEP_ADVANCE(wmap_land_event_00_a_sequence_3_end, g_wmap_land_event_00_a_sequence_3_step)

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_sequence_4, D_800D72A4, 0x6, g_wmap_land_event_00_a_sequence_4_step, g_wmap_land_event_00_a_sequence_4_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_sequence_4_reset, g_wmap_land_event_00_a_sequence_4_step, g_wmap_land_event_00_a_sequence_4_timer)

/** @brief World-map step handler: draw a framed panel and expire the step counter. */
void wmap_land_event_00_a_sequence_4_step_02(void)
{
    func_8006AEE0();
    func_8008ECF8(0x14, 0x78, D_8011CF2C, &g_wmap_effect_params[30]);
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
    g_wmap_effect_params[30] = 9999;
    g_wmap_land_event_00_a_sequence_4_step += 1;
    wmap_land_event_00_a_sequence_4_step_04();
}

/** @brief World-map step handler: draw a framed panel and expire the step counter. */
void wmap_land_event_00_a_sequence_4_step_04(void)
{
    func_8006AEE0();
    func_8008ECF8(0x14, 0x78, D_8011CF2C, &g_wmap_effect_params[30]);
    func_8006534C(0x25, 5);
    if (--g_wmap_land_event_00_a_sequence_4_timer == 0)
    {
        g_wmap_land_event_00_a_sequence_4_step += 1;
    }
}

WMAP_STEP_ADVANCE(wmap_land_event_00_a_sequence_4_end, g_wmap_land_event_00_a_sequence_4_step)

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_sequence_5, D_800D72BC, 0x4, g_wmap_land_event_00_a_sequence_5_step, g_wmap_land_event_00_a_sequence_5_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_sequence_5_reset, g_wmap_land_event_00_a_sequence_5_step, g_wmap_land_event_00_a_sequence_5_timer)

WMAP_STEP_DROP_START(wmap_land_event_00_a_sequence_5_step_01, g_wmap_land_event_00_a_sequence_5_step, g_wmap_land_event_00_a_sequence_5_timer, g_wmap_effect_model_d_rotation,
                     g_wmap_zero_rotation, g_wmap_effect_model_d_position, g_wmap_effect_fade_d, 0x80, 0xAFC8, 0x8, wmap_land_event_00_a_sequence_5_step_02)

WMAP_STEP_ADVANCE(wmap_land_event_00_a_sequence_5_end, g_wmap_land_event_00_a_sequence_5_step)

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_sequence_6, D_800D72CC, 0x6, g_wmap_land_event_00_a_sequence_6_step, g_wmap_land_event_00_a_sequence_6_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_sequence_6_reset, g_wmap_land_event_00_a_sequence_6_step, g_wmap_land_event_00_a_sequence_6_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_event_00_a_sequence_6_step_01,
    g_wmap_land_event_00_a_sequence_6_step, g_wmap_land_event_00_a_sequence_6_timer,
    D_801B2498, g_wmap_zero_rotation,
    D_80182DE4, 1, D_8013923C,
    0x159, wmap_land_event_00_a_sequence_6_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_event_00_a_sequence_6_step_03, g_wmap_land_event_00_a_sequence_6_step, g_wmap_land_event_00_a_sequence_6_timer, 0x8,
                    wmap_land_event_00_a_sequence_6_step_04)

WMAP_STEP_ADVANCE(wmap_land_event_00_a_sequence_6_end, g_wmap_land_event_00_a_sequence_6_step)

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_sequence_7, D_800D72E4, 0x6, g_wmap_land_event_00_a_sequence_7_step, g_wmap_land_event_00_a_sequence_7_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_sequence_7_reset, g_wmap_land_event_00_a_sequence_7_step, g_wmap_land_event_00_a_sequence_7_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_event_00_a_sequence_7_step_01,
    g_wmap_land_event_00_a_sequence_7_step, g_wmap_land_event_00_a_sequence_7_timer,
    g_wmap_effect_model_a_rotation, g_wmap_zero_rotation,
    g_wmap_effect_fade_a, 1, D_80139240,
    0xE9, wmap_land_event_00_a_sequence_7_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_event_00_a_sequence_7_step_03, g_wmap_land_event_00_a_sequence_7_step, g_wmap_land_event_00_a_sequence_7_timer, 0x8,
                    wmap_land_event_00_a_sequence_7_step_04)

WMAP_STEP_ADVANCE(wmap_land_event_00_a_sequence_7_end, g_wmap_land_event_00_a_sequence_7_step)

WMAP_STEP_RUNNER(wmap_land_event_00_a_run_sequence_8, D_800D72FC, 0x6, g_wmap_land_event_00_a_sequence_8_step, g_wmap_land_event_00_a_sequence_8_timer)

WMAP_STEP_RESET(wmap_land_event_00_a_sequence_8_reset, g_wmap_land_event_00_a_sequence_8_step, g_wmap_land_event_00_a_sequence_8_timer)

/**
 * @brief Set the model shade, reset its animation and rotation, and run the first update.
 */
WMAP_STEP_START_MODEL(wmap_land_event_00_a_sequence_8_step_01,
    g_wmap_land_event_00_a_sequence_8_step, g_wmap_land_event_00_a_sequence_8_timer,
    g_wmap_effect_model_b_rotation, g_wmap_zero_rotation,
    g_wmap_effect_fade_b, 1, D_8013924C,
    0xA7, wmap_land_event_00_a_sequence_8_step_02)

WMAP_STEP_ARM_TIMER(wmap_land_event_00_a_sequence_8_step_03, g_wmap_land_event_00_a_sequence_8_step, g_wmap_land_event_00_a_sequence_8_timer, 8,
                    wmap_land_event_00_a_sequence_8_step_04)

WMAP_STEP_ADVANCE(wmap_land_event_00_a_sequence_8_step_05, g_wmap_land_event_00_a_sequence_8_step)
