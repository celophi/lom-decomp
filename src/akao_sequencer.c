#include "akao_sequencer.h"
#include "akao_voice.h"

/* Channel-role AkaoChannelState.flags bits. */
#define AKAO_CH_PITCH_LFO 0x01
#define AKAO_CH_VOLUME_LFO 0x02
#define AKAO_CH_PAN_LFO 0x04
#define AKAO_CH_DRUM_MODE 0x08
#define AKAO_CH_PITCH_SIDECHAIN 0x10
#define AKAO_CH_PITCH_VOLUME_SIDECHAIN 0x20
#define AKAO_CH_FULL_GATE 0x40
#define AKAO_CH_PAN_BIAS 0x800
#define AKAO_CH_KEY_MAP 0x1000
#define AKAO_CH_SFX_PITCH_MOD 0x10000
#define AKAO_CH_DEFERRED_STOP 0x100000
#define AKAO_CH_STOP_PENDING 0x200000
#define AKAO_CH_ADSR_ATTACK 0x01000000
#define AKAO_CH_ADSR_SUSTAIN_RATE 0x08000000
#define AKAO_CH_ADSR_RELEASE_RATE 0x10000000

/** @brief Explicit ADSR overrides, cleared when a new articulation is loaded. */
#define AKAO_CH_ADSR_OVERRIDE_MASK (AKAO_CH_ADSR_ATTACK | AKAO_CH_ADSR_SUSTAIN_RATE | AKAO_CH_ADSR_RELEASE_RATE)

/** @brief Flags cleared when a plain articulation is selected. */
#define AKAO_CH_ARTICULATION_MASK (AKAO_CH_DRUM_MODE | AKAO_CH_KEY_MAP | AKAO_CH_ADSR_OVERRIDE_MASK)

/** @brief Read a little-endian signed 16-bit bytecode operand. */
#define AKAO_READ_S16(p) ((s16)((p)[0] | ((p)[1] << 8)))

/** @brief GetRCnt() spec for root counter 2 (the driver's tick timer). */
#define RCNT_SPEC_2 0xF2000002

/** @brief Root counter 2 period in timer ticks; wraps the IRQ timing delta. */
#define AKAO_TICK_TIMER_PERIOD 0x44E8

/** @brief Number of channel slots in a song's channel array. */
#define AKAO_SEQ_CHANNEL_COUNT 32

/** @brief SPU CD-audio input volume registers (left/right). */
#define SPU_CD_VOLUME_LEFT (*(s16*)0x1F801DB0)
#define SPU_CD_VOLUME_RIGHT (*(s16*)0x1F801DB2)

/** @brief The primary song's channel array, viewed as channel-state slots. */
#define AKAO_SEQ_CHANNELS ((AkaoChannelState*)g_akao_seq_channels)

/**
 * @brief Channel-role note-start expression preset (s32 at 0x5C).
 * @note Overlaps the song-role @c tempo_fade_ticks / @c unk5E halfwords.
 */
#define AKAO_CHANNEL_EXPRESSION_PRESET(channel) (*(s32*)&(channel)->tempo_fade_ticks)

/**
 * @brief Channel-role note-start expression step (s32 at 0x60).
 * @note Overlaps the song-role @c unk60 / @c noise_freq halfwords.
 */
#define AKAO_CHANNEL_EXPRESSION_PRESET_STEP(channel) (*(s32*)&(channel)->unk60)

/**
 * @brief SFX-role tick counter (u32 at 0x58).
 * @note Overlaps the song-role @c unk58 / @c master_vol_fade_ticks halfwords.
 */
#define AKAO_CHANNEL_SFX_TICKS(channel) (*(u32*)&(channel)->unk58)

/** @brief The driver articulation table, viewed as articulation entries. */
#define AKAO_ARTICULATIONS ((AkaoArticulation*)g_akao_articulation_slots)

/**
 * @brief One 8-byte note slot in the per-channel note table at
 *        @c channel->flags (the song-role note table). Each entry encodes
 *        a packed set of per-note SPU voice parameters used by
 *        @c akao_channel_start_note. Local to this file because it is the
 *        only consumer.
 */
typedef struct
{
    u8 articulation;
    u8 key;
    u8 attack_rate;
    u8 sustain_rate;
    u8 sustain_mode;
    u8 release_rate;
    u8 volume_scale;
    u8 pan_and_noise;
} AkaoNoteArticulationSlot;

/**
 * @brief One 8-byte entry of a key-to-articulation map (ext opcode FE 14).
 *
 * Entries are sorted by key range; the list ends at the first entry whose
 * following entry has a zero @c sustain_mode.
 */
typedef struct
{
    u8 articulation;
    u8 key_low;
    u8 key_high;
    u8 attack_rate;
    u8 sustain_rate;
    u8 sustain_mode;
    u8 release_rate;
    u8 volume_scale;
} AkaoKeyMapEntry;

/** @brief Parameter block passed to akao_sfx_play by ext opcode FE 0B. */
typedef struct
{
    u32 unk0;
    u32 unk4;
    u32 pan;
    s32 expression;
} AkaoSfxPlayParams;

/* Defined in the sdata segment (asm/data/sdata.data.s) at their gp-relative
 * addresses near gp_value 0x8003EC14; declared extern here so akao_sequencer does not
 * emit a second (.bss) definition. */
extern u16 g_akao_irq_frame_counter;
extern s32 D_8004D40C[];
extern u32 D_8004F758[];
extern s32 D_8004D408[];
extern s32 D_8003EC18;

void akao_seq_step_opcode(AkaoChannelState* channel, s32 channel_mask);
void akao_flush_voice_key_offs(void);
void akao_seq_flag_volume_update(AkaoChannelState* song, AkaoChannelState* channels);

/** @brief 12-entry semitone pitch-ratio table indexed by note % 12 in akao_compute_pitch. */
extern u32 g_akao_pitch_table[];

/** @brief 16-entry table of pitch, volume, and pan LFO waveform streams. */
extern s32 g_akao_lfo_waveforms[];

/** @brief Operand-length table for extended (0xFE-prefixed) opcodes; 0 = needs special handling. */
extern u8 g_akao_opcode_len_table_ext[];
/** @brief Operand-length table for primary opcodes 0xA0..0xFF; 0 = needs special handling. */
extern u8 g_akao_opcode_len_table[];
/** @brief Primary opcode dispatch table indexed by (opcode - 0xA0). */
extern void (*g_akao_opcode_handlers[])(AkaoChannelState*, s32);
/** @brief Extended (0xFE-prefixed) opcode dispatch table indexed by the following byte. */
extern void (*g_akao_opcode_handlers_ext[])(AkaoChannelState*, s32);
/** @brief Default note-duration (gate-time) table indexed by opcode % 11. */
extern u16 g_akao_note_duration_table[];
/** @brief 256-entry pitch jitter table indexed by g_akao_cdvol_tick. */
extern u8 D_8003D27C[];

extern AkaoSfxPlayParams D_8004D3A0;
extern s16 D_8004D428[];

void akao_sfx_play(AkaoSfxPlayParams* params, u8* seq_data0, u8* seq_data1, s32 skip_stop);

/**
 * @brief Write the current CD-audio volume to both SPU CD volume registers.
 * @see decomp.me (100%) https://decomp.me/scratch/hjYpL
 */
void akao_apply_cdvol_to_spu(void)
{
    s32 volume = g_akao_cdvol_current;

    SPU_CD_VOLUME_LEFT = volume;
    SPU_CD_VOLUME_RIGHT = volume;
}

/**
 * @brief Word-wise block copy (4 words per iteration, then a 1-word tail).
 * @param src Source buffer.
 * @param dst Destination buffer.
 * @param num_bytes Byte count; converted to a word count internally.
 * @see decomp.me (100%) https://decomp.me/scratch/TxNq3
 */
void akao_copy_bytes(s32* src, s32* dst, u32 num_bytes)
{

    num_bytes >>= 2;

    while ((num_bytes >> 2) != 0)
    {

        s32 second = src[1];
        s32 third = src[2];
        s32 fourth = src[3];
        s32 first = src[0];

        dst[0] = first;
        dst[1] = second;
        dst[2] = third;
        dst[3] = fourth;

        src += 4;
        dst += 4;
        num_bytes -= 4;
    }

    while (num_bytes != 0)
    {
        *dst = *src;
        src++;
        dst++;
        num_bytes--;
    }
}

/**
 * @brief Advance the 4-frame fades: CD volume, XA pan, master pan/volume,
 *        song volume and the per-SFX-channel volume, pan-bias and 0x40 fades.
 * @see decomp.me (100%) https://decomp.me/scratch/07M97
 */
void akao_tick_fades(void)
{
    AkaoXaTracker* xa;
    AkaoChannelState* channel;
    s32 volume;
    s32 next;
    u32 active;
    /* Mastervol channel count, then the SFX channel bit. One shared variable
     * is what gives the original register allocation. */
    s32 n;

    g_akao_cdvol_tick = (g_akao_cdvol_tick + 1) & 0xFF;
    if (g_akao_cdvol_fade_ticks != 0)
    {
        g_akao_cdvol_fade_ticks--;
        g_akao_cdvol_acc += g_akao_cdvol_step;
        akao_apply_cdvol_to_spu();
    }

    xa = &g_akao_xa_tracker;
    if ((xa->unkC != 0) && (xa->unk48 != 0))
    {
        xa->unk48--;
        volume = xa->unk40;
        next = volume + xa->unk44;
        if ((next & 0xFF00) != (xa->unk40 & 0xFF00))
        {
            if (D_8004F754[0] & 2)
            {
                volume = (volume * D_8003D47C[0]) >> 16;
                spu_set_voice_volume(xa->unk10, volume, volume, 0);
                spu_set_voice_volume(xa->unk10 + 1, volume, volume, 0);
            }
            else
            {
                volume = next << 15;
                volume >>= 16;
                spu_set_voice_volume(xa->unk10, volume, 0, 0);
                spu_set_voice_volume(xa->unk10 + 1, 0, volume, 0);
            }
        }
        g_akao_xa_pan_current[0] = next & 0xFFFF;
    }

    if (g_akao_masterpan_fade_ticks != 0)
    {
        g_akao_masterpan_fade_ticks--;
        g_akao_masterpan_acc += g_akao_masterpan_step;
    }

    if (g_akao_mastervol_fade_ticks != 0)
    {
        g_akao_mastervol_fade_ticks--;
        next = g_akao_mastervol_acc + g_akao_mastervol_step;
        n = AKAO_SEQ_CHANNEL_COUNT;
        if ((next & 0xFF0000) != (g_akao_mastervol_acc & 0xFF0000))
        {
            channel = AKAO_SEQ_CHANNELS;
            for (; n != 0; n--)
            {
                channel->update_flags |= 0x10;
                channel++;
            }
        }
        g_akao_mastervol_acc = next;
    }

    if ((g_akao_seq_channel0->w04.song.active_mask != 0) && ((s16)g_akao_seq_channel0->unk58 != 0))
    {
        g_akao_seq_channel0->unk58--;
        next = g_akao_seq_channel0->pitch_slide_step + g_akao_seq_channel0->detune_pitch_delta;
        if ((next & 0x7F0000) != (g_akao_seq_channel0->pitch_slide_step & 0x7F0000))
        {
            akao_seq_flag_volume_update(g_akao_seq_channel0, AKAO_SEQ_CHANNELS);
        }
        g_akao_seq_channel0->pitch_slide_step = next;
    }

    if ((g_akao_seq_channel1 != 0) && (g_akao_seq_channel1->w04.song.active_mask != 0) && ((s16)g_akao_seq_channel1->unk58 != 0))
    {
        g_akao_seq_channel1->unk58--;
        next = g_akao_seq_channel1->pitch_slide_step + g_akao_seq_channel1->detune_pitch_delta;
        if ((next & 0x7F0000) != (g_akao_seq_channel1->pitch_slide_step & 0x7F0000))
        {
            akao_seq_flag_volume_update(g_akao_seq_channel1, (AkaoChannelState*)g_akao_pending_channels);
        }
        g_akao_seq_channel1->pitch_slide_step = next;
    }

    if (g_akao_sfx_control.unk0 != 0)
    {
        active = g_akao_sfx_control.unk0;
        channel = (AkaoChannelState*)g_sfx_channels;
        n = 0x1000;
        do
        {
            if (active & n)
            {
                if (channel->unk8E != 0)
                {
                    channel->unk8E--;
                    next = (s16)channel->volume_scale + (s16)channel->unkE6;
                    if ((next & 0xFF00) != ((s16)channel->volume_scale & 0xFF00))
                    {
                        channel->update_flags |= 3;
                    }
                    channel->volume_scale = next;
                }
                if (channel->pan_bias_fade_ticks != 0)
                {
                    channel->pan_bias_fade_ticks--;
                    next = channel->pan_bias + channel->pan_bias_step;
                    if ((next & 0xFF00) != (channel->pan_bias & 0xFF00))
                    {
                        channel->update_flags |= 3;
                    }
                    channel->pan_bias = next;
                }
                if (channel->unk88 != 0)
                {
                    channel->unk88--;
                    next = channel->noise_mask + channel->pitch_mod_mask;
                    if ((next & 0xFF00) != (channel->noise_mask & 0xFF00))
                    {
                        channel->update_flags |= 0x10;
                    }
                    channel->noise_mask = next;
                }
                active ^= n;
            }
            n <<= 1;
            channel++;
        } while (active != 0);
    }
}

/**
 * @brief Advance the tempo accumulator and, on each musical tick, step every
 *        active channel's opcode/timer state.
 * @param channels Base address of the channel array to iterate (primary
 *        g_akao_seq_channels or the pending/secondary set).
 * @param is_secondary 0 for the primary sequence pass (updates pending-tick
 *        and driver-dirty state), non-zero for the secondary channel1 pass.
 * @return The active-channel bitmask (g_akao_seq_channel0->w04.song.active_mask).
 * @see decomp.me (100%) https://decomp.me/scratch/XMCUh
 */
