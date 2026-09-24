#include "saved_game.h"
#include "common.h"
#include "field_golem_layout.h"

/*
 * Golem logic-block placement: appending blocks, stamping their shapes on the
 * six-by-six grid, and validating a placement against the grid bound and the
 * edited group's logic class. The GOLEM overlay calls these functions.
 */

/** @brief Placement status of one logic block, filled by func_800CBD70. */
typedef struct
{
    u16 is_unavailable; /**< Non-zero when the block cannot join the edited group. */
    u16 clut;           /**< CLUT row used to draw the block's icon. */
} GolemLogicBlockStatus;

/**
 * @brief Shape record @p shape of the table at @p base, scaled through a shift by @p one.
 * @note @p one is a variable that holds 1; the shift by it and the integer sum
 *       (index before base) reproduce the original multiply by sizeof(GolemShape).
 */
#define GOLEM_SHAPE_SCALED_AT(base, shape, one) \
    ((GolemShape*)(((((shape) << (one)) + (shape)) * 4 - (shape)) * 8 + (u32)(base)))

extern GolemShapeTable D_800F1CD0;
extern s32 D_80122C00;
extern s32 D_800F2098[];
extern u8 D_800459AE;
extern u16 D_800F2180[];

/**
 * @brief Append a new, unassigned logic block to the golem logic-block table.
 * @param block_id Six-bit block id.
 * @param detail Four-bit detail (quantity) value.
 * @param shape Four-bit shape index into D_800F1CD0.
 */
void golem_logic_block_append(u32 block_id, u32 detail, u32 shape)
{
    GOLEM.logic_blocks[GOLEM.header.fields.block_count].f.id = block_id;
    GOLEM.logic_blocks[GOLEM.header.fields.block_count].f.quantity = detail;
    GOLEM.logic_blocks[GOLEM.header.fields.block_count].f.shape = shape;
    GOLEM.logic_blocks[GOLEM.header.fields.block_count].f.logic_type = LOGIC_BLOCK_UNASSIGNED;
    GOLEM.logic_blocks[GOLEM.header.fields.block_count].f.unknown_bit16 = 0;
    GOLEM.header.fields.block_count++;
}

/**
 * @brief Rebuild the grid owners from the placed blocks of the edited group.
 * @return Grid bound of the edited group (high nibble of its record byte 0x44).
 * @note Empty cells get 99; every cell a placed block covers gets that block's index.
 */
u32 func_800CB758(void)
{
    s32 empty;
    s32 i;
    s32 part;
    s32 count;
    s32 x;
    s32 y;
    LogicBlock block;
    LogicBlock placed;
    GolemShapeTable* shapes;
    GolemShapePoint* point;

    empty = GOLEM_GRID_EMPTY;
    for (i = GOLEM_GRID_CELL_COUNT - 1; i >= 0; i--)
    {
        GOLEM.grid[i].owner = empty;
    }

    count = GOLEM.header.fields.block_count;
    for (i = 0; i < count; i++)
    {
        block = GOLEM.logic_blocks[i];
        shapes = &D_800F1CD0;
        if (block.f.unknown_bit16 == 1 && block.f.logic_type == GOLEM.group_order[D_80122C00])
        {
            for (part = 0; part < GOLEM_SHAPE_AT(shapes, GOLEM.logic_blocks[i].f.shape)->count; part++)
            {
                placed = GOLEM.logic_blocks[i];
                point = GOLEM_SHAPE_POINT_AT(shapes, placed.f.shape,
                                             (placed.f.rotation * GOLEM_SHAPE_ROTATION_POINTS + part) * sizeof(GolemShapePoint));
                x = placed.f.grid_x + point->x;
                y = placed.f.grid_y + point->y;
                GOLEM.grid[x + y * GOLEM_GRID_WIDTH].owner = i;
            }
        }
    }
    return GOLEM.group_records[GOLEM.group_order[D_80122C00]].unknown_0x44 >> 4;
}

/**
 * @brief Place a logic block in the edited group and mark its cells on the grid.
 * @param index Logic-block index to place.
 * @param rotation Placed rotation, 0-3.
 * @param x Grid column of the shape origin.
 * @param y Grid row of the shape origin.
 */
