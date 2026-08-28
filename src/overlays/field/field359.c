#include "common.h"

/** @brief Active-sequence record; base holds unk4, records stride 0xC. */
typedef struct
{
    s32 unk0;   /* 0x00 */
    s32 unk4;   /* 0x04 current record index */
    s32 unk8;   /* 0x08 running data pointer (stored as an integer) */
} SeqRec;

extern u8 *D_80123FB8;

s32 func_800BD1B4(s32 arg0, u8 *arg1, s32 *arg2);

/**
 * @brief Decode a two-field packed sequence opcode and emit both operands.
 *
 * Reads the opcode byte at the active record's data pointer, splits it into a
 * 2-bit high field (@c >>6) and a 2-bit mid field (@c >>4 & 3), decodes each
 * through func_800BD1B4 (advancing the record's @c unk8 pointer), and forwards
 * the two decoded values to func_8008AFD8. The store target is resolved after
 * the second decode so it is not held across the call.
 *
 * @see decomp.me (100%) TODO
 */
void func_800BAB00(void)
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
    r = func_800BD1B4((temp_s0 >> 4) & 3, (u8 *)((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8, &sp14);
    temp_a3 = ((SeqRec *)D_80123FB8)->unk4;
    temp_a2 = (SeqRec *)(D_80123FB8 + (temp_a3 * 3 << 2));
    temp_a2->unk8 = r;
    func_8008AFD8(sp10, sp14, temp_a2, temp_a3);
}
