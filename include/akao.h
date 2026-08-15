#ifndef _AKAO_H
#define _AKAO_H

#include "common.h"

/**
 * @brief CPU→AKAO-driver command opcodes.
 *
 * The host CPU pokes a one-byte opcode into akao_send_command (which indexes
 * a 256-entry MESSAGE_HANDLERS dispatch table) after staging up to four
 * 32-bit parameters in @c g_akaoCmdParams. Only the opcodes LOM actually
 * issues are listed here.
 *
 * @note These are *driver-command* opcodes, not the in-sequence channel
 *       opcodes documented for FF7 — the byte numbering does not align
 *       between the two tables.
 *
 * Names follow the convention @c AKAO_CMD_<HEX> with a descriptive suffix
 * only where the LOM call shape makes the meaning unambiguous; otherwise
 * the hex byte stands on its own and the precise semantics are TBD.
 *
 * Where a wrapper masks a parameter to a specific bit width, the inferred
 * meaning of that slot follows AKAO conventions:
 *  - 7-bit  ⇒ volume / pan envelope value (0–127)
 *  - 8-bit  ⇒ byte parameter (often a duration in ticks, or a count)
 *  - 10-bit ⇒ sound or sequence id (0–1023)
 *  - 24-bit ⇒ wide param (frequency / pitch / fade duration)
 *  - <<8    ⇒ 16-bit value packed into the high byte of a word
 *
 * The 0xA0..0xAD block follows a clean per-channel / master-global pairing:
 * 0xA0..0xA5 take a leading channel index, 0xA8..0xAD apply the same effect
 * globally. Within that block, the byte/7-bit/8-bit width pattern hints at
 * the effect category (volume vs pan vs pitch).
 */
