#include "common.h"

typedef struct StateB3160
{
    u8 pad0[0xC];
    u32 flags;
} StateB3160;

typedef struct RecordB3160
{
    u8 pad0[0x10];
    StateB3160 *state;
} RecordB3160;

void func_800B3160(RecordB3160 *record, u32 index);

/**
 * @brief Ticks a record's twelve state timers and clears any that expire.
 *
 * For each of the twelve half-word timers at record offset 0x50, decrements a
 * nonzero timer and, when it reaches zero or below, clears that state via
 * func_800B3160.
 *
 * WIP: 94.29%. Body matches (27/28 rows); the sole residual is the epilogue
 * delay-slot fill - the target parks `addiu sp` in the `jr` delay slot while
 * this build emits it ahead of the `jr`, a dbr/maspsx artifact seen across this
 * overlay.
 */
void func_800B4DF0(RecordB3160 *record)
{
    s32 i;

    for (i = 0; i < 12; i++)
    {
        if (*(s16 *)((u8 *)record + i * 2 + 0x50) != 0)
        {
            s16 nv = *(u16 *)((u8 *)record + i * 2 + 0x50) - 1;
            *(s16 *)((u8 *)record + i * 2 + 0x50) = nv;
            if (nv <= 0)
            {
                func_800B3160(record, i);
            }
        }
    }
}
