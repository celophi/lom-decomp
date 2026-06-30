#ifndef _DECOMP9_H
#define _DECOMP9_H

#include "common.h"

/**
 * @brief Voice volume pair (VOLL + VOLR) at the start of each SPU voice
 *        register block. Internal to the SPU register write helpers.
 */
typedef struct
{
    s16 left;
    s16 right;
} SpuVoiceVolume;

/**
 * @brief Packed SPU voice configuration passed to spu_set_voice_attr().
 *        Mirrors the writable fields of an SPU voice register block
 *        (offsets 0x00-0x0E) in a compact form.
 *
 * Field layout matches PSY-Q @c SpuVoiceAttr but flattened for bulk
 * register writes.
 */
typedef struct
{
    s32 pad0;        /**< Padding (unused). */
    s32 status;      /**< Output: set to 0 by spu_set_voice_attr(). */
    u32 start_addr;  /**< Waveform start address (written as >> 3 to ADDR). */
    u32 loop_addr;   /**< Loop/repeat address (written as >> 3 to RADDR). */
    u16 pitch;       /**< Sample rate / pitch (PITCH register). */
    u16 adsr1;       /**< ADSR1: Attack/Decay/Sustain Level/Sustain Rate. */
    u16 adsr2;       /**< ADSR2: Sustain mode, Release mode, Release Rate. */
    u16 pad16;       /**< Padding (unused by spu_set_voice_attr). */
    s16 vol_l;       /**< Volume Left  (VOLL, clamped to 0x7FFF). */
    s16 vol_r;       /**< Volume Right (VOLR, clamped to 0x7FFF). */
} SpuVoiceSetup;

/* ---- Common (non-voice) SPU register writers ---- */

void spu_set_key(u32 key_mask);
void spu_set_voice_mode(u32 mode_mask);
void spu_set_reverb_addrs(u32 addrs);
void spu_set_reverb_control(u32 val);
void spu_set_reverb_mode(u32 mode_mask);

/* ---- Per-voice SPU register writers ---- */

void spu_set_voice_volume(s32 voice, u32 vol_l, u32 vol_r, s32 scale);
void spu_set_voice_pitch(s32 voice, s16 pitch);
void spu_set_voice_start_addr(s32 voice, u32 addr);
void spu_set_voice_loop_addr(s32 voice, u32 addr);
void spu_set_voice_adsr1(s32 voice, s16 adsr1);
void spu_set_voice_adsr2(s32 voice, s16 adsr2);
void spu_set_voice_attack_decay(s32 voice, s32 attack_decay, u32 mode_bits);
void spu_set_voice_sustain_rate(s32 voice, s32 sustain_rate);
void spu_set_voice_sustain_level(s32 voice, s32 sustain_level);
void spu_set_voice_sr_mode(s32 voice, s32 sr_bits, u32 mode_bits);
void spu_set_voice_release_rate(s32 voice, s32 release_rate, u32 mode_bit);
void spu_set_voice_attr(s32 voice, SpuVoiceSetup* attr, s32 scale);

#endif
