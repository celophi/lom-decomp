#include "common.h"

extern u8 *D_80123FC4;

/**
 * @brief Recursively search the slot chain and shift occupants toward a free entry.
 * @param arg0 Starting slot index.
 * @return 0 for an eligible special occupant, or -1 for a free or shifted entry.
 * @note The caller interprets the status while recursion performs the slot moves.
 * @note WIP: pointer reload, repeated increment and branch-tail differences remain.
 */
s32 func_800BF514(s32 arg0)
{
    s32 s0;
    s32 v0;

    if ((D_80123FC4 + arg0)[0x28] != 0xFF)
    {
        s0 = arg0 + 1;
        if (arg0 == 4)
        {
            if ((u32)(D_80123FC4[0x2C] - 0x51) < 7 && *(s32 *)(D_80123FC4 + 0x58) >= 0)
            {
                return 0;
            }
            s0 = arg0 + 1;
            if ((u32)((D_80123FC4 + arg0)[0x28] - 0x3E) < 0xE)
            {
                v0 = 0;
                if (((*(u32 *)(D_80123FC4 + 0x58) >> 0x1E) & 1) == 0)
                {
                    s0 = arg0 + 1;
                    goto block_8;
                }
                return v0;
            }
            goto block_8;
        }
    block_8:
        if (func_800BF514(s0) != 0)
        {
            (D_80123FC4 + s0)[0x28] = (D_80123FC4 + arg0)[0x28];
            (D_80123FC4 + arg0)[0x28] = 0xFF;
            return -1;
        }
        if ((u32)((D_80123FC4 + arg0)[0x28] - 0x51) < 7)
        {
            v0 = 0;
            if (*(s32 *)(D_80123FC4 + 0x58) < 0)
            {
                goto block_13;
            }
        }
        else
        {
        block_13:
            v0 = -1;
            if ((u32)((D_80123FC4 + arg0)[0x28] - 0x3E) < 0xE)
            {
                v0 = 0;
                if (((*(u32 *)(D_80123FC4 + 0x58) >> 0x1E) & 1) == 0)
                {
                    goto block_15;
                }
            }
        }
        return v0;
    }
block_15:
    v0 = -1;
    return v0;
}
