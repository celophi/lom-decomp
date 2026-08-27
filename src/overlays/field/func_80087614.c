#include "common.h"

typedef struct Record87614
{
    u8 pad0[0x3A];
    u8 index;
} Record87614;

typedef struct State87614
{
    u8 pad0[0x10];
    s32 flags;
    u8 pad14[0x23C - 0x14];
} State87614;

extern State87614 D_80105AE0[];
Record87614 *func_80087C9C(s32);

/**
 * @brief Replaces the low four state flags for the selected field record.
 *
 * Resolves a field record from @p arg0, then uses its byte at offset 0x3A to
 * select a 0x23C-byte state entry. A missing record leaves the state unchanged.
 *
 * @param arg0 Selector passed to func_80087C9C.
 * @param arg1 New low-four-bit flag value.
 */
void func_80087614(s32 arg0, s32 arg1)
{
    Record87614 *record;

    record = func_80087C9C(arg0);
    if (record != (Record87614 *)-1)
    {
        D_80105AE0[record->index].flags =
            (D_80105AE0[record->index].flags & ~0xF) | arg1;
    }
}
