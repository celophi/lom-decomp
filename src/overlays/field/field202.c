#include "common.h"

extern u8 D_800459AC;
extern s8 D_80122C12;

void func_800C9DB4(void)
{
    D_80122C12 = (s8) ((u8) D_800459AC >> 4);
}
