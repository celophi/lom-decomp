#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} SeqRec;

extern u8 *D_80123FB8;

s32 func_800BD1B4(s32 arg0, u8 *arg1, s32 *arg2);
s32 func_800BD204(s32 arg0, void *arg1, s32 *arg2);
void func_80087614(s32 arg0, s32 arg1);

void func_800BABD4(void)
{
    s32 sp10;
    s32 sp14;
    u8 temp_s0;
    u8 *temp_a1;
    SeqRec *temp_a2;
    s32 temp_a3;
    s32 r;

    temp_a1 = (u8 *)((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8;
    temp_s0 = temp_a1[1];
    ((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8 =
        func_800BD1B4(temp_s0 >> 6, temp_a1 + 2, &sp10);
    r = func_800BD204((temp_s0 >> 4) & 3, (u8 *)((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8, &sp14);
    temp_a3 = ((SeqRec *)D_80123FB8)->unk4;
    temp_a2 = (SeqRec *)(D_80123FB8 + (temp_a3 * 3 << 2));
    temp_a2->unk8 = r;
    func_80087614(sp10, sp14);
}
