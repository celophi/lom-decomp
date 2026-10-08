#include "internal/akao_sequencer.h"
#include "internal/akao_control.h"
#include "internal/akao_voice.h"
#include <libspu.h>

/* Channel-role AkaoChannelState.flags bits. */
#define AKAO_CH_DRUM_MODE 0x08
#define AKAO_CH_PAN_BIAS 0x800
#define AKAO_CH_KEY_MAP 0x1000
#define AKAO_CH_ADSR_ATTACK 0x01000000
#define AKAO_CH_ADSR_SUSTAIN_RATE 0x08000000
#define AKAO_CH_ADSR_RELEASE_RATE 0x10000000

/** @brief Explicit ADSR overrides, cleared when a new articulation is loaded. */
#define AKAO_CH_ADSR_OVERRIDE_MASK (AKAO_CH_ADSR_ATTACK | AKAO_CH_ADSR_SUSTAIN_RATE | AKAO_CH_ADSR_RELEASE_RATE)

/** @brief Flags cleared when a plain articulation is selected. */
#define AKAO_CH_ARTICULATION_MASK (AKAO_CH_DRUM_MODE | AKAO_CH_KEY_MAP | AKAO_CH_ADSR_OVERRIDE_MASK)

/** @brief Read a little-endian unsigned 16-bit bytecode operand. */
#define AKAO_READ_U16(p) (((p)[1] << AKAO_OPERAND_HIGH_BYTE_SHIFT) | (p)[0])

/** @brief Read a little-endian signed 16-bit bytecode operand. */
#define AKAO_READ_S16(p) ((s16)((p)[0] | ((p)[1] << 8)))

/** @brief Number of semitones used to wrap a pitch-table index or shift an octave. */
#define AKAO_SEMITONES_PER_OCTAVE 12
/** @brief Increment and decrement opcodes wrap the octave across 16 values. */
#define AKAO_OCTAVE_MASK 0xF

/** @brief Encoded detune uses a Q7 boost below 0x80, or a Q8 reduction at and above it. */
#define AKAO_DETUNE_VALUE_MASK 0xFF
#define AKAO_DETUNE_NEGATIVE_THRESHOLD 0x80

/** @brief Signed expression operands are placed above 23 fractional bits. */
#define AKAO_EXPRESSION_FRACTION_BITS 23
/** @brief Expression fades retain the upper half of the current accumulator. */
#define AKAO_EXPRESSION_FADE_MASK 0xFFFF0000

/** @brief Fractional bits discarded when a reverb-depth fade starts. */
#define AKAO_REVERB_DEPTH_FRACTION_MASK ((1 << AKAO_REVERB_DEPTH_FRACTION_BITS) - 1)

/** @brief Integer level bits used to detect changes during fades. */
#define AKAO_SONG_VOLUME_MASK (AKAO_VOLUME_MAX << AKAO_Q16_SHIFT)

/** @brief Mask retaining the low 16 bits of a value or accumulator. */
#define AKAO_LOW_HALF_MASK 0xFFFF
/** @brief Carry bits that trigger a sequencer tick. */
#define AKAO_TEMPO_CARRY_MASK 0xFFFF0000

/** @brief Tempo scales below this threshold add a Q7 boost; others multiply in Q8. */
#define AKAO_TEMPO_SCALE_THRESHOLD 0x80
#define AKAO_TEMPO_BOOST_SHIFT 7

/** @brief A zero byte encodes a 256-tick fade. */
#define AKAO_FADE_DEFAULT_TICKS 256

/** @brief Countdown selected by a zero noise or pitch-modulation toggle operand. */
#define AKAO_TOGGLE_DEFAULT_TICKS 257

/** @brief Driver ticks between updates of the slow fades. */
#define AKAO_FADE_TICK_INTERVAL 4

/** @brief Index mask for the 256-entry pitch-jitter cycle. */
#define AKAO_PITCH_JITTER_INDEX_MASK 0xFF

/** @brief Align the XA level's sign bit before halving it to an SPU volume. */
#define AKAO_XA_VOLUME_SIGN_SHIFT 15

/** @brief Loop stack wraps across four saved cursor/count entries. */
#define AKAO_LOOP_DEPTH_MASK 3
/** @brief A zero loop-count byte selects 256 iterations. */
#define AKAO_LOOP_DEFAULT_COUNT 256

/** @brief Clear the armed-tie and SFX full-gate flags while retaining a tied note. */
#define AKAO_NOTE_CLEAR_TIE_GATE_MASK 0xFFFA

/** @brief Ties and SFX full-gate notes bypass the normal early key-off. */
#define AKAO_NOTE_TIE_ARMED 0x1
#define AKAO_NOTE_TIED 0x2
#define AKAO_NOTE_SFX_FULL_GATE 0x4
#define AKAO_NOTE_FULL_GATE_MASK (AKAO_NOTE_TIE_ARMED | AKAO_NOTE_SFX_FULL_GATE)
#define AKAO_NOTE_CLEAR_TIED_MASK (AKAO_LOW_HALF_MASK ^ AKAO_NOTE_TIED)

/** @brief A zero period byte selects a 256-tick LFO period. */
#define AKAO_LFO_DEFAULT_PERIOD 256

/** @brief Packed note opcodes have eleven duration choices for each pitch or tie. */
#define AKAO_NOTE_DURATION_COUNT 11
#define AKAO_NOTE_KEY_OFF_LEAD_TICKS 2

/** @brief Maximum duration selected by the signed note-duration adjustment. */
#define AKAO_NOTE_DURATION_MAX 255

/** @brief Pitch jitter uses Q7 for positive offsets and Q9 for negative offsets. */
#define AKAO_PITCH_JITTER_NEGATIVE_FLAG 0x80
#define AKAO_PITCH_JITTER_NEGATIVE_SHIFT 9

/** @brief Number of bytes in a relative branch offset. */
#define AKAO_BRANCH_OFFSET_BYTES 2

/** @brief Position of the high byte in a little-endian 16-bit operand. */
#define AKAO_OPERAND_HIGH_BYTE_SHIFT 8

/** @brief Size of each of the two relative sequence offsets in the SFX opcode. */
#define AKAO_SFX_OFFSET_BYTES 2

/** @brief Number of words copied together before the single-word tail. */
#define AKAO_COPY_BLOCK_WORD_COUNT 4

/** @brief Byte offset from the key-map table base to its map data. */
#define AKAO_KEY_MAP_DATA_OFFSET 0x20
/** @brief Largest encoded map offset accepted by the selection opcode. */
#define AKAO_KEY_MAP_MAX_OFFSET 0x8000

/** @brief Key sentinel used before a mapped articulation has sounded a note. */
#define AKAO_NOTE_KEY_UNSET 0xFF

/** @brief Noise operands use six frequency bits; either high bit selects addition. */
#define AKAO_NOISE_FREQUENCY_MASK 0x3F
#define AKAO_NOISE_RELATIVE_MASK 0xC0

/** @brief Packed fields of the SPU's ADSR low register. */
#define AKAO_ADSR_ATTACK_RATE_MASK 0x7F00
#define AKAO_ADSR_ATTACK_MODE_MASK 0x8000

/** @brief Packed fields of the SPU's ADSR high register. */
#define AKAO_ADSR_SUSTAIN_RATE_MASK 0x1FC0
#define AKAO_ADSR_SUSTAIN_DIRECTION_MASK 0x4000
#define AKAO_ADSR_SUSTAIN_EXPONENTIAL_MASK 0x8000
#define AKAO_ADSR_SUSTAIN_MODE_MASK (AKAO_ADSR_SUSTAIN_DIRECTION_MASK | AKAO_ADSR_SUSTAIN_EXPONENTIAL_MASK)
#define AKAO_ADSR_RELEASE_MODE_MASK 0x0020
#define AKAO_ADSR_RELEASE_RATE_MASK 0x001F

/** @brief Clear sustain mode and release mode, optionally replacing sustain rate too. */
#define AKAO_ADSR_KEEP_SUSTAIN_RATE_MASK (AKAO_LOW_HALF_MASK ^ (AKAO_ADSR_SUSTAIN_MODE_MASK | AKAO_ADSR_RELEASE_MODE_MASK))
#define AKAO_ADSR_REPLACE_SUSTAIN_RATE_MASK (AKAO_ADSR_KEEP_SUSTAIN_RATE_MASK ^ AKAO_ADSR_SUSTAIN_RATE_MASK)

/** @brief Bias from sequence pan operands to the driver's pan index. */
#define AKAO_SEQUENCE_PAN_BIAS 0x40

/** @brief Packed pan and reverb fields of a drum-mode note-table entry. */
#define AKAO_NOTE_SLOT_PAN_MASK 0x7F
#define AKAO_NOTE_SLOT_REVERB 0x80

/** @brief The driver articulation table, viewed as articulation entries. */
#define AKAO_ARTICULATIONS ((AkaoArticulation*)g_akao_articulation_slots)

/** @brief Note classes returned by the bytecode look-ahead. */
typedef enum
{
    AKAO_NOTE_LAST_PITCH = 0x83,
    AKAO_NOTE_FIRST_TIE = 0x84,
    AKAO_NOTE_FIRST_REST = 0x8F,
    AKAO_NOTE_LIMIT = 0x9A
} AkaoNoteClass;

/** @brief Primary opcodes handled specially by the bytecode look-ahead. */
typedef enum
{
    AKAO_OP_RELEASE = 0xA0,
    AKAO_OP_LOOP_END = 0xC9,
    AKAO_OP_REPEAT_LOOP = 0xCA,
    AKAO_OP_RESET_EFFECTS = 0xCB,
    AKAO_OP_NOP_CD = 0xCD,
    AKAO_OP_NOP_D1 = 0xD1,
    AKAO_OP_DISABLE_PORTAMENTO = 0xDB,
    AKAO_OP_LENGTH_NOTE_C = 0xF0,
    AKAO_OP_LENGTH_NOTE_CS = 0xF1,
    AKAO_OP_LENGTH_NOTE_D = 0xF2,
    AKAO_OP_LENGTH_NOTE_DS = 0xF3,
    AKAO_OP_LENGTH_NOTE_E = 0xF4,
    AKAO_OP_LENGTH_NOTE_F = 0xF5,
    AKAO_OP_LENGTH_NOTE_FS = 0xF6,
    AKAO_OP_LENGTH_NOTE_G = 0xF7,
    AKAO_OP_LENGTH_NOTE_GS = 0xF8,
    AKAO_OP_LENGTH_NOTE_A = 0xF9,
    AKAO_OP_LENGTH_NOTE_AS = 0xFA,
    AKAO_OP_LENGTH_NOTE_B = 0xFB,
    AKAO_OP_LENGTH_TIE = 0xFC,
    AKAO_OP_LENGTH_REST = 0xFD,
    AKAO_OP_EXTENDED = 0xFE,
    AKAO_OP_END = 0xFF
} AkaoLookAheadOpcode;

/** @brief Extended control-flow opcodes followed by the bytecode look-ahead. */
typedef enum
{
    AKAO_EXT_JUMP = 0x06,
    AKAO_EXT_COND_JUMP = 0x07,
    AKAO_EXT_BRANCH_ON_LOOP_LAST = 0x08,
    AKAO_EXT_BRANCH_AND_END_LOOP = 0x09,
    AKAO_EXT_CALL = 0x0E,
    AKAO_EXT_RETURN = 0x0F
} AkaoControlFlowOpcode;

/** @brief Play-parameter slots filled when a sequence launches an SFX. */
typedef enum
{
    AKAO_SEQ_SFX_ID = 0,
    AKAO_SEQ_SFX_TAG = 1,
    AKAO_SEQ_SFX_PAN_BIAS = 2,
    AKAO_SEQ_SFX_VOLUME_SCALE = 3
} AkaoSequenceSfxParam;

