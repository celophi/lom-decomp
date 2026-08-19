#include "akao_voice.h"
#include "psyq/libspu.h"

/** @brief LFO waveform sample or zero-pair relative-jump marker. */
typedef struct
{
    s16 sample;
    s16 marker;
    s16 relative_offset;
} AkaoLfoSample;

/** @brief Voice-allocation bookkeeping entry used by the AKAO mixer. */
typedef struct AkaoVoiceAllocation
{
    s32 unknown_0x00;
    s16 envelope_level;
    s16 unknown_0x06;
} AkaoVoiceAllocation;

/** @brief Sequence voice masks used while refreshing SPU allocations. */
typedef struct AkaoSequenceVoiceMasks
{
    s32 unknown_0x00;
    u32 active_channel_mask;
    u32 unknown_0x08;
    u32 static_voice_mask;
} AkaoSequenceVoiceMasks;

extern u8* D_8003D0C0;
extern s16 D_8003D068[];
extern s16 D_8003D37C[];
extern s16 D_8003D47C;
extern s32 D_8004F754[];
extern AkaoVoiceAllocation D_8004C1A0[];
extern AkaoChannelState* D_8004F7C0[];
extern s32 D_8004F76C[];
extern s32 D_8004D404[];
extern s32 D_8004F834[];
extern s32 D_8003EC34[];
extern u8 D_8003D248[];
extern AkaoChannelState D_8004C038[];
extern s32 D_8004C2FC;
extern u8 D_8004D450[];
extern s16 D_8004C32E;
extern s32 D_8004D39C;
extern s32 g_akao_cdvol_step;
extern s32 g_akao_masterpan_step;
extern s32 g_akao_mastervol_step;
extern s32 D_8004D410;
extern void* g_akaoCmdParams[];
extern s32 D_8004D340[6];
extern void (*D_8003DDE0[])(s32*);
extern void (*D_8003E120)(s32*);
extern void (*D_8003E124)(s32*);
extern void (*D_8003E128)(s32*);

extern void akao_clear_voice_assignment(u8* primary_channels, s32 voice_index);
void akao_build_effect_voice_mask(s32* effect_voices, s32 secondary_effect_mask, s32 primary_effect_mask, s32 sfx_effect_voices);
void akao_set_reverb_volume(s32 reverb_left, s32 reverb_right);
s32 akao_set_noise_frequency(s32 noise_freq);
void akao_read_voice_envelope(s32 voice_index, s16* envelope_out);
void akao_channel_set_articulation(AkaoChannelState* channel, s32 articulation_index);
u32 akao_collect_voice_mask(AkaoChannelState* channels, s32 channel_mask);
void akao_sfx_release_channels(void* channel, u32 release_mask);

/**
 * @brief Write the SPU key-on voice bitmap.
 *
 * The low halfword selects voices 0-15 and the high halfword selects voices
 * 16-23. Bits 24-31 are unused.
 *
 * @param voice_mask Voices to key on.
 * @see decomp.me (100%) https://decomp.me/scratch/lKkom
 */
void spu_set_key_on(u32 voice_mask)
{
    *(u16*)0x1F801D88 = voice_mask;
    *(u16*)0x1F801D8A = voice_mask >> 0x10;
}

/**
 * @brief Write the SPU key-off voice bitmap.
 *
 * The low halfword selects voices 0-15 and the high halfword selects voices
 * 16-23. Bits 24-31 are unused.
 *
 * @param voice_mask Voices to key off.
 * @see decomp.me (100%) https://decomp.me/scratch/957fv
 */
void spu_set_key_off(u32 voice_mask)
{
    *(u16*)0x1F801D8C = voice_mask;
    *(s16*)0x1F801D8E = (s16)(voice_mask >> 0x10);
}

/**
 * @brief Write the SPU per-voice reverb-enable bitmap.
 *
 * @param voice_mask Reverb-enabled voices 0-23.
 * @see decomp.me (100%) https://decomp.me/scratch/3KFgT
 */
void spu_set_reverb_enable(u32 voice_mask)
{
    *(u16*)0x1F801D98 = voice_mask;
    *(s16*)0x1F801D9A = (s16)(voice_mask >> 0x10);
}

/**
 * @brief Write the SPU per-voice noise-enable bitmap.
 *
 * @param voice_mask Noise-enabled voices 0-23.
 * @see decomp.me (100%) https://decomp.me/scratch/HFDSO
 */
void spu_set_noise_enable(u32 voice_mask)
{
    *(u16*)0x1F801D94 = voice_mask;
    *(s16*)0x1F801D96 = (s16)(voice_mask >> 0x10);
}

/**
 * @brief Write the SPU per-voice pitch-modulation-enable bitmap.
 *
 * @param voice_mask Pitch-modulated voices 0-23.
 * @see decomp.me (100%) https://decomp.me/scratch/ZyBKt
 */
void spu_set_pitch_modulation_enable(u32 voice_mask)
{
    *(u16*)0x1F801D90 = voice_mask;
    *(s16*)0x1F801D92 = (s16)(voice_mask >> 0x10);
}

/**
 * @brief Set the Volume Left and Volume Right for a single SPU voice.
 *
 * When @p scale is non-zero, both @p vol_l and @p vol_r are multiplied by
 * @p scale and the result is arithmetic-shifted right by 7 (fixed-point
 * scaling).  Final values are clamped to 15-bit signed range (& 0x7FFF).
 *
 * @param voice Voice index (0-23).
 * @param vol_l Volume Left raw value.
 * @param vol_r Volume Right raw value.
 * @param scale If non-zero, fixed-point scale factor applied before write.
 * @see decomp.me (100%) https://decomp.me/scratch/WzWyo
 */
void spu_set_voice_volume(s32 voice, u32 vol_l, u32 vol_r, s32 scale)
{
    s32 temp_v0;
    u32 scaled_l;
    u32 scaled_r;
    SpuVoiceVolume* ptr;

    scaled_l = vol_l;
    scaled_r = vol_r;

    if (scale != 0)
    {
        scaled_l = (scaled_l * scale);
        scaled_r = (scaled_r * scale);

        scaled_l = (u32)scaled_l >> 7;
        scaled_r = (u32)scaled_r >> 7;
    }

    ptr = (SpuVoiceVolume*)0x1F801C00;
    temp_v0 = voice * 0x4;
    ((SpuVoiceVolume*)(temp_v0 + ptr))->left = (s16)(scaled_l & 0x7FFF);
    ((SpuVoiceVolume*)(temp_v0 + ptr))->right = (s16)(scaled_r & 0x7FFF);
}

/**
 * @brief Set the PITCH register (sample rate) for a single SPU voice.
 *
 * @param voice Voice index (0-23).
 * @param pitch Raw pitch value written directly to the PITCH register.
 * @see decomp.me (100%) https://decomp.me/scratch/3fXi9
 */
void spu_set_voice_pitch(s32 voice, s32 pitch)
{
    s32 ptr = (s32)0x1F801C04;
    voice = voice << 4;
    *(s16*)(ptr + voice) = pitch;
}

/**
 * @brief Set the waveform start address (ADDR) for a single SPU voice.
 *
 * The address is right-shifted by 3 (SPU addresses are in 8-byte units).
 *
 * @param voice Voice index (0-23).
 * @param addr Waveform start address in SPU RAM (byte address; will be >> 3).
 * @see decomp.me (100%) https://decomp.me/scratch/4xQ5z
 */
void spu_set_voice_start_addr(s32 voice, u32 addr)
{
    s32 ptr = (s32)0x1F801C06;
    voice = voice << 4;
    *(s16*)(ptr + voice) = (s16)(addr >> 3);
}

/**
 * @brief Set the repeat address (RADDR) for a single SPU voice.
 *
 * The address is right-shifted by 3 (SPU addresses are in 8-byte units).
 *
 * @param voice Voice index (0-23).
 * @param addr Loop start address in SPU RAM (byte address; will be >> 3).
 * @see decomp.me (100%) https://decomp.me/scratch/UI7qr
 */
void spu_set_voice_repeat_addr(s32 voice, u32 addr)
{
    s32 ptr = (s32)0x1F801C0E;
    voice = voice << 4;
    *(s16*)(ptr + voice) = (s16)(addr >> 3);
}

/**
 * @brief Set the low ADSR register for a single SPU voice.
 *
 * Fields are sustain level (bits 0-3), decay shift (bits 4-7),
 * attack shift (bits 8-14), and attack mode (bit 15).
 *
 * @param voice Voice index (0-23).
 * @param adsr_low Raw 16-bit ADSR low value.
 * @see decomp.me (100%) https://decomp.me/scratch/ghHQZ
 */
void spu_set_voice_adsr_low(s32 voice, u16 adsr_low)
{
    s32 ptr = (s32)0x1F801C08;
    voice = voice << 4;
    *(s16*)(ptr + voice) = adsr_low;
}

/**
 * @brief Set the high ADSR register for a single SPU voice.
 *
 * ADSR2 fields: Sustain Rate direction/mode, Release Mode, Release Rate
 * (bits 0-5).
 *
 * @param voice Voice index (0-23).
 * @param adsr_high Raw 16-bit ADSR high value.
 * @see decomp.me (100%) https://decomp.me/scratch/aDnJj
 */
void spu_set_voice_adsr_high(s32 voice, u16 adsr_high)
{
    s32 ptr = (s32)0x1F801C0A;
    voice = voice << 4;
    *(s16*)(ptr + voice) = adsr_high;
}

/**
 * @brief Set the attack mode and attack-shift fields in the low ADSR
 *        register, preserving its low byte.
 *
 * The high byte of ADSR1 is built from:
 *   - @p attack_shift shifted left 8
 *   - @p mode_bits right-shifted 2 then placed at bit 15 (Attack Rate Mode).
 *
 * @param voice Voice index (0-23).
 * @param attack_shift Attack shift/rate field.
 * @param mode_bits Mode flags; bit 2 maps to ADSR1 bit 15 (Attack Mode).
 * @see decomp.me (100%) https://decomp.me/scratch/Ua4UK
 */
void spu_set_voice_attack(s32 voice, s32 attack_shift, u32 mode_bits)
{
    s32 temp_a0;
    s32 ptr = (s32)0x1F801C08;

    voice = voice << 4;

    *(s16*)(ptr + voice) = (*(u8*)(ptr + voice)) | (((mode_bits >> 2) << 0xF) | (attack_shift << 8));
}

/**
 * @brief Set only the decay-shift field of the low ADSR register.
 *
 * Preserves attack (bits 8-15) and sustain level (bits 0-3); sets
 * decay shift in bits 4-7 from @p decay_shift << 4.
 *
 * @param voice Voice index (0-23).
 * @param decay_shift Decay shift value (0-15).
 * @see decomp.me (100%) https://decomp.me/scratch/ymuym
 */
void spu_set_voice_decay_shift(s32 voice, s32 decay_shift)
{
    s32 temp_a0;
    s32 ptr = (s32)0x1F801C08;

    voice = voice << 4;
    decay_shift = decay_shift << 4;

    *(s16*)(ptr + voice) = ((*(s16*)(ptr + voice)) & 0xFF0F) | (decay_shift);
}

/**
 * @brief Set only the Sustain Level field of ADSR1 for a single SPU voice.
 *
 * Preserves bits 4-15 (Attack, Decay, Sustain Rate); sets Sustain Level
 * in bits 0-3 from @p sustain_level.
 *
 * @param voice Voice index (0-23).
 * @param sustain_level Sustain Level nybble (0-15).
 * @see decomp.me (100%) https://decomp.me/scratch/2UD84
 */
void spu_set_voice_sustain_level(s32 voice, s32 sustain_level)
{
    s32 temp_a0;
    s32 ptr = (s32)0x1F801C08;

    voice = voice << 4;

    *(s16*)(ptr + voice) = ((*(s16*)(ptr + voice)) & 0xFFF0) | (sustain_level);
}

/**
 * @brief Set the sustain fields of the high ADSR register, preserving the
 *        release fields (bits 0-5).
 *
 * @param voice Voice index (0-23).
 * @param sustain_bits Sustain shift/mode bits, placed at bit 6.
 * @param mode_bits Additional sustain mode flags; bit 1 maps to bit 14.
 * @see decomp.me (100%) https://decomp.me/scratch/ZWKKM
 */
void spu_set_voice_sustain_mode(s32 voice, s32 sustain_bits, u32 mode_bits)
{
    s32 temp_a0;
    s32 ptr = (s32)0x1F801C0A;

    voice = voice << 4;

    *(s16*)(ptr + voice) = ((*(s16*)(ptr + voice)) & 0x3F) | (((mode_bits >> 1) << 0xE) | (sustain_bits << 6));
}

/**
 * @brief Set the release shift and mode fields of the high ADSR register,
 *        preserving its sustain fields.
 *
 * @param voice Voice index (0-23).
 * @param release_shift Release shift value (placed in bits 0-4).
 * @param mode_bit Release mode flags; bit 2 maps to bit 5.
 * @see decomp.me (100%) https://decomp.me/scratch/cztam
 */
void spu_set_voice_release_mode(s32 voice, s32 release_shift, u32 mode_bit)
{
    s32 temp_a0;
    s32 ptr = (s32)0x1F801C0A;

    voice = voice << 4;

    *(s16*)(ptr + voice) = ((*(s16*)(ptr + voice)) & 0xFFC0) | (((mode_bit >> 2) << 0x5) | (release_shift));
}

/**
 * @brief Write the complete SPU register image for one AKAO voice.
 *
 * Writes VOLL, VOLR, PITCH, ADDR (start), ADSR1, ADSR2, and RADDR (loop)
 * in one shot. First clears @c params->update_flags. If @p scale is
 * non-zero, the volume fields are multiplied by @p scale then shifted
 * right by 7 (fixed-point scaling).
 *
 * @param voice Voice index (0-23).
 * @param params Pointer to AKAO's packed voice-register image.
 * @param scale If non-zero, fixed-point volume scale factor.
 * @see decomp.me (100%) https://decomp.me/scratch/MkQTS
 */
void spu_write_voice_params(s32 voice, SpuVoiceParams* params, s32 scale)
{
    s32 scaled_r;
    s32 scaled_l;
    s32 temp_a0;
    u8* temp_a0_2;
    u8* temp_a0_3;
    u8* temp_a0_4;
    u8* temp_a0_5;
    u8* temp_a0_6;
    s32 ptr;

    params->update_flags = 0;
    ptr = (s32)0x1F801C00;
    voice = voice << 4;
    voice = (voice + ptr);

    if (scale == 0)
    {
        scaled_l = params->volume_left;
        scaled_r = params->volume_right;
    }
    else
    {
        scaled_l = (params->volume_left * scale);
        scaled_l = scaled_l >> 7;
        scaled_r = (params->volume_right * scale);
        scaled_r = scaled_r >> 7;
    }

    *(s16*)voice = scaled_l & 0x7FFF;

    voice += 2;
    temp_a0_3 = voice;
    *(s16*)(voice) = (scaled_r & 0x7FFF);

    voice += 2;
    temp_a0_4 = temp_a0_3 + 2;
    *(s16*)(voice) = (u16)params->pitch;

    voice += 2;
    temp_a0_5 = temp_a0_4 + 2;
    *(s16*)(voice) = (s16)((u32)params->sample_start_addr >> 3);

    voice += 2;
    temp_a0_6 = temp_a0_5 + 2;
    *(s16*)(voice) = (u16)params->adsr_low;

    voice += 2;
    *(s16*)(voice) = (u16)params->adsr_high;

    voice += 4;
    *(s16*)(voice) = (s16)((u32)params->sample_repeat_addr >> 3);
}

/**
 * @brief Apply the pending register updates for one SPU voice.
 * @param voice Voice index (0-23).
 * @param params Pending SPU register image.
 * @param flags Caller state retained by the original three-argument ABI.
 * @see decomp.me (100%) https://decomp.me/scratch/ORS8e
 */
void spu_apply_voice_updates(s32 voice, SpuVoiceParams* params, s32 flags)
{
    s32 var_s0 = params->update_flags;

    if (var_s0 == 0)
    {
        return;
    }

    params->update_flags = 0;

    // Handle Pitch (0x10)
    if (var_s0 & 0x10)
    {
        var_s0 &= ~0x10;
        spu_set_voice_pitch(voice, params->pitch);
        if (var_s0 == 0)
            return;
    }

    // Handle Volume (0x03)
    if (var_s0 & 3)
    {
        var_s0 &= ~3;
        spu_set_voice_volume(voice, (u32)params->volume_left, (u32)params->volume_right, (s32)params->volume_scale);
        if (var_s0 == 0)
            return;
    }

    // Handle Start Address (0x80)
    if (var_s0 & 0x80)
    {
        var_s0 &= ~0x80;
        spu_set_voice_start_addr(voice, params->sample_start_addr);
        if (var_s0 == 0)
            return;
    }

    // Handle Loop Address (0x10000)
    if (var_s0 & 0x10000)
    {
        var_s0 &= 0xFFFEFFFF;
        spu_set_voice_repeat_addr(voice, params->sample_repeat_addr);
        if (var_s0 == 0)
            return;
    }

    // Handle ADSR2 (0x6600)
    if (var_s0 & 0x6600)
    {
        var_s0 &= ~0x6600;
        spu_set_voice_adsr_high(voice, (s16)params->adsr_high);
        if (var_s0 == 0)
            return;
    }

    // Handle ADSR1 (0x9900)
    if (var_s0 & 0x9900)
    {
        spu_set_voice_adsr_low(voice, (s16)params->adsr_low);
    }
}

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
            if ((var_a0->sample == 0) && (var_a0->marker == 0))
            {
                var_a0 = (AkaoLfoSample*)(((u8*)var_a0) + (var_a0->relative_offset * 2));
            }

            temp_a3 = ((s32)(channel->pitch_lfo_depth_scaled * var_a0->sample)) >> 0x10;
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
            if ((var_a0->sample == 0) && (var_a0->marker == 0))
            {
                var_a0 = (AkaoLfoSample*)(((u8*)var_a0) + (var_a0->relative_offset * 2));
            }

            temp_a3 = (s32)((((s32)((((s32)(((s16*)(&channel->unk48))[1] * (channel->volume >> 8))) >> 7) * (channel->volume_lfo_depth >> 8))) << 9) >> 16);
            temp_a3 = ((s32)(temp_a3 * var_a0->sample)) >> 0xF;

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
            if ((var_a0->sample == 0) && (var_a0->marker == 0))
            {
                var_a0 = (AkaoLfoSample*)(((u8*)var_a0) + (var_a0->relative_offset * 2));
            }

            temp_a3 = ((s32)((((u16)channel->pan_lfo_depth) >> 8) * var_a0->sample)) >> 0xF;

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

