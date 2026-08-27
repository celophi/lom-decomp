#include "common.h"

s32 func_800CA364(s32, s32, s32);

/**
 * @brief Tests three connections in the fourteen-to-twenty group.
 *
 * @param arg0 Value passed as the third argument to each connection query.
 * @return Nonzero when the combined query result is positive.
 */
s32 func_800CAE88(s32 arg0)
{
    s32 result;
    s32 first;

    first = func_800CA364(0xE, 0xE, arg0);
    result = first + func_800CA364(0x14, 0xE, arg0);
    result += func_800CA364(0x14, 0x14, arg0);
    return result > 0;
}
