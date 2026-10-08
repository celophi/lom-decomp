/*
 * akao_xa_stream.c - AKAO streamed-voice (XA program) playback.
 *
 * Plays ADPCM programs through a pair of adjacent SPU voices, fed from main
 * RAM in blocks by SPU transfer and SPU IRQ callbacks. Three sources exist:
 * a RAM program streamed through a double-buffered SPU area (0xE0), a
 * resident SPU program (uploaded by 0xEC or staged for 0xED), and a CD-fed
 * ring of blocks (0xE8). All of them
 * share one state block, g_akao_xa_tracker.
 */
#include "common.h"
#include <libspu.h>
#include "internal/akao_voice.h"
#include "internal/akao_control.h"
#include "main/audio/akao_cmd.h"

/* Voice mask of SPU voices 22 and 23, the highest pair the stream may use. */
#define XA_TOP_VOICE_PAIR_MASK 0xC00000
/* Number of overlapping adjacent pairs, (12,13) through (22,23). */
#define XA_VOICE_PAIR_COUNT (AKAO_VOICE_COUNT - AKAO_SFX_FIRST_VOICE - 1)
/* Pair index 1 maps to voice 12. */
#define XA_FIRST_VOICE_BIAS (AKAO_SFX_FIRST_VOICE - 1)

/* SPU addresses of the double-buffered streaming area. */
#define XA_SPU_SILENCE 0x1030
#define XA_SPU_BLOCK_A 0x1100
#define XA_SPU_BLOCK_A_RIGHT 0x1900
#define XA_SPU_BLOCK_B 0x2100
#define XA_SPU_BLOCK_B_RIGHT 0x2900

/* Offset of the program header inside one CD ring block, and of its sample data. */
#define XA_RING_HEADER_OFFSET 0x80
#define XA_RING_DATA_OFFSET 0xD0

/* Upload sizes for one channel, a stereo block, and the initial stereo pair. */
#define XA_CHANNEL_BLOCK_BYTES 0x800
#define XA_STEREO_BLOCK_BYTES (2 * XA_CHANNEL_BLOCK_BYTES)
#define XA_INITIAL_STEREO_BYTES (2 * XA_STEREO_BLOCK_BYTES)

/** @brief Maximum bytes uploaded before the one-shot transfer callback runs. */
#define XA_ONE_SHOT_UPLOAD_LIMIT 0x2000

/** @brief SPU IRQ addresses advance in eight-byte units. */
#define XA_SPU_IRQ_STEP_BYTES 8

/**
 * @brief Minimum initial sample size that enables another ring refill.
 * @note TODO: the ring format basis for this threshold is unconfirmed.
 */
#define XA_RING_REFILL_MIN_BYTES 0xE61

/* Envelope settings for streamed voices. */
#define XA_DECAY_SHIFT 15
#define XA_SUSTAIN_LEVEL 15
#define XA_SUSTAIN_RATE 0x7F
#define XA_RELEASE_SHIFT 6

/** @brief Stereo routing used when configuring a streamed SPU voice. */
typedef enum
{
    XA_VOICE_PANNED_MONO,
    XA_VOICE_LEFT,
    XA_VOICE_RIGHT,
    XA_VOICE_CENTERED_MONO
} AkaoXaVoiceMode;

/** @brief One CD ring block containing a program header and its sample data. */
typedef struct
{
    u8 _pad00[XA_RING_HEADER_OFFSET];
    AkaoXaProgramHeader program;
    u8 _padC0[XA_RING_DATA_OFFSET - XA_RING_HEADER_OFFSET - sizeof(AkaoXaProgramHeader)];
    u8 sample_data[AKAO_XA_RING_BLOCK_BYTES - XA_RING_DATA_OFFSET];
} AkaoXaRingBlock;

void akao_release_channels(AkaoChannelState* channel, u32 release_mask);
void akao_spu_arm_xfer(void);

void akao_xa_begin_mono_buffer(void);
void akao_xa_begin_stereo_buffer(void);
void akao_xa_refill_mono_a(void);
void akao_xa_refill_mono_b(void);
void akao_xa_refill_stereo_a(void);
void akao_xa_refill_stereo_b(void);
void akao_xa_begin_ring(void);
void akao_xa_refill_ring_a(void);
void akao_xa_refill_ring_b(void);

/**
 * @brief Extended opcode FE 1E: restore the reserved-voice floor for this channel.
 * @param channel Unused opcode-handler channel argument.
 * @param channel_mask Bit of the channel being stepped.
 */
void akao_seq_op_obey_voice_reserve(AkaoChannelState* channel, s32 channel_mask)
{
    g_akao_seq_channel0->masks.voice_alloc_low_mask &= ~channel_mask;
}

/**
 * @brief Opcode 0xE1: set the pitch-jitter depth from one operand byte.
 *
 * Each note uses this depth with the free-running pitch-jitter table.
 *
 * @param channel Channel whose bytecode cursor is advanced past the depth byte.
 * @note Named by mechanism; the authoring-tool term is unconfirmed.
 */
void akao_seq_op_set_pitch_jitter_depth(AkaoChannelState* channel)
{
    channel->pitch_scale = *channel->seq_cursor++;
}

/**
 * @brief Opcode 0xE2: disable pitch jitter for this channel.
 * @param channel Channel whose pitch jitter is disabled.
 */
