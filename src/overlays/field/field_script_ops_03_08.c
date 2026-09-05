#include "field_script.h"

/* Field script opcode handlers 0x03 through 0x08 (see field_script.h). */

extern u8* D_80122B78;

void func_800BD6F4(s32 value, u8* params);

/**
 * @brief Opcode 0x03: dispatch the byte operand as a small field command.
 */
void field_script_op_03(void)
{
    s32 i;
    s32 j;
    FieldScriptRecord* rec;
    FieldScriptRecord* rec2;

    rec = (FieldScriptRecord*)g_field_script;
    i = g_field_script->active_record;
    rec += i;
    func_800BD6F4(rec->pc[1], D_80122B78 + 0x24);

    rec2 = (FieldScriptRecord*)g_field_script;
    j = g_field_script->active_record;
    rec2 += j;
    rec2->pc += 2;
}

/**
 * @brief Opcode 0x04: branch when the condition flag is set, otherwise skip the branch operand.
 */
void field_script_op_04(void)
{
    FieldScriptRecordState* rec;

    rec = (FieldScriptRecordState*)((u8*)g_field_script + g_field_script->active_record * 0xC);
    if (rec->flags & FIELD_SCRIPT_COND)
    {
        field_script_branch(1);
        return;
    }
    rec->pc += 3;
}

/**
 * @brief Opcode 0x05: branch when the condition flag is clear, otherwise skip the branch operand.
 */
void field_script_op_05(void)
{
    FieldScriptRecordState* rec;

    rec = (FieldScriptRecordState*)((u8*)g_field_script + g_field_script->active_record * 12);
    if (rec->flags & FIELD_SCRIPT_COND)
    {
        rec->pc += 3;
    }
    else
    {
        field_script_branch(1);
    }
}

/**
 * @brief Opcode 0x06: no operation; step past the opcode.
 */
void field_script_op_06(void)
{
    FieldScriptRecord* rec;

    rec = (FieldScriptRecord*)g_field_script + g_field_script->active_record;
    rec->pc = rec->pc + 1;
}

/**
 * @brief Opcode 0x07: copy flag bit 1 into the condition flag and step past the opcode.
 */
void field_script_op_07(void)
{
    FieldScriptRecordState* rec;
    FieldScriptRecordState* rec2;
    u32 flags;
    u32 masked;

    rec = (FieldScriptRecordState*)((u8*)g_field_script + g_field_script->active_record * 0xC);
    flags = rec->flags;
    masked = flags & ~FIELD_SCRIPT_COND;
    masked |= (flags >> 1) & 1;
    rec->flags = masked;
    rec2 = (FieldScriptRecordState*)((u8*)g_field_script + g_field_script->active_record * 0xC);
    rec2->pc++;
}

/**
 * @brief Opcode 0x08: set the condition flag when a script variable lies within an inclusive range.
 * @note Operands are a halfword variable reference followed by the low and high bounds.
 */
void field_script_op_08(void)
{
    u16 var_ref;
    s32 low;
    s32 high;
    u8 descriptor;
    u8* operands;
    s32 value;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(operands + 2, &var_ref);
    value = func_800BD3B0(g_field_script->status.owner_id, var_ref << 16);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(descriptor, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &low);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(descriptor >> 2, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &high);
    if ((u32)value >= (u32)low && (u32)value <= (u32)high)
    {
        FIELD_SCRIPT_ACTIVE_RECORD_STATE()->flags |= FIELD_SCRIPT_COND;
    }
    else
    {
        FIELD_SCRIPT_ACTIVE_RECORD_STATE()->flags &= ~FIELD_SCRIPT_COND;
    }
}
