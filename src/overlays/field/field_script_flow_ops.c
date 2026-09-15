/* field_script_flow_ops */
#include "field_script.h"
void akao_set_song_params(s32, s32, s32, s32);
void func_800B820C();
void func_800B8308();
void func_800BD520(s32, s32, s32);
u8 *func_800C1B60(s32);
extern void (*g_field_script_op_table[])();

/**
 * @brief Run a field script until it yields, preserving any enclosing script context.
 * @param context Script context to run.
 */
void field_script_run(FieldScriptContext *context)
{
    s32 status;
    u32 opcode_value;
    u32 wait_count;
    u32 wait_state;
    FieldScriptContext *previous_context;
    u8 *pc;
    s32 opcode;
    FieldScriptRecordState *wait_record;
    FieldScriptContext *current_context;
    FieldScriptContext *stop_context;
    FieldScriptRecordState *active_record;
    void (**op_table)();
    s32 pc_advance;
    s32 owner_id;
    s32 song_param;

    previous_context = g_field_script;
    g_field_script = context;
    func_800BD520(g_field_script->status.owner_id, 0xD000, ((u8 *)func_800C1B60(context->status.owner_id))[5]);
    wait_record = FIELD_SCRIPT_ACTIVE_RECORD_STATE();
    wait_state = wait_record->wait;
    wait_count = wait_state >> 1;
    if (wait_count != 0)
    {
        wait_record->wait = (u32)((wait_state & 1) | ((wait_count - 1) * 2));
        goto restore_context;
    }
    goto start_interpreter;

stop_script_primary:
    pc_advance = 1;
    owner_id = current_context->status.owner_id;
    song_param = *pc;
    active_record->pc = pc + pc_advance;
    akao_set_song_params(0x8001, pc_advance, owner_id, song_param);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
    goto restore_context;

stop_script_extended:
    pc_advance = 1;
    owner_id = current_context->status.owner_id;
    song_param = *pc;
    active_record->pc = pc + pc_advance;
    akao_set_song_params(0x8001, pc_advance, owner_id, song_param);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
    goto restore_context;

start_interpreter:
    status = (s32)g_field_script->status.word;
    status |= FIELD_SCRIPT_RUNNING;
    g_field_script->status.word = status;
    if (status < 0)
    {
        op_table = g_field_script_op_table;
        do
        {
            current_context = g_field_script;
            active_record = (FieldScriptRecordState *)((u8 *)current_context + (current_context->active_record * 3 << 2));
            pc = active_record->pc;
            opcode = *pc++;
            pc--;
            opcode_value = opcode & 0xFF;
            if (opcode_value < 0x40U)
            {
                opcode++;
                opcode--;
                op_table[opcode & 0xFF](opcode_value);
                continue;
            }
            if ((u32)((opcode - 0x40) & 0xFF) < 0x40U)
            {
                if (opcode_value < 0x60U)
                {
                    func_800B820C();
                    continue;
                }
                goto stop_script_primary;
            }
            if ((u32)((opcode + 0x80) & 0xFF) < 0x40U)
            {
                if (opcode_value < 0xD0U)
                {
                    func_800B8308();
                    continue;
                }
                goto stop_script_extended;
            }
            if (opcode_value >= 0xC0U)
            {
                pc_advance = 1;
                owner_id = current_context->status.owner_id;
                song_param = *pc;
                active_record->pc = pc + pc_advance;
                akao_set_song_params(0x8001, pc_advance, owner_id, song_param);
                stop_context = g_field_script;
                stop_context->status.word &= ~FIELD_SCRIPT_RUNNING;
            }
        } while ((s32)g_field_script->status.word < 0);
    }

restore_context:
    if (previous_context != NULL)
    {
        g_field_script = previous_context;
    }
}

void akao_set_song_params(s32, s32, s32, s32);

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} SeqRec;

typedef void (*FieldDispatchFn)(s32, s32);


extern FieldDispatchFn D_800F0D48[];

u8 *func_800B84B4(s32 arg0, u8 *arg1, s32 *arg2);

/**
 * @brief Decode two sequence arguments and dispatch through the field handler table.
 *
 * The high nibble is materialized before the first decode call so GCC schedules
 * the shift into that call's delay slot, matching the original sequence decoder.
 */
