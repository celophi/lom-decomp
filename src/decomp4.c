#include "decomp4.h"

u16 g_akao_irq_frame_counter;
s32 D_8004D40C;
u32 D_8004F758;
s32 D_8003EC18;

extern u32 D_8003D24C[];

typedef struct
{
    u8* unk0;      // 0x00
    u8* unk4[4];   // 0x04
    u8* unk14;     // 0x14
    u8 pad18[28];  // 0x18
    u32 unk34;     // 0x34
    u8 pad38[60];  // 0x38
    u16 unk74[21]; // 0x74
    u16 unk9E;     // 0x9E
    u8 padA0[56];  // 0xA0
    u16 unkD8;     // 0xD8
} Context;

typedef struct
{
    s16 unk8;
    u8 padA[6];
    s16 unkB;
    s16 unkA;
} UnkStruct;

typedef struct
{
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
} SmallSlot;

typedef struct
{
    u32 unk0;  /* 0x00 */
    u32 unk4;  /* 0x04 */
    u32 _pad8; /* 0x08 */
    u16 unkC;  /* 0x0C */
    u16 unkE;  /* 0x0E */
} ArticSlot; /* sizeof = 0x10 */

/**
 * decomp.me (100%) https://decomp.me/scratch/hjYpL
 */
void akao_apply_cdvol_to_spu(void)
{
    s32 val = (s32)g_akao_cdvol_current;
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
    new_var4 = g_akao_cdvol_step;
    g_akao_cdvol_tick = (g_akao_cdvol_tick + 1) & 0xFF;
    if (g_akao_cdvol_fade_ticks != 0)
    {
        g_akao_cdvol_fade_ticks--;
        g_akao_cdvol_acc += g_akao_cdvol_step;
        akao_apply_cdvol_to_spu(new_var4);
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
    if (g_akao_mastervol_fade_ticks != 0)
    {
        new_var8 = &g_akao_mastervol_step;
        g_akao_mastervol_fade_ticks--;
        temp_s1 = g_akao_mastervol_acc + (*new_var8);
        if ((g_akao_mastervol_acc & 0xFF0000) != (temp_s1 & 0xFF0000))
        {
            new_var3 = 0x100;
            ptr = (u32*)(g_akao_seq_channels + new_var3);
            i = 32;
            do
            {
                *ptr |= 0x10;
                ptr += 0x46;
                i--;
            } while (i != 0);
        }
        g_akao_mastervol_acc = temp_s1;
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
                func_80026E8C(seq_ptr, g_akao_seq_channels, new_var);
            }
            *((s32*)(((u8*)g_akao_seq_channel0) + 0x50)) = temp;
        }
    }
    ptr28 = g_akao_seq_channel1;
    new_var6 = g_akao_seq_channel1;
    if ((g_akao_seq_channel1 != 0) && ((*((s32*)(((u8*)ptr28) + 4))) != 0))
    {
        t0 = *((s16*)(((u8*)new_var6) + 0x58));
        if (t0 != 0)
        {
            u16_tmp = *((u16*)(((u8*)ptr28) + 0x58));
            *((s16*)(((u8*)g_akao_seq_channel1) + 0x58)) = u16_tmp - 1;
            temp = (*((s32*)(((u8*)ptr28) + 0x50))) + (*((s32*)(((u8*)ptr28) + 0x54)));
            if ((temp & 0x7F0000) != ((*((s32*)(((u8*)g_akao_seq_channel1) + 0x50))) & 0x7F0000))
            {
                func_80026E8C(g_akao_seq_channel1, (void*)((u32)D_8003EC24), (u32)g_akao_seq_channel1);
            }
            *((s32*)(((u8*)g_akao_seq_channel1) + 0x50)) = temp;
        }
    }
    mask = g_akao_sfx_control.unk0;
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

/**
 * decomp.me (84.65%) https://decomp.me/scratch/XMCUh
 */
