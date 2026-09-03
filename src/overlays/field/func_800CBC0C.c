#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern s32 D_80122C00;
extern u8 D_800F1CD0[];

s32 func_800CBC0C(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_a1;
    s32 temp_v1;
    s32 cell_offset;
    s32 part_offset;
    s32 valid;
    u32 limit;
    s32 saved_arg0;
    s32 in_y;
    u8 *temp_v0;
    u8 *menu;
    u8 *table;

    menu = g_menuLayoutBuffer;
    limit = menu[menu[D_80122C00 + 0x29D8] * 0x14C + 0x2B50] >> 4;
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
    if (table[((((((*(u32 *)(cell_offset + (s32)menu + 0x29DC) >> 12) & 0xF) << valid)
                  + ((*(u32 *)(cell_offset + (s32)menu + 0x29DC) >> 12) & 0xF)) * 4
                 - ((*(u32 *)(cell_offset + (s32)menu + 0x29DC) >> 12) & 0xF)) * 8)] != 0)
    {
        u8 *grid;
        grid = menu;
        do { part_offset = arg1 * 0x14; } while (0);
        do
        {
            temp_v0 = (u8 *)(part_offset + ((((u32)*(u32 *)(cell_offset + (s32)grid + 0x29DC) >> 12) & 0xF) * 0x58) + (s32)table);
            temp_v1 = *(s8 *)(temp_v0 + 0xC) + arg2;
            temp_a1 = *(s8 *)(temp_v0 + 0xD) + arg3;
            if ((temp_v1 < 0) || (temp_v1 >= (s32)limit))
            {
                valid = 0;
            }
            do { do { in_y = temp_a1 < (s32)limit; } while (0); } while (0);
            if ((temp_a1 < 0) || (cell_offset = saved_arg0 * 4, !in_y))
            {
                valid = 0;
                cell_offset = saved_arg0 * 4;
            }
            part_offset += 4;
        } while (++arg0 < (s32)table[(((*(u32 *)(cell_offset + (s32)grid + 0x29DC) >> 12) & 0xF) * 0x58)]);
    }
    return valid;
}
