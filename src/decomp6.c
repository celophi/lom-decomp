#include "decomp6.h"

void InitializeControllers(s8 arg0)
{
    u8* base;
    u8** new_var;
    u8* ptr;
    u8* new_var2;
    int new_var3;
    u16 temp;
    int i;
    int sentinel;
    unsigned int val_ff;
    int val_40;
    int a2;
    int a1;
    func_80030DF8((void*)0x801ED75C, (void*)0x801ED77E);
    D_801ED7A4 = VSyncCallback(0);
    base = (u8*)0x801ED600;
    base[0xAD] = 0;
    base[0x15B] = 0x10;
    func_80015708(base);
    func_80015708(base + 0xAE);
    func_80015708(base + 0x20);
    new_var2 = base + 0xCE;
    func_80015708(new_var2);
    i = 1;
    val_40 = 0x40;
    val_ff = 0xFF;
    sentinel = -1;
    ptr = base + 0xAE;
    do
    {
        temp = *((u16*)(ptr + 0x92));
        i--;
        ptr[0x94] = val_40;
        ptr[0x97] = 0;
        ptr[0x96] = 0;
        ptr[0x95] = 0;
        ptr[0x90] = arg0;
        ptr[0x91] = 0;
        ptr[0xAA] = 0;
        ptr[0xAB] = 0;
        ptr[0xAC] = 0;
        ptr[0x20] = val_ff;
        ptr[0x00] = val_ff;
        temp &= 0xF0FF;
        *((u16*)(ptr - -0x92)) = temp;
        ptr[0x92] = 0;
        ptr -= 0xAE;
    } while (new_var3 = i != sentinel);
    base[0x1A8] = 0 * 0;
    base[0x1A9] = 0;
    base[0x1A0] = 0;
    base[0x1A1] = 0;
    base[0x1AA] = 0;
    func_8002E958(val_ff, i, sentinel, val_40);
    do
    {
        VSync(0);
        func_80015674();
        a2 = 1;
        a1 = a2;
        ptr = base + 0xAE;
        do
        {
            temp = *((u16*)((*(new_var = &ptr)) + 0x92));
            if ((!(((temp >> 6) >> 2) & 1)) && (((temp >> 9) & 3) != 2))
            {
                a2 = 0;
            }
            a1--;
            ptr -= 0xAE;
        } while (a1 != (-1));
    } while (a2 == 0);
    base[0x1A2] = 0;
    base[0x1A3] = 0;
}

/**
 * decomp.me link (92.77%) https://decomp.me/scratch/rDO0T
 */