s32 func_80029E88(s32 arg0, s32 arg1)
{
    u32 var_a0;

    s32 var_s1;
    s32 var_s2;
    s32 var_s3;

    AkaoDriverFlags* driver_flags;

    var_a0 = g_akao_seq_channel0->unk20 >> 16;
    var_s2 = g_akao_master_vol_scalar;

    if (var_s2 != 0)
    {
        u32 product = (u32)var_a0 * (u32)var_s2;
        if (var_s2 < 0x80U)
        {
            var_a0 = var_a0 + (product >> 7);
        }
        else
        {
            var_a0 = product >> 8;
        }
    }

    g_akao_seq_channel0->unk28 = g_akao_seq_channel0->unk28 + var_a0;

    if ((g_akao_seq_channel0->unk28 & 0xFFFF0000U) || (g_akao_driver_mode_flags & 4))
    {
        g_akao_seq_channel0->unk28 = (s32)(g_akao_seq_channel0->unk28 & 0xFFFFU);

        var_s2 = arg0;
        driver_flags = &g_akao_driver_flags;

        do
        {
            var_s1 = 1;
            var_s3 = g_akao_seq_channel0->unk4;

            do
            {
                if (var_s3 & var_s1)
                {
                    AkaoChannelState* s = (AkaoChannelState*)var_s2;
                    s->unk66 = (u16)(s->unk66 - 1);
                    s->unk68 = (u16)(s->unk68 - 1);

                    if (s->unk66 == 0)
                    {
                        func_8002AD28(var_s2, var_s1);
                    }
                    else if (s->unk68 == 0)
                    {
                        g_akao_seq_channel0->unk18 = (s32)(g_akao_seq_channel0->unk18 | var_s1);
                    }

                    func_80024660(var_s2, var_s1, 0);
                    var_s3 &= ~var_s1;
                }

                var_s2 += 0x118;
                var_s1 <<= 1;
            } while (var_s3 != 0);

            if (g_akao_seq_channel0->unk5C != 0)
            {
                g_akao_seq_channel0->unk5C = (u16)(g_akao_seq_channel0->unk5C - 1);
                g_akao_seq_channel0->unk20 = g_akao_seq_channel0->unk20 + g_akao_seq_channel0->unk24;
            }

            if (g_akao_seq_channel0->unk5A != 0)
            {
                g_akao_seq_channel0->unk5A = (s16)(g_akao_seq_channel0->unk5A - 1);
                g_akao_seq_channel0->unk48 = g_akao_seq_channel0->unk48 + g_akao_seq_channel0->unk4C;
                if (arg1 == 0)
                {
                    driver_flags->unk8 |= 0x80;
                }
            }

            if (g_akao_seq_channel0->unk68 != 0)
            {

                g_akao_seq_channel0->unk6A = (u16)(g_akao_seq_channel0->unk6A + 1);
                if (g_akao_seq_channel0->unk6A == g_akao_seq_channel0->unk68)
                {
                    g_akao_seq_channel0->unk6A = 0;
                    g_akao_seq_channel0->unk66 = (u16)(g_akao_seq_channel0->unk66 + 1);
                    if (g_akao_seq_channel0->unk66 == g_akao_seq_channel0->unk64)
                    {
                        g_akao_seq_channel0->unk66 = 0;
                        g_akao_seq_channel0->unk6C = (u16)(g_akao_seq_channel0->unk6C + 1);
                        if (arg1 == 0)
                        {
                            if (D_8003EC44 != 0)
                            {
                                D_8003EC44 -= 1;
                            }
                        }
                    }
                }
            }

            if (arg1 == 0)
            {
                var_s2 = arg0;
                if (D_8003EC44 != 0)
                {
                    continue;
                }
            }
            break;
        } while (1);
    }

    return g_akao_seq_channel0->unk4;
}

/**
 * decomp.me (84.56%) https://decomp.me/scratch/ICO2k
 */