void akao_seq_op_disable_pitch_jitter(AkaoChannelState* channel)
{
    channel->pitch_scale = 0;
}

/**
 * @brief Release a channel for an unused primary or extended opcode.
 * @param channel Channel to release.
 * @param channel_mask Channel bit-mask to release.
 */
void akao_seq_op_finish_channel(AkaoChannelState* channel, u32 channel_mask)
{
    akao_release_channels(channel, channel_mask);
}

/**
 * @brief Stop the streamed voice pair.
 *
 * Disables the SPU IRQ, keys the pair off, drops it from the reverb mask and
 * marks the stream idle.
 */
void akao_xa_stop(void)
{
    if (g_akao_xa_tracker.voice_mask != 0)
    {
        SpuSetIRQ(SPU_OFF);
        SpuSetIRQCallback(NULL);
        spu_set_key_off(g_akao_xa_tracker.voice_mask);
        g_akao_sfx_control.reverb_mask &= ~g_akao_xa_tracker.voice_mask;
        g_akao_xa_tracker.voice_mask = 0;
        g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
    }
}

/**
 * @brief Find a free adjacent SPU voice pair for a stereo XA/stream voice.
 *
 * Scans pairs (22,23) down to (12,13) against the voices held by SFX
 * channels. When every pair is busy, stops the oldest untagged SFX and
 * retries until stopping frees nothing more.
 *
 * @return First voice of the free pair, or -1 when none could be freed.
 */
s32 akao_xa_alloc_voice_pair(void)
{
    u32 busy_voices;
    s32 pair_index;
    u32 pair_mask;

    while (1)
    {
        pair_mask = XA_TOP_VOICE_PAIR_MASK;
        busy_voices = g_akao_sfx_control.active_mask | g_akao_sfx_control.paused_mask;
        for (pair_index = XA_VOICE_PAIR_COUNT; pair_index != 0; pair_mask >>= 1, pair_index--)
        {
            if ((busy_voices & pair_mask) == 0)
            {
                break;
            }
        }
        if (pair_index != 0)
        {
            return pair_index + XA_FIRST_VOICE_BIAS;
        }
        akao_sfx_stop_channels(0, AKAO_SFX_STOP_OLDEST);
        if (busy_voices == (g_akao_sfx_control.active_mask | g_akao_sfx_control.paused_mask))
        {
            return -1;
        }
    }
}

/**
 * @brief SPU transfer callback: upload the second mono block of a RAM buffer.
 */
void akao_xa_upload_mono_block_b(void)
{
    u8* block_data = g_akao_xa_tracker.data_cursor + XA_CHANNEL_BLOCK_BYTES;

    SpuSetTransferStartAddr(XA_SPU_BLOCK_B);
    SpuSetTransferCallback(akao_xa_begin_mono_buffer);
    SpuWrite(block_data, XA_CHANNEL_BLOCK_BYTES);
}

/**
 * @brief Start playing an XA program from a RAM buffer (command 0xE0).
 *
 * Allocates a voice pair and uploads the first blocks into the SPU streaming
 * area; the transfer callback then sets up and keys on the voices.
 *
 * @param buffer XA program header; the ADPCM data follows it.
 * @param pan Q8 pan.
 * @param use_reverb Non-zero to send the voices through reverb.
 */
void akao_xa_start_buffer(AkaoXaProgramHeader* buffer, s32 pan, s32 use_reverb)
{
    AkaoXaProgramHeader* program;
    s32 voice;
    s32 voice_mask;
    u8* loop_start;
    s32 upload_bytes;
    s32 loop_bytes;
    AkaoXaTracker* tracker;
    AkaoXaTracker* stream;
    AkaoXaTracker* stereo_stream;

    voice = akao_xa_alloc_voice_pair();
    if (voice == -1)
    {
        return;
    }

    g_akao_xa_tracker.pan = pan;
    SpuSetIRQ(SPU_OFF);
    SpuSetIRQCallback(NULL);

    program = buffer;
    g_akao_xa_tracker.data_cursor = (u8*)(program + 1);
    g_akao_xa_tracker.bytes_remaining = program->sample_size;
    voice_mask = (1 << voice) | (1 << (voice + 1));

    g_akao_xa_tracker.last_program_spu_addr = program->spu_addr;
    g_akao_xa_tracker.first_voice = voice;
    g_akao_xa_tracker.voice_mask = voice_mask;
    spu_set_key_off(voice_mask);
    g_akao_xa_tracker.flags = program->flags;

    g_akao_xa_tracker.pitch = program->pitch;
    g_akao_xa_tracker.first_voice = voice;
    g_akao_xa_tracker.voice_mask = voice_mask;
    spu_set_key_off(voice_mask);

    SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
    SpuSetTransferStartAddr(XA_SPU_BLOCK_A);
    g_akao_spu_xfer_pending = 1;
    tracker = &g_akao_xa_tracker;

    if (g_akao_xa_tracker.flags & XA_FLAG_LOOP)
    {
        loop_start = g_akao_xa_tracker.data_cursor + program->loop_offset;
    }
    else
    {
        loop_start = NULL;
    }

    stream = &g_akao_xa_tracker;
    tracker->loop_cursor = loop_start;

    if (stream->flags & XA_FLAG_STEREO)
    {
        stereo_stream = stream;
        if (stream->flags & XA_FLAG_LOOP)
        {
            loop_bytes = stream->bytes_remaining - (program->loop_offset >> 1);
        }
        else
        {
            loop_bytes = 0;
        }
        stereo_stream->loop_bytes = loop_bytes;
        SpuSetTransferCallback(akao_xa_begin_stereo_buffer);
        upload_bytes = XA_INITIAL_STEREO_BYTES;
    }
    else
    {
        if (stream->flags & XA_FLAG_LOOP)
        {
            loop_bytes = stream->bytes_remaining - program->loop_offset;
        }
        else
        {
            loop_bytes = 0;
        }
        stream->loop_bytes = loop_bytes;
        SpuSetTransferCallback(akao_xa_upload_mono_block_b);
        upload_bytes = XA_CHANNEL_BLOCK_BYTES;
    }

    SpuWrite(g_akao_xa_tracker.data_cursor, upload_bytes);

    if (use_reverb != 0)
    {
        g_akao_sfx_control.reverb_mask |= g_akao_xa_tracker.voice_mask;
    }
    else
    {
        g_akao_sfx_control.reverb_mask &= ~g_akao_xa_tracker.voice_mask;
    }

    g_akao_sfx_control.pitch_mod_mask &= ~g_akao_xa_tracker.voice_mask;
    g_akao_sfx_control.noise_mask &= ~g_akao_xa_tracker.voice_mask;
    g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
}

