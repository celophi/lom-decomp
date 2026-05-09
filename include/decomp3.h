#ifndef _DECOMP3_H
#define _DECOMP3_H

#include "common.h"
#include "akao.h"

/**
 * AKAO command parameter buffer. Each command opcode reads its inputs from
 * the first N slots; the layout is opcode-specific and the driver consumes
 * it during akao_send_command.
 */
extern s32 g_akaoCmdParams[];
extern s32 D_8004D400;
extern u8 D_8004B430[];

extern s32 D_8004F750;
extern s32 D_8003EC4C;
extern s32 D_8004F824;

extern s32 akao_submit(AkaoSeqHeader* sequenceData, s32 waitForCompletion);

s32 FUN_80021fbc(void);
s32 func_80021FDC(void);
s32 akao_register_bank(AkaoSeqHeader* bank);
void akao_play_song(s32 seqData);
void akao_stop_song(s32 arg0);
void func_80022090(void);
void func_800220B0(s32 arg0, s32 arg1);
s32 func_800220E4(s32 arg0, s32 arg1);
void func_8002213C(s32 arg0, s32 arg1);
void akao_play_sfx(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
s32 func_800221BC(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_80022240(s32 arg0, s32 arg1);
void func_8002227C(s32 arg0);
s32 func_800222A8(void);
s32 func_80022310(s32 arg0);
void func_8002237C(s32 arg0);
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