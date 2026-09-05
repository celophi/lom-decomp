#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} SeqRec;

extern u8 *g_field_script;

s32 field_script_read_operand_or_owner(s32 arg0, u8 *arg1, s32 *arg2);
s32 field_script_read_u16(u8 *arg0, u16 *arg1);
s32 func_800BD3B0(s32 arg0, s32 arg1);
void func_800BD434(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief seq-record-nibble-decode family (4-call variant), exemplar field_script_op_0e.
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

    temp_a1 = (u8 *)((SeqRec *)(g_field_script + (((SeqRec *)g_field_script)->unk4 * 3 << 2)))->unk8;
    b = temp_a1[1];
    ((SeqRec *)(g_field_script + (((SeqRec *)g_field_script)->unk4 * 3 << 2)))->unk8 =
        field_script_read_operand_or_owner(b & 3, temp_a1 + 2, &sp10);
    ((SeqRec *)(g_field_script + (((SeqRec *)g_field_script)->unk4 * 3 << 2)))->unk8 =
        field_script_read_u16(
            (u8 *)((SeqRec *)(g_field_script + (((SeqRec *)g_field_script)->unk4 * 3 << 2)))->unk8,
            &sp14);
    do
    {
        r3 = func_800BD3B0(sp10, sp14 << 16);
        ((SeqRec *)(g_field_script + (((SeqRec *)g_field_script)->unk4 * 3 << 2)))->unk8 =
            field_script_read_operand_or_owner(
                (b >> 2) & 3,
                (u8 *)((SeqRec *)(g_field_script + (((SeqRec *)g_field_script)->unk4 * 3 << 2)))->unk8 + 2,
                &sp18);
        ((SeqRec *)(g_field_script + (((SeqRec *)g_field_script)->unk4 * 3 << 2)))->unk8 =
            field_script_read_u16(
                (u8 *)((SeqRec *)(g_field_script + (((SeqRec *)g_field_script)->unk4 * 3 << 2)))->unk8,
                &sp1C);
    } while (0);
    func_800BD434(sp18, sp1C << 16, r3);
}
