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

/** @brief Queue effect resources and establish the map-relative effect position. */
void func_800B1744(void)
{
/** @brief Four-word world-map transform state. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    s32 pad;
} WmapTransform;

extern WmapTransform g_wmap_view;
extern s16 D_800D926A;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern u8 D_800DEF18[];
extern u8 D_8011D538[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 *D_8011CF28;
extern u8 *D_8011CF2C;
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_801B2F90;

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
/** @brief Event actor state at the start of its parameter block. */
typedef struct
{
    u8 pad[0x78];
    s32 state;
} WmapEventActorState;

extern u8 D_80139988[];
extern void *D_80121538;
extern s16 D_801AFBD0;
extern u8 *D_80139280;
extern s32 D_801B2FB8;
extern s32 D_801B2FBC;
extern void func_800B2D78(void);

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
    actor = D_80139280;
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

extern s32 D_80139888[];
extern SVECTOR D_8013B240;
extern VECTOR D_8011CF60;
extern u8 D_800DCF18[];
extern s32 D_80182DF4;
extern s32 D_801B2FC0;
extern s32 D_801B2FC4;

    MATRIX m;
    s32 x;

    x = D_80139888[2] - 0xDAC;
    D_80139888[2] = x;
    if (x < 0x2710)
    {
        D_80139888[2] = 0x2710;
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

extern u8 g_wmap_camera_translation[];
extern u8 D_801B2498[];
extern s32* D_8011CF1C;
extern s32 D_8013923C;
extern s32 D_80182DE4;
extern s32 D_801B2FC8;
extern s32 D_801B2FCC;

    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(g_wmap_camera_translation, D_801B2498);
    wmap_draw_model(D_8011CF1C, (D_8013923C / 0x10) & 3, 0xA, 0x35, 0x7800, 0x1, D_80182DE4, 0, -0xA, -1);
    D_8013923C += 0x8;
    value = D_80182DE4 + 0x2;
    D_80182DE4 = value;
    if (value >= 0x82)
    {
        D_80182DE4 = 0x81;
    }
    timer = D_801B2FCC;
    ((u16*)D_801B2498)[2] += 0x4;
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

extern u8 g_wmap_camera_translation[];
extern u8 D_801B2498[];
extern s32* D_8011CF1C;
extern s32 D_8013923C;
extern s32 D_80182DE4;
extern s32 D_801B2FC8;
extern s32 D_801B2FCC;

    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(g_wmap_camera_translation, D_801B2498);
    wmap_draw_model(D_8011CF1C, (D_8013923C / 0x10) & 3, 0xA, 0x35, 0x7800, 0x1, D_80182DE4, 0, -0xA, -1);
    value = D_80182DE4 - 0x10;
    D_80182DE4 = value;
    if (value < 0)
    {
        D_80182DE4 = 0;
    }
    D_8013923C += 0x8;
    timer = D_801B2FCC;
    ((u16*)D_801B2498)[2] += 0x4;
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

extern u8 g_wmap_camera_translation[];
extern u8 D_801B24A0[];
extern s32* D_8011CF24;
extern s32 D_80139240;
extern s32 D_80182DE8;
extern s32 D_801B2FD0;
extern s32 D_801B2FD4;

    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(g_wmap_camera_translation, D_801B24A0);
    wmap_draw_model(D_8011CF24, (D_80139240 / 0x10) & 3, 0xA, 0x35, 0x7840, 0x1, D_80182DE8, 0, -0xA, -1);
    D_80139240 += 0x10;
    value = D_80182DE8 + 0x8;
    D_80182DE8 = value;
    if (value >= 0x82)
    {
        D_80182DE8 = 0x81;
    }
    timer = D_801B2FD4;
    ((u16*)D_801B24A0)[2] += 0x20;
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

extern u8 g_wmap_camera_translation[];
extern u8 D_801B24A0[];
extern s32* D_8011CF24;
extern s32 D_80139240;
extern s32 D_80182DE8;
extern s32 D_801B2FD0;
extern s32 D_801B2FD4;

    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(g_wmap_camera_translation, D_801B24A0);
    wmap_draw_model(D_8011CF24, (D_80139240 / 0x10) & 3, 0xA, 0x35, 0x7840, 0x1, D_80182DE8, 0, -0xA, -1);
    value = D_80182DE8 - 0x10;
    D_80182DE8 = value;
    if (value < 0)
    {
        D_80182DE8 = 0;
    }
    D_80139240 += 0x10;
    timer = D_801B2FD4;
    ((u16*)D_801B24A0)[2] += 0x20;
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

extern u8 g_wmap_camera_translation[];
extern u8 D_801B24A8[];
extern s32* D_8011CF28;
extern s32 D_8013924C;
extern s32 D_80182DEC;
extern s32 D_801B2FD8;
extern s32 D_801B2FDC;

    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(g_wmap_camera_translation, D_801B24A8);
    wmap_draw_model(D_8011CF28, (D_8013924C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1, D_80182DEC, 0, -0xA, -1);
    D_8013924C += 0x10;
    value = D_80182DEC + 0x8;
    D_80182DEC = value;
    if (value >= 0x82)
    {
        D_80182DEC = 0x81;
    }
    timer = D_801B2FDC;
    ((u16*)D_801B24A8)[2] += 0x8;
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

extern u8 g_wmap_camera_translation[];
extern u8 D_801B24A8[];
extern s32* D_8011CF28;
extern s32 D_8013924C;
extern s32 D_80182DEC;
extern s32 D_801B2FD8;
extern s32 D_801B2FDC;

    s32 value;
    s32 timer;
    s32 next_timer;

    wmap_set_model_transform(g_wmap_camera_translation, D_801B24A8);
    wmap_draw_model(D_8011CF28, (D_8013924C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1, D_80182DEC, 0, -0xA, -1);
    value = D_80182DEC - 0x10;
    D_80182DEC = value;
    if (value < 0)
    {
        D_80182DEC = 0;
    }
    D_8013924C += 0x10;
    timer = D_801B2FDC;
    ((u16*)D_801B24A8)[2] += 0x8;
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
extern u32 D_801B2F90;
extern s32 D_801B2F94;
extern void (*D_800D7204[])(void);
extern s32 g_wmap_view_scroll_mode;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2F90 = 1;
        D_801B2F94 = 1;
        return 1;
    }

    if (D_801B2F90 < 0x6)
    {
        D_800D7204[D_801B2F90]();
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
extern u32 D_801B2F90;
extern s32 D_801B2F94;
extern void (*D_800D7204[])(void);
extern s32 g_wmap_view_scroll_mode;

    D_801B2F90 = 1;
    D_801B2F94 = 1;
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800B2110(void)
{
extern u32 D_801B2F90;
extern s32 D_801B2F94;
extern void (*D_800D7204[])(void);
extern s32 g_wmap_view_scroll_mode;

    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B2F90 += 1;
        func_800B2150();
    }
}

/** @brief Wait for CD work, initialize sequence state, and register its callback. */
void func_800B2150(void)
{
extern s32 D_800D9228;
extern s16 g_wmap_focus_screen_position[];
extern s32 D_8011D500;
extern s32 D_8013B20C;
extern s32 D_8013B258;
extern s32 D_80182E38;
extern s32 D_801B2F90;

    cdrom_wait_queue_empty();
    g_wmap_focus_screen_position[0] = 0x94;
    g_wmap_focus_screen_position[1] = 0x31;
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
extern s32 D_8013B20C;
extern s32 D_801B2F90;

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
extern s32 D_8013B294;
extern s32 D_80139228;
extern s32 D_801B2F90;

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
extern u32 D_801B2F98;
extern s32 D_801B2F9C;
extern void (*D_800D721C[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2F98 = 1;
        D_801B2F9C = 1;
        return 1;
    }

    if (D_801B2F98 < 0x16)
    {
        D_800D721C[D_801B2F98]();
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
extern u32 D_801B2F98;
extern s32 D_801B2F9C;
extern void (*D_800D721C[])(void);

    D_801B2F98 = 1;
    D_801B2F9C = 1;
}

/** @brief Set the sequence colors and start its fifteen-frame countdown. */
void func_800B22D4(void)
{
/** @brief Three color channels. */
typedef struct
{
    u8 r, g, b;
} WmapColor;

extern s32 D_8013B208;
extern WmapColor D_80182D74;
extern WmapColor D_80182D80;
extern WmapColor D_80182D8C;
extern WmapColor D_80182D94;
extern s32 D_801B2F98;
extern s32 D_801B2F9C;

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
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

    if (--D_801B2F9C == 0)
    {
        D_801B2F98 += 1;
    }
}

/** @brief World-map step handler: register the next callback and advance the counter. */
void func_800B23A8(void)
{
extern s32 D_8011D500;
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

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
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

    if (--D_801B2F9C == 0)
    {
        D_801B2F98 += 1;
    }
}

/** @brief Set world-map color and state, register two callbacks, and begin a four-tick delay. */
void func_800B2424(void)
{
extern s32 D_801B2F98;
extern s32 D_801B2F9C;

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
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

    if (--D_801B2F9C == 0)
    {
        D_801B2F98 += 1;
    }
}

/** @brief World-map step handler: register the next callback and advance the counter. */
void func_800B24B8(void)
{
extern s32 D_8011D500;
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

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
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

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
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

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
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

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
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

    wmap_start_sequence(func_800B299C);
    D_801B2F9C = 0x4;
    D_801B2F98 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B25EC(void)
{
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

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
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

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
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

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
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

    wmap_start_sequence(func_800B2B40);
    D_801B2F9C = 0x4;
    D_801B2F98 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B26D8(void)
{
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

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
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

    wmap_start_sequence(func_800B2B40);
    D_801B2F9C = 0x10;
    D_801B2F98 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B2748(void)
{
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

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
extern s32 D_8011D500;
extern s32 D_801B2F98;
extern s32 D_801B2F9C;

    D_8011D500 = 0xFF;
    D_801B2F9C = 0x40;
    D_801B2F98 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800B27A8(void)
{
extern s32 D_801B2F9C;
extern s32 D_801B2F98;

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
extern s32 D_801B2F98;
extern s32 D_8013B20C;

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
extern u32 D_801B2FA0;
extern s32 D_801B2FA4;
extern void (*D_800D7274[])(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 g_wmap_focus_screen_position;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2FA0 = 1;
        D_801B2FA4 = 1;
        return 1;
    }

    if (D_801B2FA0 < 0x4)
    {
        D_800D7274[D_801B2FA0]();
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
extern u32 D_801B2FA0;
extern s32 D_801B2FA4;
extern void (*D_800D7274[])(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 g_wmap_focus_screen_position;

    D_801B2FA0 = 1;
    D_801B2FA4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800B2888(void)
{
extern u32 D_801B2FA0;
extern s32 D_801B2FA4;
extern void (*D_800D7274[])(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 g_wmap_focus_screen_position;

    D_801399AC = D_8011D538;
    D_800D9318[0x6] = 0xF;
    *(s16*)&D_800D9318[0x10] = -1;
    *(s16*)&D_800D9318[0x26] = 0x10;
    *(s16*)&D_800D9318[0x22] = 1;
    *(s16*)&D_800D9318[0x2] = 0;
    *(s16*)&D_800D9318[0xE] = 0;
    *(s16*)&D_800D9318[0x24] = 0x81;
    D_801B2FA4 = 0x10;
    D_801B2FA0 += 1;
    func_800B2908();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800B2908(void)
{
extern u32 D_801B2FA0;
extern s32 D_801B2FA4;
extern void (*D_800D7274[])(void);
extern void func_800B2908(void);
extern u8 D_8011D538[];
extern u8* D_801399AC;
extern u8 D_800D9318[];
extern u8 D_801399A8[];
extern s32 g_wmap_focus_screen_position;

    wmap_step_actor_animation(D_800D9318, D_801399A8);
    wmap_draw_actor_sprite(D_800D9318, g_wmap_focus_screen_position, 0x2A, 0x2, 0);
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
extern s32 D_801B2FA0;

    D_801B2FA0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B299C(s32 arg0)
{
extern u32 D_801B2FA8;
extern s32 D_801B2FAC;
extern void (*D_800D7284[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2FA8 = 1;
        D_801B2FAC = 1;
        return 1;
    }

    if (D_801B2FA8 < 0x4)
    {
        D_800D7284[D_801B2FA8]();
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
extern u32 D_801B2FA8;
extern s32 D_801B2FAC;
extern void (*D_800D7284[])(void);

    D_801B2FA8 = 1;
    D_801B2FAC = 1;
}

void func_800B2A2C(void)
{
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern s32 D_801B2FAC;
extern s32 D_801B2FA8;
extern void func_800B2AAC(void);

    s32 one = 1;

    D_801399B4 = D_8011D538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0xE] = one;
    *(s16*)&D_800D9344[0x10] = -one;
    *(s16*)&D_800D9344[0x26] = 4;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0x22] = one;
    *(s16*)&D_800D9344[0x24] = 0x81;
    D_801B2FAC = 0x10;
    D_801B2FA8 += one;
    func_800B2AAC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800B2AAC(void)
{
extern s32 D_801B2FA8;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;
extern s32 D_801B2FAC;

    wmap_step_actor_animation(D_800D9344, D_801399B0);
    wmap_draw_actor_sprite(D_800D9344, g_wmap_focus_screen_position, 0x2A, 0x2, 0);
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
extern s32 D_801B2FA8;
extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 g_wmap_focus_screen_position;
extern s32 D_801B2FAC;

    D_801B2FA8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B2B40(s32 arg0)
{
extern u32 D_801B2FB0;
extern s32 D_801B2FB4;
extern void (*D_800D7294[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2FB0 = 1;
        D_801B2FB4 = 1;
        return 1;
    }

    if (D_801B2FB0 < 0x4)
    {
        D_800D7294[D_801B2FB0]();
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
extern u32 D_801B2FB0;
extern s32 D_801B2FB4;
extern void (*D_800D7294[])(void);

    D_801B2FB0 = 1;
    D_801B2FB4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800B2BD0(void)
{
extern u8* D_801399BC;
extern u8 D_8011D538[];
extern u8 D_800D9370[];
extern s32 D_801B2FB0;
extern s32 D_801B2FB4;

    D_801399BC = D_8011D538;
    D_800D9370[0x6] = 0xF;
    *(s16*)&D_800D9370[0xE] = 2;
    *(s16*)&D_800D9370[0x10] = -1;
    *(s16*)&D_800D9370[0x26] = 0x10;
    *(s16*)&D_800D9370[0x22] = 1;
    *(s16*)&D_800D9370[0x2] = 0;
    *(s16*)&D_800D9370[0x24] = 0x81;
    D_801B2FB4 = 0x10;
    D_801B2FB0 += 1;
    func_800B2C54();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800B2C54(void)
{
extern s32 D_801B2FB0;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 g_wmap_focus_screen_position;
extern s32 D_801B2FB4;

    wmap_step_actor_animation(D_800D9370, D_801399B8);
    wmap_draw_actor_sprite(D_800D9370, g_wmap_focus_screen_position, 0x2A, 0x2, 0);
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
extern s32 D_801B2FB0;
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 g_wmap_focus_screen_position;
extern s32 D_801B2FB4;

    D_801B2FB0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B2CE8(s32 arg0)
{
extern u32 D_801B2FB8;
extern s32 D_801B2FBC;
extern void (*D_800D72A4[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2FB8 = 1;
        D_801B2FBC = 1;
        return 1;
    }

    if (D_801B2FB8 < 0x6)
    {
        D_800D72A4[D_801B2FB8]();
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
extern u32 D_801B2FB8;
extern s32 D_801B2FBC;
extern void (*D_800D72A4[])(void);

    D_801B2FB8 = 1;
    D_801B2FBC = 1;
}

/** @brief World-map step handler: draw a framed panel and expire the step counter. */
void func_800B2D78(void)
{
extern s32 D_8011CF2C;
extern s32 D_80139280;
extern s32 D_801B2FB8;
extern s32 D_801B2FBC;
extern void func_8008ECF8(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

    func_8006AEE0();
    func_8008ECF8(0x14, 0x78, D_8011CF2C, D_80139280 + 0x78);
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
extern s32* D_80139280;
extern s32 D_801B2FBC;
extern s32 D_801B2FB8;

    D_801B2FBC = 0x20;
    D_80139280[30] = 9999;
    D_801B2FB8 += 1;
    func_800B2E38();
}

/** @brief World-map step handler: draw a framed panel and expire the step counter. */
void func_800B2E38(void)
{
extern s32 D_8011CF2C;
extern s32 D_80139280;
extern s32 D_801B2FB8;
extern s32 D_801B2FBC;
extern void func_8008ECF8(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

    func_8006AEE0();
    func_8008ECF8(0x14, 0x78, D_8011CF2C, D_80139280 + 0x78);
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
extern s32 D_801B2FB8;

    D_801B2FB8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B2EC8(s32 arg0)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2FC0;
extern s32 D_801B2FC4;
extern void (*D_800D72BC[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_80139888;
extern s32 D_80182DF4;

    s32 result;

    if (arg0 != 0)
    {
        D_801B2FC0 = 1;
        D_801B2FC4 = 1;
        return 1;
    }

    if (D_801B2FC0 < 0x4)
    {
        D_800D72BC[D_801B2FC0]();
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
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2FC0;
extern s32 D_801B2FC4;
extern void (*D_800D72BC[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_80139888;
extern s32 D_80182DF4;

    D_801B2FC0 = 1;
    D_801B2FC4 = 1;
}

/**
 * @brief Seed two sequence data blocks and one field, arm the timer, advance, and run the handler.
 */
void func_800B2F58(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;
typedef struct { int w[4]; } WmapBlk16;

extern u32 D_801B2FC0;
extern s32 D_801B2FC4;
extern void (*D_800D72BC[])(void);
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_8013B240;
extern WmapBlk16 g_wmap_camera_translation;
extern WmapBlk16 D_80139888;
extern s32 D_80182DF4;

    D_8013B240 = D_80139258;
    D_80139888 = g_wmap_camera_translation;
    D_80182DF4 = 0x80;
    D_80139888.w[2] = 0xAFC8;
    D_801B2FC4 = 0x8;
    D_801B2FC0 += 1;
    func_800B1964();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800B3008(void)
{
extern s32 D_801B2FC0;

    D_801B2FC0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B3020(s32 arg0)
{
extern u32 D_801B2FC8;
extern s32 D_801B2FCC;
extern void (*D_800D72CC[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2FC8 = 1;
        D_801B2FCC = 1;
        return 1;
    }

    if (D_801B2FC8 < 0x6)
    {
        D_800D72CC[D_801B2FC8]();
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
extern u32 D_801B2FC8;
extern s32 D_801B2FCC;
extern void (*D_800D72CC[])(void);

    D_801B2FC8 = 1;
    D_801B2FCC = 1;
}

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_800B30B0(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DE4;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B2498;
extern s32 D_8013923C;
extern s32 D_801B2FC8;
extern s32 D_801B2FCC;

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
extern s32 D_801B2FC8;
extern s32 D_801B2FCC;

    D_801B2FCC = 0x8;
    D_801B2FC8 += 1;
    func_800B1B78();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800B3164(void)
{
extern s32 D_801B2FC8;
extern s32 D_801B2FCC;

    D_801B2FC8 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B317C(s32 arg0)
{
extern u32 D_801B2FD0;
extern s32 D_801B2FD4;
extern void (*D_800D72E4[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2FD0 = 1;
        D_801B2FD4 = 1;
        return 1;
    }

    if (D_801B2FD0 < 0x6)
    {
        D_800D72E4[D_801B2FD0]();
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
extern u32 D_801B2FD0;
extern s32 D_801B2FD4;
extern void (*D_800D72E4[])(void);

    D_801B2FD0 = 1;
    D_801B2FD4 = 1;
}

/**
 * @brief Arm the world-map sequence, seed its data block, and schedule the next step.
 */
void func_800B320C(void)
{
typedef struct { unsigned char b[8]; } WmapBlk8;

extern s32 D_80182DE8;
extern WmapBlk8 D_80139258;
extern WmapBlk8 D_801B24A0;
extern s32 D_80139240;
extern s32 D_801B2FD0;
extern s32 D_801B2FD4;

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
extern s32 D_801B2FD0;
extern s32 D_801B2FD4;

    D_801B2FD4 = 0x8;
    D_801B2FD0 += 1;
    func_800B1D7C();
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800B32C0(void)
{
extern s32 D_801B2FD0;
extern s32 D_801B2FD4;

    D_801B2FD0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B32D8(s32 arg0)
{
extern u32 D_801B2FD8;
extern s32 D_801B2FDC;
extern void (*D_800D72FC[])(void);

    s32 result;

    if (arg0 != 0)
    {
        D_801B2FD8 = 1;
        D_801B2FDC = 1;
        return 1;
    }

    if (D_801B2FD8 < 0x6)
    {
        D_800D72FC[D_801B2FD8]();
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
extern u32 D_801B2FD8;
extern s32 D_801B2FDC;
extern void (*D_800D72FC[])(void);

    D_801B2FD8 = 1;
    D_801B2FDC = 1;
}

void func_800B3368(void)
{
typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s16 field_04;
} WmapShort3;

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
} WmapInt3;

typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
    s32 field_0C;
} WmapAlignedQuad;

typedef struct
{
    s32 words[9];
} WmapBlock36;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 value;
    u8 pad[0x24];
} WmapCell;

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

typedef union
{
    struct
    {
        s16 field_00;
        s16 field_02;
    } fields;
    s32 value;
} WmapSignedPair16;

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

extern s32 D_80054A18[];
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
extern WmapHandler D_800D7474[];
extern WmapHandler D_800D748C[];
extern WmapHandler D_800D753C[];
extern WmapHandler D_800D7554[];
extern WmapHandler D_800D7564[];
extern WmapHandler D_800D757C[];
extern WmapHandler D_800D7594[];
extern WmapHandler D_800D75AC[];
extern WmapHandler D_800D75C4[];
extern WmapHandler D_800D75D4[];
extern WmapHandler D_800D75E4[];
extern WmapHandler D_800D75F4[];
extern WmapHandler D_800D760C[];
extern WmapHandler D_800D7624[];
extern WmapHandler D_800D763C[];
extern WmapHandler D_800D7654[];
extern WmapHandler D_800D766C[];
extern WmapHandler D_800D7684[];
extern WmapHandler D_800D769C[];
extern WmapHandler D_800D76AC[];
extern WmapHandler D_800D76C4[];
extern WmapHandler D_800D76DC[];
extern WmapHandler D_800D76F4[];
extern WmapHandler D_800D770C[];
extern WmapHandler D_800D7774[];
extern WmapHandler D_800D778C[];
extern WmapHandler D_800D77A4[];
extern WmapHandler D_800D77BC[];
extern WmapHandler D_800D77D4[];
extern WmapHandler D_800D77E4[];
extern WmapHandler D_800D77F4[];
extern WmapHandler D_800D7804[];
extern WmapHandler D_800D7814[];
extern WmapHandler D_800D7824[];
extern WmapHandler D_800D78BC[];
extern WmapHandler D_800D78CC[];
extern WmapHandler D_800D78A4[];
extern WmapHandler D_800D788C[];
extern WmapHandler D_800D7874[];
extern WmapHandler D_800D7854[];
extern WmapHandler D_800D783C[];
extern WmapHandler D_800D7924[];
extern WmapHandler D_800D793C[];
extern WmapHandler D_800D7954[];
extern WmapHandler D_800D796C[];
extern WmapHandler D_800D7984[];
extern WmapHandler D_800D799C[];
extern WmapHandler D_800D79B4[];
extern WmapHandler D_800D79CC[];
extern WmapHandler D_800D79E4[];
extern WmapHandler D_800D79FC[];
extern WmapHandler D_800D7A0C[];
extern WmapHandler D_800D7A2C[];
extern u8 D_800D95D8[];
extern u8 D_800D9D68[];
extern u8 D_800DA028[];
extern u8 D_800DAA78[];
extern u8 D_800DB310[];
extern WmapConfigA D_800DB4C8[];
extern s32 D_800DBE70;
extern WmapBlock36 D_800D06BC;
extern WmapBlock36 D_800D9240;
extern WmapAlignedQuad g_wmap_saved_view;
extern s32 D_800DCEB0;
extern WmapShort3 D_800DCEB8;
extern s32 g_wmap_cursor_column;
extern s32 g_wmap_cursor_row;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 D_800DCF18[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9318;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern s32 D_800D9158;
extern s32 D_800D9228;
extern WmapConfigA D_800D9268[];
extern WmapPair16 g_wmap_focus_screen_position;
extern s32* D_8011CF1C;
extern s32* D_8011CF24;
extern s32* D_8011CF28;
extern s32* D_8011CF2C;
extern s32* D_8011CF30;
extern s32* D_8011CF34;
extern s32* D_8011CF38;
extern s32* D_8011CF3C;
extern s32 D_8011D4FC;
extern s32 D_8011D500;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern u8 D_80123538;
extern u8 D_80125538;
extern u8 D_80127538;
extern s32 D_80139228;
extern s32 D_80139234;
extern WmapInt3 D_80139200;
extern WmapShort3 D_80139210;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139244;
extern s32 D_8013924C;
extern s32 D_80139250;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern WmapState* D_80139280;
extern WmapCell D_80139290[][6];
extern VECTOR D_80139870;
extern s32 g_wmap_view_mode;
extern s32 g_wmap_view_scroll_mode;
extern u8 D_80139A28[];
extern u8 D_80139B88[];
extern u8 D_80139C08[];
extern u8 D_80139DE8[];
extern u8 D_80139F78[];
extern WmapAlignedQuad g_wmap_view;
extern WmapInt3 D_80139968;
extern s32 D_80139978;
extern u8 D_80139988[];
extern u8 D_801399A8;
extern void* D_801399AC;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern u8 D_801399C8;
extern void* D_801399CC;
extern u8 D_801399D0;
extern void* D_801399D4;
extern u8 D_801399D8;
extern void* D_801399DC;
extern s32 D_8013B20C;
extern s32 D_8013B208;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern s32 D_8013B294;
extern s32 D_8013B29C;
extern WmapSignedPair16 D_80182D58;
extern WmapPair16 D_80182D60;
extern u8 D_80182E40;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_80182DD8;
extern s32 D_80182DE4;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern s32 D_80182E38;
extern VECTOR g_wmap_camera_translation;
extern VECTOR D_8011CF60;
extern u8 D_8018B240;
extern s32 D_801ADAE0;
extern u8 D_801AFBD0[];
extern VECTOR D_801B2478;
extern WmapPair D_801B2490;
extern WmapPair D_801B2498;
extern SVECTOR D_801B24A0;
extern WmapPair D_801B24A8;
extern s32 D_801B24B4;
extern s32 D_801B25D8;
extern s32 D_801B25DC;
extern s32 D_801B25E0;
extern VECTOR D_801B2650;
extern WmapPair D_801B2670;
extern WmapPair D_801B2678;
extern s32 D_801B2FD8;
extern s32 D_801B2FDC;
extern s32 D_801B2FE0;
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
extern s32 D_801B3048;
extern s32 D_801B304C;
extern s32 D_801B3050;
extern s32 D_801B3054;
extern s32 D_801B3058;
extern s32 D_801B305C;
extern s32 D_801B3060;
extern s32 D_801B3064;
extern s32 D_801B3068;
extern s32 D_801B306C;
extern s32 D_801B3070;
extern s32 D_801B3074;
extern s32 D_801B3078;
extern s32 D_801B307C;
extern s32 D_801B3080;
extern s32 D_801B3084;
extern s32 D_801B3088;
extern s32 D_801B308C;
extern s32 D_801B3090;
extern s32 D_801B3094;
extern s32 D_801B3098;
extern s32 D_801B309C;
extern s32 D_801B30A0;
extern s32 D_801B30A4;
extern s32 D_801B30A8;
extern s32 D_801B30AC;
extern s32 D_801B30B0;
extern s32 D_801B30B4;
extern s32 D_801B30B8;
extern s32 D_801B30BC;
extern s32 D_801B30C0;
extern s32 D_801B30C4;
extern s32 D_801B30C8;
extern s32 D_801B30CC;
extern s32 D_801B30D0;
extern s32 D_801B30D4;
extern s32 D_801B30D8;
extern s32 D_801B30DC;
extern s32 D_801B30E0;
extern s32 D_801B30E4;
extern s32 D_801B30E8;
extern s32 D_801B30EC;
extern s32 D_801B30F0;
extern s32 D_801B30F4;
extern WmapPair D_801B3118;
extern SVECTOR D_801B3120;
extern s32 D_801B3128;
extern s32 D_801B312C;
extern s32 D_801B3130;
extern s32 D_801B3134;
extern s32 D_801B3138;
extern s32 D_801B313C;
extern s32 D_801B3140;
extern s32 D_801B3144;
extern s32 D_801B3148;
extern s32 D_801B314C;
extern s32 D_801B3150;
extern s32 D_801B3154;
extern s32 D_801B3158;
extern s32 D_801B315C;
extern s32 D_801B3160;
extern s32 D_801B3164;
extern s32 D_801B3168;
extern s32 D_801B316C;
extern s32 D_801B3170;
extern s32 D_801B3174;
extern s32 D_801B3178;
extern s32 D_801B317C;
extern s32 D_801B3180;
extern s32 D_801B3184;
extern s32 D_801B3188;
extern s32 D_801B318C;
extern s32 D_801B3190;
extern s32 D_801B3194;
extern s32 D_801B3198;
extern s32 D_801B319C;
extern s32 D_801B31A0;
extern s32 D_801B31A4;
extern s32 D_801B31A8;
extern s32 D_801B31AC;
extern s32 D_801B31B0;
extern s32 D_801B31B4;
extern s32 D_801B31B8;
extern s32 D_801B31BC;
extern s32 D_801B31C0;
extern s32 D_801B31C4;
extern s32 D_801B31C8;
extern s32 D_801B31CC;
extern s32 D_801B31D0;
extern s32 D_801B31D4;
extern s32 D_801B31D8;
extern s32 D_801B31DC;
extern s32 D_801B31E0;
extern s32 D_801B31E4;
extern s32 D_801B31E8;
extern s32 D_801B31EC;
extern s32 D_801B31F0;
extern s32 D_801B31F4;
extern s32 D_801B31F8;
extern s32 D_801B31FC;
extern s32 D_801B3200;
extern s32 D_801B3204;
extern s32 D_801B3208;
extern s32 D_801B320C;
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

extern void akao_cmd_a9(s32, s32);

extern void func_8008ECF8(s32, s32, s32*, void*);
extern void func_800B37EC(void);
extern void func_800B38EC(void);
extern void func_800B35F0(void);
extern void func_800B36F0(void);
extern void func_800B4E18(void);
extern void func_800B4FBC(void);
extern void func_800B39E8(void);
extern void func_800B3AE8(void);
extern void func_800B3BE4(void);
extern void func_800B3CE0(void);
extern void func_800B3DD8(void);
extern void func_800B3EDC(void);
extern void func_800B3FDC(void);
extern void func_800B40DC(void);
extern void func_800B41D8(void);
extern void func_800B42DC(void);
extern void func_800B43DC(void);
extern void func_800B5C7C(void);
extern void func_800B619C(void);
extern void func_800B6284(void);
extern void func_800B6558(void);
extern void func_800B6644(void);
extern void func_800B6730(void);
extern void func_800B681C(void);
extern void func_800B6918(void);
extern void func_800B6A10(void);
extern void func_800B6B0C(void);
extern void func_800B6C04(void);
extern void func_800B6D00(void);
extern void func_800B6DF8(void);
extern void func_800B6EF4(void);
extern void func_800B6FEC(void);
extern void func_800B70CC(void);
extern void func_800BA108(void);
extern void func_800BA408(void);
extern void func_800BA580();
extern void func_800BA66C();
extern void func_800BA758();
extern void func_800BAAB0();
extern void func_800BAB80();
extern void func_800BA1BC();
extern void func_800B9F1C();
extern void func_800B9FE4();
extern void func_800B8794(void);
extern void func_800B8988(void);
extern void func_800B95CC(void);
extern void func_800B96B8(void);
extern void func_800B9850(void);
extern void func_800B9938(void);
extern void func_800B9B08(void);
extern void func_800B9D74(void);
extern void func_800BAEF8();
extern void func_800BAF34();
extern void func_800BAF78();
extern void func_800BAFB4();
extern s32 func_800BAFCC(s32);
extern s32 func_800BB610(s32);
extern s32 func_800BB87C(s32);
extern s32 func_800BBAE8(s32);
extern void func_800BCDE0();
extern void func_800BCCF0();
extern void func_800BCAC8();
extern void func_800BC8C4();
extern void func_800BAC44();
extern void func_800BAD38();
extern void func_800BD258();
extern void func_800BD354();
extern void func_800BD750(void);
extern s32 func_800BD998(s32);
extern void func_800BD934(void);
extern void func_800BD970(void);
extern s32 func_800BE1A8(s32);
extern s32 func_800BE40C(s32);
extern s32 func_800BE678(s32);
extern s32 func_800BE8E4(s32);
extern s32 func_800BEB50(s32);
extern s32 func_800BED50(s32);
extern s32 func_800BEF58(s32);
extern s32 func_800BF160(s32);
extern s32 func_800BF368(s32);
extern s32 func_800BDF40(s32);
extern void func_800BE04C(void);
extern void func_800BE114(void);
extern void func_800BE2B0(void);
extern void func_800BE378(void);
extern void func_800BE51C(void);
extern void func_800BE5E4(void);
extern void func_800BE788(void);
extern void func_800BE850(void);
extern void func_800BE9F4(void);
extern void func_800BEABC(void);
extern void func_800BECB0(void);
extern void func_800BEEB4(void);
extern void func_800BF0BC(void);
extern void func_800BF1F0(void);
extern void func_800BF2C4(void);
extern void func_800BEFE8(void);
extern void func_800BFEEC(void);
extern void func_800BFF28(void);
extern void func_800BFF98(void);
extern void func_800BFFD8(void);
extern s32 func_800C0034(s32);
extern void func_800C06DC(void);
extern void func_800C071C(void);
extern void func_800C075C(void);
extern void func_800C079C(void);
extern s32 func_800BC01C(s32);
extern s32 func_800BC1BC(s32);
extern s32 func_800BC360(s32);
extern s32 func_800BC4B8(s32);
extern s32 func_800BC60C(s32);
extern s32 func_800BC764(s32);
extern s32 func_800BC964(s32);
extern void func_800BC7F4(void);
extern void func_800BC9F4(void);
extern void func_800BCBFC(void);
extern s32 func_800BCB6C(s32);
extern s32 func_800BCEA8(s32);
extern s32 func_800BCFD4(s32);
extern s32 func_800BD12C(s32);
extern s32 func_800BBD54(s32);
extern void func_800BB720();
extern void func_800BB7E8();
extern void func_800BB98C();
extern void func_800BBA54();
extern void func_800BBBF8();
extern void func_800BBCC0();
extern void func_800BBF88();
extern void func_800BC128();
extern void func_800BC2CC();

extern s32 func_800B7438(s32);
extern s32 func_800B7E7C(s32);
extern s32 func_800B8084(s32);
extern s32 func_800B822C(s32);
extern s32 func_800B8B80(s32);
extern s32 func_800B8CD8(s32);
extern s32 func_800B8E30(s32);
extern s32 func_800B8F8C(s32);
extern s32 func_800B90E8(s32);
extern s32 func_800B9244(s32);
extern s32 func_800B99F8(s32);
extern s32 func_800B9B9C(s32);
extern s32 func_800BA078(s32);
extern s32 func_800B84D0(s32);
extern s32 func_800B862C(s32);
extern s32 func_800B8824(s32);
extern s32 func_800B8A2C(s32);
extern s32 func_800B93A0(s32);
extern s32 func_800B94FC(s32);
extern s32 func_800B977C(s32);
extern s32 func_800B9E08(s32);
extern s32 func_800B5050(s32);
extern s32 func_800B51AC(s32);
extern s32 func_800B4EAC(s32);
extern s32 func_800B5308(s32);
extern s32 func_800B4D08(s32);
extern s32 func_800B571C(s32);
extern s32 func_800B55C0(s32);
extern s32 func_800B5878(s32);
extern s32 func_800B5B2C(s32);
extern s32 func_800B4774(s32);
extern void func_800B4688(void);
extern void func_800B470C(void);
extern void func_800B4748(void);

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
typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s16 field_04;
} WmapShort3;

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
} WmapInt3;

typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
    s32 field_0C;
} WmapAlignedQuad;

typedef struct
{
    s32 words[9];
} WmapBlock36;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 value;
    u8 pad[0x24];
} WmapCell;

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

typedef union
{
    struct
    {
        s16 field_00;
        s16 field_02;
    } fields;
    s32 value;
} WmapSignedPair16;

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

extern s32 D_80054A18[];
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
extern WmapHandler D_800D7474[];
extern WmapHandler D_800D748C[];
extern WmapHandler D_800D753C[];
extern WmapHandler D_800D7554[];
extern WmapHandler D_800D7564[];
extern WmapHandler D_800D757C[];
extern WmapHandler D_800D7594[];
extern WmapHandler D_800D75AC[];
extern WmapHandler D_800D75C4[];
extern WmapHandler D_800D75D4[];
extern WmapHandler D_800D75E4[];
extern WmapHandler D_800D75F4[];
extern WmapHandler D_800D760C[];
extern WmapHandler D_800D7624[];
extern WmapHandler D_800D763C[];
extern WmapHandler D_800D7654[];
extern WmapHandler D_800D766C[];
extern WmapHandler D_800D7684[];
extern WmapHandler D_800D769C[];
extern WmapHandler D_800D76AC[];
extern WmapHandler D_800D76C4[];
extern WmapHandler D_800D76DC[];
extern WmapHandler D_800D76F4[];
extern WmapHandler D_800D770C[];
extern WmapHandler D_800D7774[];
extern WmapHandler D_800D778C[];
extern WmapHandler D_800D77A4[];
extern WmapHandler D_800D77BC[];
extern WmapHandler D_800D77D4[];
extern WmapHandler D_800D77E4[];
extern WmapHandler D_800D77F4[];
extern WmapHandler D_800D7804[];
extern WmapHandler D_800D7814[];
extern WmapHandler D_800D7824[];
extern WmapHandler D_800D78BC[];
extern WmapHandler D_800D78CC[];
extern WmapHandler D_800D78A4[];
extern WmapHandler D_800D788C[];
extern WmapHandler D_800D7874[];
extern WmapHandler D_800D7854[];
extern WmapHandler D_800D783C[];
extern WmapHandler D_800D7924[];
extern WmapHandler D_800D793C[];
extern WmapHandler D_800D7954[];
extern WmapHandler D_800D796C[];
extern WmapHandler D_800D7984[];
extern WmapHandler D_800D799C[];
extern WmapHandler D_800D79B4[];
extern WmapHandler D_800D79CC[];
extern WmapHandler D_800D79E4[];
extern WmapHandler D_800D79FC[];
extern WmapHandler D_800D7A0C[];
extern WmapHandler D_800D7A2C[];
extern u8 D_800D95D8[];
extern u8 D_800D9D68[];
extern u8 D_800DA028[];
extern u8 D_800DAA78[];
extern u8 D_800DB310[];
extern WmapConfigA D_800DB4C8[];
extern s32 D_800DBE70;
extern WmapBlock36 D_800D06BC;
extern WmapBlock36 D_800D9240;
extern WmapAlignedQuad g_wmap_saved_view;
extern s32 D_800DCEB0;
extern WmapShort3 D_800DCEB8;
extern s32 g_wmap_cursor_column;
extern s32 g_wmap_cursor_row;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 D_800DCF18[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9318;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern s32 D_800D9158;
extern s32 D_800D9228;
extern WmapConfigA D_800D9268[];
extern WmapPair16 g_wmap_focus_screen_position;
extern s32* D_8011CF1C;
extern s32* D_8011CF24;
extern s32* D_8011CF28;
extern s32* D_8011CF2C;
extern s32* D_8011CF30;
extern s32* D_8011CF34;
extern s32* D_8011CF38;
extern s32* D_8011CF3C;
extern s32 D_8011D4FC;
extern s32 D_8011D500;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern u8 D_80123538;
extern u8 D_80125538;
extern u8 D_80127538;
extern s32 D_80139228;
extern s32 D_80139234;
extern WmapInt3 D_80139200;
extern WmapShort3 D_80139210;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139244;
extern s32 D_8013924C;
extern s32 D_80139250;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern WmapState* D_80139280;
extern WmapCell D_80139290[][6];
extern VECTOR D_80139870;
extern s32 g_wmap_view_mode;
extern s32 g_wmap_view_scroll_mode;
extern u8 D_80139A28[];
extern u8 D_80139B88[];
extern u8 D_80139C08[];
extern u8 D_80139DE8[];
extern u8 D_80139F78[];
extern WmapAlignedQuad g_wmap_view;
extern WmapInt3 D_80139968;
extern s32 D_80139978;
extern u8 D_80139988[];
extern u8 D_801399A8;
extern void* D_801399AC;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern u8 D_801399C8;
extern void* D_801399CC;
extern u8 D_801399D0;
extern void* D_801399D4;
extern u8 D_801399D8;
extern void* D_801399DC;
extern s32 D_8013B20C;
extern s32 D_8013B208;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern s32 D_8013B294;
extern s32 D_8013B29C;
extern WmapSignedPair16 D_80182D58;
extern WmapPair16 D_80182D60;
extern u8 D_80182E40;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_80182DD8;
extern s32 D_80182DE4;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern s32 D_80182E38;
extern VECTOR g_wmap_camera_translation;
extern VECTOR D_8011CF60;
extern u8 D_8018B240;
extern s32 D_801ADAE0;
extern u8 D_801AFBD0[];
extern VECTOR D_801B2478;
extern WmapPair D_801B2490;
extern WmapPair D_801B2498;
extern SVECTOR D_801B24A0;
extern WmapPair D_801B24A8;
extern s32 D_801B24B4;
extern s32 D_801B25D8;
extern s32 D_801B25DC;
extern s32 D_801B25E0;
extern VECTOR D_801B2650;
extern WmapPair D_801B2670;
extern WmapPair D_801B2678;
extern s32 D_801B2FD8;
extern s32 D_801B2FDC;
extern s32 D_801B2FE0;
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
extern s32 D_801B3048;
extern s32 D_801B304C;
extern s32 D_801B3050;
extern s32 D_801B3054;
extern s32 D_801B3058;
extern s32 D_801B305C;
extern s32 D_801B3060;
extern s32 D_801B3064;
extern s32 D_801B3068;
extern s32 D_801B306C;
extern s32 D_801B3070;
extern s32 D_801B3074;
extern s32 D_801B3078;
extern s32 D_801B307C;
extern s32 D_801B3080;
extern s32 D_801B3084;
extern s32 D_801B3088;
extern s32 D_801B308C;
extern s32 D_801B3090;
extern s32 D_801B3094;
extern s32 D_801B3098;
extern s32 D_801B309C;
extern s32 D_801B30A0;
extern s32 D_801B30A4;
extern s32 D_801B30A8;
extern s32 D_801B30AC;
extern s32 D_801B30B0;
extern s32 D_801B30B4;
extern s32 D_801B30B8;
extern s32 D_801B30BC;
extern s32 D_801B30C0;
extern s32 D_801B30C4;
extern s32 D_801B30C8;
extern s32 D_801B30CC;
extern s32 D_801B30D0;
extern s32 D_801B30D4;
extern s32 D_801B30D8;
extern s32 D_801B30DC;
extern s32 D_801B30E0;
extern s32 D_801B30E4;
extern s32 D_801B30E8;
extern s32 D_801B30EC;
extern s32 D_801B30F0;
extern s32 D_801B30F4;
extern WmapPair D_801B3118;
extern SVECTOR D_801B3120;
extern s32 D_801B3128;
extern s32 D_801B312C;
extern s32 D_801B3130;
extern s32 D_801B3134;
extern s32 D_801B3138;
extern s32 D_801B313C;
extern s32 D_801B3140;
extern s32 D_801B3144;
extern s32 D_801B3148;
extern s32 D_801B314C;
extern s32 D_801B3150;
extern s32 D_801B3154;
extern s32 D_801B3158;
extern s32 D_801B315C;
extern s32 D_801B3160;
extern s32 D_801B3164;
extern s32 D_801B3168;
extern s32 D_801B316C;
extern s32 D_801B3170;
extern s32 D_801B3174;
extern s32 D_801B3178;
extern s32 D_801B317C;
extern s32 D_801B3180;
extern s32 D_801B3184;
extern s32 D_801B3188;
extern s32 D_801B318C;
extern s32 D_801B3190;
extern s32 D_801B3194;
extern s32 D_801B3198;
extern s32 D_801B319C;
extern s32 D_801B31A0;
extern s32 D_801B31A4;
extern s32 D_801B31A8;
extern s32 D_801B31AC;
extern s32 D_801B31B0;
extern s32 D_801B31B4;
extern s32 D_801B31B8;
extern s32 D_801B31BC;
extern s32 D_801B31C0;
extern s32 D_801B31C4;
extern s32 D_801B31C8;
extern s32 D_801B31CC;
extern s32 D_801B31D0;
extern s32 D_801B31D4;
extern s32 D_801B31D8;
extern s32 D_801B31DC;
extern s32 D_801B31E0;
extern s32 D_801B31E4;
extern s32 D_801B31E8;
extern s32 D_801B31EC;
extern s32 D_801B31F0;
extern s32 D_801B31F4;
extern s32 D_801B31F8;
extern s32 D_801B31FC;
extern s32 D_801B3200;
extern s32 D_801B3204;
extern s32 D_801B3208;
extern s32 D_801B320C;
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

extern void akao_cmd_a9(s32, s32);

extern void func_8008ECF8(s32, s32, s32*, void*);
extern void func_800B37EC(void);
extern void func_800B38EC(void);
extern void func_800B35F0(void);
extern void func_800B36F0(void);
extern void func_800B4E18(void);
extern void func_800B4FBC(void);
extern void func_800B39E8(void);
extern void func_800B3AE8(void);
extern void func_800B3BE4(void);
extern void func_800B3CE0(void);
extern void func_800B3DD8(void);
extern void func_800B3EDC(void);
extern void func_800B3FDC(void);
extern void func_800B40DC(void);
extern void func_800B41D8(void);
extern void func_800B42DC(void);
extern void func_800B43DC(void);
extern void func_800B5C7C(void);
extern void func_800B619C(void);
extern void func_800B6284(void);
extern void func_800B6558(void);
extern void func_800B6644(void);
extern void func_800B6730(void);
extern void func_800B681C(void);
extern void func_800B6918(void);
extern void func_800B6A10(void);
extern void func_800B6B0C(void);
extern void func_800B6C04(void);
extern void func_800B6D00(void);
extern void func_800B6DF8(void);
extern void func_800B6EF4(void);
extern void func_800B6FEC(void);
extern void func_800B70CC(void);
extern void func_800BA108(void);
extern void func_800BA408(void);
extern void func_800BA580();
extern void func_800BA66C();
extern void func_800BA758();
extern void func_800BAAB0();
extern void func_800BAB80();
extern void func_800BA1BC();
extern void func_800B9F1C();
extern void func_800B9FE4();
extern void func_800B8794(void);
extern void func_800B8988(void);
extern void func_800B95CC(void);
extern void func_800B96B8(void);
extern void func_800B9850(void);
extern void func_800B9938(void);
extern void func_800B9B08(void);
extern void func_800B9D74(void);
extern void func_800BAEF8();
extern void func_800BAF34();
extern void func_800BAF78();
extern void func_800BAFB4();
extern s32 func_800BAFCC(s32);
extern s32 func_800BB610(s32);
extern s32 func_800BB87C(s32);
extern s32 func_800BBAE8(s32);
extern void func_800BCDE0();
extern void func_800BCCF0();
extern void func_800BCAC8();
extern void func_800BC8C4();
extern void func_800BAC44();
extern void func_800BAD38();
extern void func_800BD258();
extern void func_800BD354();
extern void func_800BD750(void);
extern s32 func_800BD998(s32);
extern void func_800BD934(void);
extern void func_800BD970(void);
extern s32 func_800BE1A8(s32);
extern s32 func_800BE40C(s32);
extern s32 func_800BE678(s32);
extern s32 func_800BE8E4(s32);
extern s32 func_800BEB50(s32);
extern s32 func_800BED50(s32);
extern s32 func_800BEF58(s32);
extern s32 func_800BF160(s32);
extern s32 func_800BF368(s32);
extern s32 func_800BDF40(s32);
extern void func_800BE04C(void);
extern void func_800BE114(void);
extern void func_800BE2B0(void);
extern void func_800BE378(void);
extern void func_800BE51C(void);
extern void func_800BE5E4(void);
extern void func_800BE788(void);
extern void func_800BE850(void);
extern void func_800BE9F4(void);
extern void func_800BEABC(void);
extern void func_800BECB0(void);
extern void func_800BEEB4(void);
extern void func_800BF0BC(void);
extern void func_800BF1F0(void);
extern void func_800BF2C4(void);
extern void func_800BEFE8(void);
extern void func_800BFEEC(void);
extern void func_800BFF28(void);
extern void func_800BFF98(void);
extern void func_800BFFD8(void);
extern s32 func_800C0034(s32);
extern void func_800C06DC(void);
extern void func_800C071C(void);
extern void func_800C075C(void);
extern void func_800C079C(void);
extern s32 func_800BC01C(s32);
extern s32 func_800BC1BC(s32);
extern s32 func_800BC360(s32);
extern s32 func_800BC4B8(s32);
extern s32 func_800BC60C(s32);
extern s32 func_800BC764(s32);
extern s32 func_800BC964(s32);
extern void func_800BC7F4(void);
extern void func_800BC9F4(void);
extern void func_800BCBFC(void);
extern s32 func_800BCB6C(s32);
extern s32 func_800BCEA8(s32);
extern s32 func_800BCFD4(s32);
extern s32 func_800BD12C(s32);
extern s32 func_800BBD54(s32);
extern void func_800BB720();
extern void func_800BB7E8();
extern void func_800BB98C();
extern void func_800BBA54();
extern void func_800BBBF8();
extern void func_800BBCC0();
extern void func_800BBF88();
extern void func_800BC128();
extern void func_800BC2CC();

extern s32 func_800B7438(s32);
extern s32 func_800B7E7C(s32);
extern s32 func_800B8084(s32);
extern s32 func_800B822C(s32);
extern s32 func_800B8B80(s32);
extern s32 func_800B8CD8(s32);
extern s32 func_800B8E30(s32);
extern s32 func_800B8F8C(s32);
extern s32 func_800B90E8(s32);
extern s32 func_800B9244(s32);
extern s32 func_800B99F8(s32);
extern s32 func_800B9B9C(s32);
extern s32 func_800BA078(s32);
extern s32 func_800B84D0(s32);
extern s32 func_800B862C(s32);
extern s32 func_800B8824(s32);
extern s32 func_800B8A2C(s32);
extern s32 func_800B93A0(s32);
extern s32 func_800B94FC(s32);
extern s32 func_800B977C(s32);
extern s32 func_800B9E08(s32);
extern s32 func_800B5050(s32);
extern s32 func_800B51AC(s32);
extern s32 func_800B4EAC(s32);
extern s32 func_800B5308(s32);
extern s32 func_800B4D08(s32);
extern s32 func_800B571C(s32);
extern s32 func_800B55C0(s32);
extern s32 func_800B5878(s32);
extern s32 func_800B5B2C(s32);
extern s32 func_800B4774(s32);
extern void func_800B4688(void);
extern void func_800B470C(void);
extern void func_800B4748(void);

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
typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s16 field_04;
} WmapShort3;

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
} WmapInt3;

typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
    s32 field_0C;
} WmapAlignedQuad;

typedef struct
{
    s32 words[9];
} WmapBlock36;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 value;
    u8 pad[0x24];
} WmapCell;

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

typedef union
{
    struct
    {
        s16 field_00;
        s16 field_02;
    } fields;
    s32 value;
} WmapSignedPair16;

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

extern s32 D_80054A18[];
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
extern WmapHandler D_800D7474[];
extern WmapHandler D_800D748C[];
extern WmapHandler D_800D753C[];
extern WmapHandler D_800D7554[];
extern WmapHandler D_800D7564[];
extern WmapHandler D_800D757C[];
extern WmapHandler D_800D7594[];
extern WmapHandler D_800D75AC[];
extern WmapHandler D_800D75C4[];
extern WmapHandler D_800D75D4[];
extern WmapHandler D_800D75E4[];
extern WmapHandler D_800D75F4[];
extern WmapHandler D_800D760C[];
extern WmapHandler D_800D7624[];
extern WmapHandler D_800D763C[];
extern WmapHandler D_800D7654[];
extern WmapHandler D_800D766C[];
extern WmapHandler D_800D7684[];
extern WmapHandler D_800D769C[];
extern WmapHandler D_800D76AC[];
extern WmapHandler D_800D76C4[];
extern WmapHandler D_800D76DC[];
extern WmapHandler D_800D76F4[];
extern WmapHandler D_800D770C[];
extern WmapHandler D_800D7774[];
extern WmapHandler D_800D778C[];
extern WmapHandler D_800D77A4[];
extern WmapHandler D_800D77BC[];
extern WmapHandler D_800D77D4[];
extern WmapHandler D_800D77E4[];
extern WmapHandler D_800D77F4[];
extern WmapHandler D_800D7804[];
extern WmapHandler D_800D7814[];
extern WmapHandler D_800D7824[];
extern WmapHandler D_800D78BC[];
extern WmapHandler D_800D78CC[];
extern WmapHandler D_800D78A4[];
extern WmapHandler D_800D788C[];
extern WmapHandler D_800D7874[];
extern WmapHandler D_800D7854[];
extern WmapHandler D_800D783C[];
extern WmapHandler D_800D7924[];
extern WmapHandler D_800D793C[];
extern WmapHandler D_800D7954[];
extern WmapHandler D_800D796C[];
extern WmapHandler D_800D7984[];
extern WmapHandler D_800D799C[];
extern WmapHandler D_800D79B4[];
extern WmapHandler D_800D79CC[];
extern WmapHandler D_800D79E4[];
extern WmapHandler D_800D79FC[];
extern WmapHandler D_800D7A0C[];
extern WmapHandler D_800D7A2C[];
extern u8 D_800D95D8[];
extern u8 D_800D9D68[];
extern u8 D_800DA028[];
extern u8 D_800DAA78[];
extern u8 D_800DB310[];
extern WmapConfigA D_800DB4C8[];
extern s32 D_800DBE70;
extern WmapBlock36 D_800D06BC;
extern WmapBlock36 D_800D9240;
extern WmapAlignedQuad g_wmap_saved_view;
extern s32 D_800DCEB0;
extern WmapShort3 D_800DCEB8;
extern s32 g_wmap_cursor_column;
extern s32 g_wmap_cursor_row;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 D_800DCF18[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9318;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern s32 D_800D9158;
extern s32 D_800D9228;
extern WmapConfigA D_800D9268[];
extern WmapPair16 g_wmap_focus_screen_position;
extern s32* D_8011CF1C;
extern s32* D_8011CF24;
extern s32* D_8011CF28;
extern s32* D_8011CF2C;
extern s32* D_8011CF30;
extern s32* D_8011CF34;
extern s32* D_8011CF38;
extern s32* D_8011CF3C;
extern s32 D_8011D4FC;
extern s32 D_8011D500;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern u8 D_80123538;
extern u8 D_80125538;
extern u8 D_80127538;
extern s32 D_80139228;
extern s32 D_80139234;
extern WmapInt3 D_80139200;
extern WmapShort3 D_80139210;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139244;
extern s32 D_8013924C;
extern s32 D_80139250;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern WmapState* D_80139280;
extern WmapCell D_80139290[][6];
extern VECTOR D_80139870;
extern s32 g_wmap_view_mode;
extern s32 g_wmap_view_scroll_mode;
extern u8 D_80139A28[];
extern u8 D_80139B88[];
extern u8 D_80139C08[];
extern u8 D_80139DE8[];
extern u8 D_80139F78[];
extern WmapAlignedQuad g_wmap_view;
extern WmapInt3 D_80139968;
extern s32 D_80139978;
extern u8 D_80139988[];
extern u8 D_801399A8;
extern void* D_801399AC;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern u8 D_801399C8;
extern void* D_801399CC;
extern u8 D_801399D0;
extern void* D_801399D4;
extern u8 D_801399D8;
extern void* D_801399DC;
extern s32 D_8013B20C;
extern s32 D_8013B208;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern s32 D_8013B294;
extern s32 D_8013B29C;
extern WmapSignedPair16 D_80182D58;
extern WmapPair16 D_80182D60;
extern u8 D_80182E40;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_80182DD8;
extern s32 D_80182DE4;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern s32 D_80182E38;
extern VECTOR g_wmap_camera_translation;
extern VECTOR D_8011CF60;
extern u8 D_8018B240;
extern s32 D_801ADAE0;
extern u8 D_801AFBD0[];
extern VECTOR D_801B2478;
extern WmapPair D_801B2490;
extern WmapPair D_801B2498;
extern SVECTOR D_801B24A0;
extern WmapPair D_801B24A8;
extern s32 D_801B24B4;
extern s32 D_801B25D8;
extern s32 D_801B25DC;
extern s32 D_801B25E0;
extern VECTOR D_801B2650;
extern WmapPair D_801B2670;
extern WmapPair D_801B2678;
extern s32 D_801B2FD8;
extern s32 D_801B2FDC;
extern s32 D_801B2FE0;
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
extern s32 D_801B3048;
extern s32 D_801B304C;
extern s32 D_801B3050;
extern s32 D_801B3054;
extern s32 D_801B3058;
extern s32 D_801B305C;
extern s32 D_801B3060;
extern s32 D_801B3064;
extern s32 D_801B3068;
extern s32 D_801B306C;
extern s32 D_801B3070;
extern s32 D_801B3074;
extern s32 D_801B3078;
extern s32 D_801B307C;
extern s32 D_801B3080;
extern s32 D_801B3084;
extern s32 D_801B3088;
extern s32 D_801B308C;
extern s32 D_801B3090;
extern s32 D_801B3094;
extern s32 D_801B3098;
extern s32 D_801B309C;
extern s32 D_801B30A0;
extern s32 D_801B30A4;
extern s32 D_801B30A8;
extern s32 D_801B30AC;
extern s32 D_801B30B0;
extern s32 D_801B30B4;
extern s32 D_801B30B8;
extern s32 D_801B30BC;
extern s32 D_801B30C0;
extern s32 D_801B30C4;
extern s32 D_801B30C8;
extern s32 D_801B30CC;
extern s32 D_801B30D0;
extern s32 D_801B30D4;
extern s32 D_801B30D8;
extern s32 D_801B30DC;
extern s32 D_801B30E0;
extern s32 D_801B30E4;
extern s32 D_801B30E8;
extern s32 D_801B30EC;
extern s32 D_801B30F0;
extern s32 D_801B30F4;
extern WmapPair D_801B3118;
extern SVECTOR D_801B3120;
extern s32 D_801B3128;
extern s32 D_801B312C;
extern s32 D_801B3130;
extern s32 D_801B3134;
extern s32 D_801B3138;
extern s32 D_801B313C;
extern s32 D_801B3140;
extern s32 D_801B3144;
extern s32 D_801B3148;
extern s32 D_801B314C;
extern s32 D_801B3150;
extern s32 D_801B3154;
extern s32 D_801B3158;
extern s32 D_801B315C;
extern s32 D_801B3160;
extern s32 D_801B3164;
extern s32 D_801B3168;
extern s32 D_801B316C;
extern s32 D_801B3170;
extern s32 D_801B3174;
extern s32 D_801B3178;
extern s32 D_801B317C;
extern s32 D_801B3180;
extern s32 D_801B3184;
extern s32 D_801B3188;
extern s32 D_801B318C;
extern s32 D_801B3190;
extern s32 D_801B3194;
extern s32 D_801B3198;
extern s32 D_801B319C;
extern s32 D_801B31A0;
extern s32 D_801B31A4;
extern s32 D_801B31A8;
extern s32 D_801B31AC;
extern s32 D_801B31B0;
extern s32 D_801B31B4;
extern s32 D_801B31B8;
extern s32 D_801B31BC;
extern s32 D_801B31C0;
extern s32 D_801B31C4;
extern s32 D_801B31C8;
extern s32 D_801B31CC;
extern s32 D_801B31D0;
extern s32 D_801B31D4;
extern s32 D_801B31D8;
extern s32 D_801B31DC;
extern s32 D_801B31E0;
extern s32 D_801B31E4;
extern s32 D_801B31E8;
extern s32 D_801B31EC;
extern s32 D_801B31F0;
extern s32 D_801B31F4;
extern s32 D_801B31F8;
extern s32 D_801B31FC;
extern s32 D_801B3200;
extern s32 D_801B3204;
extern s32 D_801B3208;
extern s32 D_801B320C;
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

extern void akao_cmd_a9(s32, s32);

extern void func_8008ECF8(s32, s32, s32*, void*);
extern void func_800B37EC(void);
extern void func_800B38EC(void);
extern void func_800B35F0(void);
extern void func_800B36F0(void);
extern void func_800B4E18(void);
extern void func_800B4FBC(void);
extern void func_800B39E8(void);
extern void func_800B3AE8(void);
extern void func_800B3BE4(void);
extern void func_800B3CE0(void);
extern void func_800B3DD8(void);
extern void func_800B3EDC(void);
extern void func_800B3FDC(void);
extern void func_800B40DC(void);
extern void func_800B41D8(void);
extern void func_800B42DC(void);
extern void func_800B43DC(void);
extern void func_800B5C7C(void);
extern void func_800B619C(void);
extern void func_800B6284(void);
extern void func_800B6558(void);
extern void func_800B6644(void);
extern void func_800B6730(void);
extern void func_800B681C(void);
extern void func_800B6918(void);
extern void func_800B6A10(void);
extern void func_800B6B0C(void);
extern void func_800B6C04(void);
extern void func_800B6D00(void);
extern void func_800B6DF8(void);
extern void func_800B6EF4(void);
extern void func_800B6FEC(void);
extern void func_800B70CC(void);
extern void func_800BA108(void);
extern void func_800BA408(void);
extern void func_800BA580();
extern void func_800BA66C();
extern void func_800BA758();
extern void func_800BAAB0();
extern void func_800BAB80();
extern void func_800BA1BC();
extern void func_800B9F1C();
extern void func_800B9FE4();
extern void func_800B8794(void);
extern void func_800B8988(void);
extern void func_800B95CC(void);
extern void func_800B96B8(void);
extern void func_800B9850(void);
extern void func_800B9938(void);
extern void func_800B9B08(void);
extern void func_800B9D74(void);
extern void func_800BAEF8();
extern void func_800BAF34();
extern void func_800BAF78();
extern void func_800BAFB4();
extern s32 func_800BAFCC(s32);
extern s32 func_800BB610(s32);
extern s32 func_800BB87C(s32);
extern s32 func_800BBAE8(s32);
extern void func_800BCDE0();
extern void func_800BCCF0();
extern void func_800BCAC8();
extern void func_800BC8C4();
extern void func_800BAC44();
extern void func_800BAD38();
extern void func_800BD258();
extern void func_800BD354();
extern void func_800BD750(void);
extern s32 func_800BD998(s32);
extern void func_800BD934(void);
extern void func_800BD970(void);
extern s32 func_800BE1A8(s32);
extern s32 func_800BE40C(s32);
extern s32 func_800BE678(s32);
extern s32 func_800BE8E4(s32);
extern s32 func_800BEB50(s32);
extern s32 func_800BED50(s32);
extern s32 func_800BEF58(s32);
extern s32 func_800BF160(s32);
extern s32 func_800BF368(s32);
extern s32 func_800BDF40(s32);
extern void func_800BE04C(void);
extern void func_800BE114(void);
extern void func_800BE2B0(void);
extern void func_800BE378(void);
extern void func_800BE51C(void);
extern void func_800BE5E4(void);
extern void func_800BE788(void);
extern void func_800BE850(void);
extern void func_800BE9F4(void);
extern void func_800BEABC(void);
extern void func_800BECB0(void);
extern void func_800BEEB4(void);
extern void func_800BF0BC(void);
extern void func_800BF1F0(void);
extern void func_800BF2C4(void);
extern void func_800BEFE8(void);
extern void func_800BFEEC(void);
extern void func_800BFF28(void);
extern void func_800BFF98(void);
extern void func_800BFFD8(void);
extern s32 func_800C0034(s32);
extern void func_800C06DC(void);
extern void func_800C071C(void);
extern void func_800C075C(void);
extern void func_800C079C(void);
extern s32 func_800BC01C(s32);
extern s32 func_800BC1BC(s32);
extern s32 func_800BC360(s32);
extern s32 func_800BC4B8(s32);
extern s32 func_800BC60C(s32);
extern s32 func_800BC764(s32);
extern s32 func_800BC964(s32);
extern void func_800BC7F4(void);
extern void func_800BC9F4(void);
extern void func_800BCBFC(void);
extern s32 func_800BCB6C(s32);
extern s32 func_800BCEA8(s32);
extern s32 func_800BCFD4(s32);
extern s32 func_800BD12C(s32);
extern s32 func_800BBD54(s32);
extern void func_800BB720();
extern void func_800BB7E8();
extern void func_800BB98C();
extern void func_800BBA54();
extern void func_800BBBF8();
extern void func_800BBCC0();
extern void func_800BBF88();
extern void func_800BC128();
extern void func_800BC2CC();

extern s32 func_800B7438(s32);
extern s32 func_800B7E7C(s32);
extern s32 func_800B8084(s32);
extern s32 func_800B822C(s32);
extern s32 func_800B8B80(s32);
extern s32 func_800B8CD8(s32);
extern s32 func_800B8E30(s32);
extern s32 func_800B8F8C(s32);
extern s32 func_800B90E8(s32);
extern s32 func_800B9244(s32);
extern s32 func_800B99F8(s32);
extern s32 func_800B9B9C(s32);
extern s32 func_800BA078(s32);
extern s32 func_800B84D0(s32);
extern s32 func_800B862C(s32);
extern s32 func_800B8824(s32);
extern s32 func_800B8A2C(s32);
extern s32 func_800B93A0(s32);
extern s32 func_800B94FC(s32);
extern s32 func_800B977C(s32);
extern s32 func_800B9E08(s32);
extern s32 func_800B5050(s32);
extern s32 func_800B51AC(s32);
extern s32 func_800B4EAC(s32);
extern s32 func_800B5308(s32);
extern s32 func_800B4D08(s32);
extern s32 func_800B571C(s32);
extern s32 func_800B55C0(s32);
extern s32 func_800B5878(s32);
extern s32 func_800B5B2C(s32);
extern s32 func_800B4774(s32);
extern void func_800B4688(void);
extern void func_800B470C(void);
extern void func_800B4748(void);

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
