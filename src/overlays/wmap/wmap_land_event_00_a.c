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

void func_800B2110(void);
void func_800B2150(void);
void func_800B21DC(void);
s32 func_800B2244(s32 arg0);
void func_800B2218(void);
s32 func_800B3020(s32 arg0);
s32 func_800B27F8(s32 arg0);
s32 func_800B2EC8(s32 arg0);
s32 func_800B2CE8(s32 arg0);
s32 func_800B317C(s32 arg0);
s32 func_800B299C(s32 arg0);
s32 func_800B32D8(s32 arg0);
s32 func_800B2B40(s32 arg0);
void func_800B2908(void);
void func_800B2C54(void);
void func_800B2E38(void);

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
extern s32 D_801B2FBC;
extern void func_800B2D78(void);
extern VECTOR D_8011CF60;
extern s32 D_80182DF4;
extern s32 D_801B2FC4;
extern s32 D_8013923C;
extern s32 D_80182DE4;
extern s32 D_801B2FCC;
extern s32 D_80139240;
extern s32 D_80182DE8;
extern s32 D_801B2FD4;
extern s32 D_8013924C;
extern s32 D_80182DEC;
extern s32 D_801B2FDC;
extern s32 D_801B2F94;
extern WmapStepHandlerSlot D_800D7204[];
extern s32 D_800D9228;
extern s32 D_8011D500;
extern s32 D_8013B20C;
extern s32 D_8013B258;
extern s32 D_80182E38;
extern s32 D_8013B294;
extern s32 D_80139228;
extern s32 D_801B2F9C;
extern WmapStepHandlerSlot D_800D721C[];
extern s32 D_8013B208;
extern s32 D_801B2FA4;
extern WmapStepHandlerSlot D_800D7274[];
extern void func_800B2908(void);
extern s32 D_801B2FAC;
extern WmapStepHandlerSlot D_800D7284[];
extern void func_800B2AAC(void);
extern s32 D_801B2FB4;
extern WmapStepHandlerSlot D_800D7294[];
extern WmapStepHandlerSlot D_800D72A4[];
extern WmapStepHandlerSlot D_800D72BC[];
extern WmapStepHandlerSlot D_800D72CC[];
extern WmapStepHandlerSlot D_800D72E4[];
extern WmapStepHandlerSlot D_800D72FC[];
extern u8 D_8011D538[];
extern u8_ptr D_8011CF1C;
extern u8_ptr D_8011CF24;
extern s32_ptr D_8011CF28;
extern s32_ptr D_8011CF2C;
extern u32 D_801B2F90;
extern u8 D_80121538;
extern u8 D_801AFBD0[];
extern u32 D_801B2FB8;
extern s32 D_800DCF18[];
extern u32 D_801B2FC0;
extern u32 D_801B2FC8;
extern u32 D_801B2FD0;
extern u32 D_801B2FD8;
extern u32 D_801B2F98;
extern WmapColor D_80182D74;
extern WmapColor D_80182D80;
extern WmapColor D_80182D8C;
extern WmapColor D_80182D94;
extern u32 D_801B2FA0;
extern u8_ptr D_801399AC;
extern u32 D_801B2FA8;
extern void_ptr D_801399B4;
extern u32 D_801B2FB0;
extern void_ptr D_801399BC;
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

extern s32_ptr D_80139280;

/** @brief Queue effect resources and establish the map-relative effect position. */
void func_800B1744(void)
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
    D_801B2F90++;
    func_800B2110();
}

/**
 * @brief World-map step handler: seed a 100-entry table, init an actor, advance.
 */
