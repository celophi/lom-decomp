#include "decomp5.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/CJTY6
 */
void func_800235A8(s32* arg0, s32* arg1, s32 arg2, s32 arg3)
{
    s32* t0 = arg1 + 3;
    int new_var;
    s32* v1 = arg0 + 3;
    s32 new_var2;
    char new_var3;
    do
    {
        *arg1 = (*arg0) + arg2;
        new_var2 = v1[-2];
        arg0 += 4;
        arg3 -= 1;
        t0[-2] = new_var2 + arg2;
        (new_var2 = 4);
        arg1 += new_var2;
        new_var = -1;
        t0[new_var] = v1[new_var];
        *t0 = *v1;
        v1 += 4;
        t0 += 4;
    } while (arg3 != 0);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/scY8u
 */
s32 func_800235F8(s32* arg0)
{
    return *arg0 + 0xB0BEB4BF;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/qI6jZ
 */
void func_8002360C(void)
{
    func_80024230(0);
    D_8003EC4C = 0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/oy7T9
 */
void func_80023630(void)
{
    D_8003EC4C = 1;
    func_80024230(&func_8002360C);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/D2YiT
 */
void func_80023660(s32 arg0, s32 arg1)
{
    D_8003EC4C = 1;
    func_80024230(&func_8002360C);
    func_800241A0(arg0, arg1);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/lLOqn
 */
void func_800236B0(s32 arg0, s32 arg1)
{
    func_80023630();
    func_80024140(arg0, arg1);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/fqPPO
 */
void func_800236EC(void)
{
    while ((*((volatile s32*)(&D_8003EC4C))) == 1);
}

s32 func_8002371C(void* arg0, s32 arg1)
{
    s32 ret = -1;
    UnknownStruct* ptr = (UnknownStruct*)arg0;
    if (func_800235F8() == 0)
    {
        func_8002376C(arg0, arg1, ptr->unk18, ptr->unk10);
        ret = 0;
        return ret;
    }
    return ret;
}

/**
 * decomp.me link (99.90%) https://decomp.me/scratch/Awfhy
 */
s32 func_8002376C(void* arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 new_var;
    s32 var_v0;
    SomeStruct* s;
    u8* base;

    s32 ret_val;
    func_800236EC();
    var_v0 = -1;
    if (func_800235F8(arg0) == 0)
    {
        new_var = arg0;
        s = (SomeStruct*)arg0;

        SpuSetTransferStartAddr(arg3);
        base = (u8*)arg0;
        base = base + 0x40;
        func_80023660((s32)(base + (s->unk1C * 0x10)), s->unk14);
        func_800235A8((s32*)base, (s32*)(D_8004C340 + (arg2 * 0x10)), arg3, s->unk1C);
        var_v0 = 0;
        if (arg1 != 0)
        {
            func_800236EC();
        }
        ret_val = 0;
    }
    else
    {
        D_8003EC4C = -1;
        ret_val = -1;
    }
    new_var = ret_val;
    return new_var;
}

/**
 * decomp.me link (96.93%) https://decomp.me/scratch/9R0Vj
 */
void func_80023830(void)
{
    u16* hw = (u16*)0x1F801DAA;
    u32 t0 = 0x18;
    u8** new_var4;
    u32* new_var3;
    u32* new_var2;
    u32* new_var;
    int new_var7;
    u16 new_var8;
    int new_var5;
    u8* a0 = D_8004C260;
    u8* a2 = D_80049130;
    int new_var6;
    u8* new_var9;
    u32 a3;
    new_var3 = (u32*)off(D_8003EC30, 4);
    *new_var3 = 0;
    *((u32*)off(D_8003EC30, 0)) = 0;
    *((u32*)off(D_8004D388, 0x14)) = 0;
    *((u32*)off(D_8004D388, 0x10)) = 0;
    *((u32*)off(D_8004D388, 0x0C)) = 0;
    *((u32*)off(D_8004D388, 0x08)) = 0;
    *((u32*)off(D_8004D388, 0x04)) = 0;
    *((u32*)off(D_8004D388, 0x00)) = 0;
    *((u32*)off(D_8004F750, 0x00)) = 0;
    *((u32*)off(D_8004F750, 0x04)) = 1;
    new_var = (u32*)off(D_8004F830, 0x00);
    *((u32*)off(D_8004D400, 0x00)) = 0;
    *((u32*)off(a0, 0x04)) = 0;
    *((u32*)off(a0, 0x08)) = 0;
    *((u16*)off(a0, 0x5E)) = 0;
    *((u32*)off(D_8004D400, 0x10)) = 0;
    *((u32*)off(a0, 0x1C)) = 0;
    *((u16*)off(D_8004C2D0, 0x5E)) = 0;
    *((u32*)off(D_8004C2D0, 0x04)) = 0;
    *((u32*)off(a0, 0x50)) = 0x7F0000;
    D_8003EC58 = a2;
    a2 += 0x58;
    D_8003EC5C = a0;
    D_8003EC28 = 0;
    D_8003EC24 = 0;
    D_8003EC70 = 0;
    *((u16*)off(a0, 0x58)) = 0;
    D_8003EC68 = 0x7FFF0000;
    D_8003EC40 = 0;
    D_8003EC74 = 0;
    D_8003EC42 = 0;
    D_8003EC78 = 0;
    D_8003EC64 = 0;
    *((u32*)off(D_8004D400, 0x1C)) = 0;
    *((u32*)off(a0, 0x3C)) = 0;
    *((u32*)off(D_8004D400, 0x20)) = 0;
    a3 = (new_var8 = *hw);
    *((s16*)0x1F801D80) = 0x3FFF;
    *((s16*)0x1F801D82) = 0x3FFF;
    *((s16*)0x1F801DB0) = 0x7FFF;
    *((s16*)0x1F801DB2) = 0x7FFF;
    *((u32*)off(a0, 0x40)) = 0;
    a3 = 0;
    *((u32*)off(D_8004D400, 0x24)) = a3;
    *((u32*)off(a0, 0x44)) = 0;
    *((u16*)off(a0, 0x68)) = 0;
    *((u16*)off(a0, 0x66)) = 0;
    *((u16*)off(a0, 0x64)) = a3;
    *((u16*)off(a0, 0x6C)) = a3;
    *((u32*)off(D_8004F760, 0x40)) = 0x7F00;
    *((u32*)off(D_8004F760, 0x48)) = a3;
    D_8003EC44 = a3;
    D_8003EC6C = a3;
    D_8003EC7C = a3;
    *((u32*)off(D_8004F830, 0x08)) = a3;
    *((u32*)off(D_8004F830, 0x04)) = a3;
    a3 = *hw;
    *new_var = 0;
    *hw = a3;
    *hw = (*hw) & 0xFFFA;
    *hw = (*hw) | 1;
    a3 = 0;
    hw = a2 - 0x24;
    do
    {
        a3++;
        *((u32*)hw) = 0;
        *((u32*)(a2 + 0xA4)) = t0;
        *((u16*)(a2 + 0x0C)) = a3;
        *((u32*)(a0 = a2 + 0x00)) = a3;
        a2 += 0x118;
    } while ((a3 & 0xFFFF) < 0x20);
    new_var7 = 0x7F00;
    a3 = 0xC;
    new_var5 = 0x8C;
    new_var6 = 1;

    {
        u8* v1 = D_8004B430 + new_var5;
        do
        {
            u32 tmp = a3 & 0xFFFF;
            a3++;
            *((u32*)(v1 - 0x58)) = 0;
            *((u32*)(v1 + 0x70)) = tmp;
            *((u16*)(v1 - 0x28)) = new_var6;
            *((u32*)(v1 - 0x34)) = 0;
            *((u16*)(v1 + 0x58)) = new_var7;
            *((u16*)(v1 + 0x02)) = 0;
            *((u16*)(v1 - 0x04)) = a3;
            *((u32*)((*(new_var4 = &v1)) - 0x4C)) = 0;
            *((u16*)v1) = 0;
            v1 += 0x118;
        } while ((a3 & 0xFFFF) < 0x18);
    }
    {
        u8* a0_ptr = D_8003EC5C;
        u8* v0_ptr = D_8004D400;
        u8* v1_ptr = D_8004F750;
        a0 = a0_ptr;
        *((u32*)off(a0, 0x18)) = 0;
        *((u32*)off(a0, 0x14)) = 0;
        new_var2 = (u32*)off(v0_ptr, 0x18);
        *((u32*)off(a0, 0x10)) = a3;

        *new_var2 = 1;
        *((u32*)off(v0_ptr, 0x14)) = 0x66A80000;
        *((u32*)off(v0_ptr, 0x0C)) = a3;
        *((u32*)off(v0_ptr, 0x08)) = a3;
        *((u32*)off(v0_ptr, 0x04)) = a3;
        new_var6 = 0x03FFF000;
        *((u32*)off(a0, 0x48)) = new_var6;
        *((u32*)off(a0, 0x4C)) = a3;
        *((u16*)off(a0, 0x5A)) = a3;
        new_var9 = v1_ptr;
        *((u32*)off(new_var9, 0x08)) = (*((u32*)off(new_var9, 0x08))) | 0x80;
    }
    func_80028E34(4, 0x03FFF000, a2, a3);
    func_80023EF0(1);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/0YmTg
 */
void func_80023AD0(void)
{
    s32 temp_v0;

    SpuStart();
    func_80023E90(4, &D_8004D360);
    func_80024200(0);
    SpuSetTransferStartAddr(0x1010);
    func_80023660(&D_8003D170, 0x40);
    func_800236EC();
    func_80023830();
    SpuSetIRQ(0);
    func_800240D0(0);

    do
    {
        /* wait for condition */
    } while (func_80023CA0(0xF2000002, 0x44E8, 0x1000) == 0);

    do
    {
        /* wait for condition */
    } while (func_80023D74(0xF2000002) == 0);

    do
    {
        temp_v0 = func_800167AC(0xF2000002, 2, 0x1000, func_8002A134);
        D_8003EC14 = temp_v0;
    } while (temp_v0 == -1);

    do
    {
        /* wait for completion */
    } while (func_800167DC(D_8003EC14) == 0);
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/z36q3
 */
void func_80023BB8(s32 arg0)
{
    D_8003EC48 = arg0;
    arg0 += 0x600;
    D_8003EC50 = arg0;
    arg0 += 0x300;
    D_8003EC54 = arg0;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/VenON
 */
void func_80023BE0(void)
{
    if (D_8003EC4C == 1)
    {
        func_80023660(&D_8003D170, 0x40);
        func_800236EC();
    }
    do
    {

    } while (func_80023DA4(0xF2000002) == 0);
    func_80023C90(0xF2000002, 2);
    do
    {

    } while (func_80023C80(D_8003EC14) == 0);
    do
    {

    } while (func_800167BC(D_8003EC14) == 0);
    func_8002427C(0xFFFFFF);
    func_80023E10();
}