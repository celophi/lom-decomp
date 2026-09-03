#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    s32 unk4;
    s32 unk8;
} SeqRec;

extern u8 *D_80123FB8;
extern u8 *D_80122B78;

s32 func_800BD204(u32 arg0, void *arg1, s32 *arg2);
void func_8009C77C(s32 arg0, s32 arg1, s32 arg2);

#define CURRENT_SEQ_REC ((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))

void func_800BB004(void)
{
    s32 sp18;
    s32 sp14;
    s32 sp10;
    u32 temp_s0;
    u8 *temp_a1;
    s32 arg0;

    temp_a1 = (u8 *)CURRENT_SEQ_REC->unk8;
    temp_s0 = temp_a1[1];
    CURRENT_SEQ_REC->unk8 = func_800BD204(temp_s0 >> 6, temp_a1 + 2, &sp10);
    CURRENT_SEQ_REC->unk8 = func_800BD204((temp_s0 >> 4) & 3, (void *)CURRENT_SEQ_REC->unk8, &sp14);
    CURRENT_SEQ_REC->unk8 = func_800BD204((temp_s0 >> 2) & 3, (void *)CURRENT_SEQ_REC->unk8, &sp18);
    if (sp10 == 0xFF)
    {
        arg0 = *(u32 *)(D_80122B78 + 0x41C) >> 8;
        arg0 &= 3;
    }
    else
    {
        arg0 = sp10;
    }
    sp10 = arg0;
    func_8009C77C(arg0, sp14, sp18);
}
