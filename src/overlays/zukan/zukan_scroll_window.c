#include "common.h"

extern s32 D_80157D60;
extern s32 D_80157D70;
extern s32 D_80157D74;
extern s32 D_80157D68;

void func_80141740(void)
{
    s32 value = D_80157D60 << 4;
    s32 delta = value - D_80157D70;

    if (delta < 0)
    {
        D_80157D74 = value;
        D_80157D68 = 4;
    }
    else if (delta > 0x70)
    {
        D_80157D74 = value - 0x70;
        D_80157D68 = 4;
    }
}