/**
 * @brief Program one streamed voice: volume, pitch, addresses and envelope.
 * @param voice SPU voice index.
 * @param mode XA_VOICE_* routing; other values use the stream pan.
 * @param start_addr SPU sample start address.
 * @param repeat_addr SPU repeat address.
 */
void akao_xa_setup_voice(s32 voice, s32 mode, u32 start_addr, u32 repeat_addr)
{
    s16 volume_left;
    s16 volume_right;

    if (g_akao_driver_flags.output_mode & AKAO_OUTPUT_MONO)
    {
        s32 volume = (g_akao_xa_tracker.volume * g_akao_pan_gain_table[AKAO_PAN_CENTER]) >> AKAO_Q16_SHIFT;
        volume_right = volume;
        volume_left = volume;
    }
    else if (mode == XA_VOICE_LEFT)
    {
        volume_right = 0;
        volume_left = (u32)g_akao_xa_tracker.volume >> 1;
    }
    else if (mode == XA_VOICE_RIGHT)
    {
        volume_left = 0;
        volume_right = (u32)g_akao_xa_tracker.volume >> 1;
    }
    else if (mode == XA_VOICE_CENTERED_MONO)
    {
        /* Centre mono uses three quarters of the halved volume. */
        s32 volume = (g_akao_xa_tracker.volume >> 1) << AKAO_Q16_SHIFT;
        volume_right = (volume >> (AKAO_Q16_SHIFT + 1)) + (volume >> (AKAO_Q16_SHIFT + 2));
        volume_left = volume_right;
    }
    else
    {
        s32 pan = (g_akao_xa_tracker.pan >> AKAO_Q8_SHIFT) & AKAO_PAN_MASK;
        s32 volume = g_akao_xa_tracker.volume;
        volume_left = (u32)(volume * g_akao_pan_gain_table[pan]) >> AKAO_Q16_SHIFT;
        pan ^= AKAO_PAN_MASK;
        volume_right = (u32)(volume * g_akao_pan_gain_table[pan]) >> AKAO_Q16_SHIFT;
    }

    spu_set_voice_volume(voice, volume_left, volume_right, 0);
    spu_set_voice_pitch(voice, g_akao_xa_tracker.pitch);
    spu_set_voice_start_addr(voice, start_addr);
    spu_set_voice_repeat_addr(voice, repeat_addr);
    spu_set_voice_attack(voice, 0, SPU_VOICE_LINEARIncN);
    spu_set_voice_decay_shift(voice, XA_DECAY_SHIFT);
    spu_set_voice_sustain_level(voice, XA_SUSTAIN_LEVEL);
    spu_set_voice_sustain_mode(voice, XA_SUSTAIN_RATE, SPU_VOICE_LINEARDecN);
    spu_set_voice_release_mode(voice, XA_RELEASE_SHIFT, SPU_VOICE_LINEARDecN);
}

/**
 * @brief Arm the SPU IRQ for the next RAM-buffer block and key the voices on.
 * @param uploaded_bytes Bytes to advance the RAM cursor by.
 * @param irq_addr SPU address whose playback raises the next IRQ.
 * @param next_callback SPU IRQ callback that uploads the next block.
 */
void akao_xa_key_on_buffer(s32 uploaded_bytes, s32 irq_addr, SpuIRQCallbackProc next_callback)
{
    if (g_akao_xa_tracker.voice_mask != 0)
    {
        SpuSetTransferCallback(NULL);
        g_akao_spu_xfer_pending = 0;
        if (g_akao_xa_tracker.bytes_remaining > (2 * XA_CHANNEL_BLOCK_BYTES))
        {
            g_akao_xa_tracker.bytes_remaining -= (2 * XA_CHANNEL_BLOCK_BYTES);
            g_akao_xa_tracker.data_cursor += uploaded_bytes;
            SpuSetIRQCallback(next_callback);
        }
        else
        {
            SpuSetIRQCallback(akao_xa_stop);
            irq_addr = XA_SPU_SILENCE;
            g_akao_xa_tracker.bytes_remaining = 0;
        }
        SpuSetIRQAddr(irq_addr + XA_SPU_IRQ_STEP_BYTES);
        spu_set_key_on(g_akao_xa_tracker.voice_mask);
        SpuSetIRQ(SPU_ON);
    }
}

