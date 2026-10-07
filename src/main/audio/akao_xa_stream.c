/*
 * akao_xa_stream.c - AKAO streamed-voice (XA program) playback.
 *
 * Plays ADPCM programs through a pair of adjacent SPU voices, fed from main
 * RAM in blocks by SPU transfer and SPU IRQ callbacks. Three sources exist:
 * a RAM buffer played through a double-buffered SPU area (commands 0xE0/0xEC),
 * a program already resident in SPU RAM (0xED, staged by
 * akao_upload_xa_program) and a CD-fed ring of blocks (0xE8). All of them
 * share one state block, g_akao_xa_tracker.
 */
#include "common.h"
#include <libspu.h>
#include "internal/akao_driver.h"

/* Voice mask of SPU voices 22 and 23, the highest pair the stream may use. */
#define XA_TOP_VOICE_PAIR_MASK 0xC00000
/* Number of candidate voice pairs, (12,13) through (22,23). */
#define XA_VOICE_PAIR_COUNT 11
/* Pair index 1 maps to voice 12. */
#define XA_FIRST_VOICE_BIAS 11

/* "AKAO" in little-endian. */
#define XA_AKAO_MAGIC 0x4F414B41

/* SPU addresses of the double-buffered streaming area. */
#define XA_SPU_SILENCE 0x1030
#define XA_SPU_BLOCK_A 0x1100
#define XA_SPU_BLOCK_A_RIGHT 0x1900
#define XA_SPU_BLOCK_B 0x2100
#define XA_SPU_BLOCK_B_RIGHT 0x2900

/* Offset of the program header inside one CD ring block, and of its sample data. */
#define XA_RING_HEADER_OFFSET 0x80
#define XA_RING_DATA_OFFSET 0xD0

extern AkaoXaProgramStaging g_akao_xa_program_staging;

void akao_release_channels(AkaoChannelState* channel, u32 release_mask);
void akao_sfx_stop_channels(s32 arg0, s32 arg1);
void spu_set_key_off(u32 voice_mask);
void spu_set_key_on(u32 voice_mask);
void spu_set_voice_volume(s32 voice, u32 vol_l, u32 vol_r, s32 scale);
void spu_set_voice_pitch(s32 voice, s32 pitch);
void spu_set_voice_start_addr(s32 voice, u32 addr);
void spu_set_voice_repeat_addr(s32 voice, u32 addr);
void spu_set_voice_attack(s32 voice, s32 attack_shift, u32 mode_bits);
void spu_set_voice_decay_shift(s32 voice, s32 decay_shift);
void spu_set_voice_sustain_level(s32 voice, s32 sustain_level);
void spu_set_voice_sustain_mode(s32 voice, s32 sustain_bits, u32 mode_bits);
void spu_set_voice_release_mode(s32 voice, s32 release_shift, u32 mode_bit);
void akao_spu_arm_xfer(void);
void akao_spu_write(void* source, s32 byte_count);
s32 akao_fade_xa_volume(s32 value0, s32 value1);

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
 * @brief Extended opcode FE 1E: obey the reserved-voice window for this channel.
 *        Clears the channel bit in voice_alloc_low_mask so note-on voice
 *        allocation starts at the reserved base again. Inverse of FE 1D
 *        (akao_seq_op_ignore_voice_reserve).
 * @param channel Unused; present to match the opcode-handler signature.
 * @param channel_mask Bit of the channel being stepped.
 */
void akao_seq_op_obey_voice_reserve(AkaoChannelState* channel, s32 channel_mask)
{
    g_akao_seq_channel0->masks.voice_alloc_low_mask &= ~channel_mask;
}

/**
 * @brief Opcode 0xE1: set the pitch-jitter depth from one operand byte.
 *        Stores the byte into pitch_scale (0xDA), the depth of the table-driven
 *        pitch perturbation akao_seq_step_opcode applies per note (indexes the
 *        256-entry jitter table g_akao_pitch_jitter_table by the free-running modulation tick).
 * @param channel Channel whose bytecode cursor is advanced past the depth byte.
 * @note Named by mechanism; the authoring-tool term is unconfirmed.
 */
void akao_seq_op_set_pitch_jitter_depth(AkaoChannelState* channel)
{
    channel->pitch_scale = *channel->seq_cursor++;
}

