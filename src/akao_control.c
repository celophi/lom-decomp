/**
 * @file akao_control.c
 * @brief AKAO song and sound-effect control: start/stop/suspend of songs,
 *        SFX playback, volume/pan/pitch fades and the driver command dispatcher.
 * @note Its own object in the original link: the libspu objects S_SNC, S_GVEX
 *       and S_SRMD sit between akao_voice.o and this file.
 */
#include "akao_voice.h"
#include "akao_sequencer.h"
#include "akao_control.h"
#include "sdk/libspu.h"
#include "sdk/libapi.h"

/** @brief Number of SFX channel slots. */
#define AKAO_SFX_CHANNEL_COUNT 12

/**
 * @brief Resolve one u16 offset of an SFX list entry pair.
 * @note Offsets are relative to the end of the 4-byte entry pair; @p skip is the
 *       distance from @p entry to that end. The offset is added first, as an
 *       integer, because the original code adds it before the entry address.
 */
#define SFX_ENTRY_DATA(entry, skip) ((u8*)((u32)(*(u16*)(entry)) + (u32)(entry) + (skip)))

extern s32 D_8003EC34[];
extern u8 D_8003D248[];
extern AkaoChannelState D_8004C038[];
extern s32 D_8004C2FC[];
extern u8 D_8004D450[];
extern s16 D_8004C32E[];
extern s32 D_8004D39C;
extern s32 D_8004D410;
extern AkaoCommandParam D_8004D340[6];
extern void (*D_8003DDE0[])(AkaoCommandParam*);
extern void (*D_8003E120[])(AkaoCommandParam*);
extern void (*D_8003E124[])(AkaoCommandParam*);
extern void (*D_8003E128[])(AkaoCommandParam*);

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
    channel->unk48 = 0x32000000;
    channel->seq_cursor = seq_data;
    channel->transpose = 0;
    channel->detune = 0;
    channel->portamento_speed = 0;
    channel->unk30 = 0;
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
    channel->reverb_toggle_ticks = 0;
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
 * @brief Initialize the primary song-sequencer state and start playback of a
 *        song descriptor.
 *
 * Records @p song_data into the (song-role) @c pitch field, seeds
 * voice_alloc_low_mask/static_voice_mask and the seq_cursor flag word from
 * the descriptor, sets up the articulation-map and note-table self-relative
 * pointers, then walks all 32 channels: channels selected by @p start_mask
 * get a full note-start reset (LFOs, envelopes, timers) plus a fresh
 * articulation; channels present in the descriptor's own mask but not in
 * @p start_mask instead get a lighter "silence" reset. Finishes by
 * resetting the song's tempo/master-volume/measure state to defaults.
 *
 * @param song_data Song descriptor: channel mask (+0x20), voice-mask seeds
 *        (+0x24/+0x28), song id (+0x14), articulation-map and note-table
 *        self-relative offsets (+0x30/+0x34), and a per-channel note-pointer
 *        table (+0x40). No named struct yet.
 * @param start_mask Channels to actually start now.
 */
void akao_seq_start_song(u8* song_data, s32 start_mask)
{
    s32 channel_mask;
    s32 flags;
    s32 cleared_flags;
    s32 rel;
    u8* table;
    u8* rel_table30;
    u8* rel_table34;
    u8* descriptor;
    AkaoChannelState* channel;
    u32 i;
    s32 bit;
    s32 static_voice_mask;
    u8* silence_ptr;
    s32 driver_mode;
    AkaoChannelState* song_tables;
    AkaoChannelState* song;

    g_akao_seq_channel0->pitch = (s32)song_data;
    channel_mask = *(s32*)(song_data + 0x20);
    descriptor = song_data;

    if (g_akao_seq_channel1 != NULL)
    {
        bit = akao_collect_voice_mask((AkaoChannelState*)g_akao_pending_channels, g_akao_seq_channel1->w04.song.active_mask);
    }
    else
    {
        bit = 0;
    }

    g_akao_sfx_control.unkC |= (~bit & 0xFFFFFF) & ~(g_akao_sfx_control.unk0 | D_8004F76C[0]);
    driver_mode = g_akao_driver_mode_flags & 1;

    g_akao_seq_channel0->key_off_mask = 0;

    if (driver_mode)
    {
        g_akao_seq_channel0->w04.song.active_mask = 0;
        g_akao_seq_channel0->unk1C |= channel_mask & start_mask;
    }
    else
    {
        g_akao_seq_channel0->unk1C = 0;
        g_akao_seq_channel0->w04.song.active_mask |= channel_mask & start_mask;
    }

    g_akao_seq_channel0->w04.song.voice_alloc_low_mask = *(s32*)(descriptor + 0x24);
    song = g_akao_seq_channel0;
    song->w04.song.static_voice_mask = *(s32*)(descriptor + 0x28);

    flags = (s32)song->seq_cursor;
    cleared_flags = flags & ~0x63;
    song->seq_cursor = (u8*)cleared_flags;
    flags = *(s32*)(descriptor + 0x14);
    if (flags == D_8003EC34[0])
    {
        flags = cleared_flags | 0x40;
    }
    else
    {
        flags = cleared_flags | 0x20;
    }
    song->seq_cursor = (u8*)flags;

    table = NULL;
    rel = *(s32*)(descriptor + 0x30);
    song_tables = g_akao_seq_channel0;
    rel_table30 = descriptor + (rel + 0x30);
    if (rel != 0)
    {
        table = rel_table30;
    }
    song_tables->unk30 = (s32)table;

    table = NULL;
    rel = *(s32*)(descriptor + 0x34);
    rel_table34 = descriptor + (rel + 0x34);
    if (rel != 0)
    {
        table = rel_table34;
    }

    bit = 1;
    i = 0;
    channel = (AkaoChannelState*)g_akao_seq_channels;
    song_data += 0x40;
    silence_ptr = D_8003D248;

    song_tables->flags = (s32)table;
    song_tables->voice_alloc_base = 0;
    do
    {
        if ((channel_mask & bit) & start_mask)
        {
            channel->seq_cursor = song_data + *(u16*)song_data;
            channel->unk66 = 4;
            channel->unk68 = 2;
            channel->volume = 0x7F00;
            channel->unk48 = 0x3FFF0000;
            channel->volume_scale = 0x4000;
            channel->detune = 0;
            channel->transpose = 0;
            channel->portamento_speed = 0;
            channel->unk30 = 0;
            channel->pitch_slide_delta = 0;
            channel->pitch_slide_ticks = 0;
            channel->note_duration_adjust = 0;
            channel->note_duration = 0;
            channel->pan = 0x8000;
            channel->pan_fade_ticks = 0;
            channel->portamento_speed = 0;
            channel->unk8E = 0;
            channel->expression_fade_ticks = 0;
            channel->detune_pitch_delta = 0;
            channel->note_expression_ticks = 0;
            channel->pitch_scale = 0;
            channel->note_flags = 0;
            channel->pan_lfo_value = 0;
            channel->loop_depth = 0;
            static_voice_mask = g_akao_seq_channel0->w04.song.static_voice_mask;
            song_data += 2;
            channel->pan_lfo_depth = 0;
            channel->volume_lfo_depth = 0;
            channel->pitch_lfo_depth = 0;
            channel->pan_lfo_depth_fade_ticks = 0;
            channel->flags = static_voice_mask & bit;
            channel->flags = (channel->flags == 0) << 6;
            channel->volume_lfo_depth_fade_ticks = 0;
            channel->pitch_lfo_depth_fade_ticks = 0;
            channel->pitch_mod_toggle_ticks = 0;
            channel->reverb_toggle_ticks = 0;
            akao_channel_set_articulation(channel, 0);
        }
        else
        {
            if (channel_mask & bit)
            {
                if (!(bit & start_mask))
                {
                    song_data += 2;
                }
            }
            channel->unk66 = 3;
            channel->unk68 = 1;
            channel->seq_cursor = silence_ptr;
            channel->update_flags |= 0x4400;
            channel->spu_adsr_high = (channel->spu_adsr_high & 0xFFE0) | 5;
        }
        channel->voice = AKAO_VOICE_COUNT;
        channel_mask &= ~bit;
        channel++;
        i++;
        bit <<= 1;
    } while (i < AKAO_CHANNEL_COUNT);

    g_akao_seq_channel0->tempo = 0xFFFF0000;
    g_akao_seq_channel0->tempo_acc = 1;
    g_akao_seq_channel0->tempo_fade_ticks = 0;
    g_akao_seq_channel0->unk48 = 0;
    g_akao_seq_channel0->master_vol_fade_ticks = 0;
    g_akao_seq_channel0->unk4C = 0;
    g_akao_driver_flags.unk8 = 0;
    g_akao_seq_channel0->unk6A = 0;
    g_akao_seq_channel0->unk68 = 0;
    g_akao_seq_channel0->unk66 = 0;
    g_akao_seq_channel0->measure = 0;
    g_akao_seq_channel0->reverb_mask = 0;
    g_akao_seq_channel0->noise_mask = 0;
    g_akao_seq_channel0->pitch_mod_mask = 0;
    g_akao_seq_channel0->unk60 = 0;
    g_akao_seq_channel0->note_on_mask = 0;
    g_akao_seq_channel0->w04.song.key_on_mask = 0;
    g_akao_driver_flags.unk8 |= 0x100;
}

/**
 * @brief Stop a song: key off every voice, park its channels on the silent sequence and release its voices.
 * @param song Song state to stop.
 * @param channels Channel table owned by @p song.
 * @param song_key Song key that must match @p song, or 0 to stop unconditionally.
 */
void akao_seq_stop_song(AkaoChannelState* song, AkaoChannelState* channels, s32 song_key)
{
    u32 i;

    if (song->w04.song.active_mask == 0)
    {
        return;
    }
    if (song_key != 0)
    {
        if (song_key != song->unk5E)
        {
            return;
        }
    }
    song->key_off_mask = -1;

    i = 0x20;
    do
    {
        channels->unk66 = 3;
        channels->unk68 = 1;
        channels->seq_cursor = D_8003D248;
        channels++;
        i--;
    } while (i != 0);

    song->unk5E = 0;
    song->note_on_mask = 0;
    song->w04.song.key_on_mask = 0;

    for (i = 0; i < AKAO_VOICE_COUNT; i++)
    {
        if (D_8004F7C0[i] == song)
        {
            D_8004F7C0[i] = NULL;
            spu_set_voice_release_mode(i, 5, 3);
        }
    }
}

