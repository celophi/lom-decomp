#include "main/audio/akao_cmd.h"
#include "internal/akao_sequencer.h"
#include <libcd.h>
#include <libspu.h>

/**
 * @brief Pending articulation and sample bytes for a streaming bank upload.
 *
 * Primed on the first tick of a streaming upload from the AkaoBankHeader at
 * the head of the source buffer; each subsequent call to
 * akao_streaming_upload_tick consumes some bytes from the source and shrinks
 * the two remaining-byte counters.
 */
typedef struct
{
    u8* articulation_dst;       /**< Current destination in the articulation table. */
    u32 spu_addr;               /**< Next SPU write address; zero marks the first tick. */
    u32 sample_remaining;       /**< Sample bytes still to upload. */
    u32 articulation_remaining; /**< Articulation bytes still to copy. */
} AkaoStreamingState;

/** @brief Bank identity prefix; the key combines the header's id and length. */
typedef struct
{
    u32 magic;
    s32 key;
} AkaoBankIdentity;

extern AkaoXaProgramStaging g_akao_xa_program_staging;
extern CdlATV g_akao_cdmix;
extern AkaoStreamingState g_akao_streaming_state;
extern AkaoBankHeader g_akao_bank_staging;

/** @brief Number of SPU bank slots (entries of g_akao_bank_slot_keys). */
#define AKAO_BANK_SLOT_COUNT 6

/** @brief Base SPU address and byte spacing of the six instrument-bank slots. */
#define AKAO_BANK_FIRST_SPU_ADDRESS 0x43100
#define AKAO_BANK_SLOT_BYTES 0x4800
#define AKAO_BANK_SLOT_SPU_ADDRESS(slot) (AKAO_BANK_FIRST_SPU_ADDRESS + (slot) * AKAO_BANK_SLOT_BYTES)

/** @brief First articulation index and number of articulations reserved per bank slot. */
#define AKAO_BANK_FIRST_ARTICULATION 0x80
#define AKAO_BANK_SLOT_ARTICULATIONS 0x10
#define AKAO_BANK_SLOT_ARTICULATION_INDEX(slot) (AKAO_BANK_FIRST_ARTICULATION + (slot) * AKAO_BANK_SLOT_ARTICULATIONS)

/** @brief First of the three upper instrument-bank slots; also the upper XA slot. */
#define AKAO_BANK_FIRST_UPPER_SLOT 3

/** @brief akao_submit_bank result indicating that the upload must be retried. */
#define AKAO_BANK_UPLOAD_BUSY 1

/** @brief Word size used by akao_copy_bytes when rounding down its byte count. */
#define AKAO_COPY_WORD_SHIFT 2
#define AKAO_COPY_WORD_BYTES 4

/** @brief Gain of each CD-to-SPU route in mono mode, in Q17 (about 0.35437). */
#define AKAO_CD_MONO_GAIN_Q17 0xB570
/** @brief Fractional bits in the mono CD mix gain. */
#define AKAO_CD_MONO_GAIN_SHIFT 17

/** @brief Song flag that lowers the XA SPU area while song channels are active or parked. */
#define AKAO_SONG_LOWER_XA_AREA_FLAG 0x40
/** @brief Distance from the usual XA SPU area to the lower area. */
#define AKAO_XA_SPU_AREA_OFFSET 0x30000

/** @brief Fractional bits in XA volume and pan command parameters. */
#define AKAO_XA_FIXED_POINT_SHIFT 8

/** @brief XA ring block size and the next-fill index that permits playback to start. */
#define AKAO_XA_RING_BLOCK_SHIFT 12
#define AKAO_XA_RING_BLOCK_BYTES (1 << AKAO_XA_RING_BLOCK_SHIFT)
#define AKAO_XA_START_FILL_BLOCK 2

/** @brief Upload-block marker used before the first XA ring block is uploaded. */
#define AKAO_XA_UPLOAD_BLOCK_NONE (-1)

/* g_akao_seq_channel0, read through its fixed address. */
#define AKAO_PRIMARY_SONG (*(AkaoSongState**)AKAO_PRIMARY_SONG_ADDRESS)

/**
 * @brief Dispatch an AKAO sound command using g_akao_cmd_params.
 *
 * High-level wrappers store their inputs in g_akao_cmd_params before calling
 * this function. AkaoCmd documents the supported command opcodes.
 *
 * @param opcode Command opcode; only the low byte is significant.
 * @return For opcodes 0x10/0x12/0x14/0x19 (load/change song), the newly
 *         loaded sequence id, or 0 if the requested song was already active,
 *         or -1 if the header failed the AKAO magic check. Unused/ignored by
 *         most callers otherwise.
 */
s32 akao_send_command(u32 opcode);
void akao_xa_start_ring_stream(void);

/**
 * @brief Public init entry - wraps akao_driver_init and returns 0.
 * @return 0 after initialization.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/hDNyF
 */
s32 akao_init(void)
{
    akao_driver_init();
    return 0;
}

/**
 * @brief Public shutdown entry - wraps akao_driver_shutdown and returns 0.
 * @return 0 after shutdown.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/z7ZEh
 */
s32 akao_shutdown(void)
{
    akao_driver_shutdown();
    return 0;
}

/**
 * @brief Registers an AKAO instrument/sample bank with the audio driver.
 *
 * Validates the 'AKAO' magic at the start of @p bank via akao_check_magic;
 * on success, hands the payload (after the 16-byte AKAO header) to the driver
 * entry point akao_set_bank_data_ptrs, which records the bank as the active sample source.
 *
 * @param bank Address of an AKAO-tagged instrument bank in main RAM. The first
 *             four bytes must be "AKAO". In TITLE this points to 0x8013C000
 *             after EFFECT.SET is split.
 *
 * @return 0 if the magic matched and the bank was registered; otherwise the
 *         non-zero delta (bank->magic - AKAO_MAGIC) from akao_check_magic.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/0q180
 */
s32 akao_register_bank(AkaoHeader* bank)
{
    s32 result;

    result = akao_check_magic(bank);
    if (result == 0)
    {
        akao_set_bank_data_ptrs((u8*)(bank + 1));
    }
    return result;
}

/**
 * @brief AKAO command 0x10 - start playback of a sequence (song).
 *
 * Loads @p sequence_data into the AKAO command parameter buffer and dispatches the
 * "play song" command to the audio driver. The driver picks up the buffer
 * pointer from g_akao_cmd_params[0] when it processes the command.
 *
 * @param sequence_data Pointer to a loaded AKAO-tagged sequence buffer, such
 *                      as @c g_resident_song_buffer in TITLE.
 * @return Song handle returned by the AKAO command dispatcher.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/iVOOb
 */
s32 akao_play_song(AkaoHeader* sequence_data)
{
    g_akao_cmd_params[0].buffer = sequence_data;
    return akao_send_command(AKAO_CMD_PLAY_SONG);
}

/**
 * @brief Queue command 0x11: stop songs selected by their key.
 *
 * A zero key stops the primary song unconditionally. A nonzero key stops
 * matching primary and secondary songs.
 *
 * @param song_key Song key to match, or 0 to stop the primary song.
 * @see akao_seq_stop_song_by_key
 *
 * @see decomp.me (100%) https://decomp.me/scratch/9M4hF
 */
