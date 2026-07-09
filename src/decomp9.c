#include "decomp9.h"

typedef struct
{
  u32 unk0;
  u32 unk4;
  u32 unk8;
} AkaoDriverFlags;
typedef struct AkaoChannelState
{
  u32 flags;
  u32 unk4;
  u32 unk8;
  u32 unkC;
  u32 unk10;
  u32 unk14;
  u32 unk18;
  u32 unk1C;
  u32 unk20;
  u32 unk24;
  u32 unk28;
  s32 pitch;
  s32 unk30;
  u8 *unk34;
  u8 _pad38[4];
  u32 unk3C;
  u32 unk40;
  u32 unk44;
  u32 unk48;
  u32 unk4C;
  s32 unk50;
  s32 unk54;
  u16 unk58;
  s16 unk5A;
  u16 unk5C;
  u16 unk5E;
  u32 unk60;
  u16 unk64;
  u16 unk66;
  u16 unk68;
  u16 unk6A;
  u16 unk6C;
  u8 _pad6E[4];
  u16 unk72;
  u16 unk74[11];
  u16 unk8A;
  u16 unk8C;
  u8 _pad8E[6];
  u16 unk94;
  u16 unk96;
  u16 unk98;
  u16 unk9A;
  u16 unk9C;
  u16 unk9E;
  u8 _padA0[2];
  u16 unkA2;
  u16 unkA4;
  u8 _padA6[2];
  u16 unkA8;
  u16 unkAA;
  s16 unkAC;
  u16 unkAE;
  u8 _padB0[6];
  u16 unkB6;
  u16 unkB8;
  u8 _padBA[2];
  u16 unkBC;
  u16 unkBE;
  u8 _padC0[24];
  u16 unkD8;
  u16 unkDA;
  u16 unkDC;
  u16 unkDE;
  u8 _padE0[10];
  u16 unkEA;
  s16 unkEC;
  u16 unkEE;
  u16 unkF0;
  s16 unkF2;
  u16 unkF4;
  u16 unkF6;
  u8 _padF8[4];
  u32 unkFC;
  s32 unk100;
  s32 spu_sample_addr;
  s32 spu_loop_addr;
  u8 _pad10C[2];
  u16 unk10E;
  u16 unk110;
  u8 _pad112[6];
} AkaoChannelState;
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
  u32 unk1C;
  u32 unk20;
  u32 unk24;
} SfxControl;
extern AkaoDriverFlags g_akao_driver_flags;
extern AkaoChannelState *g_akao_seq_channel0;
extern SfxControl g_akao_sfx_control;

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


typedef struct
{
    s16 unk0;
    s16 unk2;
    s16 unk4;
} tiny_s;
typedef struct
{
    u8 pad0[0x1C];
    tiny_s* unk1C;
    tiny_s* unk20;
    tiny_s* unk24;
    u8 pad28[4];
    s32 unk2C;
    s32 unk30;
    u8 pad34[20];
    union
    {
        s32 unk48;
        struct
        {
            s16 unk48_half;
            s16 unk4A;
        } inner;
    } a;
    s32 unk4C;
    s32 unk50;
    u8 pad54[48];
    u16 unk84;
    u16 unk86;
    u16 unk88;
    u16 unk8A;
    u16 unk8C;
    u16 unk8E;
    u16 unk90;
    u16 unk92;
    u16 unk94;
    u8 pad96[14];
    u16 unkA4;
    u16 unkA6;
    u16 unkA8;
    u8 padAA[2];
    u16 unkAC;
    u16 unkAE;
    u16 unkB0;
    u16 unkB2;
    u8 padB4[4];
    u16 unkB8;
    u8 padBA[2];
    u16 unkBC;
    u8 padBE[2];
    u16 unkC0;
    u16 unkC2;
    u16 unkC4;
    u8 padC6[4];
    u16 unkCA;
    u8 padCC[2];
    u16 unkCE;
    u16 unkD0;
    u16 unkD2;
    u16 unkD4;
    u16 unkD6;
    u8 padD8[8];
    s16 unkE0;
    s16 unkE2;
    s32 unkE4;
    s16 unkE8;
    u8 padEA[10];
    s16 unkF4;
    s16 unkF6;
    s16 unkF8;
    u8 padFA[2];
    s32 unkFC;
    s32 unk100;
} arg0_struct;

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