/**
 * @brief Stop SFX channels selected by id, by flag mask or by priority.
 * @param sfx_id SFX id to stop (-1 selects every tagged SFX), or the first channel index when @p mode is negative.
 * @param mode Channel flag mask to match; negative stops the channel pair at @p sfx_id; 0x40000000 stops the highest-priority channels.
 */
void akao_sfx_stop_channels(s32 sfx_id, s32 mode)
{
    AkaoChannelState* ch;
    s32 mask;
    u32 i;
    s32 active;
    s32 flags;
    s32 priority;
    s32 channel_priority;

    mask = AKAO_SFX_FIRST_CHANNEL_BIT;
    ch = (AkaoChannelState*)g_sfx_channels;
    active = g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10;

    if (mode & 0x0FFFFFFF)
    {
        for (i = 0; i < AKAO_SFX_CHANNEL_COUNT; i++, ch++, mask <<= 1)
        {
            if ((active & mask) && (ch->tempo_acc & mode))
            {
                flags = ch->flags;
                if (flags & 0x100000)
                {
                    ch->flags = flags | 0x200000;
                }
                else
                {
                    g_akao_sfx_control.unkC |= mask;
                    akao_sfx_release_channels(ch, mask);
                    ch->flags = 0;
                }
            }
        }
    }
    else if (mode < 0)
    {
        ch += sfx_id;
        mask <<= sfx_id;
        if (active & mask)
        {
            akao_sfx_stop_channels(ch->reverb_mask, 0);
        }
        mask <<= 1;
        ch++;
        if (active & mask)
        {
            akao_sfx_stop_channels(ch->reverb_mask, 0);
        }
        return;
    }
    else if (mode & 0x40000000)
    {
        for (i = 0; i < AKAO_SFX_CHANNEL_COUNT; i++, ch++, mask <<= 1)
        {
            if (ch->tempo_acc != 0)
            {
                active &= ~mask;
            }
        }

        ch = (AkaoChannelState*)g_sfx_channels;
        mask = AKAO_SFX_FIRST_CHANNEL_BIT;
        priority = 0;
        for (i = 0; i < AKAO_SFX_CHANNEL_COUNT; i++, ch++, mask <<= 1)
        {
            if (active & mask)
            {
                channel_priority = *(s32*)&ch->unk58;
                if (priority < channel_priority)
                {
                    priority = channel_priority;
                }
            }
        }

        ch = (AkaoChannelState*)g_sfx_channels;
        mask = AKAO_SFX_FIRST_CHANNEL_BIT;
        for (i = 0; i < AKAO_SFX_CHANNEL_COUNT; i++, ch++, mask <<= 1)
        {
            if ((active & mask) && (priority == *(s32*)&ch->unk58))
            {
                flags = ch->flags;
                if (flags & 0x100000)
                {
                    ch->flags = flags | 0x200000;
                }
                else
                {
                    g_akao_sfx_control.unkC |= mask;
                    akao_sfx_release_channels(ch, mask);
                    ch->flags = 0;
                }
            }
        }
    }
    else
    {
        for (i = 0; i < AKAO_SFX_CHANNEL_COUNT; i++, ch++, mask <<= 1)
        {
            if (active & mask)
            {
                if (sfx_id == -1)
                {
                    if ((s32)ch->reverb_mask < 0)
                    {
                        flags = ch->flags;
                        if (flags & 0x100000)
                        {
                            ch->flags = flags | 0x200000;
                        }
                        else
                        {
                            g_akao_sfx_control.unkC |= mask;
                            akao_sfx_release_channels(ch, mask);
                            ch->flags = 0;
                        }
                    }
                }
                else if ((s32)ch->reverb_mask == sfx_id)
                {
                    flags = ch->flags;
                    if (flags & 0x100000)
                    {
                        ch->flags = flags | 0x200000;
                    }
                    else
                    {
                        g_akao_sfx_control.unkC |= mask;
                        akao_sfx_release_channels(ch, mask);
                        ch->flags = 0;
                    }
                }
            }
        }
    }
    g_akao_driver_flags.unk8 |= 0x110;
}

/**
 * @brief Initialize one SFX channel from a play-parameter block and mark it active.
 * @param channel SFX channel to start.
 * @param params Play parameters: id, flags, pan bias, volume scale and articulation bank.
 * @param channel_mask Channel-mask bit of @p channel.
 * @param seq_data Sequence bytecode the channel plays.
 */
void akao_sfx_start_channel(AkaoChannelState* channel, AkaoCommandParam* params, s32 channel_mask, u8* seq_data)
{
    s32 n;

    channel->reverb_mask = params[0].value;
    channel->tempo_acc = params[1].value;
    channel->pan_bias = (u8)params[2].value << 8;
    channel->pan_bias_fade_ticks = 0;
    channel->pan = 0x8000;
    channel->pan_fade_ticks = 0;
    channel->volume_scale = ((u16)params[3].value & 0x7F) << 8;
    channel->unk8E = 0;
    channel->voice_alloc_base = params[4].value;
    channel->unk66 = 2;
    channel->unk68 = 1;
    channel->is_sfx_channel = 1;
    *(s32*)&channel->unk58 = -2;
    channel->noise_mask = 0;
    channel->unk88 = 0;
    akao_channel_init_state(channel, seq_data);

    D_8004F7C0[channel->voice] = NULL;
    spu_set_voice_release_mode(channel->voice, 5, 3);

    g_akao_sfx_control.unk0 |= channel_mask;
    g_akao_sfx_control.unkC |= channel_mask;
    channel_mask = ~channel_mask;
    g_akao_sfx_control.unk4 &= channel_mask;
    g_akao_sfx_control.unk8 &= channel_mask;
    g_akao_sfx_control.reverb_mask &= channel_mask;
    g_akao_sfx_control.noise_mask &= channel_mask;
    g_akao_sfx_control.pitch_mod_mask &= channel_mask;

    if (g_akao_driver_mode_flags & 2)
    {
        channel_mask = AKAO_SFX_FIRST_CHANNEL_BIT;
        channel = (AkaoChannelState*)g_sfx_channels;
        for (n = AKAO_SFX_CHANNEL_COUNT; n != 0; n--, channel++, channel_mask <<= 1)
        {
            if (g_akao_sfx_control.unk0 & channel_mask)
            {
                if (!(channel->tempo_acc & AKAO_SFX_FLAG_SUPPRESS))
                {
                    g_akao_sfx_control.unk0 &= ~channel_mask;
                    g_akao_sfx_control.unk10 |= channel_mask;
                }
            }
        }
    }
}

/**
 * @brief Detach an SPU voice from every sequence channel that owns it.
 * @param channels Sequence channel table to scan.
 * @param voice_index SPU voice being taken over.
 */
void akao_unassign_voice(AkaoChannelState* channels, u32 voice_index)
{
    u32 channel_index;
    s32 unassigned_voice;
    s32 bit;
    AkaoChannelState* song;

    if (voice_index < AKAO_VOICE_COUNT)
    {
        channel_index = 0;
        unassigned_voice = AKAO_VOICE_COUNT;
        song = g_akao_seq_channel0;
        do
        {
            if (channels->voice == voice_index)
            {
                bit = 1 << channel_index;
                channels->voice = unassigned_voice;
                song->note_on_mask &= ~bit;
            }
            channel_index++;
            channels++;
        } while (channel_index < AKAO_CHANNEL_COUNT);
    }
}

/**
 * @brief Start an SFX on one free SFX channel, or on a free adjacent pair when it has two sequences.
 * @param params Play parameters (see akao_sfx_start_channel); word 1 also selects channels to stop first.
 * @param seq_data0 Sequence for the first channel, or NULL.
 * @param seq_data1 Sequence for the second channel, or NULL.
 * @param skip_stop Nonzero to skip stopping the channels selected by params word 1.
 */
