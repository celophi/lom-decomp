/*
 * akao_xa_stream.c - AKAO streamed-voice (XA program) playback.
 *
 * Plays ADPCM programs through a pair of adjacent SPU voices, fed from main
 * RAM in blocks by SPU transfer and SPU IRQ callbacks. Three sources exist:
 * a RAM buffer played through a double-buffered SPU area (commands 0xE0/0xEC),
 * a program already resident in SPU RAM (0xED, staged by
 * akao_upload_xa_program) and a CD-fed ring of blocks (0xE8). All of them
 * share one state block, g_akao_xa_tracker.
 *
 * akao_driver.h is not included: its AkaoXaTracker only describes the fields
 * akao_cmd.c needs. The full layout is defined below.
 */
#include "common.h"
#include "sdk/libspu.h"
#include "akao.h"

/* Voice mask of SPU voices 22 and 23, the highest pair the stream may use. */
#define XA_TOP_VOICE_PAIR_MASK 0xC00000
/* Number of candidate voice pairs, (12,13) through (22,23). */
#define XA_VOICE_PAIR_COUNT 11
/* Pair index 1 maps to voice 12. */
#define XA_FIRST_VOICE_BIAS 11

/* "AKAO" in little-endian. */
#define XA_AKAO_MAGIC 0x4F414B41

/* g_akao_driver_flags.unk8: reverb/noise/pitch-mod voice masks need rewriting. */
#define AKAO_EFFECT_MASKS_UPDATE_PENDING 0x100

/* D_8004F754 (g_akao_driver_flags.unk4) value selecting mono output. */
#define AKAO_SOUND_MODE_MONO 2

/* XA program flags (AkaoXaProgramHeader.flags, AkaoXaTracker.unk8). */
#define XA_FLAG_STEREO 0x1
#define XA_FLAG_LOOP 0x2
#define XA_FLAG_RING_STREAM 0x1000000

/* SPU addresses of the double-buffered streaming area. */
#define XA_SPU_SILENCE 0x1030
#define XA_SPU_BLOCK_A 0x1100
#define XA_SPU_BLOCK_A_RIGHT 0x1900
#define XA_SPU_BLOCK_B 0x2100
#define XA_SPU_BLOCK_B_RIGHT 0x2900

/* Offset of the program header inside one CD ring block, and of its sample data. */
#define XA_RING_HEADER_OFFSET 0x80
#define XA_RING_DATA_OFFSET 0xD0

/** @brief SFX channel control block (mirrors SfxControl in akao_driver.h). */
typedef struct
{
    u32 unk0; /* active-channel bitmask */
    s32 unk4;
    u32 unk8;
    u32 unkC;
    u32 unk10;
    u8 _pad14[2];
    u16 unk16; /* tick step */
    u32 unk18; /* tick accumulator */
    u32 reverb_mask;
    u32 noise_mask;
    u32 pitch_mod_mask;
    u16 unk28;
} SfxControl;

/** @brief AKAO driver state flags (mirrors AkaoDriverFlags in akao_driver.h). */
typedef struct
{
    u32 unk0;
    u32 unk4;
    u32 unk8; /* pending driver/SPU hardware update flags */
} AkaoDriverFlags;

/**
 * @brief Header of an XA program (0x40 bytes); the ADPCM data follows it.
 *
 * The same header prefixes a program in a RAM buffer, the copy staged by
 * akao_upload_xa_program, and each block of a CD ring stream (at
 * XA_RING_HEADER_OFFSET).
 */
typedef struct
{
    u32 magic;
    u32 unk4;
    u8 _pad08[8];
    u32 sample_size;
    u32 loop_offset;
    u32 flags;
    u16 pitch;
    u8 _pad1E[2];
    u32 unk20;
    u32 right_offset;
    s32 fade_in_ticks;
    u8 _pad2C[0x14];
} AkaoXaProgramHeader;

/**
 * @brief Streamed-voice playback state (g_akao_xa_tracker).
 *
 * unk0 is the next RAM block to upload, unk4 the loop restart address,
 * unk8 the program flags, unkC the voice mask of the stream's voice pair
 * (0 when idle), unk10 its first voice and unk14 the bytes still to upload.
 * unk40 is the Q8 volume (with unk44/unk48 the fade step and ticks), unk4C
 * the Q8 pan and unk58 the SPU pitch. unk20..unk3C are the CD ring counters
 * that akao_cmd.c also drives.
 */
