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
 * @brief Per-channel runtime state for the AKAO driver.
 *
 * Used for the song channel at @c g_akao_seq_channel0, the secondary slot at
 * @c g_akao_seq_channel1, the backing storage at @c g_akao_seq_master_state,
 * each entry of the per-channel array @c g_akao_seq_channels, and each entry
 * of @c g_sfx_channels. Each slot is 0x118 bytes wide. @c g_akao_seq_channel0
 * is set in akao_driver_init_state to alias the master-state slot and is
 * swapped to @c g_akao_seq_channel1 during per-frame seq processing.
 */

typedef struct AkaoChannelState
{
    u32 flags;       /* 0x00 */
    u32 unk4;        /* 0x04 */
    u32 unk8;        /* 0x08 */
    u32 unkC;        /* 0x0C */
    u32 unk10;       /* 0x10 */
    u32 unk14;       /* 0x14 */
    u32 unk18;       /* 0x18 */
    u32 unk1C;       /* 0x1C */
    u32 unk20;       /* 0x20 */
    u32 unk24;       /* 0x24 */
    u32 unk28;       /* 0x28 */
    s32 pitch;       /* 0x2C - current SPU pitch (akao_compute_pitch result) */
    s32 unk30;       /* 0x30 */
    u32 unk34;       /* 0x34 */
    s32 unk38;
    u32 reverb_mask; /* 0x3C - channels enabled in SPU reverb bitmap */
    u32 noise_mask;  /* 0x40 - channels enabled in SPU noise bitmap */
    u32 pitch_mod_mask; /* 0x44 - channels enabled in SPU pitch-mod bitmap */
    u32 unk48;       /* 0x48 */
    u32 unk4C;       /* 0x4C */
    s32 unk50;       /* 0x50 */
    s32 unk54;       /* 0x54 - pan-envelope step; also reused as a scratch pitch-delta pointer target */
    u16 unk58;       /* 0x58 - pan-envelope fade-tick countdown */
    s16 unk5A;       /* 0x5A */
    u16 unk5C;       /* 0x5C */
    u16 unk5E;       /* 0x5E */
    u16 unk60;       /* 0x60 */
    u16 unk62;
    u16 unk64;       /* 0x64 */
    u16 unk66;       /* 0x66 */
    u16 unk68;       /* 0x68 */
    u16 unk6A;       /* 0x6A */
    u16 unk6C;       /* 0x6C */
    u16 unk6E;
    u16 unk70;       /* 0x70 */
    u16 unk72;       /* 0x72 */
    u16 unk74[11];   /* 0x74 - 0x89 */
    u16 unk8A;       /* 0x8A */
    u16 unk8C;       /* 0x8C */
    u8 _pad8E[6];    /* 0x8E - 0x93 */
    u16 unk94;       /* 0x94 */
    u16 unk96;       /* 0x96 */
    u16 unk98;       /* 0x98 */
    u16 unk9A;       /* 0x9A */
    u16 unk9C;       /* 0x9C */
    u16 unk9E;       /* 0x9E */
    u8 _padA0[2];    /* 0xA0 - 0xA1 */
    u16 unkA2;       /* 0xA2 */
    u16 unkA4;       /* 0xA4 */
    u8 _padA6[2];    /* 0xA6 - 0xA7 */
    u16 unkA8;       /* 0xA8 */
    u16 unkAA;       /* 0xAA */
    s16 unkAC;       /* 0xAC */
    u16 unkAE;       /* 0xAE */
    u8 _padB0[6];    /* 0xB0 - 0xB5 */
    u16 unkB6;       /* 0xB6 */
    u16 unkB8;       /* 0xB8 */
    u8 _padBA[2];    /* 0xBA - 0xBB */
    u16 unkBC;       /* 0xBC */
    u16 unkBE;       /* 0xBE */
    u8 _padC0[24];   /* 0xC0 - 0xD7 */
    u16 unkD8;       /* 0xD8 */
    u16 unkDA;       /* 0xDA */
    u16 unkDC;       /* 0xDC */
    u16 unkDE;       /* 0xDE */
    u16 unkE0;       /* 0xE0 */
    s16 unkE2;       /* 0xE2 */
    u16 unkE4;       /* 0xE4 */
    u16 unkE6;       /* 0xE6 */
    u16 unkE8;       /* 0xE8 */
    u16 unkEA;       /* 0xEA */
    s16 unkEC;       /* 0xEC */
    u16 unkEE;       /* 0xEE */
    u16 unkF0;       /* 0xF0 */
    s16 unkF2;       /* 0xF2 */
    u16 unkF4;       /* 0xF4 */
    u16 unkF6;       /* 0xF6 */
    u8 _padF8[4];    /* 0xF8 - 0xFB */
    u32 unkFC;       /* 0xFC - assigned SPU voice index (0x18 = none) */
    s32 unk100;      /* 0x100 - pending SPU register update flags */
    s32 spu_sample_addr; /* 0x104 - SPU sample start */
    s32 spu_loop_addr;   /* 0x108 - SPU loop point */
    u16 spu_pitch;   /* 0x10C - SPU pitch/sample-rate register image */
    u16 unk10E;      /* 0x10E - SPU ADSR low halfword */
    u16 unk110;      /* 0x110 - SPU ADSR high halfword */
    u16 spu_volume_scale; /* 0x112 - optional Q7 scale */
    s16 spu_volume_left;  /* 0x114 */
    s16 spu_volume_right; /* 0x116 */
} AkaoChannelState; 

#endif
