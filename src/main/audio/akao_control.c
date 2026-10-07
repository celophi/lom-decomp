/**
 * @file akao_control.c
 * @brief AKAO song and sound-effect control: start/stop/suspend of songs,
 *        SFX playback, volume/pan/pitch fades and the driver command dispatcher.
 * @note Its own object in the original link: the libspu objects S_SNC, S_GVEX
 *       and S_SRMD sit between akao_voice.o and this file.
 */
#include "internal/akao_voice.h"
#include "internal/akao_sequencer.h"
#include "internal/akao_control.h"
#include <libspu.h>
#include <libapi.h>

/**
 * @brief Resolve one u16 offset of an SFX list entry pair.
 * @note Offsets are relative to the end of the 4-byte entry pair; @p skip is the
 *       distance from @p entry to that end. The offset is added to the entry
 *       address as an integer.
 */
#define SFX_ENTRY_DATA(entry, skip) ((u8*)((u32)(*(u16*)(entry)) + (uintptr_t)(entry) + (skip)))

/** @brief Initial channel expression, volume scale and song tempo accumulators. */
#define AKAO_SONG_INITIAL_EXPRESSION 0x3FFF0000
#define AKAO_SONG_INITIAL_VOLUME_SCALE 0x4000
#define AKAO_SONG_INITIAL_TEMPO 0xFFFF0000

/** @brief Fractional bits in a song master-volume accumulator. */
#define AKAO_SONG_VOLUME_SHIFT 16

/** @brief Fractional bits in the CD volume accumulator; its high half feeds the SPU. */
#define AKAO_CD_VOLUME_SHIFT 16

/** @brief Fractional bits in the driver master pan and volume accumulators. */
#define AKAO_MASTER_FIXED_POINT_SHIFT 16

/** @brief Startup delays for channels selected to play. */
#define AKAO_SONG_START_NOTE_TICKS 4
#define AKAO_SONG_START_GATE_TICKS 2

/** @brief Extra note and gate ticks before a restored channel resumes. */
#define AKAO_SONG_RESUME_DELAY_TICKS 2

/** @brief Restore volume, pitch, sample addresses and ADSR registers on resumed voices. */
#define AKAO_SONG_RESUME_VOICE_UPDATES 0x1FF93

/** @brief Delays for channels started on the silent sequence. */
#define AKAO_SONG_SILENT_NOTE_TICKS 3
#define AKAO_SONG_SILENT_GATE_TICKS 1

/** @brief ADSR release shift used when silencing a song or reusing an SFX voice. */
#define AKAO_VOICE_STOP_RELEASE_SHIFT 5

/** @brief Attack and sustain rate fields installed while voices are paused. */
#define AKAO_PAUSE_ENVELOPE_RATE 0x7F

/** @brief Restore volume, pitch and ADSR registers after resuming paused channels. */
#define AKAO_PAUSE_RESUME_VOICE_UPDATES 0x2B13

/** @brief Fractional bits in SFX pan, volume and pitch-bend accumulators. */
#define AKAO_SFX_FIXED_POINT_SHIFT 8

/** @brief Initial SFX note and gate countdowns. */
#define AKAO_SFX_START_NOTE_TICKS 2
#define AKAO_SFX_START_GATE_TICKS 1

/** @brief Initial SFX age, incremented on each unpaused SFX tick. */
#define AKAO_SFX_INITIAL_AGE (-2)

/** @brief Sound id and stop-selection tag used when playing supplied SFX sequences with defaults. */
#define AKAO_SFX_DEFAULT_ID 0x400
#define AKAO_SFX_DEFAULT_TAG 0x1000000

/** @brief Pending ADSR release-rate updates and mask clearing its five rate bits. */
#define AKAO_RELEASE_RATE_UPDATE_PENDING 0x4400
#define AKAO_RELEASE_RATE_CLEAR_MASK 0xFFE0

/** @brief Mask selecting every sequencer channel. */
#define AKAO_ALL_CHANNELS_MASK (-1)

/** @brief First song channel, included in every partial start request. */
#define AKAO_SONG_FIRST_CHANNEL_BIT 1

/** @brief Stop-mode bits selecting channels by overlapping SFX tags. */
#define AKAO_SFX_STOP_TAG_MASK 0x0FFFFFFF
/** @brief Stop mode selecting the sound ids on an adjacent channel pair. */
#define AKAO_SFX_STOP_PAIR 0x80000000U
/** @brief Sound-id selector matching every negative SFX id. */
#define AKAO_SFX_STOP_NEGATIVE_IDS (-1)

/** @brief Program-table offset marking an absent SFX sequence. */
#define AKAO_PROGRAM_NO_SEQUENCE 0xFFFF

/** @brief Number of sequence offsets stored per bank program. */
#define AKAO_PROGRAM_SEQUENCE_COUNT 2

/**
 * @brief Header of an SFX list; entry offsets are relative to the data after this header.
 */
typedef struct
{
    u32 magic;
    s32 entry_count;
    s32 bank_key;
    u8 reserved[4];
    s32 entry_offsets[4];
} AkaoSfxListHeader;

/** @brief Sequence offsets relative to the end of an SFX list entry; 0xFFFF marks an absent sequence. */
typedef struct
{
    u16 first_offset;
    u16 second_offset;
} AkaoSfxSequencePair;

/** @brief Reference compared with descriptor unk14 to select the song's area flags. */
extern s32 g_akao_song_descriptor_match_value[];
extern u8 g_akao_silent_sequence[];
extern AkaoCommandParam g_akao_dispatch_params[6];
extern void (*g_akao_command_handlers[256])(AkaoCommandParam*);

void akao_channel_set_articulation(AkaoChannelState* channel, s32 articulation_index);
u32 akao_collect_voice_mask(AkaoChannelState* channels, s32 channel_mask);
void akao_sfx_release_channels(void* channel, u32 release_mask);

/**
 * @brief Reset a channel's playback state and point it at new sequence bytecode.
 * @param channel Channel to reset.
 * @param seq_data Sequence bytecode the channel starts executing.
 */
void akao_channel_init_state(AkaoChannelState* channel, u8* seq_data)
{
    channel->volume = 0x6E00;
    channel->expression = 0x32000000;
    channel->seq_cursor = seq_data;
    channel->transpose = 0;
    channel->detune = 0;
    channel->portamento_speed = 0;
    channel->pitch_slide_acc = 0;
    channel->pitch_slide_delta = 0;
    channel->pitch_slide_ticks = 0;
    channel->note_duration_adjust = 0;
    channel->note_duration = 0;
    channel->expression_fade_ticks = 0;
    channel->detune_pitch_delta = 0;
    channel->pitch_scale = 0;
    channel->loop_depth = 0;
    channel->flags = 0;
    channel->pan_lfo_value = 0;
    channel->note_flags = 0;
    channel->opcode_count = 0xFFFF;
    channel->spu_volume_scale = 0;
    channel->pan_lfo_depth = 0;
    channel->volume_lfo_depth = 0;
    channel->pitch_lfo_depth = 0;
    channel->pan_lfo_depth_fade_ticks = 0;
    channel->volume_lfo_depth_fade_ticks = 0;
    channel->pitch_lfo_depth_fade_ticks = 0;
    channel->pitch_mod_toggle_ticks = 0;
    channel->noise_toggle_ticks = 0;
    akao_channel_set_articulation(channel, 0);
}

/**
 * @brief Build the SPU voice bitmap of every selected channel that owns a voice.
 * @param channels First channel corresponding to channel_mask bit zero.
 * @param channel_mask Channels to scan.
 * @return Bitmap of the SPU voices held by the selected channels.
 */
u32 akao_collect_voice_mask(AkaoChannelState* channels, s32 channel_mask)
{
    u32 i;
    u32 voice;
    u32 result;
    s32 bit;

    for (i = 0, result = 0; i < AKAO_CHANNEL_COUNT; i++)
    {
        bit = 1 << i;
        if (channel_mask & bit)
        {
            voice = channels->voice;
            if (voice < AKAO_VOICE_COUNT)
            {
                result |= 1 << voice;
            }
        }
        channels++;
    }
    return result;
}

/**
 * @brief Prepare the primary song and initialize its channels for playback.
 *
 * Selected descriptor channels receive fresh playback state. Other channels
 * run the silent sequence while releasing their notes. Channel-offset entries
 * exist only for bits present in the descriptor's channel mask.
 *
 * @param sequence_data AKAO song descriptor and its sequence data.
 * @param start_mask Descriptor channels to start; selected channels are parked
 *        instead when song playback is paused.
 */
