#include "common.h"

u32 func_800BE644(u32 arg0, u32 arg1)
{
    if (arg1 == 0)
    {
        return 0;
    }
    return arg0 % arg1;
}
