#include "gname.h"

/**
 * decomp.me (100%) https://decomp.me/scratch/ld2aW
 */
void func_801403E0(void)
{
    D_8014F828.unk0 = 0;
    D_8014F828.unk4 = 0;
    D_8014F828.unk8 = 0;
    D_8014F818.unk0 = 0;
    D_8014F818.unk4 = 0;
    D_8014F818.unk8 = 0;
    D_8014F818.unkC = 0;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/hVLdu
 * This function is just like TITLE.BIN func_8005041C
 */
void func_80140410(ArgStruct* arg0)
{
    u32* var_t4 = (u32*)arg0->unk4040; // t4
    ArgStruct* arg = arg0;             // t6 = t7 (copied after loads)
    s32 temp_a2, temp_a0, temp_v1;
    s32 var_a1;

    if (D_8014F818.unkC != 0)
    {
        temp_a2 = (D_8014F818.unk0 - D_8014F828.unk0) / D_8014F818.unkC;
        temp_a0 = (D_8014F818.unk4 - D_8014F828.unk4) / D_8014F818.unkC;
        temp_v1 = (D_8014F818.unk8 - D_8014F828.unk8) / D_8014F818.unkC;
        D_8014F818.unkC--;
        D_8014F828.unk0 += temp_a2;
        D_8014F828.unk4 += temp_a0;
        D_8014F828.unk8 += temp_v1;
    }
    else
    {
        D_8014F828.unk0 = D_8014F818.unk0;
        D_8014F828.unk4 = D_8014F818.unk4;
        D_8014F828.unk8 = D_8014F818.unk8;
    }

    if (!((D_8014F828.unk0 == 0x100) && (D_8014F828.unk4 == 0x100) && (D_8014F828.unk8 == 0x100)))
    {
        if (D_8014F828.unk0 >= 0x101)
        {
            ((u8*)var_t4)[4] = (u8)D_8014F828.unk0 - 1;
            ((u8*)var_t4)[5] = (u8)D_8014F828.unk4 - 1;
            ((u8*)var_t4)[6] = (u8)D_8014F828.unk8 - 1;
        }
        else
        {
            if (D_8014F828.unk0 == 0x100)
                ((u8*)var_t4)[4] = 0;
            else
                ((u8*)var_t4)[4] = ~(u8)D_8014F828.unk0;

            if (D_8014F828.unk4 == 0x100)
                ((u8*)var_t4)[5] = 0;
            else
                ((u8*)var_t4)[5] = ~(u8)D_8014F828.unk4;

            if (D_8014F828.unk8 == 0x100)
                ((u8*)var_t4)[6] = 0;
            else
                ((u8*)var_t4)[6] = ~(u8)D_8014F828.unk8;
        }

        ((u8*)var_t4)[3] = 3;
        ((u8*)var_t4)[7] = 0x62;
        *((u16*)((u8*)var_t4 + 12)) = 0x140;
        *((u16*)((u8*)var_t4 + 10)) = 0;
        *((u16*)((u8*)var_t4 + 8)) = 0;
        *((u16*)((u8*)var_t4 + 14)) = 0xF0;

        *var_t4 = (*var_t4 & 0xFF000000) | (arg->unk0 & 0xFFFFFF);
        arg->unk0 = (arg->unk0 & 0xFF000000) | ((u32)var_t4 & 0xFFFFFF);

        var_a1 = 0x25;
        var_t4 = (u32*)((u8*)var_t4 + 0x10);
        if (D_8014F828.unk0 < 0x101)
            var_a1 = 0x45;

        ((u8*)var_t4)[3] = 1;
        *((u32*)((u8*)var_t4 + 4)) = var_a1 | 0xE1000000;

        *var_t4 = (*var_t4 & 0xFF000000) | (arg->unk0 & 0xFFFFFF);
        arg->unk0 = (arg->unk0 & 0xFF000000) | ((u32)var_t4 & 0xFFFFFF);
        var_t4 = (u32*)((u8*)var_t4 + 8);
    }

    arg0->unk4040 = var_t4;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/jq3uD
 */
void func_801406F8(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    D_8014F818.unk0 = arg0;
    D_8014F818.unk4 = arg1;
    D_8014F818.unk8 = arg2;
    D_8014F818.unkC = arg3;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/pnzC1
 */
void func_80140714(void)
{
    volatile int dummy[2]; // forces 0x20 stack frame, ra at 0x18(sp)
    func_8014075C();
    func_800AA02C();
    D_8014F880 = 0x28;
    func_8006441C();
    func_801409EC();
    func_80063194();
}