/**
 * @brief Sample sequence-channel LFOs and recompute its pending SPU voice image.
 * @param channel Channel state to update.
 * @param channel_mask Channel bit retained by the original calling convention.
 */
void akao_update_sequence_channel_voice(AkaoChannelState* channel, s32 channel_mask)
{
    s32 effect_flags;
    s32 scaled_volume;
    s32 waveform_sample;
    s32 pan_index;
    s32 pitch_offset;
    s32 update_mask;
    s32 effect_value;

    effect_flags = channel->flags;
    scaled_volume = (((s16*)&channel->unk48)[1] * (channel->volume >> 8)) >> 7;

    if (effect_flags & 1)
    {
        if (channel->pitch_lfo_delay_ticks == 0)
        {
            channel->pitch_lfo_restart--;
            if (channel->pitch_lfo_restart == 0)
            {
                channel->pitch_lfo_restart = *((u16*)((u8*)channel + 0xA6));
                {
                    AkaoLfoSample* lfo_cursor;
                    s16* lfo_samples;

                    lfo_cursor = (AkaoLfoSample*)channel->unk1C;
                    if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
                    {
                        channel->unk1C = (s32)(((u8*)lfo_cursor) + (lfo_cursor->relative_offset * 2));
                    }

                    lfo_samples = (s16*)channel->unk1C;
                    waveform_sample = *lfo_samples++;
                    effect_value = ((s32)channel->pitch_lfo_depth_scaled * waveform_sample) >> 16;
                    channel->unk1C = (s32)lfo_samples;
                }
                if (effect_value != channel->pitch_lfo_value)
                {
                    channel->pitch_lfo_value = effect_value;
                    channel->update_flags |= 0x10;
                    if (effect_value >= 0)
                    {
                        channel->pitch_lfo_value = effect_value << 1;
                    }
                }
            }
        }
    }

    if (effect_flags & 2)
    {
        if (channel->volume_lfo_delay_ticks == 0)
        {
            channel->volume_lfo_restart--;
            if (channel->volume_lfo_restart == 0)
            {
                channel->volume_lfo_restart = *((u16*)((u8*)channel + 0xBA));
                {
                    AkaoLfoSample* lfo_cursor;
                    s16* lfo_samples;

                    lfo_cursor = (AkaoLfoSample*)channel->tempo;
                    if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
                    {
                        channel->tempo = (u32)(((u8*)lfo_cursor) + (lfo_cursor->relative_offset * 2));
                    }

                    effect_value = (scaled_volume * (channel->volume_lfo_depth >> 8) << 9) >> 16;
                    lfo_samples = (s16*)channel->tempo;
                    waveform_sample = *lfo_samples++;
                    effect_value = (effect_value * waveform_sample) >> 15;
                    channel->tempo = (u32)lfo_samples;
                }
                if (effect_value != channel->volume_lfo_value)
                {
                    channel->volume_lfo_value = effect_value;
                    channel->update_flags |= 3;
                }
            }
        }
    }

    if (effect_flags & 4)
    {
        channel->pan_lfo_restart--;
        if (channel->pan_lfo_restart == 0)
        {
            channel->pan_lfo_restart = channel->pan_lfo_period;
            {
                AkaoLfoSample* lfo_cursor;
                s16* lfo_samples;

                lfo_cursor = (AkaoLfoSample*)channel->tempo_step;
                if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
                {
                    channel->tempo_step = (s32)(((u8*)lfo_cursor) + (lfo_cursor->relative_offset * 2));
                }

                lfo_samples = (s16*)channel->tempo_step;
                waveform_sample = *lfo_samples++;
                effect_value = ((channel->pan_lfo_depth >> 8) * waveform_sample) >> 15;
                channel->tempo_step = (s32)lfo_samples;
            }
            if (effect_value != channel->pan_lfo_value)
            {
                channel->pan_lfo_value = effect_value;
                channel->update_flags |= 3;
            }
        }
    }

    if (effect_flags & 0x20)
    {
        scaled_volume = ((s16)(*((u16*)((u8*)channel - 0xC)) << 1) * (channel->volume >> 8)) >> 7;
        channel->update_flags |= 3;
    }

    update_mask = channel->update_flags & 3;
    if (update_mask)
    {
        scaled_volume += channel->volume_lfo_value;
        scaled_volume = (scaled_volume * (*((u16*)((u8*)g_akao_seq_channel0 + 0x52)) & 0x7F)) >> 7;
        pan_index = ((channel->pan >> 8) + channel->pan_lfo_value) & 0xFF;

        if (D_8004F754[0] == 2)
        {
            channel->spu_volume_right = (scaled_volume * D_8003D47C) >> 15;
            channel->spu_volume_left = channel->spu_volume_right;
        }
        else
        {
            channel->spu_volume_left = (scaled_volume * D_8003D37C[pan_index]) >> 15;
            channel->spu_volume_right = (scaled_volume * D_8003D37C[pan_index ^ 0xFF]) >> 15;
        }
    }

    update_mask = effect_flags & 0x10;
    if (update_mask)
    {
        effect_value = *((s16*)((u8*)channel + 0x32));
        pitch_offset = *((u16*)((u8*)channel - 0xC)) + channel->pitch_lfo_value + effect_value;
        effect_value = g_akao_mastervol_acc & 0xFF0000;
        if (effect_value != 0)
        {
            effect_value >>= 16;
            if (effect_value < 0x80)
            {
                pitch_offset += (pitch_offset * effect_value) >> 7;
            }
            else
            {
                pitch_offset = (pitch_offset * effect_value) >> 8;
            }
        }
        channel->spu_pitch = (*((u16*)((u8*)channel + 0x54)) + pitch_offset) & 0x3FFF;
        channel->update_flags |= 0x10;
        return;
    }

    if (channel->update_flags & 0x10)
    {
        pitch_offset = channel->pitch + channel->pitch_lfo_value + *((s16*)((u8*)channel + 0x32));
        effect_value = g_akao_mastervol_acc & 0xFF0000;
        if (effect_value != 0)
        {
            effect_value >>= 16;
            if (effect_value < 0x80)
            {
                pitch_offset += (pitch_offset * effect_value) >> 7;
            }
            else
            {
                pitch_offset = (pitch_offset * effect_value) >> 8;
            }
        }
        channel->spu_pitch = (*((u16*)((u8*)channel + 0x54)) + pitch_offset) & 0x3FFF;
    }
}

/**
 * @brief Sample SFX-channel LFOs and recompute its pending SPU voice image.
 * @param channel Channel state to update.
 * @param channel_mask Channel bit retained by the original calling convention.
 */
void akao_update_sfx_channel_voice(AkaoChannelState* channel, s32 channel_mask)
{
    s32 effect_flags;
    s32 scaled_volume;
    s32 waveform_sample;
    s32 pan_index;
    s32 pitch_offset;
    s32 effect_value;
    s32 unused_flags;

    effect_flags = channel->flags;
    scaled_volume = (((s16*)&channel->unk48)[1] * (channel->volume >> 8)) >> 7;

    if (effect_flags & 1)
    {
        channel->pitch_lfo_restart--;
        if (channel->pitch_lfo_restart == 0)
        {
            channel->pitch_lfo_restart = *((u16*)((u8*)channel + 0xA6));
            {
                AkaoLfoSample* lfo_cursor;
                s16* lfo_samples;

                lfo_cursor = (AkaoLfoSample*)channel->unk1C;
                if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
                {
                    channel->unk1C = (s32)(((u8*)lfo_cursor) + (lfo_cursor->relative_offset * 2));
                }

                lfo_samples = (s16*)channel->unk1C;
                waveform_sample = *lfo_samples++;
                effect_value = ((s32)channel->pitch_lfo_depth_scaled * waveform_sample) >> 16;
                channel->unk1C = (s32)lfo_samples;
            }
            if (effect_value != channel->pitch_lfo_value)
            {
                channel->pitch_lfo_value = effect_value;
                channel->update_flags |= 0x10;
                if (effect_value >= 0)
                {
                    channel->pitch_lfo_value = effect_value << 1;
                }
            }
        }
    }

    if (effect_flags & 2)
    {
        channel->volume_lfo_restart--;
        if (channel->volume_lfo_restart == 0)
        {
            channel->volume_lfo_restart = *((u16*)((u8*)channel + 0xBA));
            {
                AkaoLfoSample* lfo_cursor;
                s16* lfo_samples;

                lfo_cursor = (AkaoLfoSample*)channel->tempo;
                if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
                {
                    channel->tempo = (u32)(((u8*)lfo_cursor) + (lfo_cursor->relative_offset * 2));
                }

                effect_value = (scaled_volume * (channel->volume_lfo_depth >> 8) << 9) >> 16;
                lfo_samples = (s16*)channel->tempo;
                waveform_sample = *lfo_samples++;
                effect_value = (effect_value * waveform_sample) >> 15;
                channel->tempo = (u32)lfo_samples;
            }
            if (effect_value != channel->volume_lfo_value)
            {
                channel->volume_lfo_value = effect_value;
                channel->update_flags |= 3;
            }
        }
    }

    if (effect_flags & 4)
    {
        channel->pan_lfo_restart--;
        if (channel->pan_lfo_restart == 0)
        {
            channel->pan_lfo_restart = channel->pan_lfo_period;
            {
                AkaoLfoSample* lfo_cursor;
                s16* lfo_samples;

                lfo_cursor = (AkaoLfoSample*)channel->tempo_step;
                if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
                {
                    channel->tempo_step = (s32)(((u8*)lfo_cursor) + (lfo_cursor->relative_offset * 2));
                }

                lfo_samples = (s16*)channel->tempo_step;
                waveform_sample = *lfo_samples++;
                effect_value = ((channel->pan_lfo_depth >> 8) * waveform_sample) >> 15;
                channel->tempo_step = (s32)lfo_samples;
            }
            if (effect_value != channel->pan_lfo_value)
            {
                channel->pan_lfo_value = effect_value;
                channel->update_flags |= 3;
            }
        }
    }

    if (effect_flags & 0x20)
    {
        scaled_volume = ((s16)(*((u16*)((u8*)channel - 0xC)) << 1) * (channel->volume >> 8)) >> 7;
        channel->update_flags |= 3;
    }

    if (channel->update_flags & 3)
    {
        scaled_volume += channel->volume_lfo_value;
        if (channel->tempo_acc & 0x02000000)
        {
            pan_index = 0x80;
        }
        else
        {
            s32 biased_pan;
            scaled_volume = (scaled_volume * (s8)(channel->volume_scale >> 8)) >> 7;
            biased_pan = ((channel->pan + channel->pan_bias) >> 8) + channel->pan_lfo_value;
            pan_index = biased_pan + 0x80;
            pan_index &= 0xFF;
        }

        if (D_8004F754[0] == 2)
        {
            channel->spu_volume_right = (scaled_volume * D_8003D47C) >> 15;
            channel->spu_volume_left = channel->spu_volume_right;
        }
        else
        {
            channel->spu_volume_left = (scaled_volume * D_8003D37C[pan_index]) >> 15;
            channel->spu_volume_right = (scaled_volume * D_8003D37C[pan_index ^ 0xFF]) >> 15;
        }
    }

    if (effect_flags & 0x10)
    {
        effect_value = *((s16*)((u8*)channel + 0x32));
        pitch_offset = *((u16*)((u8*)channel - 0xC)) + channel->pitch_lfo_value + effect_value;
        if (!(channel->tempo_acc & 0x02000000))
        {
            effect_value = channel->noise_mask & 0xFF00;
            if (effect_value != 0)
            {
                effect_value >>= 8;
                if (effect_value < 0x80)
                {
                    pitch_offset += (pitch_offset * effect_value) >> 7;
                }
                else
                {
                    pitch_offset = (pitch_offset * effect_value) >> 8;
                }
            }
        }
        channel->spu_pitch = (*((u16*)((u8*)channel + 0x54)) + pitch_offset) & 0x3FFF;
        channel->update_flags |= 0x10;
        return;
    }

    if (channel->update_flags & 0x10)
    {
        pitch_offset = channel->pitch + channel->pitch_lfo_value + *((s16*)((u8*)channel + 0x32));
        if (!(channel->tempo_acc & 0x02000000))
        {
            effect_value = channel->noise_mask & 0xFF00;
            if (effect_value != 0)
            {
                effect_value >>= 8;
                if (effect_value < 0x80)
                {
                    pitch_offset += (pitch_offset * effect_value) >> 7;
                }
                else
                {
                    pitch_offset = (pitch_offset * effect_value) >> 8;
                }
            }
        }
        channel->spu_pitch = (*((u16*)((u8*)channel + 0x54)) + pitch_offset) & 0x3FFF;
    }
}

/**
 * @brief Select and release the quietest allocated voice.
 * @param ignore_voice_reserve Nonzero to include voices below the reserve floor.
 * @return Selected voice index, or 0x18 when no voice is available.
 */
s32 akao_steal_quietest_voice(s32 ignore_voice_reserve)
{
    s32 voice_index;
    u16 quietest_level;
    s32 quietest_voice;
    AkaoVoiceAllocation* allocation;

    if (ignore_voice_reserve != 0)
    {
        voice_index = 0;
    }
    else
    {
        voice_index = g_akao_seq_channel0->voice_alloc_base;
    }

    quietest_level = 0x7FFF;
    quietest_voice = 0x18;
    allocation = &D_8004C1A0[voice_index];

    do
    {
        if (allocation->envelope_level < (s16)quietest_level)
        {
            quietest_level = (u16)allocation->envelope_level;
            quietest_voice = voice_index;
        }
        voice_index++;
        allocation++;
    } while (voice_index < 0x18);

    if ((s16)quietest_level == 0x7FFF)
    {
        return 0x18;
    }

    akao_clear_voice_assignment(g_akao_seq_channels, quietest_voice);
    return quietest_voice;
}

/**
 * @brief Find the first unallocated voice in the requested voice range.
 * @param ignore_voice_reserve Nonzero to include voices below the reserve floor.
 * @return Available voice index, or 0x18 when none is available.
 */
s32 akao_find_free_voice(s32 ignore_voice_reserve)
{
    AkaoVoiceAllocation* allocation;

    if (ignore_voice_reserve != 0)
    {
        ignore_voice_reserve = 0;
    }
    else
    {
        ignore_voice_reserve = g_akao_seq_channel0->voice_alloc_base;
    }

    allocation = &D_8004C1A0[ignore_voice_reserve];
    if (allocation->envelope_level != 0)
    {
        ignore_voice_reserve++;
        while (1)
        {
            if (ignore_voice_reserve >= 0x18)
            {
                break;
            }

            allocation++;
            ignore_voice_reserve++;
            if (allocation->envelope_level != 0)
            {
                continue;
            }

            ignore_voice_reserve--;
            break;
        }
    }

    return ignore_voice_reserve;
}

/**
 * @brief Recompute and commit pending voice updates for sequence channels.
 * @param channels First channel corresponding to channel_mask bit zero.
 * @param channel_mask Channels whose pending voice state must be processed.
 * @param static_voice_mask Channels bound to their matching SPU voice index.
 * @param key_on_voice_mask Accumulated SPU voices to key on after processing.
 * @see decomp.me (100%) https://decomp.me/scratch/wFlR3
 */
void akao_process_sequence_voice_updates(AkaoChannelState* channels, s32 channel_mask, s32 static_voice_mask, u32* key_on_voice_mask)
{
    s32 channel_bit;
    s32 channel_index;
    s32 voice_index;
    s32 pending_key_on_mask;
    s32 voice_bit;
    unsigned char unassigned_voice;
    channel_bit = 1;
    channel_index = 0;
    voice_bit = channel_bit;
    pending_key_on_mask = channel_mask & g_akao_seq_channel0->w04.song.key_on_mask;
    do
    {
        unassigned_voice = 0x18U;
        if (channel_mask & channel_bit)
        {
            akao_update_sequence_channel_voice(channels, channel_bit);
            if (channels->update_flags != 0)
            {
                if (D_8003EC6C & channel_bit)
                {
                    channels->spu_volume_right = 0;
                    channels->spu_volume_left = 0;
                }
                if (pending_key_on_mask & channel_bit)
                {
                    if (static_voice_mask & channel_bit)
                    {
                        *key_on_voice_mask |= voice_bit << channel_index;
                        channels->voice = channel_index;
                    }
                    else
                    {
                        s32 ignore_voice_reserve;
                        ignore_voice_reserve = (g_akao_seq_channel0->w04.song.voice_alloc_low_mask & channel_bit) != 0;
                        voice_index = akao_find_free_voice(ignore_voice_reserve);
                        if (voice_index == 0x18)
                        {
                            g_akao_seq_channel0->seq_cursor = (u8*)(((u32)g_akao_seq_channel0->seq_cursor) | 2);
                            voice_index = akao_steal_quietest_voice(ignore_voice_reserve);
                            if (voice_index == 0x18)
                            {
                                channels->voice = voice_index;
                                g_akao_seq_channel0->seq_cursor = (u8*)(((u32)g_akao_seq_channel0->seq_cursor) | 1);
                            }
                            else
                            {
                                *key_on_voice_mask |= voice_bit << voice_index;
                                channels->voice = voice_index;
                                D_8004C1A0[voice_index].envelope_level = 0x7FFF;
                            }
                        }
                        else
                        {
                            *key_on_voice_mask |= voice_bit << voice_index;
                            channels->voice = voice_index;
                            D_8004C1A0[voice_index].envelope_level = 0x7FFF;
                        }
                    }
                    if (channels->voice < unassigned_voice)
                    {
                        spu_write_voice_params(channels->voice, (void*)(&channels->voice), channels->spu_volume_scale);
                        D_8004F7C0[channels->voice] = g_akao_seq_channel0;
                        g_akao_driver_flags.unk8 |= 0x100;
                    }
                }
                else if (channels->voice < unassigned_voice)
                {
                    spu_apply_voice_updates(channels->voice, (void*)(&channels->voice), channels->flags);
                }
            }
            channel_mask &= ~channel_bit;
        }
        channel_bit <<= 1;
        channels++;
        channel_index++;
    } while (channel_mask != 0);
}

