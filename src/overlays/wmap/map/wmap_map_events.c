#include "../internal/wmap_main.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_party_travel.h"
#include "../internal/wmap_pathfinding.h"
#include "../internal/wmap_map_events.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_sprite_render.h"
#include <libgte.h>
#include <inline_c.h>
#include "sdk/gte_dmpsx_compat.h"
#include "main/cdrom.h"
#include "../internal/wmap_effect_backdrop.h"
#include "../internal/wmap_effect_resources.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_map_labels.h"
#include "main/audio/akao_cmd.h"
#include "../internal/wmap_cells.h"


void func_800A76F8(void);
void func_800A7738(void);
void func_800A7C78(void);
void func_800A7D40(void);
void func_800A8060(void);
void func_800A8128(void);
void func_800A8448(void);
void func_800A8510(void);
void func_800A8864(void);
void func_800A8968(void);
void func_800A7400(void);
void func_800A7440(void);
void func_800A7544(void);
void func_800A7580(void);
void func_800A75C0(void);
void func_800A77CC(void);
void func_800A78B0(void);
void func_800A79E4(void);
void func_800A7AA0(void);
void func_800A7B78(void);
void func_800A7BB8(void);
void func_800A7CB8(void);
void func_800A7D7C(void);
void func_800A7E0C(void);
void func_800A7E4C(void);
void func_800A7F6C(void);
void func_800A7FAC(void);
void func_800A80A0(void);
void func_800A8164(void);
void func_800A81F4(void);
void func_800A8234(void);
void func_800A8354(void);
void func_800A8394(void);
void func_800A8488(void);
void func_800A854C(void);
void func_800A85DC(void);
void func_800A861C(void);
void func_800A8770(void);
void func_800A87B0(void);
void func_800A88A4(void);
void func_800A89A8(void);

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    WmapScreenPosition screen;
} WmapMotion;

typedef struct { s32 w[4]; } WmapBlk16;

extern u8 D_800DCEF4[4];
extern s32 g_wmap_sequence_count;
extern s32 D_8013B288;
extern s32 D_8018222C;
extern s8 D_800DCEF5;
extern s8 D_800DCEF6;
extern s8 D_800DCEF7;
extern s32 rand(void);
extern s32 D_80139234;
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_801B2E44;
extern u8 D_800DCA98[];
extern s32 D_801B2E74;
extern void (*D_800D6D34[])(void);
extern void wmap_draw_vehicle(void);
extern void wmap_finish_vehicle_turn(void);
extern s32 g_wmap_vehicle_phase;
extern s32 g_wmap_vehicle_screen_position;
extern WmapAnimationSlot g_wmap_vehicle_animation;
extern void (*D_800D6C14[])(void);
extern s32 D_801B2E4C;
extern void (*D_800D6C54[])(void);
extern s32 D_801B2E54;
extern void (*D_800D6C94[])(void);
extern s32 D_801B2E5C;
extern void (*D_800D6CD4[])(void);
extern s32 D_801B2E64;
extern void (*D_800D6D14[])(void);
extern s32 D_801B2E6C;
extern void (*D_800D6D24[])(void);

extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern u8 g_wmap_animation_bank_0[];
extern u8 g_wmap_animation_bank_3[];
extern u32 D_801B2E40;
extern u32 D_801B2E50;
extern u32 D_801B2E58;
extern u32 D_801B2E60;
extern u32 D_801B2E68;
extern u32 D_801B2E70;
extern s8 *wmap_turn_vehicle(s32 arg0);
extern void func_800591A8(s32 arg0);
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern u32 D_801B2E48;
extern u8 D_800DCA98[];

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

extern WmapSpriteActor g_wmap_vehicle_actor;

extern WmapAnimationSlot g_wmap_actor_animations[];

extern WmapMotion g_wmap_actor_motions[];
extern WmapMotion D_801AFD60[];

static inline s32 tile_exists(s32 x, s32 y)
{
    if (x < 0 || y < 0 || x >= 6 || y >= 6)
    {
        return 0;
    }
    return g_wmap_cells[x][y].land_id != 255;
}

