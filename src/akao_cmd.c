#include "akao_cmd.h"
#include "akao.h"
#include "akao_driver.h"
#include "sdk/libcd.h"

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
    u8* articulation_dst;     /* 0x00: current dst into the driver's
                                          articulation slot table
                                          (g_akao_articulation_slots + bank_id * 0x10),
                                          advances as bytes are copied      */
    u32 spu_addr;               /* 0x04: current SPU upload address - seeded
                                          from AkaoBankHeader.spu_dest_addr,
                                          advances as samples are written;
                                          a value of 0 marks "first tick"   */
    u32 sample_remaining;       /* 0x08: bytes of sample data still to send
                                          to the SPU                        */
    u32 articulation_remaining; /* 0x0C: bytes of articulation data still to
                                          copy into the driver's slot table */
} AkaoStreamingState;

/** @brief Bank identity prefix; the key combines the header's id and length. */
typedef struct
{
    u32 magic;
    s32 key;
} AkaoBankIdentity;

extern s32 D_8004F794;
/* 0x50-byte staging copy of an XA program's AkaoBankHeader (akao_upload_xa_program). */
extern AkaoBankHeader g_akao_xa_program_staging;
extern CdlATV g_akao_cdmix;
extern s32 D_8004F754;
extern s32 D_8004F824;
extern s32 D_8004F828;
extern AkaoStreamingState g_akao_streaming_state;
extern AkaoBankHeader g_akao_bank_staging;

#define AKAO_CHANNEL_STATE (*(AkaoChannelState**)0x8003EC5C)

/**
 * Central dispatcher for the AKAO sound driver. Each high-level wrapper
 * (akao_play_song, akao_stop_song, akao_play_sfx, etc.) writes its inputs
 * into g_akao_cmd_params and then invokes this function with a one-byte
 * command opcode. Known opcodes used in this codebase: 0x10 play song,
 * 0x11 stop song, 0x12, 0x14, 0x19, 0x20 play SFX, 0x21, 0x24, 0x30, 0x40,
 * 0x80/0x81, 0x90/0x92, 0xA0/0xA1/0xA8/0xA9, 0xC0/0xC1, 0xF0/0xF1.
 *
 * @param opcode Command opcode; only the low byte is significant.
 * @return For opcodes 0x10/0x12/0x14/0x19 (load/change song), the newly
 *         loaded sequence id, or 0 if the requested song was already active,
 *         or -1 if the header failed the AKAO magic check. Unused/ignored by
 *         most callers otherwise.
 */
s32 akao_send_command(u32 opcode);

/**
 * @brief Public init entry - wraps akao_driver_init and returns 0.
 * @return 0 after initialization.
 *
 * @see https://decomp.me/scratch/hDNyF (100%)
 */
s32 FUN_80021fbc(void)
{
    akao_driver_init();
    return 0;
}

/**
 * @brief Public shutdown entry - wraps akao_driver_shutdown and returns 0.
 * @return 0 after shutdown.
 *
 * @see https://decomp.me/scratch/z7ZEh (100%)
 */
s32 func_80021FDC(void)
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
 * @see decomp.me: (100%) https://decomp.me/scratch/0q180
 */
s32 akao_register_bank(AkaoHeader* bank)
{
    s32 result;

    result = akao_check_magic(bank);
    if (result == 0)
    {
        akao_set_bank_data_ptrs((s32)(bank + 1));
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
 *                      as @c &D_8003ECA0 in TITLE.
 * @return Song handle returned by the AKAO command dispatcher.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/iVOOb
 */
s32 akao_play_song(AkaoHeader* sequence_data)
{
    g_akao_cmd_params[0].buffer = sequence_data;
    return akao_send_command(AKAO_CMD_PLAY_SONG);
}

/**
 * @brief AKAO command 0x11 - stop the currently-playing sequence.
 *
 * Pushes @p stop_mode into the AKAO command parameter buffer and dispatches the
 * "stop song" command. Callers in TITLE/CHECKPS pass 0; the precise meaning
 * of non-zero values (likely a fade-out duration or flag) is not yet known.
 *
 * @param stop_mode  Stop-modifier parameter; observed value is 0 in all callers.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/9M4hF
 */
void akao_stop_song(s32 stop_mode)
{
    g_akao_cmd_params[0].value = stop_mode;
    akao_send_command(AKAO_CMD_STOP_SONG);
}

/**
 * @brief AKAO command 0x40 - global stop / driver halt.
 *
 * Zero-argument command. Observed callers in cdrom.c and others use this to
 * silence everything (sequences and active SFX) when entering loading screens
 * or other audio-quiescent states.
 *
 * @see https://decomp.me/scratch/4GVez (100%)
 */
void akao_cmd_40(void)
{
    akao_send_command(AKAO_CMD_GLOBAL_STOP);
}

/**
 * @brief AKAO command 0x14 - three args, third slot forced 0; semantics TBD.
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; semantics unknown.
 *
 * @see https://decomp.me/scratch/c2C3m (100%)
 */
void akao_cmd_14(s32 value0, s32 value1)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = value1;
    g_akao_cmd_params[2].value = 0;
    akao_send_command(AKAO_CMD_14);
}

/**
 * @brief Combo: dispatch AKAO command 0x19 (a) then 0xC0 (b masked to 7 bits).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; semantics unknown.
 * @return Current transfer or position latch.
 *
 * @see https://decomp.me/scratch/d6xXt (100%)
 */
s32 akao_cmd_19_c0(s32 value0, s32 value1)
{
    s32 result;

    g_akao_cmd_params[0].value = value0;
    result = akao_send_command(AKAO_CMD_19);
    g_akao_cmd_params[0].value = (value1 & 0x7F);
    g_akao_cmd_params[3].value = 0;
    akao_send_command(AKAO_CMD_SET_SONG_VOLUME);
    return result;
}

/**
 * @brief AKAO command 0x12 - two unmasked args; semantics TBD.
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; semantics unknown.
 *
 * @see https://decomp.me/scratch/jigab (100%)
 */
void akao_cmd_12(s32 value0, s32 value1)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = value1;
    akao_send_command(AKAO_CMD_12);
}