s32 akao_seq_tick_channels(AkaoChannelState* channels, s32 is_secondary)
{
    u32 tempo_step;
    u32 scalar;
    s32 channel_mask;
    AkaoChannelState* channel;
    s32 remaining;
    AkaoDriverFlags* driver_flags;

    tempo_step = g_akao_seq_channel0->tempo >> 16;
    scalar = g_akao_master_vol_scalar;

    if (scalar != 0)
    {
        if (scalar < 0x80)
        {
            tempo_step += (tempo_step * scalar) >> 7;
        }
        else
        {
            tempo_step = (tempo_step * scalar) >> 8;
        }
    }

    g_akao_seq_channel0->tempo_acc += tempo_step;

    if ((g_akao_seq_channel0->tempo_acc & 0xFFFF0000) || (g_akao_driver_mode_flags & 4))
    {
        g_akao_seq_channel0->tempo_acc &= 0xFFFF;

        driver_flags = &g_akao_driver_flags;
        channel = channels;

        do
        {
            channel_mask = 1;
            remaining = g_akao_seq_channel0->w04.song.active_mask;

            do
            {
                if (remaining & channel_mask)
                {
                    channel->unk66--;
                    channel->unk68--;

                    if (channel->unk66 == 0)
                    {
                        akao_seq_step_opcode(channel, channel_mask);
                    }
                    else if (channel->unk68 == 0)
                    {
                        g_akao_seq_channel0->key_off_mask |= channel_mask;
                    }

                    akao_tick_channel_effects(channel, channel_mask, 0);
                    remaining &= ~channel_mask;
                }

                channel++;
                channel_mask <<= 1;
            } while (remaining != 0);

            if (g_akao_seq_channel0->tempo_fade_ticks != 0)
            {
                g_akao_seq_channel0->tempo_fade_ticks--;
                g_akao_seq_channel0->tempo += g_akao_seq_channel0->tempo_step;
            }

            if (g_akao_seq_channel0->master_vol_fade_ticks != 0)
            {
                g_akao_seq_channel0->master_vol_fade_ticks--;
                g_akao_seq_channel0->unk48 += g_akao_seq_channel0->unk4C;
                {
                    u32 flags;

                    /* The do/while (0) only raises the allocation weight of
                     * driver_flags so it gets $s4 ahead of is_secondary;
                     * no natural form found does the same. */
                    do
                    {
                        flags = driver_flags->unk8;
                    } while (0);
                    if (is_secondary == 0)
                    {
                        driver_flags->unk8 = flags | 0x80;
                    }
                }
            }

            if (g_akao_seq_channel0->unk68 != 0)
            {
                g_akao_seq_channel0->unk6A++;
                if (g_akao_seq_channel0->unk6A == g_akao_seq_channel0->unk68)
                {
                    g_akao_seq_channel0->unk6A = 0;
                    g_akao_seq_channel0->unk66++;
                    /* Song role: unk66 is the beat counter and
                     * is_sfx_channel is beats-per-measure. */
                    if (g_akao_seq_channel0->unk66 == g_akao_seq_channel0->is_sfx_channel)
                    {
                        g_akao_seq_channel0->unk66 = 0;
                        g_akao_seq_channel0->measure++;
                        if ((is_secondary == 0) && (g_akao_seq_pending_ticks != 0))
                        {
                            g_akao_seq_pending_ticks--;
                        }
                    }
                }
            }

            if (is_secondary != 0)
            {
                break;
            }
            channel = channels;
        } while (g_akao_seq_pending_ticks != 0);
    }

    return g_akao_seq_channel0->w04.song.active_mask;
}

/**
 * @brief AKAO driver per-frame IRQ callback (RCNT2 event handler).
 *
 * Advances the sequencer/SFX state once per frame: services pending key-offs
 * and channel1->channel0 handoff, flushes voice register updates when any
 * channel has a note-on pending, ticks the primary and secondary song
 * channels, steps the SFX channel bitmask (note/gate countdown and opcode
 * dispatch), ticks fade envelopes every 4th frame, and updates the
 * @c D_8003D160 timing ring used for profiling.
 *
 * @note @c D_8004D40C, @c D_8004F758, and @c D_8004D408 are declared as
 *       single-element arrays (not scalars) to match the source shape used
 *       for this scratch. @c D_8004D408's extern declaration must be
 *       grouped with @c D_8004D40C / @c D_8004F758 in this file rather than
 *       living in akao_sequencer.h with the other cross-function externs: under
 *       gcc280_g4, its position relative to those two changes where the
 *       target hoists its %hi computation (a delay-slot/CSE scheduling
 *       effect, not a value difference) and was the last blocker to 100%.
 * @see decomp.me (100%) https://decomp.me/scratch/ICO2k
 */
void akao_irq_handler(void)
{
    s32 ticks;
    s32 active;
    s32 prev_sample;
    s32 total;
    s32 sample;
    AkaoChannelState* channel;
    u32 bit;

    ticks = GetRCnt(RCNT_SPEC_2);

    g_akao_irq_frame_counter += 1;
    if ((g_akao_seq_channel0->key_off_mask != 0) || (D_8004D40C[0] != 0) || ((g_akao_seq_channel1 != 0) && (g_akao_seq_channel1->key_off_mask != 0)))
    {
        akao_flush_voice_key_offs();
    }
    else if (g_akao_seq_channel1 == 0)
    {
        /* Jumping straight past the promotion block (instead of letting it
         * re-test g_akao_seq_channel1) is what the original code does; no
         * structured form found reproduces that branch target. */
        goto promote_done;
    }

    {
        AkaoChannelState* pending = g_akao_seq_channel1;

        if (pending != 0)
        {
            if (pending->w04.song.active_mask == 0)
            {
                g_akao_seq_channel1 = 0;
            }
            else if ((g_akao_seq_channel0->w04.song.active_mask | g_akao_seq_channel0->unk1C) == 0)
            {
                akao_copy_bytes((s32*)pending, (s32*)g_akao_seq_channel0, 0x70);
                akao_copy_bytes((s32*)g_akao_pending_channels, (s32*)g_akao_seq_channels, AKAO_SEQ_CHANNEL_COUNT * sizeof(AkaoChannelState));
                {
                    AkaoChannelState* promoted = g_akao_seq_channel1;

                    g_akao_seq_channel1 = 0;
                    promoted->unk5E = 0;
                    promoted->w04.song.active_mask = 0;
                }
            }
        }
    }
promote_done:

    if (((D_8004F758[0] | g_akao_seq_channel0->note_on_mask | D_8004D408[0]) != 0) || ((g_akao_seq_channel1 != 0) && (g_akao_seq_channel1->note_on_mask != 0)))
    {
        akao_flush_voice_updates(D_8004D408[0]);
    }

    if (g_akao_seq_channel0->w04.song.active_mask != 0)
    {
        akao_seq_tick_channels(AKAO_SEQ_CHANNELS, 0);
    }

    if ((g_akao_seq_channel1 != 0) && (g_akao_seq_channel1->w04.song.active_mask != 0))
    {
        g_akao_seq_channel0 = g_akao_seq_channel1;
        akao_seq_tick_channels((AkaoChannelState*)g_akao_pending_channels, 1);
        g_akao_seq_channel0 = &g_akao_seq_master_state;
    }

    if (g_akao_sfx_control.unk0 != 0)
    {
        active = g_akao_sfx_control.unk0;
        {
            u32 acc = g_akao_sfx_control.unk18 + g_akao_sfx_control.unk16;

            g_akao_sfx_control.unk18 = acc;
            if (((acc & 0xFFFF0000) != 0) || (g_akao_driver_mode_flags & 4))
            {
                g_akao_sfx_control.unk18 = acc & 0xFFFF;

                bit = 0x1000;
                channel = (AkaoChannelState*)g_sfx_channels;

                do
                {
                    if (active & bit)
                    {
                        if (!(g_akao_driver_mode_flags & 2) || (channel->tempo_acc & 0x02000000))
                        {
                            AKAO_CHANNEL_SFX_TICKS(channel)++;

                            channel->unk66--;
                            channel->unk68--;

                            if (channel->unk66 == 0)
                            {
                                akao_seq_step_opcode(channel, bit);
                            }
                            else if (channel->unk68 == 0)
                            {
                                g_akao_sfx_control.unkC |= bit;
                                g_akao_sfx_control.unk8 &= ~bit;
                            }
                            akao_tick_channel_effects(channel, bit, 1);
                        }
                        active ^= bit;
                    }
                    channel++;
                    bit <<= 1;
                } while (active != 0);
            }
        }
    }

    if (!(g_akao_irq_frame_counter & 3))
    {
        akao_tick_fades();
    }

    /* The timing-ring update below keeps decompilation scaffolding: the
     * nested do/while (0) blocks, the (x & m) | (x & ~m) identity (a plain
     * copy of ticks) and the prev_sample assignment all steer register
     * allocation and are required for the match. */
    ticks = (prev_sample = GetRCnt(RCNT_SPEC_2)) - ticks;
    if (ticks <= 0)
    {
        ticks += AKAO_TICK_TIMER_PERIOD;
    }

    {
        s32 d4 = D_8003D160.unk4;
        s32 d8 = D_8003D160.unk8;
        do
        {
            prev_sample = D_8003D160.unkC;
            do
            {
                do
                {
                    sample = (ticks & prev_sample) | (ticks & ~prev_sample);
                    D_8003D160.unkC = sample;
                } while (0);
            } while (0);
            D_8003D160.unk0 = d4;
            total = d4 + d8 + prev_sample;
        } while (0);
        D_8003D160.unk4 = d8;
        D_8003D160.unk8 = prev_sample;
        D_8003EC18 = total + sample;
    }
}

/**
 * @brief Scan ahead from the channel cursor to classify the next note opcode.
 * @param channel Channel whose bytecode is scanned; its cursor and loop
 *        state are not modified, only the tie flags in @c note_flags.
 * @return The next note opcode (< 0x9A), 0xA0 for a rest/end, or 0x83, 0x84
 *         or 0x8F for the 0xF0..0xFD length-prefixed note forms.
 * @note The extended opcodes FE 0A/0B/0C share their bodies with the primary
 *       opcodes 0xC9/0xCB/0xCA; the two gotos reproduce that shared code.
 * @see decomp.me (100%) https://decomp.me/scratch/52mKD
 */
u8 akao_seq_skip_to_next_note(AkaoChannelState* channel)
{
    u8* cursor = channel->seq_cursor;
    u32 depth = channel->loop_depth;
    u8 op;
    u8 length;
    s32 offset;

    while (1)
    {
        op = *cursor;
        if (op < 0x9A)
        {
            if (op >= 0x8F)
            {
                channel->note_flags &= 0xFFFA;
            }
            return *cursor;
        }
        if (op < 0xA0)
        {
            return 0xA0;
        }
        length = g_akao_opcode_len_table[*cursor - 0xA0];
        if (length != 0)
        {
            cursor += length;
            continue;
        }
        switch (*cursor)
        {
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
            return 0x83;
        case 0xFC:
            return 0x84;
        case 0xFD:
            return 0x8F;
        case 0xFE:
            cursor++;
            op = *cursor;
            length = g_akao_opcode_len_table_ext[op];
            if (length != 0)
            {
                cursor += length;
                continue;
            }
            switch (op - 6)
            {
            case 0:
                cursor++;
                if (*cursor == channel->loop_count[depth] + 1)
                {
                    cursor++;
                    depth--;
                    depth &= 3;
                    offset = cursor[0];
                    offset += cursor[1] << 8;
                    cursor += (s16)offset;
                }
                else
                {
                    cursor += 3;
                }
                continue;
            case 1:
                cursor++;
                offset = cursor[0];
                offset += cursor[1] << 8;
                cursor += (s16)offset;
                continue;
            case 2:
                cursor++;
                if (g_akao_seq_channel0->unk60 >= *cursor++)
                {
                    offset = cursor[0];
                    offset += cursor[1] << 8;
                    cursor += (s16)offset;
                }
                else
                {
                    cursor += 2;
                }
                continue;
            case 3:
                cursor = (u8*)channel->note_on_mask;
                continue;
            case 4:
                goto loop_end;
            case 5:
                channel->note_flags &= 0xFFFA;
                cursor++;
                continue;
            case 6:
                goto loop_break;
            case 7:
            case 8:
            case 9:
                break;
            default:
                continue;
            }
            break;
        case 0xC9:
        loop_end:
            cursor++;
            if (*cursor == channel->loop_count[depth] + 1)
            {
                cursor++;
                depth--;
                depth &= 3;
            }
            else
            {
                cursor = channel->w04.loop_cursor[depth];
            }
            continue;
        case 0xCB:
        case 0xCD:
        case 0xD1:
        case 0xDB:
            channel->note_flags &= 0xFFFA;
            cursor++;
            continue;
        case 0xCA:
        loop_break:
            if (!(channel->flags & AKAO_CH_STOP_PENDING))
            {
                cursor = channel->w04.loop_cursor[depth];
                continue;
            }
            break;
        }
        channel->note_flags &= 0xFFFA;
        return 0xA0;
    }
}

/**
 * @brief Select the articulation entry whose key range contains @p key and
 *        load its SPU envelope / pitch fields into the channel.
 * @param channel Channel to bind; its key-to-articulation map is @c key_off_mask
 *        (channel role).
 * @param key Note/key being bound; chooses the entry within the channel's
 *        articulation map.
 * @param next_opcode Next-note opcode from akao_seq_skip_to_next_note; passed
 *        by the caller but not used.
 * @see decomp.me (100%) https://decomp.me/scratch/0tdrk
 */