/**
 * @brief Opcode 0xE2: disable pitch jitter by clearing pitch_scale (0xDA).
 * @param channel Channel whose pitch jitter is disabled.
 */
void akao_seq_op_disable_pitch_jitter(AkaoChannelState* channel)
{
    channel->pitch_scale = 0;
}

/**
 * @brief Fallback opcode handler (primary 0xE3..0xFF and unused extended slots):
 *        end the channel by releasing it. Thin wrapper over akao_release_channels.
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
 * Disables the SPU IRQ, keys the pair off, drops it from the noise mask and
 * marks the stream idle.
 *
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
 * channels. When every pair is busy, stops the lowest-priority SFX and
 * retries until stopping frees nothing more.
 *
 * @return First voice of the free pair, or -1 when none could be freed.
 */
s32 akao_xa_alloc_voice_pair(void)
{
    u32 busy_voices;
    s32 pair;
    u32 pair_mask;

    while (1)
    {
        pair_mask = XA_TOP_VOICE_PAIR_MASK;
        pair = XA_VOICE_PAIR_COUNT;
        busy_voices = g_akao_sfx_control.active_mask | g_akao_sfx_control.paused_mask;
        while (1)
        {
            if ((busy_voices & pair_mask) == 0)
            {
                break;
            }
            pair--;
            pair_mask >>= 1;
            if (pair == 0)
            {
                break;
            }
        }
        if (pair != 0)
        {
            return pair + XA_FIRST_VOICE_BIAS;
        }
        akao_sfx_stop_channels(0, 0x40000000);
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
    u8* addr = g_akao_xa_tracker.data_cursor + 0x800;

    SpuSetTransferStartAddr(XA_SPU_BLOCK_B);
    SpuSetTransferCallback(akao_xa_begin_mono_buffer);
    SpuWrite(addr, 0x800);
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
void akao_xa_start_buffer(void* buffer, s32 pan, s32 use_reverb)
{
    AkaoXaProgramHeader* program;
    s32 voice;
    s32 voice_mask;
    u8* loop_start;
    s32 size;
    s32 loop_bytes;
    AkaoXaTracker* xa;
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

    g_akao_xa_tracker.unk18 = program->spu_addr;
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
    xa = &g_akao_xa_tracker;

    if (g_akao_xa_tracker.flags & XA_FLAG_LOOP)
    {
        loop_start = g_akao_xa_tracker.data_cursor + program->loop_offset;
    }
    else
    {
        loop_start = 0;
    }

    stream = &g_akao_xa_tracker;
    xa->loop_cursor = loop_start;

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
        size = 0x2000;
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
        size = 0x800;
    }

    SpuWrite(g_akao_xa_tracker.data_cursor, size);

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
 * @param mode 1 = left channel, 2 = right channel, 3 = mono centre, other = panned mono.
 * @param start SPU start address.
 * @param end SPU repeat address.
 */
void akao_xa_setup_voice(s32 voice, s32 mode, u32 start, u32 end)
{
    s16 volume_left;
    s16 volume_right;

    if (g_akao_driver_flags.output_mode & AKAO_OUTPUT_MONO)
    {
        s32 volume = (g_akao_xa_tracker.volume * g_akao_pan_gain_table[AKAO_PAN_CENTER]) >> 16;
        volume_right = volume;
        volume_left = volume;
    }
    else if (mode == 1)
    {
        volume_right = 0;
        volume_left = (u32)g_akao_xa_tracker.volume >> 1;
    }
    else if (mode == 2)
    {
        volume_left = 0;
        volume_right = (u32)g_akao_xa_tracker.volume >> 1;
    }
    else if (mode == 3)
    {
        s32 volume = (g_akao_xa_tracker.volume >> 1) << 16;
        volume_right = (volume >> 17) + (volume >> 18);
        volume_left = volume_right;
    }
    else
    {
        s32 pan = (g_akao_xa_tracker.pan >> 8) & 0xFF;
        s32 level = g_akao_xa_tracker.volume;
        volume_left = (u32)(level * g_akao_pan_gain_table[pan]) >> 16;
        pan ^= 0xFF;
        volume_right = (u32)(level * g_akao_pan_gain_table[pan]) >> 16;
    }

    spu_set_voice_volume(voice, volume_left, volume_right, 0);
    spu_set_voice_pitch(voice, g_akao_xa_tracker.pitch);
    spu_set_voice_start_addr(voice, start);
    spu_set_voice_repeat_addr(voice, end);
    spu_set_voice_attack(voice, 0, 1);
    spu_set_voice_decay_shift(voice, 0xF);
    spu_set_voice_sustain_level(voice, 0xF);
    spu_set_voice_sustain_mode(voice, 0x7F, 3);
    spu_set_voice_release_mode(voice, 6, 3);
}

/**
 * @brief Arm the SPU IRQ for the next RAM-buffer block and key the voices on.
 * @param uploaded Bytes consumed by the initial upload.
 * @param irq_addr SPU address whose playback raises the next IRQ.
 * @param next_cb SPU IRQ callback that uploads the next block.
 */
void akao_xa_key_on_buffer(s32 uploaded, s32 irq_addr, SpuIRQCallbackProc next_cb)
{
    if (g_akao_xa_tracker.voice_mask != 0)
    {
        SpuSetTransferCallback(NULL);
        g_akao_spu_xfer_pending = 0;
        if (g_akao_xa_tracker.bytes_remaining > 0x1000)
        {
            g_akao_xa_tracker.bytes_remaining -= 0x1000;
            g_akao_xa_tracker.data_cursor += uploaded;
            SpuSetIRQCallback(next_cb);
        }
        else
        {
            SpuSetIRQCallback(akao_xa_stop);
            irq_addr = XA_SPU_SILENCE;
            g_akao_xa_tracker.bytes_remaining = 0;
        }
        SpuSetIRQAddr(irq_addr + 8);
        spu_set_key_on(g_akao_xa_tracker.voice_mask);
        SpuSetIRQ(SPU_ON);
    }
}

/**
 * @brief SPU transfer callback: start a mono RAM-buffer stream.
 */
void akao_xa_begin_mono_buffer(void)
{
    akao_xa_setup_voice(g_akao_xa_tracker.first_voice, 0, XA_SPU_BLOCK_A, XA_SPU_BLOCK_B);
    akao_xa_setup_voice(g_akao_xa_tracker.first_voice + 1, 0, XA_SPU_BLOCK_A, XA_SPU_BLOCK_B);
    akao_xa_key_on_buffer(0x1000, XA_SPU_BLOCK_B, akao_xa_refill_mono_a);
}

/**
 * @brief SPU transfer callback: start a stereo RAM-buffer stream.
 */
void akao_xa_begin_stereo_buffer(void)
{
    akao_xa_setup_voice(g_akao_xa_tracker.first_voice, 1, XA_SPU_BLOCK_A, XA_SPU_BLOCK_B);
    akao_xa_setup_voice(g_akao_xa_tracker.first_voice + 1, 2, XA_SPU_BLOCK_A_RIGHT, XA_SPU_BLOCK_B_RIGHT);
    akao_xa_key_on_buffer(0x2000, XA_SPU_BLOCK_B, akao_xa_refill_stereo_a);
}

/**
 * @brief Upload the next RAM-buffer block into the SPU half that just finished.
 * @param start SPU address of the left (or mono) half.
 * @param end SPU address of the right half.
 * @param size Bytes to upload.
 * @param cb SPU IRQ callback for the following block.
 */
void akao_xa_refill_buffer(u32 start, u32 end, u32 size, SpuIRQCallbackProc cb)
{
    if (g_akao_xa_tracker.voice_mask == 0)
    {
        return;
    }
    if (g_akao_xa_tracker.bytes_remaining == 0)
    {
        return;
    }

    SpuSetTransferStartAddr(start);
    akao_spu_arm_xfer();
    SpuWrite(g_akao_xa_tracker.data_cursor, size);
    SpuSetIRQ(SPU_OFF);

    if (g_akao_xa_tracker.bytes_remaining > 0x800)
    {
        SpuSetIRQCallback(cb);
        g_akao_xa_tracker.bytes_remaining -= 0x800;
        g_akao_xa_tracker.data_cursor += size;
    }
    else if (g_akao_xa_tracker.loop_cursor != 0)
    {
        SpuSetIRQCallback(cb);
        g_akao_xa_tracker.data_cursor = g_akao_xa_tracker.loop_cursor;
        g_akao_xa_tracker.bytes_remaining = g_akao_xa_tracker.loop_bytes;
    }
    else
    {
        SpuSetIRQCallback(akao_xa_stop);
        end = XA_SPU_SILENCE;
        start = end;
        g_akao_xa_tracker.bytes_remaining = 0;
    }

    spu_set_voice_repeat_addr(g_akao_xa_tracker.first_voice, start);
    spu_set_voice_repeat_addr(g_akao_xa_tracker.first_voice + 1, end);
    SpuSetIRQAddr(start + 8);
    SpuSetIRQ(SPU_ON);
}

/**
 * @brief SPU IRQ callback: refill mono block A.
 */
void akao_xa_refill_mono_a(void)
{
    akao_xa_refill_buffer(XA_SPU_BLOCK_A, XA_SPU_BLOCK_A, 0x800, akao_xa_refill_mono_b);
}

/**
 * @brief SPU IRQ callback: refill mono block B.
 */
void akao_xa_refill_mono_b(void)
{
    akao_xa_refill_buffer(XA_SPU_BLOCK_B, XA_SPU_BLOCK_B, 0x800, akao_xa_refill_mono_a);
}

/**
 * @brief SPU IRQ callback: refill stereo block A.
 */
void akao_xa_refill_stereo_a(void)
{
    akao_xa_refill_buffer(XA_SPU_BLOCK_A, XA_SPU_BLOCK_A_RIGHT, 0x1000, akao_xa_refill_stereo_b);
}

/**
 * @brief SPU IRQ callback: refill stereo block B.
 */
void akao_xa_refill_stereo_b(void)
{
    akao_xa_refill_buffer(XA_SPU_BLOCK_B, XA_SPU_BLOCK_B_RIGHT, 0x1000, akao_xa_refill_stereo_a);
}

/**
 * @brief Command 0xE0: play an XA program from a RAM buffer.
 * @param params Queued command parameters: buffer, Q8 pan, noise flag.
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
    AkaoXaTracker* xa = &g_akao_xa_tracker;
    s32 val = params[0].value;

    xa->volume_fade_ticks = 0;
    xa->volume = val;
    if (xa->voice_mask != 0)
    {
        spu_set_voice_volume(xa->first_voice, (val << 15) >> 16, 0, 0);
        spu_set_voice_volume(xa->first_voice + 1, 0, (xa->volume << 15) >> 16, 0);
    }
}

/**
 * @brief Command 0xE5: fade the streamed voice volume.
 * @param params Queued command parameters: fade ticks (0 means 1), Q8 target volume.
 */
void akao_xa_cmd_fade_volume(AkaoCommandParam* params)
{
    s16 ticks;
    s16 delta;

    ticks = 1;
    if (params[0].value != 0)
    {
        ticks = params[0].value;
    }
    delta = (u16)params[1].value - (u16)g_akao_xa_tracker.volume;
    g_akao_xa_tracker.volume_step = (s16)(delta / ticks);
    g_akao_xa_tracker.volume_fade_ticks = ticks;
}

/**
 * @brief Command 0xE6: set the streamed voice pan.
 * @param params Queued command parameters: Q8 pan.
 */
void akao_xa_cmd_set_pan(AkaoCommandParam* params)
{
    AkaoXaTracker* xa;
    s32 pan_word;
    s32 pan;
    s32 volume_left;
    s32 volume_right;
    s32 volume;

    xa = &g_akao_xa_tracker;
    pan_word = params[0].value;
    xa->pan = pan_word;
    if (xa->voice_mask != 0)
    {
        if (g_akao_driver_flags.output_mode & AKAO_OUTPUT_MONO)
        {
            volume = (xa->volume * g_akao_pan_gain_table[AKAO_PAN_CENTER]) >> 16;
            spu_set_voice_volume(xa->first_voice, volume, volume, 0);
            spu_set_voice_volume(xa->first_voice + 1, volume, volume, 0);
        }
        else if (xa->flags & XA_FLAG_STEREO)
        {
            volume = xa->volume;
            volume <<= 15;
            volume >>= 16;
            spu_set_voice_volume(xa->first_voice, volume, 0, 0);
            spu_set_voice_volume(xa->first_voice + 1, 0, volume, 0);
        }
        else
        {
            pan = (pan_word >> 8) & 0xFF;
            volume_left = (xa->volume * g_akao_pan_gain_table[pan]) >> 16;
            pan ^= 0xFF;
            volume_right = (xa->volume * g_akao_pan_gain_table[pan]) >> 16;
            spu_set_voice_volume(xa->first_voice, volume_left, volume_right, 0);
            spu_set_voice_volume(xa->first_voice + 1, volume_left, volume_right, 0);
        }
    }
}

/**
 * @brief SPU IRQ callback: a one-shot program reached its end; stop at the silence block.
 */
void akao_xa_end_one_shot(void)
{
    SpuSetIRQAddr(XA_SPU_SILENCE + 8);
    SpuSetIRQCallback(akao_xa_stop);
}

/**
 * @brief SPU transfer callback: finish a one-shot upload and key the voices on.
 */
void akao_xa_key_on_one_shot(void)
{
    s32 addr;

    if (g_akao_xa_tracker.bytes_remaining > 0x2000)
    {
        SpuSetTransferStartAddr(g_akao_xa_tracker.source.spu_addr + 0x2000);
        akao_spu_write(g_akao_xa_tracker.data_cursor, g_akao_xa_tracker.bytes_remaining - 0x2000);
        addr = g_akao_xa_tracker.source.spu_addr + 0x1FF8;
    }
    else
    {
        addr = g_akao_xa_tracker.source.spu_addr + (g_akao_xa_tracker.bytes_remaining >> 1) + 8;
    }

    akao_xa_setup_voice(g_akao_xa_tracker.first_voice, 0, g_akao_xa_tracker.source.spu_addr, XA_SPU_SILENCE);
    akao_xa_setup_voice(g_akao_xa_tracker.first_voice + 1, 0, g_akao_xa_tracker.source.spu_addr, XA_SPU_SILENCE);
    SpuSetIRQAddr(addr);
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
    AkaoXaTracker* xa;
    s32 voice_mask;
    u32 value;

    voice = akao_xa_alloc_voice_pair();
    if (voice == -1)
    {
        return;
    }

    SpuSetIRQ(SPU_OFF);
    SpuSetIRQCallback(NULL);

    xa = &g_akao_xa_tracker;
    value = xa->voice_mask;
    xa->pan = pan;
    xa->data_cursor = (u8*)(program + 1);
    xa->bytes_remaining = program->sample_size;
    voice_mask = (1 << voice) | (1 << (voice + 1));
    xa->unk18 = program->spu_addr;
    xa->first_voice = voice;
    xa->voice_mask = voice_mask;
    spu_set_key_off(voice_mask | value);
    xa->flags = program->flags;
    xa->pitch = program->pitch;
    xa->source.spu_addr = spu_addr;
    SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
    SpuSetTransferStartAddr(spu_addr);
    g_akao_spu_xfer_pending = 1;
    SpuSetTransferCallback(akao_xa_key_on_one_shot);

    value = xa->bytes_remaining;
    if (value > 0x2000)
    {
        value = 0x2000;
    }
    SpuWrite(xa->data_cursor, value);
    xa->data_cursor += value;

    if (use_reverb != 0)
    {
        g_akao_sfx_control.reverb_mask |= xa->voice_mask;
    }
    else
    {
        g_akao_sfx_control.reverb_mask &= ~xa->voice_mask;
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
    s32 old_mask;
    s32 voice_mask;
    AkaoXaProgramHeader* program;

    if (g_akao_xa_program_staging.header.spu_addr != 0)
    {
        voice = akao_xa_alloc_voice_pair();
        if (voice != -1)
        {
            SpuSetIRQ(SPU_OFF);
            SpuSetIRQCallback(NULL);
            old_mask = g_akao_xa_tracker.voice_mask;
            voice_mask = (1 << voice) | (1 << (voice + 1));
            g_akao_xa_tracker.first_voice = voice;
            g_akao_xa_tracker.voice_mask = voice_mask;
            spu_set_key_off(voice_mask | old_mask);
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
            SpuSetIRQAddr(program->spu_addr + (program->sample_size >> 1) + 8);
            SpuSetIRQCallback(akao_xa_end_one_shot);

            if (program->flags & XA_FLAG_STEREO)
            {
                if (program->flags & XA_FLAG_LOOP)
                {
                    akao_xa_setup_voice(g_akao_xa_tracker.first_voice, 1, program->spu_addr, program->spu_addr + program->loop_offset);
                    akao_xa_setup_voice(g_akao_xa_tracker.first_voice + 1, 2, program->spu_addr + program->right_offset,
                                        (program->spu_addr + program->right_offset) + program->loop_offset);
                }
                else
                {
                    akao_xa_setup_voice(g_akao_xa_tracker.first_voice, 1, program->spu_addr, XA_SPU_SILENCE);
                    akao_xa_setup_voice(g_akao_xa_tracker.first_voice + 1, 2, program->spu_addr + program->right_offset, XA_SPU_SILENCE);
                }
            }
            else if (program->flags & XA_FLAG_LOOP)
            {
                akao_xa_setup_voice(g_akao_xa_tracker.first_voice, 3, program->spu_addr, program->spu_addr + program->loop_offset);
                akao_xa_setup_voice(g_akao_xa_tracker.first_voice + 1, 3, program->spu_addr, program->spu_addr + program->loop_offset);
            }
            else
            {
                akao_xa_setup_voice(g_akao_xa_tracker.first_voice, 3, program->spu_addr, XA_SPU_SILENCE);
                akao_xa_setup_voice(g_akao_xa_tracker.first_voice + 1, 3, program->spu_addr, XA_SPU_SILENCE);
            }
            spu_set_key_on(g_akao_xa_tracker.voice_mask);
            SpuSetIRQ(SPU_ON);
        }
    }
}

/**
 * @brief Command 0xEC: upload an XA program to SPU RAM and play it once.
 * @param params Queued command parameters: buffer, Q8 pan, SPU address, noise flag.
 */
void akao_xa_cmd_play_one_shot(AkaoCommandParam* params)
{
    akao_xa_start_one_shot(params[0].buffer, params[1].value, params[2].value, params[3].value);
    g_akao_sfx_control.active_mask &= ~g_akao_xa_tracker.voice_mask;
}

/**
 * @brief Command 0xED: play the program staged in SPU RAM.
 * @param params Queued command parameters: Q8 pan, noise flag.
 */
void akao_xa_cmd_play_staged(AkaoCommandParam* params)
{
    akao_xa_start_staged(params[0].value, params[1].value);
    g_akao_sfx_control.active_mask &= ~g_akao_xa_tracker.voice_mask;
}

/**
 * @brief Advance a ring block index, wrapping at the ring length.
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

    program = (AkaoXaProgramHeader*)(g_akao_xa_tracker.source.ring_base + XA_RING_HEADER_OFFSET);

    if (program->fade_in_ticks != 0 && g_akao_xa_tracker.volume_fade_ticks == 0)
    {
        s32 volume = g_akao_xa_tracker.volume;
        g_akao_xa_tracker.volume = 0;
        akao_fade_xa_volume(program->fade_in_ticks, volume >> 8);
    }

    g_akao_xa_tracker.data_cursor = g_akao_xa_tracker.source.ring_base;
    g_akao_xa_tracker.bytes_remaining = program->sample_size;
    g_akao_xa_tracker.unk18 = program->spu_addr;
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
    SpuWrite(g_akao_xa_tracker.data_cursor + XA_RING_DATA_OFFSET, 0x2000);

    g_akao_sfx_control.reverb_mask &= ~g_akao_xa_tracker.voice_mask;
    g_akao_sfx_control.pitch_mod_mask &= ~g_akao_xa_tracker.voice_mask;
    g_akao_sfx_control.noise_mask &= ~g_akao_xa_tracker.voice_mask;
    g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;

    akao_xa_next_ring_block(&g_akao_xa_tracker.upload_block);
    akao_xa_next_ring_block(&g_akao_xa_tracker.upload_block);
}

/**
 * @brief Arm the SPU IRQ for the next ring block and key the voices on.
 * @param uploaded Bytes to advance the ring cursor by.
 * @param irq_addr SPU address whose playback raises the next IRQ.
 * @param next_cb SPU IRQ callback that uploads the next block.
 */
void akao_xa_key_on_ring(s32 uploaded, s32 irq_addr, SpuIRQCallbackProc next_cb)
{
    if (g_akao_xa_tracker.voice_mask != 0)
    {
        SpuSetTransferCallback(NULL);
        g_akao_spu_xfer_pending = 0;
        if (g_akao_xa_tracker.bytes_remaining >= 0xE61)
        {
            g_akao_xa_tracker.data_cursor += uploaded;
            SpuSetIRQCallback(next_cb);
        }
        else
        {
            SpuSetIRQCallback(akao_xa_stop);
            irq_addr = XA_SPU_SILENCE;
        }
        SpuSetIRQAddr(irq_addr + 8);
        spu_set_key_on(g_akao_xa_tracker.voice_mask);
        SpuSetIRQ(SPU_ON);
    }
}

/**
 * @brief SPU transfer callback: start a CD ring stream.
 */
void akao_xa_begin_ring(void)
{
    akao_xa_setup_voice(g_akao_xa_tracker.first_voice, 1, XA_SPU_BLOCK_A, XA_SPU_BLOCK_B);
    akao_xa_setup_voice(g_akao_xa_tracker.first_voice + 1, 2, XA_SPU_BLOCK_A_RIGHT, XA_SPU_BLOCK_B_RIGHT);
    akao_xa_key_on_ring(0x2000, XA_SPU_BLOCK_B, akao_xa_refill_ring_a);
}

/**
 * @brief Upload the next CD ring block into the SPU half that just finished.
 * @param start SPU address of the left half.
 * @param end SPU address of the right half.
 * @param block_size Ring block size in bytes.
 * @param cb SPU IRQ callback for the following block.
 */
void akao_xa_refill_ring(s32 start, s32 end, s32 block_size, SpuIRQCallbackProc cb)
{
    u8* base;
    AkaoXaProgramHeader* hdr;

    if (g_akao_xa_tracker.voice_mask != 0)
    {
        base = g_akao_xa_tracker.data_cursor;
        hdr = (AkaoXaProgramHeader*)(base + XA_RING_HEADER_OFFSET);
        if (hdr->magic == XA_AKAO_MAGIC)
        {
            SpuSetIRQ(SPU_OFF);
            SpuSetTransferStartAddr(start);
            akao_spu_arm_xfer();
            akao_xa_next_ring_block(&g_akao_xa_tracker.upload_block);
            SpuWrite(g_akao_xa_tracker.data_cursor + XA_RING_DATA_OFFSET, block_size - XA_RING_DATA_OFFSET);
            g_akao_xa_tracker.last_ring_block_key = hdr->key;
            g_akao_xa_tracker.unk18 = hdr->spu_addr;
            if (hdr->sample_size > hdr->spu_addr)
            {
                SpuSetIRQCallback(cb);
                g_akao_xa_tracker.data_cursor += block_size;
                if (g_akao_xa_tracker.upload_block == 0)
                {
                    g_akao_xa_tracker.data_cursor = g_akao_xa_tracker.source.ring_base;
                }
            }
            else
            {
                SpuSetIRQCallback(akao_xa_stop);
                end = XA_SPU_SILENCE;
                start = XA_SPU_SILENCE;
            }
            spu_set_voice_repeat_addr(g_akao_xa_tracker.first_voice, start);
            spu_set_voice_repeat_addr(g_akao_xa_tracker.first_voice + 1, end);
            SpuSetIRQAddr(start + 8);
            SpuSetIRQ(SPU_ON);
        }
    }
}

/**
 * @brief SPU IRQ callback: refill ring block A.
 */
void akao_xa_refill_ring_a(void)
{
    akao_xa_refill_ring(XA_SPU_BLOCK_A, XA_SPU_BLOCK_A_RIGHT, 0x1000, akao_xa_refill_ring_b);
}

/**
 * @brief SPU IRQ callback: refill ring block B.
 */
void akao_xa_refill_ring_b(void)
{
    akao_xa_refill_ring(XA_SPU_BLOCK_B, XA_SPU_BLOCK_B_RIGHT, 0x1000, akao_xa_refill_ring_a);
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