/**
 * @brief AKAO command 0x20 - play a sound effect.
 *
 * Packs four caller-supplied values into the AKAO command parameter buffer,
 * each masked to the bit-width the driver expects, then dispatches the
 * "play SFX" command. The mask widths suggest:
 *   sound_id (10 bits) - sound id / SFX index
 *   parameter (24 bits) - wider opaque parameter (possibly pitch/frequency)
 *   pan ( 8 bits) - byte-sized parameter (possibly pan)
 *   volume ( 7 bits) - volume (0-127)
 * Caller in TITLE: play_title_sfx(sound_id, _, pan, 0x7F).
 *
 * @param sound_id  Sound id (lower 10 bits used).
 * @param parameter  24-bit packed parameter.
 * @param pan  8-bit parameter.
 * @param volume  Volume (0-127).
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/9AZZL
 */
void akao_play_sfx(s32 sound_id, s32 parameter, s32 pan, s32 volume)
{
    g_akao_cmd_params[0].value = (sound_id & 0x3FF);
    g_akao_cmd_params[1].value = (parameter & 0xFFFFFF);
    g_akao_cmd_params[2].value = (pan & 0xFF);
    g_akao_cmd_params[3].value = (volume & 0x7F);
    akao_send_command(AKAO_CMD_PLAY_SFX);
}

/**
 * @brief AKAO command 0x24 - play SFX from a caller-supplied AKAO buffer (magic-checked); same arg shape as
 * akao_play_sfx (24/8/7-bit).
 *
 * @param buffer_address Address of an AKAO-tagged sound buffer.
 * @param parameter Packed parameter; only the low 24 bits are used.
 * @param pan Pan parameter; only the low 8 bits are used.
 * @param volume Volume; only the low 7 bits are used.
 * @return Current transfer or position latch.
 *
 * @see https://decomp.me/scratch/FFGei (100%)
 */
s32 akao_play_sfx_from_buffer(s32 buffer_address, s32 parameter, s32 pan, s32 volume)
{
    s32 result = akao_check_magic((AkaoHeader*)buffer_address);

    if (result != 0)
    {
        return result;
    }

    g_akao_cmd_params[0].buffer = (void*)buffer_address;
    g_akao_cmd_params[1].value = parameter & 0xFFFFFF;
    g_akao_cmd_params[2].value = pan & 0xFF;
    g_akao_cmd_params[3].value = volume & 0x7F;
    akao_send_command(AKAO_CMD_PLAY_SFX_FROM_BUF);

    return buffer_address;
}

/**
 * @brief AKAO command 0x21 - (id, p24) sound id plus 24-bit param.
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 24 bits are used.
 *
 * @see https://decomp.me/scratch/lu9nS (100%)
 */
void akao_cmd_21(s32 value0, s32 value1)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = (value1 & 0xFFFFFF);
    akao_send_command(AKAO_CMD_21);
}

/**
 * @brief AKAO command 0x30 - stop SFX whose 10-bit sound id matches @p sound_id.
 *
 * @param sound_id Sound identifier; only the low 10 bits are used.
 *
 * @see https://decomp.me/scratch/0mLzI (100%)
 */
void akao_stop_sfx_by_id(s32 sound_id)
{
    g_akao_cmd_params[0].value = sound_id & 0x3FF;
    akao_send_command(AKAO_CMD_STOP_SFX_BY_ID);
}

/**
 * @brief Scans active SFX channels and ORs together their offset-0x28 fields.
 *
 * Iterates over the 12 SFX-channel slots in @c g_sfx_channels (each 0x118 bytes),
 * gated by the bitmap in @c g_akao_sfx_control (one bit per channel starting at
 * 0x1000); returns the bitwise-OR of the 32-bit value at offset 0x28 of every
 * active slot, masked to 24 bits.
 *
 * @return Bitwise OR of active channel identifiers, masked to 24 bits.
 *
 * @see https://decomp.me/scratch/yZloM (100%)
 */
s32 akao_get_active_sfx_ids(void)
{
    s32 active_channels;
    AkaoChannelState* channel;
    s32 active_ids;
    u32 channel_bit;

    active_channels = g_akao_sfx_control.unk0;
    active_ids = active_channels == 0;
    if (active_ids)
    {
        return 0;
    }
    channel = (AkaoChannelState*)g_sfx_channels;
    active_ids = 0;
    channel_bit = 0x1000;
    do
    {
        if (active_channels & channel_bit)
        {
            active_ids |= channel->tempo_acc;
        }
        channel_bit <<= 1;
        channel++;
    } while (channel_bit & 0xFFFFFF);
    return active_ids & 0xFFFFFF;
}

/**
 * @brief Returns 1 if any active SFX channel's offset-0x28 field equals @p sound_id.
 *
 * Same iteration shape as @c akao_get_active_sfx_ids over @c g_sfx_channels / @c g_akao_sfx_control,
 * but compares each active channel's offset-0x28 value to @p sound_id; returns
 * 1 on first match, 0 otherwise.
 *
 * @param sound_id  Sound id / handle to look for.
 * @return 1 if a matching active channel exists, 0 otherwise.
 *
 * @see https://decomp.me/scratch/OvqYq (100%)
 */
s32 akao_is_sfx_playing(s32 sound_id)
{
    s32 active_channels;
    AkaoChannelState* channel;
    u32 channel_bit;

    if (sound_id == 0)
    {
        return 0;
    }
    active_channels = g_akao_sfx_control.unk0;
    if (active_channels == 0)
    {
        return 0;
    }
    channel = (AkaoChannelState*)g_sfx_channels;
    channel_bit = 0x1000;
    do
    {
        if (active_channels & channel_bit)
        {
            if (sound_id == channel->tempo_acc)
            {
                return 1;
            }
        }
        channel_bit <<= 1;
        channel++;
    } while (channel_bit & 0xFFFFFF);
    return 0;
}

