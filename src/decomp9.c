#include "decomp9.h"

typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    u16 unk10;
    s16 unk12;
    s16 unk14;
    u16 unk16;
    s16 unk18;
    s16 unk1A;
} s_struct;

/**
 * @brief Write the SPU Key ON / Key OFF registers.
 *
 * Low 16 bits of @p key_mask set voices to key-on; high 16 bits set voices
 * to key-off.  Both are written atomically.
 *
 * @param key_mask Bitmask: bits 0-23 = key-on, bits 16-39 = key-off.
 * @see decomp.me (100%) https://decomp.me/scratch/lKkom
 */
void spu_set_key(u32 key_mask)
{
    *(u16*)0x1F801D88 = key_mask;
    *(u16*)0x1F801D8A = key_mask >> 0x10;
}

/**
 * @brief Write the SPU Channel FM Mode and Noise Mode registers.
 *
 * Low 16 bits of @p mode_mask go to FM Mode (0x1F801D8C), high 16 bits to
 * Noise Mode (0x1F801D8E).  Each bit enables the corresponding mode for
 * that voice.
 *
 * @param mode_mask Bitmask: low 16 = FM mode, high 16 = Noise mode.
 * @see decomp.me (100%) https://decomp.me/scratch/957fv
 */
void spu_set_voice_mode(u32 mode_mask)
{
    *(u16*)0x1F801D8C = mode_mask;
    *(s16*)0x1F801D8E = (s16)(mode_mask >> 0x10);
}

/**
 * @brief Write the SPU Reverb Work Area Start Address and IRQ Address.
 *
 * Low 16 bits go to the reverb work area base (0x1F801D98), high 16 bits
 * to the IRQ address (0x1F801D9A).  Both are in 8-word units.
 *
 * @param addrs Packed address pair.
 * @see decomp.me (100%) https://decomp.me/scratch/3KFgT
 */
void spu_set_reverb_addrs(u32 addrs)
{
    *(u16*)0x1F801D98 = addrs;
    *(s16*)0x1F801D9A = (s16)(addrs >> 0x10);
}

/**
 * @brief Write two adjacent SPU reverb control registers (0x1F801D94 /
 *        0x1F801D96).
 *
 * @param val Packed value; low 16 bits to 0x1F801D94, high 16 bits to
 *            0x1F801D96.
 * @see decomp.me (100%) https://decomp.me/scratch/HFDSO
 */
void spu_set_reverb_control(u32 val)
{
    *(u16*)0x1F801D94 = val;
    *(s16*)0x1F801D96 = (s16)(val >> 0x10);
}

/**
 * @brief Write the SPU Channel Reverb Mode registers (0x1F801D90 /
 *        0x1F801D92).
 *
 * @param mode_mask Packed bitmask; low 16 bits to Reverb Mode, high 16
 *                  bits to the paired status/mode register.
 * @see decomp.me (100%) https://decomp.me/scratch/ZyBKt
 */
