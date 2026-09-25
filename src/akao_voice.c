#include "akao_voice.h"
#include "akao_sequencer.h"
#include "sdk/libspu.h"
#include "sdk/libapi.h"

/** @brief Fixed address of the SPU voice register blocks. */
#define SPU_VOICE_REGS ((SpuVoiceRegisters*)0x1F801C00)

/**
 * @brief One register of SPU voice @p voice, addressed as voice 0's register plus the block stride.
 * @note The fixed register address is folded into one constant before the voice offset is added.
 */
#define SPU_VOICE_REG(voice, field) (*(u16*)((u8*)&SPU_VOICE_REGS[0].field + (voice) * sizeof(SpuVoiceRegisters)))

/*
 * SpuVoiceParams::update_flags groups. Each group lists every AKAO update bit
 * that dirties the named SPU voice register.
 */
#define SPU_UPDATE_VOLUME 0x3
#define SPU_UPDATE_PITCH 0x10
#define SPU_UPDATE_START_ADDR 0x80
#define SPU_UPDATE_ADSR_HIGH 0x6600
#define SPU_UPDATE_ADSR_LOW 0x9900
#define SPU_UPDATE_REPEAT_ADDR 0x10000

/* SPU voice bitmap registers; each is split into a low (voices 0-15) and high (voices 16-23) halfword. */
#define SPU_KEY_ON_LOW (*(u16*)0x1F801D88)
#define SPU_KEY_ON_HIGH (*(u16*)0x1F801D8A)
#define SPU_KEY_OFF_LOW (*(u16*)0x1F801D8C)
#define SPU_KEY_OFF_HIGH (*(u16*)0x1F801D8E)
#define SPU_PITCH_MOD_LOW (*(u16*)0x1F801D90)
#define SPU_PITCH_MOD_HIGH (*(u16*)0x1F801D92)
#define SPU_NOISE_LOW (*(u16*)0x1F801D94)
#define SPU_NOISE_HIGH (*(u16*)0x1F801D96)
#define SPU_REVERB_LOW (*(u16*)0x1F801D98)
#define SPU_REVERB_HIGH (*(u16*)0x1F801D9A)

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

/** @brief One 16-byte SPU voice register block. */
typedef struct
{
    s16 volume_left;
    s16 volume_right;
    u16 pitch;
    u16 start_addr;
    u16 adsr_low;
    u16 adsr_high;
    u16 envelope_level;
    u16 repeat_addr;
} SpuVoiceRegisters;

extern s16 D_8003D37C[];
extern AkaoVoiceAllocation D_8004C1A0[];
extern s32 D_8004D404[];
extern s32 D_8004F834[];

void akao_clear_voice_assignment(AkaoChannelState* channels, s32 voice_index);
void akao_build_effect_voice_mask(s32* effect_voices, s32 secondary_effect_mask, s32 primary_effect_mask, s32 sfx_effect_voices);

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
    SPU_KEY_ON_LOW = voice_mask;
    SPU_KEY_ON_HIGH = voice_mask >> 16;
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
    SPU_KEY_OFF_LOW = voice_mask;
    SPU_KEY_OFF_HIGH = voice_mask >> 16;
}

/**
 * @brief Write the SPU per-voice reverb-enable bitmap.
 *
 * @param voice_mask Reverb-enabled voices 0-23.
 * @see decomp.me (100%) https://decomp.me/scratch/3KFgT
 */
void spu_set_reverb_enable(u32 voice_mask)
{
    SPU_REVERB_LOW = voice_mask;
    SPU_REVERB_HIGH = voice_mask >> 16;
}

/**
 * @brief Write the SPU per-voice noise-enable bitmap.
 *
 * @param voice_mask Noise-enabled voices 0-23.
 * @see decomp.me (100%) https://decomp.me/scratch/HFDSO
 */
void spu_set_noise_enable(u32 voice_mask)
{
    SPU_NOISE_LOW = voice_mask;
    SPU_NOISE_HIGH = voice_mask >> 16;
}

/**
 * @brief Write the SPU per-voice pitch-modulation-enable bitmap.
 *
 * @param voice_mask Pitch-modulated voices 0-23.
 * @see decomp.me (100%) https://decomp.me/scratch/ZyBKt
 */
