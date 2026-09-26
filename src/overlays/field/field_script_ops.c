#include "game_audio.h"
#include "field_scene_transition.h"
#include "field_text.h"
#include "main.h"
#include "common.h"
#include "field_actor_runtime.h"
#include "field_calls.h"

#include "field_script.h"
#include "field_records.h"
#include "shop.h"

/** @brief g_field_game_state viewed as the game state it points at. */
#define FIELD_GAME ((FieldGameState*)g_field_game_state)

/** @brief g_field_game_state viewed as the saved game workspace it points at (g_saved_game). */
#define FIELD_SAVED ((SavedGame*)g_field_game_state)

/** @brief g_field_runtime viewed as the field runtime context it points at. */
#define FIELD_RUNTIME ((FieldRuntimeContext*)g_field_runtime)

/** @brief FieldGameState.lands viewed as FieldLandWords. */
#define FIELD_LAND_WORDS ((FieldLandWords*)FIELD_GAME->lands)

typedef void (*FieldDispatchFn)(s32, s32);

typedef struct
{
    s32 actor;
    u16 reference;
} FieldScriptPositionOperands;

/**
 * @brief Record field selector decoded by opcodes 0x0C and 0x0D.
 * @note Bits 30-31: element width; bits 16-29: element index; byte 1: bit shift; byte 0: bit count.
 */
typedef union
{
    u32 word;
    u8 bytes[4];
} FieldScriptFieldSpec;

/** @brief One packed shop list entry. */
typedef union
{
    u32 word;
    u8 item;
    struct
    {
        /** @brief Item type, or the item record index when @c generated is set. */
        unsigned item : 8;
        /** @brief Set when the entry names a record of FieldItemResource whose price is computed. */
        unsigned generated : 1;
        /** @brief Base price of a plain item. */
        unsigned price : 23;
    } bits;
} FieldShopListEntry;

/** @brief One shop list: a count and that many packed entries. */
typedef struct
{
    u32 count;
    FieldShopListEntry entries[1];
} FieldShopList;

/** @brief Resource 0xA (func_800C1E40): byte offsets of each FieldShopList from the resource start. */
typedef struct
{
    u32 header;
    u32 offsets[1];
} FieldShopListTable;

/** @brief Resource 5 (func_800C1E40): the item records a shop can generate. */
typedef struct
{
    u32 header;
    FieldItemRecord records[1];
} FieldItemResource;

/*
 * Helpers reached from field script handlers. Most take an actor id where
 * 0xFF means the script owner; the owner id is byte 0 of g_field_script.
 */

/*
 * Extended field script opcodes 0x80 through 0x8F.
 *
 * field_script_run hands opcodes of 0x80 and above to func_800B8308, which
 * decodes up to four operands from the descriptor bytes that follow the opcode
 * and jumps through g_field_script_ext_op_table[opcode - 0x80]. Every handler
 * here receives those decoded operands in order. An operand of 0xFF in an
 * actor-id slot means the script owner.
 */

void func_800B820C();
void func_800B8308();
void func_800BD520(s32, s32, s32);
u8* field_find_actor_record_or_default(s32);
extern void (*g_field_script_op_table[])();
extern FieldDispatchFn D_800F0D48[];
u8* func_800B84B4(s32 arg0, u8* arg1, s32* arg2);
s32 func_800BD414(s32 arg0, s32 arg1);
extern u8* g_field_runtime;
s32 field_find_object_state(s32);
s32 func_800BD650(s32, s32, s32, s32, s32);
void func_800BD55C(s32, s32, s32, s32, s32, s32);
extern u8* g_field_game_state;
extern s32 g_field_battle, D_80123FC4;
extern u8* field_get_event_script(s32);
s32 field_read_actor_binding_state(s32 key);
extern s32 D_8011F428;
extern s32 D_801227F0;
s32 field_is_actor_idle(s32 actor);
extern s32 g_field_interaction_active;
s32 field_set_actor_control_mode(s32 key, s32 mode);
s32 field_get_actor_position(s32, s32*);
/* Int parameters on purpose: with the (s32, u8, s8) definition the calls would narrow their arguments. */
s32 field_queue_actor_event(s32 owner_id, s32 event_id, s32 argument);
void field_face_actor(s32 arg0, s32 arg1, FieldScriptRecord* record, s32 record_index);
s32 field_set_actor_position(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void field_spawn_targeted_animation_actor(s32 arg0, s32 arg1, s32 arg2, s32* arg3);
u8* func_800C1E40(s32 arg0);
extern void func_800B34D0(s32);
extern extern s32* func_800C1EC8(s32*, s32*, s32);
void akao_cmd_f1(void);
void akao_stop_song(s32);
void func_8005B0F4(s32, s32);
void field_start_actor_turn(s32);
void field_toggle_actor_hidden(s32);
/* Local: field_contact_geometry.c calls it with a third argument, so it stays out of field_calls.h. */
s32 field_start_interaction(s32 actor_id, s32 script);
void func_800BCCE0();
void field_stop_actor_script(s32, s32);
void field_stop_non_script_actors(void);
void func_800C1E08(void);
void func_800C299C(s32);
s32 func_800C29CC(s32);
void func_800C2A88(s32);
extern s32 g_field_hide_actor_panels, g_field_duel_mode, g_field_pair_indicators_disabled, D_80122980;
extern s32 g_gosub_result_count, g_gosub_result_values;
void field_load_bound_animation(s32 arg0, s32 arg1);
void field_spawn_shared_animation_actor(s32 arg0, s32 arg1);
void field_retire_actor(s32 arg0, s32 arg1);
void akao_cmd_a9(s32 arg0, s32 arg1);
void field_revive_actor(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void field_script_op_00(void);

/**
 * @brief Run a field script until it yields, preserving any enclosing script context.
 * @param context Script context to run.
 */
void field_script_run(FieldScriptContext* context)
{
    s32 status;
    u32 wait_count;
    u32 wait_state;
    FieldScriptContext* previous_context;
    u8* pc;
    u8 opcode;
    FieldScriptRecordState* wait_record;
    FieldScriptContext* current_context;
    FieldScriptRecordState* active_record;
    s32 step;
    s32 owner_id;
    s32 invalid_opcode;

    previous_context = g_field_script;
    g_field_script = context;
    func_800BD520(g_field_script->status.owner_id, 0xD000, ((u8*)field_find_actor_record_or_default(context->status.owner_id))[5]);
    wait_record = FIELD_SCRIPT_ACTIVE_RECORD_STATE();
    wait_state = wait_record->wait;
    wait_count = wait_state >> 1;
    if (wait_count != 0)
    {
        wait_record->wait = (u32)((wait_state & 1) | ((wait_count - 1) * 2));
    }
    else
    {
        status = (s32)g_field_script->status.word;
        status |= FIELD_SCRIPT_RUNNING;
        g_field_script->status.word = status;
        if (status < 0)
        {
            do
            {
                current_context = g_field_script;
                active_record = (FieldScriptRecordState*)((u8*)current_context + (current_context->active_record * 3 << 2));
                pc = active_record->pc;
                opcode = *pc;
                if (opcode < 0x40)
                {
                    g_field_script_op_table[*pc]();
                }
                else if (opcode >= 0x40 && opcode < 0x80)
                {
                    if (opcode >= 0x60)
                    {
                        step = 1;
                        owner_id = current_context->status.owner_id;
                        invalid_opcode = *pc;
                        active_record->pc = pc + step;
                        record_game_diagnostic(0x8001, step, owner_id, invalid_opcode);
                        g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
                        break;
                    }
                    func_800B820C();
                }
                else if (opcode >= 0x80 && opcode < 0xC0)
                {
                    if (opcode >= 0xD0)
                    {
                        step = 1;
                        owner_id = current_context->status.owner_id;
                        invalid_opcode = *pc;
                        active_record->pc = pc + step;
                        record_game_diagnostic(0x8001, step, owner_id, invalid_opcode);
                        g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
                        break;
                    }
                    func_800B8308();
                }
                else if (opcode >= 0xC0)
                {
                    step = 1;
                    owner_id = current_context->status.owner_id;
                    invalid_opcode = *pc;
                    active_record->pc = pc + step;
                    record_game_diagnostic(0x8001, step, owner_id, invalid_opcode);
                    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
                }
            } while ((s32)g_field_script->status.word < 0);
        }
    }
    if (previous_context != NULL)
    {
        g_field_script = previous_context;
    }
}

/**
 * @brief Decode two operands and dispatch a field-script opcode in the 0x40 to 0x5F range.
 * @note Jumps through D_800F0D48[opcode - 0x40] with the decoded operands.
 */
void func_800B820C(void)
{
    s32 arg0;
    s32 arg1;
    u8 descriptor;
    u8* pc;
    s32 opcode;
    s32 high;

    pc = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = pc[1];
    opcode = pc[0] - 0x40;
    high = descriptor >> 4;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = func_800B84B4(descriptor & 0xF, pc + 2, &arg0);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = func_800B84B4(high, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &arg1);
    D_800F0D48[opcode](arg0, arg1);
}

/**
 * @brief Decode four operands and dispatch an extended field-script opcode.
 * @note Descriptor nibbles are materialized before the first decode call.
 * @note Each decoder result updates the active record's program counter.
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
    u8* pc;

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

        record_game_diagnostic(0x8001, 2, g_field_script->status.owner_id, 0);
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

/**
 * @brief Opcode 0x03: dispatch the byte operand as a small field command.
 */
void field_script_op_03(void)
{
    func_800BD6F4(FIELD_SCRIPT_ACTIVE_RECORD()->pc[1], (u8*)g_field_runtime + 0x24);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
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
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 1;
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

/**
 * @brief Skip forward to the next opcode equal to the operand (or 0xFF), then take a branch.
 * @note Scans the active record's pc in 3-byte steps.
 */
void func_800B8B80(void)
{
    s32 value;
    u8* pc;
    u8 op;
    u8** pc_slot;

    pc = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(pc[1] & 3, pc + 2, &value);
    while (1)
    {
        pc_slot = &FIELD_SCRIPT_ACTIVE_RECORD()->pc;
        pc = *pc_slot;
        op = *pc;
        if (op == 0xFF || op == value)
        {
            break;
        }
        *pc_slot = pc + 3;
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
 * @brief Copy a script variable from one owner to another.
 * @note Operands: source owner, source variable reference, destination owner, destination reference.
 * @note The block-scope prototype stands in for the unprototyped field_script.h declaration.
 */
void func_800B8CFC(void)
{
    s32 source_owner;
    FieldScriptVariableRef source_ref;
    s32 destination_owner;
    FieldScriptVariableRef destination_ref;
    u8 descriptor;
    u8* operands;
    s32 value;
    void func_800BD434(s32 owner, FieldScriptVariableRef ref, s32 new_value);

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_3(descriptor), operands + 2, &source_owner);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc, &source_ref.value);
    value = func_800BD3B0(source_owner, source_ref.value << 16);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc + 2, &destination_owner);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc, &destination_ref.value);
    func_800BD434(destination_owner, destination_ref, value);
}

/**
 * @brief Opcodes 0x0C and 0x0D: read (0x0C) or write (0x0D) a bitfield in a selected game record.
 * @note Operands: descriptor, base selector (0 to 7), record index, field spec, then the
 *       destination variable (0x0C) or the value to write (0x0D).
 */
void func_800B8E84(void)
{
    u32 value;
    u16 destination_ref;
    FieldScriptFieldSpec field;
    s32 target_index;
    s32 selected_base;
    u32 operand_type;
    s32 base_selector;
    s32 opcode;

    opcode = *FIELD_SCRIPT_ACTIVE_RECORD()->pc++;
    operand_type = *FIELD_SCRIPT_ACTIVE_RECORD()->pc++;
    base_selector = *FIELD_SCRIPT_ACTIVE_RECORD()->pc++;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(operand_type, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &target_index);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(3U, FIELD_SCRIPT_ACTIVE_RECORD()->pc, (s32*)&field.word);
    operand_type >>= 2;
    switch (base_selector)
    {
    case 0:
        selected_base = field_find_object_state(target_index);
    default:
        break;
    case 1:
        selected_base = (s32)&FIELD_RUNTIME->state;
        break;
    case 2:
        selected_base = (s32)field_find_actor_record_or_default(target_index);
        break;
    case 3:
        selected_base = D_80123FC4;
        break;
    case 4:
        selected_base = g_field_battle;
        break;
    case 5:
        selected_base = (s32)func_800B2A9C(target_index);
        break;
    case 6:
        selected_base = (s32)&FIELD_GAME->characters[target_index];
        break;
    case 7:
        selected_base = (s32)g_field_game_state;
        break;
    }
    if (opcode == 0xC)
    {
        value = func_800BD650(field.word >> 30, selected_base, (field.word >> 16) & 0x3FFF, field.bytes[1], field.bytes[0]);
        FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc, &destination_ref);
        func_800BD434(g_field_script->status.owner_id, destination_ref << 0x10, value);
        return;
    }
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(operand_type, FIELD_SCRIPT_ACTIVE_RECORD()->pc, (s32*)&value);
    func_800BD55C(field.word >> 30, selected_base, (field.word >> 16) & 0x3FFF, field.bytes[1], field.bytes[0], value);
}

