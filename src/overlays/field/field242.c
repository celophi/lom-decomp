#include "common.h"

extern s32 D_80117E6C;

void func_800A1D48(u8 *arg0, void *arg1, s32 arg2)
{
    u8 value = *arg0;

    if (value >= D_80117E6C)
    {
        *arg0 = value - (u8)D_80117E6C;
    }

    func_800A1F2C(*arg0, arg1, arg2);
}
