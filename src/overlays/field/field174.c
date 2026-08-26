#include "common.h"

extern u8 D_800459AE;
extern s16 D_80122C10;

void func_800C61D8(void)
{
    if (D_800459AE >= 0x28)
    {
        D_80122C10 = 1;
    }
    else
    {
        D_80122C10 = 0;
    }
}