void akao_bind_articulation_for_key(AkaoChannelState* channel, u32 key, s32 next_opcode)
{
    AkaoKeyMapEntry* entry;
    AkaoArticulation* art;
    u8 articulation;
    u32 flags;

    if (((s16)channel->note_key < key) || ((s16)channel->note_key == 0xFF))
    {
        entry = (AkaoKeyMapEntry*)channel->key_off_mask;
        while ((entry[1].sustain_mode != 0) && (entry->key_high < key))
        {
            entry++;
        }
    }
    else if (key < (s16)channel->note_key)
    {
        entry = (AkaoKeyMapEntry*)channel->key_off_mask;
        while (entry[1].sustain_mode != 0)
        {
            if (key < entry[1].key_low)
            {
                break;
            }
            entry++;
        }
    }
    else
    {
        return;
    }

    flags = channel->flags;
    articulation = entry->articulation;
    art = &AKAO_ARTICULATIONS[articulation];
    channel->unk6A = articulation;

    channel->spu_sample_addr = art->sample_addr;
    channel->spu_loop_addr = art->loop_addr;

    if (!(flags & AKAO_CH_ADSR_ATTACK))
    {
        channel->spu_adsr_low = entry->attack_rate << 8;
    }
    else
    {
        channel->spu_adsr_low &= 0x7F00;
    }

    channel->spu_adsr_low |= art->pitch_misc.half.lo & 0x80FF;

    if (!(flags & AKAO_CH_ADSR_SUSTAIN_RATE))
    {
        channel->spu_adsr_high &= 0x201F;
        channel->spu_adsr_high |= entry->sustain_rate << 6;
    }
    else
    {
        channel->spu_adsr_high &= 0x3FDF;
    }

    switch (entry->sustain_mode)
    {
    case 3:
        channel->spu_adsr_high |= 0x4000;
        break;
    case 5:
        channel->spu_adsr_high |= 0x8000;
        break;
    case 7:
        channel->spu_adsr_high |= 0xC000;
        break;
    }

    if (!(flags & AKAO_CH_ADSR_RELEASE_RATE))
    {
        channel->spu_adsr_high &= 0xFFE0;
        channel->spu_adsr_high |= entry->release_rate;
    }

    channel->spu_adsr_high |= art->pitch_misc.half.hi & 0x20;
    channel->spu_volume_scale = entry->volume_scale;
}

/**
 * @brief Compute the SPU pitch (and a scaled volume) for a note relative to
 *        an articulation's base note, octave-shifting both as needed.
 * @param art Articulation providing the base note / fine-tune (adsr halves).
 * @param note Target note (semitones) to sound.
 * @param volume Note volume; low 8 bits scale the output volume.
 * @param out_volume Receives the computed volume, octave-shifted to match.
 * @return 16-bit SPU pitch value.
 * @see decomp.me (100%) https://decomp.me/scratch/mWTad
 */
s32 akao_compute_pitch(AkaoArticulation* art, s32 note, s32 volume, s32* out_volume)
{
    s32 index;
    s32 octaves;
    s32 base_pitch;
    s32 semitone;
    unsigned long remainder;
    u32 scale;
    u32 shift;
    u32 pitch;
    u32 scaled_volume;
    s32 result;
    semitone = note - art->adsr.half.hi;
    if (semitone < 0)
    {
        do
        {
            semitone += 12;
        } while (semitone < 0);
    }
    /* The remainder and octave counts pass through extra variables; folding
     * them changes the register allocation. */
    remainder = semitone % 12;
    index = remainder;
    if (art->adsr.half.lo == 0)
    {
        s32 table_pitch = g_akao_pitch_table[index];

        pitch = table_pitch << 8;
    }
    else if (art->adsr.half.lo < 0)
    {
        pitch = (g_akao_pitch_table[index] * (u16)art->adsr.half.lo) >> 8;
    }
    else
    {
        base_pitch = g_akao_pitch_table[index];
        pitch = (u32)(base_pitch * art->adsr.half.lo) >> 7;
        pitch += base_pitch << 8;
    }
    scale = volume & 0xFF;
    if (scale != 0)
    {
        if (scale < 0x80)
        {
            scaled_volume = (pitch * scale) >> 7;
        }
        else
        {
            scaled_volume = ((pitch * scale) >> 8) - pitch;
        }
        *out_volume = scaled_volume;
    }
    if (note < art->adsr.half.hi)
    {
        do
        {
            *out_volume >>= 1;
            pitch = (s32)pitch >> 1;
            note += 12;
        } while (note < art->adsr.half.hi);
    }
    else
    {
        octaves = (note - art->adsr.half.hi) / 12;
        index = octaves;
        semitone = octaves;
        if (semitone != 0)
        {
            shift = index;
            pitch <<= shift;
            *out_volume <<= semitone;
        }
    }
    pitch = (s32)pitch >> 8;
    result = pitch & 0xFFFF;
    *out_volume >>= 8;
    return result;
}

/**
 * @brief Initialize an AKAO channel voice slot: bind articulation data,
 *        configure SPU envelope / pitch fields, and fire off the pitch
 *        calculation for the note.
 * @param channel Channel to start the note on.
 * @param channel_mask Channel bit-mask used to update the active-channel bitmask.
 * @param slot_idx Slot index into the small-slot table (base pointer from
 *             @c g_akao_seq_channel0->flags, the song-role note table).
 * @return Pitch result from @c akao_compute_pitch.
 * @see decomp.me (100%) https://decomp.me/scratch/9dRLX
 */
s32 akao_channel_start_note(AkaoChannelState* channel, s32 channel_mask, s32 slot_idx)
{
    AkaoNoteArticulationSlot* slot;
    AkaoArticulation* art;
    u32 flags;
    u32 sounding;
    u32 articulation;
    s32 pitch;
    u32 key_on_mask;

    slot = (AkaoNoteArticulationSlot*)g_akao_seq_channel0->flags;
    key_on_mask = g_akao_seq_channel0->w04.song.key_on_mask;
    sounding = g_akao_seq_channel0->note_on_mask;
    slot += slot_idx;
    key_on_mask |= channel_mask;
    sounding &= channel_mask;
    g_akao_seq_channel0->w04.song.key_on_mask = key_on_mask;
    if (sounding)
    {
        g_akao_seq_channel0->key_off_mask |= channel_mask;
    }
    articulation = slot->articulation;
    flags = channel->flags;
    channel->unk6A = articulation;
    art = &AKAO_ARTICULATIONS[articulation];
    channel->spu_sample_addr = art->sample_addr;
    channel->spu_loop_addr = art->loop_addr;
    if (!(flags & AKAO_CH_ADSR_ATTACK))
    {
        channel->spu_adsr_low = slot->attack_rate << 8;
    }
    else
    {
        channel->spu_adsr_low &= 0x7F00;
    }
    channel->spu_adsr_low |= art->pitch_misc.half.lo & 0x80FF;
    if (!(flags & AKAO_CH_ADSR_SUSTAIN_RATE))
    {
        channel->spu_adsr_high &= 0x201F;
        channel->spu_adsr_high |= slot->sustain_rate << 6;
    }
    else
    {
        channel->spu_adsr_high &= 0x3FDF;
    }
    switch (slot->sustain_mode)
    {
    case 3:
        channel->spu_adsr_high |= 0x4000;
        break;
    case 5:
        channel->spu_adsr_high |= 0x8000;
        break;
    case 7:
        channel->spu_adsr_high |= 0xC000;
        break;
    }
    if (!(flags & AKAO_CH_ADSR_RELEASE_RATE))
    {
        channel->spu_adsr_high &= 0xFFE0;
        channel->spu_adsr_high |= slot->release_rate;
    }
    channel->spu_adsr_high |= art->pitch_misc.half.hi & 0x20;
    pitch = akao_compute_pitch(art, slot->key, channel->detune, &channel->detune_pitch_delta);
    channel->spu_volume_scale = slot->volume_scale;
    channel->pan = ((slot->pan_and_noise & 0x7F) + 0x40) << 8;
    if (slot->pan_and_noise & 0x80)
    {
        g_akao_seq_channel0->noise_mask |= channel_mask;
    }
    else
    {
        g_akao_seq_channel0->noise_mask &= ~channel_mask;
    }
    g_akao_driver_flags.unk8 |= 0x100;
    return pitch;
}

/**
 * @brief Execute opcodes for one channel until a note or rest is reached,
 *        then start that note (pitch, envelopes, LFOs, portamento).
 * @param channel Channel to step.
 * @param channel_mask Bit of @p channel in the song or SFX channel masks.
 * @note @c value holds the extended opcode byte, then the next-note opcode,
 *       then the computed pitch; the shared variable is what gives the
 *       original register allocation.
 * @see decomp.me (100%) https://decomp.me/scratch/P4H6n
 */
void akao_seq_step_opcode(AkaoChannelState* channel, s32 channel_mask)
{
    u32 offset;
    s16 slide_key;
    s32 target_key;
    s32 flags;
    u16 expression_ticks;
    s32 key;
    u16 lfo_depth;
    u16 note_flags;
    u16 duration_adjust;
    u16 portamento;
    u16 duration;
    s32 value;
    u32 absolute_depth;
    u32 depth;
    u32 opcode;

    do
    {
        opcode = *channel->seq_cursor++;

        if (opcode >= 0xA0)
        {
            if (opcode == 0xFE)
            {
                value = *channel->seq_cursor++;
                g_akao_opcode_handlers_ext[value](channel, channel_mask);
            }
            else if ((opcode >= 0xF0) && (opcode < 0xFE))
            {
                opcode = (opcode - 0xF0) * 11;
                channel->unk66 = *channel->seq_cursor++;
            }
            else
            {
                if (opcode == 0xFF)
                {
                    opcode = 0xA0;
                }
                else if ((opcode == 0xCA) && (channel->flags & AKAO_CH_STOP_PENDING))
                {
                    opcode = 0xA0;
                    g_akao_sfx_control.unkC |= channel_mask;
                }
                g_akao_opcode_handlers[opcode - 0xA0](channel, channel_mask);
            }
        }
        channel->opcode_count++;
    } while (opcode > 0xA0);

    if (opcode == 0xA0)
    {
        if (channel->is_sfx_channel == 0)
        {
            g_akao_seq_channel0->key_off_mask |= channel_mask;
        }
    }
    else
    {
        value = akao_seq_skip_to_next_note(channel) & 0xFF;
        duration_adjust = channel->note_duration_adjust;
        if ((s16)channel->note_duration_adjust != 0)
        {
            channel->unk68 = duration_adjust;
            channel->unk66 = duration_adjust;
        }
        if (channel->unk66 != 0)
        {
            if ((value >= 0x8FU) || ((value < 0x84U) && !(channel->note_flags & 5)))
            {
                channel->unk68 -= 2;
            }
        }
        else
        {
            duration = channel->unk66 = g_akao_note_duration_table[opcode % 11];
            if (((value < 0x84) || (value >= 0x8F)) && !(channel->note_flags & 5))
            {
                duration -= 2;
            }
            channel->unk68 = duration;
        }
        if ((channel->is_sfx_channel == 0) && (channel->flags & AKAO_CH_FULL_GATE))
        {
            channel->unk68 = channel->unk66;
        }
        channel->note_duration = channel->unk66;
        channel->update_flags |= 0x4000;
        if (opcode >= 0x8F)
        {
            if (channel->is_sfx_channel == 0)
            {
                g_akao_seq_channel0->note_on_mask &= ~channel_mask;
                if (channel->voice < 0x18)
                {
                    g_akao_seq_channel0->key_off_mask |= channel_mask;
                }
            }
            channel->portamento_speed = 0;
            channel->pitch_lfo_value = 0;
            channel->volume_lfo_value = 0;
            channel->note_flags &= 0xFFFD;
            return;
        }
        if (opcode < 0x84)
        {
            flags = channel->flags;
            key = opcode / 11;
            key += channel->octave * 12;
            if (flags & AKAO_CH_DRUM_MODE)
            {
                value = akao_channel_start_note(channel, channel_mask, key);
            }
            else
            {
                if (!(channel->note_flags & 2))
                {
                    if (channel->is_sfx_channel == 0)
                    {
                        if (flags & AKAO_CH_KEY_MAP)
                        {
                            akao_bind_articulation_for_key(channel, key, value);
                        }
                        g_akao_seq_channel0->w04.song.key_on_mask |= channel_mask;
                        if ((g_akao_seq_channel0->note_on_mask & channel_mask) && (channel->voice < 0x18))
                        {
                            g_akao_seq_channel0->key_off_mask |= channel_mask;
                        }
                        expression_ticks = channel->note_expression_ticks;
                        if (expression_ticks != 0)
                        {
                            channel->expression_fade_ticks = expression_ticks;
                            channel->unk48 = AKAO_CHANNEL_EXPRESSION_PRESET(channel);
                            channel->unk4C = AKAO_CHANNEL_EXPRESSION_PRESET_STEP(channel);
                        }
                    }
                    else
                    {
                        g_akao_sfx_control.unk4 |= channel_mask;
                    }
                    channel->pitch_slide_ticks = 0;
                }
                portamento = channel->portamento_speed;
                if ((portamento != 0) && (channel->prev_key != 0))
                {
                    target_key = channel->transpose + key;
                    key = channel->prev_key + channel->prev_transpose;
                    channel->pitch_slide_duration = portamento;
                    channel->pitch_slide_delta = target_key - channel->prev_key - channel->prev_transpose;
                    channel->note_key = channel->prev_key - (channel->transpose - channel->prev_transpose);
                }
                else
                {
                    channel->note_key = key;
                    key += (s16)channel->transpose;
                }
                value = akao_compute_pitch(&AKAO_ARTICULATIONS[channel->unk6A], key, channel->detune, &channel->detune_pitch_delta);
                if (channel->pitch_scale != 0)
                {
                    offset = (u32)(value * channel->pitch_scale) >> 8;
                    offset *= D_8003D27C[g_akao_cdvol_tick];
                    if (D_8003D27C[g_akao_cdvol_tick] & 0x80)
                    {
                        offset >>= 9;
                        value -= offset;
                    }
                    else
                    {
                        offset >>= 7;
                        value += offset;
                    }
                }
            }
            channel->pitch = value;
            if (channel->is_sfx_channel == 0)
            {
                g_akao_seq_channel0->note_on_mask |= channel_mask;
            }
            else
            {
                g_akao_sfx_control.unk8 |= channel_mask;
            }
            channel->update_flags |= 0x13;
            opcode = channel->flags;
            if (opcode & AKAO_CH_PITCH_LFO)
            {
                lfo_depth = channel->pitch_lfo_depth;
                depth = lfo_depth & 0x7F00;
                absolute_depth = lfo_depth & 0x8000;
                depth >>= 8;
                {
                    u16 lfo_scaled;
                    if (!absolute_depth)
                    {
                        lfo_scaled = (depth * ((u32)(value * 0xF) >> 8)) >> 7;
                    }
                    else
                    {
                        lfo_scaled = (depth * value) >> 7;
                    }
                    channel->pitch_lfo_depth_scaled = lfo_scaled;
                }
                if (!(channel->note_flags & 2))
                {
                    channel->unk1C = g_akao_lfo_waveforms[channel->pitch_lfo_waveform];
                    channel->pitch_lfo_delay_ticks = channel->pitch_lfo_delay;
                    channel->pitch_lfo_restart = 1;
                }
            }
            if ((opcode & AKAO_CH_VOLUME_LFO) && !(channel->note_flags & 2))
            {
                /* Channel role: 0x20 is the volume-LFO waveform cursor. */
                channel->tempo = g_akao_lfo_waveforms[channel->volume_lfo_waveform];
                channel->volume_lfo_delay_ticks = channel->volume_lfo_delay;
                channel->volume_lfo_restart = 1;
            }
            channel->pitch_lfo_value = 0;
            channel->volume_lfo_value = 0;
            channel->unk30 = 0;
        }
        note_flags = channel->note_flags;
        channel->note_flags = (note_flags & 0xFFFD) | ((note_flags & 1) * 2);
        if ((s16)channel->pitch_slide_delta != 0)
        {
            slide_key = channel->note_key + channel->pitch_slide_delta;
            channel->note_key = slide_key;
            value = akao_compute_pitch(&AKAO_ARTICULATIONS[channel->unk6A], slide_key + (s16)channel->transpose, channel->detune, (s32*)&offset) << 0x10;
            channel->pitch_slide_ticks = channel->pitch_slide_duration;
            channel->pitch_slide_delta = 0;
            channel->pitch_slide_step = (value - ((channel->pitch << 16) + channel->unk30)) / channel->pitch_slide_ticks;
        }
        channel->prev_key = channel->note_key;
        channel->prev_transpose = channel->transpose;
    }
}

