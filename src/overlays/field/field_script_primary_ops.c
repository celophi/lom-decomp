#include "field_script.h"
#include "game_audio.h"

s32 func_8005A84C(s32 arg0, s32 arg1);
s32 func_8008B398(s32 key);
s32 func_8006751C(s32 arg0);

/**
 * @brief Evaluate opcode 0x12's condition selector and advance or pause the active script.
 * @param unused0 Unused value inherited from the opcode dispatcher.
 * @param unused1 Unused value inherited from the opcode dispatcher.
 * @param wait Initial wait condition; valid selectors replace it with the evaluated result.
 */
void func_800B95EC(s32 unused0, s32 unused1, s32 wait)
{
    FieldScriptRecord* rec;
    u8* pc;
    u32 selector;
    s32 operand;
    s32 result;

    pc = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    selector = pc[1];
    operand = pc[2];

    switch (selector)
    {
    case 0:
        result = func_8005A84C(0, operand) ^ 2;
        wait = 0 < (u32)result;
        break;
    case 1:
        result = func_8005A84C(0, operand) ^ 3;
        wait = 0 < (u32)result;
        break;
    case 2:
    {
        s32 key = operand;
        if (operand == 0xFF)
        {
            key = g_field_script->status.owner_id;
        }
        result = func_8008B398(key);
        wait = result < 2;
        break;
    }
    case 3:
    {
        s32 key = operand;
        if (operand == 0xFF)
        {
            key = g_field_script->status.owner_id;
        }
        result = func_8008B398(key) ^ 1;
        wait = 0 < (u32)result;
        break;
    }
    case 4:
    {
        s32 key = operand;
        if (operand == 0xFF)
        {
            key = g_field_script->status.owner_id;
        }
        result = func_8008B398(key);
        wait = 0 < (u32)result;
        break;
    }
    case 5:
        result = func_8006751C(operand & 3) ^ 1;
        wait = 0 < (u32)result;
        break;
    case 6:
        result = func_8006751C(operand & 3);
        wait = 0 < (u32)result;
        break;
    }

    if (wait)
    {
        g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
    }
    else
    {
        rec = FIELD_SCRIPT_RECORD(g_field_script->active_record);
        rec->pc += 3;
    }
}

extern s32 func_800BD414(s32 arg0, s32 arg1);
extern s32 func_8008B398(s32 key);

/**
 * @brief Opcode 0x13: resolve an operand through func_800BD414 and stop the
 *        script this frame if the resolved value maps to a slot below 2.
 */
void func_800B977C(void)
{
    FieldScriptRecord* rec;
    u8* pc;
    s32 flag;
    s32 code;
    s32 result;
    s32 param;
    s32 s1;

    rec = FIELD_SCRIPT_ACTIVE_RECORD();
    pc = rec->pc;
    flag = pc[1];
    code = pc[2] | (pc[3] << 8);
    result = func_800BD414(0, code);
    if (flag != 0)
    {
    }
    else
    {
        param = (result != 0xFF) ? result : g_field_script->status.owner_id;
        s1 = func_8008B398(param) < 2;
    }

    if (s1)
    {
        g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
    }
    else
    {
        FIELD_SCRIPT_ACTIVE_RECORD()->pc += 4;
    }
}

typedef struct
{
    u8 pad0[0x402];
    u16 unk402;
} StructB78Local;

void func_800BD520(s32 arg0, s32 arg1, s32 arg2);

extern s32 D_8011F428;
extern s32 D_801227F0;
extern u8 *D_80122B78;

/**
 * @brief Evaluate the active field-script condition and advance or pause the script record.
 */
void func_800B9868(void)
{
    FieldScriptRecord *rec;
    u8 *pc;
    u32 code;
    s32 flag;
    s32 v0;
    FieldScriptContext *ctx;
    s32 active_record;

    active_record = g_field_script->active_record;
    ctx = g_field_script;
    rec = (FieldScriptRecord *)((u8 *)ctx + ((active_record * 3) << 2));
    pc = rec->pc;
    code = pc[1];
    switch (code)
    {
    case 1:
        flag = ((StructB78Local *)D_80122B78)->unk402 & 1;
        break;
    case 2:
        flag = D_801227F0 != 2;
        break;
    case 3:
        v0 = D_8011F428 ^ 1;
        flag = v0 == 0;
        break;
    case 4:
        v0 = D_8011F428;
        flag = v0 == 0;
        break;
    }
    if (flag != 0)
    {
        g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
        return;
    }
    active_record = g_field_script->active_record;
    ctx = g_field_script;
    rec = (FieldScriptRecord *)((u8 *)ctx + ((active_record * 3) << 2));
    rec->pc += 2;
    if ((u32)(code - 3) < 2)
    {
        func_800BD520(0, 0x7100, D_8011F428);
    }
}

