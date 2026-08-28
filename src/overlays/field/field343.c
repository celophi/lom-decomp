#include "common.h"

typedef struct {
    u8 unk0;
    u8 pad1[3];
    s32 unk4;
    u8 pad8[4];
    s32 unkC;
} SeqRec;

extern u8 *D_80123FB8;
extern s32 func_800C1FFC(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Update the low flag bit of the active sequence record.
 *
 * When @p arg0 is zero, resolves a target index (from @c D_80123FB8[0] when
 * @p arg1 is the 0xFF sentinel, else @p arg1), runs func_800C1FFC for that
 * index, and writes the result's low bit into bit 0 of the current record's
 * @c unkC field (the record chosen by @c D_80123FB8->unk4).
 *
 * @param arg0 Guard; the update runs only when zero.
 * @param arg1 Target index, or 0xFF to read the default from @c D_80123FB8[0].
 * @see decomp.me (100%) TODO
 */
void func_800BBA44(s32 arg0, s32 arg1)
{
    s32 var_a0;
    SeqRec *temp_a1;
    s32 ret;

    if (arg0 == 0)
    {
        if (arg1 == 0xFF)
        {
            var_a0 = *D_80123FB8;
        }
        else
        {
            var_a0 = arg1;
        }
        ret = func_800C1FFC(var_a0, 0x1100, 0x1100);
        temp_a1 = (SeqRec *)(D_80123FB8 + (((SeqRec *)D_80123FB8)->unk4 * 0xC));
        temp_a1->unkC = (temp_a1->unkC & ~1) | (ret & 1);
    }
}
