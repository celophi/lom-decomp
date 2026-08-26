#include "common.h"

extern s32 D_80122B68[];

s32 func_800B0850(void)
{
    s32 i;

    for (i = 0; i < 2; i++)
    {
        if (D_80122B68[i] != 0)
        {
            return 1;
        }
    }

    return 0;
}
