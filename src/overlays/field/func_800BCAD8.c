#include "common.h"

extern u8 *g_field_script;

s32 func_800878B4(s32 arg0);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Resolves a target value and dispatches it through two field helpers.
 *
 * When @p arg1 is the sentinel 0xFF, the value is taken from @c g_field_script[0];
 * otherwise it is @p arg1 itself. The resolved value is passed to func_800878B4
 * and then, together with @p arg0 and that result, to func_800BD520.
 *
 * @param arg0 Forwarded to func_800BD520.
 * @param arg1 Target value, or 0xFF to read the default from @c g_field_script[0].
 */
void func_800BCAD8(s32 arg0, s32 arg1)
{
    s32 v;

    if (arg1 == 0xFF)
    {
        v = g_field_script[0];
    }
    else
    {
        v = arg1;
    }
    func_800BD520(v, arg0, func_800878B4(v));
}