/**
 * @brief Opcode 0x0E: branch through a table of signed halfword offsets indexed by a script variable.
 */
void field_script_op_0e(void)
{
    FieldScriptVariableRef var_ref;
    s32 depth;
    FieldScriptRecord* rec;
    s32 value;

    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc + 1, &var_ref.value);
    value = func_800BD3B0(g_field_script->status.owner_id, var_ref.value << 16);
    depth = g_field_script->active_record;
    rec = FIELD_SCRIPT_RECORD(depth);
    rec->pc += value * 2;
    field_script_branch(0);
}

/**
 * @brief Push a script record and resolve its new program counter.
 * @note Clamp the depth at seven and record a diagnostic on overflow.
 * @note Sets then masks the new record's wait word, keeping only bit 0.
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
        record_game_diagnostic(0x8001, 2, g_field_script->status.owner_id, 0x4B);
    }
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->pc = FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->pc;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->flags = FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->flags;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->wait |= 1;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->wait &= 1;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->pc =
        field_script_read_u16(FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->pc + 1, &operand);
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->pc = field_get_event_script(func_800BD3B0(g_field_script->status.owner_id, operand << 16) & 0x7FFF);
}

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
        call_pc = field_find_actor_record_or_default(owner);
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
        result = field_read_actor_binding_state(key);
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
        result = field_read_actor_binding_state(key) ^ 1;
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
        result = field_read_actor_binding_state(key);
        wait = 0 < (u32)result;
        break;
    }
    case 5:
        result = field_text_get_status(operand & 3) ^ 1;
        wait = 0 < (u32)result;
        break;
    case 6:
        result = field_text_get_status(operand & 3);
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

/**
 * @brief Opcode 0x13: end the step loop while the actor named by a script variable is still busy.
 * @note Selector 0 is the only one handled; any other selector tests an uninitialized value.
 */
void func_800B977C(void)
{
    u8* pc;
    s32 selector;
    s32 actor;
    s32 wait;

    pc = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    selector = pc[1];
    actor = func_800BD414(0, pc[2] | (pc[3] << 8));
    switch (selector)
    {
    case 0:
        wait = field_read_actor_binding_state((actor != 0xFF) ? actor : g_field_script->status.owner_id) < 2;
        break;
    }

    if (wait)
    {
        g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
    }
    else
    {
        FIELD_SCRIPT_ACTIVE_RECORD()->pc += 4;
    }
}

/**
 * @brief Opcode 0x14: end the step loop while a field condition selected by the operand holds.
 * @note Selectors 3 and 4 also copy D_8011F428 into script variable 0x7100 once the wait ends.
 */
void func_800B9868(void)
{
    u32 selector;
    s32 wait;
    s32 state;

    selector = FIELD_SCRIPT_ACTIVE_RECORD()->pc[1];
    switch (selector)
    {
    case 1:
        /* The redundant & 1 keeps the lhu result in its own register (99.88% without). */
        wait = FIELD_RUNTIME->state.bits.group_active & 1;
        break;
    case 2:
        wait = D_801227F0 != 2;
        break;
    case 3:
        state = D_8011F428 ^ 1;
        wait = state == 0;
        break;
    case 4:
        state = D_8011F428;
        wait = state == 0;
        break;
    }
    if (wait != 0)
    {
        g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
        return;
    }
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
    if (selector == 3 || selector == 4)
    {
        func_800BD520(0, 0x7100, D_8011F428);
    }
}

