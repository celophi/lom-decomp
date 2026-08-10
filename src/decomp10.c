typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;

typedef struct
{
  u32 unk0;
  u32 unk4;
  u32 unk8;
} AkaoDriverFlags;

typedef struct AkaoChannelState
{
  u8 *seq_cursor;
  union
  {
    u8 *loop_cursor[4];
    struct
    {
      u32 active_mask;
      u32 voice_alloc_low_mask;
      u32 static_voice_mask;
      u32 key_on_mask;
    } song;
  } w04;
  u32 note_on_mask;
  u32 key_off_mask;
  s32 unk1C;
  u32 tempo;
  s32 tempo_step;
  u32 tempo_acc;
  s32 pitch;
  s32 unk30;
  s32 flags;
  s32 voice_alloc_base;
  u32 reverb_mask;
  u32 noise_mask;
  u32 pitch_mod_mask;
  s32 unk48;
  s32 unk4C;
  s32 pitch_slide_step;
  s32 detune_pitch_delta;
  u16 unk58;
  s16 master_vol_fade_ticks;
  u16 tempo_fade_ticks;
  u16 unk5E;
  u16 unk60;
  u16 noise_freq;
  u16 is_sfx_channel;
  u16 unk66;
  u16 unk68;
  u16 unk6A;
  u16 measure;
  u16 pan_bias;
  u16 pan_bias_fade_ticks;
  u16 opcode_count;
  u16 loop_count[4];
  u16 loop_opcode_count[4];
  u16 volume;
  u16 volume_fade_ticks;
  u16 unk88;
  u16 expression_fade_ticks;
  u16 note_expression_ticks;
  u16 unk8E;
  u16 pan;
  u16 pan_fade_ticks;
  u16 pitch_slide_ticks;
  u16 octave;
  u16 pitch_slide_duration;
  u16 prev_key;
  u16 portamento_speed;
  u16 note_flags;
  u16 unkA0;
  u16 pitch_lfo_delay;
  u16 pitch_lfo_delay_ticks;
  s16 pitch_lfo_period;
  u16 pitch_lfo_restart;
  u16 pitch_lfo_waveform;
  u16 pitch_lfo_depth_scaled;
  u16 pitch_lfo_depth;
  u16 pitch_lfo_depth_fade_ticks;
  u16 pitch_lfo_depth_step;
  u16 unkB4;
  u16 volume_lfo_delay;
  u16 volume_lfo_delay_ticks;
  s16 volume_lfo_period;
  u16 volume_lfo_restart;
  u16 volume_lfo_waveform;
  u16 volume_lfo_depth;
  u16 volume_lfo_depth_fade_ticks;
  u16 volume_lfo_depth_step;
  u16 unkC6;
  u16 pan_lfo_period;
  u16 pan_lfo_restart;
  u16 pan_lfo_waveform;
  u16 pan_lfo_depth;
  u16 pan_lfo_depth_fade_ticks;
  u16 pan_lfo_depth_step;
  u16 reverb_toggle_ticks;
  u16 pitch_mod_toggle_ticks;
  u16 loop_depth;
  u16 pitch_scale;
  s16 note_duration;
  u16 note_duration_adjust;
  s16 volume_step;
  s16 pan_bias_step;
  u16 volume_scale;
  u16 unkE6;
  s16 pan_step;
  u16 transpose;
  s16 detune;
  u16 note_key;
  u16 pitch_slide_delta;
  s16 prev_transpose;
  s16 pitch_lfo_value;
  s16 volume_lfo_value;
  s16 pan_lfo_value;
  u16 unkFA;
  u32 voice;
  s32 update_flags;
  s32 spu_sample_addr;
  s32 spu_loop_addr;
  u16 spu_pitch;
  u16 spu_adsr_low;
  u16 spu_adsr_high;
  u16 spu_volume_scale;
  s16 spu_volume_left;
  s16 spu_volume_right;
} AkaoChannelState;

typedef struct
{
  s16 value;
  s16 unk2;
  s16 relative_offset;
} AkaoLfoSample;

typedef struct
{
  u32 unk0;
  s32 unk4;
  u32 unk8;
  u32 unkC;
  u32 unk10;
  u8 _pad14[2];
  u16 unk16;
  u32 unk18;
  u32 reverb_mask;
  u32 noise_mask;
  u32 pitch_mod_mask;
  u16 unk28;
} SfxControl;

extern AkaoChannelState *g_akao_seq_channel0;
extern SfxControl g_akao_sfx_control;
extern AkaoDriverFlags g_akao_driver_flags;


