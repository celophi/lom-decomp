#include "menu.h"

void func_80140908(void)
{
    volatile u8 padding;
    func_801410B0();
    func_801410E8();
    func_80141324();
    D_801690F4 = -1;
    func_800AA02C();
    D_801690E8 = 0;
    func_80140968();
    D_801690AC = 0;
    D_80169120 = 0;
    func_801423D8();
}

/**
 * decomp.me (99.64%) https://decomp.me/scratch/AGd9K
 */
void func_80140968(void)
{
    s32 s0 = 0;

    /* Force base address into s3 early */
    u8* base = D_800FE778;

    s32 s2 = 0x20;
    s32 s1 = 0;
    s16 params[4];

    do
    {
        /* First call */
        params[0] = 0x110;
        params[1] = s0 + 0x1D8;
        params[2] = 0x10;
        params[3] = 1;
        func_80019A34(params, base + ((s1 >> 2) * 4));

        /* Second call */
        params[0] = (s0 == 2) ? 0x3E8 : 0x3F4;
        params[1] = (s0 == 0) ? 0x120 : 0x150;
        params[2] = 0xC;
        params[3] = 0x30;
        func_80019A34(params, base + ((s2 >> 2) * 4));

        s2 += 0x4A0;
        s0++;
        s1 += 0x4A0;
    } while (s0 < 3);
}

/**
 * decomp.me (97.74%) https://decomp.me/scratch/vmp4D
 */
void func_80140A48(void* arg0)
{
    s32 v0;
    s32 v1;
    s32 s3;
    s32 var_s0;
    u16 temp_v1;
    s32 padding[2];
    func_80140DE8();
    v0 = D_801690AC;
    v1 = D_800F22AC;
    s3 = *((s32*)(((u8*)arg0) + 0x4040));
    D_801690AC = v0 + 1;
    D_800F22AC = v1 + 1;
    func_800A9E78(&D_801690AC, &D_800F22AC);
    if (((*((u32*)(((u8*)D_8012271C) + 0x858))) & 0x80) && ((*((u8*)(((u8*)D_8012271C) + 0x840))) != 0))
    {
        D_80122988 |= D_801229FC;
    }
    v0 = D_80122988 & 0x5000;
    if (v0)
    {
        D_80122988 = v0;
    }
    v0 = D_80122988 & 0xF000;
    if (v0)
    {
        D_80122988 = v0;
    }
    v0 = D_80122988 & 0xF;
    if (v0)
    {
        D_80122988 = v0;
    }
    if (D_8016955C != 0)
    {
        D_80122988 = 0;
    }
    D_8016955C = D_80122988;
    if (D_801228C8 != 0)
    {
        s32 idx;
        u8* base = D_80168778;
        s32 off = D_801228C8;
        off = (off << 1) + off;
        off <<= 4;
        base += off;
        idx = D_80169120;
        base += idx * 2;
        D_80122988 = 0;
        temp_v1 = *((u16*)base);
        if (temp_v1 == (v0 = 0xFFFF))
        {
            if (D_801228C8 < 4)
            {
                var_s0 = 0;
                if (D_80122730 > 0)
                {
                    do
                    {
                        func_8014B69C(1);
                        var_s0++;
                    } while (var_s0 < D_80122730);
                }
                D_80169100 = D_80122730;
            }
            D_801228C8 = 0;
        }
        else
        {
            D_80122988 = (s32)temp_v1;
            D_80169120 = idx + 1;
        }
    }
    *((s32*)(((u8*)arg0) + 0x4040)) = s3;
    func_8014134C(arg0);
}

/**
 * decomp.me (75.58%) https://decomp.me/scratch/AW5Sa
 */