/**
 * @brief Copy an articulation's SPU addresses and ADSR words into a channel.
 * @param channel Channel to update.
 * @param art Articulation entry to load.
 * @param sample_addr SPU sample start address to install.
 * @see decomp.me (100%) https://decomp.me/scratch/Fr8N0
 */
void akao_channel_load_articulation_fields(AkaoChannelState* channel, AkaoArticulation* art, s32 sample_addr)
{
    u16 adsr_low;
    u16 adsr_high;
    s32 loop_addr;
    s32 update_flags;

    channel->spu_sample_addr = sample_addr;
    loop_addr = art->loop_addr;
    channel->spu_loop_addr = loop_addr;

    adsr_low = art->pitch_misc.half.lo;
    channel->spu_adsr_low = adsr_low;

    update_flags = channel->update_flags;
    adsr_high = art->pitch_misc.half.hi;

    channel->update_flags = update_flags | 0x1FF80;
    channel->spu_adsr_high = adsr_high;
}

/**
 * @brief Select an articulation by index and load it into the channel.
 * @param channel Channel to update.
 * @param articulation Index into the driver articulation table.
 * @see decomp.me (100%) https://decomp.me/scratch/1RRHh
 */
void akao_channel_set_articulation(AkaoChannelState* channel, s32 articulation)
{
    AkaoArticulation* art;

    channel->unk6A = articulation;
    art = &AKAO_ARTICULATIONS[articulation];
    akao_channel_load_articulation_fields(channel, art, art->sample_addr);
}

/**
 * @brief Clear the given channel bits across all g_akao_sfx_control bitmasks
 *        and zero two fields of the channel object.
 * @param channel SFX channel whose secondary flag word and reverb word are cleared.
 * @param release_mask Bit-mask of channels to release.
 * @see decomp.me (100%) https://decomp.me/scratch/67vx9
 */
void akao_sfx_release_channels(AkaoChannelState* channel, u32 release_mask)
{
    u32 mask = ~release_mask;

    g_akao_sfx_control.unk0 &= mask;
    g_akao_sfx_control.unk10 &= mask;
    g_akao_sfx_control.reverb_mask &= mask;
    g_akao_sfx_control.noise_mask &= mask;
    g_akao_sfx_control.pitch_mod_mask &= mask;
    g_akao_sfx_control.unk4 &= mask;
    g_akao_sfx_control.unk8 &= mask;

    channel->tempo_acc = 0;
    channel->reverb_mask = 0;
}

/**
 * @brief Remap an SFX articulation index into the selected 16-entry bank.
 * @param bank SFX articulation bank index (0 leaves the index unchanged).
 * @param articulation Articulation index from the sequence.
 * @return The remapped articulation index.
 * @see decomp.me (100%) https://decomp.me/scratch/oTcsG
 */
s32 akao_remap_sfx_articulation(s32 bank, s32 articulation)
{
    if (bank != 0)
    {
        if ((articulation >= 0x80) && (articulation < 0xB0))
        {
            return articulation + (bank * 16);
        }
        if ((articulation >= 0xB0) && (articulation < 0xE0))
        {
            return articulation + ((bank - 3) * 16);
        }
    }
    return articulation;
}

/**
 * @brief Release sequencer or SFX channels depending on mode.
 *        When channel->is_sfx_channel is zero, clears release_mask bits from the
 *        seq-channel bitmasks in g_akao_seq_channel0.  If all active bits are
 *        cleared, also zeros g_akao_seq_pending_ticks, unk5E, and seq_cursor.  When
 *        channel->is_sfx_channel is non-zero, delegates to akao_sfx_release_channels.
 *        In both paths, channel->flags is cleared and the driver dirty flag
 *        (unk8) is OR'd with 0x110.
 * @param channel Pointer to the shared channel header.
 * @param release_mask Bit-mask of channels to release.
 * @see decomp.me (100%) https://decomp.me/scratch/vxrwL
 */
void akao_release_channels(AkaoChannelState* channel, u32 release_mask)
{
    s32 active;

    if (channel->is_sfx_channel == 0)
    {
        u32 keep = ~release_mask;

        active = g_akao_seq_channel0->w04.song.active_mask & keep;
        g_akao_seq_channel0->w04.song.active_mask = active;

        if (active == 0)
        {
            g_akao_seq_pending_ticks = 0;
            g_akao_seq_channel0->unk5E = 0;
            g_akao_seq_channel0->seq_cursor = 0;
        }

        g_akao_seq_channel0->note_on_mask &= keep;
        g_akao_seq_channel0->w04.song.voice_alloc_low_mask &= keep;
        g_akao_seq_channel0->w04.song.static_voice_mask &= keep;
        g_akao_seq_channel0->reverb_mask &= keep;
        g_akao_seq_channel0->noise_mask &= keep;
        g_akao_seq_channel0->pitch_mod_mask &= keep;
    }
    else
    {
        akao_sfx_release_channels(channel, release_mask);
    }

    channel->flags = 0;
    g_akao_driver_flags.unk8 |= 0x110;
}

/**
 * @brief Opcode handler: set the master tempo directly.
 *
 * Reads a 16-bit value from the sequence stream into the high half of
 * @c g_akao_seq_channel0->tempo (the per-tick rate added to the tempo_acc
 * accumulator in akao_seq_tick_channels) and clears the tempo-slide
 * countdown @c tempo_fade_ticks.
 *
 * @param channel Channel whose bytecode cursor is advanced by 2.
 * @see decomp.me (100%) https://decomp.me/scratch/GHtCl
 */
void akao_seq_op_set_tempo(AkaoChannelState* channel)
{
    AkaoChannelState* song = g_akao_seq_channel0;
    u32 tempo;

    tempo = channel->seq_cursor[0] << 16;
    song->tempo = tempo;
    song->tempo = tempo | (channel->seq_cursor[1] << 24);
    channel->seq_cursor += 2;
    song->tempo_fade_ticks = 0;
}

/**
 * @brief Opcode handler: slide (ramp) the master tempo to a target.
 *
 * Reads a tick count into @c tempo_fade_ticks (defaulting to 0x100 when zero)
 * and a 16-bit target tempo, then computes the per-tick step @c tempo_step =
 * (target - current) / count so akao_seq_tick_channels ramps @c tempo to the
 * target over @c tempo_fade_ticks ticks.
 *
 * @param channel Channel whose bytecode cursor is advanced past the
 *             count byte and the 2 target bytes.
 * @see decomp.me (100%) https://decomp.me/scratch/SFJAU
 */
void akao_seq_op_slide_tempo(AkaoChannelState* channel)
{
    u32 target;
    u32 current;
    s32 step;
    AkaoChannelState* song = g_akao_seq_channel0;
    u8* cursor = channel->seq_cursor;
    u32 ticks = *cursor++;

    song->tempo_fade_ticks = ticks;
    channel->seq_cursor = cursor;
    if (ticks == 0)
    {
        song->tempo_fade_ticks = 0x100;
    }
    target = (channel->seq_cursor[0] << 16) | (channel->seq_cursor[1] << 24);
    channel->seq_cursor += 2;
    current = g_akao_seq_channel0->tempo & 0xFFFF0000;
    step = (s32)(target - current) / g_akao_seq_channel0->tempo_fade_ticks;
    g_akao_seq_channel0->tempo = current;
    g_akao_seq_channel0->tempo_step = step;
}

/**
 * @brief Set the sequence-wide stereo master volume from a signed 12-bit operand.
 * @param channel Channel whose bytecode cursor is advanced past the two operand bytes.
 * @see decomp.me (100%) https://decomp.me/scratch/Og38F
 */
void akao_seq_op_set_master_volume(AkaoChannelState* channel)
{
    s32 high;
    s32 low;
    u32 volume;
    u8* cursor = channel->seq_cursor;
    AkaoChannelState* song = g_akao_seq_channel0;

    high = (s8)cursor[1];
    low = cursor[0];
    channel->seq_cursor = cursor + 2;
    song->master_vol_fade_ticks = 0;
    volume = high << 20;
    volume = volume | (low << 12);
    g_akao_driver_flags.unk8 |= 0x80;
    song->unk48 = volume;
}

/**
 * @brief Slide the sequence-wide stereo master volume to a signed 12-bit target.
 * @param channel Channel whose bytecode cursor is advanced past the three operand bytes.
 * @see decomp.me (100%) https://decomp.me/scratch/w18Xw
 */
void akao_seq_op_slide_master_volume(AkaoChannelState* channel)
{
    AkaoChannelState* song;
    s32 high;
    s32 low;
    s32 current;
    s32 target;

    song = g_akao_seq_channel0;
    song->master_vol_fade_ticks = *channel->seq_cursor++;
    if (song->master_vol_fade_ticks == 0)
    {
        song->master_vol_fade_ticks = 0x100;
    }
    high = (s8)channel->seq_cursor[1];
    low = channel->seq_cursor[0];
    channel->seq_cursor += 2;
    target = (high << 20) | (low << 12);
    current = g_akao_seq_channel0->unk48 & ~0xFFF;
    g_akao_seq_channel0->unk48 = current;
    g_akao_seq_channel0->unk4C = (target - current) / g_akao_seq_channel0->master_vol_fade_ticks;
}

/**
 * @brief Opcode handler: unconditional relative jump.
 *
 * Reads a signed 16-bit offset from the stream and adds it to the cursor.
 *
 * @param channel Channel whose bytecode cursor is repositioned by the offset.
 * @see decomp.me (100%) https://decomp.me/scratch/KW15z
 */
void akao_seq_op_jump(AkaoChannelState* channel)
{
    u8* cursor = channel->seq_cursor;

    channel->seq_cursor = cursor + AKAO_READ_S16(cursor);
}

/**
 * @brief Opcode handler: conditional relative jump.
 *
 * Reads a comparison byte from the stream; if the channel counter
 * @c g_akao_seq_channel0->unk60 is >= that byte, applies a signed 16-bit
 * relative jump, otherwise falls through past the 2 offset bytes.
 *
 * @param channel Channel whose bytecode cursor is repositioned accordingly.
 * @see decomp.me (100%) https://decomp.me/scratch/l153y
 */
void akao_seq_op_cond_jump(AkaoChannelState* channel)
{
    u8* cursor = channel->seq_cursor;
    AkaoChannelState* song = g_akao_seq_channel0;
    s32 threshold = *cursor;

    cursor++;
    channel->seq_cursor = cursor;
    if (song->unk60 >= threshold)
    {
        channel->seq_cursor = cursor + AKAO_READ_S16(cursor);
    }
    else
    {
        channel->seq_cursor = cursor + 2;
    }
}

/**
 * @brief Opcode handler: call subroutine (jump and save return cursor).
 *
 * Saves the post-operand cursor in the channel's subroutine return slot
 * (@c note_on_mask, channel role) as the
 * return address, then applies a signed 16-bit relative jump to the cursor.
 * Paired with akao_seq_op_return.
 *
 * @param channel Channel whose bytecode cursor is redirected.
 * @see decomp.me (100%) https://decomp.me/scratch/uIP0t
 */
void akao_seq_op_call(AkaoChannelState* channel)
{
    u8* cursor = channel->seq_cursor;
    s16 offset = AKAO_READ_S16(cursor);

    channel->note_on_mask = (u32)(cursor + 2);
    channel->seq_cursor += offset;
}

/**
 * @brief Opcode handler: return from subroutine.
 *
 * Restores the bytecode cursor from the saved return address in the
 * subroutine return slot. Paired with akao_seq_op_call.
 *
 * @param channel Channel whose bytecode cursor is redirected.
 * @see decomp.me (100%) https://decomp.me/scratch/os90Z
 */
