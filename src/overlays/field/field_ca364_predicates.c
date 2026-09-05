#include "common.h"

/*
 * Predicate wrappers over func_800CA364. Each function sums one or more
 * func_800CA364(type, sub, arg0) queries and reports whether the total is
 * positive. Formerly split across field210, func_800CA6BC, field313,
 * func_800CA7A0, field324, field237, func_800CA8F8, field314, func_800CAA58,
 * field238, field315, func_800CAC80, field212, func_800CAE88, field239,
 * field316, field216, field217, field317, field221, field282, field228,
 * field318, func_800CB520, field240 and field319
 * (func_800CA620 .. func_800CB5D4, gcc280_g0).
 */

/**
 * @brief Test whether func_800CA364(0xD, 0xD, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 * @note The shared local reproduces the original single-register argument setup.
 */
s32 func_800CA620(s32 arg0)
{
    s32 val = 0xD;

    return func_800CA364(val, val, arg0) > 0;
}

/**
 * @brief Test whether func_800CA364(0xD, 0xC, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 */
s32 func_800CA648(s32 arg0)
{
    return func_800CA364(0xD, 0xC, arg0) > 0;
}

/**
 * @brief Test whether the (0x17, 0x12) + (0x17, 0x13) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 */
s32 func_800CA670(s32 arg0)
{
    return (func_800CA364(0x17, 0x12, arg0) + func_800CA364(0x17, 0x13, arg0)) > 0;
}

/**
 * @brief Test whether the (0xF, 0xF) + (0x15, 0xF) + (0x15, 0x15) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
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

/**
 * @brief Test whether the four-way (0x12/0x13/0x15) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 * @see decomp.me (100%) TODO
 */
s32 func_800CA724(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0x12, 0xF, arg0);
    sum += func_800CA364(0x13, 0xF, arg0);
    sum += func_800CA364(0x15, 0x12, arg0);
    sum += func_800CA364(0x15, 0x13, arg0);
    return sum > 0;
}

/**
 * @brief Test whether the (1, 1) + (2, 1) + (2, 2) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
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

/**
 * @brief Test whether the six-way (1/2 x 0, 6/7 x 1/2) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 * @see decomp.me (100%) TODO
 */
s32 func_800CA808(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0x1, 0x0, arg0);
    sum += func_800CA364(0x2, 0x0, arg0);
    sum += func_800CA364(0x6, 0x1, arg0);
    sum += func_800CA364(0x7, 0x1, arg0);
    sum += func_800CA364(0x6, 0x2, arg0);
    sum += func_800CA364(0x7, 0x2, arg0);
    return sum > 0;
}

/**
 * @brief Test whether the (0x1A, 1) + (0x1A, 2) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 */
s32 func_800CA8AC(s32 arg0)
{
    return (func_800CA364(0x1A, 0x1, arg0) + func_800CA364(0x1A, 0x2, arg0)) > 0;
}

/**
 * @brief Test whether the (3, 3) + (4, 3) + (4, 4) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 */
s32 func_800CA8F8(s32 arg0)
{
    s32 result;
    s32 first;

    first = func_800CA364(3, 3, arg0);
    result = first + func_800CA364(4, 3, arg0);
    result += func_800CA364(4, 4, arg0);
    return result > 0;
}

/**
 * @brief Test whether the four-way (3/4 x 1/2) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 * @see decomp.me (100%) TODO
 */
s32 func_800CA960(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0x3, 0x1, arg0);
    sum += func_800CA364(0x4, 0x1, arg0);
    sum += func_800CA364(0x3, 0x2, arg0);
    sum += func_800CA364(0x4, 0x2, arg0);
    return sum > 0;
}

/**
 * @brief Test whether the four-way (0xF/0x15 x 3/4) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 * @see decomp.me (100%) TODO
 */
s32 func_800CA9DC(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0xF, 0x3, arg0);
    sum += func_800CA364(0x15, 0x3, arg0);
    sum += func_800CA364(0xF, 0x4, arg0);
    sum += func_800CA364(0x15, 0x4, arg0);
    return sum > 0;
}