void akao_stop_song(s32 song_key)
{
    g_akao_cmd_params[0].value = song_key;
    akao_send_command(AKAO_CMD_STOP_SONG);
}

/**
 * @brief Queue command 0x40: back up the playing song so it can be resumed later.
 * @see akao_seq_suspend_song
 * @see decomp.me (100%) https://decomp.me/scratch/4GVez
 */
void akao_suspend_song(void)
{
    akao_send_command(AKAO_CMD_SUSPEND_SONG);
}

/**
 * @brief Queue command 0x14: start a sequence on a subset of its channels.
 * @param sequence AKAO sequence to start.
 * @param channel_mask Channels to start; 0 starts every channel.
 * @param unused Passed by the only caller (FIELD) and ignored.
 * @return The command's result; FIELD treats -1 as failure.
 * @see decomp.me (100%) https://decomp.me/scratch/c2C3m
 */
s32 akao_start_song_channels(void* sequence, s32 channel_mask, s32 unused)
{
    g_akao_cmd_params[0].buffer = sequence;
    g_akao_cmd_params[1].value = channel_mask;
    g_akao_cmd_params[2].value = 0;
    return akao_send_command(AKAO_CMD_START_SONG_CHANNELS);
}

/**
 * @brief Queue command 0x19 to switch to a new song, then set its volume (0xC0).
 *
 * The song that was playing keeps running as the secondary song.
 *
 * @param sequence AKAO sequence to switch to.
 * @param volume Master volume of the new song; only the low 7 bits are used.
 * @return Id of the new sequence, 0 if it was already playing, or -1 for a bad header.
 * @see decomp.me (100%) https://decomp.me/scratch/d6xXt
 */
s32 akao_switch_song(void* sequence, s32 volume)
{
    s32 result;

    g_akao_cmd_params[0].buffer = sequence;
    result = akao_send_command(AKAO_CMD_SWITCH_SONG);
    g_akao_cmd_params[0].value = (volume & AKAO_VOLUME_MAX);
    g_akao_cmd_params[3].value = 0;
    akao_send_command(AKAO_CMD_SET_SONG_VOLUME);
    return result;
}

/**
 * @brief Queue command 0x12: play a sequence and seed its pending tick count.
 * @param sequence AKAO sequence to play.
 * @param ticks Initial pending tick count; 0 leaves no ticks pending.
 * @see decomp.me (100%) https://decomp.me/scratch/jigab
 */
void akao_play_song_with_ticks(s32 sequence, s32 ticks)
{
    g_akao_cmd_params[0].value = sequence;
    g_akao_cmd_params[1].value = ticks;
    akao_send_command(AKAO_CMD_PLAY_SONG_WITH_TICKS);
}

/**
 * @brief Queue command 0x20: play a sound effect from the loaded banks.
 * @param sound_id Sound id; only the low 10 bits are used.
 * @param tag Caller tag stored with the channels, used to select them later; low 24 bits.
 * @param pan Pan; only the low 8 bits are used.
 * @param volume Volume; only the low 7 bits are used.
 * @see decomp.me (100%) https://decomp.me/scratch/9AZZL
 */
void akao_play_sfx(s32 sound_id, s32 tag, s32 pan, s32 volume)
{
    g_akao_cmd_params[0].value = (sound_id & AKAO_SFX_ID_MASK);
    g_akao_cmd_params[1].value = (tag & AKAO_SFX_TAG_MASK);
    g_akao_cmd_params[2].value = (pan & AKAO_PAN_MASK);
    g_akao_cmd_params[3].value = (volume & AKAO_VOLUME_MAX);
    akao_send_command(AKAO_CMD_PLAY_SFX);
}

/**
 * @brief Queue command 0x24: play the sound effect list in an AKAO buffer.
 * @param buffer AKAO-tagged sound buffer.
 * @param tag Caller tag stored with the channels; only the low 24 bits are used.
 * @param pan Pan; only the low 8 bits are used.
 * @param volume Volume; only the low 7 bits are used.
 * @return The address of @p buffer, or the akao_check_magic result for a bad header.
 * @see decomp.me (100%) https://decomp.me/scratch/FFGei
 */
uintptr_t akao_play_sfx_from_buffer(AkaoHeader* buffer, s32 tag, s32 pan, s32 volume)
{
    s32 result = akao_check_magic(buffer);

    if (result != 0)
    {
        return result;
    }

    g_akao_cmd_params[0].buffer = buffer;
    g_akao_cmd_params[1].value = tag & AKAO_SFX_TAG_MASK;
    g_akao_cmd_params[2].value = pan & AKAO_PAN_MASK;
    g_akao_cmd_params[3].value = volume & AKAO_VOLUME_MAX;
    akao_send_command(AKAO_CMD_PLAY_SFX_LIST);

    return (uintptr_t)buffer;
}

/**
 * @brief Queue command 0x21: stop sound effects.
 * @param sound_id Sound id to stop when @p tag_mask is 0.
 * @param tag_mask Stop every channel whose tag shares a bit with this mask; low 24 bits.
 * @see akao_sfx_stop_channels
 * @see decomp.me (100%) https://decomp.me/scratch/lu9nS
 */
void akao_stop_sfx(s32 sound_id, s32 tag_mask)
{
    g_akao_cmd_params[0].value = sound_id;
    g_akao_cmd_params[1].value = (tag_mask & AKAO_SFX_TAG_MASK);
    akao_send_command(AKAO_CMD_STOP_SFX);
}

/**
 * @brief Queue command 0x30: play a sound effect with the default pan and volume.
 * @param sound_id Sound id; only the low 10 bits are used.
 * @see decomp.me (100%) https://decomp.me/scratch/0mLzI
 */
void akao_play_sound(s32 sound_id)
{
    g_akao_cmd_params[0].value = sound_id & AKAO_SFX_ID_MASK;
    akao_send_command(AKAO_CMD_PLAY_SOUND);
}

/**
 * @brief OR together the tags of active SFX channels.
 * @return Combined active channel tags, masked to their low 24 bits.
 * @see decomp.me (100%) https://decomp.me/scratch/yZloM
 */
s32 akao_get_active_sfx_tags(void)
{
    s32 active_mask;
    const AkaoChannelState* channel;
    s32 active_tags;
    u32 channel_bit;

    active_mask = g_akao_sfx_control.active_mask;
    if (active_mask == 0)
    {
        return 0;
    }
    channel = g_sfx_channels;
    active_tags = 0;
    channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    do
    {
        if (active_mask & channel_bit)
        {
            active_tags |= channel->sfx_tag;
        }
        channel_bit <<= 1;
        channel++;
    } while (channel_bit & AKAO_VOICE_MASK);
    active_tags &= AKAO_SFX_TAG_MASK;
    return active_tags;
}

/**
 * @brief Test whether an active SFX channel carries the requested tag.
 *
 * @param sfx_tag Exact channel tag to find; 0 never matches.
 * @return 1 if a matching active channel exists, otherwise 0.
 * @note Compares the complete stored tag, including flag bits.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/OvqYq
 */
