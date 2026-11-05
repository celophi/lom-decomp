#include "common.h"
#include <inline_c.h>
#include <libgte.h>
#include <rand.h>

#include "../internal/wmap_cells.h"
#include "../internal/wmap_effect_backdrop.h"
#include "../internal/wmap_effect_primitives.h"
#include "../internal/wmap_effect_resources.h"
#include "../internal/wmap_land_effect_loader.h"
#include "../internal/wmap_land_event_00_a.h"
#include "../internal/wmap_land_event_00_b.h"
#include "../internal/wmap_land_event_17.h"
#include "../internal/wmap_land_layout.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_map_events.h"
#include "../internal/wmap_map_labels.h"
#include "../internal/wmap_party_travel.h"
#include "../internal/wmap_pathfinding.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_special_effect_35.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_step_sequence.h"
#include "../internal/wmap_travel_sequences.h"
#include "../internal/wmap_view_effects.h"
#include "main/audio/akao_cmd.h"
#include "main/cdrom.h"
#include "sdk/gte_dmpsx_compat.h"

/** @brief Land the arrival event scrolls to; event 25 arrives there from all four sides. */
#define WMAP_ARRIVAL_LAND 4

/** @brief Sprite actors and effect motion slots of the four arriving sprites, one per side. */
#define WMAP_ARRIVAL_FIRST_ACTOR 20
#define WMAP_ARRIVAL_SIDES 4

/** @brief Trail sprites: 16 samples of the four arriving sprites, from actor 40 on. */
#define WMAP_ARRIVAL_TRAIL_FIRST_ACTOR 40
#define WMAP_ARRIVAL_TRAIL_SAMPLES 16
#define WMAP_ARRIVAL_TRAIL_END (WMAP_ARRIVAL_TRAIL_FIRST_ACTOR + WMAP_ARRIVAL_TRAIL_SAMPLES * WMAP_ARRIVAL_SIDES)

/** @brief Arriving sprites: start distance and the distance covered per frame. */
#define WMAP_ARRIVAL_START_RADIUS 90000
#define WMAP_ARRIVAL_RADIUS_STEP 900

/** @brief Sprite texture and draw variant of the arriving sprites and their trails. */
#define WMAP_ARRIVAL_TEXTURE 3
#define WMAP_ARRIVAL_VARIANT 0x400

/** @brief Land effects started by events 12, 13 and 24. */
#define WMAP_LAND_EVENT_12_EFFECT 22
#define WMAP_LAND_EVENT_13_EFFECT 23
#define WMAP_LAND_EVENT_24_EFFECT 33

/** @brief CD files of the event sprites; each sprite is an animation (.DAT) and a texture (.TIM). */
typedef enum
{
    WMAP_NPCRUR_ANIMATION_RESOURCE = 0x10D8, /* WM/WMAP/NPCRUR.DAT */
    WMAP_NPCRUR_TEXTURE_RESOURCE = 0x10D9,   /* WM/WMAP/NPCRUR.TIM */
    WMAP_NPCDAN_ANIMATION_RESOURCE = 0x10DA, /* WM/WMAP/NPCDAN.DAT */
    WMAP_NPCDAN_TEXTURE_RESOURCE = 0x10DB,   /* WM/WMAP/NPCDAN.TIM */
    WMAP_NPCLDY_ANIMATION_RESOURCE = 0x10DC, /* WM/WMAP/NPCLDY.DAT */
    WMAP_NPCLDY_TEXTURE_RESOURCE = 0x10DD,   /* WM/WMAP/NPCLDY.TIM */
    WMAP_NPCSIN_ANIMATION_RESOURCE = 0x10DE, /* WM/WMAP/NPCSIN.DAT */
    WMAP_NPCSIN_TEXTURE_RESOURCE = 0x10DF,   /* WM/WMAP/NPCSIN.TIM */
    WMAP_NPCCPL_ANIMATION_RESOURCE = 0x10E0, /* WM/WMAP/NPCCPL.DAT */
    WMAP_NPCCPL_TEXTURE_RESOURCE = 0x10E1,   /* WM/WMAP/NPCCPL.TIM */
    WMAP_NPCLAR_ANIMATION_RESOURCE = 0x10E2, /* WM/WMAP/NPCLAR.DAT */
    WMAP_NPCLAR_TEXTURE_RESOURCE = 0x10E3,   /* WM/WMAP/NPCLAR.TIM */
    WMAP_NPCSHD_ANIMATION_RESOURCE = 0x1149, /* WM/WKANKUN/NPCSHD.DAT */
    WMAP_NPCSHD_TEXTURE_RESOURCE = 0x114A,   /* WM/WKANKUN/NPCSHD.TIM */
    WMAP_NPCPEL_ANIMATION_RESOURCE = 0x1216, /* WM/WEFF2/NPCPEL.DAT */
    WMAP_NPCPEL_TEXTURE_RESOURCE = 0x1217    /* WM/WEFF2/NPCPEL.TIM */
} WmapEventSpriteResource;

/** @brief A g_wmap_actor_motions slot as one of the arrival event's sprites or trail samples. */
typedef struct
{
    s16 active;
    /** @brief Direction the sprite comes from, 12-bit; side n is n quarter turns. */
    s16 angle;
    s32 radius_step;
    /** @brief Distance from the land, in 1/8 GTE units. */
    s32 radius;
    /** @brief Ordering-table depth of the last draw. */
    s16 depth;
    s16 height;
    WmapScreenPosition screen;
} WmapArrivalMotion;

/** @brief One g_wmap_actor_motions slot; slot 0 drives the travel vehicle. */
typedef union
{
    WmapArrivalMotion arrival;
    WmapVehicleMotion vehicle;
} WmapMotionSlot;

extern s32 g_wmap_special_travel_pending;
/** @brief Set when the arrival event comes from all four sides; it then also draws trails. */
extern s32 g_wmap_arrival_event_all_sides;
/** @brief Sides the arriving sprites come from: +x, +y, -x, -y. */
extern u8 g_wmap_arrival_event_sides[WMAP_ARRIVAL_SIDES];
/** @brief Animation of the scripted traveler (WMAP_SCRIPTED_TRAVELER), loaded by the walk events. */
extern u8 g_wmap_scripted_traveler_animation[];
extern s32 g_wmap_vehicle_phase;
extern WmapScreenPosition g_wmap_vehicle_screen_position;
extern WmapMotionSlot g_wmap_actor_motions[];
/** @brief Motion slots from actor WMAP_ARRIVAL_FIRST_ACTOR on, addressed as their own array. */
extern WmapMotionSlot g_wmap_effect_actor_motions[];