/** @brief Partial StructB800B99A8 layout used by func_800B99A8. */
typedef struct
{
    u8 pad0[0x41C];
    u32 unk41C;
} StructB800B99A8;

extern FieldScriptContext *g_field_script;
extern u8 *D_80122B78;

s32 func_8006751C(s32 arg0);
u8 func_80067598(s32 arg0);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Handle a script mode query, advancing the PC or clearing the run flag.
 * @note A 0xFF operand selects the mode from the shared field state.
 * @note WIP: control-flow and temporary-register differences remain.
 */
void func_800B99A8(void)
{
    FieldScriptContext *rec;
    s32 mode;
    s32 selector;
    s32 result;

    rec = &g_field_script[g_field_script->active_record];
    mode = rec->pc[1];
    if (mode == 0xFF)
    {
        mode = (((StructB800B99A8 *)D_80122B78)->unk41C >> 8) & 3;
    }
    selector = mode & 3;
    result = func_8006751C(selector);
    if (!(mode & 0x80))
    {
        if (result == -1)
        {
            func_800BD520(0, 0x7100, func_80067598(selector));
            goto advance_pc;
        }
        goto clear_flag;
    }
    if (result == 3)
    {
    advance_pc:
        rec = &g_field_script[g_field_script->active_record];
        rec->pc += 2;
        return;
    }
clear_flag:
    g_field_script->status.word &= 0x7FFFFFFF;
}

s32 func_800875C4(s32 arg0, u32 arg1, void* arg2, void* arg3);
extern s32 D_8010AE78;
extern u8* D_80122B74;

/**
 * @brief Resolve an actor operand and dispatch the current field-script command.
 */
void func_800B9AC4(void)
{
    FieldScriptRecord* rec;
    FieldScriptRecord* rec2;
    u8* pc;
    u8 descriptor;
    u8 actor_id;
    u32 masked_id;
    u8* slot;

    rec = FIELD_SCRIPT_ACTIVE_RECORD();
    pc = rec->pc;
    descriptor = pc[1];
    if (descriptor == 0xFF)
    {
        actor_id = g_field_script->status.owner_id;
    }
    else
    {
        actor_id = descriptor;
    }
    masked_id = actor_id & 0xFF;
    if (masked_id < 3)
    {
        slot = D_80122B74 + masked_id * 0x250;
        if (slot[0x5F0] == 0)
        {
            rec->pc = pc + 2;
            return;
        }
        if ((slot[0x608] >> 7) != 0)
        {
            if (D_8010AE78 == 0)
            {
                rec->pc = pc + 2;
                return;
            }
        }
    }
    actor_id++;
    actor_id--;
    if (func_800875C4(actor_id & 0xFF, masked_id, pc, rec) != 0)
    {
        rec2 = FIELD_SCRIPT_ACTIVE_RECORD();
        rec2->pc = rec2->pc + 2;
        return;
    }
    g_field_script->status.word = g_field_script->status.word & 0x7FFFFFFF;
}

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

s32 func_80087F44(s32, s32 *);

/**
 * @brief Opcode 0x1D: write an actor position into three packed script variables.
 * @note Initial nonmatching C; the vertical component is negated.
 */
void func_800B9FF8(void)
{
    u16 reference;
    s32 actor;
    s32 position[4];
    s32 kind;
    s32 high;
    s32 offset;
    u32 packed;
    u8 *pc;

    pc = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(pc[1], pc + 2, &actor);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc, &reference);
    packed = reference & 0xFFFF;
    kind = packed & 0x7000;
    high = (packed >> 15) << 15;
    offset = reference & 0xFFF;
    func_80087F44(actor, position);
    func_800BD434(g_field_script->status.owner_id, reference << 16, position[0]);
    func_800BD434(g_field_script->status.owner_id, (high | kind | ((offset + 0x20) & 0xFFF)) << 16, -position[1]);
    func_800BD434(g_field_script->status.owner_id, (high | kind | ((offset + 0x40) & 0xFFF)) << 16, position[2]);
}

