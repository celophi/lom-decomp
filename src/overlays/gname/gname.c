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
 * This function is just like TITLE.BIN RenderFadeOverlay
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

/**
 * decomp.me (100%) https://decomp.me/scratch/EWwJI
 */
void func_8014075C(void)
{
    s16 arr[4]; // RECT
    arr[0] = 0x140;
    arr[1] = 0;
    arr[2] = 0;
    arr[3] = 0x1F2;
    func_80140794(arr);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/P3W9C
 */
void func_80140794(void* arg0)
{
    void* s0 = arg0;
    u16 new_var;
    u16* s1 = (u16*)D_80147494;
    s32 s2 = *((s32*)(((u8*)s1) + 8));
    u16 new_var2;
    u16* ptr = (u16*)(((u8*)s1) + 0x14);
    u16 arr[4];
    int counter;
    u16 tmp0 = *((u16*)(((u8*)s0) + 4));
    counter = 0;
    arr[0] = tmp0;
    new_var = *((u16*)(((u8*)s0) + 6));
    arr[2] = 0x100;
    arr[3] = 1;
    arr[1] = new_var;
    do
    {
        if ((*ptr) != 0)
        {
            *ptr |= 0x8000;
        }
        ptr++;
        counter++;
    } while (counter < 0x100);
    func_80019A34((u16*)arr, ((u8*)s1) + 0x14);
    arr[0] = *((u16*)(((u8*)s0) + 0));
    arr[1] = *((u16*)(((u8*)s0) + 2));
    {
        u16* p = (u16*)(((u8*)s1) + (s2 + 8));
        arr[2] = p[4];
        arr[3] = p[5];
        func_80019A34((u16*)arr, p + 6);
    }
    arr[0] = *((u16*)(((u8*)s0) + 4));
    new_var2 = *((u16*)(((u8*)s0) + 6));
    arr[2] = 0x100;
    arr[3] = 1;
    arr[1] = new_var2 + 1;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/yYkTM
 */
void func_80140888(s32 arg0)
{
    func_80142410();
    func_80141928(arg0);
    D_800F22AC += 1;
    func_801408D0();
}

/**
 * decomp.me (100%) https://decomp.me/scratch/g5Rx3
 */
void func_801408D0(void)
{
    s32 tmpA;

    if (D_8014F880 == 0)
    {
        func_8014139C();
    }
    else
    {
        D_8014F880--;
    }

    tmpA = D_8014F8A4;
    if (tmpA != 0)
    {
        D_8014F8A4--;
        D_8014F8A8 += (D_8014F8BC - D_8014F8A8) / tmpA;
    }
    else
    {
        D_8014F8A8 = D_8014F8BC;
    }

    if (D_80122988 == 0x800)
    {
        if ((func_80142720(D_8014F844) != 0) && (func_80142C50(D_8014F844) == 0))
        {
            func_800A3938(0x7E, 0x80);
            D_8014F7E4 = 5;
            return;
        }
        func_800A3938(0x78, 0x80);
    }
}