#include "common.h"

extern u8 *g_field_script;

void func_800BC268(s32 arg0)
{
    s32 var_a0;

    var_a0 = arg0;
    if (var_a0 == 0xFF)
    {
        var_a0 = *g_field_script;
    }
    func_8008B1C8(var_a0);
}
