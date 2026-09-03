#include "common.h"
typedef struct { u8 unk0; u8 pad1[3]; s32 unk4; s32 unk8; u32 unkC; } SeqRec;
extern u8 *D_80123FB8;
s32 func_800BD2FC(u8 *arg0, u16 *arg1);
s32 func_800BD3B0(s32 arg0, s32 arg1);
s32 func_800BD204(s32 arg0, void *arg1, s32 *arg2);
#define CURRENT_SEQ_REC ((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))

void func_800B89D0(void)
{
    u16 sp10;
    s32 sp14;
    s32 sp18;
    u8 temp_s0;
    u8 *temp_a1;
    s32 r;

    temp_a1 = (u8 *)CURRENT_SEQ_REC->unk8;
    temp_s0 = temp_a1[1];
    CURRENT_SEQ_REC->unk8 = func_800BD2FC(temp_a1 + 2, &sp10);
    r = func_800BD3B0(((SeqRec *)D_80123FB8)->unk0, sp10 << 16);
    CURRENT_SEQ_REC->unk8 = func_800BD204(temp_s0, (void *)CURRENT_SEQ_REC->unk8, &sp14);
    CURRENT_SEQ_REC->unk8 = func_800BD204(temp_s0 >> 2, (void *)CURRENT_SEQ_REC->unk8, &sp18);
    if ((u32)r >= (u32)sp14 && (u32)r <= (u32)sp18)
        CURRENT_SEQ_REC->unkC |= 1;
    else
        CURRENT_SEQ_REC->unkC &= ~1;
}
