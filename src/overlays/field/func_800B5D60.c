#include "common.h"

typedef struct
{
    u8 pad0[8];
    s8 unk8;
    u8 unk9;
} Counter;

typedef struct
{
    u8 pad0[8];
    s32 unk8;
} FlagRecord;

typedef struct
{
    u8 pad0[0x18];
    FlagRecord *unk18;
    u8 pad1C[4];
    Counter *unk20;
    Counter *unk24;
} FieldState;

extern FieldState *D_80123FB0;
void func_800B2B54(void *, void *, s32, s32, s32, s32);

/**
 * @brief Update the field state counters and restart expired counter effects.
 * @param arg0 Amount subtracted from the secondary counter.
 */
void func_800B5D60(s32 arg0)
{
    Counter *primary_counter;
    Counter *secondary_counter;
    Counter *reset_counter;

    primary_counter = D_80123FB0->unk20;
    if (primary_counter->unk8 <= 0)
    {
        primary_counter->unk8 = (s8)primary_counter->unk9;
        func_800B2B54(D_80123FB0->unk20, D_80123FB0->unk20, 3, 5, 0x100, 0x3C);
    }
    if (D_80123FB0->unk18->unk8 != 0)
    {
        secondary_counter = D_80123FB0->unk24;
        secondary_counter->unk8 = (u8)(secondary_counter->unk8 - arg0);
        reset_counter = D_80123FB0->unk24;
        if (reset_counter->unk8 <= 0)
        {
            reset_counter->unk8 = (s8)reset_counter->unk9;
            func_800B2B54(D_80123FB0->unk24, D_80123FB0->unk24, 3, 5, 0x100, 0xB4);
        }
    }
}
