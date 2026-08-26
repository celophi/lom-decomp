#include "common.h"

extern s32 D_80122B78;

s32 func_800C1B60(void)
{
    s32 var_v0;

    var_v0 = func_800C1B98();
    if (var_v0 == 0)
    {
        var_v0 = D_80122B78 + 0xE04;
    }
    return var_v0;
}