s32* func_80140C14(s32* arg0, s32* arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7)
{
    u8 sp10[0x90]; /* buffer – size matches target frame */
    s32 tmp, count, i;
    s32 *ptr0, *ptr1;
    s32 acc; /* accumulator for halfwords */
    u8 *base, *col;

    /* first call: fill buffer */
    func_800171CC(sp10, arg2, arg6);
    sp10[arg6] = 0;

    /* second call: get number of elements */
    count = func_800644FC(arg0, sp10, arg3);

    /* subtract halfword values according to arg7 */
    if (arg7 == 1)
    {
        /* signed halfword (lh) */
        ptr0 = arg0;
        for (i = 0; i < count; i++)
        {
            arg4 -= *(s16*)((char*)ptr0 + 0x10);
            ptr0 = (s32*)((char*)ptr0 + 0x14);
        }
    }
    else if (arg7 == 2)
    {
        /* unsigned halfword → (val << 16) >> 17 */
        ptr0 = arg0;
        for (i = 0; i < count; i++)
        {
            u16 val = *(u16*)((char*)ptr0 + 0x10);
            arg4 -= ((s16)val) >> 1; /* arithmetic right shift, matches sra */
            ptr0 = (s32*)((char*)ptr0 + 0x14);
        }
    }

    acc = 0;

    /* main loop – process each structure */
    if (count > 0)
    {
        base = (u8*)arg0;
        col = (u8*)arg1;
        tmp = arg4 + (arg5 << 16); /* constant used inside loop */

        do
        {
            /* write fields using negative offsets from base+0x10 */
            *(s32*)(base + 0x8) = tmp + acc;
            *(u32*)(base + 0x4) = 0x808080U;
            *(u8*)(base + 0x3) = 4;
            *(u8*)(base + 0x7) = 100;

            acc += *(s16*)(base + 0x10); /* accumulate halfword */

            /* blend colour words */
            *(s32*)base = (*(s32*)base & 0xFF000000U) | (*(s32*)col & 0x00FFFFFFU);
            *(s32*)col = (*(s32*)col & 0xFF000000U) | ((u32)base & 0x00FFFFFFU);

            /* advance to next structure (20 bytes) */
            base += 0x14;
            col += 0x14;
        } while (--count);
    }

    /* final writes – base and col now point to the next structure */
    ((u8*)base)[3] = 1;
    *(u32*)(base + 4) = 0xE100001FU;
    *(s32*)base = (*(s32*)base & 0xFF000000U) | (*(s32*)col & 0x00FFFFFFU);
    *(s32*)col = (*(s32*)col & 0xFF000000U) | ((u32)base & 0x00FFFFFFU);

    /* return pointer to offset 0x8 of the current structure */
    return (s32*)(base + 8);
}

/**
 * decomp.me (87.64%) https://decomp.me/scratch/ZtHxG
 */
