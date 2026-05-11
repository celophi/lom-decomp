#include "decomp4.h"

/**
 * decomp.me (100%) https://decomp.me/scratch/hjYpL
 */
void func_800299EC(void)
{
    s32 val = (s32)D_8003EC6A;
    *(s16*)0x1F801DB0 = (s16)val;
    *(s16*)0x1F801DB2 = (s16)val;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/TxNq3
 */
void func_80029A0C(s32* arg0, s32* arg1, u32 arg2)
{

    arg2 >>= 2;

    while ((arg2 >> 2) != 0)
    {

        s32 second = arg0[1];
        s32 third = arg0[2];
        s32 fourth = arg0[3];
        s32 first = arg0[0];

        arg1[0] = first;
        arg1[1] = second;
        arg1[2] = third;
        arg1[3] = fourth;

        arg0 += 4;
        arg1 += 4;
        arg2 -= 4;
    }

    while (arg2 != 0)
    {
        *arg1 = *arg0;
        arg0++;
        arg1++;
        arg2--;
    }
}

/**
 * decomp.me (86.49%) https://decomp.me/scratch/07M97
 */
void func_80029A8C(void)
{
    u8* xa;
    u8* new_var7;
    s32 temp;
    unsigned long new_var;
    s32 temp_s1;
    u32 t0;
    u32 t1;
    s32 temp_s0;
    u8** new_var2;
    u16 u16_tmp;
    int i;
    int new_var3;
    unsigned long new_var5;
    u32 mask;
    void* new_var6;
    s32* new_var8;
    u32 bit;
    u8* sfx_base;
    void* seq_ptr;
    u16* new_var9;
    void* ptr28;
    s32 new_var4;
    u32* ptr;
    new_var4 = D_8003EC60;
    D_8003EC70 = (D_8003EC70 + 1) & 0xFF;
    if (D_8003EC64 != 0)
    {
        D_8003EC64--;
        D_8003EC68 += D_8003EC60;
        func_800299EC(new_var4);
    }
    xa = g_akao_xa_tracker;
    if (((*((s32*)(xa + 0x0C))) != 0) && ((*((s32*)(xa + 0x48))) != 0))
    {
        (*((s32*)(xa + 0x48)))--;
        temp = (*((s32*)(xa + 0x40))) + (*((s32*)(xa + 0x44)));
        if ((temp & 0xFF00) != ((*((s32*)(xa + 0x40))) & 0xFF00))
        {
            if (D_8004F754 & 2)
            {
                temp_s0 = ((*((s32*)(xa + 0x40))) * D_8003D47C) >> 16;
                func_800242EC(*((s32*)(xa + 0x10)), temp_s0, temp_s0, 0);
                t0 = temp_s0;
                t1 = temp_s0;
            }
            else
            {
                t1 = (temp_s0 = (temp << 15) >> 16);
                func_800242EC(*((s32*)(xa + 0x10)), temp_s0, 0, 0);
                t0 = 0;
            }
            func_800242EC((*((s32*)(xa + 0x10))) + 1, t0, t1, 0);
        }
        D_8004F7A0 = temp & 0xFFFF;
    }
    if (D_8003EC42 != 0)
    {
        D_8003EC42--;
        D_8003EC78 += D_8003EC3C;
    }
    if (D_8003EC40 != 0)
    {
        new_var8 = &D_8003EC38;
        D_8003EC40--;
        temp_s1 = D_8003EC74 + (*new_var8);
        if ((D_8003EC74 & 0xFF0000) != (temp_s1 & 0xFF0000))
        {
            new_var3 = 0x100;
            ptr = (u32*)(D_80049130 + new_var3);
            i = 32;
            do
            {
                *ptr |= 0x10;
                ptr += 0x46;
                i--;
            } while (i != 0);
        }
        D_8003EC74 = temp_s1;
    }
    seq_ptr = g_akao_seq_channel0;
    if ((*((s32*)(((u8*)seq_ptr) + 4))) != 0)
    {
        if ((*((s16*)(((u8*)seq_ptr) + 0x58))) != 0)
        {
            new_var5 = 0x50;
            u16_tmp = *((u16*)(((u8*)seq_ptr) + 0x58));
            new_var = u16_tmp;
            *((s16*)(((u8*)seq_ptr) + 0x58)) = new_var - 1;
            temp = (*((s32*)(((u8*)seq_ptr) + new_var5))) + (*((s32*)(((u8*)seq_ptr) + 0x54)));
            if ((((unsigned long)temp) & 0x7F0000) != ((*((s32*)(((u8*)seq_ptr) + 0x50))) & 0x7F0000))
            {
                func_80026E8C(seq_ptr, D_80049130, new_var);
            }
            *((s32*)(((u8*)g_akao_seq_channel0) + 0x50)) = temp;
        }
    }
    ptr28 = D_8003EC28;
    new_var6 = D_8003EC28;
    if ((D_8003EC28 != 0) && ((*((s32*)(((u8*)ptr28) + 4))) != 0))
    {
        t0 = *((s16*)(((u8*)new_var6) + 0x58));
        if (t0 != 0)
        {
            u16_tmp = *((u16*)(((u8*)ptr28) + 0x58));
            *((s16*)(((u8*)D_8003EC28) + 0x58)) = u16_tmp - 1;
            temp = (*((s32*)(((u8*)ptr28) + 0x50))) + (*((s32*)(((u8*)ptr28) + 0x54)));
            if ((temp & 0x7F0000) != ((*((s32*)(((u8*)D_8003EC28) + 0x50))) & 0x7F0000))
            {
                func_80026E8C(D_8003EC28, (void*)((u32)D_8003EC24), (u32)D_8003EC28);
            }
            *((s32*)(((u8*)D_8003EC28) + 0x50)) = temp;
        }
    }
    mask = D_8004D400;
    if (mask != 0)
    {
        new_var3 = mask;
        sfx_base = g_sfx_channels + 0x40;
        bit = 0x1000;
        do
        {
            if (new_var3 & bit)
            {
                u16_tmp = *((u16*)(sfx_base + 0x4E));
                if (u16_tmp != 0)
                {
                    *((u16*)(sfx_base + 0x4E)) = u16_tmp - 1;
                    temp = ((s16)(*((s16*)(sfx_base + 0xA4)))) + (*((s16*)(sfx_base + 0xA6)));
                    if ((temp & 0xFF00) != ((*((s16*)(sfx_base + 0xA4))) & 0xFF00))
                    {
                        *((u32*)(sfx_base + 0xC0)) |= 3;
                    }
                    *((s16*)(sfx_base + 0xA4)) = temp;
                }
                u16_tmp = *((u16*)(sfx_base + 0x30));
                if (u16_tmp != 0)
                {
                    *((u16*)(sfx_base + 0x30)) = u16_tmp - 1;
                    temp = (*((u16*)(sfx_base + 0x2E))) + (*((s16*)(sfx_base + 0xA2)));
                    if ((temp & 0xFF00) != ((*((u16*)(sfx_base + 0x2E))) & 0xFF00))
                    {
                        *((u32*)(sfx_base + 0xC0)) |= 3;
                    }
                    *((u16*)(sfx_base + 0x2E)) = temp;
                }
                u16_tmp = *((u16*)(sfx_base + 0x48));
                if (u16_tmp != 0)
                {
                    *((u16*)(sfx_base + 0x48)) = u16_tmp - 1;
                    temp = (*((s32*)(sfx_base + 0x00))) + (*((s32*)(sfx_base + 0x04)));
                    if ((temp & 0xFF00) != ((*((s32*)(sfx_base + 0x00))) & 0xFF00))
                    {
                        *((u32*)(sfx_base + 0xC0)) |= 0x10;
                    }
                    *((s32*)((*(new_var2 = &sfx_base)) + 0x00)) = temp;
                }
                new_var3 ^= bit;
            }
            bit <<= 1;
            sfx_base += 0x118;
        } while (new_var3 != 0);
    }
}