void func_8002A134(void)
{
    s32 temp_s5;
    s32 var_s3;
    s32 temp_a3;
    s32 temp_v1_3;
    u16 temp_v0_2;
    u16 temp_v1_2;
    SfxChannel* channel;
    u32 bitMask;

    temp_s5 = GetRCnt(0xF2000002);

    /* First conditional block */
    {
        AkaoChannelState* seq0;
        u16 ec1c;
        u32 unk18_val;

        seq0 = g_akao_seq_channel0;
        ec1c = g_akao_irq_frame_counter;
        unk18_val = seq0->unk18;
        ec1c += 1;
        g_akao_irq_frame_counter = ec1c;

        if ((unk18_val == 0) && (D_8004D40C == 0))
        {
            if (g_akao_seq_channel1 != 0)
            {
                if (g_akao_seq_channel1->unk18 != 0)
                {
                    func_80025D98();
                }
            }
        }
        else
        {
            func_80025D98();
        }
    }

    /* Second block */
    {
        AkaoChannelState* ch28 = g_akao_seq_channel1;
        if (ch28 != 0)
        {
            if (ch28->unk4 == 0)
            {
                g_akao_seq_channel1 = 0;
            }
            else if ((g_akao_seq_channel0->unk4 | g_akao_seq_channel0->unk1C) == 0)
            {
                func_80029A0C((s32*)ch28, (s32*)g_akao_seq_channel0, 0x70);
                func_80029A0C((s32*)D_8003EC24, &g_akao_seq_channels, 0x2300);
                {
                    AkaoChannelState* tmp = g_akao_seq_channel1;
                    g_akao_seq_channel1 = 0;
                    tmp->unk5E = 0;
                    tmp->unk4 = 0;
                }
            }
        }
    }

    /* Third conditional */
    if (((D_8004F758 | g_akao_seq_channel0->unk14 | D_8004D408) != 0) ||
        ((g_akao_seq_channel1 != 0) && (g_akao_seq_channel1->unk14 != 0)))
    {
        func_800258B8(D_8004D408);
    }

    /* Fourth conditional */
    if (g_akao_seq_channel0->unk4 != 0)
    {
        func_80029E88(&g_akao_seq_channels, 0);
    }

    /* Fifth conditional */
    if ((g_akao_seq_channel1 != 0) && (g_akao_seq_channel1->unk4 != 0))
    {
        g_akao_seq_channel0 = g_akao_seq_channel1;
        func_80029E88(D_8003EC24, 1);
        g_akao_seq_channel0 = &g_akao_seq_master_state;
    }

    /* SFX channel processing loop */
    if (g_akao_sfx_control.unk0 != 0)
    {
        var_s3 = g_akao_sfx_control.unk0;
        {
            u32 sum = g_akao_sfx_control.unk18 + g_akao_sfx_control.unk16;
            g_akao_sfx_control.unk18 = sum;
            if (((sum & 0xFFFF0000) != 0) || (g_akao_driver_mode_flags & 4))
            {
                g_akao_sfx_control.unk18 = sum & 0xFFFF;

                bitMask = 0x1000;
                channel = &g_sfx_channels[0];

                do
                {
                    if (var_s3 & bitMask)
                    {
                        if (!(g_akao_driver_mode_flags & 2) || (channel->field_28 & 0x02000000))
                        {
                            channel->field_58++;

                            temp_v1_2 = channel->field_66 - 1;
                            channel->field_66 = temp_v1_2;
                            temp_v0_2 = channel->field_68 - 1;
                            channel->field_68 = temp_v0_2;

                            if (temp_v1_2 == 0)
                            {
                                func_8002AD28(channel, bitMask);
                            }
                            else if (temp_v0_2 == 0)
                            {
                                g_akao_sfx_control.unkC |= bitMask;
                                g_akao_sfx_control.unk8 &= ~bitMask;
                            }
                            func_80024660(channel, bitMask, 1);
                        }
                        var_s3 ^= bitMask;
                    }
                    channel++;
                    bitMask <<= 1;
                } while (var_s3 != 0);
            }
        }
    }

    /* Periodic call */
    if (!(g_akao_irq_frame_counter & 3))
    {
        func_80029A8C();
    }

    /* Timing / profiling update for D_8003D160 */
    temp_s5 = GetRCnt(0xF2000002) - temp_s5;
    if (temp_s5 <= 0)
    {
        temp_s5 += 0x44E8;
    }

    {
        s32 d4 = D_8003D160.unk4;
        s32 d8 = D_8003D160.unk8;
        temp_a3 = D_8003D160.unkC;
        D_8003D160.unkC = temp_s5;
        D_8003D160.unk0 = d4;
        temp_v1_3 = d4 + d8 + temp_a3;
        D_8003D160.unk4 = d8;
        D_8003D160.unk8 = temp_a3;
        D_8003EC18 = temp_v1_3 + temp_s5;
    }
}

/**
 * decomp.me (78.79%) https://decomp.me/scratch/9GIhP
 */