/**
 * @brief SPU transfer callback: start a mono RAM-buffer stream.
 */
void akao_xa_begin_mono_buffer(void)
{
    akao_xa_setup_voice(g_akao_xa_tracker.first_voice, XA_VOICE_PANNED_MONO, XA_SPU_BLOCK_A, XA_SPU_BLOCK_B);
    akao_xa_setup_voice(g_akao_xa_tracker.first_voice + 1, XA_VOICE_PANNED_MONO, XA_SPU_BLOCK_A, XA_SPU_BLOCK_B);
    akao_xa_key_on_buffer((2 * XA_CHANNEL_BLOCK_BYTES), XA_SPU_BLOCK_B, akao_xa_refill_mono_a);
}

/**
 * @brief SPU transfer callback: start a stereo RAM-buffer stream.
 */
void akao_xa_begin_stereo_buffer(void)
{
    akao_xa_setup_voice(g_akao_xa_tracker.first_voice, XA_VOICE_LEFT, XA_SPU_BLOCK_A, XA_SPU_BLOCK_B);
    akao_xa_setup_voice(g_akao_xa_tracker.first_voice + 1, XA_VOICE_RIGHT, XA_SPU_BLOCK_A_RIGHT, XA_SPU_BLOCK_B_RIGHT);
    akao_xa_key_on_buffer(XA_INITIAL_STEREO_BYTES, XA_SPU_BLOCK_B, akao_xa_refill_stereo_a);
}

/**
 * @brief Upload the next RAM-buffer block into the SPU half that just finished.
 * @param left_addr SPU address for the left voice, or both mono voices.
 * @param right_addr SPU address for the right voice.
 * @param upload_bytes Bytes to upload.
 * @param next_callback SPU IRQ callback for the following block.
 */
void akao_xa_refill_buffer(u32 left_addr, u32 right_addr, u32 upload_bytes, SpuIRQCallbackProc next_callback)
{
    if (g_akao_xa_tracker.voice_mask == 0 || g_akao_xa_tracker.bytes_remaining == 0)
    {
        return;
    }

    SpuSetTransferStartAddr(left_addr);
    akao_spu_arm_xfer();
    SpuWrite(g_akao_xa_tracker.data_cursor, upload_bytes);
    SpuSetIRQ(SPU_OFF);

    /* Remaining lengths advance per channel; stereo uploads carry both channels. */
    if (g_akao_xa_tracker.bytes_remaining > XA_CHANNEL_BLOCK_BYTES)
    {
        SpuSetIRQCallback(next_callback);
        g_akao_xa_tracker.bytes_remaining -= XA_CHANNEL_BLOCK_BYTES;
        g_akao_xa_tracker.data_cursor += upload_bytes;
    }
    else if (g_akao_xa_tracker.loop_cursor != NULL)
    {
        SpuSetIRQCallback(next_callback);
        g_akao_xa_tracker.data_cursor = g_akao_xa_tracker.loop_cursor;
        g_akao_xa_tracker.bytes_remaining = g_akao_xa_tracker.loop_bytes;
    }
    else
    {
        SpuSetIRQCallback(akao_xa_stop);
        right_addr = XA_SPU_SILENCE;
        left_addr = right_addr;
        g_akao_xa_tracker.bytes_remaining = 0;
    }

    spu_set_voice_repeat_addr(g_akao_xa_tracker.first_voice, left_addr);
    spu_set_voice_repeat_addr(g_akao_xa_tracker.first_voice + 1, right_addr);
    SpuSetIRQAddr(left_addr + XA_SPU_IRQ_STEP_BYTES);
    SpuSetIRQ(SPU_ON);
}

/**
 * @brief SPU IRQ callback: refill mono block A.
 */
void akao_xa_refill_mono_a(void)
{
    akao_xa_refill_buffer(XA_SPU_BLOCK_A, XA_SPU_BLOCK_A, XA_CHANNEL_BLOCK_BYTES, akao_xa_refill_mono_b);
}

/**
 * @brief SPU IRQ callback: refill mono block B.
 */
void akao_xa_refill_mono_b(void)
{
    akao_xa_refill_buffer(XA_SPU_BLOCK_B, XA_SPU_BLOCK_B, XA_CHANNEL_BLOCK_BYTES, akao_xa_refill_mono_a);
}

/**
 * @brief SPU IRQ callback: refill stereo block A.
 */
void akao_xa_refill_stereo_a(void)
{
    akao_xa_refill_buffer(XA_SPU_BLOCK_A, XA_SPU_BLOCK_A_RIGHT, XA_STEREO_BLOCK_BYTES, akao_xa_refill_stereo_b);
}

/**
 * @brief SPU IRQ callback: refill stereo block B.
 */
void akao_xa_refill_stereo_b(void)
{
    akao_xa_refill_buffer(XA_SPU_BLOCK_B, XA_SPU_BLOCK_B_RIGHT, XA_STEREO_BLOCK_BYTES, akao_xa_refill_stereo_a);
}

