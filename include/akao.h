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
 */
typedef enum AkaoCmd
{
    AKAO_CMD_PLAY_SONG          = 0x10, /**< play sequence; slot 0 = AKAO buffer pointer */
    AKAO_CMD_STOP_SONG          = 0x11, /**< stop active sequence (callers pass 0)        */
    AKAO_CMD_12                 = 0x12, /**< 2 args: (?, ?) — TBD                         */
    AKAO_CMD_14                 = 0x14, /**< 3 args: (?, ?, 0) — TBD                      */
    AKAO_CMD_19                 = 0x19, /**< 1 arg; paired with 0xC0                      */
    AKAO_CMD_PLAY_SFX           = 0x20, /**< play SFX: (id10, p24, p8, vol7)              */
    AKAO_CMD_21                 = 0x21, /**< 2 args: (id, p24) — TBD                      */
    AKAO_CMD_PLAY_SFX_FROM_BUF  = 0x24, /**< play SFX from caller-supplied AKAO buffer    */
    AKAO_CMD_STOP_SFX_BY_ID     = 0x30, /**< stop active SFX matching 10-bit sound id     */
    AKAO_CMD_GLOBAL_STOP        = 0x40, /**< zero-arg "silence everything"                */
    AKAO_CMD_PAUSE              = 0x80, /**< pause active sequence                        */
    AKAO_CMD_RESUME             = 0x81, /**< resume paused sequence                       */
    AKAO_CMD_90                 = 0x90, /**< 1 arg — TBD                                  */
    AKAO_CMD_92                 = 0x92, /**< 1 arg — TBD                                  */
    AKAO_CMD_98                 = 0x98, /**< zero-arg; selected by 99/9b/9d/9f wrapper    */
    AKAO_CMD_99                 = 0x99,
    AKAO_CMD_9A                 = 0x9A,
    AKAO_CMD_9B                 = 0x9B,
    AKAO_CMD_9C                 = 0x9C,
    AKAO_CMD_9D                 = 0x9D,
    AKAO_CMD_9E                 = 0x9E,
    AKAO_CMD_9F                 = 0x9F,
    AKAO_CMD_A0                 = 0xA0, /**< per-channel: (ch, p24, val7)                 */
    AKAO_CMD_A1                 = 0xA1, /**< per-channel: (ch, p24, p, val7)              */
    AKAO_CMD_A2                 = 0xA2, /**< per-channel: (ch, p24, val8)                 */
    AKAO_CMD_A3                 = 0xA3, /**< per-channel: (ch, p24, p, val8)              */
    AKAO_CMD_A4                 = 0xA4, /**< per-channel: (ch, p24, val8)                 */
    AKAO_CMD_A5                 = 0xA5, /**< per-channel: (ch, p24, p, val8)              */
    AKAO_CMD_A8                 = 0xA8, /**< master/global: (val7)                        */
    AKAO_CMD_A9                 = 0xA9, /**< master/global: (p, val7)                     */
    AKAO_CMD_AA                 = 0xAA, /**< master/global: (val8)                        */
    AKAO_CMD_AB                 = 0xAB, /**< master/global: (p, val8)                     */
    AKAO_CMD_AC                 = 0xAC, /**< master/global: (val8)                        */
    AKAO_CMD_AD                 = 0xAD, /**< master/global: (p, val8)                     */
    AKAO_CMD_C0                 = 0xC0, /**< 2 args ending in 7-bit value                 */
    AKAO_CMD_C1                 = 0xC1, /**< 3 args ending in 7-bit value                 */
    AKAO_CMD_C2                 = 0xC2, /**< 4 args; two 7-bit values                     */
    AKAO_CMD_C8                 = 0xC8, /**< 1 arg                                        */
    AKAO_CMD_C9                 = 0xC9, /**< 2 args                                       */
    AKAO_CMD_CA                 = 0xCA, /**< 3 args                                       */
    AKAO_CMD_D0                 = 0xD0, /**< 1 arg (8-bit)                                */
    AKAO_CMD_D1                 = 0xD1, /**< 2 args (?, 8-bit)                            */
    AKAO_CMD_D2                 = 0xD2, /**< 3 args (?, 8-bit, 8-bit)                     */
    AKAO_CMD_D4                 = 0xD4, /**< 1 arg (8-bit)                                */
    AKAO_CMD_D5                 = 0xD5, /**< 2 args (?, 8-bit)                            */
    AKAO_CMD_D6                 = 0xD6, /**< 3 args (?, 8-bit, 8-bit)                     */
    AKAO_CMD_D8                 = 0xD8, /**< 1 arg (8-bit)                                */
    AKAO_CMD_D9                 = 0xD9, /**< 2 args (?, 8-bit)                            */
    AKAO_CMD_DA                 = 0xDA, /**< 3 args (?, 8-bit, 8-bit)                     */
    AKAO_CMD_E0                 = 0xE0, /**< takes a magic-checked AKAO buffer pointer    */
    AKAO_CMD_E2                 = 0xE2, /**< zero-arg                                     */
    AKAO_CMD_E4_SET_CD_VOLUME   = 0xE4, /**< CD/XA channel mix volume                     */
    AKAO_CMD_E5                 = 0xE5, /**< 2 args                                       */
    AKAO_CMD_E6                 = 0xE6, /**< 1 arg                                        */
    AKAO_CMD_E8_START_XA_STREAM = 0xE8, /**< begin XA-streamed AKAO playback              */
    AKAO_CMD_EC                 = 0xEC, /**< takes a magic-checked AKAO buffer pointer    */
    AKAO_CMD_ED                 = 0xED, /**< 2 args                                       */
    AKAO_CMD_F0                 = 0xF0, /**< zero-arg query, return value used            */
    AKAO_CMD_F1                 = 0xF1  /**< zero-arg query, return value used            */
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
    u8 magic[4];
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
 * Per-channel runtime state for the AKAO driver.
 *
 * The driver allocates 0x20 sequence-channel slots back-to-back starting at
 * @c D_8004C260 and another 0x18 SFX-channel slots starting at @c D_8004B430.
 * Each slot is 0x118 bytes wide. @c D_8003EC5C is a pointer set in
 * akao_driver_init_state to alias the first sequence-channel slot; the
 * streaming/XA-setup code-paths read this slot's flag byte to decide whether
 * to relocate the SPU upload window.
 *
 * Only the fields that are actually inspected through @c D_8003EC5C are typed
 * here; the remainder of the slot is padded out so @c sizeof reflects the
 * real channel stride.
 */
typedef struct AkaoChannelState
{
    u32 flags;        /* 0x00: bit 0x40 set ⇒ channel is active/playing */
    u32 unk4;         /* 0x04: tested non-zero alongside unk1C            */
    u8  _pad08[0x14]; /* 0x08 - 0x1B                                       */
    u32 unk1C;        /* 0x1C: tested non-zero alongside unk4              */
    u8  _pad20[0xF8]; /* 0x20 - 0x117                                      */
} AkaoChannelState;  /* total: 0x118                                       */

#endif
