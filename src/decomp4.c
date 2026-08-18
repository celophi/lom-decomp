#include "decomp4.h"
#include "akao_voice.h"


/* Defined in the sdata segment (asm/data/sdata.data.s) at their gp-relative
 * addresses near gp_value 0x8003EC14; declared extern here so decomp4 does not
 * emit a second (.bss) definition. */
extern u16 g_akao_irq_frame_counter;
s32 D_8004D40C;
u32 D_8004F758;
extern s32 D_8003EC18;

/** @brief 12-entry semitone pitch-ratio table indexed by note % 12 in akao_compute_pitch. */
extern u32 g_akao_pitch_table[];

/** @brief 16-entry table of pitch, volume, and pan LFO waveform streams. */
extern s32 g_akao_lfo_waveforms[];

/**
 * @brief One 8-byte note slot in the per-channel note table at
 *        @c channel->flags (the song-role note table). Each entry encodes
 *        a packed set of per-note SPU voice parameters used by
 *        @c akao_channel_start_note. Local to this file because it is the
 *        only consumer.
 */
typedef struct
{
    u8 articulation;
    u8 key;
    u8 attack_rate;
    u8 sustain_rate;
    u8 sustain_mode;
    u8 release_rate;
    u8 volume_scale;
    u8 pan_and_noise;
} AkaoNoteArticulationSlot;

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
 * @return The active-channel bitmask (g_akao_seq_channel0->w04.song.active_mask).
 * @see decomp.me (84.65%) https://decomp.me/scratch/XMCUh
 */
