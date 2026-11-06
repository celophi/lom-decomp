#include "common.h"
#include <inline_c.h>
#include <libetc.h>
#include <libgte.h>

#include "../internal/wmap_cells.h"
#include "../internal/wmap_land_layout.h"
#include "../internal/wmap_land_transition.h"
#include "../internal/wmap_main.h"
#include "../internal/wmap_map_display.h"
#include "../internal/wmap_party_travel.h"
#include "../internal/wmap_pathfinding.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_travel_sequences.h"
#include "../internal/wmap_view_effects.h"
#include "common/saved_game.h"
#include "main/audio/akao.h"
#include "main/audio/akao_cmd.h"
#include "main/cdrom.h"
#include "sdk/gte_dmpsx_compat.h"

#define WMAP_TRAVEL_STEP 4
#define WMAP_TRAVEL_ANIMATION_BYTES 0x400
#define WMAP_TRAVEL_SOUND 21
#define WMAP_TRAVEL_TEXTURE_ROW 0x100
#define WMAP_TRAVEL_OT_FALLBACK 31
#define WMAP_TRAVEL_OT_BASE 42
#define WMAP_TRAVEL_DEPTH_ORIGIN 7057
#define WMAP_TRAVEL_DEPTH_RANGE 144U
#define WMAP_TRAVEL_ANIMATION_RESOURCE 0x10C9
#define WMAP_TRAVEL_SPRITE_RESOURCE 0x10CA
#define WMAP_TRAVEL_ALTERNATE_SPRITE_RESOURCE 0x10CB
#define WMAP_TRAVEL_SECOND_ANIMATION_RESOURCE 0x10CC
#define WMAP_TRAVEL_THIRD_ANIMATION_RESOURCE 0x10CD

enum WmapTravelSequence
{
    WMAP_TRAVEL_IDLE,
    WMAP_TRAVEL_DOWN,
    WMAP_TRAVEL_RIGHT,
    WMAP_TRAVEL_LEFT,
    WMAP_TRAVEL_UP
};

/** @brief Screen coordinates in the packed format used by sprite rendering. */
typedef union
{
    s32 packed;
    struct
    {
        s16 x;
        s16 y;
    } point;
} WmapTravelScreen;

extern u8 g_wmap_travel_animation_buffer[];
extern u8 g_wmap_travel_second_animation_buffer[];
extern u8 g_wmap_travel_third_animation_buffer[];
extern s32 g_wmap_travel_sound_active;

void wmap_update_travelers(void);
void wmap_advance_travel_day(void);