u8 func_8002A4E8(Context* arg0)
{
    u8* var_a1 = arg0->unk0;
    u32 var_a2 = arg0->unkD8;

    while (1)
    {
        u8 temp_v1 = *var_a1;

        if (temp_v1 < 0x9A)
        {
            if (temp_v1 >= 0x8F)
            {
                arg0->unk9E &= 0xFFFA;
            }
            return *var_a1;
        }

        if (temp_v1 < 0xA0)
        {
            return 0xA0;
        }

        /* Force reload – matches delay‑slot behaviour */
        temp_v1 = *var_a1;

        {
            u8 lookup = D_8003D1B0[temp_v1 - 0xA0];
            if (lookup != 0)
            {
                var_a1 += lookup;
                continue;
            }

            /* Unified outer switch: 0xC9 .. 0xFE */
            switch (temp_v1)
            {
            case 0xC9:
                goto L_C9_common;

            case 0xCA:
                goto L_CA_common;

            case 0xCB:
            case 0xCD:
            case 0xD1:
            case 0xDB:
                goto L_CB_common;

            case 0xF0:
            case 0xF1:
            case 0xF2:
            case 0xF3:
            case 0xF4:
            case 0xF5:
            case 0xF6:
            case 0xF7:
            case 0xF8:
            case 0xF9:
            case 0xFA:
            case 0xFB:
                return 0x83;

            case 0xFC:
                return 0x84;

            case 0xFD:
                return 0x8F;

            case 0xFE:
            {
                var_a1++;
                temp_v1 = *var_a1;
                lookup = D_8003D210[temp_v1];
                if (lookup != 0)
                {
                    var_a1 += lookup;
                    continue;
                }

                /* Inner switch for 0xFE sub‑opcodes */
                switch (temp_v1 - 6)
                {
                case 0:
                    var_a1++;
                    if (*var_a1 == arg0->unk74[var_a2] + 1)
                    {
                        var_a1++;
                        var_a2 = (var_a2 - 1) & 3;
                        var_a1 += (s16)((var_a1[1] << 8) + var_a1[0]);
                    }
                    else
                    {
                        var_a1 += 3;
                    }
                    continue;

                case 1:
                    var_a1++;
                    var_a1 += (s16)((var_a1[1] << 8) + var_a1[0]);
                    continue;

                case 2:
                    var_a1++;
                    if (g_akao_seq_channel0->unk60 < *var_a1)
                    {
                        var_a1++;
                        var_a1 += 2;
                    }
                    else
                    {
                        var_a1++;
                        var_a1 += (s16)((var_a1[1] << 8) + var_a1[0]);
                    }
                    continue;

                case 3:
                    var_a1 = arg0->unk14;
                    continue;

                case 4:
                    goto L_C9_common;

                case 5:
                    goto L_CB_common;

                case 6:
                    goto L_CA_common;

                case 7:
                case 8:
                case 9:
                    goto labelA;
                }
                continue;
            }

            default:
                goto labelA;
            }

            /* Shared code blocks – each is reached by two different control paths */

        L_C9_common:
            var_a1++;
            if (*var_a1 == arg0->unk74[var_a2] + 1)
            {
                var_a1++;
                var_a2 = (var_a2 - 1) & 3;
            }
            else
            {
                var_a1 = arg0->unk4[var_a2];
            }
            continue;

        L_CB_common:
            arg0->unk9E &= 0xFFFA;
            var_a1++;
            continue;

        L_CA_common:
            if (!(arg0->unk34 & 0x200000))
            {
                var_a1 = arg0->unk4[var_a2];
                continue;
            }

        labelA:
            arg0->unk9E &= 0xFFFA;
            return 0xA0;
        }
    }
}

/**
 * decomp.me (93.41%) https://decomp.me/scratch/0tdrk
 */