void akao_sfx_play(AkaoCommandParam* params, u8* seq_data0, u8* seq_data1, s32 skip_stop)
{
    AkaoChannelState* ch;
    u32 mask;
    u32 pair_bits;
    u32 test_mask;
    s32 busy;
    s32 n;
    s32 stop_mask;

    if (seq_data0 == 0 && seq_data1 == 0)
    {
        return;
    }

    if (skip_stop == 0)
    {
        stop_mask = params[1].value;
        if (stop_mask != 0)
        {
            akao_sfx_stop_channels(0, stop_mask);
        }
    }

    do
    {
        ch = D_8004C038;
        mask = 0x800000;
        busy = (g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10) | g_akao_xa_tracker.unkC;
        if (seq_data0 != 0 && seq_data1 != 0)
        {
            n = 0xB;
            ch--;
            mask = 0x400000;
            while (1)
            {
                pair_bits = mask << 1;
                test_mask = mask | pair_bits;
                if (busy & test_mask)
                {
                    n--;
                    ch--;
                    mask >>= 1;
                    if (n == 0)
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
            n = AKAO_SFX_CHANNEL_COUNT;
            for (; n != 0; n--, ch--, mask >>= 1)
            {
                if (!(busy & mask))
                {
                    break;
                }
            }
        }
        if (n != 0)
        {
            break;
        }
        akao_sfx_stop_channels(0, 0x40000000);
        if (busy == (s32)((g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10) | g_akao_xa_tracker.unkC))
        {
            return;
        }
    } while (n == 0);

    if (seq_data0 != 0)
    {
        akao_sfx_start_channel(ch, params, mask, seq_data0);
        akao_unassign_voice((AkaoChannelState*)g_akao_seq_channels, ch->voice);
    }
    if (seq_data1 != 0)
    {
        if (seq_data0 != 0)
        {
            ch++;
            mask <<= 1;
        }
        akao_sfx_start_channel(ch, params, mask, seq_data1);
        akao_unassign_voice((AkaoChannelState*)g_akao_seq_channels, ch->voice);
        if (seq_data0 != 0)
        {
            ch->flags |= 0x10000;
        }
    }
    g_akao_driver_flags.unk8 |= 0x110;
}

/**
 * @brief Resolve a program's two data pointers from the bank program table.
 *
 * Masks @p program_index to 10 bits and reads the two consecutive u16 offset
 * entries from the flat u16 table @c g_akao_bank_prog_base at halfword indices
 * @c 2*n and @c 2*n+1. Each entry is an offset into the region-C data block
 * (@c g_akao_bank_region_c); the sentinel @c 0xFFFF resolves to a null pointer.
 * Used by the SFX launcher to obtain the two seq-data pointers it passes to
 * akao_sfx_play.
 *
 * @param out0 Receives the resolved pointer for the first entry (2*n).
 * @param out1 Receives the resolved pointer for the second entry (2*n + 1).
 * @param program_index Program id; masked to 0..0x3FF.
 *
 */
void akao_resolve_program_data(s32* out0, s32* out1, s32 program_index)
{
    s32 result;

    program_index &= 0x3FF;
    program_index <<= 1;

    if (((u16*)g_akao_bank_prog_base)[program_index] != 0xFFFF)
    {
        result = g_akao_bank_region_c + ((u16*)g_akao_bank_prog_base)[program_index];
    }
    else
    {
        result = 0;
    }
    *out0 = result;

    program_index++;
    if (((u16*)g_akao_bank_prog_base)[program_index] != 0xFFFF)
    {
        result = g_akao_bank_region_c + ((u16*)g_akao_bank_prog_base)[program_index];
    }
    else
    {
        result = 0;
    }
    *out1 = result;
}

/**
 * @brief Flag every active channel of a song for a pending SPU volume re-apply.
 * @param song Song whose active_mask selects the channels.
 * @param channels Channel table owned by @p song.
 */
void akao_seq_flag_volume_update(AkaoChannelState* song, AkaoChannelState* channels)
{
    s32 mask;
    s32 bit;
    s32 song_mask;

    song_mask = song->w04.song.active_mask;
    if (song_mask != 0)
    {
        mask = song_mask;
        bit = 1;
        do
        {
            if (mask & bit)
            {
                mask ^= bit;
                channels->update_flags |= 3;
            }
            channels++;
            bit <<= 1;
        } while (mask != 0);
    }
}

/**
 * @brief Flag every SFX channel active in @c g_akao_sfx_control for a pending
 *        SPU volume re-apply.
 */
void akao_sfx_flag_volume_update(void)
{
    s32 mask;
    s32 bit;
    s32 sfx_mask;
    AkaoChannelState* channel;

    channel = (AkaoChannelState*)g_sfx_channels;
    sfx_mask = g_akao_sfx_control.unk0;
    if (sfx_mask != 0)
    {
        mask = sfx_mask;
        bit = AKAO_SFX_FIRST_CHANNEL_BIT;
        do
        {
            if (mask & bit)
            {
                mask ^= bit;
                channel->update_flags |= 3;
            }
            channel++;
            bit <<= 1;
        } while (mask != 0);
    }
}

/**
 * @brief Restore the previously-suspended song state from the backup slots
 *        and rebase every bytecode-relative channel pointer to @p descriptor.
 *
 * Restores @c g_akao_seq_channel0 (0x70 bytes) and the full @c g_akao_seq_channels
 * array from the @c D_8004C2D0 / @c D_8004D450 backup slots, then adds the
 * delta between @p descriptor and the previously recorded base (@c D_8004C2FC)
 * to every pointer field that was computed relative to the old descriptor
 * address (seq_cursor, the loop-cursor stack, key_off_mask's channel-role
 * pointer use).
 *
 * @param descriptor Newly (re)loaded song descriptor; same shape consumed by
 *        akao_seq_start_song.
 *
 */
void akao_seq_resume_song(u8* descriptor)
{
    AkaoChannelState* channel;
    AkaoChannelState* song;
    s32 flags;
    s32 delta;
    s32 mask;
    s32 bit;
    u32 i;
    s32 keep_mask;

    akao_copy_bytes((s32*)D_8004C2D0, (s32*)g_akao_seq_channel0, 0x70);
    akao_copy_bytes((s32*)D_8004D450, (s32*)g_akao_seq_channels, 0x2300);

    flags = (s32)g_akao_seq_channel0->seq_cursor & ~0x60;
    g_akao_seq_channel0->seq_cursor = (u8*)flags;
    song = g_akao_seq_channel0;
    if (*(s32*)(descriptor + 0x14) == D_8003EC34[0])
    {
        flags |= 0x40;
    }
    else
    {
        flags |= 0x20;
    }
    song->seq_cursor = (u8*)flags;
    g_akao_seq_channel0->pitch = (s32)descriptor;
    g_akao_seq_channel0->w04.song.key_on_mask = 0;

    g_akao_driver_flags.unk8 |= 0x90;

    mask = g_akao_seq_channel0->w04.song.active_mask;
    delta = (s32)descriptor - D_8004C2FC[0];
    g_akao_seq_channel0->unk30 += delta;
    g_akao_seq_channel0->flags += delta;
    g_akao_seq_channel0->w04.song.key_on_mask = g_akao_seq_channel0->note_on_mask;

    channel = (AkaoChannelState*)g_akao_seq_channels;
    i = 0x20;
    bit = 1;
    do
    {
        if (mask & bit)
        {
            channel->seq_cursor += delta;
            channel->key_off_mask += delta;
            channel->w04.loop_cursor[0] += delta;
            channel->w04.loop_cursor[1] += delta;
            channel->w04.loop_cursor[2] += delta;
            channel->w04.loop_cursor[3] += delta;
            channel->unk66 += 2;
            channel->unk68 += 2;
            channel->update_flags |= 0x1FF93;
        }
        else
        {
            channel->unk66 = 4;
            channel->unk68 = 2;
            channel->seq_cursor = D_8003D248;
        }
        channel->voice = AKAO_VOICE_COUNT;
        i--;
        channel++;
        bit <<= 1;
    } while (i != 0);

    mask = 0;
    if (g_akao_seq_channel1 != NULL)
    {
        mask = akao_collect_voice_mask((AkaoChannelState*)g_akao_pending_channels,
                                       g_akao_seq_channel1->w04.song.active_mask & g_akao_seq_channel1->w04.song.voice_alloc_low_mask);
    }

    g_akao_seq_channel0->key_off_mask = 0;
    D_8004C32E[0] = 0;
    keep_mask = 0xFFFFFF;
    g_akao_sfx_control.unkC |= (~mask & (~(g_akao_sfx_control.unk0 | D_8004F76C[0]) & keep_mask));
    g_akao_driver_flags.unk8 |= 0x100;

    if (g_akao_driver_mode_flags & 1)
    {
        g_akao_seq_channel0->unk1C = g_akao_seq_channel0->w04.song.active_mask;
        g_akao_seq_channel0->w04.song.active_mask = 0;
    }
}

/**
 * @brief Find which bank program slot (0-4) holds @p key, or 5 if @p key is
 *        the reserved "always resident" sentinel.
 *
 * @param key Bank program key to search for; 0 means "no bank".
 * @return Slot index 0-4 if found among @c g_akao_bank_slot_keys, 5 if
 *         @p key matches the @c D_8004D39C sentinel, otherwise 0.
 */
s32 akao_bank_find_slot(s32 key)
{
    s32 result;
    s32 index;
    s32* slot;
    s32* base;

    result = 0;
    if (key != 0)
    {
        result = 6;
        slot = &D_8004D39C;
        do
        {
            if (key == *slot)
            {
                result--;
                break;
            }

            result--;
            if (result != 0)
            {
                base = g_akao_bank_slot_keys;
                index = result - 1;
                slot = base + index;
            }
        } while (result != 0);
    }

    return result;
}

/**
 * @brief Dispatch a (re)loaded song descriptor to resume or a fresh start.
 *
 * @p params carries the descriptor pointer at offset 0 and an id at
 * offset 8. If the backed-up song state's @c unk5E matches that id, the
 * descriptor is the same song reloaded at a new address, so the suspended
 * state is resumed (akao_seq_resume_song); otherwise it is a genuinely new
 * song (akao_seq_start_song), and the new id is recorded for next time.
 *
 * @param params Descriptor load result: u8* descriptor at +0x0, s32 id
 *        at +0x8.
 */
void akao_seq_reload_song(AkaoCommandParam* params)
{
    AkaoChannelState* backup;

    backup = (AkaoChannelState*)D_8004C2D0;
    if (backup->unk5E != 0 && backup->unk5E == params[2].value)
    {
        akao_seq_resume_song(params[0].buffer);
    }
    else
    {
        akao_seq_start_song(params[0].buffer, -1);
        g_akao_seq_channel0->unk5E = (u16)params[2].value;
    }
}

/**
 * @brief Start a freshly-loaded song descriptor with an explicit channel
 *        mask, and seed the pending tick countdown.
 *
 * @param params Descriptor load result: u8* descriptor at +0x0, id at
 *        +0x8, channel mask at +0xC, initial tick count at +0x10.
 */
void akao_seq_start_loaded_song(AkaoCommandParam* params)
{
    s32 result;
    s32 ticks;

    akao_seq_start_song(params[0].buffer, params[3].value);
    g_akao_seq_channel0->unk5E = (u16)params[2].value;
    ticks = params[4].value;
    result = 0;
    if (ticks != 0)
    {
        result = ticks - 1;
    }
    g_akao_seq_pending_ticks = result;
}

/**
 * @brief Back up the current song state if a song is active.
 *
 * Counterpart to akao_seq_resume_song: saves @c g_akao_seq_channel0 (0x70
 * bytes) and the full @c g_akao_seq_channels array into the @c D_8004C2D0 /
 * @c D_8004D450 backup slots.
 */
void akao_seq_suspend_song(void)
{
    if (g_akao_seq_channel0->w04.song.active_mask != 0)
    {
        akao_copy_bytes((s32*)g_akao_seq_channel0, (s32*)D_8004C2D0, 0x70);
        akao_copy_bytes((s32*)g_akao_seq_channels, (s32*)D_8004D450, 0x2300);
    }
}

/**
 * @brief Start a new primary song, demoting the current one to the
 *        secondary slot first if it is still active and no other secondary
 *        song already holds that slot.
 *
 * When @c g_akao_seq_channel0 is active and @c g_akao_seq_channel1 is either
 * unset or not a genuinely occupied secondary song (its @c unk5E is 0), the
 * current primary state is copied into the @c D_8004C2D0 / @c D_8004D450
 * backup slots and @c g_akao_seq_channel1 / @c g_akao_pending_channels are
 * repointed there, so the old song keeps ticking as the secondary while the
 * new one takes over as primary.
 *
 * @param params Descriptor load result: u8* descriptor at +0x0, id at
 *        +0x8.
 */
void akao_seq_switch_song(AkaoCommandParam* params)
{
    if (g_akao_seq_channel0->w04.song.active_mask != 0)
    {
        if (g_akao_seq_channel1 == NULL || g_akao_seq_channel1->unk5E == 0)
        {
            g_akao_seq_channel1 = (AkaoChannelState*)D_8004C2D0;
            g_akao_pending_channels = (s32)D_8004D450;
            akao_copy_bytes((s32*)g_akao_seq_channel0, (s32*)D_8004C2D0, 0x70);
            akao_copy_bytes((s32*)g_akao_seq_channels, (s32*)g_akao_pending_channels, 0x2300);
        }
    }
    akao_seq_start_song(params[0].buffer, -1);
    g_akao_seq_channel0->unk5E = (u16)params[2].value;
}

/**
 * @brief akao_seq_reload_song, then seed the pending tick countdown from
 *        the load result.
 *
 * @param params Descriptor load result: passed through to
 *        akao_seq_reload_song; initial tick count at +0x10.
 */
void akao_seq_reload_song_with_ticks(AkaoCommandParam* params)
{
    s32 result;
    s32 ticks;

    akao_seq_reload_song(params);
    ticks = params[4].value;
    result = 0;
    if (ticks != 0)
    {
        result = ticks - 1;
    }
    g_akao_seq_pending_ticks = result;
}

/**
 * @brief Play an SFX with fixed default parameters (reverb_mask 0x400,
 *        tempo_acc 0x1000000, pan_bias byte 0x80, volume_scale 0x7F,
 *        voice_alloc_base 0).
 *
 * @p params doubles as input and output: on entry, offsets +0x0/+0x4 hold
 * the two seq_data pointers to play; this function reads them out first,
 * then overwrites the whole buffer with the default parameter block before
 * calling akao_sfx_play.
 *
 * @param params Buffer holding the two seq_data pointers on entry; rebuilt
 *        in place into an akao_sfx_play parameter block.
 */
void akao_sfx_play_default(AkaoCommandParam* params)
{
    u8* seq_data0;
    u8* seq_data1;

    seq_data0 = params[0].buffer;
    seq_data1 = params[1].buffer;
    params[0].value = 0x400;
    params[1].value = 0x1000000;
    params[2].value = 0x80;
    params[3].value = 0x7F;
    params[4].value = 0;
    akao_sfx_play(params, seq_data0, seq_data1, 0);
}

/**
 * @brief Play an SFX resolved from a bank program index, tagging the
 *        channel with the bank slot that program resolved to.
 *
 * @p params holds the program index at offset +0x0 on entry.
 * akao_resolve_program_data resolves it to the two seq_data pointers, the
 * program's key is looked up in @c g_akao_bank_region_b to find which bank
 * slot holds it (akao_bank_find_slot), and the fixed tempo_acc/pan_bias/
 * volume_scale fields are filled in like akao_sfx_play_default.
 *
 * @param params Buffer holding the program index on entry; rebuilt in
 *        place into an akao_sfx_play parameter block.
 */
void akao_sfx_play_program(AkaoCommandParam* params)
{
    s32 seq_data0;
    s32 seq_data1;
    u16 program_key;
    s32 slot;

    akao_resolve_program_data(&seq_data0, &seq_data1, params[0].value);
    params[1].value = 0x2000000;
    params[2].value = 0x80;
    params[3].value = 0x7F;
    program_key = *(u16*)(g_akao_bank_region_b + params[0].value * 2);
    slot = akao_bank_find_slot(program_key);
    params[4].value = slot;
    akao_sfx_play(params, (u8*)seq_data0, (u8*)seq_data1, 0);
}

/**
 * @brief Play an SFX resolved from a bank program index, tagging the
 *        channel with the bank slot that program resolved to, without
 *        touching the reverb/tempo/pan/volume fields (caller-set).
 *
 * @param params Buffer holding the program index at +0x0 on entry; its
 *        voice_alloc_base (+0x10) is filled in before the call.
 */
void akao_sfx_play_program_raw(AkaoCommandParam* params)
{
    s32 seq_data0;
    s32 seq_data1;
    u16 program_key;
    s32 slot;

    akao_resolve_program_data(&seq_data0, &seq_data1, params[0].value);
    program_key = *(u16*)(g_akao_bank_region_b + params[0].value * 2);
    slot = akao_bank_find_slot(program_key);
    params[4].value = slot;
    akao_sfx_play(params, (u8*)seq_data0, (u8*)seq_data1, 0);
}

/**
 * @brief Play a list of SFX entries back to back, tagging the channel with
 *        the resolved bank slot and stopping other channels only for the
 *        first entry.
 *
 * @p params holds a pointer to a list header at +0x0: +0x4 the entry
 * count, +0x8 the bank program key (for akao_bank_find_slot), +0x10 an
 * array of s32 offsets into the string/data region starting at +0x20. Each
 * entry resolves two u16 sub-offsets the same way akao_resolve_program_data
 * does (0xFFFF means no data). The first entry plays with skip_stop=0, the
 * rest with skip_stop=1.
 *
 * @param params Buffer holding the list pointer on entry; voice_alloc_base
 *        (+0x10) is filled in before the calls.
 *
 */
void akao_sfx_play_list(AkaoCommandParam* params)
{
    u8* list0;
    u8* list;
    u8* base;
    u8* cursor;
    u8* index_ptr;
    s32 count;
    s32 sentinel;
    u8* seq_data0;
    u8* seq_data1;

    list0 = params[0].buffer;
    params[4].value = akao_bank_find_slot(*(s32*)(list0 + 8));

    list = params[0].buffer;
    index_ptr = list + 0x10;
    base = list;
    base += 0x20;
    count = *(s32*)(list + 4);

    cursor = base + *(s32*)index_ptr;
    if (*(u16*)cursor != 0xFFFF)
    {
        seq_data0 = SFX_ENTRY_DATA(cursor, 4);
    }
    else
    {
        seq_data0 = NULL;
    }
    cursor += 2;
    if (*(u16*)cursor != 0xFFFF)
    {
        seq_data1 = SFX_ENTRY_DATA(cursor, 2);
    }
    else
    {
        seq_data1 = NULL;
    }
    akao_sfx_play(params, seq_data0, seq_data1, 0);

    sentinel = 0xFFFF;
    count--;
    if (count != 0)
    {
        index_ptr += 4;
        do
        {
            cursor = base + *(s32*)index_ptr;
            if (*(u16*)cursor != sentinel)
            {
                seq_data0 = SFX_ENTRY_DATA(cursor, 4);
            }
            else
            {
                seq_data0 = NULL;
            }
            cursor += 2;
            if (*(u16*)cursor != sentinel)
            {
                seq_data1 = SFX_ENTRY_DATA(cursor, 2);
            }
            else
            {
                seq_data1 = NULL;
            }
            akao_sfx_play(params, seq_data0, seq_data1, 1);
            count--;
            index_ptr += 4;
        } while (count != 0);
    }
}

/**
 * @brief Stop SFX channels using a (sfx_id, mode) pair read from a buffer.
 * @param params sfx_id at +0x0, mode at +0x4; see akao_sfx_stop_channels.
 */
void akao_sfx_stop_channels_from_params(AkaoCommandParam* params)
{
    akao_sfx_stop_channels(params[0].value, params[1].value);
}

/**
 * @brief Set a song's master volume directly, canceling any in-progress
 *        fade, and flag its channels for a volume re-apply.
 *
 * @p params holds a song id at +0x0 (0 always means the primary song) and
 * the new volume (7 bits) at +0x4. Matches against @c g_akao_seq_channel0
 * first, then @c g_akao_seq_channel1, and is a no-op if neither matches.
 *
 * @param params Song id at +0x0, new volume at +0x4.
 *
 */
void akao_seq_set_master_volume(AkaoCommandParam* params)
{
    s32 id;
    s32 volume;

    id = params[0].value;
    if (id == 0 || id == g_akao_seq_channel0->unk5E)
    {
        volume = (params[1].value & 0x7F) << 16;
        g_akao_seq_channel0->pitch_slide_step = volume;
        g_akao_seq_channel0->unk58 = 0;
        akao_seq_flag_volume_update(g_akao_seq_channel0, (AkaoChannelState*)g_akao_seq_channels);
    }
    else if (g_akao_seq_channel1 != NULL && id != 0 && id == g_akao_seq_channel1->unk5E)
    {
        AkaoChannelState* pending;

        pending = g_akao_pending_channels ? (AkaoChannelState*)g_akao_pending_channels : (AkaoChannelState*)g_akao_pending_channels;
        volume = params[1].value;
        g_akao_seq_channel1->unk58 = 0;
        volume = (volume & 0x7F) << 16;
        g_akao_seq_channel1->pitch_slide_step = volume;
        akao_seq_flag_volume_update(g_akao_seq_channel1, pending);
    }
}

/**
 * @brief Start a linear fade of a song's master volume to a target level
 *        over a given tick count, and flag its channels for a volume
 *        re-apply.
 *
 * @p params holds a song id at +0x0 (0 always means the primary song), a
 * tick count at +0x4 (0 is treated as 1), and the target volume (7 bits)
 * at +0x8. The per-tick step is computed as (target - current) / ticks and
 * stored at detune_pitch_delta; unk58 is set to the tick count so the
 * per-tick fade in akao_tick_channel_effects picks it up.
 *
 * @param params Song id at +0x0, tick count at +0x4, target volume at +0x8.
 */
void akao_seq_fade_master_volume(AkaoCommandParam* params)
{
    s32 id;
    s32 raw_ticks;
    s32 ticks;
    s32 target_volume;
    s32 current_volume;
    s32 step;
    AkaoChannelState* channels;

    raw_ticks = params[1].value;
    ticks = 1;
    if (raw_ticks != 0)
    {
        ticks = raw_ticks;
    }
    target_volume = (params[2].value & 0x7F) << 16;
    id = params[0].value;
    if (id == 0 || id == g_akao_seq_channel0->unk5E)
    {
        current_volume = g_akao_seq_channel0->pitch_slide_step;
        target_volume -= current_volume;
        step = target_volume / ticks;
        channels = (AkaoChannelState*)g_akao_seq_channels;
        g_akao_seq_channel0->unk58 = ticks;
        g_akao_seq_channel0->detune_pitch_delta = step;
        akao_seq_flag_volume_update(g_akao_seq_channel0, channels);
    }
    else if (g_akao_seq_channel1 != NULL && id != 0 && id == g_akao_seq_channel1->unk5E)
    {
        current_volume = g_akao_seq_channel1->pitch_slide_step;
        target_volume -= current_volume;
        step = target_volume / ticks;
        channels = (AkaoChannelState*)g_akao_pending_channels;
        g_akao_seq_channel1->unk58 = ticks;
        g_akao_seq_channel1->detune_pitch_delta = step;
        akao_seq_flag_volume_update(g_akao_seq_channel1, channels);
    }
}

/**
 * @brief Jump a song's master volume to an explicit start level, then fade
 *        it to a target level over a given tick count.
 *
 * Same song/tick resolution as akao_seq_fade_master_volume, but the start
 * volume comes from @p params +0x8 (written immediately) instead of the
 * song's current volume, and the target comes from +0xC.
 *
 * @param params Song id at +0x0, tick count at +0x4, start volume at +0x8,
 *        target volume at +0xC.
 */
void akao_seq_fade_master_volume_from(AkaoCommandParam* params)
{
    s32 id;
    s32 raw_ticks;
    s32 ticks;
    s32 start_volume;
    s32 target_volume;
    s32 step;
    AkaoChannelState* channels;
    AkaoChannelState* song;

    raw_ticks = params[1].value;
    ticks = 1;
    if (raw_ticks != 0)
    {
        ticks = raw_ticks;
    }
    id = params[0].value;
    if (id == 0 || id == g_akao_seq_channel0->unk5E)
    {
        song = g_akao_seq_channel0;
        channels = (AkaoChannelState*)g_akao_seq_channels;
    }
    else if (g_akao_seq_channel1 != NULL && id != 0 && id == g_akao_seq_channel1->unk5E)
    {
        song = g_akao_seq_channel1;
        channels = (AkaoChannelState*)g_akao_pending_channels;
    }
    else
    {
        return;
    }
    start_volume = (params[2].value & 0x7F) << 16;
    song->pitch_slide_step = start_volume;
    target_volume = (params[3].value & 0x7F) << 16;
    target_volume -= start_volume;
    step = target_volume / ticks;
    song->unk58 = ticks;
    song->detune_pitch_delta = step;
    akao_seq_flag_volume_update(song, channels);
}

/**
 * @brief Set the CD-audio volume accumulator directly, canceling any
 *        in-progress fade, and push it to the SPU immediately.
 * @param params Target volume (u16, shifted into the high half) at +0x0.
 */
void akao_set_cd_volume(AkaoCommandParam* params)
{
    s32 volume;

    volume = (u16)params[0].value;
    g_akao_cdvol_fade_ticks = 0;
    volume = volume << 16;
    g_akao_cdvol_acc = volume;
    akao_apply_cdvol_to_spu();
}

/**
 * @brief Start a linear fade of the CD-audio volume accumulator to a
 *        target level over a given tick count.
 *
 * @param params Tick count at +0x0 (0 is treated as 1), target volume
 *        (u16, shifted into the high half like the master-volume fades)
 *        at +0x4.
 */
void akao_fade_cd_volume(AkaoCommandParam* params)
{
    s32 raw_ticks;
    s32 ticks;
    s32 target;
    s32 step;

    raw_ticks = params[0].value;
    ticks = 1;
    if (raw_ticks != 0)
    {
        ticks = raw_ticks;
    }
    target = (u16)params[1].value;
    target = target << 16;
    target -= g_akao_cdvol_acc;
    step = target / ticks;
    g_akao_cdvol_fade_ticks = ticks;
    g_akao_cdvol_step = step;
}

/**
 * @brief Jump the CD-audio volume accumulator to an explicit start level,
 *        then fade it to a target level over a given tick count.
 * @param params Tick count at +0x0, start volume (u16) at +0x4, target
 *        volume (u16) at +0x8.
 */
void akao_fade_cd_volume_from(AkaoCommandParam* params)
{
    s32 raw_ticks;
    s32 ticks;
    s32 target;
    s32 start;
    s32 step;

    raw_ticks = params[0].value;
    ticks = 1;
    if (raw_ticks != 0)
    {
        ticks = raw_ticks;
    }
    target = (u16)params[2].value;
    start = (u16)params[1].value;
    target = target << 16;
    start = start << 16;
    target -= start;
    step = target / ticks;
    g_akao_cdvol_fade_ticks = ticks;
    g_akao_cdvol_acc = start;
    g_akao_cdvol_step = step;
}

/**
 * @brief Apply a new volume scale to active SFX channels, selected either
 *        by a tempo_acc mode mask or by an exact sfx id match.
 *
 * When @p params +0x4 is non-zero, it is used as a bitmask tested against
 * each active channel's tempo_acc (secondary flag word). Otherwise, each
 * active channel's reverb_mask (which doubles as the sfx id tag for SFX
 * channels; see akao_sfx_stop_channels) is compared against @p params +0x0.
 *
 * @param params sfx id at +0x0, tempo_acc mode mask at +0x4 (0 selects the
 *        id-match mode instead), new volume scale (7 bits) at +0x8.
 */
void akao_sfx_set_volume_scale(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s32 scale;

    channel = (AkaoChannelState*)g_sfx_channels;
    active = g_akao_sfx_control.unk0;
    bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    if (params[1].value != 0)
    {
        for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && (channel->tempo_acc & params[1].value))
            {
                scale = (u16)params[2].value;
                channel->unk8E = 0;
                channel->volume_scale = (scale & 0x7F) << 8;
                channel->update_flags |= 3;
            }
        }
    }
    else
    {
        bit = AKAO_SFX_FIRST_CHANNEL_BIT;
        for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && ((s32)channel->reverb_mask == params[0].value))
            {
                scale = (u16)params[2].value;
                channel->unk8E = 0;
                channel->volume_scale = (scale & 0x7F) << 8;
                channel->update_flags |= 3;
            }
        }
    }
}