extern WmapSequenceStep g_wmap_arrival_event_steps[];
extern u32 g_wmap_arrival_event_step;
extern s32 g_wmap_arrival_event_timer;
extern WmapSequenceStep g_wmap_land_event_16_steps[];
extern u32 g_wmap_land_event_16_step;
extern s32 g_wmap_land_event_16_timer;
extern WmapSequenceStep g_wmap_land_event_21_steps[];
extern u32 g_wmap_land_event_21_step;
extern s32 g_wmap_land_event_21_timer;
extern WmapSequenceStep g_wmap_land_event_15_steps[];
extern u32 g_wmap_land_event_15_step;
extern s32 g_wmap_land_event_15_timer;
extern WmapSequenceStep g_wmap_land_event_13_steps[];
extern u32 g_wmap_land_event_13_step;
extern s32 g_wmap_land_event_13_timer;
extern WmapSequenceStep g_wmap_land_event_12_steps[];
extern u32 g_wmap_land_event_12_step;
extern s32 g_wmap_land_event_12_timer;
extern WmapSequenceStep g_wmap_land_event_24_steps[];
extern u32 g_wmap_land_event_24_step;
extern s32 g_wmap_land_event_24_timer;

/**
 * @brief Test whether a grid cell holds a land.
 * @param x Cell column; may be off the grid.
 * @param y Cell row; may be off the grid.
 * @return Nonzero when the cell is on the grid and holds a land.
 */
static inline s32 wmap_cell_has_land(s32 x, s32 y)
{
    if (x < 0 || y < 0 || x >= WMAP_GRID_SIZE || y >= WMAP_GRID_SIZE)
    {
        return 0;
    }

    return g_wmap_cells[x][y].land_id != WMAP_NO_LAND;
}

/** @brief Start a scripted scroll that brings g_wmap_vehicle_cell_x/y into view. */
static inline void wmap_scroll_to_vehicle_cell(void)
{
    g_wmap_view_scroll_mode = WMAP_SCROLL_SCRIPTED;
    g_wmap_scroll_remaining_x = (g_wmap_vehicle_cell_x - 1) * WMAP_CELL_SPACING - g_wmap_view.x;
    g_wmap_scroll_remaining_y = (g_wmap_vehicle_cell_y - 1) * WMAP_CELL_SPACING - g_wmap_view.y;
}

/** @brief Start a scripted scroll back to g_wmap_saved_view. */
static inline void wmap_scroll_to_saved_view(void)
{
    g_wmap_view_scroll_mode = WMAP_SCROLL_SCRIPTED;
    g_wmap_scroll_remaining_x = g_wmap_saved_view.x - g_wmap_view.x;
    g_wmap_scroll_remaining_y = g_wmap_saved_view.y - g_wmap_view.y;
}

/**
 * @brief Test whether the arrival event comes from all four sides.
 * @return Nonzero when every entry of g_wmap_arrival_event_sides is set.
 */
static inline s32 wmap_arrival_event_from_all_sides(void)
{
    return g_wmap_arrival_event_sides[3] & (g_wmap_arrival_event_sides[2] & (g_wmap_arrival_event_sides[0] & g_wmap_arrival_event_sides[1]));
}

/**
 * @brief Place the scripted traveler on a land and scroll to it (first step of a walk event).
 * @param land_id Land the traveler starts on.
 * @param animation_file CD file with the traveler's animation.
 * @param texture_file Sprite texture resource of the traveler.
 */
