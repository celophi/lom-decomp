#include "common.h"

/** @brief Record with a leading value and a 40-byte stride. */
typedef struct
{
    s32 value;
    u8 unknown_4[36];
} WmapValueRecord;

extern WmapValueRecord D_80139290[][6];

/**
 * @brief Find a value in the six-by-six world-map record table.
 * @param value Value to find.
 * @param row_out Receives the first matching row.
 * @param column_out Receives the first matching column.
 * @return One if found, otherwise zero; outputs are unchanged on failure.
 */
s32 func_8006D0F0(s32 value, s32 *row_out, s32 *column_out)
{
    s32 row;
    s32 column;
    for (column = 0; column < 6; column++)
    {
        for (row = 0; row < 6; row++)
        {
            if (D_80139290[row][column].value == value)
            {
                *row_out = row;
                *column_out = column;
                return 1;
            }
        }
    }
    return 0;
}
