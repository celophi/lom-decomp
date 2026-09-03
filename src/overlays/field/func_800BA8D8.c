#include "common.h"
typedef struct { u8 unk0; u8 pad1[3]; s32 unk4; s32 unk8; } SeqRec;
extern u8 *D_80123FB8;
s32 func_800BD204(u32 arg0, void *arg1, s32 *arg2);
void func_800675C8(s32 arg0, s32 arg1, s32 arg2);
#define CURRENT_SEQ_REC ((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))
void func_800BA8D8(void)
{
    s32 sp18;
    s32 sp14;
    s32 sp10;
    u32 temp_s0;
    u8 *temp_a1;
    u32 temp;

    temp_a1 = (u8 *)CURRENT_SEQ_REC->unk8;
    temp_s0 = temp_a1[1];
    CURRENT_SEQ_REC->unk8 = func_800BD204(temp_s0 >> 6, temp_a1 + 2, &sp10);
    CURRENT_SEQ_REC->unk8 = func_800BD204((temp_s0 >> 4) & 3, (void *)CURRENT_SEQ_REC->unk8, &sp14);
    CURRENT_SEQ_REC->unk8 = func_800BD204((temp_s0 >> 2) & 3, (void *)CURRENT_SEQ_REC->unk8, &sp18);
    if (sp18 == 0)
    {
        temp = sp14;
        sp18 = 1;
        for (;;)
        {
            temp /= 10;
            if (temp == 0)
                break;
            sp18++;
        }
    }
    func_800675C8((u16)sp10, sp14, (u8)sp18);
}