void akao_seq_op_return(AkaoChannelState* channel)
{
    channel->seq_cursor = (u8*)channel->note_on_mask;
}

/**
 * @brief Set the channel volume directly.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/BN6jO
 */
void akao_seq_op_set_volume(AkaoChannelState* channel)
{
    u8* cursor;
    s16 volume;

    cursor = channel->seq_cursor;
    volume = *cursor << 8;
    channel->seq_cursor = cursor + 1;
    channel->update_flags |= 3;
    channel->volume = volume;
}

/**
 * @brief Slide the channel volume to a target over a specified tick count.
 * @param channel Channel whose bytecode cursor is advanced past the two operand bytes.
 * @see decomp.me (100%) https://decomp.me/scratch/CjVYW
 */
void akao_seq_op_slide_volume(AkaoChannelState* channel)
{
    u16 current;
    u16 value;
    u8* operand;
    u8* cursor;

    cursor = channel->seq_cursor;
    value = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->volume_fade_ticks = value;
    if (value == 0)
    {
        channel->volume_fade_ticks = 0x100;
    }
    operand = channel->seq_cursor;
    current = channel->volume;
    current &= 0x7F00;
    value = ((*channel->seq_cursor++ << 8) - current) / channel->volume_fade_ticks;
    channel->volume_step = value;
    channel->seq_cursor = operand + 1;
    channel->volume = current;
}

/**
 * @brief Set the channel expression multiplier directly.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/hGVxk
 */
void akao_seq_op_set_expression(AkaoChannelState* channel)
{
    u8* cursor;

    cursor = channel->seq_cursor;
    channel->unk48 = (s8)*cursor << 23;
    channel->seq_cursor = cursor + 1;
    channel->expression_fade_ticks = 0;
    channel->update_flags |= 3;
    channel->note_expression_ticks = 0;
}

/**
 * @brief Slide the channel expression multiplier to a target.
 * @param channel Channel whose bytecode cursor is advanced past the two operand bytes.
 * @see decomp.me (100%) https://decomp.me/scratch/jLHGo
 */
void akao_seq_op_slide_expression(AkaoChannelState* channel)
{
    s32 current;
    s32 ticks;
    u8* operand;
    u8* cursor;

    cursor = channel->seq_cursor;
    ticks = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->expression_fade_ticks = ticks;
    if (ticks == 0)
    {
        channel->expression_fade_ticks = 0x100;
    }
    operand = channel->seq_cursor;
    current = channel->unk48 & 0xFFFF0000;
    channel->unk4C = (((s8)*operand++ << 23) - current) / channel->expression_fade_ticks;
    channel->seq_cursor = operand;
    channel->unk48 = current;
    channel->note_expression_ticks = 0;
}

/**
 * @brief Configure the expression ramp that is restarted by the next note.
 * @param channel Channel whose bytecode cursor is advanced past the three operand bytes.
 * @see decomp.me (100%) https://decomp.me/scratch/aOcaK
 */
void akao_seq_op_set_note_expression_envelope(AkaoChannelState* channel)
{
    s32 ticks;
    u8* operand;
    u8* cursor;
    u32 value;

    cursor = channel->seq_cursor;

    value = (s8)*cursor;
    cursor++;

    channel->seq_cursor = cursor;
    AKAO_CHANNEL_EXPRESSION_PRESET(channel) = value << 23;
    ticks = *cursor;
    cursor++;
    channel->seq_cursor = cursor;
    channel->note_expression_ticks = ticks;

    if (ticks == 0)
    {
        channel->note_expression_ticks = 0x100;
    }

    operand = channel->seq_cursor;

    AKAO_CHANNEL_EXPRESSION_PRESET_STEP(channel) = (((s8)*operand << 23) - AKAO_CHANNEL_EXPRESSION_PRESET(channel)) / channel->note_expression_ticks;
    channel->seq_cursor = operand + 1;
}

/**
 * @brief Give sequence notes their full duration instead of an early key-off.
 * @param channel Channel state.
 * @see decomp.me (100%) https://decomp.me/scratch/cNPss
 */
void akao_seq_op_enable_full_gate(AkaoChannelState* channel)
{
    channel->flags |= AKAO_CH_FULL_GATE;
}

/**
 * @brief Restore the normal early-key-off gate duration.
 * @param channel Channel state.
 * @see decomp.me (100%) https://decomp.me/scratch/ryS0d
 */
void akao_seq_op_disable_full_gate(AkaoChannelState* channel)
{
    channel->flags &= ~AKAO_CH_FULL_GATE;
}

/**
 * @brief Set the channel pan directly.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/ZUdZO
 */
void akao_seq_op_set_pan(AkaoChannelState* channel)
{
    u8* cursor;

    cursor = channel->seq_cursor;
    channel->pan = ((*cursor + 0x40) & 0xFF) << 8;
    channel->seq_cursor = cursor + 1;
    channel->pan_fade_ticks = 0;
    channel->update_flags |= 3;
}

/**
 * @brief Slide the channel pan to a target over a tick count.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/MjjmM
 */
void akao_seq_op_slide_pan(AkaoChannelState* channel)
{
    u16 current;
    s32 ticks;
    u8* operand;
    u8* cursor;

    cursor = channel->seq_cursor;
    ticks = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->pan_fade_ticks = ticks;
    if (ticks == 0)
    {
        channel->pan_fade_ticks = 0x100;
    }
    operand = channel->seq_cursor;
    current = channel->pan;
    current &= 0xFF00;
    channel->pan_step = ((((*operand + 0x40) & 0xFF) << 8) - current) / channel->pan_fade_ticks;
    channel->seq_cursor = operand + 1;
    channel->pan = current;
}

/**
 * @brief Set the current octave.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/FRzyk
 */
void akao_seq_op_set_octave(AkaoChannelState* channel)
{
    u8* cursor;

    cursor = channel->seq_cursor;
    channel->octave = *cursor;
    channel->seq_cursor = cursor + 1;
}

/**
 * @brief Raise the current octave by one (wraps at 16).
 * @param channel Channel state.
 * @see decomp.me (100%) https://decomp.me/scratch/2bdR3
 */
void akao_seq_op_increment_octave(AkaoChannelState* channel)
{
    channel->octave = (channel->octave + 1) & 0xF;
}

/**
 * @brief Lower the current octave by one (wraps at 16).
 * @param channel Channel state.
 * @see decomp.me (100%) https://decomp.me/scratch/SJfbQ
 */
void akao_seq_op_decrement_octave(AkaoChannelState* channel)
{
    channel->octave = (channel->octave - 1) & 0xF;
}

/**
 * @brief Select an articulation; SFX channels remap it into their bank.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/1MXGr
 */
void akao_seq_op_set_mapped_articulation(AkaoChannelState* channel)
{
    AkaoArticulation* art;
    s32 index;
    s32 articulation;
    u8* cursor;

    cursor = channel->seq_cursor;
    index = *cursor;
    channel->seq_cursor = cursor + 1;
    if (channel->is_sfx_channel == 0)
    {
        articulation = index;
    }
    else
    {
        articulation = akao_remap_sfx_articulation(channel->voice_alloc_base, index);
    }

    art = &AKAO_ARTICULATIONS[articulation];
    akao_channel_load_articulation_fields(channel, art, art->sample_addr);
    channel->unk6A = articulation;
    channel->spu_volume_scale = 0;
    channel->flags &= ~AKAO_CH_ARTICULATION_MASK;
}

/**
 * @brief Select an articulation without loading its sample start address.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/GAl01
 */
void akao_seq_op_set_articulation(AkaoChannelState* channel)
{
    s32 articulation;
    u8* cursor;

    cursor = channel->seq_cursor;
    articulation = *cursor;
    channel->seq_cursor = cursor + 1;
    akao_channel_load_articulation_fields(channel, &AKAO_ARTICULATIONS[articulation], 0x1010);
    channel->unk6A = articulation;
    channel->spu_volume_scale = 0;
    channel->flags &= ~AKAO_CH_ARTICULATION_MASK;
}

/**
 * @brief Select a key-to-articulation map from the current sequence bank.
 * @param channel Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void akao_seq_op_select_articulation_map(AkaoChannelState* channel)
{
    s32 base;
    u16* entry;
    u8* cursor;
    u8 map_index;

    cursor = channel->seq_cursor;
    map_index = *cursor;
    channel->seq_cursor = cursor + 1;
    base = g_akao_seq_channel0->unk30;
    if (base != 0)
    {
        entry = (u16*)(map_index * 2 + base);
        if (*entry > 0x8000)
        {
            channel->spu_volume_scale = 0;
            channel->flags &= ~AKAO_CH_KEY_MAP;
            return;
        }
        channel->key_off_mask = base + *entry + 0x20;
        channel->note_key = 0xFF;
        channel->flags = (channel->flags & ~AKAO_CH_ARTICULATION_MASK) | AKAO_CH_KEY_MAP;
    }
}

/**
 * @brief Reloads articulation fields for the channel's current articulation.
 * @param channel Channel state to update.
 * @see decomp.me (100%)
 */
void akao_seq_op_refresh_envelope(AkaoChannelState* channel)
{
    AkaoArticulation* articulation;
    u16 adsr_low;
    u16 adsr_high;
    s32 update_flags;
    s32 flags;

    articulation = &AKAO_ARTICULATIONS[channel->unk6A];
    adsr_low = articulation->pitch_misc.half.lo;
    channel->spu_adsr_low = adsr_low;
    adsr_high = articulation->pitch_misc.half.hi;
    update_flags = channel->update_flags;
    flags = channel->flags;
    channel->update_flags = update_flags | 0xFF00;
    channel->flags = flags & ~AKAO_CH_ADSR_OVERRIDE_MASK;
    channel->spu_adsr_high = adsr_high;
}

/**
 * @brief Reads a signed byte from the channel bytecode stream and stores it
 *        as the channel transpose.
 * @param channel Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void akao_seq_op_set_transpose(AkaoChannelState* channel)
{
    u8* cursor;
    s8 transpose;

    cursor = channel->seq_cursor;
    transpose = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->transpose = transpose;
}

/**
 * @brief Reads a signed byte from the channel bytecode stream and adds it to
 *        the channel transpose.
 * @param channel Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void akao_seq_op_add_transpose(AkaoChannelState* channel)
{
    u8* cursor;
    s8 delta;

    cursor = channel->seq_cursor;
    delta = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->transpose += delta;
}

/**
 * @brief Configure a one-shot pitch slide duration and semitone delta.
 * @param channel Channel state whose bytecode cursor is advanced by two bytes.
 * @see decomp.me (100%)
 */
void akao_seq_op_set_pitch_slide(AkaoChannelState* channel)
{
    u8* cursor;
    s32 duration;
    s8 delta;

    cursor = channel->seq_cursor;
    duration = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->pitch_slide_duration = duration;
    if (duration == 0)
    {
        channel->pitch_slide_duration = 0x100;
    }
    delta = *channel->seq_cursor++;
    channel->pitch_slide_delta = delta;
}

/**
 * @brief Enable automatic portamento between successive notes.
 * @param channel Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void akao_seq_op_enable_portamento(AkaoChannelState* channel)
{
    u8* cursor;
    s32 speed;

    cursor = channel->seq_cursor;
    speed = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->portamento_speed = speed;
    if (speed == 0)
    {
        channel->portamento_speed = 0x100;
    }
    channel->prev_transpose = 0;
    channel->prev_key = 0;
    channel->note_flags = 1;
}

/**
 * @brief Disable automatic portamento between notes.
 * @param channel Channel state.
 * @see decomp.me (100%)
 */
void akao_seq_op_disable_portamento(AkaoChannelState* channel)
{
    channel->portamento_speed = 0;
}

/**
 * @brief Set fine pitch detune and recompute its pitch-register delta.
 * @param channel Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void akao_seq_op_set_detune(AkaoChannelState* channel)
{
    s32 pitch;
    u8* next;
    u32 product;
    u32 delta;
    u8* cursor;

    cursor = channel->seq_cursor;
    next = cursor + 1;
    channel->detune = (s8)*cursor;
    delta = (u8)channel->detune;
    pitch = channel->pitch;
    product = pitch * delta;
    channel->seq_cursor = next;
    if (channel->detune < 0)
    {
        delta = (product >> 8) - pitch;
    }
    else
    {
        delta = product >> 7;
    }
    channel->detune_pitch_delta = delta;
    channel->update_flags |= 0x10;
}

/**
 * @brief Add to fine pitch detune and recompute its pitch-register delta.
 * @param channel Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void akao_seq_op_add_detune(AkaoChannelState* channel)
{
    s32 pitch;
    u8* cursor;
    u32 product;
    u32 delta;

    cursor = channel->seq_cursor;
    pitch = channel->pitch;
    channel->detune += (s8)*cursor;
    channel->seq_cursor = cursor + 1;
    delta = (u8)channel->detune;
    product = pitch * delta;
    if (channel->detune < 0)
    {
        delta = (product >> 8) - pitch;
    }
    else
    {
        delta = product >> 7;
    }
    channel->detune_pitch_delta = delta;
    channel->update_flags |= 0x10;
}

/**
 * @brief Start the channel pitch LFO, selecting its delay, period, waveform,
 *        and scaled depth.
 * @param channel Channel state whose bytecode cursor is advanced.
 * @see decomp.me (100%)
 */
