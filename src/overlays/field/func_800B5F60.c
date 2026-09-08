#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
    s32 unk1C;
} ArgB5F60;

typedef struct
{
    u8 pad0[0x20];
    u8 *unk20;
    u8 *unk24;
} FieldStateB5F60;

extern FieldStateB5F60 *D_80123FB0;

s32 func_800B2D34(u8 *arg0, s32 arg1);
void func_800B5948(ArgB5F60 *arg0, s32 arg1);
s32 rand(void);

/**
 * @brief Roll a random chance against the active field state's scaled threshold.
 * @param arg0 Record forwarded to the shared field-state initializer.
 * @return -1 when the state blocks the action or the roll succeeds; otherwise 0.
 */
s32 func_800B5F60(ArgB5F60 *arg0)
{
    s32 roll;
    s32 remainder;
    s32 chance;
    s32 state;

    state = D_80123FB0->unk24[0x1A];
    if (state >= 100)
    {
        return -1;
    }

    roll = rand() & 0xFFFF;
    remainder = roll % 100;
    func_800B5948(arg0, 0);
    chance = func_800B2D34(D_80123FB0->unk20, 0);
    if (remainder < (state * chance) / func_800B2D34(D_80123FB0->unk24, 4))
    {
        return -1;
    }

    return 0;
}