void akao_seq_start_song(u8* sequence_data, s32 start_mask)
{
    s32 remaining_channels;
    s32 flags;
    s32 cleared_flags;
    s32 table_offset;
    u8* resolved_table;
    u8* key_map_data;
    u8* note_table_data;
    AkaoSongDescriptor* descriptor;
    AkaoChannelState* channel;
    u32 channel_index;
    s32 bit_mask;
    s32 static_voice_mask;
    u8* silent_sequence;
    s32 song_paused;
    AkaoSongState* table_owner;
    AkaoSongState* song;

    g_akao_seq_channel0->song_data = sequence_data;
    descriptor = (AkaoSongDescriptor*)sequence_data;
    remaining_channels = descriptor->channel_mask;

    if (g_akao_seq_channel1 != NULL)
    {
        bit_mask = akao_collect_voice_mask(g_akao_pending_channels, g_akao_seq_channel1->masks.active_mask);
    }
    else
    {
        bit_mask = 0;
    }

    g_akao_sfx_control.key_off_mask |= (~bit_mask & AKAO_VOICE_MASK) & ~(g_akao_sfx_control.active_mask | g_akao_xa_tracker.voice_mask);
    song_paused = g_akao_driver_mode_flags & AKAO_MODE_SONG_PAUSED;

    g_akao_seq_channel0->key_off_mask = 0;

    if (song_paused)
    {
        g_akao_seq_channel0->masks.active_mask = 0;
        g_akao_seq_channel0->parked_mask |= remaining_channels & start_mask;
    }
    else
    {
        g_akao_seq_channel0->parked_mask = 0;
        g_akao_seq_channel0->masks.active_mask |= remaining_channels & start_mask;
    }

    g_akao_seq_channel0->masks.voice_alloc_low_mask = descriptor->voice_alloc_low_mask;
    song = g_akao_seq_channel0;
    song->masks.static_voice_mask = descriptor->static_voice_mask;

    flags = song->flags;
    cleared_flags = flags & ~(AKAO_SONG_DESCRIPTOR_FLAGS | AKAO_SONG_VOICE_DROPPED | AKAO_SONG_VOICE_STOLEN);
    song->flags = cleared_flags;
    flags = descriptor->unk14;
    if (flags == g_akao_song_descriptor_match_value[0])
    {
        flags = cleared_flags | AKAO_SONG_LOWER_XA_AREA_FLAG;
    }
    else
    {
        flags = cleared_flags | AKAO_SONG_DESCRIPTOR_MISMATCH;
    }
    song->flags = flags;

    resolved_table = NULL;
    table_offset = descriptor->key_map_offset;
    table_owner = g_akao_seq_channel0;
    key_map_data = (u8*)&descriptor->key_map_offset + table_offset;
    if (table_offset != 0)
    {
        resolved_table = key_map_data;
    }
    table_owner->key_map_base = resolved_table;

    resolved_table = NULL;
    table_offset = descriptor->note_table_offset;
    note_table_data = (u8*)&descriptor->note_table_offset + table_offset;
    if (table_offset != 0)
    {
        resolved_table = note_table_data;
    }

    bit_mask = 1;
    channel_index = 0;
    channel = g_akao_seq_channels;
    sequence_data = (u8*)((AkaoSongDescriptor*)sequence_data)->channel_offsets;
    silent_sequence = g_akao_silent_sequence;

    table_owner->note_table = resolved_table;
    table_owner->voice_alloc_base = 0;
    do
    {
        if ((remaining_channels & bit_mask) & start_mask)
        {
            channel->seq_cursor = sequence_data + *(u16*)sequence_data;
            channel->note_ticks = AKAO_SONG_START_NOTE_TICKS;
            channel->gate_ticks = AKAO_SONG_START_GATE_TICKS;
            channel->volume = AKAO_VOLUME_MAX << 8;
            channel->expression = AKAO_SONG_INITIAL_EXPRESSION;
            channel->volume_scale = AKAO_SONG_INITIAL_VOLUME_SCALE;
            channel->detune = 0;
            channel->transpose = 0;
            channel->portamento_speed = 0;
            channel->pitch_slide_acc = 0;
            channel->pitch_slide_delta = 0;
            channel->pitch_slide_ticks = 0;
            channel->note_duration_adjust = 0;
            channel->note_duration = 0;
            channel->pan = AKAO_PAN_CENTER << 8;
            channel->pan_fade_ticks = 0;
            channel->portamento_speed = 0;
            channel->volume_scale_fade_ticks = 0;
            channel->expression_fade_ticks = 0;
            channel->detune_pitch_delta = 0;
            channel->note_expression_ticks = 0;
            channel->pitch_scale = 0;
            channel->note_flags = 0;
            channel->pan_lfo_value = 0;
            channel->loop_depth = 0;
            static_voice_mask = g_akao_seq_channel0->masks.static_voice_mask;
            sequence_data += sizeof(*descriptor->channel_offsets);
            channel->pan_lfo_depth = 0;
            channel->volume_lfo_depth = 0;
            channel->pitch_lfo_depth = 0;
            channel->pan_lfo_depth_fade_ticks = 0;
            channel->flags = static_voice_mask & bit_mask;
            channel->flags = channel->flags == 0 ? AKAO_CH_FULL_GATE : 0;
            channel->volume_lfo_depth_fade_ticks = 0;
            channel->pitch_lfo_depth_fade_ticks = 0;
            channel->pitch_mod_toggle_ticks = 0;
            channel->noise_toggle_ticks = 0;
            akao_channel_set_articulation(channel, 0);
        }
        else
        {
            if ((remaining_channels & bit_mask) && !(bit_mask & start_mask))
            {
                sequence_data += sizeof(*descriptor->channel_offsets);
            }
            channel->note_ticks = AKAO_SONG_SILENT_NOTE_TICKS;
            channel->gate_ticks = AKAO_SONG_SILENT_GATE_TICKS;
            channel->seq_cursor = silent_sequence;
            channel->update_flags |= AKAO_RELEASE_RATE_UPDATE_PENDING;
            channel->spu_adsr_high = (channel->spu_adsr_high & AKAO_RELEASE_RATE_CLEAR_MASK) | AKAO_VOICE_STOP_RELEASE_SHIFT;
        }
        channel->voice = AKAO_VOICE_COUNT;
        remaining_channels &= ~bit_mask;
        channel++;
        channel_index++;
        bit_mask <<= 1;
    } while (channel_index < AKAO_CHANNEL_COUNT);

    g_akao_seq_channel0->tempo = AKAO_SONG_INITIAL_TEMPO;
    g_akao_seq_channel0->tempo_acc = 1;
    g_akao_seq_channel0->tempo_fade_ticks = 0;
    g_akao_seq_channel0->reverb_depth = 0;
    g_akao_seq_channel0->reverb_depth_fade_ticks = 0;
    g_akao_seq_channel0->reverb_depth_step = 0;
    g_akao_driver_flags.update_flags = 0;
    g_akao_seq_channel0->tick = 0;
    g_akao_seq_channel0->ticks_per_beat = 0;
    g_akao_seq_channel0->beat = 0;
    g_akao_seq_channel0->measure = 0;
    g_akao_seq_channel0->noise_mask = 0;
    g_akao_seq_channel0->reverb_mask = 0;
    g_akao_seq_channel0->pitch_mod_mask = 0;
    g_akao_seq_channel0->condition = 0;
    g_akao_seq_channel0->note_on_mask = 0;
    g_akao_seq_channel0->masks.key_on_mask = 0;
    g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
}

/**
 * @brief Stop an active song by silencing its channels and releasing its voices.
 *
 * Songs with no active channels are left untouched. A nonzero key must match
 * the song's id. Matching songs queue key-off for all channels and relinquish
 * their SPU voices with a linear release.
 *
 * @param song Song state to stop.
 * @param channels Channel table owned by @p song.
 * @param song_key Required song id, or 0 to stop any active song.
 */
void akao_seq_stop_song(AkaoSongState* song, AkaoChannelState* channels, s32 song_key)
{
    u32 index;

    if (song->masks.active_mask == 0)
    {
        return;
    }
    if (song_key != 0 && song_key != song->song_id)
    {
        return;
    }
    song->key_off_mask = AKAO_ALL_CHANNELS_MASK;

    for (index = AKAO_CHANNEL_COUNT; index != 0; index--)
    {
        channels->note_ticks = AKAO_SONG_SILENT_NOTE_TICKS;
        channels->gate_ticks = AKAO_SONG_SILENT_GATE_TICKS;
        channels->seq_cursor = g_akao_silent_sequence;
        channels++;
    }

    song->song_id = 0;
    song->note_on_mask = 0;
    song->masks.key_on_mask = 0;

    for (index = 0; index < AKAO_VOICE_COUNT; index++)
    {
        if (g_akao_voice_owners[index] == song)
        {
            g_akao_voice_owners[index] = NULL;
            spu_set_voice_release_mode(index, AKAO_VOICE_STOP_RELEASE_SHIFT, SPU_VOICE_LINEARDecN);
        }
    }
}

/**
 * @brief Stop SFX selected by tag, channel pair, age or sound id.
 *
 * Considers active and paused channels. Tag selection takes precedence over
 * pair selection, then oldest-channel selection, then sound-id matching.
 * Deferred-stop channels record a pending stop; other matches release now.
 *
 * @param sfx_id Sound id to match, AKAO_SFX_STOP_NEGATIVE_IDS for negative ids,
 *        or the first channel index when pair selection is requested.
 * @param mode Tag mask, AKAO_SFX_STOP_PAIR, AKAO_SFX_STOP_OLDEST, or 0 for id matching.
 */
void akao_sfx_stop_channels(s32 sfx_id, s32 mode)
{
    AkaoChannelState* channel;
    s32 channel_bit;
    u32 channel_index;
    s32 active_mask;
    s32 flags;
    s32 oldest_age;
    s32 channel_age;

    channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    channel = g_sfx_channels;
    active_mask = g_akao_sfx_control.active_mask | g_akao_sfx_control.paused_mask;

    if (mode & AKAO_SFX_STOP_TAG_MASK)
    {
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if ((active_mask & channel_bit) && (channel->sfx_tag & mode))
            {
                flags = channel->flags;
                if (flags & AKAO_CH_DEFERRED_STOP)
                {
                    channel->flags = flags | AKAO_CH_STOP_PENDING;
                }
                else
                {
                    g_akao_sfx_control.key_off_mask |= channel_bit;
                    akao_sfx_release_channels(channel, channel_bit);
                    channel->flags = 0;
                }
            }
        }
    }
    else if (mode & AKAO_SFX_STOP_PAIR)
    {
        channel += sfx_id;
        channel_bit <<= sfx_id;
        if (active_mask & channel_bit)
        {
            akao_sfx_stop_channels(channel->sfx_id, 0);
        }
        channel_bit <<= 1;
        channel++;
        if (active_mask & channel_bit)
        {
            akao_sfx_stop_channels(channel->sfx_id, 0);
        }
        return;
    }
    else if (mode & AKAO_SFX_STOP_OLDEST)
    {
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if (channel->sfx_tag != 0)
            {
                active_mask &= ~channel_bit;
            }
        }

        channel = g_sfx_channels;
        channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
        oldest_age = 0;
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if (active_mask & channel_bit)
            {
                channel_age = channel->sfx_age;
                if (oldest_age < channel_age)
                {
                    oldest_age = channel_age;
                }
            }
        }

        channel = g_sfx_channels;
        channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if ((active_mask & channel_bit) && (oldest_age == channel->sfx_age))
            {
                flags = channel->flags;
                if (flags & AKAO_CH_DEFERRED_STOP)
                {
                    channel->flags = flags | AKAO_CH_STOP_PENDING;
                }
                else
                {
                    g_akao_sfx_control.key_off_mask |= channel_bit;
                    akao_sfx_release_channels(channel, channel_bit);
                    channel->flags = 0;
                }
            }
        }
    }
    else
    {
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if (active_mask & channel_bit)
            {
                if (sfx_id == AKAO_SFX_STOP_NEGATIVE_IDS)
                {
                    if ((s32)channel->sfx_id < 0)
                    {
                        flags = channel->flags;
                        if (flags & AKAO_CH_DEFERRED_STOP)
                        {
                            channel->flags = flags | AKAO_CH_STOP_PENDING;
                        }
                        else
                        {
                            g_akao_sfx_control.key_off_mask |= channel_bit;
                            akao_sfx_release_channels(channel, channel_bit);
                            channel->flags = 0;
                        }
                    }
                }
                else if ((s32)channel->sfx_id == sfx_id)
                {
                    flags = channel->flags;
                    if (flags & AKAO_CH_DEFERRED_STOP)
                    {
                        channel->flags = flags | AKAO_CH_STOP_PENDING;
                    }
                    else
                    {
                        g_akao_sfx_control.key_off_mask |= channel_bit;
                        akao_sfx_release_channels(channel, channel_bit);
                        channel->flags = 0;
                    }
                }
            }
        }
    }
    g_akao_driver_flags.update_flags |= (AKAO_EFFECT_MASKS_UPDATE_PENDING | AKAO_NOISE_CLOCK_UPDATE_PENDING);
}

/**
 * @brief Initialize SFX playback and release the channel's previous SPU voice.
 *
 * Queues key-off and clears pending note-on and effect masks for this channel.
 * When SFX playback is paused, parks every active channel without the
 * AKAO_SFX_FLAG_SUPPRESS exemption.
 *
 * @param channel SFX channel to start.
 * @param params Play parameters: sound id, tag, pan bias, volume scale and bank slot.
 * @param channel_mask Channel-mask bit of @p channel.
 * @param sequence_data Sequence bytecode the channel plays.
 */
