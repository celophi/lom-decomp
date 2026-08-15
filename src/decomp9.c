#include "decomp9.h"

typedef struct
{
    s16 value;
    s16 unk2;
    s16 relative_offset;
} AkaoLfoSample;

/** @brief Voice-allocation bookkeeping entry used by the AKAO mixer. */
typedef struct VoiceAllocEntry
{
    s32 unk0;
    s16 unk4;
    s16 unk6;
} VoiceAllocEntry;

/** @brief Minimal sequence-header view used by the voice-release pass. */
typedef struct SeqHead
{
    s32 unk0;
    u32 mask4;
    u32 unk8;
    u32 maskC;
} SeqHead;

extern s16 D_8003D37C[];
extern s16 D_8003D47C;
extern s32 D_8004F754[];
extern VoiceAllocEntry D_8004C1A0[];
extern AkaoChannelState *D_8004F7C0[];
extern s32 D_8004F76C[];
extern s32 D_8004D404[];
extern s32 D_8004F834[];

extern void func_80025760(u8 *channels, s32 voice);
extern void func_80025F48(s32 *dest, s32 target, s32 current, s32 step);
extern void func_8002613C(s32 arg0, s32 arg1);
extern void func_800260CC(u16 arg0);

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
void spu_set_voice_pitch(s32 voice, u16 pitch)
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

/**
 * @brief Advance the per-channel pitch/volume/pan LFOs and recompute the SPU
 *        volume and pitch registers for one AKAO sequencer channel.
 * @param channel Channel state to update.
 * @param channel_bit Channel mask supplied by the caller; unused here.
 */