void func_800B820C(void)
{
    s32 sp10;
    s32 sp14;
    u8 temp_s0;
    u8 *temp_a1;
    SeqRec *temp_a2;
    s32 temp_a3;
    s32 temp_s1;
    s32 r;
    s32 high;

    temp_a1 = (u8 *)((SeqRec *)((u8 *)g_field_script + (((SeqRec *)g_field_script)->unk4 * 3 << 2)))->unk8;
    temp_s0 = temp_a1[1];
    temp_s1 = temp_a1[0] - 0x40;
    high = temp_s0 >> 4;
    ((SeqRec *)((u8 *)g_field_script + (((SeqRec *)g_field_script)->unk4 * 3 << 2)))->unk8 =
        (s32)func_800B84B4(temp_s0 & 0xF, temp_a1 + 2, &sp10);
    r = (s32)func_800B84B4(
        high,
        (u8 *)((SeqRec *)((u8 *)g_field_script + (((SeqRec *)g_field_script)->unk4 * 3 << 2)))->unk8,
        &sp14);
    temp_a3 = ((SeqRec *)g_field_script)->unk4;
    temp_a2 = (SeqRec *)((u8 *)g_field_script + (temp_a3 * 3 << 2));
    temp_a2->unk8 = r;
    D_800F0D48[temp_s1](sp10, sp14);
}

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

s32 func_800BD414(s32 arg0, s32 arg1);

/**
 * @brief Decode a script operand value from a byte stream.
 * @param type Operand encoding type.
 * @param data Pointer to the encoded operand bytes.
 * @param value Receives the decoded value.
 * @return Pointer to the next unread byte.
 */
u8* func_800B84B4(s32 type, u8* data, s32* value)
{
    u8* p;

    p = data;
    switch (type)
    {
    case 0:
        *value = *p;
        return p + 1;
    case 1:
    case 4:
        *value = p[0] + (p[1] << 8);
        return p + 2;
    case 2:
        *value = p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] << 24);
        return p + 4;
    case 3:
        *value = func_800BD414(g_field_script->status.owner_id, p[0] | (p[1] << 8));
        return p + 2;
    case 5:
        *value = (p[0] + (p[1] << 8)) + 0x10000;
        return p + 2;
    case 6:
    case 7:
        break;
    case 8:
        *value = 0;
        break;
    case 9:
        *value = 1;
        break;
    case 10:
        *value = 0xFF;
        break;
    }
    return p;
}

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

/**
 * @brief Push a field-script subroutine record and branch to its target.
 */
void func_800B8684(void)
{
    s32 depth;
    s32 next_depth;

    depth = g_field_script->active_record;
    next_depth = depth + 1;
    g_field_script->active_record = next_depth;
    if (next_depth >= 8)
    {
        FieldScriptRecord* rec;

        akao_set_song_params(0x8001, 2, g_field_script->status.owner_id, 0);
        g_field_script->active_record = 7;
        rec = FIELD_SCRIPT_RECORD(7);
        rec->pc += 3;
        return;
    }

    FIELD_SCRIPT_RECORD(next_depth)->pc = FIELD_SCRIPT_RECORD(depth)->pc;
    FIELD_SCRIPT_ACTIVE_RECORD_STATE()->flags = FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->flags;
    FIELD_SCRIPT_ACTIVE_RECORD_STATE()->wait |= 1;
    FIELD_SCRIPT_ACTIVE_RECORD_STATE()->wait &= 1;
    FIELD_SCRIPT_RECORD(g_field_script->active_record - 1)->pc += 3;
    field_script_branch(1);
}

/* Field script opcode handlers 0x03 through 0x08 (see field_script.h). */

extern s32 D_80122B78;

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
    func_800BD6F4(rec->pc[1], (u8 *)D_80122B78 + 0x24);

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

/** @brief Script sequence record: 12-byte entries indexed by the header's unk4. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    u8 *unk8;
} ScanRecord;



u8 *field_script_read_operand(u32 mode, u8 *pc, s32 *out);

/**
 * @brief Script op: skip forward to the next opcode equal to the operand (or 0xFF).
 *
 * Reads the operand following the current opcode, then advances the active
 * sequence record's pc in 3-byte steps until it lands on an opcode matching
 * either the operand value or the 0xFF terminator, then takes a branch.
 */
