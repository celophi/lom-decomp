#include "wmap_main.h"
#include "wmap_party_travel.h"
#include "wmap_pathfinding.h"
#include "wmap_map_display.h"
#include "wmap_resource_support.h"
#include "wmap_sprite_render.h"
#include "wmap_sequence_runtime.h"
#include "wmap_travel_sequences.h"
#include "wmap_land_layout.h"
#include "sdk/libgte.h"
#include "sdk/libetc.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "cdrom.h"
#include "akao_cmd.h"

#define WMAP_GRID_SIZE 6
#define WMAP_CELL_SPACING 48
#define WMAP_TRAVEL_CELL_UNITS 160
#define WMAP_TRAVEL_STEP 4
#define WMAP_SCRIPTED_TRAVELER 3
#define WMAP_TRAVEL_ANIMATION_BYTES 0x400
#define WMAP_TRAVEL_SCALE_INDEX 15
#define WMAP_TRAVEL_SHADE 128
#define WMAP_TRAVEL_SOUND 21
#define WMAP_TRAVEL_SOUND_VOLUME 128
#define WMAP_TRAVEL_TEXTURE_ROW 0x100
#define WMAP_TRAVEL_OT_FALLBACK 31
#define WMAP_TRAVEL_OT_BASE 42
#define WMAP_TRAVEL_DEPTH_ORIGIN 7057
#define WMAP_TRAVEL_DEPTH_RANGE 144U
#define WMAP_SPECIAL_TRAVEL_LAND 24
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

/** @brief Sprite resource, animation selection, and shading for a map actor. */
typedef struct
{
    s16 unknown_00;
    s16 resource_index;
    u8 pad_04[2];
    u8 scale_index;
    u8 pad_07[7];
    s16 sequence;
    s16 previous_sequence;
    u8 pad_12[16];
    s16 target_shade;
    s16 shade;
    s16 shade_step;
    u8 pad_28[4];
} WmapTravelSprite;

/** @brief Animation resource slot shared with the sequence interpreter. */
typedef struct
{
    s32 unknown_00;
    u8* data;
} WmapTravelAnimation;

/** @brief Map-cell identity and the flags used to select a destination. */
typedef struct
{
    s32 land_id;
    s16 effect_enabled;
    s16 traversable;
    u8 pad_08[32];
} WmapTravelCell;

/** @brief Map translation and projection scale. */
typedef struct
{
    s32 x;
    s32 y;
    s32 scale;
} WmapTravelProjection;

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

