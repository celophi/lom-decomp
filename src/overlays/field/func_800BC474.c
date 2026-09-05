#include "common.h"

extern u8 *g_field_script;

s32 func_800C318C(s32 arg0);
void func_8006AD04(s32 arg0, s32 arg1, s32 arg2);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Applies a resolved field value and forwards it to the field command handler.
 *
 * @param arg0 Field selector, or 0xFF to use the active selector.
 * @param arg1 Value passed to func_800C318C.
 */
void func_800BC474(s32 arg0, s32 arg1)
{
    s32 value;

    value = func_800C318C(arg1);
    if (arg0 == 0xFF)
    {
        arg0 = *g_field_script;
        func_8006AD04(arg0, value, 1);
    }
    else
    {
        func_8006AD04(arg0, value, 1);
    }
    func_800BD520(0, 0x2F00, value);
}