/** @brief Advance each traveler's route and synchronize animation, scrolling, and travel sound. */
void wmap_update_travelers(void)
{
    WmapTraveler* traveler;
    WmapTraveler* scripted_traveler;
    s32 i;
    s16 position_x;
    s16 position_y;
    s16 target_x;
    s16 target_y;
    s32 interpolating;
    s32 previous_cell_x;
    s32 previous_cell_y;
    s32 path_index;
    s16 next_x, next_y;

    g_wmap_party_moving = 0;

    for (i = 0; i < WMAP_TRAVELER_COUNT; i++)
    {
        traveler = &g_wmap_travelers[i];
        position_x = traveler->position_x;
        target_x = traveler->target_x;
        interpolating = 0;

        if (position_x != target_x)
        {
            interpolating = 1;

            if (target_x < position_x)
            {
                traveler->position_x = traveler->position_x - WMAP_TRAVEL_STEP;
            }
            else
            {
                traveler->position_x = traveler->position_x + WMAP_TRAVEL_STEP;
            }
        }

        position_y = traveler->position_y;
        target_y = traveler->target_y;

        if (position_y != target_y)
        {
            interpolating = 1;

            if (target_y < position_y)
            {
                traveler->position_y = traveler->position_y - WMAP_TRAVEL_STEP;
            }
            else
            {
                traveler->position_y = traveler->position_y + WMAP_TRAVEL_STEP;
            }
        }

        if (interpolating == 0)
        {
            if ((traveler->cell_x != traveler->next_cell_x || traveler->cell_y != traveler->next_cell_y) && i == 0)
            {
                g_wmap_party_cell_dirty = 1;
            }

            traveler->cell_x = traveler->next_cell_x;
            traveler->cell_y = traveler->next_cell_y;

            if (traveler->destination_x != traveler->cell_x || traveler->destination_y != traveler->cell_y)
            {
                previous_cell_x = traveler->next_cell_x;
                previous_cell_y = traveler->next_cell_y;
                path_index = traveler->path_index + 1;
                traveler->path_index = path_index;
                next_x = g_wmap_travelers[i].path_x.steps[path_index].cell;
                traveler->next_cell_x = next_x;
                traveler->target_x = (next_x - 1) * WMAP_CELL_SIZE;
                next_y = g_wmap_travelers[i].path_y.steps[path_index].cell;
                traveler->next_cell_y = next_y;
                traveler->target_y = (next_y - 1) * WMAP_CELL_SIZE;

                if (g_wmap_scripted_travel_active != 0 && i == WMAP_SCRIPTED_TRAVELER)
                {
                    g_wmap_view_scroll_mode = WMAP_SCROLL_SCRIPTED;
                    g_wmap_scroll_remaining_x =
                        ((scripted_traveler = &g_wmap_travelers[WMAP_SCRIPTED_TRAVELER])->next_cell_x - previous_cell_x) * WMAP_CELL_SPACING;
                    g_wmap_scroll_remaining_y = (scripted_traveler->next_cell_y - previous_cell_y) * WMAP_CELL_SPACING;
                }

                traveler->moving = 1;
            }
            else
            {
                traveler->moving = 0;
                g_wmap_sprite_actors[i].sequence = WMAP_TRAVEL_IDLE;

                if (i == WMAP_SCRIPTED_TRAVELER && g_wmap_scripted_travel_active != 0)
                {
                    g_wmap_scripted_travel_active = 0;
                }
            }
        }

        if (g_wmap_travelers[i].moving != 0)
        {
            if (traveler->target_x == traveler->position_x)
            {
                target_y = traveler->target_y;
                position_y = traveler->position_y;

                if (target_y > position_y)
                {
                    g_wmap_sprite_actors[i].sequence = WMAP_TRAVEL_DOWN;
                }
                else if (target_y < position_y)
                {
                    g_wmap_sprite_actors[i].sequence = WMAP_TRAVEL_UP;
                }
            }

            if (traveler->target_y == traveler->position_y)
            {
                target_x = traveler->target_x;
                position_x = traveler->position_x;

                if (target_x > position_x)
                {
                    g_wmap_sprite_actors[i].sequence = WMAP_TRAVEL_RIGHT;
                }
                else if (target_x < position_x)
                {
                    g_wmap_sprite_actors[i].sequence = WMAP_TRAVEL_LEFT;
                }
            }
        }

        g_wmap_party_moving |= traveler->moving;
    }

    if (g_wmap_party_moving != g_wmap_travel_sound_active)
    {
        if (g_wmap_party_moving != 0 && g_wmap_sequence_count == 0)
        {
            wmap_play_sound(WMAP_TRAVEL_SOUND, AKAO_SFX_DEFAULT_VOLUME);
            g_wmap_travel_sound_active = g_wmap_party_moving;
        }

        if (g_wmap_party_moving != g_wmap_travel_sound_active && g_wmap_party_moving == 0)
        {
            akao_release_all_sfx();
            g_wmap_travel_sound_active = g_wmap_party_moving;
        }
    }

    if (g_wmap_party_cell_dirty != 0)
    {
        wmap_advance_travel_day();
        g_wmap_party_cell_dirty = 0;
    }
}