/**
 * @brief Handle a script mode query, advancing the PC or clearing the run flag.
 * @note A 0xFF operand selects the mode from the shared field state.
 */
void func_800B99A8(void)
{
    FieldScriptRecord* rec;
    FieldScriptContext* ctx;
    s32 active_record;
    s32 mode;
    s32 resolved;
    s32 selector;
    s32 result;

    active_record = g_field_script->active_record;
    ctx = g_field_script;
    rec = (FieldScriptRecord*)((u8*)ctx + ((active_record * 3) << 2));
    mode = rec->pc[1];
    if (mode == 0xFF)
    {
        resolved = ((u32)FIELD_RUNTIME->talk_window.word >> 8) & 3;
    }
    else
    {
        resolved = mode;
    }
    mode = resolved;
    selector = mode & 3;
    result = field_text_get_status(selector);
    if (!(mode & 0x80))
    {
        if (result == -1)
        {
            func_800BD520(0, 0x7100, field_text_get_choice(selector));
            FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
            return;
        }
    }
    else if (result == 3)
    {
        FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
        return;
    }
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x16: wait until field_is_actor_idle reports the actor ready, skipping absent party members.
 * @note A party slot (0 to 2) that is empty, or AI-controlled while g_field_interaction_active is clear, is skipped at once.
 */
void func_800B9AC4(void)
{
    FieldScriptRecord* rec;
    u8* pc;
    u8 descriptor;
    u8 actor;
    u32 party_index;

    rec = FIELD_SCRIPT_ACTIVE_RECORD();
    pc = rec->pc;
    descriptor = pc[1];
    if (descriptor == 0xFF)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = descriptor;
    }
    party_index = actor;
    if (party_index < 3)
    {
        if (FIELD_GAME->characters[party_index].name[0] == 0)
        {
            rec->pc = pc + 2;
            return;
        }
        if ((FIELD_GAME->characters[party_index].info.bytes[0] >> 7) != 0)
        {
            if (g_field_interaction_active == 0)
            {
                rec->pc = pc + 2;
                return;
            }
        }
    }
    if (field_is_actor_idle(actor) != 0)
    {
        FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
        return;
    }
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x17: report unsupported opcode 0x17 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_17(void)
{
    record_game_diagnostic(0x8001, 1, g_field_script->status.owner_id, 0x17);
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
    field_seek_scene_resource(entry & 0x7FFF);
    depth = g_field_script->active_record;
    rec = (FieldScriptRecord*)((u8*)g_field_script + depth * 0xC);
    rec->pc += 3;
}

/**
 * @brief Opcode 0x19: play a sound effect from two byte operands, sound id then pan.
 */
void field_script_op_19(void)
{
    field_play_sound(FIELD_SCRIPT_ACTIVE_RECORD()->pc[1], FIELD_SCRIPT_ACTIVE_RECORD()->pc[2]);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 3;
}

/**
 * @brief Opcode 0x1A: pass an actor (0xFF for the owner) and a mode byte to field_set_actor_control_mode.
 */
void field_script_op_1a(void)
{
    u8* pc;
    u8 descriptor;
    u8 actor;

    pc = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = pc[1];
    if (descriptor == 0xFF)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = descriptor;
    }
    field_set_actor_control_mode(actor, pc[2]);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 3;
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

void func_800BD434(s32, FieldScriptVariableRef, s32);

/**
 * @brief Opcode 0x1D: write an actor position into three script variables.
 * @note The Y and Z variables sit 0x20 and 0x40 past the X variable's offset (wrapping in 12 bits); Y is negated.
 */
void func_800B9FF8(void)
{
    s32 position[4];
    FieldScriptPositionOperands operands;
    s32 unused[4];
    u16 offset;
    u32 packed;
    u32 high;
    u32 first_reference;
    u32 second_reference;
    s32 first_offset;
    s32 second_offset;
    u8* pc;

    pc = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(pc[1], pc + 2, &operands.actor);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc, &operands.reference);

    packed = offset = operands.reference;
    high = packed >> 15;
    packed &= 0x7000;
    first_reference = high << 15;
    /* "+ high - high" keeps the target's s0/s1 assignment; a plain copy reaches 99.25%. */
    second_reference = first_reference + high - high;

    offset &= 0xFFF;
    first_offset = offset + 0x20;
    first_offset &= 0xFFF;
    first_reference |= packed;
    first_reference |= first_offset;
    second_offset = offset + 0x40;
    second_offset &= 0xFFF;
    second_reference |= packed;
    second_reference |= second_offset;

    field_get_actor_position(operands.actor, position);
    {
        FieldScriptVariableRef reference;

        reference.value = operands.reference;
        func_800BD434(g_field_script->status.owner_id, reference, position[0]);
    }
    {
        FieldScriptVariableRef reference;

        reference.value = first_reference;
        func_800BD434(g_field_script->status.owner_id, reference, -position[1]);
    }
    {
        FieldScriptVariableRef reference;

        reference.value = second_reference;
        func_800BD434(g_field_script->status.owner_id, reference, position[2]);
    }
}

/**
 * @brief Opcode 0x1E: start a battle with the byte operand as the monster group.
 */
void field_script_op_1e(void)
{
    field_battle_start(FIELD_SCRIPT_ACTIVE_RECORD()->pc[1]);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
}

/**
 * @brief Opcode 0x1F: suspend the battle and step past the opcode.
 */
void field_script_op_1f(void)
{
    field_battle_suspend();
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 1;
}

/**
 * @brief Opcode 0x20: report unsupported opcode 0x20 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_20(void)
{
    record_game_diagnostic(0x8001, 1, g_field_script->status.owner_id, 0x20);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x21: report unsupported opcode 0x21 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_21(void)
{
    record_game_diagnostic(0x8001, 1, g_field_script->status.owner_id, 0x21);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x22: report unsupported opcode 0x23 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter. Reports opcode 0x23 with opcode 0x23.
 */
s32 field_script_op_22(void)
{
    record_game_diagnostic(0x8001, 1, g_field_script->status.owner_id, 0x23);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x23: report unsupported opcode 0x23 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_23(void)
{
    record_game_diagnostic(0x8001, 1, g_field_script->status.owner_id, 0x23);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x24: report unsupported opcode 0x24 for the owner and step past the opcode.
 */
void field_script_op_24(void)
{
    record_game_diagnostic(0x8001, 1, g_field_script->status.owner_id, 0x24);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 1;
}

/**
 * @brief Opcode 0x25: report unsupported opcode 0x25 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_25(void)
{
    record_game_diagnostic(0x8001, 1, g_field_script->status.owner_id, 0x25);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x26: report unsupported opcode 0x26 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_26(void)
{
    record_game_diagnostic(0x8001, 1, g_field_script->status.owner_id, 0x26);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x27: free party slot 1 or 2 chosen by the byte operand, then call field_release_actor_resource_slot.
 * @note Operand 0 frees slot 1 and also clears the variable at 0xF87 + 8 * info byte 1; any other value frees slot 2.
 */
void field_script_op_27(void)
{
    s32 state;

    state = FIELD_SCRIPT_ACTIVE_RECORD()->pc[1];
    if (state == 0)
    {
        FIELD_GAME->characters[1].name[0] = 0;
        FIELD_GAME->characters[1].info.word |= 0x7F;
        func_800BD520(0, (FIELD_GAME->characters[1].info.bytes[1] << 3) + 0xF87, 0);
        func_800BD520(0, 0x2F08, 0xFF);
    }
    else
    {
        FIELD_GAME->characters[2].name[0] = 0;
        FIELD_GAME->characters[2].info.word |= 0x7F;
        func_800BD520(0, 0x2F00, 0xFF);
    }
    field_release_actor_resource_slot(state);
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
    field_queue_actor_event(0x80, 0, 0xF);
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
        field_queue_actor_event(0x80, 0, func_800A4778() & 0xFF);
        g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
        return;
    }
    func_800BD520(0, 0x7100, result);
    field_queue_actor_event(0x80, 0, 0x10);
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
 * @brief Opcode 0x2C: call field_text_close_window with one operand, or with 0 through 3 when the operand has bit 7 set.
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
            field_text_close_window((u16)value);
            value++;
        } while ((u32)value < 4);
    }
    else
    {
        field_text_close_window((u16)value);
    }
}