void akao_sfx_start_channel(AkaoChannelState* channel, AkaoCommandParam* params, s32 channel_mask, u8* sequence_data)
{
    s32 channels_remaining;

    channel->sfx_id = params[0].value;
    channel->sfx_tag = params[1].value;
    channel->pan_bias = (u8)params[2].value << AKAO_SFX_FIXED_POINT_SHIFT;
    channel->pan_bias_fade_ticks = 0;
    channel->pan = AKAO_PAN_CENTER << AKAO_SFX_FIXED_POINT_SHIFT;
    channel->pan_fade_ticks = 0;
    channel->volume_scale = ((u16)params[3].value & AKAO_VOLUME_MAX) << AKAO_SFX_FIXED_POINT_SHIFT;
    channel->volume_scale_fade_ticks = 0;
    channel->sfx_bank = params[4].value;
    channel->note_ticks = AKAO_SFX_START_NOTE_TICKS;
    channel->gate_ticks = AKAO_SFX_START_GATE_TICKS;
    channel->is_sfx_channel = 1;
    channel->sfx_age = AKAO_SFX_INITIAL_AGE;
    channel->sfx_pitch_bend = 0;
    channel->sfx_pitch_bend_fade_ticks = 0;
    akao_channel_init_state(channel, sequence_data);

    g_akao_voice_owners[channel->voice] = NULL;
    spu_set_voice_release_mode(channel->voice, AKAO_VOICE_STOP_RELEASE_SHIFT, SPU_VOICE_LINEARDecN);

    g_akao_sfx_control.active_mask |= channel_mask;
    g_akao_sfx_control.key_off_mask |= channel_mask;
    channel_mask = ~channel_mask;
    g_akao_sfx_control.key_on_mask &= channel_mask;
    g_akao_sfx_control.note_on_mask &= channel_mask;
    g_akao_sfx_control.noise_mask &= channel_mask;
    g_akao_sfx_control.reverb_mask &= channel_mask;
    g_akao_sfx_control.pitch_mod_mask &= channel_mask;

    if (g_akao_driver_mode_flags & AKAO_MODE_SFX_PAUSED)
    {
        channel_mask = AKAO_SFX_FIRST_CHANNEL_BIT;
        channel = g_sfx_channels;
        for (channels_remaining = AKAO_SFX_CHANNEL_COUNT; channels_remaining != 0; channels_remaining--, channel++, channel_mask <<= 1)
        {
            if ((g_akao_sfx_control.active_mask & channel_mask) && !(channel->sfx_tag & AKAO_SFX_FLAG_SUPPRESS))
            {
                g_akao_sfx_control.active_mask &= ~channel_mask;
                g_akao_sfx_control.paused_mask |= channel_mask;
            }
        }
    }
}

/**
 * @brief Detach an SPU voice from every sequence channel that owns it.
 *
 * Marks matching channels as unassigned and clears their sounding-note bits
 * in the current song. Voice indices outside the SPU range are ignored.
 *
 * @param channels Table of AKAO_CHANNEL_COUNT channels belonging to the current song.
 * @param voice_index SPU voice being taken over, or AKAO_VOICE_COUNT if unassigned.
 */
void akao_unassign_voice(AkaoChannelState* channels, u32 voice_index)
{
    u32 channel_index;
    s32 unassigned_voice;
    u32 channel_bit;
    AkaoSongState* song;

    if (voice_index >= AKAO_VOICE_COUNT)
    {
        return;
    }

    channel_index = 0;
    unassigned_voice = AKAO_VOICE_COUNT;
    song = g_akao_seq_channel0;
    for (; channel_index < AKAO_CHANNEL_COUNT; channel_index++, channels++)
    {
        if (channels->voice == voice_index)
        {
            channel_bit = 1U << channel_index;
            channels->voice = unassigned_voice;
            song->note_on_mask &= ~channel_bit;
        }
    }
}

/**
 * @brief Allocate SFX channels and start one or two sequences.
 *
 * Searches from the highest SFX channel down, requiring an adjacent pair when
 * both sequences are present. If none are free, stops the oldest untagged SFX
 * and retries while the occupied-channel mask changes. Sequence channels lose
 * any SPU voices taken over by the new SFX.
 *
 * @param params Play parameters (see akao_sfx_start_channel); word 1 also selects channels to stop first.
 * @param first_sequence First sequence, or NULL.
 * @param second_sequence Second sequence, or NULL; plays alone if the first is NULL.
 * @param skip_stop Nonzero to skip the initial stop request selected by params word 1.
 */
void akao_sfx_play(AkaoCommandParam* params, u8* first_sequence, u8* second_sequence, s32 skip_stop)
{
    AkaoChannelState* channel;
    u32 channel_bit;
    u32 second_channel_bit;
    u32 channel_pair_mask;
    u32 busy_mask;
    s32 candidates_remaining;
    s32 tag_mask;

    if (first_sequence == NULL && second_sequence == NULL)
    {
        return;
    }

    if (skip_stop == 0)
    {
        tag_mask = params[1].value;
        if (tag_mask != 0)
        {
            akao_sfx_stop_channels(0, tag_mask);
        }
    }

    do
    {
        channel = &g_sfx_channels[AKAO_SFX_CHANNEL_COUNT - 1];
        channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT << (AKAO_SFX_CHANNEL_COUNT - 1);
        busy_mask = (g_akao_sfx_control.active_mask | g_akao_sfx_control.paused_mask) | g_akao_xa_tracker.voice_mask;
        if (first_sequence != NULL && second_sequence != NULL)
        {
            candidates_remaining = AKAO_SFX_CHANNEL_COUNT - 1;
            channel--;
            channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT << (AKAO_SFX_CHANNEL_COUNT - 2);
            while (1)
            {
                second_channel_bit = channel_bit << 1;
                channel_pair_mask = channel_bit | second_channel_bit;
                if (busy_mask & channel_pair_mask)
                {
                    candidates_remaining--;
                    channel--;
                    channel_bit >>= 1;
                    if (candidates_remaining == 0)
                    {
                        break;
                    }
                    continue;
                }
                break;
            }
        }
        else
        {
            candidates_remaining = AKAO_SFX_CHANNEL_COUNT;
            for (; candidates_remaining != 0; candidates_remaining--, channel--, channel_bit >>= 1)
            {
                if (!(busy_mask & channel_bit))
                {
                    break;
                }
            }
        }
        if (candidates_remaining != 0)
        {
            break;
        }
        akao_sfx_stop_channels(0, AKAO_SFX_STOP_OLDEST);
        if (busy_mask == ((g_akao_sfx_control.active_mask | g_akao_sfx_control.paused_mask) | g_akao_xa_tracker.voice_mask))
        {
            return;
        }
    } while (candidates_remaining == 0);

    if (first_sequence != NULL)
    {
        akao_sfx_start_channel(channel, params, channel_bit, first_sequence);
        akao_unassign_voice(g_akao_seq_channels, channel->voice);
    }
    if (second_sequence != NULL)
    {
        if (first_sequence != NULL)
        {
            channel++;
            channel_bit <<= 1;
        }
        akao_sfx_start_channel(channel, params, channel_bit, second_sequence);
        akao_unassign_voice(g_akao_seq_channels, channel->voice);
        if (first_sequence != NULL)
        {
            channel->flags |= AKAO_CH_SFX_PITCH_MOD;
        }
    }
    g_akao_driver_flags.update_flags |= (AKAO_EFFECT_MASKS_UPDATE_PENDING | AKAO_NOISE_CLOCK_UPDATE_PENDING);
}

/**
 * @brief Resolve both SFX sequences from a bank program's offset pair.
 *
 * Each program stores two u16 offsets relative to g_akao_bank_region_c.
 * AKAO_PROGRAM_NO_SEQUENCE marks an absent sequence and resolves to NULL.
 *
 * @param out_first_sequence Receives the first sequence pointer, or NULL.
 * @param out_second_sequence Receives the second sequence pointer, or NULL.
 * @param program_index Bank program index; only the low 10 bits are used.
 */
void akao_resolve_program_data(u8** out_first_sequence, u8** out_second_sequence, s32 program_index)
{
    u8* sequence_data;

    program_index &= AKAO_SFX_ID_MASK;
    program_index *= AKAO_PROGRAM_SEQUENCE_COUNT;

    if (((u16*)g_akao_bank_prog_base)[program_index] != AKAO_PROGRAM_NO_SEQUENCE)
    {
        sequence_data = g_akao_bank_region_c + ((u16*)g_akao_bank_prog_base)[program_index];
    }
    else
    {
        sequence_data = NULL;
    }
    *out_first_sequence = sequence_data;

    program_index++;
    if (((u16*)g_akao_bank_prog_base)[program_index] != AKAO_PROGRAM_NO_SEQUENCE)
    {
        sequence_data = g_akao_bank_region_c + ((u16*)g_akao_bank_prog_base)[program_index];
    }
    else
    {
        sequence_data = NULL;
    }
    *out_second_sequence = sequence_data;
}

/**
 * @brief Mark active song channels for a pending SPU stereo volume update.
 * @param song Song whose active mask selects the channels to update.
 * @param channels Song channel table; its first entry corresponds to active-mask bit zero.
 */
void akao_seq_flag_volume_update(AkaoSongState* song, AkaoChannelState* channels)
{
    u32 remaining_channels;
    u32 channel_bit;
    u32 active_channels;

    active_channels = song->masks.active_mask;
    if (active_channels != 0)
    {
        remaining_channels = active_channels;
        channel_bit = 1U;
        do
        {
            if (remaining_channels & channel_bit)
            {
                remaining_channels ^= channel_bit;
                channels->update_flags |= SPU_UPDATE_VOLUME;
            }
            channels++;
            channel_bit <<= 1;
        } while (remaining_channels != 0);
    }
}

/**
 * @brief Mark active SFX channels for a pending SPU stereo volume update.
 */
void akao_sfx_flag_volume_update(void)
{
    u32 remaining_channels;
    u32 channel_bit;
    u32 active_channels;
    AkaoChannelState* channel;

    channel = g_sfx_channels;
    active_channels = g_akao_sfx_control.active_mask;
    if (active_channels != 0)
    {
        remaining_channels = active_channels;
        channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
        do
        {
            if (remaining_channels & channel_bit)
            {
                remaining_channels ^= channel_bit;
                channel->update_flags |= SPU_UPDATE_VOLUME;
            }
            channel++;
            channel_bit <<= 1;
        } while (remaining_channels != 0);
    }
}

/**
 * @brief Restore the suspended primary song and rebase its saved sequence pointers.
 *
 * Requeues sounding notes and releases unused voices, preserving the global
 * song-pause state.
 *
 * @param descriptor Reloaded descriptor of the suspended song, possibly at a new address.
 */
void akao_seq_resume_song(AkaoSongDescriptor* descriptor)
{
    AkaoChannelState* channel;
    AkaoSongState* song;
    u32 song_flags;
    intptr_t relocation_delta;
    u32 active_mask;
    u32 channel_bit;
    u32 channels_remaining;
    u32 voice_mask_limit;

    akao_copy_bytes((s32*)&g_akao_suspended_song, (s32*)g_akao_seq_channel0, sizeof(AkaoSongState));
    akao_copy_bytes((s32*)g_akao_suspended_channels, (s32*)g_akao_seq_channels, AKAO_CHANNEL_COUNT * sizeof(AkaoChannelState));

    song_flags = g_akao_seq_channel0->flags & ~AKAO_SONG_DESCRIPTOR_FLAGS;
    g_akao_seq_channel0->flags = song_flags;
    song = g_akao_seq_channel0;
    if (descriptor->unk14 == g_akao_song_descriptor_match_value[0])
    {
        song_flags |= AKAO_SONG_LOWER_XA_AREA_FLAG;
    }
    else
    {
        song_flags |= AKAO_SONG_DESCRIPTOR_MISMATCH;
    }
    song->flags = song_flags;
    g_akao_seq_channel0->song_data = (u8*)descriptor;
    g_akao_seq_channel0->masks.key_on_mask = 0;

    g_akao_driver_flags.update_flags |= (AKAO_REVERB_DEPTH_UPDATE_PENDING | AKAO_NOISE_CLOCK_UPDATE_PENDING);

    active_mask = g_akao_seq_channel0->masks.active_mask;
    relocation_delta = (intptr_t)descriptor - (intptr_t)g_akao_suspended_song.song_data;
    g_akao_seq_channel0->key_map_base += relocation_delta;
    g_akao_seq_channel0->note_table += relocation_delta;
    g_akao_seq_channel0->masks.key_on_mask = g_akao_seq_channel0->note_on_mask;

    channel = g_akao_seq_channels;
    for (channels_remaining = AKAO_CHANNEL_COUNT, channel_bit = 1U; channels_remaining != 0; channels_remaining--, channel++, channel_bit <<= 1)
    {
        if (active_mask & channel_bit)
        {
            channel->seq_cursor += relocation_delta;
            channel->key_map += relocation_delta;
            channel->loop_cursor[0] += relocation_delta;
            channel->loop_cursor[1] += relocation_delta;
            channel->loop_cursor[2] += relocation_delta;
            channel->loop_cursor[3] += relocation_delta;
            channel->note_ticks += AKAO_SONG_RESUME_DELAY_TICKS;
            channel->gate_ticks += AKAO_SONG_RESUME_DELAY_TICKS;
            channel->update_flags |= AKAO_SONG_RESUME_VOICE_UPDATES;
        }
        else
        {
            channel->note_ticks = AKAO_SONG_START_NOTE_TICKS;
            channel->gate_ticks = AKAO_SONG_START_GATE_TICKS;
            channel->seq_cursor = g_akao_silent_sequence;
        }
        channel->voice = AKAO_VOICE_COUNT;
    }

    /* Preserve voices used by the secondary song, SFX and XA playback. */
    active_mask = 0;
    if (g_akao_seq_channel1 != NULL)
    {
        active_mask =
            akao_collect_voice_mask(g_akao_pending_channels, g_akao_seq_channel1->masks.active_mask & g_akao_seq_channel1->masks.voice_alloc_low_mask);
    }

    g_akao_seq_channel0->key_off_mask = 0;
    g_akao_suspended_song.song_id = 0;
    voice_mask_limit = AKAO_VOICE_MASK;
    g_akao_sfx_control.key_off_mask |= (~active_mask & (~(g_akao_sfx_control.active_mask | g_akao_xa_tracker.voice_mask) & voice_mask_limit));
    g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;

    if (g_akao_driver_mode_flags & AKAO_MODE_SONG_PAUSED)
    {
        g_akao_seq_channel0->parked_mask = g_akao_seq_channel0->masks.active_mask;
        g_akao_seq_channel0->masks.active_mask = 0;
    }
}