/**
 * @brief Clear one voice assignment from active channel tables.
 * @param primary_channels Base of the primary channel table.
 * @param voice_index Voice index to clear.
 */
void akao_clear_voice_assignment(u8* primary_channels, s32 voice_index)
{
    u32 channel_index;
    s32 unassigned_voice;
    u8* pending_channels;

    channel_index = 0;
    unassigned_voice = 0x18;
    primary_channels += 0xFC;
    do
    {
        if (voice_index == *(s32*)primary_channels)
        {
            *(s32*)primary_channels = unassigned_voice;
        }
        channel_index++;
        primary_channels += 0x118;
    } while (channel_index < 0x20U);

    if (g_akao_seq_channel1 != 0)
    {
        channel_index = 0;
        pending_channels = (u8*)(g_akao_pending_channels + 0xFC);
        do
        {
            if (voice_index == *(s32*)pending_channels)
            {
                *(s32*)pending_channels = 0x18;
            }
            channel_index++;
            pending_channels += 0x118;
        } while (channel_index < 0x20U);
    }
}

/**
 * @brief Refresh allocation state for every SPU voice.
 * @param reserved_voice_mask Voices already reserved by SFX or streaming audio.
 * @param xa_voice_mask Streaming-audio voice mask retained by the original ABI.
 */
void akao_refresh_voice_allocation_state(u32 reserved_voice_mask, s32 xa_voice_mask)
{
    u32 unavailable_voice_mask;
    u32 voice_index;
    s32 voice_bit;
    s32 active_level;
    u32 primary_active_mask;
    u32 primary_static_mask;
    AkaoSequenceVoiceMasks* secondary_masks;

    primary_active_mask = ((AkaoSequenceVoiceMasks*)g_akao_seq_channel0)->active_channel_mask;
    primary_static_mask = ((AkaoSequenceVoiceMasks*)g_akao_seq_channel0)->static_voice_mask;

    secondary_masks = (AkaoSequenceVoiceMasks*)g_akao_seq_channel1;

    unavailable_voice_mask = (primary_active_mask & primary_static_mask) | reserved_voice_mask;

    if (secondary_masks != 0)
    {
        unavailable_voice_mask |= secondary_masks->active_channel_mask & secondary_masks->static_voice_mask;
    }

    voice_index = 0;
    voice_bit = 1;
    active_level = 0x7FFF;

    {
        AkaoVoiceAllocation* allocation;
        s16* envelope_level;

        allocation = D_8004C1A0;
        envelope_level = &allocation->envelope_level;

        do
        {
            s32 is_unavailable0 = (unavailable_voice_mask & (voice_bit << voice_index)) != 0;
            s32 is_unavailable1 = (is_unavailable0 != 0);
            s32 is_unavailable2 = (is_unavailable1 != 0);
            s32 is_unavailable3 = (is_unavailable2 != 0);
            s32 is_unavailable4 = (is_unavailable3 != 0);
            s32 is_unavailable5 = (is_unavailable4 != 0);
            s32 is_unavailable6 = (is_unavailable5 != 0);
            s32 is_unavailable7 = (is_unavailable6 != 0);

            if (is_unavailable7)
            {
                *envelope_level = active_level;
            }
            else
            {
                akao_read_voice_envelope(voice_index, envelope_level);

                if (*envelope_level == 0)
                {
                    akao_clear_voice_assignment(g_akao_seq_channels, voice_index);
                }
            }

            voice_index++;
            envelope_level = (s16*)((u8*)envelope_level + 8);
        } while (voice_index < 0x18U);
    }
}

/**
 * @brief Flush pending sequence, SFX, and global SPU voice updates.
 * @note decomp.me (100%) https://decomp.me/scratch/PzQRP
 */
void akao_flush_voice_updates(s32 sfx_update_mask)
{
    u32 key_on_voice_mask;
    s32 reserved_voice_mask;
    s32 secondary_update_mask;
    s32 secondary_static_voice_mask;
    s32 secondary_low_voice_mask;
    s32 work_mask;
    s32 primary_static_voice_mask;
    s32 primary_low_voice_mask;
    AkaoChannelState* sfx_channel;
    typeof(g_akao_seq_channel0->w04.song)* song_masks;
    AkaoChannelState* secondary_song;
    s32* effect_mask_center;
    s32* effect_mask_base;
    s32 master_volume;
    s32 noise_frequency;

    secondary_update_mask = 0;
    secondary_static_voice_mask = 0;
    key_on_voice_mask = 0;
    reserved_voice_mask = (g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10) | D_8004F76C[0];

    if ((g_akao_seq_channel0->w04.song.active_mask & g_akao_seq_channel0->w04.song.key_on_mask) ||
        ((g_akao_seq_channel1 != NULL) && (g_akao_seq_channel1->w04.song.active_mask & g_akao_seq_channel1->w04.song.key_on_mask)))
    {
        akao_refresh_voice_allocation_state(reserved_voice_mask, D_8004F76C[0]);
    }

    if (g_akao_seq_channel1 != NULL)
    {
        g_akao_seq_channel0 = g_akao_seq_channel1;
        secondary_update_mask = g_akao_seq_channel1->w04.song.active_mask & g_akao_seq_channel1->note_on_mask &
                                ~(g_akao_seq_channel1->w04.song.static_voice_mask & reserved_voice_mask);
        secondary_static_voice_mask = g_akao_seq_channel1->w04.song.static_voice_mask;
        secondary_low_voice_mask = secondary_update_mask & g_akao_seq_channel1->w04.song.voice_alloc_low_mask;
        secondary_static_voice_mask = secondary_update_mask & secondary_static_voice_mask & ~reserved_voice_mask;
        if (secondary_low_voice_mask != 0)
        {
            akao_process_sequence_voice_updates((AkaoChannelState*)g_akao_pending_channels, secondary_low_voice_mask, secondary_static_voice_mask,
                                                &key_on_voice_mask);
            song_masks = &g_akao_seq_channel0->w04.song;
            secondary_update_mask &= ~song_masks->voice_alloc_low_mask;
            g_akao_seq_channel0->w04.song.key_on_mask &= ~g_akao_seq_channel0->w04.song.voice_alloc_low_mask;
        }
        g_akao_seq_channel0 = &g_akao_seq_master_state;
    }

    work_mask = g_akao_seq_channel0->w04.song.active_mask & g_akao_seq_channel0->note_on_mask &
                ~(g_akao_seq_channel0->w04.song.static_voice_mask & (secondary_static_voice_mask | reserved_voice_mask));
    primary_static_voice_mask = work_mask & g_akao_seq_channel0->w04.song.static_voice_mask & ~(secondary_static_voice_mask | reserved_voice_mask);
    primary_low_voice_mask = work_mask & g_akao_seq_channel0->w04.song.voice_alloc_low_mask;
    if (primary_low_voice_mask != 0)
    {
        akao_process_sequence_voice_updates((AkaoChannelState*)g_akao_seq_channels, primary_low_voice_mask, primary_static_voice_mask, &key_on_voice_mask);
        song_masks = &g_akao_seq_channel0->w04.song;
        work_mask &= ~song_masks->voice_alloc_low_mask;
        g_akao_seq_channel0->w04.song.key_on_mask &= ~g_akao_seq_channel0->w04.song.voice_alloc_low_mask;
    }

    if ((g_akao_seq_channel1 != NULL) && (secondary_update_mask != 0))
    {
        g_akao_seq_channel0 = g_akao_seq_channel1;
        akao_process_sequence_voice_updates((AkaoChannelState*)g_akao_pending_channels, secondary_update_mask,
                                            secondary_static_voice_mask & ~primary_static_voice_mask, &key_on_voice_mask);
        secondary_song = g_akao_seq_channel0;
        g_akao_seq_channel0 = &g_akao_seq_master_state;
        secondary_song->w04.song.key_on_mask = 0;
    }

    if (work_mask != 0)
    {
        akao_process_sequence_voice_updates((AkaoChannelState*)g_akao_seq_channels, work_mask, primary_static_voice_mask, &key_on_voice_mask);
        g_akao_seq_channel0->w04.song.key_on_mask = 0;
    }

    work_mask = g_akao_sfx_control.unk0 & g_akao_sfx_control.unk8;
    if (work_mask != 0)
    {
        do
        {
            primary_static_voice_mask = 0x1000;
        } while (0);
        sfx_channel = (AkaoChannelState*)g_sfx_channels;
        key_on_voice_mask |= g_akao_sfx_control.unk4;
        do
        {
            if (work_mask & primary_static_voice_mask)
            {
                akao_update_sfx_channel_voice(sfx_channel, primary_static_voice_mask);
                if (sfx_channel->update_flags != 0)
                {
                    spu_apply_voice_updates(sfx_channel->voice, (SpuVoiceParams*)&sfx_channel->voice, sfx_channel->flags);
                }
                work_mask &= ~primary_static_voice_mask;
            }
            primary_static_voice_mask <<= 1;
            sfx_channel++;
        } while (work_mask != 0);
        D_8004D404[0] = 0;
    }

    work_mask = g_akao_driver_flags.unk8;
    if (work_mask & 0x80)
    {
        master_volume = (s32)(g_akao_seq_channel0->unk48 << 4) >> 16;
        akao_set_reverb_volume(master_volume, master_volume);
        g_akao_driver_flags.unk8 &= ~0x80;
    }

    if (work_mask & 0x10)
    {
        if (g_akao_sfx_control.unk0 != 0)
        {
            noise_frequency = g_akao_sfx_control.unk28;
        }
        else
        {
            noise_frequency = g_akao_seq_channel0->noise_freq;
        }
        akao_set_noise_frequency(noise_frequency);
        g_akao_driver_flags.unk8 &= ~0x10;
    }

    if (work_mask & 0x100)
    {
        effect_mask_center = D_8004F834;
        akao_build_effect_voice_mask(effect_mask_center, g_akao_seq_channel1->reverb_mask, g_akao_seq_channel0->reverb_mask, g_akao_sfx_control.reverb_mask);
        effect_mask_base = effect_mask_center - 1;
        akao_build_effect_voice_mask(effect_mask_base, g_akao_seq_channel1->noise_mask, g_akao_seq_channel0->noise_mask, g_akao_sfx_control.noise_mask);
        akao_build_effect_voice_mask(effect_mask_center + 1, g_akao_seq_channel1->pitch_mod_mask, g_akao_seq_channel0->pitch_mod_mask, g_akao_sfx_control.pitch_mod_mask);
        spu_set_reverb_enable(effect_mask_center[-1]);
        spu_set_noise_enable(effect_mask_base[1]);
        spu_set_pitch_modulation_enable(effect_mask_base[2]);
        g_akao_driver_flags.unk8 &= ~0x100;
    }

    if (key_on_voice_mask != 0)
    {
        spu_set_key_on(key_on_voice_mask);
    }
}

/**
 * @brief OR each active channel's assigned SPU voice bit into a mask, then
 *        restrict the result to @p keep_mask.
 *
 * For every channel selected by @p channel_mask that owns a live voice
 * (index < 0x18), the bit for that voice is set in @p *voice_mask. After all
 * selected channels have been scanned the accumulated mask is ANDed with
 * @p keep_mask.
 *
 * @param channels First channel corresponding to channel_mask bit zero.
 * @param voice_mask Accumulator receiving the collected voice bits.
 * @param channel_mask Channels to scan.
 * @param keep_mask Mask ANDed into the result once scanning completes.
 */
void akao_collect_channel_voice_mask(AkaoChannelState* channels, u32* voice_mask, s32 channel_mask, s32 keep_mask)
{
    s32 channel_bit;
    s32 voice_bit;
    u32 voice;

    channel_bit = 1;
    voice_bit = channel_bit;
    do
    {
        if (channel_mask & channel_bit)
        {
            voice = channels->voice;
            if (voice < 0x18U)
            {
                *voice_mask |= voice_bit << voice;
            }
        }
        channel_mask &= ~channel_bit;
        channels++;
        channel_bit <<= 1;
    } while (channel_mask != 0);

    *voice_mask &= keep_mask;
}

/**
 * @brief Fold every channel's pending key-off into a single SPU key-off write.
 *
 * For the secondary (@c g_akao_seq_channel1 / pending set) and primary
 * (@c g_akao_seq_channel0 / active set) songs, each song's @c key_off_mask is
 * split into its voice-alloc-low channels (handled first) and the remainder,
 * and the SPU voices those channels hold are gathered via
 * @ref akao_collect_channel_voice_mask, excluding voices reserved by SFX or XA.
 * The accumulated voices are OR'd with the SFX control key-off mask and, if any
 * remain, keyed off in one @ref spu_set_key_off call. Each processed
 * @c key_off_mask is cleared.
 *
 * @see decomp.me (100%)
 */
void akao_flush_voice_key_offs(void)
{
    u32 key_off_voice_mask;
    s32 keep_mask;
    s32 secondary_residual;
    s32 primary_residual;
    s32 secondary_active;
    s32 primary_active;
    typeof(g_akao_seq_channel0->w04.song)* song_masks;

    secondary_residual = 0;
    key_off_voice_mask = 0;
    keep_mask = ~((g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10) | D_8004F76C[0]);

    if (g_akao_seq_channel1 != NULL)
    {
        secondary_residual = g_akao_seq_channel1->key_off_mask;
        secondary_active = secondary_residual & g_akao_seq_channel1->w04.song.voice_alloc_low_mask;
        if (secondary_active != 0)
        {
            akao_collect_channel_voice_mask((AkaoChannelState*)g_akao_pending_channels, &key_off_voice_mask, secondary_active, keep_mask);
            song_masks = &g_akao_seq_channel1->w04.song;
            secondary_residual &= ~song_masks->voice_alloc_low_mask;
            g_akao_seq_channel1->key_off_mask &= ~g_akao_seq_channel1->w04.song.voice_alloc_low_mask;
        }
    }

    primary_residual = g_akao_seq_channel0->key_off_mask;
    primary_active = primary_residual & g_akao_seq_channel0->w04.song.voice_alloc_low_mask;
    if (primary_active != 0)
    {
        akao_collect_channel_voice_mask((AkaoChannelState*)g_akao_seq_channels, &key_off_voice_mask, primary_active, keep_mask);
        song_masks = &g_akao_seq_channel0->w04.song;
        primary_residual &= ~song_masks->voice_alloc_low_mask;
        g_akao_seq_channel0->key_off_mask &= ~g_akao_seq_channel0->w04.song.voice_alloc_low_mask;
    }

    if ((g_akao_seq_channel1 != NULL) && (secondary_residual != 0))
    {
        akao_collect_channel_voice_mask((AkaoChannelState*)g_akao_pending_channels, &key_off_voice_mask, secondary_residual, keep_mask);
        g_akao_seq_channel1->key_off_mask = 0;
    }

    if (primary_residual != 0)
    {
        akao_collect_channel_voice_mask((AkaoChannelState*)g_akao_seq_channels, &key_off_voice_mask, primary_residual, keep_mask);
        g_akao_seq_channel0->key_off_mask = 0;
    }

    key_off_voice_mask |= g_akao_sfx_control.unkC;
    g_akao_sfx_control.unkC = 0;
    if (key_off_voice_mask != 0)
    {
        spu_set_key_off(key_off_voice_mask);
    }
}

/**
 * @brief Build the SPU voice bitmap for one per-voice effect register.
 *
 * For the secondary (@c g_akao_seq_channel1 / pending set) and primary
 * (@c g_akao_seq_channel0 / active set) songs, the channels that are both
 * active and enabled for the effect (@c active_mask & the song's effect mask)
 * are taken, and the SPU voices they hold are gathered via
 * @ref akao_collect_channel_voice_mask, excluding voices reserved by SFX or XA.
 * The gathered voices are OR'd with @p sfx_effect_voices, stored to
 * @p *effect_voices, and an SPU effect update is flagged
 * (@c g_akao_driver_flags.unk8 bit 0x100). Used for the reverb, noise, and
 * pitch-modulation enable bitmaps.
 *
 * @param effect_voices Destination for the assembled SPU voice bitmap.
 * @param secondary_effect_mask Secondary song's per-channel effect-enable mask.
 * @param primary_effect_mask Primary song's per-channel effect-enable mask.
 * @param sfx_effect_voices SFX voices already enabled for this effect.
 * @see decomp.me (100%)
 */
