#include "internal/akao_voice.h"
#include "internal/akao_sequencer.h"
#include <libspu.h>
#include <libapi.h>

/** @brief First voice bit stored in the high bitmap register. */
#define SPU_VOICE_MASK_HIGH_SHIFT 16

/** @brief Fractional bits in signed LFO samples and stereo pan-table gains. */
#define AKAO_Q15_SHIFT 15

/* Accumulator bits that trigger an SPU parameter update during a fade. */
#define AKAO_EXPRESSION_UPDATE_MASK 0xFFE00000
#define AKAO_PITCH_SLIDE_LEVEL_MASK 0xFFFF0000

/** @brief Waveform restart sentinel used before the first sample. */
#define AKAO_LFO_RESTART_PENDING 1

/** @brief Align a volume-LFO depth product for signed high-half extraction. */
#define AKAO_VOLUME_LFO_PRECISION_SHIFT 9

/** @brief Encoded pitch bend boosts below 128 and reduces at or above it. */
#define AKAO_PITCH_BEND_REDUCTION_THRESHOLD 0x80

/** @brief Fourteen-bit SPU pitch register value. */
#define SPU_PITCH_MASK 0x3FFF

/** @brief Direct SPU volume bits, excluding the sweep-enable flag. */
#define SPU_DIRECT_VOLUME_MASK 0x7FFF

/** @brief SPU sample addresses are stored in eight-byte units. */
#define SPU_ADDRESS_UNIT_SHIFT 3

/* Packed ADSR1 fields and the mode selector shared by attack and release. */
#define SPU_ADSR_DECAY_SUSTAIN_MASK 0x00FF
#define SPU_ADSR_ATTACK_RATE_SHIFT 8
#define SPU_ADSR_ATTACK_MODE_SHIFT 15
#define SPU_ADSR_MODE_INPUT_SHIFT 2
#define SPU_ADSR_DECAY_RATE_SHIFT 4
#define SPU_ADSR_DECAY_RATE_MASK 0x00F0
#define SPU_ADSR_SUSTAIN_LEVEL_MASK 0x000F

/* ADSR2 sustain fields occupy the bits above the release fields. */
#define SPU_ADSR_RELEASE_MASK 0x003F
#define SPU_ADSR_SUSTAIN_MASK 0xFFC0
#define SPU_ADSR_SUSTAIN_RATE_SHIFT 6
#define SPU_ADSR_SUSTAIN_MODE_SHIFT 14
#define SPU_ADSR_SUSTAIN_MODE_INPUT_SHIFT 1
#define SPU_ADSR_RELEASE_MODE_SHIFT 5

/** @brief Envelope sentinel for reserved voices and voices awaiting their first key-on. */
#define AKAO_VOICE_ENVELOPE_UNAVAILABLE 0x7FFF

/** @brief Fixed address of the SPU voice register blocks. */
#define SPU_VOICE_REGS ((SpuVoiceRegisters*)0x1F801C00)

/**
 * @brief One register of SPU voice @p voice, addressed as voice 0's register plus the block stride.
 */
#define SPU_VOICE_REG(voice, field) (*(u16*)((u8*)&SPU_VOICE_REGS[0].field + (voice) * sizeof(SpuVoiceRegisters)))

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

/** @brief Indices of the driver's per-effect SPU voice masks. */
typedef enum
{
    AKAO_VOICE_EFFECT_REVERB,
    AKAO_VOICE_EFFECT_NOISE,
    AKAO_VOICE_EFFECT_PITCH_MODULATION
} AkaoVoiceEffect;

/** @brief Waveform sample or zero-pair marker whose relative jump counts s16 samples. */
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

extern AkaoVoiceAllocation g_akao_voice_allocations[];

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
    SPU_KEY_ON_HIGH = voice_mask >> SPU_VOICE_MASK_HIGH_SHIFT;
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
    SPU_KEY_OFF_HIGH = voice_mask >> SPU_VOICE_MASK_HIGH_SHIFT;
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
    SPU_REVERB_HIGH = voice_mask >> SPU_VOICE_MASK_HIGH_SHIFT;
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
    SPU_NOISE_HIGH = voice_mask >> SPU_VOICE_MASK_HIGH_SHIFT;
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
    SPU_PITCH_MOD_HIGH = voice_mask >> SPU_VOICE_MASK_HIGH_SHIFT;
}

/**
 * @brief Set the stereo volume registers for one SPU voice.
 *
 * A nonzero Q7 scale multiplies both values before a logical right shift.
 * The sweep-enable bit is cleared before each register write.
 *
 * @param voice Voice index (0-23).
 * @param vol_l Raw left volume.
 * @param vol_r Raw right volume.
 * @param scale If non-zero, fixed-point scale factor applied before write.
 * @see decomp.me (100%) https://decomp.me/scratch/WzWyo
 */
void spu_set_voice_volume(s32 voice, u32 vol_l, u32 vol_r, s32 scale)
{
    if (scale != 0)
    {
        vol_l *= scale;
        vol_r *= scale;
        vol_l >>= AKAO_Q7_SHIFT;
        vol_r >>= AKAO_Q7_SHIFT;
    }

    SPU_VOICE_REGS[voice].volume_left = vol_l & SPU_DIRECT_VOLUME_MASK;
    SPU_VOICE_REGS[voice].volume_right = vol_r & SPU_DIRECT_VOLUME_MASK;
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
    SPU_VOICE_REG(voice, start_addr) = addr >> SPU_ADDRESS_UNIT_SHIFT;
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
    SPU_VOICE_REG(voice, repeat_addr) = addr >> SPU_ADDRESS_UNIT_SHIFT;
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
 * Release shift and mode occupy bits 0-5; sustain rate, direction and mode
 * occupy bits 6-12, 14 and 15.
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
    SPU_VOICE_REG(voice, adsr_low) = (SPU_VOICE_REG(voice, adsr_low) & SPU_ADSR_DECAY_SUSTAIN_MASK) |
                                     (((mode_bits >> SPU_ADSR_MODE_INPUT_SHIFT) << SPU_ADSR_ATTACK_MODE_SHIFT) | (attack_shift << SPU_ADSR_ATTACK_RATE_SHIFT));
}

/**
 * @brief Set only the decay-shift field of the low ADSR register.
 *
 * Preserves attack (bits 8-15) and sustain level (bits 0-3); sets
 * decay shift in bits 4-7.
 *
 * @param voice Voice index (0-23).
 * @param decay_shift Decay shift value (0-15).
 * @see decomp.me (100%) https://decomp.me/scratch/ymuym
 */