/**
 * @brief Start a fade of the volume scale on active SFX channels, selected
 *        either by a tempo_acc mode mask or by an exact sfx id match (same
 *        selection rule as akao_sfx_set_volume_scale).
 *
 * @param params sfx id at +0x0, tempo_acc mode mask at +0x4 (0 selects the
 *        id-match mode instead), tick count at +0x8 (0 is treated as 1),
 *        target volume scale (7 bits) at +0xC.
 *
 */
void akao_sfx_fade_volume_scale(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s16 tick_count;
    u16 target_scale;
    s32 current_scale;
    s16 delta;

    channel = (AkaoChannelState*)g_sfx_channels;
    active = g_akao_sfx_control.unk0;
    bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    if (params[1].value != 0)
    {
        for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && (channel->tempo_acc & params[1].value))
            {
                if (params[2].value != 0)
                {
                    tick_count = (u16)params[2].value;
                }
                else
                {
                    tick_count = 1;
                }
                target_scale = ((u16)params[3].value & 0x7F) << 8;
                current_scale = channel->volume_scale;
                delta = target_scale - current_scale;
                channel->unkE6 = delta / tick_count;
                channel->unk8E = tick_count;
            }
        }
    }
    else
    {
        bit = AKAO_SFX_FIRST_CHANNEL_BIT;
        for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && ((s32)channel->reverb_mask == params[0].value))
            {
                if (params[2].value != 0)
                {
                    tick_count = (u16)params[2].value;
                }
                else
                {
                    tick_count = 1;
                }
                target_scale = ((u16)params[3].value & 0x7F) << 8;
                current_scale = channel->volume_scale;
                delta = target_scale - current_scale;
                channel->unkE6 = delta / tick_count;
                channel->unk8E = tick_count;
            }
        }
    }
}

