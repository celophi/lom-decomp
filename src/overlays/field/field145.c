#include "common.h"

s32 func_800C0D58(s32 arg0, s32 arg1)
{
    if ((rand() & 0xFF) < arg1)
    {
        return 0x22;
    }

    return 0x21;
}
