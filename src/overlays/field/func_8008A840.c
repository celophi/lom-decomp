#include "common.h"

typedef struct
{
    u8 pad0[0x2A];
    s16 unk2A;
    u8 pad2C[0x30 - 0x2C];
    u16 unk30;
    u8 pad32[0x54 - 0x32];
} Rec54;

typedef struct
{
    u8 pad0[4];
    s32 unk4;
    u8 pad8[0x14 - 8];
    s32 unk14;
    u8 pad18[0x16F - 0x18];
    u8 unk16F;
    u8 pad170[0x178 - 0x170];
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

void func_8008C4A8(s32 arg0);
s32 func_800B5534(ArgBlock *arg0);

/**
 * @brief Configure an interaction request between two field-state slots.
 * @param arg0 Source slot index.
 * @param arg1 Destination slot index.
 * @return Result returned by func_800B5534, or zero when the request cannot be started.
 */
s32 func_8008A840(s32 arg0, s32 arg1)
{
    ArgBlock arg_block;
    State23C *slot;
    State23C *other;
    State23C *base;
    s32 mask;

    if ((D_800FDF58[arg0].unk2A == 0x91) || (D_800FDF58[arg0].unk2A == 0x87))
    {
        return 0;
    }
    base = D_80105AE0;
    other = &base[arg1];
    other->unk178 = other->unk178 & ~0x80;
    if (other->unk4 == 0)
    {
        return 0;
    }
    slot = &base[arg0];
    arg_block.unk0 = slot->unk14;
    if ((u8)slot->unk16F < 0xB)
    {
        arg_block.unk4 = (s32)slot->unk16F;
    }
    else
    {
        arg_block.unk4 = 0;
    }
    arg_block.unkC = D_80105AE0[arg1].unk14;
    if (arg0 < 2)
    {
        arg_block.unk8 = (s32)D_800FDF58[arg0].unk30;
    }
    else
    {
        arg_block.unk8 = 0;
    }
    arg_block.unk10 = 0;
    arg_block.unk14 = 0;
    mask = ((u32)D_80105AE0[arg0].unk178 >> 2) & 7;
    if (mask == 0)
    {
        mask = 1;
    }
    arg_block.unk18 = mask;
    func_8008C4A8(arg0);
    return func_800B5534(&arg_block);
}
