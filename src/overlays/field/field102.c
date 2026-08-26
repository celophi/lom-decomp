#include "common.h"

extern u8 *D_80123FB8;

void func_800BC268(s32 arg0)
{
    s32 var_a0;

    var_a0 = arg0;
    if (var_a0 == 0xFF)
    {
        var_a0 = *D_80123FB8;
    }
    func_8008B1C8(var_a0);
}
