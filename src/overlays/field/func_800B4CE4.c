#include "common.h"

typedef struct UnkStruct800B4CE4
{
    u8 pad0[0x4D];
    u8 values[3];
} UnkStruct800B4CE4;

u32 func_800B4CE4(UnkStruct800B4CE4 *arg0, s32 arg1)
{
    u32 count;
    u32 i;

    i = 0;
    count = i;
    for (; i < 3; i++)
    {
        if (arg0->values[i] == arg1)
        {
            count++;
        }
    }
    return count;
}