typedef struct
{
    u8* unk0;
    u8* unk4;
    u32 unk8;
    s32 unkC;
    s32 unk10;
    u32 unk14;
    s32 unk18;
    u32 unk1C;
    s32 unk20;
    s32 unk24;
    s32 unk28;
    union
    {
        s32 spu_addr;  /* one-shot programs: SPU base address */
        u8* ring_base; /* CD ring streams: first ring block */
    } unk2C;
    u32 unk30;
    u32 unk34;
    u32 unk38;
    u32 unk3C;
    s32 unk40;
    s32 unk44;
    s32 unk48;
    s32 unk4C;
    u8 _pad50[8];
    s32 unk58;
} AkaoXaTracker;

extern SfxControl g_akao_sfx_control;
extern AkaoDriverFlags g_akao_driver_flags;
extern AkaoChannelState* g_akao_seq_channel0;
extern AkaoXaTracker g_akao_xa_tracker;
extern AkaoXaProgramHeader g_akao_xa_program_staging;
extern volatile s32 g_akao_spu_xfer_pending;
/*
 * Declared as unknown-size arrays on purpose: under -G4 a plain `extern s32`
 * (size <= 4) is treated as small-data, so gcc emits a single-register
 * symbolic load that maspsx expands into a same-register `lui/lw` pair. The
 * target uses large-data %hi/%lo addressing with a compiler-split high
 * register, which the unknown-size array form reproduces.
 *
 * g_akao_xa_pan_current, D_8004F76C and D_8004F7B8 are the addresses of
 * g_akao_xa_tracker.unk40, .unkC and .unk58. The original code addressed
 * them through these separate symbols, so they are kept distinct.
 */
extern s32 g_akao_xa_pan_current[];
extern u32 D_8004F76C[];
extern u32 D_8004F7B8[];
extern s16 D_8003D47C[];
extern u32 D_8004F754;
extern s16 D_8003D37C[];

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
s32 akao_cmd_e5(s32 value0, s32 value1);

void func_8002D764(void);
void func_8002D7C8(void);
void func_8002D978(void);
void func_8002D9A8(void);
void func_8002D9D8(void);
void func_8002DA08(void);
void func_8002E540(void);
void func_8002E6FC(void);
void func_8002E72C(void);

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
    g_akao_seq_channel0->w04.song.voice_alloc_low_mask &= ~channel_mask;
}

/**
 * @brief Opcode 0xE1: set the pitch-jitter depth from one operand byte.
 *        Stores the byte into pitch_scale (0xDA), the depth of the table-driven
 *        pitch perturbation akao_seq_step_opcode applies per note (indexes the
 *        256-entry jitter table D_8003D27C by the free-running modulation tick).
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
void func_8002D140(void)
{
    if (g_akao_xa_tracker.unkC != 0)
    {
        SpuSetIRQ(0);
        SpuSetIRQCallback(0);
        spu_set_key_off(g_akao_xa_tracker.unkC);
        g_akao_sfx_control.noise_mask &= ~g_akao_xa_tracker.unkC;
        g_akao_xa_tracker.unkC = 0;
        g_akao_driver_flags.unk8 |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
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
s32 func_8002D1C4(void)
{
    u32 busy_voices;
    s32 pair;
    u32 pair_mask;

    for (;;)
    {
        pair_mask = XA_TOP_VOICE_PAIR_MASK;
        pair = XA_VOICE_PAIR_COUNT;
        busy_voices = g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10;
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
        if (busy_voices == (g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10))
        {
            return -1;
        }
    }
}

/**
 * @brief SPU transfer callback: upload the second mono block of a RAM buffer.
 */