/**
 * @brief Test whether the (5, 5) + (9, 5) + (9, 9) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 */
s32 func_800CAA58(s32 arg0)
{
    s32 result;
    s32 first;

    first = func_800CA364(5, 5, arg0);
    result = first + func_800CA364(9, 5, arg0);
    result += func_800CA364(9, 9, arg0);
    return result > 0;
}

/**
 * @brief Test whether the (8, 5) + (9, 8) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 */
s32 func_800CAAC0(s32 arg0)
{
    return (func_800CA364(0x8, 0x5, arg0) + func_800CA364(0x9, 0x8, arg0)) > 0;
}

/**
 * @brief Test whether the four-way (0xC/0xD x 5/9) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 * @see decomp.me (100%) TODO
 */
s32 func_800CAB0C(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0xC, 0x5, arg0);
    sum += func_800CA364(0xD, 0x5, arg0);
    sum += func_800CA364(0xC, 0x9, arg0);
    sum += func_800CA364(0xD, 0x9, arg0);
    return sum > 0;
}

/**
 * @brief Test whether the four-way (0xF/0x15 x 5/9) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 * @see decomp.me (100%) TODO
 */
s32 func_800CAB88(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0xF, 0x5, arg0);
    sum += func_800CA364(0x15, 0x5, arg0);
    sum += func_800CA364(0xF, 0x9, arg0);
    sum += func_800CA364(0x15, 0x9, arg0);
    return sum > 0;
}

/**
 * @brief Test whether the four-way (0/0, 6/6, 7/6, 7/7) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 * @see decomp.me (100%) TODO
 */
s32 func_800CAC04(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0x0, 0x0, arg0);
    sum += func_800CA364(0x6, 0x6, arg0);
    sum += func_800CA364(0x7, 0x6, arg0);
    sum += func_800CA364(0x7, 0x7, arg0);
    return sum > 0;
}

/**
 * @brief Test whether the (8, 0) + (8, 6) + (8, 7) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 */
s32 func_800CAC80(s32 arg0)
{
    s32 result;
    s32 first;

    first = func_800CA364(8, 0, arg0);
    result = first + func_800CA364(8, 6, arg0);
    result += func_800CA364(8, 7, arg0);
    return result > 0;
}

/**
 * @brief Test whether the (0x17, 0) + (0x17, 6) + (0x17, 7) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
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

/**
 * @brief Test whether func_800CA364(8, 8, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 */
s32 func_800CAD50(s32 arg0)
{
    return func_800CA364(8, 8, arg0) > 0;
}

/**
 * @brief Test whether the (0xE, 8) + (0x14, 8) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 */
s32 func_800CAD78(s32 arg0)
{
    return (func_800CA364(0xE, 0x8, arg0) + func_800CA364(0x14, 0x8, arg0)) > 0;
}

/**
 * @brief Test whether func_800CA364(0x17, 8, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 */
s32 func_800CADC4(s32 arg0)
{
    return func_800CA364(0x17, 0x8, arg0) > 0;
}

/**
 * @brief Test whether func_800CA364(0xA, 0xA, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 * @note The shared local reproduces the original single-register argument setup.
 */
s32 func_800CADEC(s32 arg0)
{
    s32 val = 0xA;

    return func_800CA364(val, val, arg0) > 0;
}

/**
 * @brief Test whether func_800CA364(0x1A, 0xA, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 */
s32 func_800CAE14(s32 arg0)
{
    return func_800CA364(0x1A, 0xA, arg0) > 0;
}

/**
 * @brief Test whether the (0xB, 0xA) + (0x10, 0xA) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 */
s32 func_800CAE3C(s32 arg0)
{
    return (func_800CA364(0xB, 0xA, arg0) + func_800CA364(0x10, 0xA, arg0)) > 0;
}

