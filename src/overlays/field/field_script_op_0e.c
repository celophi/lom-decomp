#include "field_script.h"

/**
 * @brief Opcode 0x0E: branch through a table of signed halfword offsets indexed by a script variable.
 */
void field_script_op_0e(void)
{
    u16 var_ref;
    s32 depth;
    FieldScriptRecord* rec;
    s32 value;
    u8* next;

    next = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc + 1, &var_ref);
    do
    {
        FIELD_SCRIPT_ACTIVE_RECORD()->pc = next;
    } while (0);
    value = func_800BD3B0(g_field_script->status.owner_id, var_ref << 16, g_field_script);
    depth = g_field_script->active_record;
    rec = FIELD_SCRIPT_RECORD(depth);
    rec->pc += value * 2;
    field_script_branch(0, rec, depth);
}