/**
 * @brief Apply a new volume scale to every active SFX channel whose
 *        tempo_acc does not have the pan/volume-suppress bit (0x02000000)
 *        set.
 * @param params New volume scale (7 bits, u16) to apply.
 */
void akao_sfx_set_volume_scale_unsuppressed(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s32 scale;

    bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    active = g_akao_sfx_control.unk0;
    channel = (AkaoChannelState*)g_sfx_channels;
    for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
    {
        if ((active & bit) && !(channel->tempo_acc & AKAO_SFX_FLAG_SUPPRESS))
        {
            scale = (u16)params[0].value;
            channel->unk8E = 0;
            channel->volume_scale = (scale & 0x7F) << 8;
            channel->update_flags |= 3;
        }
    }
}

/**
 * @brief Start a fade of the volume scale on every active SFX channel
 *        whose tempo_acc does not have the suppress bit (0x02000000) set.
 * @param params Tick count at +0x0 (0 is treated as 1), target volume
 *        scale (7 bits, u16) at +0x4.
 *
 */
void akao_sfx_fade_volume_scale_unsuppressed(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s16 tick_count;
    u16 target_scale;
    s32 current_scale;
    s16 delta;

    bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    active = g_akao_sfx_control.unk0;
    channel = (AkaoChannelState*)g_sfx_channels;
    for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
    {
        if ((active & bit) && !(channel->tempo_acc & AKAO_SFX_FLAG_SUPPRESS))
        {
            if (params[0].value != 0)
            {
                tick_count = (u16)params[0].value;
            }
            else
            {
                tick_count = 1;
            }
            target_scale = ((u16)params[1].value & 0x7F) << 8;
            current_scale = channel->volume_scale;
            delta = target_scale - current_scale;
            channel->unkE6 = delta / tick_count;
            channel->unk8E = tick_count;
        }
    }
}