/* Field script opcode handlers 0x1E through 0x36 (see field_script.h). */

extern u8* D_80122B74;
extern u8* D_80122B78;

void func_800B4410(s32 arg0);
void func_800B4584(void);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
void func_8006AB38(s32 arg0);
void func_800A43E8(s32 arg0, s32 arg1, u16 arg2, s32 arg3);
void func_800B286C(s32 arg0, s32 arg1, s32 arg2);
s32 func_800A4744(void);
u8 func_800A4778(void);
void field_text_format_number(s32 window_index, u32 value, u8 digits);
void func_800674D8(s32 arg0);
void func_8008AFD8(s32 arg0, s32 arg1, FieldScriptRecord* record, s32 record_index);
void func_80087614(s32 arg0, s32 arg1);
s32 func_80087D8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_800B2654(s32* arg0, s32* arg1, s32* arg2, s32* arg3);
void func_8009C620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8009C77C(s32 arg0, s32 arg1, s32 arg2);
void func_800A3988(s32 arg0, s32 arg1, s32 arg2, FieldScriptRecord* record);
void func_8008B5D0(s32 arg0, s32 arg1, s32 arg2, s32* arg3);

/**
 * @brief Opcode 0x1E: forward the byte operand to func_800B4410.
 */
void field_script_op_1e(void)
{
    s32 i;
    FieldScriptRecord* rec;

    rec = (FieldScriptRecord*)g_field_script;
    i = g_field_script->active_record;
    rec += i;
    func_800B4410(rec->pc[1]);

    rec = (FieldScriptRecord*)g_field_script;
    i = g_field_script->active_record;
    rec += i;
    rec->pc += 2;
}

/**
 * @brief Opcode 0x1F: call func_800B4584 and step past the opcode.
 */
void field_script_op_1f(void)
{
    FieldScriptRecord* rec;

    func_800B4584();
    rec = (FieldScriptRecord*)g_field_script + g_field_script->active_record;
    rec->pc = rec->pc + 1;
}

/**
 * @brief Opcode 0x20: queue audio sub-command 0x20 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_20(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x20);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x21: queue audio sub-command 0x21 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_21(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x21);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x22: queue audio sub-command 0x23 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter. Shares sub-command 0x23 with opcode 0x23.
 */
s32 field_script_op_22(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x23);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x23: queue audio sub-command 0x23 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_23(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x23);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x24: queue audio sub-command 0x24 for the owner and step past the opcode.
 */
void field_script_op_24(void)
{
    FieldScriptContext* ctx = g_field_script;
    FieldScriptRecord* rec;

    akao_set_song_params(0x8001, 1, ctx->status.owner_id, 0x24);

    rec = (FieldScriptRecord*)g_field_script;
    rec += g_field_script->active_record;
    rec->pc += 1;
}

/**
 * @brief Opcode 0x25: queue audio sub-command 0x25 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_25(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x25);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x26: queue audio sub-command 0x26 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_26(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x26);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x27: reset one of two D_80122B74 slots chosen by the byte operand, then call func_8006AB38.
 * @note Operand 0 selects the slot at 0x840 and also issues the 0xF87-based command; any other value selects 0xA90.
 */
void field_script_op_27(void)
{
    s32 state;

    state = FIELD_SCRIPT_ACTIVE_RECORD()->pc[1];
    if (state == 0)
    {
        D_80122B74[0x840] = 0;
        *(s32*)(D_80122B74 + 0x858) |= 0x7F;
        func_800BD520(0, (D_80122B74[0x859] << 3) + 0xF87, 0);
        func_800BD520(0, 0x2F08, 0xFF);
    }
    else
    {
        D_80122B74[0xA90] = 0;
        *(s32*)(D_80122B74 + 0xAA8) |= 0x7F;
        func_800BD520(0, 0x2F00, 0xFF);
    }
    func_8006AB38(state);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
}

/**
 * @brief Opcode 0x28: read three operands and pass them to func_800A43E8, then claim command slot 0xF.
 * @note The low two descriptor bits are passed as the first argument; a third operand of 0xFF becomes -1.
 */