/**
 * @brief Opcode 0x2D: read two owner-substituting operands and pass them with the active record to field_face_actor.
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
    field_face_actor(arg0, arg1, rec, active);
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
    field_set_actor_group(selector, flags);
}

/**
 * @brief Opcode 0x30: report unsupported opcode 0x30 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_30(void)
{
    record_game_diagnostic(0x8001, 1, g_field_script->status.owner_id, 0x30);
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
    field_set_actor_position(key, x, y, z);
}

/**
 * @brief Opcode 0x32: read four operands, resolve them through field_resolve_talk_window and pass them to func_8009C620.
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
    field_resolve_talk_window(&arg0, &arg1, &arg2, &arg3);
    func_8009C620(arg1, arg3, arg0, arg2);
}

/**
 * @brief Opcode 0x33: read three operands and pass them to func_8009C77C.
 * @note A first operand of 0xFF is replaced by bits 8-9 of FieldRuntimeContext.talk_window.
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
        slot = (u32)FIELD_RUNTIME->talk_window.word >> 8;
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
    field_play_set_sfx_group0(sound_id, pan, unused);
}

/**
 * @brief Opcode 0x35: read two owner-substituting operands and one plain operand, then call field_spawn_targeted_animation_actor.
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
    field_spawn_targeted_animation_actor(arg0, arg2, 1, &arg1);
}

/**
 * @brief Opcode 0x36: report unsupported opcode 0x36 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_36(void)
{
    record_game_diagnostic(0x8001, 1, g_field_script->status.owner_id, 0x36);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Build the current shop item list from field-script operands and open the shop interface.
 * @note Operands: shop list index in resource 0xA, then the price scale (in sixteenths).
 */
void func_800BB3D8(void)
{
    u8* operands;
    u8 descriptor;
    s32 list_index;
    s32 price_scale;
    FieldShopListTable* lists;
    FieldShopList* list;
    ShopEntry entries[32];
    FieldItemResource* items;
    s32 index;
    s32 item;
    u32 scaled;
    u32 word;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_0(descriptor), operands + 2, &list_index);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &price_scale);

    lists = (FieldShopListTable*)func_800C1E40(0xA);
    list = (FieldShopList*)((u8*)lists + (&lists->header)[list_index + 1]);

    items = (FieldItemResource*)func_800C1E40(5);
    for (index = 0; (u32)index < list->count; index++)
    {
        item = list->entries[index].item;
        word = list->entries[index].word;
        entries[index].count = 0;
        /* Bit 8 (generated) becomes the record flag, bit 15 of the id. */
        entries[index].id = item + ((word << 7) & 0x8000);
        scaled = list->entries[index].bits.price * price_scale;
        entries[index].price = scaled >> 4;
    }

    field_open_shop_mode_1(list->count, (s32)entries, (s32)items->records, 2);
}

/* Field script opcode handlers 0x38 through 0x3F (see field_script.h). */

/**
 * @brief Opcode 0x38: no operation; step past the opcode.
 */
void field_script_op_38(void)
{
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 1;
}

/**
 * @brief Opcode 0x39: report unsupported opcode 0x39 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_39(void)
{
    record_game_diagnostic(0x8001, 1, g_field_script->status.owner_id, 0x39);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3A: report unsupported opcode 0x3A for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3a(void)
{
    record_game_diagnostic(0x8001, 1, g_field_script->status.owner_id, 0x3A);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3B: report unsupported opcode 0x3B for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3b(void)
{
    record_game_diagnostic(0x8001, 1, g_field_script->status.owner_id, 0x3B);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3C: report unsupported opcode 0x3C for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3c(void)
{
    record_game_diagnostic(0x8001, 1, g_field_script->status.owner_id, 0x3C);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3D: report unsupported opcode 0x3D for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3d(void)
{
    record_game_diagnostic(0x8001, 1, g_field_script->status.owner_id, 0x3D);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3E: report unsupported opcode 0x3E for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3e(void)
{
    record_game_diagnostic(0x8001, 1, g_field_script->status.owner_id, 0x3E);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3F: report unsupported opcode 0x3F for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3f(void)
{
    record_game_diagnostic(0x8001, 1, g_field_script->status.owner_id, 0x3F);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Dispatch the reset subcommand at the active script PC and advance by two bytes.
 */
void func_800BB7B4(void)
{
    s32 all_categories;
    s32 flag_mask;
    s32 i;
    FieldScriptRecord* record;

    switch (FIELD_SCRIPT_ACTIVE_RECORD()->pc[1])
    {
    case 0:
        all_categories = 0xFFFFFF;
        for (i = 10; i >= 0; i--)
        {
            FIELD_SAVED->layout.weapon_category_masks[i] = all_categories;
        }
        /* Plain word stores (not struct fields) keep g_field_script loaded after them. */
        *(u32*)&FIELD_SAVED->words[0x60 / 4] = 0x500;
        *(u32*)&FIELD_SAVED->words[0x64 / 4] = -0x8000;
        record = (FieldScriptRecord*)g_field_script;
        *(u32*)&FIELD_SAVED->words[0x68 / 4] = 0x803F;
        record += record->unk4;
        record->pc += 2;
        return;
    case 1:
        func_800B34D0(1);
        break;
    case 3:
        FIELD_RUNTIME->trigger_table = (FieldTriggerTable*)func_800C1E40(6);
        FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
        return;
    case 4:
        func_800C1EC8(0, FIELD_GAME->words, 0x200);
        FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
        return;
    case 5:
        field_apply_region_level_ups(0);
        field_apply_region_level_ups(1);
        field_apply_region_level_ups(2);
        field_apply_region_level_ups(3);
        field_apply_region_level_ups(4);
        break;
    case 6:
        i = 0;
        flag_mask = 0x7FFFFFFF;
        do
        {
            FIELD_GAME->regions[i].unk42 = 0;
            FIELD_GAME->regions[i].status.word &= flag_mask;
            i += 1;
        } while (i < 5);
        break;
    case 7:
        i = 0;
        do
        {
            FIELD_GAME->item_counts[i] = 99;
            i += 1;
        } while (i < 0xFD);
        break;
    }
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
}

/**
 * @brief Opcode 0x40: store a value into one of the script owner's variables.
 * @param variable Variable id passed to func_800BD520.
 * @param value Value to store.
 */
void func_800BB9C0(s32 variable, s32 value)
{
    func_800BD520(g_field_script->status.owner_id, variable, value);
}

/**
 * @brief Opcode 0x41: apply animation control op 1 to list-0 animation @p index at @p keyframe.
 * @param index Animation index in list 0.
 * @param keyframe Keyframe passed to field_control_animation.
 */
void func_800BB9F4(s32 index, s32 keyframe)
{
    field_control_animation(0, index, keyframe, 1);
}

/**
 * @brief Opcode 0x42: find or load a scene resource entry.
 * @param resource_slot_id Passed through to field_find_or_load_resource_entry.
 * @param resource_base Passed through to field_find_or_load_resource_entry.
 */
void func_800BBA24(s32 resource_slot_id, s32 resource_base)
{
    field_find_or_load_resource_entry(resource_slot_id, resource_base);
}

/**
 * @brief Opcode 0x43: set the active record's condition flag from func_800C1FFC for an actor.
 * @param mode Guard; the update runs only when zero.
 * @param actor Actor id, or 0xFF for the script owner.
 */
void func_800BBA44(s32 mode, s32 actor)
{
    s32 target;
    FieldScriptRecordState* rec;
    s32 result;

    if (mode == 0)
    {
        if (actor == 0xFF)
        {
            target = g_field_script->status.owner_id;
        }
        else
        {
            target = actor;
        }
        result = func_800C1FFC(target, 0x1100, 0x1100);
        rec = (FieldScriptRecordState*)((u8*)g_field_script + g_field_script->active_record * 0xC);
        rec->flags = (rec->flags & ~FIELD_SCRIPT_COND) | (result & 1);
    }
}

/**
 * @brief Opcode 0x44: run one of the miscellaneous field commands selected by @p command.
 * @param command Command number, 0x00 to 0x4E.
 * @param operand Command argument; commands that take an actor treat 0xFF as the script owner.
 */