void akao_seq_op_start_pitch_lfo(AkaoChannelState* channel)
{
    u32 pitch;
    u32 waveform;
    u16 depth;
    u32 depth_level;
    u32 scaled;
    u8* cursor;
    s32 value;

    channel->flags |= AKAO_CH_PITCH_LFO;
    if (channel->is_sfx_channel != 0)
    {
        cursor = channel->seq_cursor;
        channel->pitch_lfo_delay = 0;
        value = *cursor;
        channel->seq_cursor = cursor + 1;
        if (value != 0)
        {
            channel->pitch_lfo_depth = value << 8;
        }
    }
    else
    {
        cursor = channel->seq_cursor;
        channel->pitch_lfo_delay = *cursor;
        channel->seq_cursor = cursor + 1;
    }
    cursor = channel->seq_cursor;
    value = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->pitch_lfo_period = value;
    if (value == 0)
    {
        channel->pitch_lfo_period = 0x100;
    }
    cursor = channel->seq_cursor;
    depth = channel->pitch_lfo_depth;
    waveform = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->pitch_lfo_waveform = waveform;
    /* This op reads only the low halfword of the 0x2C pitch word (lhu). */
    pitch = *(u16*)&channel->pitch;
    depth_level = (u32)(depth & 0x7F00) >> 8;
    if (!(depth & 0x8000))
    {
        scaled = depth_level * ((s32)(pitch * 15) >> 8);
    }
    else
    {
        scaled = depth_level * pitch;
    }
    channel->pitch_lfo_depth_scaled = scaled >> 7;
    channel->unk1C = g_akao_lfo_waveforms[channel->pitch_lfo_waveform];
    channel->pitch_lfo_delay_ticks = channel->pitch_lfo_delay;
    channel->pitch_lfo_restart = 1;
}

/**
 * @brief Set the active pitch-LFO depth and recompute its scaled depth.
 * @param channel Channel state whose bytecode cursor is advanced by one byte.
 * @note The depth is re-read through a pointer to @c pitch_lfo_depth_scaled;
 *       reading @c pitch_lfo_depth directly changes the register allocation.
 * @see decomp.me (100%)
 */
void akao_seq_op_set_pitch_lfo_depth(AkaoChannelState* channel)
{
    s32 pitch;
    u8* cursor;
    u16* scaled_ptr;
    u32 raw_depth;
    u16 depth;
    u32 depth_level;
    u32 scaled;

    cursor = channel->seq_cursor;
    raw_depth = *cursor << 8;
    channel->seq_cursor = cursor + 1;
    pitch = channel->pitch;
    channel->pitch_lfo_depth = raw_depth;
    scaled_ptr = &channel->pitch_lfo_depth_scaled;
    depth = scaled_ptr[1];
    depth_level = (u32)(depth & 0x7F00) >> 8;
    if (!(depth & 0x8000))
    {
        scaled = depth_level * ((s32)(pitch * 15) >> 8);
    }
    else
    {
        scaled = depth_level * pitch;
    }
    channel->pitch_lfo_depth_scaled = scaled >> 7;
}

/**
 * @brief Slide the pitch-LFO depth to a target over a tick count.
 * @param channel Channel state whose bytecode cursor is advanced by two bytes.
 * @see decomp.me (100%)
 */
void akao_seq_op_slide_pitch_lfo_depth(AkaoChannelState* channel)
{
    u8* cursor;
    s32 ticks;
    s32 step;

    cursor = channel->seq_cursor;
    ticks = *cursor;
    cursor += 1;
    channel->seq_cursor = cursor;
    if (ticks == 0)
    {
        ticks = 0x100;
    }
    step = ((*cursor << 8) - channel->pitch_lfo_depth) / ticks;
    channel->seq_cursor = cursor + 1;
    channel->pitch_lfo_depth_fade_ticks = ticks;
    channel->pitch_lfo_depth_step = step;
}

/**
 * @brief Stop the pitch LFO: clear its output and enable flag, and flag a
 *        pitch register update.
 * @param channel Channel state.
 * @see decomp.me (100%)
 */
void akao_seq_op_stop_pitch_lfo(AkaoChannelState* channel)
{
    channel->pitch_lfo_value = 0;
    channel->flags &= ~AKAO_CH_PITCH_LFO;
    channel->update_flags |= 0x10;
}

/**
 * @brief Start the channel volume LFO, selecting its delay, period, waveform,
 *        and depth.
 * @param channel Channel state whose bytecode cursor is advanced by three bytes.
 * @see decomp.me (100%)
 */
void akao_seq_op_start_volume_lfo(AkaoChannelState* channel)
{
    u8* cursor;
    s32 delay;
    s32 period;
    u32 waveform;

    cursor = channel->seq_cursor;
    channel->flags |= AKAO_CH_VOLUME_LFO;
    delay = *cursor;
    channel->seq_cursor = cursor + 1;
    if (channel->is_sfx_channel != 0)
    {
        channel->volume_lfo_delay = 0;
        if (delay != 0)
        {
            channel->volume_lfo_depth = (delay & 0x7F) << 8;
        }
    }
    else
    {
        channel->volume_lfo_delay = delay;
    }
    period = *channel->seq_cursor++;
    channel->volume_lfo_period = period;
    if (period == 0)
    {
        channel->volume_lfo_period = 0x100;
    }
    waveform = *channel->seq_cursor++;
    channel->volume_lfo_waveform = waveform;
    channel->tempo = g_akao_lfo_waveforms[channel->volume_lfo_waveform];
    channel->volume_lfo_delay_ticks = channel->volume_lfo_delay;
    channel->volume_lfo_restart = 1;
}

/**
 * @brief Set the active volume-LFO depth.
 * @param channel Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void akao_seq_op_set_volume_lfo_depth(AkaoChannelState* channel)
{
    u8* cursor;
    s32 depth;

    cursor = channel->seq_cursor;
    depth = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->volume_lfo_depth = (depth & 0x7F) << 8;
}

/**
 * @brief Slide the volume-LFO depth to a target over a tick count.
 * @param channel Channel state whose bytecode cursor is advanced by two bytes.
 * @see decomp.me (100%)
 */
void akao_seq_op_slide_volume_lfo_depth(AkaoChannelState* channel)
{
    u8* cursor;
    s32 ticks;
    s32 step;

    cursor = channel->seq_cursor;
    ticks = *cursor;
    cursor += 1;
    channel->seq_cursor = cursor;
    if (ticks == 0)
    {
        ticks = 0x100;
    }
    step = (((*cursor & 0x7F) << 8) - channel->volume_lfo_depth) / ticks;
    channel->seq_cursor = cursor + 1;
    channel->volume_lfo_depth_fade_ticks = ticks;
    channel->volume_lfo_depth_step = step;
}

/**
 * @brief Stop the volume LFO: clear its output and enable flag, and flag a
 *        volume register update.
 * @param channel Channel state.
 * @see decomp.me (100%)
 */
void akao_seq_op_stop_volume_lfo(AkaoChannelState* channel)
{
    channel->volume_lfo_value = 0;
    channel->flags &= ~AKAO_CH_VOLUME_LFO;
    channel->update_flags |= 3;
}

/**
 * @brief Start the channel pan LFO, selecting its period and waveform.
 * @param channel Channel state whose bytecode cursor is advanced by two bytes.
 * @see decomp.me (100%)
 */
void akao_seq_op_start_pan_lfo(AkaoChannelState* channel)
{
    s32 period;
    u32 waveform;

    channel->flags |= AKAO_CH_PAN_LFO;
    period = *channel->seq_cursor++;
    channel->pan_lfo_period = period;
    if (period == 0)
    {
        channel->pan_lfo_period = 0x100;
    }
    waveform = *channel->seq_cursor++;
    channel->pan_lfo_waveform = waveform;
    channel->tempo_step = g_akao_lfo_waveforms[channel->pan_lfo_waveform];
    channel->pan_lfo_restart = 1;
}

/**
 * @brief Set the active pan-LFO depth.
 * @param channel Channel state whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%)
 */
void akao_seq_op_set_pan_lfo_depth(AkaoChannelState* channel)
{
    u8* cursor;
    s32 depth;

    cursor = channel->seq_cursor;
    depth = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->pan_lfo_depth = depth << 7;
}

/**
 * @brief Slide the pan-LFO depth to a target over a tick count.
 * @param channel Channel state whose bytecode cursor is advanced by two bytes.
 * @see decomp.me (100%)
 */
void akao_seq_op_slide_pan_lfo_depth(AkaoChannelState* channel)
{
    u8* cursor;
    s32 ticks;
    s32 step;

    cursor = channel->seq_cursor;
    ticks = *cursor;
    cursor += 1;
    channel->seq_cursor = cursor;
    if (ticks == 0)
    {
        ticks = 0x100;
    }
    step = ((*cursor << 7) - channel->pan_lfo_depth) / ticks;
    channel->seq_cursor = cursor + 1;
    channel->pan_lfo_depth_fade_ticks = ticks;
    channel->pan_lfo_depth_step = step;
}

/**
 * @brief Stop the pan LFO: clear its output and enable flag, and flag a
 *        volume register update.
 * @param channel Channel state.
 * @see decomp.me (100%)
 */
void akao_seq_op_stop_pan_lfo(AkaoChannelState* channel)
{
    channel->pan_lfo_value = 0;
    channel->flags &= ~AKAO_CH_PAN_LFO;
    channel->update_flags |= 3;
}

/**
 * @brief AKAO opcode handler: OR-sets a caller-supplied flag mask into either
 *        the SFX control block or the primary sequence channel (depending on
 *        whether this channel is an SFX channel), then raises driver flags 0x110.
 * @param channel Channel state; @c is_sfx_channel selects SFX vs sequence routing.
 * @param channel_mask Flag bitmask to OR in.
 * @note Residual: the g_akao_seq_channel0 %hi colors to v0 not v1 (one lui
 *       register), a gcc 2.8 coloring tie-break the permuter cannot move.
 * @see decomp.me (99.58%)
 */
void akao_seq_op_enable_reverb(AkaoChannelState* channel, s32 channel_mask)
{
    if (channel->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->reverb_mask |= channel_mask;
    }
    else
    {
        g_akao_sfx_control.reverb_mask |= channel_mask;
    }
    g_akao_driver_flags.unk8 |= 0x110;
}

/**
 * @brief AKAO opcode handler: AND-clears a caller-supplied flag mask from either
 *        the SFX control block or the primary sequence channel, raises driver
 *        flags 0x110, and clears the pending reverb toggle countdown.
 * @param channel Channel state; @c is_sfx_channel selects SFX vs sequence routing.
 * @param channel_mask Flag bitmask to clear (applied as @c &= ~channel_mask).
 * @see decomp.me (100%)
 */
void akao_seq_op_disable_reverb(AkaoChannelState* channel, s32 channel_mask)
{
    if (channel->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->reverb_mask &= ~channel_mask;
    }
    else
    {
        g_akao_sfx_control.reverb_mask &= ~channel_mask;
    }
    g_akao_driver_flags.unk8 |= 0x110;
    channel->reverb_toggle_ticks = 0;
}

/**
 * @brief AKAO opcode handler: OR-sets a caller-supplied flag mask into the
 *        song pitch-modulation mask when this channel is a sequence channel, or
 *        into the SFX pitch-modulation mask when SFX flag 0x10000 is set, then
 *        raises driver flags 0x100.
 * @param channel Channel state; @c is_sfx_channel selects sequence routing, @c flags gates SFX.
 * @param channel_mask Flag bitmask to OR in.
 * @note Residual: the g_akao_seq_channel0 %hi colors to v0 not v1 (one lui
 *       register), a gcc 2.8 coloring tie-break shared with akao_seq_op_enable_reverb.
 * @see decomp.me (99.66%)
 */
void akao_seq_op_enable_pitch_modulation(AkaoChannelState* channel, s32 channel_mask)
{
    if (channel->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->pitch_mod_mask |= channel_mask;
    }
    else if (channel->flags & AKAO_CH_SFX_PITCH_MOD)
    {
        g_akao_sfx_control.pitch_mod_mask |= channel_mask;
    }
    g_akao_driver_flags.unk8 |= 0x100;
}

/**
 * @brief AKAO opcode handler: AND-clears a caller-supplied flag mask from either
 *        the song or SFX pitch-modulation mask, raises driver flags 0x100, and
 *        clears the pending pitch-modulation toggle countdown.
 * @param channel Channel state; @c is_sfx_channel selects SFX vs sequence routing.
 * @param channel_mask Flag bitmask to clear (applied as @c &= ~channel_mask).
 * @see decomp.me (100%)
 */
void akao_seq_op_disable_pitch_modulation(AkaoChannelState* channel, s32 channel_mask)
{
    if (channel->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->pitch_mod_mask &= ~channel_mask;
    }
    else
    {
        g_akao_sfx_control.pitch_mod_mask &= ~channel_mask;
    }
    g_akao_driver_flags.unk8 |= 0x100;
    channel->pitch_mod_toggle_ticks = 0;
}

/**
 * @brief AKAO opcode handler: OR-sets a caller-supplied flag mask into either
 *        the song or SFX noise mask, then
 *        raises driver flags 0x100.
 * @param channel Channel state; @c is_sfx_channel selects SFX vs sequence routing.
 * @param channel_mask Flag bitmask to OR in.
 * @note Residual: the g_akao_seq_channel0 %hi coloring tie-break shared with
 *       akao_seq_op_enable_reverb.
 * @see decomp.me (99.58%)
 */
void akao_seq_op_enable_noise(AkaoChannelState* channel, s32 channel_mask)
{
    if (channel->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->noise_mask |= channel_mask;
    }
    else
    {
        g_akao_sfx_control.noise_mask |= channel_mask;
    }
    g_akao_driver_flags.unk8 |= 0x100;
}

/**
 * @brief AKAO opcode handler: AND-clears a caller-supplied flag mask from either
 *        the song or SFX noise mask, then
 *        raises driver flags 0x100.
 * @param channel Channel state; @c is_sfx_channel selects SFX vs sequence routing.
 * @param channel_mask Flag bitmask to clear (applied as @c &= ~channel_mask).
 * @note Residual: the seq-channel path register coloring differs (5 rows), a
 *       gcc 2.8 coloring tie-break shared with the reverb handlers.
 * @see decomp.me (98.13%)
 */
void akao_seq_op_disable_noise(AkaoChannelState* channel, s32 channel_mask)
{
    if (channel->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->noise_mask &= ~channel_mask;
    }
    else
    {
        g_akao_sfx_control.noise_mask &= ~channel_mask;
    }
    g_akao_driver_flags.unk8 |= 0x100;
}