void spu_set_voice_decay_shift(s32 voice, s32 decay_shift)
{
    SPU_VOICE_REG(voice, adsr_low) = (SPU_VOICE_REG(voice, adsr_low) & (0xFFFF ^ SPU_ADSR_DECAY_RATE_MASK)) | (decay_shift << SPU_ADSR_DECAY_RATE_SHIFT);
}

/**
 * @brief Set only the Sustain Level field of ADSR1 for a single SPU voice.
 *
 * Preserves the attack and decay fields; sets sustain level
 * in bits 0-3 from @p sustain_level.
 *
 * @param voice Voice index (0-23).
 * @param sustain_level Sustain level nibble (0-15).
 * @see decomp.me (100%) https://decomp.me/scratch/2UD84
 */
void spu_set_voice_sustain_level(s32 voice, s32 sustain_level)
{
    SPU_VOICE_REG(voice, adsr_low) = (SPU_VOICE_REG(voice, adsr_low) & (0xFFFF ^ SPU_ADSR_SUSTAIN_LEVEL_MASK)) | sustain_level;
}

/**
 * @brief Set the sustain fields of the high ADSR register, preserving the
 *        release fields (bits 0-5).
 *
 * @param voice Voice index (0-23).
 * @param sustain_bits Sustain shift/mode bits, placed at bit 6.
 * @param mode_bits Psy-Q mode selector; bit 1 sets direction and bit 2 sets exponential mode.
 * @see decomp.me (100%) https://decomp.me/scratch/ZWKKM
 */
void spu_set_voice_sustain_mode(s32 voice, s32 sustain_bits, u32 mode_bits)
{
    SPU_VOICE_REG(voice, adsr_high) =
        (SPU_VOICE_REG(voice, adsr_high) & SPU_ADSR_RELEASE_MASK) |
        (((mode_bits >> SPU_ADSR_SUSTAIN_MODE_INPUT_SHIFT) << SPU_ADSR_SUSTAIN_MODE_SHIFT) | (sustain_bits << SPU_ADSR_SUSTAIN_RATE_SHIFT));
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
    SPU_VOICE_REG(voice, adsr_high) =
        (SPU_VOICE_REG(voice, adsr_high) & SPU_ADSR_SUSTAIN_MASK) | (((mode_bit >> SPU_ADSR_MODE_INPUT_SHIFT) << SPU_ADSR_RELEASE_MODE_SHIFT) | release_shift);
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
    s32 volume_left;
    s32 volume_right;
    u16* register_cursor;

    params->update_flags = 0;
    register_cursor = (u16*)&SPU_VOICE_REGS[voice];

    if (scale == 0)
    {
        volume_left = params->volume_left;
        volume_right = params->volume_right;
    }
    else
    {
        volume_left = params->volume_left * scale;
        volume_left >>= AKAO_Q7_SHIFT;
        volume_right = params->volume_right * scale;
        volume_right >>= AKAO_Q7_SHIFT;
    }

    *register_cursor++ = volume_left & SPU_DIRECT_VOLUME_MASK;
    *register_cursor++ = volume_right & SPU_DIRECT_VOLUME_MASK;
    *register_cursor++ = params->pitch;
    *register_cursor++ = params->sample_start_addr >> SPU_ADDRESS_UNIT_SHIFT;
    *register_cursor++ = params->adsr_low;
    *register_cursor++ = params->adsr_high;
    /* The envelope-level register is read-only. */
    register_cursor++;
    *register_cursor = params->sample_repeat_addr >> SPU_ADDRESS_UNIT_SHIFT;
}