s32 akao_is_sfx_playing(s32 sfx_tag)
{
    s32 active_mask;
    const AkaoChannelState* channel;
    u32 channel_bit;

    if (sfx_tag == 0)
    {
        return 0;
    }
    active_mask = g_akao_sfx_control.active_mask;
    if (active_mask == 0)
    {
        return 0;
    }
    channel = g_sfx_channels;
    channel_bit = AKAO_SFX_FIRST_CHANNEL_BIT;
    do
    {
        if ((active_mask & channel_bit) && sfx_tag == channel->sfx_tag)
        {
            return 1;
        }
        channel_bit <<= 1;
        channel++;
    } while (channel_bit & AKAO_VOICE_MASK);
    return 0;
}

/**
 * @brief Queue command 0x81 (mono) or 0x80 (stereo) to select the sound output mode.
 * @param mono 1 selects mono output; any other value selects stereo.
 * @see decomp.me (100%) https://decomp.me/scratch/9qTjH
 */
void akao_set_mono_output(s32 mono)
{
    if (mono == 1)
    {
        akao_send_command(AKAO_CMD_SELECT_MONO);
    }
    else
    {
        akao_send_command(AKAO_CMD_SELECT_STEREO);
    }
}

/**
 * @brief Queue command 0x90: silence song channels without stopping them.
 * @param channel_mask Channels of the primary song to silence; 0 unmutes all.
 * @see decomp.me (100%) https://decomp.me/scratch/x94md
 */
void akao_mute_song_channels(s32 channel_mask)
{
    g_akao_cmd_params[0].value = channel_mask;
    akao_send_command(AKAO_CMD_MUTE_SONG_CHANNELS);
}

/**
 * @brief Queue command 0x92: set the value tested by the conditional-jump opcode.
 * @param value New condition value for the primary song.
 * @see decomp.me (100%) https://decomp.me/scratch/y9TAf
 */
void akao_set_song_condition(s32 value)
{
    g_akao_cmd_params[0].value = value;
    akao_send_command(AKAO_CMD_SET_SONG_CONDITION);
}

/**
 * @brief Pause audio playback (commands 0x99, 0x9B, 0x9D and 0x9F).
 * @param target AKAO_AUDIO_SONG, AKAO_AUDIO_SFX or AKAO_AUDIO_XA; any other value selects all three.
 * @see decomp.me (100%) https://decomp.me/scratch/qqSuG
 */
void akao_pause_audio(u32 target)
{
    s32 opcode;

    switch (target)
    {
    case AKAO_AUDIO_SONG:
        opcode = AKAO_CMD_PAUSE_SONG;
        break;
    case AKAO_AUDIO_SFX:
        opcode = AKAO_CMD_PAUSE_SFX;
        break;
    case AKAO_AUDIO_XA:
        opcode = AKAO_CMD_PAUSE_XA;
        break;
    default:
        opcode = AKAO_CMD_PAUSE_ALL;
        break;
    }

    akao_send_command(opcode);
}

/**
 * @brief Resume audio playback (commands 0x98, 0x9A, 0x9C and 0x9E).
 * @param target AKAO_AUDIO_SONG, AKAO_AUDIO_SFX or AKAO_AUDIO_XA; any other value selects all three.
 * @see decomp.me (100%) https://decomp.me/scratch/iREFc
 */
void akao_resume_audio(u32 target)
{
    s32 opcode;

    switch (target)
    {
    case AKAO_AUDIO_SONG:
        opcode = AKAO_CMD_RESUME_SONG;
        break;
    case AKAO_AUDIO_SFX:
        opcode = AKAO_CMD_RESUME_SFX;
        break;
    case AKAO_AUDIO_XA:
        opcode = AKAO_CMD_RESUME_XA;
        break;
    default:
        opcode = AKAO_CMD_RESUME_ALL;
        break;
    }

    akao_send_command(opcode);
}

/**
 * @brief Queue command 0xA8: set the volume scale of every SFX channel not marked AKAO_SFX_FLAG_SUPPRESS.
 * @param volume Volume scale; only the low 7 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/VTGCB
 */
s32 akao_set_all_sfx_volume(s32 volume)
{
    g_akao_cmd_params[0].value = volume & AKAO_VOLUME_MAX;
    return akao_send_command(AKAO_CMD_SET_ALL_SFX_VOLUME);
}

/**
 * @brief Queue command 0xA9: fade the volume scale of every SFX channel not marked AKAO_SFX_FLAG_SUPPRESS.
 * @param ticks Fade length in driver ticks; 0 is treated as 1.
 * @param volume Target volume scale; only the low 7 bits are used.
 * @see decomp.me (100%) https://decomp.me/scratch/03hNO
 */
void akao_fade_all_sfx_volume(s32 ticks, s32 volume)
{
    g_akao_cmd_params[0].value = ticks;
    g_akao_cmd_params[1].value = (volume & AKAO_VOLUME_MAX);
    akao_send_command(AKAO_CMD_FADE_ALL_SFX_VOLUME);
}

/**
 * @brief Queue command 0xA0: set the volume scale of selected SFX channels.
 * @param sound_id Sound id to match when @p tag_mask is 0.
 * @param tag_mask Select channels whose tag shares a bit with this mask; low 24 bits, 0 matches by id.
 * @param volume Volume scale; only the low 7 bits are used.
 * @see decomp.me (100%) https://decomp.me/scratch/C8UTP
 */
void akao_set_sfx_volume(s32 sound_id, s32 tag_mask, s32 volume)
{
    g_akao_cmd_params[0].value = sound_id;
    g_akao_cmd_params[1].value = (tag_mask & AKAO_SFX_TAG_MASK);
    g_akao_cmd_params[2].value = (volume & AKAO_VOLUME_MAX);
    akao_send_command(AKAO_CMD_SET_SFX_VOLUME);
}

/**
 * @brief Queue command 0xA1: fade the volume scale of selected SFX channels.
 * @param sound_id Sound id to match when @p tag_mask is 0.
 * @param tag_mask Select channels whose tag shares a bit with this mask; low 24 bits, 0 matches by id.
 * @param ticks Fade length in driver ticks; 0 is treated as 1.
 * @param volume Target volume scale; only the low 7 bits are used.
 * @see decomp.me (100%) https://decomp.me/scratch/xMNn0
 */
void akao_fade_sfx_volume(s32 sound_id, s32 tag_mask, s32 ticks, s32 volume)
{
    g_akao_cmd_params[0].value = sound_id;
    g_akao_cmd_params[1].value = (tag_mask & AKAO_SFX_TAG_MASK);
    g_akao_cmd_params[2].value = ticks;
    g_akao_cmd_params[3].value = (volume & AKAO_VOLUME_MAX);
    akao_send_command(AKAO_CMD_FADE_SFX_VOLUME);
}

/**
 * @brief Queue command 0xAA: set the pan bias of every SFX channel not marked AKAO_SFX_FLAG_SUPPRESS.
 * @param pan Pan bias; only the low 8 bits are used.
 * @see decomp.me (100%) https://decomp.me/scratch/AuyLX
 */
void akao_set_all_sfx_pan(s32 pan)
{
    g_akao_cmd_params[0].value = pan & AKAO_PAN_MASK;
    akao_send_command(AKAO_CMD_SET_ALL_SFX_PAN);
}

