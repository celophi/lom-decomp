#include "field_script.h"

/* Field script opcode handlers 0x00 and 0x01 (see field_script.h). */

/**
 * @brief Opcode 0x00: return to the previous record, or halt when already at the outermost one.
 * @note Ends the step loop unless the popped record's wait word has bit 0 set. At depth 0 it clears the program counter instead.
 */
void field_script_op_00(void)
{
    s32 depth;

    depth = g_field_script->active_record;
    if (depth > 0)
    {
        if ((FIELD_SCRIPT_RECORD_STATE(depth)->wait & 1) == 0)
        {
            g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
        }
        g_field_script->active_record = g_field_script->active_record - 1;
        return;
    }
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = 0;
}

/**
 * @brief Opcode 0x01: unconditional relative branch by the signed halfword after the opcode.
 */
void field_script_op_01(void)
{
    field_script_branch(1);
}
