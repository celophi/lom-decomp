#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} SeqRec;

extern u8 *g_field_script;

s32 func_800BD2FC(u8 *arg0, u16 *arg1);
s32 field_script_read_operand(s32 arg0, void *arg1, s32 *arg2);
void func_800BD434(u8 arg0, s32 arg1, s32 arg2);

void func_800B9DD4(void)
{
    u16 sp10;
    s32 sp14;
    u8 temp_s1;
    u8 *temp_a1;
    SeqRec *temp_a2;
    s32 temp_a3;
    s32 r;

    temp_a1 = (u8 *)((SeqRec *)(g_field_script + (((SeqRec *)g_field_script)->unk4 * 3 << 2)))->unk8;
    temp_s1 = temp_a1[1];
    ((SeqRec *)(g_field_script + (((SeqRec *)g_field_script)->unk4 * 3 << 2)))->unk8 =
        func_800BD2FC(temp_a1 + 2, &sp10);
    r = field_script_read_operand(
        temp_s1,
        (u8 *)((SeqRec *)(g_field_script + (((SeqRec *)g_field_script)->unk4 * 3 << 2)))->unk8,
        &sp14);
    temp_a3 = ((SeqRec *)g_field_script)->unk4;
    temp_a2 = (SeqRec *)(g_field_script + (temp_a3 * 3 << 2));
    temp_a2->unk8 = r;
    func_800BD434(((SeqRec *)g_field_script)->unk0, sp10 << 16, sp14);
}
