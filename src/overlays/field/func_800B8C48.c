#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
} SeqRec;

extern u8 *D_80123FB8;

s32 func_800BD204(u8 arg0, u8 *arg1, s32 *arg2);

void func_800B8C48(void)
{
    s32 sp10;
    u8 *temp_v0;
    SeqRec *temp_v0_2;

    temp_v0 = (u8 *)((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8;
    ((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8 =
        func_800BD204(temp_v0[1], temp_v0 + 2, &sp10);
    temp_v0_2 = (SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2));
    temp_v0_2->unk10 = (temp_v0_2->unk10 & 1) | (sp10 * 2);
    ((SeqRec *)D_80123FB8)->unk0 &= 0x7FFFFFFF;
}
