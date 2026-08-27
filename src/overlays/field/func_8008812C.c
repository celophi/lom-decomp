#include "common.h"

/**
 * @brief Field record fields used to locate its current command stream.
 */
typedef struct Record8812C
{
    u8 pad0[0x28];
    u8 selector;
    u8 pad29[0x2C - 0x29];
    u16 offset;
    u8 pad2E[0x3A - 0x2E];
    u8 state_index;
} Record8812C;

/**
 * @brief Field state entry containing the alternate command-stream base.
 */
typedef struct State8812C
{
    u8 pad0[0x168];
    u8 *base;
    u8 pad16C[0x23C - 0x16C];
} State8812C;

extern State8812C D_80105AE0[];
extern u16 *D_8010A02C;

/**
 * @brief Resolves the current command pointer for a field record.
 *
 * Selector 0xFE uses the record's indexed field-state base. Other selectors
 * read a halfword offset from the global command table. The record's current
 * stream offset is added in either case.
 *
 * @param record Record supplying the base selector and stream offset.
 * @return Pointer to the record's current command.
 */
u8 *func_8008812C(Record8812C *record)
{
    u8 *base;

    if (record->selector == 0xFE)
    {
        base = D_80105AE0[record->state_index].base;
    }
    else
    {
        base = (u8 *)D_8010A02C + D_8010A02C[record->selector];
    }
    return base + record->offset;
}
