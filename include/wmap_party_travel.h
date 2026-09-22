#ifndef WMAP_PARTY_TRAVEL_H
#define WMAP_PARTY_TRAVEL_H

#include "common.h"

#define WMAP_TRAVELER_COUNT 4
#define WMAP_TRAVEL_PATH_CAPACITY 32

/** @brief Route coordinates written as words and consumed as signed halfwords. */
typedef union
{
    s32 cells[WMAP_TRAVEL_PATH_CAPACITY];
    struct
    {
        s16 cell;
        s16 high;
    } steps[WMAP_TRAVEL_PATH_CAPACITY];
} WmapTravelPath;

/**
 * @brief A map traveler's current cell, interpolated position, and pending route.
 * @note Interpolated positions use 160 units per cell, relative to cell (1, 1).
 */
typedef struct
{
    s32 cell_x;
    s32 cell_y;
    s16 next_cell_x;
    s16 next_cell_y;
    s16 position_x;
    s16 position_y;
    s16 target_x;
    s16 target_y;
    s32 moving;
    s32 destination_x;
    s32 destination_y;
    s32 path_index;
    WmapTravelPath path_x;
    WmapTravelPath path_y;
} WmapTraveler;

extern WmapTraveler g_wmap_travelers[WMAP_TRAVELER_COUNT];
extern s32 g_wmap_party_cell_dirty;
extern s32 g_wmap_party_moving;
extern s32 g_wmap_party_visible;
extern s32 g_wmap_scripted_travel_active;
extern s32 g_wmap_travel_day;

void wmap_update_party_travel(void);
void wmap_init_party_travel(void);
void wmap_set_traveler_position(s32 index, s32 x, s32 y);

#endif