void func_800B8B80(void)
{
    volatile s32 value;
    u8 end_op;
    u8 *pc;
    u8 *initial_next;
    u8 op;
    ScanRecord *rec;

    {
        u8 *base;
        s32 index;
        base = *(u8 *volatile *)&g_field_script;
        index = ((ScanRecord *)base)->unk4;
        pc = ((ScanRecord *)(base + (index * 3 << 2)))->unk8;
    }
    initial_next = field_script_read_operand(pc[1] & 3, pc + 2, (s32 *)&value);
    end_op = 0xFF;
    {
        u8 *base;
        s32 index;
        ScanRecord *current;
        base = *(u8 *volatile *)&g_field_script;
        index = ((ScanRecord *)base)->unk4;
        current = (ScanRecord *)(base + (index * 3 << 2));
        current->unk8 = initial_next;
    }
    while (1)
    {
        {
            u8 *base;
            s32 index;
            base = *(u8 *volatile *)&g_field_script;
            index = ((ScanRecord *)base)->unk4;
            rec = (ScanRecord *)(base + (index * 3 << 2));
        }
        pc = rec->unk8;
        op = *pc;
        if (op == end_op || op == value)
        {
            break;
        }
        rec->unk8 = pc + 3;
    }
    field_script_branch(1);
}

/**
 * @brief Opcode 0x0A: wait for the given number of frames, then end the step loop.
 * @note The frame count is stored in the record's wait word above bit 0, which is preserved.
 */
void field_script_op_0a(void)
{
    s32 frames;
    u8* operands;
    FieldScriptRecordState* rec;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(operands[1], operands + 2, &frames);
    rec = FIELD_SCRIPT_ACTIVE_RECORD_STATE();
    rec->wait = (rec->wait & 1) | (frames * 2);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Copy a script-variable value between two script-selected owners.
 */
void func_800B8CFC(void)
{
    s32 source_owner;
    u16 source_ref;
    s32 destination_owner;
    FieldScriptVariableRef destination_ref;
    u8 descriptor;
    u8 *operands;
    s32 value;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc =
        field_script_read_operand_or_owner(OPERAND_TYPE_3(descriptor), operands + 2, &source_owner);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc =
        field_script_read_u16(
            FIELD_SCRIPT_ACTIVE_RECORD()->pc,
            &source_ref);
    do
    {
        value = func_800BD3B0(source_owner, source_ref << 16);
        {
            u8 *destination_operands;
            destination_operands = (u8 *)g_field_script;
            destination_operands += ((FieldScriptRecord *)destination_operands)->unk4 * 3 << 2;
            destination_operands = ((FieldScriptRecord *)destination_operands)->pc;
            FIELD_SCRIPT_ACTIVE_RECORD()->pc =
                field_script_read_operand_or_owner(OPERAND_TYPE_2(descriptor), destination_operands + 2, &destination_owner);
        }
    } while (0);
    {
        void func_800BD434(s32 arg0, FieldScriptVariableRef arg1, s32 arg2);
        u8 *next;
        next = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc, &destination_ref.value);
        FIELD_SCRIPT_ACTIVE_RECORD()->pc = next;
        func_800BD434(destination_owner, destination_ref, value);
    }
}

s32 func_80087F0C(s32);
u8 *func_800C1B60(s32);
s32 func_800B2A9C(s32);
s32 func_800BD650(s32, s32, s32, s32, s32);
void func_800BD55C(s32, s32, s32, s32, s32, s32);
extern s32 D_80122B74, D_80122B78, D_80123FB0, D_80123FC4;

/**
 * @brief Read or write a selected field-script record value.
 */