/**
 * @brief Find the highest-numbered bank slot holding a key.
 * @param key Bank key to search for; 0 skips the search.
 * @return Highest matching slot index, or slot 0 for a zero or missing key.
 */
s32 akao_bank_find_slot(s32 key)
{
    s32 slots_remaining;
    s32 next_slot_index;
    s32* slot_key;
    s32* slot_keys;

    slots_remaining = 0;
    if (key != 0)
    {
        slots_remaining = AKAO_BANK_SLOT_COUNT;
        slot_key = &g_akao_bank_slot_keys[AKAO_BANK_SLOT_COUNT - 1];
        do
        {
            if (key == *slot_key)
            {
                slots_remaining--;
                break;
            }

            slots_remaining--;
            if (slots_remaining != 0)
            {
                slot_keys = g_akao_bank_slot_keys;
                next_slot_index = slots_remaining - 1;
                slot_key = slot_keys + next_slot_index;
            }
        } while (slots_remaining != 0);
    }

    return slots_remaining;
}

/**
 * @brief Resume a matching suspended song or start the loaded song from the beginning.
 *
 * A nonzero suspended song id must match the requested id to resume. Otherwise,
 * start all channels selected by the descriptor and record the requested id.
 *
 * @param params Load result: descriptor in params[0].buffer and song id in params[2].value.
 */
void akao_seq_reload_song(AkaoCommandParam* params)
{
    if (g_akao_suspended_song.song_id != 0 && g_akao_suspended_song.song_id == params[2].value)
    {
        akao_seq_resume_song(params[0].buffer);
    }
    else
    {
        akao_seq_start_song(params[0].buffer, AKAO_ALL_CHANNELS_MASK);
        g_akao_seq_channel0->song_id = (u16)params[2].value;
    }
}

/**
 * @brief Start a loaded song on selected channels and seed its pending advance count.
 *
 * The sequencer consumes the pending count at measure boundaries. Zero leaves
 * advancement disabled; other requests are reduced by one.
 *
 * @param params Load result: descriptor in params[0].buffer, song id in params[2].value,
 *        channel mask in params[3].value and advance count in params[4].value.
 */
void akao_seq_start_loaded_song(AkaoCommandParam* params)
{
    s32 pending_advance_count;
    s32 requested_advance_count;

    akao_seq_start_song(params[0].buffer, params[3].value);
    g_akao_seq_channel0->song_id = (u16)params[2].value;
    requested_advance_count = params[4].value;
    pending_advance_count = 0;
    if (requested_advance_count != 0)
    {
        pending_advance_count = requested_advance_count - 1;
    }
    g_akao_seq_pending_ticks = pending_advance_count;
}

/**
 * @brief Save the active primary song and its channels for later resume.
 *
 * Playback continues after the snapshot is saved. Songs with no active
 * channels leave the previous snapshot intact.
 * @see akao_seq_resume_song
 */
void akao_seq_suspend_song(void)
{
    if (g_akao_seq_channel0->masks.active_mask == 0)
    {
        return;
    }

    akao_copy_bytes((s32*)g_akao_seq_channel0, (s32*)&g_akao_suspended_song, sizeof(g_akao_suspended_song));
    akao_copy_bytes((s32*)g_akao_seq_channels, (s32*)g_akao_suspended_channels, AKAO_CHANNEL_COUNT * sizeof(*g_akao_seq_channels));
}

/**
 * @brief Start a loaded song, retaining the active primary song in an available secondary slot.
 *
 * The old song keeps playing from the saved state when the secondary song is
 * absent or has a zero id. An occupied secondary song keeps its current state.
 *
 * @param params Load result: descriptor in params[0].buffer and song id in params[2].value.
 */
void akao_seq_switch_song(AkaoCommandParam* params)
{
    if (g_akao_seq_channel0->masks.active_mask != 0 && (g_akao_seq_channel1 == NULL || g_akao_seq_channel1->song_id == 0))
    {
        g_akao_seq_channel1 = &g_akao_suspended_song;
        g_akao_pending_channels = g_akao_suspended_channels;
        akao_copy_bytes((s32*)g_akao_seq_channel0, (s32*)&g_akao_suspended_song, sizeof(g_akao_suspended_song));
        akao_copy_bytes((s32*)g_akao_seq_channels, (s32*)g_akao_pending_channels, AKAO_CHANNEL_COUNT * sizeof(*g_akao_seq_channels));
    }
    akao_seq_start_song(params[0].buffer, AKAO_ALL_CHANNELS_MASK);
    g_akao_seq_channel0->song_id = (u16)params[2].value;
}

/**
 * @brief Reload a song and seed its pending advance count.
 *
 * The sequencer consumes the pending count at measure boundaries. Zero leaves
 * advancement disabled; other requests are reduced by one.
 *
 * @param params Load result: descriptor in params[0].buffer, song id in params[2].value
 *        and advance count in params[4].value.
 * @see akao_seq_reload_song
 */
void akao_seq_reload_song_with_ticks(AkaoCommandParam* params)
{
    s32 pending_advance_count;
    s32 requested_advance_count;

    akao_seq_reload_song(params);
    requested_advance_count = params[4].value;
    pending_advance_count = 0;
    if (requested_advance_count != 0)
    {
        pending_advance_count = requested_advance_count - 1;
    }
    g_akao_seq_pending_ticks = pending_advance_count;
}

/**
 * @brief Play supplied SFX sequences with centered pan, maximum volume and the default tag.
 *
 * Uses the default sound id and bank slot zero. Existing SFX sharing the default
 * tag receive a stop request before playback.
 *
 * @param params Sequences in params[0].buffer and params[1].buffer, each optionally NULL;
 *        entries 0-4 are replaced with the default play parameters.
 */
void akao_sfx_play_default(AkaoCommandParam* params)
{
    u8* first_sequence;
    u8* second_sequence;

    first_sequence = params[0].buffer;
    second_sequence = params[1].buffer;
    params[0].value = AKAO_SFX_DEFAULT_ID;
    params[1].value = AKAO_SFX_DEFAULT_TAG;
    params[2].value = AKAO_PAN_CENTER;
    params[3].value = AKAO_VOLUME_MAX;
    params[4].value = 0;
    akao_sfx_play(params, first_sequence, second_sequence, 0);
}

/**
 * @brief Play a bank program with centered pan, maximum volume and suppressed global SFX controls.
 *
 * Resolves the program's sequences and bank slot. SFX sharing the suppression
 * tag receive a stop request before playback.
 *
 * @param params Program index in params[0].value; tag, pan, volume and bank slot
 *        in entries 1-4 are replaced with the resolved play parameters.
 */
void akao_sfx_play_program(AkaoCommandParam* params)
{
    u8* first_sequence;
    u8* second_sequence;
    u16 program_key;
    s32 bank_slot;

    akao_resolve_program_data(&first_sequence, &second_sequence, params[0].value);
    params[1].value = AKAO_SFX_FLAG_SUPPRESS;
    params[2].value = AKAO_PAN_CENTER;
    params[3].value = AKAO_VOLUME_MAX;
    program_key = ((u16*)g_akao_bank_region_b)[params[0].value];
    bank_slot = akao_bank_find_slot(program_key);
    params[4].value = bank_slot;
    akao_sfx_play(params, first_sequence, second_sequence, 0);
}

/**
 * @brief Play a bank program using the caller's tag, pan and volume.
 * @param params Play parameters with a program index in params[0].value;
 *        params[4].value receives the resolved bank slot.
 */
void akao_sfx_play_program_raw(AkaoCommandParam* params)
{
    u8* first_sequence;
    u8* second_sequence;
    u16 program_key;
    s32 bank_slot;

    akao_resolve_program_data(&first_sequence, &second_sequence, params[0].value);
    program_key = ((u16*)g_akao_bank_region_b)[params[0].value];
    bank_slot = akao_bank_find_slot(program_key);
    params[4].value = bank_slot;
    akao_sfx_play(params, first_sequence, second_sequence, 0);
}

/**
 * @brief Play every entry of an SFX list, requesting tagged stops only before the first entry.
 *
 * Each entry can provide one or two sequences. All entries use the same tag,
 * pan, volume and resolved bank slot.
 *
 * @param params Nonempty list in params[0].buffer and play settings in entries 1-3;
 *        params[4].value receives the resolved bank slot.
 */
void akao_sfx_play_list(AkaoCommandParam* params)
{
    AkaoSfxListHeader* bank_header;
    AkaoSfxListHeader* list;
    u8* entry_data;
    u16* sequence_offset;
    s32* entry_offset;
    s32 entries_remaining;
    s32 no_sequence;
    u8* first_sequence;
    u8* second_sequence;

    bank_header = params[0].buffer;
    params[4].value = akao_bank_find_slot(bank_header->bank_key);

    list = params[0].buffer;
    entry_offset = list->entry_offsets;
    entry_data = (u8*)list;
    entry_data += sizeof(*list);
    entries_remaining = list->entry_count;

    sequence_offset = (u16*)(entry_data + *entry_offset);
    if (*sequence_offset != AKAO_PROGRAM_NO_SEQUENCE)
    {
        first_sequence = SFX_ENTRY_DATA(sequence_offset, sizeof(AkaoSfxSequencePair));
    }
    else
    {
        first_sequence = NULL;
    }
    sequence_offset++;
    if (*sequence_offset != AKAO_PROGRAM_NO_SEQUENCE)
    {
        second_sequence = SFX_ENTRY_DATA(sequence_offset, sizeof(*sequence_offset));
    }
    else
    {
        second_sequence = NULL;
    }
    akao_sfx_play(params, first_sequence, second_sequence, 0);

    no_sequence = AKAO_PROGRAM_NO_SEQUENCE;
    entries_remaining--;
    if (entries_remaining != 0)
    {
        entry_offset++;
        do
        {
            sequence_offset = (u16*)(entry_data + *entry_offset);
            if (*sequence_offset != no_sequence)
            {
                first_sequence = SFX_ENTRY_DATA(sequence_offset, sizeof(AkaoSfxSequencePair));
            }
            else
            {
                first_sequence = NULL;
            }
            sequence_offset++;
            if (*sequence_offset != no_sequence)
            {
                second_sequence = SFX_ENTRY_DATA(sequence_offset, sizeof(*sequence_offset));
            }
            else
            {
                second_sequence = NULL;
            }
            akao_sfx_play(params, first_sequence, second_sequence, 1);
            entries_remaining--;
            entry_offset++;
        } while (entries_remaining != 0);
    }
}