/**
 * @brief AKAO command 0x80 / 0x81 - pause or resume the active sequence.
 *
 * Picks opcode 0x81 when @p mode == 1 (resume) and 0x80 otherwise (pause),
 * then dispatches with no parameter buffer payload. Used by TITLE.OVL to
 * pause music while the title screen is dismissed.
 *
 * @param mode  1 = resume (0x81); any other value = pause (0x80).
 *
 * @see https://decomp.me/scratch/9qTjH (100%)
 */
void akao_set_paused(s32 mode)
{
    if (mode == 1)
    {
        mode = AKAO_CMD_RESUME;
    }
    else
    {
        mode = AKAO_CMD_PAUSE;
    }
    akao_send_command(mode);
}

/**
 * @brief AKAO command 0x90 - single unmasked arg; semantics TBD.
 *
 * @param value0 Value for command slot 0; semantics unknown.
 *
 * @see https://decomp.me/scratch/x94md (100%)
 */
void akao_cmd_90(s32 value0)
{
    g_akao_cmd_params[0].value = value0;
    akao_send_command(AKAO_CMD_90);
}

/**
 * @brief AKAO command 0x92 - single unmasked arg; semantics TBD.
 *
 * @param value0 Value for command slot 0; semantics unknown.
 *
 * @see https://decomp.me/scratch/y9TAf (100%)
 */
void akao_cmd_92(s32 value0)
{
    g_akao_cmd_params[0].value = value0;
    akao_send_command(AKAO_CMD_92);
}

/**
 * @brief Dispatch one of AKAO commands 0x99/0x9B/0x9D/0x9F (zero-arg) selected by @p mode (1/2/3/default).
 *
 * @param mode Selects one of the command family members; 1, 2, and 3 have dedicated commands.
 *
 * @see https://decomp.me/scratch/qqSuG (100%)
 */
void akao_cmd_99_9b_9d_9f(u32 mode)
{
    s32 opcode;

    switch (mode)
    {
    case 1:
        opcode = AKAO_CMD_9B;
        break;
    case 2:
        opcode = AKAO_CMD_9D;
        break;
    case 3:
        opcode = AKAO_CMD_9F;
        break;
    default:
        opcode = AKAO_CMD_99;
        break;
    }

    akao_send_command(opcode);
}

/**
 * @brief Dispatch one of AKAO commands 0x98/0x9A/0x9C/0x9E (zero-arg) selected by @p mode (1/2/3/default).
 *
 * @param mode Selects one of the command family members; 1, 2, and 3 have dedicated commands.
 *
 * @see https://decomp.me/scratch/iREFc (100%)
 */
void akao_cmd_98_9a_9c_9e(u32 mode)
{
    s32 opcode;

    switch (mode)
    {
    case 1:
        opcode = AKAO_CMD_9A;
        break;
    case 2:
        opcode = AKAO_CMD_9C;
        break;
    case 3:
        opcode = AKAO_CMD_9E;
        break;
    default:
        opcode = AKAO_CMD_98;
        break;
    }

    akao_send_command(opcode);
}

/**
 * @brief AKAO command 0xA8 - global counterpart of 0xA0; takes a 7-bit value.
 *
 * @param value0 Value for command slot 0; only the low 7 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/VTGCB (100%)
 */
s32 akao_cmd_a8(s32 value0)
{
    g_akao_cmd_params[0].value = value0 & 0x7F;
    return akao_send_command(AKAO_CMD_A8);
}

/**
 * @brief AKAO command 0xA9 - global counterpart of 0xA1; (a, 7-bit value).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 7 bits are used.
 *
 * @see https://decomp.me/scratch/03hNO (100%)
 */
void akao_cmd_a9(s32 value0, s32 value1)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = (value1 & 0x7F);
    akao_send_command(AKAO_CMD_A9);
}

/**
 * @brief AKAO command 0xA0 - per-channel: (channel, 24-bit fade duration, 7-bit target value).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 24 bits are used.
 * @param value2 Value for command slot 2; only the low 7 bits are used.
 *
 * @see https://decomp.me/scratch/C8UTP (100%)
 */
void akao_cmd_a0(s32 value0, s32 value1, s32 value2)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = (value1 & 0xFFFFFF);
    g_akao_cmd_params[2].value = (value2 & 0x7F);
    akao_send_command(AKAO_CMD_A0);
}

/**
 * @brief AKAO command 0xA1 - per-channel: (channel, 24-bit fade duration, p, 7-bit target value).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 24 bits are used.
 * @param value2 Value for command slot 2; semantics unknown.
 * @param value3 Value for command slot 3; only the low 7 bits are used.
 *
 * @see https://decomp.me/scratch/xMNn0 (100%)
 */
void akao_cmd_a1(s32 value0, s32 value1, s32 value2, s32 value3)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = (value1 & 0xFFFFFF);
    g_akao_cmd_params[2].value = value2;
    g_akao_cmd_params[3].value = (value3 & 0x7F);
    akao_send_command(AKAO_CMD_A1);
}

/**
 * @brief AKAO command 0xAA - global counterpart of 0xA2; takes an 8-bit value.
 *
 * @param value0 Value for command slot 0; only the low 8 bits are used.
 *
 * @see https://decomp.me/scratch/AuyLX (100%)
 */
void akao_cmd_aa(s32 value0)
{
    g_akao_cmd_params[0].value = value0 & 0xFF;
    akao_send_command(AKAO_CMD_AA);
}

/**
 * @brief AKAO command 0xAB - global counterpart of 0xA3; (a, 8-bit value).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 8 bits are used.
 *
 * @see https://decomp.me/scratch/IaBX9 (100%)
 */
void akao_cmd_ab(s32 value0, s32 value1)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = (value1 & 0xFF);
    akao_send_command(AKAO_CMD_AB);
}

/**
 * @brief AKAO command 0xA2 - per-channel: (channel, 24-bit fade duration, 8-bit target value).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 24 bits are used.
 * @param value2 Value for command slot 2; only the low 8 bits are used.
 *
 * @see https://decomp.me/scratch/LhoLV (100%)
 */