/**
 * @brief Set the pan bias on active SFX channels, selected either by a
 *        tempo_acc mode mask or by an exact sfx id match (same selection
 *        rule as akao_sfx_set_volume_scale), and cancel any pan-bias fade.
 * @param params sfx id at +0x0, tempo_acc mode mask at +0x4 (0 selects the
 *        id-match mode instead), new pan bias (u8) at +0x8.
 */
void akao_sfx_set_pan_bias(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s32 bias;

    channel = (AkaoChannelState*)g_sfx_channels;
    active = g_akao_sfx_control.unk0;
    bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    if (params[1].value != 0)
    {
        for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && (channel->tempo_acc & params[1].value))
            {
                bias = (u8)params[2].value;
                channel->pan_bias_fade_ticks = 0;
                channel->pan_bias = bias << 8;
                channel->update_flags |= 3;
            }
        }
    }
    else
    {
        bit = AKAO_SFX_FIRST_CHANNEL_BIT;
        for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && ((s32)channel->reverb_mask == params[0].value))
            {
                bias = (u8)params[2].value;
                channel->pan_bias_fade_ticks = 0;
                channel->pan_bias = bias << 8;
                channel->update_flags |= 3;
            }
        }
    }
}

/**
 * @brief Start a fade of the pan bias on active SFX channels, selected
 *        either by a tempo_acc mode mask or by an exact sfx id match (same
 *        selection rule as akao_sfx_set_volume_scale).
 * @param params sfx id at +0x0, tempo_acc mode mask at +0x4 (0 selects the
 *        id-match mode instead), tick count at +0x8 (0 is treated as 1),
 *        target pan bias (u8) at +0xC.
 */
void akao_sfx_fade_pan_bias(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s16 tick_count;
    u16 target_bias;
    s32 current_bias;
    s16 delta;

    channel = (AkaoChannelState*)g_sfx_channels;
    active = g_akao_sfx_control.unk0;
    bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    if (params[1].value != 0)
    {
        for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && (channel->tempo_acc & params[1].value))
            {
                if (params[2].value != 0)
                {
                    tick_count = (u16)params[2].value;
                }
                else
                {
                    tick_count = 1;
                }
                target_bias = (u8)params[3].value << 8;
                current_bias = channel->pan_bias;
                delta = target_bias - current_bias;
                channel->pan_bias_step = delta / tick_count;
                channel->pan_bias_fade_ticks = tick_count;
            }
        }
    }
    else
    {
        for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && ((s32)channel->reverb_mask == params[0].value))
            {
                if (params[2].value != 0)
                {
                    tick_count = (u16)params[2].value;
                }
                else
                {
                    tick_count = 1;
                }
                target_bias = (u8)params[3].value << 8;
                current_bias = channel->pan_bias;
                delta = target_bias - current_bias;
                channel->pan_bias_step = delta / tick_count;
                channel->pan_bias_fade_ticks = tick_count;
            }
        }
    }
}

/**
 * @brief Set the pan bias on every active SFX channel whose tempo_acc does
 *        not have the suppress bit (0x02000000) set, and cancel any
 *        pan-bias fade.
 * @param params New pan bias (u8) to apply.
 */
void akao_sfx_set_pan_bias_unsuppressed(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s32 bias;

    bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    active = g_akao_sfx_control.unk0;
    channel = (AkaoChannelState*)g_sfx_channels;
    for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
    {
        if ((active & bit) && !(channel->tempo_acc & AKAO_SFX_FLAG_SUPPRESS))
        {
            bias = (u8)params[0].value;
            channel->pan_bias_fade_ticks = 0;
            channel->pan_bias = bias << 8;
            channel->update_flags |= 3;
        }
    }
}

/**
 * @brief Start a fade of the pan bias on every active SFX channel whose
 *        tempo_acc does not have the suppress bit (0x02000000) set.
 * @param params Tick count at +0x0 (0 is treated as 1), target pan bias
 *        (u8) at +0x4.
 */
void akao_sfx_fade_pan_bias_unsuppressed(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s16 tick_count;
    u16 target_bias;
    s32 current_bias;
    s16 delta;

    bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    active = g_akao_sfx_control.unk0;
    channel = (AkaoChannelState*)g_sfx_channels;
    for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
    {
        if ((active & bit) && !(channel->tempo_acc & AKAO_SFX_FLAG_SUPPRESS))
        {
            if (params[0].value != 0)
            {
                tick_count = (u16)params[0].value;
            }
            else
            {
                tick_count = 1;
            }
            target_bias = (u8)params[1].value << 8;
            current_bias = channel->pan_bias;
            delta = target_bias - current_bias;
            channel->pan_bias_step = delta / tick_count;
            channel->pan_bias_fade_ticks = tick_count;
        }
    }
}

/**
 * @brief Set a per-channel value at offset +0x40 (channel-role meaning not
 *        yet identified; +0x40 is only documented for the song role, as
 *        noise_mask) on active SFX channels, selected either by a
 *        tempo_acc mode mask or by an exact sfx id match (same selection
 *        rule as akao_sfx_set_volume_scale), and flag a pending pitch
 *        update (update_flags bit 0x10).
 * @param params sfx id at +0x0, tempo_acc mode mask at +0x4 (0 selects the
 *        id-match mode instead), new value (u8, shifted into the high
 *        byte) at +0x8.
 */
void akao_sfx_set_pitch_bend(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s32 value;

    channel = (AkaoChannelState*)g_sfx_channels;
    active = g_akao_sfx_control.unk0;
    bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    if (params[1].value != 0)
    {
        for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && (channel->tempo_acc & params[1].value))
            {
                value = (u8)params[2].value;
                channel->unk88 = 0;
                channel->noise_mask = value << 8;
                channel->update_flags |= 0x10;
            }
        }
    }
    else
    {
        bit = AKAO_SFX_FIRST_CHANNEL_BIT;
        for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && ((s32)channel->reverb_mask == params[0].value))
            {
                value = (u8)params[2].value;
                channel->unk88 = 0;
                channel->noise_mask = value << 8;
                channel->update_flags |= 0x10;
            }
        }
    }
}

/**
 * @brief Start a fade of the same +0x40 per-channel value that
 *        akao_sfx_set_pitch_bend sets, on active SFX channels selected
 *        either by a tempo_acc mode mask or by an exact sfx id match.
 * @param params sfx id at +0x0, tempo_acc mode mask at +0x4 (0 selects the
 *        id-match mode instead), tick count at +0x8 (0 is treated as 1),
 *        target value (u8) at +0xC.
 *
 */
void akao_sfx_fade_pitch_bend(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s16 tick_count;
    u16 target;
    s32 current;
    s16 delta;
    s16 step;

    channel = (AkaoChannelState*)g_sfx_channels;
    active = g_akao_sfx_control.unk0;
    bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    if (params[1].value != 0)
    {
        for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && (channel->tempo_acc & params[1].value))
            {
                if (params[2].value != 0)
                {
                    tick_count = (u16)params[2].value;
                }
                else
                {
                    tick_count = 1;
                }
                target = (u8)params[3].value << 8;
                current = HALF_LOW_U16(channel->noise_mask);
                delta = target - current;
                step = delta / tick_count;
                channel->pitch_mod_mask = step;
                channel->unk88 = tick_count;
            }
        }
    }
    else
    {
        bit = AKAO_SFX_FIRST_CHANNEL_BIT;
        for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && ((s32)channel->reverb_mask == params[0].value))
            {
                if (params[2].value != 0)
                {
                    tick_count = (u16)params[2].value;
                }
                else
                {
                    tick_count = 1;
                }
                target = (u8)params[3].value << 8;
                current = HALF_LOW_U16(channel->noise_mask);
                delta = target - current;
                step = delta / tick_count;
                channel->pitch_mod_mask = step;
                channel->unk88 = tick_count;
            }
        }
    }
}

/**
 * @brief Set the same +0x40 per-channel value that akao_sfx_set_pitch_bend
 *        sets, on every SFX channel whose tempo_acc does not have the
 *        suppress bit (0x02000000) set (no active-channel filter here).
 * @param params New value (u8, shifted into the high byte) to apply.
 */
void akao_sfx_set_pitch_bend_unsuppressed(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    u32 count;
    s32 value;

    channel = (AkaoChannelState*)g_sfx_channels;
    for (count = AKAO_SFX_CHANNEL_COUNT; count != 0; count--, channel++)
    {
        if (!(channel->tempo_acc & AKAO_SFX_FLAG_SUPPRESS))
        {
            value = (u8)params[0].value;
            channel->unk88 = 0;
            channel->noise_mask = value << 8;
            channel->update_flags |= 0x10;
        }
    }
}

/**
 * @brief Start a fade of the same +0x40 per-channel value that
 *        akao_sfx_set_pitch_bend sets, on every active SFX channel whose
 *        tempo_acc does not have the suppress bit (0x02000000) set.
 * @param params Tick count at +0x0 (0 is treated as 1), target value (u8)
 *        at +0x4.
 *
 */
void akao_sfx_fade_pitch_bend_unsuppressed(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    s32 active;
    s32 bit;
    u32 count;
    s16 tick_count;
    u16 target;
    s32 current;
    s16 delta;
    s16 step;

    bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    active = g_akao_sfx_control.unk0;
    channel = (AkaoChannelState*)g_sfx_channels;
    for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
    {
        if ((active & bit) && !(channel->tempo_acc & AKAO_SFX_FLAG_SUPPRESS))
        {
            if (params[0].value != 0)
            {
                tick_count = (u16)params[0].value;
            }
            else
            {
                tick_count = 1;
            }
            target = (u8)params[1].value << 8;
            current = HALF_LOW_U16(channel->noise_mask);
            delta = target - current;
            step = delta / tick_count;
            channel->pitch_mod_mask = step;
            channel->unk88 = tick_count;
        }
    }
}

