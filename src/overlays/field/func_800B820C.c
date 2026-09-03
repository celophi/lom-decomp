#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} SeqRec;

typedef void (*FieldDispatchFn)(s32, s32);

extern u8 *D_80123FB8;
extern FieldDispatchFn D_800F0D48[];

s32 func_800B84B4(s32 arg0, u8 *arg1, s32 *arg2);

/**
 * @brief Decode two sequence arguments and dispatch through the field handler table.
 *
 * The high nibble is materialized before the first decode call so GCC schedules
 * the shift into that call's delay slot, matching the original sequence decoder.
 */
void func_800B820C(void)
{
    s32 sp10;
    s32 sp14;
    u8 temp_s0;
    u8 *temp_a1;
    SeqRec *temp_a2;
    s32 temp_a3;
    s32 temp_s1;
    s32 r;
    s32 high;

    temp_a1 = (u8 *)((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8;
    temp_s0 = temp_a1[1];
    temp_s1 = temp_a1[0] - 0x40;
    high = temp_s0 >> 4;
    ((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8 =
        func_800B84B4(temp_s0 & 0xF, temp_a1 + 2, &sp10);
    r = func_800B84B4(
        high,
        (u8 *)((SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 3 << 2)))->unk8,
        &sp14);
    temp_a3 = ((SeqRec *)D_80123FB8)->unk4;
    temp_a2 = (SeqRec *)(D_80123FB8 + (temp_a3 * 3 << 2));
    temp_a2->unk8 = r;
    D_800F0D48[temp_s1](sp10, sp14);
}
