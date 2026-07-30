#include "decomp4.h"
#include "decomp9.h"

/* Defined in the sdata segment (asm/data/sdata.data.s) at their gp-relative
 * addresses near gp_value 0x8003EC14; declared extern here so decomp4 does not
 * emit a second (.bss) definition. */
extern u16 g_akao_irq_frame_counter;
s32 D_8004D40C;
u32 D_8004F758;
extern s32 D_8003EC18;

/** @brief 12-entry semitone pitch-ratio table indexed by note % 12 in akao_compute_pitch. */
extern u32 g_akao_pitch_table[];

/** @brief Pointer table in the initialized data segment, indexed by field 0xAA in func_8002BFAC. */
extern s32 D_8003DD80[];

/**
 * @brief One 8-byte note slot in the per-channel note table at
 *        @c channel->unk34. Each entry encodes the articulation index plus
 *        a packed set of per-note SPU voice parameters used by
 *        @c akao_channel_start_note. Local to this file because it is the
 *        only consumer.
 */
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
    s32 unk0;
    char pad_1[0x80];
    s16 unk84;
    char pad_2[0x100 - 0x88];
    s32 unk100;
} a_struct;

typedef struct
{
    u8* unk0;
    char pad4[0x80];
    u16 unk84;
    u16 unk86;
    char pad88[0xE0 - 0x88];
    s16 unkE0;
} b_struct;

typedef struct
{
    s8* unk0;
    char pad_1[0x48 - 0x4];
    s32 unk48;
    char pad_2[0x8A - 0x4C];
    s16 unk8A;
    s16 unk8C;
    char pad_3[0x100 - 0x90];
    s32 unk100;
} c_struct;

typedef struct
{
    u8* unk0;
    char pad1[0x48 - 4];
    s32 unk48;
    s32 unk4C;
    char pad2[0x8A - 0x50];
    u16 unk8A;
    s16 unk8C;
} d_struct;

typedef struct
{
    u8* unk0;
    char pad1[0x5C - 4];
    s32 unk5C;
    s32 unk60;
    char pad2[0x8C - 0x64];
    u16 unk8C;
} e_struct;

typedef struct
{
    u8* unk0;
    char pad1[0x90 - 0x4];
    s16 unk90;
    s16 unk92;
    char pad2[0x100 - 0x94];
    s32 unk100;
} f_struct;

typedef struct
{
    char pad1[0x34];
    s32 unk34;
} ee_struct;

typedef struct
{
    char pad1[0x34];
    s32 unk34;
} ff_struct;

typedef struct
{
    u8* unk0;
    char pad1[0x90 - 0x4];
    u16 unk90;
    u16 unk92;
    char pad2[0xE8 - 0x94];
    s16 unkE8;
} g_struct;

typedef struct
{
    u8* unk0;
    char pad1[0x96 - 0x4];
    s16 unk96;
} h_struct;

typedef struct
{
    char pad1[0x96];
    u16 unk96;
} i_struct;

typedef struct
{
    char pad1[0x96];
    u16 unk96;
} j_struct;

typedef struct
{
    u8* unk0;
    char pad1[0x34 - 0x4];
    s32 unk34;
    s32 unk38;
    char pad2[0x64 - 0x3C];
    u16 unk64;
    s16 unk66;
    s16 unk68;
    s16 unk6A;
    char pad3[0x112 - 0x6C];
    s16 unk112;
} k_struct;

typedef struct
{
    u8* unk0;
    char pad1[0x34 - 0x4];
    s32 unk34;
    char pad2[0x6A - 0x38];
    s16 unk6A;
    char pad3[0x112 - 0x6C];
    s16 unk112;
} l_struct;

typedef struct
{
    u8* unk0;
    char pad1[0x18 - 0x4];
    u32 unk18;
    char pad2[0x34 - 0x1C];
    s32 unk34;
    char pad3[0xEE - 0x38];
    u16 unkEE;
    char pad4[0x112 - 0xF0];
    s16 unk112;
} m_struct;

typedef struct
{
    char pad0[0x34];
    s32 unk34;
    char pad1[0x6A - 0x38];
    u16 unk6A;
    char pad2[0x100 - 0x6C];
    s32 unk100;
    char pad3[0x10E - 0x104];
    u16 unk10E;
    u16 unk110;
} n_struct;

typedef struct
{
    u8* unk0;
    char pad1[0xEA - 0x4];
    s16 unkEA;
} o_struct;

typedef struct
{
    u8* unk0;
    char pad1[0x98 - 0x4];
    u16 unk98;
    char pad2[0xF0 - 0x9A];
    s16 unkF0;
} p_struct;

typedef struct
{
    u8* unk0;
    char pad1[0x9A - 0x4];
    u16 unk9A;
    u16 unk9C;
    u16 unk9E;
    char pad2[0xF2 - 0xA0];
    u16 unkF2;
} q_struct;

typedef struct
{
    u8* unk0;
    char pad1[0x2C - 0x4];
    s32 unk2C;
    char pad2[0x54 - 0x30];
    s32 unk54;
    char pad3[0xEC - 0x58];
    s16 unkEC;
    char pad4[0x100 - 0xEE];
    s32 unk100;
} r_struct;

typedef struct
{
    u8* unk0;
    char pad1[0x1C - 0x4];
    s32 unk1C;
    char pad2[0x2C - 0x20];
    u16 unk2C;
    char pad3[0x34 - 0x2E];
    s32 unk34;
    char pad4[0x64 - 0x38];
    u16 unk64;
    char pad5[0xA2 - 0x66];
    u16 unkA2;
    u16 unkA4;
    s16 unkA6;
    u16 unkA8;
    u16 unkAA;
    s16 unkAC;
    u16 unkAE;
} s_struct;

typedef struct
{
    u8* unk0;
    char pad1[0x2C - 0x4];
    s32 unk2C;
    char pad2[0xAC - 0x30];
    s16 unkAC;
    u16 unkAE;
} t_struct;

typedef struct
{
    u8* unk0;
    char pad1[0xAE - 0x4];
    u16 unkAE;
    s16 unkB0;
    s16 unkB2;
} u_struct;

typedef struct
{
    char pad0[0x34];
    s32 unk34;
    char pad1[0xF4 - 0x38];
    u16 unkF4;
    char pad2[0x100 - 0xF6];
    s32 unk100;
} v_struct;

typedef struct
{
    u8* unk0;
    char pad1[0x20 - 0x4];
    s32 unk20;
    char pad2[0x34 - 0x24];
    s32 unk34;
    char pad3[0x64 - 0x38];
    u16 unk64;
    char pad4[0xB6 - 0x66];
    u16 unkB6;
    u16 unkB8;
    s16 unkBA;
    u16 unkBC;
    u16 unkBE;
    s16 unkC0;
} w_struct;

typedef struct
{
    u8* unk0;
    char pad1[0xC0 - 0x4];
    s16 unkC0;
} x_struct;

typedef struct
{
    u8* unk0;
    char pad1[0xC0 - 0x4];
    u16 unkC0;
    s16 unkC2;
    s16 unkC4;
} y_struct;

typedef struct
{
    char pad0[0x34];
    s32 unk34;
    char pad1[0xF6 - 0x38];
    u16 unkF6;
    char pad2[0x100 - 0xF8];
    s32 unk100;
} z_struct;

typedef struct
{
    u8* unk0;
    char pad1[0x24 - 0x4];
    s32 unk24;
    char pad2[0x34 - 0x28];
    s32 unk34;
    char pad3[0xC8 - 0x38];
    u16 unkC8;
    s16 unkCA;
    u16 unkCC;
} aa_struct;

typedef struct
{
    u8* unk0;
    char pad1[0xCE - 0x4];
    s16 unkCE;
} ab_struct;

typedef struct
{
    u8* unk0;
    char pad1[0xCE - 0x4];
    u16 unkCE;
    s16 unkD0;
    s16 unkD2;
} ac_struct;

typedef struct
{
    char pad0[0x34];
    s32 unk34;
    char pad1[0xF8 - 0x38];
    u16 unkF8;
    char pad2[0x100 - 0xFA];
    s32 unk100;
} ad_struct;

typedef struct
{
    char pad0[0x64];
    u16 unk64;
    char pad1[0xD4 - 0x66];
    u16 unkD4;
} ae_struct;

typedef struct
{
    char pad0[0x34];
    s32 unk34;
    char pad1[0x64 - 0x38];
    u16 unk64;
} af_struct;

typedef struct
{
    char pad0[0x64];
    u16 unk64;
    char pad1[0xD6 - 0x66];
    u16 unkD6;
} ag_struct;

typedef struct
{
    char pad0[0x9E];
    u16 unk9E;
} ah_struct;

typedef struct
{
    char pad0[0x64];
    u16 unk64;
    char pad1[0x9E - 0x66];
    u16 unk9E;
} ai_struct;

typedef struct
{
    u8* unk0;
    u8 pad1[0x64 - 0x4];
    u16 unk64;
} argst1;

typedef struct
{
    u8* unk0;                  /* 0x00 */
    u8 pad_04[0x30];           /* 0x04 - 0x33 */
    u32 unk34;                 /* 0x34 */
    u8 pad_38[0x100 - 0x38];   /* 0x38 - 0xFF */
    u32 unk100;                /* 0x100 */
    u8 pad_104[0x10E - 0x104]; /* 0x104 - 0x10D */
    u16 unk10E;                /* 0x10E */
} SomeStruct;

typedef struct
{
    u8* unk0;
    u8 pad1[0x100 - 0x4];
    s32 unk100;
    u8 pad2[0x10E - 0x104];
    u16 unk10E;
} s_8002C79C;

extern s16 D_8004D428[];

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
 * @brief Word-wise block copy (4 words per iteration, then a 1-word tail).
 * @param src Source buffer.
 * @param dst Destination buffer.
 * @param num_bytes Byte count; converted to a word count internally.
 * @see decomp.me (100%) https://decomp.me/scratch/TxNq3
 */
void akao_copy_bytes(s32* src, s32* dst, u32 num_bytes)
{

    num_bytes >>= 2;

    while ((num_bytes >> 2) != 0)
    {

        s32 second = src[1];
        s32 third = src[2];
        s32 fourth = src[3];
        s32 first = src[0];

        dst[0] = first;
        dst[1] = second;
        dst[2] = third;
        dst[3] = fourth;

        src += 4;
        dst += 4;
        num_bytes -= 4;
    }

    while (num_bytes != 0)
    {
        *dst = *src;
        src++;
        dst++;
        num_bytes--;
    }
}

/**
 * decomp.me (86.49%) https://decomp.me/scratch/07M97
 */
