#include "common.h"

s32 func_800CA364(s32, s32, s32);

/**
 * @brief Tests three low-numbered connections for the supplied field value.
 *
 * @param arg0 Value passed as the third argument to each connection query.
 * @return Nonzero when the combined query result is positive.
 */
s32 func_800CA7A0(s32 arg0)
{
    s32 result;
    s32 first;

    first = func_800CA364(1, 1, arg0);
    result = first + func_800CA364(2, 1, arg0);
    result += func_800CA364(2, 2, arg0);
    return result > 0;
}