/**
 * @brief Queue command 0xAB: fade the pan bias of every SFX channel not marked AKAO_SFX_FLAG_SUPPRESS.
 * @param ticks Fade length in driver ticks; 0 is treated as 1.
 * @param pan Target pan bias; only the low 8 bits are used.
 * @see decomp.me (100%) https://decomp.me/scratch/IaBX9
 */
void akao_fade_all_sfx_pan(s32 ticks, s32 pan)
{
    g_akao_cmd_params[0].value = ticks;
    g_akao_cmd_params[1].value = (pan & AKAO_PAN_MASK);
    akao_send_command(AKAO_CMD_FADE_ALL_SFX_PAN);
}

/**
 * @brief Queue command 0xA2: set the pan bias of selected SFX channels.
 * @param sound_id Sound id to match when @p tag_mask is 0.
 * @param tag_mask Select channels whose tag shares a bit with this mask; low 24 bits, 0 matches by id.
 * @param pan Pan bias; only the low 8 bits are used.
 * @see decomp.me (100%) https://decomp.me/scratch/LhoLV
 */
void akao_set_sfx_pan(s32 sound_id, s32 tag_mask, s32 pan)
{
    g_akao_cmd_params[0].value = sound_id;
    g_akao_cmd_params[1].value = (tag_mask & AKAO_SFX_TAG_MASK);
    g_akao_cmd_params[2].value = (pan & AKAO_PAN_MASK);
    akao_send_command(AKAO_CMD_SET_SFX_PAN);
}

/**
 * @brief Queue command 0xA3: fade the pan bias of selected SFX channels.
 * @param sound_id Sound id to match when @p tag_mask is 0.
 * @param tag_mask Select channels whose tag shares a bit with this mask; low 24 bits, 0 matches by id.
 * @param ticks Fade length in driver ticks; 0 is treated as 1.
 * @param pan Target pan bias; only the low 8 bits are used.
 * @see decomp.me (100%) https://decomp.me/scratch/Al5YT
 */
void akao_fade_sfx_pan(s32 sound_id, s32 tag_mask, s32 ticks, s32 pan)
{
    g_akao_cmd_params[0].value = sound_id;
    g_akao_cmd_params[1].value = (tag_mask & AKAO_SFX_TAG_MASK);
    g_akao_cmd_params[2].value = ticks;
    g_akao_cmd_params[3].value = (pan & AKAO_PAN_MASK);
    akao_send_command(AKAO_CMD_FADE_SFX_PAN);
}

/**
 * @brief Queue command 0xAC: set the pitch bend of every SFX channel not marked AKAO_SFX_FLAG_SUPPRESS.
 * @param bend Pitch bend; only the low 8 bits are used.
 * @see decomp.me (100%) https://decomp.me/scratch/e4D90
 */
void akao_set_all_sfx_pitch_bend(s32 bend)
{
    g_akao_cmd_params[0].value = bend & AKAO_PITCH_BEND_MASK;
    akao_send_command(AKAO_CMD_SET_ALL_SFX_PITCH_BEND);
}

/**
 * @brief Queue command 0xAD: fade the pitch bend of every SFX channel not marked AKAO_SFX_FLAG_SUPPRESS.
 * @param ticks Fade length in driver ticks; 0 is treated as 1.
 * @param bend Target pitch bend; only the low 8 bits are used.
 * @see decomp.me (100%) https://decomp.me/scratch/Fw2d9
 */
void akao_fade_all_sfx_pitch_bend(s32 ticks, s32 bend)
{
    g_akao_cmd_params[0].value = ticks;
    g_akao_cmd_params[1].value = (bend & AKAO_PITCH_BEND_MASK);
    akao_send_command(AKAO_CMD_FADE_ALL_SFX_PITCH_BEND);
}

/**
 * @brief Queue command 0xA4: set the pitch bend of selected SFX channels.
 * @param sound_id Sound id to match when @p tag_mask is 0.
 * @param tag_mask Select channels whose tag shares a bit with this mask; low 24 bits, 0 matches by id.
 * @param bend Pitch bend; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/vHMVZ
 */
s32 akao_set_sfx_pitch_bend(s32 sound_id, s32 tag_mask, s32 bend)
{
    g_akao_cmd_params[0].value = sound_id;
    g_akao_cmd_params[1].value = (tag_mask & AKAO_SFX_TAG_MASK);
    g_akao_cmd_params[2].value = (bend & AKAO_PITCH_BEND_MASK);
    return akao_send_command(AKAO_CMD_SET_SFX_PITCH_BEND);
}

/**
 * @brief Queue command 0xA5: fade the pitch bend of selected SFX channels.
 * @param sound_id Sound id to match when @p tag_mask is 0.
 * @param tag_mask Select channels whose tag shares a bit with this mask; low 24 bits, 0 matches by id.
 * @param ticks Fade length in driver ticks; 0 is treated as 1.
 * @param bend Target pitch bend; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/exTVG
 */
s32 akao_fade_sfx_pitch_bend(s32 sound_id, s32 tag_mask, s32 ticks, s32 bend)
{
    g_akao_cmd_params[0].value = sound_id;
    g_akao_cmd_params[1].value = (tag_mask & AKAO_SFX_TAG_MASK);
    g_akao_cmd_params[2].value = ticks;
    g_akao_cmd_params[3].value = (bend & AKAO_PITCH_BEND_MASK);
    return akao_send_command(AKAO_CMD_FADE_SFX_PITCH_BEND);
}

/**
 * @brief Set a song's master volume through AKAO command 0xC0.
 *
 * @param song_handle Song handle returned by akao_play_song, or 0 for the
 *        primary song.
 * @param volume Master volume; only the low 7 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/QPqUd
 */
s32 akao_set_song_volume(s32 song_handle, s32 volume)
{
    g_akao_cmd_params[0].value = song_handle;
    g_akao_cmd_params[1].value = volume & AKAO_VOLUME_MAX;
    return akao_send_command(AKAO_CMD_SET_SONG_VOLUME);
}

/**
 * @brief Queue command 0xC1: fade a song's master volume.
 * @param song_handle Song handle returned by akao_play_song, or 0 for the primary song.
 * @param ticks Fade length in driver ticks; 0 is treated as 1.
 * @param volume Target volume; only the low 7 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/cSIwP
 */
s32 akao_fade_song_volume(s32 song_handle, s32 ticks, s32 volume)
{
    g_akao_cmd_params[0].value = song_handle;
    g_akao_cmd_params[1].value = ticks;
    g_akao_cmd_params[2].value = (volume & AKAO_VOLUME_MAX);
    return akao_send_command(AKAO_CMD_FADE_SONG_VOLUME);
}

/**
 * @brief Queue command 0xC2: fade a song's master volume from an explicit start level.
 * @param song_handle Song handle returned by akao_play_song, or 0 for the primary song.
 * @param ticks Fade length in driver ticks; 0 is treated as 1.
 * @param start_volume Start volume; only the low 7 bits are used.
 * @param volume Target volume; only the low 7 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/PbMJC
 */
s32 akao_fade_song_volume_from(s32 song_handle, s32 ticks, s32 start_volume, s32 volume)
{
    g_akao_cmd_params[0].value = song_handle;
    g_akao_cmd_params[1].value = ticks;
    g_akao_cmd_params[2].value = (start_volume & AKAO_VOLUME_MAX);
    g_akao_cmd_params[3].value = (volume & AKAO_VOLUME_MAX);
    return akao_send_command(AKAO_CMD_FADE_SONG_VOLUME_FROM);
}