void akao_build_effect_voice_mask(s32* effect_voices, s32 secondary_effect_mask, s32 primary_effect_mask, s32 sfx_effect_voices)
{
    u32 voice_mask;
    s32 keep_mask;
    s32 secondary_residual;
    s32 primary_residual;
    s32 secondary_active;
    s32 primary_active;

    secondary_residual = 0;
    voice_mask = 0;
    keep_mask = ~((g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10) | D_8004F76C[0]);

    if (g_akao_seq_channel1 != NULL)
    {
        secondary_residual = g_akao_seq_channel1->w04.song.active_mask & secondary_effect_mask;
        secondary_active = secondary_residual & g_akao_seq_channel1->w04.song.voice_alloc_low_mask;
        if (secondary_active != 0)
        {
            akao_collect_channel_voice_mask((AkaoChannelState*)g_akao_pending_channels, &voice_mask, secondary_active, keep_mask);
            secondary_residual &= ~g_akao_seq_channel1->w04.song.voice_alloc_low_mask;
        }
    }

    primary_residual = g_akao_seq_channel0->w04.song.active_mask & primary_effect_mask;
    primary_active = primary_residual & g_akao_seq_channel0->w04.song.voice_alloc_low_mask;
    if (primary_active != 0)
    {
        akao_collect_channel_voice_mask((AkaoChannelState*)g_akao_seq_channels, &voice_mask, primary_active, keep_mask);
        primary_residual &= ~g_akao_seq_channel0->w04.song.voice_alloc_low_mask;
    }

    if ((g_akao_seq_channel1 != NULL) && (secondary_residual != 0))
    {
        akao_collect_channel_voice_mask((AkaoChannelState*)g_akao_pending_channels, &voice_mask, secondary_residual, keep_mask);
    }

    if (primary_residual != 0)
    {
        akao_collect_channel_voice_mask((AkaoChannelState*)g_akao_seq_channels, &voice_mask, primary_residual, keep_mask);
    }

    voice_mask |= sfx_effect_voices;
    *effect_voices = voice_mask;
    g_akao_driver_flags.unk8 |= 0x100;
}

/**
 * @brief Program the SPU noise frequency field of SPUCNT.
 *
 * Clamps @p noise_freq to 0..0x3F (negative values become 0, values >= 0x40
 * saturate to 0x3F) and writes it into the noise frequency step+shift field
 * (bits 8-13) of the SPU control register (SPUCNT) at @c D_8003D0C0 + 0x1AA,
 * preserving the remaining bits via the 0xC0FF mask.
 *
 * @param noise_freq Requested noise frequency; negative clamps to 0, values >= 0x40 clamp to 0x3F.
 * @return The clamped noise frequency actually written (0..0x3F).
 * @see decomp.me (100%)
 */
s32 akao_set_noise_frequency(s32 noise_freq)
{
    u16* spu_ctrl;
    s32 clamped;
    s32 result;

    clamped = 0;
    if (noise_freq >= 0)
    {
        result = noise_freq;
        clamped = result;
        if (clamped >= 0x40)
        {
            clamped = 0x3F;
        }
    }

    spu_ctrl = (u16*)(D_8003D0C0 + 0x1AA);
    result = clamped;
    *spu_ctrl = (u16)((*spu_ctrl & 0xC0FF) | ((clamped & 0x3F) << 8));
    return result;
}

/**
 * @brief Read a voice's current ADSR envelope volume from the SPU.
 *
 * Reads the ADSR Current Volume (ENVX) halfword at offset 0xC of SPU voice
 * register block @p voice_index (each block is 16 bytes, based at
 * @c D_8003D0C0) and stores it to @p envelope_out.
 *
 * @param voice_index  SPU voice number (0-23).
 * @param envelope_out Destination for the voice's current envelope volume.
 * @see decomp.me (100%)
 */
void akao_read_voice_envelope(s32 voice_index, s16* envelope_out)
{
    *envelope_out = *(u16*)(D_8003D0C0 + (voice_index * 16) + 0xC);
}

/**
 * @brief Set the SPU reverb output volume and cache it.
 *
 * Writes @p reverb_left and @p reverb_right into the SPU Reverb Output Volume
 * Left/Right registers (at @c D_8003D0C0 + 0x184 / +0x186) and mirrors both
 * values into the @c D_8003D068 software shadow.
 *
 * @param reverb_left  Reverb output volume for the left channel.
 * @param reverb_right Reverb output volume for the right channel.
 * @see decomp.me (100%)
 */
void akao_set_reverb_volume(s32 reverb_left, s32 reverb_right)
{
    s16* shadow_l = D_8003D068;
    s16* shadow_r = shadow_l + 1;

    *(u16*)(D_8003D0C0 + 0x184) = reverb_left;
    *(u16*)(D_8003D0C0 + 0x186) = reverb_right;
    shadow_r[-1] = reverb_left;
    *shadow_r = reverb_right;
}

/**
 * @see decomp.me (100%)
 */
void akao_channel_init_state(AkaoChannelState* channel, u8* seq_data)
{
    channel->volume = 0x6E00;
    channel->unk48 = 0x32000000;
    channel->seq_cursor = seq_data;
    channel->transpose = 0;
    channel->detune = 0;
    channel->portamento_speed = 0;
    channel->unk30 = 0;
    channel->pitch_slide_delta = 0;
    channel->pitch_slide_ticks = 0;
    channel->note_duration_adjust = 0;
    channel->note_duration = 0;
    channel->expression_fade_ticks = 0;
    channel->detune_pitch_delta = 0;
    channel->pitch_scale = 0;
    channel->loop_depth = 0;
    channel->flags = 0;
    channel->pan_lfo_value = 0;
    channel->note_flags = 0;
    channel->opcode_count = 0xFFFF;
    channel->spu_volume_scale = 0;
    channel->pan_lfo_depth = 0;
    channel->volume_lfo_depth = 0;
    channel->pitch_lfo_depth = 0;
    channel->pan_lfo_depth_fade_ticks = 0;
    channel->volume_lfo_depth_fade_ticks = 0;
    channel->pitch_lfo_depth_fade_ticks = 0;
    channel->pitch_mod_toggle_ticks = 0;
    channel->reverb_toggle_ticks = 0;
    akao_channel_set_articulation(channel, 0);
}

/**
 * @see decomp.me (100%)
 */
u32 akao_collect_voice_mask(AkaoChannelState* channels, s32 channel_mask)
{
    u32 i;
    u32 voice;
    u32 result;
    s32 bit;

    for (i = 0, result = 0; i < 0x20; i++)
    {
        bit = 1 << i;
        if (channel_mask & bit)
        {
            voice = channels->voice;
            if (voice < 0x18U)
            {
                result |= 1 << voice;
            }
        }
        channels++;
    }
    return result;
}

/**
 * @brief Initialize the primary song-sequencer state and start playback of a
 *        song descriptor.
 *
 * Records @p descriptor into the (song-role) @c pitch field, seeds
 * voice_alloc_low_mask/static_voice_mask and the seq_cursor flag word from
 * the descriptor, sets up the articulation-map and note-table self-relative
 * pointers, then walks all 32 channels: channels selected by @p channel_mask
 * get a full note-start reset (LFOs, envelopes, timers) plus a fresh
 * articulation; channels present in the descriptor's own mask but not in
 * @p channel_mask instead get a lighter "silence" reset. Finishes by
 * resetting the song's tempo/master-volume/measure state to defaults.
 *
 * @param descriptor  Song descriptor: channel mask (+0x20), voice-mask seeds
 *        (+0x24/+0x28), song id (+0x14), articulation-map and note-table
 *        self-relative offsets (+0x30/+0x34), and a per-channel note-pointer
 *        table (+0x40). No named struct yet.
 * @param channel_mask Channels to actually start now.
 * @see decomp.me (100%)
 */
void akao_seq_start_song(u8* arg0, s32 arg1)
{
    s32 channel_mask;
    s32 flag_word;
    s32 flag_word2;
    s32 rel;
    u8* articulation_base;
    u8* note_table;
    u8* rel_table30;
    u8* rel_table34;
    u8* desc;
    AkaoChannelState* channel;
    u32 i;
    s32 bit;
    s32 static_voice_mask;
    u8* silence_ptr;
    s32 driver_mode;
    AkaoChannelState* song_b;
    AkaoChannelState* song_a;

    g_akao_seq_channel0->pitch = (s32)arg0;
    channel_mask = *(s32*)(arg0 + 0x20);
    desc = arg0;

    if (g_akao_seq_channel1 != 0)
    {
        flag_word = (s32)g_akao_pending_channels;
        bit = akao_collect_voice_mask((AkaoChannelState*)flag_word, g_akao_seq_channel1->w04.song.active_mask);
    }
    else
    {
        bit = 0;
    }

    g_akao_sfx_control.unkC |= (~bit & 0xFFFFFF) & ~(g_akao_sfx_control.unk0 | D_8004F76C[0]);
    driver_mode = g_akao_driver_mode_flags & 1;

    g_akao_seq_channel0->key_off_mask = 0;

    if (driver_mode)
    {
        g_akao_seq_channel0->w04.song.active_mask = 0;
        g_akao_seq_channel0->unk1C |= channel_mask & arg1;
    }
    else
    {
        g_akao_seq_channel0->unk1C = 0;
        g_akao_seq_channel0->w04.song.active_mask |= channel_mask & arg1;
    }

    g_akao_seq_channel0->w04.song.voice_alloc_low_mask = *(s32*)(desc + 0x24);
    song_a = g_akao_seq_channel0;
    song_a->w04.song.static_voice_mask = *(s32*)(desc + 0x28);

    flag_word = (s32)song_a->seq_cursor;
    flag_word2 = flag_word & ~0x63;
    song_a->seq_cursor = (u8*)flag_word2;
    flag_word = *(s32*)(desc + 0x14);
    if (flag_word == D_8003EC34[0])
    {
        flag_word = flag_word2 | 0x40;
    }
    else
    {
        flag_word = flag_word2 | 0x20;
    }
    song_a->seq_cursor = (u8*)flag_word;

    while (0) { }

    articulation_base = NULL;
    rel = *(s32*)(desc + 0x30);
    song_b = *(AkaoChannelState* volatile *)&g_akao_seq_channel0;
    rel_table30 = desc + (rel + 0x30);
    if (rel != 0)
    {
        articulation_base = rel_table30;
    }
    song_b->unk30 = (s32)articulation_base;

    articulation_base = NULL;
    rel = *(s32*)(desc + 0x34);
    rel_table34 = desc + (rel + 0x34);
    if (rel != 0)
    {
        articulation_base = rel_table34;
    }

    bit = 1;
    i = 0;
    channel = (AkaoChannelState*)g_akao_seq_channels;
    arg0 += 0x40;
    silence_ptr = D_8003D248;

    song_b->flags = (s32)articulation_base;
    song_b->voice_alloc_base = 0;
    do
    {
        if ((channel_mask & bit) & arg1)
        {
            channel->seq_cursor = arg0 + *(u16*)arg0;
            channel->unk66 = 4;
            channel->unk68 = 2;
            channel->volume = 0x7F00;
            channel->unk48 = 0x3FFF0000;
            channel->volume_scale = 0x4000;
            channel->detune = 0;
            channel->transpose = 0;
            channel->portamento_speed = 0;
            channel->unk30 = 0;
            channel->pitch_slide_delta = 0;
            channel->pitch_slide_ticks = 0;
            channel->note_duration_adjust = 0;
            channel->note_duration = 0;
            channel->pan = 0x8000;
            channel->pan_fade_ticks = 0;
            channel->portamento_speed = 0;
            channel->unk8E = 0;
            channel->expression_fade_ticks = 0;
            channel->detune_pitch_delta = 0;
            channel->note_expression_ticks = 0;
            channel->pitch_scale = 0;
            channel->note_flags = 0;
            channel->pan_lfo_value = 0;
            channel->loop_depth = 0;
            static_voice_mask = g_akao_seq_channel0->w04.song.static_voice_mask;
            arg0 += 2;
            channel->pan_lfo_depth = 0;
            channel->volume_lfo_depth = 0;
            channel->pitch_lfo_depth = 0;
            channel->pan_lfo_depth_fade_ticks = 0;
            channel->flags = static_voice_mask & bit;
            channel->flags = (channel->flags == 0) << 6;
            channel->volume_lfo_depth_fade_ticks = 0;
            channel->pitch_lfo_depth_fade_ticks = 0;
            channel->pitch_mod_toggle_ticks = 0;
            channel->reverb_toggle_ticks = 0;
            akao_channel_set_articulation(channel, 0);
        }
        else
        {
            if (channel_mask & bit)
            {
                if (!(bit & arg1))
                {
                    arg0 += 2;
                }
            }
            channel->unk66 = 3;
            channel->unk68 = 1;
            channel->seq_cursor = silence_ptr;
            channel->update_flags |= 0x4400;
            channel->spu_adsr_high = (channel->spu_adsr_high & 0xFFE0) | 5;
        }
        channel->voice = 0x18;
        channel_mask &= ~bit;
        channel++;
        i++;
        bit <<= 1;
    } while (i < 0x20U);

    g_akao_seq_channel0->tempo = 0xFFFF0000;
    g_akao_seq_channel0->tempo_acc = 1;
    g_akao_seq_channel0->tempo_fade_ticks = 0;
    g_akao_seq_channel0->unk48 = 0;
    g_akao_seq_channel0->master_vol_fade_ticks = 0;
    g_akao_seq_channel0->unk4C = 0;
    g_akao_driver_flags.unk8 = 0;
    g_akao_seq_channel0->unk6A = 0;
    g_akao_seq_channel0->unk68 = 0;
    g_akao_seq_channel0->unk66 = 0;
    g_akao_seq_channel0->measure = 0;
    g_akao_seq_channel0->reverb_mask = 0;
    g_akao_seq_channel0->noise_mask = 0;
    g_akao_seq_channel0->pitch_mod_mask = 0;
    g_akao_seq_channel0->unk60 = 0;
    g_akao_seq_channel0->note_on_mask = 0;
    g_akao_seq_channel0->w04.song.key_on_mask = 0;
    g_akao_driver_flags.unk8 |= 0x100;
}

/**
 * @see decomp.me (100%)
 */
void akao_seq_stop_song(AkaoChannelState* song, AkaoChannelState* channels, s32 song_key)
{
    u32 i;

    if (song->w04.song.active_mask == 0)
    {
        return;
    }
    if (song_key != 0)
    {
        if (song_key != song->unk5E)
        {
            return;
        }
    }
    song->key_off_mask = -1;

    i = 0x20;
    do
    {
        channels->unk66 = 3;
        channels->unk68 = 1;
        channels->seq_cursor = D_8003D248;
        channels++;
        i--;
    } while (i != 0);

    song->unk5E = 0;
    song->note_on_mask = 0;
    song->w04.song.key_on_mask = 0;

    for (i = 0; i < 0x18; i++)
    {
        if (D_8004F7C0[i] == song)
        {
            D_8004F7C0[i] = NULL;
            spu_set_voice_release_mode(i, 5, 3);
        }
    }
}

/**
 * @see decomp.me (100%)
 */
void akao_sfx_stop_channels(s32 sfx_id, s32 mode)
{
    AkaoChannelState* ch;
    s32 mask;
    u32 i;
    s32 active;
    s32 flags;
    s32 priority;
    s32 tmp;

    mask = 0x1000;
    ch = (AkaoChannelState*)g_sfx_channels;
    active = g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10;

    if (mode & 0x0FFFFFFF)
    {
        for (i = 0; i < 0xC; i++, ch++, mask <<= 1)
        {
            if ((active & mask) && (ch->tempo_acc & mode))
            {
                flags = ch->flags;
                if (flags & 0x100000)
                {
                    ch->flags = flags | 0x200000;
                }
                else
                {
                    g_akao_sfx_control.unkC |= mask;
                    akao_sfx_release_channels(ch, mask);
                    ch->flags = 0;
                }
            }
        }
    }
    else if (mode < 0)
    {
        ch += sfx_id;
        mask <<= sfx_id;
        if (active & mask)
        {
            akao_sfx_stop_channels(ch->reverb_mask, 0);
        }
        mask <<= 1;
        ch++;
        if (active & mask)
        {
            akao_sfx_stop_channels(ch->reverb_mask, 0);
        }
        return;
    }
    else if (mode & 0x40000000)
    {
        for (i = 0; i < 0xC; i++, ch++, mask <<= 1)
        {
            if (ch->tempo_acc != 0)
            {
                active &= ~mask;
            }
        }

        ch = (AkaoChannelState*)g_sfx_channels;
        mask = 0x1000;
        priority = 0;
        for (i = 0; i < 0xC; i++, ch++, mask <<= 1)
        {
            if (active & mask)
            {
                tmp = *(s32*)&ch->unk58;
                if (priority < tmp)
                {
                    priority = tmp;
                }
            }
        }

        ch = (AkaoChannelState*)g_sfx_channels;
        mask = 0x1000;
        for (i = 0; i < 0xC; i++, ch++, mask <<= 1)
        {
            if ((active & mask) && (priority == *(s32*)&ch->unk58))
            {
                flags = ch->flags;
                if (flags & 0x100000)
                {
                    ch->flags = flags | 0x200000;
                }
                else
                {
                    g_akao_sfx_control.unkC |= mask;
                    akao_sfx_release_channels(ch, mask);
                    ch->flags = 0;
                }
            }
        }
    }
    else
    {
        for (i = 0; i < 0xC; i++, ch++, mask <<= 1)
        {
            if (active & mask)
            {
                if (sfx_id == -1)
                {
                    if ((s32)ch->reverb_mask < 0)
                    {
                        flags = ch->flags;
                        if (flags & 0x100000)
                        {
                            ch->flags = flags | 0x200000;
                        }
                        else
                        {
                            g_akao_sfx_control.unkC |= mask;
                            akao_sfx_release_channels(ch, mask);
                            ch->flags = 0;
                        }
                    }
                }
                else if ((s32)ch->reverb_mask == sfx_id)
                {
                    flags = ch->flags;
                    if (flags & 0x100000)
                    {
                        ch->flags = flags | 0x200000;
                    }
                    else
                    {
                        g_akao_sfx_control.unkC |= mask;
                        akao_sfx_release_channels(ch, mask);
                        ch->flags = 0;
                    }
                }
            }
        }
    }
    g_akao_driver_flags.unk8 |= 0x110;
}

