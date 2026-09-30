#ifndef WMAP_CELLS_H
#define WMAP_CELLS_H

#include "common.h"

/** @brief Width and height of the world map's land grid, in cells. */
#define WMAP_GRID_SIZE 6

/** @brief One cell of the world map's land grid (40 bytes). */
typedef struct
{
    /** @brief Land placed in the cell: its artifact id with bit 0x100 set; 0 when empty. */
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
