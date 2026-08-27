#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern s32 g_gosub_result_count;
extern u16 D_80122C16;

/**
 * @brief Counts active gosub-result entries whose bit 30 flag is set.
 *
 * When there are gosub results, walks the five 0x60-byte entries starting at
 * g_menuLayoutBuffer[0x2EF4]; for each entry whose leading byte is nonzero and
 * whose word at +0x44 has bit 30 set, increments the tally stored to
 * D_80122C16.
 *
 * WIP: 90.11%. Body structure matches; residuals are two coupled gcc loop
 * optimizations - the target hoists the comparison constant 1 into the loop
 * preheader (filling the entry branch delay slot) and keeps the induction
 * pointer based at the buffer (offsets 0x2EF4/0x2F38), whereas this build folds
 * the compare to a bit test and rebases the induction pointer to +0x2F38. The
 * permuter cannot reach these cleanly either.
 */
void func_800C7168(void)
{
    s32 count = 0;
    s32 i;
    u8 *p;

    if (g_gosub_result_count != 0)
    {
        p = g_menuLayoutBuffer;
        for (i = 0; i < 5; i++)
        {
            if (p[0x2EF4] != 0)
            {
                if (((*(u32 *)(p + 0x2F38) >> 30) & 1) == 1)
                {
                    count++;
                }
            }
            p += 0x60;
        }
    }
    D_80122C16 = count;
}