/**
 * @see decomp.me (100%)
 */
void akao_sfx_start_channel(AkaoChannelState* channel, u8* params, s32 channel_mask, u8* seq_data)
{
    s32 n;

    channel->reverb_mask = *(s32*)(params + 0x0);
    channel->tempo_acc = *(s32*)(params + 0x4);
    channel->pan_bias = *(u8*)(params + 0x8) << 8;
    channel->pan_bias_fade_ticks = 0;
    channel->pan = 0x8000;
    channel->pan_fade_ticks = 0;
    channel->volume_scale = (*(u16*)(params + 0xC) & 0x7F) << 8;
    channel->unk8E = 0;
    channel->voice_alloc_base = *(s32*)(params + 0x10);
    channel->unk66 = 2;
    channel->unk68 = 1;
    channel->is_sfx_channel = 1;
    *(s32*)&channel->unk58 = -2;
    channel->noise_mask = 0;
    channel->unk88 = 0;
    akao_channel_init_state(channel, seq_data);

    D_8004F7C0[channel->voice] = NULL;
    spu_set_voice_release_mode(channel->voice, 5, 3);

    g_akao_sfx_control.unk0 |= channel_mask;
    g_akao_sfx_control.unkC |= channel_mask;
    channel_mask = ~channel_mask;
    g_akao_sfx_control.unk4 &= channel_mask;
    g_akao_sfx_control.unk8 &= channel_mask;
    g_akao_sfx_control.reverb_mask &= channel_mask;
    g_akao_sfx_control.noise_mask &= channel_mask;
    g_akao_sfx_control.pitch_mod_mask &= channel_mask;

    if (g_akao_driver_mode_flags & 2)
    {
        channel_mask = 0x1000;
        channel = (AkaoChannelState*)g_sfx_channels;
        for (n = 0xC; n != 0; n--, channel++, channel_mask <<= 1)
        {
            if (g_akao_sfx_control.unk0 & channel_mask)
            {
                if (!(channel->tempo_acc & 0x2000000))
                {
                    g_akao_sfx_control.unk0 &= ~channel_mask;
                    g_akao_sfx_control.unk10 |= channel_mask;
                }
            }
        }
    }
}

/**
 * @see decomp.me (100%)
 */
void akao_unassign_voice(AkaoChannelState* channels, u32 voice_index)
{
    u32 channel_index;
    s32 unassigned_voice;
    s32 bit;
    AkaoChannelState* song;

    if (voice_index < 0x18U)
    {
        channel_index = 0;
        unassigned_voice = 0x18;
        song = g_akao_seq_channel0;
        do
        {
            if (channels->voice == voice_index)
            {
                bit = 1 << channel_index;
                channels->voice = unassigned_voice;
                song->note_on_mask &= ~bit;
            }
            channel_index++;
            channels++;
        } while (channel_index < 0x20U);
    }
}

/**
 * @see decomp.me (100%)
 */
void akao_sfx_play(u8* params, u8* seq_data0, u8* seq_data1, s32 skip_stop)
{
    AkaoChannelState* ch;
    u32 mask;
    u32 pair_bits;
    u32 test_mask;
    s32 busy;
    s32 n;
    s32 stop_mask;

    if (seq_data0 == 0 && seq_data1 == 0)
    {
        return;
    }

    if (skip_stop == 0)
    {
        stop_mask = *(s32*)(params + 4);
        if (stop_mask != 0)
        {
            akao_sfx_stop_channels(0, stop_mask);
        }
    }

    do
    {
        ch = D_8004C038;
        mask = 0x800000;
        busy = (g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10) | g_akao_xa_tracker.unkC;
        if (seq_data0 != 0 && seq_data1 != 0)
        {
            n = 0xB;
            ch--;
            mask = 0x400000;
            while (1)
            {
                pair_bits = mask << 1;
                test_mask = mask | pair_bits;
                if (busy & test_mask)
                {
                    n--;
                    ch--;
                    mask >>= 1;
                    if (n == 0)
                    {
                        break;
                    }
                    continue;
                }
                break;
            }
        }
        else
        {
            n = 0xC;
            for (; n != 0; n--, ch--, mask >>= 1)
            {
                if (!(busy & mask))
                {
                    break;
                }
            }
        }
        if (n != 0)
        {
            break;
        }
        akao_sfx_stop_channels(0, 0x40000000);
        if (busy == (s32)((g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10) | g_akao_xa_tracker.unkC))
        {
            return;
        }
    } while (n == 0);

    if (seq_data0 != 0)
    {
        akao_sfx_start_channel(ch, params, mask, seq_data0);
        akao_unassign_voice((AkaoChannelState*)g_akao_seq_channels, ch->voice);
    }
    if (seq_data1 != 0)
    {
        if (seq_data0 != 0)
        {
            ch++;
            mask <<= 1;
        }
        akao_sfx_start_channel(ch, params, mask, seq_data1);
        akao_unassign_voice((AkaoChannelState*)g_akao_seq_channels, ch->voice);
        if (seq_data0 != 0)
        {
            ch->flags |= 0x10000;
        }
    }
    g_akao_driver_flags.unk8 |= 0x110;
}

/**
 * @brief Resolve a program's two data pointers from the bank program table.
 *
 * Masks @p program_index to 10 bits and reads the two consecutive u16 offset
 * entries from the flat u16 table @c g_akao_bank_prog_base at halfword indices
 * @c 2*n and @c 2*n+1. Each entry is an offset into the region-C data block
 * (@c g_akao_bank_region_c); the sentinel @c 0xFFFF resolves to a null pointer.
 * Used by the SFX launcher to obtain the two seq-data pointers it passes to
 * akao_sfx_play.
 *
 * @param out0 Receives the resolved pointer for the first entry (2*n).
 * @param out1 Receives the resolved pointer for the second entry (2*n + 1).
 * @param program_index Program id; masked to 0..0x3FF.
 *
 * @see decomp.me (100%)
 */
void akao_resolve_program_data(s32* out0, s32* out1, s32 program_index)
{
    s32 result;

    program_index &= 0x3FF;
    program_index <<= 1;

    if (((u16*)g_akao_bank_prog_base)[program_index] != 0xFFFF)
    {
        result = g_akao_bank_region_c + ((u16*)g_akao_bank_prog_base)[program_index];
    }
    else
    {
        result = 0;
    }
    *out0 = result;

    program_index++;
    if (((u16*)g_akao_bank_prog_base)[program_index] != 0xFFFF)
    {
        result = g_akao_bank_region_c + ((u16*)g_akao_bank_prog_base)[program_index];
    }
    else
    {
        result = 0;
    }
    *out1 = result;
}

/**
 * @see decomp.me (100%)
 */
void akao_seq_flag_volume_update(AkaoChannelState* song, AkaoChannelState* channels)
{
    s32 mask;
    s32 bit;
    s32 song_mask;

    song_mask = song->w04.song.active_mask;
    if (song_mask != 0)
    {
        mask = song_mask;
        bit = 1;
        do
        {
            if (mask & bit)
            {
                mask ^= bit;
                channels->update_flags |= 3;
            }
            channels++;
            bit <<= 1;
        } while (mask != 0);
    }
}

/**
 * @brief Flag every SFX channel active in @c g_akao_sfx_control for a pending
 *        SPU volume re-apply.
 * @see decomp.me (100%)
 */
void akao_sfx_flag_volume_update(void)
{
    s32 mask;
    s32 bit;
    s32 sfx_mask;
    AkaoChannelState* channel;

    channel = (AkaoChannelState*)g_sfx_channels;
    sfx_mask = g_akao_sfx_control.unk0;
    if (sfx_mask != 0)
    {
        mask = sfx_mask;
        bit = 0x1000;
        do
        {
            if (mask & bit)
            {
                mask ^= bit;
                channel->update_flags |= 3;
            }
            channel++;
            bit <<= 1;
        } while (mask != 0);
    }
}

/**
 * @brief Restore the previously-suspended song state from the backup slots
 *        and rebase every bytecode-relative channel pointer to @p descriptor.
 *
 * Restores @c g_akao_seq_channel0 (0x70 bytes) and the full @c g_akao_seq_channels
 * array from the @c D_8004C2D0 / @c D_8004D450 backup slots, then adds the
 * delta between @p descriptor and the previously recorded base (@c D_8004C2FC)
 * to every pointer field that was computed relative to the old descriptor
 * address (seq_cursor, the loop-cursor stack, key_off_mask's channel-role
 * pointer use).
 *
 * @param descriptor Newly (re)loaded song descriptor; same shape consumed by
 *        akao_seq_start_song.
 *
 * @note NOT YET 100% (94.42%, 139/154 exact). sched_oracle confirms the
 *       residue is not a source-fixable emit-order issue (all inferred
 *       constraints SATISFIED); it is register-allocation coloring
 *       (g_akao_seq_channel0 pointer vs the masked flags value early on,
 *       and voice_mask's register choice) plus a CSE-FOLD-shaped reordering
 *       of the D_8004C32E store relative to the g_akao_sfx_control.unkC
 *       update near the end. See working/func_80026F28/code.c for the
 *       current best source and permuter session state.
 * @see decomp.me (94.42%)
 */
void akao_seq_resume_song(u8* descriptor)
{
    AkaoChannelState* channel;
    AkaoChannelState* song;
    s32 flags;
    s32 delta;
    s32 mask;
    s32 bit;
    u32 i;
    s32 voice_mask;
    s32 keep_mask;

    akao_copy_bytes((s32*)D_8004C2D0, (s32*)g_akao_seq_channel0, 0x70);
    akao_copy_bytes((s32*)D_8004D450, (s32*)g_akao_seq_channels, 0x2300);

    flags = (s32)g_akao_seq_channel0->seq_cursor & ~0x60;
    g_akao_seq_channel0->seq_cursor = (u8*)flags;
    song = g_akao_seq_channel0;
    if (*(s32*)(descriptor + 0x14) == D_8003EC34[0])
    {
        flags |= 0x40;
    }
    else
    {
        flags |= 0x20;
    }
    song->seq_cursor = (u8*)flags;
    g_akao_seq_channel0->pitch = (s32)descriptor;
    g_akao_seq_channel0->w04.song.key_on_mask = 0;

    g_akao_driver_flags.unk8 |= 0x90;

    mask = g_akao_seq_channel0->w04.song.active_mask;
    delta = (s32)descriptor - D_8004C2FC;
    g_akao_seq_channel0->unk30 += delta;
    g_akao_seq_channel0->flags += delta;
    g_akao_seq_channel0->w04.song.key_on_mask = g_akao_seq_channel0->note_on_mask;

    channel = (AkaoChannelState*)g_akao_seq_channels;
    i = 0x20;
    bit = 1;
    do
    {
        if (mask & bit)
        {
            channel->seq_cursor += delta;
            channel->key_off_mask += delta;
            channel->w04.loop_cursor[0] += delta;
            channel->w04.loop_cursor[1] += delta;
            channel->w04.loop_cursor[2] += delta;
            channel->w04.loop_cursor[3] += delta;
            channel->unk66 += 2;
            channel->unk68 += 2;
            channel->update_flags |= 0x1FF93;
        }
        else
        {
            channel->unk66 = 4;
            channel->unk68 = 2;
            channel->seq_cursor = D_8003D248;
        }
        channel->voice = 0x18;
        i--;
        channel++;
        bit <<= 1;
    } while (i != 0);

    voice_mask = 0;
    if (g_akao_seq_channel1 != NULL)
    {
        voice_mask = akao_collect_voice_mask((AkaoChannelState*)g_akao_pending_channels,
                                              g_akao_seq_channel1->w04.song.active_mask & g_akao_seq_channel1->w04.song.voice_alloc_low_mask);
    }

    g_akao_seq_channel0->key_off_mask = 0;
    D_8004C32E = 0;
    keep_mask = 0xFFFFFF;
    g_akao_sfx_control.unkC |= (~voice_mask & (~(D_8004F76C[0] | g_akao_sfx_control.unk0) & keep_mask));
    g_akao_driver_flags.unk8 |= 0x100;

    if (g_akao_driver_mode_flags & 1)
    {
        mask = g_akao_seq_channel0->w04.song.active_mask;
        g_akao_seq_channel0->w04.song.active_mask = 0;
        g_akao_seq_channel0->unk1C = mask;
    }
}

/**
 * @brief Find which bank program slot (0-4) holds @p key, or 5 if @p key is
 *        the reserved "always resident" sentinel.
 *
 * @param key Bank program key to search for; 0 means "no bank".
 * @return Slot index 0-4 if found among @c g_akao_bank_slot_keys, 5 if
 *         @p key matches the @c D_8004D39C sentinel, otherwise 0.
 * @see decomp.me (100%)
 */
s32 akao_bank_find_slot(s32 key)
{
    s32 result;
    s32 index;
    s32 *slot;
    s32 *base;

    result = 0;
    if (key != 0)
    {
        result = 6;
        slot = &D_8004D39C;
        do
        {
            if (key == *slot)
            {
                result--;
                break;
            }

            result--;
            if (result != 0)
            {
                base = g_akao_bank_slot_keys;
                index = result - 1;
                slot = base + index;
            }
        } while (result != 0);
    }

    return result;
}

/**
 * @brief Dispatch a (re)loaded song descriptor to resume or a fresh start.
 *
 * @p load_result carries the descriptor pointer at offset 0 and an id at
 * offset 8. If the backed-up song state's @c unk5E matches that id, the
 * descriptor is the same song reloaded at a new address, so the suspended
 * state is resumed (akao_seq_resume_song); otherwise it is a genuinely new
 * song (akao_seq_start_song), and the new id is recorded for next time.
 *
 * @param load_result Descriptor load result: u8* descriptor at +0x0, s32 id
 *        at +0x8.
 * @see decomp.me (100%)
 */
void akao_seq_reload_song(u8* load_result)
{
    AkaoChannelState* backup;

    backup = (AkaoChannelState*)D_8004C2D0;
    if (backup->unk5E != 0 && backup->unk5E == *(s32*)(load_result + 8))
    {
        akao_seq_resume_song(*(u8**)load_result);
    }
    else
    {
        akao_seq_start_song(*(u8**)load_result, -1);
        g_akao_seq_channel0->unk5E = *(u16*)(load_result + 8);
    }
}

/**
 * @brief Start a freshly-loaded song descriptor with an explicit channel
 *        mask, and seed the pending tick countdown.
 *
 * @param load_result Descriptor load result: u8* descriptor at +0x0, id at
 *        +0x8, channel mask at +0xC, initial tick count at +0x10.
 * @see decomp.me (100%)
 */
void akao_seq_start_loaded_song(u8* load_result)
{
    s32 result;
    s32 ticks;

    akao_seq_start_song(*(u8**)load_result, *(s32*)(load_result + 0xC));
    g_akao_seq_channel0->unk5E = *(u16*)(load_result + 8);
    ticks = *(s32*)(load_result + 0x10);
    result = 0;
    if (ticks != 0)
    {
        result = ticks - 1;
    }
    g_akao_seq_pending_ticks = result;
}

/**
 * @brief Back up the current song state if a song is active.
 *
 * Counterpart to akao_seq_resume_song: saves @c g_akao_seq_channel0 (0x70
 * bytes) and the full @c g_akao_seq_channels array into the @c D_8004C2D0 /
 * @c D_8004D450 backup slots.
 * @see decomp.me (100%)
 */
void akao_seq_suspend_song(void)
{
    if (g_akao_seq_channel0->w04.song.active_mask != 0)
    {
        akao_copy_bytes((s32*)g_akao_seq_channel0, (s32*)D_8004C2D0, 0x70);
        akao_copy_bytes((s32*)g_akao_seq_channels, (s32*)D_8004D450, 0x2300);
    }
}

/**
 * @brief Start a new primary song, demoting the current one to the
 *        secondary slot first if it is still active and no other secondary
 *        song already holds that slot.
 *
 * When @c g_akao_seq_channel0 is active and @c g_akao_seq_channel1 is either
 * unset or not a genuinely occupied secondary song (its @c unk5E is 0), the
 * current primary state is copied into the @c D_8004C2D0 / @c D_8004D450
 * backup slots and @c g_akao_seq_channel1 / @c g_akao_pending_channels are
 * repointed there, so the old song keeps ticking as the secondary while the
 * new one takes over as primary.
 *
 * @param load_result Descriptor load result: u8* descriptor at +0x0, id at
 *        +0x8.
 * @see decomp.me (100%)
 */
void akao_seq_switch_song(u8* load_result)
{
    if (g_akao_seq_channel0->w04.song.active_mask != 0)
    {
        if (g_akao_seq_channel1 == NULL || g_akao_seq_channel1->unk5E == 0)
        {
            g_akao_seq_channel1 = (AkaoChannelState*)D_8004C2D0;
            g_akao_pending_channels = (s32)D_8004D450;
            akao_copy_bytes((s32*)g_akao_seq_channel0, (s32*)D_8004C2D0, 0x70);
            akao_copy_bytes((s32*)g_akao_seq_channels, (s32*)g_akao_pending_channels, 0x2300);
        }
    }
    akao_seq_start_song(*(u8**)load_result, -1);
    g_akao_seq_channel0->unk5E = *(u16*)(load_result + 8);
}

/**
 * @brief akao_seq_reload_song, then seed the pending tick countdown from
 *        the load result.
 *
 * @param load_result Descriptor load result: passed through to
 *        akao_seq_reload_song; initial tick count at +0x10.
 * @see decomp.me (100%)
 */
void akao_seq_reload_song_with_ticks(u8* load_result)
{
    s32 result;
    s32 ticks;

    akao_seq_reload_song(load_result);
    ticks = *(s32*)(load_result + 0x10);
    result = 0;
    if (ticks != 0)
    {
        result = ticks - 1;
    }
    g_akao_seq_pending_ticks = result;
}