/**
 * decomp.me (99.44%) https://decomp.me/scratch/0WomW
 */
void func_80024660(arg0_struct* arg0, s32 arg1, s32 arg2)
{
    tiny_s* var_a0;
    tiny_s* var_a0_2;
    tiny_s* var_a0_3;
    s32 temp_a3_2;
    s32 temp_a3_4;
    s32 temp_a3_5;
    s32 temp_a3_6;
    s32 temp_a3_7;
    s32 temp_v1_2;
    s32 temp_v1_5;
    s32 temp_a3;
    s32 temp_a3_3;
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
    if (arg2 == 0)
    {
        if (arg0->unk86 != 0)
        {
            arg0->unk86 = (u16)(arg0->unk86 - 1);
            temp_a3 = arg0->unk84 + arg0->unkE0;
            if ((temp_a3 & 0x7F00) != (arg0->unk84 & 0x7F00))
            {
                arg0->unk100 = (s32)(arg0->unk100 | 3);
            }
            arg0->unk84 = temp_a3;
        }
    }
    temp_v0_2 = arg0->unk8A;
    if (temp_v0_2 != 0)
    {
        temp_v1_2 = arg0->a.unk48;
        arg0->unk8A = (u16)(temp_v0_2 - 1);
        temp_a3_2 = temp_v1_2 + arg0->unk4C;
        if ((temp_a3_2 & 0xFFE00000) != (temp_v1_2 & 0xFFE00000))
        {
            arg0->unk100 = (s32)(arg0->unk100 | 3);
        }
        arg0->a.unk48 = temp_a3_2;
    }
    temp_v0_3 = arg0->unk92;
    if (temp_v0_3 != 0)
    {
        temp_v1_3 = arg0->unk90;
        arg0->unk92 = (u16)(temp_v0_3 - 1);
        temp_a3_3 = temp_v1_3 + arg0->unkE8;
        if ((temp_a3_3 & 0xFF00) != (temp_v1_3 & 0xFF00))
        {
            arg0->unk100 = (s32)(arg0->unk100 | 3);
        }
        arg0->unk90 = temp_a3_3;
    }
    temp_v0_4 = arg0->unkA4;
    if (temp_v0_4 != 0)
    {
        arg0->unkA4 = (u16)(temp_v0_4 - 1);
    }
    temp_v0_5 = arg0->unkB8;
    if (temp_v0_5 != 0)
    {
        arg0->unkB8 = (u16)(temp_v0_5 - 1);
    }
    temp_v0_6 = arg0->unkD4;
    if (temp_v0_6 != 0)
    {
        temp_v0_7 = temp_v0_6 - 1;
        arg0->unkD4 = temp_v0_7;
        if (!(temp_v0_7 & 0xFFFF))
        {
            if (arg2 == 0)
            {
                g_akao_seq_channel0->unk3C ^= arg1;
            }
            else
            {
                g_akao_sfx_control.unk1C ^= arg1;
            }
            g_akao_driver_flags.unk8 |= 0x110;
        }
    }
    temp_v0_8 = arg0->unkD6;
    if (temp_v0_8 != 0)
    {
        temp_v0_9 = temp_v0_8 - 1;
        arg0->unkD6 = temp_v0_9;
        if (!(temp_v0_9 & 0xFFFF))
        {
            if (arg2 == 0)
            {
                g_akao_seq_channel0->unk44 ^= arg1;
            }
            else
            {
                g_akao_sfx_control.unk24 ^= arg1;
            }
            g_akao_driver_flags.unk8 |= 0x100;
        }
    }
    temp_v1_4 = arg0->unkB0;
    if (temp_v1_4 != 0)
    {
        arg0->unkB0 = (u16)(temp_v1_4 - 1);
        temp_v0_10 = arg0->unkAE + arg0->unkB2;
        arg0->unkAE = temp_v0_10;
        temp_a0 = ((u32)(temp_v0_10 & 0x7F00)) >> 8;
        if (temp_v0_10 & 0x8000)
        {
            var_lo = (temp_a0 * arg0->unk2C) >> 7;
        }
        else
        {
            var_lo = (temp_a0 * (((u32)(arg0->unk2C * 0xF)) >> 8)) >> 7;
        }
        arg0->unkAC = (u16)var_lo;
        if ((arg0->unkA4 == 0) && (arg0->unkA8 != 1))
        {
            var_a0 = arg0->unk1C;
            if ((var_a0->unk0 == 0) && (var_a0->unk2 == 0))
            {
                var_a0 = (tiny_s*)(((u8*)var_a0) + (var_a0->unk4 * 2));
            }
            temp_a3_4 = ((s32)(arg0->unkAC * var_a0->unk0)) >> 0x10;
            if (temp_a3_4 != arg0->unkF4)
            {
                arg0->unkF4 = (s16)temp_a3_4;
                arg0->unk100 = (s32)(arg0->unk100 | 0x10);
                if (temp_a3_4 >= 0)
                {
                    arg0->unkF4 = (s16)(temp_a3_4 * 2);
                }
            }
        }
    }
    temp_v0_11 = arg0->unkC2;
    if (temp_v0_11 != 0)
    {
        s32 inter;
        arg0->unkC2 = (u16)(temp_v0_11 - 1);
        arg0->unkC0 = (u16)(arg0->unkC0 + arg0->unkC4);
        if ((arg0->unkB8 == 0) && (arg0->unkBC != 1))
        {
            var_a0_2 = arg0->unk20;
            if ((var_a0_2->unk0 == 0) && (var_a0_2->unk2 == 0))
            {
                var_a0_2 = (tiny_s*)(((u8*)var_a0_2) + (var_a0_2->unk4 * 2));
            }
            inter = (s32)(((s32)(((s32)(arg0->a.inner.unk4A * (arg0->unk84 >> 8)) >> 7) * (arg0->unkC0 >> 8)) << 9) >> 16);
            temp_a3_5 = (s32)(inter * var_a0_2->unk0) >> 0xF;
            // temp_a3_5 = ((s32) ((((s32) (((((s32) (arg0->a.inner.unk4A * (((u16) arg0->unk84) >> 8))) >> 7) * (((u16) arg0->unkC0) >> 8)) << 9)) >> 0x10) *
            // var_a0_2->unk0)) >> 0xF;
            if (temp_a3_5 != arg0->unkF6)
            {
                arg0->unkF6 = (s16)temp_a3_5;
                arg0->unk100 = (s32)(arg0->unk100 | 3);
            }
        }
    }
    temp_v0_12 = arg0->unkD0;
    if (temp_v0_12 != 0)
    {
        arg0->unkD0 = (u16)(temp_v0_12 - 1);
        arg0->unkCE = (u16)(arg0->unkCE + arg0->unkD2);
        if (arg0->unkCA != 1)
        {
            var_a0_3 = arg0->unk24;
            if ((var_a0_3->unk0 == 0) && (var_a0_3->unk2 == 0))
            {
                var_a0_3 = (tiny_s*)(((u8*)var_a0_3) + (var_a0_3->unk4 * 2));
            }
            temp_a3_6 = ((s32)((((u16)arg0->unkCE) >> 8) * var_a0_3->unk0)) >> 0xF;
            temp_a3_2 = temp_a3_6 != arg0->unkF8;
            if (temp_a3_2)
            {
                arg0->unkF8 = (s16)temp_a3_6;
                arg0->unk100 = (s32)(arg0->unk100 | 3);
            }
        }
    }
    temp_v0_13 = arg0->unk94;
    if (temp_v0_13 != 0)
    {
        temp_v1_5 = arg0->unk30;
        arg0->unk94 = (u16)(temp_v0_13 - 1);
        temp_a3_7 = temp_v1_5 + arg0->unk50;
        if ((temp_a3_7 & 0xFFFF0000) != (temp_v1_5 & 0xFFFF0000))
        {
            arg0->unk100 = (s32)(arg0->unk100 | 0x10);
        }
        arg0->unk30 = temp_a3_7;
    }
}