/**
 * @brief Set the master pan accumulator directly, canceling any
 *        in-progress fade.
 * @param params Signed target pan value (byte, shifted into the high half).
 */
void akao_set_master_pan(AkaoCommandParam* params)
{
    s32 pan;

    pan = (s8)params[0].value;
    g_akao_masterpan_fade_ticks = 0;
    pan = pan << 16;
    g_akao_masterpan_acc = pan;
}

/**
 * @brief Start a linear fade of the master pan accumulator to a target
 *        level over a given tick count.
 * @param params Tick count at +0x0 (0 is treated as 1), signed target pan
 *        (byte) at +0x4.
 */
void akao_fade_master_pan(AkaoCommandParam* params)
{
    s32 raw_ticks;
    s32 ticks;
    s32 target;
    s32 step;

    raw_ticks = params[0].value;
    ticks = 1;
    if (raw_ticks != 0)
    {
        ticks = raw_ticks;
    }
    target = (s8)params[1].value;
    target = target << 16;
    target -= g_akao_masterpan_acc;
    step = target / ticks;
    g_akao_masterpan_fade_ticks = ticks;
    g_akao_masterpan_step = step;
}

/**
 * @brief Jump the master pan accumulator to an explicit start level, then
 *        fade it to a target level over a given tick count.
 *
 * Unlike the other fade functions in this file, the tick-count value and
 * its zero-check read different offsets: the check reads +0x4 (the same
 * word as the start pan, tested as an s32 before being reread as a
 * signed byte), and the actual tick count comes from +0x0.
 *
 * @param params Tick count at +0x0, signed start pan (byte) at +0x4,
 *        signed target pan (byte) at +0x8.
 */
void akao_fade_master_pan_from(AkaoCommandParam* params)
{
    s32 raw_ticks;
    s32 ticks;
    s32 target;
    s32 start;
    s32 step;

    raw_ticks = params[1].value;
    ticks = 1;
    if (raw_ticks != 0)
    {
        ticks = params[0].value;
    }
    start = (s8)params[1].value;
    start = start << 16;
    g_akao_masterpan_acc = start;
    target = (s8)params[2].value;
    target = target << 16;
    target -= start;
    step = target / ticks;
    g_akao_masterpan_fade_ticks = ticks;
    g_akao_masterpan_step = step;
}

/**
 * @brief Set the driver-wide master volume accumulator directly,
 *        canceling any in-progress fade.
 * @param params Signed target volume (byte, shifted into the high half).
 */
void akao_set_driver_master_volume(AkaoCommandParam* params)
{
    s32 volume;

    volume = (s8)params[0].value;
    g_akao_mastervol_fade_ticks = 0;
    volume = volume << 16;
    g_akao_mastervol_acc = volume;
}

/**
 * @brief Start a linear fade of the driver-wide master volume accumulator
 *        to a target level over a given tick count.
 * @param params Tick count at +0x0 (0 is treated as 1), signed target
 *        volume (byte) at +0x4.
 */
void akao_fade_driver_master_volume(AkaoCommandParam* params)
{
    s32 raw_ticks;
    s32 ticks;
    s32 target;
    s32 step;

    raw_ticks = params[0].value;
    ticks = 1;
    if (raw_ticks != 0)
    {
        ticks = raw_ticks;
    }
    target = (s8)params[1].value;
    target = target << 16;
    target -= g_akao_mastervol_acc;
    step = target / ticks;
    g_akao_mastervol_fade_ticks = ticks;
    g_akao_mastervol_step = step;
}

/**
 * @brief Jump the driver-wide master volume accumulator to an explicit
 *        start level, then fade it to a target level over a given tick
 *        count.
 *
 * Same offset quirk as akao_fade_master_pan_from: the tick-count zero
 * check reads +0x4 (the same word as the start volume), the actual tick
 * count comes from +0x0.
 *
 * @param params Tick count at +0x0, signed start volume (byte) at +0x4,
 *        signed target volume (byte) at +0x8.
 */
void akao_fade_driver_master_volume_from(AkaoCommandParam* params)
{
    s32 raw_ticks;
    s32 ticks;
    s32 target;
    s32 start;
    s32 step;

    raw_ticks = params[1].value;
    ticks = 1;
    if (raw_ticks != 0)
    {
        ticks = params[0].value;
    }
    start = (s8)params[1].value;
    start = start << 16;
    g_akao_mastervol_acc = start;
    target = (s8)params[2].value;
    target = target << 16;
    target -= start;
    step = target / ticks;
    g_akao_mastervol_fade_ticks = ticks;
    g_akao_mastervol_step = step;
}

/**
 * @brief Unconditionally stop the primary song, and the secondary song
 *        too if one is loaded.
 */
void akao_seq_stop_all_songs(void)
{
    akao_seq_stop_song(g_akao_seq_channel0, (AkaoChannelState*)g_akao_seq_channels, 0);
    if (g_akao_seq_channel1 != NULL)
    {
        akao_seq_stop_song(g_akao_seq_channel1, (AkaoChannelState*)g_akao_pending_channels, 0);
    }
}

/**
 * @brief Stop the primary song by key, and the secondary song too if one
 *        is loaded and the key is nonzero.
 * @param params Song key at +0x0; see akao_seq_stop_song.
 */
