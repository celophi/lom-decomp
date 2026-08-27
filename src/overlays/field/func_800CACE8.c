#include "common.h"

s32 func_800CA364(s32, s32, s32);

/**
 * @brief Tests three connections associated with connection 23.
 *
 * @param arg0 Value passed as the third argument to each connection query.
 * @return Nonzero when the combined query result is positive.
 */
s32 func_800CACE8(s32 arg0)
{
    s32 result;
    s32 first;

    first = func_800CA364(0x17, 0, arg0);
    result = first + func_800CA364(0x17, 6, arg0);
    result += func_800CA364(0x17, 7, arg0);
    return result > 0;
}
