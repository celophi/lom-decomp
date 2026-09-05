#include "common.h"

typedef struct { u8 unk0; u8 pad1[3]; s32 unk4; s32 unk8; } SeqRec;
extern u8 *g_field_script;
s32 field_script_read_operand(s32 arg0, void *arg1, s32 *arg2);
s32 func_800BE5C8(s32 arg0, s32 arg1, s32 arg2);
s32 func_800BD2FC(u8 *arg0, u16 *arg1);
void func_800BD434(s32 arg0, s32 arg1, s32 arg2);
#define CURRENT_SEQ_REC ((SeqRec *)(g_field_script + (((SeqRec *)g_field_script)->unk4 * 3 << 2)))

void func_800B9EA8(void)
{
    s32 sp10;
    s32 sp14;
    u16 sp18;
    u32 b;
    u8 *p;
    s32 r;

    p = (u8 *)CURRENT_SEQ_REC->unk8;
    b = p[1];
    CURRENT_SEQ_REC->unk8 = (s32)(p + 2);
    CURRENT_SEQ_REC->unk8 = field_script_read_operand(b, (void *)CURRENT_SEQ_REC->unk8, &sp10);
    b >>= 2;
    CURRENT_SEQ_REC->unk8 = field_script_read_operand(b, (void *)CURRENT_SEQ_REC->unk8, &sp14);
    r = func_800BE5C8(b >> 2, sp10, sp14);
    CURRENT_SEQ_REC->unk8 = func_800BD2FC((u8 *)CURRENT_SEQ_REC->unk8, &sp18);
    func_800BD434(((SeqRec *)g_field_script)->unk0, sp18 << 16, r);
}
