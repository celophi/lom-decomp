#include "common.h"

typedef struct
{
    u8 _pad15[0x15];
    u8 unk15;
    u8 _pad16[0x52 - 0x16];
    u16 unk52;
    u8 _pad54[0x5A - 0x54];
    u16 unk5A;
    u16 unk5C;
    u16 unk5E;
    u16 unk60;
    u16 unk62;
} Struct_8006429C;

typedef struct {
    u8  _pad00[0x14];
    u32 unk14;
    u32 unk18;
    u16 unk1C;
    u16 unk1E;
    u16 unk20;
    u16 unk22;
    u16 unk24;
    u16 unk26;
    u16 unk28;
} Struct_801ED000;

typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

void func_800640B4(Struct_8006429C* arg0);

/**
 * @brief Copy unk60/unk62 into unk5C/unk5E, zero unk15, store unk52 into unk5A,
 *        then call func_800640B4 with the same struct pointer.
 * @param arg0 Pointer to the target struct.
 * @see decomp.me (100%) TODO
 */
void func_8006429C(Struct_8006429C* arg0)
{
    u16 temp_v0 = (u16)arg0->unk60;
    u16 temp_v1 = (u16)arg0->unk62;
    u16 temp_a1 = arg0->unk52;

    arg0->unk15 = 0;
    arg0->unk5C = temp_v0;
    arg0->unk5E = temp_v1;
    arg0->unk5A = temp_a1;
    func_800640B4(arg0);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/OcIvj
 */
void func_800642D4(void)
{
    RECT rect;
    s32 var_a0;
    s32* var_v1;
    u32 hw_val;
    s32 mask;
    s32 limit;
    Struct_801ED000* hw_regs = (Struct_801ED000*)0x801ED000;

    cdrom_stream(0xB1, 0x801DE000);

    rect.x = 0x130;
    rect.y = 0x1FC;
    rect.w = 0x10;
    rect.h = 4;
    LoadImage(&rect, (u32*)0x801DE000);

    rect.x = 0x3C0;
    rect.y = 0x1E0;
    rect.w = 0x40;
    rect.h = 0x20;
    LoadImage(&rect, (u32*)0x801DE080);

    hw_val = 0xE100041F;
    var_a0 = 3;
    mask = -8;
    limit = -1;
    var_v1 = (s32*)0x801ED044;

    hw_regs->unk14 = hw_val;
    hw_regs->unk18 = hw_val;
    hw_regs->unk1C = 0x7F13;
    hw_regs->unk1E = 0x7FD3;
    hw_regs->unk20 = 0x7F53;
    hw_regs->unk22 = 0x7F93;
    hw_regs->unk24 = 0x7E93;
    hw_regs->unk26 = 0x7ED3;
    hw_regs->unk28 = 0;

    while (var_a0 != limit)
    {
        *var_v1 &= mask;
        var_a0 -= 1;
        var_v1 = (s32*)((u8*)var_v1 + 0x98);
    }

    DrawSync(0);
}