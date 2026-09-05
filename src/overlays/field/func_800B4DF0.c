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

void field_clear_record_state(RecordB3160 *record, u32 index);

/**
 * @brief Ticks a record's twelve state timers and clears any that expire.
 *
 * For each of the twelve half-word timers at record offset 0x50, decrements a
 * nonzero timer and, when it reaches zero or below, clears that state via
 * field_clear_record_state.
 *
 * 100% match with the FIELD GCC 2.8.0 G0 toolchain. The former 94.29%
 * result was caused by routing this standalone unit through GCC 2.7.2 CDK,
 * whose epilogue scheduling differs from the target.
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
                field_clear_record_state(record, i);
            }
        }
    }
}