void func_8002A6FC(u8* arg0, u32 arg1)
{
    u8* a0 = (u8*)arg0;
    s16 temp_v1;
    u8* a2;
    u8* v1;
    u8* a1;
    u8 new_var2;
    u32 temp_a3;

    temp_v1 = *(s16*)(a0 + 0xEE);

    if (((u32)temp_v1 < arg1) || (temp_v1 == 0xFF))
    {
        a2 = *(u8**)(a0 + 0x18);
        v1 = a2 + 0xD;
        if (a2[0xD] != 0)
        {
            while (*v1 != 0)
            {
                if (arg1 <= (u8)v1[-0xB])
                {
                    break;
                }
                v1 += 8;
                a2 += 8;
            }
        }
    }
    else if (arg1 < *(s16*)((u8*)arg0 + 0xEE))
    {

        a2 = *(u8**)(arg0 + 0x18);
        v1 = a2 + 0xD;
        if (a2[0xD] != 0)
        {
            while (*v1 != 0)
            {
                if (arg1 < (u8)v1[-4])
                {
                    break;
                }
                v1 += 8;
                a2 += 8;
            }
        }
    }
    else
    {
        return;
    }

    temp_a3 = *((u32*)(arg0 + 0x34));
    new_var2 = a2[0];
    a1 = g_akao_articulation_slots + (new_var2 * 0x10);
    *((s16*)(arg0 + 0x6A)) = new_var2;

    *(u32*)(arg0 + 0x104) = *(u32*)(a1 + 0);
    *(u32*)(arg0 + 0x108) = *(u32*)(a1 + 4);

    if (!(temp_a3 & 0x01000000))
    {
        *(u16*)(arg0 + 0x10E) = a2[3] << 8;
    }
    else
    {
        *(u16*)(arg0 + 0x10E) &= 0x7F00;
    }

    *(u16*)(arg0 + 0x10E) |= (*(u16*)(a1 + 12) & 0x80FF);

    if (!(temp_a3 & 0x08000000))
    {
        *(u16*)(arg0 + 0x110) &= 0x201F;
        *(u16*)(arg0 + 0x110) |= (a2[4] << 6);
    }
    else
    {
        *(u16*)(arg0 + 0x110) &= 0x3FDF;
    }

    switch (a2[5])
    {
    case 3:
        *(u16*)(arg0 + 0x110) |= 0x4000;
        break;
    case 5:
        *(u16*)(arg0 + 0x110) |= 0x8000;
        break;
    case 7:
        *(u16*)(arg0 + 0x110) |= 0xC000;
        break;
    }

    if (!(temp_a3 & 0x10000000))
    {
        *(u16*)(arg0 + 0x110) &= 0xFFE0;
        *(u16*)(arg0 + 0x110) |= a2[6];
    }

    *(u16*)(arg0 + 0x110) |= (*(u16*)(a1 + 14) & 0x20);
    *(s16*)(arg0 + 0x112) = (s16)a2[7];
}

/**
 * decomp.me (99.70%) https://decomp.me/scratch/GBbit
 */
s32 func_8002A924(UnkStruct* arg0, s32 arg1, s32 arg2, s32* arg3)
{
    unsigned int new_var;
    s32 tmp3;
    s32 temp_a0;
    s32 temp_a0_3;
    s32 temp_v0;
    s32 var_a0;
    s32 var_a0_2;
    unsigned long new_var2;
    u32 temp_a2;
    u32 temp_lo;
    u32 var_t0;
    u32 var_v0;
    s32 result;
    var_a0 = arg1 - arg0->unkA;
    if (var_a0 < 0)
    {
        s32 tmp;
        tmp = var_a0 + 0xC;
        while (1)
        {
            new_var = tmp >= 0;
            if (new_var)
            {
                break;
            }
            tmp += 0xC;
        }

        var_a0 -= 12;
    }
    new_var2 = var_a0 % 12;
    temp_a0 = new_var2;
    if (arg0->unkB == 0)
    {
        s32 tmp2 = D_8003D24C[temp_a0];
        var_t0 = tmp2 << 8;
    }
    else if (arg0->unkB < 0)
    {
        var_t0 = ((u32)(D_8003D24C[temp_a0] * ((u16)arg0->unkB))) >> 8;
    }
    else
    {
        temp_v0 = D_8003D24C[temp_a0];
        var_t0 = ((u32)(temp_v0 * arg0->unkB)) >> 7;
        var_t0 = var_t0 + (temp_v0 << 8);
    }
    temp_a2 = arg2 & 0xFF;
    if (temp_a2 != 0)
    {
        if (temp_a2 < 0x80U)
        {
            u32 temp_lo2 = var_t0 * temp_a2;
            var_v0 = temp_lo2 >> 7;
        }
        else
        {
            u32 temp_lo2 = var_t0 * temp_a2;
            var_v0 = (temp_lo2 >> 8) - var_t0;
        }
        *arg3 = var_v0;
    }
    if (arg1 < arg0->unkA)
    {
        do
        {
            *arg3 = (u32)(((s32)(*arg3)) >> 1);
            var_t0 = (u32)(((s32)var_t0) >> 1);
            arg1 += 0xC;
        } while (arg1 < arg0->unkA);
    }
    else
    {
        temp_a0_3 = (arg1 - arg0->unkA) / 12;
        temp_a0 = temp_a0_3;
        if (temp_a0_3 != 0)
        {
            temp_lo = temp_a0;
            var_t0 <<= temp_lo;
            *arg3 <<= temp_a0_3;
        }
    }
    temp_a0 = ((s32)var_t0) >> 8;
    result = temp_a0 & 0xFFFF;
    *arg3 = (u32)(((s32)(*arg3)) >> 8);
    return result;
}

