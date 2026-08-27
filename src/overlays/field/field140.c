#include "common.h"

u32 func_800BE6D0(u32 arg0, u32 arg1)
{
    u32 var_v0 = arg1;

    if (arg0 < var_v0)
    {
        return var_v0;
    }
    return arg0;
}

u32 func_800BE6EC(u32 arg0, u32 arg1)
{
    if (arg1 < arg0)
    {
        return arg1;
    }
    return arg0;
}

/**
 * @brief Empty stub function; body is a no-op.
 */
void func_800BE708(void)
{
}