/**
 * @brief Apply the pending register updates for one SPU voice.
 * @param voice Voice index (0-23).
 * @param params Pending SPU register image.
 * @param flags Unused.
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
    s32 pitch_slide;
    s32 next_value;
    u16 pitch_lfo_depth;
    s32 pan;
    u32 depth_level;
    u32 scaled_depth;

    if ((is_sfx == 0) && (channel->volume_fade_ticks != 0))
    {
        channel->volume_fade_ticks--;
        next_value = channel->volume + channel->volume_step;
        if ((next_value & AKAO_CHANNEL_VOLUME_MASK) != (channel->volume & AKAO_CHANNEL_VOLUME_MASK))
        {
            channel->update_flags |= SPU_UPDATE_VOLUME;
        }
        channel->volume = next_value;
    }

    if (channel->expression_fade_ticks != 0)
    {
        expression = channel->expression;
        channel->expression_fade_ticks--;
        next_value = expression + channel->expression_step;
        if ((next_value & AKAO_EXPRESSION_UPDATE_MASK) != (expression & AKAO_EXPRESSION_UPDATE_MASK))
        {
            channel->update_flags |= SPU_UPDATE_VOLUME;
        }
        channel->expression = next_value;
    }

    if (channel->pan_fade_ticks != 0)
    {
        pan = channel->pan;
        channel->pan_fade_ticks--;
        next_value = pan + channel->pan_step;
        if ((next_value & AKAO_Q8_LEVEL_MASK) != (pan & AKAO_Q8_LEVEL_MASK))
        {
            channel->update_flags |= SPU_UPDATE_VOLUME;
        }
        channel->pan = next_value;
    }

    if (channel->pitch_lfo_delay_ticks != 0)
    {
        channel->pitch_lfo_delay_ticks--;
    }

    if (channel->volume_lfo_delay_ticks != 0)
    {
        channel->volume_lfo_delay_ticks--;
    }

    if (channel->noise_toggle_ticks != 0)
    {
        channel->noise_toggle_ticks--;
        if (channel->noise_toggle_ticks == 0)
        {
            if (is_sfx == 0)
            {
                g_akao_seq_channel0->noise_mask ^= channel_bit;
            }
            else
            {
                g_akao_sfx_control.noise_mask ^= channel_bit;
            }
            g_akao_driver_flags.update_flags |= (AKAO_EFFECT_MASKS_UPDATE_PENDING | AKAO_NOISE_CLOCK_UPDATE_PENDING);
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
            g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
        }
    }

    if (channel->pitch_lfo_depth_fade_ticks != 0)
    {
        channel->pitch_lfo_depth_fade_ticks--;
        pitch_lfo_depth = channel->pitch_lfo_depth.raw + channel->pitch_lfo_depth_step;
        channel->pitch_lfo_depth.raw = pitch_lfo_depth;
        depth_level = (u32)(pitch_lfo_depth & AKAO_PITCH_LFO_DEPTH_MASK) >> AKAO_Q8_SHIFT;

        if (pitch_lfo_depth & AKAO_PITCH_LFO_ABSOLUTE_FLAG)
        {
            scaled_depth = (depth_level * channel->pitch) >> AKAO_Q7_SHIFT;
        }
        else
        {
            scaled_depth = (depth_level * ((u32)(channel->pitch * AKAO_PITCH_LFO_RELATIVE_SCALE) >> AKAO_Q8_SHIFT)) >> AKAO_Q7_SHIFT;
        }

        channel->pitch_lfo_depth.scaled = scaled_depth;

        if ((channel->pitch_lfo_delay_ticks == 0) && (channel->pitch_lfo_restart != AKAO_LFO_RESTART_PENDING))
        {
            lfo_cursor = (AkaoLfoSample*)channel->pitch_lfo_cursor;
            if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
            {
                lfo_cursor = (AkaoLfoSample*)((s16*)lfo_cursor + lfo_cursor->relative_offset);
            }

            next_value = (channel->pitch_lfo_depth.scaled * lfo_cursor->sample) >> AKAO_Q16_SHIFT;
            if (next_value != channel->pitch_lfo_value)
            {
                channel->pitch_lfo_value = next_value;
                channel->update_flags |= SPU_UPDATE_PITCH;
                if (next_value >= 0)
                {
                    channel->pitch_lfo_value = next_value * 2;
                }
            }
        }
    }

    if (channel->volume_lfo_depth_fade_ticks != 0)
    {
        channel->volume_lfo_depth_fade_ticks--;
        channel->volume_lfo_depth += channel->volume_lfo_depth_step;

        if ((channel->volume_lfo_delay_ticks == 0) && (channel->volume_lfo_restart != AKAO_LFO_RESTART_PENDING))
        {
            lfo_cursor = (AkaoLfoSample*)channel->volume_lfo_cursor;
            if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
            {
                lfo_cursor = (AkaoLfoSample*)((s16*)lfo_cursor + lfo_cursor->relative_offset);
            }

            next_value =
                (((HALF_HIGH_S16(channel->expression) * (channel->volume >> AKAO_Q8_SHIFT)) >> AKAO_Q7_SHIFT) * (channel->volume_lfo_depth >> AKAO_Q8_SHIFT)
                 << AKAO_VOLUME_LFO_PRECISION_SHIFT) >>
                AKAO_Q16_SHIFT;
            next_value = (next_value * lfo_cursor->sample) >> AKAO_Q15_SHIFT;

            if (next_value != channel->volume_lfo_value)
            {
                channel->volume_lfo_value = next_value;
                channel->update_flags |= SPU_UPDATE_VOLUME;
            }
        }
    }

    if (channel->pan_lfo_depth_fade_ticks != 0)
    {
        channel->pan_lfo_depth_fade_ticks--;
        channel->pan_lfo_depth += channel->pan_lfo_depth_step;

        if (channel->pan_lfo_restart != AKAO_LFO_RESTART_PENDING)
        {
            lfo_cursor = (AkaoLfoSample*)channel->pan_lfo_cursor;
            if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
            {
                lfo_cursor = (AkaoLfoSample*)((s16*)lfo_cursor + lfo_cursor->relative_offset);
            }

            next_value = ((channel->pan_lfo_depth >> AKAO_Q8_SHIFT) * lfo_cursor->sample) >> AKAO_Q15_SHIFT;

            if (next_value != channel->pan_lfo_value)
            {
                channel->pan_lfo_value = next_value;
                channel->update_flags |= SPU_UPDATE_VOLUME;
            }
        }
    }

    if (channel->pitch_slide_ticks != 0)
    {
        pitch_slide = channel->pitch_slide_acc;
        channel->pitch_slide_ticks--;
        next_value = pitch_slide + channel->pitch_slide_step;

        if ((next_value & AKAO_PITCH_SLIDE_LEVEL_MASK) != (pitch_slide & AKAO_PITCH_SLIDE_LEVEL_MASK))
        {
            channel->update_flags |= SPU_UPDATE_PITCH;
        }
        channel->pitch_slide_acc = next_value;
    }
}

/**
 * @brief Sample sequence-channel LFOs and recompute its pending SPU voice image.
 * @param channel Channel state to update.
 * @param channel_mask Unused.
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
    scaled_volume = (HALF_HIGH_S16(channel->expression) * (channel->volume >> AKAO_Q8_SHIFT)) >> AKAO_Q7_SHIFT;

    if ((effect_flags & AKAO_CH_PITCH_LFO) && (channel->pitch_lfo_delay_ticks == 0))
    {
        channel->pitch_lfo_restart--;
        if (channel->pitch_lfo_restart == 0)
        {
            AkaoLfoSample* lfo_cursor;
            s16* lfo_samples;

            channel->pitch_lfo_restart = channel->pitch_lfo_period;

            lfo_cursor = (AkaoLfoSample*)channel->pitch_lfo_cursor;
            if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
            {
                channel->pitch_lfo_cursor = (s16*)lfo_cursor + lfo_cursor->relative_offset;
            }

            lfo_samples = channel->pitch_lfo_cursor;
            waveform_sample = *lfo_samples++;
            effect_value = (channel->pitch_lfo_depth.scaled * waveform_sample) >> AKAO_Q16_SHIFT;
            channel->pitch_lfo_cursor = lfo_samples;

            if (effect_value != channel->pitch_lfo_value)
            {
                channel->pitch_lfo_value = effect_value;
                channel->update_flags |= SPU_UPDATE_PITCH;
                if (effect_value >= 0)
                {
                    channel->pitch_lfo_value = effect_value << 1;
                }
            }
        }
    }

    if ((effect_flags & AKAO_CH_VOLUME_LFO) && (channel->volume_lfo_delay_ticks == 0))
    {
        channel->volume_lfo_restart--;
        if (channel->volume_lfo_restart == 0)
        {
            AkaoLfoSample* lfo_cursor;
            s16* lfo_samples;

            channel->volume_lfo_restart = channel->volume_lfo_period;

            lfo_cursor = (AkaoLfoSample*)channel->volume_lfo_cursor;
            if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
            {
                channel->volume_lfo_cursor = (s16*)lfo_cursor + lfo_cursor->relative_offset;
            }

            effect_value = (scaled_volume * (channel->volume_lfo_depth >> AKAO_Q8_SHIFT) << AKAO_VOLUME_LFO_PRECISION_SHIFT) >> AKAO_Q16_SHIFT;
            lfo_samples = channel->volume_lfo_cursor;
            waveform_sample = *lfo_samples++;
            effect_value = (effect_value * waveform_sample) >> AKAO_Q15_SHIFT;
            channel->volume_lfo_cursor = lfo_samples;

            if (effect_value != channel->volume_lfo_value)
            {
                channel->volume_lfo_value = effect_value;
                channel->update_flags |= SPU_UPDATE_VOLUME;
            }
        }
    }

    if (effect_flags & AKAO_CH_PAN_LFO)
    {
        channel->pan_lfo_restart--;
        if (channel->pan_lfo_restart == 0)
        {
            AkaoLfoSample* lfo_cursor;
            s16* lfo_samples;

            channel->pan_lfo_restart = channel->pan_lfo_period;

            lfo_cursor = (AkaoLfoSample*)channel->pan_lfo_cursor;
            if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
            {
                channel->pan_lfo_cursor = (s16*)lfo_cursor + lfo_cursor->relative_offset;
            }

            lfo_samples = channel->pan_lfo_cursor;
            waveform_sample = *lfo_samples++;
            effect_value = ((channel->pan_lfo_depth >> AKAO_Q8_SHIFT) * waveform_sample) >> AKAO_Q15_SHIFT;
            channel->pan_lfo_cursor = lfo_samples;

            if (effect_value != channel->pan_lfo_value)
            {
                channel->pan_lfo_value = effect_value;
                channel->update_flags |= SPU_UPDATE_VOLUME;
            }
        }
    }

    if (effect_flags & AKAO_CH_PITCH_VOLUME_SIDECHAIN)
    {
        scaled_volume = ((s16)(channel[-1].spu_pitch << 1) * (channel->volume >> AKAO_Q8_SHIFT)) >> AKAO_Q7_SHIFT;
        channel->update_flags |= SPU_UPDATE_VOLUME;
    }

    if (channel->update_flags & SPU_UPDATE_VOLUME)
    {
        scaled_volume += channel->volume_lfo_value;
        scaled_volume = (scaled_volume * (HALF_HIGH_U16(g_akao_seq_channel0->volume) & AKAO_VOLUME_MAX)) >> AKAO_Q7_SHIFT;
        pan_index = ((channel->pan >> AKAO_Q8_SHIFT) + channel->pan_lfo_value) & AKAO_PAN_MASK;

        if (g_akao_driver_flags.output_mode == AKAO_OUTPUT_MONO)
        {
            channel->spu_volume_right = (scaled_volume * g_akao_pan_gain_table[AKAO_PAN_CENTER]) >> AKAO_Q15_SHIFT;
            channel->spu_volume_left = channel->spu_volume_right;
        }
        else
        {
            channel->spu_volume_left = (scaled_volume * g_akao_pan_gain_table[pan_index]) >> AKAO_Q15_SHIFT;
            channel->spu_volume_right = (scaled_volume * g_akao_pan_gain_table[pan_index ^ AKAO_PAN_MASK]) >> AKAO_Q15_SHIFT;
        }
    }

    if (effect_flags & AKAO_CH_PITCH_SIDECHAIN)
    {
        effect_value = HALF_HIGH_S16(channel->pitch_slide_acc);
        pitch_offset = channel[-1].spu_pitch + channel->pitch_lfo_value + effect_value;
        effect_value = g_akao_mastervol_acc & AKAO_MASTER_LEVEL_MASK;
        if (effect_value != 0)
        {
            effect_value >>= AKAO_Q16_SHIFT;
            if (effect_value < AKAO_PITCH_BEND_REDUCTION_THRESHOLD)
            {
                pitch_offset += (pitch_offset * effect_value) >> AKAO_Q7_SHIFT;
            }
            else
            {
                pitch_offset = (pitch_offset * effect_value) >> AKAO_Q8_SHIFT;
            }
        }
        channel->spu_pitch = (HALF_LOW_U16(channel->detune_pitch_delta) + pitch_offset) & SPU_PITCH_MASK;
        channel->update_flags |= SPU_UPDATE_PITCH;
        return;
    }

    if (channel->update_flags & SPU_UPDATE_PITCH)
    {
        pitch_offset = channel->pitch + channel->pitch_lfo_value + HALF_HIGH_S16(channel->pitch_slide_acc);
        effect_value = g_akao_mastervol_acc & AKAO_MASTER_LEVEL_MASK;
        if (effect_value != 0)
        {
            effect_value >>= AKAO_Q16_SHIFT;
            if (effect_value < AKAO_PITCH_BEND_REDUCTION_THRESHOLD)
            {
                pitch_offset += (pitch_offset * effect_value) >> AKAO_Q7_SHIFT;
            }
            else
            {
                pitch_offset = (pitch_offset * effect_value) >> AKAO_Q8_SHIFT;
            }
        }
        channel->spu_pitch = (HALF_LOW_U16(channel->detune_pitch_delta) + pitch_offset) & SPU_PITCH_MASK;
    }
}

/**
 * @brief Sample SFX-channel LFOs and recompute its pending SPU voice image.
 * @param channel Channel state to update.
 * @param channel_mask Unused.
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
    scaled_volume = (HALF_HIGH_S16(channel->expression) * (channel->volume >> AKAO_Q8_SHIFT)) >> AKAO_Q7_SHIFT;

    if (effect_flags & AKAO_CH_PITCH_LFO)
    {
        channel->pitch_lfo_restart--;
        if (channel->pitch_lfo_restart == 0)
        {
            AkaoLfoSample* lfo_cursor;
            s16* lfo_samples;

            channel->pitch_lfo_restart = channel->pitch_lfo_period;

            lfo_cursor = (AkaoLfoSample*)channel->pitch_lfo_cursor;
            if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
            {
                channel->pitch_lfo_cursor = (s16*)lfo_cursor + lfo_cursor->relative_offset;
            }

            lfo_samples = channel->pitch_lfo_cursor;
            waveform_sample = *lfo_samples++;
            effect_value = (channel->pitch_lfo_depth.scaled * waveform_sample) >> AKAO_Q16_SHIFT;
            channel->pitch_lfo_cursor = lfo_samples;

            if (effect_value != channel->pitch_lfo_value)
            {
                channel->pitch_lfo_value = effect_value;
                channel->update_flags |= SPU_UPDATE_PITCH;
                if (effect_value >= 0)
                {
                    channel->pitch_lfo_value = effect_value << 1;
                }
            }
        }
    }

    if (effect_flags & AKAO_CH_VOLUME_LFO)
    {
        channel->volume_lfo_restart--;
        if (channel->volume_lfo_restart == 0)
        {
            AkaoLfoSample* lfo_cursor;
            s16* lfo_samples;

            channel->volume_lfo_restart = channel->volume_lfo_period;

            lfo_cursor = (AkaoLfoSample*)channel->volume_lfo_cursor;
            if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
            {
                channel->volume_lfo_cursor = (s16*)lfo_cursor + lfo_cursor->relative_offset;
            }

            effect_value = (scaled_volume * (channel->volume_lfo_depth >> AKAO_Q8_SHIFT) << AKAO_VOLUME_LFO_PRECISION_SHIFT) >> AKAO_Q16_SHIFT;
            lfo_samples = channel->volume_lfo_cursor;
            waveform_sample = *lfo_samples++;
            effect_value = (effect_value * waveform_sample) >> AKAO_Q15_SHIFT;
            channel->volume_lfo_cursor = lfo_samples;

            if (effect_value != channel->volume_lfo_value)
            {
                channel->volume_lfo_value = effect_value;
                channel->update_flags |= SPU_UPDATE_VOLUME;
            }
        }
    }

    if (effect_flags & AKAO_CH_PAN_LFO)
    {
        channel->pan_lfo_restart--;
        if (channel->pan_lfo_restart == 0)
        {
            AkaoLfoSample* lfo_cursor;
            s16* lfo_samples;

            channel->pan_lfo_restart = channel->pan_lfo_period;

            lfo_cursor = (AkaoLfoSample*)channel->pan_lfo_cursor;
            if ((lfo_cursor->sample == 0) && (lfo_cursor->marker == 0))
            {
                channel->pan_lfo_cursor = (s16*)lfo_cursor + lfo_cursor->relative_offset;
            }

            lfo_samples = channel->pan_lfo_cursor;
            waveform_sample = *lfo_samples++;
            effect_value = ((channel->pan_lfo_depth >> AKAO_Q8_SHIFT) * waveform_sample) >> AKAO_Q15_SHIFT;
            channel->pan_lfo_cursor = lfo_samples;

            if (effect_value != channel->pan_lfo_value)
            {
                channel->pan_lfo_value = effect_value;
                channel->update_flags |= SPU_UPDATE_VOLUME;
            }
        }
    }

    if (effect_flags & AKAO_CH_PITCH_VOLUME_SIDECHAIN)
    {
        scaled_volume = ((s16)(channel[-1].spu_pitch << 1) * (channel->volume >> AKAO_Q8_SHIFT)) >> AKAO_Q7_SHIFT;
        channel->update_flags |= SPU_UPDATE_VOLUME;
    }

    if (channel->update_flags & SPU_UPDATE_VOLUME)
    {
        scaled_volume += channel->volume_lfo_value;
        if (channel->sfx_tag & AKAO_SFX_FLAG_SUPPRESS)
        {
            pan_index = AKAO_PAN_CENTER;
        }
        else
        {
            s32 biased_pan;
            scaled_volume = (scaled_volume * (s8)(channel->volume_scale >> AKAO_Q8_SHIFT)) >> AKAO_Q7_SHIFT;
            biased_pan = ((channel->pan + channel->pan_bias) >> AKAO_Q8_SHIFT) + channel->pan_lfo_value;
            pan_index = biased_pan + AKAO_PAN_CENTER;
            pan_index &= AKAO_PAN_MASK;
        }

        if (g_akao_driver_flags.output_mode == AKAO_OUTPUT_MONO)
        {
            channel->spu_volume_right = (scaled_volume * g_akao_pan_gain_table[AKAO_PAN_CENTER]) >> AKAO_Q15_SHIFT;
            channel->spu_volume_left = channel->spu_volume_right;
        }
        else
        {
            channel->spu_volume_left = (scaled_volume * g_akao_pan_gain_table[pan_index]) >> AKAO_Q15_SHIFT;
            channel->spu_volume_right = (scaled_volume * g_akao_pan_gain_table[pan_index ^ AKAO_PAN_MASK]) >> AKAO_Q15_SHIFT;
        }
    }

    if (effect_flags & AKAO_CH_PITCH_SIDECHAIN)
    {
        effect_value = HALF_HIGH_S16(channel->pitch_slide_acc);
        pitch_offset = channel[-1].spu_pitch + channel->pitch_lfo_value + effect_value;
        if (!(channel->sfx_tag & AKAO_SFX_FLAG_SUPPRESS))
        {
            effect_value = channel->sfx_pitch_bend & AKAO_Q8_LEVEL_MASK;
            if (effect_value != 0)
            {
                effect_value >>= AKAO_Q8_SHIFT;
                if (effect_value < AKAO_PITCH_BEND_REDUCTION_THRESHOLD)
                {
                    pitch_offset += (pitch_offset * effect_value) >> AKAO_Q7_SHIFT;
                }
                else
                {
                    pitch_offset = (pitch_offset * effect_value) >> AKAO_Q8_SHIFT;
                }
            }
        }
        channel->spu_pitch = (HALF_LOW_U16(channel->detune_pitch_delta) + pitch_offset) & SPU_PITCH_MASK;
        channel->update_flags |= SPU_UPDATE_PITCH;
        return;
    }

    if (channel->update_flags & SPU_UPDATE_PITCH)
    {
        pitch_offset = channel->pitch + channel->pitch_lfo_value + HALF_HIGH_S16(channel->pitch_slide_acc);
        if (!(channel->sfx_tag & AKAO_SFX_FLAG_SUPPRESS))
        {
            effect_value = channel->sfx_pitch_bend & AKAO_Q8_LEVEL_MASK;
            if (effect_value != 0)
            {
                effect_value >>= AKAO_Q8_SHIFT;
                if (effect_value < AKAO_PITCH_BEND_REDUCTION_THRESHOLD)
                {
                    pitch_offset += (pitch_offset * effect_value) >> AKAO_Q7_SHIFT;
                }
                else
                {
                    pitch_offset = (pitch_offset * effect_value) >> AKAO_Q8_SHIFT;
                }
            }
        }
        channel->spu_pitch = (HALF_LOW_U16(channel->detune_pitch_delta) + pitch_offset) & SPU_PITCH_MASK;
    }
}

/**
 * @brief Select and release the quietest allocated voice.
 * @param ignore_voice_reserve Nonzero to include voices below the reserve floor.
 * @return Selected voice index, or AKAO_VOICE_COUNT when no voice is available.
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

    quietest_level = AKAO_VOICE_ENVELOPE_UNAVAILABLE;
    quietest_voice = AKAO_VOICE_COUNT;
    allocation = &g_akao_voice_allocations[voice_index];

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

    if ((s16)quietest_level == AKAO_VOICE_ENVELOPE_UNAVAILABLE)
    {
        return AKAO_VOICE_COUNT;
    }

    akao_clear_voice_assignment(g_akao_seq_channels, quietest_voice);
    return quietest_voice;
}

/**
 * @brief Find the first unallocated voice in the requested voice range.
 * @param ignore_voice_reserve Nonzero to include voices below the reserve floor.
 * @return Available voice index, or AKAO_VOICE_COUNT when none is available.
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

    for (allocation = &g_akao_voice_allocations[voice_index]; allocation->envelope_level != 0; allocation++)
    {
        voice_index++;
        if (voice_index >= AKAO_VOICE_COUNT)
        {
            break;
        }
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

    channel_bit = 1;
    channel_index = 0;
    pending_key_on_mask = channel_mask & g_akao_seq_channel0->masks.key_on_mask;
    do
    {
        if (channel_mask & channel_bit)
        {
            akao_update_sequence_channel_voice(channels, channel_bit);
            if (channels->update_flags != 0)
            {
                if (g_akao_muted_channel_mask & channel_bit)
                {
                    channels->spu_volume_right = 0;
                    channels->spu_volume_left = 0;
                }
                if (pending_key_on_mask & channel_bit)
                {
                    if (static_voice_mask & channel_bit)
                    {
                        *key_on_voice_mask |= 1 << channel_index;
                        channels->voice = channel_index;
                    }
                    else
                    {
                        s32 ignore_voice_reserve;
                        ignore_voice_reserve = (g_akao_seq_channel0->masks.voice_alloc_low_mask & channel_bit) != 0;
                        voice_index = akao_find_free_voice(ignore_voice_reserve);
                        if (voice_index == AKAO_VOICE_COUNT)
                        {
                            g_akao_seq_channel0->flags |= AKAO_SONG_VOICE_STOLEN;
                            voice_index = akao_steal_quietest_voice(ignore_voice_reserve);
                            if (voice_index == AKAO_VOICE_COUNT)
                            {
                                channels->voice = voice_index;
                                g_akao_seq_channel0->flags |= AKAO_SONG_VOICE_DROPPED;
                            }
                            else
                            {
                                *key_on_voice_mask |= 1 << voice_index;
                                channels->voice = voice_index;
                                g_akao_voice_allocations[voice_index].envelope_level = AKAO_VOICE_ENVELOPE_UNAVAILABLE;
                            }
                        }
                        else
                        {
                            *key_on_voice_mask |= 1 << voice_index;
                            channels->voice = voice_index;
                            g_akao_voice_allocations[voice_index].envelope_level = AKAO_VOICE_ENVELOPE_UNAVAILABLE;
                        }
                    }
                    if (channels->voice < AKAO_VOICE_COUNT)
                    {
                        spu_write_voice_params(channels->voice, (SpuVoiceParams*)&channels->voice, channels->spu_volume_scale);
                        g_akao_voice_owners[channels->voice] = g_akao_seq_channel0;
                        g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
                    }
                }
                else if (channels->voice < AKAO_VOICE_COUNT)
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
    u32 channel_index;
    AkaoChannelState* channel;

    channel = channels;
    for (channel_index = 0; channel_index < AKAO_CHANNEL_COUNT; channel_index++, channel++)
    {
        if (voice_index == channel->voice)
        {
            channel->voice = AKAO_VOICE_COUNT;
        }
    }

    if (g_akao_seq_channel1 != NULL)
    {
        channel = g_akao_pending_channels;
        for (channel_index = 0; channel_index < AKAO_CHANNEL_COUNT; channel_index++, channel++)
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
 * @param xa_voice_mask Unused.
 */