void spu_set_pitch_modulation_enable(u32 voice_mask)
{
    SPU_PITCH_MOD_LOW = voice_mask;
    SPU_PITCH_MOD_HIGH = voice_mask >> 16;
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
    if (scale != 0)
    {
        vol_l *= scale;
        vol_r *= scale;
        vol_l >>= 7;
        vol_r >>= 7;
    }

    SPU_VOICE_REGS[voice].volume_left = vol_l & 0x7FFF;
    SPU_VOICE_REGS[voice].volume_right = vol_r & 0x7FFF;
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
    SPU_VOICE_REG(voice, pitch) = pitch;
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
    SPU_VOICE_REG(voice, start_addr) = addr >> 3;
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
    SPU_VOICE_REG(voice, repeat_addr) = addr >> 3;
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
    SPU_VOICE_REG(voice, adsr_low) = adsr_low;
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
    SPU_VOICE_REG(voice, adsr_high) = adsr_high;
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
    SPU_VOICE_REG(voice, adsr_low) = (SPU_VOICE_REG(voice, adsr_low) & 0xFF) | (((mode_bits >> 2) << 15) | (attack_shift << 8));
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
    SPU_VOICE_REG(voice, adsr_low) = (SPU_VOICE_REG(voice, adsr_low) & 0xFF0F) | (decay_shift << 4);
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
    SPU_VOICE_REG(voice, adsr_low) = (SPU_VOICE_REG(voice, adsr_low) & 0xFFF0) | sustain_level;
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
    SPU_VOICE_REG(voice, adsr_high) = (SPU_VOICE_REG(voice, adsr_high) & 0x3F) | (((mode_bits >> 1) << 14) | (sustain_bits << 6));
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
    SPU_VOICE_REG(voice, adsr_high) = (SPU_VOICE_REG(voice, adsr_high) & 0xFFC0) | (((mode_bit >> 2) << 5) | release_shift);
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
    s32 scaled_l;
    s32 scaled_r;
    u16* reg;

    params->update_flags = 0;
    reg = (u16*)&SPU_VOICE_REGS[voice];

    if (scale == 0)
    {
        scaled_l = params->volume_left;
        scaled_r = params->volume_right;
    }
    else
    {
        scaled_l = params->volume_left * scale;
        scaled_l >>= 7;
        scaled_r = params->volume_right * scale;
        scaled_r >>= 7;
    }

    *reg++ = scaled_l & 0x7FFF;
    *reg++ = scaled_r & 0x7FFF;
    *reg++ = params->pitch;
    *reg++ = params->sample_start_addr >> 3;
    *reg++ = params->adsr_low;
    *reg++ = params->adsr_high;
    reg++;
    *reg = params->sample_repeat_addr >> 3;
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
    s32 pending = params->update_flags;

    if (pending == 0)
    {
        return;
    }

    params->update_flags = 0;

    if (pending & SPU_UPDATE_PITCH)
    {
        pending &= ~SPU_UPDATE_PITCH;
        spu_set_voice_pitch(voice, params->pitch);
        if (pending == 0)
        {
            return;
        }
    }

    if (pending & SPU_UPDATE_VOLUME)
    {
        pending &= ~SPU_UPDATE_VOLUME;
        spu_set_voice_volume(voice, params->volume_left, params->volume_right, params->volume_scale);
        if (pending == 0)
        {
            return;
        }
    }

    if (pending & SPU_UPDATE_START_ADDR)
    {
        pending &= ~SPU_UPDATE_START_ADDR;
        spu_set_voice_start_addr(voice, params->sample_start_addr);
        if (pending == 0)
        {
            return;
        }
    }

    if (pending & SPU_UPDATE_REPEAT_ADDR)
    {
        pending &= ~SPU_UPDATE_REPEAT_ADDR;
        spu_set_voice_repeat_addr(voice, params->sample_repeat_addr);
        if (pending == 0)
        {
            return;
        }
    }

    if (pending & SPU_UPDATE_ADSR_HIGH)
    {
        pending &= ~SPU_UPDATE_ADSR_HIGH;
        spu_set_voice_adsr_high(voice, params->adsr_high);
        if (pending == 0)
        {
            return;
        }
    }

    if (pending & SPU_UPDATE_ADSR_LOW)
    {
        spu_set_voice_adsr_low(voice, params->adsr_low);
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
    AkaoLfoSample* lfo_cursor;
    s32 expression;
    s32 pitch_bend;
    s32 next;
    u16 depth;
    s32 pan;
    u32 depth_level;
    u32 scaled_depth;

    if (is_sfx == 0)
    {
        if (channel->volume_fade_ticks != 0)
        {
            channel->volume_fade_ticks--;
            next = channel->volume + channel->volume_step;
            if ((next & 0x7F00) != (channel->volume & 0x7F00))
            {
                channel->update_flags |= 3;
            }
            channel->volume = next;
        }
    }

    if (channel->expression_fade_ticks != 0)
    {
        expression = channel->unk48;
        channel->expression_fade_ticks--;
        next = expression + channel->unk4C;
        if ((next & 0xFFE00000) != (expression & 0xFFE00000))
        {
            channel->update_flags |= 3;
        }
        channel->unk48 = next;
    }

    if (channel->pan_fade_ticks != 0)
    {
        pan = channel->pan;
        channel->pan_fade_ticks--;
        next = pan + channel->pan_step;
        if ((next & 0xFF00) != (pan & 0xFF00))
        {
            channel->update_flags |= 3;
        }
        channel->pan = next;
    }

    if (channel->pitch_lfo_delay_ticks != 0)
    {
        channel->pitch_lfo_delay_ticks--;
    }

    if (channel->volume_lfo_delay_ticks != 0)
    {
        channel->volume_lfo_delay_ticks--;
    }

    if (channel->reverb_toggle_ticks != 0)
    {
        channel->reverb_toggle_ticks--;
        if (channel->reverb_toggle_ticks == 0)
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

    if (channel->pitch_mod_toggle_ticks != 0)
    {
        channel->pitch_mod_toggle_ticks--;
        if (channel->pitch_mod_toggle_ticks == 0)
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

    if (channel->pitch_lfo_depth_fade_ticks != 0)
    {
        channel->pitch_lfo_depth_fade_ticks--;
        depth = channel->pitch_lfo_depth + channel->pitch_lfo_depth_step;
        channel->pitch_lfo_depth = depth;
        depth_level = (u32)(depth & 0x7F00) >> 8;

        if (depth & 0x8000)
        {
            scaled_depth = (depth_level * channel->pitch) >> 7;
        }
        else
        {
            scaled_depth = (depth_level * ((u32)(channel->pitch * 15) >> 8)) >> 7;
        }

        channel->pitch_lfo_depth_scaled = scaled_depth;

        if ((channel->pitch_lfo_delay_ticks == 0) && (channel->pitch_lfo_restart != 1))
        {
            lfo_cursor = (AkaoLfoSample*)channel->unk1C;
            if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
            {
                lfo_cursor = (AkaoLfoSample*)((u8*)lfo_cursor + lfo_cursor->relative_offset * 2);
            }

            next = (channel->pitch_lfo_depth_scaled * lfo_cursor->sample) >> 16;
            if (next != channel->pitch_lfo_value)
            {
                channel->pitch_lfo_value = next;
                channel->update_flags |= 0x10;
                if (next >= 0)
                {
                    channel->pitch_lfo_value = next * 2;
                }
            }
        }
    }

    if (channel->volume_lfo_depth_fade_ticks != 0)
    {
        channel->volume_lfo_depth_fade_ticks--;
        channel->volume_lfo_depth += channel->volume_lfo_depth_step;

        if ((channel->volume_lfo_delay_ticks == 0) && (channel->volume_lfo_restart != 1))
        {
            lfo_cursor = (AkaoLfoSample*)channel->tempo;
            if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
            {
                lfo_cursor = (AkaoLfoSample*)((u8*)lfo_cursor + lfo_cursor->relative_offset * 2);
            }

            next = (((HALF_HIGH_S16(channel->unk48) * (channel->volume >> 8)) >> 7) * (channel->volume_lfo_depth >> 8) << 9) >> 16;
            next = (next * lfo_cursor->sample) >> 15;

            if (next != channel->volume_lfo_value)
            {
                channel->volume_lfo_value = next;
                channel->update_flags |= 3;
            }
        }
    }

    if (channel->pan_lfo_depth_fade_ticks != 0)
    {
        channel->pan_lfo_depth_fade_ticks--;
        channel->pan_lfo_depth += channel->pan_lfo_depth_step;

        if (channel->pan_lfo_restart != 1)
        {
            lfo_cursor = (AkaoLfoSample*)channel->tempo_step;
            if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
            {
                lfo_cursor = (AkaoLfoSample*)((u8*)lfo_cursor + lfo_cursor->relative_offset * 2);
            }

            next = ((channel->pan_lfo_depth >> 8) * lfo_cursor->sample) >> 15;

            if (next != channel->pan_lfo_value)
            {
                channel->pan_lfo_value = next;
                channel->update_flags |= 3;
            }
        }
    }

    if (channel->pitch_slide_ticks != 0)
    {
        pitch_bend = channel->unk30;
        channel->pitch_slide_ticks--;
        next = pitch_bend + channel->pitch_slide_step;

        if ((next & 0xFFFF0000) != (pitch_bend & 0xFFFF0000))
        {
            channel->update_flags |= 0x10;
        }
        channel->unk30 = next;
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
    s32 effect_value;

    effect_flags = channel->flags;
    scaled_volume = (HALF_HIGH_S16(channel->unk48) * (channel->volume >> 8)) >> 7;

    if (effect_flags & 1)
    {
        if (channel->pitch_lfo_delay_ticks == 0)
        {
            channel->pitch_lfo_restart--;
            if (channel->pitch_lfo_restart == 0)
            {
                channel->pitch_lfo_restart = channel->pitch_lfo_period;
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
                    effect_value = (channel->pitch_lfo_depth_scaled * waveform_sample) >> 16;
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
                channel->volume_lfo_restart = channel->volume_lfo_period;
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
        scaled_volume = ((s16)(channel[-1].spu_pitch << 1) * (channel->volume >> 8)) >> 7;
        channel->update_flags |= 3;
    }

    if (channel->update_flags & 3)
    {
        scaled_volume += channel->volume_lfo_value;
        scaled_volume = (scaled_volume * (HALF_HIGH_U16(g_akao_seq_channel0->pitch_slide_step) & 0x7F)) >> 7;
        pan_index = ((channel->pan >> 8) + channel->pan_lfo_value) & 0xFF;

        if (D_8004F754[0] == 2)
        {
            channel->spu_volume_right = (scaled_volume * D_8003D47C[0]) >> 15;
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
        effect_value = HALF_HIGH_S16(channel->unk30);
        pitch_offset = channel[-1].spu_pitch + channel->pitch_lfo_value + effect_value;
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
        channel->spu_pitch = (HALF_LOW_U16(channel->detune_pitch_delta) + pitch_offset) & 0x3FFF;
        channel->update_flags |= 0x10;
        return;
    }

    if (channel->update_flags & 0x10)
    {
        pitch_offset = channel->pitch + channel->pitch_lfo_value + HALF_HIGH_S16(channel->unk30);
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
        channel->spu_pitch = (HALF_LOW_U16(channel->detune_pitch_delta) + pitch_offset) & 0x3FFF;
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

    effect_flags = channel->flags;
    scaled_volume = (HALF_HIGH_S16(channel->unk48) * (channel->volume >> 8)) >> 7;

    if (effect_flags & 1)
    {
        channel->pitch_lfo_restart--;
        if (channel->pitch_lfo_restart == 0)
        {
            channel->pitch_lfo_restart = channel->pitch_lfo_period;
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
                effect_value = (channel->pitch_lfo_depth_scaled * waveform_sample) >> 16;
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
            channel->volume_lfo_restart = channel->volume_lfo_period;
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
        scaled_volume = ((s16)(channel[-1].spu_pitch << 1) * (channel->volume >> 8)) >> 7;
        channel->update_flags |= 3;
    }

    if (channel->update_flags & 3)
    {
        scaled_volume += channel->volume_lfo_value;
        if (channel->tempo_acc & AKAO_SFX_FLAG_SUPPRESS)
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
            channel->spu_volume_right = (scaled_volume * D_8003D47C[0]) >> 15;
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
        effect_value = HALF_HIGH_S16(channel->unk30);
        pitch_offset = channel[-1].spu_pitch + channel->pitch_lfo_value + effect_value;
        if (!(channel->tempo_acc & AKAO_SFX_FLAG_SUPPRESS))
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
        channel->spu_pitch = (HALF_LOW_U16(channel->detune_pitch_delta) + pitch_offset) & 0x3FFF;
        channel->update_flags |= 0x10;
        return;
    }

    if (channel->update_flags & 0x10)
    {
        pitch_offset = channel->pitch + channel->pitch_lfo_value + HALF_HIGH_S16(channel->unk30);
        if (!(channel->tempo_acc & AKAO_SFX_FLAG_SUPPRESS))
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
        channel->spu_pitch = (HALF_LOW_U16(channel->detune_pitch_delta) + pitch_offset) & 0x3FFF;
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
    quietest_voice = AKAO_VOICE_COUNT;
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
    } while (voice_index < AKAO_VOICE_COUNT);

    if ((s16)quietest_level == 0x7FFF)
    {
        return AKAO_VOICE_COUNT;
    }

    akao_clear_voice_assignment((AkaoChannelState*)g_akao_seq_channels, quietest_voice);
    return quietest_voice;
}

/**
 * @brief Find the first unallocated voice in the requested voice range.
 * @param ignore_voice_reserve Nonzero to include voices below the reserve floor.
 * @return Available voice index, or 0x18 when none is available.
 */
s32 akao_find_free_voice(s32 ignore_voice_reserve)
{
    s32 voice_index;
    AkaoVoiceAllocation* allocation;

    if (ignore_voice_reserve != 0)
    {
        voice_index = 0;
    }
    else
    {
        voice_index = g_akao_seq_channel0->voice_alloc_base;
    }

    allocation = &D_8004C1A0[voice_index];
    while (allocation->envelope_level != 0)
    {
        voice_index++;
        if (voice_index >= AKAO_VOICE_COUNT)
        {
            break;
        }
        allocation++;
    }

    return voice_index;
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
    u8 unassigned_voice;
    channel_bit = 1;
    channel_index = 0;
    voice_bit = channel_bit;
    pending_key_on_mask = channel_mask & g_akao_seq_channel0->w04.song.key_on_mask;
    do
    {
        unassigned_voice = AKAO_VOICE_COUNT;
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
                        if (voice_index == AKAO_VOICE_COUNT)
                        {
                            g_akao_seq_channel0->seq_cursor = (u8*)(((u32)g_akao_seq_channel0->seq_cursor) | 2);
                            voice_index = akao_steal_quietest_voice(ignore_voice_reserve);
                            if (voice_index == AKAO_VOICE_COUNT)
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
                        spu_write_voice_params(channels->voice, (SpuVoiceParams*)&channels->voice, channels->spu_volume_scale);
                        D_8004F7C0[channels->voice] = g_akao_seq_channel0;
                        g_akao_driver_flags.unk8 |= 0x100;
                    }
                }
                else if (channels->voice < unassigned_voice)
                {
                    spu_apply_voice_updates(channels->voice, (SpuVoiceParams*)&channels->voice, channels->flags);
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
 * @param channels Primary channel table; the pending table is also scanned when a secondary song is loaded.
 * @param voice_index Voice index to clear.
 */
void akao_clear_voice_assignment(AkaoChannelState* channels, s32 voice_index)
{
    u32 i;
    AkaoChannelState* channel;

    channel = channels;
    for (i = 0; i < AKAO_CHANNEL_COUNT; i++, channel++)
    {
        if (voice_index == channel->voice)
        {
            channel->voice = AKAO_VOICE_COUNT;
        }
    }

    if (g_akao_seq_channel1 != NULL)
    {
        channel = (AkaoChannelState*)g_akao_pending_channels;
        for (i = 0; i < AKAO_CHANNEL_COUNT; i++, channel++)
        {
            if (voice_index == channel->voice)
            {
                channel->voice = AKAO_VOICE_COUNT;
            }
        }
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

    unavailable_voice_mask = (g_akao_seq_channel0->w04.song.active_mask & g_akao_seq_channel0->w04.song.static_voice_mask) | reserved_voice_mask;
    if (g_akao_seq_channel1 != NULL)
    {
        unavailable_voice_mask |= g_akao_seq_channel1->w04.song.active_mask & g_akao_seq_channel1->w04.song.static_voice_mask;
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
                SpuGetVoiceEnvelope(voice_index, envelope_level);

                if (*envelope_level == 0)
                {
                    akao_clear_voice_assignment((AkaoChannelState*)g_akao_seq_channels, voice_index);
                }
            }

            voice_index++;
            envelope_level = (s16*)((u8*)envelope_level + sizeof(AkaoVoiceAllocation));
        } while (voice_index < AKAO_VOICE_COUNT);
    }
}

/**
 * @brief Flush pending sequence, SFX, and global SPU voice updates.
 * @param sfx_update_mask Unused; retained by the original calling convention.
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
            primary_static_voice_mask = AKAO_SFX_FIRST_CHANNEL_BIT;
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
        SpuSetReverbModeDepth(master_volume, master_volume);
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
        SpuSetNoiseClock(noise_frequency);
        g_akao_driver_flags.unk8 &= ~0x10;
    }

    if (work_mask & 0x100)
    {
        effect_mask_center = D_8004F834;
        akao_build_effect_voice_mask(effect_mask_center, g_akao_seq_channel1->reverb_mask, g_akao_seq_channel0->reverb_mask, g_akao_sfx_control.reverb_mask);
        effect_mask_base = effect_mask_center - 1;
        akao_build_effect_voice_mask(effect_mask_base, g_akao_seq_channel1->noise_mask, g_akao_seq_channel0->noise_mask, g_akao_sfx_control.noise_mask);
        akao_build_effect_voice_mask(effect_mask_center + 1, g_akao_seq_channel1->pitch_mod_mask, g_akao_seq_channel0->pitch_mod_mask,
                                     g_akao_sfx_control.pitch_mod_mask);
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
            if (voice < AKAO_VOICE_COUNT)
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