extern WmapTravelSprite D_800D9268[];
extern WmapTravelAnimation D_80139988[];
extern WmapTravelCell D_80139290[WMAP_GRID_SIZE][WMAP_GRID_SIZE];
extern WmapTravelProjection D_80139950;
extern const WmapTravelScreen D_8004FD04[];
extern u8 D_800DBE98[];
extern u8 D_800DC298[];
extern u8 D_800DC698[];
extern s16 D_800D926A;
extern s16 D_800D9296;
extern s16 D_800D92C2;
extern s32 D_800D9224;
extern s32 D_800DBE78;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_8011CF18;
extern s32 D_8011CF20;
extern s32 D_8011CF44;
extern s32 D_80129550;
extern s32 D_8013986C;
extern s32 D_801398B8;
extern s32 D_801398D0;
extern s32 D_8013B294;
extern s32 D_80182D5C;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_80182E1C;
extern s32 D_80182E34;
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
                traveler->position_x = (u16)traveler->position_x - WMAP_TRAVEL_STEP;
            }
            else
            {
                traveler->position_x = (u16)traveler->position_x + WMAP_TRAVEL_STEP;
            }
        }
        position_y = traveler->position_y;
        target_y = traveler->target_y;
        if (position_y != target_y)
        {
            interpolating = 1;
            if (target_y < position_y)
            {
                traveler->position_y = (u16)traveler->position_y - WMAP_TRAVEL_STEP;
            }
            else
            {
                traveler->position_y = (u16)traveler->position_y + WMAP_TRAVEL_STEP;
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
                traveler->target_x = (next_x - 1) * WMAP_TRAVEL_CELL_UNITS;
                next_y = g_wmap_travelers[i].path_y.steps[path_index].cell;
                traveler->next_cell_y = next_y;
                traveler->target_y = (next_y - 1) * WMAP_TRAVEL_CELL_UNITS;
                if (g_wmap_scripted_travel_active != 0 && i == WMAP_SCRIPTED_TRAVELER)
                {
                    D_801398D0 = 2;
                    D_80182D68 = ((scripted_traveler = &g_wmap_travelers[WMAP_SCRIPTED_TRAVELER])->next_cell_x - previous_cell_x) * WMAP_CELL_SPACING;
                    D_80182D78 = (scripted_traveler->next_cell_y - previous_cell_y) * WMAP_CELL_SPACING;
                }
                traveler->moving = 1;
            }
            else
            {
                traveler->moving = 0;
                D_800D9268[i].sequence = WMAP_TRAVEL_IDLE;
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
                    D_800D9268[i].sequence = WMAP_TRAVEL_DOWN;
                }
                else if (target_y < position_y)
                {
                    D_800D9268[i].sequence = WMAP_TRAVEL_UP;
                }
            }
            if (traveler->target_y == traveler->position_y)
            {
                target_x = traveler->target_x;
                position_x = traveler->position_x;
                if (target_x > position_x)
                {
                    D_800D9268[i].sequence = WMAP_TRAVEL_RIGHT;
                }
                else if (target_x < position_x)
                {
                    D_800D9268[i].sequence = WMAP_TRAVEL_LEFT;
                }
            }
        }
        g_wmap_party_moving |= traveler->moving;
    }
    if (g_wmap_party_moving != g_wmap_travel_sound_active)
    {
        if (g_wmap_party_moving != 0 && D_8011CF44 == 0)
        {
            func_800652A8(WMAP_TRAVEL_SOUND, WMAP_TRAVEL_SOUND_VOLUME);
            g_wmap_travel_sound_active = g_wmap_party_moving;
        }
        if (g_wmap_party_moving != g_wmap_travel_sound_active && g_wmap_party_moving == 0)
        {
            akao_cmd_f1();
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
    WmapTravelSprite* sprite;
    s32 i;
    s32 ot_index;
    s32 depth_index;
    s32 selected_x;
    s32 selected_y;
    s32 at_destination;

    if (g_wmap_party_visible != 0)
    {
        i = 0;
        do
        {
            sprite = &D_800D9268[i];
            if (sprite->resource_index != -1)
            {
                func_8006CC4C(sprite, &D_80139988[i]);
                if (D_8013986C == 0)
                {
                    screen.point.x = g_wmap_travelers[i].position_x;
                    screen.point.y = g_wmap_travelers[i].position_y;
                    scale = D_80139950.scale;
                    view_x = D_80139950.x * 0x14000 / scale - 20;
                    position.vx = (g_wmap_travelers[i].position_x - view_x) * 0x6000 / scale;
                    view_y = D_80139950.y * 0x14000 / scale - 20;
                    position.vy = (g_wmap_travelers[i].position_y - view_y) * 0x6000 / scale;
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
                    if (wmap_get_point_display_mode(g_wmap_travelers[i].position_x * WMAP_CELL_SPACING / WMAP_TRAVEL_CELL_UNITS + WMAP_CELL_SPACING,
                                                    g_wmap_travelers[i].position_y * WMAP_CELL_SPACING / WMAP_TRAVEL_CELL_UNITS + WMAP_CELL_SPACING,
                                                    scale) != 0)
                    {
                        func_80066F9C(sprite, screen.packed, i, ot_index, WMAP_TRAVEL_TEXTURE_ROW + i * WMAP_TRAVEL_TEXTURE_ROW);
                    }
                }
                if (D_8013986C == 1)
                {
                    packed_position = D_8004FD04[g_wmap_travelers[i].cell_x + g_wmap_travelers[i].cell_y * WMAP_GRID_SIZE].packed;
                    flat_x_bits = packed_position + 14;
                    packed_position &= 0xFFFF0000;
                    flat_x_bits &= 0xFFFF;
                    packed_position |= flat_x_bits;
                    flat_y = packed_position >> 16;
                    packed_position &= 0xFFFF;
                    func_80066F9C(sprite, (u16)packed_position | ((flat_y + 28) << 16), i, WMAP_TRAVEL_OT_FALLBACK, WMAP_TRAVEL_TEXTURE_ROW + i * WMAP_TRAVEL_TEXTURE_ROW);
                }
            }
            i++;
        } while (i < WMAP_TRAVELER_COUNT);
    }
    if ((g_wmap_buttons_repeat & PADRdown) && g_wmap_party_moving == 0 && D_80129550 == 0)
    {
        selected_x = D_80139950.x / WMAP_CELL_SPACING + D_800DCEEC;
        selected_y = D_80139950.y / WMAP_CELL_SPACING + D_800DCEF0;
        if (D_80139290[selected_x][selected_y].traversable != 0 && D_8013986C == 0 && D_8011CF18 == 0)
        {
            if (selected_x != g_wmap_travelers[0].cell_x || (at_destination = 1, selected_y != g_wmap_travelers[0].cell_y))
            {
                at_destination = 0;
            }
            D_8013B294 = at_destination;
            if (at_destination == 0)
            {
                if (D_80139290[selected_x][selected_y].land_id == WMAP_SPECIAL_TRAVEL_LAND)
                {
                    func_8006CAC0(func_8009A420);
                }
                else
                {
                    g_wmap_travelers[0].destination_x = selected_x;
                    g_wmap_travelers[0].destination_y = selected_y;
                    if (selected_x != g_wmap_travelers[0].cell_x || selected_y != g_wmap_travelers[0].cell_y)
                    {
                        func_8005EB68(g_wmap_travelers[0].cell_x, g_wmap_travelers[0].cell_y, selected_x, selected_y, g_wmap_travelers[0].path_x.cells,
                                      g_wmap_travelers[0].path_y.cells);
                        g_wmap_travelers[0].moving = 1;
                        /* The route begins with the cell the party already occupies. */
                        g_wmap_travelers[0].path_index = 1;
                        g_wmap_travelers[0].next_cell_x = g_wmap_travelers[0].path_x.steps[1].cell;
                        g_wmap_travelers[0].target_x = (g_wmap_travelers[0].path_x.steps[1].cell - 1) * WMAP_TRAVEL_CELL_UNITS;
                        g_wmap_travelers[0].next_cell_y = g_wmap_travelers[0].path_y.steps[1].cell;
                        g_wmap_travelers[0].target_y = (g_wmap_travelers[0].path_y.steps[1].cell - 1) * WMAP_TRAVEL_CELL_UNITS;
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
    WmapTravelSprite* sprite;

    cdrom_queue_read(WMAP_TRAVEL_ANIMATION_RESOURCE, D_800DBE98);
    cdrom_wait_queue_empty();
    resource = D_800DBE98;
    D_800D9268[0].resource_index = 0;
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
        sprite = &D_800D9268[i];
        sprite->scale_index = WMAP_TRAVEL_SCALE_INDEX;
        sprite->previous_sequence = -1;
        sprite->target_shade = WMAP_TRAVEL_SHADE;
        sprite->shade = WMAP_TRAVEL_SHADE;
        D_80139988[i].data = resource + i * WMAP_TRAVEL_ANIMATION_BYTES;
    }
    D_80182D5C = wmap_get_starting_cell(&g_wmap_travelers[0].cell_x, &g_wmap_travelers[0].cell_y);
    wmap_set_traveler_position(0, g_wmap_travelers[0].cell_x, g_wmap_travelers[0].cell_y);
    if (D_80139290[g_wmap_travelers[0].cell_x][g_wmap_travelers[0].cell_y].land_id == WMAP_SPECIAL_TRAVEL_LAND)
    {
        g_wmap_input_locked = 1;
        g_wmap_buttons_held = 0;
        g_wmap_buttons_repeat = 0;
        D_80182E34 = 3;
        D_800DBE78 = 3;
        D_8011CF20 = 1;
        D_800D926A = -1;
        D_800D9224++;
    }
    resource_id = WMAP_TRAVEL_SPRITE_RESOURCE;
    if (D_80182D5C != 0)
    {
        resource_id = WMAP_TRAVEL_ALTERNATE_SPRITE_RESOURCE;
    }
    func_80064F64(resource_id);
    if (D_801398B8 != 0)
    {
        cdrom_queue_read(WMAP_TRAVEL_SECOND_ANIMATION_RESOURCE, D_800DC298);
        D_800D9296 = 1;
        func_8006D0F0(27, &first_x, &first_y);
        wmap_set_traveler_position(1, first_x, first_y);
        cdrom_wait_queue_empty();
    }
    if (D_80182E1C != 0)
    {
        cdrom_queue_read(WMAP_TRAVEL_THIRD_ANIMATION_RESOURCE, D_800DC698);
        D_800D92C2 = 2;
        func_8006D0F0(3, &second_x, &second_y);
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
    position_x = ((s16)x - 1) * WMAP_TRAVEL_CELL_UNITS;
    traveler->cell_y = y;
    traveler->destination_y = y;
    traveler->next_cell_y = (s16)y;
    traveler->target_x = position_x;
    traveler->position_x = position_x;
    position_y = ((s16)y - 1) * WMAP_TRAVEL_CELL_UNITS;
    traveler->target_y = position_y;
    traveler->position_y = position_y;
}

/** @brief Apply the travel-day update and refresh the day shown on the map. */
void wmap_advance_travel_day(void)
{
    wmap_update_travel_growth();
    g_wmap_travel_day = wmap_get_day();
}
