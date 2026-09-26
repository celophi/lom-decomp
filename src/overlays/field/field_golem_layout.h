#ifndef FIELD_GOLEM_LAYOUT_H
#define FIELD_GOLEM_LAYOUT_H

/**
 * @file field_golem_layout.h
 * @brief FIELD view of the golem logic-block layout kept in g_saved_game.
 *
 * The logic blocks, their placement grid and the three golem group records
 * live in the same buffer that main.h maps as PadContext. PadContext does not
 * map the header word at 0x29D4 or the grid at 0x2A7C yet, so FIELD reads the
 * region through GolemLayoutView, built from the main.h element types. Each
 * group record (LargeHistoryRecord in main.h) is read through GolemGroupRecord.
 */

#include "common.h"
#include "main.h"
#include "saved_game.h"
#include "field_records.h"

/** @brief Number of cells in the six-by-six placement grid. */
#define GOLEM_GRID_CELL_COUNT 36
/** @brief Number of columns in the placement grid. */
#define GOLEM_GRID_WIDTH 6
/** @brief Grid cell owner value for an empty cell. */
#define GOLEM_GRID_EMPTY 99
/** @brief Length of a golem name (group record and companion record). */
#define GOLEM_NAME_LENGTH 21
/** @brief joined_group value when no golem group is in the party. */
#define GOLEM_NO_GROUP 3
/** @brief Mask of the saved_group bits (4-7) in the header word. */
#define GOLEM_SAVED_GROUP_MASK 0xF0
/** @brief Shift of the saved_group bits in the header word. */
#define GOLEM_SAVED_GROUP_SHIFT 4

/** @brief Number of source items (weapons and armor) kept in a group record. */
#define GOLEM_SOURCE_ITEM_COUNT 4

/** @brief Number of logic-block shapes. */
#define GOLEM_SHAPE_COUNT 11
/** @brief Number of rotations of a logic-block shape. */
#define GOLEM_SHAPE_ROTATION_COUNT 4
/** @brief Number of parts listed per rotation after its origin point. */
#define GOLEM_SHAPE_PART_COUNT 4

/** @brief One cell of a rotated shape, relative to the block origin. */
typedef struct
{
    s8 x;
    s8 y;
    s16 glyph_id; /**< Icon glyph the GOLEM editor draws in the cell. */
} GolemShapePoint;

/** @brief One rotation of a shape: its origin point and the cells it covers. */
typedef struct
{
    GolemShapePoint origin;
    GolemShapePoint parts[GOLEM_SHAPE_PART_COUNT];
} GolemShapeRotation;

/**
 * @brief Golem logic-block shape: cell count and the four rotated layouts.
 * @note Same layout as GolemCompositeIconRow in the GOLEM overlay.
 */
typedef struct
{
    u8 count; /**< Number of parts per rotation that cover a grid cell. */
    u8 reserved;
    u8 grid_width;
    u8 grid_height;
    s16 origin_x;
    s16 origin_y;
    GolemShapeRotation rotations[GOLEM_SHAPE_ROTATION_COUNT];
} GolemShape;

/** @brief Table of the logic-block shapes. */
typedef struct
{
    GolemShape shapes[GOLEM_SHAPE_COUNT];
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
 * @brief Golem group record: the golem built from a set of weapons and armor.
 * @note Same bytes as LargeHistoryRecord (main.h), which MENU and GOSUB read.
 *       field_golem_build_companion copies it into the party companion record;
 *       the field names follow the members they fill.
 */
typedef struct
{
    u8 name[GOLEM_NAME_LENGTH];
    u8 unknown_0x15;
    u16 hp;                                           /**< LargeHistoryRecord secondary_value. */
    u16 power;                                        /**< LargeHistoryRecord primary_value; weapon power. */
    u16 equipment_totals[HISTORY_RECORD_STAT_COUNT];  /**< Summed armor values, 0-99; LargeHistoryRecord stats. */
    u8 unknown_0x22[2];
    FieldNibbles weapon_bonus;                        /**< Summed weapon bonus nibbles, 0-9. */
    FieldNibbles armor_bonus;                         /**< Summed armor bonus nibbles, 0-9. */
    FieldStat stats[FIELD_CHARACTER_STAT_COUNT];      /**< Effective value 20-99, base 0. */
    u8 armor_flags;                                   /**< OR of the armor items' flags2C. */
    u8 weapon_flags;                                  /**< OR of the weapon items' flags2C. */
    u8 armor_flags2;                                  /**< OR of the armor items' flags2D. */
    u8 unknown_0x3F;
    u32 unknown_0x40;
    u8 logic_class : 4;                               /**< Logic class a class-bound logic block must match. */
    u8 grid_bound : 4;                                /**< Usable width and height of the placement grid. */
    u8 unknown_0x45;
    u8 unknown_0x46;                                  /**< 75 - 10 * grid_bound, clamped to 0-50. */
    u8 unknown_0x47;
    s32 unknown_0x48;
    FieldItemRecord source_items[GOLEM_SOURCE_ITEM_COUNT];
} GolemGroupRecord;

/**
 * @brief Golem logic-block state inside the shared game-state buffer.
 * @note Same bytes as PadContext (main.h): companion starts at gname_name,
 *       header byte 2 is logic_block_count, header byte 3 is
 *       large_history_index, group_order is large_history_order, group_records
 *       is large_history_records.
 */
typedef struct
{
    u8 pad_0000[0xA90];
    FieldCharacterRecord companion; /**< Party slot 2 (FieldGameState characters[2]). */
    u8 pad_0CE0[0x29D4 - 0xCE0];
    union
    {
        u32 word;
        struct
        {
            u8 saved_group; /**< Bits 4-7 hold the group that last left the party. */
            u8 created_count; /**< Golems created so far, saturating at 200. */
            u8 block_count;
            s8 joined_group; /**< Golem group in the party slot, or GOLEM_NO_GROUP. */
        } fields;
    } header;
    u8 group_order[LARGE_HISTORY_RECORD_COUNT]; /**< Order slot -> logic type (group). */
    u8 pad_29DB;
    LogicBlock logic_blocks[LOGIC_BLOCK_CAPACITY];
    GolemGridCell grid[GOLEM_GRID_CELL_COUNT];
    GolemGroupRecord group_records[LARGE_HISTORY_RECORD_COUNT];
} GolemLayoutView;

/** @brief The game-state buffer viewed as the golem layout. */
#define GOLEM_LAYOUT ((GolemLayoutView*)g_saved_game.bytes)
/** @brief The golem layout as an lvalue. */
#define GOLEM (*GOLEM_LAYOUT)

#endif /* FIELD_GOLEM_LAYOUT_H */