/**
 * @brief Queue command 0xC8: set the CD audio volume.
 * @param volume CD audio volume (0x7FFF is full volume).
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/BeJR1
 */
s32 akao_set_cd_volume(s32 volume)
{
    g_akao_cmd_params[0].value = volume;
    return akao_send_command(AKAO_CMD_SET_CD_VOLUME);
}

/**
 * @brief Queue command 0xC9: fade the CD audio volume.
 * @param ticks Fade length in driver ticks; 0 is treated as 1.
 * @param volume Target CD audio volume.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/yo40G
 */
s32 akao_fade_cd_volume(s32 ticks, s32 volume)
{
    g_akao_cmd_params[0].value = ticks;
    g_akao_cmd_params[1].value = volume;
    return akao_send_command(AKAO_CMD_FADE_CD_VOLUME);
}

/**
 * @brief Queue command 0xCA: fade the CD audio volume from an explicit start level.
 * @param ticks Fade length in driver ticks; 0 is treated as 1.
 * @param start_volume Start CD audio volume.
 * @param volume Target CD audio volume.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/pLMBi
 */
s32 akao_fade_cd_volume_from(s32 ticks, s32 start_volume, s32 volume)
{
    g_akao_cmd_params[0].value = ticks;
    g_akao_cmd_params[1].value = start_volume;
    g_akao_cmd_params[2].value = volume;
    return akao_send_command(AKAO_CMD_FADE_CD_VOLUME_FROM);
}

/**
 * @brief Queue command 0xD0: set the driver master pan.
 * @param pan Signed master pan; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/klUxi
 */
s32 akao_set_master_pan(s32 pan)
{
    g_akao_cmd_params[0].value = pan & AKAO_PAN_MASK;
    return akao_send_command(AKAO_CMD_SET_MASTER_PAN);
}

/**
 * @brief Queue command 0xD1: fade the driver master pan.
 * @param ticks Fade length in driver ticks; 0 is treated as 1.
 * @param pan Signed target pan; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/XXHwt
 */
s32 akao_fade_master_pan(s32 ticks, s32 pan)
{
    g_akao_cmd_params[0].value = ticks;
    g_akao_cmd_params[1].value = (pan & AKAO_PAN_MASK);
    return akao_send_command(AKAO_CMD_FADE_MASTER_PAN);
}

/**
 * @brief Queue command 0xD2: fade the driver master pan from an explicit start level.
 * @param ticks Fade length in driver ticks; 0 is treated as 1.
 * @param start_pan Signed start pan; only the low 8 bits are used.
 * @param pan Signed target pan; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/074UT
 */
s32 akao_fade_master_pan_from(s32 ticks, s32 start_pan, s32 pan)
{
    g_akao_cmd_params[0].value = ticks;
    g_akao_cmd_params[1].value = (start_pan & AKAO_PAN_MASK);
    g_akao_cmd_params[2].value = (pan & AKAO_PAN_MASK);
    return akao_send_command(AKAO_CMD_FADE_MASTER_PAN_FROM);
}

/**
 * @brief Queue command 0xD4: set the driver master volume.
 * @param volume Master volume; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/yJdLv
 */
s32 akao_set_master_volume(s32 volume)
{
    g_akao_cmd_params[0].value = volume & AKAO_MASTER_VOLUME_MASK;
    return akao_send_command(AKAO_CMD_SET_MASTER_VOLUME);
}

/**
 * @brief Queue command 0xD5: fade the driver master volume.
 * @param ticks Fade length in driver ticks; 0 is treated as 1.
 * @param volume Target master volume; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/u6Eys
 */
s32 akao_fade_master_volume(s32 ticks, s32 volume)
{
    g_akao_cmd_params[0].value = ticks;
    g_akao_cmd_params[1].value = (volume & AKAO_MASTER_VOLUME_MASK);
    return akao_send_command(AKAO_CMD_FADE_MASTER_VOLUME);
}

/**
 * @brief Queue command 0xD6: fade the driver master volume from an explicit start level.
 * @param ticks Fade length in driver ticks; 0 is treated as 1.
 * @param start_volume Start master volume; only the low 8 bits are used.
 * @param volume Target master volume; only the low 8 bits are used.
 * @see decomp.me (100%) https://decomp.me/scratch/ITNFU
 */
void akao_fade_master_volume_from(s32 ticks, s32 start_volume, s32 volume)
{
    g_akao_cmd_params[0].value = ticks;
    g_akao_cmd_params[1].value = (start_volume & AKAO_MASTER_VOLUME_MASK);
    g_akao_cmd_params[2].value = (volume & AKAO_MASTER_VOLUME_MASK);
    akao_send_command(AKAO_CMD_FADE_MASTER_VOLUME_FROM);
}

/**
 * @brief Queue command 0xD8: set the driver master pan and master volume to one value (0xD0 then 0xD4).
 * @param value Pan and volume; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/JS2nD
 */
s32 akao_set_master_pan_and_volume(s32 value)
{
    g_akao_cmd_params[0].value = value & AKAO_MASTER_PAN_VOLUME_MASK;
    return akao_send_command(AKAO_CMD_SET_MASTER_PAN_AND_VOLUME);
}

/**
 * @brief Queue command 0xD9: fade the driver master pan and master volume (0xD1 then 0xD5).
 * @param ticks Fade length in driver ticks; 0 is treated as 1.
 * @param value Target pan and volume; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/YD6rZ
 */
s32 akao_fade_master_pan_and_volume(s32 ticks, s32 value)
{
    g_akao_cmd_params[0].value = ticks;
    g_akao_cmd_params[1].value = (value & AKAO_MASTER_PAN_VOLUME_MASK);
    return akao_send_command(AKAO_CMD_FADE_MASTER_PAN_AND_VOLUME);
}

/**
 * @brief Queue command 0xDA: fade the driver master pan and master volume from a start level (0xD2 then 0xD6).
 * @param ticks Fade length in driver ticks; 0 is treated as 1.
 * @param start_value Start pan and volume; only the low 8 bits are used.
 * @param value Target pan and volume; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/jzW0l
 */
s32 akao_fade_master_pan_and_volume_from(s32 ticks, s32 start_value, s32 value)
{
    g_akao_cmd_params[0].value = ticks;
    g_akao_cmd_params[1].value = (start_value & AKAO_MASTER_PAN_VOLUME_MASK);
    g_akao_cmd_params[2].value = (value & AKAO_MASTER_PAN_VOLUME_MASK);
    return akao_send_command(AKAO_CMD_FADE_MASTER_PAN_AND_VOLUME_FROM);
}

/**
 * @brief Queue command 0xF0: stop the primary and secondary songs and release their voices.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/dgbnE
 */
s32 akao_stop_all_songs(void)
{
    return akao_send_command(AKAO_CMD_STOP_ALL_SONGS);
}

/**
 * @brief Queue command 0xF1: release every SFX channel.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/IMYAL
 */
s32 akao_release_all_sfx(void)
{
    return akao_send_command(AKAO_CMD_RELEASE_ALL_SFX);
}

