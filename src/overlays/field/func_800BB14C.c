#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    s32 unk4;
    s32 unk8;
} SeqRec;

extern u8 *D_80123FB8;

void func_800A3988(s32 arg0, s32 arg1, s32 arg2, SeqRec *arg3);
s32 func_800BD204(s32 arg0, void *arg1, s32 *arg2);

#define CURRENT_SEQ_REC ((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))

void func_800BB14C(void)
{
    s32 sp18;
    s32 sp14;
    s32 sp10;
    u32 temp_s0;
    u8 *temp_a1;
    SeqRec *temp_a3;
    s32 r;

    temp_a1 = (u8 *)CURRENT_SEQ_REC->unk8;
    temp_s0 = temp_a1[1];
    CURRENT_SEQ_REC->unk8 = func_800BD204(temp_s0 & 3, temp_a1 + 2, &sp10);
    CURRENT_SEQ_REC->unk8 = func_800BD204((temp_s0 >> 2) & 3, (void *)CURRENT_SEQ_REC->unk8, &sp14);
    r = func_800BD204((temp_s0 >> 4) & 3, (void *)CURRENT_SEQ_REC->unk8, &sp18);
    temp_a3 = CURRENT_SEQ_REC;
    temp_a3->unk8 = r;
    func_800A3988(sp10, sp14, sp18, temp_a3);
}
