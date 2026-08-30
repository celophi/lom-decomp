#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} SeqRec;

extern u8 *D_80123FB8;

s32 func_800BD1B4(s32 arg0, u8 *arg1, s32 *arg2);
s32 func_800BD2FC(u8 *arg0, u16 *arg1);
s32 func_800BD3B0(s32 arg0, s32 arg1);
void func_800BD434(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief seq-record-nibble-decode family (4-call variant), exemplar func_800B91C4.
 *
 * WIP 95.92% (gcc280_g0). The do-while(0) around the middle three statements
 * supplies NOTE_INSN_LOOP markers that fix the store-vs-arg sched1 ordering;
 * do not remove it. The remaining ~4% is a block-4 a1/a2 register-coloring
 * race with no clean natural-C spelling (the permuter only reaches lower
 * scores via scaffolding). See working/func_800B8CFC/STATUS.md.
 */
void func_800B8CFC(void)
{
    s32 sp10;
    u16 sp14;
    s32 sp18;
    u16 sp1C;
    u8 b;
    u8 *temp_a1;
    s32 r3;

    temp_a1 = (u8 *)((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8;
    b = temp_a1[1];
    ((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8 =
        func_800BD1B4(b & 3, temp_a1 + 2, &sp10);
    ((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8 =
        func_800BD2FC(
            (u8 *)((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8,
            &sp14);
    do
    {
        r3 = func_800BD3B0(sp10, sp14 << 16);
        ((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8 =
            func_800BD1B4(
                (b >> 2) & 3,
                (u8 *)((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8 + 2,
                &sp18);
        ((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8 =
            func_800BD2FC(
                (u8 *)((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8,
                &sp1C);
    } while (0);
    func_800BD434(sp18, sp1C << 16, r3);
}