void func_80024B00(AkaoChannelState* channel, s32 channel_bit)
{
    s32 flags;
    s32 lfo_pitch;
    s32 value;
    s32 pan;
    s32 pitch_value;
    s32 pitch_mode;
    s32 master_scale;

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

    pitch_mode = channel->update_flags & 3;
    if (pitch_mode)
    {
        lfo_pitch += channel->volume_lfo_value;
        lfo_pitch = (lfo_pitch * (*((u16*)((u8*)g_akao_seq_channel0 + 0x52)) & 0x7F)) >> 7;
        pan = ((channel->pan >> 8) + channel->pan_lfo_value) & 0xFF;

        if (D_8004F754[0] == 2)
        {
            channel->spu_volume_right = (lfo_pitch * D_8003D47C) >> 15;
            channel->spu_volume_left = channel->spu_volume_right;
        }
        else
        {
            channel->spu_volume_left = (lfo_pitch * D_8003D37C[pan]) >> 15;
            channel->spu_volume_right = (lfo_pitch * D_8003D37C[pan ^ 0xFF]) >> 15;
        }
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

/**
 * @brief Update pitch, volume, and pan effects for one SFX channel.
 * @param channel Channel state to update.
 * @param channel_bit Channel mask supplied by the caller; unused here.
 */
void func_80024F60(AkaoChannelState* channel, s32 channel_bit)
{
    s32 flags;
    s32 lfo_pitch;
    s32 value;
    s32 pan;
    s32 pitch_value;
    s32 master_scale;
    s32 next_flags;

    flags = channel->flags;
    lfo_pitch = (((s16*)&channel->unk48)[1] * (channel->volume >> 8)) >> 7;

    if (flags & 1)
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

    if (flags & 2)
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
        if (channel->tempo_acc & 0x02000000)
        {
            pan = 0x80;
        }
        else
        {
            s32 pan_base;
            lfo_pitch = (lfo_pitch * (s8)(channel->volume_scale >> 8)) >> 7;
            pan_base = ((channel->pan + channel->pan_bias) >> 8) + channel->pan_lfo_value;
            pan = pan_base + 0x80;
            pan &= 0xFF;
        }

        if (D_8004F754[0] == 2)
        {
            channel->spu_volume_right = (lfo_pitch * D_8003D47C) >> 15;
            channel->spu_volume_left = channel->spu_volume_right;
        }
        else
        {
            channel->spu_volume_left = (lfo_pitch * D_8003D37C[pan]) >> 15;
            channel->spu_volume_right = (lfo_pitch * D_8003D37C[pan ^ 0xFF]) >> 15;
        }
    }

    if (flags & 0x10)
    {
        master_scale = *((s16*)((u8*)channel + 0x32));
        pitch_value = *((u16*)((u8*)channel - 0xC)) + channel->pitch_lfo_value + master_scale;
        if (!(channel->tempo_acc & 0x02000000))
        {
            master_scale = channel->noise_mask & 0xFF00;
            if (master_scale != 0)
            {
                master_scale >>= 8;
                if (master_scale < 0x80)
                {
                    pitch_value += (pitch_value * master_scale) >> 7;
                }
                else
                {
                    pitch_value = (pitch_value * master_scale) >> 8;
                }
            }
        }
        channel->spu_pitch = (*((u16*)((u8*)channel + 0x54)) + pitch_value) & 0x3FFF;
        channel->update_flags |= 0x10;
        return;
    }

    if (channel->update_flags & 0x10)
    {
        pitch_value = channel->pitch + channel->pitch_lfo_value + *((s16*)((u8*)channel + 0x32));
        if (!(channel->tempo_acc & 0x02000000))
        {
            master_scale = channel->noise_mask & 0xFF00;
            if (master_scale != 0)
            {
                master_scale >>= 8;
                if (master_scale < 0x80)
                {
                    pitch_value += (pitch_value * master_scale) >> 7;
                }
                else
                {
                    pitch_value = (pitch_value * master_scale) >> 8;
                }
            }
        }
        channel->spu_pitch = (*((u16*)((u8*)channel + 0x54)) + pitch_value) & 0x3FFF;
    }
}

/**
 * @brief Select and release the quietest allocated voice.
 * @param arg0 Nonzero to search from voice zero.
 * @return Selected voice index, or 0x18 when no voice is available.
 */
s32 func_800253E8(s32 arg0)
{
    s32 index;
    u16 best_value;
    s32 best_index;
    VoiceAllocEntry* entry;

    if (arg0 != 0)
    {
        index = 0;
    }
    else
    {
        index = g_akao_seq_channel0->voice_alloc_base;
    }

    best_value = 0x7FFF;
    best_index = 0x18;
    entry = &D_8004C1A0[index];

    do
    {
        if (entry->unk4 < (s16)best_value)
        {
            best_value = (u16)entry->unk4;
            best_index = index;
        }
        index++;
        entry++;
    } while (index < 0x18);

    if ((s16)best_value == 0x7FFF)
    {
        return 0x18;
    }

    func_80025760(g_akao_seq_channels, best_index);
    return best_index;
}

/**
 * @brief Find the first unallocated voice in the requested voice range.
 * @param arg0 Nonzero to search from voice zero.
 * @return Available voice index, or 0x18 when none is available.
 */
s32 func_80025498(s32 arg0)
{
    VoiceAllocEntry* entry;

    if (arg0 != 0)
    {
        arg0 = 0;
    }
    else
    {
        arg0 = g_akao_seq_channel0->voice_alloc_base;
    }

    entry = &D_8004C1A0[arg0];
    if (entry->unk4 != 0)
    {
        arg0++;
        while (1)
        {
            if (arg0 >= 0x18)
            {
                break;
            }

            entry++;
            arg0++;
            if (entry->unk4 != 0)
            {
                continue;
            }

            arg0--;
            break;
        }
    }

    return arg0;
}

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

/**
 * @brief Clear one voice assignment from active channel tables.
 * @param channels Base of the primary channel table.
 * @param voice Voice index to clear.
 */
void func_80025760(u8* channels, s32 voice)
{
    u32 i;
    s32 none;
    u8* pending;

    i = 0;
    none = 0x18;
    channels += 0xFC;
    do {
        if (voice == *(s32*)channels) {
            *(s32*)channels = none;
        }
        i++;
        channels += 0x118;
    } while (i < 0x20U);

    if (g_akao_seq_channel1 != 0) {
        i = 0;
        pending = (u8*)(g_akao_pending_channels + 0xFC);
        do {
            if (voice == *(s32*)pending) {
                *(s32*)pending = 0x18;
            }
            i++;
            pending += 0x118;
        } while (i < 0x20U);
    }
}

/**
 * @brief Refresh allocation state for every SPU voice.
 * @param mask Voices already reserved by the caller.
 * @param arg1 Secondary caller mask; unused by this function.
 */
void func_800257E0(u32 mask, s32 arg1)
{
    u32 used;
    u32 i;
    s32 one;
    s32 max;
    u32 m4;
    u32 mC;
    SeqHead* seq1;

    m4 = ((SeqHead*)g_akao_seq_channel0)->mask4;
    do {
        mC = ((SeqHead*)g_akao_seq_channel0)->maskC;
    } while (0);

    seq1 = (SeqHead*)g_akao_seq_channel1;

    do {
        used = (m4 & mC) | mask;
    } while (0);

    if (seq1 != 0) {
        used |= seq1->mask4 & seq1->maskC;
    }

    i = 0;
    one = 1;
    max = 0x7FFF;

    {
        VoiceAllocEntry* entry;
        s16* value;

        entry = D_8004C1A0;
        value = &entry->unk4;

        do {
            s32 c0 = (used & (one << i)) != 0;
            s32 c1 = (c0 != 0);
            s32 c2 = (c1 != 0);
            s32 c3 = (c2 != 0);
            s32 c4 = (c3 != 0);
            s32 c5 = (c4 != 0);
            s32 c6 = (c5 != 0);
            s32 c7 = (c6 != 0);

            if (c7) {
                *value = max;
            } else {
                func_8002611C(i, value);

                if (*value == 0) {
                    func_80025760(g_akao_seq_channels, i);
                }
            }

            i++;
            value = (s16*)((u8*)value + 8);
        } while (i < 0x18U);
    }
}

/**
 * @brief Per-tick AKAO note-off / voice-deallocation pass: releases voices
 *        for notes no longer sounding on the song channel(s) and the SFX
 *        channel array, then applies pending SPU hardware updates (LFO
 *        recompute, key-off frequency, reverb/noise/pitch-mod fades, and
 *        key-on) gated by g_akao_driver_flags.unk8.
 * @note 99.97% match (gcc280_g4, 286/288 exact rows, +0 insns). The only
 *       residual is 2 argdiff rows: the master-state addiu/sw in the
 *       dealloc_mask1 block use s1 (sfx_channel) where the target uses v0.
 *       Shapes required to match, each measured via probe_variants:
 *       - array-form extern declarations above (+21 exact);
 *       - a SINGLE loop pointer with plain field accesses in the SFX loop:
 *         gcc's loop.c combines the flags/voice/update_flags mem givs into
 *         one reduced giv anchored at flags (channel+0x34), reproducing the
 *         target's second walker exactly. A hand-built second pointer makes
 *         the giv family anchor at +0xC8 and adds a third walker instead;
 *       - ONE variable (dealloc_mask0) reused for the channel-0 dealloc
 *         mask, the SFX active mask, and the driver update flags (+20): all
 *         three land in s3 like the target. Separate variables give each a
 *         different callee-saved reg;
 *       - `(static_voice_mask1 | mask)` inlined in both dealloc_mask0 and
 *         static_voice_mask0 (no combined_mask local, +10);
 *       - `dealloc_mask1 = 0` initialized before `static_voice_mask1 = 0`
 *         (+2); `key` as s32 not u16 (+3);
 *       - routing the master-state store through sfx_channel in the
 *         dealloc_mask1 block (+29): the early def raises sfx_channel's
 *         priority above bit (refs 12 vs 11, so it wins s1) and creates the
 *         sfx_channel x static_voice_mask0 conflict that forces
 *         static_voice_mask0 into s2 and static_voice_mask1 into s4. The
 *         target emits that store from v0 (no sfx_channel def there), so the
 *         original produced the same allocation by some other means - that
 *         is the open 2-row residual.
 *       - fade block anchored at &D_8004F834 with `fade1 = fade0 - 1` and
 *         relative loads, matching the target's addiu s2, s0, -4 shape.
 */
void func_800258B8(void)
{
    u32 voice_mask;
    s32 mask;
    s32 dealloc_mask1;
    s32 static_voice_mask1;
    s32 key_on_submask1;
    s32 dealloc_mask0;
    s32 static_voice_mask0;
    s32 key_on_submask0;
    s32 bit;
    AkaoChannelState *sfx_channel;
    typeof(g_akao_seq_channel0->w04.song) *song_ptr;
    AkaoChannelState *old_channel0;
    s32 *fade0;
    s32 *fade1;
    s32 lfo_arg;
    s32 key;

    dealloc_mask1 = 0;
    static_voice_mask1 = 0;
    voice_mask = 0;
    mask = (g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10) | D_8004F76C[0];

    if ((g_akao_seq_channel0->w04.song.active_mask & g_akao_seq_channel0->w04.song.key_on_mask) ||
        ((g_akao_seq_channel1 != NULL) && (g_akao_seq_channel1->w04.song.active_mask & g_akao_seq_channel1->w04.song.key_on_mask)))
    {
        func_800257E0(mask, D_8004F76C[0]);
    }

    if (g_akao_seq_channel1 != NULL)
    {
        g_akao_seq_channel0 = g_akao_seq_channel1;
        dealloc_mask1 = g_akao_seq_channel1->w04.song.active_mask & g_akao_seq_channel1->note_on_mask & ~(g_akao_seq_channel1->w04.song.static_voice_mask & mask);
        static_voice_mask1 = g_akao_seq_channel1->w04.song.static_voice_mask;
        key_on_submask1 = dealloc_mask1 & g_akao_seq_channel1->w04.song.voice_alloc_low_mask;
        static_voice_mask1 = dealloc_mask1 & static_voice_mask1 & ~mask;
        if (key_on_submask1 != 0)
        {
            func_80025500((AkaoChannelState *)g_akao_pending_channels, key_on_submask1, static_voice_mask1, &voice_mask);
            song_ptr = &g_akao_seq_channel0->w04.song;
            dealloc_mask1 &= ~song_ptr->voice_alloc_low_mask;
            g_akao_seq_channel0->w04.song.key_on_mask &= ~g_akao_seq_channel0->w04.song.voice_alloc_low_mask;
        }
        g_akao_seq_channel0 = &g_akao_seq_master_state;
    }

    dealloc_mask0 = g_akao_seq_channel0->w04.song.active_mask & g_akao_seq_channel0->note_on_mask & ~(g_akao_seq_channel0->w04.song.static_voice_mask & (static_voice_mask1 | mask));
    static_voice_mask0 = dealloc_mask0 & g_akao_seq_channel0->w04.song.static_voice_mask & ~(static_voice_mask1 | mask);
    key_on_submask0 = dealloc_mask0 & g_akao_seq_channel0->w04.song.voice_alloc_low_mask;
    if (key_on_submask0 != 0)
    {
        func_80025500((AkaoChannelState *)g_akao_seq_channels, key_on_submask0, static_voice_mask0, &voice_mask);
        song_ptr = &g_akao_seq_channel0->w04.song;
        dealloc_mask0 &= ~song_ptr->voice_alloc_low_mask;
        g_akao_seq_channel0->w04.song.key_on_mask &= ~g_akao_seq_channel0->w04.song.voice_alloc_low_mask;
    }

    if ((g_akao_seq_channel1 != NULL) && (dealloc_mask1 != 0))
    {
        g_akao_seq_channel0 = g_akao_seq_channel1;
        func_80025500((AkaoChannelState *)g_akao_pending_channels, dealloc_mask1, static_voice_mask1 & ~static_voice_mask0, &voice_mask);
        old_channel0 = g_akao_seq_channel0;
        sfx_channel = &g_akao_seq_master_state;
        g_akao_seq_channel0 = sfx_channel;
        old_channel0->w04.song.key_on_mask = 0;
    }

    if (dealloc_mask0 != 0)
    {
        func_80025500((AkaoChannelState *)g_akao_seq_channels, dealloc_mask0, static_voice_mask0, &voice_mask);
        g_akao_seq_channel0->w04.song.key_on_mask = 0;
    }

    dealloc_mask0 = g_akao_sfx_control.unk0 & g_akao_sfx_control.unk8;
    if (dealloc_mask0 != 0)
    {
        bit = 0x1000;
        sfx_channel = (AkaoChannelState *)g_sfx_channels;
        voice_mask |= g_akao_sfx_control.unk4;
        do
        {
            if (dealloc_mask0 & bit)
            {
                func_80024F60(sfx_channel, bit);
                if (sfx_channel->update_flags != 0)
                {
                    spu_apply_voice_updates(sfx_channel->voice, &sfx_channel->voice, sfx_channel->flags);
                }
                dealloc_mask0 &= ~bit;
            }
            bit <<= 1;
            sfx_channel++;
        } while (dealloc_mask0 != 0);
        D_8004D404[0] = 0;
    }

    dealloc_mask0 = g_akao_driver_flags.unk8;
    if (dealloc_mask0 & 0x80)
    {
        lfo_arg = (s32)(g_akao_seq_channel0->unk48 << 4) >> 16;
        func_8002613C(lfo_arg, lfo_arg);
        g_akao_driver_flags.unk8 &= ~0x80;
    }

    if (dealloc_mask0 & 0x10)
    {
        if (g_akao_sfx_control.unk0 != 0)
        {
            key = g_akao_sfx_control.unk28;
        }
        else
        {
            key = g_akao_seq_channel0->noise_freq;
        }
        func_800260CC(key);
        g_akao_driver_flags.unk8 &= ~0x10;
    }

    if (dealloc_mask0 & 0x100)
    {
        fade0 = D_8004F834;
        func_80025F48(fade0, g_akao_seq_channel1->reverb_mask, g_akao_seq_channel0->reverb_mask, g_akao_sfx_control.reverb_mask);
        fade1 = fade0 - 1;
        func_80025F48(fade1, g_akao_seq_channel1->noise_mask, g_akao_seq_channel0->noise_mask, g_akao_sfx_control.noise_mask);
        func_80025F48(fade0 + 1, g_akao_seq_channel1->pitch_mod_mask, g_akao_seq_channel0->pitch_mod_mask, g_akao_sfx_control.pitch_mod_mask);
        spu_set_reverb_enable(fade0[-1]);
        spu_set_noise_enable(fade1[1]);
        spu_set_pitch_modulation_enable(fade1[2]);
        g_akao_driver_flags.unk8 &= ~0x100;
    }

    if (voice_mask != 0)
    {
        spu_set_key_on(voice_mask);
    }
}
