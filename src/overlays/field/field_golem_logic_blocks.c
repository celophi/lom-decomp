/**
 * @file field_golem_logic_blocks.c
 * @brief Golem logic-block placement for the GOLEM editor overlay.
 *
 * Appends logic blocks, stamps their shapes on the six-by-six grid, and
 * checks a placement against the grid bound and the edited group's logic
 * class. The GOLEM overlay calls these functions.
 */

#include "saved_game.h"
#include "common.h"
#include "field_golem_layout.h"
#include "field_menu_vars.h"

/** @brief Logic class of a block that fits every group. */
#define GOLEM_LOGIC_CLASS_ANY 0
/** @brief CLUT of an unavailable block's icon. */
#define GOLEM_UNAVAILABLE_CLUT 0xF
/** @brief Number of divider marker words: vertical edges, then horizontal edges. */
#define GOLEM_MARKER_COUNT (2 * GOLEM_GRID_WIDTH * (GOLEM_GRID_WIDTH - 1))
/** @brief Index of the first horizontal-edge marker (one per cell above the last row). */
#define GOLEM_HORIZONTAL_MARKER_BASE (GOLEM_GRID_WIDTH * (GOLEM_GRID_WIDTH - 1))
/** @brief Marker value for an edge between two different blocks. */
#define GOLEM_MARKER_BLOCK_EDGE 0x4F

/** @brief Placement status of one logic block, filled by golem_fill_logic_block_status. */
typedef struct
{
    u16 is_unavailable; /**< Non-zero when the block cannot join the edited group. */
    u16 clut;           /**< CLUT row used to draw the block's icon. */
} GolemLogicBlockStatus;

/** @brief Icon CLUT of one logic-block id. */
typedef struct
{
    u16 clut;
    u16 reserved;
} GolemLogicBlockIcon;

extern GolemShape g_golem_shape_table[GOLEM_SHAPE_COUNT];
extern s32 g_golem_logic_block_class[];
extern GolemLogicBlockIcon g_golem_logic_block_icons[];
extern u8 D_800459AE;

/**
 * @brief Append a new, unassigned logic block to the golem logic-block table.
 * @param block_id Six-bit block id.
 * @param detail Four-bit detail (quantity) value.
 * @param shape Four-bit shape index into g_golem_shape_table.
 */
void golem_logic_block_append(u32 block_id, u32 detail, u32 shape)
{
    GOLEM.logic_blocks[GOLEM.header.fields.block_count].f.id = block_id;
    GOLEM.logic_blocks[GOLEM.header.fields.block_count].f.quantity = detail;
    GOLEM.logic_blocks[GOLEM.header.fields.block_count].f.shape = shape;
    GOLEM.logic_blocks[GOLEM.header.fields.block_count].f.logic_type = LOGIC_BLOCK_UNASSIGNED;
    GOLEM.logic_blocks[GOLEM.header.fields.block_count].f.placed = 0;
    GOLEM.header.fields.block_count++;
}

/**
 * @brief Rebuild the grid owners from the placed blocks of the edited group.
 * @return Grid bound of the edited group.
 */
u32 golem_rebuild_grid_owners(void)
{
    s32 i;
    s32 part;
    s32 count;
    s32 offset_x;
    s32 offset_y;
    LogicBlock block;
    LogicBlock placed;

    for (i = 0; i < GOLEM_GRID_CELL_COUNT; i++)
    {
        GOLEM.grid[i].owner = GOLEM_GRID_EMPTY;
    }

    count = GOLEM.header.fields.block_count;
    for (i = 0; i < count; i++)
    {
        block = GOLEM.logic_blocks[i];
        if ((block.f.placed == 1) && (block.f.logic_type == GOLEM.group_order[D_80122C00.golem.slot]))
        {
            for (part = 0; part < g_golem_shape_table[GOLEM.logic_blocks[i].f.shape].count; part++)
            {
                placed = GOLEM.logic_blocks[i];
                offset_x = g_golem_shape_table[placed.f.shape].rotations[placed.f.rotation].parts[part].x;
                offset_y = g_golem_shape_table[placed.f.shape].rotations[placed.f.rotation].parts[part].y;
                GOLEM.grid[placed.f.grid_x + offset_x + (placed.f.grid_y + offset_y) * GOLEM_GRID_WIDTH].owner = i;
            }
        }
    }
    return GOLEM.group_records[GOLEM.group_order[D_80122C00.golem.slot]].grid_bound;
}

/**
 * @brief Place a logic block in the edited group and mark its cells on the grid.
 * @param index Logic-block index to place.
 * @param rotation Placed rotation, 0-3.
 * @param x Grid column of the shape origin.
 * @param y Grid row of the shape origin.
 */
void golem_place_logic_block(s32 index, s32 rotation, s32 x, s32 y)
{
    s32 i;
    u8 group;
    LogicBlock block;

    group = GOLEM.group_order[D_80122C00.golem.slot];
    block = GOLEM.logic_blocks[index];
    block.f.logic_type = group;
    block.f.placed = 1;
    block.f.rotation = rotation;
    block.f.grid_x = x;
    block.f.grid_y = y;
    GOLEM.logic_blocks[index] = block;
    for (i = 0; i < g_golem_shape_table[GOLEM.logic_blocks[index].f.shape].count; i++)
    {
        s32 cell_x = x + g_golem_shape_table[GOLEM.logic_blocks[index].f.shape].rotations[rotation].parts[i].x;
        s32 cell_y = y + g_golem_shape_table[GOLEM.logic_blocks[index].f.shape].rotations[rotation].parts[i].y;

        GOLEM.grid[cell_x + cell_y * GOLEM_GRID_WIDTH].owner = index;
    }
}