/**
 * @brief Enable tied notes; subsequent notes change pitch without retriggering.
 * @param channel Channel state.
 * @see decomp.me (100%)
 */
void akao_seq_op_enable_note_tie(AkaoChannelState* channel)
{
    channel->note_flags = 1;
}

/**
 * @brief No-op handler for primary opcode 0xCD.
 * @see decomp.me (100%)
 */
void akao_seq_op_nop_cd(void)
{
}

/**
 * @brief Give SFX notes their full duration instead of an early key-off.
 * @param channel Channel state; @c is_sfx_channel selects whether the store happens.
 * @see decomp.me (100%)
 */
void akao_seq_op_enable_sfx_full_gate(AkaoChannelState* channel)
{
    if (channel->is_sfx_channel != 0)
    {
        channel->note_flags = 4;
    }
}

/**
 * @brief No-op handler for primary opcode 0xD1.
 * @see decomp.me (100%)
 */
void akao_seq_op_nop_d1(void)
{
}

/**
 * @brief Set or add to the SPU noise frequency of the song or the SFX set.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/Get0N
 */
void akao_seq_op_set_noise_frequency(AkaoChannelState* channel)
{
    s16 frequency;
    u8* cursor;

    cursor = channel->seq_cursor;
    frequency = *cursor;
    channel->seq_cursor = cursor + 1;
    if (channel->is_sfx_channel == 0)
    {
        if (frequency & 0xC0)
        {
            g_akao_seq_channel0->noise_freq = (g_akao_seq_channel0->noise_freq + (frequency & 0x3F)) & 0x3F;
        }
        else
        {
            g_akao_seq_channel0->noise_freq = frequency;
        }
    }
    else if (frequency & 0xC0)
    {
        g_akao_sfx_control.unk28 = (g_akao_sfx_control.unk28 + (frequency & 0x3F)) & 0x3F;
    }
    else
    {
        D_8004D428[0] = frequency;
    }
    g_akao_driver_flags.unk8 |= 0x10;
}

/**
 * @brief Override the ADSR attack rate.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/gwBIb
 */
void akao_seq_op_set_adsr_attack(AkaoChannelState* channel)
{
    u8* cursor;
    u32 value;
    u32 update_flags;
    u32 flags;
    u16 adsr_low;

    cursor = channel->seq_cursor;
    value = *cursor;
    channel->seq_cursor = cursor + 1;

    update_flags = channel->update_flags | 0x900;
    channel->update_flags = update_flags;

    flags = channel->flags | AKAO_CH_ADSR_ATTACK;

    adsr_low = (channel->spu_adsr_low & 0x80FF) | ((u16)value << 8);

    channel->flags = flags;
    channel->spu_adsr_low = adsr_low;
}

/**
 * @brief Override the ADSR decay rate.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @param channel_mask Bit of the channel being stepped (unused).
 * @see decomp.me (100%) https://decomp.me/scratch/ID5s7
 */
void akao_seq_op_set_adsr_decay(AkaoChannelState* channel, s32 channel_mask)
{
    u8* cursor;
    u32 value;
    u32 update_flags;
    u16 adsr_low;

    cursor = channel->seq_cursor;
    value = *cursor;
    channel->seq_cursor = cursor + 1;

    update_flags = channel->update_flags | 0x1000;
    adsr_low = ((channel->spu_adsr_low & 0xFF0F) | (value * 0x10));

    channel->update_flags = update_flags;
    channel->spu_adsr_low = adsr_low;
}

/**
 * @brief Override the ADSR sustain level.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @param channel_mask Bit of the channel being stepped (unused).
 * @see decomp.me (100%) https://decomp.me/scratch/ZOmaE
 */
void akao_seq_op_set_adsr_sustain_level(AkaoChannelState* channel, s32 channel_mask)
{
    u8* cursor;
    u32 value;
    u32 update_flags;
    u16 adsr_low;

    cursor = channel->seq_cursor;
    value = *cursor;
    channel->seq_cursor = cursor + 1;
    update_flags = channel->update_flags | 0x8000;
    adsr_low = (channel->spu_adsr_low & 0xFFF0) | value;
    channel->update_flags = update_flags;
    channel->spu_adsr_low = adsr_low;
}

/**
 * @brief Override the ADSR sustain rate.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/Tun26
 */
void akao_seq_op_set_adsr_sustain_rate(AkaoChannelState* channel)
{
    u8* cursor;
    u32 value;
    u32 update_flags;
    u32 flags;
    u16 adsr_high;

    cursor = channel->seq_cursor;
    value = *cursor;
    channel->seq_cursor = cursor + 1;

    update_flags = channel->update_flags | 0x2200;
    flags = channel->flags | AKAO_CH_ADSR_SUSTAIN_RATE;
    adsr_high = (channel->spu_adsr_high & 0xE03F) | (value << 6);

    channel->update_flags = update_flags;
    channel->flags = flags;
    channel->spu_adsr_high = adsr_high;
}

/**
 * @brief Override the ADSR release rate.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/DR9uQ
 */
void akao_seq_op_set_adsr_release_rate(AkaoChannelState* channel)
{
    u8* cursor;
    u32 value;
    u32 update_flags;
    u32 flags;
    u16 adsr_high;

    cursor = channel->seq_cursor;
    value = *cursor;
    channel->seq_cursor = cursor + 1;

    update_flags = channel->update_flags | 0x4400;
    flags = channel->flags | AKAO_CH_ADSR_RELEASE_RATE;
    adsr_high = (channel->spu_adsr_high & 0xFFE0) | value;

    channel->update_flags = update_flags;
    channel->flags = flags;
    channel->spu_adsr_high = adsr_high;
}

/**
 * @brief Select linear or exponential ADSR attack (5 = exponential).
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/dP97Y
 */
void akao_seq_op_set_adsr_attack_mode(AkaoChannelState* channel)
{
    u8* cursor;
    u32 value;
    u16 adsr_low;

    cursor = channel->seq_cursor;
    value = *cursor;
    channel->seq_cursor = cursor + 1;

    adsr_low = channel->spu_adsr_low & 0x7FFF;
    channel->spu_adsr_low = adsr_low;
    if (value == 5)
    {
        adsr_low |= 0x8000;
        channel->spu_adsr_low = adsr_low;
    }

    channel->update_flags |= 0x100;
}

/**
 * @brief Select the ADSR sustain direction/mode (3, 5 or 7).
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/8od0h
 */
void akao_seq_op_set_adsr_sustain_mode(AkaoChannelState* channel)
{
    s32 mode;
    u16 adsr_high;
    u8* cursor;
    u32 value;

    cursor = channel->seq_cursor;
    value = *cursor;
    adsr_high = channel->spu_adsr_high & 0x3FFF;
    channel->seq_cursor = cursor + 1;
    mode = value & 0xFFFF;
    channel->spu_adsr_high = adsr_high;

    switch (mode)
    {
    case 3:
        channel->spu_adsr_high = adsr_high | 0x4000;
        break;
    case 5:
        channel->spu_adsr_high = adsr_high | 0x8000;
        break;
    case 7:
        channel->spu_adsr_high = adsr_high | 0xC000;
        break;
    }

    channel->update_flags |= 0x200;
}

/**
 * @brief Select linear or exponential ADSR release (7 = exponential).
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/1Pqa0
 */
void akao_seq_op_set_adsr_release_mode(AkaoChannelState* channel)
{

    u8* cursor;
    u16 adsr_high;
    u32 value;

    cursor = channel->seq_cursor;
    value = *cursor;
    channel->seq_cursor = cursor + 1;

    adsr_high = channel->spu_adsr_high & 0xFFDF;
    channel->spu_adsr_high = adsr_high;

    if (value == 7)
    {
        adsr_high |= 0x20;
        channel->spu_adsr_high = adsr_high;
    }

    channel->update_flags |= 0x400;
}

/**
 * @brief Extended opcode FE 10: reserve SPU voices below the operand base.
 *        Installs the operand byte as g_akao_seq_channel0->voice_alloc_base, the
 *        first freely-allocatable voice index; voices below it become reserved.
 * @param channel Channel whose bytecode cursor is advanced past the base byte.
 * @see decomp.me (100%) https://decomp.me/scratch/rLRQL
 */
void akao_seq_op_allocate_reserved_voices(AkaoChannelState* channel)
{
    u8* cursor;

    cursor = channel->seq_cursor;
    g_akao_seq_channel0->voice_alloc_base = *cursor;
    channel->seq_cursor = cursor + 1;
}

/**
 * @brief Extended opcode FE 11: release the reserved-voice window (base = 0).
 * @see decomp.me (100%) https://decomp.me/scratch/9FX05
 */
void akao_seq_op_free_reserved_voices(void)
{
    g_akao_seq_channel0->voice_alloc_base = 0;
}

/**
 * @brief Push a loop level and remember its start cursor.
 * @param channel Channel state.
 * @see decomp.me (100%) https://decomp.me/scratch/pTtu4
 */
void akao_seq_op_loop_start(AkaoChannelState* channel)
{
    channel->loop_depth = (channel->loop_depth + 1) & 3;
    channel->w04.loop_cursor[channel->loop_depth] = channel->seq_cursor;
    channel->loop_count[channel->loop_depth] = 0;
    channel->loop_opcode_count[channel->loop_depth] = channel->opcode_count;
}

/**
 * @brief Close a loop: repeat until the operand count is reached, then pop.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/xSadm
 */
void akao_seq_op_loop_end(AkaoChannelState* channel)
{
    u32 count;

    count = *channel->seq_cursor;
    channel->seq_cursor++;

    if (count == 0)
    {
        count = 0x100;
    }

    if (++channel->loop_count[channel->loop_depth] != count)
    {
        channel->seq_cursor = channel->w04.loop_cursor[channel->loop_depth];
        channel->opcode_count = channel->loop_opcode_count[channel->loop_depth];
        return;
    }

    channel->loop_depth = (channel->loop_depth - 1) & 3;
}

/**
 * @brief Branch by a relative offset on the last pass of the current loop.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/XFsED
 */
void akao_seq_op_branch_on_loop_last(AkaoChannelState* channel)
{
    u8* cursor;
    u32 count;

    cursor = channel->seq_cursor;
    count = *cursor;
    cursor++;
    channel->seq_cursor = cursor;

    if (count == 0)
    {
        count = 0x100;
    }

    if (channel->loop_count[channel->loop_depth] + 1 != count)
    {
        channel->seq_cursor = cursor + 2;
        return;
    }

    channel->seq_cursor = cursor + AKAO_READ_S16(cursor);
}

/**
 * @brief On the last loop pass, branch by a relative offset and pop the loop.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/Gx2w7
 */
void akao_seq_op_branch_and_end_loop(AkaoChannelState* channel)
{
    u8* cursor;
    u32 count;

    cursor = channel->seq_cursor;
    count = *cursor;
    cursor++;
    channel->seq_cursor = cursor;

    if (count == 0)
    {
        count = 0x100;
    }

    if (channel->loop_count[channel->loop_depth] + 1 != count)
    {
        channel->seq_cursor = cursor + 2;
        return;
    }

    channel->seq_cursor = cursor + AKAO_READ_S16(cursor);
    channel->loop_depth = (channel->loop_depth - 1) & 3;
}

/**
 * @brief Unconditionally repeat the current loop.
 * @param channel Channel state.
 * @see decomp.me (100%) https://decomp.me/scratch/Eytqk
 */
void akao_seq_op_repeat_loop(AkaoChannelState* channel)
{
    channel->loop_count[channel->loop_depth]++;
    channel->seq_cursor = channel->w04.loop_cursor[channel->loop_depth];
    channel->opcode_count = channel->loop_opcode_count[channel->loop_depth];
}

/**
 * @brief Set a fixed duration for the following notes.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/FnAXw
 */
void akao_seq_op_set_note_duration(AkaoChannelState* channel)
{
    s32 duration;

    duration = *channel->seq_cursor;
    channel->seq_cursor++;

    channel->note_duration_adjust = 0;
    channel->unk68 = duration;
    channel->unk66 = duration;
    channel->note_duration = duration;
}

/**
 * @brief Add a signed delta to the note duration (clamped to 1..255).
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/PBFzu
 */
void akao_seq_op_adjust_note_duration(AkaoChannelState* channel)
{
    s32 duration;

    duration = (s8)*channel->seq_cursor;
    channel->seq_cursor++;

    if (duration != 0)
    {
        duration += channel->note_duration;

        if (duration <= 0)
        {
            duration = 1;
        }
        else if (duration >= 256)
        {
            duration = 255;
        }
    }

    channel->note_duration_adjust = duration;
}

/**
 * @brief Extended opcode FE 04: enable drum mode (per-note articulation table).
 *        Sets channel flag 0x8 when the song note table is present, so each note
 *        resolves its own articulation/key/pan via akao_channel_start_note.
 * @param channel Channel to switch into drum mode.
 * @see decomp.me (100%) https://decomp.me/scratch/CB7Yy
 */
void akao_seq_op_enable_drum_mode(AkaoChannelState* channel)
{
    /* Song role: 0x34 is the note-table pointer, tested for presence. */
    if (g_akao_seq_channel0->flags != 0)
    {
        channel->flags = (channel->flags & ~AKAO_CH_ARTICULATION_MASK) | AKAO_CH_DRUM_MODE;
    }
}

/**
 * @brief Extended opcode FE 05: disable drum mode (clear flag 0x8, reset volume scale).
 * @param channel Channel to switch out of drum mode.
 * @see decomp.me (100%) https://decomp.me/scratch/FnMZ3
 */
void akao_seq_op_disable_drum_mode(AkaoChannelState* channel)
{
    channel->spu_volume_scale = 0;
    channel->flags &= ~8;
}

/**
 * @brief Extended opcode FE 15: set the time signature (ticks-per-beat and
 *        beats-per-measure) and reset the beat/tick counters.
 * @param channel Channel whose bytecode cursor is advanced past the two operand bytes.
 * @see decomp.me (100%) https://decomp.me/scratch/wuYt5
 */