/**
 * @brief Stop SFX channels selected by a sound id and stop mode.
 * @param params Sound id in params[0].value and stop mode in params[1].value.
 * @see akao_sfx_stop_channels
 */
void akao_sfx_stop_channels_from_params(AkaoCommandParam* params)
{
    akao_sfx_stop_channels(params[0].value, params[1].value);
}

/**
 * @brief Set a song's master volume, cancel its fade and request channel volume updates.
 *
 * A zero id selects the primary song. Other ids match the primary song first,
 * then the secondary song; unmatched ids leave both songs unchanged.
 *
 * @param params Song id in params[0].value and volume in params[1].value; only the low seven volume bits are used.
 */
void akao_seq_set_master_volume(AkaoCommandParam* params)
{
    s32 song_id;
    s32 volume;

    song_id = params[0].value;
    if (song_id == 0 || song_id == g_akao_seq_channel0->song_id)
    {
        volume = (params[1].value & AKAO_VOLUME_MAX) << AKAO_SONG_VOLUME_SHIFT;
        g_akao_seq_channel0->volume = volume;
        g_akao_seq_channel0->volume_fade_ticks = 0;
        akao_seq_flag_volume_update(g_akao_seq_channel0, g_akao_seq_channels);
    }
    else if (g_akao_seq_channel1 != NULL && song_id != 0 && song_id == g_akao_seq_channel1->song_id)
    {
        intptr_t pending;

        pending = g_akao_pending_channels ? (intptr_t)g_akao_pending_channels : (intptr_t)g_akao_pending_channels;
        volume = params[1].value;
        g_akao_seq_channel1->volume_fade_ticks = 0;
        volume = (volume & AKAO_VOLUME_MAX) << AKAO_SONG_VOLUME_SHIFT;
        g_akao_seq_channel1->volume = volume;
        akao_seq_flag_volume_update(g_akao_seq_channel1, (AkaoChannelState*)pending);
    }
}

/**
 * @brief Fade a song's master volume from its current level to the requested level.
 *
 * A zero id selects the primary song; other ids match the primary song first,
 * then the secondary song. A zero duration is treated as one tick.
 *
 * @param params Song id in params[0].value, duration in params[1].value and target volume
 *        in params[2].value; only the low seven volume bits are used.
 */
void akao_seq_fade_master_volume(AkaoCommandParam* params)
{
    s32 song_id;
    s32 requested_ticks;
    s32 fade_ticks;
    s32 target_volume;
    s32 current_volume;
    s32 volume_step;
    AkaoChannelState* channels;

    requested_ticks = params[1].value;
    fade_ticks = 1;
    if (requested_ticks != 0)
    {
        fade_ticks = requested_ticks;
    }
    target_volume = (params[2].value & AKAO_VOLUME_MAX) << AKAO_SONG_VOLUME_SHIFT;
    song_id = params[0].value;
    if (song_id == 0 || song_id == g_akao_seq_channel0->song_id)
    {
        current_volume = g_akao_seq_channel0->volume;
        target_volume -= current_volume;
        volume_step = target_volume / fade_ticks;
        channels = g_akao_seq_channels;
        g_akao_seq_channel0->volume_fade_ticks = fade_ticks;
        g_akao_seq_channel0->volume_step = volume_step;
        akao_seq_flag_volume_update(g_akao_seq_channel0, channels);
    }
    else if (g_akao_seq_channel1 != NULL && song_id != 0 && song_id == g_akao_seq_channel1->song_id)
    {
        current_volume = g_akao_seq_channel1->volume;
        target_volume -= current_volume;
        volume_step = target_volume / fade_ticks;
        channels = g_akao_pending_channels;
        g_akao_seq_channel1->volume_fade_ticks = fade_ticks;
        g_akao_seq_channel1->volume_step = volume_step;
        akao_seq_flag_volume_update(g_akao_seq_channel1, channels);
    }
}

/**
 * @brief Set a song's starting master volume and fade to the requested target level.
 *
 * Song selection and zero-duration handling follow akao_seq_fade_master_volume.
 * Both volume levels use their low seven bits.
 *
 * @param params Song id in params[0].value, duration in params[1].value,
 *        starting volume in params[2].value and target volume in params[3].value.
 */
void akao_seq_fade_master_volume_from(AkaoCommandParam* params)
{
    s32 song_id;
    s32 requested_ticks;
    s32 fade_ticks;
    s32 start_volume;
    s32 target_volume;
    s32 volume_step;
    AkaoChannelState* channels;
    AkaoSongState* song;

    requested_ticks = params[1].value;
    fade_ticks = 1;
    if (requested_ticks != 0)
    {
        fade_ticks = requested_ticks;
    }
    song_id = params[0].value;
    if (song_id == 0 || song_id == g_akao_seq_channel0->song_id)
    {
        song = g_akao_seq_channel0;
        channels = g_akao_seq_channels;
    }
    else if (g_akao_seq_channel1 != NULL && song_id != 0 && song_id == g_akao_seq_channel1->song_id)
    {
        song = g_akao_seq_channel1;
        channels = g_akao_pending_channels;
    }
    else
    {
        return;
    }
    start_volume = (params[2].value & AKAO_VOLUME_MAX) << AKAO_SONG_VOLUME_SHIFT;
    song->volume = start_volume;
    target_volume = (params[3].value & AKAO_VOLUME_MAX) << AKAO_SONG_VOLUME_SHIFT;
    target_volume -= start_volume;
    volume_step = target_volume / fade_ticks;
    song->volume_fade_ticks = fade_ticks;
    song->volume_step = volume_step;
    akao_seq_flag_volume_update(song, channels);
}

/**
 * @brief Set the CD volume immediately and cancel its fade.
 * @param params CD volume in the low 16 bits of params[0].value.
 */
void akao_cd_set_volume(AkaoCommandParam* params)
{
    s32 volume;

    volume = (u16)params[0].value;
    g_akao_cdvol_fade_ticks = 0;
    volume = (u32)volume << AKAO_CD_VOLUME_SHIFT;
    g_akao_cdvol_acc = volume;
    akao_apply_cdvol_to_spu();
}

/**
 * @brief Fade the CD volume from its current level to a target.
 * @param params Tick count in params[0].value (zero becomes one) and target volume
 *        in the low 16 bits of params[1].value.
 */
void akao_cd_fade_volume(AkaoCommandParam* params)
{
    s32 requested_ticks;
    s32 fade_ticks;
    s32 target_volume;
    s32 volume_step;

    requested_ticks = params[0].value;
    fade_ticks = 1;
    if (requested_ticks != 0)
    {
        fade_ticks = requested_ticks;
    }
    target_volume = (u16)params[1].value;
    target_volume = (u32)target_volume << AKAO_CD_VOLUME_SHIFT;
    target_volume -= g_akao_cdvol_acc;
    volume_step = target_volume / fade_ticks;
    g_akao_cdvol_fade_ticks = fade_ticks;
    g_akao_cdvol_step = volume_step;
}

/**
 * @brief Fade the CD volume between explicit start and target levels.
 * @param params Tick count in params[0].value (zero becomes one), start volume in
 *        params[1].value and target volume in params[2].value; volumes use their low 16 bits.
 */
void akao_cd_fade_volume_from(AkaoCommandParam* params)
{
    s32 requested_ticks;
    s32 fade_ticks;
    s32 target_volume;
    s32 start_volume;
    s32 volume_step;

    requested_ticks = params[0].value;
    fade_ticks = 1;
    if (requested_ticks != 0)
    {
        fade_ticks = requested_ticks;
    }
    target_volume = (u16)params[2].value;
    start_volume = (u16)params[1].value;
    target_volume = (u32)target_volume << AKAO_CD_VOLUME_SHIFT;
    start_volume = (u32)start_volume << AKAO_CD_VOLUME_SHIFT;
    target_volume -= start_volume;
    volume_step = target_volume / fade_ticks;
    g_akao_cdvol_fade_ticks = fade_ticks;
    g_akao_cdvol_acc = start_volume;
    g_akao_cdvol_step = volume_step;
}

/**
 * @brief Set the volume scale of selected active SFX channels and cancel their fades.
 *
 * A nonzero tag mask selects channels with any overlapping tag bit. A zero mask
 * selects channels by their exact sound id.
 *
 * @param params Sound id in params[0].value, tag mask in params[1].value and
 *        volume scale in the low 7 bits of params[2].value.
 */
void akao_sfx_set_volume_scale(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active_channels;
    s32 channel_bit;
    u32 channel_index;
    s32 volume_scale;

    channel = g_sfx_channels;
    active_channels = g_akao_sfx_control.active_mask;
    channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    if (params[1].value != 0)
    {
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if ((active_channels & channel_bit) && (channel->sfx_tag & params[1].value))
            {
                volume_scale = (u16)params[2].value;
                channel->volume_scale_fade_ticks = 0;
                channel->volume_scale = (volume_scale & AKAO_VOLUME_MAX) << AKAO_SFX_FIXED_POINT_SHIFT;
                channel->update_flags |= SPU_UPDATE_VOLUME;
            }
        }
    }
    else
    {
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if ((active_channels & channel_bit) && ((s32)channel->sfx_id == params[0].value))
            {
                volume_scale = (u16)params[2].value;
                channel->volume_scale_fade_ticks = 0;
                channel->volume_scale = (volume_scale & AKAO_VOLUME_MAX) << AKAO_SFX_FIXED_POINT_SHIFT;
                channel->update_flags |= SPU_UPDATE_VOLUME;
            }
        }
    }
}

/**
 * @brief Fade the volume scale of active SFX channels selected by tag mask or sound id.
 * @param params Sound id in params[0].value, tag mask in params[1].value (zero selects
 *        by id), tick count in params[2].value (zero becomes one) and target scale
 *        in the low 7 bits of params[3].value.
 * @see akao_sfx_set_volume_scale
 */
void akao_sfx_fade_volume_scale(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active_channels;
    s32 channel_bit;
    u32 channel_index;
    s16 fade_ticks;
    u16 target_scale;
    s32 current_scale;
    s16 volume_delta;

    channel = g_sfx_channels;
    active_channels = g_akao_sfx_control.active_mask;
    channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    if (params[1].value != 0)
    {
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if ((active_channels & channel_bit) && (channel->sfx_tag & params[1].value))
            {
                if (params[2].value != 0)
                {
                    fade_ticks = (u16)params[2].value;
                }
                else
                {
                    fade_ticks = 1;
                }
                target_scale = ((u16)params[3].value & AKAO_VOLUME_MAX) << AKAO_SFX_FIXED_POINT_SHIFT;
                current_scale = channel->volume_scale;
                volume_delta = target_scale - current_scale;
                channel->volume_scale_step = volume_delta / fade_ticks;
                channel->volume_scale_fade_ticks = fade_ticks;
            }
        }
    }
    else
    {
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if ((active_channels & channel_bit) && ((s32)channel->sfx_id == params[0].value))
            {
                if (params[2].value != 0)
                {
                    fade_ticks = (u16)params[2].value;
                }
                else
                {
                    fade_ticks = 1;
                }
                target_scale = ((u16)params[3].value & AKAO_VOLUME_MAX) << AKAO_SFX_FIXED_POINT_SHIFT;
                current_scale = channel->volume_scale;
                volume_delta = target_scale - current_scale;
                channel->volume_scale_step = volume_delta / fade_ticks;
                channel->volume_scale_fade_ticks = fade_ticks;
            }
        }
    }
}

