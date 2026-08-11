#include "akao.h"
#include "akao_driver.h"
#include "decomp4.h"

extern s16 D_8003D37C[];

typedef struct
{
    s16 value;
    s16 unk2;
    s16 relative_offset;
} AkaoLfoSample;

/**
 * @brief Advance the per-channel pitch/volume/pan LFOs and recompute the SPU
 *        volume and pitch registers for one AKAO sequencer channel.
 * @param channel Channel state to update.
 * @note Match: 99.93%. Residual is 4 register-allocation rows, no structural
 *       diff (280/280 insns): target folds the mastervol address to
 *       `lui v1; lw v1,%lo(g_akao_mastervol_acc)(v1)` (one register, the %hi
 *       tied to the load destination) where we emit `lui v0; lw v1,...(v0)`.
 *       This is [ALLOC-02] in tools/lom-dev-mcp/idioms.md. The tie is blocked
 *       because the 0xFF0000 mask needs its own constant register, which
 *       conflicts with the loaded value; contrast the g_akao_seq_channel0 load
 *       above, whose `& 0x7F` is an andi immediate and which folds correctly.
 *       Retired (measured inert or worse): giving the loaded value its own
 *       carrier, block-local vs function-level scope, splitting the load from
 *       the mask, commuting the &, 0x00FF0000 spelling, unsigned cast,
 *       reordering the site-2 statements, and do/while(0) block boundaries at
 *       every position around the read.
 */