void akao_tick_fades(void)
{
    AkaoXaTracker* xa;
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
        akao_apply_cdvol_to_spu();
    }
    xa = &g_akao_xa_tracker;
    if ((xa->unkC != 0) && (xa->unk48 != 0))
    {
        xa->unk48--;
        temp = xa->unk40 + xa->unk44;
        if ((temp & 0xFF00) != (xa->unk40 & 0xFF00))
        {
            if (D_8004F754 & 2)
            {
                temp_s0 = (xa->unk40 * D_8003D47C) >> 16;
                spu_set_voice_volume(xa->unk10, temp_s0, temp_s0, 0);
                t0 = temp_s0;
                t1 = temp_s0;
            }
            else
            {
                t1 = (temp_s0 = (temp << 15) >> 16);
                spu_set_voice_volume(xa->unk10, temp_s0, 0, 0);
                t0 = 0;
            }
            spu_set_voice_volume(xa->unk10 + 1, t0, t1, 0);
        }
        g_akao_xa_pan_current = temp & 0xFFFF;
    }
    if (g_akao_masterpan_fade_ticks != 0)
    {
        g_akao_masterpan_fade_ticks--;
        g_akao_masterpan_acc += g_akao_masterpan_step;
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
                func_80026E8C(g_akao_seq_channel1, (void*)((u32)g_akao_pending_channels), (u32)g_akao_seq_channel1);
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
 * @brief Advance the tempo accumulator and, on each musical tick, step every
 *        active channel's opcode/timer state.
 * @param channel_base Base address of the channel array to iterate (primary
 *        g_akao_seq_channels or the pending/secondary set).
 * @param is_secondary 0 for the primary sequence pass (updates pending-tick
 *        and driver-dirty state), non-zero for the secondary channel1 pass.
 * @return The active-channel bitmask (g_akao_seq_channel0->unk4).
 * @see decomp.me (84.65%) https://decomp.me/scratch/XMCUh
 */
s32 akao_seq_tick_channels(s32 channel_base, s32 is_secondary)
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

        var_s2 = channel_base;
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
                        akao_seq_step_opcode(var_s2, var_s1);
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
                if (is_secondary == 0)
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
                        if (is_secondary == 0)
                        {
                            if (g_akao_seq_pending_ticks != 0)
                            {
                                g_akao_seq_pending_ticks -= 1;
                            }
                        }
                    }
                }
            }

            if (is_secondary == 0)
            {
                var_s2 = channel_base;
                if (g_akao_seq_pending_ticks != 0)
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
 * decomp.me (95.79%) https://decomp.me/scratch/ICO2k
 */
void akao_irq_handler(void)
{
    s32 temp_s5;
    s32 var_s3;
    s32 temp_a3;
    s32 temp_v1_3;
    u16 temp_v0_2;
    u16 temp_v1_2;
    SfxChannel* channel;
    u32 bitMask;
    AkaoChannelState* new_var;

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
                akao_copy_bytes((s32*)ch28, (s32*)g_akao_seq_channel0, 0x70);
                akao_copy_bytes((s32*)g_akao_pending_channels, &g_akao_seq_channels, 0x2300);
                {
                    AkaoChannelState* tmp = g_akao_seq_channel1;
                    g_akao_seq_channel1 = 0;
                    tmp->unk5E = 0;
                    tmp->unk4 = 0;
                }
            }
        }
    }

    new_var = &g_akao_seq_master_state;

    /* Third conditional */
    if (((D_8004F758 | g_akao_seq_channel0->unk14 | D_8004D408) != 0) || ((g_akao_seq_channel1 != 0) && (g_akao_seq_channel1->unk14 != 0)))
    {
        func_800258B8(D_8004D408);
    }

    /* Fourth conditional */
    if (g_akao_seq_channel0->unk4 != 0)
    {
        akao_seq_tick_channels(&g_akao_seq_channels, 0);
    }

    /* Fifth conditional */
    if ((g_akao_seq_channel1 != 0) && (g_akao_seq_channel1->unk4 != 0))
    {
        g_akao_seq_channel0 = g_akao_seq_channel1;
        akao_seq_tick_channels(g_akao_pending_channels, 1);
        g_akao_seq_channel0 = new_var;
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
                                akao_seq_step_opcode(channel, bitMask);
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
        akao_tick_fades();
    }

    /* Timing / profiling update for D_8003D160 */
    temp_s5 = GetRCnt(0xF2000002) - temp_s5;
    if (temp_s5 <= 0)
    {
        temp_s5 += 0x44E8;
    }

    var_s3 = temp_s5;

    {
        s32 d4 = D_8003D160.unk4;
        s32 d8 = D_8003D160.unk8;
        temp_a3 = D_8003D160.unkC;
        D_8003D160.unkC = var_s3;
        D_8003D160.unk0 = d4;
        temp_v1_3 = d4 + d8 + temp_a3;
        D_8003D160.unk4 = d8;
        D_8003D160.unk8 = temp_a3;
        D_8003EC18 = temp_v1_3 + var_s3;
    }
}

/** @brief Operand-length table for extended (0xFE-prefixed) opcodes; 0 = needs special handling. */
extern u8 g_akao_opcode_len_table_ext[];
/** @brief Operand-length table for primary opcodes 0xA0..0xFF; 0 = needs special handling. */
extern u8 g_akao_opcode_len_table[];

/**
 * decomp.me (78.79%) https://decomp.me/scratch/52mKD
 */
u8 akao_seq_skip_to_next_note(AkaoChannelState* arg0)
{
    u8* var_a1 = (u8*)arg0->flags;
    u8* new_var;
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
            new_var = var_a1;
            return *new_var;
        }
        if (temp_v1 < 0xA0)
        {
            return 0xA0;
        }
        temp_v1 = *new_var;
        {
            u8 lookup = g_akao_opcode_len_table[temp_v1 - 0xA0];
            if (0 != lookup)
            {
                var_a1 += lookup;
                continue;
            }
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
                lookup = g_akao_opcode_len_table_ext[temp_v1];
                if (lookup != (0 & 0xFFFFu))
                {
                    var_a1 += lookup;
                    continue;
                }
                switch (temp_v1 - 6)
                {
                case 0:
                    var_a1++;
                    if ((*var_a1) == (arg0->unk74[var_a2] + 1))
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
                    continue;
                    var_a1 += (s16)((var_a1[1] << 8) + var_a1[0]);

                case 2:
                    var_a1++;
                    do
                    {
                        if (g_akao_seq_channel0->unk60 < (*var_a1))
                        {
                            var_a1++;
                            var_a1 += 2;
                        }
                        else
                        {
                            var_a1++;
                            var_a1 += (s16)((var_a1[1] << (8 ^ 0)) + var_a1[0]);
                        }
                    } while (0);
                    continue;

                case 3:
                    var_a1 = (u8*)arg0->unk14;
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

        L_C9_common:
            var_a1++;

            if ((*var_a1) == (arg0->unk74[var_a2] + 1))
            {
                var_a1++;
                var_a2 = (var_a2 - 1) & 3;
            }
            else
            {
                var_a1 = ((u8**)&arg0->unk4)[var_a2];
            }
            continue;
        L_CB_common:
            arg0->unk9E &= 0xFFFA;

            var_a1++;
            continue;
        L_CA_common:
            if (!((u32)arg0->unk34 & 0x200000))
            {
                var_a1 = ((u8**)&arg0->unk4)[var_a2];
                continue;
            }

        labelA:
            arg0->unk9E &= 0xFFFA;

            return 0xA0;
        }
    }
}

/**
 * @brief Select the articulation entry whose key range contains @p key and
 *        load its SPU envelope / pitch fields into the channel.
 * @param channel Pointer to the channel state block (raw u8* for offset math).
 * @param key Note/key being bound; chooses the entry within the channel's
 *        articulation map (pointer at offset 0x18).
 * @param next_opcode Next-note opcode from akao_seq_skip_to_next_note; passed
 *        by the caller but not consumed by the current body. TODO: the
 *        original likely uses this; reconstructing it may close the 93% gap.
 * @see decomp.me (93.41%) https://decomp.me/scratch/0tdrk
 */
