#include "common.h"

extern s32 D_80117E68;
extern s32 D_80117E70;
extern s32 D_80117E74;
extern s32 D_80117E80;

/**
 * @brief Build interpolation samples from the selected coefficient column.
 * @param source Two-word coefficient records.
 * @param output Destination sample array.
 */
void func_800A2128(s32 (*source)[2], s32 *output)
{
    s32 selector;
    s32 index;
    s32 *output_cursor;
    s32 value0;
    s32 value1;

    index = 0;
    if (D_80117E74 > 0)
    {
        s32 count;
        s32 offset;

        count = D_80117E74;
        output_cursor = output;
        selector = (D_80117E68 - 1) & 1;
        offset = D_80117E70;
        do
        {
            value1 = source[index + count - 1][selector];
            value0 = source[index + offset + count - 1][selector];
            index++;
            *output_cursor = value1 + value0;
            output_cursor++;
        } while (index < count);
    }

    index = 0;
    if (D_80117E70 - D_80117E80 > 0)
    {
        s32 output_offset;
        s32 source_offset;
        s32 count;

        output_offset = D_80117E74;
        source_offset = D_80117E80;
        count = D_80117E70 - D_80117E80;
        selector = (D_80117E68 - 1) & 1;
        do
        {
            s32 output_index;
            s32 source_index;

            output_index = index + output_offset;
            source_index = index + source_offset;
            index++;
            output[output_index] = source[source_index][selector];
        } while (index < count);
    }

    index = 0;
    if (D_80117E74 - 1 > 0)
    {
        s32 output_offset;
        s32 count;
        s32 source_offset;

        output_offset = D_80117E74;
        count = D_80117E74 - 1;
        source_offset = D_80117E70;
        selector = (D_80117E68 - 1) & 1;
        do
        {
            s32 source_index;

            source_index = index + source_offset;
            output[source_index - output_offset + 1] = source[source_index][selector] + source[index][selector];
            index++;
        } while (index < count);
    }
}