void func_80024B00(AkaoChannelState* channel)
{
    s32 flags;
    s32 lfo_pitch;
    s32 value;
    s32 pan;
    s32 pitch_value;
    s32 master_scale;
    s32 pitch_mode;

    flags = channel->flags;
    lfo_pitch = (((s16*)&channel->unk48)[1] * (channel->volume >> 8)) >> 7;

    if (flags & 1)
    {
        if (channel->pitch_lfo_delay_ticks == 0)
        {
            channel->pitch_lfo_restart--;
            if (channel->pitch_lfo_restart == 0)
            {
                channel->pitch_lfo_restart = *((u16*)((u8*)channel + 0xA6));
                {
                    AkaoLfoSample* cursor;
                    s16* waveform;

                    cursor = (AkaoLfoSample*)channel->unk1C;
                    if ((cursor->value == 0) && (cursor->unk2 == 0))
                    {
                        channel->unk1C = (s32)(((u8*)cursor) + (cursor->relative_offset * 2));
                    }

                    waveform = (s16*)channel->unk1C;
                    value = *waveform++;
                    master_scale = ((s32)channel->pitch_lfo_depth_scaled * value) >> 16;
                    channel->unk1C = (s32)waveform;
                }
                if (master_scale != channel->pitch_lfo_value)
                {
                    channel->pitch_lfo_value = master_scale;
                    channel->update_flags |= 0x10;
                    if (master_scale >= 0)
                    {
                        channel->pitch_lfo_value = master_scale << 1;
                    }
                }
            }
        }
    }

    if (flags & 2)
    {
        if (channel->volume_lfo_delay_ticks == 0)
        {
            channel->volume_lfo_restart--;
            if (channel->volume_lfo_restart == 0)
            {
                channel->volume_lfo_restart = *((u16*)((u8*)channel + 0xBA));
                {
                    AkaoLfoSample* cursor;
                    s16* waveform;

                    cursor = (AkaoLfoSample*)channel->tempo;
                    if ((cursor->value == 0) && (cursor->unk2 == 0))
                    {
                        channel->tempo = (u32)(((u8*)cursor) + (cursor->relative_offset * 2));
                    }

                    master_scale = (lfo_pitch * (channel->volume_lfo_depth >> 8) << 9) >> 16;
                    waveform = (s16*)channel->tempo;
                    value = *waveform++;
                    master_scale = (master_scale * value) >> 15;
                    channel->tempo = (u32)waveform;
                }
                if (master_scale != channel->volume_lfo_value)
                {
                    channel->volume_lfo_value = master_scale;
                    channel->update_flags |= 3;
                }
            }
        }
    }

    if (flags & 4)
    {
        channel->pan_lfo_restart--;
        if (channel->pan_lfo_restart == 0)
        {
            channel->pan_lfo_restart = channel->pan_lfo_period;
            {
                AkaoLfoSample* cursor;
                s16* waveform;

                cursor = (AkaoLfoSample*)channel->tempo_step;
                if ((cursor->value == 0) && (cursor->unk2 == 0))
                {
                    channel->tempo_step = (s32)(((u8*)cursor) + (cursor->relative_offset * 2));
                }

                waveform = (s16*)channel->tempo_step;
                value = *waveform++;
                master_scale = ((channel->pan_lfo_depth >> 8) * value) >> 15;
                channel->tempo_step = (s32)waveform;
            }
            if (master_scale != channel->pan_lfo_value)
            {
                channel->pan_lfo_value = master_scale;
                channel->update_flags |= 3;
            }
        }
    }

    if (flags & 0x20)
    {
        lfo_pitch = ((s16)(*((u16*)((u8*)channel - 0xC)) << 1) * (channel->volume >> 8)) >> 7;
        channel->update_flags |= 3;
    }

    if (channel->update_flags & 3)
    {
        lfo_pitch += channel->volume_lfo_value;
        lfo_pitch = (lfo_pitch * (*((u16*)((u8*)g_akao_seq_channel0 + 0x52)) & 0x7F)) >> 7;
        pan = ((channel->pan >> 8) + channel->pan_lfo_value) & 0xFF;

        if (D_8004F754 == 2)
        {
            channel->spu_volume_right = (lfo_pitch * D_8003D47C) >> 15;
            channel->spu_volume_left = channel->spu_volume_right;
        }
        else
        {
            channel->spu_volume_left = (lfo_pitch * D_8003D37C[pan]) >> 15;
            channel->spu_volume_right = (lfo_pitch * D_8003D37C[pan ^ 0xFF]) >> 15;
        }
        /* Empty do/while(0): plants a NOTE_INSN_LOOP_BEG immediately before
         * the branch target below, so gcc 2.8 mostly_true_jump() predicts the
         * `update_flags & 3` branch taken and fill_eager_delay_slots() clones
         * the `flags & 0x10` andi into its delay slot instead of hoisting the
         * fall-through lui. Required to match; removing it costs 2 exact rows
         * and one instruction. */
        do { } while (0);
    }

    pitch_mode = flags & 0x10;
    if (pitch_mode)
    {
        master_scale = *((s16*)((u8*)channel + 0x32));
        pitch_value = *((u16*)((u8*)channel - 0xC)) + channel->pitch_lfo_value + master_scale;
        master_scale = g_akao_mastervol_acc & 0xFF0000;
        if (master_scale != 0)
        {
            master_scale >>= 16;
            if (master_scale < 0x80)
            {
                pitch_value += (pitch_value * master_scale) >> 7;
            }
            else
            {
                pitch_value = (pitch_value * master_scale) >> 8;
            }
        }
        channel->spu_pitch = (*((u16*)((u8*)channel + 0x54)) + pitch_value) & 0x3FFF;
        channel->update_flags |= 0x10;
        return;
    }

    if (channel->update_flags & 0x10)
    {
        pitch_value = channel->pitch + channel->pitch_lfo_value + *((s16*)((u8*)channel + 0x32));
        master_scale = g_akao_mastervol_acc & 0xFF0000;
        if (master_scale != 0)
        {
            master_scale >>= 16;
            if (master_scale < 0x80)
            {
                pitch_value += (pitch_value * master_scale) >> 7;
            }
            else
            {
                pitch_value = (pitch_value * master_scale) >> 8;
            }
        }
        channel->spu_pitch = (*((u16*)((u8*)channel + 0x54)) + pitch_value) & 0x3FFF;
    }
}