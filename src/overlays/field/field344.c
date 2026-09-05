#include "common.h"

typedef struct {
    u8 unk0;
    u8 pad1[3];
    s32 unk4;
    u8 pad8[4];
    s32 unkC;
} SeqRec;

extern u8 *g_field_script;
extern s32 func_8008C2EC(s32 arg0, s32 arg1);

/**
 * @brief Update the low flag bit of the active sequence record from a pair query.
 *
 * Resolves @p arg0 and @p arg1 (each from @c g_field_script[0] when it is the 0xFF
 * sentinel), runs func_8008C2EC for the pair, and stores the result's low bit
 * into bit 0 of the current record's @c unkC field (chosen by @c g_field_script->unk4).
 *
 * @param arg0 First index, or 0xFF to read the default from @c g_field_script[0].
 * @param arg1 Second index, or 0xFF to read the default from @c g_field_script[0].
 * @see decomp.me (100%) TODO
 */
void func_800BC2A0(s32 arg0, s32 arg1)
{
    SeqRec *temp_a1;
    s32 ret;
    s32 var_a0;
    s32 var_a1;

    if (arg0 == 0xFF)
    {
        var_a0 = *g_field_script;
    }
    else
    {
        var_a0 = arg0;
    }
    if (arg1 == 0xFF)
    {
        var_a1 = *g_field_script;
    }
    else
    {
        var_a1 = arg1;
    }
    ret = func_8008C2EC(var_a0, var_a1);
    temp_a1 = (SeqRec *)(g_field_script + (((SeqRec *)g_field_script)->unk4 * 0xC));
    temp_a1->unkC = (temp_a1->unkC & ~1) | (ret & 1);
}
