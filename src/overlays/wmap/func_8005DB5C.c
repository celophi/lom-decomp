#include "common.h"

#include "saved_game.h"

/**
 * @brief Add eight saved-record adjustments, each biased by minus three.
 * @param record_index Record index; 0xFF disables the update.
 * @param other_index A second index; 0xFF disables the update.
 * @param unused Unused argument retained by the calling convention.
 * @param values Eight values updated in place.
 */
void func_8005DB5C(s32 record_index, s32 other_index, s32 unused, s32 *values)
{
    s32 index;
    s32 value;
    if (other_index != 0xFF)
    {
        index = 0;
        if (record_index != 0xFF)
        {
            do
            {
                value = *values - 3;
                *values = value + g_saved_game.bytes[index + record_index * 12 + 0x2F4];
                index++;
                values++;
            } while (index < 8);
        }
    }
}