void func_80140DE8(Arg* arg0)
{
    volatile u8 sp0;
    volatile u16 sp2;
    volatile u16 sp4;
    volatile u16 sp6;
    s32 var_t2;
    u8* var_a2;
    u8* var_a1;
    u8* var_t0;
    u8* var_t3;
    u8* temp_t1;
    Arg* t7 = arg0;
    Arg* t4 = t7;

    var_t3 = (u8*)D_80168AA8;
    var_t2 = 0;
    temp_t1 = t7->unk4040;
    sp6 = 0xFF;
    sp4 = 0xFF;
    var_t0 = var_t3 + 8;
    sp2 = 0;
    sp0 = 0;

    temp_t1[3] = 2;
    {
        u32 a2 = ((sp2 & 0xFF) >> 3) << 0xF;
        u32 v0 = ((sp0 & 0xFF) >> 3) << 0xA;
        u32 a0 = (sp6 << 0x10) >> 0xE;
        u32 a1 = (s16)sp4;
        u32 val = a2 | v0 | 0xE2000000;
        val |= (-(s32)a0) & 0x3E0;
        val |= ((-(s32)a1) & 0xFF) >> 3;
        *(u32*)(temp_t1 + 8) = 0;
        *(u32*)(temp_t1 + 4) = val;
    }
    *(u32*)temp_t1 = (*(u32*)temp_t1 & 0xFF000000) | (t4->unk3C & 0xFFFFFF);
    t4->unk3C = (t4->unk3C & 0xFF000000) | ((u32)temp_t1 & 0xFFFFFF);

    temp_t1 += 0xC;
    var_a2 = temp_t1;
    var_a1 = var_a2 + 0xE;

    do
    {
        *(u32*)(var_a1 - 0xA) = 0x808080;
        *(var_a1 - 0xB) = 4;
        *(var_a1 - 7) = 0x64;
        *(u16*)(var_a1 - 2) = *(u16*)var_t3;
        *(u32*)(var_a1 - 6) = *(u32*)(var_t0 - 4);
        *(u32*)(var_a1 + 2) = *(u32*)var_t0;

        if (var_t2 >= 0x11)
        {
            *(u16*)var_a1 = 0x7C81;
        }
        else
        {
            *(u16*)var_a1 = 0x7C80;
        }

        var_t2++;
        var_a1 += 0x14;
        var_t0 += 0xC;
        var_t3 += 0xC;

        *(u32*)var_a2 = (*(u32*)var_a2 & 0xFF000000) | (t4->unk3C & 0xFFFFFF);
        t4->unk3C = (t4->unk3C & 0xFF000000) | ((u32)var_a2 & 0xFFFFFF);
        var_a2 += 0x14;
    } while (var_t2 < 0x1D);

    temp_t1 = var_a2;

    sp4 = 0xFF;
    sp6 = 0xFF;
    sp0 = 0;
    sp2 = 0;

    temp_t1[3] = 2;
    {
        u32 a2 = ((sp2 & 0xFF) >> 3) << 0xF;
        u32 v0 = ((sp0 & 0xFF) >> 3) << 0xA;
        u32 a0 = (sp6 << 0x10) >> 0xE;
        u32 a1 = (s16)sp4;
        u32 val = a2 | v0 | 0xE2000000;
        val |= (-(s32)a0) & 0x3E0;
        val |= ((-(s32)a1) & 0xFF) >> 3;
        *(u32*)(temp_t1 + 8) = 0;
        *(u32*)(temp_t1 + 4) = val;
    }
    *(u32*)temp_t1 = (*(u32*)temp_t1 & 0xFF000000) | (t4->unk3C & 0xFFFFFF);
    t4->unk3C = (t4->unk3C & 0xFF000000) | ((u32)temp_t1 & 0xFFFFFF);

    temp_t1 += 0xC;
    temp_t1[3] = 1;
    *(u32*)(temp_t1 + 4) = 0xE1000005;
    *(u32*)temp_t1 = (*(u32*)temp_t1 & 0xFF000000) | (t4->unk3C & 0xFFFFFF);
    t4->unk3C = (t4->unk3C & 0xFF000000) | ((u32)temp_t1 & 0xFFFFFF);

    t7->unk4040 = temp_t1 + 8;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/CKNIH
 */
void func_801410B0(void)
{
    RECT rect;

    rect.x = 0x140;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0x1F2;
    menu_func_801410FC(&rect);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/A1YTp
 */
void func_801410E8(void)
{
    D_80169108 = &D_80151EBC;
}


/**
 * decomp.me (100%) https://decomp.me/scratch/tG03R
 */
void menu_func_801410FC(ArgStruct* arg0)
{
    u8* base = D_80160260;
    u8* s0 = base + 0xC;
    s32 s3 = *(s32*)(s0 + 8);
    ArgStruct sp10;
    u16* p;
    int i;

    D_8016910C = *(s32*)(s0 + 0x14);

    /* First loop */
    sp10.unk0 = arg0->unk4;
    sp10.unk2 = arg0->unk6;
    sp10.unk4 = 0x100;
    sp10.unk6 = 1;

    p = (u16*)(base + 0x20);
    for (i = 0; i < 0x100; i++)
    {
        if (*p != 0)
            *p |= 0x8000;
        p++;
    }
    func_80019A34(&sp10, s0 + 0x14);

    /* Second call */
    sp10.unk0 = arg0->unk0;
    sp10.unk2 = arg0->unk2;
    {
        // Enforce specific instruction ordering: addiu a1, s3, 8 then addu a1, s0, a1
        u8* temp = s0 + (s3 + 8);
        sp10.unk4 = *(u16*)(temp + 8);
        sp10.unk6 = *(u16*)(temp + 10);
        func_80019A34(&sp10, temp + 0xC);
    }

    /* Third loop */
    sp10.unk0 = arg0->unk4;
    sp10.unk2 = arg0->unk6 + 1;
    sp10.unk4 = 0x100;
    sp10.unk6 = 1;

    p = (u16*)(base + 0x822C);
    for (i = 0; i < 0x100; i++)
    {
        if (*p != 0)
            *p |= 0x8000;
        p++;
    }
    func_80019A34(&sp10, base + 0x822C);
}