/**
 * @brief Set the volume scale of active SFX channels that allow global controls.
 *
 * Cancels their volume-scale fades and skips channels tagged AKAO_SFX_FLAG_SUPPRESS.
 *
 * @param params Volume scale in the low 7 bits of params[0].value.
 */
void akao_sfx_set_volume_scale_unsuppressed(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active_channels;
    s32 channel_bit;
    u32 channel_index;
    s32 volume_scale;

    channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    active_channels = g_akao_sfx_control.active_mask;
    channel = g_sfx_channels;
    for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
    {
        if ((active_channels & channel_bit) && !(channel->sfx_tag & AKAO_SFX_FLAG_SUPPRESS))
        {
            volume_scale = (u16)params[0].value;
            channel->volume_scale_fade_ticks = 0;
            channel->volume_scale = (volume_scale & AKAO_VOLUME_MAX) << AKAO_SFX_FIXED_POINT_SHIFT;
            channel->update_flags |= SPU_UPDATE_VOLUME;
        }
    }
}

/**
 * @brief Fade the volume scale of active SFX channels that allow global controls.
 *
 * Skips channels tagged AKAO_SFX_FLAG_SUPPRESS.
 *
 * @param params Tick count in params[0].value (zero becomes one) and target scale
 *        in the low 7 bits of params[1].value.
 */
void akao_sfx_fade_volume_scale_unsuppressed(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active_channels;
    s32 channel_bit;
    u32 channel_index;
    s16 fade_ticks;
    u16 target_scale;
    s32 current_scale;
    s16 volume_delta;

    channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    active_channels = g_akao_sfx_control.active_mask;
    channel = g_sfx_channels;
    for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
    {
        if ((active_channels & channel_bit) && !(channel->sfx_tag & AKAO_SFX_FLAG_SUPPRESS))
        {
            if (params[0].value != 0)
            {
                fade_ticks = (u16)params[0].value;
            }
            else
            {
                fade_ticks = 1;
            }
            target_scale = ((u16)params[1].value & AKAO_VOLUME_MAX) << AKAO_SFX_FIXED_POINT_SHIFT;
            current_scale = channel->volume_scale;
            volume_delta = target_scale - current_scale;
            channel->volume_scale_step = volume_delta / fade_ticks;
            channel->volume_scale_fade_ticks = fade_ticks;
        }
    }
}

/**
 * @brief Set the pan bias of selected active SFX channels and cancel their fades.
 * @param params Sound id in params[0].value, tag mask in params[1].value (zero selects
 *        by id) and pan bias in the low 8 bits of params[2].value.
 * @see akao_sfx_set_volume_scale
 */
void akao_sfx_set_pan_bias(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active_channels;
    s32 channel_bit;
    u32 channel_index;
    s32 pan_bias;

    channel = g_sfx_channels;
    active_channels = g_akao_sfx_control.active_mask;
    channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    if (params[1].value != 0)
    {
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if ((active_channels & channel_bit) && (channel->sfx_tag & params[1].value))
            {
                pan_bias = (u8)params[2].value;
                channel->pan_bias_fade_ticks = 0;
                channel->pan_bias = pan_bias << AKAO_SFX_FIXED_POINT_SHIFT;
                channel->update_flags |= SPU_UPDATE_VOLUME;
            }
        }
    }
    else
    {
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if ((active_channels & channel_bit) && ((s32)channel->sfx_id == params[0].value))
            {
                pan_bias = (u8)params[2].value;
                channel->pan_bias_fade_ticks = 0;
                channel->pan_bias = pan_bias << AKAO_SFX_FIXED_POINT_SHIFT;
                channel->update_flags |= SPU_UPDATE_VOLUME;
            }
        }
    }
}

/**
 * @brief Fade the pan bias of active SFX channels selected by tag mask or sound id.
 * @param params Sound id in params[0].value, tag mask in params[1].value (zero selects
 *        by id), tick count in params[2].value (zero becomes one) and target bias
 *        in the low 8 bits of params[3].value.
 * @see akao_sfx_set_volume_scale
 */
void akao_sfx_fade_pan_bias(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active_channels;
    s32 channel_bit;
    u32 channel_index;
    s16 fade_ticks;
    u16 target_bias;
    s32 current_bias;
    s16 pan_delta;

    channel = g_sfx_channels;
    active_channels = g_akao_sfx_control.active_mask;
    channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    if (params[1].value != 0)
    {
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if ((active_channels & channel_bit) && (channel->sfx_tag & params[1].value))
            {
                if (params[2].value != 0)
                {
                    fade_ticks = (u16)params[2].value;
                }
                else
                {
                    fade_ticks = 1;
                }
                target_bias = (u8)params[3].value << AKAO_SFX_FIXED_POINT_SHIFT;
                current_bias = channel->pan_bias;
                pan_delta = target_bias - current_bias;
                channel->pan_bias_step = pan_delta / fade_ticks;
                channel->pan_bias_fade_ticks = fade_ticks;
            }
        }
    }
    else
    {
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if ((active_channels & channel_bit) && ((s32)channel->sfx_id == params[0].value))
            {
                if (params[2].value != 0)
                {
                    fade_ticks = (u16)params[2].value;
                }
                else
                {
                    fade_ticks = 1;
                }
                target_bias = (u8)params[3].value << AKAO_SFX_FIXED_POINT_SHIFT;
                current_bias = channel->pan_bias;
                pan_delta = target_bias - current_bias;
                channel->pan_bias_step = pan_delta / fade_ticks;
                channel->pan_bias_fade_ticks = fade_ticks;
            }
        }
    }
}

/**
 * @brief Set the pan bias of active SFX channels that allow global controls.
 *
 * Cancels their pan-bias fades and skips channels tagged AKAO_SFX_FLAG_SUPPRESS.
 *
 * @param params Pan bias in the low 8 bits of params[0].value.
 */
void akao_sfx_set_pan_bias_unsuppressed(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active_channels;
    s32 channel_bit;
    u32 channel_index;
    s32 pan_bias;

    channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    active_channels = g_akao_sfx_control.active_mask;
    channel = g_sfx_channels;
    for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
    {
        if ((active_channels & channel_bit) && !(channel->sfx_tag & AKAO_SFX_FLAG_SUPPRESS))
        {
            pan_bias = (u8)params[0].value;
            channel->pan_bias_fade_ticks = 0;
            channel->pan_bias = pan_bias << AKAO_SFX_FIXED_POINT_SHIFT;
            channel->update_flags |= SPU_UPDATE_VOLUME;
        }
    }
}

/**
 * @brief Fade the pan bias of active SFX channels that allow global controls.
 *
 * Skips channels tagged AKAO_SFX_FLAG_SUPPRESS.
 *
 * @param params Tick count in params[0].value (zero becomes one) and target bias
 *        in the low 8 bits of params[1].value.
 */
void akao_sfx_fade_pan_bias_unsuppressed(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active_channels;
    s32 channel_bit;
    u32 channel_index;
    s16 fade_ticks;
    u16 target_bias;
    s32 current_bias;
    s16 pan_delta;

    channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    active_channels = g_akao_sfx_control.active_mask;
    channel = g_sfx_channels;
    for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
    {
        if ((active_channels & channel_bit) && !(channel->sfx_tag & AKAO_SFX_FLAG_SUPPRESS))
        {
            if (params[0].value != 0)
            {
                fade_ticks = (u16)params[0].value;
            }
            else
            {
                fade_ticks = 1;
            }
            target_bias = (u8)params[1].value << AKAO_SFX_FIXED_POINT_SHIFT;
            current_bias = channel->pan_bias;
            pan_delta = target_bias - current_bias;
            channel->pan_bias_step = pan_delta / fade_ticks;
            channel->pan_bias_fade_ticks = fade_ticks;
        }
    }
}

/**
 * @brief Set the pitch bend of selected active SFX channels and cancel their fades.
 * @param params Sound id in params[0].value, tag mask in params[1].value (zero selects
 *        by id) and pitch bend in the low 8 bits of params[2].value.
 * @see akao_sfx_set_volume_scale
 */
void akao_sfx_set_pitch_bend(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active_channels;
    s32 channel_bit;
    u32 channel_index;
    s32 pitch_bend;

    channel = g_sfx_channels;
    active_channels = g_akao_sfx_control.active_mask;
    channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    if (params[1].value != 0)
    {
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if ((active_channels & channel_bit) && (channel->sfx_tag & params[1].value))
            {
                pitch_bend = (u8)params[2].value;
                channel->sfx_pitch_bend_fade_ticks = 0;
                channel->sfx_pitch_bend = pitch_bend << AKAO_SFX_FIXED_POINT_SHIFT;
                channel->update_flags |= SPU_UPDATE_PITCH;
            }
        }
    }
    else
    {
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if ((active_channels & channel_bit) && ((s32)channel->sfx_id == params[0].value))
            {
                pitch_bend = (u8)params[2].value;
                channel->sfx_pitch_bend_fade_ticks = 0;
                channel->sfx_pitch_bend = pitch_bend << AKAO_SFX_FIXED_POINT_SHIFT;
                channel->update_flags |= SPU_UPDATE_PITCH;
            }
        }
    }
}

/**
 * @brief Fade the pitch bend of active SFX channels selected by tag mask or sound id.
 * @param params Sound id in params[0].value, tag mask in params[1].value (zero selects
 *        by id), tick count in params[2].value (zero becomes one) and target bend
 *        in the low 8 bits of params[3].value.
 * @see akao_sfx_set_volume_scale
 */
void akao_sfx_fade_pitch_bend(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active_channels;
    s32 channel_bit;
    u32 channel_index;
    s16 fade_ticks;
    u16 target_bend;
    s32 current_bend;
    s16 bend_delta;
    s16 bend_step;

    channel = g_sfx_channels;
    active_channels = g_akao_sfx_control.active_mask;
    channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    if (params[1].value != 0)
    {
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if ((active_channels & channel_bit) && (channel->sfx_tag & params[1].value))
            {
                if (params[2].value != 0)
                {
                    fade_ticks = (u16)params[2].value;
                }
                else
                {
                    fade_ticks = 1;
                }
                target_bend = (u8)params[3].value << AKAO_SFX_FIXED_POINT_SHIFT;
                current_bend = HALF_LOW_U16(channel->sfx_pitch_bend);
                bend_delta = target_bend - current_bend;
                bend_step = bend_delta / fade_ticks;
                channel->sfx_pitch_bend_step = bend_step;
                channel->sfx_pitch_bend_fade_ticks = fade_ticks;
            }
        }
    }
    else
    {
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if ((active_channels & channel_bit) && ((s32)channel->sfx_id == params[0].value))
            {
                if (params[2].value != 0)
                {
                    fade_ticks = (u16)params[2].value;
                }
                else
                {
                    fade_ticks = 1;
                }
                target_bend = (u8)params[3].value << AKAO_SFX_FIXED_POINT_SHIFT;
                current_bend = HALF_LOW_U16(channel->sfx_pitch_bend);
                bend_delta = target_bend - current_bend;
                bend_step = bend_delta / fade_ticks;
                channel->sfx_pitch_bend_step = bend_step;
                channel->sfx_pitch_bend_fade_ticks = fade_ticks;
            }
        }
    }
}

/**
 * @brief Set the pitch bend of SFX channels that allow global controls.
 *
 * Includes inactive channels, cancels their pitch-bend fades and skips channels
 * tagged AKAO_SFX_FLAG_SUPPRESS.
 *
 * @param params Pitch bend in the low 8 bits of params[0].value.
 */
