#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern s32 D_80122C00;
extern u8 D_800F1CD0[];

/**
 * @brief Check whether all parts of a selected composite layout fit within the active menu grid.
 * @param arg0 Packed menu-record index used to select the layout variant.
 * @param arg1 Part-group index within the selected layout.
 * @param arg2 Horizontal offset added to each part.
 * @param arg3 Vertical offset added to each part.
 * @return 1 when every examined part is within the clamped grid bound, otherwise 0.
 */
s32 func_800CBC0C(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_a1;
    s32 temp_v1;
    s32 cell_offset;
    s32 valid;
    u32 limit;
    s32 saved_arg0;
    u8 *temp_v0;
    s32 cursor;
    u8 *table;

    cursor = (s32)g_menuLayoutBuffer;
    limit = ((u8 *)cursor)[((u8 *)cursor)[D_80122C00 + 0x29D8] * 0x14C + 0x2B50] & 0xF0;
    limit >>= 4;
    saved_arg0 = arg0;
    if ((s32)limit >= 6)
    {
        limit = 6;
        valid = 1;
    }
    else
    {
        valid = 1;
    }
    cell_offset = saved_arg0 * 4;
    table = D_800F1CD0;
    arg0 = 0;
    if (table[((((((*(u32 *)(cell_offset + cursor + 0x29DC) >> 12) & 0xF) << valid)
                  + ((*(u32 *)(cell_offset + cursor + 0x29DC) >> 12) & 0xF)) * 4
                 - ((*(u32 *)(cell_offset + cursor + 0x29DC) >> 12) & 0xF)) * 8)] != 0)
    {
        u8 *grid;
        grid = (u8 *)cursor;
        cursor = arg1 * 0x14;
        do
        {
            temp_v0 = (u8 *)(cursor + ((((u32)*(u32 *)(cell_offset + (s32)grid + 0x29DC) >> 12) & 0xF) * 0x58) + (s32)table);
            temp_v1 = *(s8 *)(temp_v0 + 0xC) + arg2;
            temp_a1 = *(s8 *)(temp_v0 + 0xD) + arg3;
            if ((temp_v1 < 0) || (temp_v1 >= (s32)limit))
            {
                valid = 0;
            }
            if ((temp_a1 < 0) || (cell_offset = saved_arg0 * 4, temp_a1 >= (s32)limit))
            {
                valid = 0;
                cell_offset = saved_arg0 * 4;
            }
            cursor += 4;
        } while (++arg0 < (s32)table[(((*(u32 *)(cell_offset + (s32)grid + 0x29DC) >> 12) & 0xF) * 0x58)]);
    }
    return valid;
}