/** @brief Dispatch the first pending map event and consume one event tick. */
void func_800A5DFC(void)
{
    s32 wmap_run_special_travel(s32 initialize);
    s32 wmap_run_special_return(s32 initialize);
    s32 wmap_land_event_17_run(s32 initialize);
    s32 wmap_land_event_00_a_run(s32 initialize);
    s32 wmap_land_event_00_b_run_sequence_1(s32 initialize);
    s32 wmap_effect35_run(s32 initialize);

    s32 event_index;
    u8 *event;

    if (g_wmap_sequence_count == 0)
    {
        g_wmap_input_locked = 1;
        g_wmap_buttons_held = 0;
        g_wmap_buttons_repeat = 0;
        if (g_wmap_land_event_27_pending != 0)
        {
            g_wmap_land_event_27_pending = 0;
            wmap_start_sequence(&wmap_effect35_run);
        }
        else if (D_8013B288 != 0)
        {
            D_8013B288 = 0;
            wmap_start_sequence(&wmap_run_special_travel);
        }
        else if (g_wmap_land_event_24_pending != 0)
        {
            cdrom_queue_read(0x1145, &g_wmap_animation_bank_0);
            cdrom_queue_read(0x1146, (u8 *)&g_wmap_animation_bank_0 + 0x2000);
            cdrom_wait_queue_empty();
            g_wmap_land_event_24_pending = 0;
            wmap_start_sequence(&func_800A7370);
        }
        else if (g_wmap_special_return_pending != 0)
        {
            g_wmap_special_return_pending = 0;
            wmap_start_sequence(&wmap_run_special_return);
        }
        else if (g_wmap_land_event_10_active != 0)
        {
            g_wmap_land_event_09_pending = 0;
            wmap_start_sequence(&wmap_land_event_00_b_run_sequence_1);
        }
        else if (g_wmap_land_event_09_pending != 0)
        {
            g_wmap_land_event_09_pending = 0;
            wmap_start_sequence(&wmap_land_event_00_a_run);
        }
        else if (g_wmap_land_event_11_pending != 0)
        {
            g_wmap_land_event_11_pending = 0;
            wmap_start_sequence(&wmap_land_event_17_run);
        }
        else if (g_wmap_land_event_16_pending != 0)
        {
            g_wmap_land_event_16_pending = 0;
            wmap_start_sequence(&func_800A7BE8);
        }
        else if (g_wmap_land_event_21_pending != 0)
        {
            g_wmap_land_event_21_pending = 0;
            wmap_start_sequence(&func_800A7FD0);
        }
        else if (g_wmap_land_event_15_pending != 0)
        {
            g_wmap_land_event_15_pending = 0;
            wmap_start_sequence(&func_800A83B8);
        }
        else if (g_wmap_land_event_13_pending != 0)
        {
            g_wmap_land_event_13_pending = 0;
            wmap_start_sequence(&func_800A87D4);
        }
        else if (g_wmap_land_event_12_pending != 0)
        {
            g_wmap_land_event_12_pending = 0;
            wmap_start_sequence(&func_800A88D8);
        }
        else
        {
            for (event_index = 0; event_index < 8; event_index++)
            {
                event = &g_wmap_pending_vehicle_events[event_index];
                if (*event != 0)
                {
                    *event = 0;
                    switch (event_index)
                    {
                    case 0:
                        cdrom_queue_read(0x1149, &g_wmap_animation_bank_3);
                        cdrom_wait_queue_empty();
                        func_80064F64(0x114A);
                        wmap_find_land_cell(4, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
                        D_8018222C = 1;
                        D_800DCEF4[3] = 1;
                        D_800DCEF4[2] = 1;
                        D_800DCEF4[1] = 1;
                        D_800DCEF4[0] = 1;
                        break;
                    case 1:
                        cdrom_queue_read(0x10DE, &g_wmap_animation_bank_3);
                        cdrom_wait_queue_empty();
                        func_80064F64(0x10DF);
                        func_800A61BC(11);
                        break;
                    case 2:
                        cdrom_queue_read(0x10D8, &g_wmap_animation_bank_3);
                        cdrom_wait_queue_empty();
                        func_80064F64(0x10D9);
                        func_800A61BC(17);
                        break;
                    case 3:
                        cdrom_queue_read(0x10DC, &g_wmap_animation_bank_3);
                        cdrom_wait_queue_empty();
                        func_80064F64(0x10DD);
                        func_800A61BC(11);
                        break;
                    case 4:
                        cdrom_queue_read(0x10DA, &g_wmap_animation_bank_3);
                        cdrom_wait_queue_empty();
                        func_80064F64(0x10DB);
                        func_800A61BC(10);
                        break;
                    }
                    wmap_start_sequence(func_800A7668);
                    break;
                }
            }
        }
        g_wmap_pending_event_count -= 1;
    }
}

/** @brief Select the first occupied neighbor, or a random direction when isolated. */
void func_800A61BC(s32 tile)
{
    wmap_find_land_cell(tile, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    D_8018222C = 0;
    D_800DCEF4[3] = 0;
    D_800DCEF4[2] = 0;
    D_800DCEF4[1] = 0;
    D_800DCEF4[0] = 0;
    if (tile_exists(g_wmap_vehicle_cell_x + 1, g_wmap_vehicle_cell_y))
    {
        D_800DCEF4[0] = 1;
        return;
    }
    if (tile_exists(g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y + 1))
    {
        D_800DCEF5 = 1;
        return;
    }
    if (tile_exists(g_wmap_vehicle_cell_x - 1, g_wmap_vehicle_cell_y))
    {
        D_800DCEF6 = 1;
        return;
    }
    if (tile_exists(g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y - 1))
    {
        D_800DCEF7 = 1;
        return;
    }
    D_800DCEF4[rand() & 3] = 1;
}

/** @brief Save the projection state and set the next effect's map-relative position. */
void func_800A643C(void)
{
    D_8013B288 = 0;
    D_80139234 = 0;
    g_wmap_input_locked = 1;
    g_wmap_event_active = 1;
    g_wmap_saved_view = g_wmap_view;
    wmap_find_land_cell(4, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 48) - g_wmap_view.x;
    g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 48) - g_wmap_view.y;
    D_801B2E40++;
    func_800A76F8();
}

/** @brief Initialize active directional actors and play the transition sound. */
void func_800A6540(void)
{
    s32 i;
    WmapSpriteActor *actor;

    for (i = 40; i < 120; i++)
    {
        g_wmap_actor_motions[i].state = 0;
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_3;
    }
    for (i = 0; i < 4; i++)
    {
        actor = &g_wmap_sprite_actors[20 + i];
        if (D_800DCEF4[i] != 0)
        {
            g_wmap_actor_animations[i + 20].data = g_wmap_animation_bank_3;
            actor->scale_index = 15;
            actor->sequence = i + 1;
            actor->previous_sequence = -1;
            actor->target_shade = 128;
            actor->shade_step = 2;
            actor->resource_index = 0;
            actor->shade = 0;
            D_801AFD60[i].state = 1;
            D_801AFD60[i].angle = i << 10;
            D_801AFD60[i].z = 90000;
            D_801AFD60[i].x = 900;
            D_801AFD60[i].field_0E = 0;
        }
    }
    akao_fade_song_volume_from(0, 30, 127, 48);
    if (D_800DCEF4[3] & (D_800DCEF4[2] & (D_800DCEF4[0] & D_800DCEF4[1])))
    {
        wmap_play_sound(49, 128);
    }
    else
    {
        wmap_play_sound(21, 128);
    }
    D_801B2E44 = 64;
    D_801B2E40++;
    func_800A7738();
}

/** @brief Project four rotating effect actors and update their draw depths. */
void func_800A66C0(void)
{
    SVECTOR position;
    s32 depth;
    s32 draw_depth;
    s32 i;
    WmapSpriteActor *actor;
    WmapMotion *motion;

    for (i = 0; i < 4; i++)
    {
        motion = &D_801AFD60[i];
        actor = &g_wmap_sprite_actors[20 + i];
        position.vx = ((motion->z >> 3) * (ccos(motion->angle) >> 6)) >> 12;
        position.vy = ((motion->z >> 3) * (csin(motion->angle) >> 6)) >> 12;
        position.vz = motion->field_0E;
        gte_ldv0(&position);
        gte_rtps();
        wmap_step_actor_animation(actor, &g_wmap_actor_animations[20 + i]);
        gte_stsxy(&motion->screen);
        gte_stszotz(&depth);
        draw_depth = (7057 - depth) / 4 + 42;
        wmap_draw_actor_sprite(actor, motion->screen.packed, 3, draw_depth, 0x400);
        motion->scale = draw_depth;
    }
}

/** @brief Draw active trail actors and periodically copy four new trail samples. */
void func_800A6800(void)
{
    s32 i;
    s32 destination;
    WmapSpriteActor *actor;
    WmapMotion *motion;

    if (D_8018222C != 0)
    {
        for (i = 40; i < 104; i++)
        {
            motion = &g_wmap_actor_motions[i];
            if (motion->state != 0)
            {
                actor = &g_wmap_sprite_actors[i];
                wmap_step_actor_animation(actor, &g_wmap_actor_animations[i]);
                wmap_draw_actor_sprite(actor, motion->screen.packed, 3, motion->scale + 1, 0x400);
            }
        }
        if (g_wmap_frame_count % 10 == 0)
        {
            if (D_80139234 != -1)
            {
                for (i = 0; i < 4; i++)
                {
                    destination = i + D_80139234 * 4 + 40;
                    g_wmap_actor_motions[destination] = g_wmap_actor_motions[i + 20];
                    g_wmap_sprite_actors[destination] = g_wmap_sprite_actors[i + 20];
                    g_wmap_sprite_actors[destination].target_shade = 0;
                    g_wmap_sprite_actors[destination].shade_step = 2;
                }
                D_80139234 = (D_80139234 + 1) & 15;
            }
        }
    }
}

/** @brief Load resources and set the effect's map-relative position. */
void func_800A6A20(void)
{
    g_wmap_saved_view = g_wmap_view;
    g_wmap_event_active = 1;
    wmap_find_land_cell(12, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    cdrom_queue_read(0x10E2, D_800DCA98);
    func_80064F64(0x10E3);
    wmap_set_traveler_position(3, g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 48) - g_wmap_view.x;
    g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 48) - g_wmap_view.y;
    D_801B2E48++;
    func_800A7C78();
}

/**
 * @brief World-map step handler: seed a pathfinding move for the actor, populate its
 *        motion record, and advance the step counter.
 */
void func_800A6B34(void)
{
    u8* base;
    u16 a;
    u16 b;

    wmap_find_land_cell(0x10, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    base = (u8*)g_wmap_travelers;
    func_8005EB68(*(s32*)(base + 0x36C), *(s32*)(base + 0x370), g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y,
                  (s32*)(base + 0x390), (s32*)(base + 0x410));
    *(s32*)(base + 0x384) = g_wmap_vehicle_cell_x;
    *(s32*)(base + 0x388) = g_wmap_vehicle_cell_y;
    a = *(u16*)(base + 0x394);
    g_wmap_travelers[3].moving = 1;
    g_wmap_travelers[3].path_index = 1;
    g_wmap_scripted_travel_active = 1;
    *(u16*)(base + 0x374) = a;
    *(u16*)(base + 0x37C) = ((s16)a - 1) * 0xA0;
    b = *(u16*)(base + 0x414);
    *(u16*)(base + 0x376) = b;
    *(u16*)(base + 0x37E) = ((s16)b - 1) * 0xA0;
    D_801B2E48 += 1;
    func_800A7D40();
}

/**
 * @brief World-map step handler: kick off the streamed cell load and seed the scroll
 *        target from the current cell, then advance the step.
 */
void func_800A6C24(void)
{
    *(WmapBlk16*)&g_wmap_saved_view = *(WmapBlk16*)&g_wmap_view;
    g_wmap_event_active = 1;
    wmap_find_land_cell(1, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    cdrom_queue_read(0x10E0, D_800DCA98);
    func_80064F64(0x10E1);
    wmap_set_traveler_position(3, g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = (g_wmap_vehicle_cell_x - 1) * 0x30 - g_wmap_view.x;
    g_wmap_scroll_remaining_y = (g_wmap_vehicle_cell_y - 1) * 0x30 - g_wmap_view.y;
    D_801B2E50 += 1;
    func_800A8060();
}

/**
 * @brief World-map step handler: seed a pathfinding move for the actor, populate its
 *        motion record, and advance the step counter.
 */
void func_800A6D38(void)
{
    u8* base;
    u16 a;
    u16 b;

    wmap_find_land_cell(0x2, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    base = (u8*)g_wmap_travelers;
    func_8005EB68(*(s32*)(base + 0x36C), *(s32*)(base + 0x370), g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y,
                  (s32*)(base + 0x390), (s32*)(base + 0x410));
    *(s32*)(base + 0x384) = g_wmap_vehicle_cell_x;
    *(s32*)(base + 0x388) = g_wmap_vehicle_cell_y;
    a = *(u16*)(base + 0x394);
    g_wmap_travelers[3].moving = 1;
    g_wmap_travelers[3].path_index = 1;
    g_wmap_scripted_travel_active = 1;
    *(u16*)(base + 0x374) = a;
    *(u16*)(base + 0x37C) = ((s16)a - 1) * 0xA0;
    b = *(u16*)(base + 0x414);
    *(u16*)(base + 0x376) = b;
    *(u16*)(base + 0x37E) = ((s16)b - 1) * 0xA0;
    D_801B2E50 += 1;
    func_800A8128();
}

/**
 * @brief World-map step handler: kick off the streamed cell load and seed the scroll
 *        target from the current cell, then advance the step.
 */
void func_800A6E28(void)
{
    *(WmapBlk16*)&g_wmap_saved_view = *(WmapBlk16*)&g_wmap_view;
    g_wmap_event_active = 1;
    wmap_find_land_cell(1, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    cdrom_queue_read(0x1216, D_800DCA98);
    func_80064F64(0x1217);
    wmap_set_traveler_position(3, g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = (g_wmap_vehicle_cell_x - 1) * 0x30 - g_wmap_view.x;
    g_wmap_scroll_remaining_y = (g_wmap_vehicle_cell_y - 1) * 0x30 - g_wmap_view.y;
    D_801B2E58 += 1;
    func_800A8448();
}

/**
 * @brief Build the effect route, initialize its position, and advance the sequence.
 */
void func_800A6F3C(void)
{
    u8* base;
    u16 cell_x;
    u16 cell_y;
    s32 state;
    s32 coordinate;

    wmap_find_land_cell(0x12, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    base = (u8*)g_wmap_travelers;
    func_8005EB68(*(s32*)(base + 0x36C), *(s32*)(base + 0x370), g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y,
                  (s32*)(base + 0x390), (s32*)(base + 0x410));
    *(s32*)(base + 0x384) = g_wmap_vehicle_cell_x;
    *(s32*)(base + 0x388) = g_wmap_vehicle_cell_y;
    state = 1;
    cell_x = *(u16*)(base + 0x394);
    g_wmap_scripted_travel_active = state;
    g_wmap_travelers[3].moving = state;
    g_wmap_travelers[3].path_index = state;
    *(u16*)(base + 0x374) = cell_x;
    coordinate = (s16)cell_x - 1;
    *(u16*)(base + 0x37C) = coordinate * 0xA0;
    cell_y = *(u16*)(base + 0x414);
    coordinate = cell_y;
    *(u16*)(base + 0x376) = cell_y;
    *(u16*)(base + 0x37E) = ((s16)coordinate - 1) * 0xA0;
    wmap_play_sound(50, 128);
    D_801B2E58 += 1;
    func_800A8510();
}

/** @brief World-map step: seed the scroll target from the current cell, then advance. */
void func_800A703C(void)
{
    wmap_select_mesh_motion(1);
    wmap_start_mesh_transition(1);
    wmap_find_land_cell(0x10, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_focus_cell_x = g_wmap_vehicle_cell_x;
    g_wmap_focus_cell_y = g_wmap_vehicle_cell_y;
    g_wmap_scroll_remaining_x = (g_wmap_vehicle_cell_x - 1) * 0x30 - g_wmap_view.x;
    g_wmap_scroll_remaining_y = (g_wmap_vehicle_cell_y - 1) * 0x30 - g_wmap_view.y;
    D_801B2E60 += 1;
    func_800A8864();
}

/** @brief World-map step: seed the scroll target from the current cell, then advance. */
void func_800A7108(void)
{
    wmap_select_mesh_motion(1);
    wmap_start_mesh_transition(1);
    wmap_find_land_cell(0x17, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_focus_cell_x = g_wmap_vehicle_cell_x;
    g_wmap_focus_cell_y = g_wmap_vehicle_cell_y;
    g_wmap_scroll_remaining_x = (g_wmap_vehicle_cell_x - 1) * 0x30 - g_wmap_view.x;
    g_wmap_scroll_remaining_y = (g_wmap_vehicle_cell_y - 1) * 0x30 - g_wmap_view.y;
    D_801B2E68 += 1;
    func_800A8968();
}

/** @brief Set the map-relative effect position, load resources, and advance the sequence. */
void func_800A71D4(void)
{
    g_wmap_vehicle_flap_sound_enabled = 0;
    g_wmap_forced_animated_land_id = 0x18;
    g_wmap_land_display_limit = 0;
    wmap_find_land_cell(0x18, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_focus_cell_x = g_wmap_vehicle_cell_x;
    g_wmap_focus_cell_y = g_wmap_vehicle_cell_y;
    g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 0x30) - g_wmap_view.x;
    g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 0x30) - g_wmap_view.y;
    wmap_queue_effect_resources(0x21);
    D_801B2E70 += 1;
    func_800A7400();
}

/**
 * @brief World-map actor tick: advance timers, bump a wave index, and expire the step.
 */
void func_800A72B8(void)
{
    s8 *obj;
    u8 *base;

    obj = wmap_turn_vehicle(1);
    base = (u8*)g_wmap_actor_motions;
    if (*(s16 *)(base + 0xE) < 100)
    {
        *(s16 *)(base + 0xE) += 1;
    }
    if (*(s32 *)(base + 0x8) < 0x3E8)
    {
        *(s32 *)(base + 0x8) += 0x1E;
    }
    if ((g_wmap_frame_count & 3) == 0)
    {
        if (obj[6] < 0xF)
        {
            obj[6] += 1;
        }
    }
    if (--D_801B2E74 == 0)
    {
        D_801B2E70 += 1;
    }
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
WMAP_STEP_RUNNER(func_800A7370,
    D_800D6D34, 0xA,
    D_801B2E70, D_801B2E74)

/**
 * @brief Restart the sequence at step 1 with a one-frame timer.
 */
WMAP_STEP_RESET(func_800A73E8, D_801B2E70, D_801B2E74)

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(func_800A7400,
    D_801B2E70, func_800A7440)

/** @brief Initialize the actor, register its callback, and start the sequence delay. */
void func_800A7440(void)
{
    g_wmap_vehicle_animation.data = g_wmap_animation_bank_0;
    g_wmap_vehicle_actor.previous_sequence = -1;
    g_wmap_vehicle_actor.resource_index = 0;
    g_wmap_vehicle_actor.scale_index = 0;
    g_wmap_vehicle_actor.sequence = 0;
    g_wmap_vehicle_actor.shade_step = 0;
    g_wmap_vehicle_actor.target_shade = 0x80;
    g_wmap_vehicle_actor.shade = 0x80;
    g_wmap_actor_motions[0].state = 1;
    g_wmap_actor_motions[0].z = 0xC8;
    g_wmap_actor_motions[0].field_0E = 0xA;
    g_wmap_actor_motions[0].screen.point.x = 0x3C;
    g_wmap_actor_motions[0].angle = 0;
    g_wmap_actor_motions[0].x = 2;
    wmap_install_callback(&wmap_draw_vehicle);
    func_800591A8(0x21);
    D_801B2E74 = 0x16E;
    D_801B2E70 += 1;
    func_800A72B8();
}

/** @brief World-map step handler: install a callback, advance the step counter, chain to the next step. */
void func_800A74FC(void)
{
    g_wmap_vehicle_phase = 1;
    wmap_start_sequence(wmap_finish_vehicle_turn);
    D_801B2E70 += 1;
    func_800A7544();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A7544(void)
{
    if (g_wmap_vehicle_phase == 0)
    {
        D_801B2E70 += 1;
        func_800A7580();
    }
}

/**
 * @brief Reset a world-map animation flag and schedule the next step.
 */
void func_800A7580(void)
{
    g_wmap_actor_motions[0].state = 0;
    D_801B2E74 = 0x78;
    D_801B2E70 += 1;
    func_800A75C0();
}

/** @brief Draw the sprite, move its packed coordinate, and update the countdown. */
void func_800A75C0(void)
{
    s32 remaining_ticks;
    u8 *sprite = (u8*)&g_wmap_vehicle_actor;

    wmap_turn_vehicle(0);
    wmap_step_actor_animation(sprite, &g_wmap_vehicle_animation);
    wmap_draw_actor_sprite(sprite, g_wmap_vehicle_screen_position, 0x28, 2, 2);
    remaining_ticks = D_801B2E74 - 1;
    *(s16 *)&g_wmap_vehicle_screen_position -= 4;
    D_801B2E74 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2E70 += 1;
    }
}

/**
 * @brief Advance past the last sequence step.
 */
WMAP_STEP_ADVANCE(func_800A7650, D_801B2E70)

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
WMAP_STEP_RUNNER(func_800A7668,
    D_800D6C14, 0x10,
    D_801B2E40, D_801B2E44)

/**
 * @brief Restart the sequence at step 1 with a one-frame timer.
 */
WMAP_STEP_RESET(func_800A76E0, D_801B2E40, D_801B2E44)

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(func_800A76F8,
    D_801B2E40, func_800A6540)

/** @brief World-map step: run the two sub-steps, then advance after the timer. */
WMAP_STEP_UPDATE_TWO_AND_WAIT(func_800A7738, D_801B2E40, D_801B2E44,
                          func_8006AEE0(),
                          func_800A66C0())

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A778C(void)
{
    g_wmap_land_label_updates_enabled = 0;
    D_801B2E44 = 0x1E;
    D_801B2E40 += 1;
    func_800A77CC();
}

/** @brief Step the world-map particle set, decaying each slot's velocity field. */
void func_800A77CC(void)
{
    s32 i;
    u8* p;

    func_8006AEE0();
    func_800A66C0();
    func_800A6800();
    for (i = 0; i < 4; i++)
    {
        p = (u8*)g_wmap_actor_motions + (0x14 + i) * 0x14;
        *(s32*)(p + 0x8) -= *(s32*)(p + 0x4);
    }
    if (--D_801B2E44 == 0)
    {
        D_801B2E40 += 1;
    }
}

/** @brief Set up the effect, select its delay, and run the next sequence step. */
void func_800A785C(void)
{
    s32 delay;

    func_8005FF88(-1);
    delay = 0x46;
    if (D_8018222C != 0)
    {
        delay = 0x32;
    }
    D_801B2E44 = delay;
    D_801B2E40 += 1;
    func_800A78B0();
}

/** @brief Step the world-map particle set, decaying each slot's velocity field. */
void func_800A78B0(void)
{
    s32 i;
    u8* p;

    func_8006AEE0();
    func_800A66C0();
    func_800A6800();
    for (i = 0; i < 4; i++)
    {
        p = (u8*)g_wmap_actor_motions + (0x14 + i) * 0x14;
        *(s32*)(p + 0x8) -= *(s32*)(p + 0x4);
    }
    if (--D_801B2E44 == 0)
    {
        D_801B2E40 += 1;
    }
}

/** @brief Clear four actor states and start the next timed sequence step. */
void func_800A7940(void)
{
    s32 i;

    if (!(D_800DCEF4[3] & (D_800DCEF4[2] & (D_800DCEF4[0] & D_800DCEF4[1]))))
    {
        akao_release_all_sfx();
    }
    for (i = 0; i < 4; i++)
    {
        g_wmap_sprite_actors[i + 20].sequence = 0;
    }
    D_80139234 = -1;
    D_801B2E44 = 0x28;
    D_801B2E40++;
    func_800A79E4();
}

/** @brief Run three drawing updates and advance when the countdown expires. */
void func_800A79E4(void)
{
    s32 remaining_ticks;

    func_8006AEE0();
    func_800A66C0();
    func_800A6800();
    remaining_ticks = D_801B2E44 - 1;
    D_801B2E44 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2E40 += 1;
    }
}

/** @brief Clear four resource fields and start a 64-tick sequence step. */
void func_800A7A40(void)
{
    s32 index;
    for (index = 0; index < 4; index++)
    {
        g_wmap_sprite_actors[index + 20].target_shade = 0;
    }
    D_801B2E44 = 64;
    D_801B2E40 += 1;
    func_800A7AA0();
}

/** @brief World-map step: run the two sub-steps, then advance after the timer. */
WMAP_STEP_UPDATE_TWO_AND_WAIT(func_800A7AA0, D_801B2E40, D_801B2E44,
                          func_8006AEE0(),
                          func_800A66C0())

/** @brief Start audio, compute the coordinate delta, and advance the sequence. */
void func_800A7AF4(void)
{
    akao_fade_song_volume_from(0, 0x1E, 0x30, 0x7F);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = g_wmap_saved_view.x - g_wmap_view.x;
    g_wmap_scroll_remaining_y = g_wmap_saved_view.y - g_wmap_view.y;
    D_801B2E40 += 1;
    func_800A7B78();
}

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(func_800A7B78,
    D_801B2E40, func_800A7BB8)

/** @brief Reset two world-map values, set the enable flag, and advance the state. */
void func_800A7BB8(void)
{
    g_wmap_input_locked = 0;
    g_wmap_event_active = 0;
    g_wmap_land_label_updates_enabled = 1;
    D_801B2E40 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
WMAP_STEP_RUNNER(func_800A7BE8,
    D_800D6C54, 0x10,
    D_801B2E48, D_801B2E4C)

/**
 * @brief Restart the sequence at step 1 with a one-frame timer.
 */
WMAP_STEP_RESET(func_800A7C60, D_801B2E48, D_801B2E4C)

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(func_800A7C78,
    D_801B2E48, func_800A7CB8)

/** @brief Reset a world-map HUD sprite record, then bump its shared refcount. */
void func_800A7CB8(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[3];

    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->target_shade = 0x80;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->shade = 0;
    actor->shade_step = 8;
    D_801B2E4C = 0x10;
    D_801B2E48 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
WMAP_STEP_WAIT(func_800A7D0C, D_801B2E48, D_801B2E4C)

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A7D40(void)
{
    if (g_wmap_scripted_travel_active == 0)
    {
        D_801B2E48 += 1;
        func_800A7D7C();
    }
}

/**
 * @brief World-map step: recompute the scroll offsets from the camera position and
 *        advance to the next handler.
 */
void func_800A7D7C(void)
{
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = (g_wmap_vehicle_cell_x - 1) * 0x30 - g_wmap_view.x;
    g_wmap_scroll_remaining_y = (g_wmap_vehicle_cell_y - 1) * 0x30 - g_wmap_view.y;
    D_801B2E48 += 1;
    func_800A7E0C();
}

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(func_800A7E0C,
    D_801B2E48, func_800A7E4C)

/**
 * @brief Reset a world-map step slot: arm its wait and advance the step index.
 */
void func_800A7E4C(void)
{
    D_801B2E4C = 0x3C;
    D_801B2E48 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
WMAP_STEP_WAIT(func_800A7E6C, D_801B2E48, D_801B2E4C)

/** @brief World-map step handler: clear a flag and advance the step. */
void func_800A7EA0(void)
{
    g_wmap_sprite_actors[3].target_shade = 0;
    D_801B2E4C = 0x1E;
    D_801B2E48 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
WMAP_STEP_WAIT(func_800A7EC8, D_801B2E48, D_801B2E4C)

/**
 * @brief World-map step handler: cache the relative scroll delta, bump the frame
 *        counter, and run the sub-step handler.
 */
void func_800A7EFC(void)
{
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = g_wmap_saved_view.x - g_wmap_view.x;
    g_wmap_scroll_remaining_y = g_wmap_saved_view.y - g_wmap_view.y;
    D_801B2E48 += 1;
    func_800A7F6C();
}

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(func_800A7F6C,
    D_801B2E48, func_800A7FAC)

/** @brief World-map step handler: clear two flags and advance the step counter. */
void func_800A7FAC(void)
{
    g_wmap_input_locked = 0;
    g_wmap_event_active = 0;
    D_801B2E48 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
WMAP_STEP_RUNNER(func_800A7FD0,
    D_800D6C94, 0x10,
    D_801B2E50, D_801B2E54)

/**
 * @brief Restart the sequence at step 1 with a one-frame timer.
 */
WMAP_STEP_RESET(func_800A8048, D_801B2E50, D_801B2E54)

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(func_800A8060,
    D_801B2E50, func_800A80A0)

/** @brief Reset a world-map HUD sprite record, then bump its shared refcount. */
void func_800A80A0(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[3];

    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->target_shade = 0x80;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->shade = 0;
    actor->shade_step = 8;
    D_801B2E54 = 0x10;
    D_801B2E50 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
WMAP_STEP_WAIT(func_800A80F4, D_801B2E50, D_801B2E54)

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A8128(void)
{
    if (g_wmap_scripted_travel_active == 0)
    {
        D_801B2E50 += 1;
        func_800A8164();
    }
}

/**
 * @brief World-map step: recompute the scroll offsets from the camera position and
 *        advance to the next handler.
 */
void func_800A8164(void)
{
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = (g_wmap_vehicle_cell_x - 1) * 0x30 - g_wmap_view.x;
    g_wmap_scroll_remaining_y = (g_wmap_vehicle_cell_y - 1) * 0x30 - g_wmap_view.y;
    D_801B2E50 += 1;
    func_800A81F4();
}

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(func_800A81F4,
    D_801B2E50, func_800A8234)

/**
 * @brief Reset a world-map step slot: arm its wait and advance the step index.
 */
void func_800A8234(void)
{
    D_801B2E54 = 0x3C;
    D_801B2E50 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
WMAP_STEP_WAIT(func_800A8254, D_801B2E50, D_801B2E54)

/** @brief World-map step handler: clear a flag and advance the step. */
void func_800A8288(void)
{
    g_wmap_sprite_actors[3].target_shade = 0;
    D_801B2E54 = 0x1E;
    D_801B2E50 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
WMAP_STEP_WAIT(func_800A82B0, D_801B2E50, D_801B2E54)

/**
 * @brief World-map step handler: cache the relative scroll delta, bump the frame
 *        counter, and run the sub-step handler.
 */
void func_800A82E4(void)
{
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = g_wmap_saved_view.x - g_wmap_view.x;
    g_wmap_scroll_remaining_y = g_wmap_saved_view.y - g_wmap_view.y;
    D_801B2E50 += 1;
    func_800A8354();
}

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(func_800A8354,
    D_801B2E50, func_800A8394)

/** @brief World-map step handler: clear two flags and advance the step counter. */
void func_800A8394(void)
{
    g_wmap_input_locked = 0;
    g_wmap_event_active = 0;
    D_801B2E50 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
WMAP_STEP_RUNNER(func_800A83B8,
    D_800D6CD4, 0x10,
    D_801B2E58, D_801B2E5C)

/**
 * @brief Restart the sequence at step 1 with a one-frame timer.
 */
WMAP_STEP_RESET(func_800A8430, D_801B2E58, D_801B2E5C)

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(func_800A8448,
    D_801B2E58, func_800A8488)

/** @brief Reset a world-map HUD sprite record, then bump its shared refcount. */
void func_800A8488(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[3];

    actor->scale_index = 0xF;
    actor->previous_sequence = -1;
    actor->target_shade = 0x80;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->shade = 0;
    actor->shade_step = 8;
    D_801B2E5C = 0x10;
    D_801B2E58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
WMAP_STEP_WAIT(func_800A84DC, D_801B2E58, D_801B2E5C)

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A8510(void)
{
    if (g_wmap_scripted_travel_active == 0)
    {
        D_801B2E58 += 1;
        func_800A854C();
    }
}

/**
 * @brief World-map step: recompute the scroll offsets from the camera position and
 *        advance to the next handler.
 */
void func_800A854C(void)
{
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = (g_wmap_vehicle_cell_x - 1) * 0x30 - g_wmap_view.x;
    g_wmap_scroll_remaining_y = (g_wmap_vehicle_cell_y - 1) * 0x30 - g_wmap_view.y;
    D_801B2E58 += 1;
    func_800A85DC();
}

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(func_800A85DC,
    D_801B2E58, func_800A861C)

/** @brief Issue audio command F1, start a 60-tick delay, and advance the state. */
void func_800A861C(void)
{
    akao_release_all_sfx();
    D_801B2E5C = 60;
    D_801B2E58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
WMAP_STEP_WAIT(func_800A8654, D_801B2E58, D_801B2E5C)

/** @brief Play sound 57, clear its field, and start a 30-tick delay. */
void func_800A8688(void)
{
    wmap_play_sound(0x39, 0x80);
    g_wmap_sprite_actors[3].target_shade = 0;
    D_801B2E5C = 0x1E;
    D_801B2E58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
WMAP_STEP_WAIT(func_800A86CC, D_801B2E58, D_801B2E5C)

/**
 * @brief World-map step handler: cache the relative scroll delta, bump the frame
 *        counter, and run the sub-step handler.
 */
void func_800A8700(void)
{
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = g_wmap_saved_view.x - g_wmap_view.x;
    g_wmap_scroll_remaining_y = g_wmap_saved_view.y - g_wmap_view.y;
    D_801B2E58 += 1;
    func_800A8770();
}

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(func_800A8770,
    D_801B2E58, func_800A87B0)

/** @brief World-map step handler: clear two flags and advance the step counter. */
void func_800A87B0(void)
{
    g_wmap_input_locked = 0;
    g_wmap_event_active = 0;
    D_801B2E58 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
WMAP_STEP_RUNNER(func_800A87D4,
    D_800D6D14, 0x4,
    D_801B2E60, D_801B2E64)

/**
 * @brief Restart the sequence at step 1 with a one-frame timer.
 */
WMAP_STEP_RESET(func_800A884C, D_801B2E60, D_801B2E64)

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(func_800A8864,
    D_801B2E60, func_800A88A4)

/** @brief World-map step handler: kick two sub-tasks and expire the step counter. */
void func_800A88A4(void)
{
    wmap_queue_effect_resources(0x17);
    func_800591A8(0x17);
    D_801B2E60 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
WMAP_STEP_RUNNER(func_800A88D8,
    D_800D6D24, 0x4,
    D_801B2E68, D_801B2E6C)

/**
 * @brief Restart the sequence at step 1 with a one-frame timer.
 */
WMAP_STEP_RESET(func_800A8950, D_801B2E68, D_801B2E6C)

/**
 * @brief Wait for scripted map scrolling, then run the next step.
 */
WMAP_STEP_WAIT_SCROLL(func_800A8968,
    D_801B2E68, func_800A89A8)

/** @brief World-map step handler: kick two sub-tasks and expire the step counter. */
void func_800A89A8(void)
{
    wmap_queue_effect_resources(0x16);
    func_800591A8(0x16);
    D_801B2E68 += 1;
}
