#include "common.h"

extern u8 *g_pad_ctx;

/**
 * @brief Find the first free inventory record in g_pad_ctx's item table.
 * @return Pointer to the first record whose first byte is 0, or NULL if all
 *         0x64 records are occupied.
 */
u8 *func_800A9060(void)
{
    s32 count;
    u8* rec;

    rec = g_pad_ctx + 0xCE0;
    for (count = 0; count < 0x64; count++)
    {
        if (*rec == 0)
        {
            return rec;
        }
        rec += 0x40;
    }
    return NULL;
}