typedef enum AkaoCmd
{
    AKAO_CMD_PLAY_SONG          = 0x10, /**< play sequence; slot 0 = AKAO buffer pointer    */
    AKAO_CMD_STOP_SONG          = 0x11, /**< stop active sequence (callers pass 0)          */
    AKAO_CMD_12                 = 0x12, /**< (a, b) — both unmasked; semantics TBD          */
    AKAO_CMD_14                 = 0x14, /**< (a, b, 0) — third slot forced 0; semantics TBD */
    AKAO_CMD_19                 = 0x19, /**< (a) — issued just before 0xC0 in akao_cmd_19_c0 */
    AKAO_CMD_PLAY_SFX           = 0x20, /**< (id10, p24, p8, vol7) — sound id + 24-bit param + byte param + volume */
    AKAO_CMD_21                 = 0x21, /**< (id, p24) — sound id + 24-bit param            */
    AKAO_CMD_PLAY_SFX_FROM_BUF  = 0x24, /**< (buf*, p24, p8, vol7) — magic-checked AKAO buffer + same shape as 0x20 */
    AKAO_CMD_STOP_SFX_BY_ID     = 0x30, /**< (id10) — stop SFX matching the 10-bit sound id */
    AKAO_CMD_GLOBAL_STOP        = 0x40, /**< zero-arg "silence everything"                  */
    AKAO_CMD_PAUSE              = 0x80, /**< zero-arg — pause active sequence               */
    AKAO_CMD_RESUME             = 0x81, /**< zero-arg — resume paused sequence              */
    AKAO_CMD_90                 = 0x90, /**< (a) — semantics TBD                            */
    AKAO_CMD_92                 = 0x92, /**< (a) — semantics TBD                            */
    AKAO_CMD_98                 = 0x98, /**< zero-arg; selected by akao_cmd_98_9a_9c_9e (default branch) */
    AKAO_CMD_99                 = 0x99, /**< zero-arg; selected by akao_cmd_99_9b_9d_9f (default branch) */
    AKAO_CMD_9A                 = 0x9A, /**< zero-arg; akao_cmd_98_9a_9c_9e branch 1        */
    AKAO_CMD_9B                 = 0x9B, /**< zero-arg; akao_cmd_99_9b_9d_9f branch 1        */
    AKAO_CMD_9C                 = 0x9C, /**< zero-arg; akao_cmd_98_9a_9c_9e branch 2        */
    AKAO_CMD_9D                 = 0x9D, /**< zero-arg; akao_cmd_99_9b_9d_9f branch 2        */
    AKAO_CMD_9E                 = 0x9E, /**< zero-arg; akao_cmd_98_9a_9c_9e branch 3        */
    AKAO_CMD_9F                 = 0x9F, /**< zero-arg; akao_cmd_99_9b_9d_9f branch 3        */
    AKAO_CMD_A0                 = 0xA0, /**< per-channel: (ch, fade24, target_vol7)         */
    AKAO_CMD_A1                 = 0xA1, /**< per-channel: (ch, fade24, p, target_vol7)      */
    AKAO_CMD_A2                 = 0xA2, /**< per-channel: (ch, fade24, target_pan8)         */
    AKAO_CMD_A3                 = 0xA3, /**< per-channel: (ch, fade24, p, target_pan8)      */
    AKAO_CMD_A4                 = 0xA4, /**< per-channel: (ch, fade24, target_byte8)        */
    AKAO_CMD_A5                 = 0xA5, /**< per-channel: (ch, fade24, p, target_byte8)     */
    AKAO_CMD_A8                 = 0xA8, /**< master/global: (vol7) — global vol counterpart of 0xA0 */
    AKAO_CMD_A9                 = 0xA9, /**< master/global: (a, vol7) — counterpart of 0xA1 */
    AKAO_CMD_AA                 = 0xAA, /**< master/global: (pan8) — global pan counterpart of 0xA2 */
    AKAO_CMD_AB                 = 0xAB, /**< master/global: (a, pan8) — counterpart of 0xA3 */
    AKAO_CMD_AC                 = 0xAC, /**< master/global: (byte8) — counterpart of 0xA4   */
    AKAO_CMD_AD                 = 0xAD, /**< master/global: (a, byte8) — counterpart of 0xA5 */
    AKAO_CMD_C0                 = 0xC0, /**< (a, vol7) — possibly per-channel transpose / pitch-bend */
    AKAO_CMD_C1                 = 0xC1, /**< (a, b, vol7) — 0xC0 with extra parameter        */
    AKAO_CMD_C2                 = 0xC2, /**< (a, b, vol7, vol7) — 0xC0 with two trailing 7-bit values */
    AKAO_CMD_C8                 = 0xC8, /**< (a) — unmasked single arg; semantics TBD       */
    AKAO_CMD_C9                 = 0xC9, /**< (a, b) — unmasked args; semantics TBD          */
    AKAO_CMD_CA                 = 0xCA, /**< (a, b, c) — unmasked args; semantics TBD       */
    AKAO_CMD_D0                 = 0xD0, /**< (byte8)                                        */
    AKAO_CMD_D1                 = 0xD1, /**< (a, byte8)                                     */
    AKAO_CMD_D2                 = 0xD2, /**< (a, byte8, byte8)                              */
    AKAO_CMD_D4                 = 0xD4, /**< (byte8)                                        */
    AKAO_CMD_D5                 = 0xD5, /**< (a, byte8)                                     */
    AKAO_CMD_D6                 = 0xD6, /**< (a, byte8, byte8)                              */
    AKAO_CMD_D8                 = 0xD8, /**< (byte8)                                        */
    AKAO_CMD_D9                 = 0xD9, /**< (a, byte8)                                     */
    AKAO_CMD_DA                 = 0xDA, /**< (a, byte8, byte8)                              */
    AKAO_CMD_E0                 = 0xE0, /**< (buf*, packed16, c) — magic-checked AKAO buffer + 16-bit packed param */
    AKAO_CMD_E2                 = 0xE2, /**< zero-arg                                       */
    AKAO_CMD_E4_SET_CD_VOLUME   = 0xE4, /**< (vol_packed16) — CD/XA channel mix volume      */
    AKAO_CMD_E5                 = 0xE5, /**< (a, vol_packed16) — 7-bit value packed into <<8 */
    AKAO_CMD_E6                 = 0xE6, /**< (val_packed16) — 8-bit value packed into <<8   */
    AKAO_CMD_E8_START_XA_STREAM = 0xE8, /**< begin XA-streamed AKAO playback                */
    AKAO_CMD_EC                 = 0xEC, /**< (buf*, packed16, var, d) — magic-checked AKAO buffer with mode flags */
    AKAO_CMD_ED                 = 0xED, /**< (val_packed16, b) — 8-bit value packed into <<8 + extra arg */
    AKAO_CMD_F0                 = 0xF0, /**< zero-arg query; return value consumed by caller */
    AKAO_CMD_F1                 = 0xF1  /**< zero-arg query; return value consumed by caller */
} AkaoCmd;

