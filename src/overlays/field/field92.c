#include "common.h"

extern void func_800B28E0(s32, s32, s32);

void func_800B61EC(void)
{
    s32 i;

    i = 0;
    do
    {
        func_800B28E0(i, 0xC, 3);
        i++;
    } while (i < 3);
}
