#ifndef _DECOMP3_H
#define _DECOMP3_H

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
 * func_80022FAC opcode 0xE0, func_800231E4 opcode 0xEC) store an
 * AkaoSeqHeader-compatible **buffer pointer** there. Typed @c void* to
 * acknowledge that dual use; scalar stores rely on GCC 2.7.2's permissive
 * implicit int→pointer conversion.
 */
extern void *g_akaoCmdParams[];
extern s32 D_8004D400;
extern u8 D_8004B430[];

typedef struct
{
    void* unk0;
    u32 unk4;
    u32 unk8;
    u32 unkC;
} F820_t;

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
} AkakoStruct;  /* Total size: 0x40 (64 bytes) */

extern s32 D_8004F794;
extern s32 D_8004C170;
extern u32 D_8004C150;
extern AkaoChannelState* D_8003EC5C;
extern CdlATV D_8003EC20;
extern s32 D_8004F754;
extern u8 D_8004C340[];
extern s32 D_8004D388;
extern s32 D_8004D38C;
extern s32 D_8004D390;
extern s32 D_8004D394;
extern s32 D_8004D398;
extern s32 D_8004D39C;
extern s32 D_8004F750;
extern s32 D_8003EC4C;
extern s32 D_8004F824;
extern s32 D_8004F828;
extern F820_t D_8004F820;
extern AkaoBankHeader D_8004D3C0;

extern AkakoStruct D_8004F760;

extern s32 akao_submit(AkaoSeqHeader* sequenceData, s32 waitForCompletion);

s32 FUN_80021fbc(void);
s32 func_80021FDC(void);
s32 akao_register_bank(AkaoSeqHeader* bank);
void akao_play_song(s32 seqData);
void akao_stop_song(s32 arg0);
void akao_cmd_40(void);
void func_800220B0(s32 arg0, s32 arg1);
s32 func_800220E4(s32 arg0, s32 arg1);
void func_8002213C(s32 arg0, s32 arg1);
void akao_play_sfx(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
s32 func_800221BC(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_80022240(s32 arg0, s32 arg1);
void func_8002227C(s32 arg0);
s32 func_800222A8(void);
s32 func_80022310(s32 arg0);
void akao_set_paused(s32 arg0);
void func_800223B0(s32 arg0);
void func_800223D8(s32 arg0);
void FUN_80022400(u32 param_1);
void func_8002246C(u32 arg0);

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