#include "common.h"

/** @brief Partial actor state used by the scaled movement update. */
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

/** @brief Actor-part scaling bytes in the 0x48-byte definition table. */
typedef struct
{
    u8 pad0[0x2E];
    u8 unk2E;
    u8 pad2F[0x33 - 0x2F];
    u8 unk33;
    u8 pad34[0x48 - 0x34];
} FieldActorPartDef;

extern FieldActorPartDef D_800FE3A0[];

extern void func_8008EBA4(FieldActorState *, s32, s32);
extern s32 func_80097FA0(FieldActorState *, s32 *, s32);

/**
 * @brief Update actor movement and clear selected states when the transform fails.
 * @param arg0 Actor state to update.
 * @param arg1 Horizontal scale input.
 * @param arg2 Vertical scale input.
 * @param arg3 Depth scale input.
 * @return Unspecified value; callers do not consume the result.
 */
s32 func_80094508(FieldActorState *arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_lo;
    FieldActorPartDef *part;
    s32 *out;
    s16 state;

    out = (s32 *)0x1F800000;
    if (arg0->unk2E == 0)
    {
        arg0->unk2A = 0;
    }
    else
    {
        func_8008EBA4(arg0, arg1, arg3);
        temp_lo = (s8)arg0->unk36 / arg0->unk16;
        arg0->unk36 = (u8)arg0->unk36 - temp_lo;
        part = &D_800FE3A0[arg0->unk3A];
        out[0] = (temp_lo * arg1 * part->unk2E) >> 6;
        out[1] = (arg2 * part->unk33) >> 6;
        out[2] = (temp_lo * arg3 * part->unk2E) >> 6;
        if (func_80097FA0(arg0, out, 0) == 0)
        {
            state = arg0->unk2A;
            if (state == 0x8B || state == 0xAC || state == 0x8C || state == 0xB0 || state == 0xB1)
            {
                arg0->unk2A = 0;
            }
        }
    }
}
