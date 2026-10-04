#ifndef GOLEM_SHAPE_H
#define GOLEM_SHAPE_H

/**
 * @file golem_shape.h
 * @brief Golem logic-block shapes (g_golem_shape_table, resident with FIELD).
 * @note FIELD places blocks with these shapes, GOSUB draws their icons from rotation 0.
 */

#include "common.h"

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

/** @brief Shapes indexed by LogicBlock::f.shape. */
extern GolemShape g_golem_shape_table[GOLEM_SHAPE_COUNT];

#endif
