#include "common.h"

/** @brief Table of 64 world-map values copied to a local buffer. */
typedef struct
{
    s32 values[64];
} WmapValueTable;

extern u8 D_800432BD;
extern WmapValueTable D_800514F4;

/**
 * @brief Read the current table value, clamped to the range zero through nine.
 * @return Clamped table value.
 */
s32 func_8005D4A4(void)
{
    WmapValueTable table = D_800514F4;
    s32 value;
    s32 result;

    value = table.values[D_800432BD];
    if (value >= 0)
   
   {
        result = 9;
        if (value < 10)
       
       {
            result = value;
        }
    }
    else
   
   {
        result = 0;
    }
    return result;
}
