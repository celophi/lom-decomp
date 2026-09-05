#include "field_script.h"
#include "game_audio.h"

/* Field script opcode handlers 0x17 through 0x1C (see field_script.h). */

void func_8009AFBC(s32 arg0);
void func_800A3938(s32 sound_id, s32 pan);
void func_80087FC0(s32 arg0, u8 arg1, u8* arg2);
s32 func_800BE5C8(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Opcode 0x17: queue audio sub-command 0x17 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_17(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x17);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x18: queue a CD seek for the resource entry named by the halfword operand.
 */
void field_script_op_18(void)
{
    FieldScriptRecord* rec;
    s32 depth;
    s32 entry;

    depth = g_field_script->active_record;
    rec = (FieldScriptRecord*)((u8*)g_field_script + depth * 0xC);
    entry = rec->pc[1] + (rec->pc[2] << 8);
    func_8009AFBC(entry & 0x7FFF);
    depth = g_field_script->active_record;
    rec = (FieldScriptRecord*)((u8*)g_field_script + depth * 0xC);
    rec->pc += 3;
}

/**
 * @brief Opcode 0x19: play a sound effect from two byte operands, sound id then pan.
 */
void field_script_op_19(void)
{
    s32 i;
    FieldScriptRecord* rec;

    rec = (FieldScriptRecord*)g_field_script;
    i = g_field_script->active_record;
    rec += i;
    func_800A3938(rec->pc[1], rec->pc[2]);

    rec = (FieldScriptRecord*)g_field_script;
    i = g_field_script->active_record;
    rec += i;
    rec->pc += 3;
}

/**
 * @brief Opcode 0x1A: pass two byte operands to func_80087FC0, substituting the owner id for 0xFF.
 */
void field_script_op_1a(void)
{
    FieldScriptContext* base;
    u8* p;
    s32 i;
    s32 cmd;

    base = g_field_script;
    i = base->active_record;
    p = FIELD_SCRIPT_RECORD(i)->pc;
    i = p[1];
    cmd = (short)i;
    if (i == 0xFF)
    {
        cmd = base->status.owner_id;
    }
    func_80087FC0(cmd & 0xFF, p[2], p);

    {
        FieldScriptRecord* rec;
        s32 j;
        rec = (FieldScriptRecord*)g_field_script;
        j = g_field_script->active_record;
        rec += j;
        rec->pc += 3;
    }
}

/**
 * @brief Opcode 0x1B: store an operand into a script variable.
 * @note Operands are a halfword variable reference followed by the value.
 */
void field_script_op_1b(void)
{
    u16 var_ref;
    s32 value;
    u8 descriptor;
    u8* operands;
    FieldScriptRecord* rec;
    s32 depth;
    u8* next;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(operands + 2, &var_ref);
    next = field_script_read_operand(descriptor, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &value);
    depth = g_field_script->active_record;
    rec = FIELD_SCRIPT_RECORD(depth);
    rec->pc = next;
    func_800BD434(g_field_script->status.owner_id, var_ref << 16, value);
}

/**
 * @brief Opcode 0x1C: combine two operands through func_800BE5C8 and store the result in a script variable.
 * @note The operation selector is the top two bits of the descriptor; the variable reference follows the operands.
 */
void field_script_op_1c(void)
{
    s32 arg0;
    s32 arg1;
    u16 var_ref;
    u32 descriptor;
    u8* operands;
    s32 result;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = operands + 2;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(descriptor, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg0);
    descriptor >>= 2;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(descriptor, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg1);
    result = func_800BE5C8(descriptor >> 2, arg0, arg1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc, &var_ref);
    func_800BD434(g_field_script->status.owner_id, var_ref << 16, result);
}