void func_8002D254(void)
{
    u8* addr = g_akao_xa_tracker.unk0 + 0x800;

    SpuSetTransferStartAddr(XA_SPU_BLOCK_B);
    SpuSetTransferCallback(func_8002D764);
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
 * @param use_noise Non-zero to route the voices through the noise generator.
 */
void func_8002D29C(void* buffer, s32 pan, s32 use_noise)
{
    AkaoXaProgramHeader* program;
    s32 voice;
    s32 voice_mask;
    u8* loop_start;
    s32 size;
    s32 loop_bytes;
    /* Three separate pointers to g_akao_xa_tracker, as in the original code. */
    AkaoXaTracker* xa;
    AkaoXaTracker* stream;
    AkaoXaTracker* stereo_stream;

    voice = func_8002D1C4();
    if (voice == -1)
    {
        return;
    }

    g_akao_xa_tracker.unk4C = pan;
    SpuSetIRQ(0);
    SpuSetIRQCallback(0);

    program = buffer;
    g_akao_xa_tracker.unk0 = (u8*)(program + 1);
    g_akao_xa_tracker.unk14 = program->sample_size;
    voice_mask = (1 << voice) | (1 << (voice + 1));

    g_akao_xa_tracker.unk18 = program->unk20;
    g_akao_xa_tracker.unk10 = voice;
    g_akao_xa_tracker.unkC = voice_mask;
    spu_set_key_off(voice_mask);
    g_akao_xa_tracker.unk8 = program->flags;

    g_akao_xa_tracker.unk58 = program->pitch;
    g_akao_xa_tracker.unk10 = voice;
    g_akao_xa_tracker.unkC = voice_mask;
    spu_set_key_off(voice_mask);

    SpuSetTransferMode(0);
    SpuSetTransferStartAddr(XA_SPU_BLOCK_A);
    g_akao_spu_xfer_pending = 1;
    xa = &g_akao_xa_tracker;

    if (g_akao_xa_tracker.unk8 & XA_FLAG_LOOP)
    {
        loop_start = g_akao_xa_tracker.unk0 + program->loop_offset;
    }
    else
    {
        loop_start = 0;
    }

    stream = &g_akao_xa_tracker;
    xa->unk4 = loop_start;

    if (stream->unk8 & XA_FLAG_STEREO)
    {
        stereo_stream = stream;
        if (stream->unk8 & XA_FLAG_LOOP)
        {
            loop_bytes = stream->unk14 - (program->loop_offset >> 1);
        }
        else
        {
            loop_bytes = 0;
        }
        stereo_stream->unk1C = loop_bytes;
        SpuSetTransferCallback(func_8002D7C8);
        size = 0x2000;
    }
    else
    {
        if (stream->unk8 & XA_FLAG_LOOP)
        {
            loop_bytes = stream->unk14 - program->loop_offset;
        }
        else
        {
            loop_bytes = 0;
        }
        stream->unk1C = loop_bytes;
        SpuSetTransferCallback(func_8002D254);
        size = 0x800;
    }

    SpuWrite(g_akao_xa_tracker.unk0, size);

    if (use_noise != 0)
    {
        g_akao_sfx_control.noise_mask |= g_akao_xa_tracker.unkC;
    }
    else
    {
        g_akao_sfx_control.noise_mask &= ~g_akao_xa_tracker.unkC;
    }

    g_akao_sfx_control.pitch_mod_mask &= ~D_8004F76C[0];
    g_akao_sfx_control.reverb_mask &= ~D_8004F76C[0];
    g_akao_driver_flags.unk8 |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
}

/**
 * @brief Program one streamed voice: volume, pitch, addresses and envelope.
 * @param voice SPU voice index.
 * @param mode 1 = left channel, 2 = right channel, 3 = mono centre, other = panned mono.
 * @param start SPU start address.
 * @param end SPU repeat address.
 */
void func_8002D4D8(s32 voice, s32 mode, u32 start, u32 end)
{
    s16 volume_left;
    s16 volume_right;

    if (D_8004F754 & AKAO_SOUND_MODE_MONO)
    {
        s32 volume = (g_akao_xa_pan_current[0] * D_8003D47C[0]) >> 16;
        volume_right = volume;
        volume_left = volume;
    }
    else if (mode == 1)
    {
        volume_right = 0;
        volume_left = (u32)g_akao_xa_pan_current[0] >> 1;
    }
    else if (mode == 2)
    {
        volume_left = 0;
        volume_right = (u32)g_akao_xa_pan_current[0] >> 1;
    }
    else if (mode == 3)
    {
        s32 volume = (g_akao_xa_pan_current[0] >> 1) << 16;
        volume_right = (volume >> 17) + (volume >> 18);
        volume_left = volume_right;
    }
    else
    {
        s32 pan = (g_akao_xa_tracker.unk4C >> 8) & 0xFF;
        s32 level = g_akao_xa_tracker.unk40;
        volume_left = (u32)(level * D_8003D37C[pan]) >> 16;
        pan ^= 0xFF;
        volume_right = (u32)(level * D_8003D37C[pan]) >> 16;
    }

    spu_set_voice_volume(voice, volume_left, volume_right, 0);
    spu_set_voice_pitch(voice, D_8004F7B8[0]);
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
void func_8002D694(s32 uploaded, s32 irq_addr, SpuIRQCallbackProc next_cb)
{
    if (g_akao_xa_tracker.unkC != 0)
    {
        SpuSetTransferCallback(0);
        g_akao_spu_xfer_pending = 0;
        if (g_akao_xa_tracker.unk14 > 0x1000)
        {
            g_akao_xa_tracker.unk14 -= 0x1000;
            g_akao_xa_tracker.unk0 += uploaded;
            SpuSetIRQCallback(next_cb);
        }
        else
        {
            SpuSetIRQCallback(func_8002D140);
            irq_addr = XA_SPU_SILENCE;
            g_akao_xa_tracker.unk14 = 0;
        }
        SpuSetIRQAddr(irq_addr + 8);
        spu_set_key_on(D_8004F76C[0]);
        SpuSetIRQ(1);
    }
}

/**
 * @brief SPU transfer callback: start a mono RAM-buffer stream.
 */
void func_8002D764(void)
{
    func_8002D4D8(g_akao_xa_tracker.unk10, 0, XA_SPU_BLOCK_A, XA_SPU_BLOCK_B);
    func_8002D4D8(g_akao_xa_tracker.unk10 + 1, 0, XA_SPU_BLOCK_A, XA_SPU_BLOCK_B);
    func_8002D694(0x1000, XA_SPU_BLOCK_B, func_8002D978);
}

/**
 * @brief SPU transfer callback: start a stereo RAM-buffer stream.
 */
void func_8002D7C8(void)
{
    func_8002D4D8(g_akao_xa_tracker.unk10, 1, XA_SPU_BLOCK_A, XA_SPU_BLOCK_B);
    func_8002D4D8(g_akao_xa_tracker.unk10 + 1, 2, XA_SPU_BLOCK_A_RIGHT, XA_SPU_BLOCK_B_RIGHT);
    func_8002D694(0x2000, XA_SPU_BLOCK_B, func_8002D9D8);
}

/**
 * @brief Upload the next RAM-buffer block into the SPU half that just finished.
 * @param start SPU address of the left (or mono) half.
 * @param end SPU address of the right half.
 * @param size Bytes to upload.
 * @param cb SPU IRQ callback for the following block.
 */
void func_8002D82C(u32 start, u32 end, u32 size, SpuIRQCallbackProc cb)
{
    if (g_akao_xa_tracker.unkC == 0)
    {
        return;
    }
    if (g_akao_xa_tracker.unk14 == 0)
    {
        return;
    }

    SpuSetTransferStartAddr(start);
    akao_spu_arm_xfer();
    SpuWrite(g_akao_xa_tracker.unk0, size);
    SpuSetIRQ(0);

    if (g_akao_xa_tracker.unk14 > 0x800)
    {
        SpuSetIRQCallback(cb);
        g_akao_xa_tracker.unk14 -= 0x800;
        g_akao_xa_tracker.unk0 += size;
    }
    else if (g_akao_xa_tracker.unk4 != 0)
    {
        SpuSetIRQCallback(cb);
        g_akao_xa_tracker.unk0 = g_akao_xa_tracker.unk4;
        g_akao_xa_tracker.unk14 = g_akao_xa_tracker.unk1C;
    }
    else
    {
        SpuSetIRQCallback(func_8002D140);
        end = XA_SPU_SILENCE;
        start = end;
        g_akao_xa_tracker.unk14 = 0;
    }

    spu_set_voice_repeat_addr(g_akao_xa_tracker.unk10, start);
    spu_set_voice_repeat_addr(g_akao_xa_tracker.unk10 + 1, end);
    SpuSetIRQAddr(start + 8);
    SpuSetIRQ(1);
}

/**
 * @brief SPU IRQ callback: refill mono block A.
 */
void func_8002D978(void)
{
    func_8002D82C(XA_SPU_BLOCK_A, XA_SPU_BLOCK_A, 0x800, func_8002D9A8);
}

/**
 * @brief SPU IRQ callback: refill mono block B.
 */
void func_8002D9A8(void)
{
    func_8002D82C(XA_SPU_BLOCK_B, XA_SPU_BLOCK_B, 0x800, func_8002D978);
}

/**
 * @brief SPU IRQ callback: refill stereo block A.
 */
void func_8002D9D8(void)
{
    func_8002D82C(XA_SPU_BLOCK_A, XA_SPU_BLOCK_A_RIGHT, 0x1000, func_8002DA08);
}

/**
 * @brief SPU IRQ callback: refill stereo block B.
 */
void func_8002DA08(void)
{
    func_8002D82C(XA_SPU_BLOCK_B, XA_SPU_BLOCK_B_RIGHT, 0x1000, func_8002D9D8);
}

/**
 * @brief Command 0xE0: play an XA program from a RAM buffer.
 * @param params Queued command parameters: buffer, Q8 pan, noise flag.
 */
void func_8002DA38(s32* params)
{
    func_8002D29C((void*)params[0], params[1], params[2]);
    g_akao_sfx_control.unk0 &= ~D_8004F76C[0];
}

/**
 * @brief Command 0xE2: stop the streamed voice pair.
 */
void func_8002DA80(void)
{
    func_8002D140();
}

/**
 * @brief Command 0xE4: set the streamed voice volume immediately.
 * @param params Queued command parameters: Q8 volume.
 */
void func_8002DAA0(s32* params)
{
    AkaoXaTracker* xa = &g_akao_xa_tracker;
    s32 val = params[0];

    xa->unk48 = 0;
    xa->unk40 = val;
    if (xa->unkC != 0)
    {
        spu_set_voice_volume(xa->unk10, (val << 15) >> 16, 0, 0);
        spu_set_voice_volume(xa->unk10 + 1, 0, (xa->unk40 << 15) >> 16, 0);
    }
}

/**
 * @brief Command 0xE5: fade the streamed voice volume.
 * @param params Queued command parameters: fade ticks (0 means 1), Q8 target volume.
 */
void func_8002DB10(s32* params)
{
    s16 ticks;
    s16 delta;

    ticks = 1;
    if (params[0] != 0)
    {
        ticks = params[0];
    }
    delta = (u16)params[1] - (u16)g_akao_xa_tracker.unk40;
    g_akao_xa_tracker.unk44 = (s16)(delta / ticks);
    g_akao_xa_tracker.unk48 = ticks;
}

/**
 * @brief Command 0xE6: set the streamed voice pan.
 * @param params Queued command parameters: Q8 pan.
 */
void func_8002DB90(s32* params)
{
    AkaoXaTracker* xa;
    s32 pan_word;
    s32 pan;
    s32 volume_left;
    s32 volume_right;
    s32 volume;

    xa = &g_akao_xa_tracker;
    pan_word = params[0];
    xa->unk4C = pan_word;
    if (xa->unkC != 0)
    {
        if (D_8004F754 & AKAO_SOUND_MODE_MONO)
        {
            volume = (xa->unk40 * D_8003D47C[0]) >> 16;
            spu_set_voice_volume(xa->unk10, volume, volume, 0);
            spu_set_voice_volume(xa->unk10 + 1, volume, volume, 0);
        }
        else if (xa->unk8 & XA_FLAG_STEREO)
        {
            volume = xa->unk40;
            volume <<= 15;
            volume >>= 16;
            spu_set_voice_volume(xa->unk10, volume, 0, 0);
            spu_set_voice_volume(xa->unk10 + 1, 0, volume, 0);
        }
        else
        {
            pan = (pan_word >> 8) & 0xFF;
            volume_left = (xa->unk40 * D_8003D37C[pan]) >> 16;
            pan ^= 0xFF;
            volume_right = (xa->unk40 * D_8003D37C[pan]) >> 16;
            spu_set_voice_volume(xa->unk10, volume_left, volume_right, 0);
            spu_set_voice_volume(xa->unk10 + 1, volume_left, volume_right, 0);
        }
    }
}

/**
 * @brief SPU IRQ callback: a one-shot program reached its end; stop at the silence block.
 */
void func_8002DCDC(void)
{
    SpuSetIRQAddr(XA_SPU_SILENCE + 8);
    SpuSetIRQCallback(func_8002D140);
}

/**
 * @brief SPU transfer callback: finish a one-shot upload and key the voices on.
 */
void func_8002DD08(void)
{
    s32 addr;

    if (g_akao_xa_tracker.unk14 > 0x2000)
    {
        SpuSetTransferStartAddr(g_akao_xa_tracker.unk2C.spu_addr + 0x2000);
        akao_spu_write(g_akao_xa_tracker.unk0, g_akao_xa_tracker.unk14 - 0x2000);
        addr = g_akao_xa_tracker.unk2C.spu_addr + 0x1FF8;
    }
    else
    {
        addr = g_akao_xa_tracker.unk2C.spu_addr + (g_akao_xa_tracker.unk14 >> 1) + 8;
    }

    func_8002D4D8(g_akao_xa_tracker.unk10, 0, g_akao_xa_tracker.unk2C.spu_addr, XA_SPU_SILENCE);
    func_8002D4D8(g_akao_xa_tracker.unk10 + 1, 0, g_akao_xa_tracker.unk2C.spu_addr, XA_SPU_SILENCE);
    SpuSetIRQAddr(addr);
    SpuSetIRQCallback(func_8002DCDC);
    spu_set_key_on(g_akao_xa_tracker.unkC);
    SpuSetIRQ(1);
}

/**
 * @brief Upload a whole XA program to SPU RAM and play it once.
 * @param program XA program header; the ADPCM data follows it.
 * @param pan Q8 pan.
 * @param spu_addr SPU destination address.
 * @param use_noise Non-zero to route the voices through the noise generator.
 */
void func_8002DDDC(AkaoXaProgramHeader* program, s32 pan, s32 spu_addr, s32 use_noise)
{
    s32 voice;
    AkaoXaTracker* xa;
    s32 voice_mask;
    u32 value; /* old voice mask, then the first chunk size; one variable in the original */

    voice = func_8002D1C4();
    if (voice == -1)
    {
        return;
    }

    SpuSetIRQ(0);
    SpuSetIRQCallback(0);

    xa = &g_akao_xa_tracker;
    value = xa->unkC;
    xa->unk4C = pan;
    xa->unk0 = (u8*)(program + 1);
    xa->unk14 = program->sample_size;
    voice_mask = (1 << voice) | (1 << (voice + 1));
    xa->unk18 = program->unk20;
    xa->unk10 = voice;
    xa->unkC = voice_mask;
    spu_set_key_off(voice_mask | value);
    xa->unk8 = program->flags;
    xa->unk58 = program->pitch;
    xa->unk2C.spu_addr = spu_addr;
    SpuSetTransferMode(0);
    SpuSetTransferStartAddr(spu_addr);
    g_akao_spu_xfer_pending = 1;
    SpuSetTransferCallback(func_8002DD08);

    value = xa->unk14;
    if (value > 0x2000)
    {
        value = 0x2000;
    }
    SpuWrite(xa->unk0, value);
    xa->unk0 += value;

    if (use_noise != 0)
    {
        g_akao_sfx_control.noise_mask |= xa->unkC;
    }
    else
    {
        g_akao_sfx_control.noise_mask &= ~xa->unkC;
    }
    g_akao_sfx_control.pitch_mod_mask &= ~D_8004F76C[0];
    g_akao_sfx_control.reverb_mask &= ~D_8004F76C[0];
    g_akao_driver_flags.unk8 |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
}

/**
 * @brief Play the XA program staged in SPU RAM by akao_upload_xa_program.
 * @param pan Q8 pan; not used.
 * @param use_noise Non-zero to route the voices through the noise generator.
 */
void func_8002DFA4(s32 pan, s32 use_noise)
{
    s32 voice;
    s32 old_mask;
    s32 voice_mask;
    AkaoXaProgramHeader* program;

    if (g_akao_xa_program_staging.unk20 != 0)
    {
        voice = func_8002D1C4();
        if (voice != -1)
        {
            SpuSetIRQ(0);
            SpuSetIRQCallback(0);
            old_mask = g_akao_xa_tracker.unkC;
            voice_mask = (1 << voice) | (1 << (voice + 1));
            g_akao_xa_tracker.unk10 = voice;
            g_akao_xa_tracker.unkC = voice_mask;
            spu_set_key_off(voice_mask | old_mask);
            program = &g_akao_xa_program_staging;
            g_akao_xa_tracker.unk8 = program->flags;
            g_akao_xa_tracker.unk58 = program->pitch;
            if (use_noise != 0)
            {
                g_akao_sfx_control.noise_mask |= g_akao_xa_tracker.unkC;
            }
            else
            {
                g_akao_sfx_control.noise_mask &= ~g_akao_xa_tracker.unkC;
            }
            g_akao_sfx_control.pitch_mod_mask &= ~g_akao_xa_tracker.unkC;
            g_akao_sfx_control.reverb_mask &= ~g_akao_xa_tracker.unkC;
            g_akao_driver_flags.unk8 |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
            SpuSetIRQAddr(program->unk20 + (program->sample_size >> 1) + 8);
            SpuSetIRQCallback(func_8002DCDC);

            if (program->flags & XA_FLAG_STEREO)
            {
                if (program->flags & XA_FLAG_LOOP)
                {
                    func_8002D4D8(g_akao_xa_tracker.unk10, 1, program->unk20, program->unk20 + program->loop_offset);
                    func_8002D4D8(g_akao_xa_tracker.unk10 + 1, 2, program->unk20 + program->right_offset, (program->unk20 + program->right_offset) + program->loop_offset);
                }
                else
                {
                    func_8002D4D8(g_akao_xa_tracker.unk10, 1, program->unk20, XA_SPU_SILENCE);
                    func_8002D4D8(g_akao_xa_tracker.unk10 + 1, 2, program->unk20 + program->right_offset, XA_SPU_SILENCE);
                }
            }
            else if (program->flags & XA_FLAG_LOOP)
            {
                func_8002D4D8(g_akao_xa_tracker.unk10, 3, program->unk20, program->unk20 + program->loop_offset);
                func_8002D4D8(g_akao_xa_tracker.unk10 + 1, 3, program->unk20, program->unk20 + program->loop_offset);
            }
            else
            {
                func_8002D4D8(g_akao_xa_tracker.unk10, 3, program->unk20, XA_SPU_SILENCE);
                func_8002D4D8(g_akao_xa_tracker.unk10 + 1, 3, program->unk20, XA_SPU_SILENCE);
            }
            spu_set_key_on(D_8004F76C[0]);
            SpuSetIRQ(1);
        }
    }
}

/**
 * @brief Command 0xEC: upload an XA program to SPU RAM and play it once.
 * @param params Queued command parameters: buffer, Q8 pan, SPU address, noise flag.
 */
void func_8002E204(s32* params)
{
    func_8002DDDC((AkaoXaProgramHeader*)params[0], params[1], params[2], params[3]);
    g_akao_sfx_control.unk0 &= ~D_8004F76C[0];
}

/**
 * @brief Command 0xED: play the program staged in SPU RAM.
 * @param params Queued command parameters: Q8 pan, noise flag.
 */
void func_8002E250(s32* params)
{
    func_8002DFA4(params[0], params[1]);
    g_akao_sfx_control.unk0 &= ~D_8004F76C[0];
}

/**
 * @brief Advance a ring block index, wrapping at the ring length.
 * @param block_index Block index to advance.
 * @return The new block index.
 */
s32 func_8002E294(u32* block_index)
{
    g_akao_xa_tracker.unk28++;
    (*block_index)++;
    if (*block_index > g_akao_xa_tracker.unk3C - 1)
    {
        *block_index = 0;
    }
    return *block_index;
}

/**
 * @brief Start playing a CD ring stream from its first block.
 */
void func_8002E2E8(void)
{
    s32 voice;
    AkaoXaProgramHeader* program;
    s32 voice_mask;

    voice = func_8002D1C4();
    if (voice == -1)
    {
        return;
    }

    SpuSetIRQ(0);
    SpuSetIRQCallback(0);

    program = (AkaoXaProgramHeader*)(g_akao_xa_tracker.unk2C.ring_base + XA_RING_HEADER_OFFSET);

    if (program->fade_in_ticks != 0 && g_akao_xa_tracker.unk48 == 0)
    {
        s32 volume = g_akao_xa_tracker.unk40;
        g_akao_xa_tracker.unk40 = 0;
        akao_cmd_e5(program->fade_in_ticks, volume >> 8);
    }

    g_akao_xa_tracker.unk0 = g_akao_xa_tracker.unk2C.ring_base;
    g_akao_xa_tracker.unk14 = program->sample_size;
    g_akao_xa_tracker.unk18 = program->unk20;
    g_akao_xa_tracker.unk10 = voice;
    voice_mask = (1 << voice) | (1 << (voice + 1));
    g_akao_xa_tracker.unkC = voice_mask;
    g_akao_xa_tracker.unk34 = 0;
    spu_set_key_off(voice_mask);
    g_akao_xa_tracker.unk8 = program->flags;
    g_akao_xa_tracker.unk58 = program->pitch;
    SpuSetTransferMode(0);
    SpuSetTransferStartAddr(XA_SPU_BLOCK_A);
    g_akao_spu_xfer_pending = 1;
    SpuSetTransferCallback(func_8002E540);
    SpuWrite(g_akao_xa_tracker.unk0 + XA_RING_DATA_OFFSET, 0x2000);

    g_akao_sfx_control.noise_mask &= ~g_akao_xa_tracker.unkC;
    g_akao_sfx_control.pitch_mod_mask &= ~g_akao_xa_tracker.unkC;
    g_akao_sfx_control.reverb_mask &= ~g_akao_xa_tracker.unkC;
    g_akao_driver_flags.unk8 |= AKAO_EFFECT_MASKS_UPDATE_PENDING;

    func_8002E294(&g_akao_xa_tracker.unk34);
    func_8002E294(&g_akao_xa_tracker.unk34);
}

/**
 * @brief Arm the SPU IRQ for the next ring block and key the voices on.
 * @param uploaded Bytes to advance the ring cursor by.
 * @param irq_addr SPU address whose playback raises the next IRQ.
 * @param next_cb SPU IRQ callback that uploads the next block.
 */
void func_8002E478(s32 uploaded, s32 irq_addr, SpuIRQCallbackProc next_cb)
{
    if (g_akao_xa_tracker.unkC != 0)
    {
        SpuSetTransferCallback(0);
        g_akao_spu_xfer_pending = 0;
        if (g_akao_xa_tracker.unk14 >= 0xE61)
        {
            g_akao_xa_tracker.unk0 += uploaded;
            SpuSetIRQCallback(next_cb);
        }
        else
        {
            SpuSetIRQCallback(func_8002D140);
            irq_addr = XA_SPU_SILENCE;
        }
        SpuSetIRQAddr(irq_addr + 8);
        spu_set_key_on(D_8004F76C[0]);
        SpuSetIRQ(1);
    }
}

/**
 * @brief SPU transfer callback: start a CD ring stream.
 */
void func_8002E540(void)
{
    func_8002D4D8(g_akao_xa_tracker.unk10, 1, XA_SPU_BLOCK_A, XA_SPU_BLOCK_B);
    func_8002D4D8(g_akao_xa_tracker.unk10 + 1, 2, XA_SPU_BLOCK_A_RIGHT, XA_SPU_BLOCK_B_RIGHT);
    func_8002E478(0x2000, XA_SPU_BLOCK_B, func_8002E6FC);
}

/**
 * @brief Upload the next CD ring block into the SPU half that just finished.
 * @param start SPU address of the left half.
 * @param end SPU address of the right half.
 * @param block_size Ring block size in bytes.
 * @param cb SPU IRQ callback for the following block.
 */
void func_8002E5A4(s32 start, s32 end, s32 block_size, SpuIRQCallbackProc cb)
{
    u8* base;
    AkaoXaProgramHeader* hdr;

    if (g_akao_xa_tracker.unkC != 0)
    {
        base = g_akao_xa_tracker.unk0;
        hdr = (AkaoXaProgramHeader*)(base + XA_RING_HEADER_OFFSET);
        if (hdr->magic == XA_AKAO_MAGIC)
        {
            SpuSetIRQ(0);
            SpuSetTransferStartAddr(start);
            akao_spu_arm_xfer();
            func_8002E294(&g_akao_xa_tracker.unk34);
            SpuWrite(g_akao_xa_tracker.unk0 + XA_RING_DATA_OFFSET, block_size - XA_RING_DATA_OFFSET);
            g_akao_xa_tracker.unk20 = hdr->unk4;
            g_akao_xa_tracker.unk18 = hdr->unk20;
            if (hdr->sample_size > hdr->unk20)
            {
                SpuSetIRQCallback(cb);
                g_akao_xa_tracker.unk0 += block_size;
                if (g_akao_xa_tracker.unk34 == 0)
                {
                    g_akao_xa_tracker.unk0 = g_akao_xa_tracker.unk2C.ring_base;
                }
            }
            else
            {
                SpuSetIRQCallback(func_8002D140);
                end = XA_SPU_SILENCE;
                start = XA_SPU_SILENCE;
            }
            spu_set_voice_repeat_addr(g_akao_xa_tracker.unk10, start);
            spu_set_voice_repeat_addr(g_akao_xa_tracker.unk10 + 1, end);
            SpuSetIRQAddr(start + 8);
            SpuSetIRQ(1);
        }
    }
}

/**
 * @brief SPU IRQ callback: refill ring block A.
 */
void func_8002E6FC(void)
{
    func_8002E5A4(XA_SPU_BLOCK_A, XA_SPU_BLOCK_A_RIGHT, 0x1000, func_8002E72C);
}

/**
 * @brief SPU IRQ callback: refill ring block B.
 */
void func_8002E72C(void)
{
    func_8002E5A4(XA_SPU_BLOCK_B, XA_SPU_BLOCK_B_RIGHT, 0x1000, func_8002E6FC);
}

/**
 * @brief Command 0xE8: prepare a CD ring stream.
 * @param params Queued command parameters: first ring block, ring size.
 */
void func_8002E75C(s32* params)
{
    func_8002D140();
    g_akao_xa_tracker.unk8 = XA_FLAG_RING_STREAM;
    g_akao_xa_tracker.unk2C.ring_base = (u8*)params[0];
    g_akao_xa_tracker.unk30 = params[1];
}