void field_script_op_28(void)
{
    s32 arg2;
    s32 arg1;
    s32 arg0;
    u32 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_0(descriptor), operands + 2, &arg0);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg2);
    if (arg2 == 0xFF)
    {
        arg2 = -1;
    }
    func_800A43E8(descriptor & 3, arg0, (u16)arg1, arg2);
    func_800B286C(0x80, 0, 0xF);
}

/**
 * @brief Opcode 0x29: issue command 0x7100 with func_800A4744's result, or end the step loop when it is negative.
 * @note On success claims command slot 0x10 and steps past the opcode; on failure claims the slot func_800A4778 names.
 */
void field_script_op_29(void)
{
    s32 result;
    FieldScriptRecord* rec;

    result = func_800A4744();
    if (result < 0)
    {
        func_800B286C(0x80, 0, func_800A4778() & 0xFF);
        g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
        return;
    }
    func_800BD520(0, 0x7100, result);
    func_800B286C(0x80, 0, 0x10);
    rec = FIELD_SCRIPT_ACTIVE_RECORD();
    rec->pc += 1;
}

/**
 * @brief Opcode 0x2A: consume four operands without acting on them.
 * @note A fourth operand of 0xFF is normalised to -1 but the values are otherwise unused.
 */
void field_script_op_2a(void)
{
    s32 arg3;
    s32 arg2;
    s32 arg1;
    s32 arg0;
    u32 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_0(descriptor), operands + 2, &arg0);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg2);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_3(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg3);
    if (arg3 == 0xFF)
    {
        arg3 = -1;
    }
}

/**
 * @brief Opcode 0x2B: format a number into a text window's inline expansion buffer.
 * @note Operands are window index, value and digit count; a zero digit count is replaced by the value's decimal length.
 */
void field_script_op_2b(void)
{
    s32 digits;
    s32 value;
    s32 window_index;
    u32 descriptor;
    u8* operands;
    u32 remaining_value;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_0(descriptor), operands + 2, &window_index);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &value);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &digits);
    if (digits == 0)
    {
        remaining_value = value;
        digits = 1;
        for (;;)
        {
            remaining_value /= 10;
            if (remaining_value == 0)
            {
                break;
            }
            digits++;
        }
    }
    field_text_format_number((u16)window_index, value, (u8)digits);
}

/**
 * @brief Opcode 0x2C: call func_800674D8 with one operand, or with 0 through 3 when the operand has bit 7 set.
 */
void field_script_op_2c(void)
{
    s32 value;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(operands[1] & 3, operands + 2, &value);
    if (value & 0x80)
    {
        value = 0;
        do
        {
            func_800674D8((u16)value);
            value++;
        } while ((u32)value < 4);
    }
    else
    {
        func_800674D8((u16)value);
    }
}

/**
 * @brief Opcode 0x2D: read two owner-substituting operands and pass them with the active record to func_8008AFD8.
 */
void field_script_op_2d(void)
{
    s32 arg0;
    s32 arg1;
    u8 descriptor;
    u8* operands;
    FieldScriptRecord* rec;
    s32 active;
    u8* next;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_0(descriptor), operands + 2, &arg0);
    next = field_script_read_operand_or_owner(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg1);
    active = g_field_script->active_record;
    rec = FIELD_SCRIPT_RECORD(active);
    rec->pc = next;
    func_8008AFD8(arg0, arg1, rec, active);
}

/**
 * @brief Opcode 0x2E: no operation.
 */
void field_script_op_2e(void)
{
}

/**
 * @brief Opcode 0x2F: replace the low four state flags of a field record.
 * @note The first operand selects the record and substitutes the owner for 0xFF; the second is the new flag value.
 */
void field_script_op_2f(void)
{
    s32 selector;
    s32 flags;
    u8 descriptor;
    u8* operands;
    FieldScriptRecord* rec;
    s32 active;
    u8* next;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_0(descriptor), operands + 2, &selector);
    next = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &flags);
    active = g_field_script->active_record;
    rec = FIELD_SCRIPT_RECORD(active);
    rec->pc = next;
    func_80087614(selector, flags);
}

/**
 * @brief Opcode 0x30: queue audio sub-command 0x30 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_30(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x30);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x31: store a scaled position into the actor record matching a key.
 * @note Operands are key (owner substituted for 0xFF), x, y and z.
 */
void field_script_op_31(void)
{
    s32 key;
    s32 x;
    s32 y;
    s32 z;
    u32 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_0(descriptor), operands + 2, &key);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &x);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &y);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_3(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &z);
    func_80087D8C(key, x, y, z);
}