void spu_set_reverb_mode(u32 mode_mask)
{
    *(u16*)0x1F801D90 = mode_mask;
    *(s16*)0x1F801D92 = (s16)(mode_mask >> 0x10);
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
 * @brief Set the loop/repeat address (RADDR) for a single SPU voice.
 *
 * The address is right-shifted by 3 (SPU addresses are in 8-byte units).
 *
 * @param voice Voice index (0-23).
 * @param addr Loop start address in SPU RAM (byte address; will be >> 3).
 * @see decomp.me (100%) https://decomp.me/scratch/UI7qr
 */
void spu_set_voice_loop_addr(s32 voice, u32 addr)
{
    s32 ptr = (s32)0x1F801C0E;
    voice = voice << 4;
    *(s16*)(ptr + voice) = (s16)(addr >> 3);
}

/**
 * @brief Set the full ADSR1 register for a single SPU voice.
 *
 * ADSR1 fields: Attack Rate (bits 12-15), Decay Rate (bits 8-11),
 * Sustain Level (bits 4-7), Sustain Rate (bits 0-3).
 *
 * @param voice Voice index (0-23).
 * @param adsr1 Raw 16-bit ADSR1 value.
 * @see decomp.me (100%) https://decomp.me/scratch/ghHQZ
 */
void spu_set_voice_adsr1(s32 voice, u16 adsr1)
{
    s32 ptr = (s32)0x1F801C08;
    voice = voice << 4;
    *(s16*)(ptr + voice) = adsr1;
}

/**
 * @brief Set the full ADSR2 register for a single SPU voice.
 *
 * ADSR2 fields: Sustain Rate direction/mode, Release Mode, Release Rate
 * (bits 0-5).
 *
 * @param voice Voice index (0-23).
 * @param adsr2 Raw 16-bit ADSR2 value.
 * @see decomp.me (100%) https://decomp.me/scratch/aDnJj
 */
void spu_set_voice_adsr2(s32 voice, u16 adsr2)
{
    s32 ptr = (s32)0x1F801C0A;
    voice = voice << 4;
    *(s16*)(ptr + voice) = adsr2;
}

/**
 * @brief Set the Attack Rate and Decay Rate fields of ADSR1 for a single
 *        SPU voice, preserving the low byte (Sustain Level / Sustain Rate).
 *
 * The high byte of ADSR1 is built from:
 *   - @p attack_decay shifted left 8 (Attack Rate in bits 12-15,
 *     Decay Rate in bits 8-11)
 *   - @p mode_bits right-shifted 2 then placed at bit 15 (Attack Rate Mode).
 *
 * @param voice Voice index (0-23).
 * @param attack_decay Packed Attack/Decay rate nybbles.
 * @param mode_bits Mode flags; bit 2 maps to ADSR1 bit 15 (Attack Mode).
 * @see decomp.me (100%) https://decomp.me/scratch/Ua4UK
 */
void spu_set_voice_attack_decay(s32 voice, s32 attack_decay, u32 mode_bits)
{
    s32 temp_a0;
    s32 ptr = (s32)0x1F801C08;

    voice = voice << 4;

    *(s16*)(ptr + voice) = (*(u8*)(ptr + voice)) | (((mode_bits >> 2) << 0xF) | (attack_decay << 8));
}

/**
 * @brief Set only the Sustain Rate field of ADSR1 for a single SPU voice.
 *
 * Preserves Attack/Decay (bits 8-15) and Sustain Level (bits 0-3); sets
 * Sustain Rate in bits 4-7 from @p sustain_rate << 4.
 *
 * @param voice Voice index (0-23).
 * @param sustain_rate Sustain Rate nybble (0-15).
 * @see decomp.me (100%) https://decomp.me/scratch/ymuym
 */
void spu_set_voice_sustain_rate(s32 voice, s32 sustain_rate)
{
    s32 temp_a0;
    s32 ptr = (s32)0x1F801C08;

    voice = voice << 4;
    sustain_rate = sustain_rate << 4;

    *(s16*)(ptr + voice) = ((*(s16*)(ptr + voice)) & 0xFF0F) | (sustain_rate);
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
 * @brief Set the upper fields of ADSR2 (Sustain Rate mode / Release Mode)
 *        for a single SPU voice, preserving the Release Rate (bits 0-5).
 *
 * @param voice Voice index (0-23).
 * @param sr_bits Sustain Rate / Release mode bits, placed at bit 6.
 * @param mode_bits Mode flags; bit 1 maps to ADSR2 bit 14.
 * @see decomp.me (100%) https://decomp.me/scratch/ZWKKM
 */
void spu_set_voice_sr_mode(s32 voice, s32 sr_bits, u32 mode_bits)
{
    s32 temp_a0;
    s32 ptr = (s32)0x1F801C0A;

    voice = voice << 4;

    *(s16*)(ptr + voice) = ((*(s16*)(ptr + voice)) & 0x3F) | (((mode_bits >> 1) << 0xE) | (sr_bits << 6));
}

/**
 * @brief Set only the Release Rate field of ADSR2 for a single SPU voice,
 *        preserving the upper bits (Sustain/Release mode).
 *
 * @param voice Voice index (0-23).
 * @param release_rate Release Rate value (placed in bits 0-5).
 * @param mode_bit Extra mode bit; bit 2 maps to ADSR2 bit 5.
 * @see decomp.me (100%) https://decomp.me/scratch/cztam
 */
void spu_set_voice_release_rate(s32 voice, s32 release_rate, u32 mode_bit)
{
    s32 temp_a0;
    s32 ptr = (s32)0x1F801C0A;

    voice = voice << 4;

    *(s16*)(ptr + voice) = ((*(s16*)(ptr + voice)) & 0xFFC0) | (((mode_bit >> 2) << 0x5) | (release_rate));
}

/**
 * @brief Bulk-configure a single SPU voice from a @ref SpuVoiceSetup struct.
 *
 * Writes VOLL, VOLR, PITCH, ADDR (start), ADSR1, ADSR2, and RADDR (loop)
 * in one shot.  First clears @c attr->status to 0.  If @p scale is
 * non-zero, the volume fields are multiplied by @p scale then shifted
 * right by 7 (fixed-point scaling).
 *
 * @param voice Voice index (0-23).
 * @param attr  Pointer to packed voice configuration.
 * @param scale If non-zero, fixed-point volume scale factor.
 * @see decomp.me (100%) https://decomp.me/scratch/MkQTS
 */
void spu_set_voice_attr(s32 voice, SpuVoiceSetup* attr, s32 scale)
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

    attr->status = 0;
    ptr = (s32)0x1F801C00;
    voice = voice << 4;
    voice = (voice + ptr);

    if (scale == 0)
    {
        scaled_l = attr->vol_l;
        scaled_r = attr->vol_r;
    }
    else
    {
        scaled_l = (attr->vol_l * scale);
        scaled_l = scaled_l >> 7;
        scaled_r = (attr->vol_r * scale);
        scaled_r = scaled_r >> 7;
    }

    *(s16*)voice = scaled_l & 0x7FFF;

    voice += 2;
    temp_a0_3 = voice;
    *(s16*)(voice) = (scaled_r & 0x7FFF);

    voice += 2;
    temp_a0_4 = temp_a0_3 + 2;
    *(s16*)(voice) = (u16)attr->pitch;

    voice += 2;
    temp_a0_5 = temp_a0_4 + 2;
    *(s16*)(voice) = (s16)((u32)attr->start_addr >> 3);

    voice += 2;
    temp_a0_6 = temp_a0_5 + 2;
    *(s16*)(voice) = (u16)attr->adsr1;

    voice += 2;
    *(s16*)(voice) = (u16)attr->adsr2;

    voice += 4;
    *(s16*)(voice) = (s16)((u32)attr->loop_addr >> 3);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/ORS8e
 */
void func_80024544(s32 arg0, s_struct* arg1)
{
    s32 var_s0 = arg1->unk4;

    if (var_s0 == 0)
    {
        return;
    }

    arg1->unk4 = 0;

    // Handle Pitch (0x10)
    if (var_s0 & 0x10)
    {
        var_s0 &= ~0x10;
        spu_set_voice_pitch(arg0, arg1->unk10);
        if (var_s0 == 0)
            return;
    }

    // Handle Volume (0x03)
    if (var_s0 & 3)
    {
        var_s0 &= ~3;
        spu_set_voice_volume(arg0, (u32)arg1->unk18, (u32)arg1->unk1A, (s32)arg1->unk16);
        if (var_s0 == 0)
            return;
    }

    // Handle Start Address (0x80)
    if (var_s0 & 0x80)
    {
        var_s0 &= ~0x80;
        spu_set_voice_start_addr(arg0, arg1->unk8);
        if (var_s0 == 0)
            return;
    }

    // Handle Loop Address (0x10000)
    if (var_s0 & 0x10000)
    {
        var_s0 &= 0xFFFEFFFF;
        spu_set_voice_loop_addr(arg0, arg1->unkC);
        if (var_s0 == 0)
            return;
    }

    // Handle ADSR2 (0x6600)
    if (var_s0 & 0x6600)
    {
        var_s0 &= ~0x6600;
        spu_set_voice_adsr2(arg0, (s16)arg1->unk14);
        if (var_s0 == 0)
            return;
    }

    // Handle ADSR1 (0x9900)
    if (var_s0 & 0x9900)
    {
        spu_set_voice_adsr1(arg0, (s16)arg1->unk12);
    }
}