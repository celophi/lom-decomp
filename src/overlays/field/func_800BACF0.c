#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    s32 unk4;
    s32 unk8;
} SeqRec;

extern u8 *D_80123FB8;

s32 func_800BD1B4(s32 arg0, void *arg1, s32 *arg2);
s32 func_800BD204(s32 arg0, void *arg1, s32 *arg2);
s32 func_80087D8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

#define CURRENT_SEQ_REC ((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))

void func_800BACF0(void)
{
    s32 sp10;
    s32 sp14;
    s32 sp18;
    s32 sp1C;
    u32 opcode;
    u8 *seq;

    seq = (u8 *)CURRENT_SEQ_REC->unk8;
    opcode = seq[1];
    CURRENT_SEQ_REC->unk8 = func_800BD1B4(opcode >> 6, seq + 2, &sp10);
    CURRENT_SEQ_REC->unk8 = func_800BD204((opcode >> 4) & 3, (void *)CURRENT_SEQ_REC->unk8, &sp14);
    CURRENT_SEQ_REC->unk8 = func_800BD204((opcode >> 2) & 3, (void *)CURRENT_SEQ_REC->unk8, &sp18);
    CURRENT_SEQ_REC->unk8 = func_800BD204(opcode & 3, (void *)CURRENT_SEQ_REC->unk8, &sp1C);
    func_80087D8C(sp10, sp14, sp18, sp1C);
}