/**
 * @brief Opcode 0x32: read four operands, resolve them through func_800B2654 and pass them to func_8009C620.
 * @note A first operand of 0xFF is replaced by the owner id before resolution.
 */
void field_script_op_32(void)
{
    s32 arg3;
    s32 arg2;
    s32 arg1;
    s32 arg0;
    u32 descriptor;
    u8* operands;
    s32 target;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_0(descriptor), operands + 2, &arg0);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg2);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_3(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg3);
    target = arg0;
    if (target == 0xFF)
    {
        target = g_field_script->status.owner_id;
    }
    arg0 = target;
    func_800B2654(&arg0, &arg1, &arg2, &arg3);
    func_8009C620(arg1, arg3, arg0, arg2);
}

/**
 * @brief Opcode 0x33: read three operands and pass them to func_8009C77C.
 * @note A first operand of 0xFF is replaced by bits 8-9 of the D_80122B78 word at 0x41C.
 */
void field_script_op_33(void)
{
    s32 arg2;
    s32 arg1;
    s32 arg0;
    u32 descriptor;
    u8* operands;
    s32 slot;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_0(descriptor), operands + 2, &arg0);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg2);
    if (arg0 == 0xFF)
    {
        slot = *(u32*)(D_80122B78 + 0x41C) >> 8;
        slot &= 3;
    }
    else
    {
        slot = arg0;
    }
    arg0 = slot;
    func_8009C77C(slot, arg1, arg2);
}

/**
 * @brief Opcode 0x34: play a field sound effect.
 * @note Operand types are taken from the low descriptor bits upward: sound id, pan, then an unused value.
 */
void field_script_op_34(void)
{
    s32 unused;
    s32 pan;
    s32 sound_id;
    u32 descriptor;
    u8* operands;
    FieldScriptRecord* rec;
    u8* next;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_3(descriptor), operands + 2, &sound_id);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &pan);
    next = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &unused);
    rec = FIELD_SCRIPT_ACTIVE_RECORD();
    rec->pc = next;
    func_800A3988(sound_id, pan, unused, rec);
}

/**
 * @brief Opcode 0x35: read two owner-substituting operands and one plain operand, then call func_8008B5D0.
 * @note Operand types are taken from the low descriptor bits upward.
 */
void field_script_op_35(void)
{
    s32 arg2;
    s32 arg1;
    s32 arg0;
    u32 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_3(descriptor), operands + 2, &arg0);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg2);
    func_8008B5D0(arg0, arg2, 1, &arg1);
}

/**
 * @brief Opcode 0x36: queue audio sub-command 0x36 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_36(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x36);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/** @brief One entry of the shop item list built on the stack. */
typedef struct
{
    s16 price;
    s16 pad2;
    s32 scaled;
} ShopItemEntry;

u8 *field_script_read_operand(u32 type, u8 *data, s32 *value);
u8 *func_800C1E40(s32 arg0);
void field_open_shop_mode_1(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Build the current shop item list from field-script operands and open the shop interface.
 */
void func_800BB3D8(void)
{
    u8 *operands;
    u8 descriptor;
    s32 shop_id;
    s32 scale;
    u8 *list_base;
    u8 *header;
    ShopItemEntry local_buf[32];
    u8 *resource;
    s32 i;
    s32 kind;
    u32 lo;
    u32 word;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_0(descriptor), operands + 2, &shop_id);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &scale);

    list_base = func_800C1E40(0xA);
    header = list_base + *(s32 *)(list_base + shop_id * 4 + 4);

    resource = func_800C1E40(5);
    i = 0;

    if (*(s32 *)header != 0)
    {
        do
        {
            kind = *(header + i * 4 + 4);
            word = *(u32 *)(header + i * 4 + 4);
            local_buf[i].pad2 = 0;
            local_buf[i].price = (s16)(kind + ((word << 7) & 0x8000));
            lo = (*(u32 *)(header + i * 4 + 4) >> 9) * scale;
            local_buf[i].scaled = (s32)(lo >> 4);
            i++;
        } while ((u32)i < *(s32 *)header);
    }

    field_open_shop_mode_1(*(s32 *)header, (s32)local_buf, (s32)(resource + 4), 2);
}