void akao_refresh_voice_allocation_state(u32 reserved_voice_mask, s32 xa_voice_mask)
{
    u32 unavailable_voice_mask;
    u32 voice_index;
    AkaoVoiceAllocation* allocation;

    unavailable_voice_mask = (g_akao_seq_channel0->masks.active_mask & g_akao_seq_channel0->masks.static_voice_mask) | reserved_voice_mask;
    if (g_akao_seq_channel1 != NULL)
    {
        unavailable_voice_mask |= g_akao_seq_channel1->masks.active_mask & g_akao_seq_channel1->masks.static_voice_mask;
    }

    for (voice_index = 0, allocation = g_akao_voice_allocations; voice_index < AKAO_VOICE_COUNT; voice_index++, allocation++)
    {
        if (unavailable_voice_mask & (1 << voice_index))
        {
            allocation->envelope_level = AKAO_VOICE_ENVELOPE_UNAVAILABLE;
        }
        else
        {
            SpuGetVoiceEnvelope(voice_index, &allocation->envelope_level);
            if (allocation->envelope_level == 0)
            {
                akao_clear_voice_assignment(g_akao_seq_channels, voice_index);
            }
        }
    }
}

/**
 * @brief Flush pending sequence, SFX, and global SPU voice updates.
 * @param sfx_update_mask Unused.
 * @see decomp.me (100%) https://decomp.me/scratch/PzQRP
 */
