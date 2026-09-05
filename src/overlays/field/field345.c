#include "common.h"

typedef struct {
    u16 unk0;
    u16 unk2;
    s32 unk4;
    s32 unk8;
    s32 unkC;
} SomeStruct;

extern u8 *g_field_script;
extern void func_8006B984(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);

/**
 * @brief Dispatch a command record, decoding its priority flag.
 *
 * Resolves the record's target index (@c g_field_script[0] when @c unk0 is the 0xFF
 * sentinel), decodes @c unk2: when bit 7 is set the priority flag is 1 and the
 * value is the low 7 bits, otherwise the flag is 0 and the value is @c unk2 in
 * full. Forwards the record's three payload words plus flag, value, and target
 * index to func_8006B984.
 *
 * @param arg0 Unused.
 * @param arg1 Command record.
 * @see decomp.me (100%) TODO
 */
void func_800BE37C(s32 arg0, SomeStruct *arg1)
{
    s32 var_a0;
    u16 temp_v1;
    s32 var_v0;
    s32 var_a3;

    if (arg1->unk0 == 0xFF)
    {
        var_a0 = *g_field_script;
    }
    else
    {
        var_a0 = arg1->unk0;
    }
    temp_v1 = arg1->unk2;
    var_a3 = 1;
    if (temp_v1 & 0x80)
    {
        var_v0 = temp_v1 & 0x7F;
    }
    else
    {
        var_a3 = 0;
        var_v0 = arg1->unk2;
    }
    func_8006B984(arg1->unk4, arg1->unk8, arg1->unkC, var_a3, var_v0, var_a0);
}