void func_800B1898(void)
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
    D_801B2FBC = 0xE1;
    actor = (u8*)D_80139280;
    *(s32 *)(actor + 0x7C) = setting = 5;
    *(s32 *)(actor + 0x8C) = setting;
    ((WmapEventActorState*)actor)->state = 1;
    *(s32 *)(actor + 0x7C) = setting;
    current = D_801B2FB8;
    *(s32 *)(actor + 0x80) = -0x3E8;
    *(s32 *)(actor + 0x84) = 0x1770;
    *(s32 *)(actor + 0x88) = 0x1388;
    *(s32 *)(actor + 0x90) = -0x64;
    *(s32 *)(actor + 0x98) = -0x12C;
    *(s32 *)(actor + 0x94) = 0;
    *(s32 *)(actor + 0xA0) = -1;
    *(s32 *)(actor + 0xA4) = 0;
    D_801B2FB8 = current + 1;
    func_800B2D78();
}

/**
 * @brief World-map step handler: advance the model's spin toward a floor, draw it
 *        while active, then countdown-advance the step.
 */
void func_800B1964(void)
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
    if (--D_801B2FC4 == 0)
    {
        D_801B2FC0 += 1;
    }
}

/**
 * @brief World-map step handler: render the animated actor, ramp its size up to a
 *        cap, scroll the shadow field, then advance when the frame counter expires.
 */
void func_800B1A74(void)
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
    timer = D_801B2FCC;
    D_801B2498.vz += 0x4;
    next_timer = timer - 1;
    D_801B2FCC = next_timer;
    if (next_timer == 0)
    {
        D_801B2FC8++;
    }
}

/**
 * @brief World-map step handler: render the animated actor, ramp its size down to a
 *        floor, scroll the shadow field, then advance when the frame counter expires.
 */
void func_800B1B78(void)
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
    timer = D_801B2FCC;
    D_801B2498.vz += 0x4;
    next_timer = timer - 1;
    D_801B2FCC = next_timer;
    if (next_timer == 0)
    {
        D_801B2FC8++;
    }
}

/**
 * @brief World-map step handler: render the animated actor, ramp its size up to a
 *        cap, scroll the shadow field, then advance when the frame counter expires.
 */
void func_800B1C78(void)
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
    timer = D_801B2FD4;
    D_801B24A0.vz += 0x20;
    next_timer = timer - 1;
    D_801B2FD4 = next_timer;
    if (next_timer == 0)
    {
        D_801B2FD0++;
    }
}

/**
 * @brief World-map step handler: render the animated actor, ramp its size down to a
 *        floor, scroll the shadow field, then advance when the frame counter expires.
 */
void func_800B1D7C(void)
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
    timer = D_801B2FD4;
    D_801B24A0.vz += 0x20;
    next_timer = timer - 1;
    D_801B2FD4 = next_timer;
    if (next_timer == 0)
    {
        D_801B2FD0++;
    }
}

/**
 * @brief World-map step handler: render the animated actor, ramp its size up to a
 *        cap, scroll the shadow field, then advance when the frame counter expires.
 */
void func_800B1E7C(void)
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
    timer = D_801B2FDC;
    D_801B24A8.vz += 0x8;
    next_timer = timer - 1;
    D_801B2FDC = next_timer;
    if (next_timer == 0)
    {
        D_801B2FD8++;
    }
}

/**
 * @brief World-map step handler: render the animated actor, ramp its size down to a
 *        floor, scroll the shadow field, then advance when the frame counter expires.
 */
