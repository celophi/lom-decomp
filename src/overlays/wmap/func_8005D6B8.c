/* Partial WMAP decompilation: 91.206894% (gcc280_g0). */
#include "common.h"
#include "saved_game.h"

extern s32 func_8005D670(u32, s32);
extern u8 *D_800D8B3C;
extern s32 D_800D00CC[][8];

/**
 * @brief Resolve a map cell's eight entries into resource values.
 * @param column Map column.
 * @param row Map row.
 * @param output Destination for eight words.
 */
void func_8005D6B8(u32 column, s32 row, s32 *output)
{
    s32 *entry;
    s32 record;
    s32 i;
    s32 index;
    s32 value;
    s32 cell;
    u8 *map_data;

    if (column < 6U && row >= 0 && row < 6)
    {
        cell = column + row * 6;
        record = func_8005D670(column, row);
        i = 0;
        map_data = D_800D8B3C;
        entry = output;
        do
        {
            if (record == 255)
            {
                index = *(map_data + (i + cell * 12) + 8) + 3;
            }
            else
            {
                index = g_saved_game.bytes[i + record * 12 + 0x2F4];
            }
            value = 0;
            if (index >= 0)
            {
                value = D_800D00CC[index][0];
            }
            *entry = value;
            i++;
            entry++;
        } while (i < 8);
    }
}
