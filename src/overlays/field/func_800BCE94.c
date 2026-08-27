#include "common.h"

u8 *func_800C1E40(s32 arg0);
void func_800B2844(s32 arg0, void *arg1, s32 arg2);

/**
 * @brief Looks up a record and forwards a derived address to func_800B2844.
 *
 * Fetches the record for @p arg1 via func_800C1E40; when non-NULL, reads the
 * halfword at @c arg2*2 + 4 within it and calls func_800B2844 with @p arg0, the
 * record address offset by that halfword plus 4, and @p arg3.
 *
 * @param arg0 Forwarded to func_800B2844.
 * @param arg1 Record selector passed to func_800C1E40.
 * @param arg2 Halfword index within the record (scaled by 2).
 * @param arg3 Forwarded to func_800B2844.
 */
void func_800BCE94(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    u8 *p = func_800C1E40(arg1);

    if (p != NULL)
    {
        u16 h = *(u16 *)(p + (arg2 << 1) + 4);
        func_800B2844(arg0, p + (h + 4), arg3);
    }
}
