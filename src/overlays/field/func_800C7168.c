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
 * The target initializes the tally only on the nonzero-result path and keeps a
 * separate comparison value of 1 live across the loop. Re-deriving each record
 * from its index preserves the buffer base and produces the target offsets and
 * register lifetimes exactly.
 */
void func_800C7168(void)
{
    s32 count;
    s32 i;
    s32 one = 1;
    u8 *p;

    if (g_gosub_result_count != 0)
    {
        count = 0;
        for (i = 0; i < 5; i++)
        {
            p = &g_menuLayoutBuffer[i * 0x60];
            if (p[0x2EF4] != 0 &&
                (((*(u32 *)(p + 0x2F38) >> 30) & 1) == one))
            {
                count++;
            }
        }
    }
    D_80122C16 = count;
}
