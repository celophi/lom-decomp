#ifndef _AKAO_CMD_H
#define _AKAO_CMD_H

#include "common.h"
#include "akao.h"
#include "psyq/libcd.h"

/**
 * AKAO command parameter buffer. Each command opcode reads its inputs from
 * the first N slots; the layout is opcode-specific and the driver consumes
 * it during akao_send_command.
 *
 * Slot 0 is dual-purpose: most opcodes treat it as a scalar (channel index,
 * sound id, volume), but several wrappers (akao_play_song, akao_register_bank,
 * akao_cmd_e0 opcode 0xE0, akao_cmd_ec opcode 0xEC) store an
 * AkaoSeqHeader-compatible **buffer pointer** there. Typed @c void* to
 * acknowledge that dual use; scalar stores rely on GCC 2.7.2's permissive
 * implicit int→pointer conversion.
 */
extern void *g_akaoCmdParams[];
extern s32 D_8004D400;
extern u8 g_sfx_channels[];

/**
 * Per-tick scratch state for the AKAO bank-streaming uploader.
 *
 * Primed on the first tick of a streaming upload from the AkaoBankHeader at
 * the head of the source buffer; each subsequent call to
 * akao_streaming_upload_tick consumes some bytes from the source and shrinks
 * the two `*_remaining` counters. When both reach zero (and the external
 * latch @c D_8004F828 is also clear), the streaming-pending bit in
 * @c g_akao_driver_flags is cleared.
 */
typedef struct
{
    void* articulation_dst;     /* 0x00: current dst into the driver's
                                          articulation slot table
                                          (D_8004C340 + bank_id * 0x10),
                                          advances as bytes are copied      */
    u32 spu_addr;               /* 0x04: current SPU upload address — seeded
                                          from AkaoBankHeader.spu_dest_addr,
                                          advances as samples are written;
                                          a value of 0 marks "first tick"   */
    u32 sample_remaining;       /* 0x08: bytes of sample data still to send
                                          to the SPU                        */
    u32 articulation_remaining; /* 0x0C: bytes of articulation data still to
                                          copy into the driver's slot table */
} AkaoStreamingState;

typedef struct {
    u8   pad0[0x08];           /* 0x00 - 0x07 */
    u32  unk8;                 /* 0x08 - from func_80023334 */
    u8   pad1[0x14];           /* 0x0C - 0x1F (20 bytes) */
    s32  unk20;                /* 0x20 - from func_800232A8 */
    s32  unk24;                /* 0x24 - used in both functions */
    s32  unk28;                /* 0x28 - from func_800232A8 */
    u8   pad2[0x08];           /* 0x2C - 0x33 (8 bytes) */
    s32  unk34;                /* 0x34 - from func_800232A8 */
    s32  unk38;                /* 0x38 - used in both functions */
    s32  unk3C;                /* 0x3C - from func_800232A8 */
} AkaoXaTracker;  /* Total size: 0x40 (64 bytes) */

extern s32 D_8004F794;
extern s32 D_8004C170;
extern u32 D_8004C150;
extern AkaoChannelState* g_akao_seq_channel0;
extern CdlATV g_akao_cdmix;
extern s32 D_8004F754;
extern u8 D_8004C340[];
extern s32 D_8004D388;
extern s32 D_8004D38C;
extern s32 D_8004D390;
extern s32 D_8004D394;
extern s32 D_8004D398;
extern s32 D_8004D39C;
extern s32 g_akao_driver_flags;
extern s32 g_akao_spu_xfer_pending;
extern s32 D_8004F824;
extern s32 D_8004F828;
extern AkaoStreamingState g_akao_streaming_state;
extern AkaoBankHeader g_akao_bank_staging;

extern AkaoXaTracker g_akao_xa_tracker;

#define AKAO_CHANNEL_STATE (*(AkaoChannelState**)0x8003EC5C)

extern s32 akao_submit(AkaoSeqHeader* sequenceData, s32 waitForCompletion);

s32 FUN_80021fbc(void);
s32 func_80021FDC(void);
s32 akao_register_bank(AkaoSeqHeader* bank);
void akao_play_song(s32 seqData);
void akao_stop_song(s32 arg0);
void akao_cmd_40(void);
void akao_cmd_14(s32 arg0, s32 arg1);
s32 akao_cmd_19_c0(s32 arg0, s32 arg1);
void akao_cmd_12(s32 arg0, s32 arg1);
void akao_play_sfx(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
s32 akao_play_sfx_from_buffer(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void akao_cmd_21(s32 arg0, s32 arg1);
void akao_stop_sfx_by_id(s32 arg0);
s32 func_800222A8(void);
s32 func_80022310(s32 arg0);
void akao_set_paused(s32 arg0);
void akao_cmd_90(s32 arg0);
void akao_cmd_92(s32 arg0);
void akao_cmd_99_9b_9d_9f(u32 param_1);
void akao_cmd_98_9a_9c_9e(u32 arg0);

/**
 * Central dispatcher for the AKAO sound driver. Each high-level wrapper
 * (akao_play_song, akao_stop_song, akao_play_sfx, etc.) writes its inputs
 * into g_akaoCmdParams and then invokes this function with a one-byte
 * command opcode. Known opcodes used in this codebase: 0x10 play song,
 * 0x11 stop song, 0x12, 0x14, 0x19, 0x20 play SFX, 0x21, 0x24, 0x30, 0x40,
 * 0x80/0x81, 0x90/0x92, 0xA0/0xA1/0xA8/0xA9, 0xC0/0xC1, 0xF0/0xF1.
 *
 * Declared @c void here even though some call sites read its return value;
 * the original GCC 2.7.2 build tolerates the mixed declaration (callers rely
 * on the $v0 register convention).
 */
extern s32 akao_send_command(s32 opcode);

#endif