/**
 * @brief Command 0xE0: play an XA program from a RAM buffer.
 * @param params Queued command parameters: buffer, Q8 pan, reverb flag.
 */
void akao_xa_cmd_play_buffer(AkaoCommandParam* params)
{
    akao_xa_start_buffer(params[0].buffer, params[1].value, params[2].value);
    g_akao_sfx_control.active_mask &= ~g_akao_xa_tracker.voice_mask;
}

/**
 * @brief Command 0xE2: stop the streamed voice pair.
 */
void akao_xa_cmd_stop(void)
{
    akao_xa_stop();
}

/**
 * @brief Command 0xE4: set the streamed voice volume immediately.
 * @param params Queued command parameters: Q8 volume.
 */
void akao_xa_cmd_set_volume(AkaoCommandParam* params)
{
    AkaoXaTracker* stream = &g_akao_xa_tracker;
    s32 volume = params[0].value;

    stream->volume_fade_ticks = 0;
    stream->volume = volume;
    if (stream->voice_mask != 0)
    {
        spu_set_voice_volume(stream->first_voice, (volume << (AKAO_Q16_SHIFT - 1)) >> AKAO_Q16_SHIFT, 0, 0);
        spu_set_voice_volume(stream->first_voice + 1, 0, (stream->volume << (AKAO_Q16_SHIFT - 1)) >> AKAO_Q16_SHIFT, 0);
    }
}

/**
 * @brief Command 0xE5: fade the streamed voice volume.
 * @param params Queued command parameters: fade ticks (0 means 1), Q8 target volume.
 */
void akao_xa_cmd_fade_volume(AkaoCommandParam* params)
{
    s16 fade_ticks;
    s16 volume_delta;

    fade_ticks = 1;
    if (params[0].value != 0)
    {
        fade_ticks = params[0].value;
    }
    volume_delta = (u16)params[1].value - (u16)g_akao_xa_tracker.volume;
    g_akao_xa_tracker.volume_step = (s16)(volume_delta / fade_ticks);
    g_akao_xa_tracker.volume_fade_ticks = fade_ticks;
}

/**
 * @brief Command 0xE6: set the streamed voice pan.
 * @param params Queued command parameters: Q8 pan.
 */
void akao_xa_cmd_set_pan(AkaoCommandParam* params)
{
    AkaoXaTracker* stream;
    s32 pan_q8;
    s32 pan;
    s32 volume_left;
    s32 volume_right;
    s32 volume;

    stream = &g_akao_xa_tracker;
    pan_q8 = params[0].value;
    stream->pan = pan_q8;
    if (stream->voice_mask != 0)
    {
        if (g_akao_driver_flags.output_mode & AKAO_OUTPUT_MONO)
        {
            volume = (stream->volume * g_akao_pan_gain_table[AKAO_PAN_CENTER]) >> AKAO_Q16_SHIFT;
            spu_set_voice_volume(stream->first_voice, volume, volume, 0);
            spu_set_voice_volume(stream->first_voice + 1, volume, volume, 0);
        }
        else if (stream->flags & XA_FLAG_STEREO)
        {
            volume = stream->volume;
            volume <<= (AKAO_Q16_SHIFT - 1);
            volume >>= AKAO_Q16_SHIFT;
            spu_set_voice_volume(stream->first_voice, volume, 0, 0);
            spu_set_voice_volume(stream->first_voice + 1, 0, volume, 0);
        }
        else
        {
            pan = (pan_q8 >> AKAO_Q8_SHIFT) & AKAO_PAN_MASK;
            volume_left = (stream->volume * g_akao_pan_gain_table[pan]) >> AKAO_Q16_SHIFT;
            pan ^= AKAO_PAN_MASK;
            volume_right = (stream->volume * g_akao_pan_gain_table[pan]) >> AKAO_Q16_SHIFT;
            spu_set_voice_volume(stream->first_voice, volume_left, volume_right, 0);
            spu_set_voice_volume(stream->first_voice + 1, volume_left, volume_right, 0);
        }
    }
}

/**
 * @brief SPU IRQ callback: a one-shot program reached its end; stop at the silence block.
 */
void akao_xa_end_one_shot(void)
{
    SpuSetIRQAddr(XA_SPU_SILENCE + XA_SPU_IRQ_STEP_BYTES);
    SpuSetIRQCallback(akao_xa_stop);
}

/**
 * @brief SPU transfer callback: finish a one-shot upload and key the voices on.
 */
