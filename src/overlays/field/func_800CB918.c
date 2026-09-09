#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern s32 D_80122C00;
extern u8 D_800F1CD0[];

/**
 * @brief Update a composite layout record and mark each occupied part in the menu grid.
 * @param arg0 Layout record index to update.
 * @param arg1 Part-group index within the selected layout.
 * @param arg2 Horizontal grid offset applied to each part.
 * @param arg3 Vertical grid offset applied to each part.
 */
void func_800CB918(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 cell_offset;
    s32 count;
    s32 base;
    s32 cursor;
    s32 x;
    s32 y;
    u32 packed;
    u32 mask1;
    u32 mask2;
    u32 mask3;
    s32 layout_index;
    u8 *table;
    u8 *part;

    do
    {
        cell_offset = arg0 * 4;
    } while (0);
    mask1 = 0xFFF9FFFF;
    mask2 = 0xFF07FFFF;
    mask3 = 0xE0FFFFFF;
    base = (s32)g_menuLayoutBuffer;
    layout_index = D_80122C00;
    packed = (((((((*(u32 *)(cell_offset + base + 0x29DC) & ~3)
                        | (((u8 *)base)[layout_index + 0x29D8] & 3) | 0x10000)
                       & mask1)
                      | ((arg1 & 3) << 17))
                     & mask2)
                    | ((arg2 & 0x1F) << 19))
                   & mask3)
                  | ((arg3 & 0x1F) << 24);
    table = D_800F1CD0;
    *(u32 *)(cell_offset + base + 0x29DC) = packed;
    count = 0;
    if (table[((packed >> 12) & 0xF) * 0x58] != 0)
    {
        u8 *loop_table;
        u8 *grid;
        u32 shape;
        u32 limit_shape;

        do
        {
            loop_table = table;
        } while (0);
        if (cell_offset != 0)
        {
            grid = (u8 *)base;
        }
        else
        {
            grid = g_menuLayoutBuffer;
        }
        cursor = arg1 * 0x14;
        do
        {
            shape = *(u32 *)(cell_offset + (s32)grid + 0x29DC);
            shape >>= 12;
            shape &= 0xF;
            part = (u8 *)(cursor + shape * 0x58 + (s32)loop_table);
            x = arg2 + *(s8 *)(part + 0xC);
            y = arg3 + *(s8 *)(part + 0xD);
            x += y * 6;
            grid[x * 4 + 0x2A7F] = arg0;
            limit_shape = *(u32 *)(cell_offset + (s32)grid + 0x29DC);
            do
            {
                count++;
            } while (0);
            limit_shape >>= 12;
            limit_shape &= 0xF;
            cursor += 4;
        } while (count < loop_table[limit_shape * 0x58]);
    }
}