/**
 * @brief Test whether the (0xE, 0xE) + (0x14, 0xE) + (0x14, 0x14) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
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

/**
 * @brief Test whether the (0x1A, 0xE) + (0x1A, 0x14) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 */
s32 func_800CAEF0(s32 arg0)
{
    return (func_800CA364(0x1A, 0xE, arg0) + func_800CA364(0x1A, 0x14, arg0)) > 0;
}

/**
 * @brief Test whether the four-way (0xE/0x14 x 0xC/0xD) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 * @see decomp.me (100%) TODO
 */
s32 func_800CAF3C(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0xE, 0xC, arg0);
    sum += func_800CA364(0x14, 0xC, arg0);
    sum += func_800CA364(0xE, 0xD, arg0);
    sum += func_800CA364(0x14, 0xD, arg0);
    return sum > 0;
}

/**
 * @brief Test whether the four-way (0x11/0x16 x 5/9) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 * @see decomp.me (100%) TODO
 */
s32 func_800CAFB8(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0x11, 0x5, arg0);
    sum += func_800CA364(0x16, 0x5, arg0);
    sum += func_800CA364(0x11, 0x9, arg0);
    sum += func_800CA364(0x16, 0x9, arg0);
    return sum > 0;
}

/**
 * @brief Stub predicate that always reports no match.
 * @return Always 0.
 * @see decomp.me (100%) N/A -- trivial 2-instruction leaf function, no scratch needed.
 */
s32 func_800CB034(void)
{
    return 0;
}

/**
 * @brief Test whether func_800CA364(0x17, 0x17, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 */
s32 func_800CB03C(s32 arg0)
{
    return func_800CA364(0x17, 0x17, arg0) > 0;
}

/**
 * @brief Test whether func_800CA364(0x18, 0x17, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 */
s32 func_800CB064(s32 arg0)
{
    return func_800CA364(0x18, 0x17, arg0) > 0;
}

/**
 * @brief Test whether the (0x17, 0xC) + (0x17, 0xD) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 */
s32 func_800CB08C(s32 arg0)
{
    return (func_800CA364(0x17, 0xC, arg0) + func_800CA364(0x17, 0xD, arg0)) > 0;
}

/**
 * @brief Test whether func_800CA364(0x19, 0x19, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 * @note The shared local reproduces the original single-register argument setup.
 */
s32 func_800CB0D8(s32 arg0)
{
    s32 val = 0x19;

    return func_800CA364(val, val, arg0) > 0;
}

/**
 * @brief Test whether func_800CA364(0x19, 0xA, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 */
s32 func_800CB100(s32 arg0)
{
    return func_800CA364(0x19, 0xA, arg0) > 0;
}

/**
 * @brief Test whether the (0x1A, 0xF) + (0x19, 0x18) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 */
s32 func_800CB128(s32 arg0)
{
    return (func_800CA364(0x1A, 0xF, arg0) + func_800CA364(0x19, 0x18, arg0)) > 0;
}

/**
 * @brief Test whether the four-way (0x12/0x13/0x14) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 * @see decomp.me (100%) TODO
 */
s32 func_800CB174(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0x12, 0xE, arg0);
    sum += func_800CA364(0x13, 0xE, arg0);
    sum += func_800CA364(0x14, 0x12, arg0);
    sum += func_800CA364(0x14, 0x13, arg0);
    return sum > 0;
}

/**
 * @brief Test whether the (0x18, 0xC) + (0x18, 0xD) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 */
s32 func_800CB1F0(s32 arg0)
{
    return (func_800CA364(0x18, 0xC, arg0) + func_800CA364(0x18, 0xD, arg0)) > 0;
}

/**
 * @brief Test whether func_800CA364(0x1A, 0xD, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 */
s32 func_800CB23C(s32 arg0)
{
    return func_800CA364(0x1A, 0xD, arg0) > 0;
}

/**
 * @brief Test whether func_800CA364(0x1A, 0x1A, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 * @note The shared local reproduces the original single-register argument setup.
 */
