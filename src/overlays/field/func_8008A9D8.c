#include "common.h"

typedef struct
{
    u8 pad0[0x2A];
    s16 unk2A;
    u8 pad2C[0x54 - 0x2C];
} Rec54;

typedef struct
{
    u8 pad0[4];
    s32 unk4;
    u8 pad8[0x14 - 0x8];
    s32 unk14;
    u8 pad18[0x178 - 0x18];
    s32 unk178;
    u8 pad17C[0x23C - 0x17C];
} State23C;

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
} ArgBlock;

extern Rec54 D_800FDF58[];
extern State23C D_80105AE0[];

/**
 * @brief Update an actor slot and dispatch the associated field action.
 * @param arg0 Source actor index.
 * @param arg1 Destination slot index.
 * @param arg2 Action argument.
 * @return Result from the dispatched action, or 0 when no action is performed.
 */
s32 func_8008A9D8(s32 arg0, s32 arg1, s32 arg2)
{
    State23C *base;
    State23C *slot;
    ArgBlock arg_block;

    if ((D_800FDF58[arg0].unk2A != 0x91) && (D_800FDF58[arg0].unk2A != 0x87))
    {
        base = D_80105AE0;
        slot = &base[arg1];
        slot->unk178 = slot->unk178 & ~0x80;
        if (slot->unk4 != 0)
        {
            arg_block.unk0 = base[arg0].unk14;
            arg_block.unk4 = arg2;
            arg_block.unkC = slot->unk14;
            arg_block.unk8 = 0;
            arg_block.unk10 = 0;
            arg_block.unk14 = 0;
            arg_block.unk18 = 1;
            func_8008C4A8(arg0, slot, arg2);
            return func_800B5534(&arg_block);
        }
    }
    return 0;
}