/**
 * Creation time embedded in every AKAO file header.
 * All fields are binary-coded decimal (BCD).
 */
typedef struct AkaoTimeStamp
{
    u8 year_bcd;
    u8 month_bcd;
    u8 day_bcd;
    u8 hours_bcd;
    u8 minutes_bcd;
    u8 seconds_bcd;
} AkaoTimeStamp;

/**
 * 16-byte common header that prefixes every AKAO-tagged audio resource
 * (sequences and sample/instrument sets alike). Validated via akao_check_magic
 * before the payload that follows is handed to the driver.
 *
 * The first four bytes are always the ASCII string "AKAO" (0x4F414B41 LE).
 *
 * Field semantics derive from the FF7 decomp project; LOM uses the same
 * common header but the post-header payload layout differs by file type:
 * for instrument banks the next 16 bytes are AkaoBankHeader's bank-specific
 * fields; for sequences it is a channel-opcode dispatch table.
 *
 * @note Despite the "Seq" suffix, this struct is the *common* AKAO header,
 *       not specific to sequences. AkaoBankHeader embeds it as its first
 *       member.
 */
typedef struct AkaoSeqHeader
{
    u32 magic;       /* "AKAO" in little-endian = 0x4F414B41 */
    u16 id;
    u16 length;
    u16 reverb_type;
    AkaoTimeStamp timestamp;
} AkaoSeqHeader;

/**
 * 64-byte (0x40) instrument-bank header. Begins every AKAO-tagged
 * sample/instrument file (e.g. EFFECT.SET fragments).
 *
 * Layout:
 *   0x00..0x0F  AkaoSeqHeader (common AKAO magic + metadata)
 *   0x10        spu_dest_addr        — SPU base address for sample upload
 *   0x14        sample_size          — byte count of the sample blob
 *   0x18        bank_id              — instrument-bank slot index
 *                                      (multiplied by 0x10 to index the
 *                                      driver's articulation slot table)
 *   0x1C        articulation_count   — number of 16-byte articulation entries
 *                                      stored at offset 0x40
 *   0x20        cached_spu_addr      — SPU base address cached by
 *                                      akao_upload_xa_program (cmd 0xEC)
 *                                      after a streaming upload starts
 *   0x24..0x3F  reserved             — unused / unknown
 *
 * The articulation table starts at offset 0x40 and is articulation_count
 * entries long (16 bytes per entry); the sample blob follows the
 * articulation table. akao_upload_bank uploads the sample blob to the SPU
 * and akao_relocate_articulations rebases the articulation entries onto
 * the SPU base before installing them in the driver's instrument slot.
 */
typedef struct AkaoBankHeader
{
    AkaoSeqHeader header;
    u32 spu_dest_addr;
    u32 sample_size;
    u32 bank_id;
    u32 articulation_count;
    u32 cached_spu_addr;
    u8  reserved[0x1C];
} AkaoBankHeader;

/**
 * @brief One entry in an AKAO instrument-bank articulation table.
 *
 * Each AkaoBankHeader is followed (starting at file offset 0x40) by
 * @c articulation_count of these 16-byte entries, one per SPU voice setup
 * the bank exposes. They are copied into the driver's articulation slot
 * table @c g_akao_articulation_slots by akao_relocate_articulations, which
 * biases @c sample_addr and @c loop_addr by the SPU upload base so the
 * in-RAM table holds absolute SPU offsets. The trailing two words are
 * pre-baked SPU voice parameters and are copied verbatim.
 *
 * Field semantics for words 0x08 and 0x0C are inferred from the PSX SPU
 * voice-register layout and the matched relocation code; they likely
 * encode ADSR envelope and pitch/voice flags but exact bit-packings are
 * not yet confirmed against the AKAO source. Treat @c adsr and @c pitch_misc
 * as opaque u32 blobs until verified.
 */
