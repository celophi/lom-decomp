#include "common.h"

extern u8 g_menuLayoutBuffer[];

/**
 * @brief Calculate the quantity contribution for a pair of equipment records.
 * @param record_indices Two indices into the equipment record table.
 * @return Combined quantity divided by 17 and clamped to the range 0 through 9.
 */
s32 equipment_combination_quantity(s32* record_indices)
{
    s32 record_cursor;
    s32 total_quantity;
    s32 menu_base;
    s32 quantity_base;
    s32 record_end;
    s32 record_offset;
    s16 record_class;
    s32 scratch;
    s32 quantity_index;
    s32 result;
    s32 class_one;

    record_cursor = (s32)record_indices;
    total_quantity = 0;
    menu_base = (s32)g_menuLayoutBuffer;
    quantity_base = menu_base + 0xCE0;
    class_one = 1;
    record_end = record_cursor + 8;
    do
    {
        record_offset = *(s32*)record_cursor << 6;
        scratch = *(u32*)(record_offset + menu_base + 0xCF4);
        scratch = (u32)scratch >> 8;
        record_class = scratch & 3;
        if (record_class == 0)
        {
            total_quantity += *(u16*)(record_offset + quantity_base + 0x24);
            goto next_record;
        }
        if (record_class == class_one)
        {
            quantity_index = 0;
            do
            {
                total_quantity += *(u16*)(record_offset + quantity_base + 0x24 + quantity_index * 2);
                quantity_index += 1;
            } while (quantity_index < 4);
            record_cursor += 4;
        }
        else
        {
            scratch = 2;
            if (record_class == scratch)
            {
                total_quantity += *(u8*)(record_offset + quantity_base + 0x26);
            }
next_record:
            record_cursor += 4;
        }
    } while (record_cursor < record_end);

    total_quantity /= 17;
    if (total_quantity >= 0)
    {
        result = 9;
        if (total_quantity < 10)
        {
            result = total_quantity;
        }
    }
    else
    {
        result = 0;
    }

    return result;
}