void akao_xa_key_on_one_shot(void)
{
    s32 irq_addr;

    if (g_akao_xa_tracker.bytes_remaining > XA_ONE_SHOT_UPLOAD_LIMIT)
    {
        SpuSetTransferStartAddr(g_akao_xa_tracker.source.spu_addr + XA_ONE_SHOT_UPLOAD_LIMIT);
        akao_spu_write(g_akao_xa_tracker.data_cursor, g_akao_xa_tracker.bytes_remaining - XA_ONE_SHOT_UPLOAD_LIMIT);
        irq_addr = g_akao_xa_tracker.source.spu_addr + (XA_ONE_SHOT_UPLOAD_LIMIT - XA_SPU_IRQ_STEP_BYTES);
    }
    else
    {
        irq_addr = g_akao_xa_tracker.source.spu_addr + (g_akao_xa_tracker.bytes_remaining >> 1) + XA_SPU_IRQ_STEP_BYTES;
    }

    akao_xa_setup_voice(g_akao_xa_tracker.first_voice, XA_VOICE_PANNED_MONO, g_akao_xa_tracker.source.spu_addr, XA_SPU_SILENCE);
    akao_xa_setup_voice(g_akao_xa_tracker.first_voice + 1, XA_VOICE_PANNED_MONO, g_akao_xa_tracker.source.spu_addr, XA_SPU_SILENCE);
    SpuSetIRQAddr(irq_addr);
    SpuSetIRQCallback(akao_xa_end_one_shot);
    spu_set_key_on(g_akao_xa_tracker.voice_mask);
    SpuSetIRQ(SPU_ON);
}

/**
 * @brief Upload a whole XA program to SPU RAM and play it once.
 * @param program XA program header; the ADPCM data follows it.
 * @param pan Q8 pan.
 * @param spu_addr SPU destination address.
 * @param use_reverb Non-zero to send the voices through reverb.
 */
void akao_xa_start_one_shot(AkaoXaProgramHeader* program, s32 pan, s32 spu_addr, s32 use_reverb)
{
    s32 voice;
    AkaoXaTracker* stream;
    s32 voice_mask;
    u32 value;

    voice = akao_xa_alloc_voice_pair();
    if (voice == -1)
    {
        return;
    }

    SpuSetIRQ(SPU_OFF);
    SpuSetIRQCallback(NULL);

    stream = &g_akao_xa_tracker;
    value = stream->voice_mask;
    stream->pan = pan;
    stream->data_cursor = (u8*)(program + 1);
    stream->bytes_remaining = program->sample_size;
    voice_mask = (1 << voice) | (1 << (voice + 1));
    stream->last_program_spu_addr = program->spu_addr;
    stream->first_voice = voice;
    stream->voice_mask = voice_mask;
    spu_set_key_off(voice_mask | value);
    stream->flags = program->flags;
    stream->pitch = program->pitch;
    stream->source.spu_addr = spu_addr;
    SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
    SpuSetTransferStartAddr(spu_addr);
    g_akao_spu_xfer_pending = 1;
    SpuSetTransferCallback(akao_xa_key_on_one_shot);

    value = stream->bytes_remaining;
    if (value > XA_ONE_SHOT_UPLOAD_LIMIT)
    {
        value = XA_ONE_SHOT_UPLOAD_LIMIT;
    }
    SpuWrite(stream->data_cursor, value);
    stream->data_cursor += value;

    if (use_reverb != 0)
    {
        g_akao_sfx_control.reverb_mask |= stream->voice_mask;
    }
    else
    {
        g_akao_sfx_control.reverb_mask &= ~stream->voice_mask;
    }
    g_akao_sfx_control.pitch_mod_mask &= ~g_akao_xa_tracker.voice_mask;
    g_akao_sfx_control.noise_mask &= ~g_akao_xa_tracker.voice_mask;
    g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
}

/**
 * @brief Play the XA program staged in SPU RAM by akao_upload_xa_program.
 * @param pan Q8 pan; not used.
 * @param use_reverb Non-zero to send the voices through reverb.
 */
void akao_xa_start_staged(s32 pan, s32 use_reverb)
{
    s32 voice;
    s32 previous_voice_mask;
    s32 voice_mask;
    AkaoXaProgramHeader* program;

    if (g_akao_xa_program_staging.header.spu_addr == 0)
    {
        return;
    }
    voice = akao_xa_alloc_voice_pair();
    if (voice == -1)
    {
        return;
    }

    SpuSetIRQ(SPU_OFF);
    SpuSetIRQCallback(NULL);
    previous_voice_mask = g_akao_xa_tracker.voice_mask;
    voice_mask = (1 << voice) | (1 << (voice + 1));
    g_akao_xa_tracker.first_voice = voice;
    g_akao_xa_tracker.voice_mask = voice_mask;
    spu_set_key_off(voice_mask | previous_voice_mask);
    program = &g_akao_xa_program_staging.header;
    g_akao_xa_tracker.flags = program->flags;
    g_akao_xa_tracker.pitch = program->pitch;
    if (use_reverb != 0)
    {
        g_akao_sfx_control.reverb_mask |= g_akao_xa_tracker.voice_mask;
    }
    else
    {
        g_akao_sfx_control.reverb_mask &= ~g_akao_xa_tracker.voice_mask;
    }
    g_akao_sfx_control.pitch_mod_mask &= ~g_akao_xa_tracker.voice_mask;
    g_akao_sfx_control.noise_mask &= ~g_akao_xa_tracker.voice_mask;
    g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
    SpuSetIRQAddr(program->spu_addr + (program->sample_size >> 1) + XA_SPU_IRQ_STEP_BYTES);
    SpuSetIRQCallback(akao_xa_end_one_shot);

    if (program->flags & XA_FLAG_STEREO)
    {
        if (program->flags & XA_FLAG_LOOP)
        {
            akao_xa_setup_voice(g_akao_xa_tracker.first_voice, XA_VOICE_LEFT, program->spu_addr, program->spu_addr + program->loop_offset);
            akao_xa_setup_voice(g_akao_xa_tracker.first_voice + 1, XA_VOICE_RIGHT, program->spu_addr + program->right_offset,
                                (program->spu_addr + program->right_offset) + program->loop_offset);
        }
        else
        {
            akao_xa_setup_voice(g_akao_xa_tracker.first_voice, XA_VOICE_LEFT, program->spu_addr, XA_SPU_SILENCE);
            akao_xa_setup_voice(g_akao_xa_tracker.first_voice + 1, XA_VOICE_RIGHT, program->spu_addr + program->right_offset, XA_SPU_SILENCE);
        }
    }
    else if (program->flags & XA_FLAG_LOOP)
    {
        akao_xa_setup_voice(g_akao_xa_tracker.first_voice, XA_VOICE_CENTERED_MONO, program->spu_addr, program->spu_addr + program->loop_offset);
        akao_xa_setup_voice(g_akao_xa_tracker.first_voice + 1, XA_VOICE_CENTERED_MONO, program->spu_addr, program->spu_addr + program->loop_offset);
    }
    else
    {
        akao_xa_setup_voice(g_akao_xa_tracker.first_voice, XA_VOICE_CENTERED_MONO, program->spu_addr, XA_SPU_SILENCE);
        akao_xa_setup_voice(g_akao_xa_tracker.first_voice + 1, XA_VOICE_CENTERED_MONO, program->spu_addr, XA_SPU_SILENCE);
    }
    spu_set_key_on(g_akao_xa_tracker.voice_mask);
    SpuSetIRQ(SPU_ON);
}

