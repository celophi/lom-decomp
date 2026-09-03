#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    s32 unk4;
    s32 unk8;
} SeqRec;

extern u8 *D_80123FB8;

s32 func_800BD1B4(u32 arg0, void *arg1, s32 *arg2);
s32 func_800BD204(u32 arg0, void *arg1, s32 *arg2);
void func_800B2654(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3);
void func_8009C620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

#define CURRENT_SEQ_REC ((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))

void func_800BAE54(void)
{
    s32 sp1C;
    s32 sp18;
    s32 sp14;
    s32 sp10;
    u32 temp_s0;
    u8 *temp_a1;
    s32 temp_t0;

    temp_a1 = (u8 *)CURRENT_SEQ_REC->unk8;
    temp_s0 = temp_a1[1];
    CURRENT_SEQ_REC->unk8 = func_800BD1B4(temp_s0 >> 6, temp_a1 + 2, &sp10);
    CURRENT_SEQ_REC->unk8 = func_800BD204((temp_s0 >> 4) & 3, (void *)CURRENT_SEQ_REC->unk8, &sp14);
    CURRENT_SEQ_REC->unk8 = func_800BD204((temp_s0 >> 2) & 3, (void *)CURRENT_SEQ_REC->unk8, &sp18);
    CURRENT_SEQ_REC->unk8 = func_800BD204(temp_s0 & 3, (void *)CURRENT_SEQ_REC->unk8, &sp1C);
    temp_t0 = sp10;
    if (temp_t0 == 0xFF)
    {
        temp_t0 = D_80123FB8[0];
    }
    sp10 = temp_t0;
    func_800B2654(&sp10, &sp14, &sp18, &sp1C);
    func_8009C620(sp14, sp1C, sp10, sp18);
}
