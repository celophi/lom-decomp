#include "common.h"
#include "field_calls.h"
#include "sdk/rand.h"

/*
 * Binary operators of the field script calculate opcode (0x1C). The opcode
 * takes the operator index from the high nibble of its descriptor byte and
 * calls field_script_calc, which dispatches through g_field_script_calc_ops.
 * Every operator is called with the two decoded operands; the division,
 * modulo, minimum and maximum operators treat them as unsigned.
 */

/** @brief Number of entries in g_field_script_calc_ops. */
#define FIELD_SCRIPT_CALC_OP_COUNT 12

typedef s32 (*FieldScriptCalcOp)(s32 left, s32 right);

extern FieldScriptCalcOp g_field_script_calc_ops[FIELD_SCRIPT_CALC_OP_COUNT];

/**
 * @brief Apply one of the script calculation operators.
 * @param op Operator index into g_field_script_calc_ops.
 * @param left First operand.
 * @param right Second operand.
 * @return The operator's result.
 */
s32 field_script_calc(s32 op, s32 left, s32 right)
{
    return g_field_script_calc_ops[op](left, right);
}

/**
 * @brief Operator 0: addition.
 * @param left First operand.
 * @param right Second operand.
 * @return left + right.
 */
s32 field_script_calc_add(s32 left, s32 right)
{
    return left + right;
}

/**
 * @brief Operator 1: subtraction.
 * @param left First operand.
 * @param right Second operand.
 * @return left - right.
 */
s32 field_script_calc_sub(s32 left, s32 right)
{
    return left - right;
}

/**
 * @brief Operator 2: multiplication.
 * @param left First operand.
 * @param right Second operand.
 * @return left * right.
 */
s32 field_script_calc_mul(s32 left, s32 right)
{
    return left * right;
}

/**
 * @brief Operator 3: unsigned division.
 * @param left Dividend.
 * @param right Divisor.
 * @return left / right, or 0xFFFFFFFF when @p right is 0.
 */
u32 field_script_calc_div(u32 left, u32 right)
{
    if (right == 0)
    {
        return -1;
    }
    return left / right;
}

/**
 * @brief Operator 4: unsigned remainder.
 * @param left Dividend.
 * @param right Divisor.
 * @return left % right, or 0 when @p right is 0.
 */
u32 field_script_calc_mod(u32 left, u32 right)
{
    if (right == 0)
    {
        return 0;
    }
    return left % right;
}

/**
 * @brief Operator 5: bitwise AND.
 * @param left First operand.
 * @param right Second operand.
 * @return left & right.
 */
s32 field_script_calc_and(s32 left, s32 right)
{
    return left & right;
}

/**
 * @brief Operator 6: bitwise OR.
 * @param left First operand.
 * @param right Second operand.
 * @return left | right.
 */
s32 field_script_calc_or(s32 left, s32 right)
{
    return left | right;
}

/**
 * @brief Operator 7: bitwise XOR.
 * @param left First operand.
 * @param right Second operand.
 * @return left ^ right.
 */
s32 field_script_calc_xor(s32 left, s32 right)
{
    return left ^ right;
}

/**
 * @brief Operator 8: random number up to an inclusive maximum.
 * @param maximum Largest value to return; -1 returns the raw rand() value.
 * @return A random value from 0 to @p maximum.
 * @note Called with two operands like the other operators; the second is ignored.
 */
u32 field_script_calc_random(s32 maximum)
{
    u32 range = maximum + 1;

    if (range != 0)
    {
        return rand() % range;
    }
    return rand();
}

/**
 * @brief Operator 9: unsigned maximum.
 * @param left First operand.
 * @param right Second operand.
 * @return The larger operand.
 */
u32 field_script_calc_max(u32 left, u32 right)
{
    if (left < right)
    {
        return right;
    }
    return left;
}

/**
 * @brief Operator 10: unsigned minimum.
 * @param left First operand.
 * @param right Second operand.
 * @return The smaller operand.
 */
u32 field_script_calc_min(u32 left, u32 right)
{
    if (right < left)
    {
        return right;
    }
    return left;
}

/**
 * @brief Operator 11: does nothing.
 */
void field_script_calc_nop(void)
{
}
