#include "common.h"

extern u8 *g_pad_ctx;

/**
 * @brief Compact the active field-object table, then clear the tail.
 *
 * Walks the 0x64 fixed-size (0x40-byte) records starting at @c g_pad_ctx +
 * 0xCE0: each live record is moved down to the next free slot (via bcopy) and
 * its old copy zeroed, closing gaps. Any slots left between the compacted end
 * and @c g_pad_ctx + 0x25E0 are then zeroed.
 *
 * @see decomp.me (100%) TODO
 */
void func_800A8FB4(void)
{
    s32 var_s2;
    u8 *var_s0;
    u8 *var_s1;

    var_s2 = 0;
    var_s0 = g_pad_ctx + 0xCE0;
    var_s1 = var_s0;
    do
    {
        if (*var_s1 != 0)
        {
            if (var_s1 != var_s0)
            {
                bcopy(var_s1, var_s0, 0x40);
                *var_s1 = 0;
            }
            var_s0 += 0x40;
        }
        var_s2 += 1;
        var_s1 += 0x40;
    } while (var_s2 < 0x64);
    while ((u32)var_s0 < (u32)(g_pad_ctx + 0x25E0))
    {
        *var_s0 = 0;
        var_s0 += 0x40;
    }
}