void func_800B1F80(void)
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
    timer = D_801B2FDC;
    D_801B24A8.vz += 0x8;
    next_timer = timer - 1;
    D_801B2FDC = next_timer;
    if (next_timer == 0)
    {
        D_801B2FD8++;
    }
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B2080(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F90 = 1;
        D_801B2F94 = 1;
        return 1;
    }

    if (D_801B2F90 < 0x6)
    {
        PS1_CALL(D_800D7204[D_801B2F90])();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B20F8(void)
{
    D_801B2F90 = 1;
    D_801B2F94 = 1;
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800B2110(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B2F90 += 1;
        func_800B2150();
    }
}

/** @brief Wait for CD work, initialize sequence state, and register its callback. */
void func_800B2150(void)
{
    cdrom_wait_queue_empty();
    g_wmap_focus_screen_position.point.x = 0x94;
    g_wmap_focus_screen_position.point.y = 0x31;
    D_8013B258 = 1;
    D_80182E38 = 4;
    D_8011D500 = 0x35;
    D_800D9228 = 0x35;
    wmap_start_sequence(&func_800B2244);
    D_8013B20C = 1;
    D_801B2F90 += 1;
    func_800B21DC();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800B21DC(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2F90 += 1;
        func_800B2218();
    }
}

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void func_800B2218(void)
{
    D_8013B294 = 1;
    D_80139228 = 0x2;
    D_801B2F90 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B2244(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F98 = 1;
        D_801B2F9C = 1;
        return 1;
    }

    if (D_801B2F98 < 0x16)
    {
        PS1_CALL(D_800D721C[D_801B2F98])();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B22BC(void)
{
    D_801B2F98 = 1;
    D_801B2F9C = 1;
}

/** @brief Set the sequence colors and start its fifteen-frame countdown. */
void func_800B22D4(void)
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
    D_801B2F9C = 0xF;
    D_801B2F98 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B2374(void)
{
    if (--D_801B2F9C == 0)
    {
        D_801B2F98 += 1;
    }
}

/** @brief World-map step handler: register the next callback and advance the counter. */
void func_800B23A8(void)
{
    wmap_start_sequence(func_800B3020);
    D_8011D500 = 0x71;
    D_801B2F9C = 0x5A;
    D_801B2F98 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B23F0(void)
{
    if (--D_801B2F9C == 0)
    {
        D_801B2F98 += 1;
    }
}

/** @brief Set world-map color and state, register two callbacks, and begin a four-tick delay. */
void func_800B2424(void)
{
    wmap_start_map_tint(0x903065);
    g_wmap_backdrop_target_level = 0xD;
    wmap_start_sequence(&func_800B2EC8);
    wmap_start_sequence(&func_800B27F8);
    D_801B2F9C = 4;
    D_801B2F98 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B2484(void)
{
    if (--D_801B2F9C == 0)
    {
        D_801B2F98 += 1;
    }
}

/** @brief World-map step handler: register the next callback and advance the counter. */
void func_800B24B8(void)
{
    wmap_start_sequence(func_800B27F8);
    D_8011D500 = 0x87;
    D_801B2F9C = 0xF;
    D_801B2F98 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B2500(void)
{
    if (--D_801B2F9C == 0)
    {
        D_801B2F98 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800B2534(void)
{
    wmap_start_sequence(func_800B2CE8);
    wmap_start_sequence(func_800B317C);
    D_801B2F9C = 0x3C;
    D_801B2F98 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B257C(void)
{
    if (--D_801B2F9C == 0)
    {
        D_801B2F98 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800B25B0(void)
{
    wmap_start_sequence(func_800B299C);
    D_801B2F9C = 0x4;
    D_801B2F98 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B25EC(void)
{
    if (--D_801B2F9C == 0)
    {
        D_801B2F98 += 1;
    }
}

/**
 * @brief Register two sequence steps, arm the frame timer, and advance the counter.
 */
void func_800B2620(void)
{
    wmap_start_sequence(func_800B299C);
    wmap_start_sequence(func_800B32D8);
    D_801B2F9C = 0x5A;
    D_801B2F98 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B2668(void)
{
    if (--D_801B2F9C == 0)
    {
        D_801B2F98 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800B269C(void)
{
    wmap_start_sequence(func_800B2B40);
    D_801B2F9C = 0x4;
    D_801B2F98 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B26D8(void)
{
    if (--D_801B2F9C == 0)
    {
        D_801B2F98 += 1;
    }
}

/**
 * @brief Register the next sequence step, arm its frame timer, and advance the counter.
 */
void func_800B270C(void)
{
    wmap_start_sequence(func_800B2B40);
    D_801B2F9C = 0x10;
    D_801B2F98 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B2748(void)
{
    if (--D_801B2F9C == 0)
    {
        D_801B2F98 += 1;
    }
}

/**
 * @brief World-map step handler: set flags and advance the step counter.
 */
void func_800B277C(void)
{
    D_8011D500 = 0xFF;
    D_801B2F9C = 0x40;
    D_801B2F98 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B27A8(void)
{
    if (--D_801B2F9C == 0)
    {
        D_801B2F98 += 1;
    }
}

/**
 * @brief World-map step handler: clear the shared flag and advance the step counter.
 */
void func_800B27DC(void)
{
    D_8013B20C = 0;
    D_801B2F98 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B27F8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2FA0 = 1;
        D_801B2FA4 = 1;
        return 1;
    }

    if (D_801B2FA0 < 0x4)
    {
        PS1_CALL(D_800D7274[D_801B2FA0])();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B2870(void)
{
    D_801B2FA0 = 1;
    D_801B2FA4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800B2888(void)
{
    D_801399AC = D_8011D538;
    D_800D9318.scale_index = 0xF;
    D_800D9318.previous_sequence = -1;
    D_800D9318.shade_step = 0x10;
    D_800D9318.target_shade = 1;
    D_800D9318.resource_index = 0;
    D_800D9318.sequence = 0;
    D_800D9318.shade = 0x81;
    D_801B2FA4 = 0x10;
    D_801B2FA0 += 1;
    func_800B2908();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800B2908(void)
{
    wmap_step_actor_animation(&D_800D9318, &D_801399A8);
    wmap_draw_actor_sprite(&D_800D9318, g_wmap_focus_screen_position.packed, 0x2A, 0x2, 0);
    if (--D_801B2FA4 == 0)
    {
        D_801B2FA0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800B2984(void)
{
    D_801B2FA0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B299C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2FA8 = 1;
        D_801B2FAC = 1;
        return 1;
    }

    if (D_801B2FA8 < 0x4)
    {
        PS1_CALL(D_800D7284[D_801B2FA8])();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B2A14(void)
{
    D_801B2FA8 = 1;
    D_801B2FAC = 1;
}

void func_800B2A2C(void)
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
    D_801B2FAC = 0x10;
    D_801B2FA8 += one;
    func_800B2AAC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800B2AAC(void)
{
    wmap_step_actor_animation(&D_800D9344, &D_801399B0);
    wmap_draw_actor_sprite(&D_800D9344, g_wmap_focus_screen_position.packed, 0x2A, 0x2, 0);
    if (--D_801B2FAC == 0)
    {
        D_801B2FA8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800B2B28(void)
{
    D_801B2FA8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B2B40(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2FB0 = 1;
        D_801B2FB4 = 1;
        return 1;
    }

    if (D_801B2FB0 < 0x4)
    {
        PS1_CALL(D_800D7294[D_801B2FB0])();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B2BB8(void)
{
    D_801B2FB0 = 1;
    D_801B2FB4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800B2BD0(void)
{
    D_801399BC = D_8011D538;
    D_800D9370.scale_index = 0xF;
    D_800D9370.sequence = 2;
    D_800D9370.previous_sequence = -1;
    D_800D9370.shade_step = 0x10;
    D_800D9370.target_shade = 1;
    D_800D9370.resource_index = 0;
    D_800D9370.shade = 0x81;
    D_801B2FB4 = 0x10;
    D_801B2FB0 += 1;
    func_800B2C54();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800B2C54(void)
{
    wmap_step_actor_animation(&D_800D9370, &D_801399B8);
    wmap_draw_actor_sprite(&D_800D9370, g_wmap_focus_screen_position.packed, 0x2A, 0x2, 0);
    if (--D_801B2FB4 == 0)
    {
        D_801B2FB0 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800B2CD0(void)
{
    D_801B2FB0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B2CE8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2FB8 = 1;
        D_801B2FBC = 1;
        return 1;
    }

    if (D_801B2FB8 < 0x6)
    {
        PS1_CALL(D_800D72A4[D_801B2FB8])();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B2D60(void)
{
    D_801B2FB8 = 1;
    D_801B2FBC = 1;
}

/** @brief World-map step handler: draw a framed panel and expire the step counter. */
void func_800B2D78(void)
{
    func_8006AEE0();
    func_8008ECF8(0x14, 0x78, D_8011CF2C, (s32)D_80139280 + 0x78);
    func_8006534C(0x25, 5);
    if (--D_801B2FBC == 0)
    {
        D_801B2FB8 += 1;
    }
}

/**
 * @brief Set a sequence parameter, initialise one object field, advance, and run the handler.
 */
void func_800B2DF0(void)
{
    D_801B2FBC = 0x20;
    D_80139280[30] = 9999;
    D_801B2FB8 += 1;
    func_800B2E38();
}

/** @brief World-map step handler: draw a framed panel and expire the step counter. */
void func_800B2E38(void)
{
    func_8006AEE0();
    func_8008ECF8(0x14, 0x78, D_8011CF2C, (s32)D_80139280 + 0x78);
    func_8006534C(0x25, 5);
    if (--D_801B2FBC == 0)
    {
        D_801B2FB8 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800B2EB0(void)
{
    D_801B2FB8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B2EC8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2FC0 = 1;
        D_801B2FC4 = 1;
        return 1;
    }

    if (D_801B2FC0 < 0x4)
    {
        PS1_CALL(D_800D72BC[D_801B2FC0])();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B2F40(void)
{
    D_801B2FC0 = 1;
    D_801B2FC4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_800B2F58(void)
{
    D_8013B240 = D_80139258;
    D_80139888 = g_wmap_camera_translation;
    D_80182DF4 = 0x80;
    D_80139888.vz = 0xAFC8;
    D_801B2FC4 = 0x8;
    D_801B2FC0 += 1;
    func_800B1964();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800B3008(void)
{
    D_801B2FC0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B3020(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2FC8 = 1;
        D_801B2FCC = 1;
        return 1;
    }

    if (D_801B2FC8 < 0x6)
    {
        PS1_CALL(D_800D72CC[D_801B2FC8])();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B3098(void)
{
    D_801B2FC8 = 1;
    D_801B2FCC = 1;
}

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_800B30B0(void)
{
    D_80182DE4 = 1;
    D_801B2498 = D_80139258;
    D_8013923C = 0;
    D_801B2FCC = 0x159;
    D_801B2FC8 += 1;
    func_800B1A74();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800B312C(void)
{
    D_801B2FCC = 0x8;
    D_801B2FC8 += 1;
    func_800B1B78();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800B3164(void)
{
    D_801B2FC8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B317C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2FD0 = 1;
        D_801B2FD4 = 1;
        return 1;
    }

    if (D_801B2FD0 < 0x6)
    {
        PS1_CALL(D_800D72E4[D_801B2FD0])();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B31F4(void)
{
    D_801B2FD0 = 1;
    D_801B2FD4 = 1;
}

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_800B320C(void)
{
    D_80182DE8 = 1;
    D_801B24A0 = D_80139258;
    D_80139240 = 0;
    D_801B2FD4 = 0xE9;
    D_801B2FD0 += 1;
    func_800B1C78();
}

/**
 * @brief Set the sequence parameter, advance the counter, and run the handler.
 */
void func_800B3288(void)
{
    D_801B2FD4 = 0x8;
    D_801B2FD0 += 1;
    func_800B1D7C();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800B32C0(void)
{
    D_801B2FD0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B32D8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2FD8 = 1;
        D_801B2FDC = 1;
        return 1;
    }

    if (D_801B2FD8 < 0x6)
    {
        PS1_CALL(D_800D72FC[D_801B2FD8])();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800B3350(void)
{
    D_801B2FD8 = 1;
    D_801B2FDC = 1;
}

void func_800B3368(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_80182DEC = 1;
    D_801B24A8 = D_80139258;
    D_8013924C = 0;
    D_801B2FDC = 0xA7;
    D_801B2FD8++;
    func_800B1E7C();
}

void func_800B33E4(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B2FDC = 8;
    D_801B2FD8++;
    func_800B1F80();
}

void func_800B341C(void)
{
void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

    D_801B2FD8++;
}
