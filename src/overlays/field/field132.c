#include "common.h"

s32 func_800BE600(s32 a, s32 b)
{
    return a + b;
}

/**
 * @brief Subtract two integers.
 * @param a First operand.
 * @param b Second operand, subtracted from the first.
 * @return a - b.
 * @see decomp.me (100%) local match - no scratch link created for this trivial function.
 */
s32 func_800BE608(s32 a, s32 b)
{
    return a - b;
}

/**
 * @brief Multiply two signed 32-bit integers.
 * @param arg0 First operand.
 * @param arg1 Second operand.
 * @return arg0 * arg1.
 */
s32 func_800BE610(s32 arg0, s32 arg1)
{
    return arg0 * arg1;
}

u32 func_800BE620(u32 arg0, u32 arg1)
{
    if (arg1 == 0)
    {
        return -1;
    }
    return arg0 / arg1;
}

u32 func_800BE644(u32 arg0, u32 arg1)
{
    if (arg1 == 0)
    {
        return 0;
    }
    return arg0 % arg1;
}

/**
 * @param arg0 TODO: unknown.
 * @param arg1 TODO: unknown.
 * @return arg0 & arg1.
 * @see decomp.me (100%) N/A -- trivial 2-instruction leaf function, no scratch needed.
 */
s32 func_800BE668(s32 arg0, s32 arg1)
{
    return arg0 & arg1;
}

s32 func_800BE670(s32 arg0, s32 arg1)
{
    return arg0 | arg1;
}

s32 func_800BE678(s32 a, s32 b)
{
    return a ^ b;
}
