#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} SeqRec;

extern u8 *D_80123FB8;

s32 func_800BD2FC(u8 *arg0, u16 *arg1);
s32 func_800BD3B0(u8 arg0, s32 arg1, u8 *arg2);
void func_800BD128(s32 arg0, SeqRec *arg1, s32 arg2);

void func_800B91C4(void)
{
    u16 sp10;
    s32 temp_a2;
    SeqRec *temp_a1;
    s32 r;
    s32 r0;

    r0 = func_800BD2FC(
        (u8 *)((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8 + 1,
        &sp10);
    do
    {
        ((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8 = r0;
    } while (0);
    r = func_800BD3B0(((SeqRec *)D_80123FB8)->unk0, sp10 << 16, D_80123FB8);
    temp_a2 = ((SeqRec *)D_80123FB8)->unk4;
    temp_a1 = (SeqRec *)(D_80123FB8 + (temp_a2 * 3 << 2));
    temp_a1->unk8 += r * 2;
    func_800BD128(0, temp_a1, temp_a2);
}
