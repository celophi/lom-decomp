#include "common.h"

typedef struct
{
    u8 pad0[0x16];
    s16 unk16;
    u8 pad18[0x2A - 0x18];
    s16 unk2A;
    u8 pad2C[0x2E - 0x2C];
    u16 unk2E;
    u8 pad30[0x36 - 0x30];
    u8 unk36;
    u8 pad37[0x3A - 0x37];
    u8 unk3A;
} FieldActorState;

typedef struct
{
    u8 pad0[0x2E];
    u8 unk2E;
    u8 pad2F[0x33 - 0x2F];
    u8 unk33;
    u8 pad34[0x48 - 0x34];
} FieldActorPartDef;

extern FieldActorPartDef D_800FE3A0[];

/**
 * @brief Update a scaled actor vector and submit it through the field transform helper.
 * @param arg0 Actor state to update.
 * @param arg1 Horizontal scale input.
 * @param arg2 Secondary scale input.
 * @param arg3 Depth scale input.
 */
void func_800951CC(FieldActorState *arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_lo;
    FieldActorPartDef *part;
    s32 *out;

    out = (s32 *)0x1F800000;
    if (arg0->unk2E == 0)
    {
        arg0->unk2A = 0;
        return;
    }
    temp_lo = (s8)arg0->unk36 / arg0->unk16;
    arg0->unk36 = (u8)arg0->unk36 - temp_lo;
    part = &D_800FE3A0[arg0->unk3A];
    out[0] = (temp_lo * arg1 * part->unk2E) >> 6;
    out[1] = (arg2 * part->unk33) >> 6;
    out[2] = (temp_lo * arg3 * part->unk2E) >> 6;
    func_80097FA0(arg0, out, 0);
}
