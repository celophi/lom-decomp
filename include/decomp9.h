#ifndef _DECOMP9_H
#define _DECOMP9_H

#include "akao_driver.h"

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
 * @brief AKAO's pending register image for one SPU voice.
 *
 * This is not Psy-Q's SpuVoiceAttr. In an AkaoChannelState it overlays the
 * final 0x1C bytes (0xFC..0x117): the assigned SPU voice is followed by an
 * update mask, the register values, a volume scale, and the computed stereo
 * volume. The same image is passed to both the full and selective writers.
 */
typedef struct
{
    s32 voice;             /**< Assigned SPU voice index in the channel overlay. */
    s32 update_flags;      /**< Pending SPU register update mask; cleared on write. */
    u32 sample_start_addr; /**< ADPCM sample start address in SPU RAM. */
    u32 sample_repeat_addr;/**< ADPCM repeat address in SPU RAM. */
    u16 pitch;             /**< SPU pitch/sample-rate register image. */
    u16 adsr_low;          /**< SPU ADSR low halfword (voice register +0x08). */
    u16 adsr_high;         /**< SPU ADSR high halfword (voice register +0x0A). */
    u16 volume_scale;      /**< Optional Q7 scale applied to both volume fields. */
    s16 volume_left;       /**< Computed left volume. */
    s16 volume_right;      /**< Computed right volume. */
} SpuVoiceParams;

typedef struct AkaoChannelEffects AkaoChannelEffects;

/* ---- Common (non-voice) SPU register writers ---- */

void spu_set_key_on(u32 voice_mask);
void spu_set_key_off(u32 voice_mask);
void spu_set_reverb_enable(u32 voice_mask);
void spu_set_noise_enable(u32 voice_mask);
void spu_set_pitch_modulation_enable(u32 voice_mask);

/* ---- Per-voice SPU register writers ---- */

void spu_set_voice_volume(s32 voice, u32 vol_l, u32 vol_r, s32 scale);
void spu_set_voice_pitch(s32 voice, u16 pitch);
void spu_set_voice_start_addr(s32 voice, u32 addr);
void spu_set_voice_repeat_addr(s32 voice, u32 addr);
void spu_set_voice_adsr_low(s32 voice, u16 adsr_low);
void spu_set_voice_adsr_high(s32 voice, u16 adsr_high);
void spu_set_voice_attack(s32 voice, s32 attack_shift, u32 mode_bits);
void spu_set_voice_decay_shift(s32 voice, s32 decay_shift);
void spu_set_voice_sustain_level(s32 voice, s32 sustain_level);
void spu_set_voice_sustain_mode(s32 voice, s32 sustain_bits, u32 mode_bits);
void spu_set_voice_release_mode(s32 voice, s32 release_shift, u32 mode_bit);
void spu_write_voice_params(s32 voice, SpuVoiceParams* params, s32 scale);
void spu_apply_voice_updates(s32 voice, SpuVoiceParams* params);
void akao_tick_channel_effects(AkaoChannelEffects* channel, s32 channel_bit, s32 is_sfx);

#endif