void akao_cmd_a2(s32 value0, s32 value1, s32 value2)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = (value1 & 0xFFFFFF);
    g_akao_cmd_params[2].value = (value2 & 0xFF);
    akao_send_command(AKAO_CMD_A2);
}

/**
 * @brief AKAO command 0xA3 - per-channel: (channel, 24-bit fade duration, p, 8-bit target value).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 24 bits are used.
 * @param value2 Value for command slot 2; semantics unknown.
 * @param value3 Value for command slot 3; only the low 8 bits are used.
 *
 * @see https://decomp.me/scratch/Al5YT (100%)
 */
void akao_cmd_a3(s32 value0, s32 value1, s32 value2, s32 value3)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = (value1 & 0xFFFFFF);
    g_akao_cmd_params[2].value = value2;
    g_akao_cmd_params[3].value = (value3 & 0xFF);
    akao_send_command(AKAO_CMD_A3);
}

/**
 * @brief AKAO command 0xAC - global counterpart of 0xA4; takes an 8-bit value.
 *
 * @param value0 Value for command slot 0; only the low 8 bits are used.
 *
 * @see https://decomp.me/scratch/e4D90 (100%)
 */
void akao_cmd_ac(s32 value0)
{
    g_akao_cmd_params[0].value = value0 & 0xFF;
    akao_send_command(AKAO_CMD_AC);
}

/**
 * @brief AKAO command 0xAD - global counterpart of 0xA5; (a, 8-bit value).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 8 bits are used.
 *
 * @see https://decomp.me/scratch/Fw2d9 (100%)
 */
void akao_cmd_ad(s32 value0, s32 value1)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = (value1 & 0xFF);
    akao_send_command(AKAO_CMD_AD);
}

/**
 * @brief AKAO command 0xA4 - per-channel: (channel, 24-bit fade duration, 8-bit target value).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 24 bits are used.
 * @param value2 Value for command slot 2; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/vHMVZ (100%)
 */
s32 akao_cmd_a4(s32 value0, s32 value1, s32 value2)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = (value1 & 0xFFFFFF);
    g_akao_cmd_params[2].value = (value2 & 0xFF);
    return akao_send_command(AKAO_CMD_A4);
}

/**
 * @brief AKAO command 0xA5 - per-channel: (channel, 24-bit fade duration, p, 8-bit target value).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 24 bits are used.
 * @param value2 Value for command slot 2; semantics unknown.
 * @param value3 Value for command slot 3; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/exTVG (100%)
 */
s32 akao_cmd_a5(s32 value0, s32 value1, s32 value2, s32 value3)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = (value1 & 0xFFFFFF);
    g_akao_cmd_params[2].value = value2;
    g_akao_cmd_params[3].value = (value3 & 0xFF);
    return akao_send_command(AKAO_CMD_A5);
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
    s32 masked_volume;

    g_akao_cmd_params[0].value = song_handle;
    masked_volume = volume & AKAO_VOLUME_MAX;
    g_akao_cmd_params[1].value = masked_volume;
    return akao_send_command(AKAO_CMD_SET_SONG_VOLUME);
}

/**
 * @brief AKAO command 0xC1 - 0xC0 with extra middle parameter: (a, b, 7-bit value).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; semantics unknown.
 * @param value2 Value for command slot 2; only the low 7 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/cSIwP (100%)
 */
s32 akao_cmd_c1(s32 value0, s32 value1, s32 value2)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = value1;
    g_akao_cmd_params[2].value = (value2 & 0x7F);
    return akao_send_command(AKAO_CMD_C1);
}

/**
 * @brief AKAO command 0xC2 - 0xC0 with two trailing 7-bit values: (a, b, 7-bit, 7-bit).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; semantics unknown.
 * @param value2 Value for command slot 2; only the low 7 bits are used.
 * @param value3 Value for command slot 3; only the low 7 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/PbMJC (100%)
 */
s32 akao_cmd_c2(s32 value0, s32 value1, s32 value2, s32 value3)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = value1;
    g_akao_cmd_params[2].value = (value2 & 0x7F);
    g_akao_cmd_params[3].value = (value3 & 0x7F);
    return akao_send_command(AKAO_CMD_C2);
}

/**
 * @brief AKAO command 0xC8 - single unmasked arg; semantics TBD.
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/BeJR1 (100%)
 */
s32 akao_cmd_c8(s32 value0)
{
    g_akao_cmd_params[0].value = value0;
    return akao_send_command(AKAO_CMD_C8);
}

/**
 * @brief AKAO command 0xC9 - two unmasked args; semantics TBD.
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; semantics unknown.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/yo40G (100%)
 */
s32 akao_cmd_c9(s32 value0, s32 value1)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = value1;
    return akao_send_command(AKAO_CMD_C9);
}

/**
 * @brief AKAO command 0xCA - three unmasked args; semantics TBD.
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; semantics unknown.
 * @param value2 Value for command slot 2; semantics unknown.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/pLMBi (100%)
 */
s32 akao_cmd_ca(s32 value0, s32 value1, s32 value2)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = value1;
    g_akao_cmd_params[2].value = value2;
    return akao_send_command(AKAO_CMD_CA);
}

/**
 * @brief AKAO command 0xD0 - (8-bit value).
 *
 * @param value0 Value for command slot 0; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/klUxi (100%)
 */
s32 akao_cmd_d0(s32 value0)
{
    g_akao_cmd_params[0].value = value0 & 0xFF;
    return akao_send_command(AKAO_CMD_D0);
}

/**
 * @brief AKAO command 0xD1 - (a, 8-bit value).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/XXHwt (100%)
 */
s32 akao_cmd_d1(s32 value0, s32 value1)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = (value1 & 0xFF);
    return akao_send_command(AKAO_CMD_D1);
}

/**
 * @brief AKAO command 0xD2 - (a, 8-bit value, 8-bit value).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 8 bits are used.
 * @param value2 Value for command slot 2; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/074UT (100%)
 */
