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

/**
 * decomp.me (100%) https://decomp.me/scratch/FboaU
 */
void func_801409EC(void)
{
    D_8014F888 = 0xFF;
    D_8014F8AC = func_80140AB8(0, 0);
    D_8014F884 = 0;
    D_8014F8B4 = 0;
    D_8014F8C0 = 0;
    D_8014F8C4 = 0;
    D_8014F8D0 = 0;
    D_8014F850 = 0;
    D_8014F88C = D_8014F894;
    D_8014F890 = D_8014F89C;
    func_801428A4(D_8014F844, &D_8014F7E8); /* matches 'la a1, D_8014F7E8' */
    D_8014F8A8 = 0;
    func_80142928();
    D_8014F8A4 = 5;
    D_8014F8B0 = 0;
    D_8014F8B8 = 2;
    D_8014F848 = 0;
}

/**
 * decomp.me (85.93%) https://decomp.me/scratch/JHyuJ
 * WARNING. MIGHT NOT BE FUNCTIONALLY EQUIVALENT YET.
 */
s32 func_80140AB8(s32 arg0, s32 arg1)
{
    /* s-register mapped locals */
    s32 repeat;         /* s0: loop condition, init 0xFF */
    s32 reg_s1;         /* s1: arg0 */
    s32 reg_s2;         /* s2: arg1 */
    u32 reg_s6;         /* s6: (u32)(&D_80142F04) - 0x10 */
    const s32 five = 5; /* s7 */
    const s32 four = 4; /* s8 */

    /* Other locals */
    u8* new_var14;
    const s32 new_var5;
    s32 var_a0;
    void* var_a1;
    s32 temp_a0;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v1;
    u32 new_var;
    s32 temp_v1_2;
    int new_var10;
    s32 temp_v1_3;
    s32 temp_v1_5;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    int new_var8;
    s32 var_v0_4;
    s32 var_v0_5;
    int new_var3;
    s32 var_v0_6;
    s32 var_v0_7;
    u8* new_var9;
    const s32 new_var2;
    s32 var_v0_8;
    u8* new_var13;
    s32 var_v1;
    void* temp_v1_4;
    u8* f00;
    u8* ef8;
    u32 f00_addr;
    u32 base_f04_addr;
    u8* new_var11;
    int new_var7;

    /* Prologue: initialize register-mapped locals */
    new_var14 = &D_8014F7B0;
    new_var11 = &D_8014F7E8;
    reg_s1 = arg0;
    reg_s2 = arg1;
    repeat = 0xFF;
    reg_s6 = (u32)(&D_80142F04) - 0x10;

    while (repeat == 0xFF)
    {
        if (reg_s1 < 0)
            goto block_62;
        if (reg_s1 < 4)
            goto s1_lt4_block;
        if (reg_s1 >= 8)
            goto block_62;
        else
            goto s14to8_block;
    s1_lt4_block:
        if (reg_s2 & 0x220)
        {
            D_8014F888 = reg_s1;
            if (reg_s1 != 1)
            {
                if (reg_s1 < 2)
                {
                    if (reg_s1 != 0)
                    {
                        repeat = 0;
                        continue;
                    }
                }
                else if (reg_s1 == 2)
                {
                    goto s1_eq2_code;
                }
                else if (reg_s1 == 3)
                {
                    goto s1_eq3_code;
                }
                else
                {
                    repeat = 0;
                    continue;
                }

                if ((func_80142720(D_8014F844) != 0) && (func_80142C50(D_8014F844) == 0))
                {
                    func_800A3938(0x7E, 0x80);
                    D_8014F7E4 = five;
                }
                else
                {
                    func_800A3938(0x78, 0x80);
                }

                repeat = 0;
                continue;
            }
            else
            {
                func_800A3938(0x7E, 0x80);
                func_80142844(D_8014F844);
                goto block_38;
            }

        s1_eq2_code:
            func_800A3938(0x7E, 0x80);
            if (D_8014F7E0 == four)
            {
                u16 half4;
                D_8014F850 = 0;
                base_f04_addr = func_80016F5C();
                var_v0_3 = base_f04_addr;
                temp_v1_2 = var_v0_3;
                if (temp_v1_2 < 0)
                {
                    var_v0_3 = temp_v1_2 + 0x7F;
                }
                {
                    u32 rem4 = temp_v1_2 - ((var_v0_3 >> 7) << 7);
                    half4 = *((u16*)((D_80142F04 + (rem4 * 2)) + reg_s6));
                    var_a1 = (void*)((D_80142F04 + half4) + reg_s6);
                }
                goto block_37;
            }
            else if (D_8014F7E0 == five)
            {
                u16 half5;
                u32 addr_f04;
                D_8014F850 = 0;
                var_v0_4 = func_80016F5C();
                temp_v1_3 = var_v0_4;
                if (temp_v1_3 < 0)
                {
                    var_v0_4 = temp_v1_3 + 0x7F;
                }
                {
                    u32 rem5 = temp_v1_3 - ((var_v0_4 >> 7) << 7);
                    addr_f04 = (u32)(&D_80142F04);
                    half5 = *((u16*)((((u32)D_80142F04) + (addr_f04 + (rem5 * 2))) + 0xF0));
                    var_a1 = (void*)((D_80142F04 + half5) + reg_s6);
                }
                goto block_37;
            }
            else if (D_8014F7E0 == 3)
            {
                u32 idx3;
                unsigned short half3a;
                u16 half3b;
                u32 rem3;
                D_8014F850 = 0;
                idx3 = D_8014F83C;
                new_var8 = idx3;
                if (new_var8 >= 0x81)
                {
                    var_a1 = &D_8014F7E8;
                    goto block_37;
                }
                new_var3 = new_var8 * 2;
                f00 = D_80142F00;
                f00_addr = (u32)(&D_80142F00);
                half3a = *((u16*)((f00 + new_var3) + reg_s6));
                func_801428A4(D_8014F844, (void*)((f00 + half3a) + reg_s6));
                temp_v0 = func_80016F5C();
                var_v0_5 = temp_v0 >> 7;
                f00_addr = (u32)(&D_80142F00);
                if (temp_v0 < 0)
                {
                    var_v0_5 = ((s32)((((((temp_v0 & 0xFFFFFFFFFFFFFFFF) & 0xFFFFFFFFFFFFFFFF) & 0xFFFFFFFFFFFFFFFF) &
                                        0xFFFFFFFFFFFFFFFF) &
                                       0xFFFFFFFFFFFFFFFF) +
                                      0x7F)) >>
                               7;
                }
                rem3 = temp_v0 - (var_v0_5 << 7);
                var_v0_3 = rem3;
                f00 = D_80142F00;
                half3b = *((u16*)(((f00_addr + (var_v0_3 * 2)) + ((u32)f00)) + 0xF4));
                func_80142764(D_8014F844, (void*)((f00 + half3b) + reg_s6), (s32)(new_var13 = f00));
                goto block_38;
            }
            else if (D_8014F7E0 == 1)
            {
                var_a1 = new_var14;
                goto block_36;
            }
        s1_eq3_code:
            func_800A3938(0x7E, 0x80);
            var_a1 = new_var11;
        block_36:
            D_8014F850 = 0;
        block_37:
            func_801428A4(D_8014F844, var_a1);
        block_38:
            func_80142928();
            repeat = 0;
            D_8014F8A4 = five;
            repeat = 0;
            continue;
        }
        else
        {
            if (reg_s2 == 0)
                goto block_61;
            if (reg_s2 & 0x4000)
                goto block_51;
            if (reg_s2 & 0x8000)
            {
                var_v0 = 3;
                if (reg_s1 != 0)
                    var_v0 = reg_s1 - 1;
                goto block_55;
            }
            if (!(reg_s2 & 0x2000))
                goto block_61;
            var_v1 = 0;
            var_v0_2 = reg_s1 < 3;
            goto block_58;
        }
        goto end_if_s1_ge0;
    s14to8_block:
        if ((reg_s2 & 0x220) && ((D_8014F888 = reg_s1, temp_v1 = reg_s1 - 4, D_8014F848 != temp_v1)))
        {
            reg_s1 = 0x10;
            reg_s2 = 0;
            D_8014F8C0 = 0;
            D_8014F8B4 = 0;
            D_8014F848 = temp_v1;
            D_8014F8C4 = 0;
            D_8014F8D0 = 0;
            func_800A3938(0x7E, 0x80);
            continue;
        }
        else
        {
            if (reg_s2 != 0)
            {
                if (reg_s2 & 0x2000)
                {
                block_51:
                    reg_s1 = 0x10;
                    reg_s2 = 0;
                    continue;
                }
                if (reg_s2 & 0x1000)
                {
                    reg_s1 = (reg_s1 == four) ? 6 : reg_s1 - 1;
                    goto block_61;
                }
                if (reg_s2 & 0x4000)
                {
                    var_v1 = 4;
                    var_v0_2 = reg_s1 < 6;
                    goto block_58;
                }
            }
            goto block_61;
        }
        goto end_if_s1_ge0;
    block_55:
        reg_s1 = var_v0;
        goto block_61;
    block_58:
        if (var_v0_2 != 0)
            var_v1 = reg_s1 + 1;
        reg_s1 = var_v1;
    block_61:
        func_800A3938(0x7D, 0x80);
        temp_v1_4 = (void*)(((reg_s1 + 2) * 4) + ((u32)(&D_80142E0C)));
        repeat = 0;
        D_8014F894 = ((*((u32*)temp_v1_4)) & 0x1FF) - 8;
        new_var8 = (s32)(*((u8*)((u32)temp_v1_4 + 2)));
        D_8014F884 = five;
        D_8014F89C = new_var8;
        repeat = 0;
        continue;
    end_if_s1_ge0:;
    block_62:
        if ((reg_s2 & 0x220) && (((D_8014F8A0 * 0xA) + D_8014F898) >= D_8014F8D0))
        {
            if (D_8014F848 < 3)
            {
                if (func_80142720(D_8014F844) < 0xA)
                {
                    u32 t1;
                    u16 hw;
                    D_8014F8B8 = 2;
                    {
                        u32 idx_lt3 = D_8014F8D0;
                        ef8 = D_80142EF8;
                        t1 = *((u32*)((D_8014F848 * 4) + ((u32)(&D_80142C98))));
                        hw = *((u16*)(((ef8 + (t1 * 2)) + (idx_lt3 * 2)) + reg_s6));
                        D_8014F8B0 = 0;
                        func_80142764(D_8014F844, (void*)((D_80142EF8 + hw) + reg_s6), (s32)ef8);
                    }
                    func_80142928();
                    D_8014F8A4 = five;
                    func_800A3938(0x7D, 0x80);
                    repeat = 0;
                    continue;
                }
                else
                {
                    goto block_73;
                }
            }
            else if (D_8014F848 == 3)
            {
                u16 off;
                if ((*((u32*)((D_8014F8D0 * 4) + ((u32)(&D_80142CAC))))) != 0xFF)
                {
                    var_a0 = 0x7E;
                    D_8014F8C8 = D_8014F8D0;
                    D_8014F8C0 = 0;
                    D_8014F8B4 = 0;
                    D_8014F8C4 = 0;
                    D_8014F848 = four;
                    D_8014F894 = 0x54;
                    D_8014F89C = 0x68;
                    D_8014F884 = four;
                    temp_v1_5 = D_8014F8D0 * 2;
                    D_8014F8D0 = 0;
                    {
                        ef8 = D_80142EF8;
                        off = *((u16*)(((ef8 + (D_80142CA4 * 2)) + temp_v1_5) + reg_s6));
                        D_8014F84C = (void*)((ef8 + off) + reg_s6);
                    }
                    repeat = 0;
                    func_800A3938(var_a0, 0x80);
                }
                else
                {
                    repeat = 0;
                    continue;
                }
            }
            else if (D_8014F848 == 4)
            {
                if (func_80142720(D_8014F844) < 0xA)
                {
                    u32 t1;
                    u32 t2;
                    u16 hw;
                    u8* efc;
                    new_var9 = D_80142EFC;
                    D_8014F8B8 = 2;
                    D_8014F8B0 = 0;
                    {
                        u32 idx4 = D_8014F8C8;
                        efc = new_var9;
                        t1 = *((u32*)((idx4 * 4) + ((u32)(&D_80142CAC))));
                        t2 = *((u32*)((t1 * 4) + ((u32)(&D_80142E40))));
                        hw = *((u16*)(((efc + (t2 * 2)) + (D_8014F8D0 * 2)) + reg_s6));
                        func_80142764(D_8014F844, (void*)((efc + hw) + reg_s6), (s32)efc);
                    }
                    func_80142928();
                    D_8014F8A4 = five;
                    var_a0 = 0x7D;
                }
                else
                {
                block_73:
                    var_a0 = 0x78;
                }
                repeat = 0;
                func_800A3938(var_a0, 0x80);
            }
            else
            {
                repeat = 0;
                continue;
            }
        }
        else
        {
            if (reg_s2 != 0)
            {
                var_v0_6 = reg_s2 & 0x8000;
                if ((reg_s2 & 0x1000) && ((D_8014F8D0 / 10) == (((s32)D_8014F8D0) >> 0x1F)))
                {
                    reg_s1 = 0;
                    reg_s2 = 0;
                    continue;
                }
                var_v0_7 = reg_s2 & 0x1000;
                if (var_v0_6 != 0 && D_8014F8D0 == ((D_8014F8D0 / 10) * 0xA))
                {
                    reg_s1 = 4;
                    reg_s2 = 0;
                    continue;
                }
                if (var_v0_7 != 0 && (D_8014F8D0 / 10) != (((s32)D_8014F8D0) >> 0x1F))
                {
                    D_8014F8D0 = D_8014F8D0 - 0xA;
                }
                else if ((reg_s2 & 0x4000) && (D_8014F8D0 / 10) != D_8014F8A0)
                {
                    D_8014F8D0 = D_8014F8D0 + 0xA;
                }
                else if ((reg_s2 & 0x8000) && D_8014F8D0 != (D_8014F8D0 / 10) * 0xA)
                {
                    D_8014F8D0 = D_8014F8D0 - 1;
                }
                else if ((reg_s2 & 0x2000) && ((D_8014F8D0 / 10) * 0xA) != (D_8014F8D0 - 9))
                {
                    D_8014F8D0 = D_8014F8D0 + 1;
                }
                else
                {
                    repeat = 0;
                    continue;
                }
            }

            func_800A3938(0x7D, 0x80);

            D_8014F894 = ((D_8014F8D0 % 10) * 0x10) + 0x54;
            new_var10 = 4 * (4 * (D_8014F8D0 / 10));
            temp_a0 = new_var10;
            new_var7 = D_8014F8B4 - 0x68;
            temp_v0_2 = temp_a0 - new_var7;
            D_8014F89C = temp_v0_2;
            if (temp_v0_2 < 0x68)
            {
                D_8014F89C = 0x68;
                D_8014F8C0 = temp_a0;
                D_8014F8C4 = four;
            }
            if (D_8014F89C >= 0xA9)
            {
                D_8014F89C = 0xA8;
                D_8014F8C0 = temp_a0 - 0x40;
                D_8014F8C4 = four;
            }
            D_8014F884 = four;
            repeat = 0;
        }
    }
    return reg_s1;
}