void akao_seq_stop_song_by_key(AkaoCommandParam* params)
{
    s32 song_key;

    akao_seq_stop_song(g_akao_seq_channel0, (AkaoChannelState*)g_akao_seq_channels, params[0].value);
    if (g_akao_seq_channel1 != NULL)
    {
        song_key = params[0].value;
        if (song_key != 0)
        {
            akao_seq_stop_song(g_akao_seq_channel1, (AkaoChannelState*)g_akao_pending_channels, song_key);
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
    s32 bit;
    u32 count;

    channel = (AkaoChannelState*)g_sfx_channels;
    bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
    {
        if ((g_akao_sfx_control.unk0 & bit) && !(channel->tempo_acc & AKAO_SFX_FLAG_SUPPRESS))
        {
            g_akao_sfx_control.unkC |= bit;
            akao_sfx_release_channels(channel, bit);
            channel->flags = 0;
        }
    }
    g_akao_driver_flags.unk8 |= 0x110;
}

/**
 * @brief Flag every channel of every active song and every active SFX
 *        channel for a pending SPU volume re-apply.
 */
void akao_flag_all_volume_updates(void)
{
    D_8004F754[0] = 1;
    akao_seq_flag_volume_update(g_akao_seq_channel0, (AkaoChannelState*)g_akao_seq_channels);
    if (g_akao_seq_channel1 != NULL)
    {
        akao_seq_flag_volume_update(g_akao_seq_channel1, (AkaoChannelState*)g_akao_pending_channels);
    }
    akao_sfx_flag_volume_update();
}

/**
 * @brief Flag every channel of every active song and every active SFX
 *        channel for a pending SPU update, tagged mode 2 (see
 *        akao_flag_all_volume_updates, tagged mode 1).
 */
void akao_flag_all_pan_updates(void)
{
    D_8004F754[0] = 2;
    akao_seq_flag_volume_update(g_akao_seq_channel0, (AkaoChannelState*)g_akao_seq_channels);
    if (g_akao_seq_channel1 != NULL)
    {
        akao_seq_flag_volume_update(g_akao_seq_channel1, (AkaoChannelState*)g_akao_pending_channels);
    }
    akao_sfx_flag_volume_update();
}

/**
 * @brief Store a new value into D_8003EC6C, then flag every primary song
 *        channel for a pending SPU update.
 * @param params New value for D_8003EC6C.
 */
void akao_set_mode_flag_and_flag_all_channels(AkaoCommandParam* params)
{
    AkaoChannelState* channel;
    u32 count;

    D_8003EC6C = params[0].value;
    channel = (AkaoChannelState*)g_akao_seq_channels;
    count = 0;
    do
    {
        count++;
        channel->update_flags |= 3;
        channel++;
    } while (count < AKAO_CHANNEL_COUNT);
}

/**
 * @brief Set the primary song's conditional-jump variable (unk60).
 * @param params New value in word 0.
 */
void akao_seq_set_unk60(AkaoCommandParam* params)
{
    g_akao_seq_channel0->unk60 = (u16)params[0].value;
}

/**
 * @brief Silence every SPU voice not currently claimed by an SFX channel
 *        or the XA/streaming reservation, park the primary song's
 *        active_mask into unk1C (pausing it without releasing voices),
 *        and set the driver "paused" mode bit.
 */
void akao_seq_silence_unused_voices_and_pause(void)
{
    s32 keep_mask;
    s32 bit;
    s32 voice;
    s32 active_mask;
    s32 mode;

    if (g_akao_seq_channel0->w04.song.active_mask != 0)
    {
        keep_mask = ~(g_akao_sfx_control.unk0 | D_8004F76C[0]) & 0xFFFFFF;
        if (keep_mask != 0)
        {
            bit = 1;
            voice = 0;
            do
            {
                if (keep_mask & bit)
                {
                    spu_set_voice_volume(voice, 0, 0, 0);
                    spu_set_voice_pitch(voice, 0);
                    spu_set_voice_attack(voice, 0x7F, 1);
                    spu_set_voice_sustain_mode(voice, 0x7F, 3);
                    keep_mask &= ~bit;
                }
                bit <<= 1;
                voice++;
            } while (keep_mask != 0);
        }
        active_mask = g_akao_seq_channel0->w04.song.active_mask;
        g_akao_seq_channel0->w04.song.active_mask = 0;
        g_akao_seq_channel0->unk1C = active_mask;
    }
    mode = g_akao_driver_mode_flags;
    mode |= 1;
    g_akao_driver_mode_flags = mode;
}

/**
 * @brief Resume the primary song from the paused state set by
 *        akao_seq_silence_unused_voices_and_pause: restore active_mask
 *        from unk1C, flag every channel that was parked for a full SPU
 *        re-apply, and clear the driver "paused" mode bit.
 */
void akao_seq_resume_and_apply_pending_voices(void)
{
    s32 raw_mask;
    s32 mask;
    s32 bit;
    AkaoChannelState* channel;
    s32 saved_mask;
    s32 mode;

    raw_mask = g_akao_seq_channel0->unk1C;
    if (raw_mask != 0)
    {
        mask = raw_mask;
        bit = 1;
        channel = (AkaoChannelState*)g_akao_seq_channels;
        do
        {
            if (mask & bit)
            {
                mask &= ~bit;
                channel->update_flags |= 0x2B13;
            }
            bit <<= 1;
            channel++;
        } while (mask != 0);

        saved_mask = g_akao_seq_channel0->unk1C;
        g_akao_seq_channel0->unk1C = 0;
        g_akao_seq_channel0->w04.song.active_mask = saved_mask;
        g_akao_driver_flags.unk8 |= 0x100;
    }
    mode = g_akao_driver_mode_flags;
    mode &= ~1;
    g_akao_driver_mode_flags = mode;
}

/**
 * @brief SFX counterpart of akao_seq_silence_unused_voices_and_pause:
 *        filters the active SFX channel mask down to non-suppressed
 *        channels, parks it in g_akao_sfx_control.unk10, clears those bits
 *        from unk0, silences their SPU voices, and sets the driver
 *        "SFX paused" mode bit (0x2).
 */
void akao_sfx_silence_unused_voices_and_pause(void)
{
    AkaoChannelState* channel;
    s32 raw_active;
    s32 voice;
    s32 active;
    s32 bit;
    u32 count;
    s32 mode;

    raw_active = g_akao_sfx_control.unk0;
    if (raw_active != 0)
    {
        active = raw_active;
        channel = (AkaoChannelState*)g_sfx_channels;
        bit = AKAO_SFX_FIRST_CHANNEL_BIT;
        for (count = 0; count < AKAO_SFX_CHANNEL_COUNT; count++, channel++, bit <<= 1)
        {
            if ((active & bit) && (channel->tempo_acc & AKAO_SFX_FLAG_SUPPRESS))
            {
                active &= ~bit;
            }
        }
        bit = AKAO_SFX_FIRST_CHANNEL_BIT;
        voice = 0xC;
        g_akao_sfx_control.unk10 = active;
        g_akao_sfx_control.unk0 &= ~active;
        if (active != 0)
        {
            do
            {
                if (active & bit)
                {
                    spu_set_voice_volume(voice, 0, 0, 0);
                    spu_set_voice_pitch(voice, 0);
                    spu_set_voice_attack(voice, 0x7F, 1);
                    spu_set_voice_sustain_mode(voice, 0x7F, 3);
                    active &= ~bit;
                }
                bit <<= 1;
                voice++;
            } while (active != 0);
        }
    }
    mode = g_akao_driver_mode_flags;
    mode |= 2;
    g_akao_driver_mode_flags = mode;
}

/**
 * @brief Resume SFX channels from the paused state set by
 *        akao_sfx_silence_unused_voices_and_pause: flag every parked
 *        channel for a full SPU re-apply, restore the active mask from
 *        the parked D_8004D410 value, and clear the driver "SFX paused"
 *        mode bit.
 */
void akao_sfx_resume_and_apply_pending_voices(void)
{
    AkaoChannelState* channel;
    s32 raw_mask;
    s32 mask;
    s32 bit;
    s32 parked;
    s32 mode;

    raw_mask = D_8004D410;
    if (raw_mask != 0)
    {
        mask = raw_mask;
        channel = (AkaoChannelState*)g_sfx_channels;
        bit = AKAO_SFX_FIRST_CHANNEL_BIT;
        do
        {
            if (mask & bit)
            {
                mask &= ~bit;
                channel->update_flags |= 0x2B13;
            }
            bit <<= 1;
            channel++;
        } while (mask != 0);

        parked = g_akao_sfx_control.unk10;
        g_akao_sfx_control.unk10 = 0;
        g_akao_sfx_control.unk0 |= parked;
        g_akao_driver_flags.unk8 |= 0x100;
    }
    mode = g_akao_driver_mode_flags;
    mode &= ~2;
    g_akao_driver_mode_flags = mode;
}

/**
 * @brief Zero the pitch of the streamed XA voice pair while a stream is active.
 * @note 100% match (lom-dev-mcp diff tool; no decomp.me scratch created).
 */
void akao_xa_silence_voice_pitch(void)
{
    if (g_akao_xa_tracker.unkC != 0)
    {
        spu_set_voice_pitch(g_akao_xa_tracker.unk10, 0);
        spu_set_voice_pitch(g_akao_xa_tracker.unk10 + 1, 0);
    }
}

/**
 * @brief Restore the streamed XA voice pair's pitch to the tracker's cached
 *        value while a stream is active.
 * @note 100% match (lom-dev-mcp diff tool; no decomp.me scratch created).
 */
void akao_xa_restore_voice_pitch(void)
{
    if (g_akao_xa_tracker.unkC != 0)
    {
        spu_set_voice_pitch(g_akao_xa_tracker.unk10, g_akao_xa_tracker.unk58);
        spu_set_voice_pitch(g_akao_xa_tracker.unk10 + 1, g_akao_xa_tracker.unk58);
    }
}

/**
 * @brief Empty function; body is a bare return.
 * @note 100% match (lom-dev-mcp diff tool; no decomp.me scratch created).
 */
void func_80028E2C(void)
{
}

/**
 * @brief Toggle the SPU global reverb mode if @p reverb_type differs from the
 *        currently active one; brackets the change with reverb off/on so the
 *        SPU does not glitch mid-update.
 * @param reverb_type New reverb mode (an @c AkaoHeader::reverb_type value).
 * @note 100% match (lom-dev-mcp diff tool; no decomp.me scratch created).
 */
void akao_apply_reverb_type(s32 reverb_type)
{
    long current_mode;

    SpuGetReverbModeType(&current_mode);
    if (current_mode != reverb_type)
    {
        SpuSetReverb(0);
        SpuSetReverbModeType(reverb_type | 0x100);
        SpuSetReverb(1);
    }
}

/**
 * @brief Central low-level dispatcher for the AKAO sound driver.
 *
 * Masks @p opcode to a byte and routes it to a fixed handler-pointer table
 * (@c D_8003DDE0), indexed either directly by the opcode or by a small set of
 * remapped indices for opcodes that fan out into several handlers at once
 * (0x98/0x99) or that go through a dedicated indirect callback instead of the
 * table (0xD8/0xD9/0xDA). Opcodes 0x10/0x12/0x14/0x19 (song load/change)
 * additionally validate the AKAO magic on @c g_akao_cmd_params[0] and skip the
 * update entirely when the requested song is already active on both channels.
 * The driver's rcnt2 tick event is disabled for the duration of the dispatch
 * so a tick cannot observe a half-updated command-parameter buffer.
 *
 * @param opcode Command opcode; only the low byte is significant.
 * @return For opcodes 0x10/0x12/0x14/0x19, the newly loaded sequence id, 0 if
 *         the requested song was already active on both channels, or -1 if
 *         the header failed the AKAO magic check. Ignored by most other
 *         callers (see the doc comment on the forward declaration in
 *         akao_cmd.c).
 */
s32 akao_send_command(u32 opcode)
{
    s32 result;
    s32 requested_mask;
    s32 start_mask;
    AkaoCommandParam* params;
    AkaoHeader* header;
    u16 current_id;

    result = 0;
    DisableEvent(g_akao_rcnt2_event);
    opcode = opcode & 0xFF;
    params = D_8004D340;

    switch (opcode)
    {
    case 0x10:
    case 0x12:
    case 0x14:
    case 0x19:
        if (akao_check_magic(g_akao_cmd_params[0].buffer) == 0)
        {
            header = g_akao_cmd_params[0].buffer;
            current_id = g_akao_seq_channel0->unk5E;
            if ((current_id != header->id) || ((g_akao_seq_channel1 != 0) && (g_akao_seq_channel1->unk5E != current_id)))
            {
                akao_apply_reverb_type(header->reverb_type);
                params[0].buffer = header;
                params[2].value = header->id;
                if (opcode == 0x12)
                {
                    params[4].value = g_akao_cmd_params[1].value;
                }
                else
                {
                    requested_mask = g_akao_cmd_params[1].value;
                    start_mask = -1;
                    if (requested_mask != 0)
                    {
                        start_mask = requested_mask | 1;
                    }
                    params[3].value = start_mask;
                    params[4].value = g_akao_cmd_params[2].value;
                }
                result = header->id;
            }
            else
            {
                opcode = 0;
                result = 0;
            }
        }
        else
        {
            opcode = 0;
            result = -1;
        }
        break;

    case 0xD8:
        params[0].value = g_akao_cmd_params[0].value;
        D_8003E120[0](params);
        opcode = 0xD4;
        break;

    case 0xD9:
        params[0].value = g_akao_cmd_params[0].value;
        params[1].value = g_akao_cmd_params[1].value;
        D_8003E124[0](params);
        opcode = 0xD5;
        break;

    case 0xDA:
        params[0].value = g_akao_cmd_params[0].value;
        params[1].value = g_akao_cmd_params[1].value;
        params[2].value = g_akao_cmd_params[2].value;
        D_8003E128[0](params);
        opcode = 0xD6;
        break;

    case 0x99:
        D_8003DDE0[0x9B](params);
        D_8003DDE0[0x9D](params);
        opcode = 0x9F;
        break;

    case 0x98:
        D_8003DDE0[0x9A](params);
        D_8003DDE0[0x9C](params);
        opcode = 0x9E;
        break;

    default:
        params[0].value = g_akao_cmd_params[0].value;
        params[1].value = g_akao_cmd_params[1].value;
        params[2].value = g_akao_cmd_params[2].value;
        params[3].value = g_akao_cmd_params[3].value;
        params[4].value = g_akao_cmd_params[4].value;
        params[5].value = g_akao_cmd_params[5].value;
        break;
    }

    D_8003DDE0[opcode](params);
    EnableEvent(g_akao_rcnt2_event);
    return result;
}