void func_80014C54(arg0_struct* arg0, s32* arg1)
{
    int new_var;
    u8* base;
    u32 temp_v0;
    s32 cnt;
    int new_var2;
    s32 idx;
    unsigned int new_var6;
    s32 rem;
    s32 new_var5;
    s32 loop_end;
    int new_var4;
    u8 const_ff;
    unsigned short var_t1;
    int var_a0_2;
    u8 var_a3;
    s32 var_t0;
    s32 v;
    u16 temp_v1_4;
    s32 diff;
    u16 mask;
    u16 a1_mask;
    int new_var3;
    u8* var_a2;
    base = (u8*)0x801ED600;
    temp_v0 = func_8002E9E4(arg0->unkAD);
    switch (temp_v0)
    {
    case 0:
        new_var2 = arg0->unk92 | 0x100;
        arg0->unk20 = 0xFF;
        arg0->unk92 = new_var2 & 0xF9FF;
        return;

    case 1:
        arg0->unk92 &= 0xFEFF;
        if (arg0->unk92 & 0x600)
        {
            arg0->unk92 = (arg0->unk92 & 0xF8FF) | 0x200;
        }

    case 4:

    case 5:
        arg0->unk20 = 0xFE;
        func_80015708(((u8*)arg0) + 0x20);
        return;

    case 2:
        arg0->unk94 = 0x40;
        if (arg0->unk90 != 0)
        {
            if (arg0->unk91 & 1)
            {
                arg0->unk95 = arg0->unk91;
                *arg1 += arg0->unkAB;
            }
            else
            {
                u8 temp = arg0->unk92;
                if ((temp != 0) && ((temp * 0x10) >= base[0x1A9]))
                {
                    arg0->unk95 = 1;
                    *arg1 += arg0->unkAB;
                }
                else
                {
                    arg0->unk95 = 0;
                }
            }
        }
        else
        {
            arg0->unk95 = 0;
        }
        if ((arg0->unk92 & 0x600) != 0x400)
        {
            func_8002EDA4(arg0->unkAD, &arg0->unk94, 2);
            arg0->unkAB = 10;
            arg0->unk92 = (arg0->unk92 | 0x400) & 0xF5FF;
        }
        break;

    case 6:
        if (arg0->unk90 != 0)
        {
            u8 temp = arg0->unk92;
            if (temp != 0)
            {
                arg0->unk96 = temp;
                *arg1 += arg0->unkAC;
            }
            else
            {
                arg0->unk96 = 0;
            }
            if (arg0->unk91 & 1)
            {
                arg0->unk95 = 1;
                *arg1 += arg0->unkAB;
            }
            else
            {
                arg0->unk95 = 0;
            }
        }
        else
        {
            arg0->unk95 = 0;
            arg0->unk96 = 0;
        }
        switch ((arg0->unk92 >> 9) & 3)
        {
        case 0:
            idx = 0;
            arg0->unk92 = (arg0->unk92 & 0xF9FF) | 0x200;
            cnt = func_8002EAB0(arg0->unkAD, 4, -1);
            new_var6 = cnt;
            if (new_var6 != 0)
            {
                cnt--;
                loop_end = -1;
                do
                {
                    if (func_8002EAB0(arg0->unkAD, 4, idx) != 7)
                    {
                        idx++;
                        cnt--;
                        continue;
                    }
                    if (func_8002EAB0(arg0->unkAD, 3, 0) != idx)
                    {
                        func_8002ED5C(arg0->unkAD, idx, 0);
                        arg0->unk20 = 0xFE;
                        func_80015708(((u8*)arg0) + 0x20);
                        return;
                    }
                    idx++;
                    cnt--;
                } while (cnt != loop_end);
            }

        case 1:
            arg0->unk92 = (arg0->unk92 & 0xF9FF) | 0xC00;
            cnt = 5;
            loop_end = -1;
            const_ff = 0xFF;
            while (cnt != loop_end)
            {
                ((u8*)arg0)[0x98 + cnt] = 0xFF;
                cnt--;
            }

            idx++;
            rem = func_8002EBA8(arg0->unkAD, -1, idx = 0);
            new_var3 = rem;
            arg0->unkAA = new_var3;
            func_8002EDA4(arg0->unkAD, &arg0->unk95, rem);
            cnt = idx;
            rem--;
            arg0->unkAB = 0;
            arg0->unkAC = 0;
            while (rem != (-1))
            {
                int temp1 = func_8002EBA8(arg0->unkAD, cnt, 1);
                if (temp1 == 1)
                {
                    int val = func_8002EBA8(arg0->unkAD, cnt, 3);
                    if (val == 0)
                    {
                        if (arg0->unk98 == 0xFF)
                        {
                            arg0->unkAB = func_8002EBA8(arg0->unkAD, cnt, 4);
                            arg0->unk98 = cnt;
                        }
                    }
                    else if (val == temp1)
                    {
                        if (arg0->unk99 == 0xFF)
                        {
                            arg0->unkAC = func_8002EBA8(arg0->unkAD, cnt, 4);
                            arg0->unk99 = cnt;
                        }
                    }
                }
                rem--;
            }

            func_8002ED24(arg0->unkAD, &arg0->unk98);
            arg0->unk20 = 0xFE;
            func_80015708(((u8*)arg0) + 0x20);
            return;

        default:
            break;
        }

        break;

    default:
        break;
    }

    if (arg0->unkAD & 0x10)
    {
        var_a2 = base + 0x15C;
    }
    else
    {
        var_a2 = base + 0x17E;
    }
    new_var = 1;
    new_var4 = 0x8000;
    if ((arg0->unkAD & 0xF) != 0)
    {
        if ((*((u16*)var_a2)) == new_var4)
        {
            var_a2 += (temp_v0 * 8) + 2;
        }
        else
        {
            arg0->unk20 = 0xFF;
            return;
        }
    }
    else if ((*((u16*)var_a2)) == 0x8000)
    {
        var_a2 += 2;
    }
    if (var_a2[0] == 0)
    {
        u8 tmp = var_a2[1];
        u8 shift = tmp >> 4;
        var_t1 = 0;
        if (shift == 5)
        {
        }
        else if (shift == 4)
        {
        }
        else if (shift == 7)
        {
            var_t1 = 2;
        }
        else
        {
            var_t1 = -1;
            arg0->unkA6 = 0x80;
        }
        var_a0_2 = var_t1;
        if (var_a0_2 < 3)
        {
            temp_v1_4 = ~(*((u16*)(var_a2 + 2)));
            if (arg0->unk20 == var_a0_2)
            {
                arg0->unk26 = (arg0->unk24 = temp_v1_4 & (temp_v1_4 ^ arg0->unk22));
            }
            else
            {
                arg0->unk26 = (arg0->unk24 = temp_v1_4);
                if (var_a0_2 == 1)
                {
                    arg0->unkA6 = var_a2[4];
                    arg0->unkA7 = var_a2[5];
                    arg0->unkA8 = var_a2[6];
                    arg0->unkA9 = var_a2[7];
                    arg0->unk21 = 0;
                }
                else if (var_a0_2 == 2)
                {
                    if (arg0->unk92 & 0x800)
                    {
                        arg0->unkA7 = 0x80;
                        arg0->unkA8 = 0x80;
                        arg0->unkA9 = 0x80;
                        arg0->unk21 = 0;
                    }
                    else
                    {
                        arg0->unkA6 = var_a2[4];
                        arg0->unkA7 = var_a2[5];
                        arg0->unkA8 = var_a2[6];
                        arg0->unkA9 = var_a2[7];
                        arg0->unk21 = 0;
                    }
                }
                arg0->unk20 = var_t1;
            }
            arg0->unk22 = temp_v1_4;
            if (base[0x1A8] != 0)
            {
                var_a3 = 0x0B;
                var_t0 = 3;
            }
            else
            {
                var_a3 = 0x16;
                var_t0 = 6;
            }
            var_a0_2 = new_var;
            if (temp_v1_4 & 0x10)
            {
                if ((arg0->unk24 & 0x10) && (base[0x1AA] == 0))
                {
                    arg0->unk9E = var_a3;
                }
                else
                {
                    v = arg0->unk9E - var_a0_2;
                    if (v <= 0)
                    {
                        v = var_t0;
                        arg0->unk26 |= 0x10;
                    }
                    arg0->unk9E = v;
                }
            }
            if (temp_v1_4 & 0x20)
            {
                if ((arg0->unk24 & 0x20) && (base[0x1AA] == 0))
                {
                    arg0->unk9F = var_a3;
                }
                else
                {
                    v = arg0->unk9F - var_a0_2;
                    if (v <= 0)
                    {
                        v = var_t0;
                        arg0->unk26 |= 0x20;
                    }
                    arg0->unk9F = v;
                }
            }
            if (temp_v1_4 & 0x40)
            {
                if ((arg0->unk24 & 0x40) && (base[0x1AA] == 0))
                {
                    arg0->unkA0 = var_a3;
                }
                else
                {
                    v = arg0->unkA0 - var_a0_2;
                    if (v <= 0)
                    {
                        v = var_t0;
                        arg0->unk26 |= 0x40;
                    }
                    arg0->unkA0 = v;
                }
            }
            if (temp_v1_4 & 0x80)
            {
                if ((arg0->unk24 & 0x80) && (base[0x1AA] == 0))
                {
                    arg0->unkA1 = var_a3;
                }
                else
                {
                    v = arg0->unkA1 - var_a0_2;
                    if (v <= 0)
                    {
                        v = var_t0;
                        arg0->unk26 |= 0x80;
                    }
                    arg0->unkA1 = v;
                }
            }
            if (var_t1 != 0)
            {
                diff = var_a2[4] - arg0->unkA6;
                if (((u32)(diff + 0x38)) < 0x71U)
                {
                    diff = 0;
                }
                if (diff < (-0x80))
                {
                    diff = -0x80;
                }
                else if (diff >= 0x80)
                {
                    diff = 0x7F;
                }
                new_var5 = diff >> 4;
                if (diff < 0)
                {
                    arg0->unk28 = (diff + 0xF) >> 4;
                }
                else
                {
                    arg0->unk28 = new_var5;
                }
                diff = var_a2[5] - arg0->unkA7;
                if (((u32)(diff + 0x38)) < 0x71U)
                {
                    diff = 0;
                }
                if (diff < (-0x80))
                {
                    diff = -0x80;
                }
                else if (diff >= 0x80)
                {
                    diff = 0x7F;
                }
                new_var6 = diff >> 4;
                if (diff < 0)
                {
                    arg0->unk2A = (diff + 0xF) >> 4;
                }
                else
                {
                    arg0->unk2A = new_var6;
                }
                diff = var_a2[6] - arg0->unkA8;
                if (((u32)(diff + 0x38)) < 0x71U)
                {
                    diff = 0;
                }
                if (diff < (-0x80))
                {
                    diff = -0x80;
                }
                else if (diff >= 0x80)
                {
                    diff = 0x7F;
                }
                if (diff < 0)
                {
                    arg0->unk2C = (diff + 0xF) >> 4;
                }
                else
                {
                    arg0->unk2C = diff >> 4;
                }
                a1_mask = 0x80;
                if (arg0->unk2C >= 0)
                {
                    if (arg0->unk2C > 0)
                    {
                        a1_mask = 0x20;
                    }
                    else
                    {
                        a1_mask = 0;
                    }
                }
                diff = var_a2[7] - arg0->unkA9;
                if (((u32)(diff + 0x38)) < 0x71U)
                {
                    diff = 0;
                }
                if (diff < (-0x80))
                {
                    diff = -0x80;
                }
                else if (diff >= 0x80)
                {
                    diff = 0x7F;
                }
                loop_end = arg0->unk2E;
                if (diff < 0)
                {
                    arg0->unk2E = (diff + 0xF) >> 4;
                }
                else
                {
                    arg0->unk2E = diff >> 4;
                }
                if (loop_end < 0)
                {
                    a1_mask |= 0x10;
                }
                else if (loop_end > 0)
                {
                    a1_mask |= 0x40;
                }
                mask = a1_mask & (a1_mask ^ ((arg0->unk21 & 0xF) * 0x10));
                var_a0_2 = mask | ((a1_mask & 0xFF) >> 4);
                if (a1_mask & 0x10)
                {
                    if ((mask & 0x10) && (base[0x1AA] == 0))
                    {
                        arg0->unkA2 = var_a3;
                    }
                    else
                    {
                        short v2;
                        if ((arg0->unkA2 - var_a0_2) <= 0)
                        {
                            var_a0_2 |= 0x10;
                            v2 = var_t0;
                        }
                        arg0->unkA2 = v2;
                    }
                }
                if (a1_mask & 0x20)
                {
                    if ((mask & 0x20) && (base[0x1AA] == 0))
                    {
                        arg0->unkA3 = var_a3;
                    }
                    else
                    {
                        s32 v2 = arg0->unkA3 - var_a0_2;
                        if (v2 <= 0)
                        {
                            var_a0_2 |= 0x20;
                            v2 = var_t0;
                        }
                        arg0->unkA3 = v2;
                    }
                }
                if (a1_mask & 0x40)
                {
                    if ((mask & 0x40) && (base[0x1AA] == 0))
                    {
                        arg0->unkA4 = var_a3;
                    }
                    else
                    {
                        s32 v2 = arg0->unkA4 - var_a0_2;
                        if (v2 <= 0)
                        {
                            var_a0_2 |= 0x40;
                            v2 = var_t0;
                        }
                        arg0->unkA4 = v2;
                    }
                }
                if (a1_mask & 0x80)
                {
                    if ((mask & 0x80) && (base[0x1AA] == 0))
                    {
                        arg0->unkA5 = var_a3;
                    }
                    else
                    {
                        s32 v2 = arg0->unkA5 - var_a0_2;
                        if (v2 <= 0)
                        {
                            var_a0_2 |= 0x80;
                            v2 = var_t0;
                        }
                        arg0->unkA5 = v2;
                    }
                }
                arg0->unk21 = var_a0_2;
                return;
            }
        }
        else
        {
            arg0->unk20 = 0xFF;
            return;
        }
    }
    else
    {
        func_80015708(((u8*)arg0) + 0x20);
        return;
    }
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/FnYh0
 */
void func_80015674(void)
{
    s32 sp10;
    u8* base = (u8*)0x801ED600;

    if (func_8002E938() != 0)
    {
        sp10 = 0;
        base[0x1A9] = (base[0x1A9] + 1) & 0xF;
        func_80014C54((arg0_struct*)base, &sp10);
        func_80014C54((arg0_struct*)(base + 0xAE), &sp10);
        base[0x1AA] = 0;
        return;
    }
    func_80015708((void*)(base + 0x20));
    func_80015708((void*)(base + 0xCE));
    base[0x1AA] = 1;
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/TSmff
 */
void func_80015708(void* arg0)
{
    unsigned char* p8 = (unsigned char*)arg0;
    unsigned short* p16 = (unsigned short*)arg0;

    p16[3] = 0; // sh zero,6(a0)
    p16[2] = 0; // sh zero,4(a0)
    p16[1] = 0; // sh zero,2(a0)
    p8[1] = 0;  // sb zero,1(a0)
    p16[7] = 0; // sh zero,0xe(a0)
    p16[6] = 0; // sh zero,0xc(a0)
    p16[5] = 0; // sh zero,0xa(a0)
    p16[4] = 0; // sh zero,8(a0)
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/NkZqx
 */
void func_8001572C(void)
{
    u8* base = (u8*)0x801ED600;
    VSync(0);
    func_8002E978();
    VSyncCallback(*(void (**)(void))(base + 0x1A4));
    base[0x1AA] = 0;
}