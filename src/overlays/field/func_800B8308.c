#include "field_script.h"

extern u8 *func_800B84B4(s32, u8 *, s32 *);

/**
 * @brief Decode four operands and dispatch an extended field-script opcode.
 * @note Descriptor nibbles are materialized before the first decode call.
 * @note Each decoder result updates the active record's program counter.
 * @note GCC 2.8.0 G0: 100% match, 107 instructions (428 bytes).
 */
void func_800B8308(void)
{
    s32 arg0;
    s32 arg1;
    s32 arg2;
    s32 arg3;
    u8 first;
    u8 second;
    s32 opcode;
    s32 high_first;
    s32 low_second;
    s32 high_second;
    u8 *pc;

    pc = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    first = pc[1];
    second = pc[2];
    opcode = pc[0] - 0x80;
    high_first = first >> 4;
    low_second = second & 0xF;
    high_second = second >> 4;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = func_800B84B4(first & 0xF, pc + 3, &arg0);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = func_800B84B4(high_first, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = func_800B84B4(low_second, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg2);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = func_800B84B4(high_second, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg3);
    g_field_script_ext_op_table[opcode](arg0, arg1, arg2, arg3);
}