/**
 * @brief Upload an AKAO instrument bank and spin until it is accepted.
 *
 * Clears bit 0 of the driver status word, then repeatedly calls
 * akao_submit_bank until it stops returning the busy sentinel.
 *
 * @param bank Pointer to an AKAO instrument bank in main RAM.
 * @param wait_for_completion Non-zero to wait for the SPU DMA to complete.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/Mz7yX
 */
void akao_upload_bank_blocking(AkaoBankHeader* bank, s32 wait_for_completion)
{
    g_akao_driver_flags.upload_flags &= ~AKAO_UPLOAD_STREAMING;
    while (akao_submit_bank(bank, wait_for_completion) == AKAO_BANK_UPLOAD_BUSY)
    {
    }
}

/**
 * @brief Return the current SPU/AKAO transfer-status latch.
 *
 * @return Current value of g_akao_spu_xfer_pending.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/ecQHb
 */
s32 akao_get_xfer_state(void)
{
    return g_akao_spu_xfer_pending;
}

/**
 * @brief Restart the streaming bank upload and mark it pending.
 *
 * Clears the next SPU address (g_akao_streaming_state.spu_addr),
 * so the next akao_streaming_upload_tick treats its input as a new bank.
 *
 * @return 0 after the operation completes.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/qBE70
 */
s32 akao_reset_xfer_state(void)
{
    g_akao_streaming_state.spu_addr = 0;
    g_akao_driver_flags.upload_flags |= AKAO_UPLOAD_STREAMING;
    return 0;
}

/**
 * @brief Continue a streaming bank upload through its header, articulations and samples.
 *
 * The first call validates and stages the bank header. Available articulation
 * bytes are copied and relocated when the table is complete, then remaining
 * input is uploaded as samples. The streaming flag clears when no samples
 * remain, including when the header fails its magic check.
 *
 * @param source Input chunk, starting with the bank header on the first call.
 * @param available_bytes Number of input bytes available in this chunk.
 * @param wait_for_spu Non-zero to wait for the sample transfer to complete.
 * @return Sample bytes still to upload.
 * @note The first chunk must contain the complete bank header. Articulation
 *       chunks must contain whole words; akao_copy_bytes drops partial words.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/0IPqT
 */
s32 akao_streaming_upload_tick(u8* source, u32 available_bytes, s32 wait_for_spu)
{
    s32 copied_bytes;
    u32 articulation_chunk;
    u32 sample_chunk;
    AkaoArticulation* articulations;

    if (g_akao_driver_flags.upload_flags & AKAO_UPLOAD_STREAMING)
    {
        if (g_akao_streaming_state.spu_addr == 0)
        {
            if (akao_check_magic((AkaoHeader*)source) == 0)
            {
                akao_copy_bytes((s32*)source, (s32*)&g_akao_bank_staging, sizeof(AkaoBankHeader));
                source = (u8*)((AkaoBankHeader*)source + 1);
                available_bytes -= sizeof(AkaoBankHeader);
                g_akao_streaming_state.spu_addr = g_akao_bank_staging.spu_dest_addr;
                g_akao_streaming_state.sample_remaining = g_akao_bank_staging.sample_size;
                g_akao_streaming_state.articulation_dst = (u8*)&((AkaoArticulation*)g_akao_articulation_slots)[g_akao_bank_staging.bank_id];
                g_akao_streaming_state.articulation_remaining = g_akao_bank_staging.articulation_count * sizeof(AkaoArticulation);
            }
            else
            {
                available_bytes = 0;
                g_akao_streaming_state.sample_remaining = 0U;
                g_akao_streaming_state.articulation_remaining = 0U;
            }
        }
        if (g_akao_streaming_state.articulation_remaining != 0)
        {
            articulation_chunk = g_akao_streaming_state.articulation_remaining;
            if (available_bytes != 0)
            {
                if (articulation_chunk >= available_bytes)
                {
                    articulation_chunk = available_bytes;
                }
                akao_copy_bytes((s32*)source, (s32*)g_akao_streaming_state.articulation_dst, articulation_chunk);
                copied_bytes = (articulation_chunk >> AKAO_COPY_WORD_SHIFT) * AKAO_COPY_WORD_BYTES;
                source += copied_bytes;
                available_bytes -= articulation_chunk;
                g_akao_streaming_state.articulation_dst = g_akao_streaming_state.articulation_dst + copied_bytes;
                g_akao_streaming_state.articulation_remaining -= articulation_chunk;
                if (g_akao_streaming_state.articulation_remaining == 0)
                {
                    articulations = &((AkaoArticulation*)g_akao_articulation_slots)[g_akao_bank_staging.bank_id];
                    akao_relocate_articulations(articulations, articulations, g_akao_bank_staging.spu_dest_addr, g_akao_bank_staging.articulation_count);
                }
            }
        }
        if (available_bytes != 0 && g_akao_streaming_state.sample_remaining == 0)
        {
            g_akao_driver_flags.upload_flags &= ~AKAO_UPLOAD_STREAMING;
        }
        else
        {
            if (available_bytes != 0)
            {
                sample_chunk = g_akao_streaming_state.sample_remaining;
                if (g_akao_streaming_state.sample_remaining >= available_bytes)
                {
                    sample_chunk = available_bytes;
                }
                available_bytes = sample_chunk;
                SpuSetTransferStartAddr(g_akao_streaming_state.spu_addr);
                akao_spu_write(source, available_bytes);
                g_akao_streaming_state.spu_addr += available_bytes;
                g_akao_streaming_state.sample_remaining -= available_bytes;
                if (wait_for_spu != 0)
                {
                    akao_spu_wait();
                }
            }
            if (g_akao_streaming_state.sample_remaining == 0)
            {
                g_akao_driver_flags.upload_flags &= ~AKAO_UPLOAD_STREAMING;
            }
        }
    }
    return g_akao_streaming_state.sample_remaining;
}

/**
 * @brief Upload an AKAO instrument bank and return zero.
 * @param bank Pointer to an AKAO instrument bank in main RAM.
 * @param wait_for_completion Non-zero to wait for the SPU DMA to complete.
 * @return 0 after the upload call returns.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/0f3IK
 */
s32 akao_load_bank(AkaoBankHeader* bank, s32 wait_for_completion)
{
    akao_upload_bank_blocking(bank, wait_for_completion);
    return 0;
}

/**
 * @brief Route a bank to one of six SPU slots, record its key and upload it.
 *
 * @param bank AKAO instrument bank in RAM.
 * @param slot Slot 1 through 5; other values select slot 0.
 * @param wait_for_completion Non-zero to wait for the SPU transfer.
 * @return 0 after the operation completes.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/FWcdy
 */
