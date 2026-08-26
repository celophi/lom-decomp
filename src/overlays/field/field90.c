#include "common.h"

void func_800B4F38(u8 *arg0)
{
    s32 i;
    u8 *p;
    u8 current;
    u8 target;

    for (i = 0; i < 8; i++)
    {
        p = &arg0[i];
        current = p[0x28];
        target = p[0x30];
        if (target < current)
        {
            p[0x28] = current - 1;
        }
        else if (current < target)
        {
            p[0x28] = current + 1;
        }
    }
}