void akao_sfx_set_pitch_bend_unsuppressed(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    u32 channels_remaining;
    s32 pitch_bend;

    channel = g_sfx_channels;
    for (channels_remaining = AKAO_SFX_CHANNEL_COUNT; channels_remaining != 0; channels_remaining--, channel++)
    {
        if (!(channel->sfx_tag & AKAO_SFX_FLAG_SUPPRESS))
        {
            pitch_bend = (u8)params[0].value;
            channel->sfx_pitch_bend_fade_ticks = 0;
            channel->sfx_pitch_bend = pitch_bend << AKAO_SFX_FIXED_POINT_SHIFT;
            channel->update_flags |= SPU_UPDATE_PITCH;
        }
    }
}

/**
 * @brief Fade the pitch bend of active SFX channels that allow global controls.
 *
 * Skips channels tagged AKAO_SFX_FLAG_SUPPRESS.
 *
 * @param params Tick count in params[0].value (zero becomes one) and target bend
 *        in the low 8 bits of params[1].value.
 */
void akao_sfx_fade_pitch_bend_unsuppressed(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active_channels;
    s32 channel_bit;
    u32 channel_index;
    s16 fade_ticks;
    u16 target_bend;
    s32 current_bend;
    s16 bend_delta;
    s16 bend_step;

    channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    active_channels = g_akao_sfx_control.active_mask;
    channel = g_sfx_channels;
    for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
    {
        if ((active_channels & channel_bit) && !(channel->sfx_tag & AKAO_SFX_FLAG_SUPPRESS))
        {
            if (params[0].value != 0)
            {
                fade_ticks = (u16)params[0].value;
            }
            else
            {
                fade_ticks = 1;
            }
            target_bend = (u8)params[1].value << AKAO_SFX_FIXED_POINT_SHIFT;
            current_bend = HALF_LOW_U16(channel->sfx_pitch_bend);
            bend_delta = target_bend - current_bend;
            bend_step = bend_delta / fade_ticks;
            channel->sfx_pitch_bend_step = bend_step;
            channel->sfx_pitch_bend_fade_ticks = fade_ticks;
        }
    }
}

/**
 * @brief Set the driver master pan and cancel its fade.
 * @param params Signed pan in the low 8 bits of params[0].value.
 */
void akao_master_set_pan(AkaoCommandParam* params)
{
    s32 pan;

    pan = (s8)params[0].value;
    g_akao_masterpan_fade_ticks = 0;
    pan = (u32)pan << AKAO_MASTER_FIXED_POINT_SHIFT;
    g_akao_masterpan_acc = pan;
}

/**
 * @brief Fade the driver master pan from its current position to a target.
 * @param params Tick count in params[0].value (zero becomes one) and signed target pan
 *        in the low 8 bits of params[1].value.
 */
void akao_master_fade_pan(AkaoCommandParam* params)
{
    s32 requested_ticks;
    s32 fade_ticks;
    s32 target_pan;
    s32 pan_step;

    requested_ticks = params[0].value;
    fade_ticks = 1;
    if (requested_ticks != 0)
    {
        fade_ticks = requested_ticks;
    }
    target_pan = (s8)params[1].value;
    target_pan = (u32)target_pan << AKAO_MASTER_FIXED_POINT_SHIFT;
    target_pan -= g_akao_masterpan_acc;
    pan_step = target_pan / fade_ticks;
    g_akao_masterpan_fade_ticks = fade_ticks;
    g_akao_masterpan_step = pan_step;
}

/**
 * @brief Fade the driver master pan between explicit start and target positions.
 *
 * A zero start-pan parameter selects a one-tick fade. Otherwise the tick count
 * comes directly from params[0].value, including when that count is zero.
 *
 * @param params Tick count in params[0].value, signed start pan in params[1].value
 *        and signed target pan in params[2].value; pan values use their low 8 bits.
 */
void akao_master_fade_pan_from(AkaoCommandParam* params)
{
    s32 start_pan_param;
    s32 fade_ticks;
    s32 target_pan;
    s32 start_pan;
    s32 pan_step;

    start_pan_param = params[1].value;
    fade_ticks = 1;
    if (start_pan_param != 0)
    {
        fade_ticks = params[0].value;
    }
    start_pan = (s8)params[1].value;
    start_pan = (u32)start_pan << AKAO_MASTER_FIXED_POINT_SHIFT;
    g_akao_masterpan_acc = start_pan;
    target_pan = (s8)params[2].value;
    target_pan = (u32)target_pan << AKAO_MASTER_FIXED_POINT_SHIFT;
    target_pan -= start_pan;
    pan_step = target_pan / fade_ticks;
    g_akao_masterpan_fade_ticks = fade_ticks;
    g_akao_masterpan_step = pan_step;
}

/**
 * @brief Set the driver master volume and cancel its fade.
 * @param params Signed volume in the low 8 bits of params[0].value.
 */
void akao_master_set_volume(AkaoCommandParam* params)
{
    s32 volume;

    volume = (s8)params[0].value;
    g_akao_mastervol_fade_ticks = 0;
    volume = (u32)volume << AKAO_MASTER_FIXED_POINT_SHIFT;
    g_akao_mastervol_acc = volume;
}

/**
 * @brief Fade the driver master volume from its current level to a target.
 * @param params Tick count in params[0].value (zero becomes one) and signed target volume
 *        in the low 8 bits of params[1].value.
 */
void akao_master_fade_volume(AkaoCommandParam* params)
{
    s32 requested_ticks;
    s32 fade_ticks;
    s32 target_volume;
    s32 volume_step;

    requested_ticks = params[0].value;
    fade_ticks = 1;
    if (requested_ticks != 0)
    {
        fade_ticks = requested_ticks;
    }
    target_volume = (s8)params[1].value;
    target_volume = (u32)target_volume << AKAO_MASTER_FIXED_POINT_SHIFT;
    target_volume -= g_akao_mastervol_acc;
    volume_step = target_volume / fade_ticks;
    g_akao_mastervol_fade_ticks = fade_ticks;
    g_akao_mastervol_step = volume_step;
}

/**
 * @brief Fade the driver master volume between explicit start and target levels.
 *
 * A zero start-volume parameter selects a one-tick fade. Otherwise the tick count
 * comes directly from params[0].value, including when that count is zero.
 *
 * @param params Tick count in params[0].value, signed start volume in params[1].value
 *        and signed target volume in params[2].value; volumes use their low 8 bits.
 */
void akao_master_fade_volume_from(AkaoCommandParam* params)
{
    s32 start_volume_param;
    s32 fade_ticks;
    s32 target_volume;
    s32 start_volume;
    s32 volume_step;

    start_volume_param = params[1].value;
    fade_ticks = 1;
    if (start_volume_param != 0)
    {
        fade_ticks = params[0].value;
    }
    start_volume = (s8)params[1].value;
    start_volume = (u32)start_volume << AKAO_MASTER_FIXED_POINT_SHIFT;
    g_akao_mastervol_acc = start_volume;
    target_volume = (s8)params[2].value;
    target_volume = (u32)target_volume << AKAO_MASTER_FIXED_POINT_SHIFT;
    target_volume -= start_volume;
    volume_step = target_volume / fade_ticks;
    g_akao_mastervol_fade_ticks = fade_ticks;
    g_akao_mastervol_step = volume_step;
}

/**
 * @brief Unconditionally stop the primary song, and the secondary song
 *        too if one is loaded.
 */
void akao_seq_stop_all_songs(void)
{
    akao_seq_stop_song(g_akao_seq_channel0, g_akao_seq_channels, 0);
    if (g_akao_seq_channel1 != NULL)
    {
        akao_seq_stop_song(g_akao_seq_channel1, g_akao_pending_channels, 0);
    }
}

/**
 * @brief Stop songs matching a key, or stop only the primary song when the key is zero.
 * @param params Song key in params[0].value; zero stops the primary song unconditionally.
 * @see akao_seq_stop_song
 */
void akao_seq_stop_song_by_key(AkaoCommandParam* params)
{
    s32 song_key;

    akao_seq_stop_song(g_akao_seq_channel0, g_akao_seq_channels, params[0].value);
    if (g_akao_seq_channel1 != NULL)
    {
        song_key = params[0].value;
        if (song_key != 0)
        {
            akao_seq_stop_song(g_akao_seq_channel1, g_akao_pending_channels, song_key);
        }
    }
}

/**
 * @brief Release every active, non-suppressed SFX channel and clear its
 *        flags word.
 */
void akao_sfx_release_all_channels(void)
{
    AkaoChannelState* channel;
    s32 channel_bit;
    u32 channel_index;

    channel = g_sfx_channels;
    channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
    {
        if ((g_akao_sfx_control.active_mask & channel_bit) && !(channel->sfx_tag & AKAO_SFX_FLAG_SUPPRESS))
        {
            g_akao_sfx_control.key_off_mask |= channel_bit;
            akao_sfx_release_channels(channel, channel_bit);
            channel->flags = 0;
        }
    }
    g_akao_driver_flags.update_flags |= (AKAO_EFFECT_MASKS_UPDATE_PENDING | AKAO_NOISE_CLOCK_UPDATE_PENDING);
}

/**
 * @brief Select stereo output and flag active song and SFX channels for a volume update.
 */
void akao_select_stereo_output(void)
{
    g_akao_driver_flags.output_mode = AKAO_OUTPUT_STEREO;
    akao_seq_flag_volume_update(g_akao_seq_channel0, g_akao_seq_channels);
    if (g_akao_seq_channel1 != NULL)
    {
        akao_seq_flag_volume_update(g_akao_seq_channel1, g_akao_pending_channels);
    }
    akao_sfx_flag_volume_update();
}

/**
 * @brief Select mono output and flag active song and SFX channels for a volume update.
 */
void akao_select_mono_output(void)
{
    g_akao_driver_flags.output_mode = AKAO_OUTPUT_MONO;
    akao_seq_flag_volume_update(g_akao_seq_channel0, g_akao_seq_channels);
    if (g_akao_seq_channel1 != NULL)
    {
        akao_seq_flag_volume_update(g_akao_seq_channel1, g_akao_pending_channels);
    }
    akao_sfx_flag_volume_update();
}

/**
 * @brief Set the primary song's muted channels and request volume updates.
 * @param params Muted channel mask in params[0].value.
 */
void akao_seq_set_muted_channels(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    u32 channel_index;

    g_akao_muted_channel_mask = params[0].value;
    channel = g_akao_seq_channels;
    for (channel_index = 0; channel_index < AKAO_CHANNEL_COUNT; channel_index++, channel++)
    {
        channel->update_flags |= SPU_UPDATE_VOLUME;
    }
}

/**
 * @brief Set the primary song's conditional-jump variable (condition).
 * @param params Condition value in the low 16 bits of params[0].value.
 */
void akao_seq_set_condition(AkaoCommandParam* params)
{
    g_akao_seq_channel0->condition = (u16)params[0].value;
}

/**
 * @brief Pause the primary song and silence voices outside the SFX and XA reservations.
 *
 * Saves active channels in parked_mask without releasing their assigned voices.
 */
void akao_seq_silence_unused_voices_and_pause(void)
{
    s32 voices_to_silence;
    s32 voice_bit;
    s32 voice_index;
    s32 active_channels;

    if (g_akao_seq_channel0->masks.active_mask != 0)
    {
        voices_to_silence = ~(g_akao_sfx_control.active_mask | g_akao_xa_tracker.voice_mask) & AKAO_VOICE_MASK;
        if (voices_to_silence != 0)
        {
            voice_bit = 1;
            voice_index = 0;
            do
            {
                if (voices_to_silence & voice_bit)
                {
                    spu_set_voice_volume(voice_index, 0, 0, 0);
                    spu_set_voice_pitch(voice_index, 0);
                    spu_set_voice_attack(voice_index, AKAO_PAUSE_ENVELOPE_RATE, SPU_VOICE_LINEARIncN);
                    spu_set_voice_sustain_mode(voice_index, AKAO_PAUSE_ENVELOPE_RATE, SPU_VOICE_LINEARDecN);
                    voices_to_silence &= ~voice_bit;
                }
                voice_bit <<= 1;
                voice_index++;
            } while (voices_to_silence != 0);
        }
        active_channels = g_akao_seq_channel0->masks.active_mask;
        g_akao_seq_channel0->masks.active_mask = 0;
        g_akao_seq_channel0->parked_mask = active_channels;
    }
    g_akao_driver_mode_flags |= AKAO_MODE_SONG_PAUSED;
}

