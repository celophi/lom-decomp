#include "common.h"
#include "sdk/rand.h"
#include "field_script.h"

/*
 * Two-operand helpers used by the field script interpreter. func_800BE5C8
 * dispatches through the D_800F0E58 table of (s32, s32) functions; the
 * arithmetic, logic, min/max and random helpers below share that signature.
 */

/** @brief Field command record containing a selector and resolved position. */
typedef struct
{
    u16 unk0;
    u16 pad2;
    s32 unk4;
    s32 unk8;
    s32 unkC;
} FieldPositionCommand;

/** @brief Three-component field position returned by func_80087F44. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} FieldPosition;

typedef void (*UnkFunc800F0E58)(s32, s32);

void func_80087F44(s32 index, FieldPosition *position);

extern UnkFunc800F0E58 D_800F0E58[];

/**
 * @brief Resolves a field position and stores it in a command record.
 *
 * @param arg0 Unused command argument.
 * @param command Destination record; selector 0xFF uses the script owner.
 */
void func_800BE550(s32 arg0, FieldPositionCommand *command)
{
    s32 index;
    FieldPosition position;

    if (command->unk0 == 0xFF)
    {
        index = g_field_script->status.owner_id;
    }
    else
    {
        index = command->unk0;
    }

    func_80087F44(index, &position);
    command->unk4 = position.x;
    command->unk8 = -position.y;
    command->unkC = position.z;
}

/**
 * @brief Call entry idx of the D_800F0E58 handler table with two arguments.
 * @param idx Table index.
 * @param arg1 First handler argument.
 * @param arg2 Second handler argument.
 */
void func_800BE5C8(s32 idx, s32 arg1, s32 arg2)
{
    D_800F0E58[idx](arg1, arg2);
}

/**
 * @brief Add two integers.
 * @param a First operand.
 * @param b Second operand.
 * @return a + b.
 */
s32 func_800BE600(s32 a, s32 b)
{
    return a + b;
}

/**
 * @brief Subtract two integers.
 * @param a First operand.
 * @param b Second operand, subtracted from the first.
 * @return a - b.
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

/**
 * @brief Unsigned divide with a -1 result for a zero divisor.
 * @param arg0 Dividend.
 * @param arg1 Divisor.
 * @return arg0 / arg1, or 0xFFFFFFFF when arg1 is 0.
 */
u32 func_800BE620(u32 arg0, u32 arg1)
{
    if (arg1 == 0)
    {
        return -1;
    }
    return arg0 / arg1;
}

/**
 * @brief Unsigned remainder with a 0 result for a zero divisor.
 * @param arg0 Dividend.
 * @param arg1 Divisor.
 * @return arg0 % arg1, or 0 when arg1 is 0.
 */
u32 func_800BE644(u32 arg0, u32 arg1)
{
    if (arg1 == 0)
    {
        return 0;
    }
    return arg0 % arg1;
}

/**
 * @brief Bitwise AND.
 * @param arg0 First operand.
 * @param arg1 Second operand.
 * @return arg0 & arg1.
 */
s32 func_800BE668(s32 arg0, s32 arg1)
{
    return arg0 & arg1;
}

/**
 * @brief Bitwise OR.
 * @param arg0 First operand.
 * @param arg1 Second operand.
 * @return arg0 | arg1.
 */
s32 func_800BE670(s32 arg0, s32 arg1)
{
    return arg0 | arg1;
}

/**
 * @brief Bitwise XOR.
 * @param a First operand.
 * @param b Second operand.
 * @return a ^ b.
 */
s32 func_800BE678(s32 a, s32 b)
{
    return a ^ b;
}

/**
 * @brief Returns a random value bounded by an inclusive maximum.
 *
 * @param maximum Inclusive upper bound. A value of -1 returns the raw random
 *                 value.
 * @return The raw random value for -1, or the random value modulo
 *         `maximum + 1` otherwise.
 */
u32 func_800BE680(s32 maximum)
{
    u32 range = maximum + 1;

    if (range == 0)
    {
        return rand();
    }
    if (range != 0)
    {
        return rand() % range;
    }
}

/**
 * @brief Unsigned maximum.
 * @param arg0 First operand.
 * @param arg1 Second operand.
 * @return The larger of the two.
 */
u32 func_800BE6D0(u32 arg0, u32 arg1)
{
    u32 var_v0 = arg1;

    if (arg0 < var_v0)
    {
        return var_v0;
    }
    return arg0;
}

/**
 * @brief Unsigned minimum.
 * @param arg0 First operand.
 * @param arg1 Second operand.
 * @return The smaller of the two.
 */
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