/**
 * decomp.me (96.22%) https://decomp.me/scratch/ctu1w
 */
void func_8014139C(void)
{
    s8 sp10;
    s8 sp11;
    s8 sp12;
    s32 var_a0;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 temp_a1;
    s32 temp_a1_2;
    s32 temp_s1;
    s32 temp_v0_3;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 var_v0;
    s8 temp_v0;
    u16 temp_v0_2;
    u8* base;
    u32 idx;
    u32 offset;
    u16 tmp;
    s32 original;
    s32 temp_c98;
    u8* ptr;
    u16 val;
    temp_a1 = D_80122988 & 0xF220;
    D_8014F888 = 0xFF;
    if (temp_a1 != 0)
    {
        D_8014F8AC = func_80140AB8(D_8014F8AC, temp_a1);
    }
    else if (D_80122988 & 1)
    {
        temp_s1 = func_80142844(D_8014F844);
        while (func_80142720(&D_8014F850) >= 0xB)
        {
            func_80142844(&D_8014F850);
        }

        func_801429A0(&D_8014F850, temp_s1 & 0xFFFF);
        func_80142928();
        D_8014F8A4 = 5;
        var_a0 = 0x7D;
        func_800A3938(var_a0, 0x80);
    }
    else if (D_80122988 & 2)
    {
        if (func_80142720(D_8014F844) < 0xA)
        {
            temp_v0 = func_80142A54(&D_8014F850);
            temp_v0_2 = (u16)temp_v0;
            if (temp_v0_2 != 0)
            {
                sp10 = temp_v0;
                (&sp10)[1] = (s8)(temp_v0_2 >> 8);
                (&sp10)[2] = 0;
                func_80142764(D_8014F844, &sp10);
                func_80142928();
                D_8014F8A4 = 5;
            }
            var_a0 = 0x7D;
            func_800A3938(var_a0, 0x80);
        }
        else
        {
            func_800A3938(0x78, 0x80);
        }
    }
    else if (D_80122988 & 0x40)
    {
        if (D_8014F838 != 0)
        {
            if (func_80142720(D_8014F844) == 0)
            {
                D_8014F7E4 = 2;
                func_800A3938(0x7F, 0x80);
                return;
            }
        }
        func_800A3938(0x7F, 0x80);
        func_80142844(D_8014F844);
        func_80142928();
        D_8014F8A4 = 5;
    }
    if (((D_8014F8AC == 0x10) && (D_8014F848 == 4)) && (D_80122988 & 0xC))
    {
        func_800A3938(0x7D, 0x80);
        if (D_80122988 & 0xC)
        {
            do
            {
                if (D_80122988 & 4)
                {
                    temp_a0 = D_8014F8C8;
                    temp_v1 = temp_a0 - 0xA;
                    D_8014F8C8 = temp_v1;
                    if (temp_v1 == (-1))
                    {
                        D_8014F8C8 = 0;
                    }
                    else if (temp_v1 < 0)
                    {
                        D_8014F8C8 = temp_a0 + 0x29;
                    }
                }
                else
                {
                    original = D_8014F8C8;
                    temp_v1_2 = original + 10;
                    D_8014F8C8 = temp_v1_2;
                    if (temp_v1_2 == 0x32)
                    {
                        D_8014F8C8 = 9;
                    }
                    else if (temp_v1_2 >= 0x32)
                    {
                        D_8014F8C8 = original - 0x29;
                    }
                }
                if (D_80142CAC[D_8014F8C8] != 0xFF)
                {
                    D_8014F8C0 = 0;
                    D_8014F8B4 = 0;
                    var_v0 = D_80142C98[3];
                    D_8014F8C4 = 0;
                    D_8014F8D0 = 0;
                    D_8014F894 = 0x54;
                    D_8014F89C = 0x68;
                    D_8014F884 = 4;
                    temp_c98 = var_v0;
                    base = D_80142EF4;
                    idx = D_8014F8C8;
                    offset = (idx * 2) + ((temp_c98 * 2) + D_80142EF8);
                    tmp = *((u16*)(base + offset));
                    D_80122988 &= ~0xC;
                    D_8014F84C = (void*)((D_80142EF8 + tmp) + ((unsigned long)base));
                }
            } while (D_80122988 & 0xC);
        }
    }
    if (D_8014F884 != 0)
    {
        temp_a1_2 = ((s32)(D_8014F894 - D_8014F88C)) / ((s32)D_8014F884);
        temp_v1_3 = ((s32)(D_8014F89C - D_8014F890)) / ((s32)D_8014F884);
        D_8014F884 -= 1;
        D_8014F88C += temp_a1_2;
        D_8014F890 += temp_v1_3;
    }
    else
    {
        D_8014F88C = D_8014F894;
        D_8014F890 = D_8014F89C;
    }
    if (D_8014F8C4 != 0)
    {
        temp_v0_3 = ((s32)(D_8014F8C0 - D_8014F8B4)) / ((s32)D_8014F8C4);
        D_8014F8C4 -= 1;
        D_8014F8B4 += temp_v0_3;
        return;
    }
    D_8014F8B4 = D_8014F8C0;
}