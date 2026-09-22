/* Partial WMAP decompilation: 87.153850% (gcc280_g0). */
#include "common.h"
#include "saved_game.h"

extern u8 D_800D8B18[];

/** @brief Append a map cell's saved values and optional change indicators.
 * @param mode Nonzero to suppress change indicators.
 * @param x Map column.
 * @param y Map row.
 * @param mask Flag to set when the cell is occupied.
 * @param count Current output length, updated for each appended value.
 * @param flags Output flags.
 * @param output Destination values.
 * @param unused Unused argument slot.
 * @param comparison Per-cell comparison values.
 */
void func_8005CE44(s32 mode, u32 x, s32 y, s32 mask, s32 *count,
                  s32 *flags, s32 *output, s32 unused, s32 *comparison)
{
    s32 i;
    s32 difference;
    s32 marker;
    s32 *values;
    u8 cell = 255;

    if (x < 6U && y >= 0 && y < 6)
    {
        cell = D_800D8B18[x + y * 6];
    }
    if (cell != 255)
    {
        *flags |= mask;
        for (i = 0; i < 8; i++)
        {
            output[*count] = g_saved_game.bytes[0x2F4 + i + cell * 12] + 10;
            (*count)++;
        }
        if (mode == 0)
        {
            values = comparison + cell * 8;
            for (i = 0; i < 8; i++)
            {
                difference = *values - g_saved_game.bytes[0x2F4 + i + cell * 12];
                if (difference == 0)
                {
                    output[*count] = 0;
                    (*count)++;
                    output[*count] = 0;
                }
                else
                {
                    marker = 18;
                    if (difference < 0)
                    {
                        marker = 17;
                    }
                    output[*count] = marker;
                    (*count)++;
                    output[*count] = difference + 13;
                }
                (*count)++;
                values++;
            }
        }
        else
        {
            for (i = 0; i < 16; i++)
            {
                output[*count] = 0;
                (*count)++;
            }
        }
    }
}
