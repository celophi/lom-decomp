#include "common.h"

extern u8 *D_80122B78;
extern u8 *g_field_script;

/**
 * @brief Route a resolved sequence index to one of two audio handlers.
 *
 * Resolves @p arg0 (0xFF reads the default from @c g_field_script[0]). When scene
 * flag bit 0x10000 at @c D_80122B78 + 0x400 is set and the resolved index is
 * below 3, it plays the pair channel via func_800C2928 / func_80087E00;
 * otherwise it plays the single channel via func_80087CE0. Finally clears bit
 * 31 of the word at @c g_field_script[0].
 *
 * @param arg0 Channel index, or 0xFF to read the default from @c g_field_script[0].
 * @param arg1 Pair parameter forwarded (low 16 bits) to func_800C2928.
 * @see decomp.me (100%) TODO
 */
void func_800BC4E8(s32 arg0, s32 arg1)
{
    s32 var_a0;
    u32 temp_s0;

    var_a0 = arg0;
    if (arg0 == 0xFF)
    {
        var_a0 = *g_field_script;
    }
    temp_s0 = var_a0 & 0xFF;
    if ((*(s32 *)(D_80122B78 + 0x400) & 0x10000) && temp_s0 < 3)
    {
        func_80087E00(temp_s0, func_800C2928(temp_s0, arg1 & 0xFFFF));
    }
    else
    {
        func_80087CE0(var_a0 & 0xFF);
    }
    *(s32 *)g_field_script = *(s32 *)g_field_script & 0x7FFFFFFF;
}
