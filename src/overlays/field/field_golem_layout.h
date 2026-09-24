#ifndef FIELD_GOLEM_LAYOUT_H
#define FIELD_GOLEM_LAYOUT_H

/**
 * @file field_golem_layout.h
 * @brief FIELD view of the golem logic-block layout kept in g_saved_game.
 *
 * The logic blocks, their placement grid and the three golem group records
 * live in the same buffer that main.h maps as PadContext. PadContext does not
 * map the header word at 0x29D4 or the grid at 0x2A7C yet, so FIELD reads the
 * region through GolemLayoutView, built from the main.h element types.
 */

#include "common.h"
#include "main.h"
#include "saved_game.h"

/** @brief Number of cells in the six-by-six placement grid. */
#define GOLEM_GRID_CELL_COUNT 36
/** @brief Number of columns in the placement grid. */
#define GOLEM_GRID_WIDTH 6
/** @brief Grid cell owner value for an empty cell. */
#define GOLEM_GRID_EMPTY 99
/** @brief Number of shape points per rotation row. */
#define GOLEM_SHAPE_ROTATION_POINTS 5

/** @brief LogicBlock logic_type bits (bits 0-1). */
#define LOGIC_BLOCK_TYPE_MASK 0x3
/** @brief LogicBlock id bits (bits 2-7). */
#define LOGIC_BLOCK_ID_MASK 0xFC
/** @brief LogicBlock unknown_bit16: set while the block is placed on the grid. */
#define LOGIC_BLOCK_FLAG_PLACED 0x10000
/** @brief Shift of the LogicBlock rotation bits. */
#define LOGIC_BLOCK_ROTATION_SHIFT 17
/** @brief LogicBlock rotation bits (bits 17-18). */
#define LOGIC_BLOCK_ROTATION_MASK 0x60000
/** @brief Shift of the LogicBlock grid_x bits. */
#define LOGIC_BLOCK_GRID_X_SHIFT 19
/** @brief LogicBlock grid_x bits (bits 19-23). */
#define LOGIC_BLOCK_GRID_X_MASK 0xF80000
/** @brief Shift of the LogicBlock grid_y bits. */
#define LOGIC_BLOCK_GRID_Y_SHIFT 24
/** @brief LogicBlock grid_y bits (bits 24-28). */
#define LOGIC_BLOCK_GRID_Y_MASK 0x1F000000

/** @brief One grid offset of a shape, relative to the block origin. */
typedef struct
{
    s8 x;
    s8 y;
    u8 pad[2];
} GolemShapePoint;

/** @brief Golem logic-block shape: cell count and per-rotation cell offsets. */
typedef struct
{
    u8 count;
    u8 pad[11];
    GolemShapePoint points[19];
} GolemShape;

/** @brief Table of the eleven logic-block shapes. */
typedef struct
{
    GolemShape shapes[11];
} GolemShapeTable;

/** @brief One cell of the six-by-six golem logic-block placement grid. */
typedef struct
{
    u8 block_id; /**< Id of the block covering the cell. */
    u8 detail;   /**< Detail value of that block. */
    u8 edge;     /**< Index of the cell below when it belongs to another block, else 99. */
    u8 owner;    /**< Logic-block index covering the cell, or 99 when empty. */
} GolemGridCell;

/**
 * @brief Golem logic-block state inside the shared game-state buffer.
 * @note Same bytes as PadContext (main.h): edit_record is gname_name, header
 *       byte 2 is logic_block_count, header byte 3 is large_history_index,
 *       group_order is large_history_order, group_records is
 *       large_history_records.
 */
typedef struct
{
    u8 pad_0000[0xA90];
    u8 edit_record[0x15];
    u8 pad_0AA5[0x29D4 - 0xAA5];
    union
    {
        u32 word;
        struct
        {
            u8 saved_group; /**< Bits 4-7 hold the group saved by func_800C3A00. */
            u8 pad_29D5;
            u8 block_count;
            s8 active_group; /**< Group being edited, or 3 for none. */
        } fields;
    } header;
    u8 group_order[LARGE_HISTORY_RECORD_COUNT]; /**< Order slot -> logic type (group). */
    u8 pad_29DB;
    LogicBlock logic_blocks[LOGIC_BLOCK_CAPACITY];
    GolemGridCell grid[GOLEM_GRID_CELL_COUNT];
    LargeHistoryRecord group_records[LARGE_HISTORY_RECORD_COUNT];
} GolemLayoutView;

/** @brief Byte view of the logic-block table; byte 0 of each block holds its type and id. */
typedef struct
{
    u8 pad_0000[0x29DC];
    u8 logic_block_bytes[LOGIC_BLOCK_CAPACITY][sizeof(LogicBlock)];
} GolemLogicBlockBytes;

/** @brief The game-state buffer viewed as the golem layout. */
#define GOLEM_LAYOUT ((GolemLayoutView*)g_saved_game.bytes)
/** @brief The golem layout as an lvalue. */
#define GOLEM (*GOLEM_LAYOUT)
/** @brief The game-state buffer viewed as logic-block bytes. */
#define GOLEM_BLOCK_BYTES ((GolemLogicBlockBytes*)g_saved_game.bytes)

/**
 * @brief Logic block at byte offset @p offset of the table in layout @p layout.
 * @note Summed as integers so the byte offset is added before the layout base.
 */
#define GOLEM_BLOCK_AT(layout, offset) (&((GolemLayoutView*)((offset) + (u32)(layout)))->logic_blocks[0])

/**
 * @brief Grid cell @p cell of layout @p layout.
 * @note Summed as integers so the scaled cell index is added before the layout base.
 */
#define GOLEM_GRID_CELL_AT(layout, cell) (&((GolemLayoutView*)((cell) * sizeof(GolemGridCell) + (u32)(layout)))->grid[0])

/**
 * @brief First point of a shape record displaced by a byte step into its point list.
 * @note The shape offset and step are summed as integers before the table base is added.
 */
#define GOLEM_SHAPE_POINT(table, shape, step) \
    (&((GolemShape*)((u8*)&(table) + ((step) + (shape) * sizeof(GolemShape))))->points[0])

/**
 * @brief Shape record @p shape of the shape table at @p base.
 * @note Summed as integers so the scaled index is added before the table base.
 */
#define GOLEM_SHAPE_AT(base, shape) ((GolemShape*)((shape) * sizeof(GolemShape) + (u32)(base)))

/**
 * @brief First point of shape @p shape of the table at @p base, displaced by a byte step.
 * @note Summed as integers so the step and shape offset are added before the table base.
 */
#define GOLEM_SHAPE_POINT_AT(base, shape, step) \
    (&((GolemShape*)((step) + (shape) * sizeof(GolemShape) + (u32)(base)))->points[0])

#endif /* FIELD_GOLEM_LAYOUT_H */