/**
 * @brief Advance the per-channel volume, pan, pitch-bend, and LFO/envelope
 *        effects for one AKAO tick.
 *
 * @param channel Channel whose effect accumulators are advanced.
 * @param channel_bit Bit for this channel in the sequence/SFX control bitmaps.
 * @param is_sfx Nonzero for an SFX channel; zero for a sequence channel.
 * @see decomp.me (100%) https://decomp.me/scratch/0WomW
 */
void akao_tick_channel_effects(AkaoChannelState* channel, s32 channel_bit, s32 is_sfx)
{
    AkaoLfoSample* var_a0;
    s32 temp_v1_2;
    s32 temp_v1_5;
    s32 temp_a3;
    u16 temp_v0_10;
    u16 temp_v0_11;
    u16 temp_v0_12;
    u16 temp_v0_13;
    u16 temp_v0_2;
    u16 temp_v0_3;
    u16 temp_v0_4;
    u16 temp_v0_5;
    u16 temp_v0_6;
    u16 temp_v0_7;
    u16 temp_v0_8;
    u16 temp_v0_9;
    s32 temp_v1_3;
    u16 temp_v1_4;
    u32 temp_a0;
    u32 var_lo;

    if (is_sfx == 0)
    {
        if (channel->volume_fade_ticks != 0)
        {
            channel->volume_fade_ticks = (u16)(channel->volume_fade_ticks - 1);
            temp_a3 = channel->volume + channel->volume_step;
            if ((temp_a3 & 0x7F00) != (channel->volume & 0x7F00))
            {
                channel->update_flags = (s32)(channel->update_flags | 3);
            }
            channel->volume = temp_a3;
        }
    }

    temp_v0_2 = channel->expression_fade_ticks;
    if (temp_v0_2 != 0)
    {
        temp_v1_2 = channel->unk48;
        channel->expression_fade_ticks = (u16)(temp_v0_2 - 1);
        temp_a3 = temp_v1_2 + channel->unk4C;
        if ((temp_a3 & 0xFFE00000) != (temp_v1_2 & 0xFFE00000))
        {
            channel->update_flags = (s32)(channel->update_flags | 3);
        }
        channel->unk48 = temp_a3;
    }

    temp_v0_3 = channel->pan_fade_ticks;
    if (temp_v0_3 != 0)
    {
        temp_v1_3 = channel->pan;
        channel->pan_fade_ticks = (u16)(temp_v0_3 - 1);
        temp_a3 = temp_v1_3 + channel->pan_step;
        if ((temp_a3 & 0xFF00) != (temp_v1_3 & 0xFF00))
        {
            channel->update_flags = (s32)(channel->update_flags | 3);
        }
        channel->pan = temp_a3;
    }

    temp_v0_4 = channel->pitch_lfo_delay_ticks;
    if (temp_v0_4 != 0)
    {
        channel->pitch_lfo_delay_ticks = (u16)(temp_v0_4 - 1);
    }

    temp_v0_5 = channel->volume_lfo_delay_ticks;
    if (temp_v0_5 != 0)
    {
        channel->volume_lfo_delay_ticks = (u16)(temp_v0_5 - 1);
    }

    temp_v0_6 = channel->reverb_toggle_ticks;
    if (temp_v0_6 != 0)
    {
        temp_v0_7 = temp_v0_6 - 1;
        channel->reverb_toggle_ticks = temp_v0_7;
        if (!(temp_v0_7 & 0xFFFF))
        {
            if (is_sfx == 0)
            {
                g_akao_seq_channel0->reverb_mask ^= channel_bit;
            }
            else
            {
                g_akao_sfx_control.reverb_mask ^= channel_bit;
            }
            g_akao_driver_flags.unk8 |= 0x110;
        }
    }

    temp_v0_8 = channel->pitch_mod_toggle_ticks;
    if (temp_v0_8 != 0)
    {
        temp_v0_9 = temp_v0_8 - 1;
        channel->pitch_mod_toggle_ticks = temp_v0_9;
        if (!(temp_v0_9 & 0xFFFF))
        {
            if (is_sfx == 0)
            {
                g_akao_seq_channel0->pitch_mod_mask ^= channel_bit;
            }
            else
            {
                g_akao_sfx_control.pitch_mod_mask ^= channel_bit;
            }
            g_akao_driver_flags.unk8 |= 0x100;
        }
    }

    temp_v1_4 = channel->pitch_lfo_depth_fade_ticks;
    if (temp_v1_4 != 0)
    {
        channel->pitch_lfo_depth_fade_ticks = (u16)(temp_v1_4 - 1);
        temp_v0_10 = channel->pitch_lfo_depth + channel->pitch_lfo_depth_step;
        channel->pitch_lfo_depth = temp_v0_10;
        temp_a0 = ((u32)(temp_v0_10 & 0x7F00)) >> 8;

        if (temp_v0_10 & 0x8000)
        {
            var_lo = (temp_a0 * channel->pitch) >> 7;
        }
        else
        {
            var_lo = (temp_a0 * (((u32)(channel->pitch * 0xF)) >> 8)) >> 7;
        }

        channel->pitch_lfo_depth_scaled = (u16)var_lo;

        if ((channel->pitch_lfo_delay_ticks == 0) && (channel->pitch_lfo_restart != 1))
        {
            var_a0 = (AkaoLfoSample*)channel->unk1C;
            if ((var_a0->value == 0) && (var_a0->unk2 == 0))
            {
                var_a0 = (AkaoLfoSample*)(((u8*)var_a0) + (var_a0->relative_offset * 2));
            }

            temp_a3 = ((s32)(channel->pitch_lfo_depth_scaled * var_a0->value)) >> 0x10;
            if (temp_a3 != channel->pitch_lfo_value)
            {
                channel->pitch_lfo_value = (s16)temp_a3;
                channel->update_flags = (s32)(channel->update_flags | 0x10);
                if (temp_a3 >= 0)
                {
                    channel->pitch_lfo_value = (s16)(temp_a3 * 2);
                }
            }
        }
    }

    temp_v0_11 = channel->volume_lfo_depth_fade_ticks;
    if (temp_v0_11 != 0)
    {
        channel->volume_lfo_depth_fade_ticks = (u16)(temp_v0_11 - 1);
        channel->volume_lfo_depth = (u16)(channel->volume_lfo_depth + channel->volume_lfo_depth_step);

        if ((channel->volume_lfo_delay_ticks == 0) && (channel->volume_lfo_restart != 1))
        {
            var_a0 = (AkaoLfoSample*)channel->tempo;
            if ((var_a0->value == 0) && (var_a0->unk2 == 0))
            {
                var_a0 = (AkaoLfoSample*)(((u8*)var_a0) + (var_a0->relative_offset * 2));
            }

            temp_a3 = (s32)((((s32)((((s32)(((s16*)(&channel->unk48))[1] * (channel->volume >> 8))) >> 7) * (channel->volume_lfo_depth >> 8))) << 9) >> 16);
            temp_a3 = ((s32)(temp_a3 * var_a0->value)) >> 0xF;

            if (temp_a3 != channel->volume_lfo_value)
            {
                channel->volume_lfo_value = (s16)temp_a3;
                channel->update_flags = (s32)(channel->update_flags | 3);
            }
        }
    }

    temp_v0_12 = channel->pan_lfo_depth_fade_ticks;
    if (temp_v0_12 != 0)
    {
        channel->pan_lfo_depth_fade_ticks = (u16)(temp_v0_12 - 1);
        channel->pan_lfo_depth = (u16)(channel->pan_lfo_depth + channel->pan_lfo_depth_step);

        if (channel->pan_lfo_restart != 1)
        {
            var_a0 = (AkaoLfoSample*)channel->tempo_step;
            if ((var_a0->value == 0) && (var_a0->unk2 == 0))
            {
                var_a0 = (AkaoLfoSample*)(((u8*)var_a0) + (var_a0->relative_offset * 2));
            }

            temp_a3 = ((s32)((((u16)channel->pan_lfo_depth) >> 8) * var_a0->value)) >> 0xF;

            if (temp_a3 != channel->pan_lfo_value)
            {
                channel->pan_lfo_value = (s16)temp_a3;
                channel->update_flags = (s32)(channel->update_flags | 3);
            }
        }
    }

    temp_v0_13 = channel->pitch_slide_ticks;
    if (temp_v0_13 != 0)
    {
        temp_v1_5 = channel->unk30;
        channel->pitch_slide_ticks = (u16)(temp_v0_13 - 1);
        temp_a3 = temp_v1_5 + channel->pitch_slide_step;

        if ((temp_a3 & 0xFFFF0000) != (temp_v1_5 & 0xFFFF0000))
        {
            channel->update_flags = (s32)(channel->update_flags | 0x10);
        }
        channel->unk30 = temp_a3;
    }
}