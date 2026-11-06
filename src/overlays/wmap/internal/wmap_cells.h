#ifndef WMAP_CELLS_H
#define WMAP_CELLS_H

#include "common.h"

/** @brief Width and height of the world map's land grid, in cells. */
#define WMAP_GRID_SIZE 6

/** @brief wmap_get_land_at_cell result and WmapCell land_id for a cell with no land, or outside the map. */
#define WMAP_NO_LAND 0xFF

/** @brief WmapCell land_id flag: draw the land at full size at once, skipping its grow-in. */
#define WMAP_LAND_FULL_SIZE 0x100

/** @brief Map distance between neighboring land cells. */
#define WMAP_CELL_SPACING 48

/** @brief One cell of the world map's land grid (40 bytes). */
typedef struct
{
    /** @brief Land display id of the cell's land (g_wmap_land_display index), or WMAP_NO_LAND; may carry WMAP_LAND_FULL_SIZE. */
    s32 land_id;
    /** @brief Nonzero when the selected artifact can be placed in this cell. */
    s16 placement_allowed;
    /** @brief Nonzero when the party can travel here: a land stands in the cell and it is not being replaced. */
    s16 travel_allowed;
    /** @brief Spirit sprites drawn on the cell. */
    s32 spirit_sprites[8];
} WmapCell;

/** @brief The land grid, indexed [x][y]. */
extern WmapCell g_wmap_cells[WMAP_GRID_SIZE][WMAP_GRID_SIZE];

#endif