void func_800BBAC8(u32 command, s32 operand)
{
    s32 actor_index;
    FieldScriptRecordState* script_record;

    if (operand == 0xFF)
    {
        actor_index = g_field_script->status.owner_id;
    }
    else
    {
        actor_index = operand;
    }
    switch (command)
    {
    case 0x0:
        field_request_return_to_title((s32)operand);
        return;
    case 0x1:
        field_start_timed_panel((s32)operand);
        return;
    case 0x2:
        func_800C2094((s32)operand);
        return;
    case 0x3:
        field_stop_actor((s32)operand);
        return;
    case 0x4:
        func_8005B0F4((s32)operand, 1);
        return;
    case 0x5:
        func_8005B0F4((s32)operand, 0);
        return;
    case 0x6:
        FIELD_GAME->lands[operand].flags |= 4;
        return;
    case 0x7:
        field_control_animation(0, operand, 0, 2);
        return;
    case 0x8:
        field_control_animation(0, (s32)operand, -1, 4);
        return;
    case 0x9:
        field_stop_second_song();
        return;
    case 0xA:
        field_run_zukan((s32)operand);
        return;
    case 0xB:
        field_run_menu_op((s32)operand);
        return;
    case 0xC:
        field_open_gosub_screen_sequence(g_field_runtime + (operand * 4));
        return;
    case 0xD:
        func_800BE710((s32)operand);
        return;
    case 0xE:
        FIELD_RUNTIME->scene_entry |= 0x8000;
        field_play_second_song();
        return;
    case 0xF:
        func_800C35AC((s32)operand);
        return;
    case 0x10:
        field_leave_party((s32)operand);
        return;
    case 0x11:
        field_stop_actor_script((s32)actor_index, 0);
        return;
    case 0x12:
        /* Called as returning int: the original does not mask the u8 result. */
        func_800BD520(0, 0x7100, ((s32 (*)(s32))func_800C20D8)((s32)operand));
        return;
    case 0x13:
        func_800C2138((s32)operand);
        return;
    case 0x14:
        func_800C21C0((s32)operand);
        return;
    case 0x15:
        func_800BD520(0, 0x7100, func_800C35E4((s32)operand));
        return;
    case 0x16:
        field_toggle_actor_hidden((s32)actor_index);
        return;
    case 0x17:
        field_start_actor_turn((s32)actor_index);
        return;
    case 0x18:
        func_800BD520(0, 0x7100, func_800C2264((s32)operand));
        return;
    case 0x19:
        func_800BD520(0, 0x7100, func_800C23F4());
        return;
    case 0x1A:
        field_set_actor_record_script_only((s32)actor_index, 2);
        return;
    case 0x1B:
        field_start_interaction(0x80, operand & 0xFFFF);
        return;
    case 0x1C:
        field_control_animation(1, operand, 0, 2);
        return;
    case 0x1D:
        field_control_animation(1, operand, -1, 1);
        return;
    case 0x1E:
        field_control_animation(2, operand, 0, 2);
        return;
    case 0x1F:
        field_control_animation(2, operand, -1, 1);
        return;
    case 0x20:
        func_8005B228((s32)operand, 1);
        return;
    case 0x21:
        func_8005B228((s32)operand, 0);
        return;
    case 0x22:
        func_800BD520(0, 0x7100, func_800C29CC((s32)operand));
        return;
    case 0x23:
        func_800B31CC((s32)actor_index);
        return;
    case 0x24:
        func_800B32FC((s32)operand);
        return;
    case 0x25:
        func_800B3420((s32)operand);
        return;
    case 0x26:
        func_800C2A88((s32)operand);
        return;
    case 0x27:
        field_stop_non_script_actors();
        return;
    case 0x28:
        func_800C1E08();
        return;
    case 0x29:
        func_800C299C((s32)operand);
        return;
    case 0x2A:
        field_open_carda((s32)operand);
        return;
    case 0x2B:
        func_800BD520(0, (s32)operand, 1);
        return;
    case 0x2C:
        field_clear_actor_record_script_only((s32)actor_index);
        return;
    case 0x2D:
        func_800BD520(0, 0x7100, func_800C24BC((s32)operand));
        return;
    case 0x2E:
        func_800C25A0((s32)operand);
        return;
    case 0x32:
        D_80122980 = (s32)operand;
        return;
    case 0x33:
        g_field_pair_indicators_disabled = (s32)operand;
        return;
    case 0x34:
        func_800BD520(0, 0x7100, field_find_nearest_faced_item((s32)operand));
        return;
    case 0x35:
        func_8005A67C((s32)operand, 0);
        return;
    case 0x36:
        func_8005A67C((s32)operand, 1);
        return;
    case 0x37:
        func_8009C974((s32)operand);
        return;
    case 0x38:
        field_battle_defeat_record((s32)actor_index);
        return;
    case 0x39:
        func_800BD520(0, 0x7100, func_800C3860((s32)operand));
        return;
    case 0x3A:
        func_800BD520(0, 0x7100, func_800C3894((s32)operand));
        return;
    case 0x3B:
        if ((s32)operand >= 0x40)
        {
            record_game_diagnostic(0x8001, 1, 0x2C, (s32)operand);
            return;
        }
        g_music_track_index = (s16)operand;
        return;
    case 0x3C:
    {
        typedef struct
        {
            unsigned low : 16;
            unsigned phase : 7;
            unsigned high : 9;
        } PhaseWord;
        PhaseWord* state = (PhaseWord*)&FIELD_GAME->control;
        state->phase++;
        state->phase %= 6U;
        return;
    }
    case 0x3D:
        g_field_hide_actor_panels = (s32)operand;
        return;
    case 0x3E:
        field_open_shop_mode_0((s32)operand);
        return;
    case 0x3F:
        func_800C396C();
        return;
    case 0x40:
        field_stop_actor((s32)actor_index);
        return;
    case 0x41:
        field_apply_pending_region_effects();
        return;
    case 0x42:
        func_8005B1EC();
        return;
    case 0x2F:
    case 0x43:
        func_8005B288((s32)operand);
        return;
    case 0x44:
        g_field_duel_mode = (s32)operand;
        return;
    case 0x45:
        akao_stop_song(0);
        return;
    case 0x46:
        akao_cmd_f1();
        return;
    case 0x47:
        field_reset_party_to_level((s32)operand);
        return;
    case 0x48:
        field_apply_region_level_ups((s32)operand);
        return;
    case 0x49:
        g_script_pair_value_49 = (s32)operand;
        return;
    case 0x4A:
        script_record = FIELD_SCRIPT_ACTIVE_RECORD_STATE();
        script_record->wait = (script_record->wait & 1) | (operand * 2);
        g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
        return;
    case 0x4B:
        g_gosub_result_count = 1;
        g_gosub_result_values = (s32)operand;
        return;
    case 0x4C:
        FIELD_GAME->money = operand;
        return;
    case 0x4D:
        g_music_track_index = 0;
        field_open_carda(0);
        func_800BCCE0(0xFFFE, 0, 0, 0);
        return;
    case 0x4E:
        FIELD_SAVED->layout.option_flags |= SAVED_OPTION_FLAG_2 | SAVED_OPTION_FLAG_3;
        func_800C1EC8(0, FIELD_GAME->words, 0x200);
        func_800C1EC8(0, (s32*)&FIELD_GAME->control, 0x30C);
        FIELD_GAME->control.fields.placed_land_count = 1;
        FIELD_GAME->flag_bits[0] |= 0x10000000;
        /* actor_index doubles as the land counter; a separate local changes allocation. */
        for (actor_index = 0; actor_index < FIELD_LAND_COUNT; actor_index++)
        {
            FIELD_GAME->lands[actor_index].position |= 0xF;
            FIELD_GAME->lands[actor_index].position |= 0xF0;
        }
        FIELD_LAND_WORDS[0].word |= 1;
        FIELD_GAME->lands[0].count = 1;
        FIELD_LAND_WORDS[0].word |= 4;
        FIELD_LAND_WORDS[1].word |= 4;
        FIELD_LAND_WORDS[2].word |= 4;
        FIELD_LAND_WORDS[3].word |= 4;
        FIELD_LAND_WORDS[4].word |= 4;
        FIELD_LAND_WORDS[5].word |= 4;
        FIELD_LAND_WORDS[6].word |= 4;
        FIELD_LAND_WORDS[32].word |= 4;
        break;
    }
}