static inline void wmap_walk_event_start(s32 land_id, s32 animation_file, s32 texture_file)
{
    g_wmap_saved_view = g_wmap_view;
    g_wmap_event_active = 1;
    wmap_find_land_cell(land_id, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    cdrom_queue_read(animation_file, g_wmap_scripted_traveler_animation);
    wmap_load_tim_file(texture_file);
    wmap_set_traveler_position(WMAP_SCRIPTED_TRAVELER, g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y);
    wmap_scroll_to_vehicle_cell();
}

/**
 * @brief Route the scripted traveler to a land and start it walking.
 * @param land_id Destination land.
 */
static inline void wmap_walk_event_route(s32 land_id)
{
    wmap_find_land_cell(land_id, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    wmap_build_route(g_wmap_travelers[WMAP_SCRIPTED_TRAVELER].cell_x, g_wmap_travelers[WMAP_SCRIPTED_TRAVELER].cell_y, g_wmap_vehicle_cell_x,
                     g_wmap_vehicle_cell_y, g_wmap_travelers[WMAP_SCRIPTED_TRAVELER].path_x.cells, g_wmap_travelers[WMAP_SCRIPTED_TRAVELER].path_y.cells);
    g_wmap_travelers[WMAP_SCRIPTED_TRAVELER].destination_x = g_wmap_vehicle_cell_x;
    g_wmap_travelers[WMAP_SCRIPTED_TRAVELER].destination_y = g_wmap_vehicle_cell_y;
    g_wmap_travelers[WMAP_SCRIPTED_TRAVELER].moving = 1;
    g_wmap_travelers[WMAP_SCRIPTED_TRAVELER].path_index = 1;
    g_wmap_scripted_travel_active = 1;
    g_wmap_travelers[WMAP_SCRIPTED_TRAVELER].next_cell_x = g_wmap_travelers[WMAP_SCRIPTED_TRAVELER].path_x.steps[1].cell;
    g_wmap_travelers[WMAP_SCRIPTED_TRAVELER].target_x = (g_wmap_travelers[WMAP_SCRIPTED_TRAVELER].path_x.steps[1].cell - 1) * WMAP_CELL_SIZE;
    g_wmap_travelers[WMAP_SCRIPTED_TRAVELER].next_cell_y = g_wmap_travelers[WMAP_SCRIPTED_TRAVELER].path_y.steps[1].cell;
    g_wmap_travelers[WMAP_SCRIPTED_TRAVELER].target_y = (g_wmap_travelers[WMAP_SCRIPTED_TRAVELER].path_y.steps[1].cell - 1) * WMAP_CELL_SIZE;
}

/** @brief Show the scripted traveler's sprite and start fading it in. */
static inline void wmap_walk_event_show_traveler(void)
{
    WmapSpriteActor* actor = &g_wmap_sprite_actors[WMAP_SCRIPTED_TRAVELER];

    actor->scale_index = WMAP_STEP_ACTOR_SCALE_INDEX;
    actor->previous_sequence = -1;
    actor->target_shade = 128;
    actor->resource_index = 0;
    actor->sequence = 0;
    actor->shade = 0;
    actor->shade_step = 8;
}

/** @brief Move the arriving sprites one step in, draw them and their trails, and advance when the timer ends. */
static inline void wmap_arrival_event_approach(void)
{
    s32 i;

    wmap_set_effect_camera_transform();
    wmap_arrival_event_draw_sprites();
    wmap_arrival_event_draw_trails();

    for (i = 0; i < WMAP_ARRIVAL_SIDES; i++)
    {
        g_wmap_actor_motions[WMAP_ARRIVAL_FIRST_ACTOR + i].arrival.radius -= g_wmap_actor_motions[WMAP_ARRIVAL_FIRST_ACTOR + i].arrival.radius_step;
    }

    if (--g_wmap_arrival_event_timer == 0)
    {
        g_wmap_arrival_event_step++;
    }
}

/** @brief Start the first queued map event: a pending land event, special travel, or arrival event. */
void wmap_start_pending_event(void)
{
    s32 event_index;
    u8* event;

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
        else if (g_wmap_special_travel_pending != 0)
        {
            g_wmap_special_travel_pending = 0;
            wmap_start_sequence(&wmap_run_special_travel);
        }
        else if (g_wmap_land_event_24_pending != 0)
        {
            cdrom_queue_read(WMAP_VEHICLE_BANK_FILE, g_wmap_animation_bank_0);
            cdrom_queue_read(WMAP_VEHICLE_BANK_FILE_2, g_wmap_animation_bank_0 + WMAP_VEHICLE_BANK_SIZE);
            cdrom_wait_queue_empty();
            g_wmap_land_event_24_pending = 0;
            wmap_start_sequence(&wmap_land_event_24_run);
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
            wmap_start_sequence(&wmap_land_event_16_run);
        }
        else if (g_wmap_land_event_21_pending != 0)
        {
            g_wmap_land_event_21_pending = 0;
            wmap_start_sequence(&wmap_land_event_21_run);
        }
        else if (g_wmap_land_event_15_pending != 0)
        {
            g_wmap_land_event_15_pending = 0;
            wmap_start_sequence(&wmap_land_event_15_run);
        }
        else if (g_wmap_land_event_13_pending != 0)
        {
            g_wmap_land_event_13_pending = 0;
            wmap_start_sequence(&wmap_land_event_13_run);
        }
        else if (g_wmap_land_event_12_pending != 0)
        {
            g_wmap_land_event_12_pending = 0;
            wmap_start_sequence(&wmap_land_event_12_run);
        }
        else
        {
            for (event_index = 0; event_index < 8; event_index++)
            {
                event = &g_wmap_pending_arrival_events[event_index];

                if (*event != 0)
                {
                    *event = 0;
                    switch (event_index)
                    {
                        case 0:
                            cdrom_queue_read(WMAP_NPCSHD_ANIMATION_RESOURCE, g_wmap_animation_bank_3);
                            cdrom_wait_queue_empty();
                            wmap_load_tim_file(WMAP_NPCSHD_TEXTURE_RESOURCE);
                            wmap_find_land_cell(WMAP_ARRIVAL_LAND, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
                            g_wmap_arrival_event_all_sides = 1;
                            g_wmap_arrival_event_sides[3] = 1;
                            g_wmap_arrival_event_sides[2] = 1;
                            g_wmap_arrival_event_sides[1] = 1;
                            g_wmap_arrival_event_sides[0] = 1;
                            break;
                        case 1:
                            cdrom_queue_read(WMAP_NPCSIN_ANIMATION_RESOURCE, g_wmap_animation_bank_3);
                            cdrom_wait_queue_empty();
                            wmap_load_tim_file(WMAP_NPCSIN_TEXTURE_RESOURCE);
                            wmap_arrival_event_pick_sides(11);
                            break;
                        case 2:
                            cdrom_queue_read(WMAP_NPCRUR_ANIMATION_RESOURCE, g_wmap_animation_bank_3);
                            cdrom_wait_queue_empty();
                            wmap_load_tim_file(WMAP_NPCRUR_TEXTURE_RESOURCE);
                            wmap_arrival_event_pick_sides(17);
                            break;
                        case 3:
                            cdrom_queue_read(WMAP_NPCLDY_ANIMATION_RESOURCE, g_wmap_animation_bank_3);
                            cdrom_wait_queue_empty();
                            wmap_load_tim_file(WMAP_NPCLDY_TEXTURE_RESOURCE);
                            wmap_arrival_event_pick_sides(11);
                            break;
                        case 4:
                            cdrom_queue_read(WMAP_NPCDAN_ANIMATION_RESOURCE, g_wmap_animation_bank_3);
                            cdrom_wait_queue_empty();
                            wmap_load_tim_file(WMAP_NPCDAN_TEXTURE_RESOURCE);
                            wmap_arrival_event_pick_sides(10);
                            break;
                    }

                    wmap_start_sequence(wmap_arrival_event_run);
                    break;
                }
            }
        }

        g_wmap_pending_event_count--;
    }
}

/**
 * @brief Pick the side the arriving sprite comes from: the first neighbor with a land, else a random side.
 * @param land_id Land whose neighbors are checked.
 */
void wmap_arrival_event_pick_sides(s32 land_id)
{
    wmap_find_land_cell(land_id, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_arrival_event_all_sides = 0;
    g_wmap_arrival_event_sides[3] = 0;
    g_wmap_arrival_event_sides[2] = 0;
    g_wmap_arrival_event_sides[1] = 0;
    g_wmap_arrival_event_sides[0] = 0;

    if (wmap_cell_has_land(g_wmap_vehicle_cell_x + 1, g_wmap_vehicle_cell_y))
    {
        g_wmap_arrival_event_sides[0] = 1;
        return;
    }

    if (wmap_cell_has_land(g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y + 1))
    {
        g_wmap_arrival_event_sides[1] = 1;
        return;
    }

    if (wmap_cell_has_land(g_wmap_vehicle_cell_x - 1, g_wmap_vehicle_cell_y))
    {
        g_wmap_arrival_event_sides[2] = 1;
        return;
    }

    if (wmap_cell_has_land(g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y - 1))
    {
        g_wmap_arrival_event_sides[3] = 1;
        return;
    }

    g_wmap_arrival_event_sides[rand() & 3] = 1;
}

/** @brief Arrival event: lock input, save the view and scroll to WMAP_ARRIVAL_LAND. */
void wmap_arrival_event_step_01(void)
{
    g_wmap_special_travel_pending = 0;
    D_80139234 = 0;
    g_wmap_input_locked = 1;
    g_wmap_event_active = 1;
    g_wmap_saved_view = g_wmap_view;
    wmap_find_land_cell(WMAP_ARRIVAL_LAND, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    wmap_scroll_to_vehicle_cell();
    g_wmap_arrival_event_step++;
    wmap_arrival_event_step_02();
}

/** @brief Arrival event: start one sprite per chosen side, fade the song down and play the arrival sound. */
void wmap_arrival_event_step_03(void)
{
    s32 i;
    WmapSpriteActor* actor;

    for (i = WMAP_ARRIVAL_TRAIL_FIRST_ACTOR; i < 120; i++)
    {
        g_wmap_actor_motions[i].arrival.active = 0;
        g_wmap_actor_animations[i].data = g_wmap_animation_bank_3;
    }

    for (i = 0; i < WMAP_ARRIVAL_SIDES; i++)
    {
        actor = &g_wmap_sprite_actors[WMAP_ARRIVAL_FIRST_ACTOR + i];

        if (g_wmap_arrival_event_sides[i] != 0)
        {
            g_wmap_actor_animations[i + WMAP_ARRIVAL_FIRST_ACTOR].data = g_wmap_animation_bank_3;
            actor->scale_index = WMAP_STEP_ACTOR_SCALE_INDEX;
            actor->sequence = i + 1;
            actor->previous_sequence = -1;
            actor->target_shade = 128;
            actor->shade_step = 2;
            actor->resource_index = 0;
            actor->shade = 0;
            g_wmap_effect_actor_motions[i].arrival.active = 1;
            g_wmap_effect_actor_motions[i].arrival.angle = i << 10;
            g_wmap_effect_actor_motions[i].arrival.radius = WMAP_ARRIVAL_START_RADIUS;
            g_wmap_effect_actor_motions[i].arrival.radius_step = WMAP_ARRIVAL_RADIUS_STEP;
            g_wmap_effect_actor_motions[i].arrival.height = 0;
        }
    }

    akao_fade_song_volume_from(0, 30, 127, 48);

    if (wmap_arrival_event_from_all_sides())
    {
        wmap_play_sound(49, 128);
    }
    else
    {
        wmap_play_sound(21, 128);
    }

    g_wmap_arrival_event_timer = 64;
    g_wmap_arrival_event_step++;
    wmap_arrival_event_step_04();
}

/** @brief Project and draw the arriving sprites around the event land. */
void wmap_arrival_event_draw_sprites(void)
{
    SVECTOR position;
    s32 depth;
    s32 draw_depth;
    s32 i;
    WmapSpriteActor* actor;
    WmapArrivalMotion* motion;

    for (i = 0; i < WMAP_ARRIVAL_SIDES; i++)
    {
        motion = &g_wmap_effect_actor_motions[i].arrival;
        actor = &g_wmap_sprite_actors[WMAP_ARRIVAL_FIRST_ACTOR + i];
        position.vx = ((motion->radius >> 3) * (ccos(motion->angle) >> 6)) >> 12;
        position.vy = ((motion->radius >> 3) * (csin(motion->angle) >> 6)) >> 12;
        position.vz = motion->height;
        gte_ldv0(&position);
        gte_rtps();
        wmap_step_actor_animation(actor, &g_wmap_actor_animations[WMAP_ARRIVAL_FIRST_ACTOR + i]);
        gte_stsxy(&motion->screen);
        gte_stszotz(&depth);
        draw_depth = (7057 - depth) / 4 + 42;
        wmap_draw_actor_sprite(actor, motion->screen.packed, WMAP_ARRIVAL_TEXTURE, draw_depth, WMAP_ARRIVAL_VARIANT);
        motion->depth = draw_depth;
    }
}

/** @brief Draw the trail sprites and, every 10 frames, copy the arriving sprites into the next trail sample. */
void wmap_arrival_event_draw_trails(void)
{
    s32 i;
    s32 destination;
    WmapSpriteActor* actor;
    WmapArrivalMotion* motion;

    if (g_wmap_arrival_event_all_sides != 0)
    {
        for (i = WMAP_ARRIVAL_TRAIL_FIRST_ACTOR; i < WMAP_ARRIVAL_TRAIL_END; i++)
        {
            motion = &g_wmap_actor_motions[i].arrival;

            if (motion->active != 0)
            {
                actor = &g_wmap_sprite_actors[i];
                wmap_step_actor_animation(actor, &g_wmap_actor_animations[i]);
                wmap_draw_actor_sprite(actor, motion->screen.packed, WMAP_ARRIVAL_TEXTURE, motion->depth + 1, WMAP_ARRIVAL_VARIANT);
            }
        }

        if (g_wmap_frame_count % 10 == 0)
        {
            if (D_80139234 != -1)
            {
                for (i = 0; i < WMAP_ARRIVAL_SIDES; i++)
                {
                    destination = i + D_80139234 * WMAP_ARRIVAL_SIDES + WMAP_ARRIVAL_TRAIL_FIRST_ACTOR;
                    g_wmap_actor_motions[destination] = g_wmap_actor_motions[i + WMAP_ARRIVAL_FIRST_ACTOR];
                    g_wmap_sprite_actors[destination] = g_wmap_sprite_actors[i + WMAP_ARRIVAL_FIRST_ACTOR];
                    g_wmap_sprite_actors[destination].target_shade = 0;
                    g_wmap_sprite_actors[destination].shade_step = 2;
                }

                D_80139234 = (D_80139234 + 1) & (WMAP_ARRIVAL_TRAIL_SAMPLES - 1);
            }
        }
    }
}

/** @brief Land event 16: put the scripted traveler on land 12 and scroll to it. */
void wmap_land_event_16_step_01(void)
{
    wmap_walk_event_start(12, WMAP_NPCLAR_ANIMATION_RESOURCE, WMAP_NPCLAR_TEXTURE_RESOURCE);
    g_wmap_land_event_16_step++;
    wmap_land_event_16_step_02();
}

/** @brief Land event 16: walk the scripted traveler to land 16. */
void wmap_land_event_16_step_05(void)
{
    wmap_walk_event_route(16);
    g_wmap_land_event_16_step++;
    wmap_land_event_16_step_06();
}

/** @brief Land event 21: put the scripted traveler on land 1 and scroll to it. */
void wmap_land_event_21_step_01(void)
{
    wmap_walk_event_start(1, WMAP_NPCCPL_ANIMATION_RESOURCE, WMAP_NPCCPL_TEXTURE_RESOURCE);
    g_wmap_land_event_21_step++;
    wmap_land_event_21_step_02();
}

/** @brief Land event 21: walk the scripted traveler to land 2. */
void wmap_land_event_21_step_05(void)
{
    wmap_walk_event_route(2);
    g_wmap_land_event_21_step++;
    wmap_land_event_21_step_06();
}

/** @brief Land event 15: put the scripted traveler on land 1 and scroll to it. */
void wmap_land_event_15_step_01(void)
{
    wmap_walk_event_start(1, WMAP_NPCPEL_ANIMATION_RESOURCE, WMAP_NPCPEL_TEXTURE_RESOURCE);
    g_wmap_land_event_15_step++;
    wmap_land_event_15_step_02();
}

/** @brief Land event 15: walk the scripted traveler to land 18 and play its departure sound. */
void wmap_land_event_15_step_05(void)
{
    wmap_walk_event_route(18);
    wmap_play_sound(50, 128);
    g_wmap_land_event_15_step++;
    wmap_land_event_15_step_06();
}

/** @brief Land event 13: start the mesh transition and scroll to land 16. */
void wmap_land_event_13_step_01(void)
{
    wmap_select_mesh_motion(1);
    wmap_start_mesh_transition(1);
    wmap_find_land_cell(16, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_focus_cell_x = g_wmap_vehicle_cell_x;
    g_wmap_focus_cell_y = g_wmap_vehicle_cell_y;
    wmap_scroll_to_vehicle_cell();
    g_wmap_land_event_13_step++;
    wmap_land_event_13_step_02();
}

/** @brief Land event 12: start the mesh transition and scroll to land 23. */
void wmap_land_event_12_step_01(void)
{
    wmap_select_mesh_motion(1);
    wmap_start_mesh_transition(1);
    wmap_find_land_cell(23, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_focus_cell_x = g_wmap_vehicle_cell_x;
    g_wmap_focus_cell_y = g_wmap_vehicle_cell_y;
    wmap_scroll_to_vehicle_cell();
    g_wmap_land_event_12_step++;
    wmap_land_event_12_step_02();
}

/** @brief Land event 24: animate the special travel land, scroll to it and queue its effect's resources. */
void wmap_land_event_24_step_01(void)
{
    g_wmap_vehicle_flap_sound_enabled = 0;
    g_wmap_forced_animated_land_id = WMAP_SPECIAL_TRAVEL_LAND;
    g_wmap_land_display_limit = 0;
    wmap_find_land_cell(WMAP_SPECIAL_TRAVEL_LAND, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_focus_cell_x = g_wmap_vehicle_cell_x;
    g_wmap_focus_cell_y = g_wmap_vehicle_cell_y;
    wmap_scroll_to_vehicle_cell();
    wmap_queue_effect_resources(WMAP_LAND_EVENT_24_EFFECT);
    g_wmap_land_event_24_step++;
    wmap_land_event_24_step_02();
}

/** @brief Land event 24: turn the vehicle while it climbs, moves out and grows, until the timer ends. */
void wmap_land_event_24_step_04(void)
{
    WmapSpriteActor* vehicle;

    vehicle = wmap_turn_vehicle(1);

    if (g_wmap_actor_motions[0].vehicle.height < WMAP_VEHICLE_MAX_HEIGHT)
    {
        g_wmap_actor_motions[0].vehicle.height++;
    }

    if (g_wmap_actor_motions[0].vehicle.radius < 1000)
    {
        g_wmap_actor_motions[0].vehicle.radius += 30;
    }

    if ((g_wmap_frame_count & 3) == 0)
    {
        if (vehicle->scale_index < WMAP_VEHICLE_SCALE_MAX)
        {
            vehicle->scale_index++;
        }
    }

    if (--g_wmap_land_event_24_timer == 0)
    {
        g_wmap_land_event_24_step++;
    }
}

/**
 * @brief Run land event 24: the travel vehicle rises from Lucemia (land 24) and flies off.
 * @param arg0 Nonzero restarts the sequence at step 1.
 * @return 1 while the sequence runs or after a restart, 0 once it has finished.
 */
WMAP_STEP_RUNNER(wmap_land_event_24_run, g_wmap_land_event_24_steps, 0xA, g_wmap_land_event_24_step, g_wmap_land_event_24_timer)

/** @brief Land event 24: restart at step 1 with a one-frame timer. */
WMAP_STEP_RESET(wmap_land_event_24_reset, g_wmap_land_event_24_step, g_wmap_land_event_24_timer)

/** @brief Land event 24: wait for the scroll, then show the vehicle. */
WMAP_STEP_WAIT_SCROLL(wmap_land_event_24_step_02, g_wmap_land_event_24_step, wmap_land_event_24_step_03)

/** @brief Land event 24: show the vehicle above the land, install its draw callback and start the land effect. */
void wmap_land_event_24_step_03(void)
{
    g_wmap_vehicle_animation.data = g_wmap_animation_bank_0;
    g_wmap_vehicle_actor.previous_sequence = -1;
    g_wmap_vehicle_actor.resource_index = 0;
    g_wmap_vehicle_actor.scale_index = 0;
    g_wmap_vehicle_actor.sequence = 0;
    g_wmap_vehicle_actor.shade_step = 0;
    g_wmap_vehicle_actor.target_shade = WMAP_VEHICLE_SHADE_NEUTRAL;
    g_wmap_vehicle_actor.shade = WMAP_VEHICLE_SHADE_NEUTRAL;
    g_wmap_actor_motions[0].vehicle.active = 1;
    g_wmap_actor_motions[0].vehicle.radius = WMAP_VEHICLE_START_RADIUS;
    g_wmap_actor_motions[0].vehicle.height = WMAP_VEHICLE_START_HEIGHT;
    g_wmap_actor_motions[0].vehicle.ot_index = WMAP_VEHICLE_OT_START;
    g_wmap_actor_motions[0].vehicle.heading = 0;
    g_wmap_actor_motions[0].vehicle.ot_override = 2;
    wmap_install_callback(&wmap_draw_vehicle);
    wmap_start_land_effect(WMAP_LAND_EVENT_24_EFFECT);
    g_wmap_land_event_24_timer = 366;
    g_wmap_land_event_24_step++;
    wmap_land_event_24_step_04();
}

/** @brief Land event 24: start the vehicle's final turn. */
void wmap_land_event_24_step_05(void)
{
    g_wmap_vehicle_phase = 1;
    wmap_start_sequence(wmap_finish_vehicle_turn);
    g_wmap_land_event_24_step++;
    wmap_land_event_24_step_06();
}

/** @brief Land event 24: wait for the turn to finish. */
void wmap_land_event_24_step_06(void)
{
    if (g_wmap_vehicle_phase == 0)
    {
        g_wmap_land_event_24_step++;
        wmap_land_event_24_step_07();
    }
}

/** @brief Land event 24: release the vehicle's motion slot and start the 120-frame fly-off. */
void wmap_land_event_24_step_07(void)
{
    g_wmap_actor_motions[0].vehicle.active = 0;
    g_wmap_land_event_24_timer = 120;
    g_wmap_land_event_24_step++;
    wmap_land_event_24_step_08();
}

/** @brief Land event 24: draw the vehicle 4 pixels further left each frame until the timer ends. */
void wmap_land_event_24_step_08(void)
{
    WmapSpriteActor* vehicle = &g_wmap_vehicle_actor;

    wmap_turn_vehicle(0);
    wmap_step_actor_animation(vehicle, &g_wmap_vehicle_animation);
    wmap_draw_actor_sprite(vehicle, g_wmap_vehicle_screen_position.packed, WMAP_VEHICLE_TEXTURE, 2, WMAP_VEHICLE_VARIANT);
    g_wmap_vehicle_screen_position.point.x -= 4;

    if (--g_wmap_land_event_24_timer == 0)
    {
        g_wmap_land_event_24_step++;
    }
}

/** @brief Land event 24: finish. */
WMAP_STEP_ADVANCE(wmap_land_event_24_step_09, g_wmap_land_event_24_step)

/**
 * @brief Run the arrival event: a sprite comes in to a land from a neighboring land (NPCSHD, event 25: four of them, to Lumina).
 * @param arg0 Nonzero restarts the sequence at step 1.
 * @return 1 while the sequence runs or after a restart, 0 once it has finished.
 */
WMAP_STEP_RUNNER(wmap_arrival_event_run, g_wmap_arrival_event_steps, 0x10, g_wmap_arrival_event_step, g_wmap_arrival_event_timer)

/** @brief Arrival event: restart at step 1 with a one-frame timer. */
WMAP_STEP_RESET(wmap_arrival_event_reset, g_wmap_arrival_event_step, g_wmap_arrival_event_timer)

/** @brief Arrival event: wait for the scroll, then start the sprites. */
WMAP_STEP_WAIT_SCROLL(wmap_arrival_event_step_02, g_wmap_arrival_event_step, wmap_arrival_event_step_03)

/** @brief Arrival event: draw the sprites until the timer ends. */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_arrival_event_step_04, g_wmap_arrival_event_step, g_wmap_arrival_event_timer, wmap_set_effect_camera_transform(),
                              wmap_arrival_event_draw_sprites())

/** @brief Arrival event: stop updating land labels and start a 30-frame approach. */
void wmap_arrival_event_step_05(void)
{
    g_wmap_land_label_updates_enabled = 0;
    g_wmap_arrival_event_timer = 30;
    g_wmap_arrival_event_step++;
    wmap_arrival_event_step_06();
}

/** @brief Arrival event: move the sprites in and draw them and their trails until the timer ends. */
void wmap_arrival_event_step_06(void)
{
    wmap_arrival_event_approach();
}

/** @brief Arrival event: start the second approach, shorter when the sprites come from all sides. */
void wmap_arrival_event_step_07(void)
{
    s32 delay;

    wmap_set_map_label(-1);
    delay = 70;

    if (g_wmap_arrival_event_all_sides != 0)
    {
        delay = 50;
    }

    g_wmap_arrival_event_timer = delay;
    g_wmap_arrival_event_step++;
    wmap_arrival_event_step_08();
}

/** @brief Arrival event: move the sprites in and draw them and their trails until the timer ends. */
void wmap_arrival_event_step_08(void)
{
    wmap_arrival_event_approach();
}

/** @brief Arrival event: stop the sounds unless all sides came, stop the sprite animations and the trail sampling. */
void wmap_arrival_event_step_09(void)
{
    s32 i;

    if (!wmap_arrival_event_from_all_sides())
    {
        akao_release_all_sfx();
    }

    for (i = 0; i < WMAP_ARRIVAL_SIDES; i++)
    {
        g_wmap_sprite_actors[i + WMAP_ARRIVAL_FIRST_ACTOR].sequence = 0;
    }

    D_80139234 = -1;
    g_wmap_arrival_event_timer = 40;
    g_wmap_arrival_event_step++;
    wmap_arrival_event_step_10();
}

/** @brief Arrival event: draw the sprites and trails until the timer ends. */
void wmap_arrival_event_step_10(void)
{
    wmap_set_effect_camera_transform();
    wmap_arrival_event_draw_sprites();
    wmap_arrival_event_draw_trails();

    if (--g_wmap_arrival_event_timer == 0)
    {
        g_wmap_arrival_event_step++;
    }
}

/** @brief Arrival event: fade the sprites out over 64 frames. */
void wmap_arrival_event_step_11(void)
{
    s32 i;

    for (i = 0; i < WMAP_ARRIVAL_SIDES; i++)
    {
        g_wmap_sprite_actors[i + WMAP_ARRIVAL_FIRST_ACTOR].target_shade = 0;
    }

    g_wmap_arrival_event_timer = 64;
    g_wmap_arrival_event_step++;
    wmap_arrival_event_step_12();
}

/** @brief Arrival event: draw the fading sprites until the timer ends. */
WMAP_STEP_UPDATE_TWO_AND_WAIT(wmap_arrival_event_step_12, g_wmap_arrival_event_step, g_wmap_arrival_event_timer, wmap_set_effect_camera_transform(),
                              wmap_arrival_event_draw_sprites())

/** @brief Arrival event: fade the song back up and scroll back to the saved view. */
void wmap_arrival_event_step_13(void)
{
    akao_fade_song_volume_from(0, 30, 48, 127);
    wmap_scroll_to_saved_view();
    g_wmap_arrival_event_step++;
    wmap_arrival_event_step_14();
}

/** @brief Arrival event: wait for the scroll back. */
WMAP_STEP_WAIT_SCROLL(wmap_arrival_event_step_14, g_wmap_arrival_event_step, wmap_arrival_event_step_15)

/** @brief Arrival event: unlock input, end the event and resume land label updates. */
void wmap_arrival_event_step_15(void)
{
    g_wmap_input_locked = 0;
    g_wmap_event_active = 0;
    g_wmap_land_label_updates_enabled = 1;
    g_wmap_arrival_event_step++;
}

/**
 * @brief Run land event 16: the NPCLAR sprite walks from White Forest (land 12) to the Underworld (land 16).
 * @param arg0 Nonzero restarts the sequence at step 1.
 * @return 1 while the sequence runs or after a restart, 0 once it has finished.
 */
WMAP_STEP_RUNNER(wmap_land_event_16_run, g_wmap_land_event_16_steps, 0x10, g_wmap_land_event_16_step, g_wmap_land_event_16_timer)

/** @brief Land event 16: restart at step 1 with a one-frame timer. */
WMAP_STEP_RESET(wmap_land_event_16_reset, g_wmap_land_event_16_step, g_wmap_land_event_16_timer)

/** @brief Land event 16: wait for the scroll, then show the traveler. */
WMAP_STEP_WAIT_SCROLL(wmap_land_event_16_step_02, g_wmap_land_event_16_step, wmap_land_event_16_step_03)

/** @brief Land event 16: fade the traveler in over 16 frames. */
void wmap_land_event_16_step_03(void)
{
    wmap_walk_event_show_traveler();
    g_wmap_land_event_16_timer = 16;
    g_wmap_land_event_16_step++;
}

/** @brief Land event 16: wait for the fade-in. */
WMAP_STEP_WAIT(wmap_land_event_16_step_04, g_wmap_land_event_16_step, g_wmap_land_event_16_timer)

/** @brief Land event 16: wait for the walk to end. */
void wmap_land_event_16_step_06(void)
{
    if (g_wmap_scripted_travel_active == 0)
    {
        g_wmap_land_event_16_step++;
        wmap_land_event_16_step_07();
    }
}

/** @brief Land event 16: scroll to the destination land. */
void wmap_land_event_16_step_07(void)
{
    wmap_scroll_to_vehicle_cell();
    g_wmap_land_event_16_step++;
    wmap_land_event_16_step_08();
}

/** @brief Land event 16: wait for the scroll. */
WMAP_STEP_WAIT_SCROLL(wmap_land_event_16_step_08, g_wmap_land_event_16_step, wmap_land_event_16_step_09)

/** @brief Land event 16: pause for 60 frames. */
void wmap_land_event_16_step_09(void)
{
    g_wmap_land_event_16_timer = 60;
    g_wmap_land_event_16_step++;
}

/** @brief Land event 16: wait for the pause. */
WMAP_STEP_WAIT(wmap_land_event_16_step_10, g_wmap_land_event_16_step, g_wmap_land_event_16_timer)

/** @brief Land event 16: fade the traveler out over 30 frames. */
void wmap_land_event_16_step_11(void)
{
    g_wmap_sprite_actors[WMAP_SCRIPTED_TRAVELER].target_shade = 0;
    g_wmap_land_event_16_timer = 30;
    g_wmap_land_event_16_step++;
}

/** @brief Land event 16: wait for the fade-out. */
WMAP_STEP_WAIT(wmap_land_event_16_step_12, g_wmap_land_event_16_step, g_wmap_land_event_16_timer)

/** @brief Land event 16: scroll back to the saved view. */
void wmap_land_event_16_step_13(void)
{
    wmap_scroll_to_saved_view();
    g_wmap_land_event_16_step++;
    wmap_land_event_16_step_14();
}

/** @brief Land event 16: wait for the scroll back. */
WMAP_STEP_WAIT_SCROLL(wmap_land_event_16_step_14, g_wmap_land_event_16_step, wmap_land_event_16_step_15)

/** @brief Land event 16: unlock input and end the event. */
void wmap_land_event_16_step_15(void)
{
    g_wmap_input_locked = 0;
    g_wmap_event_active = 0;
    g_wmap_land_event_16_step++;
}

/**
 * @brief Run land event 21: the NPCCPL sprite walks from Domina (land 1) to Gato Grottoes (land 2).
 * @param arg0 Nonzero restarts the sequence at step 1.
 * @return 1 while the sequence runs or after a restart, 0 once it has finished.
 */
WMAP_STEP_RUNNER(wmap_land_event_21_run, g_wmap_land_event_21_steps, 0x10, g_wmap_land_event_21_step, g_wmap_land_event_21_timer)

/** @brief Land event 21: restart at step 1 with a one-frame timer. */
WMAP_STEP_RESET(wmap_land_event_21_reset, g_wmap_land_event_21_step, g_wmap_land_event_21_timer)

/** @brief Land event 21: wait for the scroll, then show the traveler. */
WMAP_STEP_WAIT_SCROLL(wmap_land_event_21_step_02, g_wmap_land_event_21_step, wmap_land_event_21_step_03)

/** @brief Land event 21: fade the traveler in over 16 frames. */
void wmap_land_event_21_step_03(void)
{
    wmap_walk_event_show_traveler();
    g_wmap_land_event_21_timer = 16;
    g_wmap_land_event_21_step++;
}

/** @brief Land event 21: wait for the fade-in. */
WMAP_STEP_WAIT(wmap_land_event_21_step_04, g_wmap_land_event_21_step, g_wmap_land_event_21_timer)

/** @brief Land event 21: wait for the walk to end. */
void wmap_land_event_21_step_06(void)
{
    if (g_wmap_scripted_travel_active == 0)
    {
        g_wmap_land_event_21_step++;
        wmap_land_event_21_step_07();
    }
}

/** @brief Land event 21: scroll to the destination land. */
void wmap_land_event_21_step_07(void)
{
    wmap_scroll_to_vehicle_cell();
    g_wmap_land_event_21_step++;
    wmap_land_event_21_step_08();
}

/** @brief Land event 21: wait for the scroll. */
WMAP_STEP_WAIT_SCROLL(wmap_land_event_21_step_08, g_wmap_land_event_21_step, wmap_land_event_21_step_09)

/** @brief Land event 21: pause for 60 frames. */
void wmap_land_event_21_step_09(void)
{
    g_wmap_land_event_21_timer = 60;
    g_wmap_land_event_21_step++;
}

/** @brief Land event 21: wait for the pause. */
WMAP_STEP_WAIT(wmap_land_event_21_step_10, g_wmap_land_event_21_step, g_wmap_land_event_21_timer)

/** @brief Land event 21: fade the traveler out over 30 frames. */
void wmap_land_event_21_step_11(void)
{
    g_wmap_sprite_actors[WMAP_SCRIPTED_TRAVELER].target_shade = 0;
    g_wmap_land_event_21_timer = 30;
    g_wmap_land_event_21_step++;
}

/** @brief Land event 21: wait for the fade-out. */
WMAP_STEP_WAIT(wmap_land_event_21_step_12, g_wmap_land_event_21_step, g_wmap_land_event_21_timer)

/** @brief Land event 21: scroll back to the saved view. */
void wmap_land_event_21_step_13(void)
{
    wmap_scroll_to_saved_view();
    g_wmap_land_event_21_step++;
    wmap_land_event_21_step_14();
}

/** @brief Land event 21: wait for the scroll back. */
WMAP_STEP_WAIT_SCROLL(wmap_land_event_21_step_14, g_wmap_land_event_21_step, wmap_land_event_21_step_15)

/** @brief Land event 21: unlock input and end the event. */
void wmap_land_event_21_step_15(void)
{
    g_wmap_input_locked = 0;
    g_wmap_event_active = 0;
    g_wmap_land_event_21_step++;
}

/**
 * @brief Run land event 15: the NPCPEL sprite walks from Domina (land 1) to Luon Highway (land 18).
 * @param arg0 Nonzero restarts the sequence at step 1.
 * @return 1 while the sequence runs or after a restart, 0 once it has finished.
 */
WMAP_STEP_RUNNER(wmap_land_event_15_run, g_wmap_land_event_15_steps, 0x10, g_wmap_land_event_15_step, g_wmap_land_event_15_timer)

/** @brief Land event 15: restart at step 1 with a one-frame timer. */
WMAP_STEP_RESET(wmap_land_event_15_reset, g_wmap_land_event_15_step, g_wmap_land_event_15_timer)

/** @brief Land event 15: wait for the scroll, then show the traveler. */
WMAP_STEP_WAIT_SCROLL(wmap_land_event_15_step_02, g_wmap_land_event_15_step, wmap_land_event_15_step_03)

/** @brief Land event 15: fade the traveler in over 16 frames. */
void wmap_land_event_15_step_03(void)
{
    wmap_walk_event_show_traveler();
    g_wmap_land_event_15_timer = 16;
    g_wmap_land_event_15_step++;
}

/** @brief Land event 15: wait for the fade-in. */
WMAP_STEP_WAIT(wmap_land_event_15_step_04, g_wmap_land_event_15_step, g_wmap_land_event_15_timer)

/** @brief Land event 15: wait for the walk to end. */
void wmap_land_event_15_step_06(void)
{
    if (g_wmap_scripted_travel_active == 0)
    {
        g_wmap_land_event_15_step++;
        wmap_land_event_15_step_07();
    }
}

/** @brief Land event 15: scroll to the destination land. */
void wmap_land_event_15_step_07(void)
{
    wmap_scroll_to_vehicle_cell();
    g_wmap_land_event_15_step++;
    wmap_land_event_15_step_08();
}

/** @brief Land event 15: wait for the scroll. */
WMAP_STEP_WAIT_SCROLL(wmap_land_event_15_step_08, g_wmap_land_event_15_step, wmap_land_event_15_step_09)

/** @brief Land event 15: stop the departure sound and pause for 60 frames. */
void wmap_land_event_15_step_09(void)
{
    akao_release_all_sfx();
    g_wmap_land_event_15_timer = 60;
    g_wmap_land_event_15_step++;
}

/** @brief Land event 15: wait for the pause. */
WMAP_STEP_WAIT(wmap_land_event_15_step_10, g_wmap_land_event_15_step, g_wmap_land_event_15_timer)

/** @brief Land event 15: play the arrival sound and fade the traveler out over 30 frames. */
void wmap_land_event_15_step_11(void)
{
    wmap_play_sound(57, 128);
    g_wmap_sprite_actors[WMAP_SCRIPTED_TRAVELER].target_shade = 0;
    g_wmap_land_event_15_timer = 30;
    g_wmap_land_event_15_step++;
}

/** @brief Land event 15: wait for the fade-out. */
WMAP_STEP_WAIT(wmap_land_event_15_step_12, g_wmap_land_event_15_step, g_wmap_land_event_15_timer)

/** @brief Land event 15: scroll back to the saved view. */
void wmap_land_event_15_step_13(void)
{
    wmap_scroll_to_saved_view();
    g_wmap_land_event_15_step++;
    wmap_land_event_15_step_14();
}

/** @brief Land event 15: wait for the scroll back. */
WMAP_STEP_WAIT_SCROLL(wmap_land_event_15_step_14, g_wmap_land_event_15_step, wmap_land_event_15_step_15)

/** @brief Land event 15: unlock input and end the event. */
void wmap_land_event_15_step_15(void)
{
    g_wmap_input_locked = 0;
    g_wmap_event_active = 0;
    g_wmap_land_event_15_step++;
}

/**
 * @brief Run land event 13: scroll to the Underworld (land 16) and start land effect 23.
 * @param arg0 Nonzero restarts the sequence at step 1.
 * @return 1 while the sequence runs or after a restart, 0 once it has finished.
 */
WMAP_STEP_RUNNER(wmap_land_event_13_run, g_wmap_land_event_13_steps, 0x4, g_wmap_land_event_13_step, g_wmap_land_event_13_timer)

/** @brief Land event 13: restart at step 1 with a one-frame timer. */
WMAP_STEP_RESET(wmap_land_event_13_reset, g_wmap_land_event_13_step, g_wmap_land_event_13_timer)

/** @brief Land event 13: wait for the scroll, then start the land effect. */
WMAP_STEP_WAIT_SCROLL(wmap_land_event_13_step_02, g_wmap_land_event_13_step, wmap_land_event_13_step_03)

/** @brief Land event 13: queue and start the land effect, ending the event. */
void wmap_land_event_13_step_03(void)
{
    wmap_queue_effect_resources(WMAP_LAND_EVENT_13_EFFECT);
    wmap_start_land_effect(WMAP_LAND_EVENT_13_EFFECT);
    g_wmap_land_event_13_step++;
}

/**
 * @brief Run land event 12: scroll to The Flames (land 23) and start land effect 22.
 * @param arg0 Nonzero restarts the sequence at step 1.
 * @return 1 while the sequence runs or after a restart, 0 once it has finished.
 */
WMAP_STEP_RUNNER(wmap_land_event_12_run, g_wmap_land_event_12_steps, 0x4, g_wmap_land_event_12_step, g_wmap_land_event_12_timer)

/** @brief Land event 12: restart at step 1 with a one-frame timer. */
WMAP_STEP_RESET(wmap_land_event_12_reset, g_wmap_land_event_12_step, g_wmap_land_event_12_timer)

/** @brief Land event 12: wait for the scroll, then start the land effect. */
WMAP_STEP_WAIT_SCROLL(wmap_land_event_12_step_02, g_wmap_land_event_12_step, wmap_land_event_12_step_03)

/** @brief Land event 12: queue and start the land effect, ending the event. */
void wmap_land_event_12_step_03(void)
{
    wmap_queue_effect_resources(WMAP_LAND_EVENT_12_EFFECT);
    wmap_start_land_effect(WMAP_LAND_EVENT_12_EFFECT);
    g_wmap_land_event_12_step++;
}
