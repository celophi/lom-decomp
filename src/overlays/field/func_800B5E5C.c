#include "common.h"

typedef struct
{
    u8 pad0[0xC];
    u32 unkC;
    u8 pad10[0x48 - 0x10];
    u16 unk48;
} FieldAccumulator;

typedef struct
{
    u8 pad0[4];
    u8 unk4;
    u8 pad5[0x10 - 5];
    FieldAccumulator *unk10;
} FieldAccumulatorRecord;

typedef struct
{
    u8 pad0[0x1C];
    u32 *unk1C;
    FieldAccumulatorRecord *unk20;
} FieldAccumulatorState;

extern FieldAccumulatorState *D_80123FB0;
s32 func_800B4CE4(void *, s32);

/**
 * @brief Advance and clamp a field-state accumulator when its record is eligible.
 */
void func_800B5E5C(void)
{
    FieldAccumulatorRecord *record;
    FieldAccumulator *accumulator;

    record = D_80123FB0->unk20;
    if (((u8)record->unk4 < 2U) && !(record->unk10->unkC & 0x200))
    {
        if (func_800B4CE4(record, 7) != 0)
        {
            FieldAccumulatorState *state;
            s32 index;
            FieldAccumulator *inner;

            state = D_80123FB0;
            index = ((u32)*state->unk1C >> 8) & 7;
            inner = state->unk20->unk10;
            inner->unk48 = (u16)(inner->unk48 + (8 << index));
        }
        else
        {
            FieldAccumulatorState *state;
            s32 index;
            FieldAccumulator *inner;

            state = D_80123FB0;
            index = ((u32)*state->unk1C >> 8) & 7;
            inner = state->unk20->unk10;
            inner->unk48 = (u16)(inner->unk48 + (4 << index));
        }
        accumulator = D_80123FB0->unk20->unk10;
        if ((u16)accumulator->unk48 >= 0x100U)
        {
            accumulator->unk48 = 0xFFU;
        }
    }
}