/**
 * @brief Command 0xEC: upload an XA program to SPU RAM and play it once.
 * @param params Queued command parameters: buffer, Q8 pan, SPU address, reverb flag.
 */
void akao_xa_cmd_play_one_shot(AkaoCommandParam* params)
{
    akao_xa_start_one_shot(params[0].buffer, params[1].value, params[2].value, params[3].value);
    g_akao_sfx_control.active_mask &= ~g_akao_xa_tracker.voice_mask;
}

/**
 * @brief Command 0xED: play the program staged in SPU RAM.
 * @param params Queued command parameters: Q8 pan, reverb flag.
 */
void akao_xa_cmd_play_staged(AkaoCommandParam* params)
{
    akao_xa_start_staged(params[0].value, params[1].value);
    g_akao_sfx_control.active_mask &= ~g_akao_xa_tracker.voice_mask;
}

/**
 * @brief Count an uploaded ring block and advance its index, wrapping at the ring length.
 * @param block_index Block index to advance.
 * @return The new block index.
 */
s32 akao_xa_next_ring_block(u32* block_index)
{
    g_akao_xa_tracker.uploaded_blocks++;
    (*block_index)++;
    if (*block_index > g_akao_xa_tracker.ring_block_count - 1)
    {
        *block_index = 0;
    }
    return *block_index;
}

/**
 * @brief Start playing a CD ring stream from its first block.
 */
void akao_xa_start_ring_stream(void)
{
    s32 voice;
    AkaoXaProgramHeader* program;
    s32 voice_mask;

    voice = akao_xa_alloc_voice_pair();
    if (voice == -1)
    {
        return;
    }

    SpuSetIRQ(SPU_OFF);
    SpuSetIRQCallback(NULL);

    program = &((AkaoXaRingBlock*)g_akao_xa_tracker.source.ring_base)->program;

    if (program->fade_in_ticks != 0 && g_akao_xa_tracker.volume_fade_ticks == 0)
    {
        s32 volume = g_akao_xa_tracker.volume;
        g_akao_xa_tracker.volume = 0;
        akao_fade_xa_volume(program->fade_in_ticks, volume >> AKAO_Q8_SHIFT);
    }

    g_akao_xa_tracker.data_cursor = g_akao_xa_tracker.source.ring_base;
    g_akao_xa_tracker.bytes_remaining = program->sample_size;
    g_akao_xa_tracker.last_program_spu_addr = program->spu_addr;
    g_akao_xa_tracker.first_voice = voice;
    voice_mask = (1 << voice) | (1 << (voice + 1));
    g_akao_xa_tracker.voice_mask = voice_mask;
    g_akao_xa_tracker.upload_block = 0;
    spu_set_key_off(voice_mask);
    g_akao_xa_tracker.flags = program->flags;
    g_akao_xa_tracker.pitch = program->pitch;
    SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
    SpuSetTransferStartAddr(XA_SPU_BLOCK_A);
    g_akao_spu_xfer_pending = 1;
    SpuSetTransferCallback(akao_xa_begin_ring);
    SpuWrite(((AkaoXaRingBlock*)g_akao_xa_tracker.data_cursor)->sample_data, XA_INITIAL_STEREO_BYTES);

    g_akao_sfx_control.reverb_mask &= ~g_akao_xa_tracker.voice_mask;
    g_akao_sfx_control.pitch_mod_mask &= ~g_akao_xa_tracker.voice_mask;
    g_akao_sfx_control.noise_mask &= ~g_akao_xa_tracker.voice_mask;
    g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;

    akao_xa_next_ring_block(&g_akao_xa_tracker.upload_block);
    akao_xa_next_ring_block(&g_akao_xa_tracker.upload_block);
}