/**
 * @brief Initialize an AKAO channel voice slot: bind articulation data,
 *        configure SPU envelope / pitch fields, and fire off the pitch
 *        calculation for the note.
 * @param arg0 Pointer to the channel state block (void* to match codegen).
 * @param arg1 Channel bit-mask used to update the active-channel bitmask.
 * @param arg2 Slot index into the small-slot table (base pointer from
 *             @c g_akao_seq_channel0->unk34).
 * @return Pitch result from @c func_8002A924.
 * @see decomp.me (98.76%) https://decomp.me/scratch/9dRLX
 */
s32 func_8002AAB4(void* arg0, s32 arg1, s32 arg2)
{
    SmallSlot* slot;
    ArticSlot* art;
    u32 temp_a1;
    u16 tmp;
    u32 v1_idx;
    s32 ret;

    u8* base_ptr = g_akao_seq_channel0->unk34;
    u32 chan_unk10 = g_akao_seq_channel0->unk10;
    slot = (SmallSlot*)(base_ptr + (arg2 << 3));
    base_ptr = (u8*)arg0;
    ret = chan_unk10 | arg1;
    v1_idx = g_akao_seq_channel0->unk14 & arg1;
    g_akao_seq_channel0->unk10 = ret;
    if (v1_idx)
    {
        g_akao_seq_channel0->unk18 |= arg1;
    }
    v1_idx = slot->unk0;
    temp_a1 = *((u32*)(((u8*)arg0) + 0x34));
    *((s16*)(((u8*)arg0) + 0x6A)) = (s16)v1_idx;
    art = (ArticSlot*)(g_akao_articulation_slots + v1_idx * 0x10);
    *((u32*)(((u8*)arg0) + 0x104)) = art->unk0;
    *((u32*)(((u8*)arg0) + 0x108)) = art->unk4;
    if (!(temp_a1 & 0x01000000))
    {
        tmp = (u16)(slot->unk2 << 8);
    }
    else
    {
        tmp = (*((u16*)(((u8*)arg0) + 0x10E))) & 0x7F00;
    }
    *((u16*)(((u8*)arg0) + 0x10E)) = tmp;
    tmp = (u16)(slot->unk2 << 8);
    *((u16*)(base_ptr + 0x10E)) |= art->unkC & 0x80FF;
    if (!(temp_a1 & 0x08000000))
    {
        tmp = (*((u16*)(((u8*)arg0) + 0x110))) & 0x201F;
        *((u16*)(((u8*)arg0) + 0x110)) = tmp;
        *((u16*)(((u8*)arg0) + 0x110)) |= slot->unk3 << 6;
    }
    else
    {
        *((u16*)(((u8*)arg0) + 0x110)) &= 0x3FDF;
    }
    switch (slot->unk4)
    {
    case 3:
        *((u16*)(((u8*)arg0) + 0x110)) |= 0x4000;
        break;
    case 5:
        *((u16*)(((u8*)arg0) + 0x110)) |= 0x8000;
        break;
    case 7:
        *((u16*)(((u8*)arg0) + 0x110)) |= 0xC000;
        break;
    }
    if (!(temp_a1 & 0x10000000))
    {
        tmp = (*((u16*)(((u8*)arg0) + 0x110))) & 0xFFE0;
        *((u16*)(((u8*)arg0) + 0x110)) = tmp;
        *((u16*)(((u8*)arg0) + 0x110)) |= slot->unk5;
    }
    *((u16*)(((u8*)arg0) + 0x110)) |= art->unkE & 0x20;
    ret = func_8002A924((UnkStruct*)art, slot->unk1, *((s16*)(((u8*)arg0) + 0xEC)), (s32*)(((u8*)arg0) + 0x54));
    *((s16*)(((u8*)arg0) + 0x112)) = (s16)slot->unk6;
    *((s16*)(((u8*)arg0) + 0x90)) = (s16)(((slot->unk7 & 0x7F) + 0x40) << 8);
    if (slot->unk7 & 0x80)
    {
        g_akao_seq_channel0->unk40 |= arg1;
    }
    else
    {
        u32 target_val = g_akao_seq_channel0->unk40;
        g_akao_seq_channel0->unk40 = target_val & (~arg1);
    }
    g_akao_driver_flags.unk8 |= 0x100;
    return ret;
}