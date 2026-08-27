#include "common.h"

void func_800C1F28(u32 *arg0)
{
    u32 i;
    u32 j;
    u32 key;
    u32 data;

    for (i = 0; i < arg0[0]; i++)
    {
        for (j = 1; j < arg0[0]; j++)
        {
            key = arg0[2 * i + 2];
            if (arg0[2 * j + 2] < key)
            {
                data = arg0[2 * i + 1];
                arg0[2 * i + 1] = arg0[2 * j + 1];
                arg0[2 * i + 2] = arg0[2 * j + 2];
                arg0[2 * j + 1] = data;
                arg0[2 * j + 2] = key;
            }
        }
    }
}
