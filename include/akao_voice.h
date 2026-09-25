#ifndef _AKAO_VOICE_H
#define _AKAO_VOICE_H

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
 * This is not Psy-Q's SpuVoiceAttr. It mirrors the final 0x1C bytes of
 * @ref AkaoChannelState (0xFC..0x117, @c voice through @c spu_volume_right):
 * the assigned SPU voice, an update mask, the register values, a volume scale
 * and the computed stereo volume. The same image is passed to both the full
 * and selective writers.
 *
 * @note Kept as its own type rather than folded into AkaoChannelState because
 *       the callers pass @c channel + 0xFC, not the channel base - it is the
 *       ABI of these writers, not a second view of the whole block.
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

/* ---- Driver constants and tables shared by akao_voice.c and akao_control.c ---- */

/** @brief Number of SPU hardware voices; also the "no voice assigned" marker. */
#define AKAO_VOICE_COUNT 24

/** @brief Number of channel slots in one song channel table. */
#define AKAO_CHANNEL_COUNT 32

/** @brief Channel-mask bit of the first SFX channel; SFX channels use bits 12-23. */
#define AKAO_SFX_FIRST_CHANNEL_BIT 0x1000

/** @brief SFX channel flag (AkaoChannelState::tempo_acc) that exempts it from global volume/pan/pause control. */
#define AKAO_SFX_FLAG_SUPPRESS 0x02000000

/*
 * Halfword views of AkaoChannelState words that the driver keeps as Q16
 * fixed-point accumulators (integer part in the high halfword).
 */
#define HALF_LOW_U16(word) (((u16*)&(word))[0])
#define HALF_HIGH_U16(word) (((u16*)&(word))[1])
#define HALF_HIGH_S16(word) (((s16*)&(word))[1])

extern AkaoChannelState* D_8004F7C0[];

extern s32 D_8004F76C[];

/* ---- Common (non-voice) SPU register writers ---- */

void spu_set_key_on(u32 voice_mask);
void spu_set_key_off(u32 voice_mask);
void spu_set_reverb_enable(u32 voice_mask);
void spu_set_noise_enable(u32 voice_mask);
void spu_set_pitch_modulation_enable(u32 voice_mask);

/* ---- Per-voice SPU register writers ---- */

void spu_set_voice_volume(s32 voice, u32 vol_l, u32 vol_r, s32 scale);
void spu_set_voice_pitch(s32 voice, s32 pitch);
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
void spu_apply_voice_updates(s32 voice, SpuVoiceParams* params, s32 flags);
void akao_tick_channel_effects(AkaoChannelState* channel, s32 channel_bit, s32 is_sfx);
void akao_flush_voice_updates(s32 sfx_update_mask);

#endif
