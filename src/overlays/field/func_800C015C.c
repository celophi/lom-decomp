#include "common.h"

extern u8 *D_80123FC0;
extern u8 *D_80123FC4;

/**
 * @brief Populate the mode-1 derived fields in a field record.
 * @param record Destination record to update from the active field tables.
 */
void func_800C015C(u8 *record)
{
    s32 i;
    s32 scaled_value;
    s32 product;

    i = 0;
    do
    {
        {
            u8 *source;

            source = D_80123FC0 + (i + D_80123FC4[5] * 0xC);
            scaled_value = (source[0xC8] * (D_80123FC4 + i)[0x40]) >> 6;
        }

        *(s16 *)(record + 0x24 + i * 2) = (s16)scaled_value;
        if ((u32)scaled_value >= 0x3E8)
        {
            *(s16 *)(record + 0x24 + i * 2) = 0x3E7;
        }

        {
            u8 *source;

            source = D_80123FC0 + (i + D_80123FC4[5] * 0xC);
            product = source[0xCC] * (D_80123FC4 + i)[0x40];
        }

        {
            u8 *output;

            output = record + i;
            i += 1;
            output[0x30] = (s8)(product >> 6);
        }
    } while (i < 4);

    record[0x2C] = D_80123FC4[0x36];
    record[0x2D] = D_80123FC4[0x35];
    *(s16 *)(record + 0x2E) = D_80123FC4[0x1C];
}
