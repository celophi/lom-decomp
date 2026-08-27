#include "common.h"

/**
 * @brief Flag word owned by a field record.
 */
typedef struct StateB3160
{
    u8 pad0[0xC];
    u32 flags;
} StateB3160;

/**
 * @brief Field record containing a flag owner and indexed halfword states.
 */
typedef struct RecordB3160
{
    u8 pad0[0x10];
    StateB3160 *state;
} RecordB3160;

/**
 * @brief Clears one indexed record state or all twelve states.
 *
 * Indices zero through eleven clear the corresponding low flag bit and
 * halfword at record offset 0x50. Any other index clears all twelve halfwords
 * and the low sixteen bits of the flag word.
 *
 * @param record Record whose states are cleared.
 * @param index State index, or an out-of-range value to clear all states.
 */
void func_800B3160(RecordB3160 *record, u32 index)
{
    s32 count;
    u8 *cursor;

    if (index < 0xC)
    {
        record->state->flags &= ~(1 << index);
        *(u16 *)((u8 *)record + (index << 1) + 0x50) = 0;
        return;
    }
    count = 0xB;
    record->state->flags &= 0xFFFF0000;
    cursor = (u8 *)record + 0x16;
    do
    {
        *(u16 *)(cursor + 0x50) = 0;
        count--;
        cursor -= 2;
    } while (count >= 0);
}
