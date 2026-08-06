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
 * @note decomp9.c declares @c AkaoChannelEffects over the same 0x118 bytes
 *       with its own (channel-role) names; decomp4.c additionally carries a
 *       set of per-opcode ad-hoc structs (@c a_struct, @c b_struct, ...) over
 *       the same layout. All of them should eventually collapse into this one.
 */

typedef struct AkaoChannelState
{
    /* --- 0x00..0x6F: meaning depends on the role (see the docblock) ------- */

    u32 flags;       /* 0x00 channel: bytecode cursor (u8*, read via a cast)
                             song:    flag word; bit 0x40 gates the akao_cmd.c
                                      "song is running" tests, cleared when the
                                      last channel is released */
    u32 unk4;        /* 0x04 channel: loop-start cursor slot 0
                             song:    active-channel bitmask (drives the tick
                                      loop in akao_seq_tick_channels) */
    u32 unk8;        /* 0x08 channel: loop-start cursor slot 1
                             song:    "allocate SPU voices from voice 0"
                                      channel mask - set by ext opcode FE 1D,
                                      cleared by FE 1E. func_80025500 passes
                                      (unk8 & bit) to the voice allocator,
                                      which then ignores unk38 (below). */
    u32 unkC;        /* 0x0C channel: loop-start cursor slot 2
                             song:    channel mask masked against the busy
                                      voices in the key-on path (func_800258B8) */
    u32 unk10;       /* 0x10 channel: loop-start cursor slot 3
                             song:    key-on request mask */
    u32 unk14;       /* 0x14 channel: subroutine return cursor (ext FE 0E/0F)
                             song:    sounding-channel mask */
    u32 unk18;       /* 0x18 channel: pointer to the selected key->articulation
                                      map (ext FE 14)
                             song:    key-off request mask, consumed and
                                      cleared by func_80025D98 */
    u32 unk1C;       /* 0x1C channel: pitch-LFO waveform cursor
                             song:    OR'd with unk4 as a "song still busy" test */
    u32 unk20;       /* 0x20 channel: volume-LFO waveform cursor
                             song:    tempo, Q16 (high half is the per-tick rate) */
    u32 unk24;       /* 0x24 channel: pan-LFO waveform cursor
                             song:    tempo-slide step per tick */
    u32 unk28;       /* 0x28 channel: secondary flag word; bit 0x02000000
                                      suppresses the pan-bias / volume-scale
                                      math in func_80024F60 and the SFX gate in
                                      akao_irq_handler
                             song:    tempo accumulator (wraps every 0x10000) */
    s32 pitch;       /* 0x2C channel: current SPU pitch (akao_compute_pitch result) */
    s32 unk30;       /* 0x30 channel: pitch-bend accumulator
                             song:    base of the articulation-map table used
                                      by ext FE 14 */
    u32 unk34;       /* 0x34 channel: main flag word. Known bits:
                                      0x01 pitch LFO on, 0x02 volume LFO on,
                                      0x04 pan LFO on, 0x08 drum/note-table mode,
                                      0x10/0x20 set by ops 0xD4..0xD7,
                                      0x40 full gate time, 0x800 pan bias active,
                                      0x1000 articulation map active,
                                      0x10000 SFX pitch-mod allowed,
                                      0x100000 set by op 0xE0,
                                      0x200000 "loop breaks on 0xCA",
                                      0x01000000 explicit ADSR attack */
    s32 unk38;       /* 0x38 channel: SFX articulation bank index (see
                                      akao_remap_sfx_articulation)
                             song:    first SPU voice the sequencer may
                                      allocate; set by ext FE 10, cleared by
                                      FE 11, read by func_80025498 */
    u32 reverb_mask; /* 0x3C song: channels enabled in the SPU reverb bitmap */
    u32 noise_mask;  /* 0x40 song: channels enabled in the SPU noise bitmap */
    u32 pitch_mod_mask; /* 0x44 song: channels enabled in the SPU pitch-mod bitmap */
    u32 unk48;       /* 0x48 channel: expression accumulator (value << 23)
                             song:    master-volume accumulator */
    u32 unk4C;       /* 0x4C channel: expression slide step
                             song:    master-volume slide step */
    s32 unk50;       /* 0x50 channel: pitch-slide (portamento) step */
    s32 unk54;       /* 0x54 channel: detune contribution to the SPU pitch */
    u16 unk58;       /* 0x58 channel: SFX tick counter - accessed as a u32 by
                                      the SFX path in akao_irq_handler */
    s16 unk5A;       /* 0x5A song: master-volume fade-tick countdown */
    u16 unk5C;       /* 0x5C channel: note-start expression preset, s32 at 0x5C
                             song:    tempo-slide fade-tick countdown */
    u16 unk5E;       /* 0x5E song: cleared when the last channel is released */
    u16 unk60;       /* 0x60 channel: note-start expression step, s32 at 0x60
                             song:    variable compared by the conditional jump
                                      opcode FE 07 */
    u16 unk62;       /* 0x62 song: SPU noise frequency (6 bits) */
    u16 unk64;       /* 0x64 channel: non-zero marks this slot as an SFX channel
                             song:    beats per measure (ext FE 15) */
    u16 unk66;       /* 0x66 channel: ticks left in the current note
                             song:    beat-within-measure counter */
    u16 unk68;       /* 0x68 channel: ticks left before the early key-off
                             song:    ticks per beat (ext FE 15) */
    u16 unk6A;       /* 0x6A channel: current articulation index
                             song:    tick-within-beat counter */
    u16 unk6C;       /* 0x6C song: measure counter (ext FE 16) */

    /* --- 0x6E..0x117: channel role only ----------------------------------- */

    u16 unk6E;       /* 0x6E pan bias added to the pan accumulator (ext FE 17/18) */
    u16 unk70;       /* 0x70 pan-bias fade-tick countdown */
    u16 unk72;       /* 0x72 opcodes executed; saved/restored by the loop ops */
    u16 unk74[4];    /* 0x74 iteration counter per loop-stack level */
    u16 unk7C[4];    /* 0x7C saved unk72 per loop-stack level */
    u16 unk84;       /* 0x84 channel volume accumulator (high byte is the level) */
    u16 unk86;       /* 0x86 volume fade-tick countdown */
    u16 unk88;       /* 0x88 zeroed when an SFX channel starts */
    u16 unk8A;       /* 0x8A expression fade-tick countdown */
    u16 unk8C;       /* 0x8C note-start expression fade length (ext FE 19) */
    u16 unk8E;       /* 0x8E zeroed when an SFX channel starts */
    u16 unk90;       /* 0x90 pan accumulator (high byte is the pan) */
    u16 unk92;       /* 0x92 pan fade-tick countdown */
    u16 unk94;       /* 0x94 pitch-slide ticks remaining */
    u16 unk96;       /* 0x96 current octave */
    u16 unk98;       /* 0x98 pitch-slide duration in ticks */
    u16 unk9A;       /* 0x9A previous note key (portamento source) */
    u16 unk9C;       /* 0x9C portamento speed; 0 disables portamento */
    u16 unk9E;       /* 0x9E note flags: 0x1 tie armed, 0x2 note is tied,
                             0x4 SFX full gate time */
    u16 unkA0;       /* 0xA0 */
    u16 unkA2;       /* 0xA2 pitch-LFO delay */
    u16 unkA4;       /* 0xA4 pitch-LFO delay countdown */
    u16 unkA6;       /* 0xA6 pitch-LFO period */
    u16 unkA8;       /* 0xA8 pitch-LFO restart flag (1 = waveform not started) */
    u16 unkAA;       /* 0xAA pitch-LFO waveform index into g_akao_lfo_waveforms */
    s16 unkAC;       /* 0xAC pitch-LFO depth scaled by the current pitch */
    u16 unkAE;       /* 0xAE pitch-LFO raw depth; bit 15 selects the scaling mode */
    u16 unkB0;       /* 0xB0 pitch-LFO depth-slide tick countdown */
    u16 unkB2;       /* 0xB2 pitch-LFO depth-slide step */
    u16 unkB4;       /* 0xB4 */
    u16 unkB6;       /* 0xB6 volume-LFO delay */
    u16 unkB8;       /* 0xB8 volume-LFO delay countdown */
    u16 unkBA;       /* 0xBA volume-LFO period */
    u16 unkBC;       /* 0xBC volume-LFO restart flag */
    u16 unkBE;       /* 0xBE volume-LFO waveform index */
    u16 unkC0;       /* 0xC0 volume-LFO depth */
    u16 unkC2;       /* 0xC2 volume-LFO depth-slide tick countdown */
    u16 unkC4;       /* 0xC4 volume-LFO depth-slide step */
    u16 unkC6;       /* 0xC6 */
    u16 unkC8;       /* 0xC8 pan-LFO period */
    u16 unkCA;       /* 0xCA pan-LFO restart flag */
    u16 unkCC;       /* 0xCC pan-LFO waveform index */
    u16 unkCE;       /* 0xCE pan-LFO depth */
    u16 unkD0;       /* 0xD0 pan-LFO depth-slide tick countdown */
    u16 unkD2;       /* 0xD2 pan-LFO depth-slide step */
    u16 unkD4;       /* 0xD4 ticks until the reverb enable bit is toggled back */
    u16 unkD6;       /* 0xD6 ticks until the pitch-mod enable bit is toggled back */
    u16 unkD8;       /* 0xD8 loop-stack index (0..3, wraps) */
    u16 unkDA;       /* 0xDA pitch scalar applied to the computed pitch (Q8) */
    u16 unkDC;       /* 0xDC last note duration set by opcode 0xA2 */
    u16 unkDE;       /* 0xDE note-duration adjustment (opcode 0xDC) */
    u16 unkE0;       /* 0xE0 volume slide step */
    s16 unkE2;       /* 0xE2 pan-bias slide step */
    u16 unkE4;       /* 0xE4 extra volume scale; high byte is a signed Q7 factor */
    u16 unkE6;       /* 0xE6 */
    u16 unkE8;       /* 0xE8 pan slide step */
    u16 unkEA;       /* 0xEA transpose in semitones (opcodes 0xC0/0xC1) */
    s16 unkEC;       /* 0xEC fine detune (opcodes 0xD8/0xD9) */
    u16 unkEE;       /* 0xEE key of the note currently sounding */
    u16 unkF0;       /* 0xF0 pending pitch-slide delta in semitones */
    s16 unkF2;       /* 0xF2 transpose in effect for the previous note */
    u16 unkF4;       /* 0xF4 current pitch-LFO output */
    u16 unkF6;       /* 0xF6 current volume-LFO output */
    s16 unkF8;       /* 0xF8 current pan-LFO output */
    u16 unkFA;       /* 0xFA */
    u32 unkFC;       /* 0xFC assigned SPU voice index (0x18 = none) */
    s32 unk100;      /* 0x100 pending SPU register update flags */
    s32 spu_sample_addr; /* 0x104 SPU sample start */
    s32 spu_loop_addr;   /* 0x108 SPU loop point */
    u16 spu_pitch;   /* 0x10C SPU pitch/sample-rate register image */
    u16 unk10E;      /* 0x10E SPU ADSR low halfword */
    u16 unk110;      /* 0x110 SPU ADSR high halfword */
    u16 spu_volume_scale; /* 0x112 optional Q7 scale */
    s16 spu_volume_left;  /* 0x114 */
    s16 spu_volume_right; /* 0x116 */
} AkaoChannelState; /* sizeof = 0x118 */

#endif
