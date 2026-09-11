#include "common.h"

/** @brief Partial FieldActorState layout used by func_8008C620. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u32 unkC;
    s16 unk10;
    s16 unk12;
    s16 unk14;
    s16 unk16;
    u8 unk18;
    u8 unk19;
    u8 unk1A;
    u8 pad1B[1];
    s32 unk1C;
    u8 unk20;
    u8 unk21;
    u8 unk22;
    u8 unk23;
    u8 unk24;
    u8 unk25;
    u8 pad26[1];
    u8 unk27;
    u8 unk28;
    u8 pad29[1];
    s16 unk2A;
    s16 unk2C;
    u16 unk2E;
    s16 unk30;
    u8 unk32;
    u8 unk33;
    u8 unk34;
    u8 unk35;
    u8 unk36;
    u8 unk37;
    u8 unk38;
    u8 pad39[1];
    u8 unk3A;
    u8 unk3B;
    u32 unk3C;
    s32 unk40;
    u32 unk44;
    u32 unk48;
    u32 unk4C;
    u8 pad50[4];
} FieldActorState;

/** @brief Partial ActorSlotData layout used by func_8008C620. */
typedef struct
{
    u8 pad0[0xC];
    s32 unkC;
    u8 pad10[0x170 - 0x10];
    u8 unk170;
    u8 pad171[0x174 - 0x171];
    u32 unk174;
    u32 unk178;
    u8 pad17C[0x18D - 0x17C];
    u8 unk18D;
    u8 pad18E[0x23C - 0x18E];
} ActorSlotData;

void func_8006C5FC(FieldActorState *rec);

extern ActorSlotData D_80105AE0[];

/**
 * @brief Reset actor control fields and slot flags, then refresh its resources.
 * @param rec Actor state to reset.
 * @see decomp.me (100%) TODO
 */
void func_8008C620(FieldActorState *rec)
{
    D_80105AE0[rec->unk3A].unk18D = 0;
    D_80105AE0[rec->unk3A].unkC &= ~0x8000;
    D_80105AE0[rec->unk3A].unkC &= ~0x4000;
    rec->unk2A = 0xB7;
    rec->unk20 = 0x1E;
    rec->unk2E = 1;
    rec->unk30 = 0;
    rec->unk4 = 0;
    rec->unk27 = 0;
    rec->unk21 = (rec->unk21 & 0x80) + 0x13;
    rec->unk24 = 1;
    D_80105AE0[rec->unk3A].unk174 &= ~0x1800;
    func_8006C5FC(rec);
}