/**
 * @brief Forward an actor id to field_set_actor_animation.
 * @param actor_id Actor id, or 0xFF for the script owner.
 */
void func_800BC268(s32 actor_id)
{
    s32 actor;

    actor = actor_id;
    if (actor == 0xFF)
    {
        actor = g_field_script->status.owner_id;
    }
    field_set_actor_animation(actor);
}

/**
 * @brief Update the condition bit of the active script record from a pair query.
 *
 * Resolves @p first_id and @p second_id (each the script owner when 0xFF), runs
 * field_test_actor_depth_overlap for the pair, and stores the result's low bit into bit 0 of
 * the active record's flags word.
 *
 * @param first_id First actor id, or 0xFF for the script owner.
 * @param second_id Second actor id, or 0xFF for the script owner.
 */
void func_800BC2A0(s32 first_id, s32 second_id)
{
    FieldScriptRecordState* rec;
    s32 result;
    s32 first;
    s32 second;

    if (first_id == 0xFF)
    {
        first = g_field_script->status.owner_id;
    }
    else
    {
        first = first_id;
    }
    if (second_id == 0xFF)
    {
        second = g_field_script->status.owner_id;
    }
    else
    {
        second = second_id;
    }
    result = field_test_actor_depth_overlap(first, second);
    rec = FIELD_SCRIPT_ACTIVE_RECORD_STATE();
    rec->flags = (rec->flags & ~1) | (result & 1);
}

/**
 * @brief Dispatch a resolved sequence action and latch a scene-state flag.
 *
 * Runs field_join_guest for @p record_id, resolves @p actor_id (0xFF is the script
 * owner), and forwards the pair to field_activate_actor_resource_slot. When @c g_field_interaction_active is set
 * it triggers field_set_actor_control_mode and sets the runtime party_mode bits (17-18) to 1. Always finishes by writing @p record_id to
 * script variable 0x2F08.
 *
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param record_id Secondary parameter forwarded to the dispatched calls.
 */
void func_800BC328(s32 actor_id, s32 record_id)
{
    s32 actor;

    field_join_guest(record_id);
    if (actor_id == 0xFF)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = actor_id;
    }
    field_activate_actor_resource_slot(actor, record_id, 0);
    if (g_field_interaction_active != 0)
    {
        field_set_actor_control_mode(1, 2);
        FIELD_RUNTIME->state.flags = (FIELD_RUNTIME->state.flags & 0xFFF9FFFF) | 0x20000;
    }
    func_800BD520(0, 0x2F08, record_id);
}

/**
 * @brief Pick a value from field_rejoin_companion or field_join_companion, apply it to the actor when valid, and write it to script variable 0x2F00.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param source 1 selects field_rejoin_companion, anything else field_join_companion.
 */
void func_800BC3DC(s32 actor_id, s32 source)
{
    s32 actor;
    s32 slot_value;

    if (actor_id == 0xFF)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = actor_id;
    }
    if (source == 1)
    {
        slot_value = field_rejoin_companion();
    }
    else
    {
        slot_value = field_join_companion();
    }
    if (slot_value != 0xFF)
    {
        field_activate_actor_resource_slot(actor, slot_value, 1);
    }
    func_800BD520(0, 0x2F00, slot_value);
}

/**
 * @brief Apply field_join_golem's value to the actor through field_activate_actor_resource_slot and write it to script variable 0x2F00.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param argument Passed in $a0 to field_join_golem, whose definition takes no parameters.
 */
void func_800BC474(s32 actor_id, s32 argument)
{
    s32 value;

    /* field_join_golem takes no parameters; the original still passes the argument in $a0. */
    value = ((s32 (*)(s32))field_join_golem)(argument);
    if (actor_id == 0xFF)
    {
        actor_id = g_field_script->status.owner_id;
        field_activate_actor_resource_slot(actor_id, value, 1);
    }
    else
    {
        field_activate_actor_resource_slot(actor_id, value, 1);
    }
    func_800BD520(0, 0x2F00, value);
}

/**
 * @brief Route a resolved actor index to one of two handlers, then clear bit 31 of the script status word.
 *
 * When bit 0x10000 of the runtime state word is set and the index
 * is below 3, calls field_start_actor_private_script with func_800C2928's record; otherwise calls
 * field_start_actor_script with the index.
 *
 * @param actor_index Actor index, or 0xFF for the script owner.
 * @param entry Forwarded (low 16 bits) to func_800C2928.
 */
void func_800BC4E8(s32 actor_index, s32 entry)
{
    s32 actor;
    u32 actor_id;

    actor = actor_index;
    if (actor_index == 0xFF)
    {
        actor = g_field_script->status.owner_id;
    }
    actor_id = actor & 0xFF;
    if ((FIELD_RUNTIME->state.flags & 0x10000) && actor_id < 3)
    {
        field_start_actor_private_script(actor_id, func_800C2928(actor_id, entry & 0xFFFF));
    }
    else
    {
        field_start_actor_script(actor & 0xFF);
    }
    g_field_script->status.word = g_field_script->status.word & 0x7FFFFFFF;
}

/**
 * @brief Call func_8005AF04 in mode 1 with 0xFF mapped to -1.
 * @param obj_index Object index.
 * @param part_index Part index; 0xFF becomes -1.
 */
void func_800BC58C(s32 obj_index, s32 part_index)
{
    if (part_index == 0xFF)
    {
        part_index = -1;
    }
    func_8005AF04(obj_index, part_index, 1);
}

/**
 * @brief Call func_8005AF04 in mode 0 with 0xFF mapped to -1.
 * @param obj_index Object index.
 * @param part_index Part index; 0xFF becomes -1.
 */
void func_800BC5B8(s32 obj_index, s32 part_index)
{
    if (part_index == 0xFF)
    {
        part_index = -1;
    }

    func_8005AF04(obj_index, part_index, 0);
}

/**
 * @brief Re-emit an actor's position with its Y replaced by -height and X and Z scaled down by 256.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param height Negated and used as the new Y.
 */
void func_800BC5E4(s32 actor_id, s32 height)
{
    s32 actor;
    s32 position[4];

    if (actor_id == 0xFF)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = actor_id;
    }
    field_get_actor_position(actor, position);
    position[1] = -height;
    field_set_actor_position(actor, position[0] >> 8, position[1], position[2] >> 8);
}

/**
 * @brief Route an actor to field_load_bound_animation or field_spawn_shared_animation_actor depending on bit 15 of resource.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param resource Bit 15 selects field_load_bound_animation with the low 15 bits; otherwise field_spawn_shared_animation_actor gets it whole.
 */
void func_800BC65C(s32 actor_id, s32 resource)
{
    if (actor_id == 0xFF)
    {
        actor_id = g_field_script->status.owner_id;
    }

    if (resource & 0x8000)
    {
        field_load_bound_animation(actor_id, resource & 0x7FFF);
    }
    else
    {
        field_spawn_shared_animation_actor(actor_id, resource);
    }
}

/**
 * @brief Build a shop inventory from a packed item list and open shop mode one.
 * @param list_index Index of the packed shop list to load.
 * @param price_scale Scale applied to each generated item price.
 */
void func_800BC6B0(s32 list_index, s32 price_scale)
{
    FieldShopListTable* lists;
    FieldShopList* list;
    FieldItemResource* items;
    ShopEntry entries[32];
    s32 index;
    s32 price;
    u32 scaled;
    FieldItemRecord* item;

    lists = (FieldShopListTable*)func_800C1E40(0xA);
    /* offsets[list_index] written from the header word; the direct index swaps the addu operands. */
    list = (FieldShopList*)((u8*)lists + (&lists->header)[list_index + 1]);
    items = (FieldItemResource*)func_800C1E40(5);
    for (index = 0; (u32)index < list->count; index++)
    {
        if (list->entries[index].bits.generated)
        {
            entries[index].id = list->entries[index].item | 0x8000;
            item = &items->records[list->entries[index].item];
            entries[index].count = 0;
            price = func_800C38C8(item);
            item->handle = price;
            scaled = (u32)(price * price_scale) >> 3;
            entries[index].price = scaled;
        }
        else
        {
            entries[index].id = list->entries[index].item;
            entries[index].count = 0;
            scaled = (s32)(list->entries[index].bits.price * price_scale) >> 3;
            entries[index].price = scaled;
        }
    }

    field_open_shop_mode_1(list->count, (s32)entries, (s32)items->records, 2);
}