void akao_flush_voice_updates(s32 sfx_update_mask)
{
    u32 key_on_voice_mask;
    s32 reserved_voice_mask;
    s32 secondary_update_mask;
    s32 secondary_static_voice_mask;
    s32 secondary_low_voice_mask;
    s32 update_mask;
    s32 voice_mask;
    s32 primary_low_voice_mask;
    AkaoChannelState* sfx_channel;
    AkaoSongMasks* song_masks;
    AkaoSongState* secondary_song;
    s32 reverb_depth;
    s32 noise_frequency;

    secondary_update_mask = 0;
    secondary_static_voice_mask = 0;
    key_on_voice_mask = 0;
    reserved_voice_mask = (g_akao_sfx_control.active_mask | g_akao_sfx_control.paused_mask) | g_akao_xa_tracker.voice_mask;

    if ((g_akao_seq_channel0->masks.active_mask & g_akao_seq_channel0->masks.key_on_mask) ||
        ((g_akao_seq_channel1 != NULL) && (g_akao_seq_channel1->masks.active_mask & g_akao_seq_channel1->masks.key_on_mask)))
    {
        akao_refresh_voice_allocation_state(reserved_voice_mask, g_akao_xa_tracker.voice_mask);
    }

    if (g_akao_seq_channel1 != NULL)
    {
        g_akao_seq_channel0 = g_akao_seq_channel1;
        secondary_update_mask =
            g_akao_seq_channel1->masks.active_mask & g_akao_seq_channel1->note_on_mask & ~(g_akao_seq_channel1->masks.static_voice_mask & reserved_voice_mask);
        secondary_static_voice_mask = g_akao_seq_channel1->masks.static_voice_mask;
        secondary_low_voice_mask = secondary_update_mask & g_akao_seq_channel1->masks.voice_alloc_low_mask;
        secondary_static_voice_mask = secondary_update_mask & secondary_static_voice_mask & ~reserved_voice_mask;
        if (secondary_low_voice_mask != 0)
        {
            akao_process_sequence_voice_updates(g_akao_pending_channels, secondary_low_voice_mask, secondary_static_voice_mask, &key_on_voice_mask);
            song_masks = &g_akao_seq_channel0->masks;
            secondary_update_mask &= ~song_masks->voice_alloc_low_mask;
            g_akao_seq_channel0->masks.key_on_mask &= ~g_akao_seq_channel0->masks.voice_alloc_low_mask;
        }
        g_akao_seq_channel0 = &g_akao_seq_master_state;
    }

    update_mask = g_akao_seq_channel0->masks.active_mask & g_akao_seq_channel0->note_on_mask &
                  ~(g_akao_seq_channel0->masks.static_voice_mask & (secondary_static_voice_mask | reserved_voice_mask));
    voice_mask = g_akao_seq_channel0->masks.static_voice_mask;
    primary_low_voice_mask = update_mask & g_akao_seq_channel0->masks.voice_alloc_low_mask;
    voice_mask = update_mask & voice_mask & ~(secondary_static_voice_mask | reserved_voice_mask);
    if (primary_low_voice_mask != 0)
    {
        akao_process_sequence_voice_updates(g_akao_seq_channels, primary_low_voice_mask, voice_mask, &key_on_voice_mask);
        song_masks = &g_akao_seq_channel0->masks;
        update_mask &= ~song_masks->voice_alloc_low_mask;
        g_akao_seq_channel0->masks.key_on_mask &= ~g_akao_seq_channel0->masks.voice_alloc_low_mask;
    }

    if ((g_akao_seq_channel1 != NULL) && (secondary_update_mask != 0))
    {
        g_akao_seq_channel0 = g_akao_seq_channel1;
        akao_process_sequence_voice_updates(g_akao_pending_channels, secondary_update_mask, secondary_static_voice_mask & ~voice_mask, &key_on_voice_mask);
        secondary_song = g_akao_seq_channel0;
        g_akao_seq_channel0 = &g_akao_seq_master_state;
        secondary_song->masks.key_on_mask = 0;
    }

    if (update_mask != 0)
    {
        akao_process_sequence_voice_updates(g_akao_seq_channels, update_mask, voice_mask, &key_on_voice_mask);
        g_akao_seq_channel0->masks.key_on_mask = 0;
    }

    /* SFX channel bits map directly to their SPU voice bits. */
    update_mask = g_akao_sfx_control.active_mask & g_akao_sfx_control.note_on_mask;
    if (update_mask != 0)
    {
        key_on_voice_mask |= g_akao_sfx_control.key_on_mask;
        for (voice_mask = AKAO_SFX_FIRST_CHANNEL_BIT, sfx_channel = g_sfx_channels; update_mask != 0; voice_mask <<= 1, sfx_channel++)
        {
            if (update_mask & voice_mask)
            {
                akao_update_sfx_channel_voice(sfx_channel, voice_mask);
                if (sfx_channel->update_flags != 0)
                {
                    spu_apply_voice_updates(sfx_channel->voice, (SpuVoiceParams*)&sfx_channel->voice, sfx_channel->flags);
                }
                update_mask &= ~voice_mask;
            }
        }
        g_akao_sfx_control.key_on_mask = 0;
    }

    update_mask = g_akao_driver_flags.update_flags;
    if (update_mask & AKAO_REVERB_DEPTH_UPDATE_PENDING)
    {
        reverb_depth = (s32)(g_akao_seq_channel0->reverb_depth << (AKAO_Q16_SHIFT - AKAO_REVERB_DEPTH_FRACTION_BITS)) >> AKAO_Q16_SHIFT;
        SpuSetReverbModeDepth(reverb_depth, reverb_depth);
        g_akao_driver_flags.update_flags &= ~AKAO_REVERB_DEPTH_UPDATE_PENDING;
    }

    if (update_mask & AKAO_NOISE_CLOCK_UPDATE_PENDING)
    {
        if (g_akao_sfx_control.active_mask != 0)
        {
            noise_frequency = g_akao_sfx_control.noise_freq;
        }
        else
        {
            noise_frequency = g_akao_seq_channel0->noise_freq;
        }
        SpuSetNoiseClock(noise_frequency);
        g_akao_driver_flags.update_flags &= ~AKAO_NOISE_CLOCK_UPDATE_PENDING;
    }

    if (update_mask & AKAO_EFFECT_MASKS_UPDATE_PENDING)
    {
        akao_build_effect_voice_mask(&g_akao_effect_voice_masks[AKAO_VOICE_EFFECT_NOISE], g_akao_seq_channel1->noise_mask, g_akao_seq_channel0->noise_mask,
                                     g_akao_sfx_control.noise_mask);
        akao_build_effect_voice_mask(&g_akao_effect_voice_masks[AKAO_VOICE_EFFECT_REVERB], g_akao_seq_channel1->reverb_mask, g_akao_seq_channel0->reverb_mask,
                                     g_akao_sfx_control.reverb_mask);
        akao_build_effect_voice_mask(&g_akao_effect_voice_masks[AKAO_VOICE_EFFECT_PITCH_MODULATION], g_akao_seq_channel1->pitch_mod_mask,
                                     g_akao_seq_channel0->pitch_mod_mask, g_akao_sfx_control.pitch_mod_mask);
        spu_set_reverb_enable(g_akao_effect_voice_masks[AKAO_VOICE_EFFECT_REVERB]);
        spu_set_noise_enable(g_akao_effect_voice_masks[AKAO_VOICE_EFFECT_NOISE]);
        spu_set_pitch_modulation_enable(g_akao_effect_voice_masks[AKAO_VOICE_EFFECT_PITCH_MODULATION]);
        g_akao_driver_flags.update_flags &= ~AKAO_EFFECT_MASKS_UPDATE_PENDING;
    }

    if (key_on_voice_mask != 0)
    {
        spu_set_key_on(key_on_voice_mask);
    }
}

