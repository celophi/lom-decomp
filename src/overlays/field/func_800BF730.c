#include "common.h"

extern u8 *D_80123FC4;
extern s8 D_800F0C38[];
extern u8 D_800F0E88[];

/**
 * @brief Clamp eight slot level nibbles against their selected table bounds.
 * @note Preserve the high nibble and choose the input using absolute table values.
 * @note WIP: the target retains a separate low-nibble copy; see working notes.
 */
void func_800BF730(void)
{
    s32 i;
    u8 *entry;
    u8 byte20;
    u8 byte50;
    s32 v1;
    s32 v0;
    s32 idx;
    s32 val;
    s8 *tbl1;
    u8 *table2;
    u8 lower;
    s32 upper;
    u8 result;

    for (i = 0; i < 8; i++)
    {
        tbl1 = D_800F0C38;
        entry = D_80123FC4 + i;
        byte20 = entry[0x20];
        byte50 = entry[0x50];

        val = byte20 & 0xF;

        v1 = tbl1[val];
        v0 = tbl1[byte50];

        if (v1 < 0)
        {
            v1 = -v1;
        }
        if (v0 < 0)
        {
            v0 = -v0;
        }

        v1 = (v1 < v0);
        if (v1)
        {
            val = byte50;
        }

        idx = (byte20 >> 4) * 2;
        table2 = &D_800F0E88[idx];

        result = table2[0];
        if (!(val < result))
        {
            upper = table2[1];
            result = upper;
            if (!(upper < val))
            {
                result = val;
            }
        }

        entry[0x20] = (entry[0x20] & 0xF0) | (result & 0xF);
    }
}