/**
 * @brief Check that a block's cells are free and that it fits the edited group's class.
 * @param index Logic-block index to test.
 * @param rotation Rotation to test, 0-3.
 * @param x Grid column of the shape origin.
 * @param y Grid row of the shape origin.
 * @return 1 when every covered cell is empty and the class is compatible, otherwise 0.
 */
s32 golem_can_place_logic_block(s32 index, s32 rotation, s32 x, s32 y)
{
    s32 i;
    s32 valid;
    s32 cell_x;
    s32 cell_y;
    s32 logic_class;

    valid = 1;
    for (i = 0; i < g_golem_shape_table[GOLEM.logic_blocks[index].f.shape].count; i++)
    {
        cell_x = x + g_golem_shape_table[GOLEM.logic_blocks[index].f.shape].rotations[rotation].parts[i].x;
        cell_y = y + g_golem_shape_table[GOLEM.logic_blocks[index].f.shape].rotations[rotation].parts[i].y;
        if (GOLEM.grid[cell_x + cell_y * GOLEM_GRID_WIDTH].owner != GOLEM_GRID_EMPTY)
        {
            valid = 0;
        }
    }

    logic_class = g_golem_logic_block_class[GOLEM.logic_blocks[index].f.id];
    if (logic_class != GOLEM_LOGIC_CLASS_ANY)
    {
        if (logic_class != GOLEM.group_records[GOLEM.group_order[D_80122C00.golem.slot]].logic_class)
        {
            valid = 0;
        }
    }
    return valid;
}

/**
 * @brief Check that every cell of a block lies inside the edited group's grid bound.
 * @param index Logic-block index to test.
 * @param rotation Rotation to test, 0-3.
 * @param x Grid column of the shape origin.
 * @param y Grid row of the shape origin.
 * @return 1 when every part is within the bound (clamped to the grid width), otherwise 0.
 */
s32 golem_logic_block_fits_grid(s32 index, s32 rotation, s32 x, s32 y)
{
    s32 i;
    s32 valid;
    s32 limit;
    s32 cell_x;
    s32 cell_y;

    limit = GOLEM.group_records[GOLEM.group_order[D_80122C00.golem.slot]].grid_bound;
    if (limit >= GOLEM_GRID_WIDTH)
    {
        limit = GOLEM_GRID_WIDTH;
    }
    valid = 1;
    for (i = 0; i < g_golem_shape_table[GOLEM.logic_blocks[index].f.shape].count; i++)
    {
        cell_x = g_golem_shape_table[GOLEM.logic_blocks[index].f.shape].rotations[rotation].parts[i].x + x;
        cell_y = g_golem_shape_table[GOLEM.logic_blocks[index].f.shape].rotations[rotation].parts[i].y + y;
        if ((cell_x < 0) || (cell_x >= limit))
        {
            valid = 0;
        }
        if ((cell_y < 0) || (cell_y >= limit))
        {
            valid = 0;
        }
    }
    return valid;
}

/**
 * @brief Fill the placement status of every logic block for the edited group.
 * @param results Output status per logic block.
 * @return The global status byte D_800459AE.
 */
u8 golem_fill_logic_block_status(GolemLogicBlockStatus* results)
{
    s32 i;
    LogicBlock block;
    u8 group;
    s32 logic_class;

    for (i = 0; i < GOLEM.header.fields.block_count; i++)
    {
        block = GOLEM.logic_blocks[i];
        group = GOLEM.group_order[D_80122C00.golem.slot];
        if (((block.f.logic_type != group) && (block.f.logic_type != LOGIC_BLOCK_UNASSIGNED)) ||
            (((logic_class = g_golem_logic_block_class[block.f.id]) != GOLEM_LOGIC_CLASS_ANY) && (GOLEM.group_records[group].logic_class != logic_class)))
        {
            results[i].is_unavailable = 1;
            results[i].clut = GOLEM_UNAVAILABLE_CLUT;
        }
        else
        {
            results[i].is_unavailable = 0;
            results[i].clut = g_golem_logic_block_icons[GOLEM.logic_blocks[i].f.id].clut;
        }
    }
    return D_800459AE;
}

/**
 * @brief Take a logic block off the grid.
 * @param index Logic-block index to remove; its cells become empty.
 */
void golem_remove_logic_block(s32 index)
{
    s32 i;
    s32 unused[2]; /* never used, but without it the 8-byte stack frame is lost */

    GOLEM.logic_blocks[index].f.placed = 0;
    for (i = 0; i < GOLEM_GRID_CELL_COUNT; i++)
    {
        if (GOLEM.grid[i].owner == index)
        {
            GOLEM.grid[i].owner = GOLEM_GRID_EMPTY;
        }
    }
}

/**
 * @brief Build the grid divider markers between cells owned by different blocks.
 * @param markers Output: GOLEM_MARKER_COUNT marker words; only the horizontal
 *        edges (below each cell) are set, the vertical ones are cleared.
 */
void golem_build_grid_markers(s32* markers)
{
    s32 i;
    u8 owner;
    u8 below;

    for (i = GOLEM_MARKER_COUNT - 1; i >= 0; i--)
    {
        markers[i] = 0;
    }

    for (i = 0; i < GOLEM_GRID_CELL_COUNT - GOLEM_GRID_WIDTH; i++)
    {
        owner = GOLEM.grid[i].owner;
        if (owner != GOLEM_GRID_EMPTY)
        {
            below = GOLEM.grid[i + GOLEM_GRID_WIDTH].owner;
            if ((below != GOLEM_GRID_EMPTY) && (owner != below))
            {
                markers[GOLEM_HORIZONTAL_MARKER_BASE + i] = GOLEM_MARKER_BLOCK_EDGE;
            }
        }
    }
}
