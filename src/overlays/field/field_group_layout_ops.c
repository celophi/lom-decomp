/**
 * @file field_group_layout_ops.c
 * @brief Golem group bookkeeping and the logic-block placement grid.
 *
 * The golem companion is built from one of three saved groups (the
 * LargeHistoryRecord entries). These functions reorder the groups, save the
 * party golem back into its group when it leaves, and rebuild the six-by-six
 * grid that records which logic block covers each cell.
 */

#include "saved_game.h"
#include "common.h"
#include "field_calls.h"
#include "field_golem_layout.h"

/** @brief field_golem_commit_group_edit command: move the joined group to the order slot in D_80122C00. */
#define GOLEM_COMMAND_REORDER 0x92BC
/** @brief Number of cells whose lower neighbour is still on the grid. */
#define GOLEM_GRID_UPPER_CELL_COUNT (GOLEM_GRID_CELL_COUNT - GOLEM_GRID_WIDTH)
/** @brief D_80122C06 value when the joined group kept its order slot. */
#define GOLEM_SLOT_STATUS_UNCHANGED 3

extern s32 D_80122C00;
extern s16 D_80122C06;
extern s16 D_80122C1A;
extern s8 D_800459AF;
extern GolemShapeTable D_80051888;

static void field_golem_rebuild_grid(s32 type);
static void field_golem_mark_grid_edges(void);
static void field_golem_stamp_block_shape(s32 index, s32 rotation, s32 x, s32 y);

/**
 * @brief Move the joined golem group to a new order slot, or save it when it leaves.
 * @param command GOLEM_COMMAND_REORDER swaps the joined group into the order
 *        slot in D_80122C00; any other value copies the companion name back
 *        into the group record and marks no group as joined.
 */
void field_golem_commit_group_edit(s32 command)
{
    s32 i;
    s32 found;
    s32 target;
    s8 joined;
    GolemLayoutView* layout;
    s32 group;

    i = 0;
    if (command == GOLEM_COMMAND_REORDER)
    {
        target = D_80122C00;
        if ((u32)target < LARGE_HISTORY_RECORD_COUNT)
        {
            layout = GOLEM_LAYOUT;
            joined = layout->header.fields.joined_group;
            do
            {
                /* Written index first: the address sum adds i before the base. */
                if (*(i + layout->group_order) == joined)
                {
                    found = i;
                }
                i++;
            } while (i < LARGE_HISTORY_RECORD_COUNT);

            GOLEM.group_order[found] = GOLEM.group_order[target];
            GOLEM.group_order[target] = GOLEM.header.fields.joined_group;
            if (GOLEM.group_order[found] == GOLEM.header.fields.joined_group)
            {
                D_80122C06 = GOLEM_SLOT_STATUS_UNCHANGED;
            }
            else
            {
                D_80122C06 = GOLEM.group_order[found];
            }
            D_80122C1A = D_800459AF;
        }
    }
    else
    {
        for (; i < GOLEM_NAME_LENGTH; i++)
        {
            GOLEM.group_records[GOLEM.header.fields.joined_group].name[i] = GOLEM.companion.name[i];
        }
        group = GOLEM.header.fields.joined_group;
        if (group != GOLEM_NO_GROUP)
        {
            GOLEM.header.word = (GOLEM.header.word & ~GOLEM_SAVED_GROUP_MASK) | ((group & 0xF) << GOLEM_SAVED_GROUP_SHIFT);
        }
        GOLEM.header.fields.joined_group = GOLEM_NO_GROUP;
    }
}

/**
 * @brief Show one logic type on the grid and rebuild the golem companion record.
 * @param type Logic type to show, or GOLEM_NO_GROUP to rejoin the saved group.
 */
void field_golem_select_logic_type(s32 type)
{
    if (type == GOLEM_NO_GROUP)
    {
        GOLEM.header.fields.joined_group = GOLEM.header.fields.saved_group >> GOLEM_SAVED_GROUP_SHIFT;
    }
    else
    {
        D_800459AF = type;
    }

    field_golem_rebuild_current_grid();
    field_golem_build_companion(GOLEM.header.fields.joined_group, &GOLEM.companion);
}

/** @brief Rebuild the placement grid for the logic type in D_800459AF. */
void field_golem_rebuild_current_grid(void)
{
    field_golem_rebuild_grid(D_800459AF);
}

/**
 * @brief Clear the placement grid and place every block of one logic type on it.
 * @param type Logic type whose placed blocks are stamped; GOLEM_NO_GROUP only clears the grid.
 */
static void field_golem_rebuild_grid(s32 type)
{
    s32 i;
    LogicBlock block;

    for (i = 0; i < GOLEM_GRID_CELL_COUNT; i++)
    {
        GOLEM.grid[i].block_id = 0;
        GOLEM.grid[i].detail = 0;
    }

    if (type != GOLEM_NO_GROUP)
    {
        for (i = 0; i < GOLEM.header.fields.block_count; i++)
        {
            block = GOLEM.logic_blocks[i];
            if ((block.f.placed == 1) && (block.f.logic_type == type))
            {
                field_golem_stamp_block_shape(i, block.f.rotation, block.f.grid_x, block.f.grid_y);
            }
        }
        field_golem_mark_grid_edges();
    }
}

/** @brief Mark each grid cell whose lower neighbour belongs to a different block. */
static void field_golem_mark_grid_edges(void)
{
    s32 i;
    u8 owner;
    u8 below;
    u8 empty;

    /* A local, not the constant: loading it before i is set is what the code does. */
    empty = GOLEM_GRID_EMPTY;
    for (i = GOLEM_GRID_CELL_COUNT - 1; i >= 0; i--)
    {
        GOLEM.grid[i].edge = empty;
    }

    for (i = 0; i < GOLEM_GRID_UPPER_CELL_COUNT; i++)
    {
        owner = GOLEM.grid[i].owner;
        if (owner != GOLEM_GRID_EMPTY)
        {
            below = GOLEM.grid[i + GOLEM_GRID_WIDTH].owner;
            if ((below != GOLEM_GRID_EMPTY) && (owner != below))
            {
                GOLEM.grid[i].edge = i + GOLEM_GRID_WIDTH;
            }
        }
    }
}

/**
 * @brief Stamp one logic block's shape onto the placement grid.
 * @param index Logic-block index written to each covered cell.
 * @param rotation Rotation of the shape to place.
 * @param x Grid column of the shape origin.
 * @param y Grid row of the shape origin.
 */
static void field_golem_stamp_block_shape(s32 index, s32 rotation, s32 x, s32 y)
{
    GolemShapeTable table;
    s32 i;
    s32 cell;

    table = D_80051888;
    for (i = 0; i < table.shapes[GOLEM.logic_blocks[index].f.shape].count; i++)
    {
        cell = x + table.shapes[GOLEM.logic_blocks[index].f.shape].rotations[rotation].parts[i].x +
               (y + table.shapes[GOLEM.logic_blocks[index].f.shape].rotations[rotation].parts[i].y) * GOLEM_GRID_WIDTH;
        GOLEM.grid[cell].owner = index;
        GOLEM.grid[cell].block_id = GOLEM.logic_blocks[index].f.id;
        GOLEM.grid[cell].detail = GOLEM.logic_blocks[index].f.quantity;
    }
}