/**
 * @brief Resolve a target index and clear two flag bits on its state record.
 *
 * When @p actor_id is the sentinel 0xFF the index is the script owner; otherwise
 * it is @p actor_id itself. The resolved index selects a state record via
 * field_find_actor_record_or_default, whose flags word has bits 31 and 29 cleared, then the
 * index and @p resource_index are dispatched to field_retire_actor.
 *
 * @param actor_id Target index, or 0xFF for the script owner.
 * @param resource_index Forwarded to field_retire_actor.
 */
void func_800BC7EC(s32 actor_id, s32 resource_index)
{
    s32 actor;
    FieldActorRecord* record;

    if (actor_id == 0xFF)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = actor_id;
    }
    record = (FieldActorRecord*)field_find_actor_record_or_default(actor);
    record->flags.word &= 0x7FFFFFFF;
    record->flags.word &= 0xDFFFFFFF;
    field_retire_actor(actor, resource_index);
}

/**
 * @brief Write field_actor_faces_actor's result for two actors to script variable 0x7100.
 * @param first_key First actor id, or 0xFF for the script owner.
 * @param second_key Second actor id, or 0xFF for the script owner.
 */
void func_800BC86C(s32 first_key, s32 second_key)
{
    first_key = (first_key == 0xFF) ? g_field_script->status.owner_id : first_key;
    second_key = (second_key == 0xFF) ? g_field_script->status.owner_id : second_key;

    func_800BD520(0, 0x7100, field_actor_faces_actor(first_key, second_key));
}

/**
 * @brief Copy script variable source_variable to script variable dest_variable in the owner's slot.
 * @param dest_variable Destination variable id.
 * @param source_variable Source variable id (low 16 bits).
 */
void func_800BC8CC(s32 dest_variable, s32 source_variable)
{
    func_800BD520(g_field_script->status.owner_id, dest_variable, func_800BD414(g_field_script->status.owner_id, source_variable & 0xFFFF));
}

/**
 * @brief Call field_fade_song in mode 1 with value clamped to 0x7F and count defaulting to 1.
 * @param value Value clamped to 0x7F.
 * @param count Count; 0 becomes 1.
 */
void func_800BC91C(s32 value, s32 count)
{
    count = (count != 0) ? count : 1;
    if (value >= 0x80)
    {
        value = 0x7F;
    }
    field_fade_song(1, count, value);
}

/**
 * @brief Thin stack-frame wrapper around field_play_song_section.
 */
void func_800BC960(void)
{
    field_play_song_section();
}

/**
 * @brief Call field_fade_song in mode 0 with value clamped to 0x7F and count defaulting to 1.
 * @param value Value clamped to 0x7F.
 * @param count Count; 0 becomes 1.
 */
void func_800BC980(s32 value, s32 count)
{
    count = (count != 0) ? count : 1;
    if (value >= 0x80)
    {
        value = 0x7F;
    }
    field_fade_song(0, count, value);
}

/**
 * @brief Store a 2-bit value into bits 4-5 of a land record's flags byte.
 * @param land Land index.
 * @param value Value; only the low 2 bits are stored.
 */
void func_800BC9C4(s32 land, s32 value)
{
    FIELD_GAME->lands[land].flags = (FIELD_GAME->lands[land].flags & 0xCF) | ((value & 3) << 4);
}

/**
 * @brief Fetch an actor's position, scale it down by 256, and forward it to field_reset_actor_at.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param resource_entry_index Forwarded to field_reset_actor_at.
 */
void func_800BC9F8(s32 actor_id, s32 resource_entry_index)
{
    s32 position[3];
    s32 actor;

    if (actor_id == 0xFF)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = actor_id;
    }
    field_get_actor_position(actor, position);
    {
        s32 x = position[0] >> 8;
        s32 y = position[1] >> 8;
        s32 z = position[2] >> 8;
        field_reset_actor_at(actor, resource_entry_index, FIELD_RUNTIME->state.bytes.trigger_group, x, y, z);
    }
}

/**
 * @brief Forward two values to field_play_sound.
 * @param sound_id Forwarded unchanged.
 * @param pan Forwarded unchanged.
 */
void func_800BCA88(s32 sound_id, s32 pan)
{
    field_play_sound(sound_id, pan);
}

/**
 * @brief Write the party's money to script variable @p variable_id.
 * @param variable_id Script variable id.
 */
void func_800BCAA8(s32 variable_id)
{
    func_800BD520(0, variable_id, FIELD_GAME->money);
}

/**
 * @brief Write field_get_actor_binding_state's result for an actor to script variable @p variable_id in that actor's slot.
 * @param variable_id Script variable id.
 * @param actor_id Actor id, or 0xFF for the script owner.
 */
void func_800BCAD8(s32 variable_id, s32 actor_id)
{
    s32 actor;

    if (actor_id == 0xFF)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = actor_id;
    }
    func_800BD520(actor, variable_id, field_get_actor_binding_state(actor));
}

/**
 * @brief Empty function; no-op.
 */
void func_800BCB40(void)
{
}

/**
 * @brief Empty function; no-op.
 */
void func_800BCB48(void)
{
}

/**
 * @brief Empty function; no-op.
 */
void func_800BCB50(void)
{
}

/**
 * @brief Empty function; no-op.
 */
void func_800BCB58(void)
{
}

/**
 * @brief Empty function; no-op.
 */
void func_800BCB60(void)
{
}

/**
 * @brief Record a diagnostic supplied by a field script.
 * @param status Diagnostic status.
 * @param code Diagnostic code.
 * @param value0 First diagnostic value.
 * @param value1 Second diagnostic value.
 */
void func_800BCB68(s32 status, s32 code, s32 value0, s32 value1)
{
    record_game_diagnostic(status, code, value0, value1);
}

/**
 * @brief Split bit 7 of mode into a flag and forward the rest to field_set_all_actor_render_state.
 * @param mode Id with an optional 0x80 flag bit.
 * @param red Forwarded as the first argument.
 * @param green Forwarded as the second argument.
 * @param blue Forwarded as the third argument.
 */
void func_800BCB88(s32 mode, s32 red, s32 green, s32 blue)
{
    s32 flag;

    if (mode & 0x80)
    {
        flag = 1;
        mode &= 0x7F;
    }
    else
    {
        flag = 0;
    }
    field_set_all_actor_render_state(red, green, blue, flag, mode);
}

/**
 * @brief Split bit 7 of key into a flag, run field_clear_actor_record_script_only on the id, then forward everything to field_reload_actor.
 * @param key Actor id with an optional 0x80 flag bit.
 * @param resource_entry_index Forwarded to field_reload_actor.
 * @param resource_slot_id Forwarded to field_reload_actor.
 * @param resource_base Forwarded to field_reload_actor.
 */
void func_800BCBD0(s32 key, s32 resource_entry_index, s32 resource_slot_id, s32 resource_base)
{
    s32 actor;
    s32 flag;

    if (key & 0x80)
    {
        flag = 1;
        actor = key & 0x7F;
    }
    else
    {
        flag = 0;
        actor = key;
    }
    field_clear_actor_record_script_only(actor);
    field_reload_actor(actor, resource_entry_index, resource_slot_id, (u8*)resource_base, 0, -1, -1, -1, 0, flag);
}

/**
 * @brief Empty function; no-op.
 */
void func_800BCC6C(void)
{
}

/**
 * @brief Forward four values to func_800B0710, mapping 0xFF to the script owner in the first and to -1 in the rest.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param value1 0xFF becomes -1.
 * @param value2 0xFF becomes -1.
 * @param value3 0xFF becomes -1.
 */
void func_800BCC74(s32 actor_id, s32 value1, s32 value2, s32 value3)
{
    s32 actor;

    if (actor_id == 0xFF)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = actor_id;
    }
    func_800B0710(actor, (value1 == 0xFF) ? -1 : value1, (value2 == 0xFF) ? -1 : value2, (value3 == 0xFF) ? -1 : value3);
}