/**
 * @brief Play an SFX with fixed default parameters (reverb_mask 0x400,
 *        tempo_acc 0x1000000, pan_bias byte 0x80, volume_scale 0x7F,
 *        voice_alloc_base 0).
 *
 * @p params doubles as input and output: on entry, offsets +0x0/+0x4 hold
 * the two seq_data pointers to play; this function reads them out first,
 * then overwrites the whole buffer with the default parameter block before
 * calling akao_sfx_play.
 *
 * @param params Buffer holding the two seq_data pointers on entry; rebuilt
 *        in place into an akao_sfx_play parameter block.
 * @see decomp.me (100%)
 */
void akao_sfx_play_default(u8* params)
{
    u8* seq_data0;
    u8* seq_data1;

    seq_data0 = *(u8**)(params + 0x0);
    seq_data1 = *(u8**)(params + 0x4);
    *(s32*)(params + 0x0) = 0x400;
    *(s32*)(params + 0x4) = 0x1000000;
    *(s32*)(params + 0x8) = 0x80;
    *(s32*)(params + 0xC) = 0x7F;
    *(s32*)(params + 0x10) = 0;
    akao_sfx_play(params, seq_data0, seq_data1, 0);
}

/**
 * @brief Play an SFX resolved from a bank program index, tagging the
 *        channel with the bank slot that program resolved to.
 *
 * @p params holds the program index at offset +0x0 on entry.
 * akao_resolve_program_data resolves it to the two seq_data pointers, the
 * program's key is looked up in @c g_akao_bank_region_b to find which bank
 * slot holds it (akao_bank_find_slot), and the fixed tempo_acc/pan_bias/
 * volume_scale fields are filled in like akao_sfx_play_default.
 *
 * @param params Buffer holding the program index on entry; rebuilt in
 *        place into an akao_sfx_play parameter block.
 *
 * @note NOT YET 100% (87.50%, 30/32 exact). The only residue is where one
 *       independent store (the voice_alloc_base write) lands relative to
 *       the final akao_sfx_play call: the target schedules it into that
 *       call's branch delay slot, this compile emits it immediately after
 *       akao_bank_find_slot returns. sched_oracle finds no inferable
 *       constraint here (0 constraints in the block); permuter mutations
 *       that scored better on its own metric did not move this specific
 *       pair when re-measured. See working/func_80027460/code.c.
 * @see decomp.me (87.50%)
 */
void akao_sfx_play_program(u8* params)
{
    s32 seq_data0;
    s32 seq_data1;
    u16 program_key;

    akao_resolve_program_data(&seq_data0, &seq_data1, *(s32*)(params + 0x0));
    *(s32*)(params + 0x4) = 0x2000000;
    *(s32*)(params + 0x8) = 0x80;
    *(s32*)(params + 0xC) = 0x7F;
    program_key = *(u16*)(g_akao_bank_region_b + *(s32*)(params + 0x0) * 2);
    *(s32*)(params + 0x10) = akao_bank_find_slot(program_key);
    akao_sfx_play(params, (u8*)seq_data0, (u8*)seq_data1, 0);
}

/**
 * @brief Play an SFX resolved from a bank program index, tagging the
 *        channel with the bank slot that program resolved to, without
 *        touching the reverb/tempo/pan/volume fields (caller-set).
 *
 * @param params Buffer holding the program index at +0x0 on entry; its
 *        voice_alloc_base (+0x10) is filled in before the call.
 *
 * @note NOT YET 100% (84.62%, 24/26 exact). Same residue and cause as
 *       akao_sfx_play_program: the voice_alloc_base store lands in the
 *       akao_sfx_play call's delay slot in the target but immediately
 *       after akao_bank_find_slot returns here.
 * @see decomp.me (84.62%)
 */
void akao_sfx_play_program_raw(u8* params)
{
    s32 seq_data0;
    s32 seq_data1;
    u16 program_key;

    akao_resolve_program_data(&seq_data0, &seq_data1, *(s32*)(params + 0x0));
    program_key = *(u16*)(g_akao_bank_region_b + *(s32*)(params + 0x0) * 2);
    *(s32*)(params + 0x10) = akao_bank_find_slot(program_key);
    akao_sfx_play(params, (u8*)seq_data0, (u8*)seq_data1, 0);
}

/**
 * @brief Play a list of SFX entries back to back, tagging the channel with
 *        the resolved bank slot and stopping other channels only for the
 *        first entry.
 *
 * @p params holds a pointer to a list header at +0x0: +0x4 the entry
 * count, +0x8 the bank program key (for akao_bank_find_slot), +0x10 an
 * array of s32 offsets into the string/data region starting at +0x20. Each
 * entry resolves two u16 sub-offsets the same way akao_resolve_program_data
 * does (0xFFFF means no data). The first entry plays with skip_stop=0, the
 * rest with skip_stop=1.
 *
 * @param params Buffer holding the list pointer on entry; voice_alloc_base
 *        (+0x10) is filled in before the calls.
 *
 * @note NOT YET 100% (92.89%, 53/76 exact). Residue is a repeated small
 *       register-role swap: the target copies the u16 entry value into its
 *       own register before adding the cursor (`addu v0,v1,zero` then
 *       `addu v0,v0,a0`), this compile folds the add directly. A separate
 *       named temp for the widened entry and swapping the addition operand
 *       order were both measured inert. See working/func_80027548/code.c.
 * @see decomp.me (92.89%)
 */
void akao_sfx_play_list(u8* params)
{
    u8* list;
    u8* base;
    u8* cursor;
    u8* index_ptr;
    s32 count;
    u16 entry;
    u16 sentinel;
    u8* seq_data0;
    u8* seq_data1;

    list = *(u8**)(params + 0x0);
    *(s32*)(params + 0x10) = akao_bank_find_slot(*(s32*)(list + 8));

    list = *(u8**)(params + 0x0);
    index_ptr = list + 0x10;
    base = list + 0x20;
    count = *(s32*)(list + 4);

    cursor = base + *(s32*)index_ptr;
    entry = *(u16*)cursor;
    if (entry != 0xFFFF)
    {
        seq_data0 = cursor + entry + 4;
    }
    else
    {
        seq_data0 = NULL;
    }
    cursor += 2;
    entry = *(u16*)cursor;
    if (entry != 0xFFFF)
    {
        seq_data1 = cursor + entry + 2;
    }
    else
    {
        seq_data1 = NULL;
    }
    akao_sfx_play(params, seq_data0, seq_data1, 0);

    count--;
    if (count != 0)
    {
        sentinel = 0xFFFF;
        index_ptr += 4;
        do
        {
            cursor = base + *(s32*)index_ptr;
            entry = *(u16*)cursor;
            if (entry != sentinel)
            {
                seq_data0 = cursor + entry + 4;
            }
            else
            {
                seq_data0 = NULL;
            }
            cursor += 2;
            entry = *(u16*)cursor;
            if (entry != sentinel)
            {
                seq_data1 = cursor + entry + 2;
            }
            else
            {
                seq_data1 = NULL;
            }
            akao_sfx_play(params, seq_data0, seq_data1, 1);
            count--;
            index_ptr += 4;
        } while (count != 0);
    }
}

/**
 * @brief Stop SFX channels using a (sfx_id, mode) pair read from a buffer.
 * @param params sfx_id at +0x0, mode at +0x4; see akao_sfx_stop_channels.
 * @see decomp.me (100%)
 */
void akao_sfx_stop_channels_from_params(u8* params)
{
    akao_sfx_stop_channels(*(s32*)(params + 0x0), *(s32*)(params + 0x4));
}

/**
 * @brief Set a song's master volume directly, canceling any in-progress
 *        fade, and flag its channels for a volume re-apply.
 *
 * @p params holds a song id at +0x0 (0 always means the primary song) and
 * the new volume (7 bits) at +0x4. Matches against @c g_akao_seq_channel0
 * first, then @c g_akao_seq_channel1, and is a no-op if neither matches.
 *
 * @param params Song id at +0x0, new volume at +0x4.
 *
 * @note NOT YET 100% (95.51%, 45/47 exact). The only residue is the order
 *       of two independent loads (g_akao_pending_channels vs params[4])
 *       right before the secondary-song call; the scheduler reorders them
 *       the same way regardless of source statement order, matching the
 *       sched1/sched2 attribution seen on other functions in this file.
 * @see decomp.me (95.51%)
 */
void akao_seq_set_master_volume(u8* params)
{
    s32 id;
    s32 volume;

    id = *(s32*)(params + 0x0);
    if (id == 0 || id == g_akao_seq_channel0->unk5E)
    {
        volume = (*(s32*)(params + 4) & 0x7F) << 16;
        g_akao_seq_channel0->pitch_slide_step = volume;
        g_akao_seq_channel0->unk58 = 0;
        akao_seq_flag_volume_update(g_akao_seq_channel0, (AkaoChannelState*)g_akao_seq_channels);
    }
    else if (g_akao_seq_channel1 != NULL && id != 0 && id == g_akao_seq_channel1->unk5E)
    {
        volume = *(s32*)(params + 4);
        g_akao_seq_channel1->unk58 = 0;
        volume = (volume & 0x7F) << 16;
        g_akao_seq_channel1->pitch_slide_step = volume;
        akao_seq_flag_volume_update(g_akao_seq_channel1, (AkaoChannelState*)g_akao_pending_channels);
    }
}

/**
 * @brief Start a linear fade of a song's master volume to a target level
 *        over a given tick count, and flag its channels for a volume
 *        re-apply.
 *
 * @p params holds a song id at +0x0 (0 always means the primary song), a
 * tick count at +0x4 (0 is treated as 1), and the target volume (7 bits)
 * at +0x8. The per-tick step is computed as (target - current) / ticks and
 * stored at detune_pitch_delta; unk58 is set to the tick count so the
 * per-tick fade in akao_tick_channel_effects picks it up.
 *
 * @param params Song id at +0x0, tick count at +0x4, target volume at +0x8.
 * @see decomp.me (100%)
 */
void akao_seq_fade_master_volume(u8* params)
{
    s32 id;
    s32 raw_ticks;
    s32 ticks;
    s32 target_volume;
    s32 current_volume;
    s32 step;
    AkaoChannelState* channels;

    raw_ticks = *(s32*)(params + 4);
    ticks = 1;
    if (raw_ticks != 0)
    {
        ticks = raw_ticks;
    }
    target_volume = (*(s32*)(params + 8) & 0x7F) << 16;
    id = *(s32*)(params + 0x0);
    if (id == 0 || id == g_akao_seq_channel0->unk5E)
    {
        current_volume = g_akao_seq_channel0->pitch_slide_step;
        target_volume -= current_volume;
        step = target_volume / ticks;
        channels = (AkaoChannelState*)g_akao_seq_channels;
        g_akao_seq_channel0->unk58 = ticks;
        g_akao_seq_channel0->detune_pitch_delta = step;
        akao_seq_flag_volume_update(g_akao_seq_channel0, channels);
    }
    else if (g_akao_seq_channel1 != NULL && id != 0 && id == g_akao_seq_channel1->unk5E)
    {
        current_volume = g_akao_seq_channel1->pitch_slide_step;
        target_volume -= current_volume;
        step = target_volume / ticks;
        channels = (AkaoChannelState*)g_akao_pending_channels;
        g_akao_seq_channel1->unk58 = ticks;
        g_akao_seq_channel1->detune_pitch_delta = step;
        akao_seq_flag_volume_update(g_akao_seq_channel1, channels);
    }
}

/**
 * @brief Jump a song's master volume to an explicit start level, then fade
 *        it to a target level over a given tick count.
 *
 * Same song/tick resolution as akao_seq_fade_master_volume, but the start
 * volume comes from @p params +0x8 (written immediately) instead of the
 * song's current volume, and the target comes from +0xC.
 *
 * @param params Song id at +0x0, tick count at +0x4, start volume at +0x8,
 *        target volume at +0xC.
 * @see decomp.me (100%)
 */
void akao_seq_fade_master_volume_from(u8* params)
{
    s32 id;
    s32 raw_ticks;
    s32 ticks;
    s32 start_volume;
    s32 target_volume;
    s32 step;
    AkaoChannelState* channels;
    AkaoChannelState* song;

    raw_ticks = *(s32*)(params + 4);
    ticks = 1;
    if (raw_ticks != 0)
    {
        ticks = raw_ticks;
    }
    id = *(s32*)(params + 0x0);
    if (id == 0 || id == g_akao_seq_channel0->unk5E)
    {
        song = g_akao_seq_channel0;
        channels = (AkaoChannelState*)g_akao_seq_channels;
    }
    else if (g_akao_seq_channel1 != NULL && id != 0 && id == g_akao_seq_channel1->unk5E)
    {
        song = g_akao_seq_channel1;
        channels = (AkaoChannelState*)g_akao_pending_channels;
    }
    else
    {
        return;
    }
    start_volume = (*(s32*)(params + 8) & 0x7F) << 16;
    song->pitch_slide_step = start_volume;
    target_volume = (*(s32*)(params + 0xC) & 0x7F) << 16;
    target_volume -= start_volume;
    step = target_volume / ticks;
    song->unk58 = ticks;
    song->detune_pitch_delta = step;
    akao_seq_flag_volume_update(song, channels);
}

/**
 * @brief Set the CD-audio volume accumulator directly, canceling any
 *        in-progress fade, and push it to the SPU immediately.
 * @param params Target volume (u16, shifted into the high half) at +0x0.
 * @see decomp.me (100%)
 */
void akao_set_cd_volume(u8* params)
{
    s32 volume;

    volume = *(u16*)(params + 0x0);
    g_akao_cdvol_fade_ticks = 0;
    volume = volume << 16;
    g_akao_cdvol_acc = volume;
    akao_apply_cdvol_to_spu();
}

/**
 * @brief Start a linear fade of the CD-audio volume accumulator to a
 *        target level over a given tick count.
 *
 * @param params Tick count at +0x0 (0 is treated as 1), target volume
 *        (u16, shifted into the high half like the master-volume fades)
 *        at +0x4.
 * @see decomp.me (100%)
 */
void akao_fade_cd_volume(u8* params)
{
    s32 raw_ticks;
    s32 ticks;
    s32 target;
    s32 step;

    raw_ticks = *(s32*)(params + 0x0);
    ticks = 1;
    if (raw_ticks != 0)
    {
        ticks = raw_ticks;
    }
    target = *(u16*)(params + 4);
    target = target << 16;
    target -= g_akao_cdvol_acc;
    step = target / ticks;
    g_akao_cdvol_fade_ticks = ticks;
    g_akao_cdvol_step = step;
}

/**
 * @brief Jump the CD-audio volume accumulator to an explicit start level,
 *        then fade it to a target level over a given tick count.
 * @param params Tick count at +0x0, start volume (u16) at +0x4, target
 *        volume (u16) at +0x8.
 * @see decomp.me (100%)
 */
void akao_fade_cd_volume_from(u8* params)
{
    s32 raw_ticks;
    s32 ticks;
    s32 target;
    s32 start;
    s32 step;

    raw_ticks = *(s32*)(params + 0x0);
    ticks = 1;
    if (raw_ticks != 0)
    {
        ticks = raw_ticks;
    }
    target = *(u16*)(params + 8);
    start = *(u16*)(params + 4);
    target = target << 16;
    start = start << 16;
    target -= start;
    step = target / ticks;
    g_akao_cdvol_fade_ticks = ticks;
    g_akao_cdvol_acc = start;
    g_akao_cdvol_step = step;
}

/**
 * @brief Apply a new volume scale to active SFX channels, selected either
 *        by a tempo_acc mode mask or by an exact sfx id match.
 *
 * When @p params +0x4 is non-zero, it is used as a bitmask tested against
 * each active channel's tempo_acc (secondary flag word). Otherwise, each
 * active channel's reverb_mask (which doubles as the sfx id tag for SFX
 * channels; see akao_sfx_stop_channels) is compared against @p params +0x0.
 *
 * @param params sfx id at +0x0, tempo_acc mode mask at +0x4 (0 selects the
 *        id-match mode instead), new volume scale (7 bits) at +0x8.
 * @see decomp.me (100%)
 */
void akao_sfx_set_volume_scale(u8* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s32 scale;

    channel = (AkaoChannelState*)g_sfx_channels;
    active = g_akao_sfx_control.unk0;
    bit = 0x1000;
    if (*(s32*)(params + 4) != 0)
    {
        for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && (channel->tempo_acc & *(s32*)(params + 4)))
            {
                scale = *(u16*)(params + 8);
                channel->unk8E = 0;
                channel->volume_scale = (scale & 0x7F) << 8;
                channel->update_flags |= 3;
            }
        }
    }
    else
    {
        bit = 0x1000;
        for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && ((s32)channel->reverb_mask == *(s32*)(params + 0)))
            {
                scale = *(u16*)(params + 8);
                channel->unk8E = 0;
                channel->volume_scale = (scale & 0x7F) << 8;
                channel->update_flags |= 3;
            }
        }
    }
}

/**
 * @brief Start a fade of the volume scale on active SFX channels, selected
 *        either by a tempo_acc mode mask or by an exact sfx id match (same
 *        selection rule as akao_sfx_set_volume_scale).
 *
 * @param params sfx id at +0x0, tempo_acc mode mask at +0x4 (0 selects the
 *        id-match mode instead), tick count at +0x8 (0 is treated as 1),
 *        target volume scale (7 bits) at +0xC.
 *
 * @note NOT YET 100% (99.90%, 86/96 exact). The only remaining residue is
 *       which struct offset the compiler anchors the per-channel loop
 *       cursor at (0x8E in the target, 0xE6 here); every other instruction
 *       matches. Introducing an explicit raw-offset cursor variable made
 *       this measurably worse (94.24%), so the natural struct-access form
 *       above is the better basin. See working/func_80027B88/code.c.
 * @see decomp.me (99.90%)
 */