void func_800B8E84(void)
{
    u32 value;
    u16 destination_ref;
    u32 packed_field;
    s32 target_index;
    s32 selected_base;
    u8 *opcode_pc;
    u8 *operand_pc;
    u8 *selector_pc;
    u32 operand_type;
    s32 base_selector;
    s32 opcode;
    FieldScriptRecord *opcode_record;
    FieldScriptRecord *operand_record;
    FieldScriptRecord *selector_record;

    opcode_record = FIELD_SCRIPT_ACTIVE_RECORD();
    opcode_pc = opcode_record->pc;
    opcode = *opcode_pc;
    opcode_record->pc = (u8 *) (opcode_pc + 1);
    operand_record = FIELD_SCRIPT_ACTIVE_RECORD();
    operand_pc = operand_record->pc;
    operand_type = *operand_pc;
    operand_record->pc = (u8 *) (operand_pc + 1);
    selector_record = FIELD_SCRIPT_ACTIVE_RECORD();
    selector_pc = selector_record->pc;
    base_selector = *selector_pc;
    selector_record->pc = (u8 *) (selector_pc + 1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(operand_type, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &target_index);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(3U, FIELD_SCRIPT_ACTIVE_RECORD()->pc, (s32 *)&packed_field);
    operand_type >>= 2;
    switch (base_selector)
    {
    case 0:
        selected_base = func_80087F0C(target_index);
    default:
        break;
    case 1:
        selected_base = D_80122B78 + 0x400;
        break;
    case 2:
        selected_base = (s32)func_800C1B60(target_index);
        break;
    case 3:
        selected_base = D_80123FC4;
        break;
    case 4:
        selected_base = D_80123FB0;
        break;
    case 5:
        selected_base = func_800B2A9C(target_index);
        break;
    case 6:
    {
        s32 *base_ptr = &D_80122B74;
        s32 offset = target_index * 0x250 + 0x5F0;
        selected_base = *base_ptr + offset;
        break;
    }
    case 7:
        selected_base = D_80122B74;
        break;
    }
    if (opcode == 0xC)
    {
        value = func_800BD650(packed_field >> 0x1E, selected_base, (packed_field >> 0x10) & 0x3FFF, *((u8 *)&packed_field + 1), (s32) (u8) packed_field);
        FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc, &destination_ref);
        func_800BD434(g_field_script->status.owner_id, destination_ref << 0x10, value);
        return;
    }
    {
        u8 *next;
        u8 **pc;
        s32 call_base;
        next = field_script_read_operand(operand_type, FIELD_SCRIPT_ACTIVE_RECORD()->pc, (s32 *)&value);
        call_base = selected_base;
        pc = &FIELD_SCRIPT_ACTIVE_RECORD()->pc;
        *pc = next;
        func_800BD55C(packed_field >> 0x1E, call_base, (packed_field >> 0x10) & 0x3FFF, *((u8 *)&packed_field + 1), (s32) (u8) packed_field, value);
    }
}

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

extern void akao_set_song_params(s32, s32, s32, s32);
extern u8 *func_80087EF0(s32);

/**
 * @brief Push a script record and resolve its new program counter.
 * @note Clamp the depth at seven and report overflow through AKAO.
 * @note Keep both wait-word mask updates and the header's shifted record stride;
 * the target reloads the active depth between stores.
 * @note GCC 2.8.0 G0: 100% match, 105 instructions (420 bytes).
 */
void func_800B9278(void)
{
    u16 operand;
    s32 depth;

    depth = g_field_script->active_record + 1;
    g_field_script->active_record = depth;
    if (depth >= 8)
    {
        g_field_script->active_record = 7;
        akao_set_song_params(0x8001, 2, g_field_script->status.owner_id, 0x4B);
    }
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->pc = FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->pc;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->flags = FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->flags;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->wait |= 1;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->wait &= 1;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->pc = field_script_read_u16(FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->pc + 1, &operand);
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->pc = func_80087EF0(func_800BD3B0(g_field_script->status.owner_id, operand << 16) & 0x7FFF);
}


/* func_800B941C */
#include "field_script.h"

extern u8* func_800C1B60(s32);
extern void func_8009C620(s32, s32, s32, s32);
extern void func_8009C77C(s32, s32, s32);

/**
 * @brief Decode a seven-byte field command and dispatch its action parameters.
 */
void func_800B941C(void)
{
    u8* first_pc;
    u8* command_pc;
    u8* state_pc;
    u8* call_pc;
    s32 owner;
    s32 value;
    s32 flags;
    s32 target;
    u8 mode;
    u8 kind;

    first_pc = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    if (first_pc[1] != 0xFF)
    {
        owner = first_pc[1];
    }
    else
    {
        owner = g_field_script->status.owner_id;
    }

    command_pc = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    mode = command_pc[4];
    value = command_pc[2] + (command_pc[3] << 8);

    switch (mode)
    {
    case 0xFE:
        target = -1;
        break;
    case 0xFF:
        call_pc = func_800C1B60(owner);
        target = -1;
        if (call_pc[1] != mode)
        {
            target = call_pc[1];
        }
        break;
    default:
        target = FIELD_SCRIPT_ACTIVE_RECORD()->pc[4];
        break;
    }

    state_pc = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    kind = state_pc[6];
    flags = state_pc[5];
    if (kind == 7)
    {
        target = -1;
    }
    if (!(flags & 0x80))
    {
        if (flags & 0x40)
        {
            target |= 0x40;
        }
        else
        {
            target |= (flags & 1) << 6;
        }
    }

    func_8009C620(flags & 3, kind, owner, target);
    func_8009C77C(flags, value, 1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 7;
}