typedef struct AkaoArticulation
{
    u32 sample_addr; /* 0x00: SPU sample start - biased by spu_base on upload */
    u32 loop_addr;   /* 0x04: SPU loop point   - biased by spu_base on upload */
    union {
        u32 word;     /* 0x08: full 32-bit ADSR envelope (TODO: bit layout)   */
        struct {
            s16 lo;   /* 0x08: signed low half  (fine-tune cents in pitch calc) */
            s16 hi;   /* 0x0A: signed high half (transpose semitones in pitch calc) */
        } half;
    } adsr;
    union {
        u32 word;     /* 0x0C: pitch / voice flags (TODO: bit layout)         */
        struct {
            u16 lo;   /* 0x0C: low halfword  - masked against 0x80FF on note bind */
            u16 hi;   /* 0x0E: high halfword - masked against 0x0020 on note bind */
        } half;
    } pitch_misc;
} AkaoArticulation; /* sizeof = 0x10 */

/**
 * @brief One 0x118-byte AKAO driver state slot.
 *
 * This type is currently used for two structurally different blocks, which is
 * why so many fields are still @c unkNN:
 *
 * - <b>Channel role</b>: each entry of @c g_akao_seq_channels, each entry of
 *   the pending set at @c g_akao_pending_channels, and each entry of
 *   @c g_sfx_channels. This is a sequencer track: bytecode cursor, loop stack,
 *   volume/pan/expression envelopes, three LFOs, and the pending SPU voice
 *   register image.
 * - <b>Song role</b>: the block reached through @c g_akao_seq_channel0 (which
 *   aliases @c g_akao_seq_master_state, or @c g_akao_seq_channel1 while the
 *   secondary set is being ticked). This is the per-song master state: tempo,
 *   master volume, bar/beat counters and the driver's per-channel bitmasks.
 *   Only the first 0x70 bytes are meaningful - akao_irq_handler promotes the
 *   pending song with @c akao_copy_bytes(ch1, ch0, 0x70).
 *
 * The two roles assign completely different meanings to 0x00..0x6F, so those
 * fields deliberately keep neutral @c unkNN names and both meanings are listed
 * in the comment. Everything from 0x6E up is channel-only and unambiguous.
 * Splitting this into @c AkaoChannelState and an @c AkaoSongState is the
 * obvious cleanup; it is not done yet because it touches every matched
 * function in decomp4.c.
 *
 * @note This is now the single type for the block. It replaced 47 per-opcode
 *       ad-hoc structs in decomp4.c, @c SfxChannel and @c AkaoSFXState in
 *       decomp4.h, and @c AkaoChannelEffects in decomp9.c.
 *
 * @note Where the two roles disagree on a field's width or signedness the
 *       declaration follows whichever role reads the field, and the other
 *       role's site carries an explicit cast with a comment. The known cases
 *       are 0x2C (read as a halfword by the pitch-LFO op), 0x5C/0x60 (an s32
 *       expression envelope overlapping two song halfwords), 0x58 (a u32 SFX
 *       tick counter overlapping a song halfword) and 0x1C/0x20/0x24 (LFO
 *       waveform cursors overlapping the tempo words).
 */