s32 akao_upload_bank_slot(void* bank, s32 slot, s32 wait_for_completion)
{
    s32 articulation_index;
    s32* slot_key;
    u32 spu_base;
    AkaoBankIdentity* identity = bank;

    for (spu_base = 0, slot_key = g_akao_bank_slot_keys; spu_base < AKAO_BANK_SLOT_COUNT; spu_base++, slot_key++)
    {
        if (*slot_key == identity->key)
        {
            *slot_key = 0;
        }
    }

    switch (slot)
    {
    case 1:
        spu_base = AKAO_BANK_SLOT_SPU_ADDRESS(1);
        articulation_index = AKAO_BANK_SLOT_ARTICULATION_INDEX(1);
        g_akao_bank_slot_keys[1] = identity->key;
        break;

    case 2:
        spu_base = AKAO_BANK_SLOT_SPU_ADDRESS(2);
        articulation_index = AKAO_BANK_SLOT_ARTICULATION_INDEX(2);
        g_akao_bank_slot_keys[2] = identity->key;
        break;

    case 3:
        spu_base = AKAO_BANK_SLOT_SPU_ADDRESS(3);
        articulation_index = AKAO_BANK_SLOT_ARTICULATION_INDEX(3);
        g_akao_bank_slot_keys[3] = identity->key;
        break;

    case 4:
        spu_base = AKAO_BANK_SLOT_SPU_ADDRESS(4);
        articulation_index = AKAO_BANK_SLOT_ARTICULATION_INDEX(4);
        g_akao_bank_slot_keys[4] = identity->key;
        break;

    case 5:
        spu_base = AKAO_BANK_SLOT_SPU_ADDRESS(5);
        articulation_index = AKAO_BANK_SLOT_ARTICULATION_INDEX(5);
        g_akao_bank_slot_keys[5] = identity->key;
        break;

    default:
        spu_base = AKAO_BANK_SLOT_SPU_ADDRESS(0);
        articulation_index = AKAO_BANK_SLOT_ARTICULATION_INDEX(0);
        g_akao_bank_slot_keys[0] = identity->key;
        break;
    }

    akao_upload_bank(bank, wait_for_completion, articulation_index, spu_base);
    return 0;
}

/**
 * @brief Wrapper: forwards @p slot unchanged to akao_upload_bank_slot
 *        (selects bank slots 0..5 directly).
 * @param bank AKAO instrument bank in RAM.
 * @param slot Slot selector forwarded to akao_upload_bank_slot.
 * @param wait_for_completion Non-zero to wait for the SPU transfer.
 * @return 0 after the operation completes.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/sa1fh
 */
s32 akao_load_bank_slot(void* bank, s32 slot, s32 wait_for_completion)
{
    akao_upload_bank_slot(bank, slot, wait_for_completion);
    return 0;
}

/**
 * @brief Add AKAO_BANK_FIRST_UPPER_SLOT to @p slot and upload the bank.
 * @param bank AKAO instrument bank in RAM.
 * @param slot Slot offset; 0 through 2 select slots 3 through 5.
 * @param wait_for_completion Non-zero to wait for the SPU transfer.
 * @return 0 after the operation completes.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/PnDWc
 */
s32 akao_load_upper_bank_slot(void* bank, s32 slot, s32 wait_for_completion)
{
    akao_upload_bank_slot(bank, slot + AKAO_BANK_FIRST_UPPER_SLOT, wait_for_completion);
    return 0;
}

/**
 * @brief Program the CD audio mix for the current output mode.
 *
 * Stereo routes CD left to SPU left and CD right to SPU right. Mono sends both
 * CD channels to both outputs, each scaled by about 0.35437. For identical
 * left and right signals, the combined mono gain is about 0.70874 (-3 dB)
 * before integer rounding.
 *
 * @param volume CD mix volume (0-127).
 * @return 0.
 * @see decomp.me (100%) https://decomp.me/scratch/hcfmi
 */
s32 akao_set_cd_mix(s32 volume)
{
    if (g_akao_driver_flags.output_mode & AKAO_OUTPUT_MONO)
    {
        g_akao_cdmix.val3 = (u32)(volume * AKAO_CD_MONO_GAIN_Q17) >> AKAO_CD_MONO_GAIN_SHIFT;
        g_akao_cdmix.val1 = (u32)(volume * AKAO_CD_MONO_GAIN_Q17) >> AKAO_CD_MONO_GAIN_SHIFT;
        g_akao_cdmix.val2 = (u32)(volume * AKAO_CD_MONO_GAIN_Q17) >> AKAO_CD_MONO_GAIN_SHIFT;
        g_akao_cdmix.val0 = (u32)(volume * AKAO_CD_MONO_GAIN_Q17) >> AKAO_CD_MONO_GAIN_SHIFT;
    }
    else
    {
        g_akao_cdmix.val2 = volume;
        g_akao_cdmix.val0 = volume;
        g_akao_cdmix.val3 = 0;
        g_akao_cdmix.val1 = 0;
    }
    CdMix(&g_akao_cdmix);
    return 0;
}

/**
 * @brief Queue command 0xE0: stream an XA program from a RAM buffer.
 * @param buffer AKAO-tagged XA program in main RAM; ignored if the magic does not match.
 * @param pan Pan; only the low 8 bits are used.
 * @param use_reverb Non-zero to send the stream voices through reverb.
 * @see decomp.me (100%) https://decomp.me/scratch/vw9QX
 */
void akao_play_xa_buffer(AkaoHeader* buffer, s32 pan, s32 use_reverb)
{
    if (akao_check_magic(buffer) == 0)
    {
        g_akao_cmd_params[0].buffer = buffer;
        g_akao_cmd_params[1].value = ((pan & AKAO_PAN_MASK) << AKAO_XA_FIXED_POINT_SHIFT);
        g_akao_cmd_params[2].value = use_reverb;
        akao_send_command(AKAO_CMD_PLAY_XA_BUFFER);
    }
}

/**
 * @brief Queue command 0xE2: stop the streamed voice pair.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/kd4bK
 */
s32 akao_stop_xa(void)
{
    return akao_send_command(AKAO_CMD_STOP_XA);
}

/**
 * @brief Queue command 0xE4: set the streamed voice volume.
 * @param volume Volume; only the low 7 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/3oPkP
 */
s32 akao_set_xa_volume(s32 volume)
{
    g_akao_cmd_params[0].value = (volume & AKAO_VOLUME_MAX) << AKAO_XA_FIXED_POINT_SHIFT;
    return akao_send_command(AKAO_CMD_SET_XA_VOLUME);
}

/**
 * @brief Queue command 0xE5: fade the streamed voice volume.
 * @param ticks Fade length in driver ticks; 0 is treated as 1.
 * @param volume Target volume; only the low 7 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/7PxF8
 */
s32 akao_fade_xa_volume(s32 ticks, s32 volume)
{
    g_akao_cmd_params[0].value = ticks;
    g_akao_cmd_params[1].value = ((volume & AKAO_VOLUME_MAX) << AKAO_XA_FIXED_POINT_SHIFT);
    return akao_send_command(AKAO_CMD_FADE_XA_VOLUME);
}

/**
 * @brief Queue command 0xE6: set the streamed voice pan.
 * @param pan Pan; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/XeUon
 */
s32 akao_set_xa_pan(s32 pan)
{
    g_akao_cmd_params[0].value = (pan & AKAO_PAN_MASK) << AKAO_XA_FIXED_POINT_SHIFT;
    return akao_send_command(AKAO_CMD_SET_XA_PAN);
}

