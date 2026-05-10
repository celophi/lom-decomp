#ifndef _AKAO_H
#define _AKAO_H

#include "common.h"

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
 *                                      akao_setup_xa_buffer (func_800230C8)
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
