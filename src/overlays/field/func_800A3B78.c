#include "common.h"

void akao_cmd_21(s32, s32);

/**
 * @brief Issues three AKAO command-21 voice masks for a clamped channel index.
 *
 * Clamps @p idx to a maximum of 7, then emits akao_cmd_21(0, 1 << bit) for the
 * three consecutive bits starting at idx * 3.
 */
void func_800A3B78(s32 idx)
{
    s32 bit;

    if (idx >= 8)
    {
        idx = 7;
    }
    bit = idx * 3;
    akao_cmd_21(0, 1 << bit);
    akao_cmd_21(0, 1 << (bit + 1));
    akao_cmd_21(0, 1 << (bit + 2));
}