/**
 * @brief Magic-checks an AKAO XA program and stages it for the SPU.
 *
 * Uses bank slot 0 or AKAO_BANK_FIRST_UPPER_SLOT as the SPU destination,
 * lowered by AKAO_XA_SPU_AREA_OFFSET when a loaded song selects the lower area.
 * Waits for the previous SPU transfer, uploads the sample data and records the
 * destination in the input header. Stages the header and first 16 sample bytes
 * in g_akao_xa_program_staging.
 *
 * @param buffer  Pointer to an AKAO buffer in main RAM.
 * @param upper_slot  Selects the upper SPU slot (non-zero) vs the lower slot.
 *
 * @return 0 on success; the akao_check_magic delta on failure (also clears
 *         @c g_akao_xa_program_staging.header.spu_addr).
 *
 * @see decomp.me (100%) https://decomp.me/scratch/C06sg
 */
s32 akao_upload_xa_program(void* buffer, s32 upper_slot)
{
    s32 result;
    s32 spu_base;
    AkaoXaProgramHeader* program;

    result = akao_check_magic(buffer);
    if (result == 0)
    {
        akao_spu_wait();
        spu_base = AKAO_BANK_SLOT_SPU_ADDRESS(AKAO_BANK_FIRST_UPPER_SLOT);
        if (upper_slot == 0)
        {
            spu_base = AKAO_BANK_SLOT_SPU_ADDRESS(0);
        }
        /* Loaded songs with this flag use the lower XA area. */
        if (((AKAO_PRIMARY_SONG->masks.active_mask | AKAO_PRIMARY_SONG->parked_mask) != 0) &&
            (AKAO_PRIMARY_SONG->flags & AKAO_SONG_LOWER_XA_AREA_FLAG))
        {
            spu_base -= AKAO_XA_SPU_AREA_OFFSET;
        }
        program = buffer;
        buffer = program + 1;
        SpuSetTransferStartAddr(spu_base);
        akao_spu_write(buffer, program->sample_size);
        program->spu_addr = spu_base;
        akao_copy_bytes((s32*)program, (s32*)&g_akao_xa_program_staging, sizeof(g_akao_xa_program_staging));
        return result;
    }

    g_akao_xa_program_staging.header.spu_addr = 0;
    return result;
}

/**
 * @brief Queue command 0xED: play the XA program staged by akao_upload_xa_program.
 * @param pan Pan; only the low 8 bits are used.
 * @param use_reverb Non-zero to send the stream voices through reverb.
 * @return Result returned by the AKAO command dispatcher.
 * @see decomp.me (100%) https://decomp.me/scratch/ULEGL
 */
s32 akao_play_staged_xa(s32 pan, s32 use_reverb)
{
    g_akao_cmd_params[0].value = ((pan & AKAO_PAN_MASK) << AKAO_XA_FIXED_POINT_SHIFT);
    g_akao_cmd_params[1].value = use_reverb;
    return akao_send_command(AKAO_CMD_PLAY_STAGED_XA);
}

/**
 * @brief Queue command 0xEC: upload an XA program to SPU RAM and play it once.
 *
 * Uses the same SPU placement rule as akao_upload_xa_program.
 *
 * @param buffer AKAO-tagged XA program in main RAM; ignored if the magic does not match.
 * @param pan Pan; only the low 8 bits are used.
 * @param upper_slot Non-zero selects the upper SPU area.
 * @param use_reverb Non-zero to send the stream voices through reverb.
 * @see decomp.me (100%) https://decomp.me/scratch/SgcFo
 */
void akao_play_xa_one_shot(void* buffer, s32 pan, s32 upper_slot, s32 use_reverb)
{
    s32 spu_base;

    if (akao_check_magic(buffer) != 0)
    {
        return;
    }

    spu_base = upper_slot == 0 ? AKAO_BANK_SLOT_SPU_ADDRESS(0) : AKAO_BANK_SLOT_SPU_ADDRESS(AKAO_BANK_FIRST_UPPER_SLOT);

    if (((AKAO_PRIMARY_SONG->masks.active_mask | AKAO_PRIMARY_SONG->parked_mask) != 0) &&
        (AKAO_PRIMARY_SONG->flags & AKAO_SONG_LOWER_XA_AREA_FLAG))
    {
        spu_base -= AKAO_XA_SPU_AREA_OFFSET;
    }

    g_akao_cmd_params[0].buffer = buffer;
    g_akao_cmd_params[1].value = ((pan & AKAO_PAN_MASK) << AKAO_XA_FIXED_POINT_SHIFT);
    g_akao_cmd_params[2].value = spu_base;
    g_akao_cmd_params[3].value = use_reverb;
    akao_send_command(AKAO_CMD_PLAY_XA_ONE_SHOT);
}

/**
 * @brief Queue command 0xE8: prepare a CD-fed XA ring stream.
 *
 * The ring holds @p byte_count / AKAO_XA_RING_BLOCK_BYTES blocks. The caller
 * fills it and reports each block with akao_xa_advance_frame.
 *
 * @param ring_base First ring block in main RAM.
 * @param byte_count Ring size in bytes.
 * @return 0 on success, -1 if @p byte_count is 0.
 * @see decomp.me (100%) https://decomp.me/scratch/bRIJX
 */
s32 akao_start_xa_stream(void* ring_base, u32 byte_count)
{
    if (byte_count == 0)
    {
        return -1;
    }
    SpuSetIRQ(SPU_OFF);
    SpuSetIRQAddr(0);
    g_akao_cmd_params[0].buffer = ring_base;
    g_akao_cmd_params[1].value = byte_count;
    g_akao_xa_tracker.upload_block = AKAO_XA_UPLOAD_BLOCK_NONE;
    g_akao_xa_tracker.last_ring_block_key = 0;
    g_akao_xa_tracker.filled_blocks = 0;
    g_akao_xa_tracker.uploaded_blocks = 0;
    g_akao_xa_tracker.fill_block = 0;
    g_akao_xa_tracker.ring_block_count = byte_count >> AKAO_XA_RING_BLOCK_SHIFT;
    akao_send_command(AKAO_CMD_PREPARE_XA_RING);
    return 0;
}

/**
 * @brief Report that the CD reader has filled one more XA ring block.
 *
 * Increments @c g_akao_xa_tracker.filled_blocks and advances @c fill_block,
 * wrapping after the last ring block. For a ring stream (XA_FLAG_RING_STREAM),
 * calls akao_xa_start_ring_stream whenever the next-fill index is at least
 * AKAO_XA_START_FILL_BLOCK so the SPU upload can begin.
 *
 * @return Index of the next ring block the SPU upload will read.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/gKZ5G
 */
s32 akao_xa_advance_frame(void)
{
    u32 next_fill_block;

    g_akao_xa_tracker.filled_blocks = g_akao_xa_tracker.filled_blocks + 1;
    next_fill_block = g_akao_xa_tracker.fill_block + 1;
    g_akao_xa_tracker.fill_block = next_fill_block;
    if (next_fill_block > g_akao_xa_tracker.ring_block_count - 1)
    {
        g_akao_xa_tracker.fill_block = 0;
    }
    if ((g_akao_xa_tracker.flags & XA_FLAG_RING_STREAM) && (g_akao_xa_tracker.fill_block >= AKAO_XA_START_FILL_BLOCK))
    {
        akao_xa_start_ring_stream();
    }
    return g_akao_xa_tracker.upload_block;
}

/**
 * @brief Returns the current XA ring block index.
 *
 * @return Index of the next ring block the SPU upload will read.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/2DiS3
 */
s32 akao_xa_get_position(void)
{
    return g_akao_xa_tracker.upload_block;
}
