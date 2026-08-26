#include "common.h"

extern u8 D_8011F358[];

void func_800A43C0(void)
{
    s32 i = 0x1D;
    u8 *p = &D_8011F358[i];

    for (; i >= 0; i--)
    {
        *p = 0;
        p--;
    }
}