/**
 * @brief Arm the SPU IRQ for the next ring block and key the voices on.
 * @param uploaded_bytes Bytes to advance the ring cursor by.
 * @param irq_addr SPU address whose playback raises the next IRQ.
 * @param next_callback SPU IRQ callback that uploads the next block.
 */
void akao_xa_key_on_ring(s32 uploaded_bytes, s32 irq_addr, SpuIRQCallbackProc next_callback)
{
    if (g_akao_xa_tracker.voice_mask != 0)
    {
        SpuSetTransferCallback(NULL);
        g_akao_spu_xfer_pending = 0;
        if (g_akao_xa_tracker.bytes_remaining >= XA_RING_REFILL_MIN_BYTES)
        {
            g_akao_xa_tracker.data_cursor += uploaded_bytes;
            SpuSetIRQCallback(next_callback);
        }
        else
        {
            SpuSetIRQCallback(akao_xa_stop);
            irq_addr = XA_SPU_SILENCE;
        }
        SpuSetIRQAddr(irq_addr + XA_SPU_IRQ_STEP_BYTES);
        spu_set_key_on(g_akao_xa_tracker.voice_mask);
        SpuSetIRQ(SPU_ON);
    }
}

/**
 * @brief SPU transfer callback: start a CD ring stream.
 */
void akao_xa_begin_ring(void)
{
    akao_xa_setup_voice(g_akao_xa_tracker.first_voice, XA_VOICE_LEFT, XA_SPU_BLOCK_A, XA_SPU_BLOCK_B);
    akao_xa_setup_voice(g_akao_xa_tracker.first_voice + 1, XA_VOICE_RIGHT, XA_SPU_BLOCK_A_RIGHT, XA_SPU_BLOCK_B_RIGHT);
    akao_xa_key_on_ring(XA_INITIAL_STEREO_BYTES, XA_SPU_BLOCK_B, akao_xa_refill_ring_a);
}

/**
 * @brief Upload the next CD ring block into the SPU half that just finished.
 * @param left_addr SPU address for the left voice.
 * @param right_addr SPU address for the right voice.
 * @param block_bytes Ring block size in bytes.
 * @param next_callback SPU IRQ callback for the following block.
 */
void akao_xa_refill_ring(s32 left_addr, s32 right_addr, s32 block_bytes, SpuIRQCallbackProc next_callback)
{
    AkaoXaRingBlock* block;
    AkaoXaProgramHeader* program;

    if (g_akao_xa_tracker.voice_mask == 0)
    {
        return;
    }
    block = (AkaoXaRingBlock*)g_akao_xa_tracker.data_cursor;
    program = &block->program;
    if (program->magic != AKAO_MAGIC)
    {
        return;
    }

    SpuSetIRQ(SPU_OFF);
    SpuSetTransferStartAddr(left_addr);
    akao_spu_arm_xfer();
    akao_xa_next_ring_block(&g_akao_xa_tracker.upload_block);
    SpuWrite(((AkaoXaRingBlock*)g_akao_xa_tracker.data_cursor)->sample_data, block_bytes - OFFSETOF(AkaoXaRingBlock, sample_data));
    g_akao_xa_tracker.last_ring_block_key = program->key;
    g_akao_xa_tracker.last_program_spu_addr = program->spu_addr;
    if (program->sample_size > program->spu_addr)
    {
        SpuSetIRQCallback(next_callback);
        g_akao_xa_tracker.data_cursor += block_bytes;
        if (g_akao_xa_tracker.upload_block == 0)
        {
            g_akao_xa_tracker.data_cursor = g_akao_xa_tracker.source.ring_base;
        }
    }
    else
    {
        SpuSetIRQCallback(akao_xa_stop);
        right_addr = XA_SPU_SILENCE;
        left_addr = XA_SPU_SILENCE;
    }
    spu_set_voice_repeat_addr(g_akao_xa_tracker.first_voice, left_addr);
    spu_set_voice_repeat_addr(g_akao_xa_tracker.first_voice + 1, right_addr);
    SpuSetIRQAddr(left_addr + XA_SPU_IRQ_STEP_BYTES);
    SpuSetIRQ(SPU_ON);
}

/**
 * @brief SPU IRQ callback: refill ring block A.
 */
void akao_xa_refill_ring_a(void)
{
    akao_xa_refill_ring(XA_SPU_BLOCK_A, XA_SPU_BLOCK_A_RIGHT, AKAO_XA_RING_BLOCK_BYTES, akao_xa_refill_ring_b);
}

/**
 * @brief SPU IRQ callback: refill ring block B.
 */
void akao_xa_refill_ring_b(void)
{
    akao_xa_refill_ring(XA_SPU_BLOCK_B, XA_SPU_BLOCK_B_RIGHT, AKAO_XA_RING_BLOCK_BYTES, akao_xa_refill_ring_a);
}

/**
 * @brief Command 0xE8: prepare a CD ring stream.
 * @param params Queued command parameters: first ring block, ring size.
 */
void akao_xa_cmd_prepare_ring(AkaoCommandParam* params)
{
    akao_xa_stop();
    g_akao_xa_tracker.flags = XA_FLAG_RING_STREAM;
    g_akao_xa_tracker.source.ring_base = params[0].buffer;
    g_akao_xa_tracker.ring_size = params[1].value;
}
