#include "common.h"

extern u8 *D_80122B74;

/**
 * @brief Measures the packed coordinate distance for a field record.
 *
 * Compares the two coordinate nibbles in the selected 12-byte record against
 * the corresponding nibbles in the base record, sums their absolute
 * differences, and adds the selected record's byte at offset 0x2F2.
 *
 * @param arg0 Index of the 12-byte field record to measure.
 * @return The two-nibble Manhattan distance plus the record's extra byte.
 */
s32 func_800C3688(s32 arg0)
{
    s32 var_a2;
    s32 split_tmp;
    u32 temp_a0;
    u8 temp_v0;
    u8 temp_v1;
    u8 *new_var;
    u8 *base;
    u8 *temp_a1;
    s32 offset;

    base = D_80122B74;
    offset = arg0 * 3;
    offset *= 4;
    new_var = base + offset;
    temp_a1 = new_var;
    temp_v1 = temp_a1[0x2F1];
    temp_a0 = *(u32 *)(base + 0x2F0);
    split_tmp = temp_v1;
    split_tmp &= 0xF;
    split_tmp -= (temp_a0 >> 8) & 0xF;
    var_a2 = split_tmp;
    if (var_a2 < 0)
    {
        var_a2 = -var_a2;
    }
    arg0 = temp_a0 >> 12;
    arg0 = (temp_v1 >> 4) - (arg0 & 0xF);
    temp_v0 = temp_a1[0x2F2];
    if (arg0 < 0)
    {
        arg0 = -arg0;
    }
    ;
    return temp_v0 + var_a2 + arg0;
}
