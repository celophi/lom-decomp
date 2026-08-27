#include "common.h"

s32 func_800CA364(s32, s32, s32);

/**
 * @brief Tests three connections associated with the supplied field value.
 *
 * Queries the (15, 15), (21, 15), and (21, 21) pairs and reports whether
 * their combined result is positive.
 *
 * @param arg0 Value passed as the third argument to each connection query.
 * @return Nonzero when the sum of the three query results is positive.
 */
s32 func_800CA6BC(s32 arg0)
{
    s32 result;
    s32 first;

    first = func_800CA364(0xF, 0xF, arg0);
    result = first + func_800CA364(0x15, 0xF, arg0);
    result += func_800CA364(0x15, 0x15, arg0);
    return result > 0;
}
