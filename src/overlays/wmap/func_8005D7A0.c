#include "common.h"

extern u8 D_800CFDCC[];
extern s32 D_800D00CC[][8];

/**
 * @brief Resolve eight table entries into the output buffer.
 * @param table_index Table row, or -1 to use entry three for every output.
 * @param output Destination for eight words.
 */
void func_8005D7A0(s32 table_index, s32 *output)
{
    s32 *entry;
    s32 i;
    s32 value;
    s32 index;

    entry = output;
    i = 0;
    do
    {
        index = 3;
        if (table_index != -1)
    {
            index = D_800CFDCC[i + table_index * 12];
        }
        value = 0;
        if ((s32) index >= 0)
    {
            value = D_800D00CC[index][0];
        }
        *entry = value;
        i += 1;
        entry += 1;
    } while (i < 8);
}
