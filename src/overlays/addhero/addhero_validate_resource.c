#include "common.h"

s32 func_80144140(u8 *base)
{
    if (*(s32 *)(base + 0x33E0) == func_80144194(base))
    {
        if (*(s32 *)(base + 0x33E4) == 0x414E41)
        {
            return 1;
        }
    }
    return 0;
}
