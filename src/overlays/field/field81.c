#include "common.h"

extern s32 D_80122B68[];
extern s32 D_80122B10;
extern s32 D_80122B20;

void func_800B01FC(void)
{
    s32 i = 1;
    s32 *p = &D_80122B68[i];

    D_80122B10 = 0;
    D_80122B20 = 0;

    for (; i >= 0; i--)
    {
        *p = 0;
        p--;
    }
}