s32 akao_cmd_d2(s32 value0, s32 value1, s32 value2)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = (value1 & 0xFF);
    g_akao_cmd_params[2].value = (value2 & 0xFF);
    return akao_send_command(AKAO_CMD_D2);
}

/**
 * @brief AKAO command 0xD4 - (8-bit value).
 *
 * @param value0 Value for command slot 0; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/yJdLv (100%)
 */
s32 akao_cmd_d4(s32 value0)
{
    g_akao_cmd_params[0].value = value0 & 0xFF;
    return akao_send_command(AKAO_CMD_D4);
}

/**
 * @brief AKAO command 0xD5 - (a, 8-bit value).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/u6Eys (100%)
 */
s32 akao_cmd_d5(s32 value0, s32 value1)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = (value1 & 0xFF);
    return akao_send_command(AKAO_CMD_D5);
}

/**
 * @brief AKAO command 0xD6 - (a, 8-bit value, 8-bit value).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 8 bits are used.
 * @param value2 Value for command slot 2; only the low 8 bits are used.
 *
 * @see https://decomp.me/scratch/ITNFU (100%)
 */
void akao_cmd_d6(s32 value0, s32 value1, s32 value2)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = (value1 & 0xFF);
    g_akao_cmd_params[2].value = (value2 & 0xFF);
    akao_send_command(AKAO_CMD_D6);
}

/**
 * @brief AKAO command 0xD8 - (8-bit value).
 *
 * @param value0 Value for command slot 0; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/JS2nD (100%)
 */
s32 akao_cmd_d8(s32 value0)
{
    g_akao_cmd_params[0].value = value0 & 0xFF;
    return akao_send_command(AKAO_CMD_D8);
}

/**
 * @brief AKAO command 0xD9 - (a, 8-bit value).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/YD6rZ (100%)
 */
s32 akao_cmd_d9(s32 value0, s32 value1)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = (value1 & 0xFF);
    return akao_send_command(AKAO_CMD_D9);
}

/**
 * @brief AKAO command 0xDA - (a, 8-bit value, 8-bit value).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 8 bits are used.
 * @param value2 Value for command slot 2; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/jzW0l (100%)
 */
s32 akao_cmd_da(s32 value0, s32 value1, s32 value2)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = (value1 & 0xFF);
    g_akao_cmd_params[2].value = (value2 & 0xFF);
    return akao_send_command(AKAO_CMD_DA);
}

/**
 * @brief AKAO command 0xF0 - zero-arg query; return value consumed by caller.
 *
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/dgbnE (100%)
 */
s32 akao_cmd_f0(void)
{
    return akao_send_command(AKAO_CMD_F0);
}

/**
 * @brief AKAO command 0xF1 - zero-arg query; return value consumed by caller.
 *
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/IMYAL (100%)
 */