s32 akao_seq_tick_channels(s32 channel_base, s32 is_secondary)
{
    u32 var_a0;

    s32 var_s1;
    s32 var_s2;
    s32 var_s3;

    AkaoDriverFlags* driver_flags;

    var_a0 = g_akao_seq_channel0->tempo >> 16;
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

    g_akao_seq_channel0->tempo_acc = g_akao_seq_channel0->tempo_acc + var_a0;

    if ((g_akao_seq_channel0->tempo_acc & 0xFFFF0000U) || (g_akao_driver_mode_flags & 4))
    {
        g_akao_seq_channel0->tempo_acc = (s32)(g_akao_seq_channel0->tempo_acc & 0xFFFFU);

        var_s2 = channel_base;
        driver_flags = &g_akao_driver_flags;

        do
        {
            var_s1 = 1;
            var_s3 = g_akao_seq_channel0->w04.song.active_mask;

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
                        g_akao_seq_channel0->key_off_mask = (s32)(g_akao_seq_channel0->key_off_mask | var_s1);
                    }

                    akao_tick_channel_effects((AkaoChannelState*)var_s2, var_s1, 0);
                    var_s3 &= ~var_s1;
                }

                var_s2 += 0x118;
                var_s1 <<= 1;
            } while (var_s3 != 0);

            if (g_akao_seq_channel0->tempo_fade_ticks != 0)
            {
                g_akao_seq_channel0->tempo_fade_ticks = (u16)(g_akao_seq_channel0->tempo_fade_ticks - 1);
                g_akao_seq_channel0->tempo = g_akao_seq_channel0->tempo + g_akao_seq_channel0->tempo_step;
            }

            if (g_akao_seq_channel0->master_vol_fade_ticks != 0)
            {
                g_akao_seq_channel0->master_vol_fade_ticks = (s16)(g_akao_seq_channel0->master_vol_fade_ticks - 1);
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
                    /* Song role: unk66 is the beat counter and
                     * is_sfx_channel is beats-per-measure. */
                    if (g_akao_seq_channel0->unk66 == g_akao_seq_channel0->is_sfx_channel)
                    {
                        g_akao_seq_channel0->unk66 = 0;
                        g_akao_seq_channel0->measure = (u16)(g_akao_seq_channel0->measure + 1);
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

    return g_akao_seq_channel0->w04.song.active_mask;
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
    AkaoChannelState* channel;
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
        unk18_val = seq0->key_off_mask;
        ec1c += 1;
        g_akao_irq_frame_counter = ec1c;

        if ((unk18_val == 0) && (D_8004D40C == 0))
        {
            if (g_akao_seq_channel1 != 0)
            {
                if (g_akao_seq_channel1->key_off_mask != 0)
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
            if (ch28->w04.song.active_mask == 0)
            {
                g_akao_seq_channel1 = 0;
            }
            else if ((g_akao_seq_channel0->w04.song.active_mask | g_akao_seq_channel0->unk1C) == 0)
            {
                akao_copy_bytes((s32*)ch28, (s32*)g_akao_seq_channel0, 0x70);
                akao_copy_bytes((s32*)g_akao_pending_channels, &g_akao_seq_channels, 0x2300);
                {
                    AkaoChannelState* tmp = g_akao_seq_channel1;
                    g_akao_seq_channel1 = 0;
                    tmp->unk5E = 0;
                    tmp->w04.song.active_mask = 0;
                }
            }
        }
    }

    new_var = &g_akao_seq_master_state;

    /* Third conditional */
    if (((D_8004F758 | g_akao_seq_channel0->note_on_mask | D_8004D408) != 0) || ((g_akao_seq_channel1 != 0) && (g_akao_seq_channel1->note_on_mask != 0)))
    {
        akao_flush_voice_updates(D_8004D408);
    }

    /* Fourth conditional */
    if (g_akao_seq_channel0->w04.song.active_mask != 0)
    {
        akao_seq_tick_channels(&g_akao_seq_channels, 0);
    }

    /* Fifth conditional */
    if ((g_akao_seq_channel1 != 0) && (g_akao_seq_channel1->w04.song.active_mask != 0))
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
                channel = (AkaoChannelState*)&g_sfx_channels[0];

                do
                {
                    if (var_s3 & bitMask)
                    {
                        if (!(g_akao_driver_mode_flags & 2) || (channel->tempo_acc & 0x02000000))
                        {
                            (*(u32*)&channel->unk58)++;

                            temp_v1_2 = channel->unk66 - 1;
                            channel->unk66 = temp_v1_2;
                            temp_v0_2 = channel->unk68 - 1;
                            channel->unk68 = temp_v0_2;

                            if (temp_v1_2 == 0)
                            {
                                akao_seq_step_opcode(channel, bitMask);
                            }
                            else if (temp_v0_2 == 0)
                            {
                                g_akao_sfx_control.unkC |= bitMask;
                                g_akao_sfx_control.unk8 &= ~bitMask;
                            }
                            akao_tick_channel_effects(channel, bitMask, 1);
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
    u8* var_a1 = (u8*)arg0->seq_cursor;
    u8* new_var;
    u32 var_a2 = arg0->loop_depth;
    while (1)
    {
        u8 temp_v1 = *var_a1;
        if (temp_v1 < 0x9A)
        {
            if (temp_v1 >= 0x8F)
            {
                arg0->note_flags &= 0xFFFA;
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
                    if ((*var_a1) == (arg0->loop_count[var_a2] + 1))
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
                    /* 0x14 in the channel role is the subroutine return
                     * cursor, not a mask - this is the FE 0F "return" op. */
                    var_a1 = (u8*)arg0->note_on_mask;
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

            if ((*var_a1) == (arg0->loop_count[var_a2] + 1))
            {
                var_a1++;
                var_a2 = (var_a2 - 1) & 3;
            }
            else
            {
                /* 0x04..0x10 in the channel role is the loop-start cursor stack. */
/* The original reaches the loop stack through a pointer here, not
                 * as an array member as the loop opcodes do - the two spellings
                 * produce a different addu operand order. */
                var_a1 = ((u8**)&arg0->w04.song.active_mask)[var_a2];
            }
            continue;
        L_CB_common:
            arg0->note_flags &= 0xFFFA;

            var_a1++;
            continue;
        L_CA_common:
            if (!((u32)arg0->flags & 0x200000))
            {
                /* 0x04..0x10 in the channel role is the loop-start cursor stack. */
/* The original reaches the loop stack through a pointer here, not
                 * as an array member as the loop opcodes do - the two spellings
                 * produce a different addu operand order. */
                var_a1 = ((u8**)&arg0->w04.song.active_mask)[var_a2];
                continue;
            }

        labelA:
            arg0->note_flags &= 0xFFFA;

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
 * @see decomp.me (100%) https://decomp.me/scratch/0tdrk
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
        if (a2[0xD] != 0)
        {
            v1 = a2 + 0xD;
loop_first_288:
            if ((u8)v1[-0xB] >= key) goto search_done_288;
            v1 += 8;
            a2 += 8;
            if (*v1 == 0) goto search_done_288;
            goto loop_first_288;
        }
        goto search_done_288;
    }
    else if (key < *(s16*)((u8*)((((u32)a0 & key) | ((u32)a0 & ~key))) + 0xEE))
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

search_done_288:
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
 *             @c g_akao_seq_channel0->flags, the song-role note table).
 * @return Pitch result from @c akao_compute_pitch.
 * @see decomp.me (100%) https://decomp.me/scratch/9dRLX
 */
s32 akao_channel_start_note(void* channel, s32 channel_mask, s32 slot_idx)
{
    AkaoNoteArticulationSlot* slot;
    AkaoArticulation* art;
    u32 temp_a1;
    u16 tmp;
    u32 v1_idx;
    s32 ret;

    u32 chan_unk10;
    slot = (AkaoNoteArticulationSlot*)g_akao_seq_channel0->flags;
    chan_unk10 = g_akao_seq_channel0->w04.song.key_on_mask;
    v1_idx = g_akao_seq_channel0->note_on_mask;
    slot += slot_idx;
    chan_unk10 |= channel_mask;
    v1_idx &= channel_mask;
    g_akao_seq_channel0->w04.song.key_on_mask = chan_unk10;
    if (v1_idx)
    {
        g_akao_seq_channel0->key_off_mask |= channel_mask;
    }
    v1_idx = slot->articulation;
    temp_a1 = *((u32*)(((u8*)channel) + 0x34));
    *((s16*)(((u8*)channel) + 0x6A)) = (s16)v1_idx;
    art = (AkaoArticulation*)(g_akao_articulation_slots + v1_idx * 0x10);
    *((u32*)(((u8*)channel) + 0x104)) = art->sample_addr;
    *((u32*)(((u8*)channel) + 0x108)) = art->loop_addr;
    if (!(temp_a1 & 0x01000000))
    {
        tmp = (u16)(slot->attack_rate << 8);
    }
    else
    {
        tmp = (*((u16*)(((u8*)channel) + 0x10E))) & 0x7F00;
    }
    *((u16*)(((u8*)channel) + 0x10E)) = tmp;
    tmp = (u16)(slot->attack_rate << 8);
    *((u16*)(((u8*)channel) + 0x10E)) |= art->pitch_misc.half.lo & 0x80FF;
    if (!(temp_a1 & 0x08000000))
    {
        tmp = (*((u16*)(((u8*)channel) + 0x110))) & 0x201F;
        *((u16*)(((u8*)channel) + 0x110)) = tmp;
        *((u16*)(((u8*)channel) + 0x110)) |= slot->sustain_rate << 6;
    }
    else
    {
        *((u16*)(((u8*)channel) + 0x110)) &= 0x3FDF;
    }
    switch (slot->sustain_mode)
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
        *((u16*)(((u8*)channel) + 0x110)) |= slot->release_rate;
    }
    *((u16*)(((u8*)channel) + 0x110)) |= art->pitch_misc.half.hi & 0x20;
    ret = akao_compute_pitch(art, slot->key, *((s16*)(((u8*)channel) + 0xEC)), &((AkaoChannelState*)channel)->detune_pitch_delta);
    *((s16*)(((u8*)channel) + 0x112)) = (s16)slot->volume_scale;
    *((s16*)(((u8*)channel) + 0x90)) = (s16)(((slot->pan_and_noise & 0x7F) + 0x40) << 8);
    if (slot->pan_and_noise & 0x80)
    {
        g_akao_seq_channel0->noise_mask |= channel_mask;
    }
    else
    {
        u32 target_val = g_akao_seq_channel0->noise_mask;
        g_akao_seq_channel0->noise_mask = target_val & (~channel_mask);
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
extern s32 g_akao_lfo_waveforms[];

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
        var_s1 = *(*(u8**)&arg0->seq_cursor)++;

        if (var_s1 >= 0xA0U)
        {
            temp_v1 = var_s1 - 0xF0;
            if (var_s1 == 0xFE)
            {
                var_v0 = &g_akao_opcode_handlers_ext[*(*(u8**)&arg0->seq_cursor)++];
            }
            else if (temp_v1 < 0xEU)
            {
                var_s1 = temp_v1 * 0xB;
                arg0->unk66 = *(*(u8**)&arg0->seq_cursor)++;
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
                    if ((u32)arg0->flags & 0x200000)
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
        arg0->opcode_count++;
    } while (var_s1 >= 0xA1U);

    if (var_s1 == 0xA0)
    {
        if (arg0->is_sfx_channel == 0)
        {
            g_akao_seq_channel0->key_off_mask |= arg1;
        }
    }
    else
    {
        temp_a2 = akao_seq_skip_to_next_note(arg0) & 0xFF;
        temp_v1_2 = arg0->note_duration_adjust;
        if ((s16)arg0->note_duration_adjust != 0)
        {
            arg0->unk68 = temp_v1_2;
            arg0->unk66 = temp_v1_2;
        }
        if (arg0->unk66 != 0)
        {
            if ((temp_a2 >= 0x8FU) || ((temp_a2 < 0x84U) && !(arg0->note_flags & 5)))
            {
                arg0->unk68 -= 2;
            }
        }
        else
        {
            var_v1 = g_akao_note_duration_table[var_s1 % 11];
            arg0->unk66 = var_v1;
            if (((temp_a2 - 0x84) >= 0xBU) && !(arg0->note_flags & 5))
            {
                var_v1 -= 2;
            }
            arg0->unk68 = var_v1;
        }
        if ((arg0->is_sfx_channel == 0) && ((u32)arg0->flags & 0x40))
        {
            arg0->unk68 = arg0->unk66;
        }
        arg0->note_duration = arg0->unk66;
        arg0->update_flags |= 0x4000;
        if (var_s1 >= 0x8FU)
        {
            if (arg0->is_sfx_channel == 0)
            {
                g_akao_seq_channel0->note_on_mask &= ~arg1;
                if (*(volatile u32*)&arg0->voice < 0x18U)
                {
                    g_akao_seq_channel0->key_off_mask |= arg1;
                }
            }
            arg0->portamento_speed = 0U;
            arg0->pitch_lfo_value = 0;
            arg0->volume_lfo_value = 0;
            arg0->note_flags &= 0xFFFD;
            return;
        }
        if (var_s1 < 0x84U)
        {
            temp_v1_3 = (s32)arg0->flags;
            temp_s1 = (var_s1 / 11) + (arg0->octave * 0xC);
            if (temp_v1_3 & 8)
            {
                var_a2 = akao_channel_start_note(arg0, arg1, temp_s1);
            }
            else
            {
                if (!(arg0->note_flags & 2))
                {
                    if (arg0->is_sfx_channel == 0)
                    {
                        if (temp_v1_3 & 0x1000)
                        {
                            akao_bind_articulation_for_key(arg0, temp_s1, temp_a2);
                        }
                        g_akao_seq_channel0->w04.song.key_on_mask |= arg1;
                        if ((g_akao_seq_channel0->note_on_mask & arg1) && (*(volatile u32*)&arg0->voice < 0x18U))
                        {
                            g_akao_seq_channel0->key_off_mask |= arg1;
                        }
                        temp_a0 = arg0->note_expression_ticks;
                        if (temp_a0 != 0)
                        {
                            arg0->expression_fade_ticks = temp_a0;
                            /* Channel role: 0x5C/0x60 are the note-start
                             * expression preset, not tempo state. */
                            arg0->unk48 = *(s32*)&arg0->tempo_fade_ticks;
                            arg0->unk4C = arg0->unk60;
                        }
                    }
                    else
                    {
                        g_akao_sfx_control.unk4 |= arg1;
                    }
                    arg0->pitch_slide_ticks = 0U;
                }
                temp_v1_4 = arg0->portamento_speed;
                if ((temp_v1_4 != 0) && (arg0->prev_key != 0))
                {
                    arg0->pitch_slide_duration = temp_v1_4;
                    var_s1_2 = arg0->prev_key + arg0->prev_transpose;
                    temp_a0_2 = arg0->prev_transpose;
                    arg0->pitch_slide_delta = ((arg0->transpose + temp_s1) - arg0->prev_key) - temp_a0_2;
                    arg0->note_key = arg0->prev_key - (arg0->transpose - temp_a0_2);
                }
                else
                {
                    arg0->note_key = temp_s1;
                    var_s1_2 = temp_s1 + (s16)arg0->transpose;
                }
                var_a2 = akao_compute_pitch((AkaoArticulation*)((arg0->unk6A * 0x10) + g_akao_articulation_slots), var_s1_2, arg0->detune, &arg0->detune_pitch_delta);
                temp_v1_5 = arg0->pitch_scale;
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
            if (arg0->is_sfx_channel == 0)
            {
                g_akao_seq_channel0->note_on_mask |= arg1;
            }
            else
            {
                g_akao_sfx_control.unk8 |= arg1;
            }
            arg0->update_flags |= 0x13;
            temp_s1_2 = (s32)arg0->flags;
            var_v0_3 = temp_s1_2 & 2;
            if (temp_s1_2 & 1)
            {
                temp_v0_3 = arg0->pitch_lfo_depth;
                temp_v1_7 = (u32)(temp_v0_3 & 0x7F00) >> 8;
                if (!(temp_v0_3 & 0x8000))
                {
                    temp_lo = temp_v1_7 * ((u32)(var_a2 * 0xF) >> 8);
                }
                else
                {
                    temp_lo = temp_v1_7 * var_a2;
                }
                arg0->pitch_lfo_depth_scaled = (s16)(temp_lo >> 7);
                if (!(arg0->note_flags & 2))
                {
                    temp_v0_3 = arg0->pitch_lfo_delay;
                    arg0->unk1C = g_akao_lfo_waveforms[arg0->pitch_lfo_waveform];
                    arg0->pitch_lfo_restart = 1;
                    arg0->pitch_lfo_delay_ticks = temp_v0_3;
                }
                var_v0_3 = temp_s1_2 & 2;
            }
            if ((var_v0_3 != 0) && !(arg0->note_flags & 2))
            {
                temp_v0_3 = arg0->volume_lfo_delay;
                /* Channel role: 0x20 is the volume-LFO waveform cursor. */
                arg0->tempo = g_akao_lfo_waveforms[arg0->volume_lfo_waveform];
                arg0->volume_lfo_restart = 1;
                arg0->volume_lfo_delay_ticks = temp_v0_3;
            }
            arg0->pitch_lfo_value = 0;
            arg0->volume_lfo_value = 0;
            arg0->unk30 = 0;
        }
        temp_v0_4 = arg0->note_flags;
        arg0->note_flags = (temp_v0_4 & 0xFFFD) | ((temp_v0_4 & 1) * 2);
        if ((s16)arg0->pitch_slide_delta != 0)
        {
            temp_v0_5 = arg0->note_key + arg0->pitch_slide_delta;
            arg0->note_key = temp_v0_5;
            temp_a2_2 =
                akao_compute_pitch((AkaoArticulation*)((arg0->unk6A * 0x10) + g_akao_articulation_slots), temp_v0_5 + (s16)arg0->transpose, arg0->detune, &sp10)
                << 0x10;
            arg0->pitch_slide_ticks = arg0->pitch_slide_duration;
            arg0->pitch_slide_delta = 0U;
            arg0->pitch_slide_step = (s32)((s32)(temp_a2_2 - ((arg0->pitch << 0x10) + arg0->unk30)) / (s32)arg0->pitch_slide_ticks);
        }
        arg0->prev_key = arg0->note_key;
        arg0->prev_transpose = arg0->transpose;
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
    arg0->spu_adsr_low = tmp_c; /* store unk10E early */

    old_val = arg0->update_flags; /* load unk100 after that store */
    tmp_e = arg1->pitch_misc.half.hi;

    arg0->update_flags = old_val | 0x1FF80; /* OR and write back */
    arg0->spu_adsr_high = tmp_e;
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
    g_akao_sfx_control.reverb_mask &= mask;
    g_akao_sfx_control.noise_mask &= mask;
    g_akao_sfx_control.pitch_mod_mask &= mask;
    g_akao_sfx_control.unk4 &= mask;
    g_akao_sfx_control.unk8 &= mask;

    /* Zero out two fields in the object pointed to by channel */
    *(u32*)((u8*)channel + 0x28) = 0;
    *(u32*)((u8*)channel + 0x3C) = 0;
}

/**
 * @brief Remap an SFX articulation index into the selected 16-entry bank.
 * @see decomp.me (100%) https://decomp.me/scratch/oTcsG
 */
s32 akao_remap_sfx_articulation(s32 arg0, s32 arg1)
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
 *        When channel->is_sfx_channel is zero, clears release_mask bits from the
 *        seq-channel bitmasks in g_akao_seq_channel0.  If all active bits are
 *        cleared, also zeros g_akao_seq_pending_ticks, unk5E, and seq_cursor.  When
 *        channel->is_sfx_channel is non-zero, delegates to akao_sfx_release_channels.
 *        In both paths, channel->flags is cleared and the driver dirty flag
 *        (unk8) is OR'd with 0x110.
 * @param channel Pointer to the shared channel header.
 * @param release_mask Bit-mask of channels to release.
 * @see decomp.me (100%) https://decomp.me/scratch/vxrwL
 */
void akao_release_channels(AkaoChannelState* channel, u32 release_mask)
{
    s32 temp_v0;

    if (channel->is_sfx_channel == 0)
    {
        u32 tmp = ~release_mask;

        temp_v0 = g_akao_seq_channel0->w04.song.active_mask & tmp;
        g_akao_seq_channel0->w04.song.active_mask = temp_v0;

        if (temp_v0 == 0)
        {
            g_akao_seq_pending_ticks = 0;
            g_akao_seq_channel0->unk5E = 0;
            g_akao_seq_channel0->seq_cursor = 0;
        }

        g_akao_seq_channel0->note_on_mask &= tmp;
        g_akao_seq_channel0->w04.song.voice_alloc_low_mask &= tmp;
        g_akao_seq_channel0->w04.song.static_voice_mask &= tmp;
        g_akao_seq_channel0->reverb_mask &= tmp;
        g_akao_seq_channel0->noise_mask &= tmp;
        g_akao_seq_channel0->pitch_mod_mask &= tmp;
    }
    else
    {
        akao_sfx_release_channels(channel, release_mask);
    }

    channel->flags = 0;
    g_akao_driver_flags.unk8 |= 0x110;
}

/**
 * @brief Opcode handler: set the master tempo directly.
 *
 * Reads a 16-bit value from the sequence stream into the high half of
 * @c g_akao_seq_channel0->tempo (the per-tick rate added to the tempo_acc
 * accumulator in akao_seq_tick_channels) and clears the tempo-slide
 * countdown @c tempo_fade_ticks.
 *
 * @param arg0 Pointer to the channel's stream cursor; advanced by 2.
 * @see decomp.me (100%) https://decomp.me/scratch/GHtCl
 */
void akao_seq_op_set_tempo(u8** arg0)
{
    AkaoChannelState* ch = g_akao_seq_channel0;
    u32 temp_v1;

    temp_v1 = (*arg0)[0] << 0x10;
    ch->tempo = temp_v1;
    ch->tempo = temp_v1 | ((*arg0)[1] << 0x18);
    *arg0 += 2;
    ch->tempo_fade_ticks = 0;
}

/**
 * @brief Opcode handler: slide (ramp) the master tempo to a target.
 *
 * Reads a tick count into @c tempo_fade_ticks (defaulting to 0x100 when zero)
 * and a 16-bit target tempo, then computes the per-tick step @c tempo_step =
 * (target - current) / count so akao_seq_tick_channels ramps @c tempo to the
 * target over @c tempo_fade_ticks ticks.
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
    ch->tempo_fade_ticks = temp;
    *arg0 = ptr;
    if (temp == 0)
    {
        ch->tempo_fade_ticks = 0x100;
    }
    ptr = *arg0;
    combined = (ptr[0] << 16) | ((*(new_var = &ptr))[1] << 24);
    *arg0 = ptr + 2;
    masked = g_akao_seq_channel0->tempo & 0xFFFF0000;
    quotient = ((s32)(combined - masked)) / ((s32)g_akao_seq_channel0->tempo_fade_ticks);
    g_akao_seq_channel0->tempo = masked;
    g_akao_seq_channel0->tempo_step = quotient;
}

/**
 * @brief Set the sequence-wide stereo master volume from a signed 12-bit operand.
 * @see decomp.me (100%) https://decomp.me/scratch/Og38F
 */
void akao_seq_op_set_master_volume(u8** arg0)
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
    ch->master_vol_fade_ticks = 0;
    temp = val1 << 20;
    temp = temp | (val0 << 12);
    flags->unk8 |= 0x80;
    ch->unk48 = temp;
}

/**
 * @brief Slide the sequence-wide stereo master volume to a signed 12-bit target.
 * @see decomp.me (100%) https://decomp.me/scratch/w18Xw
 */
void akao_seq_op_slide_master_volume(u8** arg0)
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
    ch->master_vol_fade_ticks = ptr[0];
    *arg0 = (*(new_var = &ptr)) + 1;
    if (ch->master_vol_fade_ticks == 0)
    {
        ch->master_vol_fade_ticks = 0x100;
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
    new_var2->unk4C = (val_combined - temp_a0) / new_var2->master_vol_fade_ticks;
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
 * @brief Set the channel volume directly.
 * @see decomp.me (100%) https://decomp.me/scratch/BN6jO
 */
void akao_seq_op_set_volume(AkaoChannelState* arg0)
{
    u8* temp_v0;
    s16 tmp;

    temp_v0 = arg0->seq_cursor;
    tmp = (s16)(*temp_v0 << 8);
    arg0->seq_cursor = (s32)(u8*)(temp_v0 + 1);
    arg0->update_flags = (s32)(arg0->update_flags | 3);
    arg0->volume = tmp;
}

/**
 * @brief Slide the channel volume to a target over a specified tick count.
 * @see decomp.me (100%) https://decomp.me/scratch/CjVYW
 */
void akao_seq_op_slide_volume(AkaoChannelState* arg0)
{
    u16 temp_a0;
    u16 temp_v1;
    u8* temp_a1;
    u8* temp_v0;

    temp_v0 = arg0->seq_cursor;
    temp_v1 = *temp_v0;

    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    arg0->volume_fade_ticks = temp_v1;

    if (temp_v1 == 0)
    {
        arg0->volume_fade_ticks = 0x100U;
    }

    temp_a1 = arg0->seq_cursor;
    temp_a0 = arg0->volume;
    temp_a0 &= 0x7F00;

    temp_v1 = (s16)(((s32)(((*(arg0->seq_cursor++)) << 8) - temp_a0)) / ((s32)arg0->volume_fade_ticks));
    arg0->volume_step = temp_v1;

    arg0->seq_cursor = (u8*)(temp_a1 + 1);
    arg0->volume = temp_a0;
}

/**
 * @brief Set the channel expression multiplier directly.
 * @see decomp.me (100%) https://decomp.me/scratch/hGVxk
 */
void akao_seq_op_set_expression(AkaoChannelState* arg0)
{
    s8* temp_v0;

    temp_v0 = (s8*)arg0->seq_cursor;
    arg0->unk48 = (s32)(*temp_v0 << 0x17);
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    arg0->expression_fade_ticks = 0;
    arg0->update_flags = (s32)(arg0->update_flags | 3);
    arg0->note_expression_ticks = 0;
}

/**
 * @brief Slide the channel expression multiplier to a target.
 * @see decomp.me (100%) https://decomp.me/scratch/jLHGo
 */
void akao_seq_op_slide_expression(AkaoChannelState* arg0)
{
    s32 temp_a0;
    s32 temp_v1;
    u8* temp_a1;
    u8* temp_v0;
    s32 tmp;
    s32 new_var;

    temp_v0 = arg0->seq_cursor;

    temp_v1 = *temp_v0;
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    arg0->expression_fade_ticks = (u16)temp_v1;
    if (temp_v1 == 0)
    {
        arg0->expression_fade_ticks = 0x100U;
    }
    temp_a1 = arg0->seq_cursor;
    new_var = arg0->unk48;
    temp_a0 = new_var;
    temp_a0 &= 0xFFFF0000;

    tmp = (s32)((s32)(((s8)*temp_a1++ << 0x17) - temp_a0) / (s32)arg0->expression_fade_ticks);
    arg0->unk4C = tmp;
    arg0->seq_cursor = (u8*)(temp_a1);
    arg0->unk48 = temp_a0;
    arg0->note_expression_ticks = 0;
}

/**
 * @brief Configure the expression ramp that is restarted by the next note.
 * @see decomp.me (100%) https://decomp.me/scratch/aOcaK
 */
void akao_seq_op_set_note_expression_envelope(AkaoChannelState* arg0)
{
    s32 temp_v1;
    u8* temp_a0;
    s8* temp_v0;

    u32 tmp;

    temp_v0 = arg0->seq_cursor;

    tmp = *temp_v0;
    temp_v0++;

    arg0->seq_cursor = temp_v0;
    /* Channel role: 0x5C/0x60 are s32 and overlap the song-role u16s. */
    *(s32*)&arg0->tempo_fade_ticks = (s32)(tmp << 0x17);
    temp_v1 = *(u8*)temp_v0;
    temp_v0++;
    arg0->seq_cursor = (void*)((u8*)temp_v0);
    arg0->note_expression_ticks = (u16)temp_v1;

    if (temp_v1 == 0)
    {
        arg0->note_expression_ticks = 0x100U;
    }

    temp_a0 = arg0->seq_cursor;

    *(s32*)&arg0->unk60 =
        (s32)((s32)((*(s8*)temp_a0 << 0x17) - *(s32*)&arg0->tempo_fade_ticks) / (s32)arg0->note_expression_ticks);
    arg0->seq_cursor = (void*)((u8*)temp_a0 + 1);
}

/**
 * @brief Give sequence notes their full duration instead of an early key-off.
 * @see decomp.me (100%) https://decomp.me/scratch/cNPss
 */
void akao_seq_op_enable_full_gate(AkaoChannelState* arg0)
{
    arg0->flags = (s32)(arg0->flags | 0x40);
}

/**
 * @brief Restore the normal early-key-off gate duration.
 * @see decomp.me (100%) https://decomp.me/scratch/ryS0d
 */
void akao_seq_op_disable_full_gate(AkaoChannelState* arg0)
{
    arg0->flags = (s32)(arg0->flags & ~0x40);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/ZUdZO
 */
void akao_seq_op_set_pan(AkaoChannelState* arg0)
{
    u8* temp_v0;

    temp_v0 = arg0->seq_cursor;
    arg0->pan = (s16)(((*temp_v0 + 0x40) & 0xFF) << 8);
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    arg0->pan_fade_ticks = 0;
    arg0->update_flags = (s32)(arg0->update_flags | 3);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/MjjmM
 */
void akao_seq_op_slide_pan(AkaoChannelState* arg0)
{
    u16 temp_a0;
    s32 temp_v1;
    u8* temp_a1;
    u8* temp_v0;

    temp_v0 = arg0->seq_cursor;
    temp_v1 = *temp_v0;
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    arg0->pan_fade_ticks = (u16)temp_v1;
    if (temp_v1 == 0)
    {
        arg0->pan_fade_ticks = 0x100U;
    }
    temp_a1 = arg0->seq_cursor;
    temp_a0 = arg0->pan;
    temp_a0 &= 0xFF00;
    arg0->pan_step = (s16)((s32)((((*temp_a1 + 0x40) & 0xFF) << 8) - temp_a0) / (s32)arg0->pan_fade_ticks);
    arg0->seq_cursor = (u8*)(temp_a1 + 1);
    arg0->pan = temp_a0;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/FRzyk
 */
void akao_seq_op_set_octave(AkaoChannelState* arg0)
{
    u8* temp_v0;

    temp_v0 = arg0->seq_cursor;
    arg0->octave = (s16)*temp_v0;
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/2bdR3
 */
void akao_seq_op_increment_octave(AkaoChannelState* arg0)
{
    arg0->octave = (u16)((arg0->octave + 1) & 0xF);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/SJfbQ
 */
void akao_seq_op_decrement_octave(AkaoChannelState* arg0)
{
    arg0->octave = (u16)((arg0->octave - 1) & 0xF);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/1MXGr
 */
void akao_seq_op_set_mapped_articulation(AkaoChannelState* arg0)
{
    s32* temp_a1_2;
    s32 temp_a1;
    s32 var_s1;
    u8* temp_v0;

    temp_v0 = arg0->seq_cursor;
    temp_a1 = *temp_v0;
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    if (arg0->is_sfx_channel == 0)
    {
        var_s1 = temp_a1;
    }
    else
    {
        var_s1 = akao_remap_sfx_articulation(arg0->voice_alloc_base, temp_a1);
    }

    temp_a1_2 = (var_s1 * 0x10) + g_akao_articulation_slots;
    akao_channel_load_articulation_fields(arg0, temp_a1_2, *temp_a1_2);
    arg0->unk6A = (s16)var_s1;
    arg0->spu_volume_scale = 0;
    arg0->flags = (s32)(arg0->flags & 0xE6FFEFF7);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/GAl01
 */
void akao_seq_op_set_articulation(AkaoChannelState* arg0)
{
    s32 temp_s1;
    u8* temp_v0;

    temp_v0 = arg0->seq_cursor;
    temp_s1 = *temp_v0;
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    akao_channel_load_articulation_fields(arg0, (temp_s1 * 0x10) + g_akao_articulation_slots, 0x1010);
    arg0->unk6A = (s16)temp_s1;
    arg0->spu_volume_scale = 0;
    arg0->flags = (s32)(arg0->flags & 0xE6FFEFF7);
}

/**
 * @brief Select a key-to-articulation map from the current sequence bank.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void akao_seq_op_select_articulation_map(AkaoChannelState* arg0)
{
    s32 base;
    u16* entry;
    u8* temp_v0;
    u8 opcode;

    temp_v0 = arg0->seq_cursor;
    opcode = *temp_v0;
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    base = g_akao_seq_channel0->unk30;
    if (base != 0)
    {
        entry = (u16*)((opcode & 0xFF) * 2 + base);
        if ((u16)*entry > 0x8000U)
        {
            arg0->spu_volume_scale = 0;
            arg0->flags = (s32)(arg0->flags & ~0x1000);
            return;
        }
        arg0->key_off_mask = (s32)(base + *entry + 0x20);
        arg0->note_key = 0xFF;
        arg0->flags = (s32)((arg0->flags & 0xE6FFEFF7) | 0x1000);
    }
}

/**
 * @brief Reloads articulation fields for the channel's current articulation.
 * @param arg0 Channel state to update.
 * @see decomp.me (100%)
 */
void akao_seq_op_refresh_envelope(AkaoChannelState* arg0)
{
    AkaoArticulation* articulation;
    u16 tmp_c;
    u16 tmp_e;
    s32 old_value;
    s32 flags;

    articulation = (AkaoArticulation*)(g_akao_articulation_slots + (arg0->unk6A * 0x10));
    tmp_c = articulation->pitch_misc.half.lo;
    arg0->spu_adsr_low = tmp_c;
    tmp_e = articulation->pitch_misc.half.hi;
    old_value = arg0->update_flags;
    flags = arg0->flags;
    arg0->update_flags = old_value | 0xFF00;
    arg0->flags = flags & 0xE6FFFFFF;
    arg0->spu_adsr_high = tmp_e;
}

/**
 * @brief Reads a signed byte from the channel bytecode stream and stores it
 *        as a halfword at offset 0xEA.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void akao_seq_op_set_transpose(AkaoChannelState* arg0)
{
    u8* temp_v0;
    s8 val;

    temp_v0 = arg0->seq_cursor;
    val = *temp_v0;
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    arg0->transpose = val;
}

/**
 * @brief Reads a signed byte from the channel bytecode stream and adds it to
 *        the halfword at offset 0xEA.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void akao_seq_op_add_transpose(AkaoChannelState* arg0)
{
    u8* temp_v0;
    s8 val;

    temp_v0 = arg0->seq_cursor;
    val = *temp_v0;
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    arg0->transpose += val;
}

/**
 * @brief Configure a one-shot pitch slide duration and semitone delta.
 * @param arg0 Channel state whose bytecode cursor is advanced by two bytes.
 * @see decomp.me (100%)
 */
void akao_seq_op_set_pitch_slide(AkaoChannelState* arg0)
{
    u8* temp_v0;
    u8* temp_v0_2;
    s32 b;
    s8 b2;

    temp_v0 = arg0->seq_cursor;
    b = *temp_v0;
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    arg0->pitch_slide_duration = b;
    if (b == 0)
    {
        arg0->pitch_slide_duration = 0x100;
    }
    temp_v0_2 = arg0->seq_cursor;
    b2 = *temp_v0_2;
    arg0->seq_cursor = (u8*)(temp_v0_2 + 1);
    arg0->pitch_slide_delta = b2;
}

/**
 * @brief Enable automatic portamento between successive notes.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void akao_seq_op_enable_portamento(AkaoChannelState* arg0)
{
    u8* temp_v0;
    s32 b;

    temp_v0 = arg0->seq_cursor;
    b = *temp_v0;
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    arg0->portamento_speed = b;
    if (b == 0)
    {
        arg0->portamento_speed = 0x100;
    }
    arg0->prev_transpose = 0;
    arg0->prev_key = 0;
    arg0->note_flags = 1;
}

/**
 * @brief Disable automatic portamento between notes.
 * @param arg0 Channel state.
 * @see decomp.me (100%)
 */
void akao_seq_op_disable_portamento(AkaoChannelState* arg0)
{
    arg0->portamento_speed = 0;
}

/**
 * @brief Set fine pitch detune and recompute its pitch-register delta.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void akao_seq_op_set_detune(AkaoChannelState* arg0)
{
    s32 scale;
    u8* next;
    u32 prod;
    u32 result;
    u8* temp_v0;

    temp_v0 = arg0->seq_cursor;
    next = (u8*)(temp_v0 + 1);
    arg0->detune = (s16)(s8)*temp_v0;
    result = (u8)arg0->detune;
    scale = arg0->pitch;
    prod = scale * result;
    arg0->seq_cursor = next;
    if (arg0->detune < 0)
    {
        result = (prod >> 8) - scale;
    }
    else
    {
        result = prod >> 7;
    }
    arg0->detune_pitch_delta = result;
    arg0->update_flags = (s32)(arg0->update_flags | 0x10);
}

/**
 * @brief Add to fine pitch detune and recompute its pitch-register delta.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void akao_seq_op_add_detune(AkaoChannelState* arg0)
{
    s32 scale;
    u8* temp_v0;
    u32 prod;
    u32 result;

    temp_v0 = arg0->seq_cursor;
    scale = arg0->pitch;
    arg0->detune += (s8)*temp_v0;
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    result = (u8)arg0->detune;
    prod = scale * result;
    if (arg0->detune < 0)
    {
        result = (prod >> 8) - scale;
    }
    else
    {
        result = prod >> 7;
    }
    arg0->detune_pitch_delta = result;
    arg0->update_flags = (s32)(arg0->update_flags | 0x10);
}

/**
 * @brief Start the channel pitch LFO, selecting its delay, period, waveform,
 *        and scaled depth.
 * @param arg0 Channel state whose bytecode cursor is advanced.
 * @note @c temp_a0 must be widened past u16 so gcc keeps the second mult
 *       operand order (@c hi first) matching the other branch.
 * @see decomp.me (100%)
 */
void akao_seq_op_start_pitch_lfo(AkaoChannelState* arg0)
{
    AkaoChannelState* p;
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
    p->flags = (s32)(p->flags | 1);
    if (p->is_sfx_channel != 0)
    {
        temp_v0 = p->seq_cursor;
        p->pitch_lfo_delay = 0;
        temp_v1 = *temp_v0;
        p->seq_cursor = (u8*)(temp_v0 + 1);
        if (temp_v1 != 0)
        {
            p->pitch_lfo_depth = temp_v1 << 8;
        }
    }
    else
    {
        temp_v0_2 = p->seq_cursor;
        p->pitch_lfo_delay = *temp_v0_2;
        p->seq_cursor = (u8*)(temp_v0_2 + 1);
    }
    temp_v0_3 = p->seq_cursor;
    temp_v1_2 = *temp_v0_3;
    p->seq_cursor = (u8*)(temp_v0_3 + 1);
    p->pitch_lfo_period = temp_v1_2;
    if (temp_v1_2 == 0)
    {
        p->pitch_lfo_period = 0x100;
    }
    temp_v0_4 = p->seq_cursor;
    flags = p->pitch_lfo_depth;
    raw = *temp_v0_4;
    p->seq_cursor = (u8*)(temp_v0_4 + 1);
    p->pitch_lfo_waveform = raw;
    /* This op reads only the low halfword of the 0x2C pitch word (lhu). */
    temp_a0 = *(u16*)&p->pitch;
    hi = (u32)(flags & 0x7F00) >> 8;
    if (!(flags & 0x8000))
    {
        var_lo = hi * ((s32)(temp_a0 * 0xF) >> 8);
    }
    else
    {
        var_lo = hi * temp_a0;
    }
    p->pitch_lfo_depth_scaled = var_lo >> 7;
    p->unk1C = g_akao_lfo_waveforms[p->pitch_lfo_waveform];
    p->pitch_lfo_delay_ticks = p->pitch_lfo_delay;
    p->pitch_lfo_restart = 1;
}

/**
 * @brief Set the active pitch-LFO depth and recompute its scaled depth.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @note The @c flags re-read of 0xAE is required to keep the base pointer in
 *       a1 to match; it leaves one residual @c andi (see status).
 * @see decomp.me (90.96%)
 */
void akao_seq_op_set_pitch_lfo_depth(AkaoChannelState* arg0)
{
    AkaoChannelState* p;
    s32 scale;
    u8* temp_v0;
    u32 shifted;
    u32 flags;
    u32 hi;
    u32 var_lo;

    p = arg0;
    temp_v0 = p->seq_cursor;
    shifted = *temp_v0 << 8;
    p->seq_cursor = (u8*)(temp_v0 + 1);
    scale = p->pitch;
    p->pitch_lfo_depth = shifted;
    flags = p->pitch_lfo_depth;
    hi = (u32)(flags & 0x7F00) >> 8;
    if (!(flags & 0x8000))
    {
        var_lo = hi * ((s32)(scale * 0xF) >> 8);
    }
    else
    {
        var_lo = hi * scale;
    }
    p->pitch_lfo_depth_scaled = var_lo >> 7;
}

/**
 * @brief Slide the pitch-LFO depth to a target over a tick count.
 * @param arg0 Channel state whose bytecode cursor is advanced by two bytes.
 * @see decomp.me (100%)
 */
void akao_seq_op_slide_pitch_lfo_depth(AkaoChannelState* arg0)
{
    u8* temp_a1;
    s32 divisor;
    s32 result;

    temp_a1 = arg0->seq_cursor;
    divisor = *temp_a1;
    temp_a1 += 1;
    arg0->seq_cursor = temp_a1;
    if (divisor == 0)
    {
        divisor = 0x100;
    }
    result = ((s32)(*temp_a1 << 8) - arg0->pitch_lfo_depth) / divisor;
    arg0->seq_cursor = (u8*)(temp_a1 + 1);
    arg0->pitch_lfo_depth_fade_ticks = divisor;
    arg0->pitch_lfo_depth_step = result;
}

/**
 * @brief AKAO opcode handler: clears field 0xF4, clears bit 0 of the flags at
 *        0x34, and sets bit 0x10 in the flags at 0x100.
 * @param arg0 Channel state.
 * @see decomp.me (100%)
 */
void akao_seq_op_stop_pitch_lfo(AkaoChannelState* arg0)
{
    arg0->pitch_lfo_value = 0;
    arg0->flags = arg0->flags & ~1;
    arg0->update_flags = arg0->update_flags | 0x10;
}

/**
 * @brief Start the channel volume LFO, selecting its delay, period, waveform,
 *        and depth.
 * @param arg0 Channel state whose bytecode cursor is advanced by three bytes.
 * @see decomp.me (100%)
 */
void akao_seq_op_start_volume_lfo(AkaoChannelState* arg0)
{
    AkaoChannelState* p;
    u8* temp_v0;
    u8* temp_v0_2;
    u8* temp_v1;
    s32 temp_a0;
    s32 temp_v1_2;
    u32 raw;

    p = arg0;
    temp_v1 = p->seq_cursor;
    p->flags = (s32)(p->flags | 2);
    temp_a0 = *temp_v1;
    p->seq_cursor = (u8*)(temp_v1 + 1);
    if (p->is_sfx_channel != 0)
    {
        p->volume_lfo_delay = 0;
        if (temp_a0 != 0)
        {
            p->volume_lfo_depth = (temp_a0 & 0x7F) << 8;
        }
    }
    else
    {
        p->volume_lfo_delay = temp_a0;
    }
    temp_v0 = p->seq_cursor;
    temp_v1_2 = *temp_v0;
    p->seq_cursor = (u8*)(temp_v0 + 1);
    p->volume_lfo_period = temp_v1_2;
    if (temp_v1_2 == 0)
    {
        p->volume_lfo_period = 0x100;
    }
    temp_v0_2 = p->seq_cursor;
    raw = *temp_v0_2;
    p->seq_cursor = (u8*)(temp_v0_2 + 1);
    p->volume_lfo_waveform = raw;
    p->tempo = g_akao_lfo_waveforms[p->volume_lfo_waveform];
    p->volume_lfo_delay_ticks = p->volume_lfo_delay;
    p->volume_lfo_restart = 1;
}

/**
 * @brief Set the active volume-LFO depth.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void akao_seq_op_set_volume_lfo_depth(AkaoChannelState* arg0)
{
    u8* temp_v0;
    s32 b;

    temp_v0 = arg0->seq_cursor;
    b = *temp_v0;
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    arg0->volume_lfo_depth = (b & 0x7F) << 8;
}

/**
 * @brief Slide the volume-LFO depth to a target over a tick count.
 * @param arg0 Channel state whose bytecode cursor is advanced by two bytes.
 * @see decomp.me (100%)
 */
void akao_seq_op_slide_volume_lfo_depth(AkaoChannelState* arg0)
{
    u8* temp_a1;
    s32 divisor;
    s32 result;

    temp_a1 = arg0->seq_cursor;
    divisor = *temp_a1;
    temp_a1 += 1;
    arg0->seq_cursor = temp_a1;
    if (divisor == 0)
    {
        divisor = 0x100;
    }
    result = (((s32)(*temp_a1 & 0x7F) << 8) - arg0->volume_lfo_depth) / divisor;
    arg0->seq_cursor = (u8*)(temp_a1 + 1);
    arg0->volume_lfo_depth_fade_ticks = divisor;
    arg0->volume_lfo_depth_step = result;
}

/**
 * @brief AKAO opcode handler: clears field 0xF6, clears bit 1 of the flags at
 *        0x34, and sets bits 0x3 in the flags at 0x100.
 * @param arg0 Channel state.
 * @see decomp.me (100%)
 */
void akao_seq_op_stop_volume_lfo(AkaoChannelState* arg0)
{
    arg0->volume_lfo_value = 0;
    arg0->flags = arg0->flags & ~2;
    arg0->update_flags = arg0->update_flags | 3;
}

/**
 * @brief Start the channel pan LFO, selecting its period and waveform.
 * @param arg0 Channel state whose bytecode cursor is advanced by two bytes.
 * @see decomp.me (100%)
 */
void akao_seq_op_start_pan_lfo(AkaoChannelState* arg0)
{
    u8* temp_v0;
    u8* temp_v0_2;
    s32 b1;
    u32 raw;

    arg0->flags = (s32)(arg0->flags | 4);
    temp_v0 = arg0->seq_cursor;
    b1 = *temp_v0;
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    arg0->pan_lfo_period = b1;
    if (b1 == 0)
    {
        arg0->pan_lfo_period = 0x100;
    }
    temp_v0_2 = arg0->seq_cursor;
    raw = *temp_v0_2;
    arg0->seq_cursor = (u8*)(temp_v0_2 + 1);
    arg0->pan_lfo_waveform = raw;
    arg0->tempo_step = g_akao_lfo_waveforms[arg0->pan_lfo_waveform];
    arg0->pan_lfo_restart = 1;
}

/**
 * @brief Set the active pan-LFO depth.
 * @param arg0 Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void akao_seq_op_set_pan_lfo_depth(AkaoChannelState* arg0)
{
    u8* temp_v0;
    s32 b;

    temp_v0 = arg0->seq_cursor;
    b = *temp_v0;
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    arg0->pan_lfo_depth = b << 7;
}

/**
 * @brief Slide the pan-LFO depth to a target over a tick count.
 * @param arg0 Channel state whose bytecode cursor is advanced by two bytes.
 * @see decomp.me (100%)
 */
void akao_seq_op_slide_pan_lfo_depth(AkaoChannelState* arg0)
{
    u8* temp_a1;
    s32 divisor;
    s32 result;

    temp_a1 = arg0->seq_cursor;
    divisor = *temp_a1;
    temp_a1 += 1;
    arg0->seq_cursor = temp_a1;
    if (divisor == 0)
    {
        divisor = 0x100;
    }
    result = (((s32)*temp_a1 << 7) - arg0->pan_lfo_depth) / divisor;
    arg0->seq_cursor = (u8*)(temp_a1 + 1);
    arg0->pan_lfo_depth_fade_ticks = divisor;
    arg0->pan_lfo_depth_step = result;
}

/**
 * @brief AKAO opcode handler: clears field 0xF8, clears bit 2 of the flags at
 *        0x34, and sets bits 0x3 in the flags at 0x100.
 * @param arg0 Channel state.
 * @see decomp.me (100%)
 */
void akao_seq_op_stop_pan_lfo(AkaoChannelState* arg0)
{
    arg0->pan_lfo_value = 0;
    arg0->flags = arg0->flags & ~4;
    arg0->update_flags = arg0->update_flags | 3;
}

/**
 * @brief AKAO opcode handler: OR-sets a caller-supplied flag mask into either
 *        the SFX control block or the primary sequence channel (depending on
 *        whether this channel is an SFX channel), then raises driver flags 0x110.
 * @param arg0 Channel state; @c is_sfx_channel selects SFX vs sequence routing.
 * @param arg1 Flag bitmask to OR in.
 * @note Residual: the g_akao_seq_channel0 %hi colors to v0 not v1 (one lui
 *       register), a gcc 2.8 coloring tie-break the permuter cannot move.
 * @see decomp.me (99.58%)
 */
void akao_seq_op_enable_reverb(AkaoChannelState* arg0, s32 arg1)
{
    if (arg0->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->reverb_mask |= arg1;
    }
    else
    {
        g_akao_sfx_control.reverb_mask |= arg1;
    }
    g_akao_driver_flags.unk8 |= 0x110;
}

/**
 * @brief AKAO opcode handler: AND-clears a caller-supplied flag mask from either
 *        the SFX control block or the primary sequence channel, raises driver
 *        flags 0x110, and clears channel field 0xD4.
 * @param arg0 Channel state; @c is_sfx_channel selects SFX vs sequence routing.
 * @param arg1 Flag bitmask to clear (applied as @c &= ~arg1).
 * @note Residual: the seq-channel path register coloring differs (5 rows), a
 *       gcc 2.8 coloring tie-break the permuter cannot move (shared with
 *       akao_seq_op_enable_reverb).
 * @see decomp.me (98.27%)
 */
void akao_seq_op_disable_reverb(AkaoChannelState* arg0, s32 arg1)
{
    if (arg0->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->reverb_mask &= ~arg1;
    }
    else
    {
        g_akao_sfx_control.reverb_mask &= ~arg1;
    }
    g_akao_driver_flags.unk8 |= 0x110;
    arg0->reverb_toggle_ticks = 0;
}

/**
 * @brief AKAO opcode handler: OR-sets a caller-supplied flag mask into the
 *        sequence channel (0x44) when this channel is a sequence channel, or
 *        into the SFX control block (0x24) when SFX flag 0x10000 is set, then
 *        raises driver flags 0x100.
 * @param arg0 Channel state; @c is_sfx_channel selects sequence routing, @c flags gates SFX.
 * @param arg1 Flag bitmask to OR in.
 * @note Residual: the g_akao_seq_channel0 %hi colors to v0 not v1 (one lui
 *       register), a gcc 2.8 coloring tie-break shared with akao_seq_op_enable_reverb.
 * @see decomp.me (99.66%)
 */
void akao_seq_op_enable_pitch_modulation(AkaoChannelState* arg0, s32 arg1)
{
    if (arg0->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->pitch_mod_mask |= arg1;
    }
    else if (arg0->flags & 0x10000)
    {
        g_akao_sfx_control.pitch_mod_mask |= arg1;
    }
    g_akao_driver_flags.unk8 |= 0x100;
}

/**
 * @brief AKAO opcode handler: AND-clears a caller-supplied flag mask from either
 *        the sequence channel (0x44) or the SFX control block (0x24), raises
 *        driver flags 0x100, and clears channel field 0xD6.
 * @param arg0 Channel state; @c is_sfx_channel selects SFX vs sequence routing.
 * @param arg1 Flag bitmask to clear (applied as @c &= ~arg1).
 * @note Residual: the seq-channel path register coloring differs (5 rows), a
 *       gcc 2.8 coloring tie-break shared with the reverb handlers.
 * @see decomp.me (98.27%)
 */
void akao_seq_op_disable_pitch_modulation(AkaoChannelState* arg0, s32 arg1)
{
    if (arg0->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->pitch_mod_mask &= ~arg1;
    }
    else
    {
        g_akao_sfx_control.pitch_mod_mask &= ~arg1;
    }
    g_akao_driver_flags.unk8 |= 0x100;
    arg0->pitch_mod_toggle_ticks = 0;
}

/**
 * @brief AKAO opcode handler: OR-sets a caller-supplied flag mask into either
 *        the sequence channel (0x40) or the SFX control block (0x20), then
 *        raises driver flags 0x100.
 * @param arg0 Channel state; @c is_sfx_channel selects SFX vs sequence routing.
 * @param arg1 Flag bitmask to OR in.
 * @note Residual: the g_akao_seq_channel0 %hi coloring tie-break shared with
 *       akao_seq_op_enable_reverb.
 * @see decomp.me (99.58%)
 */
void akao_seq_op_enable_noise(AkaoChannelState* arg0, s32 arg1, s32 arg2)
{
    if (arg0->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->noise_mask |= arg1;
    }
    else
    {
        g_akao_sfx_control.noise_mask |= arg1;
    }
    g_akao_driver_flags.unk8 |= 0x100;
}

/**
 * @brief AKAO opcode handler: AND-clears a caller-supplied flag mask from either
 *        the sequence channel (0x40) or the SFX control block (0x20), then
 *        raises driver flags 0x100.
 * @param arg0 Channel state; @c is_sfx_channel selects SFX vs sequence routing.
 * @param arg1 Flag bitmask to clear (applied as @c &= ~arg1).
 * @note Residual: the seq-channel path register coloring differs (5 rows), a
 *       gcc 2.8 coloring tie-break shared with the reverb handlers.
 * @see decomp.me (98.13%)
 */
void akao_seq_op_disable_noise(AkaoChannelState* arg0, s32 arg1)
{
    if (arg0->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->noise_mask &= ~arg1;
    }
    else
    {
        g_akao_sfx_control.noise_mask &= ~arg1;
    }
    g_akao_driver_flags.unk8 |= 0x100;
}

/**
 * @brief Enable tied notes; subsequent notes change pitch without retriggering.
 * @param arg0 Channel state.
 * @see decomp.me (100%)
 */
void akao_seq_op_enable_note_tie(AkaoChannelState* arg0)
{
    arg0->note_flags = 1;
}

/**
 * @brief No-op handler for primary opcode 0xCD.
 * @see decomp.me (100%)
 */
void akao_seq_op_nop_cd(void)
{
}

/**
 * @brief Give SFX notes their full duration instead of an early key-off.
 * @param arg0 Channel state; @c is_sfx_channel selects whether the store happens.
 * @see decomp.me (100%)
 */
void akao_seq_op_enable_sfx_full_gate(AkaoChannelState* arg0)
{
    if (arg0->is_sfx_channel != 0)
    {
        arg0->note_flags = 4;
    }
}

/**
 * @brief No-op handler for primary opcode 0xD1.
 * @see decomp.me (100%)
 */
void akao_seq_op_nop_d1(void)
{
}

/**
 * decomp.me (100%) https://decomp.me/scratch/Get0N
 */
void akao_seq_op_set_noise_frequency(AkaoChannelState* arg0)
{
    s16 temp_a1;
    u8* temp_v0;

    temp_v0 = arg0->seq_cursor;
    temp_a1 = *temp_v0;
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    if (arg0->is_sfx_channel == 0)
    {
        if (temp_a1 & 0xC0)
        {
            g_akao_seq_channel0->noise_freq = (u16)((g_akao_seq_channel0->noise_freq + (temp_a1 & 0x3F)) & 0x3F);
        }
        else
        {
            g_akao_seq_channel0->noise_freq = (u16)temp_a1;
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
void akao_seq_op_set_adsr_attack(AkaoChannelState* arg0)
{
    u8* ptr;
    u32 byte_val;
    u32 new_unk100;
    u32 new_unk34;
    u16 new_unk10E;

    ptr = arg0->seq_cursor;
    byte_val = *ptr;
    arg0->seq_cursor = ptr + 1;

    new_unk100 = arg0->update_flags | 0x900;
    arg0->update_flags = new_unk100;

    new_unk34 = arg0->flags | 0x01000000;

    new_unk10E = (arg0->spu_adsr_low & 0x80FF) | ((u16)byte_val << 8);

    arg0->flags = new_unk34;
    arg0->spu_adsr_low = new_unk10E;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/ID5s7
 */
void akao_seq_op_set_adsr_decay(AkaoChannelState* arg0, AkaoChannelState* arg1)
{
    u8* ptr;
    u32 byte_val;
    u32 new_unk100;
    u16 new_unk10E;

    ptr = arg0->seq_cursor;
    byte_val = *ptr;
    arg0->seq_cursor = ptr + 1;

    new_unk100 = arg0->update_flags | 0x1000;
    new_unk10E = ((arg0->spu_adsr_low & 0xFF0F) | (byte_val * 0x10));

    arg0->update_flags = new_unk100;
    arg0->spu_adsr_low = new_unk10E;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/ZOmaE
 */
void akao_seq_op_set_adsr_sustain_level(AkaoChannelState* arg0, AkaoChannelState* arg1)
{
    u8* ptr;
    u32 byte_val;
    u32 new_unk100;
    u16 new_unk10E;

    ptr = arg0->seq_cursor;
    byte_val = *ptr;
    arg0->seq_cursor = ptr + 1;
    new_unk100 = arg0->update_flags | 0x8000;
    new_unk10E = (arg0->spu_adsr_low & 0xFFF0) | byte_val;
    arg0->update_flags = new_unk100;
    arg0->spu_adsr_low = new_unk10E;
}


/**
 * decomp.me (100%) https://decomp.me/scratch/Tun26
 */
void akao_seq_op_set_adsr_sustain_rate(AkaoChannelState* arg0)
{
    u8* ptr;
    u32 byte_val;
    u32 new_unk100;
    u32 new_unk34;
    u16 new_unk110;

    ptr = arg0->seq_cursor;
    byte_val = *ptr;
    arg0->seq_cursor = ptr + 1;

    new_unk100 = arg0->update_flags | 0x2200;
    new_unk34 = arg0->flags | 0x08000000;
    new_unk110 = (arg0->spu_adsr_high & 0xE03F) | (byte_val << 6);

    arg0->update_flags = new_unk100;
    arg0->flags = new_unk34;
    arg0->spu_adsr_high = new_unk110;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/DR9uQ
 */
void akao_seq_op_set_adsr_release_rate(AkaoChannelState* arg0)
{
    u8* ptr;
    u32 byte_val;
    u32 new_unk100;
    u32 new_unk34;
    u16 new_unk110;

    ptr = arg0->seq_cursor;
    byte_val = *ptr;
    arg0->seq_cursor = ptr + 1;

    new_unk100 = arg0->update_flags | 0x4400;
    new_unk34 = arg0->flags | 0x10000000;
    new_unk110 = (arg0->spu_adsr_high & 0xFFE0) | byte_val;

    arg0->update_flags = new_unk100;
    arg0->flags = new_unk34;
    arg0->spu_adsr_high = new_unk110;
}


/**
 * decomp.me (100%) https://decomp.me/scratch/dP97Y
 */
void akao_seq_op_set_adsr_attack_mode(AkaoChannelState* arg0)
{
    u8* ptr;
    u32 byte_val;
    u16 new_unk10E;

    ptr = arg0->seq_cursor;
    byte_val = *ptr;
    arg0->seq_cursor = ptr + 1;

    new_unk10E = arg0->spu_adsr_low & 0x7FFF;
    arg0->spu_adsr_low = new_unk10E;
    if (byte_val == 5)
    {
        new_unk10E |= 0x8000;
        arg0->spu_adsr_low = new_unk10E;
    }

    arg0->update_flags = arg0->update_flags | 0x100;
}


/**
 * decomp.me (100%) https://decomp.me/scratch/8od0h
 */
void akao_seq_op_set_adsr_sustain_mode(AkaoChannelState* arg0)
{
    s32 temp_a0;
    u16 temp_v1;
    u8* temp_v0;
    u32 byte_val;

    temp_v0 = arg0->seq_cursor;
    byte_val = *temp_v0;
    temp_v1 = arg0->spu_adsr_high & 0x3FFF;
    arg0->seq_cursor = (u8*)(temp_v0 + 1);
    temp_a0 = byte_val & 0xFFFF;
    arg0->spu_adsr_high = temp_v1;

    switch (temp_a0)
    {
    case 3:
        arg0->spu_adsr_high = temp_v1 | 0x4000;
        break;
    case 5:
        arg0->spu_adsr_high = temp_v1 | 0x8000;
        break;
    case 7:
        arg0->spu_adsr_high = temp_v1 | 0xC000;
        break;
    }

    arg0->update_flags = (s32)(arg0->update_flags | 0x200);
}


/**
 * decomp.me (100%) https://decomp.me/scratch/1Pqa0
 */
void akao_seq_op_set_adsr_release_mode(AkaoChannelState* arg0)
{

    u8* temp_v0;
    u16 new_unk110;
    u32 byte_val;

    temp_v0 = arg0->seq_cursor;
    byte_val = *temp_v0;
    arg0->seq_cursor = (u8*)(temp_v0 + 1);

    new_unk110 = arg0->spu_adsr_high & 0xFFDF;
    arg0->spu_adsr_high = new_unk110;

    if (byte_val == 7)
    {
        new_unk110 = (u16)(new_unk110 | 0x20);
        arg0->spu_adsr_high = new_unk110;
    }

    arg0->update_flags = (s32)(arg0->update_flags | 0x400);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/rLRQL
 */
void func_8002C984(u8** arg0)
{
    u8* temp_v0;

    temp_v0 = *arg0;
    g_akao_seq_channel0->voice_alloc_base = (s32)*temp_v0;
    *arg0 = temp_v0 + 1;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/9FX05
 */
void func_8002C9A4(void)
{
    g_akao_seq_channel0->voice_alloc_base = 0;
}


/**
 * decomp.me (100%) https://decomp.me/scratch/pTtu4
 */
void akao_seq_op_loop_start(AkaoChannelState* arg0)
{
    arg0->loop_depth = (arg0->loop_depth + 1) & 3;
    arg0->w04.loop_cursor[arg0->loop_depth] = arg0->seq_cursor;
    arg0->loop_count[arg0->loop_depth] = 0;
    arg0->loop_opcode_count[arg0->loop_depth] = arg0->opcode_count;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/xSadm
 */
void akao_seq_op_loop_end(AkaoChannelState* arg0)
{
    u32 var_a1;

    var_a1 = *arg0->seq_cursor;
    arg0->seq_cursor++;

    if (var_a1 == 0)
    {
        var_a1 = 0x100;
    }

    if (++arg0->loop_count[arg0->loop_depth] != var_a1)
    {
        arg0->seq_cursor = arg0->w04.loop_cursor[arg0->loop_depth];
        arg0->opcode_count = arg0->loop_opcode_count[arg0->loop_depth];
        return;
    }

    arg0->loop_depth = (arg0->loop_depth - 1) & 3;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/XFsED
 */
void akao_seq_op_branch_on_loop_last(AkaoChannelState* arg0)
{
    u8* temp_a1;
    u32 var_v1;

    temp_a1 = arg0->seq_cursor;
    var_v1 = *temp_a1;
    temp_a1++;
    arg0->seq_cursor = temp_a1;

    if (var_v1 == 0)
    {
        var_v1 = 0x100;
    }

    if (arg0->loop_count[arg0->loop_depth] + 1 != var_v1)
    {
        arg0->seq_cursor = temp_a1 + 2;
        return;
    }

    arg0->seq_cursor = temp_a1 + (s16)(temp_a1[0] | (temp_a1[1] << 8));
}

/**
 * decomp.me (100%) https://decomp.me/scratch/Gx2w7
 */
void akao_seq_op_branch_and_end_loop(AkaoChannelState* arg0)
{
    u8* temp_a1;
    u32 var_v1;

    temp_a1 = arg0->seq_cursor;
    var_v1 = *temp_a1;
    temp_a1++;
    arg0->seq_cursor = temp_a1;

    if (var_v1 == 0)
    {
        var_v1 = 0x100;
    }

    if (arg0->loop_count[arg0->loop_depth] + 1 != var_v1)
    {
        arg0->seq_cursor = temp_a1 + 2;
        return;
    }

    arg0->seq_cursor = temp_a1 + (s16)(temp_a1[0] | (temp_a1[1] << 8));
    arg0->loop_depth = (arg0->loop_depth - 1) & 3;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/Eytqk
 */
void akao_seq_op_repeat_loop(AkaoChannelState* arg0)
{
    arg0->loop_count[arg0->loop_depth]++;
    arg0->seq_cursor = arg0->w04.loop_cursor[arg0->loop_depth];
    arg0->opcode_count = arg0->loop_opcode_count[arg0->loop_depth];
}


/**
 * decomp.me (100%) https://decomp.me/scratch/FnAXw
 */
void akao_seq_op_set_note_duration(AkaoChannelState* arg0)
{
    s32 temp_v1;

    temp_v1 = *arg0->seq_cursor;
    arg0->seq_cursor++;

    arg0->note_duration_adjust = 0;
    arg0->unk68 = temp_v1;
    arg0->unk66 = temp_v1;
    arg0->note_duration = temp_v1;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/PBFzu
 */
void akao_seq_op_adjust_note_duration(AkaoChannelState* arg0)
{
    s32 var_v1;

    var_v1 = (s8)*arg0->seq_cursor;
    arg0->seq_cursor++;

    if (var_v1 != 0)
    {
        var_v1 += arg0->note_duration;

        if (var_v1 <= 0)
        {
            var_v1 = 1;
        }
        else if (var_v1 >= 256)
        {
            var_v1 = 255; // 0xFF
        }
    }

    arg0->note_duration_adjust = var_v1;
}


/**
 * decomp.me (100%) https://decomp.me/scratch/CB7Yy
 */
void func_8002CC44(AkaoChannelState* arg0)
{
    /* Song role: 0x34 is the note-table pointer, tested for presence. */
    if (g_akao_seq_channel0->flags != 0)
    {
        arg0->flags = (s32)((arg0->flags & 0xE6FFEFF7) | 8);
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/FnMZ3
 */
void func_8002CC7C(AkaoChannelState* arg0)
{
    arg0->spu_volume_scale = 0;
    arg0->flags = (s32)(arg0->flags & ~8);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/wuYt5
 */
void func_8002CC94(u8** arg0)
{
    AkaoChannelState* ch = g_akao_seq_channel0;

    /* Song role: 0x68 is ticks-per-beat and is_sfx_channel is
     * beats-per-measure - this is the FE 15 time-signature opcode. */
    ch->unk68 = *(*arg0)++;
    ch->is_sfx_channel = *(*arg0)++;
    ch->unk6A = 0;
    ch->unk66 = 0;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/FcDgE
 */
void func_8002CCCC(u8** arg0)
{
    AkaoChannelState* channel = g_akao_seq_channel0;

    channel->measure = *(*arg0)++;
    channel->measure |= (*(*arg0)++ << 8);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/XqM1L
 */
void func_8002CD08(AkaoChannelState* arg0, AkaoChannelState* arg1)
{
    akao_seq_op_set_adsr_decay(arg0, arg1);
    akao_seq_op_set_adsr_sustain_level(arg0, arg1);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/cAsju
 */
void func_8002CD44(AkaoChannelState* arg0, s32 arg1)
{
    u8* temp_v0;
    s32 temp_v1;
    s16 var_v0;

    temp_v0 = *(u8**)arg0;
    temp_v1 = *temp_v0;

    *(u8**)arg0 = temp_v0 + 1;

    if (temp_v1 != 0)
    {
        var_v0 = temp_v1 + 1;
    }
    else
    {
        var_v0 = 0x101;
    }

    *(s16*)((u8*)arg0 + 0xD4) = var_v0;
    akao_seq_op_enable_reverb(arg0, arg1);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/Basyw
 */
void func_8002CD88(AkaoChannelState* arg0)
{
    u8* temp_v0;
    s32 temp_v1;

    temp_v0 = *(u8**)arg0;
    temp_v1 = *temp_v0;

    *(u8**)arg0 = temp_v0 + 1;

    if (temp_v1 != 0)
    {
        *(s16*)((u8*)arg0 + 0xD4) = temp_v1 + 1;
    }
    else
    {
        *(s16*)((u8*)arg0 + 0xD4) = 0x101;
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/j1qFh
 */
void func_8002CDB8(void* arg0, s32 arg1)
{
    u8* temp_v0;
    s32 temp_v1;
    s16 var_v0;

    temp_v0 = *(u8**)arg0;
    temp_v1 = *temp_v0;

    *(u8**)arg0 = temp_v0 + 1;

    if (temp_v1 != 0)
    {
        var_v0 = temp_v1 + 1;
    }
    else
    {
        var_v0 = 0x101;
    }

    *(s16*)((u8*)arg0 + 0xD6) = var_v0;
    akao_seq_op_enable_pitch_modulation(arg0, arg1);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/3922q
 */
void func_8002CDFC(void* arg0)
{
    u8* temp_v0;
    s32 temp_v1;

    temp_v0 = *(u8**)arg0;
    temp_v1 = *temp_v0;

    *(u8**)arg0 = temp_v0 + 1;

    if (temp_v1 != 0)
    {
        *(s16*)((u8*)arg0 + 0xD6) = temp_v1 + 1;
    }
    else
    {
        *(s16*)((u8*)arg0 + 0xD6) = 0x101;
    }
}

/**
 * decomp.me (100%) https://decomp.me/scratch/LEJvC
 */
void func_8002CE2C(AkaoChannelState* arg0, s32 arg1)
{
    arg0->flags = (u8*)((u32)arg0->flags & ~0x37);

    akao_seq_op_disable_reverb((AkaoChannelState*)arg0, arg1);
    akao_seq_op_disable_pitch_modulation((AkaoChannelState*)arg0, arg1);
    akao_seq_op_disable_noise(arg0, arg1);

    arg0->note_flags = arg0->note_flags & 0xFFFA;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/8l07k
 */
void func_8002CE94(AkaoChannelState* arg0)
{
    arg0->flags = (u8*)((u32)arg0->flags | 0x10);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/YSIzI
 */
void func_8002CEA8(AkaoChannelState* arg0)
{
    arg0->flags = (u8*)((u32)arg0->flags & ~0x10);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/iCXHA
 */
void func_8002CEBC(AkaoChannelState* arg0)
{
    arg0->flags = (u8*)((u32)arg0->flags | 0x20);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/p5dcS
 */
void func_8002CED0(AkaoChannelState* arg0)
{
    arg0->flags = (u8*)((u32)arg0->flags & ~0x20);
}

typedef struct
{
    u32 unk0;
    u32 unk4;
    u32 unk8;
    s32 unkC;
} Struct_D_8004D3A0;

extern Struct_D_8004D3A0 D_8004D3A0;
void func_80026C14(Struct_D_8004D3A0* arg0, void* arg1, void* arg2, s32 arg3);

/**
 * decomp.me (100%) https://decomp.me/scratch/nwIop
 */
void func_8002CEE4(AkaoChannelState* arg0)
{
    u8* temp_a0;
    s32 temp_v0;
    void* var_a1;
    void* var_a2;

    temp_a0 = *(u8**)arg0;

    temp_v0 = (temp_a0[1] << 8) | temp_a0[0];

    if (temp_v0 != 0)
    {
        var_a1 = temp_a0 + temp_v0 + 2;
    }
    else
    {
        var_a1 = 0;
    }

    temp_a0 += 2;

    temp_v0 = (temp_a0[1] << 8) | temp_a0[0];
    if (temp_v0 != 0)
    {
        var_a2 = temp_a0 + temp_v0 + 2;
    }
    else
    {
        var_a2 = 0;
    }

    D_8004D3A0.unk0 = 0;
    D_8004D3A0.unk4 = 0;
    D_8004D3A0.unk8 = *(u16*)((u8*)arg0 + 0x90) >> 8;
    D_8004D3A0.unkC = (s32)arg0->unk48 >> 23;

    func_80026C14(&D_8004D3A0, var_a1, var_a2, 0);

    *(u8**)arg0 = *(u8**)arg0 + 4;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/B5HO1
 */
void func_8002CF9C(AkaoChannelState* arg0, s32 arg1)
{
    s32 temp_byte;
    s32 temp_a2;

    temp_byte = *(u8*)arg0->seq_cursor;
    arg0->seq_cursor++;

    arg0->pan_bias_fade_ticks = 0;
    arg0->flags |= 0x800;

    temp_a2 = temp_byte << 8;
    arg0->pan_bias = temp_a2;

    akao_seq_op_enable_noise(arg0, arg1, temp_a2);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/fM3EA
 */
void func_8002CFE0(AkaoChannelState* arg0, s32 arg1)
{
    u8* temp_a1;
    u16 temp_a0;
    u32 temp_v1;
    u8* temp_v0;

    temp_v0 = arg0->seq_cursor;
    temp_v1 = *temp_v0;
    arg0->seq_cursor = temp_v0 + 1;

    arg0->pan_bias_fade_ticks = temp_v1;
    if (temp_v1 == 0)
    {
        arg0->pan_bias_fade_ticks = 0x100;
    }

    temp_a0 = arg0->pan_bias & 0xFF00;
    temp_a1 = arg0->seq_cursor;

    arg0->pan_bias_step = ((s16)(*temp_a1 << 8) - temp_a0) / arg0->pan_bias_fade_ticks;
    arg0->pan_bias = temp_a0;
    arg0->seq_cursor = temp_a1 + 1;
    arg0->flags |= 0x800;

    akao_seq_op_enable_noise(arg0, arg1, arg0);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/ws5Yw
 */
void func_8002D094(AkaoChannelState* arg0)
{
    arg0->flags = (s32)(arg0->flags | 0x100000);
}

/**
 * @brief Extended opcode FE 1C: consume one operand byte and do nothing else.
 *
 * The operand-length table @c g_akao_opcode_len_table_ext gives this opcode a
 * length of 2 (sub-opcode plus one operand), and the handler only advances the
 * bytecode cursor past that operand, so the command is inert in this driver
 * build. Its meaning in the authoring tool is unknown.
 *
 * @param arg0 Channel whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%) https://decomp.me/scratch/ySVnh
 */
void akao_seq_op_skip_operand_byte(AkaoChannelState* arg0)
{
    arg0->seq_cursor += 1;
}

/**
 * @brief Extended opcode FE 1D: let this channel allocate SPU voices from
 *        voice 0, ignoring the reserved voice base.
 *
 * Sets the channel's bit in the song-state mask at offset 0x08. When
 * When akao_process_sequence_voice_updates keys a note on, it passes
 * @c (song->w04.song.voice_alloc_low_mask & channel_mask) to the voice
 * allocator akao_find_free_voice. A set bit makes the free-voice search start at
 * voice 0 instead of at @c song->unk38, the reserved base installed by extended
 * opcode FE 10 and cleared by FE 11. func_8002D0DC (FE 1E) clears the same bit,
 * and akao_release_channels clears it for every released channel.
 *
 * @param channel Channel state; unused, but required to match because the
 *                handler is called through the opcode-table signature.
 * @param channel_mask Bit of the channel being stepped, as passed by
 *                     akao_seq_step_opcode.
 * @see decomp.me (100%) https://decomp.me/scratch/q71gK
 */
void akao_seq_op_ignore_voice_reserve(AkaoChannelState* channel, s32 channel_mask)
{
    g_akao_seq_channel0->w04.song.voice_alloc_low_mask = (s32)(g_akao_seq_channel0->w04.song.voice_alloc_low_mask | channel_mask);
}