typedef struct AkaoChannelState
{
    /* --- 0x00..0x6F: meaning depends on the role (see the docblock) ------- */

    u8* seq_cursor;  /* 0x00 channel: bytecode cursor (a u8*, read via a cast)
                             song:    flag word. func_80026254 clears 0x63 and
                                      then sets 0x20 or 0x40 depending on the
                                      song descriptor; bit 0x40 gates the
                                      akao_cmd.c "song is running" tests, and
                                      akao_process_sequence_voice_updates
                                      sets 0x1/0x2 when the voice
                                      allocator comes up empty. The whole word
                                      is zeroed when the last channel is
                                      released. */
    union
    {
        /* channel: the loop-start cursor stack, one entry per nesting level.
         * Declared as a real array member so that a variable-index access
         * builds the same address expression as the original code. */
        u8* loop_cursor[4];

        /* song: four independent per-channel bitmasks. */
        struct
        {
            u32 active_mask;          /* 0x04 channels currently being ticked;
                                              drives the loop in
                                              akao_seq_tick_channels, and
                                              reaching 0 ends the song */
            u32 voice_alloc_low_mask; /* 0x08 channels exempt from the reserved
                                              voice floor. Seeded from song
                                              descriptor +0x24, set by ext
                                              opcode FE 1D
                                              (akao_seq_op_ignore_voice_reserve),
                                              cleared by FE 1E.
                                              akao_process_sequence_voice_updates
                                              passes (mask & bit) to
                                              akao_find_free_voice, which then scans
                                              for a free voice from 0 instead
                                              of from voice_alloc_base. */
            u32 static_voice_mask;    /* 0x0C channels that skip voice
                                              allocation and always play on the
                                              SPU voice matching their own
                                              channel index. Seeded from song
                                              descriptor +0x28; honoured by
                                              akao_process_sequence_voice_updates
                                              only while that
                                              voice is not held by SFX or XA. */
            u32 key_on_mask;          /* 0x10 channels whose note still needs a
                                              voice keyed on; consumed and
                                              cleared by the key-on pass in
                                              akao_flush_voice_updates */
        } song;
    } w04;               /* 0x04 - 0x13 */
    u32 note_on_mask;/* 0x14 song:    channels with a note currently sounding;
                                      set once the pitch is computed, cleared
                                      at note-off
                             channel: subroutine return cursor (ext FE 0E/0F) */
    u32 key_off_mask;/* 0x18 song:    channels whose voice still needs a key-off;
                                      consumed and cleared by func_80025D98,
                                      which folds them into spu_set_key_off
                             channel: pointer to the selected key-to-articulation
                                      map (ext FE 14) */
    s32 unk1C;       /* 0x1C song:    parked copy of active_mask. When
                                      g_akao_driver_mode_flags bit 0 is set the
                                      song's channels are moved here and
                                      active_mask is zeroed, so the song stops
                                      ticking but still counts as loaded
                                      ((active_mask | unk1C) != 0).
                             channel: pitch-LFO waveform cursor
                             Left unnamed: the two roles are used about equally
                             often, so either name misreads at the other's
                             call sites. */
    u32 tempo;       /* 0x20 song:    tempo, Q16; the high half is added to
                                      tempo_acc once per driver tick
                             channel: volume-LFO waveform cursor */
    s32 tempo_step;  /* 0x24 song:    tempo-slide step per tick
                             channel: pan-LFO waveform cursor */
    u32 tempo_acc;   /* 0x28 song:    tempo accumulator; a carry out of the low
                                      16 bits advances one musical tick
                             channel: secondary flag word. Bit 0x02000000
                                      suppresses the pan-bias / volume-scale
                                      math in akao_update_sfx_channel_voice
                                      and the SFX gate in
                                      akao_irq_handler. */
    s32 pitch;       /* 0x2C channel: current SPU pitch (akao_compute_pitch result) */
    s32 unk30;       /* 0x30 channel: pitch-bend accumulator
                             song:    base of the articulation-map table used
                                      by ext FE 14 */
    s32 flags;       /* 0x34 song:    pointer to the note/articulation table
                                      used by akao_channel_start_note; only
                                      tested for non-zero elsewhere
                             channel: main flag word. Known bits:
                                      0x01 pitch LFO on, 0x02 volume LFO on,
                                      0x04 pan LFO on, 0x08 drum/note-table mode,
                                      0x10/0x20 set by ops 0xD4..0xD7,
                                      0x40 full gate time, 0x800 pan bias active,
                                      0x1000 articulation map active,
                                      0x10000 SFX pitch-mod allowed,
                                      0x100000 set by op 0xE0,
                                      0x200000 "loop breaks on 0xCA",
                                      0x01000000 explicit ADSR attack */
    s32 voice_alloc_base;
                     /* 0x38 song:    first SPU voice the sequencer may
                                      allocate; set by ext FE 10, cleared by
                                      FE 11, read by akao_find_free_voice
                             channel: SFX articulation bank index (see
                                      akao_remap_sfx_articulation) */
    u32 reverb_mask; /* 0x3C song: channels enabled in the SPU reverb bitmap */
    u32 noise_mask;  /* 0x40 song: channels enabled in the SPU noise bitmap */
    u32 pitch_mod_mask; /* 0x44 song: channels enabled in the SPU pitch-mod bitmap */
    s32 unk48;       /* 0x48 channel: expression accumulator (value << 23)
                             song:    master-volume accumulator */
    s32 unk4C;       /* 0x4C channel: expression slide step
                             song:    master-volume slide step */
    s32 pitch_slide_step; /* 0x50 channel: pitch-slide (portamento) step */
    s32 detune_pitch_delta; /* 0x54 channel: detune contribution to the SPU pitch */
    u16 unk58;       /* 0x58 channel: SFX tick counter - accessed as a u32 by
                                      the SFX path in akao_irq_handler */
    s16 master_vol_fade_ticks; /* 0x5A song: master-volume fade-tick countdown */
    u16 tempo_fade_ticks;
                     /* 0x5C song:    tempo-slide fade-tick countdown
                             channel: note-start expression preset, read as an
                                      s32 spanning 0x5C..0x5F */
    u16 unk5E;       /* 0x5E song: cleared when the last channel is released */
    u16 unk60;       /* 0x60 channel: note-start expression step, s32 at 0x60
                             song:    variable compared by the conditional jump
                                      opcode FE 07 */
    u16 noise_freq;  /* 0x62 song: SPU noise frequency (6 bits) */
    u16 is_sfx_channel;
                     /* 0x64 channel: non-zero marks this slot as an SFX channel;
                                      selects SFX vs sequence routing all over
                                      decomp4.c
                             song:    beats per measure (ext FE 15) */
    u16 unk66;       /* 0x66 channel: ticks left in the current note
                             song:    beat-within-measure counter */
    u16 unk68;       /* 0x68 channel: ticks left before the early key-off
                             song:    ticks per beat (ext FE 15) */
    u16 unk6A;       /* 0x6A channel: current articulation index
                             song:    tick-within-beat counter */
    u16 measure;     /* 0x6C song: measure counter (ext FE 16) */

    /* --- 0x6E..0x117: channel role only ----------------------------------- */

    u16 pan_bias;    /* 0x6E pan bias added to the pan accumulator (ext FE 17/18) */
    u16 pan_bias_fade_ticks; /* 0x70 pan-bias fade-tick countdown */
    u16 opcode_count; /* 0x72 opcodes executed; saved/restored by the loop ops */
    u16 loop_count[4]; /* 0x74 iteration counter per loop-stack level */
    u16 loop_opcode_count[4]; /* 0x7C saved unk72 per loop-stack level */
    u16 volume;      /* 0x84 channel volume accumulator (high byte is the level) */
    u16 volume_fade_ticks; /* 0x86 volume fade-tick countdown */
    u16 unk88;       /* 0x88 zeroed when an SFX channel starts */
    u16 expression_fade_ticks; /* 0x8A expression fade-tick countdown */
    u16 note_expression_ticks; /* 0x8C note-start expression fade length (ext FE 19) */
    u16 unk8E;       /* 0x8E zeroed when an SFX channel starts */
    u16 pan;         /* 0x90 pan accumulator (high byte is the pan) */
    u16 pan_fade_ticks; /* 0x92 pan fade-tick countdown */
    u16 pitch_slide_ticks; /* 0x94 pitch-slide ticks remaining */
    u16 octave;      /* 0x96 current octave */
    u16 pitch_slide_duration; /* 0x98 pitch-slide duration in ticks */
    u16 prev_key;    /* 0x9A previous note key (portamento source) */
    u16 portamento_speed; /* 0x9C portamento speed; 0 disables portamento */
    u16 note_flags;  /* 0x9E note flags: 0x1 tie armed, 0x2 note is tied,
                             0x4 SFX full gate time */
    u16 unkA0;       /* 0xA0 */
    u16 pitch_lfo_delay; /* 0xA2 pitch-LFO delay */
    u16 pitch_lfo_delay_ticks; /* 0xA4 pitch-LFO delay countdown */
    s16 pitch_lfo_period; /* 0xA6 pitch-LFO period */
    u16 pitch_lfo_restart; /* 0xA8 pitch-LFO restart flag (1 = waveform not started) */
    u16 pitch_lfo_waveform; /* 0xAA pitch-LFO waveform index into g_akao_lfo_waveforms */
    u16 pitch_lfo_depth_scaled; /* 0xAC pitch-LFO depth scaled by the current pitch */
    u16 pitch_lfo_depth; /* 0xAE pitch-LFO raw depth; bit 15 selects the scaling mode */
    u16 pitch_lfo_depth_fade_ticks; /* 0xB0 pitch-LFO depth-slide tick countdown */
    u16 pitch_lfo_depth_step; /* 0xB2 pitch-LFO depth-slide step */
    u16 unkB4;       /* 0xB4 */
    u16 volume_lfo_delay; /* 0xB6 volume-LFO delay */
    u16 volume_lfo_delay_ticks; /* 0xB8 volume-LFO delay countdown */
    s16 volume_lfo_period; /* 0xBA volume-LFO period */
    u16 volume_lfo_restart; /* 0xBC volume-LFO restart flag */
    u16 volume_lfo_waveform; /* 0xBE volume-LFO waveform index */
    u16 volume_lfo_depth; /* 0xC0 volume-LFO depth */
    u16 volume_lfo_depth_fade_ticks; /* 0xC2 volume-LFO depth-slide tick countdown */
    u16 volume_lfo_depth_step; /* 0xC4 volume-LFO depth-slide step */
    u16 unkC6;       /* 0xC6 */
    u16 pan_lfo_period; /* 0xC8 pan-LFO period */
    u16 pan_lfo_restart; /* 0xCA pan-LFO restart flag */
    u16 pan_lfo_waveform; /* 0xCC pan-LFO waveform index */
    u16 pan_lfo_depth; /* 0xCE pan-LFO depth */
    u16 pan_lfo_depth_fade_ticks; /* 0xD0 pan-LFO depth-slide tick countdown */
    u16 pan_lfo_depth_step; /* 0xD2 pan-LFO depth-slide step */
    u16 reverb_toggle_ticks; /* 0xD4 ticks until the reverb enable bit is toggled back */
    u16 pitch_mod_toggle_ticks; /* 0xD6 ticks until the pitch-mod enable bit is toggled back */
    u16 loop_depth;  /* 0xD8 loop-stack index (0..3, wraps) */
    u16 pitch_scale; /* 0xDA pitch scalar applied to the computed pitch (Q8) */
    s16 note_duration; /* 0xDC last note duration set by opcode 0xA2 */
    u16 note_duration_adjust; /* 0xDE note-duration adjustment (opcode 0xDC) */
    s16 volume_step; /* 0xE0 volume slide step */
    s16 pan_bias_step; /* 0xE2 pan-bias slide step */
    u16 volume_scale; /* 0xE4 extra volume scale; high byte is a signed Q7 factor */
    u16 unkE6;       /* 0xE6 */
    s16 pan_step;    /* 0xE8 pan slide step */
    u16 transpose;   /* 0xEA transpose in semitones (opcodes 0xC0/0xC1) */
    s16 detune;      /* 0xEC fine detune (opcodes 0xD8/0xD9) */
    u16 note_key;    /* 0xEE key of the note currently sounding */
    u16 pitch_slide_delta; /* 0xF0 pending pitch-slide delta in semitones */
    s16 prev_transpose; /* 0xF2 transpose in effect for the previous note */
    s16 pitch_lfo_value; /* 0xF4 current pitch-LFO output */
    s16 volume_lfo_value; /* 0xF6 current volume-LFO output */
    s16 pan_lfo_value; /* 0xF8 current pan-LFO output */
    u16 unkFA;       /* 0xFA */
    u32 voice;       /* 0xFC assigned SPU voice index (0x18 = none) */
    s32 update_flags; /* 0x100 pending SPU register update flags */
    s32 spu_sample_addr; /* 0x104 SPU sample start */
    s32 spu_loop_addr;   /* 0x108 SPU loop point */
    u16 spu_pitch;   /* 0x10C SPU pitch/sample-rate register image */
    u16 spu_adsr_low; /* 0x10E SPU ADSR low halfword */
    u16 spu_adsr_high; /* 0x110 SPU ADSR high halfword */
    u16 spu_volume_scale; /* 0x112 optional Q7 scale */
    s16 spu_volume_left;  /* 0x114 */
    s16 spu_volume_right; /* 0x116 */
} AkaoChannelState; /* sizeof = 0x118 */

#endif
