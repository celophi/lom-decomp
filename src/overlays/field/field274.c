#include "common.h"

/**
 * @see decomp.me (100%)
 */
s32 func_800B2D34(u8 *arg0, s32 arg1)
{
    u32 result;

    if (arg1 < 8)
    {
        arg0 += arg1;
        if (arg0[0x28] == 0)
        {
            result = 1;
        }
        else
        {
            result = arg0[0x28];
        }
        return result;
    }

    return 1;
}