/** @brief Drum-mode note entry with articulation, envelope overrides, pan and reverb. */
typedef struct
{
    u8 articulation;
    u8 key;
    u8 attack_rate;
    u8 sustain_rate;
    u8 sustain_mode;
    u8 release_rate;
    u8 volume_scale;
    u8 pan_and_reverb;
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

/** @brief Driver tick counter used to schedule slow fades. */
extern u16 g_akao_irq_frame_counter;
/** @brief Sum of g_akao_irq_timing.samples. */
extern s32 g_akao_irq_timing_total;

void akao_seq_step_opcode(AkaoChannelState* channel, s32 channel_bit);
void akao_flush_voice_key_offs(void);

/** @brief 12-entry semitone pitch-ratio table indexed by note % 12 in akao_compute_pitch. */
extern u32 g_akao_pitch_table[];

/** @brief 16-entry table of pitch, volume, and pan LFO waveform streams. */
extern s16* g_akao_lfo_waveforms[];

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
extern u8 g_akao_pitch_jitter_table[];

/** @brief akao_sfx_play parameter slots filled by ext opcode FE 0B: reverb mask, flags, pan, volume. */
extern AkaoCommandParam g_akao_seq_sfx_params[];

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
 * @brief Copy whole 32-bit words, four at a time followed by the remaining words.
 * @param source Word-aligned source buffer.
 * @param destination Word-aligned destination buffer.
 * @param count Number of bytes to copy; a trailing partial word is ignored.
 * @see decomp.me (100%) https://decomp.me/scratch/TxNq3
 */
void akao_copy_bytes(s32* source, s32* destination, u32 count)
{
    count /= sizeof(*source);

    while (count / AKAO_COPY_BLOCK_WORD_COUNT != 0)
    {
        s32 second_word = source[1];
        s32 third_word = source[2];
        s32 fourth_word = source[3];
        s32 first_word = source[0];

        destination[0] = first_word;
        destination[1] = second_word;
        destination[2] = third_word;
        destination[3] = fourth_word;

        source += AKAO_COPY_BLOCK_WORD_COUNT;
        destination += AKAO_COPY_BLOCK_WORD_COUNT;
        count -= AKAO_COPY_BLOCK_WORD_COUNT;
    }

    while (count != 0)
    {
        *destination = *source;
        source++;
        destination++;
        count--;
    }
}

/**
 * @brief Advance CD, XA, master, song and SFX fades once every four driver ticks.
 * @see decomp.me (100%) https://decomp.me/scratch/07M97
 */
void akao_tick_fades(void)
{
    AkaoXaTracker* stream;
    AkaoChannelState* channel;
    s32 voice_volume;
    s32 next_level;
    u32 remaining_channels;
    s32 channel_cursor;

    g_akao_cdvol_tick = (g_akao_cdvol_tick + 1) & AKAO_PITCH_JITTER_INDEX_MASK;
    if (g_akao_cdvol_fade_ticks != 0)
    {
        g_akao_cdvol_fade_ticks--;
        g_akao_cdvol_acc += g_akao_cdvol_step;
        akao_apply_cdvol_to_spu();
    }

    stream = &g_akao_xa_tracker;
    if ((stream->voice_mask != 0) && (stream->volume_fade_ticks != 0))
    {
        stream->volume_fade_ticks--;
        voice_volume = stream->volume;
        next_level = voice_volume + stream->volume_step;
        if ((next_level & AKAO_Q8_LEVEL_MASK) != (stream->volume & AKAO_Q8_LEVEL_MASK))
        {
            if (g_akao_driver_flags.output_mode & AKAO_OUTPUT_MONO)
            {
                voice_volume = (voice_volume * g_akao_pan_gain_table[AKAO_PAN_CENTER]) >> AKAO_Q16_SHIFT;
                spu_set_voice_volume(stream->first_voice, voice_volume, voice_volume, 0);
                spu_set_voice_volume(stream->first_voice + 1, voice_volume, voice_volume, 0);
            }
            else
            {
                /* Halve the level and sign-extend the 16-bit SPU volume. */
                voice_volume = next_level << AKAO_XA_VOLUME_SIGN_SHIFT;
                voice_volume >>= AKAO_Q16_SHIFT;
                spu_set_voice_volume(stream->first_voice, voice_volume, 0, 0);
                spu_set_voice_volume(stream->first_voice + 1, 0, voice_volume, 0);
            }
        }
        g_akao_xa_tracker.volume = next_level & AKAO_LOW_HALF_MASK;
    }

    if (g_akao_masterpan_fade_ticks != 0)
    {
        g_akao_masterpan_fade_ticks--;
        g_akao_masterpan_acc += g_akao_masterpan_step;
    }

    if (g_akao_mastervol_fade_ticks != 0)
    {
        g_akao_mastervol_fade_ticks--;
        next_level = g_akao_mastervol_acc + g_akao_mastervol_step;
        channel_cursor = AKAO_CHANNEL_COUNT;
        if ((next_level & AKAO_MASTER_LEVEL_MASK) != (g_akao_mastervol_acc & AKAO_MASTER_LEVEL_MASK))
        {
            channel = g_akao_seq_channels;
            for (; channel_cursor != 0; channel_cursor--)
            {
                channel->update_flags |= SPU_UPDATE_PITCH;
                channel++;
            }
        }
        g_akao_mastervol_acc = next_level;
    }

    if ((g_akao_seq_channel0->masks.active_mask != 0) && ((s16)g_akao_seq_channel0->volume_fade_ticks != 0))
    {
        g_akao_seq_channel0->volume_fade_ticks--;
        next_level = g_akao_seq_channel0->volume + g_akao_seq_channel0->volume_step;
        if ((next_level & AKAO_SONG_VOLUME_MASK) != (g_akao_seq_channel0->volume & AKAO_SONG_VOLUME_MASK))
        {
            akao_seq_flag_volume_update(g_akao_seq_channel0, g_akao_seq_channels);
        }
        g_akao_seq_channel0->volume = next_level;
    }

    if ((g_akao_seq_channel1 != NULL) && (g_akao_seq_channel1->masks.active_mask != 0) && ((s16)g_akao_seq_channel1->volume_fade_ticks != 0))
    {
        g_akao_seq_channel1->volume_fade_ticks--;
        next_level = g_akao_seq_channel1->volume + g_akao_seq_channel1->volume_step;
        if ((next_level & AKAO_SONG_VOLUME_MASK) != (g_akao_seq_channel1->volume & AKAO_SONG_VOLUME_MASK))
        {
            akao_seq_flag_volume_update(g_akao_seq_channel1, g_akao_pending_channels);
        }
        g_akao_seq_channel1->volume = next_level;
    }

    if (g_akao_sfx_control.active_mask != 0)
    {
        remaining_channels = g_akao_sfx_control.active_mask;
        channel = g_sfx_channels;
        channel_cursor = AKAO_SFX_FIRST_CHANNEL_BIT;
        do
        {
            if (remaining_channels & channel_cursor)
            {
                if (channel->volume_scale_fade_ticks != 0)
                {
                    channel->volume_scale_fade_ticks--;
                    next_level = (s16)channel->volume_scale + (s16)channel->volume_scale_step;
                    if ((next_level & AKAO_Q8_LEVEL_MASK) != ((s16)channel->volume_scale & AKAO_Q8_LEVEL_MASK))
                    {
                        channel->update_flags |= SPU_UPDATE_VOLUME;
                    }
                    channel->volume_scale = next_level;
                }
                if (channel->pan_bias_fade_ticks != 0)
                {
                    channel->pan_bias_fade_ticks--;
                    next_level = channel->pan_bias + channel->pan_bias_step;
                    if ((next_level & AKAO_Q8_LEVEL_MASK) != (channel->pan_bias & AKAO_Q8_LEVEL_MASK))
                    {
                        channel->update_flags |= SPU_UPDATE_VOLUME;
                    }
                    channel->pan_bias = next_level;
                }
                if (channel->sfx_pitch_bend_fade_ticks != 0)
                {
                    channel->sfx_pitch_bend_fade_ticks--;
                    next_level = channel->sfx_pitch_bend + channel->sfx_pitch_bend_step;
                    if ((next_level & AKAO_Q8_LEVEL_MASK) != (channel->sfx_pitch_bend & AKAO_Q8_LEVEL_MASK))
                    {
                        channel->update_flags |= SPU_UPDATE_PITCH;
                    }
                    channel->sfx_pitch_bend = next_level;
                }
                remaining_channels ^= channel_cursor;
            }
            channel_cursor <<= 1;
            channel++;
        } while (remaining_channels != 0);
    }
}

/**
 * @brief Advance a song's tempo, channel timers, fades and musical position.
 * @param channels Channel table of the song selected by g_akao_seq_channel0.
 * @param is_secondary Nonzero for the secondary song, which skips primary driver updates and catch-up.
 * @return The song's active-channel mask.
 * @see decomp.me (100%) https://decomp.me/scratch/XMCUh
 */
s32 akao_seq_tick_channels(AkaoChannelState* channels, s32 is_secondary)
{
    u32 tempo_step;
    u32 tempo_scale;
    s32 channel_bit;
    AkaoChannelState* channel;
    s32 remaining_channels;
    AkaoDriverFlags* driver_flags;

    tempo_step = g_akao_seq_channel0->tempo >> AKAO_Q16_SHIFT;
    tempo_scale = g_akao_master_vol_scalar;

    if (tempo_scale != 0)
    {
        if (tempo_scale < AKAO_TEMPO_SCALE_THRESHOLD)
        {
            tempo_step += (tempo_step * tempo_scale) >> AKAO_TEMPO_BOOST_SHIFT;
        }
        else
        {
            tempo_step = (tempo_step * tempo_scale) >> AKAO_Q8_SHIFT;
        }
    }

    g_akao_seq_channel0->tempo_acc += tempo_step;

    if ((g_akao_seq_channel0->tempo_acc & AKAO_TEMPO_CARRY_MASK) || (g_akao_driver_mode_flags & AKAO_MODE_FORCE_TICK))
    {
        g_akao_seq_channel0->tempo_acc &= AKAO_LOW_HALF_MASK;

        driver_flags = &g_akao_driver_flags;
        channel = channels;

        /* The primary song can catch up through several measures in this pass. */
        do
        {
            channel_bit = 1;
            remaining_channels = g_akao_seq_channel0->masks.active_mask;

            do
            {
                if (remaining_channels & channel_bit)
                {
                    channel->note_ticks--;
                    channel->gate_ticks--;

                    if (channel->note_ticks == 0)
                    {
                        akao_seq_step_opcode(channel, channel_bit);
                    }
                    else if (channel->gate_ticks == 0)
                    {
                        g_akao_seq_channel0->key_off_mask |= channel_bit;
                    }

                    akao_tick_channel_effects(channel, channel_bit, 0);
                    remaining_channels &= ~channel_bit;
                }

                channel++;
                channel_bit <<= 1;
            } while (remaining_channels != 0);

            if (g_akao_seq_channel0->tempo_fade_ticks != 0)
            {
                g_akao_seq_channel0->tempo_fade_ticks--;
                g_akao_seq_channel0->tempo += g_akao_seq_channel0->tempo_step;
            }

            if (g_akao_seq_channel0->reverb_depth_fade_ticks != 0)
            {
                u32 pending_updates;

                g_akao_seq_channel0->reverb_depth_fade_ticks--;
                g_akao_seq_channel0->reverb_depth += g_akao_seq_channel0->reverb_depth_step;
                do
                {
                    pending_updates = driver_flags->update_flags;
                } while (0);
                if (is_secondary == 0)
                {
                    driver_flags->update_flags = pending_updates | AKAO_REVERB_DEPTH_UPDATE_PENDING;
                }
            }

            if (g_akao_seq_channel0->ticks_per_beat != 0)
            {
                g_akao_seq_channel0->tick++;
                if (g_akao_seq_channel0->tick == g_akao_seq_channel0->ticks_per_beat)
                {
                    g_akao_seq_channel0->tick = 0;
                    g_akao_seq_channel0->beat++;
                    if (g_akao_seq_channel0->beat == g_akao_seq_channel0->beats_per_measure)
                    {
                        g_akao_seq_channel0->beat = 0;
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

    return g_akao_seq_channel0->masks.active_mask;
}

/**
 * @brief Service pending SPU updates and advance songs, SFX and fades on counter 2's IRQ.
 *
 * The secondary song becomes primary once the primary has no active or parked channels.
 * Execution time is recorded in the four-sample timing history.
 * @return Root-counter ticks elapsed during this callback.
 * @see decomp.me (100%) https://decomp.me/scratch/ICO2k
 */
long akao_irq_handler(void)
{
    s32 counter_ticks;
    s32 remaining_channels;
    AkaoChannelState* channel;
    u32 channel_bit;

    counter_ticks = GetRCnt(RCntCNT2);

    g_akao_irq_frame_counter += 1;
    if ((g_akao_seq_channel0->key_off_mask != 0) || (g_akao_sfx_control.key_off_mask != 0) ||
        ((g_akao_seq_channel1 != NULL) && (g_akao_seq_channel1->key_off_mask != 0)))
    {
        akao_flush_voice_key_offs();
    }

    if (g_akao_seq_channel1 != NULL)
    {
        AkaoSongState* secondary_song = g_akao_seq_channel1;

        if (secondary_song->masks.active_mask == 0)
        {
            g_akao_seq_channel1 = NULL;
        }
        else if ((g_akao_seq_channel0->masks.active_mask | g_akao_seq_channel0->parked_mask) == 0)
        {
            akao_copy_bytes((s32*)secondary_song, (s32*)g_akao_seq_channel0, sizeof(*g_akao_seq_channel0));
            akao_copy_bytes((s32*)g_akao_pending_channels, (s32*)g_akao_seq_channels, AKAO_CHANNEL_COUNT * sizeof(*g_akao_seq_channels));
            {
                AkaoSongState* copied_song = g_akao_seq_channel1;

                g_akao_seq_channel1 = NULL;
                copied_song->song_id = 0;
                copied_song->masks.active_mask = 0;
            }
        }
    }

    if (((g_akao_driver_flags.update_flags | g_akao_seq_channel0->note_on_mask | g_akao_sfx_control.note_on_mask) != 0) ||
        ((g_akao_seq_channel1 != NULL) && (g_akao_seq_channel1->note_on_mask != 0)))
    {
        akao_flush_voice_updates(g_akao_sfx_control.note_on_mask);
    }

    if (g_akao_seq_channel0->masks.active_mask != 0)
    {
        akao_seq_tick_channels(g_akao_seq_channels, 0);
    }

    if ((g_akao_seq_channel1 != NULL) && (g_akao_seq_channel1->masks.active_mask != 0))
    {
        g_akao_seq_channel0 = g_akao_seq_channel1;
        akao_seq_tick_channels(g_akao_pending_channels, 1);
        g_akao_seq_channel0 = &g_akao_seq_master_state;
    }

    if (g_akao_sfx_control.active_mask != 0)
    {
        u32 tempo_acc;

        remaining_channels = g_akao_sfx_control.active_mask;
        tempo_acc = g_akao_sfx_control.tempo_acc + HALF_HIGH_U16(g_akao_sfx_control.tempo);
        g_akao_sfx_control.tempo_acc = tempo_acc;
        if (((tempo_acc & AKAO_TEMPO_CARRY_MASK) != 0) || (g_akao_driver_mode_flags & AKAO_MODE_FORCE_TICK))
        {
            g_akao_sfx_control.tempo_acc = tempo_acc & AKAO_LOW_HALF_MASK;

            channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
            channel = g_sfx_channels;

            do
            {
                if (remaining_channels & channel_bit)
                {
                    if (!(g_akao_driver_mode_flags & AKAO_MODE_SFX_PAUSED) || (channel->sfx_tag & AKAO_SFX_FLAG_SUPPRESS))
                    {
                        channel->sfx_age++;

                        channel->note_ticks--;
                        channel->gate_ticks--;

                        if (channel->note_ticks == 0)
                        {
                            akao_seq_step_opcode(channel, channel_bit);
                        }
                        else if (channel->gate_ticks == 0)
                        {
                            g_akao_sfx_control.key_off_mask |= channel_bit;
                            g_akao_sfx_control.note_on_mask &= ~channel_bit;
                        }
                        akao_tick_channel_effects(channel, channel_bit, 1);
                    }
                    remaining_channels ^= channel_bit;
                }
                channel++;
                channel_bit <<= 1;
            } while (remaining_channels != 0);
        }
    }

    if (!(g_akao_irq_frame_counter & (AKAO_FADE_TICK_INTERVAL - 1)))
    {
        akao_tick_fades();
    }

    counter_ticks = GetRCnt(RCntCNT2) - counter_ticks;
    if (counter_ticks <= 0)
    {
        counter_ticks += AKAO_TICK_PERIOD;
    }

    g_akao_irq_timing.samples[0] = g_akao_irq_timing.samples[1];
    g_akao_irq_timing.samples[1] = g_akao_irq_timing.samples[2];
    g_akao_irq_timing.samples[2] = g_akao_irq_timing.samples[3];
    g_akao_irq_timing.samples[3] = counter_ticks;
    g_akao_irq_timing_total = g_akao_irq_timing.samples[0] + g_akao_irq_timing.samples[1] + g_akao_irq_timing.samples[2] + g_akao_irq_timing.samples[3];
    return counter_ticks;
}

/**
 * @brief Look ahead through channel bytecode to classify the next note, tie, rest or release.
 * @param channel Channel to inspect; only its armed-tie and SFX full-gate flags may be cleared.
 * @return The next note opcode, a representative opcode for a length-prefixed note, or AKAO_OP_RELEASE.
 * @note The scan follows branches and loop cursors without advancing the channel's own cursor or loop state.
 * @see decomp.me (100%) https://decomp.me/scratch/52mKD
 */
u8 akao_seq_skip_to_next_note(AkaoChannelState* channel)
{
    u8* cursor = channel->seq_cursor;
    u32 loop_depth = channel->loop_depth;
    u8 opcode;
    u8 encoded_length;
    s32 branch_offset;

    while (1)
    {
        opcode = *cursor;
        if (opcode < AKAO_NOTE_LIMIT)
        {
            if (opcode >= AKAO_NOTE_FIRST_REST)
            {
                channel->note_flags &= AKAO_NOTE_CLEAR_TIE_GATE_MASK;
            }
            return *cursor;
        }
        if (opcode < AKAO_OP_RELEASE)
        {
            return AKAO_OP_RELEASE;
        }
        encoded_length = g_akao_opcode_len_table[*cursor - AKAO_OP_RELEASE];
        if (encoded_length != 0)
        {
            cursor += encoded_length;
            continue;
        }
        switch (*cursor)
        {
        case AKAO_OP_LENGTH_NOTE_C:
        case AKAO_OP_LENGTH_NOTE_CS:
        case AKAO_OP_LENGTH_NOTE_D:
        case AKAO_OP_LENGTH_NOTE_DS:
        case AKAO_OP_LENGTH_NOTE_E:
        case AKAO_OP_LENGTH_NOTE_F:
        case AKAO_OP_LENGTH_NOTE_FS:
        case AKAO_OP_LENGTH_NOTE_G:
        case AKAO_OP_LENGTH_NOTE_GS:
        case AKAO_OP_LENGTH_NOTE_A:
        case AKAO_OP_LENGTH_NOTE_AS:
        case AKAO_OP_LENGTH_NOTE_B:
            return AKAO_NOTE_LAST_PITCH;
        case AKAO_OP_LENGTH_TIE:
            return AKAO_NOTE_FIRST_TIE;
        case AKAO_OP_LENGTH_REST:
            return AKAO_NOTE_FIRST_REST;
        case AKAO_OP_EXTENDED:
            cursor++;
            opcode = *cursor;
            encoded_length = g_akao_opcode_len_table_ext[opcode];
            if (encoded_length != 0)
            {
                cursor += encoded_length;
                continue;
            }
            switch (opcode)
            {
            case AKAO_EXT_BRANCH_ON_LOOP_LAST:
            case AKAO_EXT_BRANCH_AND_END_LOOP:
                cursor++;
                if (*cursor == channel->loop_count[loop_depth] + 1)
                {
                    cursor++;
                    loop_depth--;
                    loop_depth &= AKAO_LOOP_DEPTH_MASK;
                    branch_offset = cursor[0];
                    branch_offset += cursor[1] << 8;
                    cursor += (s16)branch_offset;
                }
                else
                {
                    cursor += 1 + AKAO_BRANCH_OFFSET_BYTES;
                }
                continue;
            case AKAO_EXT_JUMP:
            case AKAO_EXT_CALL:
                cursor++;
                branch_offset = cursor[0];
                branch_offset += cursor[1] << 8;
                cursor += (s16)branch_offset;
                continue;
            case AKAO_EXT_COND_JUMP:
                cursor++;
                if (g_akao_seq_channel0->condition >= *cursor++)
                {
                    branch_offset = cursor[0];
                    branch_offset += cursor[1] << 8;
                    cursor += (s16)branch_offset;
                }
                else
                {
                    cursor += AKAO_BRANCH_OFFSET_BYTES;
                }
                continue;
            case AKAO_EXT_RETURN:
                cursor = channel->return_cursor;
                continue;
            default:
                continue;
            }
        case AKAO_OP_LOOP_END:
            cursor++;
            if (*cursor == channel->loop_count[loop_depth] + 1)
            {
                cursor++;
                loop_depth--;
                loop_depth &= AKAO_LOOP_DEPTH_MASK;
            }
            else
            {
                cursor = channel->loop_cursor[loop_depth];
            }
            continue;
        case AKAO_OP_RESET_EFFECTS:
        case AKAO_OP_NOP_CD:
        case AKAO_OP_NOP_D1:
        case AKAO_OP_DISABLE_PORTAMENTO:
            channel->note_flags &= AKAO_NOTE_CLEAR_TIE_GATE_MASK;
            cursor++;
            continue;
        case AKAO_OP_REPEAT_LOOP:
            if (!(channel->flags & AKAO_CH_STOP_PENDING))
            {
                cursor = channel->loop_cursor[loop_depth];
                continue;
            }
            break;
        }
        channel->note_flags &= AKAO_NOTE_CLEAR_TIE_GATE_MASK;
        return AKAO_OP_RELEASE;
    }
}

/**
 * @brief Bind a key-map articulation and load its sample addresses and ADSR settings.
 * @param channel Channel with the key map and current note key.
 * @param key Key used to select an articulation range.
 * @param next_opcode Unused next-note opcode supplied by the caller.
 * @see decomp.me (100%) https://decomp.me/scratch/0tdrk
 */
void akao_bind_articulation_for_key(AkaoChannelState* channel, u32 key, s32 next_opcode)
{
    AkaoKeyMapEntry* entry;
    AkaoArticulation* articulation_data;
    u8 articulation_index;
    u32 channel_flags;

    if (((s16)channel->note_key < key) || ((s16)channel->note_key == AKAO_NOTE_KEY_UNSET))
    {
        entry = (AkaoKeyMapEntry*)channel->key_map;
        while ((entry[1].sustain_mode != 0) && (entry->key_high < key))
        {
            entry++;
        }
    }
    else if (key < (s16)channel->note_key)
    {
        entry = (AkaoKeyMapEntry*)channel->key_map;
        while ((entry[1].sustain_mode != 0) && (key >= entry[1].key_low))
        {
            entry++;
        }
    }
    else
    {
        return;
    }

    channel_flags = channel->flags;
    articulation_index = entry->articulation;
    articulation_data = &AKAO_ARTICULATIONS[articulation_index];
    channel->articulation = articulation_index;

    channel->spu_sample_addr = articulation_data->sample_addr;
    channel->spu_loop_addr = articulation_data->loop_addr;

    if (!(channel_flags & AKAO_CH_ADSR_ATTACK))
    {
        channel->spu_adsr_low = entry->attack_rate << SPU_ADSR_ATTACK_RATE_SHIFT;
    }
    else
    {
        channel->spu_adsr_low &= AKAO_ADSR_ATTACK_RATE_MASK;
    }

    channel->spu_adsr_low |= articulation_data->pitch_misc.half.lo & (AKAO_ADSR_ATTACK_MODE_MASK | SPU_ADSR_DECAY_SUSTAIN_MASK);

    if (!(channel_flags & AKAO_CH_ADSR_SUSTAIN_RATE))
    {
        channel->spu_adsr_high &= AKAO_ADSR_REPLACE_SUSTAIN_RATE_MASK;
        channel->spu_adsr_high |= entry->sustain_rate << SPU_ADSR_SUSTAIN_RATE_SHIFT;
    }
    else
    {
        channel->spu_adsr_high &= AKAO_ADSR_KEEP_SUSTAIN_RATE_MASK;
    }

    switch (entry->sustain_mode)
    {
    case SPU_VOICE_LINEARDecN:
        channel->spu_adsr_high |= AKAO_ADSR_SUSTAIN_DIRECTION_MASK;
        break;
    case SPU_VOICE_EXPIncN:
        channel->spu_adsr_high |= AKAO_ADSR_SUSTAIN_EXPONENTIAL_MASK;
        break;
    case SPU_VOICE_EXPDec:
        channel->spu_adsr_high |= AKAO_ADSR_SUSTAIN_MODE_MASK;
        break;
    }

    if (!(channel_flags & AKAO_CH_ADSR_RELEASE_RATE))
    {
        channel->spu_adsr_high &= (AKAO_LOW_HALF_MASK ^ AKAO_ADSR_RELEASE_RATE_MASK);
        channel->spu_adsr_high |= entry->release_rate;
    }

    channel->spu_adsr_high |= articulation_data->pitch_misc.half.hi & AKAO_ADSR_RELEASE_MODE_MASK;
    channel->spu_volume_scale = entry->volume_scale;
}

/**
 * @brief Compute a note's SPU pitch and detune delta relative to an articulation's base key.
 * @param articulation Source of the base key and fine-tuning factor.
 * @param note Note to sound, in semitones.
 * @param detune Encoded pitch adjustment in the low byte.
 * @param detune_pitch_delta Pitch delta updated and rescaled alongside the base pitch.
 * @return SPU pitch truncated to 16 bits.
 * @note Zero detune skips delta initialization; the caller must supply its starting value.
 * @see decomp.me (100%) https://decomp.me/scratch/mWTad
 */
s32 akao_compute_pitch(AkaoArticulation* articulation, s32 note, s32 detune, s32* detune_pitch_delta)
{
    s32 base_pitch;
    s32 pitch_interval;
    u32 detune_scale;
    u32 pitch;
    u32 detune_delta;
    s32 spu_pitch;

    pitch_interval = note - articulation->adsr.half.hi;
    while (pitch_interval < 0)
    {
        pitch_interval += AKAO_SEMITONES_PER_OCTAVE;
    }
    pitch_interval %= AKAO_SEMITONES_PER_OCTAVE;

    if (articulation->adsr.half.lo == 0)
    {
        s32 table_pitch = g_akao_pitch_table[pitch_interval];

        pitch = table_pitch << AKAO_Q8_SHIFT;
    }
    else if (articulation->adsr.half.lo < 0)
    {
        pitch = (g_akao_pitch_table[pitch_interval] * (u16)articulation->adsr.half.lo) >> AKAO_Q8_SHIFT;
    }
    else
    {
        base_pitch = g_akao_pitch_table[pitch_interval];
        pitch = (u32)(base_pitch * articulation->adsr.half.lo) >> AKAO_Q7_SHIFT;
        pitch += base_pitch << AKAO_Q8_SHIFT;
    }

    detune_scale = detune & AKAO_DETUNE_VALUE_MASK;
    if (detune_scale != 0)
    {
        if (detune_scale < AKAO_DETUNE_NEGATIVE_THRESHOLD)
        {
            detune_delta = (pitch * detune_scale) >> AKAO_Q7_SHIFT;
        }
        else
        {
            detune_delta = ((pitch * detune_scale) >> AKAO_Q8_SHIFT) - pitch;
        }
        *detune_pitch_delta = detune_delta;
    }

    /* Shift pitch and detune together to the requested octave. */
    if (note < articulation->adsr.half.hi)
    {
        do
        {
            *detune_pitch_delta >>= 1;
            pitch = (s32)pitch >> 1;
            note += AKAO_SEMITONES_PER_OCTAVE;
        } while (note < articulation->adsr.half.hi);
    }
    else
    {
        pitch_interval = (note - articulation->adsr.half.hi) / AKAO_SEMITONES_PER_OCTAVE;
        if (pitch_interval != 0)
        {
            pitch <<= pitch_interval;
            *detune_pitch_delta <<= pitch_interval;
        }
    }

    pitch = (s32)pitch >> AKAO_Q8_SHIFT;
    spu_pitch = pitch & AKAO_LOW_HALF_MASK;
    *detune_pitch_delta >>= AKAO_Q8_SHIFT;
    return spu_pitch;
}

/**
 * @brief Prepare a drum-mode note from the song's note/articulation table.
 * @param channel Channel to receive the sample addresses, envelope, volume scale and pan.
 * @param channel_bit Channel bit used to request key-on, key-off and reverb updates.
 * @param slot_index Entry to select from g_akao_seq_channel0->note_table.
 * @return SPU pitch calculated for the entry's key.
 * @see decomp.me (100%) https://decomp.me/scratch/9dRLX
 */
s32 akao_channel_start_note(AkaoChannelState* channel, s32 channel_bit, s32 slot_index)
{
    AkaoNoteArticulationSlot* slot;
    AkaoArticulation* articulation_data;
    u32 channel_flags;
    u32 sounding_channel_mask;
    u32 articulation_index;
    s32 pitch;
    u32 key_on_mask;

    slot = (AkaoNoteArticulationSlot*)g_akao_seq_channel0->note_table;
    key_on_mask = g_akao_seq_channel0->masks.key_on_mask;
    sounding_channel_mask = g_akao_seq_channel0->note_on_mask;
    slot += slot_index;
    key_on_mask |= channel_bit;
    sounding_channel_mask &= channel_bit;
    g_akao_seq_channel0->masks.key_on_mask = key_on_mask;
    if (sounding_channel_mask)
    {
        g_akao_seq_channel0->key_off_mask |= channel_bit;
    }
    articulation_index = slot->articulation;
    channel_flags = channel->flags;
    channel->articulation = articulation_index;
    articulation_data = &AKAO_ARTICULATIONS[articulation_index];
    channel->spu_sample_addr = articulation_data->sample_addr;
    channel->spu_loop_addr = articulation_data->loop_addr;
    if (!(channel_flags & AKAO_CH_ADSR_ATTACK))
    {
        channel->spu_adsr_low = slot->attack_rate << SPU_ADSR_ATTACK_RATE_SHIFT;
    }
    else
    {
        channel->spu_adsr_low &= AKAO_ADSR_ATTACK_RATE_MASK;
    }
    channel->spu_adsr_low |= articulation_data->pitch_misc.half.lo & (AKAO_ADSR_ATTACK_MODE_MASK | SPU_ADSR_DECAY_SUSTAIN_MASK);
    if (!(channel_flags & AKAO_CH_ADSR_SUSTAIN_RATE))
    {
        channel->spu_adsr_high &= AKAO_ADSR_REPLACE_SUSTAIN_RATE_MASK;
        channel->spu_adsr_high |= slot->sustain_rate << SPU_ADSR_SUSTAIN_RATE_SHIFT;
    }
    else
    {
        channel->spu_adsr_high &= AKAO_ADSR_KEEP_SUSTAIN_RATE_MASK;
    }
    switch (slot->sustain_mode)
    {
    case SPU_VOICE_LINEARDecN:
        channel->spu_adsr_high |= AKAO_ADSR_SUSTAIN_DIRECTION_MASK;
        break;
    case SPU_VOICE_EXPIncN:
        channel->spu_adsr_high |= AKAO_ADSR_SUSTAIN_EXPONENTIAL_MASK;
        break;
    case SPU_VOICE_EXPDec:
        channel->spu_adsr_high |= AKAO_ADSR_SUSTAIN_MODE_MASK;
        break;
    }
    if (!(channel_flags & AKAO_CH_ADSR_RELEASE_RATE))
    {
        channel->spu_adsr_high &= (AKAO_LOW_HALF_MASK ^ AKAO_ADSR_RELEASE_RATE_MASK);
        channel->spu_adsr_high |= slot->release_rate;
    }
    channel->spu_adsr_high |= articulation_data->pitch_misc.half.hi & AKAO_ADSR_RELEASE_MODE_MASK;
    pitch = akao_compute_pitch(articulation_data, slot->key, channel->detune, &channel->detune_pitch_delta);
    channel->spu_volume_scale = slot->volume_scale;
    channel->pan = ((slot->pan_and_reverb & AKAO_NOTE_SLOT_PAN_MASK) + AKAO_SEQUENCE_PAN_BIAS) << AKAO_Q8_SHIFT;
    if (slot->pan_and_reverb & AKAO_NOTE_SLOT_REVERB)
    {
        g_akao_seq_channel0->reverb_mask |= channel_bit;
    }
    else
    {
        g_akao_seq_channel0->reverb_mask &= ~channel_bit;
    }
    g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
    return pitch;
}

/**
 * @brief Execute channel commands through the next note, tie, rest or release.
 * @param channel Channel whose cursor, timers, pitch and note effects are advanced.
 * @param channel_bit Bit of the channel in the song or SFX masks.
 * @see decomp.me (100%) https://decomp.me/scratch/P4H6n
 */
void akao_seq_step_opcode(AkaoChannelState* channel, s32 channel_bit)
{
    u32 pitch_delta;
    s16 slide_key;
    s32 target_key;
    s32 channel_flags;
    u16 expression_fade_ticks;
    s32 key;
    u16 packed_lfo_depth;
    u16 note_flags;
    u16 duration_override;
    u16 portamento_ticks;
    u16 duration;
    s32 note_value;
    u32 absolute_lfo_depth;
    u32 lfo_depth_level;
    u32 opcode;

    do
    {
        opcode = *channel->seq_cursor++;

        if (opcode >= AKAO_OP_RELEASE)
        {
            if (opcode == AKAO_OP_EXTENDED)
            {
                note_value = *channel->seq_cursor++;
                g_akao_opcode_handlers_ext[note_value](channel, channel_bit);
            }
            else if ((opcode >= AKAO_OP_LENGTH_NOTE_C) && (opcode < AKAO_OP_EXTENDED))
            {
                opcode = (opcode - AKAO_OP_LENGTH_NOTE_C) * AKAO_NOTE_DURATION_COUNT;
                channel->note_ticks = *channel->seq_cursor++;
            }
            else
            {
                if (opcode == AKAO_OP_END)
                {
                    opcode = AKAO_OP_RELEASE;
                }
                else if ((opcode == AKAO_OP_REPEAT_LOOP) && (channel->flags & AKAO_CH_STOP_PENDING))
                {
                    opcode = AKAO_OP_RELEASE;
                    g_akao_sfx_control.key_off_mask |= channel_bit;
                }
                g_akao_opcode_handlers[opcode - AKAO_OP_RELEASE](channel, channel_bit);
            }
        }
        channel->opcode_count++;
    } while (opcode > AKAO_OP_RELEASE);

    if (opcode == AKAO_OP_RELEASE)
    {
        if (channel->is_sfx_channel == 0)
        {
            g_akao_seq_channel0->key_off_mask |= channel_bit;
        }
    }
    else
    {
        note_value = akao_seq_skip_to_next_note(channel) & 0xFF;
        duration_override = channel->note_duration_adjust;
        if ((s16)channel->note_duration_adjust != 0)
        {
            channel->gate_ticks = duration_override;
            channel->note_ticks = duration_override;
        }
        if (channel->note_ticks != 0)
        {
            if ((note_value >= (u32)AKAO_NOTE_FIRST_REST) || ((note_value < (u32)AKAO_NOTE_FIRST_TIE) && !(channel->note_flags & AKAO_NOTE_FULL_GATE_MASK)))
            {
                channel->gate_ticks -= AKAO_NOTE_KEY_OFF_LEAD_TICKS;
            }
        }
        else
        {
            duration = channel->note_ticks = g_akao_note_duration_table[opcode % AKAO_NOTE_DURATION_COUNT];
            if (((note_value < AKAO_NOTE_FIRST_TIE) || (note_value >= AKAO_NOTE_FIRST_REST)) && !(channel->note_flags & AKAO_NOTE_FULL_GATE_MASK))
            {
                duration -= AKAO_NOTE_KEY_OFF_LEAD_TICKS;
            }
            channel->gate_ticks = duration;
        }
        if ((channel->is_sfx_channel == 0) && (channel->flags & AKAO_CH_FULL_GATE))
        {
            channel->gate_ticks = channel->note_ticks;
        }
        channel->note_duration = channel->note_ticks;
        channel->update_flags |= SPU_VOICE_ADSR_RR;
        if (opcode >= AKAO_NOTE_FIRST_REST)
        {
            if (channel->is_sfx_channel == 0)
            {
                g_akao_seq_channel0->note_on_mask &= ~channel_bit;
                if (channel->voice < AKAO_VOICE_COUNT)
                {
                    g_akao_seq_channel0->key_off_mask |= channel_bit;
                }
            }
            channel->portamento_speed = 0;
            channel->pitch_lfo_value = 0;
            channel->volume_lfo_value = 0;
            channel->note_flags &= AKAO_NOTE_CLEAR_TIED_MASK;
            return;
        }
        if (opcode < AKAO_NOTE_FIRST_TIE)
        {
            channel_flags = channel->flags;
            key = opcode / AKAO_NOTE_DURATION_COUNT;
            key += channel->octave * AKAO_SEMITONES_PER_OCTAVE;
            if (channel_flags & AKAO_CH_DRUM_MODE)
            {
                note_value = akao_channel_start_note(channel, channel_bit, key);
            }
            else
            {
                if (!(channel->note_flags & AKAO_NOTE_TIED))
                {
                    if (channel->is_sfx_channel == 0)
                    {
                        if (channel_flags & AKAO_CH_KEY_MAP)
                        {
                            akao_bind_articulation_for_key(channel, key, note_value);
                        }
                        g_akao_seq_channel0->masks.key_on_mask |= channel_bit;
                        if ((g_akao_seq_channel0->note_on_mask & channel_bit) && (channel->voice < AKAO_VOICE_COUNT))
                        {
                            g_akao_seq_channel0->key_off_mask |= channel_bit;
                        }
                        expression_fade_ticks = channel->note_expression_ticks;
                        if (expression_fade_ticks != 0)
                        {
                            channel->expression_fade_ticks = expression_fade_ticks;
                            channel->expression = channel->expression_preset;
                            channel->expression_step = channel->expression_preset_step;
                        }
                    }
                    else
                    {
                        g_akao_sfx_control.key_on_mask |= channel_bit;
                    }
                    channel->pitch_slide_ticks = 0;
                }
                portamento_ticks = channel->portamento_speed;
                if ((portamento_ticks != 0) && (channel->prev_key != 0))
                {
                    target_key = channel->transpose + key;
                    key = channel->prev_key + channel->prev_transpose;
                    channel->pitch_slide_duration = portamento_ticks;
                    channel->pitch_slide_delta = target_key - channel->prev_key - channel->prev_transpose;
                    channel->note_key = channel->prev_key - (channel->transpose - channel->prev_transpose);
                }
                else
                {
                    channel->note_key = key;
                    key += (s16)channel->transpose;
                }
                note_value = akao_compute_pitch(&AKAO_ARTICULATIONS[channel->articulation], key, channel->detune, &channel->detune_pitch_delta);
                if (channel->pitch_scale != 0)
                {
                    pitch_delta = (u32)(note_value * channel->pitch_scale) >> AKAO_Q8_SHIFT;
                    pitch_delta *= g_akao_pitch_jitter_table[g_akao_cdvol_tick];
                    if (g_akao_pitch_jitter_table[g_akao_cdvol_tick] & AKAO_PITCH_JITTER_NEGATIVE_FLAG)
                    {
                        pitch_delta >>= AKAO_PITCH_JITTER_NEGATIVE_SHIFT;
                        note_value -= pitch_delta;
                    }
                    else
                    {
                        pitch_delta >>= AKAO_Q7_SHIFT;
                        note_value += pitch_delta;
                    }
                }
            }
            channel->pitch = note_value;
            if (channel->is_sfx_channel == 0)
            {
                g_akao_seq_channel0->note_on_mask |= channel_bit;
            }
            else
            {
                g_akao_sfx_control.note_on_mask |= channel_bit;
            }
            channel->update_flags |= (SPU_UPDATE_PITCH | SPU_UPDATE_VOLUME);
            opcode = channel->flags;
            if (opcode & AKAO_CH_PITCH_LFO)
            {
                u16 scaled_lfo_depth;

                packed_lfo_depth = channel->pitch_lfo_depth.raw;
                lfo_depth_level = packed_lfo_depth & AKAO_PITCH_LFO_DEPTH_MASK;
                absolute_lfo_depth = packed_lfo_depth & AKAO_PITCH_LFO_ABSOLUTE_FLAG;
                lfo_depth_level >>= AKAO_Q8_SHIFT;
                if (!absolute_lfo_depth)
                {
                    scaled_lfo_depth = (lfo_depth_level * ((u32)(note_value * AKAO_PITCH_LFO_RELATIVE_SCALE) >> AKAO_Q8_SHIFT)) >> AKAO_Q7_SHIFT;
                }
                else
                {
                    scaled_lfo_depth = (lfo_depth_level * note_value) >> AKAO_Q7_SHIFT;
                }
                channel->pitch_lfo_depth.scaled = scaled_lfo_depth;
                if (!(channel->note_flags & AKAO_NOTE_TIED))
                {
                    channel->pitch_lfo_cursor = g_akao_lfo_waveforms[channel->pitch_lfo_waveform];
                    channel->pitch_lfo_delay_ticks = channel->pitch_lfo_delay;
                    channel->pitch_lfo_restart = 1;
                }
            }
            if ((opcode & AKAO_CH_VOLUME_LFO) && !(channel->note_flags & AKAO_NOTE_TIED))
            {
                channel->volume_lfo_cursor = g_akao_lfo_waveforms[channel->volume_lfo_waveform];
                channel->volume_lfo_delay_ticks = channel->volume_lfo_delay;
                channel->volume_lfo_restart = 1;
            }
            channel->pitch_lfo_value = 0;
            channel->volume_lfo_value = 0;
            channel->pitch_slide_acc = 0;
        }
        /* Carry the armed tie into the next note's active-tie flag. */
        note_flags = channel->note_flags;
        channel->note_flags = (note_flags & AKAO_NOTE_CLEAR_TIED_MASK) | ((note_flags & AKAO_NOTE_TIE_ARMED) << 1);
        if ((s16)channel->pitch_slide_delta != 0)
        {
            slide_key = channel->note_key + channel->pitch_slide_delta;
            channel->note_key = slide_key;
            note_value =
                akao_compute_pitch(&AKAO_ARTICULATIONS[channel->articulation], slide_key + (s16)channel->transpose, channel->detune, (s32*)&pitch_delta)
                << AKAO_Q16_SHIFT;
            channel->pitch_slide_ticks = channel->pitch_slide_duration;
            channel->pitch_slide_delta = 0;
            channel->pitch_slide_step = (note_value - ((channel->pitch << AKAO_Q16_SHIFT) + channel->pitch_slide_acc)) / channel->pitch_slide_ticks;
        }
        channel->prev_key = channel->note_key;
        channel->prev_transpose = channel->transpose;
    }
}

/**
 * @brief Copy an articulation's SPU addresses and ADSR words into a channel.
 * @param channel Channel to update.
 * @param articulation Articulation entry to load.
 * @param sample_addr SPU sample start address to install.
 * @see decomp.me (100%) https://decomp.me/scratch/Fr8N0
 */
void akao_channel_load_articulation_fields(AkaoChannelState* channel, AkaoArticulation* articulation, s32 sample_addr)
{
    u16 adsr_low;
    u16 adsr_high;
    s32 loop_addr;
    s32 pending_updates;

    channel->spu_sample_addr = sample_addr;
    loop_addr = articulation->loop_addr;
    channel->spu_loop_addr = loop_addr;

    adsr_low = articulation->pitch_misc.half.lo;
    channel->spu_adsr_low = adsr_low;

    pending_updates = channel->update_flags;
    adsr_high = articulation->pitch_misc.half.hi;

    channel->update_flags = pending_updates | (SPU_UPDATE_START_ADDR | SPU_UPDATE_REPEAT_ADDR | SPU_UPDATE_ADSR_LOW | SPU_UPDATE_ADSR_HIGH);
    channel->spu_adsr_high = adsr_high;
}

/**
 * @brief Select an articulation by index and load it into the channel.
 * @param channel Channel to update.
 * @param articulation_index Index into the driver articulation table.
 * @see decomp.me (100%) https://decomp.me/scratch/1RRHh
 */
void akao_channel_set_articulation(AkaoChannelState* channel, s32 articulation_index)
{
    AkaoArticulation* articulation;

    channel->articulation = articulation_index;
    articulation = &AKAO_ARTICULATIONS[articulation_index];
    akao_channel_load_articulation_fields(channel, articulation, articulation->sample_addr);
}

/**
 * @brief Remove released SFX channel bits from playback and effect masks.
 * @param channel Channel whose tag and sound id are cleared.
 * @param release_mask Bits of the SFX channels to release.
 * @see decomp.me (100%) https://decomp.me/scratch/67vx9
 */
void akao_sfx_release_channels(AkaoChannelState* channel, u32 release_mask)
{
    u32 keep_mask = ~release_mask;

    g_akao_sfx_control.active_mask &= keep_mask;
    g_akao_sfx_control.paused_mask &= keep_mask;
    g_akao_sfx_control.noise_mask &= keep_mask;
    g_akao_sfx_control.reverb_mask &= keep_mask;
    g_akao_sfx_control.pitch_mod_mask &= keep_mask;
    g_akao_sfx_control.key_on_mask &= keep_mask;
    g_akao_sfx_control.note_on_mask &= keep_mask;

    channel->sfx_tag = 0;
    channel->sfx_id = 0;
}

/**
 * @brief Offset an SFX articulation within the lower or upper three-bank range.
 * @param bank_index Bank offset; zero preserves the sequence's articulation index.
 * @param articulation_index Articulation index from the sequence.
 * @return Remapped index, or the original index outside the banked ranges.
 * @see decomp.me (100%) https://decomp.me/scratch/oTcsG
 */
s32 akao_remap_sfx_articulation(s32 bank_index, s32 articulation_index)
{
    if (bank_index != 0)
    {
        if ((articulation_index >= AKAO_BANK_SLOT_ARTICULATION_INDEX(0)) &&
            (articulation_index < AKAO_BANK_SLOT_ARTICULATION_INDEX(AKAO_BANK_FIRST_UPPER_SLOT)))
        {
            return articulation_index + (bank_index * AKAO_BANK_SLOT_ARTICULATIONS);
        }
        if ((articulation_index >= AKAO_BANK_SLOT_ARTICULATION_INDEX(AKAO_BANK_FIRST_UPPER_SLOT)) &&
            (articulation_index < AKAO_BANK_SLOT_ARTICULATION_INDEX(AKAO_BANK_SLOT_COUNT)))
        {
            return articulation_index + ((bank_index - AKAO_BANK_FIRST_UPPER_SLOT) * AKAO_BANK_SLOT_ARTICULATIONS);
        }
    }
    return articulation_index;
}

/**
 * @brief Release song or SFX channel masks and request SPU effect updates.
 * @param channel Channel selecting song or SFX state; its flags are cleared.
 * @param release_mask Bits of the channels to release.
 * @note Releasing the last active song channel also clears its id, flags and pending catch-up ticks.
 * @see decomp.me (100%) https://decomp.me/scratch/vxrwL
 */
void akao_release_channels(AkaoChannelState* channel, u32 release_mask)
{
    s32 active_channels;

    if (channel->is_sfx_channel == 0)
    {
        u32 keep_mask = ~release_mask;

        active_channels = g_akao_seq_channel0->masks.active_mask & keep_mask;
        g_akao_seq_channel0->masks.active_mask = active_channels;

        if (active_channels == 0)
        {
            g_akao_seq_pending_ticks = 0;
            g_akao_seq_channel0->song_id = 0;
            g_akao_seq_channel0->flags = 0;
        }

        g_akao_seq_channel0->note_on_mask &= keep_mask;
        g_akao_seq_channel0->masks.voice_alloc_low_mask &= keep_mask;
        g_akao_seq_channel0->masks.static_voice_mask &= keep_mask;
        g_akao_seq_channel0->noise_mask &= keep_mask;
        g_akao_seq_channel0->reverb_mask &= keep_mask;
        g_akao_seq_channel0->pitch_mod_mask &= keep_mask;
    }
    else
    {
        akao_sfx_release_channels(channel, release_mask);
    }

    channel->flags = 0;
    g_akao_driver_flags.update_flags |= (AKAO_EFFECT_MASKS_UPDATE_PENDING | AKAO_NOISE_CLOCK_UPDATE_PENDING);
}

/**
 * @brief Set the song's Q16 tempo from a little-endian operand and stop its fade.
 * @param channel Channel whose bytecode cursor advances past the two tempo bytes.
 * @see decomp.me (100%) https://decomp.me/scratch/GHtCl
 */
void akao_seq_op_set_tempo(AkaoChannelState* channel)
{
    AkaoSongState* song = g_akao_seq_channel0;
    u32 tempo;

    tempo = channel->seq_cursor[0] << AKAO_Q16_SHIFT;
    song->tempo = tempo;
    song->tempo = tempo | (channel->seq_cursor[1] << (AKAO_Q16_SHIFT + AKAO_Q8_SHIFT));
    channel->seq_cursor += sizeof(u16);
    song->tempo_fade_ticks = 0;
}

/**
 * @brief Slide the song's tempo to a little-endian target over the encoded tick count.
 * @param channel Channel whose bytecode cursor advances past the count and two tempo bytes.
 * @note A zero count selects a 256-tick fade.
 * @see decomp.me (100%) https://decomp.me/scratch/SFJAU
 */
void akao_seq_op_slide_tempo(AkaoChannelState* channel)
{
    u32 target_tempo;
    u32 current_tempo;
    s32 tempo_step;
    AkaoSongState* song = g_akao_seq_channel0;
    u8* cursor = channel->seq_cursor;
    u32 fade_ticks = *cursor++;

    song->tempo_fade_ticks = fade_ticks;
    channel->seq_cursor = cursor;
    if (fade_ticks == 0)
    {
        song->tempo_fade_ticks = AKAO_FADE_DEFAULT_TICKS;
    }
    target_tempo = (channel->seq_cursor[0] << AKAO_Q16_SHIFT) | (channel->seq_cursor[1] << (AKAO_Q16_SHIFT + AKAO_Q8_SHIFT));
    channel->seq_cursor += sizeof(u16);
    current_tempo = g_akao_seq_channel0->tempo & AKAO_TEMPO_CARRY_MASK;
    tempo_step = (s32)(target_tempo - current_tempo) / g_akao_seq_channel0->tempo_fade_ticks;
    g_akao_seq_channel0->tempo = current_tempo;
    g_akao_seq_channel0->tempo_step = tempo_step;
}

/**
 * @brief Set reverb depth from a signed 16-bit operand and stop its fade.
 * @param channel Channel whose bytecode cursor advances past the two depth bytes.
 * @see decomp.me (100%) https://decomp.me/scratch/Og38F
 */
void akao_seq_op_set_reverb_depth(AkaoChannelState* channel)
{
    s32 high_byte;
    s32 low_byte;
    u32 depth;
    u8* cursor = channel->seq_cursor;
    AkaoSongState* song = g_akao_seq_channel0;

    high_byte = (s8)cursor[1];
    low_byte = cursor[0];
    channel->seq_cursor = cursor + sizeof(s16);
    song->reverb_depth_fade_ticks = 0;
    depth = high_byte << (AKAO_REVERB_DEPTH_FRACTION_BITS + AKAO_Q8_SHIFT);
    depth |= low_byte << AKAO_REVERB_DEPTH_FRACTION_BITS;
    g_akao_driver_flags.update_flags |= AKAO_REVERB_DEPTH_UPDATE_PENDING;
    song->reverb_depth = depth;
}

/**
 * @brief Slide reverb depth to a signed 16-bit target over the encoded tick count.
 * @param channel Channel whose bytecode cursor advances past the count and two depth bytes.
 * @note A zero count selects a 256-tick fade.
 * @see decomp.me (100%) https://decomp.me/scratch/w18Xw
 */
void akao_seq_op_slide_reverb_depth(AkaoChannelState* channel)
{
    AkaoSongState* song;
    s32 high_byte;
    s32 low_byte;
    s32 current_depth;
    s32 target_depth;

    song = g_akao_seq_channel0;
    song->reverb_depth_fade_ticks = *channel->seq_cursor++;
    if (song->reverb_depth_fade_ticks == 0)
    {
        song->reverb_depth_fade_ticks = AKAO_FADE_DEFAULT_TICKS;
    }
    high_byte = (s8)channel->seq_cursor[1];
    low_byte = channel->seq_cursor[0];
    channel->seq_cursor += sizeof(s16);
    target_depth = (high_byte << (AKAO_REVERB_DEPTH_FRACTION_BITS + AKAO_Q8_SHIFT)) | (low_byte << AKAO_REVERB_DEPTH_FRACTION_BITS);
    current_depth = g_akao_seq_channel0->reverb_depth & ~AKAO_REVERB_DEPTH_FRACTION_MASK;
    g_akao_seq_channel0->reverb_depth = current_depth;
    g_akao_seq_channel0->reverb_depth_step = (target_depth - current_depth) / g_akao_seq_channel0->reverb_depth_fade_ticks;
}

/**
 * @brief Jump by a signed 16-bit offset relative to the operand's start.
 * @param channel Channel whose bytecode cursor is redirected.
 * @see decomp.me (100%) https://decomp.me/scratch/KW15z
 */
void akao_seq_op_jump(AkaoChannelState* channel)
{
    u8* cursor = channel->seq_cursor;

    channel->seq_cursor = cursor + AKAO_READ_S16(cursor);
}

/**
 * @brief Take a relative branch when the song condition meets the encoded threshold.
 * @param channel Channel whose bytecode cursor branches from the offset operand or skips it.
 * @see decomp.me (100%) https://decomp.me/scratch/l153y
 */
void akao_seq_op_cond_jump(AkaoChannelState* channel)
{
    u8* cursor = channel->seq_cursor;
    AkaoSongState* song = g_akao_seq_channel0;
    s32 threshold = *cursor++;

    channel->seq_cursor = cursor;
    if (song->condition >= threshold)
    {
        channel->seq_cursor = cursor + AKAO_READ_S16(cursor);
    }
    else
    {
        channel->seq_cursor = cursor + AKAO_BRANCH_OFFSET_BYTES;
    }
}

/**
 * @brief Save the post-operand return cursor and call a relative bytecode subroutine.
 * @param channel Channel whose single return slot and bytecode cursor are updated.
 * @see decomp.me (100%) https://decomp.me/scratch/uIP0t
 */
void akao_seq_op_call(AkaoChannelState* channel)
{
    u8* cursor = channel->seq_cursor;
    s16 offset = AKAO_READ_S16(cursor);

    channel->return_cursor = cursor + AKAO_BRANCH_OFFSET_BYTES;
    channel->seq_cursor += offset;
}

/**
 * @brief Resume bytecode at the return cursor saved by akao_seq_op_call.
 * @param channel Channel whose bytecode cursor is restored.
 * @see decomp.me (100%) https://decomp.me/scratch/os90Z
 */
void akao_seq_op_return(AkaoChannelState* channel)
{
    channel->seq_cursor = channel->return_cursor;
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
    volume = *cursor << AKAO_Q8_SHIFT;
    channel->seq_cursor = cursor + 1;
    channel->update_flags |= SPU_UPDATE_VOLUME;
    channel->volume = volume;
}

/**
 * @brief Slide channel volume to the encoded target over the encoded tick count.
 * @param channel Channel whose bytecode cursor advances past the count and target bytes.
 * @note A zero count selects a 256-tick fade.
 * @see decomp.me (100%) https://decomp.me/scratch/CjVYW
 */
void akao_seq_op_slide_volume(AkaoChannelState* channel)
{
    u16 current_volume;
    u16 fade_ticks;
    u16 volume_step;
    u8* target_cursor;
    u8* cursor;

    cursor = channel->seq_cursor;
    fade_ticks = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->volume_fade_ticks = fade_ticks;
    if (fade_ticks == 0)
    {
        channel->volume_fade_ticks = AKAO_FADE_DEFAULT_TICKS;
    }
    target_cursor = channel->seq_cursor;
    current_volume = channel->volume;
    current_volume &= AKAO_CHANNEL_VOLUME_MASK;
    volume_step = ((*channel->seq_cursor++ << AKAO_Q8_SHIFT) - current_volume) / channel->volume_fade_ticks;
    channel->volume_step = volume_step;
    channel->seq_cursor = target_cursor + 1;
    channel->volume = current_volume;
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
    channel->expression = (s8)*cursor << AKAO_EXPRESSION_FRACTION_BITS;
    channel->seq_cursor = cursor + 1;
    channel->expression_fade_ticks = 0;
    channel->update_flags |= SPU_UPDATE_VOLUME;
    channel->note_expression_ticks = 0;
}

/**
 * @brief Slide the channel expression multiplier to a target.
 * @param channel Channel whose bytecode cursor advances past the count and target bytes.
 * @note A zero count selects a 256-tick fade.
 * @see decomp.me (100%) https://decomp.me/scratch/jLHGo
 */
void akao_seq_op_slide_expression(AkaoChannelState* channel)
{
    s32 current_expression;
    s32 fade_ticks;
    u8* target_cursor;
    u8* cursor;

    cursor = channel->seq_cursor;
    fade_ticks = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->expression_fade_ticks = fade_ticks;
    if (fade_ticks == 0)
    {
        channel->expression_fade_ticks = AKAO_FADE_DEFAULT_TICKS;
    }
    target_cursor = channel->seq_cursor;
    current_expression = channel->expression & AKAO_EXPRESSION_FADE_MASK;
    channel->expression_step = (((s8)*target_cursor++ << AKAO_EXPRESSION_FRACTION_BITS) - current_expression) / channel->expression_fade_ticks;
    channel->seq_cursor = target_cursor;
    channel->expression = current_expression;
    channel->note_expression_ticks = 0;
}

/**
 * @brief Configure the per-note expression ramp.
 * @param channel Channel whose bytecode cursor advances past the start, count and target bytes.
 * @note A zero count selects a 256-tick ramp.
 * @see decomp.me (100%) https://decomp.me/scratch/aOcaK
 */
void akao_seq_op_set_note_expression_envelope(AkaoChannelState* channel)
{
    s32 fade_ticks;
    u8* target_cursor;
    u8* cursor;
    u32 initial_expression;

    cursor = channel->seq_cursor;
    initial_expression = (s8)*cursor++;
    channel->seq_cursor = cursor;
    channel->expression_preset = initial_expression << AKAO_EXPRESSION_FRACTION_BITS;
    fade_ticks = *cursor++;
    channel->seq_cursor = cursor;
    channel->note_expression_ticks = fade_ticks;
    if (fade_ticks == 0)
    {
        channel->note_expression_ticks = AKAO_FADE_DEFAULT_TICKS;
    }
    target_cursor = channel->seq_cursor;
    channel->expression_preset_step = (((s8)*target_cursor << AKAO_EXPRESSION_FRACTION_BITS) - channel->expression_preset) / channel->note_expression_ticks;
    channel->seq_cursor = target_cursor + 1;
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
    channel->pan = ((*cursor + AKAO_SEQUENCE_PAN_BIAS) & AKAO_PAN_MASK) << AKAO_Q8_SHIFT;
    channel->seq_cursor = cursor + 1;
    channel->pan_fade_ticks = 0;
    channel->update_flags |= SPU_UPDATE_VOLUME;
}

/**
 * @brief Slide the channel pan to a target over a tick count.
 * @param channel Channel whose bytecode cursor advances past the count and target bytes.
 * @note A zero count selects a 256-tick fade.
 * @see decomp.me (100%) https://decomp.me/scratch/MjjmM
 */
void akao_seq_op_slide_pan(AkaoChannelState* channel)
{
    u16 current_pan;
    s32 fade_ticks;
    u8* target_cursor;
    u8* cursor;

    cursor = channel->seq_cursor;
    fade_ticks = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->pan_fade_ticks = fade_ticks;
    if (fade_ticks == 0)
    {
        channel->pan_fade_ticks = AKAO_FADE_DEFAULT_TICKS;
    }
    target_cursor = channel->seq_cursor;
    current_pan = channel->pan;
    current_pan &= AKAO_Q8_LEVEL_MASK;
    channel->pan_step = ((((*target_cursor + AKAO_SEQUENCE_PAN_BIAS) & AKAO_PAN_MASK) << AKAO_Q8_SHIFT) - current_pan) / channel->pan_fade_ticks;
    channel->seq_cursor = target_cursor + 1;
    channel->pan = current_pan;
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
    channel->octave = (channel->octave + 1) & AKAO_OCTAVE_MASK;
}

/**
 * @brief Lower the current octave by one (wraps at 16).
 * @param channel Channel state.
 * @see decomp.me (100%) https://decomp.me/scratch/SJfbQ
 */
void akao_seq_op_decrement_octave(AkaoChannelState* channel)
{
    channel->octave = (channel->octave - 1) & AKAO_OCTAVE_MASK;
}

/**
 * @brief Select an articulation; SFX channels remap it into their bank.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/1MXGr
 */
void akao_seq_op_set_mapped_articulation(AkaoChannelState* channel)
{
    AkaoArticulation* articulation_data;
    s32 sequence_articulation;
    s32 articulation_index;
    u8* cursor;

    cursor = channel->seq_cursor;
    sequence_articulation = *cursor;
    channel->seq_cursor = cursor + 1;
    if (channel->is_sfx_channel == 0)
    {
        articulation_index = sequence_articulation;
    }
    else
    {
        articulation_index = akao_remap_sfx_articulation(channel->sfx_bank, sequence_articulation);
    }

    articulation_data = &AKAO_ARTICULATIONS[articulation_index];
    akao_channel_load_articulation_fields(channel, articulation_data, articulation_data->sample_addr);
    channel->articulation = articulation_index;
    channel->spu_volume_scale = 0;
    channel->flags &= ~AKAO_CH_ARTICULATION_MASK;
}

/**
 * @brief Load an articulation's envelope and loop address with the silent primer as its sample.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/GAl01
 */
void akao_seq_op_set_articulation(AkaoChannelState* channel)
{
    s32 articulation_index;
    u8* cursor;

    cursor = channel->seq_cursor;
    articulation_index = *cursor;
    channel->seq_cursor = cursor + 1;
    akao_channel_load_articulation_fields(channel, &AKAO_ARTICULATIONS[articulation_index], AKAO_SPU_PRIMER_ADDR);
    channel->articulation = articulation_index;
    channel->spu_volume_scale = 0;
    channel->flags &= ~AKAO_CH_ARTICULATION_MASK;
}

/**
 * @brief Select a key-to-articulation map from the current sequence bank.
 * @param channel Channel state whose bytecode cursor is advanced by one byte.
 */
void akao_seq_op_select_articulation_map(AkaoChannelState* channel)
{
    uintptr_t map_base_addr;
    u16* entry;
    u8* cursor;
    u8 map_index;

    cursor = channel->seq_cursor;
    map_index = *cursor;
    channel->seq_cursor = cursor + 1;
    map_base_addr = (uintptr_t)g_akao_seq_channel0->key_map_base;
    if (map_base_addr != 0)
    {
        entry = (u16*)(map_index * sizeof(*entry) + map_base_addr);
        if (*entry > AKAO_KEY_MAP_MAX_OFFSET)
        {
            channel->spu_volume_scale = 0;
            channel->flags &= ~AKAO_CH_KEY_MAP;
            return;
        }
        channel->key_map = (u8*)(map_base_addr + *entry + AKAO_KEY_MAP_DATA_OFFSET);
        channel->note_key = AKAO_NOTE_KEY_UNSET;
        channel->flags = (channel->flags & ~AKAO_CH_ARTICULATION_MASK) | AKAO_CH_KEY_MAP;
    }
}

/**
 * @brief Restore the current articulation's ADSR settings and clear envelope overrides.
 * @param channel Channel state to update.
 */
void akao_seq_op_refresh_envelope(AkaoChannelState* channel)
{
    AkaoArticulation* articulation;
    u16 adsr_low;
    u16 adsr_high;
    s32 pending_updates;
    s32 channel_flags;

    articulation = &AKAO_ARTICULATIONS[channel->articulation];
    adsr_low = articulation->pitch_misc.half.lo;
    channel->spu_adsr_low = adsr_low;
    adsr_high = articulation->pitch_misc.half.hi;
    pending_updates = channel->update_flags;
    channel_flags = channel->flags;
    channel->update_flags = pending_updates | (SPU_UPDATE_ADSR_LOW | SPU_UPDATE_ADSR_HIGH);
    channel->flags = channel_flags & ~AKAO_CH_ADSR_OVERRIDE_MASK;
    channel->spu_adsr_high = adsr_high;
}

/**
 * @brief Set the channel transpose in semitones from a signed operand.
 * @param channel Channel state whose bytecode cursor is advanced by one byte.
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
 * @brief Add a signed semitone offset to the channel transpose.
 * @param channel Channel state whose bytecode cursor is advanced by one byte.
 */
void akao_seq_op_add_transpose(AkaoChannelState* channel)
{
    u8* cursor;
    s8 semitone_delta;

    cursor = channel->seq_cursor;
    semitone_delta = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->transpose += semitone_delta;
}

/**
 * @brief Configure a one-shot pitch slide duration and semitone delta.
 * @param channel Channel whose bytecode cursor advances past the duration and semitone-delta bytes.
 * @note A zero duration selects a 256-tick slide.
 */
void akao_seq_op_set_pitch_slide(AkaoChannelState* channel)
{
    u8* cursor;
    s32 slide_ticks;
    s8 semitone_delta;

    cursor = channel->seq_cursor;
    slide_ticks = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->pitch_slide_duration = slide_ticks;
    if (slide_ticks == 0)
    {
        channel->pitch_slide_duration = AKAO_FADE_DEFAULT_TICKS;
    }
    semitone_delta = *channel->seq_cursor++;
    channel->pitch_slide_delta = semitone_delta;
}

/**
 * @brief Enable automatic portamento between successive notes.
 * @param channel Channel whose bytecode cursor advances past the slide-duration byte.
 * @note A zero duration selects a 256-tick slide.
 */
void akao_seq_op_enable_portamento(AkaoChannelState* channel)
{
    u8* cursor;
    s32 slide_ticks;

    cursor = channel->seq_cursor;
    slide_ticks = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->portamento_speed = slide_ticks;
    if (slide_ticks == 0)
    {
        channel->portamento_speed = AKAO_FADE_DEFAULT_TICKS;
    }
    channel->prev_transpose = 0;
    channel->prev_key = 0;
    channel->note_flags = AKAO_NOTE_TIE_ARMED;
}

/**
 * @brief Disable automatic portamento between notes.
 * @param channel Channel state.
 */
void akao_seq_op_disable_portamento(AkaoChannelState* channel)
{
    channel->portamento_speed = 0;
}

/**
 * @brief Set fine pitch detune and recompute its pitch-register delta.
 * @param channel Channel state whose bytecode cursor is advanced by one byte.
 */
void akao_seq_op_set_detune(AkaoChannelState* channel)
{
    s32 pitch;
    u32 detune_product;
    u32 detune_scale;
    u32 pitch_delta;
    u8* cursor;

    cursor = channel->seq_cursor;
    channel->detune = (s8)*cursor;
    detune_scale = (u8)channel->detune;
    pitch = channel->pitch;
    detune_product = pitch * detune_scale;
    channel->seq_cursor = cursor + 1;
    if (channel->detune < 0)
    {
        pitch_delta = (detune_product >> AKAO_Q8_SHIFT) - pitch;
    }
    else
    {
        pitch_delta = detune_product >> AKAO_Q7_SHIFT;
    }
    channel->detune_pitch_delta = pitch_delta;
    channel->update_flags |= SPU_UPDATE_PITCH;
}

/**
 * @brief Add to fine pitch detune and recompute its pitch-register delta.
 * @param channel Channel state whose bytecode cursor is advanced by one byte.
 */
void akao_seq_op_add_detune(AkaoChannelState* channel)
{
    s32 pitch;
    u8* cursor;
    u32 detune_product;
    u32 pitch_delta;

    cursor = channel->seq_cursor;
    pitch = channel->pitch;
    channel->detune += (s8)*cursor;
    channel->seq_cursor = cursor + 1;
    pitch_delta = (u8)channel->detune;
    detune_product = pitch * pitch_delta;
    if (channel->detune < 0)
    {
        pitch_delta = (detune_product >> AKAO_Q8_SHIFT) - pitch;
    }
    else
    {
        pitch_delta = detune_product >> AKAO_Q7_SHIFT;
    }
    channel->detune_pitch_delta = pitch_delta;
    channel->update_flags |= SPU_UPDATE_PITCH;
}

/**
 * @brief Start the pitch LFO with the encoded period and waveform.
 * @param channel Channel whose bytecode cursor advances past three parameters.
 * @note The first operand sets song delay or, when nonzero, SFX depth.
 */
void akao_seq_op_start_pitch_lfo(AkaoChannelState* channel)
{
    u32 pitch;
    u32 waveform_index;
    u16 packed_depth;
    u32 depth_level;
    u32 scaled_depth;
    u8* cursor;
    s32 sfx_depth;
    s32 period_ticks;

    channel->flags |= AKAO_CH_PITCH_LFO;
    if (channel->is_sfx_channel != 0)
    {
        cursor = channel->seq_cursor;
        channel->pitch_lfo_delay = 0;
        sfx_depth = *cursor;
        channel->seq_cursor = cursor + 1;
        if (sfx_depth != 0)
        {
            channel->pitch_lfo_depth.raw = sfx_depth << AKAO_Q8_SHIFT;
        }
    }
    else
    {
        cursor = channel->seq_cursor;
        channel->pitch_lfo_delay = *cursor;
        channel->seq_cursor = cursor + 1;
    }
    cursor = channel->seq_cursor;
    period_ticks = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->pitch_lfo_period = period_ticks;
    if (period_ticks == 0)
    {
        channel->pitch_lfo_period = AKAO_LFO_DEFAULT_PERIOD;
    }
    cursor = channel->seq_cursor;
    packed_depth = channel->pitch_lfo_depth.raw;
    waveform_index = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->pitch_lfo_waveform = waveform_index;
    pitch = (u16)channel->pitch;
    depth_level = (u32)(packed_depth & AKAO_PITCH_LFO_DEPTH_MASK) >> AKAO_Q8_SHIFT;
    if (!(packed_depth & AKAO_PITCH_LFO_ABSOLUTE_FLAG))
    {
        scaled_depth = depth_level * ((s32)(pitch * AKAO_PITCH_LFO_RELATIVE_SCALE) >> AKAO_Q8_SHIFT);
    }
    else
    {
        scaled_depth = depth_level * pitch;
    }
    channel->pitch_lfo_depth.scaled = scaled_depth >> AKAO_Q7_SHIFT;
    channel->pitch_lfo_cursor = g_akao_lfo_waveforms[channel->pitch_lfo_waveform];
    channel->pitch_lfo_delay_ticks = channel->pitch_lfo_delay;
    channel->pitch_lfo_restart = 1;
}

/**
 * @brief Set the active pitch-LFO depth and recompute its scaled depth.
 * @param channel Channel state whose bytecode cursor is advanced by one byte.
 */
void akao_seq_op_set_pitch_lfo_depth(AkaoChannelState* channel)
{
    s32 pitch;
    u8* cursor;
    const AkaoPitchLfoDepth* depth;
    u32 encoded_depth;
    u32 depth_level;
    u32 scaled_depth;

    cursor = channel->seq_cursor;
    encoded_depth = *cursor << AKAO_Q8_SHIFT;
    channel->seq_cursor = cursor + 1;
    pitch = channel->pitch;
    channel->pitch_lfo_depth.raw = encoded_depth;
    depth = &channel->pitch_lfo_depth;
    depth_level = (u32)(depth->raw & AKAO_PITCH_LFO_DEPTH_MASK) >> AKAO_Q8_SHIFT;
    if (!(depth->raw & AKAO_PITCH_LFO_ABSOLUTE_FLAG))
    {
        scaled_depth = depth_level * ((s32)(pitch * AKAO_PITCH_LFO_RELATIVE_SCALE) >> AKAO_Q8_SHIFT);
    }
    else
    {
        scaled_depth = depth_level * pitch;
    }
    channel->pitch_lfo_depth.scaled = scaled_depth >> AKAO_Q7_SHIFT;
}

/**
 * @brief Slide the pitch-LFO depth to a target over a tick count.
 * @param channel Channel state whose bytecode cursor is advanced by two bytes.
 * @note A zero count selects a 256-tick fade.
 */
void akao_seq_op_slide_pitch_lfo_depth(AkaoChannelState* channel)
{
    u8* cursor;
    s32 fade_ticks;
    s32 depth_step;

    cursor = channel->seq_cursor;
    fade_ticks = *cursor;
    cursor++;
    channel->seq_cursor = cursor;
    if (fade_ticks == 0)
    {
        fade_ticks = AKAO_FADE_DEFAULT_TICKS;
    }
    depth_step = ((*cursor << AKAO_Q8_SHIFT) - channel->pitch_lfo_depth.raw) / fade_ticks;
    channel->seq_cursor = cursor + 1;
    channel->pitch_lfo_depth_fade_ticks = fade_ticks;
    channel->pitch_lfo_depth_step = depth_step;
}

/**
 * @brief Stop the pitch LFO, clear its output and request a pitch update.
 * @param channel Channel state.
 */
void akao_seq_op_stop_pitch_lfo(AkaoChannelState* channel)
{
    channel->pitch_lfo_value = 0;
    channel->flags &= ~AKAO_CH_PITCH_LFO;
    channel->update_flags |= SPU_UPDATE_PITCH;
}

/**
 * @brief Start the volume LFO with the encoded period and waveform.
 * @param channel Channel whose bytecode cursor advances past three parameters.
 * @note The first operand sets song delay or, when nonzero, SFX depth.
 */
void akao_seq_op_start_volume_lfo(AkaoChannelState* channel)
{
    u8* cursor;
    s32 first_operand;
    s32 period;
    u32 waveform_index;

    cursor = channel->seq_cursor;
    channel->flags |= AKAO_CH_VOLUME_LFO;
    first_operand = *cursor;
    channel->seq_cursor = cursor + 1;
    if (channel->is_sfx_channel != 0)
    {
        channel->volume_lfo_delay = 0;
        if (first_operand != 0)
        {
            channel->volume_lfo_depth = (first_operand & AKAO_VOLUME_MAX) << AKAO_Q8_SHIFT;
        }
    }
    else
    {
        channel->volume_lfo_delay = first_operand;
    }
    period = *channel->seq_cursor++;
    channel->volume_lfo_period = period;
    if (period == 0)
    {
        channel->volume_lfo_period = AKAO_LFO_DEFAULT_PERIOD;
    }
    waveform_index = *channel->seq_cursor++;
    channel->volume_lfo_waveform = waveform_index;
    channel->volume_lfo_cursor = g_akao_lfo_waveforms[channel->volume_lfo_waveform];
    channel->volume_lfo_delay_ticks = channel->volume_lfo_delay;
    channel->volume_lfo_restart = 1;
}

/**
 * @brief Set the active volume-LFO depth.
 * @param channel Channel state whose bytecode cursor is advanced by one byte.
 */
void akao_seq_op_set_volume_lfo_depth(AkaoChannelState* channel)
{
    u8* cursor;
    s32 depth;

    cursor = channel->seq_cursor;
    depth = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->volume_lfo_depth = (depth & AKAO_VOLUME_MAX) << AKAO_Q8_SHIFT;
}

/**
 * @brief Slide the volume-LFO depth to a target over a tick count.
 * @param channel Channel state whose bytecode cursor is advanced by two bytes.
 * @note A zero count selects a 256-tick fade.
 */
void akao_seq_op_slide_volume_lfo_depth(AkaoChannelState* channel)
{
    u8* cursor;
    s32 fade_ticks;
    s32 depth_step;

    cursor = channel->seq_cursor;
    fade_ticks = *cursor;
    cursor++;
    channel->seq_cursor = cursor;
    if (fade_ticks == 0)
    {
        fade_ticks = AKAO_FADE_DEFAULT_TICKS;
    }
    depth_step = (((*cursor & AKAO_VOLUME_MAX) << AKAO_Q8_SHIFT) - channel->volume_lfo_depth) / fade_ticks;
    channel->seq_cursor = cursor + 1;
    channel->volume_lfo_depth_fade_ticks = fade_ticks;
    channel->volume_lfo_depth_step = depth_step;
}

/**
 * @brief Stop the volume LFO, clear its output and request a volume update.
 * @param channel Channel state.
 */
void akao_seq_op_stop_volume_lfo(AkaoChannelState* channel)
{
    channel->volume_lfo_value = 0;
    channel->flags &= ~AKAO_CH_VOLUME_LFO;
    channel->update_flags |= SPU_UPDATE_VOLUME;
}

/**
 * @brief Start the channel pan LFO, selecting its period and waveform.
 * @param channel Channel state whose bytecode cursor is advanced by two bytes.
 */
void akao_seq_op_start_pan_lfo(AkaoChannelState* channel)
{
    s32 period;
    u32 waveform_index;

    channel->flags |= AKAO_CH_PAN_LFO;
    period = *channel->seq_cursor++;
    channel->pan_lfo_period = period;
    if (period == 0)
    {
        channel->pan_lfo_period = AKAO_LFO_DEFAULT_PERIOD;
    }
    waveform_index = *channel->seq_cursor++;
    channel->pan_lfo_waveform = waveform_index;
    channel->pan_lfo_cursor = g_akao_lfo_waveforms[channel->pan_lfo_waveform];
    channel->pan_lfo_restart = 1;
}

/**
 * @brief Set the active pan-LFO depth.
 * @param channel Channel state whose bytecode cursor is advanced by one byte.
 */
void akao_seq_op_set_pan_lfo_depth(AkaoChannelState* channel)
{
    u8* cursor;
    s32 depth;

    cursor = channel->seq_cursor;
    depth = *cursor;
    channel->seq_cursor = cursor + 1;
    channel->pan_lfo_depth = depth << AKAO_Q7_SHIFT;
}

/**
 * @brief Slide the pan-LFO depth to a target over a tick count.
 * @param channel Channel state whose bytecode cursor is advanced by two bytes.
 * @note A zero count selects a 256-tick fade.
 */
void akao_seq_op_slide_pan_lfo_depth(AkaoChannelState* channel)
{
    u8* cursor;
    s32 fade_ticks;
    s32 depth_step;

    cursor = channel->seq_cursor;
    fade_ticks = *cursor;
    cursor++;
    channel->seq_cursor = cursor;
    if (fade_ticks == 0)
    {
        fade_ticks = AKAO_FADE_DEFAULT_TICKS;
    }
    depth_step = ((*cursor << AKAO_Q7_SHIFT) - channel->pan_lfo_depth) / fade_ticks;
    channel->seq_cursor = cursor + 1;
    channel->pan_lfo_depth_fade_ticks = fade_ticks;
    channel->pan_lfo_depth_step = depth_step;
}

/**
 * @brief Stop the pan LFO, clear its output and request a volume update.
 * @param channel Channel state.
 */
void akao_seq_op_stop_pan_lfo(AkaoChannelState* channel)
{
    channel->pan_lfo_value = 0;
    channel->flags &= ~AKAO_CH_PAN_LFO;
    channel->update_flags |= SPU_UPDATE_VOLUME;
}

/**
 * @brief Enable channel noise and request effect-mask and noise-clock updates.
 * @param channel Channel whose role selects song or SFX state.
 * @param channel_bit Bit of the channel in the effect mask.
 */
void akao_seq_op_enable_noise(AkaoChannelState* channel, s32 channel_bit)
{
    if (channel->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->noise_mask |= channel_bit;
    }
    else
    {
        g_akao_sfx_control.noise_mask |= channel_bit;
    }
    g_akao_driver_flags.update_flags |= (AKAO_EFFECT_MASKS_UPDATE_PENDING | AKAO_NOISE_CLOCK_UPDATE_PENDING);
}

/**
 * @brief Disable channel noise and cancel its pending toggle.
 * @param channel Channel whose role selects song or SFX state.
 * @param channel_bit Bit of the channel in the effect mask.
 */
void akao_seq_op_disable_noise(AkaoChannelState* channel, s32 channel_bit)
{
    if (channel->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->noise_mask &= ~channel_bit;
    }
    else
    {
        g_akao_sfx_control.noise_mask &= ~channel_bit;
    }
    g_akao_driver_flags.update_flags |= (AKAO_EFFECT_MASKS_UPDATE_PENDING | AKAO_NOISE_CLOCK_UPDATE_PENDING);
    channel->noise_toggle_ticks = 0;
}

/**
 * @brief Enable pitch modulation for a song channel or an eligible SFX channel.
 * @param channel Channel whose role selects song or SFX state.
 * @param channel_bit Bit of the channel in the effect mask.
 * @note SFX channels require AKAO_CH_SFX_PITCH_MOD.
 */
void akao_seq_op_enable_pitch_modulation(AkaoChannelState* channel, s32 channel_bit)
{
    if (channel->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->pitch_mod_mask |= channel_bit;
    }
    else if (channel->flags & AKAO_CH_SFX_PITCH_MOD)
    {
        g_akao_sfx_control.pitch_mod_mask |= channel_bit;
    }
    g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
}

/**
 * @brief Disable channel pitch modulation and cancel its pending toggle.
 * @param channel Channel whose role selects song or SFX state.
 * @param channel_bit Bit of the channel in the effect mask.
 */
void akao_seq_op_disable_pitch_modulation(AkaoChannelState* channel, s32 channel_bit)
{
    if (channel->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->pitch_mod_mask &= ~channel_bit;
    }
    else
    {
        g_akao_sfx_control.pitch_mod_mask &= ~channel_bit;
    }
    g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
    channel->pitch_mod_toggle_ticks = 0;
}

/**
 * @brief Enable reverb for the song or SFX channel.
 * @param channel Channel whose role selects song or SFX state.
 * @param channel_bit Bit of the channel in the effect mask.
 */
void akao_seq_op_enable_reverb(AkaoChannelState* channel, s32 channel_bit)
{
    if (channel->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->reverb_mask |= channel_bit;
    }
    else
    {
        g_akao_sfx_control.reverb_mask |= channel_bit;
    }
    g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
}

/**
 * @brief Disable reverb for the song or SFX channel.
 * @param channel Channel whose role selects song or SFX state.
 * @param channel_bit Bit of the channel in the effect mask.
 */
void akao_seq_op_disable_reverb(AkaoChannelState* channel, s32 channel_bit)
{
    if (channel->is_sfx_channel == 0)
    {
        g_akao_seq_channel0->reverb_mask &= ~channel_bit;
    }
    else
    {
        g_akao_sfx_control.reverb_mask &= ~channel_bit;
    }
    g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
}

/**
 * @brief Enable tied notes; subsequent notes change pitch without retriggering.
 * @param channel Channel state.
 */
void akao_seq_op_enable_note_tie(AkaoChannelState* channel)
{
    channel->note_flags = AKAO_NOTE_TIE_ARMED;
}

/**
 * @brief No-op handler for primary opcode 0xCD.
 */
void akao_seq_op_nop_cd(void)
{
}

/**
 * @brief Give SFX notes their full duration instead of an early key-off.
 * @param channel Channel state; @c is_sfx_channel selects whether the store happens.
 */
void akao_seq_op_enable_sfx_full_gate(AkaoChannelState* channel)
{
    if (channel->is_sfx_channel != 0)
    {
        channel->note_flags = AKAO_NOTE_SFX_FULL_GATE;
    }
}

/**
 * @brief No-op handler for primary opcode 0xD1.
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
    s16 noise_operand;
    u8* cursor;

    cursor = channel->seq_cursor;
    noise_operand = *cursor;
    channel->seq_cursor = cursor + 1;
    if (channel->is_sfx_channel == 0)
    {
        if (noise_operand & AKAO_NOISE_RELATIVE_MASK)
        {
            g_akao_seq_channel0->noise_freq = (g_akao_seq_channel0->noise_freq + (noise_operand & AKAO_NOISE_FREQUENCY_MASK)) & AKAO_NOISE_FREQUENCY_MASK;
        }
        else
        {
            g_akao_seq_channel0->noise_freq = noise_operand;
        }
    }
    else if (noise_operand & AKAO_NOISE_RELATIVE_MASK)
    {
        g_akao_sfx_control.noise_freq = (g_akao_sfx_control.noise_freq + (noise_operand & AKAO_NOISE_FREQUENCY_MASK)) & AKAO_NOISE_FREQUENCY_MASK;
    }
    else
    {
        g_akao_sfx_control.noise_freq = noise_operand;
    }
    g_akao_driver_flags.update_flags |= AKAO_NOISE_CLOCK_UPDATE_PENDING;
}

/**
 * @brief Override the ADSR attack rate.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/gwBIb
 */
void akao_seq_op_set_adsr_attack(AkaoChannelState* channel)
{
    u8* cursor;
    u32 attack_rate;
    u32 pending_updates;
    u32 channel_flags;
    u16 adsr_low;

    cursor = channel->seq_cursor;
    attack_rate = *cursor;
    channel->seq_cursor = cursor + 1;

    pending_updates = channel->update_flags | (SPU_VOICE_ADSR_AMODE | SPU_VOICE_ADSR_AR);
    channel->update_flags = pending_updates;

    channel_flags = channel->flags | AKAO_CH_ADSR_ATTACK;

    adsr_low = (channel->spu_adsr_low & (AKAO_ADSR_ATTACK_MODE_MASK | SPU_ADSR_DECAY_SUSTAIN_MASK)) | ((u16)attack_rate << SPU_ADSR_ATTACK_RATE_SHIFT);

    channel->flags = channel_flags;
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
    u32 decay_rate;
    u32 pending_updates;
    u16 adsr_low;

    cursor = channel->seq_cursor;
    decay_rate = *cursor;
    channel->seq_cursor = cursor + 1;

    pending_updates = channel->update_flags | SPU_VOICE_ADSR_DR;
    adsr_low = (channel->spu_adsr_low & (AKAO_LOW_HALF_MASK ^ SPU_ADSR_DECAY_RATE_MASK)) | (decay_rate << SPU_ADSR_DECAY_RATE_SHIFT);

    channel->update_flags = pending_updates;
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
    u32 sustain_level;
    u32 pending_updates;
    u16 adsr_low;

    cursor = channel->seq_cursor;
    sustain_level = *cursor;
    channel->seq_cursor = cursor + 1;
    pending_updates = channel->update_flags | SPU_VOICE_ADSR_SL;
    adsr_low = (channel->spu_adsr_low & (AKAO_LOW_HALF_MASK ^ SPU_ADSR_SUSTAIN_LEVEL_MASK)) | sustain_level;
    channel->update_flags = pending_updates;
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
    u32 sustain_rate;
    u32 pending_updates;
    u32 channel_flags;
    u16 adsr_high;

    cursor = channel->seq_cursor;
    sustain_rate = *cursor;
    channel->seq_cursor = cursor + 1;

    pending_updates = channel->update_flags | (SPU_VOICE_ADSR_SMODE | SPU_VOICE_ADSR_SR);
    channel_flags = channel->flags | AKAO_CH_ADSR_SUSTAIN_RATE;
    adsr_high = (channel->spu_adsr_high & (AKAO_LOW_HALF_MASK ^ AKAO_ADSR_SUSTAIN_RATE_MASK)) | (sustain_rate << SPU_ADSR_SUSTAIN_RATE_SHIFT);

    channel->update_flags = pending_updates;
    channel->flags = channel_flags;
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
    u32 release_rate;
    u32 pending_updates;
    u32 channel_flags;
    u16 adsr_high;

    cursor = channel->seq_cursor;
    release_rate = *cursor;
    channel->seq_cursor = cursor + 1;

    pending_updates = channel->update_flags | (SPU_VOICE_ADSR_RMODE | SPU_VOICE_ADSR_RR);
    channel_flags = channel->flags | AKAO_CH_ADSR_RELEASE_RATE;
    adsr_high = (channel->spu_adsr_high & (AKAO_LOW_HALF_MASK ^ AKAO_ADSR_RELEASE_RATE_MASK)) | release_rate;

    channel->update_flags = pending_updates;
    channel->flags = channel_flags;
    channel->spu_adsr_high = adsr_high;
}

/**
 * @brief Select linear or exponential ADSR attack.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/dP97Y
 */
void akao_seq_op_set_adsr_attack_mode(AkaoChannelState* channel)
{
    u8* cursor;
    u32 attack_mode;
    u16 adsr_low;

    cursor = channel->seq_cursor;
    attack_mode = *cursor;
    channel->seq_cursor = cursor + 1;

    adsr_low = channel->spu_adsr_low & (AKAO_LOW_HALF_MASK ^ AKAO_ADSR_ATTACK_MODE_MASK);
    channel->spu_adsr_low = adsr_low;
    if (attack_mode == SPU_VOICE_EXPIncN)
    {
        adsr_low |= AKAO_ADSR_ATTACK_MODE_MASK;
        channel->spu_adsr_low = adsr_low;
    }

    channel->update_flags |= SPU_VOICE_ADSR_AMODE;
}

/**
 * @brief Select the ADSR sustain direction and linear or exponential mode.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/8od0h
 */
void akao_seq_op_set_adsr_sustain_mode(AkaoChannelState* channel)
{
    u16 adsr_high;
    u8* cursor;
    s32 sustain_mode;
    u32 encoded_mode;

    cursor = channel->seq_cursor;
    encoded_mode = *cursor;
    adsr_high = channel->spu_adsr_high & (AKAO_LOW_HALF_MASK ^ AKAO_ADSR_SUSTAIN_MODE_MASK);
    channel->seq_cursor = cursor + 1;
    sustain_mode = (u16)encoded_mode;
    channel->spu_adsr_high = adsr_high;

    switch (sustain_mode)
    {
    case SPU_VOICE_LINEARDecN:
        channel->spu_adsr_high = adsr_high | AKAO_ADSR_SUSTAIN_DIRECTION_MASK;
        break;
    case SPU_VOICE_EXPIncN:
        channel->spu_adsr_high = adsr_high | AKAO_ADSR_SUSTAIN_EXPONENTIAL_MASK;
        break;
    case SPU_VOICE_EXPDec:
        channel->spu_adsr_high = adsr_high | AKAO_ADSR_SUSTAIN_MODE_MASK;
        break;
    }

    channel->update_flags |= SPU_VOICE_ADSR_SMODE;
}

/**
 * @brief Select linear or exponential ADSR release.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @see decomp.me (100%) https://decomp.me/scratch/1Pqa0
 */
void akao_seq_op_set_adsr_release_mode(AkaoChannelState* channel)
{
    u8* cursor;
    u16 adsr_high;
    u32 release_mode;

    cursor = channel->seq_cursor;
    release_mode = *cursor;
    channel->seq_cursor = cursor + 1;

    adsr_high = channel->spu_adsr_high & (AKAO_LOW_HALF_MASK ^ AKAO_ADSR_RELEASE_MODE_MASK);
    channel->spu_adsr_high = adsr_high;

    if (release_mode == SPU_VOICE_EXPDec)
    {
        adsr_high |= AKAO_ADSR_RELEASE_MODE_MASK;
        channel->spu_adsr_high = adsr_high;
    }

    channel->update_flags |= SPU_VOICE_ADSR_RMODE;
}

/**
 * @brief Reserve voices below the encoded allocation base.
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
 * @brief Allow normal voice allocation to start at voice zero.
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
    channel->loop_depth = (channel->loop_depth + 1) & AKAO_LOOP_DEPTH_MASK;
    channel->loop_cursor[channel->loop_depth] = channel->seq_cursor;
    channel->loop_count[channel->loop_depth] = 0;
    channel->loop_opcode_count[channel->loop_depth] = channel->opcode_count;
}

/**
 * @brief Close a loop: repeat until the operand count is reached, then pop.
 * @param channel Channel whose loop count and bytecode cursor are advanced.
 * @note A zero count selects 256 iterations.
 * @see decomp.me (100%) https://decomp.me/scratch/xSadm
 */
void akao_seq_op_loop_end(AkaoChannelState* channel)
{
    u32 iteration_limit;

    iteration_limit = *channel->seq_cursor++;

    if (iteration_limit == 0)
    {
        iteration_limit = AKAO_LOOP_DEFAULT_COUNT;
    }

    if (++channel->loop_count[channel->loop_depth] != iteration_limit)
    {
        channel->seq_cursor = channel->loop_cursor[channel->loop_depth];
        channel->opcode_count = channel->loop_opcode_count[channel->loop_depth];
        return;
    }

    channel->loop_depth = (channel->loop_depth - 1) & AKAO_LOOP_DEPTH_MASK;
}

/**
 * @brief Branch by a relative offset on the last pass of the current loop.
 * @param channel Channel whose cursor skips the operands or branches from the offset operand.
 * @note A zero count selects 256 iterations.
 * @see decomp.me (100%) https://decomp.me/scratch/XFsED
 */
void akao_seq_op_branch_on_loop_last(AkaoChannelState* channel)
{
    u8* cursor;
    u32 iteration_limit;

    cursor = channel->seq_cursor;
    iteration_limit = *cursor++;
    channel->seq_cursor = cursor;

    if (iteration_limit == 0)
    {
        iteration_limit = AKAO_LOOP_DEFAULT_COUNT;
    }

    if (channel->loop_count[channel->loop_depth] + 1 != iteration_limit)
    {
        channel->seq_cursor = cursor + AKAO_BRANCH_OFFSET_BYTES;
        return;
    }

    channel->seq_cursor = cursor + AKAO_READ_S16(cursor);
}

/**
 * @brief On the last loop pass, branch by a relative offset and pop the loop.
 * @param channel Channel whose cursor skips the operands or branches from the offset operand.
 * @note A zero count selects 256 iterations.
 * @see decomp.me (100%) https://decomp.me/scratch/Gx2w7
 */
void akao_seq_op_branch_and_end_loop(AkaoChannelState* channel)
{
    u8* cursor;
    u32 iteration_limit;

    cursor = channel->seq_cursor;
    iteration_limit = *cursor++;
    channel->seq_cursor = cursor;

    if (iteration_limit == 0)
    {
        iteration_limit = AKAO_LOOP_DEFAULT_COUNT;
    }

    if (channel->loop_count[channel->loop_depth] + 1 != iteration_limit)
    {
        channel->seq_cursor = cursor + AKAO_BRANCH_OFFSET_BYTES;
        return;
    }

    channel->seq_cursor = cursor + AKAO_READ_S16(cursor);
    channel->loop_depth = (channel->loop_depth - 1) & AKAO_LOOP_DEPTH_MASK;
}

/**
 * @brief Unconditionally repeat the current loop.
 * @param channel Channel state.
 * @see decomp.me (100%) https://decomp.me/scratch/Eytqk
 */
void akao_seq_op_repeat_loop(AkaoChannelState* channel)
{
    channel->loop_count[channel->loop_depth]++;
    channel->seq_cursor = channel->loop_cursor[channel->loop_depth];
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

    duration = *channel->seq_cursor++;

    channel->note_duration_adjust = 0;
    channel->gate_ticks = duration;
    channel->note_ticks = duration;
    channel->note_duration = duration;
}

/**
 * @brief Adjust the last note duration, or clear the adjustment for a zero operand.
 * @param channel Channel whose bytecode cursor is advanced past the operand.
 * @note Nonzero signed deltas select a duration clamped to 1..255 ticks.
 * @see decomp.me (100%) https://decomp.me/scratch/PBFzu
 */
void akao_seq_op_adjust_note_duration(AkaoChannelState* channel)
{
    s32 duration;

    duration = (s8)*channel->seq_cursor++;

    if (duration != 0)
    {
        duration += channel->note_duration;

        if (duration <= 0)
        {
            duration = 1;
        }
        else if (duration > AKAO_NOTE_DURATION_MAX)
        {
            duration = AKAO_NOTE_DURATION_MAX;
        }
    }

    channel->note_duration_adjust = duration;
}

/**
 * @brief Enable per-note drum articulations when the song has a note table.
 * @param channel Channel to switch into drum mode.
 * @see decomp.me (100%) https://decomp.me/scratch/CB7Yy
 */
void akao_seq_op_enable_drum_mode(AkaoChannelState* channel)
{
    if (g_akao_seq_channel0->note_table != 0)
    {
        channel->flags = (channel->flags & ~AKAO_CH_ARTICULATION_MASK) | AKAO_CH_DRUM_MODE;
    }
}

/**
 * @brief Disable drum mode and reset the channel's volume scale.
 * @param channel Channel to switch out of drum mode.
 * @see decomp.me (100%) https://decomp.me/scratch/FnMZ3
 */
void akao_seq_op_disable_drum_mode(AkaoChannelState* channel)
{
    channel->spu_volume_scale = 0;
    channel->flags &= ~AKAO_CH_DRUM_MODE;
}

/**
 * @brief Set ticks per beat and beats per measure, resetting the tick and beat counters.
 * @param channel Channel whose bytecode cursor is advanced past the two operand bytes.
 * @see decomp.me (100%) https://decomp.me/scratch/wuYt5
 */
void akao_seq_op_set_time_signature(AkaoChannelState* channel)
{
    AkaoSongState* song = g_akao_seq_channel0;

    song->ticks_per_beat = *channel->seq_cursor++;
    song->beats_per_measure = *channel->seq_cursor++;
    song->tick = 0;
    song->beat = 0;
}

/**
 * @brief Set the current song measure from a little-endian 16-bit operand.
 * @param channel Channel whose bytecode cursor is advanced past the two operand bytes.
 * @see decomp.me (100%) https://decomp.me/scratch/FcDgE
 */
void akao_seq_op_set_measure(AkaoChannelState* channel)
{
    AkaoSongState* song = g_akao_seq_channel0;

    song->measure = *channel->seq_cursor++;
    song->measure |= *channel->seq_cursor++ << AKAO_OPERAND_HIGH_BYTE_SHIFT;
}

/**
 * @brief Set the ADSR decay rate and sustain level from consecutive operand bytes.
 * @param channel Channel being programmed.
 * @param channel_mask Unused argument forwarded to both ADSR handlers.
 * @see decomp.me (100%) https://decomp.me/scratch/XqM1L
 */
void akao_seq_op_set_adsr_decay_sustain_level(AkaoChannelState* channel, s32 channel_mask)
{
    akao_seq_op_set_adsr_decay(channel, channel_mask);
    akao_seq_op_set_adsr_sustain_level(channel, channel_mask);
}

/**
 * @brief Enable noise and schedule its next toggle.
 * @param channel Channel whose bytecode cursor is advanced past the delay byte.
 * @param channel_bit Bit of the channel in the noise mask.
 * @note The countdown is the operand plus one; zero selects 257 ticks.
 * @see decomp.me (100%) https://decomp.me/scratch/cAsju
 */
void akao_seq_op_enable_noise_then_toggle(AkaoChannelState* channel, s32 channel_bit)
{
    s32 delay_ticks;
    s16 toggle_ticks;

    delay_ticks = *channel->seq_cursor++;

    if (delay_ticks != 0)
    {
        toggle_ticks = delay_ticks + 1;
    }
    else
    {
        toggle_ticks = AKAO_TOGGLE_DEFAULT_TICKS;
    }

    channel->noise_toggle_ticks = toggle_ticks;
    akao_seq_op_enable_noise(channel, channel_bit);
}

/**
 * @brief Schedule a noise toggle without changing its current state.
 * @param channel Channel whose bytecode cursor is advanced past the delay byte.
 * @note The countdown is the operand plus one; zero selects 257 ticks.
 * @see decomp.me (100%) https://decomp.me/scratch/Basyw
 */
void akao_seq_op_schedule_noise_toggle(AkaoChannelState* channel)
{
    s32 delay_ticks;

    delay_ticks = *channel->seq_cursor++;

    if (delay_ticks != 0)
    {
        channel->noise_toggle_ticks = delay_ticks + 1;
    }
    else
    {
        channel->noise_toggle_ticks = AKAO_TOGGLE_DEFAULT_TICKS;
    }
}

/**
 * @brief Request pitch modulation and schedule its next toggle.
 * @param channel Channel whose bytecode cursor is advanced past the delay byte.
 * @param channel_bit Bit of the channel in the pitch-modulation mask.
 * @note The countdown is the operand plus one; zero selects 257 ticks.
 * @see decomp.me (100%) https://decomp.me/scratch/j1qFh
 */
void akao_seq_op_enable_pitch_mod_then_toggle(AkaoChannelState* channel, s32 channel_bit)
{
    s32 delay_ticks;
    s16 toggle_ticks;

    delay_ticks = *channel->seq_cursor++;

    if (delay_ticks != 0)
    {
        toggle_ticks = delay_ticks + 1;
    }
    else
    {
        toggle_ticks = AKAO_TOGGLE_DEFAULT_TICKS;
    }

    channel->pitch_mod_toggle_ticks = toggle_ticks;
    akao_seq_op_enable_pitch_modulation(channel, channel_bit);
}

/**
 * @brief Schedule a pitch-modulation toggle without changing its current state.
 * @param channel Channel whose bytecode cursor is advanced past the delay byte.
 * @note The countdown is the operand plus one; zero selects 257 ticks.
 * @see decomp.me (100%) https://decomp.me/scratch/3922q
 */
void akao_seq_op_schedule_pitch_mod_toggle(AkaoChannelState* channel)
{
    s32 delay_ticks;

    delay_ticks = *channel->seq_cursor++;

    if (delay_ticks != 0)
    {
        channel->pitch_mod_toggle_ticks = delay_ticks + 1;
    }
    else
    {
        channel->pitch_mod_toggle_ticks = AKAO_TOGGLE_DEFAULT_TICKS;
    }
}

/**
 * @brief Clear LFO and sidechain flags, disable SPU effects, and disarm note ties and SFX full gate.
 * @param channel Channel to reset.
 * @param channel_bit Bit of the channel in the SPU effect masks.
 * @see decomp.me (100%) https://decomp.me/scratch/LEJvC
 */
void akao_seq_op_reset_effects(AkaoChannelState* channel, s32 channel_bit)
{
    channel->flags &= ~(AKAO_CH_PITCH_LFO | AKAO_CH_VOLUME_LFO | AKAO_CH_PAN_LFO | AKAO_CH_PITCH_SIDECHAIN | AKAO_CH_PITCH_VOLUME_SIDECHAIN);

    akao_seq_op_disable_noise(channel, channel_bit);
    akao_seq_op_disable_pitch_modulation(channel, channel_bit);
    akao_seq_op_disable_reverb(channel, channel_bit);

    channel->note_flags &= AKAO_NOTE_CLEAR_TIE_GATE_MASK;
}

/**
 * @brief Use the previous channel's SPU pitch when updating this channel's pitch.
 * @param channel Channel to enable.
 * @see decomp.me (100%) https://decomp.me/scratch/8l07k
 */
void akao_seq_op_enable_pitch_sidechain(AkaoChannelState* channel)
{
    channel->flags |= AKAO_CH_PITCH_SIDECHAIN;
}

/**
 * @brief Disable the channel's pitch sidechain.
 * @param channel Channel to disable.
 * @see decomp.me (100%) https://decomp.me/scratch/YSIzI
 */
void akao_seq_op_disable_pitch_sidechain(AkaoChannelState* channel)
{
    channel->flags &= ~AKAO_CH_PITCH_SIDECHAIN;
}

/**
 * @brief Use the previous channel's SPU pitch when updating this channel's volume.
 * @param channel Channel to enable.
 * @see decomp.me (100%) https://decomp.me/scratch/iCXHA
 */
void akao_seq_op_enable_pitch_volume_sidechain(AkaoChannelState* channel)
{
    channel->flags |= AKAO_CH_PITCH_VOLUME_SIDECHAIN;
}

/**
 * @brief Disable the channel's pitch-to-volume sidechain.
 * @param channel Channel to disable.
 * @see decomp.me (100%) https://decomp.me/scratch/p5dcS
 */
void akao_seq_op_disable_pitch_volume_sidechain(AkaoChannelState* channel)
{
    channel->flags &= ~AKAO_CH_PITCH_VOLUME_SIDECHAIN;
}

/**
 * @brief Play up to two relative SFX sequences using the channel's pan and expression.
 * @param channel Channel whose bytecode cursor advances past two unsigned 16-bit offsets.
 * @note Each nonzero offset is relative to the byte after its operand; zero omits that sequence.
 * @see decomp.me (100%) https://decomp.me/scratch/nwIop
 */
void akao_seq_op_play_sfx(AkaoChannelState* channel)
{
    u8* cursor;
    s32 sequence_offset;
    u8* first_sequence;
    u8* second_sequence;

    cursor = channel->seq_cursor;

    sequence_offset = AKAO_READ_U16(cursor);

    if (sequence_offset != 0)
    {
        first_sequence = cursor + sequence_offset + AKAO_SFX_OFFSET_BYTES;
    }
    else
    {
        first_sequence = NULL;
    }

    cursor += AKAO_SFX_OFFSET_BYTES;

    sequence_offset = AKAO_READ_U16(cursor);
    if (sequence_offset != 0)
    {
        second_sequence = cursor + sequence_offset + AKAO_SFX_OFFSET_BYTES;
    }
    else
    {
        second_sequence = NULL;
    }

    g_akao_seq_sfx_params[AKAO_SEQ_SFX_ID].value = 0;
    g_akao_seq_sfx_params[AKAO_SEQ_SFX_TAG].value = 0;
    g_akao_seq_sfx_params[AKAO_SEQ_SFX_PAN_BIAS].value = channel->pan >> AKAO_Q8_SHIFT;
    g_akao_seq_sfx_params[AKAO_SEQ_SFX_VOLUME_SCALE].value = channel->expression >> AKAO_EXPRESSION_FRACTION_BITS;

    akao_sfx_play(g_akao_seq_sfx_params, first_sequence, second_sequence, 0);

    channel->seq_cursor += 2 * AKAO_SFX_OFFSET_BYTES;
}

/**
 * @brief Set the channel's pan bias immediately and enable reverb.
 * @param channel Channel whose bytecode cursor advances past the bias byte.
 * @param channel_bit Bit of the channel in the reverb mask.
 * @note Why this pan-bias opcode also enables reverb is unknown.
 * @see decomp.me (100%) https://decomp.me/scratch/B5HO1
 */
void akao_seq_op_set_pan_bias(AkaoChannelState* channel, s32 channel_bit)
{
    s32 pan_bias;

    pan_bias = *channel->seq_cursor++;

    channel->pan_bias_fade_ticks = 0;
    channel->flags |= AKAO_CH_PAN_BIAS;
    channel->pan_bias = pan_bias << AKAO_Q8_SHIFT;

    akao_seq_op_enable_reverb(channel, channel_bit);
}

/**
 * @brief Slide the channel's pan bias to a target and enable reverb.
 * @param channel Channel whose bytecode cursor advances past the duration and target bytes.
 * @param channel_bit Bit of the channel in the reverb mask.
 * @note Zero duration selects 256 ticks; why the opcode also enables reverb is unknown.
 * @see decomp.me (100%) https://decomp.me/scratch/fM3EA
 */
void akao_seq_op_slide_pan_bias(AkaoChannelState* channel, s32 channel_bit)
{
    u8* target_cursor;
    u16 current_bias;
    u32 fade_ticks;

    fade_ticks = *channel->seq_cursor++;

    channel->pan_bias_fade_ticks = fade_ticks;
    if (fade_ticks == 0)
    {
        channel->pan_bias_fade_ticks = AKAO_FADE_DEFAULT_TICKS;
    }

    current_bias = channel->pan_bias & AKAO_Q8_LEVEL_MASK;
    target_cursor = channel->seq_cursor;

    channel->pan_bias_step = ((s16)(*target_cursor << AKAO_Q8_SHIFT) - current_bias) / channel->pan_bias_fade_ticks;
    channel->pan_bias = current_bias;
    channel->seq_cursor = target_cursor + 1;
    channel->flags |= AKAO_CH_PAN_BIAS;

    akao_seq_op_enable_reverb(channel, channel_bit);
}

/**
 * @brief Allow a stop request to defer release until a repeat-loop opcode.
 * @param channel Channel to mark.
 * @see decomp.me (100%) https://decomp.me/scratch/ws5Yw
 */
void akao_seq_op_enable_deferred_stop(AkaoChannelState* channel)
{
    channel->flags |= AKAO_CH_DEFERRED_STOP;
}

/**
 * @brief Skip the single operand byte of this inert extended opcode.
 * @param channel Channel whose bytecode cursor advances by one byte.
 * @note The opcode's meaning in the authoring tool is unknown.
 * @see decomp.me (100%) https://decomp.me/scratch/ySVnh
 */
void akao_seq_op_skip_operand_byte(AkaoChannelState* channel)
{
    channel->seq_cursor++;
}

/**
 * @brief Let this channel allocate voices below the song's reserved base.
 * @param channel Unused; part of the opcode-handler signature.
 * @param channel_bit Bit of the channel in the low-voice allocation mask.
 * @see decomp.me (100%) https://decomp.me/scratch/q71gK
 */
void akao_seq_op_ignore_voice_reserve(AkaoChannelState* channel, s32 channel_bit)
{
    g_akao_seq_channel0->masks.voice_alloc_low_mask |= channel_bit;
}