/** @brief Draw the travelers, accept a destination, and advance their movement. */
void wmap_update_party_travel(void)
{
    SVECTOR position;
    s32 depth;
    s32 scale;
    s32 view_x, view_y;
    WmapTravelScreen screen;
    s32 packed_position;
    s32 flat_y;
    s32 flat_x_bits;
    WmapSpriteActor* sprite;
    s32 i;
    s32 ot_index;
    s32 depth_index;
    s32 selected_x;
    s32 selected_y;
    s32 at_destination;

    if (g_wmap_party_visible != 0)
    {
        for (i = 0; i < WMAP_TRAVELER_COUNT; i++)
        {
            sprite = &g_wmap_sprite_actors[i];

            if (sprite->resource_index != -1)
            {
                wmap_step_actor_animation(sprite, &g_wmap_actor_animations[i]);

                if (g_wmap_view_mode == WMAP_VIEW_MODE_MAP)
                {
                    screen.point.x = g_wmap_travelers[i].position_x;
                    screen.point.y = g_wmap_travelers[i].position_y;
                    scale = g_wmap_view.projection_scale;
                    view_x = g_wmap_view.x * WMAP_MAP_PROJECTION_SCALE / scale - 20;
                    position.vx = (g_wmap_travelers[i].position_x - view_x) * WMAP_VIEW_SCALE / scale;
                    view_y = g_wmap_view.y * WMAP_MAP_PROJECTION_SCALE / scale - 20;
                    position.vy = (g_wmap_travelers[i].position_y - view_y) * WMAP_VIEW_SCALE / scale;
                    position.vz = 0;
                    gte_ldv0(&position);
                    gte_rtps();
                    gte_stsxy(&screen.packed);
                    gte_stszotz(&depth);
                    depth_index = (WMAP_TRAVEL_DEPTH_ORIGIN - depth) / 4;
                    ot_index = depth_index + WMAP_TRAVEL_OT_BASE;

                    if ((u32)(depth_index + 11) >= WMAP_TRAVEL_DEPTH_RANGE)
                    {
                        ot_index = WMAP_TRAVEL_OT_FALLBACK;
                    }

                    if (wmap_get_point_display_mode(g_wmap_travelers[i].position_x * WMAP_CELL_SPACING / WMAP_CELL_SIZE + WMAP_CELL_SPACING,
                                                    g_wmap_travelers[i].position_y * WMAP_CELL_SPACING / WMAP_CELL_SIZE + WMAP_CELL_SPACING, scale) != 0)
                    {
                        wmap_draw_actor_sprite(sprite, screen.packed, i, ot_index, WMAP_TRAVEL_TEXTURE_ROW + i * WMAP_TRAVEL_TEXTURE_ROW);
                    }
                }

                if (g_wmap_view_mode == WMAP_VIEW_MODE_SPIRITS)
                {
                    packed_position = g_wmap_spirit_view_cell_positions[g_wmap_travelers[i].cell_x + g_wmap_travelers[i].cell_y * WMAP_GRID_SIZE].packed;
                    flat_x_bits = packed_position + 14;
                    packed_position &= 0xFFFF0000;
                    flat_x_bits &= 0xFFFF;
                    packed_position |= flat_x_bits;
                    flat_y = packed_position >> 16;
                    packed_position &= 0xFFFF;
                    wmap_draw_actor_sprite(sprite, (u16)packed_position | ((flat_y + 28) << 16), i, WMAP_TRAVEL_OT_FALLBACK,
                                           WMAP_TRAVEL_TEXTURE_ROW + i * WMAP_TRAVEL_TEXTURE_ROW);
                }
            }
        }
    }

    if ((g_wmap_buttons_repeat & WMAP_PAD_CONFIRM) && g_wmap_party_moving == 0 && g_wmap_artifact_placement_active == 0)
    {
        selected_x = g_wmap_view.x / WMAP_CELL_SPACING + g_wmap_cursor_column;
        selected_y = g_wmap_view.y / WMAP_CELL_SPACING + g_wmap_cursor_row;

        if (g_wmap_cells[selected_x][selected_y].travel_allowed != 0 && g_wmap_view_mode == WMAP_VIEW_MODE_MAP && g_wmap_selection_phase == 0)
        {
            if (selected_x != g_wmap_travelers[0].cell_x || (at_destination = 1, selected_y != g_wmap_travelers[0].cell_y))
            {
                at_destination = 0;
            }

            g_wmap_exit_frame = at_destination;

            if (at_destination == 0)
            {
                if (g_wmap_cells[selected_x][selected_y].land_id == FIELD_LAND_LUCEMIA)
                {
                    wmap_start_sequence(wmap_run_special_travel);
                }
                else
                {
                    g_wmap_travelers[0].destination_x = selected_x;
                    g_wmap_travelers[0].destination_y = selected_y;

                    if (selected_x != g_wmap_travelers[0].cell_x || selected_y != g_wmap_travelers[0].cell_y)
                    {
                        wmap_build_route(g_wmap_travelers[0].cell_x, g_wmap_travelers[0].cell_y, selected_x, selected_y, g_wmap_travelers[0].path_x.cells,
                                         g_wmap_travelers[0].path_y.cells);
                        g_wmap_travelers[0].moving = 1;
                        /* The route begins with the cell the party already occupies. */
                        g_wmap_travelers[0].path_index = 1;
                        g_wmap_travelers[0].next_cell_x = g_wmap_travelers[0].path_x.steps[1].cell;
                        g_wmap_travelers[0].target_x = (g_wmap_travelers[0].path_x.steps[1].cell - 1) * WMAP_CELL_SIZE;
                        g_wmap_travelers[0].next_cell_y = g_wmap_travelers[0].path_y.steps[1].cell;
                        g_wmap_travelers[0].target_y = (g_wmap_travelers[0].path_y.steps[1].cell - 1) * WMAP_CELL_SIZE;
                    }
                    else
                    {
                        g_wmap_travelers[0].moving = 0;
                    }
                }
            }
        }
    }

    wmap_update_travelers();
}

