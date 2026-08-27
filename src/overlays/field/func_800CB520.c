#include "common.h"

s32 func_800CA364(s32, s32, s32);

/**
 * @brief Tests three connections in the seventeen-to-twenty-two group.
 *
 * @param arg0 Value passed as the third argument to each connection query.
 * @return Nonzero when the combined query result is positive.
 */
s32 func_800CB520(s32 arg0)
{
    s32 result;
    s32 first;

    first = func_800CA364(0x11, 0x11, arg0);
    result = first + func_800CA364(0x16, 0x11, arg0);
    result += func_800CA364(0x16, 0x16, arg0);
    return result > 0;
}
