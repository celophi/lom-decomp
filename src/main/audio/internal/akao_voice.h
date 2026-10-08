#ifndef _AKAO_VOICE_H
#define _AKAO_VOICE_H

#include "akao_driver.h"

/** @brief Channel LFO and preceding-channel sidechain flags. */
#define AKAO_CH_PITCH_LFO 0x01
#define AKAO_CH_VOLUME_LFO 0x02
#define AKAO_CH_PAN_LFO 0x04
#define AKAO_CH_PITCH_SIDECHAIN 0x10
#define AKAO_CH_PITCH_VOLUME_SIDECHAIN 0x20

/** @brief Integer level bits used to detect accumulator changes. */
#define AKAO_CHANNEL_VOLUME_MASK (AKAO_VOLUME_MAX << AKAO_Q8_SHIFT)
#define AKAO_Q8_LEVEL_MASK 0xFF00
#define AKAO_MASTER_LEVEL_MASK (AKAO_MASTER_VOLUME_MASK << AKAO_Q16_SHIFT)

/** @brief Packed pitch-LFO depth and relative-pitch scaling. */
#define AKAO_PITCH_LFO_DEPTH_MASK 0x7F00
#define AKAO_PITCH_LFO_ABSOLUTE_FLAG 0x8000
#define AKAO_PITCH_LFO_RELATIVE_SCALE 15

/** @brief Fractional bits in the song reverb-depth accumulator. */
#define AKAO_REVERB_DEPTH_FRACTION_BITS 12

/** @brief Pending updates for the SPU stereo volume registers. */
#define SPU_UPDATE_VOLUME 0x3

/** @brief Pending update for the SPU pitch register. */
#define SPU_UPDATE_PITCH 0x10

/** @brief Pending sample-start and loop-address updates. */
#define SPU_UPDATE_START_ADDR 0x80
#define SPU_UPDATE_REPEAT_ADDR 0x10000

/** @brief All update bits that dirty each packed ADSR register. */
#define SPU_UPDATE_ADSR_HIGH 0x6600
#define SPU_UPDATE_ADSR_LOW 0x9900

/** @brief CD input volume registers shared by initialization and volume fades. */
#define SPU_CD_VOLUME_LEFT (*(s16*)0x1F801DB0)
#define SPU_CD_VOLUME_RIGHT (*(s16*)0x1F801DB2)

/** @brief Packed ADSR fields shared by sequence operands and SPU register writers. */
#define SPU_ADSR_DECAY_SUSTAIN_MASK 0x00FF
#define SPU_ADSR_ATTACK_RATE_SHIFT 8
#define SPU_ADSR_DECAY_RATE_SHIFT 4
#define SPU_ADSR_DECAY_RATE_MASK 0x00F0
#define SPU_ADSR_SUSTAIN_LEVEL_MASK 0x000F
#define SPU_ADSR_SUSTAIN_RATE_SHIFT 6

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

extern AkaoSongState* g_akao_voice_owners[];

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
