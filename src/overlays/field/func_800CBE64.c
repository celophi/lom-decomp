#include "common.h"

extern u8 g_menuLayoutBuffer[];

/**
 * @brief Clears a record's logic-block flag and reactivates matching entries.
 *
 * Clears bit 0x10000 of the packed 32-bit logic-block word at
 * @c g_menuLayoutBuffer[arg0*4 + 0x29DC], then scans all 0x24 records (stride
 * 4) and sets the byte flag at +0x2A7F to 0x63 for every record whose flag
 * currently equals @p arg0.
 *
 * @param arg0 Record index whose logic-block bit is cleared and whose value is
 *             matched against each record's +0x2A7F flag.
 */
void func_800CBE64(s32 arg0)
{
    s32 i;
    u8 *p;
    u8 *base = g_menuLayoutBuffer;
    u8 *rec = base + arg0 * 4;
    /* Reserves the target's unused 8-byte stack frame slot (FRAME-03). */
    volatile s32 pad;

    *(u32 *)(rec + 0x29DC) &= ~0x10000;

    for (i = 0; i < 0x24; i++)
    {
        p = base + i * 4;
        if (p[0x2A7F] == arg0)
        {
            p[0x2A7F] = 0x63;
        }
    }
}
