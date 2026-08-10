#include "decomp9.h"

typedef struct
{
    s16 value;
    s16 unk2;
    s16 relative_offset;
} AkaoLfoSample;

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
 * decomp.me (100%) https://decomp.me/scratch/ORS8e
 */
void spu_apply_voice_updates(s32 voice, SpuVoiceParams* params)
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
