#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    s32 unk4;
    s32 unk8;
} SeqRec;

extern u8 *D_80123FB8;

s32 func_800BD204(u32 arg0, void *arg1, s32 *arg2);
void func_800A43E8(s32 arg0, s32 arg1, u16 arg2, s32 arg3);
void func_800B286C(s32 arg0, s32 arg1, s32 arg2);

#define CURRENT_SEQ_REC ((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))

void func_800BA574(void)
{
    s32 sp18;
    s32 sp14;
    s32 sp10;
    u32 temp_s1;
    u8 *temp_a1;

    temp_a1 = (u8 *)CURRENT_SEQ_REC->unk8;
    temp_s1 = temp_a1[1];
    CURRENT_SEQ_REC->unk8 = func_800BD204(temp_s1 >> 6, temp_a1 + 2, &sp10);
    CURRENT_SEQ_REC->unk8 = func_800BD204((temp_s1 >> 4) & 3, (void *)CURRENT_SEQ_REC->unk8, &sp14);
    CURRENT_SEQ_REC->unk8 = func_800BD204((temp_s1 >> 2) & 3, (void *)CURRENT_SEQ_REC->unk8, &sp18);
    if (sp18 == 0xFF)
    {
        sp18 = -1;
    }
    func_800A43E8(temp_s1 & 3, sp10, (u16)sp14, sp18);
    func_800B286C(0x80, 0, 0xF);
}