void akao_bind_articulation_for_key(u8* channel, u32 key, s32 next_opcode)
{
    u8* a0 = (u8*)channel;
    s16 temp_v1;
    u8* a2;
    u8* v1;
    u8* a1;
    u8 new_var2;
    u32 temp_a3;

    temp_v1 = *(s16*)(a0 + 0xEE);

    if (((u32)temp_v1 < key) || (temp_v1 == 0xFF))
    {
        a2 = *(u8**)(a0 + 0x18);
        v1 = a2 + 0xD;
        if (a2[0xD] != 0)
        {
            while (*v1 != 0)
            {
                if (key <= (u8)v1[-0xB])
                {
                    break;
                }
                v1 += 8;
                a2 += 8;
            }
        }
    }
    else if (key < *(s16*)((u8*)channel + 0xEE))
    {

        a2 = *(u8**)(channel + 0x18);
        v1 = a2 + 0xD;
        if (a2[0xD] != 0)
        {
            while (*v1 != 0)
            {
                if (key < (u8)v1[-4])
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

    temp_a3 = *((u32*)(channel + 0x34));
    new_var2 = a2[0];
    a1 = g_akao_articulation_slots + (new_var2 * 0x10);
    *((s16*)(channel + 0x6A)) = new_var2;

    *(u32*)(channel + 0x104) = *(u32*)(a1 + 0);
    *(u32*)(channel + 0x108) = *(u32*)(a1 + 4);

    if (!(temp_a3 & 0x01000000))
    {
        *(u16*)(channel + 0x10E) = a2[3] << 8;
    }
    else
    {
        *(u16*)(channel + 0x10E) &= 0x7F00;
    }

    *(u16*)(channel + 0x10E) |= (*(u16*)(a1 + 12) & 0x80FF);

    if (!(temp_a3 & 0x08000000))
    {
        *(u16*)(channel + 0x110) &= 0x201F;
        *(u16*)(channel + 0x110) |= (a2[4] << 6);
    }
    else
    {
        *(u16*)(channel + 0x110) &= 0x3FDF;
    }

    switch (a2[5])
    {
    case 3:
        *(u16*)(channel + 0x110) |= 0x4000;
        break;
    case 5:
        *(u16*)(channel + 0x110) |= 0x8000;
        break;
    case 7:
        *(u16*)(channel + 0x110) |= 0xC000;
        break;
    }

    if (!(temp_a3 & 0x10000000))
    {
        *(u16*)(channel + 0x110) &= 0xFFE0;
        *(u16*)(channel + 0x110) |= a2[6];
    }

    *(u16*)(channel + 0x110) |= (*(u16*)(a1 + 14) & 0x20);
    *(s16*)(channel + 0x112) = (s16)a2[7];
}

/**
 * @brief Compute the SPU pitch (and a scaled volume) for a note relative to
 *        an articulation's base note, octave-shifting both as needed.
 * @param art Articulation providing the base note / fine-tune (adsr halves).
 * @param note Target note (semitones) to sound.
 * @param vol Note volume; low 8 bits scale the output volume.
 * @param out_vol Receives the computed volume, octave-shifted to match.
 * @return 16-bit SPU pitch value.
 * @see decomp.me (100%) https://decomp.me/scratch/mWTad
 */
s32 akao_compute_pitch(AkaoArticulation* arg0, s32 arg1, s32 arg2, s32* arg3)
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
    var_a0 = arg1 - arg0->adsr.half.hi;
    if (var_a0 < 0)
    {
        do
        {
            var_a0 = var_a0 + 0xC;
        } while (var_a0 < 0);
    }
    new_var2 = var_a0 % 12;
    temp_a0 = new_var2;
    if (arg0->adsr.half.lo == 0)
    {
        s32 tmp2 = g_akao_pitch_table[temp_a0];
        var_t0 = tmp2 << 8;
    }
    else if (arg0->adsr.half.lo < 0)
    {
        var_t0 = ((u32)(g_akao_pitch_table[temp_a0] * ((u16)arg0->adsr.half.lo))) >> 8;
    }
    else
    {
        temp_v0 = g_akao_pitch_table[temp_a0];
        var_t0 = ((u32)(temp_v0 * arg0->adsr.half.lo)) >> 7;
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
    if (arg1 < arg0->adsr.half.hi)
    {
        do
        {
            *arg3 = (u32)(((s32)(*arg3)) >> 1);
            var_t0 = (u32)(((s32)var_t0) >> 1);
            arg1 += 0xC;
        } while (arg1 < arg0->adsr.half.hi);
    }
    else
    {
        temp_a0_3 = (arg1 - arg0->adsr.half.hi) / 12;
        temp_a0 = temp_a0_3;
        var_a0 = temp_a0_3;
        if (var_a0 != 0)
        {
            temp_lo = temp_a0;
            var_t0 <<= temp_lo;
            *arg3 <<= var_a0;
        }
    }
    var_t0 = ((s32)var_t0) >> 8;
    result = var_t0 & 0xFFFF;
    *arg3 = (u32)(((s32)(*arg3)) >> 8);
    return result;
}

/**
 * @brief Initialize an AKAO channel voice slot: bind articulation data,
 *        configure SPU envelope / pitch fields, and fire off the pitch
 *        calculation for the note.
 * @param channel Pointer to the channel state block (void* to match codegen).
 * @param channel_mask Channel bit-mask used to update the active-channel bitmask.
 * @param slot_idx Slot index into the small-slot table (base pointer from
 *             @c g_akao_seq_channel0->unk34).
 * @return Pitch result from @c akao_compute_pitch.
 * @see decomp.me (98.76%) https://decomp.me/scratch/9dRLX
 */
s32 akao_channel_start_note(void* channel, s32 channel_mask, s32 slot_idx)
{
    SmallSlot* slot;
    AkaoArticulation* art;
    u32 temp_a1;
    u16 tmp;
    u32 v1_idx;
    s32 ret;

    u8* base_ptr = g_akao_seq_channel0->unk34;
    u32 chan_unk10 = g_akao_seq_channel0->unk10;
    slot = (SmallSlot*)(base_ptr + (slot_idx << 3));
    base_ptr = (u8*)channel;
    ret = chan_unk10 | channel_mask;
    v1_idx = g_akao_seq_channel0->unk14 & channel_mask;
    g_akao_seq_channel0->unk10 = ret;
    if (v1_idx)
    {
        g_akao_seq_channel0->unk18 |= channel_mask;
    }
    v1_idx = slot->unk0;
    temp_a1 = *((u32*)(((u8*)channel) + 0x34));
    *((s16*)(((u8*)channel) + 0x6A)) = (s16)v1_idx;
    art = (AkaoArticulation*)(g_akao_articulation_slots + v1_idx * 0x10);
    *((u32*)(((u8*)channel) + 0x104)) = art->sample_addr;
    *((u32*)(((u8*)channel) + 0x108)) = art->loop_addr;
    if (!(temp_a1 & 0x01000000))
    {
        tmp = (u16)(slot->unk2 << 8);
    }
    else
    {
        tmp = (*((u16*)(((u8*)channel) + 0x10E))) & 0x7F00;
    }
    *((u16*)(((u8*)channel) + 0x10E)) = tmp;
    tmp = (u16)(slot->unk2 << 8);
    *((u16*)(base_ptr + 0x10E)) |= art->pitch_misc.half.lo & 0x80FF;
    if (!(temp_a1 & 0x08000000))
    {
        tmp = (*((u16*)(((u8*)channel) + 0x110))) & 0x201F;
        *((u16*)(((u8*)channel) + 0x110)) = tmp;
        *((u16*)(((u8*)channel) + 0x110)) |= slot->unk3 << 6;
    }
    else
    {
        *((u16*)(((u8*)channel) + 0x110)) &= 0x3FDF;
    }
    switch (slot->unk4)
    {
    case 3:
        *((u16*)(((u8*)channel) + 0x110)) |= 0x4000;
        break;
    case 5:
        *((u16*)(((u8*)channel) + 0x110)) |= 0x8000;
        break;
    case 7:
        *((u16*)(((u8*)channel) + 0x110)) |= 0xC000;
        break;
    }
    if (!(temp_a1 & 0x10000000))
    {
        tmp = (*((u16*)(((u8*)channel) + 0x110))) & 0xFFE0;
        *((u16*)(((u8*)channel) + 0x110)) = tmp;
        *((u16*)(((u8*)channel) + 0x110)) |= slot->unk5;
    }
    *((u16*)(((u8*)channel) + 0x110)) |= art->pitch_misc.half.hi & 0x20;
    ret = akao_compute_pitch(art, slot->unk1, *((s16*)(((u8*)channel) + 0xEC)), &((AkaoChannelState*)channel)->unk54);
    *((s16*)(((u8*)channel) + 0x112)) = (s16)slot->unk6;
    *((s16*)(((u8*)channel) + 0x90)) = (s16)(((slot->unk7 & 0x7F) + 0x40) << 8);
    if (slot->unk7 & 0x80)
    {
        g_akao_seq_channel0->unk40 |= channel_mask;
    }
    else
    {
        u32 target_val = g_akao_seq_channel0->unk40;
        g_akao_seq_channel0->unk40 = target_val & (~channel_mask);
    }
    g_akao_driver_flags.unk8 |= 0x100;
    return ret;
}

/** @brief Primary opcode dispatch table indexed by (opcode - 0xA0). */
extern void (*g_akao_opcode_handlers[])(AkaoChannelState*, s32);
/** @brief Extended (0xFE-prefixed) opcode dispatch table indexed by the following byte. */
extern void (*g_akao_opcode_handlers_ext[])(AkaoChannelState*, s32);
/** @brief Default note-duration (gate-time) table indexed by opcode % 11. */
extern u16 g_akao_note_duration_table[];
extern u8 D_8003D27C[];
extern s32 D_8003DD80[];

/**
 * decomp.me (89.58%) https://decomp.me/scratch/P4H6n
 */
void akao_seq_step_opcode(AkaoChannelState* arg0, s32 arg1)
{
    s32 sp10;
    void (**var_v0)(AkaoChannelState*, s32);
    s16 temp_v0_5;
    s32 temp_a2_2;
    s32 temp_s1_2;
    s32 temp_v1_3;
    s32 var_a2;
    s32 var_s1_2;
    s32 var_v0_3;
    u16 temp_a0;
    u16 temp_a0_2;
    s32 temp_s1;
    u16 temp_v0_3;
    u16 temp_v0_4;
    u16 temp_v1_2;
    u16 temp_v1_4;
    u32 temp_v1_5;
    u16 var_v1;
    u32 temp_a0_3;
    u32 temp_a2;
    u32 temp_lo;
    u32 temp_v0;
    u32 temp_v0_2;
    u32 temp_v1;
    u32 temp_v1_7;
    u32 var_s1;
    u8* temp_v1_6;

    do
    {
        var_s1 = *(*(u8**)&arg0->flags)++;

        if (var_s1 >= 0xA0U)
        {
            temp_v1 = var_s1 - 0xF0;
            if (var_s1 == 0xFE)
            {
                var_v0 = &g_akao_opcode_handlers_ext[*(*(u8**)&arg0->flags)++];
            }
            else if (temp_v1 < 0xEU)
            {
                var_s1 = temp_v1 * 0xB;
                arg0->unk66 = *(*(u8**)&arg0->flags)++;
                goto skip_call;
            }
            else
            {
                if (var_s1 == 0xFF)
                {
                    var_s1 = 0xA0;
                }
                else if (var_s1 == 0xCA)
                {
                    if ((u32)arg0->unk34 & 0x200000)
                    {
                        var_s1 = 0xA0;
                        g_akao_sfx_control.unkC |= arg1;
                    }
                }
                var_v0 = &g_akao_opcode_handlers[var_s1 - 0xA0];
            }
            (*var_v0)(arg0, arg1);
        skip_call:;
        }
        arg0->unk72++;
    } while (var_s1 >= 0xA1U);

    if (var_s1 == 0xA0)
    {
        if (arg0->unk64 == 0)
        {
            g_akao_seq_channel0->unk18 |= arg1;
        }
    }
    else
    {
        temp_a2 = akao_seq_skip_to_next_note(arg0) & 0xFF;
        temp_v1_2 = arg0->unkDE;
        if ((s16)arg0->unkDE != 0)
        {
            arg0->unk68 = temp_v1_2;
            arg0->unk66 = temp_v1_2;
        }
        if (arg0->unk66 != 0)
        {
            if ((temp_a2 >= 0x8FU) || ((temp_a2 < 0x84U) && !(arg0->unk9E & 5)))
            {
                arg0->unk68 -= 2;
            }
        }
        else
        {
            var_v1 = g_akao_note_duration_table[var_s1 % 11];
            arg0->unk66 = var_v1;
            if (((temp_a2 - 0x84) >= 0xBU) && !(arg0->unk9E & 5))
            {
                var_v1 -= 2;
            }
            arg0->unk68 = var_v1;
        }
        if ((arg0->unk64 == 0) && ((u32)arg0->unk34 & 0x40))
        {
            arg0->unk68 = arg0->unk66;
        }
        arg0->unkDC = arg0->unk66;
        arg0->unk100 |= 0x4000;
        if (var_s1 >= 0x8FU)
        {
            if (arg0->unk64 == 0)
            {
                g_akao_seq_channel0->unk14 &= ~arg1;
                if (*(volatile u32*)&arg0->unkFC < 0x18U)
                {
                    g_akao_seq_channel0->unk18 |= arg1;
                }
            }
            arg0->unk9C = 0U;
            arg0->unkF4 = 0;
            arg0->unkF6 = 0;
            arg0->unk9E &= 0xFFFD;
            return;
        }
        if (var_s1 < 0x84U)
        {
            temp_v1_3 = (s32)arg0->unk34;
            temp_s1 = (var_s1 / 11) + (arg0->unk96 * 0xC);
            if (temp_v1_3 & 8)
            {
                var_a2 = akao_channel_start_note(arg0, arg1, temp_s1);
            }
            else
            {
                if (!(arg0->unk9E & 2))
                {
                    if (arg0->unk64 == 0)
                    {
                        if (temp_v1_3 & 0x1000)
                        {
                            akao_bind_articulation_for_key(arg0, temp_s1, temp_a2);
                        }
                        g_akao_seq_channel0->unk10 |= arg1;
                        if ((g_akao_seq_channel0->unk14 & arg1) && (*(volatile u32*)&arg0->unkFC < 0x18U))
                        {
                            g_akao_seq_channel0->unk18 |= arg1;
                        }
                        temp_a0 = arg0->unk8C;
                        if (temp_a0 != 0)
                        {
                            arg0->unk8A = temp_a0;
                            arg0->unk48 = *(s32*)&arg0->unk5C;
                            arg0->unk4C = arg0->unk60;
                        }
                    }
                    else
                    {
                        g_akao_sfx_control.unk4 |= arg1;
                    }
                    arg0->unk94 = 0U;
                }
                temp_v1_4 = arg0->unk9C;
                if ((temp_v1_4 != 0) && (arg0->unk9A != 0))
                {
                    arg0->unk98 = temp_v1_4;
                    var_s1_2 = arg0->unk9A + arg0->unkF2;
                    temp_a0_2 = arg0->unkF2;
                    arg0->unkF0 = ((arg0->unkEA + temp_s1) - arg0->unk9A) - temp_a0_2;
                    arg0->unkEE = arg0->unk9A - (arg0->unkEA - temp_a0_2);
                }
                else
                {
                    arg0->unkEE = temp_s1;
                    var_s1_2 = temp_s1 + (s16)arg0->unkEA;
                }
                var_a2 = akao_compute_pitch((AkaoArticulation*)((arg0->unk6A * 0x10) + g_akao_articulation_slots), var_s1_2, arg0->unkEC, &arg0->unk54);
                temp_v1_5 = arg0->unkDA;
                if (temp_v1_5 != 0)
                {
                    temp_a0_3 = (u32)(var_a2 * temp_v1_5) >> 8;
                    sp10 = temp_a0_3;
                    temp_v1_6 = D_8003D27C + g_akao_cdvol_tick;
                    temp_lo = temp_a0_3 * temp_v1_6[0];
                    sp10 = temp_lo;
                    temp_v0 = temp_lo >> 9;
                    if (temp_v1_6[0] & 0x80)
                    {
                        sp10 = temp_v0;
                        var_a2 -= temp_v0;
                    }
                    else
                    {
                        temp_v0_2 = temp_lo >> 7;
                        sp10 = temp_v0_2;
                        var_a2 += temp_v0_2;
                    }
                }
            }
            arg0->pitch = var_a2;
            if (arg0->unk64 == 0)
            {
                g_akao_seq_channel0->unk14 |= arg1;
            }
            else
            {
                g_akao_sfx_control.unk8 |= arg1;
            }
            arg0->unk100 |= 0x13;
            temp_s1_2 = (s32)arg0->unk34;
            var_v0_3 = temp_s1_2 & 2;
            if (temp_s1_2 & 1)
            {
                temp_v0_3 = arg0->unkAE;
                temp_v1_7 = (u32)(temp_v0_3 & 0x7F00) >> 8;
                if (!(temp_v0_3 & 0x8000))
                {
                    temp_lo = temp_v1_7 * ((u32)(var_a2 * 0xF) >> 8);
                }
                else
                {
                    temp_lo = temp_v1_7 * var_a2;
                }
                arg0->unkAC = (s16)(temp_lo >> 7);
                if (!(arg0->unk9E & 2))
                {
                    temp_v0_3 = arg0->unkA2;
                    arg0->unk1C = D_8003DD80[arg0->unkAA];
                    arg0->unkA8 = 1;
                    arg0->unkA4 = temp_v0_3;
                }
                var_v0_3 = temp_s1_2 & 2;
            }
            if ((var_v0_3 != 0) && !(arg0->unk9E & 2))
            {
                temp_v0_3 = arg0->unkB6;
                arg0->unk20 = D_8003DD80[arg0->unkBE];
                arg0->unkBC = 1;
                arg0->unkB8 = temp_v0_3;
            }
            arg0->unkF4 = 0;
            arg0->unkF6 = 0;
            arg0->unk30 = 0;
        }
        temp_v0_4 = arg0->unk9E;
        arg0->unk9E = (temp_v0_4 & 0xFFFD) | ((temp_v0_4 & 1) * 2);
        if ((s16)arg0->unkF0 != 0)
        {
            temp_v0_5 = arg0->unkEE + arg0->unkF0;
            arg0->unkEE = temp_v0_5;
            temp_a2_2 =
                akao_compute_pitch((AkaoArticulation*)((arg0->unk6A * 0x10) + g_akao_articulation_slots), temp_v0_5 + (s16)arg0->unkEA, arg0->unkEC, &sp10)
                << 0x10;
            arg0->unk94 = arg0->unk98;
            arg0->unkF0 = 0U;
            arg0->unk50 = (s32)((s32)(temp_a2_2 - ((arg0->pitch << 0x10) + arg0->unk30)) / (s32)arg0->unk94);
        }
        arg0->unk9A = arg0->unkEE;
        arg0->unkF2 = arg0->unkEA;
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/Fr8N0
 */
void akao_channel_load_articulation_fields(AkaoChannelState* arg0, AkaoArticulation* arg1, s32 arg2)
{
    u16 tmp_c, tmp_e;
    s32 tmp_4;
    s32 old_val;

    arg0->spu_sample_addr = arg2;
    tmp_4 = arg1->loop_addr;
    arg0->spu_loop_addr = tmp_4;

    tmp_c = arg1->pitch_misc.half.lo;
    arg0->unk10E = tmp_c; /* store unk10E early */

    old_val = arg0->unk100; /* load unk100 after that store */
    tmp_e = arg1->pitch_misc.half.hi;

    arg0->unk100 = old_val | 0x1FF80; /* OR and write back */
    arg0->unk110 = tmp_e;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/1RRHh
 */
void akao_channel_set_articulation(AkaoChannelState* arg0, s32 arg1)
{
    AkaoArticulation* temp_a1;

    arg0->unk6A = arg1;
    temp_a1 = (AkaoArticulation*)(g_akao_articulation_slots + (arg1 << 4));
    akao_channel_load_articulation_fields(arg0, temp_a1, *(s32*)temp_a1);
}

/**
 * @brief Clear the given channel bits across all g_akao_sfx_control bitmasks
 *        and zero two fields of the channel object.
 * @param channel Pointer to the SFX channel object whose 0x28/0x3C fields are cleared.
 * @param release_mask Bit-mask of channels to release.
 * @see decomp.me (100%) https://decomp.me/scratch/67vx9
 */
void akao_sfx_release_channels(void* channel, u32 release_mask)
{
    u32 mask = ~release_mask;

    /* Clear bits in the specified fields of g_akao_sfx_control */
    g_akao_sfx_control.unk0 &= mask;
    g_akao_sfx_control.unk10 &= mask;
    g_akao_sfx_control.unk1C &= mask;
    g_akao_sfx_control.unk20 &= mask;
    g_akao_sfx_control.unk24 &= mask;
    g_akao_sfx_control.unk4 &= mask;
    g_akao_sfx_control.unk8 &= mask;

    /* Zero out two fields in the object pointed to by channel */
    *(u32*)((u8*)channel + 0x28) = 0;
    *(u32*)((u8*)channel + 0x3C) = 0;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/oTcsG
 */
s32 func_8002B540(s32 arg0, s32 arg1)
{
    if (arg0 != 0)
    {
        if ((u32)(arg1 - 0x80) < 0x30U)
        {
            return arg1 + (arg0 * 0x10);
        }
        if ((u32)(arg1 - 0xB0) < 0x30U)
        {
            return arg1 + ((arg0 - 3) * 0x10);
        }
    }
    return arg1;
}

/**
 * @brief Release sequencer or SFX channels depending on mode.
 *        When channel->unk64 is zero, clears release_mask bits from the
 *        seq-channel bitmasks in g_akao_seq_channel0.  If all active bits are
 *        cleared, also zeros g_akao_seq_pending_ticks, unk5E, and flags.  When
 *        channel->unk64 is non-zero, delegates to akao_sfx_release_channels.
 *        In both paths, channel->unk34 is cleared and the driver dirty flag
 *        (unk8) is OR'd with 0x110.
 * @param channel Pointer to the shared channel header.
 * @param release_mask Bit-mask of channels to release.
 * @see decomp.me (100%) https://decomp.me/scratch/vxrwL
 */
void akao_release_channels(AkaoSFXState* channel, u32 release_mask)
{
    s32 temp_v0;

    if (channel->unk64 == 0)
    {
        u32 tmp = ~release_mask;

        temp_v0 = g_akao_seq_channel0->unk4 & tmp;
        g_akao_seq_channel0->unk4 = temp_v0;

        if (temp_v0 == 0)
        {
            g_akao_seq_pending_ticks = 0;
            g_akao_seq_channel0->unk5E = 0;
            g_akao_seq_channel0->flags = 0;
        }

        g_akao_seq_channel0->unk14 &= tmp;
        g_akao_seq_channel0->unk8 &= tmp;
        g_akao_seq_channel0->unkC &= tmp;
        g_akao_seq_channel0->unk3C &= tmp;
        g_akao_seq_channel0->unk40 &= tmp;
        g_akao_seq_channel0->unk44 &= tmp;
    }
    else
    {
        akao_sfx_release_channels(channel, release_mask);
    }

    channel->unk34 = 0;
    g_akao_driver_flags.unk8 |= 0x110;
}

/**
 * @brief Opcode handler: set the master tempo directly.
 *
 * Reads a 16-bit value from the sequence stream into the high half of
 * @c g_akao_seq_channel0->unk20 (the per-tick rate added to the unk28
 * accumulator in akao_seq_tick_channels) and clears the tempo-slide
 * countdown @c unk5C.
 *
 * @param arg0 Pointer to the channel's stream cursor; advanced by 2.
 * @see decomp.me (100%) https://decomp.me/scratch/GHtCl
 */
void akao_seq_op_set_tempo(u8** arg0)
{
    AkaoChannelState* ch = g_akao_seq_channel0;
    u32 temp_v1;

    temp_v1 = (*arg0)[0] << 0x10;
    ch->unk20 = temp_v1;
    ch->unk20 = temp_v1 | ((*arg0)[1] << 0x18);
    *arg0 += 2;
    ch->unk5C = 0;
}

/**
 * @brief Opcode handler: slide (ramp) the master tempo to a target.
 *
 * Reads a tick count into @c unk5C (defaulting to 0x100 when zero) and a
 * 16-bit target tempo, then computes the per-tick step @c unk24 =
 * (target - current) / count so akao_seq_tick_channels ramps @c unk20 to
 * the target over @c unk5C ticks.
 *
 * @param arg0 Pointer to the channel's stream cursor; advanced past the
 *             count byte and the 2 target bytes.
 * @see decomp.me (100%) https://decomp.me/scratch/SFJAU
 */
void akao_seq_op_slide_tempo(u8** arg0)
{
    u32 combined;
    u32 masked;
    u8** new_var;
    s32 quotient;
    AkaoChannelState* ch = g_akao_seq_channel0;
    u8* ptr = *arg0;
    u32 temp = *(ptr++);
    ch->unk5C = temp;
    *arg0 = ptr;
    if (temp == 0)
    {
        ch->unk5C = 0x100;
    }
    ptr = *arg0;
    combined = (ptr[0] << 16) | ((*(new_var = &ptr))[1] << 24);
    *arg0 = ptr + 2;
    masked = g_akao_seq_channel0->unk20 & 0xFFFF0000;
    quotient = ((s32)(combined - masked)) / ((s32)g_akao_seq_channel0->unk5C);
    g_akao_seq_channel0->unk20 = masked;
    g_akao_seq_channel0->unk24 = quotient;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/Og38F
 */
void func_8002B750(u8** arg0)
{
    s32 val1;
    s32 val0;
    u32 temp;
    AkaoDriverFlags* flags = &g_akao_driver_flags;
    u8* ptr = *arg0;
    AkaoChannelState* ch = g_akao_seq_channel0;
    val1 = (s8)ptr[1];
    val0 = ptr[0];
    *arg0 = ptr + 2;
    ch->unk5A = 0;
    temp = val1 << 20;
    temp = temp | (val0 << 12);
    flags->unk8 |= 0x80;
    ch->unk48 = temp;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/w18Xw
 */
void func_8002B798(u8** arg0)
{
    AkaoChannelState* new_var2;
    AkaoChannelState* ch;
    s32 val1;
    u8* ptr;
    s8 new_var3;
    s32 val0;
    s32 temp_a0;
    u8** new_var;
    s32 val_combined;
    ptr = *arg0;
    ch = g_akao_seq_channel0;
    ch->unk5A = ptr[0];
    *arg0 = (*(new_var = &ptr)) + 1;
    if (ch->unk5A == 0)
    {
        ch->unk5A = 0x100;
    }
    ptr = *arg0;
    new_var3 = (s8)ptr[1];
    val1 = new_var3;
    val0 = ptr[temp_a0 * 0];
    *arg0 = ptr + (2 & 0xFFu);
    ch = (new_var2 = g_akao_seq_channel0);
    val_combined = (val1 << 20) | (val0 << 12);
    temp_a0 = ch->unk48 & (~0xFFF);
    ch->unk48 = temp_a0;
    new_var2->unk4C = (val_combined - temp_a0) / new_var2->unk5A;
}

/**
 * @brief Opcode handler: unconditional relative jump.
 *
 * Reads a signed 16-bit offset from the stream and adds it to the cursor.
 *
 * @param arg0 Pointer to the channel's stream cursor; repositioned by the offset.
 * @see decomp.me (100%) https://decomp.me/scratch/KW15z
 */
void akao_seq_op_jump(void** arg0)
{
    unsigned char* ptr = (unsigned char*)*arg0;
    *arg0 = ptr + (s16)(ptr[0] | (ptr[1] << 8));
}

/**
 * @brief Opcode handler: conditional relative jump.
 *
 * Reads a comparison byte from the stream; if the channel counter
 * @c g_akao_seq_channel0->unk60 is >= that byte, applies a signed 16-bit
 * relative jump, otherwise falls through past the 2 offset bytes.
 *
 * @param arg0 Pointer to the channel's stream cursor; repositioned accordingly.
 * @return TODO: declared u16 but no value is returned; return is unused by callers.
 * @see decomp.me (100%) https://decomp.me/scratch/l153y
 */
unsigned short akao_seq_op_cond_jump(void** arg0)
{
    unsigned char* ptr = (unsigned char*)(*arg0);
    unsigned char* new_var;
    AkaoChannelState* ch = g_akao_seq_channel0;
    int byte0 = *ptr;
    ptr++;
    new_var = &ptr[0];
    *arg0 = ptr;
    if ((*((unsigned short*)(&ch->unk60))) >= byte0)
    {
        s16 offset = (s16)((*new_var) | (ptr[1] << 8));
        *arg0 = ptr + offset;
    }
    else
    {
        *arg0 = ptr + 2;
    }
}

/**
 * @brief Opcode handler: call subroutine (jump and save return cursor).
 *
 * Saves the post-operand cursor (ptr + 2) into slot [5] (offset 0x14) as the
 * return address, then applies a signed 16-bit relative jump to the cursor.
 * Paired with akao_seq_op_return.
 *
 * @param arg0 Pointer to the channel's stream-cursor array.
 * @see decomp.me (100%) https://decomp.me/scratch/uIP0t
 */
void akao_seq_op_call(void* arg0)
{
    unsigned char* ptr2;
    unsigned char** arg = (unsigned char**)arg0; // pointer to array of pointers
    unsigned char* ptr = arg[0];                 // original pointer
    s16 offset = (s16)(ptr[0] | (ptr[1] << 8));  // bytes → signed offset

    arg[5] = ptr + 2;       // store ptr+2 at offset 0x14
    ptr2 = arg[0];          // reload the original pointer
    arg[0] = ptr2 + offset; // add signed offset and store back
}

/**
 * @brief Opcode handler: return from subroutine.
 *
 * Restores the stream cursor (slot [0]) from the saved return address in
 * slot [5] (offset 0x14). Paired with akao_seq_op_call.
 *
 * @param arg0 Pointer to the channel's stream-cursor array.
 * @see decomp.me (100%) https://decomp.me/scratch/os90Z
 */
void akao_seq_op_return(s32* arg0)
{
    arg0[0] = (s32)arg0[5];
}

/**
 * decomp.me (100%) https://decomp.me/scratch/BN6jO
 */
void func_8002B90C(a_struct* arg0)
{
    u8* temp_v0;
    s16 tmp;

    temp_v0 = arg0->unk0;
    tmp = (s16)(*temp_v0 << 8);
    arg0->unk0 = (s32)(u8*)(temp_v0 + 1);
    arg0->unk100 = (s32)(arg0->unk100 | 3);
    arg0->unk84 = tmp;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/CjVYW
 */
void func_8002B934(b_struct* arg0)
{
    u16 temp_a0;
    u16 temp_v1;
    u8* temp_a1;
    u8* temp_v0;

    temp_v0 = arg0->unk0;
    temp_v1 = *temp_v0;

    arg0->unk0 = (u8*)(temp_v0 + 1);
    arg0->unk86 = temp_v1;

    if (temp_v1 == 0)
    {
        arg0->unk86 = 0x100U;
    }

    temp_a1 = arg0->unk0;
    temp_a0 = arg0->unk84;
    temp_a0 &= 0x7F00;

    temp_v1 = (s16)(((s32)(((*(arg0->unk0++)) << 8) - temp_a0)) / ((s32)arg0->unk86));
    arg0->unkE0 = temp_v1;

    arg0->unk0 = (u8*)(temp_a1 + 1);
    arg0->unk84 = temp_a0;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/hGVxk
 */
void func_8002B9B8(c_struct* arg0)
{
    s8* temp_v0;

    temp_v0 = arg0->unk0;
    arg0->unk48 = (s32)(*temp_v0 << 0x17);
    arg0->unk0 = (s8*)(temp_v0 + 1);
    arg0->unk8A = 0;
    arg0->unk100 = (s32)(arg0->unk100 | 3);
    arg0->unk8C = 0;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/jLHGo
 */
void func_8002B9E8(d_struct* arg0)
{
    s32 temp_a0;
    s32 temp_v1;
    u8* temp_a1;
    u8* temp_v0;
    s32 tmp;
    s32 new_var;

    temp_v0 = arg0->unk0;

    temp_v1 = *temp_v0;
    arg0->unk0 = (u8*)(temp_v0 + 1);
    arg0->unk8A = (u16)temp_v1;
    if (temp_v1 == 0)
    {
        arg0->unk8A = 0x100U;
    }
    temp_a1 = arg0->unk0;
    new_var = arg0->unk48;
    temp_a0 = new_var;
    temp_a0 &= 0xFFFF0000;

    tmp = (s32)((s32)(((s8)*temp_a1++ << 0x17) - temp_a0) / (s32)arg0->unk8A);
    arg0->unk4C = tmp;
    arg0->unk0 = (u8*)(temp_a1);
    arg0->unk48 = temp_a0;
    arg0->unk8C = 0;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/aOcaK
 */
void func_8002BA74(e_struct* arg0)
{
    s32 temp_v1;
    u8* temp_a0;
    s8* temp_v0;

    u32 tmp;

    temp_v0 = arg0->unk0;

    tmp = *temp_v0;
    temp_v0++;

    arg0->unk0 = temp_v0;
    arg0->unk5C = (s32)(tmp << 0x17);
    temp_v1 = *(u8*)temp_v0;
    temp_v0++;
    arg0->unk0 = (void*)((u8*)temp_v0);
    arg0->unk8C = (u16)temp_v1;

    if (temp_v1 == 0)
    {
        arg0->unk8C = 0x100U;
    }

    temp_a0 = arg0->unk0;

    arg0->unk60 = (s32)((s32)((*(s8*)temp_a0 << 0x17) - arg0->unk5C) / (s32)arg0->unk8C);
    arg0->unk0 = (void*)((u8*)temp_a0 + 1);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/cNPss
 */
void func_8002BB04(ee_struct* arg0)
{
    arg0->unk34 = (s32)(arg0->unk34 | 0x40);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/ryS0d
 */
void func_8002BB18(ff_struct* arg0)
{
    arg0->unk34 = (s32)(arg0->unk34 & ~0x40);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/ZUdZO
 */
void func_8002BB2C(f_struct* arg0)
{
    u8* temp_v0;

    temp_v0 = arg0->unk0;
    arg0->unk90 = (s16)(((*temp_v0 + 0x40) & 0xFF) << 8);
    arg0->unk0 = (u8*)(temp_v0 + 1);
    arg0->unk92 = 0;
    arg0->unk100 = (s32)(arg0->unk100 | 3);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/MjjmM
 */
void func_8002BB60(g_struct* arg0)
{
    u16 temp_a0;
    s32 temp_v1;
    u8* temp_a1;
    u8* temp_v0;

    temp_v0 = arg0->unk0;
    temp_v1 = *temp_v0;
    arg0->unk0 = (u8*)(temp_v0 + 1);
    arg0->unk92 = (u16)temp_v1;
    if (temp_v1 == 0)
    {
        arg0->unk92 = 0x100U;
    }
    temp_a1 = arg0->unk0;
    temp_a0 = arg0->unk90;
    temp_a0 &= 0xFF00;
    arg0->unkE8 = (s16)((s32)((((*temp_a1 + 0x40) & 0xFF) << 8) - temp_a0) / (s32)arg0->unk92);
    arg0->unk0 = (u8*)(temp_a1 + 1);
    arg0->unk90 = temp_a0;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/FRzyk
 */
void func_8002BBEC(h_struct* arg0)
{
    u8* temp_v0;

    temp_v0 = arg0->unk0;
    arg0->unk96 = (s16)*temp_v0;
    arg0->unk0 = (u8*)(temp_v0 + 1);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/2bdR3
 */
void func_8002BC08(i_struct* arg0)
{
    arg0->unk96 = (u16)((arg0->unk96 + 1) & 0xF);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/SJfbQ
 */
void func_8002BC20(j_struct* arg0)
{
    arg0->unk96 = (u16)((arg0->unk96 - 1) & 0xF);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/1MXGr
 */
void func_8002BC38(k_struct* arg0)
{
    s32* temp_a1_2;
    s32 temp_a1;
    s32 var_s1;
    u8* temp_v0;

    temp_v0 = arg0->unk0;
    temp_a1 = *temp_v0;
    arg0->unk0 = (u8*)(temp_v0 + 1);
    if (arg0->unk64 == 0)
    {
        var_s1 = temp_a1;
    }
    else
    {
        var_s1 = func_8002B540(arg0->unk38, temp_a1);
    }

    temp_a1_2 = (var_s1 * 0x10) + g_akao_articulation_slots;
    akao_channel_load_articulation_fields(arg0, temp_a1_2, *temp_a1_2);
    arg0->unk6A = (s16)var_s1;
    arg0->unk112 = 0;
    arg0->unk34 = (s32)(arg0->unk34 & 0xE6FFEFF7);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/GAl01
 */
void func_8002BCC8(l_struct* arg0)
{
    s32 temp_s1;
    u8* temp_v0;

    temp_v0 = arg0->unk0;
    temp_s1 = *temp_v0;
    arg0->unk0 = (u8*)(temp_v0 + 1);
    akao_channel_load_articulation_fields(arg0, (temp_s1 * 0x10) + g_akao_articulation_slots, 0x1010);
    arg0->unk6A = (s16)temp_s1;
    arg0->unk112 = 0;
    arg0->unk34 = (s32)(arg0->unk34 & 0xE6FFEFF7);
}

/**
 * @brief Handles AKAO extended sequence opcode 0x14.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void func_8002BD34(m_struct* arg0)
{
    s32 base;
    u16* entry;
    u8* temp_v0;
    u8 opcode;

    temp_v0 = arg0->unk0;
    opcode = *temp_v0;
    arg0->unk0 = (u8*)(temp_v0 + 1);
    base = g_akao_seq_channel0->unk30;
    if (base != 0)
    {
        entry = (u16*)((opcode & 0xFF) * 2 + base);
        if ((u16)*entry > 0x8000U)
        {
            arg0->unk112 = 0;
            arg0->unk34 = (s32)(arg0->unk34 & ~0x1000);
            return;
        }
        arg0->unk18 = (s32)(base + *entry + 0x20);
        arg0->unkEE = 0xFF;
        arg0->unk34 = (s32)((arg0->unk34 & 0xE6FFEFF7) | 0x1000);
    }
}

/**
 * @brief Reloads articulation fields for the channel's current articulation.
 * @param arg0 Channel state to update.
 * @see decomp.me (100%)
 */
void func_8002BDC8(n_struct* arg0)
{
    AkaoArticulation* articulation;
    u16 tmp_c;
    u16 tmp_e;
    s32 old_value;
    s32 flags;

    articulation = (AkaoArticulation*)(g_akao_articulation_slots + (arg0->unk6A * 0x10));
    tmp_c = articulation->pitch_misc.half.lo;
    arg0->unk10E = tmp_c;
    tmp_e = articulation->pitch_misc.half.hi;
    old_value = arg0->unk100;
    flags = arg0->unk34;
    arg0->unk100 = old_value | 0xFF00;
    arg0->unk34 = flags & 0xE6FFFFFF;
    arg0->unk110 = tmp_e;
}

/**
 * @brief Reads a signed byte from the channel bytecode stream and stores it
 *        as a halfword at offset 0xEA.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void func_8002BE10(o_struct* arg0)
{
    u8* temp_v0;
    s8 val;

    temp_v0 = arg0->unk0;
    val = *temp_v0;
    arg0->unk0 = (u8*)(temp_v0 + 1);
    arg0->unkEA = val;
}

/**
 * @brief Reads a signed byte from the channel bytecode stream and adds it to
 *        the halfword at offset 0xEA.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void func_8002BE34(o_struct* arg0)
{
    u8* temp_v0;
    s8 val;

    temp_v0 = arg0->unk0;
    val = *temp_v0;
    arg0->unk0 = (u8*)(temp_v0 + 1);
    arg0->unkEA += val;
}

/**
 * @brief Reads a byte for field 0x98 (defaulting 0 to 0x100), then a signed
 *        byte stored at field 0xF0.
 * @param arg0 Channel state whose bytecode cursor is advanced by two bytes.
 * @see decomp.me (100%)
 */
void func_8002BE60(p_struct* arg0)
{
    u8* temp_v0;
    u8* temp_v0_2;
    s32 b;
    s8 b2;

    temp_v0 = arg0->unk0;
    b = *temp_v0;
    arg0->unk0 = (u8*)(temp_v0 + 1);
    arg0->unk98 = b;
    if (b == 0)
    {
        arg0->unk98 = 0x100;
    }
    temp_v0_2 = arg0->unk0;
    b2 = *temp_v0_2;
    arg0->unk0 = (u8*)(temp_v0_2 + 1);
    arg0->unkF0 = b2;
}

/**
 * @brief Reads one byte for field 0x9C (defaulting 0 to 0x100) and resets the
 *        related state fields 0x9A/0x9E/0xF2.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void func_8002BEA8(q_struct* arg0)
{
    u8* temp_v0;
    s32 b;

    temp_v0 = arg0->unk0;
    b = *temp_v0;
    arg0->unk0 = (u8*)(temp_v0 + 1);
    arg0->unk9C = b;
    if (b == 0)
    {
        arg0->unk9C = 0x100;
    }
    arg0->unkF2 = 0;
    arg0->unk9A = 0;
    arg0->unk9E = 1;
}

/**
 * @brief Clears field 0x9C.
 * @param arg0 Channel state.
 * @see decomp.me (100%)
 */
void func_8002BEE0(q_struct* arg0)
{
    arg0->unk9C = 0;
}

/**
 * @brief Reads a signed byte scale factor into field 0xEC and computes the
 *        scaled result stored at field 0x54, then sets flag 0x10 in field 0x100.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void func_8002BEE8(r_struct* arg0)
{
    s32 scale;
    u8* next;
    u32 prod;
    u32 result;
    u8* temp_v0;

    temp_v0 = arg0->unk0;
    next = (u8*)(temp_v0 + 1);
    arg0->unkEC = (s16)(s8)*temp_v0;
    result = (u8)arg0->unkEC;
    scale = arg0->unk2C;
    prod = scale * result;
    arg0->unk0 = next;
    if (arg0->unkEC < 0)
    {
        result = (prod >> 8) - scale;
    }
    else
    {
        result = prod >> 7;
    }
    arg0->unk54 = result;
    arg0->unk100 = (s32)(arg0->unk100 | 0x10);
}

/**
 * @brief Adds a signed byte to field 0xEC, then recomputes the scaled result
 *        at field 0x54 and sets flag 0x10 in field 0x100.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void func_8002BF48(r_struct* arg0)
{
    s32 scale;
    u8* temp_v0;
    u32 prod;
    u32 result;

    temp_v0 = arg0->unk0;
    scale = arg0->unk2C;
    arg0->unkEC += (s8)*temp_v0;
    arg0->unk0 = (u8*)(temp_v0 + 1);
    result = (u8)arg0->unkEC;
    prod = scale * result;
    if (arg0->unkEC < 0)
    {
        result = (prod >> 8) - scale;
    }
    else
    {
        result = prod >> 7;
    }
    arg0->unk54 = result;
    arg0->unk100 = (s32)(arg0->unk100 | 0x10);
}

/**
 * @brief AKAO opcode handler that reads several bytecode bytes into the
 *        channel's portamento/pitch state (0xA2..0xAE), computes the scaled
 *        slide value at 0xAC, and looks up a table entry into 0x1C.
 * @param arg0 Channel state whose bytecode cursor is advanced.
 * @note @c temp_a0 must be widened past u16 so gcc keeps the second mult
 *       operand order (@c hi first) matching the other branch.
 * @see decomp.me (100%)
 */
void func_8002BFAC(s_struct* arg0)
{
    s_struct* p;
    u32 temp_a0;
    u32 raw;
    u16 flags;
    u32 hi;
    u32 var_lo;
    u8* temp_v0;
    u8* temp_v0_2;
    u8* temp_v0_3;
    u8* temp_v0_4;
    s32 temp_v1;
    s32 temp_v1_2;

    p = arg0;
    p->unk34 = (s32)(p->unk34 | 1);
    if (p->unk64 != 0)
    {
        temp_v0 = p->unk0;
        p->unkA2 = 0;
        temp_v1 = *temp_v0;
        p->unk0 = (u8*)(temp_v0 + 1);
        if (temp_v1 != 0)
        {
            p->unkAE = temp_v1 << 8;
        }
    }
    else
    {
        temp_v0_2 = p->unk0;
        p->unkA2 = *temp_v0_2;
        p->unk0 = (u8*)(temp_v0_2 + 1);
    }
    temp_v0_3 = p->unk0;
    temp_v1_2 = *temp_v0_3;
    p->unk0 = (u8*)(temp_v0_3 + 1);
    p->unkA6 = temp_v1_2;
    if (temp_v1_2 == 0)
    {
        p->unkA6 = 0x100;
    }
    temp_v0_4 = p->unk0;
    flags = p->unkAE;
    raw = *temp_v0_4;
    p->unk0 = (u8*)(temp_v0_4 + 1);
    p->unkAA = raw;
    temp_a0 = p->unk2C;
    hi = (u32)(flags & 0x7F00) >> 8;
    if (!(flags & 0x8000))
    {
        var_lo = hi * ((s32)(temp_a0 * 0xF) >> 8);
    }
    else
    {
        var_lo = hi * temp_a0;
    }
    p->unkAC = var_lo >> 7;
    p->unk1C = D_8003DD80[p->unkAA];
    p->unkA4 = p->unkA2;
    p->unkA8 = 1;
}

/**
 * @brief AKAO opcode handler: reads one bytecode byte, stores it shifted at
 *        field 0xAE, and computes a scaled slide value into field 0xAC.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @note The @c flags re-read of 0xAE is required to keep the base pointer in
 *       a1 to match; it leaves one residual @c andi (see status).
 * @see decomp.me (90.96%)
 */
void func_8002C0A4(t_struct* arg0)
{
    t_struct* p;
    s32 scale;
    u8* temp_v0;
    u32 shifted;
    u32 flags;
    u32 hi;
    u32 var_lo;

    p = arg0;
    temp_v0 = p->unk0;
    shifted = *temp_v0 << 8;
    p->unk0 = (u8*)(temp_v0 + 1);
    scale = p->unk2C;
    p->unkAE = shifted;
    flags = p->unkAE;
    hi = (u32)(flags & 0x7F00) >> 8;
    if (!(flags & 0x8000))
    {
        var_lo = hi * ((s32)(scale * 0xF) >> 8);
    }
    else
    {
        var_lo = hi * scale;
    }
    p->unkAC = var_lo >> 7;
}

/**
 * @brief AKAO opcode handler: reads a divisor byte (0 defaults to 0x100) and a
 *        target byte, storing the divisor at 0xB0 and the scaled quotient at 0xB2.
 * @param arg0 Channel state whose bytecode cursor is advanced by two bytes.
 * @see decomp.me (100%)
 */
void func_8002C104(u_struct* arg0)
{
    u8* temp_a1;
    s32 divisor;
    s32 result;

    temp_a1 = arg0->unk0;
    divisor = *temp_a1;
    temp_a1 += 1;
    arg0->unk0 = temp_a1;
    if (divisor == 0)
    {
        divisor = 0x100;
    }
    result = ((s32)(*temp_a1 << 8) - arg0->unkAE) / divisor;
    arg0->unk0 = (u8*)(temp_a1 + 1);
    arg0->unkB0 = divisor;
    arg0->unkB2 = result;
}

/**
 * @brief AKAO opcode handler: clears field 0xF4, clears bit 0 of the flags at
 *        0x34, and sets bit 0x10 in the flags at 0x100.
 * @param arg0 Channel state.
 * @see decomp.me (100%)
 */
void func_8002C170(v_struct* arg0)
{
    arg0->unkF4 = 0;
    arg0->unk34 = arg0->unk34 & ~1;
    arg0->unk100 = arg0->unk100 | 0x10;
}

/**
 * @brief AKAO opcode handler: reads three bytecode bytes into the channel's
 *        portamento state (0xB6..0xC0), defaults 0xBA when zero, and looks up a
 *        table entry into field 0x20.
 * @param arg0 Channel state whose bytecode cursor is advanced by three bytes.
 * @see decomp.me (100%)
 */
void func_8002C194(w_struct* arg0)
{
    w_struct* p;
    u8* temp_v0;
    u8* temp_v0_2;
    u8* temp_v1;
    s32 temp_a0;
    s32 temp_v1_2;
    u32 raw;

    p = arg0;
    temp_v1 = p->unk0;
    p->unk34 = (s32)(p->unk34 | 2);
    temp_a0 = *temp_v1;
    p->unk0 = (u8*)(temp_v1 + 1);
    if (p->unk64 != 0)
    {
        p->unkB6 = 0;
        if (temp_a0 != 0)
        {
            p->unkC0 = (temp_a0 & 0x7F) << 8;
        }
    }
    else
    {
        p->unkB6 = temp_a0;
    }
    temp_v0 = p->unk0;
    temp_v1_2 = *temp_v0;
    p->unk0 = (u8*)(temp_v0 + 1);
    p->unkBA = temp_v1_2;
    if (temp_v1_2 == 0)
    {
        p->unkBA = 0x100;
    }
    temp_v0_2 = p->unk0;
    raw = *temp_v0_2;
    p->unk0 = (u8*)(temp_v0_2 + 1);
    p->unkBE = raw;
    p->unk20 = D_8003DD80[p->unkBE];
    p->unkB8 = p->unkB6;
    p->unkBC = 1;
}

/**
 * @brief AKAO opcode handler: reads a byte, masks it to 7 bits and shifts it
 *        left 8, storing the result at field 0xC0.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void func_8002C244(x_struct* arg0)
{
    u8* temp_v0;
    s32 b;

    temp_v0 = arg0->unk0;
    b = *temp_v0;
    arg0->unk0 = (u8*)(temp_v0 + 1);
    arg0->unkC0 = (b & 0x7F) << 8;
}

/**
 * @brief AKAO opcode handler: reads a divisor byte (0 defaults to 0x100) and a
 *        target byte, storing the divisor at 0xC2 and the scaled quotient at 0xC4.
 * @param arg0 Channel state whose bytecode cursor is advanced by two bytes.
 * @see decomp.me (100%)
 */
void func_8002C268(y_struct* arg0)
{
    u8* temp_a1;
    s32 divisor;
    s32 result;

    temp_a1 = arg0->unk0;
    divisor = *temp_a1;
    temp_a1 += 1;
    arg0->unk0 = temp_a1;
    if (divisor == 0)
    {
        divisor = 0x100;
    }
    result = (((s32)(*temp_a1 & 0x7F) << 8) - arg0->unkC0) / divisor;
    arg0->unk0 = (u8*)(temp_a1 + 1);
    arg0->unkC2 = divisor;
    arg0->unkC4 = result;
}

/**
 * @brief AKAO opcode handler: clears field 0xF6, clears bit 1 of the flags at
 *        0x34, and sets bits 0x3 in the flags at 0x100.
 * @param arg0 Channel state.
 * @see decomp.me (100%)
 */
void func_8002C2D8(z_struct* arg0)
{
    arg0->unkF6 = 0;
    arg0->unk34 = arg0->unk34 & ~2;
    arg0->unk100 = arg0->unk100 | 3;
}

/**
 * @brief AKAO opcode handler: sets flag 0x4 at 0x34, reads a byte (0 defaults
 *        to 0x100) into 0xC8, reads an index byte into 0xCC, sets 0xCA, and
 *        looks up a table entry into field 0x24.
 * @param arg0 Channel state whose bytecode cursor is advanced by two bytes.
 * @see decomp.me (100%)
 */
void func_8002C2FC(aa_struct* arg0)
{
    u8* temp_v0;
    u8* temp_v0_2;
    s32 b1;
    u32 raw;

    arg0->unk34 = (s32)(arg0->unk34 | 4);
    temp_v0 = arg0->unk0;
    b1 = *temp_v0;
    arg0->unk0 = (u8*)(temp_v0 + 1);
    arg0->unkC8 = b1;
    if (b1 == 0)
    {
        arg0->unkC8 = 0x100;
    }
    temp_v0_2 = arg0->unk0;
    raw = *temp_v0_2;
    arg0->unk0 = (u8*)(temp_v0_2 + 1);
    arg0->unkCC = raw;
    arg0->unk24 = D_8003DD80[arg0->unkCC];
    arg0->unkCA = 1;
}

/**
 * @brief AKAO opcode handler: reads a byte, shifts it left 7, and stores the
 *        result at field 0xCE.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void func_8002C368(ab_struct* arg0)
{
    u8* temp_v0;
    s32 b;

    temp_v0 = arg0->unk0;
    b = *temp_v0;
    arg0->unk0 = (u8*)(temp_v0 + 1);
    arg0->unkCE = b << 7;
}

/**
 * @brief AKAO opcode handler: reads a divisor byte (0 defaults to 0x100) and a
 *        target byte, storing the divisor at 0xD0 and the scaled quotient at 0xD2.
 * @param arg0 Channel state whose bytecode cursor is advanced by two bytes.
 * @see decomp.me (100%)
 */
void func_8002C388(ac_struct* arg0)
{
    u8* temp_a1;
    s32 divisor;
    s32 result;

    temp_a1 = arg0->unk0;
    divisor = *temp_a1;
    temp_a1 += 1;
    arg0->unk0 = temp_a1;
    if (divisor == 0)
    {
        divisor = 0x100;
    }
    result = (((s32)*temp_a1 << 7) - arg0->unkCE) / divisor;
    arg0->unk0 = (u8*)(temp_a1 + 1);
    arg0->unkD0 = divisor;
    arg0->unkD2 = result;
}

/**
 * @brief AKAO opcode handler: clears field 0xF8, clears bit 2 of the flags at
 *        0x34, and sets bits 0x3 in the flags at 0x100.
 * @param arg0 Channel state.
 * @see decomp.me (100%)
 */
void func_8002C3F4(ad_struct* arg0)
{
    arg0->unkF8 = 0;
    arg0->unk34 = arg0->unk34 & ~4;
    arg0->unk100 = arg0->unk100 | 3;
}

/**
 * @brief AKAO opcode handler: OR-sets a caller-supplied flag mask into either
 *        the SFX control block or the primary sequence channel (depending on
 *        whether this channel is an SFX channel), then raises driver flags 0x110.
 * @param arg0 Channel state; @c unk64 selects SFX vs sequence routing.
 * @param arg1 Flag bitmask to OR in.
 * @note Residual: the g_akao_seq_channel0 %hi colors to v0 not v1 (one lui
 *       register), a gcc 2.8 coloring tie-break the permuter cannot move.
 * @see decomp.me (99.58%)
 */
void func_8002C418(AkaoChannelState* arg0, s32 arg1)
{
    if (arg0->unk64 == 0)
    {
        g_akao_seq_channel0->unk3C |= arg1;
    }
    else
    {
        g_akao_sfx_control.unk1C |= arg1;
    }
    g_akao_driver_flags.unk8 |= 0x110;
}

/**
 * @brief AKAO opcode handler: AND-clears a caller-supplied flag mask from either
 *        the SFX control block or the primary sequence channel, raises driver
 *        flags 0x110, and clears channel field 0xD4.
 * @param arg0 Channel state; @c unk64 selects SFX vs sequence routing.
 * @param arg1 Flag bitmask to clear (applied as @c &= ~arg1).
 * @note Residual: the seq-channel path register coloring differs (5 rows), a
 *       gcc 2.8 coloring tie-break the permuter cannot move (shared with
 *       func_8002C418).
 * @see decomp.me (98.27%)
 */
void func_8002C478(ae_struct* arg0, s32 arg1)
{
    if (arg0->unk64 == 0)
    {
        g_akao_seq_channel0->unk3C &= ~arg1;
    }
    else
    {
        g_akao_sfx_control.unk1C &= ~arg1;
    }
    g_akao_driver_flags.unk8 |= 0x110;
    arg0->unkD4 = 0;
}

/**
 * @brief AKAO opcode handler: OR-sets a caller-supplied flag mask into the
 *        sequence channel (0x44) when this channel is a sequence channel, or
 *        into the SFX control block (0x24) when SFX flag 0x10000 is set, then
 *        raises driver flags 0x100.
 * @param arg0 Channel state; @c unk64 selects sequence routing, @c unk34 gates SFX.
 * @param arg1 Flag bitmask to OR in.
 * @note Residual: the g_akao_seq_channel0 %hi colors to v0 not v1 (one lui
 *       register), a gcc 2.8 coloring tie-break shared with func_8002C418.
 * @see decomp.me (99.66%)
 */
void func_8002C4E0(af_struct* arg0, s32 arg1)
{
    if (arg0->unk64 == 0)
    {
        g_akao_seq_channel0->unk44 |= arg1;
    }
    else if (arg0->unk34 & 0x10000)
    {
        g_akao_sfx_control.unk24 |= arg1;
    }
    g_akao_driver_flags.unk8 |= 0x100;
}

/**
 * @brief AKAO opcode handler: AND-clears a caller-supplied flag mask from either
 *        the sequence channel (0x44) or the SFX control block (0x24), raises
 *        driver flags 0x100, and clears channel field 0xD6.
 * @param arg0 Channel state; @c unk64 selects SFX vs sequence routing.
 * @param arg1 Flag bitmask to clear (applied as @c &= ~arg1).
 * @note Residual: the seq-channel path register coloring differs (5 rows), a
 *       gcc 2.8 coloring tie-break shared with func_8002C418/C478.
 * @see decomp.me (98.27%)
 */
void func_8002C554(ag_struct* arg0, s32 arg1)
{
    if (arg0->unk64 == 0)
    {
        g_akao_seq_channel0->unk44 &= ~arg1;
    }
    else
    {
        g_akao_sfx_control.unk24 &= ~arg1;
    }
    g_akao_driver_flags.unk8 |= 0x100;
    arg0->unkD6 = 0;
}

/**
 * @brief AKAO opcode handler: OR-sets a caller-supplied flag mask into either
 *        the sequence channel (0x40) or the SFX control block (0x20), then
 *        raises driver flags 0x100.
 * @param arg0 Channel state; @c unk64 selects SFX vs sequence routing.
 * @param arg1 Flag bitmask to OR in.
 * @note Residual: the g_akao_seq_channel0 %hi coloring tie-break shared with
 *       func_8002C418.
 * @see decomp.me (99.58%)
 */
void func_8002C5BC(AkaoChannelState* arg0, s32 arg1)
{
    if (arg0->unk64 == 0)
    {
        g_akao_seq_channel0->unk40 |= arg1;
    }
    else
    {
        g_akao_sfx_control.unk20 |= arg1;
    }
    g_akao_driver_flags.unk8 |= 0x100;
}

/**
 * @brief AKAO opcode handler: AND-clears a caller-supplied flag mask from either
 *        the sequence channel (0x40) or the SFX control block (0x20), then
 *        raises driver flags 0x100.
 * @param arg0 Channel state; @c unk64 selects SFX vs sequence routing.
 * @param arg1 Flag bitmask to clear (applied as @c &= ~arg1).
 * @note Residual: the seq-channel path register coloring differs (5 rows), a
 *       gcc 2.8 coloring tie-break shared with func_8002C418/C478.
 * @see decomp.me (98.13%)
 */
void func_8002C61C(AkaoChannelState* arg0, s32 arg1)
{
    if (arg0->unk64 == 0)
    {
        g_akao_seq_channel0->unk40 &= ~arg1;
    }
    else
    {
        g_akao_sfx_control.unk20 &= ~arg1;
    }
    g_akao_driver_flags.unk8 |= 0x100;
}

/**
 * @brief AKAO opcode handler: sets field 0x9E to 1.
 * @param arg0 Channel state.
 * @see decomp.me (100%)
 */
void func_8002C67C(ah_struct* arg0)
{
    arg0->unk9E = 1;
}

/**
 * @brief AKAO opcode handler stub: no operation.
 * @see decomp.me (100%)
 */
void func_8002C688(void)
{
}

/**
 * @brief AKAO opcode handler: sets field 0x9E to 4 when the channel is active.
 * @param arg0 Channel state; @c unk64 selects whether the store happens.
 * @see decomp.me (100%)
 */
void func_8002C690(ai_struct* arg0)
{
    if (arg0->unk64 != 0)
    {
        arg0->unk9E = 4;
    }
}

/**
 * @brief AKAO opcode handler stub: no operation.
 * @see decomp.me (100%)
 */
void func_8002C6AC(void)
{
}

/**
 * decomp.me (100%) https://decomp.me/scratch/Get0N
 */
void func_8002C6B4(argst1* arg0)
{
    s16 temp_a1;
    u8* temp_v0;

    temp_v0 = arg0->unk0;
    temp_a1 = *temp_v0;
    arg0->unk0 = (u8*)(temp_v0 + 1);
    if (arg0->unk64 == 0)
    {
        if (temp_a1 & 0xC0)
        {
            g_akao_seq_channel0->unk62 = (u16)((g_akao_seq_channel0->unk62 + (temp_a1 & 0x3F)) & 0x3F);
        }
        else
        {
            g_akao_seq_channel0->unk62 = (u16)temp_a1;
        }
    }
    else if (temp_a1 & 0xC0)
    {
        g_akao_sfx_control.unk28 = (u16)((g_akao_sfx_control.unk28 + (temp_a1 & 0x3F)) & 0x3F);
    }
    else
    {
        D_8004D428[0] = (s16)temp_a1;
    }
    g_akao_driver_flags.unk8 |= 0x10;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/gwBIb
 */
void func_8002C758(SomeStruct* arg0)
{
    u8* ptr;
    u32 byte_val;
    u32 new_unk100;
    u32 new_unk34;
    u16 new_unk10E;

    ptr = arg0->unk0;
    byte_val = *ptr;
    arg0->unk0 = ptr + 1;

    new_unk100 = arg0->unk100 | 0x900;
    arg0->unk100 = new_unk100;

    new_unk34 = arg0->unk34 | 0x01000000;

    new_unk10E = (arg0->unk10E & 0x80FF) | ((u16)byte_val << 8);

    arg0->unk34 = new_unk34;
    arg0->unk10E = new_unk10E;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/ID5s7
 */
void func_8002C79C(s_8002C79C* arg0)
{
    u8* ptr;
    u32 byte_val;
    u32 new_unk100;
    u16 new_unk10E;

    ptr = arg0->unk0;
    byte_val = *ptr;
    arg0->unk0 = ptr + 1;

    new_unk100 = arg0->unk100 | 0x1000;
    new_unk10E = ((arg0->unk10E & 0xFF0F) | (byte_val * 0x10));

    arg0->unk100 = new_unk100;
    arg0->unk10E = new_unk10E;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/ZOmaE
 */
void func_8002C7D0(s_8002C79C* arg0)
{
    u8* ptr;
    u32 byte_val;
    u32 new_unk100;
    u16 new_unk10E;

    ptr = arg0->unk0;
    byte_val = *ptr;
    arg0->unk0 = ptr + 1;
    new_unk100 = arg0->unk100 | 0x8000;
    new_unk10E = (arg0->unk10E & 0xFFF0) | byte_val;
    arg0->unk100 = new_unk100;
    arg0->unk10E = new_unk10E;
}

typedef struct
{
    u8* unk0;
    u8 pad1[0x34 - 0x4];
    s32 unk34;
    u8 pad2[0x100 - 0x38];
    s32 unk100;
    u8 pad3[0x110 - 0x104];
    u16 unk110;
} s_8002C800;

/**
 * decomp.me (100%) https://decomp.me/scratch/Tun26
 */
void func_8002C800(s_8002C800* arg0)
{
    u8* ptr;
    u32 byte_val;
    u32 new_unk100;
    u32 new_unk34;
    u16 new_unk110;

    ptr = arg0->unk0;
    byte_val = *ptr;
    arg0->unk0 = ptr + 1;

    new_unk100 = arg0->unk100 | 0x2200;
    new_unk34 = arg0->unk34 | 0x08000000;
    new_unk110 = (arg0->unk110 & 0xE03F) | (byte_val << 6);

    arg0->unk100 = new_unk100;
    arg0->unk34 = new_unk34;
    arg0->unk110 = new_unk110;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/DR9uQ
 */
void func_8002C844(s_8002C800* arg0)
{
    u8* ptr;
    u32 byte_val;
    u32 new_unk100;
    u32 new_unk34;
    u16 new_unk110;

    ptr = arg0->unk0;
    byte_val = *ptr;
    arg0->unk0 = ptr + 1;

    new_unk100 = arg0->unk100 | 0x4400;
    new_unk34 = arg0->unk34 | 0x10000000;
    new_unk110 = (arg0->unk110 & 0xFFE0) | byte_val;

    arg0->unk100 = new_unk100;
    arg0->unk34 = new_unk34;
    arg0->unk110 = new_unk110;
}

typedef struct
{
    u8* unk0;               /* 0x00 */
    u8 pad1[0x100 - 0x4];   /* 0x04 */
    s32 unk100;             /* 0x100 */
    u8 pad2[0x10E - 0x104]; /* 0x104 */
    u16 unk10E;             /* 0x10E */
} s_8002C884;

/**
 * decomp.me (100%) https://decomp.me/scratch/dP97Y
 */
void func_8002C884(s_8002C884* arg0)
{
    u8* ptr;
    u32 byte_val;
    u16 new_unk10E;

    ptr = arg0->unk0;
    byte_val = *ptr;
    arg0->unk0 = ptr + 1;

    new_unk10E = arg0->unk10E & 0x7FFF;
    arg0->unk10E = new_unk10E;
    if (byte_val == 5)
    {
        new_unk10E |= 0x8000;
        arg0->unk10E = new_unk10E;
    }

    arg0->unk100 = arg0->unk100 | 0x100;
}

typedef struct
{
    u8* unk0;               /* 0x00 */
    u8 pad1[0x100 - 0x4];   /* 0x04 */
    s32 unk100;             /* 0x100 */
    u8 pad2[0x110 - 0x104]; /* 0x104 */
    u16 unk110;             /* 0x110 */
} Struct_8002C8C8;

/**
 * decomp.me (100%) https://decomp.me/scratch/8od0h
 */
void func_8002C8C8(Struct_8002C8C8* arg0)
{
    s32 temp_a0;
    u16 temp_v1;
    u8* temp_v0;
    u32 byte_val;

    temp_v0 = arg0->unk0;
    byte_val = *temp_v0;
    temp_v1 = arg0->unk110 & 0x3FFF;
    arg0->unk0 = (u8*)(temp_v0 + 1);
    temp_a0 = byte_val & 0xFFFF;
    arg0->unk110 = temp_v1;

    switch (temp_a0)
    {
    case 3:
        arg0->unk110 = temp_v1 | 0x4000;
        break;
    case 5:
        arg0->unk110 = temp_v1 | 0x8000;
        break;
    case 7:
        arg0->unk110 = temp_v1 | 0xC000;
        break;
    }

    arg0->unk100 = (s32)(arg0->unk100 | 0x200);
}

typedef struct
{
    u8* unk0;               /* 0x00 */
    u8 pad1[0x100 - 0x4];   /* 0x04 */
    s32 unk100;             /* 0x100 */
    u8 pad2[0x110 - 0x104]; /* 0x104 */
    u16 unk110;             /* 0x10E */
} s_8002C940;

/**
 * decomp.me (100%) https://decomp.me/scratch/1Pqa0
 */
void func_8002C940(s_8002C940* arg0)
{

    u8* temp_v0;
    u16 new_unk110;
    u32 byte_val;

    temp_v0 = arg0->unk0;
    byte_val = *temp_v0;
    arg0->unk0 = (u8*)(temp_v0 + 1);

    new_unk110 = arg0->unk110 & 0xFFDF;
    arg0->unk110 = new_unk110;

    if (byte_val == 7)
    {
        new_unk110 = (u16)(new_unk110 | 0x20);
        arg0->unk110 = new_unk110;
    }

    arg0->unk100 = (s32)(arg0->unk100 | 0x400);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/rLRQL
 */
void func_8002C984(u8** arg0)
{
    u8* temp_v0;

    temp_v0 = *arg0;
    g_akao_seq_channel0->unk38 = (s32)*temp_v0;
    *arg0 = temp_v0 + 1;
}