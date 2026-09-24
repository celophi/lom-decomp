#include "saved_game.h"
#include "common.h"
#include "field_calls.h"
#include "field_golem_layout.h"

extern s32 D_80122C00;
extern s16 D_80122C06;
extern s16 D_80122C1A;
extern s8 D_800459AF;
extern GolemShapeTable D_80051888;

void func_800C3BB0(void);
void func_800C3BD8(s32 type);
void func_800C3CB4(void);
void func_800C3D38(s32 index, s32 rotation, s32 x, s32 y);

/**
 * @brief Swap the edited golem logic group into a new order slot, or save it.
 * @param command 0x92BC moves the active group to the order slot in D_80122C00;
 *        any other value stores the edit record and clears the active group.
 */
void func_800C3A00(s32 command)
{
    s32 i;
    s32 found;
    s32 target;
    s8 active;
    GolemLayoutView* layout;
    s32 group;

    i = 0;
    if (command == 0x92BC)
    {
        target = D_80122C00;
        if ((u32)target < 3)
        {
            layout = GOLEM_LAYOUT;
            active = layout->header.fields.active_group;
            do
            {
                /* Index-first sum: group_order[i] adds the base first. */
                if (*(i + layout->group_order) == active)
                {
                    found = i;
                }
                i++;
            } while (i < 3);

            GOLEM.group_order[found] = GOLEM.group_order[target];
            GOLEM.group_order[target] = GOLEM.header.fields.active_group;
            if (GOLEM.group_order[found] == GOLEM.header.fields.active_group)
            {
                D_80122C06 = 3;
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
        for (; i < 0x15; i++)
        {
            GOLEM.group_records[GOLEM.header.fields.active_group].name[i] = GOLEM.edit_record[i];
        }
        group = GOLEM.header.fields.active_group;
        if (group != 3)
        {
            GOLEM.header.word = (GOLEM.header.word & ~0xF0) | ((group & 0xF) << 4);
        }
        GOLEM.header.fields.active_group = 3;
    }
}

/**
 * @brief Select the logic type to lay out, rebuild the grid and reload the edit record.
 * @param type Logic type to show, or 3 to restore the saved active group.
 */
void func_800C3B50(s32 type)
{
    if (type == 3)
    {
        GOLEM.header.fields.active_group = GOLEM.header.fields.saved_group >> 4;
    }
    else
    {
        D_800459AF = type;
    }

    func_800C3BB0();
    func_800C3F18(GOLEM.header.fields.active_group, GOLEM.edit_record);
}

/** @brief Rebuild the placement grid for the current logic type in D_800459AF. */
void func_800C3BB0(void)
{
    func_800C3BD8(D_800459AF);
}

/**
 * @brief Clear the placement grid and place every block of one logic type on it.
 * @param type Logic type whose placed blocks are drawn; 3 only clears the grid.
 */
void func_800C3BD8(s32 type)
{
    s32 i;
    LogicBlock block;

    for (i = 0; i < 36; i++)
    {
        GOLEM.grid[i].block_id = 0;
        GOLEM.grid[i].detail = 0;
    }

    if (type != 3)
    {
        for (i = 0; i < GOLEM.header.fields.block_count; i++)
        {
            block = GOLEM.logic_blocks[i];
            if ((block.f.unknown_bit16 == 1) && (block.f.logic_type == type))
            {
                func_800C3D38(i, block.f.rotation, block.f.grid_x, block.f.grid_y);
            }
        }
        func_800C3CB4();
    }
}

/** @brief Mark each grid cell whose lower neighbour belongs to a different block. */
void func_800C3CB4(void)
{
    s32 i;
    u8 owner;
    u8 below;
    u8 none;

    none = 99;
    for (i = 35; i >= 0; i--)
    {
        GOLEM.grid[i].edge = none;
    }

    for (i = 0; i < 30; i++)
    {
        owner = GOLEM.grid[i].owner;
        if (owner != 99)
        {
            below = GOLEM.grid[i + 6].owner;
            if ((below != 99) && (owner != below))
            {
                GOLEM.grid[i].edge = i + 6;
            }
        }
    }
}

/**
 * @brief Stamp one logic block's shape onto the placement grid.
 * @param index Logic-block index written to each covered cell.
 * @param rotation Orientation selecting a five-point row of the shape.
 * @param x Grid column of the shape origin.
 * @param y Grid row of the shape origin (six columns per row).
 */
void func_800C3D38(s32 index, s32 rotation, s32 x, s32 y)
{
    GolemShapeTable table;
    s32 offset;
    s32 i;
    s32 step;
    GolemLayoutView* layout;
    GolemLayoutView* entry;
    GolemLayoutView* grid;
    GolemShapePoint* point;
    GolemLayoutView* cell;

    /* The single-pass do/while wrappers set allocation priorities; the "- -" sum keeps the test out of CSE. */
    table = D_80051888;
    offset = index * sizeof(LogicBlock);
    layout = GOLEM_LAYOUT;
    do
    {
        i = 0;
    } while (0);

    if (table.shapes[((GolemLayoutView*)(offset - -(s32)layout))->logic_blocks[0].f.shape].count != 0)
    {
        do
        {
            do
            {
                do
                {
                    grid = layout;
                } while (0);
            } while (0);
        } while (0);

        step = rotation * 5 * sizeof(GolemShapePoint);
        /* entry and cell are layout views displaced by one element's byte offset. */
        do
        {
            entry = (GolemLayoutView*)((u8*)grid + offset);
        } while (0);

        do
        {
            point = GOLEM_SHAPE_POINT(table, entry->logic_blocks[0].f.shape, step);
            cell = (GolemLayoutView*)((x + point->x + (y + point->y) * 6) * sizeof(GolemGridCell) + (s32)grid);
            cell->grid[0].owner = index;
            cell->grid[0].block_id = entry->logic_blocks[0].f.id;
            cell->grid[0].detail = entry->logic_blocks[0].f.quantity;
            i++;
            step += sizeof(GolemShapePoint);
        } while (i < table.shapes[entry->logic_blocks[0].f.shape].count);
    }
}
