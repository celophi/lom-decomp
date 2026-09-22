#include "common.h"

extern u8 D_800CFDCC[];

/**
 * @brief Apply eight table adjustments and clamp each result to zero through six.
 * @param table_row Adjustment row, or 255 to skip.
 * @param output_row Destination row, or 255 to skip.
 * @param output_address Address of the destination table, with eight words per row.
 */
void func_8005DAD8(s32 table_row, s32 output_row, s32 output_address)
{
    s32 *entry;
    u8 *table;
    s32 row_offset;
    s32 adjustment;
    s32 value;
    s32 clamped;
    s32 index;

    if (output_row != 0xFF)
    {
        index = 0;
        if (table_row != 0xFF)
        {
            table = D_800CFDCC;
            row_offset = table_row * 12;
            entry = (s32 *)((output_row << 5) + output_address);
            for (; index < 8; index++, entry++)
            {
                adjustment = *(u8 *)(index + row_offset + (s32)table) - 3;
                value = *entry + adjustment;
                *entry = value;
                if (value >= 0)
                {
                    clamped = 6;
                    if (value < 7)
                    {
                        clamped = value;
                    }
                }
                else
                {
                    clamped = 0;
                }
                *entry = clamped;
            }
        }
    }
}