/**
 * @brief Add selected channels' assigned voices to a mask, then apply keep_mask.
 * @note Channels without an assigned voice contribute no bits.
 *
 * @param channels First channel corresponding to channel_mask bit zero.
 * @param voice_mask Accumulator receiving the collected voice bits.
 * @param channel_mask Channels to scan.
 * @param keep_mask Mask ANDed into the result once scanning completes.
 */
void akao_collect_channel_voice_mask(AkaoChannelState* channels, u32* voice_mask, s32 channel_mask, s32 keep_mask)
{
    s32 channel_bit;
    u32 voice;

    channel_bit = 1;
    do
    {
        if (channel_mask & channel_bit)
        {
            voice = channels->voice;
            if (voice < AKAO_VOICE_COUNT)
            {
                *voice_mask |= 1 << voice;
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
 * Each song's channels below the voice-allocation floor are gathered first.
 * Voices reserved by SFX or XA are excluded from the song masks. SFX key-offs
 * are then added, and the processed pending masks are cleared.
 */
void akao_flush_voice_key_offs(void)
{
    u32 key_off_voice_mask;
    s32 keep_mask;
    s32 secondary_residual;
    s32 primary_residual;
    s32 secondary_active;
    s32 primary_active;
    AkaoSongMasks* song_masks;

    secondary_residual = 0;
    key_off_voice_mask = 0;
    keep_mask = ~((g_akao_sfx_control.active_mask | g_akao_sfx_control.paused_mask) | g_akao_xa_tracker.voice_mask);

    if (g_akao_seq_channel1 != NULL)
    {
        secondary_residual = g_akao_seq_channel1->key_off_mask;
        secondary_active = secondary_residual & g_akao_seq_channel1->masks.voice_alloc_low_mask;
        if (secondary_active != 0)
        {
            akao_collect_channel_voice_mask(g_akao_pending_channels, &key_off_voice_mask, secondary_active, keep_mask);
            song_masks = &g_akao_seq_channel1->masks;
            secondary_residual &= ~song_masks->voice_alloc_low_mask;
            g_akao_seq_channel1->key_off_mask &= ~g_akao_seq_channel1->masks.voice_alloc_low_mask;
        }
    }

    primary_residual = g_akao_seq_channel0->key_off_mask;
    primary_active = primary_residual & g_akao_seq_channel0->masks.voice_alloc_low_mask;
    if (primary_active != 0)
    {
        akao_collect_channel_voice_mask(g_akao_seq_channels, &key_off_voice_mask, primary_active, keep_mask);
        song_masks = &g_akao_seq_channel0->masks;
        primary_residual &= ~song_masks->voice_alloc_low_mask;
        g_akao_seq_channel0->key_off_mask &= ~g_akao_seq_channel0->masks.voice_alloc_low_mask;
    }

    if ((g_akao_seq_channel1 != NULL) && (secondary_residual != 0))
    {
        akao_collect_channel_voice_mask(g_akao_pending_channels, &key_off_voice_mask, secondary_residual, keep_mask);
        g_akao_seq_channel1->key_off_mask = 0;
    }

    if (primary_residual != 0)
    {
        akao_collect_channel_voice_mask(g_akao_seq_channels, &key_off_voice_mask, primary_residual, keep_mask);
        g_akao_seq_channel0->key_off_mask = 0;
    }

    key_off_voice_mask |= g_akao_sfx_control.key_off_mask;
    g_akao_sfx_control.key_off_mask = 0;
    if (key_off_voice_mask != 0)
    {
        spu_set_key_off(key_off_voice_mask);
    }
}

/**
 * @brief Build the SPU voice bitmap for one per-voice effect register.
 *
 * Only active, effect-enabled song channels contribute voices. Voices reserved
 * by SFX or XA are excluded before adding sfx_effect_voices. The result marks
 * the driver's reverb, noise and pitch-modulation masks for a hardware update.
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
    keep_mask = ~((g_akao_sfx_control.active_mask | g_akao_sfx_control.paused_mask) | g_akao_xa_tracker.voice_mask);

    if (g_akao_seq_channel1 != NULL)
    {
        secondary_residual = g_akao_seq_channel1->masks.active_mask & secondary_effect_mask;
        secondary_active = secondary_residual & g_akao_seq_channel1->masks.voice_alloc_low_mask;
        if (secondary_active != 0)
        {
            akao_collect_channel_voice_mask(g_akao_pending_channels, &voice_mask, secondary_active, keep_mask);
            secondary_residual &= ~g_akao_seq_channel1->masks.voice_alloc_low_mask;
        }
    }

    primary_residual = g_akao_seq_channel0->masks.active_mask & primary_effect_mask;
    primary_active = primary_residual & g_akao_seq_channel0->masks.voice_alloc_low_mask;
    if (primary_active != 0)
    {
        akao_collect_channel_voice_mask(g_akao_seq_channels, &voice_mask, primary_active, keep_mask);
        primary_residual &= ~g_akao_seq_channel0->masks.voice_alloc_low_mask;
    }

    if ((g_akao_seq_channel1 != NULL) && (secondary_residual != 0))
    {
        akao_collect_channel_voice_mask(g_akao_pending_channels, &voice_mask, secondary_residual, keep_mask);
    }

    if (primary_residual != 0)
    {
        akao_collect_channel_voice_mask(g_akao_seq_channels, &voice_mask, primary_residual, keep_mask);
    }

    voice_mask |= sfx_effect_voices;
    *effect_voices = voice_mask;
    g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
}