s32 akao_cmd_f1(void)
{
    return akao_send_command(AKAO_CMD_F1);
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
 * @see decomp.me: (100%) https://decomp.me/scratch/Mz7yX
 */
void akao_upload_bank_blocking(AkaoBankHeader* bank, s32 wait_for_completion)
{
    g_akao_driver_flags.unk0 &= ~1;
    while (akao_submit_bank(bank, wait_for_completion) == 1)
    {
    }
}

/**
 * @brief Returns the current SPU/AKAO transfer state latch (g_akao_spu_xfer_pending).
 *
 * @return Current transfer or position latch.
 *
 * @see https://decomp.me/scratch/ecQHb (100%)
 */
s32 akao_get_xfer_state(void)
{
    return g_akao_spu_xfer_pending;
}

/**
 * @brief Clears the streaming-upload state (D_8004F824) and asserts the transfer-pending flag.
 *
 * @return 0 after the operation completes.
 *
 * @see https://decomp.me/scratch/qBE70 (100%)
 */
s32 akao_reset_xfer_state(void)
{
    D_8004F824 = 0;
    g_akao_driver_flags.unk0 |= 1;
    return 0;
}

/**
 * @brief Advances one tick of the AKAO bank-streaming upload state machine.
 *
 * Each tick is a single bounded copy step. The driver loads an instrument
 * bank in three stages - header, articulation table, sample blob - and the
 * caller drives this function repeatedly with whatever fresh bytes it has
 * read from disk so far.
 *
 * Stage 1 (first tick - @c g_akao_streaming_state.spu_addr == 0):
 *   - Magic-check @p source_address. On failure, zero the residual counters so the
 *     subsequent stages all short-circuit and the streaming-pending bit
 *     gets cleared at the bottom.
 *   - Copy the 0x40-byte AkaoBankHeader into @c g_akao_bank_staging and
 *     copied_bytes @p source_address/@p avail past it.
 *   - Seed @c g_akao_streaming_state from the staged header:
 *       @c spu_addr               = spu_dest_addr
 *       @c sample_remaining       = sample_size
 *       @c articulation_dst       = &g_akao_articulation_slots[bank_id * 0x10]
 *       @c articulation_remaining = articulation_count * 0x10
 *
 * Stage 2 (articulation copy):
 *   - memcpy up to @c articulation_remaining bytes of the source into the
 *     driver's articulation slot, advancing both pointers and shrinking
 *     the residual.
 *   - When the residual hits zero, rebase the articulation entries onto
 *     the SPU base via @c akao_relocate_articulations.
 *
 * Stage 3 (sample upload):
 *   - SpuSetTransferStartAddr(@c spu_addr), then akao_spu_write the source.
 *   - Advance @c spu_addr by the sample chunk size and shrink @c sample_remaining.
 *   - If @p wait_for_spu is non-zero, block on akao_spu_wait.
 *
 * Clears the pending bit when input remains after sample exhaustion, or when
 * the external status latch is zero. Articulation copies advance by whole words.
 *
 * @param source_address          Source byte pointer in main RAM. Starts at the AKAO
 *                     header on the first tick and advances through the
 *                     articulation and sample regions across subsequent
 *                     ticks.
 * @param avail        Number of fresh bytes available to consume this tick.
 * @param wait_for_spu Non-zero means block on @c akao_spu_wait after the SPU
 *                     write completes.
 *
 * @return @c D_8004F828 (the streaming-status latch read by callers).
 *
 * @see decomp.me (100%) https://decomp.me/scratch/0IPqT
 */
s32 akao_streaming_upload_tick(s32 source_address, u32 avail, s32 wait_for_spu)
{
    s32 copied_bytes;
    u32 articulation_chunk;
    u32 sample_chunk;
    AkaoArticulation* articulations;

    if ((g_akao_driver_flags.unk0 & 1) == 0)
    {
        return D_8004F828;
    }
    if (g_akao_streaming_state.spu_addr == 0)
    {
        if (akao_check_magic((AkaoHeader*)source_address) == 0)
        {
            akao_copy_bytes(*(&source_address), &g_akao_bank_staging, sizeof(AkaoBankHeader));
            source_address = (s32)((AkaoBankHeader*)source_address + 1);
            avail -= sizeof(AkaoBankHeader);
            g_akao_streaming_state.spu_addr = g_akao_bank_staging.spu_dest_addr;
            g_akao_streaming_state.sample_remaining = g_akao_bank_staging.sample_size;
            g_akao_streaming_state.articulation_dst = &((AkaoArticulation*)g_akao_articulation_slots)[g_akao_bank_staging.bank_id];
            g_akao_streaming_state.articulation_remaining = g_akao_bank_staging.articulation_count * sizeof(AkaoArticulation);
        }
        else
        {
            avail = 0;
            g_akao_streaming_state.sample_remaining = 0U;
            g_akao_streaming_state.articulation_remaining = 0U;
        }
    }
    if (g_akao_streaming_state.articulation_remaining != 0)
    {
        articulation_chunk = g_akao_streaming_state.articulation_remaining;
        if (avail != 0)
        {
            if (articulation_chunk >= avail)
            {
                articulation_chunk = avail;
            }
            akao_copy_bytes(source_address, g_akao_streaming_state.articulation_dst, articulation_chunk);
            copied_bytes = (articulation_chunk >> 2) * 4;
            source_address = (s32)((u8*)source_address + copied_bytes);
            avail -= articulation_chunk;
            g_akao_streaming_state.articulation_dst = g_akao_streaming_state.articulation_dst + copied_bytes;
            g_akao_streaming_state.articulation_remaining -= articulation_chunk;
            if (g_akao_streaming_state.articulation_remaining == 0)
            {
                articulations = &((AkaoArticulation*)g_akao_articulation_slots)[g_akao_bank_staging.bank_id];
                akao_relocate_articulations(articulations, articulations, g_akao_bank_staging.spu_dest_addr, g_akao_bank_staging.articulation_count);
            }
        }
    }
    if (avail != 0 && g_akao_streaming_state.sample_remaining == 0)
    {
        g_akao_driver_flags.unk0 &= ~1;
    }
    else
    {
        if (avail != 0)
        {
            sample_chunk = g_akao_streaming_state.sample_remaining;
            if (g_akao_streaming_state.sample_remaining >= avail)
            {
                sample_chunk = avail;
            }
            avail = sample_chunk;
            SpuSetTransferStartAddr(g_akao_streaming_state.spu_addr);
            akao_spu_write(source_address, avail);
            g_akao_streaming_state.spu_addr += avail;
            g_akao_streaming_state.sample_remaining -= avail;
            if (wait_for_spu != 0)
            {
                akao_spu_wait();
            }
        }
        if (D_8004F828 == 0)
        {
            g_akao_driver_flags.unk0 &= ~1;
        }
    }
    return D_8004F828;
}

/**
 * @brief Upload an AKAO instrument bank and return zero.
 * @param bank Pointer to an AKAO instrument bank in main RAM.
 * @param wait_for_completion Non-zero to wait for the SPU DMA to complete.
 * @return 0 after the upload call returns.
 *
 * @see https://decomp.me/scratch/0f3IK (100%)
 */
s32 akao_load_bank(AkaoBankHeader* bank, s32 wait_for_completion)
{
    akao_upload_bank_blocking(bank, wait_for_completion);
    return 0;
}

/**
 * @brief Routes an AKAO bank to one of six SPU base/slot pairs by @p slot, records the bank id, and uploads.
 *
 * @param bank AKAO instrument bank in RAM.
 * @param slot Slot 1 through 5; other values select slot 0.
 * @param wait_for_completion Non-zero to wait for the SPU transfer.
 * @return 0 after the operation completes.
 *
 * @see https://decomp.me/scratch/FWcdy (100%)
 */
s32 akao_upload_bank_slot(void* bank, s32 slot, s32 wait_for_completion)
{
    s32 articulation_index;
    s32* slot_key;
    u32 spu_base;
    AkaoBankIdentity* identity = bank;

    for (spu_base = 0, slot_key = g_akao_bank_slot_keys; spu_base < 6; spu_base++, slot_key++)
    {
        if (*slot_key == identity->key)
        {
            *slot_key = 0;
        }

    }

    switch (slot)
    {
    case 1:
        spu_base = 0x47900;
        articulation_index = 0x90;
        g_akao_bank_slot_keys[1] = identity->key;
        break;

    case 2:
        spu_base = 0x4C100;
        articulation_index = 0xA0;
        g_akao_bank_slot_keys[2] = identity->key;
        break;

    case 3:
        spu_base = 0x50900;
        articulation_index = 0xB0;
        g_akao_bank_slot_keys[3] = identity->key;
        break;

    case 4:
        spu_base = 0x55100;
        articulation_index = 0xC0;
        g_akao_bank_slot_keys[4] = identity->key;
        break;

    case 5:
        spu_base = 0x59900;
        articulation_index = 0xD0;
        g_akao_bank_slot_keys[5] = identity->key;
        break;

    default:
        spu_base = 0x43100;
        articulation_index = 0x80;
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
 * @see https://decomp.me/scratch/sa1fh (100%)
 */
s32 func_80022ED8(void* bank, s32 slot, s32 wait_for_completion)
{
    akao_upload_bank_slot(bank, slot, wait_for_completion);
    return 0;
}

/**
 * @brief Wrapper: biases @p slot by 3 before calling akao_upload_bank_slot
 *        (0 through 2 select slots 3 through 5; other values follow the callee
 *        fallback to slot 0 after the bias).
 * @param bank AKAO instrument bank in RAM.
 * @param slot Slot selector forwarded to akao_upload_bank_slot.
 * @param wait_for_completion Non-zero to wait for the SPU transfer.
 * @return 0 after the operation completes.
 *
 * @see https://decomp.me/scratch/PnDWc (100%)
 */
s32 func_80022EF8(void* bank, s32 slot, s32 wait_for_completion)
{
    akao_upload_bank_slot(bank, slot + 3, wait_for_completion);
    return 0;
}

/**
 * @brief Programs the CD/XA mix volume registers (@c CdMix on @c g_akao_cdmix).
 *
 * If bit 1 of @c D_8004F754 is set, all four CdlATV slots get
 * @c (volume * 0xB570) >> 0x11 - a 16-bit-fixed-point scale of @p volume across
 * a stereo pair. Otherwise only the two "main" slots get @p volume and the
 * "side" slots are zeroed. @p reserved is ignored here but participates in the
 * larger XA-streaming setup at the callers.
 *
 * @param volume  Target CD volume (0-127 expected).
 * @param reserved  Reserved / unused at this call site.
 *
 * @return 0 after the operation completes.
 *
 * @see https://decomp.me/scratch/hcfmi (100%)
 */
s32 akao_xa_setup_panning(s32 volume, void* reserved)
{
    if (D_8004F754 & 2)
    {
        g_akao_cdmix.val3 = (u32)(volume * 0xB570) >> 17;
        g_akao_cdmix.val1 = (u32)(volume * 0xB570) >> 17;
        g_akao_cdmix.val2 = (u32)(volume * 0xB570) >> 17;
        g_akao_cdmix.val0 = (u32)(volume * 0xB570) >> 17;
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
 * @brief AKAO command 0xE0 - magic-checks @p value0 (AKAO buffer) then dispatches with (buf*, 16-bit packed, c).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 8 bits are used.
 * @param value2 Value for command slot 2; semantics unknown.
 *
 * @see https://decomp.me/scratch/vw9QX (100%)
 */
void akao_cmd_e0(s32 value0, s32 value1, s32 value2)
{
    if (akao_check_magic((AkaoHeader*)value0) == 0)
    {
        g_akao_cmd_params[0].value = value0;
        g_akao_cmd_params[1].value = ((value1 & 0xFF) << 8);
        g_akao_cmd_params[2].value = value2;
        akao_send_command(AKAO_CMD_E0);
    }
}

/**
 * @brief AKAO command 0xE2 - zero-arg.
 *
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/kd4bK (100%)
 */
s32 akao_cmd_e2(void)
{
    return akao_send_command(AKAO_CMD_E2);
}

/**
 * @brief AKAO command 0xE4 - set the CD/XA channel mix volume.
 *
 * Packs the 7-bit volume (0-127) into the high byte of slot 0
 * (@c (value0 & 0x7F) << 8) per the AKAO 16-bit-packed-param convention,
 * then dispatches.
 *
 * @param value0  Target CD/XA volume (0-127).
 *
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/3oPkP (100%)
 */
s32 akao_cmd_e4_set_cd_volume(s32 value0)
{
    g_akao_cmd_params[0].value = (value0 & 0x7F) << 8;
    return akao_send_command(AKAO_CMD_E4_SET_CD_VOLUME);
}

/**
 * @brief AKAO command 0xE5 - (a, 7-bit value packed into <<8).
 *
 * @param value0 Value for command slot 0; semantics unknown.
 * @param value1 Value for command slot 1; only the low 7 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/7PxF8 (100%)
 */
s32 akao_cmd_e5(s32 value0, s32 value1)
{
    g_akao_cmd_params[0].value = value0;
    g_akao_cmd_params[1].value = ((value1 & 0x7F) << 8);
    return akao_send_command(AKAO_CMD_E5);
}

/**
 * @brief AKAO command 0xE6 - (8-bit value packed into <<8).
 *
 * @param value0 Value for command slot 0; only the low 8 bits are used.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/XeUon (100%)
 */
s32 akao_cmd_e6(s32 value0)
{
    g_akao_cmd_params[0].value = (value0 & 0xFF) << 8;
    return akao_send_command(AKAO_CMD_E6);
}

/**
 * @brief Magic-checks an AKAO XA program and stages it for the SPU.
 *
 * After verifying the AKAO magic, picks a hardcoded SPU base
 * (@c 0x50900 if @p upper_slot != 0, otherwise @c 0x43100) - and biases it by
 * @c 0xFFFD0000 when channel 0's @c flags & 0x40 is set with any in-flight
 * activity. Programs @c SpuSetTransferStartAddr, kicks off the sample upload
 * (akao_spu_write), caches the SPU base back into the buffer's
 * @c cached_spu_addr field, then memcpys the 0x50-byte header to the staging
 * area @c g_akao_xa_program_staging.
 *
 * @param buffer  Pointer to an AKAO buffer in main RAM.
 * @param upper_slot  Selects the upper SPU slot (non-zero) vs the lower slot.
 *
 * @return 0 on success; the akao_check_magic delta on failure (also clears
 *         @c g_akao_xa_program_staging.cached_spu_addr).
 *
 * @see https://decomp.me/scratch/C06sg (100%)
 */
s32 akao_upload_xa_program(void* buffer, s32 upper_slot)
{
    s32 result;
    s32 spu_base;
    AkaoBankHeader* bank;

    result = akao_check_magic(buffer);
    if (result == 0)
    {
        akao_spu_wait();
        spu_base = 0x50900;
        if (upper_slot == 0)
        {
            spu_base = 0x43100;
        }
        /* A song is loaded (running or suspended) AND its 0x40 state bit is set.
         * seq_cursor is the song-role flag word here, not a bytecode cursor. */
        if (((AKAO_CHANNEL_STATE->w04.song.active_mask | AKAO_CHANNEL_STATE->unk1C) != 0) && ((u32)AKAO_CHANNEL_STATE->seq_cursor & 0x40))
        {
            spu_base += 0xFFFD0000;
        }
        bank = buffer;
        buffer = bank + 1;
        SpuSetTransferStartAddr(spu_base);
        akao_spu_write(buffer, bank->spu_dest_addr);
        bank->cached_spu_addr = spu_base;
        akao_copy_bytes(bank, &g_akao_xa_program_staging, 0x50);
        return result;
    }

    g_akao_xa_program_staging.cached_spu_addr = 0;
    return result;
}

/**
 * @brief AKAO command 0xED - (8-bit value packed into <<8, b).
 *
 * @param value0 Value for command slot 0; only the low 8 bits are used.
 * @param value1 Value for command slot 1; semantics unknown.
 * @return Result returned by the AKAO command dispatcher.
 *
 * @see https://decomp.me/scratch/ULEGL (100%)
 */
s32 akao_cmd_ed(s32 value0, s32 value1)
{
    g_akao_cmd_params[0].value = ((value0 & 0xFF) << 8);
    g_akao_cmd_params[1].value = value1;
    return akao_send_command(AKAO_CMD_ED);
}

/**
 * @brief AKAO command 0xEC - magic-checked AKAO buffer with mode flags.
 *
 * Picks a hardcoded SPU base (@c 0x50900 if @p upper_slot != 0, else
 * @c 0x43100), biases by @c 0xFFFD0000 when channel 0 is active and busy,
 * then dispatches with (buf, 8-bit packed into <<8, spu_base, value3).
 *
 * @param buf        Pointer to an AKAO buffer in main RAM (validated via
 *                   akao_check_magic).
 * @param value1       8-bit value packed into bits 8..15 of slot 1. TODO:
 *                   meaning unknown.
 * @param upper_slot Selects the upper SPU slot (non-zero) vs the lower slot.
 * @param value3       Passed through verbatim into slot 3. TODO: meaning
 *                   unknown.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/SgcFo
 */
void akao_cmd_ec(void* buf, s32 value1, s32 upper_slot, s32 value3)
{
    s32 spu_base;

    if (akao_check_magic(buf) != 0)
    {
        return;
    }

    spu_base = upper_slot == 0 ? 0x43100 : 0x50900;

    if (((AKAO_CHANNEL_STATE->w04.song.active_mask | AKAO_CHANNEL_STATE->unk1C) != 0) && ((u32)AKAO_CHANNEL_STATE->seq_cursor & 0x40))
    {
        spu_base += 0xFFFD0000;
    }

    g_akao_cmd_params[0].buffer = buf;
    g_akao_cmd_params[1].value = ((value1 & 0xFF) << 8);
    g_akao_cmd_params[2].value = spu_base;
    g_akao_cmd_params[3].value = value3;
    akao_send_command(AKAO_CMD_EC);
}

/**
 * @brief AKAO command 0xE8 - begin XA-streamed AKAO playback.
 *
 * Validates @p byte_count != 0, disables SPU IRQ, primes the XA tracker
 * (@c g_akao_xa_tracker) for a stream of @c byte_count / 0x1000 frames, and dispatches.
 *
 * @param stream_id  Stream identifier / control word in slot 0.
 * @param byte_count  Total stream byte length (frame count = byte_count >> 12).
 *
 * @return 0 on success, -1 if @p byte_count is 0.
 *
 * @see https://decomp.me/scratch/bRIJX (100%)
 */
s32 akao_cmd_e8_start_xa_stream(s32 stream_id, u32 byte_count)
{
    if (byte_count == 0)
    {
        return -1;
    }
    SpuSetIRQ(0);
    SpuSetIRQAddr(0);
    g_akao_cmd_params[0].value = stream_id;
    g_akao_cmd_params[1].value = byte_count;
    g_akao_xa_tracker.unk34 = -1;
    g_akao_xa_tracker.unk20 = 0;
    g_akao_xa_tracker.unk24 = 0;
    g_akao_xa_tracker.unk28 = 0;
    g_akao_xa_tracker.unk38 = 0;
    g_akao_xa_tracker.unk3C = (s32)(byte_count >> 12);
    akao_send_command(AKAO_CMD_E8_START_XA_STREAM);
    return 0;
}

/**
 * @brief Advances one frame of an in-flight XA-streamed AKAO sequence.
 *
 * Increments @c g_akao_xa_tracker.unk24 (frame count) and the per-frame index
 * @c .unk38, wrapping at @c .unk3C - 1; once two frames have streamed and
 * bit 0x01000000 of @c .unk8 is set, calls @c func_8002E2E8 to refill the
 * SPU ring buffer.
 *
 * @return @c D_8004F794 (the streaming-status latch read by callers).
 *
 * @see https://decomp.me/scratch/gKZ5G (100%)
 */
s32 akao_xa_advance_frame(void)
{
    u32 next_frame;

    g_akao_xa_tracker.unk24 = g_akao_xa_tracker.unk24 + 1;
    next_frame = g_akao_xa_tracker.unk38 + 1;
    g_akao_xa_tracker.unk38 = next_frame;
    if ((u32)(g_akao_xa_tracker.unk3C - 1) < next_frame)
    {
        g_akao_xa_tracker.unk38 = 0;
    }
    if ((g_akao_xa_tracker.unk8 & 0x01000000) && ((u32)g_akao_xa_tracker.unk38 >= 2U))
    {
        func_8002E2E8(&g_akao_xa_tracker);
    }
    return D_8004F794;
}

/**
 * @brief Returns the current XA-stream position latch (@c D_8004F794).
 *
 * @return Current transfer or position latch.
 *
 * @see https://decomp.me/scratch/2DiS3 (100%)
 */
s32 akao_xa_get_position(void)
{
    return D_8004F794;
}