void akao_seq_op_set_time_signature(AkaoChannelState* channel)
{
    AkaoChannelState* song = g_akao_seq_channel0;

    /* Song role: 0x68 is ticks-per-beat and is_sfx_channel is
     * beats-per-measure - this is the FE 15 time-signature opcode. */
    song->unk68 = *channel->seq_cursor++;
    song->is_sfx_channel = *channel->seq_cursor++;
    song->unk6A = 0;
    song->unk66 = 0;
}

/**
 * @brief Extended opcode FE 16: set the current song measure from a 16-bit operand.
 * @param channel Channel whose bytecode cursor is advanced past the two operand bytes.
 * @see decomp.me (100%) https://decomp.me/scratch/FcDgE
 */
void akao_seq_op_set_measure(AkaoChannelState* channel)
{
    AkaoChannelState* song = g_akao_seq_channel0;

    song->measure = *channel->seq_cursor++;
    song->measure |= *channel->seq_cursor++ << 8;
}

/**
 * @brief Opcode 0xB0: set ADSR decay rate and sustain level together.
 *        Delegates to akao_seq_op_set_adsr_decay then akao_seq_op_set_adsr_sustain_level.
 * @param channel Channel being programmed.
 * @param channel_mask Second argument forwarded to both sub-handlers.
 * @see decomp.me (100%) https://decomp.me/scratch/XqM1L
 */
void akao_seq_op_set_adsr_decay_sustain_level(AkaoChannelState* channel, s32 channel_mask)
{
    akao_seq_op_set_adsr_decay(channel, channel_mask);
    akao_seq_op_set_adsr_sustain_level(channel, channel_mask);
}

/**
 * @brief Opcode 0xCE: enable reverb now and schedule an auto-toggle after N ticks.
 *        Sets reverb_toggle_ticks (operand+1, or 0x101 when zero) then enables
 *        reverb; the countdown XOR-toggles the reverb bit when it reaches zero.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @param channel_mask Channel bit-mask forwarded to akao_seq_op_enable_reverb.
 * @see decomp.me (100%) https://decomp.me/scratch/cAsju
 */
void akao_seq_op_enable_reverb_then_toggle(AkaoChannelState* channel, s32 channel_mask)
{
    u8* cursor;
    s32 ticks;
    s16 toggle_ticks;

    cursor = channel->seq_cursor;
    ticks = *cursor;

    channel->seq_cursor = cursor + 1;

    if (ticks != 0)
    {
        toggle_ticks = ticks + 1;
    }
    else
    {
        toggle_ticks = 0x101;
    }

    channel->reverb_toggle_ticks = toggle_ticks;
    akao_seq_op_enable_reverb(channel, channel_mask);
}

/**
 * @brief Opcode 0xCF: schedule a reverb toggle after N ticks without enabling now.
 *        Sets reverb_toggle_ticks (operand+1, or 0x101 when zero).
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/Basyw
 */
void akao_seq_op_schedule_reverb_toggle(AkaoChannelState* channel)
{
    u8* cursor;
    s32 ticks;

    cursor = channel->seq_cursor;
    ticks = *cursor;

    channel->seq_cursor = cursor + 1;

    if (ticks != 0)
    {
        channel->reverb_toggle_ticks = ticks + 1;
    }
    else
    {
        channel->reverb_toggle_ticks = 0x101;
    }
}

/**
 * @brief Opcode 0xD2: enable pitch modulation now and schedule an auto-toggle.
 *        Sets pitch_mod_toggle_ticks (operand+1, or 0x101 when zero) then enables
 *        pitch modulation; the countdown XOR-toggles the bit when it reaches zero.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @param channel_mask Channel bit-mask forwarded to akao_seq_op_enable_pitch_modulation.
 * @see decomp.me (100%) https://decomp.me/scratch/j1qFh
 */
void akao_seq_op_enable_pitch_mod_then_toggle(AkaoChannelState* channel, s32 channel_mask)
{
    u8* cursor;
    s32 ticks;
    s16 toggle_ticks;

    cursor = channel->seq_cursor;
    ticks = *cursor;

    channel->seq_cursor = cursor + 1;

    if (ticks != 0)
    {
        toggle_ticks = ticks + 1;
    }
    else
    {
        toggle_ticks = 0x101;
    }

    channel->pitch_mod_toggle_ticks = toggle_ticks;
    akao_seq_op_enable_pitch_modulation(channel, channel_mask);
}

/**
 * @brief Opcode 0xD3: schedule a pitch-modulation toggle after N ticks.
 *        Sets pitch_mod_toggle_ticks (operand+1, or 0x101 when zero).
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/3922q
 */
void akao_seq_op_schedule_pitch_mod_toggle(AkaoChannelState* channel)
{
    u8* cursor;
    s32 ticks;

    cursor = channel->seq_cursor;
    ticks = *cursor;

    channel->seq_cursor = cursor + 1;

    if (ticks != 0)
    {
        channel->pitch_mod_toggle_ticks = ticks + 1;
    }
    else
    {
        channel->pitch_mod_toggle_ticks = 0x101;
    }
}

/**
 * @brief Opcode 0xCB: reset channel effects. Clears the LFO and side-chain flags
 *        (0x37 = pitch/vol/pan LFO + pitch/pitch-volume side-chain), disables
 *        reverb, pitch modulation and noise, and clears the note-tie bits.
 * @param channel Channel to reset.
 * @param channel_mask Channel bit-mask forwarded to the disable-* sub-handlers.
 * @see decomp.me (100%) https://decomp.me/scratch/LEJvC
 */
void akao_seq_op_reset_effects(AkaoChannelState* channel, s32 channel_mask)
{
    channel->flags &= ~(AKAO_CH_PITCH_LFO | AKAO_CH_VOLUME_LFO | AKAO_CH_PAN_LFO | AKAO_CH_PITCH_SIDECHAIN | AKAO_CH_PITCH_VOLUME_SIDECHAIN);

    akao_seq_op_disable_reverb(channel, channel_mask);
    akao_seq_op_disable_pitch_modulation(channel, channel_mask);
    akao_seq_op_disable_noise(channel, channel_mask);

    channel->note_flags &= 0xFFFA;
}

/**
 * @brief Opcode 0xD4: enable pitch side-chaining (channel flag 0x10). While set,
 *        the voice tick recomputes this channel's SPU pitch every tick from the
 *        previous channel's spu_pitch (see akao_voice.c).
 * @param channel Channel to enable.
 * @see decomp.me (100%) https://decomp.me/scratch/8l07k
 */
void akao_seq_op_enable_pitch_sidechain(AkaoChannelState* channel)
{
    channel->flags |= AKAO_CH_PITCH_SIDECHAIN;
}

/**
 * @brief Opcode 0xD5: disable pitch side-chaining (clear channel flag 0x10).
 * @param channel Channel to disable.
 * @see decomp.me (100%) https://decomp.me/scratch/YSIzI
 */
void akao_seq_op_disable_pitch_sidechain(AkaoChannelState* channel)
{
    channel->flags &= ~AKAO_CH_PITCH_SIDECHAIN;
}

/**
 * @brief Opcode 0xD6: enable pitch->volume side-chaining (channel flag 0x20).
 *        While set, the voice tick folds the previous channel's pitch into this
 *        channel's volume each tick (see akao_voice.c).
 * @param channel Channel to enable.
 * @see decomp.me (100%) https://decomp.me/scratch/iCXHA
 */
void akao_seq_op_enable_pitch_volume_sidechain(AkaoChannelState* channel)
{
    channel->flags |= AKAO_CH_PITCH_VOLUME_SIDECHAIN;
}

/**
 * @brief Opcode 0xD7: disable pitch->volume side-chaining (clear channel flag 0x20).
 * @param channel Channel to disable.
 * @see decomp.me (100%) https://decomp.me/scratch/p5dcS
 */
void akao_seq_op_disable_pitch_volume_sidechain(AkaoChannelState* channel)
{
    channel->flags &= ~AKAO_CH_PITCH_VOLUME_SIDECHAIN;
}

/**
 * @brief Extended opcode FE 0B: play a sound effect from the sequence stream.
 *        Reads two relative pointers, fills the SFX parameter block D_8004D3A0
 *        (pan, expression) and launches it via akao_sfx_play.
 * @param channel Channel whose bytecode cursor is advanced past the two 16-bit operands.
 * @see decomp.me (100%) https://decomp.me/scratch/nwIop
 */
void akao_seq_op_play_sfx(AkaoChannelState* channel)
{
    u8* cursor;
    s32 offset;
    u8* seq_data0;
    u8* seq_data1;

    cursor = channel->seq_cursor;

    offset = (cursor[1] << 8) | cursor[0];

    if (offset != 0)
    {
        seq_data0 = cursor + offset + 2;
    }
    else
    {
        seq_data0 = 0;
    }

    cursor += 2;

    offset = (cursor[1] << 8) | cursor[0];
    if (offset != 0)
    {
        seq_data1 = cursor + offset + 2;
    }
    else
    {
        seq_data1 = 0;
    }

    D_8004D3A0.unk0 = 0;
    D_8004D3A0.unk4 = 0;
    D_8004D3A0.pan = channel->pan >> 8;
    D_8004D3A0.expression = channel->unk48 >> 23;

    akao_sfx_play(&D_8004D3A0, seq_data0, seq_data1, 0);

    channel->seq_cursor += 4;
}

/**
 * @brief Extended opcode FE 17: set the channel pan bias immediately.
 *        Stores operand<<8 into pan_bias, sets flag 0x800, and (side effect)
 *        enables this channel in the noise mask via akao_seq_op_enable_noise.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @param channel_mask Channel bit-mask forwarded to akao_seq_op_enable_noise.
 * @note The noise-mask enable is an intentional side effect of this LoM opcode;
 *       its purpose alongside the pan bias is not yet understood.
 * @see decomp.me (100%) https://decomp.me/scratch/B5HO1
 */
void akao_seq_op_set_pan_bias(AkaoChannelState* channel, s32 channel_mask)
{
    s32 value;

    value = *channel->seq_cursor;
    channel->seq_cursor++;

    channel->pan_bias_fade_ticks = 0;
    channel->flags |= AKAO_CH_PAN_BIAS;
    channel->pan_bias = value << 8;

    akao_seq_op_enable_noise(channel, channel_mask);
}

/**
 * @brief Extended opcode FE 18: slide the channel pan bias to a target over N ticks.
 *        Sets pan_bias_step / pan_bias_fade_ticks, sets flag 0x800, and (side
 *        effect) enables this channel in the noise mask.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @param channel_mask Channel bit-mask forwarded to akao_seq_op_enable_noise.
 * @note See akao_seq_op_set_pan_bias regarding the noise-mask side effect.
 * @see decomp.me (100%) https://decomp.me/scratch/fM3EA
 */
void akao_seq_op_slide_pan_bias(AkaoChannelState* channel, s32 channel_mask)
{
    u8* operand;
    u16 current;
    u32 ticks;
    u8* cursor;

    cursor = channel->seq_cursor;
    ticks = *cursor;
    channel->seq_cursor = cursor + 1;

    channel->pan_bias_fade_ticks = ticks;
    if (ticks == 0)
    {
        channel->pan_bias_fade_ticks = 0x100;
    }

    current = channel->pan_bias & 0xFF00;
    operand = channel->seq_cursor;

    channel->pan_bias_step = ((s16)(*operand << 8) - current) / channel->pan_bias_fade_ticks;
    channel->pan_bias = current;
    channel->seq_cursor = operand + 1;
    channel->flags |= AKAO_CH_PAN_BIAS;

    akao_seq_op_enable_noise(channel, channel_mask);
}

/**
 * @brief Opcode 0xE0: enable deferred stop (channel flag 0x100000). When a stop
 *        is later requested, the channel sets 0x200000 and plays on instead of
 *        releasing immediately; the loop opcodes then end rather than repeat.
 * @param channel Channel to mark.
 * @see decomp.me (100%) https://decomp.me/scratch/ws5Yw
 */
void akao_seq_op_enable_deferred_stop(AkaoChannelState* channel)
{
    channel->flags |= AKAO_CH_DEFERRED_STOP;
}

/**
 * @brief Extended opcode FE 1C: consume one operand byte and do nothing else.
 *
 * The operand-length table @c g_akao_opcode_len_table_ext gives this opcode a
 * length of 2 (sub-opcode plus one operand), and the handler only advances the
 * bytecode cursor past that operand, so the command is inert in this driver
 * build. Its meaning in the authoring tool is unknown.
 *
 * @param channel Channel whose bytecode cursor is advanced by one byte.
 * @see decomp.me (100%) https://decomp.me/scratch/ySVnh
 */
void akao_seq_op_skip_operand_byte(AkaoChannelState* channel)
{
    channel->seq_cursor += 1;
}

/**
 * @brief Extended opcode FE 1D: let this channel allocate SPU voices from
 *        voice 0, ignoring the reserved voice base.
 *
 * Sets the channel's bit in the song-state mask at offset 0x08.
 * When akao_process_sequence_voice_updates keys a note on, it passes
 * @c (song->w04.song.voice_alloc_low_mask & channel_mask) to the voice
 * allocator akao_find_free_voice. A set bit makes the free-voice search start at
 * voice 0 instead of at @c song->unk38, the reserved base installed by extended
 * opcode FE 10 and cleared by FE 11. akao_seq_op_obey_voice_reserve (FE 1E) clears the same bit,
 * and akao_release_channels clears it for every released channel.
 *
 * @param channel Channel state; unused, but required to match because the
 *                handler is called through the opcode-table signature.
 * @param channel_mask Bit of the channel being stepped, as passed by
 *                     akao_seq_step_opcode.
 * @see decomp.me (100%) https://decomp.me/scratch/q71gK
 */
void akao_seq_op_ignore_voice_reserve(AkaoChannelState* channel, s32 channel_mask)
{
    g_akao_seq_channel0->w04.song.voice_alloc_low_mask |= channel_mask;
}