s32 func_800CB264(s32 arg0)
{
    s32 val = 0x1A;

    return func_800CA364(val, val, arg0) > 0;
}

/**
 * @brief Test whether func_800CA364(0x18, 0x18, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 */
s32 func_800CB28C(s32 arg0)
{
    return func_800CA364(0x18, 0x18, arg0) > 0;
}

/**
 * @brief Test whether func_800CA364(0x1A, 0x18, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 */
s32 func_800CB2B4(s32 arg0)
{
    return func_800CA364(0x1A, 0x18, arg0) > 0;
}

/**
 * @brief Test whether the (0x19, 0xC) + (0x19, 0xD) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 */
s32 func_800CB2DC(s32 arg0)
{
    return (func_800CA364(0x19, 0xC, arg0) + func_800CA364(0x19, 0xD, arg0)) > 0;
}

/**
 * @brief Test whether func_800CA364(0x18, 0x14, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 */
s32 func_800CB328(s32 arg0)
{
    return func_800CA364(0x18, 0x14, arg0) > 0;
}

/**
 * @brief Test whether func_800CA364(0x19, 0x14, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 */
s32 func_800CB350(s32 arg0)
{
    return func_800CA364(0x19, 0x14, arg0) > 0;
}

/**
 * @brief Test whether func_800CA364(0xC, 0xC, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 */
s32 func_800CB378(s32 arg0)
{
    return func_800CA364(0xC, 0xC, arg0) > 0;
}

/**
 * @brief Test whether the five-way (0xC/0xD/0x10) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 */
s32 func_800CB3A0(s32 arg0)
{
    s32 sum;

    sum = func_800CA364(0xC, 0xB, arg0) + func_800CA364(0xD, 0xB, arg0);
    sum += func_800CA364(0x10, 0xB, arg0);
    sum += func_800CA364(0x10, 0xC, arg0);
    sum += func_800CA364(0x10, 0xD, arg0);
    return sum > 0;
}

/**
 * @brief Test whether the (0x18, 0xB) + (0x18, 0x10) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 */
s32 func_800CB430(s32 arg0)
{
    return (func_800CA364(0x18, 0xB, arg0) + func_800CA364(0x18, 0x10, arg0)) > 0;
}

/**
 * @brief Test whether func_800CA364(0x1A, 0x19, arg0) is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the query result is greater than 0, otherwise 0.
 */
s32 func_800CB47C(s32 arg0)
{
    return func_800CA364(0x1A, 0x19, arg0) > 0;
}

/**
 * @brief Test whether the four-way (0xE/0x14/0x10) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 * @see decomp.me (100%) TODO
 */
s32 func_800CB4A4(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0xE, 0xB, arg0);
    sum += func_800CA364(0x14, 0xB, arg0);
    sum += func_800CA364(0x10, 0xE, arg0);
    sum += func_800CA364(0x14, 0x10, arg0);
    return sum > 0;
}

/**
 * @brief Test whether the (0x11, 0x11) + (0x16, 0x11) + (0x16, 0x16) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
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

/**
 * @brief Test whether the (0x11, 0xA) + (0x16, 0xA) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 */
s32 func_800CB588(s32 arg0)
{
    return (func_800CA364(0x11, 0xA, arg0) + func_800CA364(0x16, 0xA, arg0)) > 0;
}

/**
 * @brief Test whether the four-way (0x11/0x15/0x16) query sum is positive.
 * @param arg0 Value forwarded as the third argument to func_800CA364.
 * @return 1 if the summed result is greater than 0, otherwise 0.
 * @see decomp.me (100%) TODO
 */
s32 func_800CB5D4(s32 arg0)
{
    s32 sum;

    sum = 0;
    sum += func_800CA364(0x11, 0xF, arg0);
    sum += func_800CA364(0x16, 0xF, arg0);
    sum += func_800CA364(0x15, 0x11, arg0);
    sum += func_800CA364(0x16, 0x15, arg0);
    return sum > 0;
}