void akao_sfx_fade_volume_scale(u8* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s16 tick_count;
    u16 target_scale;
    s32 current_scale;
    s16 delta;

    channel = (AkaoChannelState*)g_sfx_channels;
    active = g_akao_sfx_control.unk0;
    bit = 0x1000;
    if (*(s32*)(params + 4) != 0)
    {
        for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && (channel->tempo_acc & *(s32*)(params + 4)))
            {
                if (*(s32*)(params + 8) != 0)
                {
                    tick_count = *(u16*)(params + 8);
                }
                else
                {
                    tick_count = 1;
                }
                target_scale = (*(u16*)(params + 0xC) & 0x7F) << 8;
                current_scale = channel->volume_scale;
                delta = target_scale - current_scale;
                channel->unk8E = tick_count;
                channel->unkE6 = delta / tick_count;
            }
        }
    }
    else
    {
        bit = 0x1000;
        for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && ((s32)channel->reverb_mask == *(s32*)(params + 0)))
            {
                if (*(s32*)(params + 8) != 0)
                {
                    tick_count = *(u16*)(params + 8);
                }
                else
                {
                    tick_count = 1;
                }
                target_scale = (*(u16*)(params + 0xC) & 0x7F) << 8;
                current_scale = channel->volume_scale;
                delta = target_scale - current_scale;
                channel->unk8E = tick_count;
                channel->unkE6 = delta / tick_count;
            }
        }
    }
}

/**
 * @brief Apply a new volume scale to every active SFX channel whose
 *        tempo_acc does not have the pan/volume-suppress bit (0x02000000)
 *        set.
 * @param param New volume scale (7 bits, u16) to apply.
 * @see decomp.me (100%)
 */
void akao_sfx_set_volume_scale_unsuppressed(u16* param)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s32 scale;

    bit = 0x1000;
    active = g_akao_sfx_control.unk0;
    channel = (AkaoChannelState*)g_sfx_channels;
    for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
    {
        if ((active & bit) && !(channel->tempo_acc & 0x02000000))
        {
            scale = *param;
            channel->unk8E = 0;
            channel->volume_scale = (scale & 0x7F) << 8;
            channel->update_flags |= 3;
        }
    }
}

/**
 * @brief Start a fade of the volume scale on every active SFX channel
 *        whose tempo_acc does not have the suppress bit (0x02000000) set.
 * @param params Tick count at +0x0 (0 is treated as 1), target volume
 *        scale (7 bits, u16) at +0x4.
 *
 * @note NOT YET 100% (99.90%, 45/50 exact). Same loop-cursor anchor
 *       residue as akao_sfx_fade_volume_scale (0x8E vs 0xE6); every other
 *       instruction matches.
 * @see decomp.me (99.90%)
 */
void akao_sfx_fade_volume_scale_unsuppressed(u8* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s16 tick_count;
    u16 target_scale;
    s32 current_scale;
    s16 delta;

    bit = 0x1000;
    active = g_akao_sfx_control.unk0;
    channel = (AkaoChannelState*)g_sfx_channels;
    for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
    {
        if ((active & bit) && !(channel->tempo_acc & 0x02000000))
        {
            if (*(s32*)(params + 0) != 0)
            {
                tick_count = *(u16*)(params + 0);
            }
            else
            {
                tick_count = 1;
            }
            target_scale = (*(u16*)(params + 4) & 0x7F) << 8;
            current_scale = channel->volume_scale;
            delta = target_scale - current_scale;
            channel->unk8E = tick_count;
            channel->unkE6 = delta / tick_count;
        }
    }
}

/**
 * @brief Set the pan bias on active SFX channels, selected either by a
 *        tempo_acc mode mask or by an exact sfx id match (same selection
 *        rule as akao_sfx_set_volume_scale), and cancel any pan-bias fade.
 * @param params sfx id at +0x0, tempo_acc mode mask at +0x4 (0 selects the
 *        id-match mode instead), new pan bias (u8) at +0x8.
 * @see decomp.me (100%)
 */
void akao_sfx_set_pan_bias(u8* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s32 bias;

    channel = (AkaoChannelState*)g_sfx_channels;
    active = g_akao_sfx_control.unk0;
    bit = 0x1000;
    if (*(s32*)(params + 4) != 0)
    {
        for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && (channel->tempo_acc & *(s32*)(params + 4)))
            {
                bias = *(u8*)(params + 8);
                channel->pan_bias_fade_ticks = 0;
                channel->pan_bias = bias << 8;
                channel->update_flags |= 3;
            }
        }
    }
    else
    {
        bit = 0x1000;
        for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && ((s32)channel->reverb_mask == *(s32*)(params + 0)))
            {
                bias = *(u8*)(params + 8);
                channel->pan_bias_fade_ticks = 0;
                channel->pan_bias = bias << 8;
                channel->update_flags |= 3;
            }
        }
    }
}

/**
 * @brief Start a fade of the pan bias on active SFX channels, selected
 *        either by a tempo_acc mode mask or by an exact sfx id match (same
 *        selection rule as akao_sfx_set_volume_scale).
 * @param params sfx id at +0x0, tempo_acc mode mask at +0x4 (0 selects the
 *        id-match mode instead), tick count at +0x8 (0 is treated as 1),
 *        target pan bias (u8) at +0xC.
 *
 * @note NOT YET 100% (99.89%, 84/94 exact). Same loop-cursor anchor
 *       residue as akao_sfx_fade_volume_scale: the target anchors the
 *       per-channel cursor at pan_bias_fade_ticks (+0x70), this compile
 *       anchors it at pan_bias_step (+0xE2) instead. This is the third
 *       function in this file with the exact same symptom (see also
 *       akao_sfx_fade_volume_scale, akao_sfx_fade_volume_scale_unsuppressed) -
 *       a recurring GCC 2.8 loop-cursor choice when a fade writes one field
 *       near the filter test and a second field far from it.
 * @see decomp.me (99.89%)
 */
void akao_sfx_fade_pan_bias(u8* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s16 tick_count;
    u16 target_bias;
    s32 current_bias;
    s16 delta;

    channel = (AkaoChannelState*)g_sfx_channels;
    active = g_akao_sfx_control.unk0;
    bit = 0x1000;
    if (*(s32*)(params + 4) != 0)
    {
        for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && (channel->tempo_acc & *(s32*)(params + 4)))
            {
                if (*(s32*)(params + 8) != 0)
                {
                    tick_count = *(u16*)(params + 8);
                }
                else
                {
                    tick_count = 1;
                }
                target_bias = *(u8*)(params + 0xC) << 8;
                current_bias = channel->pan_bias;
                delta = target_bias - current_bias;
                channel->pan_bias_fade_ticks = tick_count;
                channel->pan_bias_step = delta / tick_count;
            }
        }
    }
    else
    {
        bit = 0x1000;
        for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && ((s32)channel->reverb_mask == *(s32*)(params + 0)))
            {
                if (*(s32*)(params + 8) != 0)
                {
                    tick_count = *(u16*)(params + 8);
                }
                else
                {
                    tick_count = 1;
                }
                target_bias = *(u8*)(params + 0xC) << 8;
                current_bias = channel->pan_bias;
                delta = target_bias - current_bias;
                channel->pan_bias_fade_ticks = tick_count;
                channel->pan_bias_step = delta / tick_count;
            }
        }
    }
}

/**
 * @brief Set the pan bias on every active SFX channel whose tempo_acc does
 *        not have the suppress bit (0x02000000) set, and cancel any
 *        pan-bias fade.
 * @param param New pan bias (u8) to apply.
 * @see decomp.me (100%)
 */
void akao_sfx_set_pan_bias_unsuppressed(u8* param)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s32 bias;

    bit = 0x1000;
    active = g_akao_sfx_control.unk0;
    channel = (AkaoChannelState*)g_sfx_channels;
    for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
    {
        if ((active & bit) && !(channel->tempo_acc & 0x02000000))
        {
            bias = *param;
            channel->pan_bias_fade_ticks = 0;
            channel->pan_bias = bias << 8;
            channel->update_flags |= 3;
        }
    }
}

/**
 * @brief Start a fade of the pan bias on every active SFX channel whose
 *        tempo_acc does not have the suppress bit (0x02000000) set.
 * @param params Tick count at +0x0 (0 is treated as 1), target pan bias
 *        (u8) at +0x4.
 *
 * @note NOT YET 100% (99.90%, 44/49 exact). Same loop-cursor anchor
 *       residue as akao_sfx_fade_pan_bias (0x70 vs 0xE2).
 * @see decomp.me (99.90%)
 */
void akao_sfx_fade_pan_bias_unsuppressed(u8* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s16 tick_count;
    u16 target_bias;
    s32 current_bias;
    s16 delta;

    bit = 0x1000;
    active = g_akao_sfx_control.unk0;
    channel = (AkaoChannelState*)g_sfx_channels;
    for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
    {
        if ((active & bit) && !(channel->tempo_acc & 0x02000000))
        {
            if (*(s32*)(params + 0) != 0)
            {
                tick_count = *(u16*)(params + 0);
            }
            else
            {
                tick_count = 1;
            }
            target_bias = *(u8*)(params + 4) << 8;
            current_bias = channel->pan_bias;
            delta = target_bias - current_bias;
            channel->pan_bias_fade_ticks = tick_count;
            channel->pan_bias_step = delta / tick_count;
        }
    }
}

/**
 * @brief Set a per-channel value at offset +0x40 (channel-role meaning not
 *        yet identified; +0x40 is only documented for the song role, as
 *        noise_mask) on active SFX channels, selected either by a
 *        tempo_acc mode mask or by an exact sfx id match (same selection
 *        rule as akao_sfx_set_volume_scale), and flag a pending pitch
 *        update (update_flags bit 0x10).
 * @param params sfx id at +0x0, tempo_acc mode mask at +0x4 (0 selects the
 *        id-match mode instead), new value (u8, shifted into the high
 *        byte) at +0x8.
 * @see decomp.me (100%)
 */
void akao_sfx_set_pitch_bend(u8* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s32 value;

    channel = (AkaoChannelState*)g_sfx_channels;
    active = g_akao_sfx_control.unk0;
    bit = 0x1000;
    if (*(s32*)(params + 4) != 0)
    {
        for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && (channel->tempo_acc & *(s32*)(params + 4)))
            {
                value = *(u8*)(params + 8);
                channel->unk88 = 0;
                *(s32*)((u8*)channel + 0x40) = value << 8;
                channel->update_flags |= 0x10;
            }
        }
    }
    else
    {
        bit = 0x1000;
        for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && ((s32)channel->reverb_mask == *(s32*)(params + 0)))
            {
                value = *(u8*)(params + 8);
                channel->unk88 = 0;
                *(s32*)((u8*)channel + 0x40) = value << 8;
                channel->update_flags |= 0x10;
            }
        }
    }
}

/**
 * @brief Start a fade of the same +0x40 per-channel value that
 *        akao_sfx_set_pitch_bend sets, on active SFX channels selected
 *        either by a tempo_acc mode mask or by an exact sfx id match.
 * @param params sfx id at +0x0, tempo_acc mode mask at +0x4 (0 selects the
 *        id-match mode instead), tick count at +0x8 (0 is treated as 1),
 *        target value (u8) at +0xC.
 *
 * @note NOT YET 100% (99.90%, 88/98 exact). Same recurring loop-cursor
 *       anchor residue as akao_sfx_fade_volume_scale and akao_sfx_fade_pan_bias
 *       (target anchors at unk88 / +0x88, this compile anchors at +0x44).
 * @see decomp.me (99.90%)
 */
void akao_sfx_fade_pitch_bend(u8* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s16 tick_count;
    u16 target;
    s32 current;
    s16 delta;
    s16 step;

    channel = (AkaoChannelState*)g_sfx_channels;
    active = g_akao_sfx_control.unk0;
    bit = 0x1000;
    if (*(s32*)(params + 4) != 0)
    {
        for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && (channel->tempo_acc & *(s32*)(params + 4)))
            {
                if (*(s32*)(params + 8) != 0)
                {
                    tick_count = *(u16*)(params + 8);
                }
                else
                {
                    tick_count = 1;
                }
                target = *(u8*)(params + 0xC) << 8;
                current = *(u16*)((u8*)channel + 0x40);
                delta = target - current;
                step = delta / tick_count;
                channel->unk88 = tick_count;
                *(s32*)((u8*)channel + 0x44) = step;
            }
        }
    }
    else
    {
        bit = 0x1000;
        for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && ((s32)channel->reverb_mask == *(s32*)(params + 0)))
            {
                if (*(s32*)(params + 8) != 0)
                {
                    tick_count = *(u16*)(params + 8);
                }
                else
                {
                    tick_count = 1;
                }
                target = *(u8*)(params + 0xC) << 8;
                current = *(u16*)((u8*)channel + 0x40);
                delta = target - current;
                step = delta / tick_count;
                channel->unk88 = tick_count;
                *(s32*)((u8*)channel + 0x44) = step;
            }
        }
    }
}

/**
 * @brief Set the same +0x40 per-channel value that akao_sfx_set_pitch_bend
 *        sets, on every SFX channel whose tempo_acc does not have the
 *        suppress bit (0x02000000) set (no active-channel filter here).
 * @param param New value (u8, shifted into the high byte) to apply.
 * @see decomp.me (100%)
 */
void akao_sfx_set_pitch_bend_unsuppressed(u8* param)
{
    AkaoChannelState* channel;
    u32 count;
    s32 value;

    channel = (AkaoChannelState*)g_sfx_channels;
    for (count = 0xC; count != 0; count--, channel++)
    {
        if (!(channel->tempo_acc & 0x02000000))
        {
            value = *param;
            channel->unk88 = 0;
            *(s32*)((u8*)channel + 0x40) = value << 8;
            channel->update_flags |= 0x10;
        }
    }
}

/**
 * @brief Start a fade of the same +0x40 per-channel value that
 *        akao_sfx_set_pitch_bend sets, on every active SFX channel whose
 *        tempo_acc does not have the suppress bit (0x02000000) set.
 * @param params Tick count at +0x0 (0 is treated as 1), target value (u8)
 *        at +0x4.
 *
 * @see decomp.me (100%)
 */
void akao_sfx_fade_pitch_bend_unsuppressed(u8* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s16 tick_count;
    u16 target;
    s32 current;
    s16 delta;
    s16 step;

    bit = 0x1000;
    active = g_akao_sfx_control.unk0;
    channel = (AkaoChannelState*)g_sfx_channels;
    for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
    {
        if ((active & bit) && !(channel->tempo_acc & 0x02000000))
        {
            if (*(s32*)(params + 0) != 0)
            {
                tick_count = *(u16*)(params + 0);
            }
            else
            {
                tick_count = 1;
            }
            target = *(u8*)(params + 4) << 8;
            current = *(u16*)((u8*)channel + 0x40);
            delta = target - current;
            step = delta / tick_count;
            *(s32*)((u8*)channel + 0x44) = step;
            channel->unk88 = tick_count;
        }
    }
}

/**
 * @brief Set the master pan accumulator directly, canceling any
 *        in-progress fade.
 * @param param Signed target pan value (byte, shifted into the high half).
 * @see decomp.me (100%)
 */
void akao_set_master_pan(s8* param)
{
    s32 pan;

    pan = *param;
    g_akao_masterpan_fade_ticks = 0;
    pan = pan << 16;
    g_akao_masterpan_acc = pan;
}

/**
 * @brief Start a linear fade of the master pan accumulator to a target
 *        level over a given tick count.
 * @param params Tick count at +0x0 (0 is treated as 1), signed target pan
 *        (byte) at +0x4.
 * @see decomp.me (100%)
 */
void akao_fade_master_pan(u8* params)
{
    s32 raw_ticks;
    s32 ticks;
    s32 target;
    s32 step;

    raw_ticks = *(s32*)(params + 0x0);
    ticks = 1;
    if (raw_ticks != 0)
    {
        ticks = raw_ticks;
    }
    target = *(s8*)(params + 4);
    target = target << 16;
    target -= g_akao_masterpan_acc;
    step = target / ticks;
    g_akao_masterpan_fade_ticks = ticks;
    g_akao_masterpan_step = step;
}

/**
 * @brief Jump the master pan accumulator to an explicit start level, then
 *        fade it to a target level over a given tick count.
 *
 * Unlike the other fade functions in this file, the tick-count value and
 * its zero-check read different offsets: the check reads +0x4 (the same
 * word as the start pan, tested as an s32 before being reread as a
 * signed byte), and the actual tick count comes from +0x0.
 *
 * @param params Tick count at +0x0, signed start pan (byte) at +0x4,
 *        signed target pan (byte) at +0x8.
 * @see decomp.me (100%)
 */
void akao_fade_master_pan_from(u8* params)
{
    s32 raw_ticks;
    s32 ticks;
    s32 target;
    s32 start;
    s32 step;

    raw_ticks = *(s32*)(params + 0x4);
    ticks = 1;
    if (raw_ticks != 0)
    {
        ticks = *(s32*)(params + 0x0);
    }
    start = *(s8*)(params + 4);
    start = start << 16;
    g_akao_masterpan_acc = start;
    target = *(s8*)(params + 8);
    target = target << 16;
    target -= start;
    step = target / ticks;
    g_akao_masterpan_fade_ticks = ticks;
    g_akao_masterpan_step = step;
}

/**
 * @brief Set the driver-wide master volume accumulator directly,
 *        canceling any in-progress fade.
 * @param param Signed target volume (byte, shifted into the high half).
 * @see decomp.me (100%)
 */
void akao_set_driver_master_volume(s8* param)
{
    s32 volume;

    volume = *param;
    g_akao_mastervol_fade_ticks = 0;
    volume = volume << 16;
    g_akao_mastervol_acc = volume;
}

/**
 * @brief Start a linear fade of the driver-wide master volume accumulator
 *        to a target level over a given tick count.
 * @param params Tick count at +0x0 (0 is treated as 1), signed target
 *        volume (byte) at +0x4.
 * @see decomp.me (100%)
 */
