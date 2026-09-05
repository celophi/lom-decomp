#include "common.h"

typedef struct {
    u8 pad[0x90];
    s32 unk90;
} SomeStruct;

extern u8 *g_field_script;
extern SomeStruct *func_800C1B60(s32 arg0);
extern void func_80089AE4(s32 arg0, s32 arg1);

/**
 * @brief Resolve a target index and clear two flag bits on its state record.
 *
 * When @p arg0 is the sentinel 0xFF the index is read from @c g_field_script[0];
 * otherwise it is @p arg0 itself. The resolved index selects a state record via
 * func_800C1B60, whose @c unk90 field has bits 31 and 29 cleared, then the index
 * and @p arg1 are dispatched to func_80089AE4.
 *
 * @param arg0 Target index, or 0xFF to read the default from @c g_field_script[0].
 * @param arg1 Forwarded to func_80089AE4.
 * @see decomp.me (100%) TODO
 */
void func_800BC7EC(s32 arg0, s32 arg1)
{
    s32 var_s0;
    SomeStruct *temp_v0;

    if (arg0 == 0xFF)
    {
        var_s0 = *g_field_script;
    }
    else
    {
        var_s0 = arg0;
    }
    temp_v0 = func_800C1B60(var_s0);
    temp_v0->unk90 &= 0x7FFFFFFF;
    temp_v0->unk90 &= 0xDFFFFFFF;
    func_80089AE4(var_s0, arg1);
}