/**
 * @brief Set pending layout selectors and reset execution to script record zero.
 * @param transition_id Stored in FieldRuntimeContext.transition.fields.scene_id.
 * @param mode Stored in FieldRuntimeContext.transition.fields.unk41A.
 * @param selectors Packed bytes for scene_argument1, scene_entry and scene_argument2; 0xFE and 0xFF select sentinel behavior.
 * @param flags Five-bit flags stored at transition bits 24 through 28.
 */
void func_800BCCE0(transition_id, mode, selectors, flags) s16 transition_id;

s8 mode;

s32 selectors;

s32 flags;

{
    s32 layout;
    s32 third_selector;
    s32 option;

    FieldRuntimeContext* runtime = FIELD_RUNTIME;
    option = selectors & 0xFF;
    runtime->transition.flags |= 0x40000000;
    runtime->transition.fields.unk41A = mode;
    layout = (selectors >> 8) & 0xFF;
    third_selector = (selectors >> 0x10) & 0xFF;
    runtime->transition.fields.scene_id = transition_id;
    switch (option)
    {
    case 0xFE:
        FIELD_RUNTIME->scene_argument1 = -2;
        break;
    case 0xFF:
        g_layout_option = -1;
        FIELD_RUNTIME->scene_argument1 = -1;
        break;
    default:
        if (option == g_layout_option)
        {
            FIELD_RUNTIME->scene_argument1 = -2;
        }
        else
        {
            FIELD_RUNTIME->scene_argument1 = option;
        }
        break;
    }
    switch (layout)
    {
    case 0xFE:
        FIELD_RUNTIME->scene_entry = -2;
        break;
    case 0xFF:
        FIELD_RUNTIME->scene_entry = -1;
        break;
    default:
        if (layout == g_layout_flag)
        {
            FIELD_RUNTIME->scene_entry = -1;
        }
        else
        {
            FIELD_RUNTIME->scene_entry = layout;
        }
        break;
    }
    switch (third_selector)
    {
    case 0xFE:
        FIELD_RUNTIME->scene_argument2 = -2;
        break;
    case 0xFF:
        FIELD_RUNTIME->scene_argument2 = -1;
        break;
    default:
        FIELD_RUNTIME->scene_argument2 = third_selector;
        break;
    }
    FIELD_RUNTIME->transition.flags = (FIELD_RUNTIME->transition.flags & 0xE0FFFFFF) | ((flags & 0x1F) << 24);
    g_field_script->active_record = 0;
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = 0;
}

/**
 * @brief Opcode 0x86: dispatch an entry of a resource record through field_set_text_macro.
 *
 * Fetches the record for @p resource_id via func_800C1E40; when non-NULL, reads
 * the halfword at @c entry_index*2 + 4 within it and calls field_set_text_macro with
 * the record address offset by that halfword plus 4.
 *
 * @param operand_0 Forwarded to field_set_text_macro as its first argument.
 * @param resource_id Record selector passed to func_800C1E40.
 * @param entry_index Halfword index within the record (scaled by 2).
 * @param operand_3 Forwarded to field_set_text_macro as its third argument.
 */
void field_script_op_86(s32 operand_0, s32 resource_id, s32 entry_index, s32 operand_3)
{
    u8* p = func_800C1E40(resource_id);

    if (p != NULL)
    {
        u16 h = *(u16*)(p + (entry_index << 1) + 4);
        /* Int limit on purpose: the original passes operand_3 without narrowing it to u8. */
        ((void (*)(s32, u8 *, s32))field_set_text_macro)(operand_0, p + (h + 4), operand_3);
    }
}

/**
 * @brief Opcode 0x87: route an actor to field_run_actor_event or field_queue_actor_event by selector.
 *
 * Selector 0 forwards to field_run_actor_event and selector 1 to field_queue_actor_event, each
 * with the resolved actor id and the low bytes of the last two operands.
 *
 * @param selector Handler selector, 0 or 1.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param operand_2 Low byte forwarded to the handler.
 * @param operand_3 Low byte forwarded to the handler.
 */
void field_script_op_87(s32 selector, s32 actor_id, s32 operand_2, s32 operand_3)
{
    if (actor_id == 0xFF)
    {
        actor_id = g_field_script->status.owner_id;
    }
    switch (selector)
    {
    case 0:
        field_run_actor_event(actor_id, operand_2 & 0xFF, operand_3 & 0xFF);
        break;
    case 1:
        field_queue_actor_event(actor_id, operand_2 & 0xFF, operand_3 & 0xFF);
        break;
    }
}

/**
 * @brief Opcode 0x88: reset the layout option globals and start the field_begin_gover_transition transition.
 * @param operand_0 Forwarded unchanged.
 * @param operand_1 0xFF becomes -1.
 * @param operand_2 0xFF becomes -1.
 */
void field_script_op_88(s32 operand_0, s32 operand_1, s32 operand_2)
{
    s32 option;
    s32 sub_mode;

    option = -1;
    if (operand_1 != 0xFF)
    {
        option = operand_1;
    }
    operand_1 = option;

    sub_mode = -1;
    if (operand_2 != 0xFF)
    {
        sub_mode = operand_2;
    }

    g_layout_option = -1;

    operand_2 = sub_mode;

    g_layout_sub_mode = -1;

    field_begin_gover_transition(operand_0, operand_1, operand_2);
}

/**
 * @brief Opcode 0x89: issue AKAO command 0xA9 with a minimum first value of 1.
 * @param operand_0 Unused.
 * @param operand_1 Unused.
 * @param value First AKAO operand; 0 is promoted to 1.
 * @param operand_3 Second AKAO operand.
 */
void field_script_op_89(s32 operand_0, s32 operand_1, s32 value, s32 operand_3)
{
    if (value == 0)
    {
        value = 1;
    }
    akao_cmd_a9(value, operand_3);
}

/**
 * @brief Opcode 0x8A: forward an actor pair to field_spawn_targeted_animation_actor.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param operand_1 Forwarded unchanged.
 * @param target_id Target id, or 0xFF for the script owner; passed by address.
 * @param operand_3 Unused.
 */
void field_script_op_8a(s32 actor_id, s32 operand_1, s32 target_id, s32 operand_3)
{
    s32 resolved_target;
    s32 resolved_actor;

    resolved_actor = actor_id;
    if (target_id == 0xFF)
    {
        resolved_target = (s32)g_field_script->status.owner_id;
    }
    else
    {
        resolved_target = target_id;
    }
    if (resolved_actor == 0xFF)
    {
        resolved_actor = g_field_script->status.owner_id;
    }
    field_spawn_targeted_animation_actor(resolved_actor, operand_1, 1, &resolved_target);
}

/**
 * @brief Opcode 0x8B: set the fade color's three 10-bit components and the fade timer.
 * @param red Red component (low 10 bits).
 * @param green Green component (low 10 bits).
 * @param blue Blue component (low 10 bits).
 * @param timer Fade timer value.
 */
void field_script_op_8b(s32 red, s32 green, s32 blue, s32 timer)
{
    FieldRuntimeContext* runtime;

    runtime = FIELD_RUNTIME;
    runtime->fade_timer = timer;
    runtime->fade_color.bits.red = red;
    runtime->fade_color.bits.green = green;
    runtime->fade_color.bits.blue = blue;
}

/**
 * @brief Opcode 0x8C: forward to field_revive_actor with 0xFF operands mapped to the owner id or -1.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param operand_1 0xFF becomes -1.
 * @param operand_2 0xFF becomes -1.
 * @param operand_3 0xFF becomes -1.
 */
void field_script_op_8c(s32 actor_id, s32 operand_1, s32 operand_2, s32 operand_3)
{
    s32 resolved_actor;

    if (actor_id == 0xFF)
    {
        resolved_actor = g_field_script->status.owner_id;
    }
    else
    {
        resolved_actor = actor_id;
    }
    field_revive_actor(resolved_actor, (operand_1 == 0xFF) ? -1 : operand_1, (operand_2 == 0xFF) ? -1 : operand_2, (operand_3 == 0xFF) ? -1 : operand_3);
}

/**
 * @brief Opcode 0x8D: no-op.
 */
void field_script_op_8d(void)
{
}

/**
 * @brief Opcode 0x8E: no-op.
 */
void field_script_op_8e(void)
{
}

/**
 * @brief Opcode 0x8F: no-op.
 */
void field_script_op_8f(void)
{
}
