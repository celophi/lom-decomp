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