/** @brief Load traveler animations and restore the party and optional actors to their map cells. */
void wmap_init_party_travel(void)
{
    s32 first_x, first_y, second_x, second_y;
    s32 i;
    s32 resource_id;
    u8* resource;
    WmapTraveler* traveler;
    WmapSpriteActor* sprite;

    cdrom_queue_read(WMAP_TRAVEL_ANIMATION_RESOURCE, g_wmap_travel_animation_buffer);
    cdrom_wait_queue_empty();
    resource = g_wmap_travel_animation_buffer;
    g_wmap_sprite_actors[0].resource_index = 0;

    for (i = 0; i < WMAP_TRAVELER_COUNT; i++)
    {
        traveler = &g_wmap_travelers[i];
        traveler->cell_y = 1;
        traveler->cell_x = 1;
        traveler->next_cell_y = 1;
        traveler->next_cell_x = 1;
        traveler->destination_y = 1;
        traveler->destination_x = 1;
        traveler->target_y = 0;
        traveler->target_x = 0;
        traveler->position_y = 0;
        traveler->position_x = 0;
        traveler->moving = 0;
        sprite = &g_wmap_sprite_actors[i];
        sprite->scale_index = WMAP_SPRITE_SCALE_FULL;
        sprite->previous_sequence = -1;
        sprite->target_shade = WMAP_SPRITE_SHADE_NEUTRAL;
        sprite->shade = WMAP_SPRITE_SHADE_NEUTRAL;
        g_wmap_actor_animations[i].data = resource + i * WMAP_TRAVEL_ANIMATION_BYTES;
    }

    g_wmap_entry_day = wmap_get_starting_cell(&g_wmap_travelers[0].cell_x, &g_wmap_travelers[0].cell_y);
    wmap_set_traveler_position(0, g_wmap_travelers[0].cell_x, g_wmap_travelers[0].cell_y);

    if (g_wmap_cells[g_wmap_travelers[0].cell_x][g_wmap_travelers[0].cell_y].land_id == FIELD_LAND_LUCEMIA)
    {
        g_wmap_input_locked = 1;
        g_wmap_buttons_held = 0;
        g_wmap_buttons_repeat = 0;
        g_wmap_status_panel_mode = 3;
        g_wmap_carousel_mode = 3;
        g_wmap_special_return_pending = 1;
        g_wmap_sprite_actors[0].resource_index = -1;
        g_wmap_pending_event_count++;
    }

    resource_id = WMAP_TRAVEL_SPRITE_RESOURCE;

    if (g_wmap_entry_day != 0)
    {
        resource_id = WMAP_TRAVEL_ALTERNATE_SPRITE_RESOURCE;
    }

    wmap_load_tim_file(resource_id);

    if (g_wmap_second_traveler_enabled != 0)
    {
        cdrom_queue_read(WMAP_TRAVEL_SECOND_ANIMATION_RESOURCE, g_wmap_travel_second_animation_buffer);
        g_wmap_sprite_actors[1].resource_index = 1;
        wmap_find_land_cell(FIELD_LAND_SS_BUCCANEER, &first_x, &first_y);
        wmap_set_traveler_position(1, first_x, first_y);
        cdrom_wait_queue_empty();
    }

    if (g_wmap_third_traveler_enabled != 0)
    {
        cdrom_queue_read(WMAP_TRAVEL_THIRD_ANIMATION_RESOURCE, g_wmap_travel_third_animation_buffer);
        g_wmap_sprite_actors[2].resource_index = 2;
        wmap_find_land_cell(FIELD_LAND_POLPOTA_HARBOR, &second_x, &second_y);
        wmap_set_traveler_position(2, second_x, second_y);
        cdrom_wait_queue_empty();
    }
}

/**
 * @brief Set a traveler's cell, destination, and position without changing route activity.
 * @param index Traveler slot, from zero through three.
 * @param x Map column.
 * @param y Map row.
 */
void wmap_set_traveler_position(s32 index, s32 x, s32 y)
{
    s16 position_x;
    s16 position_y;
    WmapTraveler* traveler;
    WmapTraveler* base;

    base = g_wmap_travelers;
    traveler = &base[index];
    traveler->cell_x = x;
    traveler->destination_x = x;
    traveler->next_cell_x = (s16)x;
    position_x = ((s16)x - 1) * WMAP_CELL_SIZE;
    traveler->cell_y = y;
    traveler->destination_y = y;
    traveler->next_cell_y = (s16)y;
    traveler->target_x = position_x;
    traveler->position_x = position_x;
    position_y = ((s16)y - 1) * WMAP_CELL_SIZE;
    traveler->target_y = position_y;
    traveler->position_y = position_y;
}

/** @brief Apply the travel-day update and refresh the day shown on the map. */
void wmap_advance_travel_day(void)
{
    wmap_update_travel_growth();
    g_wmap_travel_day = wmap_get_day();
}