void akao_fade_driver_master_volume(u8* params)
{
    s32 raw_ticks;
    s32 ticks;
    s32 target;
    s32 step;

    raw_ticks = *(s32*)(params + 0x0);
    ticks = 1;
    if (raw_ticks != 0)
    {
        ticks = raw_ticks;
    }
    target = *(s8*)(params + 4);
    target = target << 16;
    target -= g_akao_mastervol_acc;
    step = target / ticks;
    g_akao_mastervol_fade_ticks = ticks;
    g_akao_mastervol_step = step;
}

/**
 * @brief Jump the driver-wide master volume accumulator to an explicit
 *        start level, then fade it to a target level over a given tick
 *        count.
 *
 * Same offset quirk as akao_fade_master_pan_from: the tick-count zero
 * check reads +0x4 (the same word as the start volume), the actual tick
 * count comes from +0x0.
 *
 * @param params Tick count at +0x0, signed start volume (byte) at +0x4,
 *        signed target volume (byte) at +0x8.
 * @see decomp.me (100%)
 */
void akao_fade_driver_master_volume_from(u8* params)
{
    s32 raw_ticks;
    s32 ticks;
    s32 target;
    s32 start;
    s32 step;

    raw_ticks = *(s32*)(params + 0x4);
    ticks = 1;
    if (raw_ticks != 0)
    {
        ticks = *(s32*)(params + 0x0);
    }
    start = *(s8*)(params + 4);
    start = start << 16;
    g_akao_mastervol_acc = start;
    target = *(s8*)(params + 8);
    target = target << 16;
    target -= start;
    step = target / ticks;
    g_akao_mastervol_fade_ticks = ticks;
    g_akao_mastervol_step = step;
}

/**
 * @brief Unconditionally stop the primary song, and the secondary song
 *        too if one is loaded.
 * @see decomp.me (100%)
 */
void akao_seq_stop_all_songs(void)
{
    akao_seq_stop_song(g_akao_seq_channel0, (AkaoChannelState*)g_akao_seq_channels, 0);
    if (g_akao_seq_channel1 != NULL)
    {
        akao_seq_stop_song(g_akao_seq_channel1, (AkaoChannelState*)g_akao_pending_channels, 0);
    }
}

/**
 * @brief Stop the primary song by key, and the secondary song too if one
 *        is loaded and the key is nonzero.
 * @param params Song key at +0x0; see akao_seq_stop_song.
 * @see decomp.me (100%)
 */
void akao_seq_stop_song_by_key(u8* params)
{
    s32 song_key;

    akao_seq_stop_song(g_akao_seq_channel0, (AkaoChannelState*)g_akao_seq_channels, *(s32*)(params + 0x0));
    if (g_akao_seq_channel1 != NULL)
    {
        song_key = *(s32*)(params + 0x0);
        if (song_key != 0)
        {
            akao_seq_stop_song(g_akao_seq_channel1, (AkaoChannelState*)g_akao_pending_channels, song_key);
        }
    }
}

/**
 * @brief Release every active, non-suppressed SFX channel and clear its
 *        flags word.
 * @see decomp.me (100%)
 */
void akao_sfx_release_all_channels(void)
{
    AkaoChannelState* channel;
    s32 bit;
    u32 count;

    channel = (AkaoChannelState*)g_sfx_channels;
    bit = 0x1000;
    for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
    {
        if ((g_akao_sfx_control.unk0 & bit) && !(channel->tempo_acc & 0x02000000))
        {
            g_akao_sfx_control.unkC |= bit;
            akao_sfx_release_channels(channel, bit);
            channel->flags = 0;
        }
    }
    g_akao_driver_flags.unk8 |= 0x110;
}

/**
 * @brief Flag every channel of every active song and every active SFX
 *        channel for a pending SPU volume re-apply.
 * @see decomp.me (100%)
 */
void akao_flag_all_volume_updates(void)
{
    D_8004F754[0] = 1;
    akao_seq_flag_volume_update(g_akao_seq_channel0, (AkaoChannelState*)g_akao_seq_channels);
    if (g_akao_seq_channel1 != NULL)
    {
        akao_seq_flag_volume_update(g_akao_seq_channel1, (AkaoChannelState*)g_akao_pending_channels);
    }
    akao_sfx_flag_volume_update();
}

/**
 * @brief Flag every channel of every active song and every active SFX
 *        channel for a pending SPU update, tagged mode 2 (see
 *        akao_flag_all_volume_updates, tagged mode 1).
 * @see decomp.me (100%)
 */
void akao_flag_all_pan_updates(void)
{
    D_8004F754[0] = 2;
    akao_seq_flag_volume_update(g_akao_seq_channel0, (AkaoChannelState*)g_akao_seq_channels);
    if (g_akao_seq_channel1 != NULL)
    {
        akao_seq_flag_volume_update(g_akao_seq_channel1, (AkaoChannelState*)g_akao_pending_channels);
    }
    akao_sfx_flag_volume_update();
}

/**
 * @brief Store a new value into D_8003EC6C, then flag every primary song
 *        channel for a pending SPU update.
 * @param param New value for D_8003EC6C.
 * @see decomp.me (100%)
 */
void akao_set_mode_flag_and_flag_all_channels(s32* param)
{
    AkaoChannelState* channel;
    u32 count;

    D_8003EC6C = *param;
    channel = (AkaoChannelState*)g_akao_seq_channels;
    count = 0;
    do
    {
        count++;
        channel->update_flags |= 3;
        channel++;
    } while (count < 0x20);
}

/**
 * @see decomp.me (100%)
 */
void akao_seq_set_unk60(u16* param)
{
    g_akao_seq_channel0->unk60 = *param;
}

/**
 * @brief Silence every SPU voice not currently claimed by an SFX channel
 *        or the XA/streaming reservation, park the primary song's
 *        active_mask into unk1C (pausing it without releasing voices),
 *        and set the driver "paused" mode bit.
 * @see decomp.me (100%)
 */
void akao_seq_silence_unused_voices_and_pause(void)
{
    s32 keep_mask;
    s32 bit;
    s32 voice;
    s32 active_mask;
    s32 mode;

    if (g_akao_seq_channel0->w04.song.active_mask != 0)
    {
        keep_mask = ~(g_akao_sfx_control.unk0 | D_8004F76C[0]) & 0xFFFFFF;
        if (keep_mask != 0)
        {
            bit = 1;
            voice = 0;
            do
            {
                if (keep_mask & bit)
                {
                    spu_set_voice_volume(voice, 0, 0, 0);
                    spu_set_voice_pitch(voice, 0);
                    spu_set_voice_attack(voice, 0x7F, 1);
                    spu_set_voice_sustain_mode(voice, 0x7F, 3);
                    keep_mask &= ~bit;
                }
                bit <<= 1;
                voice++;
            } while (keep_mask != 0);
        }
        active_mask = g_akao_seq_channel0->w04.song.active_mask;
        g_akao_seq_channel0->w04.song.active_mask = 0;
        g_akao_seq_channel0->unk1C = active_mask;
    }
    mode = g_akao_driver_mode_flags;
    mode |= 1;
    g_akao_driver_mode_flags = mode;
}

/**
 * @brief Resume the primary song from the paused state set by
 *        akao_seq_silence_unused_voices_and_pause: restore active_mask
 *        from unk1C, flag every channel that was parked for a full SPU
 *        re-apply, and clear the driver "paused" mode bit.
 * @see decomp.me (100%)
 */
void akao_seq_resume_and_apply_pending_voices(void)
{
    s32 raw_mask;
    s32 mask;
    s32 bit;
    AkaoChannelState* channel;
    s32 saved_mask;
    s32 mode;

    raw_mask = g_akao_seq_channel0->unk1C;
    if (raw_mask != 0)
    {
        mask = raw_mask;
        bit = 1;
        channel = (AkaoChannelState*)g_akao_seq_channels;
        do
        {
            if (mask & bit)
            {
                mask &= ~bit;
                channel->update_flags |= 0x2B13;
            }
            bit <<= 1;
            channel++;
        } while (mask != 0);

        saved_mask = g_akao_seq_channel0->unk1C;
        g_akao_seq_channel0->unk1C = 0;
        g_akao_seq_channel0->w04.song.active_mask = saved_mask;
        g_akao_driver_flags.unk8 |= 0x100;
    }
    mode = g_akao_driver_mode_flags;
    mode &= ~1;
    g_akao_driver_mode_flags = mode;
}

/**
 * @brief SFX counterpart of akao_seq_silence_unused_voices_and_pause:
 *        filters the active SFX channel mask down to non-suppressed
 *        channels, parks it in g_akao_sfx_control.unk10, clears those bits
 *        from unk0, silences their SPU voices, and sets the driver
 *        "SFX paused" mode bit (0x2).
 * @see decomp.me (100%)
 */
void akao_sfx_silence_unused_voices_and_pause(void)
{
    AkaoChannelState* channel;
    s32 raw_active;
    s32 voice;
    s32 active;
    s32 bit;
    u32 count;
    s32 mode;

    raw_active = g_akao_sfx_control.unk0;
    if (raw_active != 0)
    {
        active = raw_active;
        channel = (AkaoChannelState*)g_sfx_channels;
        bit = 0x1000;
        for (count = 0; count < 0xC; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && (channel->tempo_acc & 0x02000000))
            {
                active &= ~bit;
            }
        }
        bit = 0x1000;
        voice = 0xC;
        g_akao_sfx_control.unk10 = active;
        g_akao_sfx_control.unk0 &= ~active;
        if (active != 0)
        {
            do
            {
                if (active & bit)
                {
                    spu_set_voice_volume(voice, 0, 0, 0);
                    spu_set_voice_pitch(voice, 0);
                    spu_set_voice_attack(voice, 0x7F, 1);
                    spu_set_voice_sustain_mode(voice, 0x7F, 3);
                    active &= ~bit;
                }
                bit <<= 1;
                voice++;
            } while (active != 0);
        }
    }
    mode = g_akao_driver_mode_flags;
    mode |= 2;
    g_akao_driver_mode_flags = mode;
}

/**
 * @brief Resume SFX channels from the paused state set by
 *        akao_sfx_silence_unused_voices_and_pause: flag every parked
 *        channel for a full SPU re-apply, restore the active mask from
 *        the parked D_8004D410 value, and clear the driver "SFX paused"
 *        mode bit.
 * @see decomp.me (100%)
 */
void akao_sfx_resume_and_apply_pending_voices(void)
{
    AkaoChannelState* channel;
    s32 raw_mask;
    s32 mask;
    s32 bit;
    s32 parked;
    s32 mode;

    raw_mask = D_8004D410;
    if (raw_mask != 0)
    {
        mask = raw_mask;
        channel = (AkaoChannelState*)g_sfx_channels;
        bit = 0x1000;
        do
        {
            if (mask & bit)
            {
                mask &= ~bit;
                channel->update_flags |= 0x2B13;
            }
            bit <<= 1;
            channel++;
        } while (mask != 0);

        parked = g_akao_sfx_control.unk10;
        g_akao_sfx_control.unk10 = 0;
        g_akao_sfx_control.unk0 |= parked;
        g_akao_driver_flags.unk8 |= 0x100;
    }
    mode = g_akao_driver_mode_flags;
    mode &= ~2;
    g_akao_driver_mode_flags = mode;
}

/**
 * @brief Zero the pitch of the streamed XA voice pair while a stream is active.
 * @note 100% match (lom-dev-mcp diff tool; no decomp.me scratch created).
 */
void akao_xa_silence_voice_pitch(void)
{
    if (g_akao_xa_tracker.unkC != 0)
    {
        spu_set_voice_pitch(g_akao_xa_tracker.unk10, 0);
        spu_set_voice_pitch(g_akao_xa_tracker.unk10 + 1, 0);
    }
}

/**
 * @brief Restore the streamed XA voice pair's pitch to the tracker's cached
 *        value while a stream is active.
 * @note 100% match (lom-dev-mcp diff tool; no decomp.me scratch created).
 */
void akao_xa_restore_voice_pitch(void)
{
    if (g_akao_xa_tracker.unkC != 0)
    {
        spu_set_voice_pitch(g_akao_xa_tracker.unk10, g_akao_xa_tracker.unk58);
        spu_set_voice_pitch(g_akao_xa_tracker.unk10 + 1, g_akao_xa_tracker.unk58);
    }
}

/**
 * @brief Empty function; body is a bare return.
 * @note 100% match (lom-dev-mcp diff tool; no decomp.me scratch created).
 */
void func_80028E2C(void)
{
}

/**
 * @brief Toggle the SPU global reverb mode if @p reverb_type differs from the
 *        currently active one; brackets the change with reverb off/on so the
 *        SPU does not glitch mid-update.
 * @param reverb_type New reverb mode (an @c AkaoSeqHeader::reverb_type value).
 * @note 100% match (lom-dev-mcp diff tool; no decomp.me scratch created).
 */
void akao_apply_reverb_type(s32 reverb_type)
{
    s32 current_mode;

    SpuGetReverbModeType(&current_mode);
    if (current_mode != reverb_type)
    {
        SpuSetReverb(0);
        SpuSetReverbModeType(reverb_type | 0x100);
        SpuSetReverb(1);
    }
}

/**
 * @brief Central low-level dispatcher for the AKAO sound driver.
 *
 * Masks @p opcode to a byte and routes it to a fixed handler-pointer table
 * (@c D_8003DDE0), indexed either directly by the opcode or by a small set of
 * remapped indices for opcodes that fan out into several handlers at once
 * (0x98/0x99) or that go through a dedicated indirect callback instead of the
 * table (0xD8/0xD9/0xDA). Opcodes 0x10/0x12/0x14/0x19 (song load/change)
 * additionally validate the AKAO magic on @c g_akaoCmdParams[0] and skip the
 * update entirely when the requested song is already active on both channels.
 * The driver's rcnt2 tick event is disabled for the duration of the dispatch
 * so a tick cannot observe a half-updated command-parameter buffer.
 *
 * @param opcode Command opcode; only the low byte is significant.
 * @return For opcodes 0x10/0x12/0x14/0x19, the newly loaded sequence id, 0 if
 *         the requested song was already active on both channels, or -1 if
 *         the header failed the AKAO magic check. Ignored by most other
 *         callers (see the doc comment on the forward declaration in
 *         akao_cmd.c).
 * @note 95.74% match (lom-dev-mcp diff tool; no decomp.me scratch created).
 *       Residual (8 of 194 instructions) is two instances of the same
 *       mechanism: for the 0xD9/0xDA cases, the target computes the
 *       D_8003E124/D_8003E128 callback pointer's %hi in one place and its
 *       %lo-offset load right before the call, while this source's
 *       equivalent expression keeps them adjacent; measured via probe_variants
 *       and the permuter without finding a source shape that reproduces the
 *       target's split. Mechanism: CSE-FOLD/EXPAND-SHAPE (see idioms.md
 *       EXPAND-29, CSE-11 - neither's stated fix measured positive here).
 */
s32 akao_send_command(u32 opcode)
{
    s32 result;
    s32 tmp;
    s32 masked;
    s32* params;
    AkaoSeqHeader* hdr;
    u16 current_id;

    result = 0;
    DisableEvent(g_akao_rcnt2_event);
    opcode = opcode & 0xFF;
    params = D_8004D340;

    switch (opcode)
    {
    case 0x10:
    case 0x12:
    case 0x14:
    case 0x19:
        if (akao_check_magic(g_akaoCmdParams[0]) == 0)
        {
            hdr = g_akaoCmdParams[0];
            current_id = g_akao_seq_channel0->unk5E;
            if ((current_id != hdr->id) ||
                ((g_akao_seq_channel1 != 0) && (g_akao_seq_channel1->unk5E != current_id)))
            {
                akao_apply_reverb_type(hdr->reverb_type);
                params[2] = hdr->id;
                if (opcode == 0x12)
                {
                    params[0] = (s32)hdr;
                    params[4] = (s32)g_akaoCmdParams[1];
                }
                else
                {
                    tmp = (s32)g_akaoCmdParams[1];
                    masked = -1;
                    if (tmp != 0)
                    {
                        params[0] = (s32)hdr;
                        masked = tmp | 1;
                    }
                    params[3] = masked;
                    params[4] = (s32)g_akaoCmdParams[2];
                }
                result = hdr->id;
            }
            else
            {
                opcode = 0;
                result = 0;
            }
        }
        else
        {
            opcode = 0;
            result = -1;
        }
        break;

    case 0xD8:
        params[0] = (s32)g_akaoCmdParams[0];
        (*D_8003E120)(params);
        opcode = 0xD4;
        break;

    case 0xD9:
        params[0] = (s32)g_akaoCmdParams[0];
        params[1] = (s32)g_akaoCmdParams[1];
        (*D_8003E124)(params);
        opcode = 0xD5;
        break;

    case 0xDA:
        params[0] = (s32)g_akaoCmdParams[0];
        params[1] = (s32)g_akaoCmdParams[1];
        params[2] = (s32)g_akaoCmdParams[2];
        (*D_8003E128)(params);
        opcode = 0xD6;
        break;

    case 0x99:
        D_8003DDE0[0x9B](params);
        D_8003DDE0[0x9D](params);
        opcode = 0x9F;
        break;

    case 0x98:
        D_8003DDE0[0x9A](params);
        D_8003DDE0[0x9C](params);
        opcode = 0x9E;
        break;

    default:
        params[0] = (s32)g_akaoCmdParams[0];
        params[1] = (s32)g_akaoCmdParams[1];
        params[2] = (s32)g_akaoCmdParams[2];
        params[3] = (s32)g_akaoCmdParams[3];
        params[4] = (s32)g_akaoCmdParams[4];
        params[5] = (s32)g_akaoCmdParams[5];
        break;
    }

    D_8003DDE0[opcode](params);
    EnableEvent(g_akao_rcnt2_event);
    return result;
}