/* Field script opcode handlers 0x38 through 0x3F (see field_script.h). */

/**
 * @brief Opcode 0x38: no operation; step past the opcode.
 */
void field_script_op_38(void)
{
    FieldScriptRecord* rec = (FieldScriptRecord*)g_field_script;
    s32 depth = g_field_script->active_record;

    rec += depth;
    rec->pc += 1;
}

/**
 * @brief Opcode 0x39: queue audio sub-command 0x39 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_39(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x39);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3A: queue audio sub-command 0x3A for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3a(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x3A);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3B: queue audio sub-command 0x3B for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3b(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x3B);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3C: queue audio sub-command 0x3C for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3c(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x3C);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3D: queue audio sub-command 0x3D for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3d(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x3D);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3E: queue audio sub-command 0x3E for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3e(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x3E);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3F: queue audio sub-command 0x3F for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3f(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x3F);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/** @brief Reset fields addressed relative to successive 0x60-byte slots. */
typedef struct
{
    u8 pad[0x2F36];
    u16 count;
    s32 flags;
} Reset;
/** @brief Script header and 12-byte slot view used to locate the current bytecode. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    u8 *unk8;
} Script;

extern u8 *D_80122B74;
extern u8 *D_80122B78;
extern void func_800B34D0(s32);
extern void func_800C1230(s32);
extern u8 *func_800C1E40(s32);
extern s32 *func_800C1EC8(s32 *, s32 *, s32);
/** @brief Dispatch a reset command and advance the current script by two bytes. */
void func_800BB7B4(void)
{
    s32 fill;
    s32 mask;
    s32 var_a0;
    s32 index;
    s32 var_a0_2;
    u8 temp_v1;
    Script *temp_a0;
    u8 *temp_v0;
    Script *temp_v1_2;
    u8 *var_v0;
    Reset *var_v1;

    temp_v1_2 = (Script *)g_field_script;
    index = temp_v1_2->unk4;
    temp_v1_2 += index;
    temp_v1 = temp_v1_2->unk8[1];
    switch (temp_v1)
    {
    case 0:
        fill = 0xFFFFFF;
        var_a0 = 0xA;
        var_v0 = D_80122B74 + 0x28;
        do
        {
            *(s32 *)(var_v0 + 0x34) = fill;
            var_a0 -= 1;
            var_v0 -= 4;
        } while (var_a0 >= 0);
        *(s32 *)(D_80122B74 + 0x60) = 0x500;
        *(s32 *)(D_80122B74 + 0x64) = -0x8000;
        temp_a0 = (Script *)g_field_script;
        *(s32 *)(D_80122B74 + 0x68) = 0x803F;
        goto block_7;
    case 1:
        func_800B34D0(1);
        goto block_16;
    case 3:
        fill = (s32)func_800C1E40(6);
        temp_v0 = D_80122B78;
        temp_a0 = (Script *)g_field_script;
        *(s32 *)(temp_v0 + 0xF00) = fill;
    block_7:
        temp_a0 += temp_a0->unk4;
        temp_a0->unk8 = temp_a0->unk8 + 2;
        return;
    case 4:
        func_800C1EC8(0, (s32 *)(D_80122B74 + 0xE4), 0x200);
        goto block_16;
    case 5:
        func_800C1230(0);
        func_800C1230(1);
        func_800C1230(2);
        func_800C1230(3);
        func_800C1230(4);
        goto block_16;
    case 6:
        var_a0 = 0;
        mask = 0x7FFFFFFF;
        var_v1 = (Reset *)D_80122B74;
        do
        {
            var_a0 += 1;
            var_v1->count = 0;
            var_v1->flags = (s32)(var_v1->flags & mask);
            var_v1 = (Reset *)((u8 *)var_v1 + 0x60);
        } while (var_a0 < 5);
        goto block_16;
    case 7:
        var_a0 = 0;
        do
        {
            temp_v0 = D_80122B74 + var_a0;
            var_a0 += 1;
            *(u8 *)(temp_v0 + 0x25E0) = 0x63;
        } while (var_a0 < 0xFD);
        /* fallthrough */
    default:
        goto block_16;
    }
block_16:
    temp_v1_2 = (Script *)g_field_script;
    index = temp_v1_2->unk4;
    temp_v1_2 += index;
    temp_v1_2->unk8 = temp_v1_2->unk8 + 2;
    return;
}
