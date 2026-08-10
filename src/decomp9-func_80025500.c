typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef struct VoiceAllocEntry
{
  s32 unk0;
  s16 unk4;
  s16 unk6;
} VoiceAllocEntry;
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
  u32 unk0;
  u32 unk4;
  u32 unk8;
} AkaoDriverFlags;
extern VoiceAllocEntry D_8004C1A0[];
extern AkaoChannelState *D_8004F7C0[];
extern s32 D_8003EC6C;
extern AkaoChannelState *g_akao_seq_channel0;
extern AkaoDriverFlags g_akao_driver_flags;
extern void func_80024B00(AkaoChannelState *, s32);
extern s32 func_80025498(s32);
extern s32 func_800253E8(s32);
extern void spu_write_voice_params(u32, void *, u16);
extern void spu_apply_voice_updates(u32, void *, s32);

/**
 * decomp.me (100%) https://decomp.me/scratch/wFlR3
 */
void func_80025500(AkaoChannelState *channel, s32 channel_mask, s32 static_voice_mask, u32 *voice_mask)
{
  s32 bit;
  s32 channel_index;
  s32 voice;
  s32 key_on_mask;
  s32 one;
  unsigned char new_var;
  bit = 1;
  channel_index = 0;
  one = bit;
  key_on_mask = channel_mask & g_akao_seq_channel0->w04.song.key_on_mask;
  do
  {
    new_var = 0x18U;
    if (channel_mask & bit)
    {
      func_80024B00(channel, bit);
      if (channel->update_flags != 0)
      {
        if (D_8003EC6C & bit)
        {
          channel->spu_volume_right = 0;
          channel->spu_volume_left = 0;
        }
        if (key_on_mask & bit)
        {
          if (static_voice_mask & bit)
          {
            *voice_mask |= one << channel_index;
            channel->voice = channel_index;
          }
          else
          {
            s32 use_low;
            use_low = (g_akao_seq_channel0->w04.song.voice_alloc_low_mask & bit) != 0;
            voice = func_80025498(use_low);
            if (voice == 0x18)
            {
              g_akao_seq_channel0->seq_cursor = (u8 *) (((u32) g_akao_seq_channel0->seq_cursor) | 2);
              voice = func_800253E8(use_low);
              if (voice == 0x18)
              {
                channel->voice = voice;
                g_akao_seq_channel0->seq_cursor = (u8 *) (((u32) g_akao_seq_channel0->seq_cursor) | 1);
              }
              else
              {
                *voice_mask |= one << voice;
                channel->voice = voice;
                D_8004C1A0[voice].unk4 = 0x7FFF;
              }
            }
            else
            {
              *voice_mask |= one << voice;
              channel->voice = voice;
              D_8004C1A0[voice].unk4 = 0x7FFF;
            }
          }
          if (channel->voice < new_var)
          {
            spu_write_voice_params(channel->voice, (void *) (&channel->voice), channel->spu_volume_scale);
            D_8004F7C0[channel->voice] = g_akao_seq_channel0;
            g_akao_driver_flags.unk8 |= 0x100;
          }
        }
        else
          if (channel->voice < new_var)
        {
          spu_apply_voice_updates(channel->voice, (void *) (&channel->voice), channel->flags);
        }
      }
      channel_mask &= ~bit;
    }
    bit <<= 1;
    channel++;
    channel_index++;
  }
  while (channel_mask != 0);
}