/**
 * @brief Resume the primary song and request volume, pitch and ADSR updates.
 *
 * Restores its parked channels and clears the song-pause mode flag.
 * @see akao_seq_silence_unused_voices_and_pause
 */
void akao_seq_resume_and_apply_pending_voices(void)
{
    s32 parked_channels;
    s32 remaining_channels;
    u32 channel_bit;
    AkaoChannelState* channel;
    s32 restored_channels;

    parked_channels = g_akao_seq_channel0->parked_mask;
    if (parked_channels != 0)
    {
        remaining_channels = parked_channels;
        channel_bit = 1;
        channel = g_akao_seq_channels;
        do
        {
            if (remaining_channels & channel_bit)
            {
                remaining_channels &= ~channel_bit;
                channel->update_flags |= AKAO_PAUSE_RESUME_VOICE_UPDATES;
            }
            channel_bit <<= 1;
            channel++;
        } while (remaining_channels != 0);

        restored_channels = g_akao_seq_channel0->parked_mask;
        g_akao_seq_channel0->parked_mask = 0;
        g_akao_seq_channel0->masks.active_mask = restored_channels;
        g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
    }
    g_akao_driver_mode_flags &= ~AKAO_MODE_SONG_PAUSED;
}

/**
 * @brief Park and silence active SFX channels that allow global controls.
 *
 * Channels tagged AKAO_SFX_FLAG_SUPPRESS keep playing. Sets the SFX-pause mode flag.
 */
void akao_sfx_silence_unused_voices_and_pause(void)
{
    AkaoChannelState* channel;
    s32 active_channels;
    s32 voice_index;
    s32 channels_to_pause;
    s32 channel_bit;
    u32 channel_index;

    active_channels = g_akao_sfx_control.active_mask;
    if (active_channels != 0)
    {
        channels_to_pause = active_channels;
        channel = g_sfx_channels;
        channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
        for (channel_index = 0; channel_index < AKAO_SFX_CHANNEL_COUNT; channel_index++, channel++, channel_bit <<= 1)
        {
            if ((channels_to_pause & channel_bit) && (channel->sfx_tag & AKAO_SFX_FLAG_SUPPRESS))
            {
                channels_to_pause &= ~channel_bit;
            }
        }
        channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
        voice_index = AKAO_SFX_FIRST_VOICE;
        g_akao_sfx_control.paused_mask = channels_to_pause;
        g_akao_sfx_control.active_mask &= ~channels_to_pause;
        if (channels_to_pause != 0)
        {
            do
            {
                if (channels_to_pause & channel_bit)
                {
                    spu_set_voice_volume(voice_index, 0, 0, 0);
                    spu_set_voice_pitch(voice_index, 0);
                    spu_set_voice_attack(voice_index, AKAO_PAUSE_ENVELOPE_RATE, SPU_VOICE_LINEARIncN);
                    spu_set_voice_sustain_mode(voice_index, AKAO_PAUSE_ENVELOPE_RATE, SPU_VOICE_LINEARDecN);
                    channels_to_pause &= ~channel_bit;
                }
                channel_bit <<= 1;
                voice_index++;
            } while (channels_to_pause != 0);
        }
    }
    g_akao_driver_mode_flags |= AKAO_MODE_SFX_PAUSED;
}

/**
 * @brief Resume parked SFX channels and request volume, pitch and ADSR updates.
 *
 * Keeps already active channels and clears the SFX-pause mode flag.
 * @see akao_sfx_silence_unused_voices_and_pause
 */
void akao_sfx_resume_and_apply_pending_voices(void)
{
    AkaoChannelState* channel;
    s32 parked_channels;
    s32 remaining_channels;
    s32 channel_bit;
    s32 restored_channels;

    parked_channels = g_akao_sfx_control.paused_mask;
    if (parked_channels != 0)
    {
        remaining_channels = parked_channels;
        channel = g_sfx_channels;
        channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
        do
        {
            if (remaining_channels & channel_bit)
            {
                remaining_channels &= ~channel_bit;
                channel->update_flags |= AKAO_PAUSE_RESUME_VOICE_UPDATES;
            }
            channel_bit <<= 1;
            channel++;
        } while (remaining_channels != 0);

        restored_channels = g_akao_sfx_control.paused_mask;
        g_akao_sfx_control.paused_mask = 0;
        g_akao_sfx_control.active_mask |= restored_channels;
        g_akao_driver_flags.update_flags |= AKAO_EFFECT_MASKS_UPDATE_PENDING;
    }
    g_akao_driver_mode_flags &= ~AKAO_MODE_SFX_PAUSED;
}

/**
 * @brief Zero the pitch of the streamed XA voice pair while a stream is active.
 */
void akao_xa_silence_voice_pitch(void)
{
    if (g_akao_xa_tracker.voice_mask != 0)
    {
        spu_set_voice_pitch(g_akao_xa_tracker.first_voice, 0);
        spu_set_voice_pitch(g_akao_xa_tracker.first_voice + 1, 0);
    }
}

/**
 * @brief Restore the streamed XA voice pair's pitch to the tracker's cached
 *        value while a stream is active.
 */
void akao_xa_restore_voice_pitch(void)
{
    if (g_akao_xa_tracker.voice_mask != 0)
    {
        spu_set_voice_pitch(g_akao_xa_tracker.first_voice, g_akao_xa_tracker.pitch);
        spu_set_voice_pitch(g_akao_xa_tracker.first_voice + 1, g_akao_xa_tracker.pitch);
    }
}

/**
 * @brief Ignore an unused command opcode.
 */
void akao_ignore_command(void)
{
}

/**
 * @brief Change the SPU reverb mode if needed, clearing its work area with reverb disabled.
 * @param reverb_type New reverb mode (an @c AkaoHeader::reverb_type value).
 */
void akao_apply_reverb_type(s32 reverb_type)
{
    long current_mode;

    SpuGetReverbModeType(&current_mode);
    if (current_mode != reverb_type)
    {
        SpuSetReverb(SPU_OFF);
        SpuSetReverbModeType(reverb_type | SPU_REV_MODE_CLEAR_WA);
        SpuSetReverb(SPU_ON);
    }
}

/**
 * @brief Dispatch an AKAO command while the sequencer tick event is disabled.
 *
 * Song commands validate the header and prepare the descriptor, id, channel
 * mask and pending advance count for their handler. A matching primary song is skipped
 * unless a loaded secondary song has a different id.
 *
 * Combined master pan/volume commands call the pan handler before the volume
 * handler. Pause/resume-all commands call the song, SFX and XA handlers in order.
 * Other commands copy all six input slots before calling their table handler.
 *
 * @param opcode Command opcode; only the low byte is significant.
 * @return Song id for a dispatched song command, -1 for a bad song header,
 *         or 0 for a skipped song command or any other command.
 */
s32 akao_send_command(u32 opcode)
{
    s32 result;
    s32 requested_mask;
    s32 start_mask;
    AkaoCommandParam* dispatch_params;
    AkaoHeader* song_header;
    u16 current_song_id;

    result = 0;
    DisableEvent(g_akao_rcnt2_event);
    opcode = (u8)opcode;
    dispatch_params = g_akao_dispatch_params;

    switch (opcode)
    {
    case AKAO_CMD_PLAY_SONG:
    case AKAO_CMD_PLAY_SONG_WITH_TICKS:
    case AKAO_CMD_START_SONG_CHANNELS:
    case AKAO_CMD_SWITCH_SONG:
        if (akao_check_magic(g_akao_cmd_params[0].buffer) == 0)
        {
            song_header = g_akao_cmd_params[0].buffer;
            current_song_id = g_akao_seq_channel0->song_id;
            if ((current_song_id != song_header->id) || ((g_akao_seq_channel1 != NULL) && (g_akao_seq_channel1->song_id != current_song_id)))
            {
                akao_apply_reverb_type(song_header->reverb_type);
                dispatch_params[0].buffer = song_header;
                dispatch_params[2].value = song_header->id;
                if (opcode == AKAO_CMD_PLAY_SONG_WITH_TICKS)
                {
                    dispatch_params[4].value = g_akao_cmd_params[1].value;
                }
                else
                {
                    requested_mask = g_akao_cmd_params[1].value;
                    start_mask = AKAO_ALL_CHANNELS_MASK;
                    if (requested_mask != 0)
                    {
                        start_mask = requested_mask | AKAO_SONG_FIRST_CHANNEL_BIT;
                    }
                    dispatch_params[3].value = start_mask;
                    dispatch_params[4].value = g_akao_cmd_params[2].value;
                }
                result = song_header->id;
            }
            else
            {
                opcode = AKAO_CMD_IGNORE;
                result = 0;
            }
        }
        else
        {
            opcode = AKAO_CMD_IGNORE;
            result = -1;
        }
        break;

    case AKAO_CMD_SET_MASTER_PAN_AND_VOLUME:
        dispatch_params[0].value = g_akao_cmd_params[0].value;
        g_akao_command_handlers[AKAO_CMD_SET_MASTER_PAN](dispatch_params);
        opcode = AKAO_CMD_SET_MASTER_VOLUME;
        break;

    case AKAO_CMD_FADE_MASTER_PAN_AND_VOLUME:
        dispatch_params[0].value = g_akao_cmd_params[0].value;
        dispatch_params[1].value = g_akao_cmd_params[1].value;
        g_akao_command_handlers[AKAO_CMD_FADE_MASTER_PAN](dispatch_params);
        opcode = AKAO_CMD_FADE_MASTER_VOLUME;
        break;

    case AKAO_CMD_FADE_MASTER_PAN_AND_VOLUME_FROM:
        dispatch_params[0].value = g_akao_cmd_params[0].value;
        dispatch_params[1].value = g_akao_cmd_params[1].value;
        dispatch_params[2].value = g_akao_cmd_params[2].value;
        g_akao_command_handlers[AKAO_CMD_FADE_MASTER_PAN_FROM](dispatch_params);
        opcode = AKAO_CMD_FADE_MASTER_VOLUME_FROM;
        break;

    case AKAO_CMD_PAUSE_ALL:
        g_akao_command_handlers[AKAO_CMD_PAUSE_SONG](dispatch_params);
        g_akao_command_handlers[AKAO_CMD_PAUSE_SFX](dispatch_params);
        opcode = AKAO_CMD_PAUSE_XA;
        break;

    case AKAO_CMD_RESUME_ALL:
        g_akao_command_handlers[AKAO_CMD_RESUME_SONG](dispatch_params);
        g_akao_command_handlers[AKAO_CMD_RESUME_SFX](dispatch_params);
        opcode = AKAO_CMD_RESUME_XA;
        break;

    default:
        dispatch_params[0].value = g_akao_cmd_params[0].value;
        dispatch_params[1].value = g_akao_cmd_params[1].value;
        dispatch_params[2].value = g_akao_cmd_params[2].value;
        dispatch_params[3].value = g_akao_cmd_params[3].value;
        dispatch_params[4].value = g_akao_cmd_params[4].value;
        dispatch_params[5].value = g_akao_cmd_params[5].value;
        break;
    }

    g_akao_command_handlers[opcode](dispatch_params);
    EnableEvent(g_akao_rcnt2_event);
    return result;
}