void func_800CB918(s32 index, s32 rotation, s32 x, s32 y)
{
    s32 step;
    s32 part;
    s32 offset;
    s32 slot;
    u32 rotation_mask;
    u32 x_mask;
    u32 y_mask;
    s32 cell;
    s32 row;
    LogicBlock block;
    LogicBlock next;
    GolemShapeTable* shapes;
    GolemShapeTable* loop_shapes;
    GolemShapePoint* point;
    GolemLayoutView* layout;
    GolemLayoutView* grid;

    /* The single-pass do/while wrappers and the equal-branch if set allocation priorities. */
    do
    {
        offset = index * sizeof(LogicBlock);
    } while (0);
    rotation_mask = ~LOGIC_BLOCK_ROTATION_MASK;
    x_mask = ~LOGIC_BLOCK_GRID_X_MASK;
    y_mask = ~LOGIC_BLOCK_GRID_Y_MASK;
    layout = GOLEM_LAYOUT;
    slot = D_80122C00;
    block.word = (((((((GOLEM_BLOCK_AT(layout, offset)->word & ~LOGIC_BLOCK_TYPE_MASK) |
                      (layout->group_order[slot] & LOGIC_BLOCK_TYPE_MASK) | LOGIC_BLOCK_FLAG_PLACED) &
                     rotation_mask) |
                    ((rotation & 3) << LOGIC_BLOCK_ROTATION_SHIFT)) &
                   x_mask) |
                  ((x & 0x1F) << LOGIC_BLOCK_GRID_X_SHIFT)) &
                 y_mask) |
                ((y & 0x1F) << LOGIC_BLOCK_GRID_Y_SHIFT);
    shapes = &D_800F1CD0;
    *GOLEM_BLOCK_AT(layout, offset) = block;
    part = 0;
    if (GOLEM_SHAPE_AT(shapes, block.f.shape)->count != 0)
    {
        do
        {
            loop_shapes = shapes;
        } while (0);
        if (offset != 0)
        {
            grid = layout;
        }
        else
        {
            grid = GOLEM_LAYOUT;
        }
        step = rotation * GOLEM_SHAPE_ROTATION_POINTS * sizeof(GolemShapePoint);
        do
        {
            point = GOLEM_SHAPE_POINT_AT(loop_shapes, GOLEM_BLOCK_AT(grid, offset)->f.shape, step);
            cell = x + point->x;
            row = y + point->y;
            cell += row * GOLEM_GRID_WIDTH;
            GOLEM_GRID_CELL_AT(grid, cell)->owner = index;
            next = *GOLEM_BLOCK_AT(grid, offset);
            do
            {
                part++;
            } while (0);
            step += sizeof(GolemShapePoint);
        } while (part < GOLEM_SHAPE_AT(loop_shapes, next.f.shape)->count);
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
s32 func_800CBA9C(s32 index, s32 rotation, s32 x, s32 y)
{
    s32 row;
    s32 cell;
    s32 offset;
    s32 valid;
    u8* point;
    s32 step;
    s32 part;
    s32 shape_offset;
    s32 shape;
    s32 logic_class;

    valid = 1;
    offset = index * sizeof(LogicBlock);
    part = 0;
    if (D_800F1CD0.shapes[GOLEM.logic_blocks[index].f.shape].count != 0)
    {
        u8* shapes;
        GolemLayoutView* layout;

        shapes = (u8*)&D_800F1CD0;
        layout = GOLEM_LAYOUT;
        step = rotation * GOLEM_SHAPE_ROTATION_POINTS * sizeof(GolemShapePoint);
        /* A label loop, so loop.c cannot hoist the block reload. The net-zero pairs and
         * the shared point temp set allocation priorities. */
    next_part:
        part++;
        part--;
        part++;
        part--;
        offset++;
        offset--;
        offset++;
        offset--;
        offset++;
        offset--;
        layout++;
        layout--;
        shapes++;
        shapes--;
        step++;
        step--;
        shape = GOLEM_BLOCK_AT(layout, offset)->f.shape;
        point = (u8*)(shape * 11);
        shape_offset = (s32)point * 8;
        point = (u8*)(step + shape_offset + (s32)shapes);
        row = y + ((GolemShape*)point)->points[0].y;
        cell = x + ((GolemShape*)point)->points[0].x;
        cell += row * GOLEM_GRID_WIDTH;
        if (GOLEM_GRID_CELL_AT(layout, cell)->owner != GOLEM_GRID_EMPTY)
        {
            valid = 0;
        }
        step += sizeof(GolemShapePoint);
        if (++part < ((GolemShape*)(shape_offset + (s32)shapes))->count)
        {
            goto next_part;
        }
    }

    logic_class = D_800F2098[GOLEM.logic_blocks[index].f.id];
    if (logic_class != 0)
    {
        if (logic_class != (GOLEM.group_records[GOLEM.group_order[D_80122C00]].unknown_0x44 & 0xF))
        {
            valid = 0;
        }
    }
    return valid;
}

/**
 * @brief Check that every cell of a block lies inside the edited group's grid bound.
 * @param index Logic-block index to test; reused as the part counter.
 * @param rotation Rotation to test, 0-3.
 * @param x Horizontal offset added to each part.
 * @param y Vertical offset added to each part.
 * @return 1 when every part is within the bound (clamped to 6), otherwise 0.
 */
s32 func_800CBC0C(s32 index, s32 rotation, s32 x, s32 y)
{
    s32 cell_x;
    s32 cell_y;
    s32 offset;
    s32 valid;
    u32 limit;
    s32 block_index;
    GolemShapePoint* point;
    s32 cursor;
    GolemShapeTable* shapes;

    /* index and cursor are shared temps and valid is set in both branches: allocation levers. */
    cursor = (s32)GOLEM_LAYOUT;
    limit = ((GolemLayoutView*)cursor)->group_records[((GolemLayoutView*)cursor)->group_order[D_80122C00]].unknown_0x44 & 0xF0;
    limit >>= 4;
    block_index = index;
    if ((s32)limit >= GOLEM_GRID_WIDTH)
    {
        limit = GOLEM_GRID_WIDTH;
        valid = 1;
    }
    else
    {
        valid = 1;
    }
    offset = block_index * sizeof(LogicBlock);
    shapes = &D_800F1CD0;
    index = 0;
    if (GOLEM_SHAPE_SCALED_AT(shapes, GOLEM_BLOCK_AT(cursor, offset)->f.shape, valid)->count != 0)
    {
        GolemLayoutView* grid;

        grid = (GolemLayoutView*)cursor;
        cursor = rotation * GOLEM_SHAPE_ROTATION_POINTS * sizeof(GolemShapePoint);
        do
        {
            point = GOLEM_SHAPE_POINT_AT(shapes, GOLEM_BLOCK_AT(grid, offset)->f.shape, cursor);
            cell_x = point->x + x;
            cell_y = point->y + y;
            if ((cell_x < 0) || (cell_x >= (s32)limit))
            {
                valid = 0;
            }
            if ((cell_y < 0) || (cell_y >= (s32)limit))
            {
                valid = 0;
            }
            offset = block_index * sizeof(LogicBlock);
            cursor += sizeof(GolemShapePoint);
        } while (++index < GOLEM_SHAPE_AT(shapes, GOLEM_BLOCK_AT(grid, offset)->f.shape)->count);
    }
    return valid;
}

/**
 * @brief Fill the placement status of every logic block for the edited group.
 * @param results Output status per logic block.
 * @return The global status byte D_800459AE.
 * @note The CLUT table is read at byte offset id * 2 while its entries are four bytes apart.
 */
u8 func_800CBD70(GolemLogicBlockStatus* results)
{
    s32 i;
    LogicBlock block;
    u8 group;
    s32 logic_class;

    for (i = 0; i < GOLEM.header.fields.block_count; i++)
    {
        block = GOLEM.logic_blocks[i];
        group = GOLEM.group_order[D_80122C00];
        /* u16 pointer stores: struct-member stores would let D_80122C00 be hoisted. */
        if (((block.f.logic_type != group) && (block.f.logic_type != LOGIC_BLOCK_UNASSIGNED)) ||
            (((logic_class = D_800F2098[block.f.id]) != 0) && ((GOLEM.group_records[group].unknown_0x44 & 0xF) != logic_class)))
        {
            *(u16*)&results[i] = 1;
            *((u16*)&results[i] + 1) = 0xF;
        }
        else
        {
            *(u16*)&results[i] = 0;
            *((u16*)&results[i] + 1) = D_800F2180[(GOLEM_BLOCK_BYTES->logic_block_bytes[i][0] & LOGIC_BLOCK_ID_MASK) >> 1];
        }
    }
    return D_800459AE;
}

/**
 * @brief Take a logic block off the grid.
 * @param index Logic-block index to remove; its cells become empty.
 */
void func_800CBE64(s32 index)
{
    s32 i;
    s32 unused[2];

    GOLEM.logic_blocks[index].f.unknown_bit16 = 0;
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
 * @param markers Output: 60 marker words; entry 30 + i is set for cell i when the
 *        cell below belongs to another block.
 */
void func_800CBEC4(s32* markers)
{
    s32 i;
    u8 owner;
    u8 below;

    for (i = 59; i >= 0; i--)
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
                markers[i + 30] = 0x4F;
            }
        }
    }
}
