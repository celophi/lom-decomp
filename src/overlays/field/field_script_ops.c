#include "game_audio.h"
#include "field_scene_transition.h"
#include "field_text.h"
#include "main.h"
#include "common.h"

/* field_script_flow_ops */
#include "field_script.h"
#define FIELD_B74 ((UnkStruct80122B74*)D_80122B74)

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} SeqRec;

typedef void (*FieldDispatchFn)(s32, s32);

/** @brief Script sequence record: 12-byte entries indexed by the header's unk4. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    u8* unk8;
} ScanRecord;

typedef struct
{
    s32 actor;
    u16 reference;
} FieldScriptPositionOperands;

/** @brief View of reset fields addressed relative to successive 0x60-byte slots. */
typedef struct
{
    u8 pad[0x2F36];
    u16 count;
    s32 flags;
} FieldResetSlotView;

typedef struct
{
    u8 pad0[0x402];
    u16 unk402;
} StructB78Local;

/** @brief Partial StructB800B99A8 layout used by func_800B99A8. */
typedef struct
{
    u8 pad0[0x41C];
    u32 unk41C;
} StructB800B99A8;

/** @brief One entry of the shop item list built on the stack. */
typedef struct
{
    s16 price;
    s16 pad2;
    s32 scaled;
} ShopItemEntry;

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    s32 unk4;
    u8 pad8[4];
    s32 unkC;
} PairSeqRec;

/** @brief One eight-byte shop entry built from a packed resource word. */
typedef struct
{
    u16 item;
    u16 unused;
    u32 price;
} ScriptShopEntry;

/*
 * Helpers reached from field script handlers. Most take an actor id where
 * 0xFF means the script owner; the owner id is byte 0 of g_field_script.
 */

/** @brief Actor record from func_800C1B60; unk90 holds the flag word. */
typedef struct
{
    u8 pad[0x90];
    s32 unk90;
} SomeStruct;

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Struct80087F44;

/** @brief View of D_80122B74 exposing the word at 0x2C. */
typedef struct
{
    s8 pad[0x2C];
    s32 unk2C;
} UnkStruct80122B74;

/*
 * Extended field script opcodes 0x80 through 0x8F.
 *
 * field_script_run hands opcodes of 0x80 and above to func_800B8308, which
 * decodes up to four operands from the descriptor bytes that follow the opcode
 * and jumps through g_field_script_ext_op_table[opcode - 0x80]. Every handler
 * here receives those decoded operands in order. An operand of 0xFF in an
 * actor-id slot means the script owner.
 */

/** @brief View of D_80122B78 exposing the three packed 10-bit fields at 0x410. */
typedef struct
{
    u8 pad0[0x404];
    s32 unk404; /* 0x404 */
    u8 pad408[0x410 - 0x408];
    u32 unk410; /* 0x410 three 10-bit fields */
    u32 unk414; /* 0x414 */
} StructB78;

/** @brief Pending layout transition state with overlapping control fields at 0x418. */
typedef struct
{
    u8 pad[0x404];
    s32 layout, option, third_selector;
    u8 pad410[8];
    union
    {
        s32 word;
        struct
        {
            s16 id;
            s8 mode;
            u8 flags;
        } fields;
    } control;
} State;

void func_800B820C();
void func_800B8308();
void func_800BD520(s32, s32, s32);
u8* func_800C1B60(s32);
extern void (*g_field_script_op_table[])();
extern FieldDispatchFn D_800F0D48[];
u8* func_800B84B4(s32 arg0, u8* arg1, s32* arg2);
extern u8* func_800B84B4(s32, u8*, s32*);
s32 func_800BD414(s32 arg0, s32 arg1);
/* Field script opcode handlers 0x03 through 0x08 (see field_script.h). */

extern u8* D_80122B78;
void func_800BD6F4(s32 value, u8* params);
u8* field_script_read_operand(u32 mode, u8* pc, s32* out);
s32 func_80087F0C(s32);
s32 func_800B2A9C(s32);
s32 func_800BD650(s32, s32, s32, s32, s32);
void func_800BD55C(s32, s32, s32, s32, s32, s32);
extern u8* D_80122B74;
extern s32 D_80123FB0, D_80123FC4;
extern u8* func_80087EF0(s32);
extern u8* func_800C1B60(s32);
extern void func_8009C620(s32, s32, s32, s32);
extern void func_8009C77C(s32, s32, s32);
s32 func_8005A84C(s32 arg0, s32 arg1);
s32 func_8008B398(s32 key);
extern s32 func_800BD414(s32 arg0, s32 arg1);
extern s32 func_8008B398(s32 key);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
extern s32 D_8011F428;
extern s32 D_801227F0;
extern FieldScriptContext* g_field_script;
s32 func_800875C4(s32 arg0, u32 arg1, void* arg2, void* arg3);
extern s32 D_8010AE78;
/* Field script opcode handlers 0x17 through 0x1C (see field_script.h). */

void func_800A3938(s32 sound_id, s32 pan);
void func_80087FC0();
s32 func_800BE5C8(s32 arg0, s32 arg1, s32 arg2);
s32 func_80087F44(s32, s32*);
void func_800B4410(s32 arg0);
void func_800B4584(void);
void field_release_actor_resource_slot(s32 arg0);
void func_800A43E8(s32 arg0, s32 arg1, u16 arg2, s32 arg3);
void func_800B286C(s32 arg0, s32 arg1, s32 arg2);
s32 func_800A4744(void);
u8 func_800A4778(void);
void func_8008AFD8(s32 arg0, s32 arg1, FieldScriptRecord* record, s32 record_index);
void func_80087614(s32 arg0, s32 arg1);
s32 func_80087D8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_800B2654(s32* arg0, s32* arg1, s32* arg2, s32* arg3);
void func_8009C620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8009C77C(s32 arg0, s32 arg1, s32 arg2);
void func_800A3988(s32 arg0, s32 arg1, s32 arg2, FieldScriptRecord* record);
void func_8008B5D0(s32 arg0, s32 arg1, s32 arg2, s32* arg3);
u8* field_script_read_operand(u32 type, u8* data, s32* value);
u8* func_800C1E40(s32 arg0);
void field_open_shop_mode_1();
extern void func_800B34D0(s32);
extern void func_800C1230(s32);
extern u8* func_800C1E40(s32);
extern s32* func_800C1EC8(s32*, s32*, s32);
void akao_cmd_f1(void);
void akao_stop_song(s32);
void field_control_animation(s32, s32, s32, s32);
void field_open_gosub_screen_sequence(void*);
void field_open_shop_mode_0(s32);
void field_run_zukan(s32);
void func_8005A67C(s32, s32);
void func_8005B0F4(s32, s32);
void func_8005B1EC(void);
void func_8005B228(s32, s32);
void func_8005B288(s32);
void func_800681C0(s32);
void func_80089980(s32);
void func_80089A68(s32);
void func_8008BD88(s32);
void func_8009C974(s32);
void func_800A37E4(void);
void func_800A38D4(void);
void func_800A5670(s32);
void func_800AD030(s32);
void func_800B22F0(s32, s32);
void func_800B31CC(s32);
void func_800B32FC(s32);
void func_800B3420(s32);
void func_800B60DC(s32);
void func_800B66F0(s32);
void func_800BCCE0();
void func_800BE710(s32);
void func_800C06E8(void);
void func_800C1230(s32);
void func_800C1D14(s32, s32);
void func_800C1D68(void);
void func_800C1E08(void);
void func_800C2094(s32);
s32 func_800C20D8(s32);
void func_800C2138(s32);
void func_800C21C0(s32);
s32 func_800C2264(s32);
s32 func_800C23F4(void);
s32 func_800C24BC(s32);
void func_800C25A0(s32);
s32 func_800C2724(s32);
void func_800C2848(s32, s32);
void func_800C28B8(s32);
void func_800C299C(s32);
s32 func_800C29CC(s32);
void func_800C2A88(s32);
void func_800C31BC(s32);
void func_800C35AC(s32);
s32 func_800C35E4(s32);
s32 func_800C3860(s32);
s32 func_800C3894(s32);
void func_800C396C(void);
void func_800C5704(s32);
extern u8 *D_80122B74, *D_80122B78;
extern s32 g_field_hide_actor_panels, D_8010D020, D_80117EC4, D_80122980;
extern s32 g_gosub_result_count, g_gosub_result_values;
void field_control_animation(s32 list_kind, s32 index, s32 keyframe, s32 op);
extern void field_find_or_load_resource_entry(s32 arg0, s32 arg1);
extern s32 func_800C1FFC(s32 arg0, s32 arg1, s32 arg2);
/*
 * Helpers reached from field script handlers that act on an actor id, where
 * 0xFF means the script owner (byte 0 of g_field_script).
 */

s32 func_8008C2EC(s32 arg0, s32 arg1);
void field_activate_actor_resource_slot(s32 arg0, s32 arg1, s32 arg2);
s32 func_800C2DC0(void);
s32 func_800C2D08(void);
s32 func_800C318C(s32 arg0);
s32 func_80087F44(s32 arg0, s32* out);
void func_8008A580(s32 arg0, s32 arg1);
void func_8008B500(s32 arg0, s32 arg1);
u8* func_800C1E40(s32);
s32 func_800C38C8(u8*);
void func_80089AE4(s32 arg0, s32 arg1);
s32 func_80087770(s32 arg0, s32 arg1);
void func_800A3904(s32 arg0, s32 arg1, s32 arg2);
void func_800A3938();
s32 func_800878B4(s32 arg0);
void field_set_all_actor_render_state(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
void func_800C28B8(s32 arg0);
void func_80087A9C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 arg8, s32 arg9);
void func_800B0710(s32, s32, s32, s32);
void func_800B2844(s32 arg0, void* arg1, s32 arg2);
void func_800B28E0(s32, s32, s32);
void func_800B286C(s32, s32, s32);
void field_begin_gover_transition(s32 arg0, s32 arg1, s32 arg2);
void akao_cmd_a9(s32 arg0, s32 arg1);
void func_80089D44(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void field_script_op_00(void);
extern s32 g_layout_option;
extern s32 g_layout_sub_mode;
void field_set_all_actor_render_state(s32, s32, s32, s32, s32);
void func_80087A9C(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 g_layout_flag;

/**
 * @brief Run a field script until it yields, preserving any enclosing script context.
 * @param context Script context to run.
 */
void field_script_run(FieldScriptContext* context)
{
    s32 status;
    u32 opcode_value;
    u32 wait_count;
    u32 wait_state;
    FieldScriptContext* previous_context;
    u8* pc;
    s32 opcode;
    FieldScriptRecordState* wait_record;
    FieldScriptContext* current_context;
    FieldScriptContext* stop_context;
    FieldScriptRecordState* active_record;
    void (**op_table)();
    s32 pc_advance;
    s32 owner_id;
    s32 invalid_opcode;

    previous_context = g_field_script;
    g_field_script = context;
    func_800BD520(g_field_script->status.owner_id, 0xD000, ((u8*)func_800C1B60(context->status.owner_id))[5]);
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
    invalid_opcode = *pc;
    active_record->pc = pc + pc_advance;
    record_game_diagnostic(0x8001, pc_advance, owner_id, invalid_opcode);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
    goto restore_context;

stop_script_extended:
    pc_advance = 1;
    owner_id = current_context->status.owner_id;
    invalid_opcode = *pc;
    active_record->pc = pc + pc_advance;
    record_game_diagnostic(0x8001, pc_advance, owner_id, invalid_opcode);
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
            active_record = (FieldScriptRecordState*)((u8*)current_context + (current_context->active_record * 3 << 2));
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
                invalid_opcode = *pc;
                active_record->pc = pc + pc_advance;
                record_game_diagnostic(0x8001, pc_advance, owner_id, invalid_opcode);
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
    u8* temp_a1;
    SeqRec* temp_a2;
    s32 temp_a3;
    s32 temp_s1;
    s32 r;
    s32 high;

    temp_a1 = (u8*)((SeqRec*)((u8*)g_field_script + (((SeqRec*)g_field_script)->unk4 * 3 << 2)))->unk8;
    temp_s0 = temp_a1[1];
    temp_s1 = temp_a1[0] - 0x40;
    high = temp_s0 >> 4;
    ((SeqRec*)((u8*)g_field_script + (((SeqRec*)g_field_script)->unk4 * 3 << 2)))->unk8 = (s32)func_800B84B4(temp_s0 & 0xF, temp_a1 + 2, &sp10);
    r = (s32)func_800B84B4(high, (u8*)((SeqRec*)((u8*)g_field_script + (((SeqRec*)g_field_script)->unk4 * 3 << 2)))->unk8, &sp14);
    temp_a3 = ((SeqRec*)g_field_script)->unk4;
    temp_a2 = (SeqRec*)((u8*)g_field_script + (temp_a3 * 3 << 2));
    temp_a2->unk8 = r;
    D_800F0D48[temp_s1](sp10, sp14);
}

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
    s32 i;
    s32 j;
    FieldScriptRecord* rec;
    FieldScriptRecord* rec2;

    rec = (FieldScriptRecord*)g_field_script;
    i = g_field_script->active_record;
    rec += i;
    func_800BD6F4(rec->pc[1], (u8*)D_80122B78 + 0x24);

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
    u8* pc;
    u8* initial_next;
    u8 op;
    ScanRecord* rec;

    {
        u8* base;
        s32 index;
        base = *(u8* volatile*)&g_field_script;
        index = ((ScanRecord*)base)->unk4;
        pc = ((ScanRecord*)(base + (index * 3 << 2)))->unk8;
    }
    initial_next = field_script_read_operand(pc[1] & 3, pc + 2, (s32*)&value);
    end_op = 0xFF;
    {
        u8* base;
        s32 index;
        ScanRecord* current;
        base = *(u8* volatile*)&g_field_script;
        index = ((ScanRecord*)base)->unk4;
        current = (ScanRecord*)(base + (index * 3 << 2));
        current->unk8 = initial_next;
    }
    while (1)
    {
        {
            u8* base;
            s32 index;
            base = *(u8* volatile*)&g_field_script;
            index = ((ScanRecord*)base)->unk4;
            rec = (ScanRecord*)(base + (index * 3 << 2));
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
    u8* operands;
    s32 value;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_3(descriptor), operands + 2, &source_owner);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc, &source_ref);
    do
    {
        value = func_800BD3B0(source_owner, source_ref << 16);
        {
            u8* destination_operands;
            destination_operands = (u8*)g_field_script;
            destination_operands += ((FieldScriptRecord*)destination_operands)->unk4 * 3 << 2;
            destination_operands = ((FieldScriptRecord*)destination_operands)->pc;
            FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_2(descriptor), destination_operands + 2, &destination_owner);
        }
    } while (0);
    {
        void func_800BD434(s32 arg0, FieldScriptVariableRef arg1, s32 arg2);
        u8* next;
        next = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc, &destination_ref.value);
        FIELD_SCRIPT_ACTIVE_RECORD()->pc = next;
        func_800BD434(destination_owner, destination_ref, value);
    }
}

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
    u8* opcode_pc;
    u8* operand_pc;
    u8* selector_pc;
    u32 operand_type;
    s32 base_selector;
    s32 opcode;
    FieldScriptRecord* opcode_record;
    FieldScriptRecord* operand_record;
    FieldScriptRecord* selector_record;

    opcode_record = FIELD_SCRIPT_ACTIVE_RECORD();
    opcode_pc = opcode_record->pc;
    opcode = *opcode_pc;
    opcode_record->pc = (u8*)(opcode_pc + 1);
    operand_record = FIELD_SCRIPT_ACTIVE_RECORD();
    operand_pc = operand_record->pc;
    operand_type = *operand_pc;
    operand_record->pc = (u8*)(operand_pc + 1);
    selector_record = FIELD_SCRIPT_ACTIVE_RECORD();
    selector_pc = selector_record->pc;
    base_selector = *selector_pc;
    selector_record->pc = (u8*)(selector_pc + 1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(operand_type, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &target_index);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(3U, FIELD_SCRIPT_ACTIVE_RECORD()->pc, (s32*)&packed_field);
    operand_type >>= 2;
    switch (base_selector)
    {
    case 0:
        selected_base = func_80087F0C(target_index);
    default:
        break;
    case 1:
        selected_base = (s32)(D_80122B78 + 0x400);
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
        s32* base_ptr = (s32*)&D_80122B74;
        s32 offset = target_index * 0x250 + 0x5F0;
        selected_base = *base_ptr + offset;
        break;
    }
    case 7:
        selected_base = (s32)D_80122B74;
        break;
    }
    if (opcode == 0xC)
    {
        value = func_800BD650(packed_field >> 0x1E, selected_base, (packed_field >> 0x10) & 0x3FFF, *((u8*)&packed_field + 1), (s32)(u8)packed_field);
        FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc, &destination_ref);
        func_800BD434(g_field_script->status.owner_id, destination_ref << 0x10, value);
        return;
    }
    {
        u8* next;
        u8** pc;
        s32 call_base;
        next = field_script_read_operand(operand_type, FIELD_SCRIPT_ACTIVE_RECORD()->pc, (s32*)&value);
        call_base = selected_base;
        pc = &FIELD_SCRIPT_ACTIVE_RECORD()->pc;
        *pc = next;
        func_800BD55C(packed_field >> 0x1E, call_base, (packed_field >> 0x10) & 0x3FFF, *((u8*)&packed_field + 1), (s32)(u8)packed_field, value);
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

/**
 * @brief Push a script record and resolve its new program counter.
 * @note Clamp the depth at seven and record a diagnostic on overflow.
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
        record_game_diagnostic(0x8001, 2, g_field_script->status.owner_id, 0x4B);
    }
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->pc = FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->pc;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->flags = FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->flags;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->wait |= 1;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->wait &= 1;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->pc =
        field_script_read_u16(FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->pc + 1, &operand);
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->pc = func_80087EF0(func_800BD3B0(g_field_script->status.owner_id, operand << 16) & 0x7FFF);
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

/**
 * @brief Evaluate the active field-script condition and advance or pause the script record.
 */
void func_800B9868(void)
{
    FieldScriptRecord* rec;
    u8* pc;
    u32 code;
    s32 flag;
    s32 v0;
    FieldScriptContext* ctx;
    s32 active_record;

    active_record = g_field_script->active_record;
    ctx = g_field_script;
    rec = (FieldScriptRecord*)((u8*)ctx + ((active_record * 3) << 2));
    pc = rec->pc;
    code = pc[1];
    switch (code)
    {
    case 1:
        flag = ((StructB78Local*)D_80122B78)->unk402 & 1;
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
    rec = (FieldScriptRecord*)((u8*)ctx + ((active_record * 3) << 2));
    rec->pc += 2;
    if ((u32)(code - 3) < 2)
    {
        func_800BD520(0, 0x7100, D_8011F428);
    }
}

/**
 * @brief Handle a script mode query, advancing the PC or clearing the run flag.
 * @note A 0xFF operand selects the mode from the shared field state.
 * @see decomp.me (100%)
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
        resolved = (((StructB800B99A8*)D_80122B78)->unk41C >> 8) & 3;
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
            goto advance_pc;
        }
        goto clear_flag;
    }
    if (result == 3)
    {
    advance_pc:
        active_record = g_field_script->active_record;
        ctx = g_field_script;
        rec = (FieldScriptRecord*)((u8*)ctx + ((active_record * 3) << 2));
        rec->pc += 2;
        return;
    }
clear_flag:
    g_field_script->status.word &= 0x7FFFFFFF;
}

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

void func_800BD434(s32, FieldScriptVariableRef, s32);

/**
 * @brief Opcode 0x1D: write an actor position into three packed script variables.
 */
void func_800B9FF8(void)
{
    s32 position[4];
    FieldScriptPositionOperands operands;
    s32 scratch[4];
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

    func_80087F44(operands.actor, position);
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
    FieldScriptContext* ctx = g_field_script;
    FieldScriptRecord* rec;

    record_game_diagnostic(0x8001, 1, ctx->status.owner_id, 0x24);

    rec = (FieldScriptRecord*)g_field_script;
    rec += g_field_script->active_record;
    rec->pc += 1;
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
 * @brief Opcode 0x27: reset one of two D_80122B74 slots chosen by the byte operand, then call field_release_actor_resource_slot.
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
 */
void func_800BB3D8(void)
{
    u8* operands;
    u8 descriptor;
    s32 shop_id;
    s32 scale;
    u8* list_base;
    u8* header;
    ShopItemEntry local_buf[32];
    u8* resource;
    s32 i;
    s32 kind;
    u32 lo;
    u32 word;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_0(descriptor), operands + 2, &shop_id);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &scale);

    list_base = func_800C1E40(0xA);
    header = list_base + *(s32*)(list_base + shop_id * 4 + 4);

    resource = func_800C1E40(5);
    i = 0;

    if (*(s32*)header != 0)
    {
        do
        {
            kind = *(header + i * 4 + 4);
            word = *(u32*)(header + i * 4 + 4);
            local_buf[i].pad2 = 0;
            local_buf[i].price = (s16)(kind + ((word << 7) & 0x8000));
            lo = (*(u32*)(header + i * 4 + 4) >> 9) * scale;
            local_buf[i].scaled = (s32)(lo >> 4);
            i++;
        } while ((u32)i < *(s32*)header);
    }

    field_open_shop_mode_1(*(s32*)header, (s32)local_buf, (s32)(resource + 4), 2);
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
    s32 value;
    s32 flag_mask;
    s32 i;
    s32 record_index;
    u8 subcommand;
    FieldScriptRecord* record;
    u8* counter_ptr;
    FieldScriptRecord* active_record;
    u8* word_ptr;
    FieldResetSlotView* slot;
    s32 buffer_base;

    active_record = (FieldScriptRecord*)g_field_script;
    record_index = active_record->unk4;
    active_record += record_index;
    subcommand = active_record->pc[1];
    switch (subcommand)
    {
    case 0:
        value = 0xFFFFFF;
        i = 0xA;
        word_ptr = D_80122B74 + 0x28;
        do
        {
            *(s32*)(word_ptr + 0x34) = value;
            i -= 1;
            word_ptr -= 4;
        } while (i >= 0);
        *(s32*)(D_80122B74 + 0x60) = 0x500;
        *(s32*)(D_80122B74 + 0x64) = -0x8000;
        record = (FieldScriptRecord*)g_field_script;
        *(s32*)(D_80122B74 + 0x68) = 0x803F;
        value = record->unk4;
        record += value;
        record->pc += 2;
        return;
    case 1:
        func_800B34D0(1);
        break;
    case 3:
        value = (s32)func_800C1E40(6);
        buffer_base = (s32) * (u8* volatile*)&D_80122B78;
        record = *(FieldScriptRecord* volatile*)&g_field_script;
        *(s32*)(buffer_base + 0xF00) = value;
        value = record->unk4;
        record += value;
        record->pc += 2;
        return;
    case 4:
    {
        FieldScriptRecord* current_record;

        func_800C1EC8(0, (s32*)(D_80122B74 + 0xE4), 0x200);
        current_record = (FieldScriptRecord*)g_field_script;
        current_record += current_record->unk4;
        current_record->pc += 2;
        return;
    }
    case 5:
        func_800C1230(0);
        func_800C1230(1);
        func_800C1230(2);
        func_800C1230(3);
        func_800C1230(4);
        break;
    case 6:
        i = 0;
        flag_mask = 0x7FFFFFFF;
        do
        {
            slot = (FieldResetSlotView*)(D_80122B74 + i * 0x60);
            slot->count = 0;
            slot->flags = (s32)(slot->flags & flag_mask);
            i += 1;
        } while (i < 5);
        break;
    case 7:
        i = 0;
        do
        {
            counter_ptr = D_80122B74 + i;
            i += 1;
            *(u8*)(counter_ptr + 0x25E0) = 0x63;
        } while (i < 0xFD);
        /* fallthrough */
    default:
        break;
    }
    active_record = (FieldScriptRecord*)g_field_script;
    record_index = active_record->unk4;
    active_record += record_index;
    active_record->pc += 2;
}

void func_800BB9C0(s32 arg0, s32 arg1)
{
    func_800BD520(g_field_script->status.owner_id, arg0, arg1);
}

void func_800BB9F4(s32 arg0, s32 arg1)
{
    field_control_animation(0, arg0, arg1, 1);
}

/**
 * @param arg0 Passed through to field_find_or_load_resource_entry.
 * @param arg1 Passed through to field_find_or_load_resource_entry.
 * @see decomp.me (100%) N/A -- trivial 8-instruction wrapper function, no scratch needed.
 */
void func_800BBA24(s32 arg0, s32 arg1)
{
    field_find_or_load_resource_entry(arg0, arg1);
}

/**
 * @brief Update the low flag bit of the active sequence record.
 *
 * When @p arg0 is zero, resolves a target index (from @c g_field_script[0] when
 * @p arg1 is the 0xFF sentinel, else @p arg1), runs func_800C1FFC for that
 * index, and writes the result's low bit into bit 0 of the current record's
 * @c unkC field (the record chosen by @c g_field_script->unk4).
 *
 * @param arg0 Guard; the update runs only when zero.
 * @param arg1 Target index, or 0xFF to read the default from @c g_field_script[0].
 * @see decomp.me (100%) TODO
 */
void func_800BBA44(s32 arg0, s32 arg1)
{
    s32 var_a0;
    PairSeqRec* temp_a1;
    s32 ret;

    if (arg0 == 0)
    {
        if (arg1 == 0xFF)
        {
            var_a0 = g_field_script->status.owner_id;
        }
        else
        {
            var_a0 = arg1;
        }
        ret = func_800C1FFC(var_a0, 0x1100, 0x1100);
        temp_a1 = (PairSeqRec*)((u8*)g_field_script + (((PairSeqRec*)g_field_script)->unk4 * 0xC));
        temp_a1->unkC = (temp_a1->unkC & ~1) | (ret & 1);
    }
}

/**
 * @brief Dispatch a two-operand runtime subcommand and update script or field state.
 * @see decomp.me (100%)
 */
void func_800BBAC8(u32 command, s32 operand)
{
    s32 record_offset;
    s32 actor_index;
    u8* script_record;
    u8* record;
    u8* low_flags_record;
    u8* high_flags_record;

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
        func_800681C0((s32)operand);
        return;
    case 0x1:
        func_800A5670((s32)operand);
        return;
    case 0x2:
        func_800C2094((s32)operand);
        return;
    case 0x3:
        func_8008BD88((s32)operand);
        return;
    case 0x4:
        func_8005B0F4((s32)operand, 1);
        return;
    case 0x5:
        func_8005B0F4((s32)operand, 0);
        return;
    case 0x6:
        record = D_80122B74 + (operand * 0xC);
        record[0x2F0] = (u8)(record[0x2F0] | 4);
        return;
    case 0x7:
        field_control_animation(0, operand, 0, 2);
        return;
    case 0x8:
        field_control_animation(0, (s32)operand, -1, 4);
        return;
    case 0x9:
        func_800A37E4();
        return;
    case 0xA:
        field_run_zukan((s32)operand);
        return;
    case 0xB:
        func_800C5704((s32)operand);
        return;
    case 0xC:
        field_open_gosub_screen_sequence(D_80122B78 + (operand * 4));
        return;
    case 0xD:
        func_800BE710((s32)operand);
        return;
    case 0xE:
        (*(s32*)(D_80122B78 + 0x404)) = (s32)((*(s32*)(D_80122B78 + 0x404)) | 0x8000);
        func_800A38D4();
        return;
    case 0xF:
        func_800C35AC((s32)operand);
        return;
    case 0x10:
        func_800C31BC((s32)operand);
        return;
    case 0x11:
        func_800C1D14((s32)actor_index, 0);
        return;
    case 0x12:
        func_800BD520(0, 0x7100, func_800C20D8((s32)operand));
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
        func_80089A68((s32)actor_index);
        return;
    case 0x17:
        func_80089980((s32)actor_index);
        return;
    case 0x18:
        func_800BD520(0, 0x7100, func_800C2264((s32)operand));
        return;
    case 0x19:
        func_800BD520(0, 0x7100, func_800C23F4());
        return;
    case 0x1A:
        func_800C2848((s32)actor_index, 2);
        return;
    case 0x1B:
        func_800B22F0(0x80, operand & 0xFFFF);
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
        func_800C1D68();
        return;
    case 0x28:
        func_800C1E08();
        return;
    case 0x29:
        func_800C299C((s32)operand);
        return;
    case 0x2A:
        func_800AD030((s32)operand);
        return;
    case 0x2B:
        func_800BD520(0, (s32)operand, 1);
        return;
    case 0x2C:
        func_800C28B8((s32)actor_index);
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
        D_80117EC4 = (s32)operand;
        return;
    case 0x34:
        func_800BD520(0, 0x7100, func_800C2724((s32)operand));
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
        func_800B66F0((s32)actor_index);
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
        PhaseWord* state = (PhaseWord*)(D_80122B74 + 0x2E4);
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
        func_8008BD88((s32)actor_index);
        return;
    case 0x41:
        func_800C06E8();
        return;
    case 0x42:
        func_8005B1EC();
        return;
    case 0x2F:
    case 0x43:
        func_8005B288((s32)operand);
        return;
    case 0x44:
        D_8010D020 = (s32)operand;
        return;
    case 0x45:
        akao_stop_song(0);
        return;
    case 0x46:
        akao_cmd_f1();
        return;
    case 0x47:
        func_800B60DC((s32)operand);
        return;
    case 0x48:
        func_800C1230((s32)operand);
        return;
    case 0x49:
        g_script_pair_value_49 = (s32)operand;
        return;
    case 0x4A:
        script_record = (u8*)g_field_script + (g_field_script->active_record * 0xC);
        (*(s32*)(script_record + 0x10)) = (s32)(((*(s32*)(script_record + 0x10)) & 1) | (operand * 2));
        g_field_script->status.word = (s32)((s32)g_field_script->status.word & 0x7FFFFFFF);
        return;
    case 0x4B:
        g_gosub_result_count = 1;
        g_gosub_result_values = (s32)operand;
        return;
    case 0x4C:
        (*(s32*)(D_80122B74 + 0x2C)) = (s32)operand;
        return;
    case 0x4D:
        g_music_track_index = 0;
        func_800AD030(0);
        func_800BCCE0(0xFFFE, 0, 0, 0);
        return;
    case 0x4E:
        (*(s32*)(D_80122B74 + 0x28)) = (s32)((*(s32*)(D_80122B74 + 0x28)) | 0xC);
        func_800C1EC8(0, D_80122B74 + 0xE4, 0x200);
        func_800C1EC8(0, D_80122B74 + 0x2E4, 0x30C);
        actor_index = 0;
        D_80122B74[0x2E4] = 1;
        record_offset = 0;
        (*(s32*)(D_80122B74 + 0x2E8)) = (s32)((*(s32*)(D_80122B74 + 0x2E8)) | 0x10000000);
        do
        {
            low_flags_record = D_80122B74 + record_offset;
            low_flags_record[0x2F1] = (u8)(low_flags_record[0x2F1] | 0xF);
            actor_index += 1;
            high_flags_record = D_80122B74 + record_offset;
            high_flags_record[0x2F1] = (u8)(high_flags_record[0x2F1] | 0xF0);
            record_offset += 0xC;
        } while (actor_index < 0x40);
        (*(s32*)(D_80122B74 + 0x2F0)) = (s32)((*(s32*)(D_80122B74 + 0x2F0)) | 1);
        D_80122B74[0x2F3] = 1;
        (*(s32*)(D_80122B74 + 0x2F0)) = (s32)((*(s32*)(D_80122B74 + 0x2F0)) | 4);
        (*(s32*)(D_80122B74 + 0x2FC)) = (s32)((*(s32*)(D_80122B74 + 0x2FC)) | 4);
        (*(s32*)(D_80122B74 + 0x308)) = (s32)((*(s32*)(D_80122B74 + 0x308)) | 4);
        (*(s32*)(D_80122B74 + 0x314)) = (s32)((*(s32*)(D_80122B74 + 0x314)) | 4);
        (*(s32*)(D_80122B74 + 0x320)) = (s32)((*(s32*)(D_80122B74 + 0x320)) | 4);
        (*(s32*)(D_80122B74 + 0x32C)) = (s32)((*(s32*)(D_80122B74 + 0x32C)) | 4);
        (*(s32*)(D_80122B74 + 0x338)) = (s32)((*(s32*)(D_80122B74 + 0x338)) | 4);
        (*(s32*)(D_80122B74 + 0x470)) = (s32)((*(s32*)(D_80122B74 + 0x470)) | 4);
        break;
    }
}

/**
 * @brief Forward an actor id to func_8008B1C8.
 * @param arg0 Actor id, or 0xFF for the script owner.
 */
void func_800BC268(s32 arg0)
{
    s32 var_a0;

    var_a0 = arg0;
    if (var_a0 == 0xFF)
    {
        var_a0 = g_field_script->status.owner_id;
    }
    func_8008B1C8(var_a0);
}

/**
 * @brief Update the condition bit of the active script record from a pair query.
 *
 * Resolves @p arg0 and @p arg1 (each the script owner when 0xFF), runs
 * func_8008C2EC for the pair, and stores the result's low bit into bit 0 of
 * the active record's flags word.
 *
 * @param arg0 First actor id, or 0xFF for the script owner.
 * @param arg1 Second actor id, or 0xFF for the script owner.
 * @see decomp.me (100%) TODO
 */
void func_800BC2A0(s32 arg0, s32 arg1)
{
    FieldScriptRecordState* temp_a1;
    s32 ret;
    s32 var_a0;
    s32 var_a1;

    if (arg0 == 0xFF)
    {
        var_a0 = g_field_script->status.owner_id;
    }
    else
    {
        var_a0 = arg0;
    }
    if (arg1 == 0xFF)
    {
        var_a1 = g_field_script->status.owner_id;
    }
    else
    {
        var_a1 = arg1;
    }
    ret = func_8008C2EC(var_a0, var_a1);
    temp_a1 = FIELD_SCRIPT_ACTIVE_RECORD_STATE();
    temp_a1->flags = (temp_a1->flags & ~1) | (ret & 1);
}

/**
 * @brief Dispatch a resolved sequence action and latch a scene-state flag.
 *
 * Runs func_800C2B14 for @p arg1, resolves @p arg0 (0xFF is the script
 * owner), and forwards the pair to field_activate_actor_resource_slot. When @c D_8010AE78 is set
 * it triggers func_80087FC0 and rewrites bits 17-19 of the word at
 * @c D_80122B78 + 0x400 to 0x20000. Always finishes by writing @p arg1 to
 * script variable 0x2F08.
 *
 * @param arg0 Actor id, or 0xFF for the script owner.
 * @param arg1 Secondary parameter forwarded to the dispatched calls.
 * @see decomp.me (100%) TODO
 */
void func_800BC328(s32 arg0, s32 arg1)
{
    s32 var_a0;

    func_800C2B14(arg1);
    if (arg0 == 0xFF)
    {
        var_a0 = g_field_script->status.owner_id;
    }
    else
    {
        var_a0 = arg0;
    }
    field_activate_actor_resource_slot(var_a0, arg1, 0);
    if (D_8010AE78 != 0)
    {
        func_80087FC0(1, 2);
        *(s32*)(D_80122B78 + 0x400) = (*(s32*)(D_80122B78 + 0x400) & 0xFFF9FFFF) | 0x20000;
    }
    func_800BD520(0, 0x2F08, arg1);
}

/**
 * @brief Pick a value from func_800C2DC0 or func_800C2D08, apply it to the actor when valid, and write it to script variable 0x2F00.
 * @param arg0 Actor id, or 0xFF for the script owner.
 * @param arg1 1 selects func_800C2DC0, anything else func_800C2D08.
 */
void func_800BC3DC(s32 arg0, s32 arg1)
{
    s32 var_s1;
    s32 var_s0;

    if (arg0 == 0xFF)
    {
        var_s1 = g_field_script->status.owner_id;
    }
    else
    {
        var_s1 = arg0;
    }
    if (arg1 == 1)
    {
        var_s0 = func_800C2DC0();
    }
    else
    {
        var_s0 = func_800C2D08();
    }
    if (var_s0 != 0xFF)
    {
        field_activate_actor_resource_slot(var_s1, var_s0, 1);
    }
    func_800BD520(0, 0x2F00, var_s0);
}

/**
 * @brief Apply func_800C318C's value to the actor through field_activate_actor_resource_slot and write it to script variable 0x2F00.
 * @param arg0 Actor id, or 0xFF for the script owner.
 * @param arg1 Value passed to func_800C318C.
 */
void func_800BC474(s32 arg0, s32 arg1)
{
    s32 value;

    value = func_800C318C(arg1);
    if (arg0 == 0xFF)
    {
        arg0 = g_field_script->status.owner_id;
        field_activate_actor_resource_slot(arg0, value, 1);
    }
    else
    {
        field_activate_actor_resource_slot(arg0, value, 1);
    }
    func_800BD520(0, 0x2F00, value);
}

/**
 * @brief Route a resolved actor index to one of two handlers, then clear bit 31 of the script status word.
 *
 * When bit 0x10000 of the word at @c D_80122B78 + 0x400 is set and the index
 * is below 3, calls func_80087E00 with func_800C2928's record; otherwise calls
 * func_80087CE0 with the index.
 *
 * @param arg0 Actor index, or 0xFF for the script owner.
 * @param arg1 Forwarded (low 16 bits) to func_800C2928.
 * @see decomp.me (100%) TODO
 */
void func_800BC4E8(s32 arg0, s32 arg1)
{
    s32 var_a0;
    u32 temp_s0;

    var_a0 = arg0;
    if (arg0 == 0xFF)
    {
        var_a0 = g_field_script->status.owner_id;
    }
    temp_s0 = var_a0 & 0xFF;
    if ((*(s32*)(D_80122B78 + 0x400) & 0x10000) && temp_s0 < 3)
    {
        func_80087E00(temp_s0, func_800C2928(temp_s0, arg1 & 0xFFFF));
    }
    else
    {
        func_80087CE0(var_a0 & 0xFF);
    }
    g_field_script->status.word = g_field_script->status.word & 0x7FFFFFFF;
}

/**
 * @brief Call func_8005AF04 in mode 1 with 0xFF mapped to -1.
 * @param arg0 Object index.
 * @param arg1 Part index; 0xFF becomes -1.
 */
void func_800BC58C(s32 arg0, s32 arg1)
{
    if (arg1 == 0xFF)
    {
        arg1 = -1;
    }
    func_8005AF04(arg0, arg1, 1);
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
 * @brief Re-emit an actor's position with its Y replaced by -arg1 and X and Z scaled down by 256.
 * @param arg0 Actor id, or 0xFF for the script owner.
 * @param arg1 Negated and used as the new Y.
 */
void func_800BC5E4(s32 arg0, s32 arg1)
{
    s32 v;
    s32 buf[4];

    if (arg0 == 0xFF)
    {
        v = g_field_script->status.owner_id;
    }
    else
    {
        v = arg0;
    }
    func_80087F44(v, buf);
    buf[1] = -arg1;
    func_80087D8C(v, buf[0] >> 8, buf[1], buf[2] >> 8);
}

/**
 * @brief Route an actor to func_8008A580 or func_8008B500 depending on bit 15 of arg1.
 * @param arg0 Actor id, or 0xFF for the script owner.
 * @param arg1 Bit 15 selects func_8008A580 with the low 15 bits; otherwise func_8008B500 gets it whole.
 */
void func_800BC65C(s32 arg0, s32 arg1)
{
    if (arg0 == 0xFF)
    {
        arg0 = g_field_script->status.owner_id;
    }

    if (arg1 & 0x8000)
    {
        func_8008A580(arg0, arg1 & 0x7FFF);
    }
    else
    {
        func_8008B500(arg0, arg1);
    }
}

/**
 * @brief Build a shop inventory from a packed item list and open shop mode one.
 * @param list_index Index of the packed shop list to load.
 * @param price_scale Scale applied to each generated item price.
 */
void func_800BC6B0(s32 list_index, s32 price_scale)
{
    u8* lists;
    u8* list;
    u8* items;
    ScriptShopEntry entries[32];
    s32 index;
    s32 price;
    u32 scaled;
    u8* item;

    lists = func_800C1E40(0xA);
    list = lists + *(u32*)(lists + list_index * 4 + 4);
    items = func_800C1E40(5);
    index = 0;

    if (*(u32*)list != 0)
    {
        do
        {
            if ((*(u32*)(list + index * 4 + 4) >> 8) & 1)
            {
                entries[index].item = *(u8*)(list + index * 4 + 4) | 0x8000;
                item = items + ((*(u8*)(list + index * 4 + 4) << 6) + 4);
                entries[index].unused = 0;
                price = func_800C38C8(item);
                *(s32*)(item + 0x34) = price;
                scaled = (u32)(price * price_scale) >> 3;
                entries[index].price = scaled;
            }
            else
            {
                entries[index].item = *(u8*)(list + index * 4 + 4);
                entries[index].unused = 0;
                scaled = (s32)((*(u32*)(list + index * 4 + 4) >> 9) * price_scale) >> 3;
                entries[index].price = scaled;
            }
            index++;
        } while ((u32)index < *(u32*)list);
    }

    field_open_shop_mode_1(*(u32*)list, entries, items + 4, 2);
}

/**
 * @brief Resolve a target index and clear two flag bits on its state record.
 *
 * When @p arg0 is the sentinel 0xFF the index is the script owner; otherwise
 * it is @p arg0 itself. The resolved index selects a state record via
 * func_800C1B60, whose @c unk90 field has bits 31 and 29 cleared, then the
 * index and @p arg1 are dispatched to func_80089AE4.
 *
 * @param arg0 Target index, or 0xFF for the script owner.
 * @param arg1 Forwarded to func_80089AE4.
 * @see decomp.me (100%) TODO
 */
void func_800BC7EC(s32 arg0, s32 arg1)
{
    s32 var_s0;
    SomeStruct* temp_v0;

    if (arg0 == 0xFF)
    {
        var_s0 = g_field_script->status.owner_id;
    }
    else
    {
        var_s0 = arg0;
    }
    temp_v0 = (SomeStruct*)func_800C1B60(var_s0);
    temp_v0->unk90 &= 0x7FFFFFFF;
    temp_v0->unk90 &= 0xDFFFFFFF;
    func_80089AE4(var_s0, arg1);
}

/**
 * @brief Write func_80087770's result for two actors to script variable 0x7100.
 * @param arg0 First actor id, or 0xFF for the script owner.
 * @param arg1 Second actor id, or 0xFF for the script owner.
 */
void func_800BC86C(s32 arg0, s32 arg1)
{
    arg0 = (arg0 == 0xFF) ? g_field_script->status.owner_id : arg0;
    arg1 = (arg1 == 0xFF) ? g_field_script->status.owner_id : arg1;

    func_800BD520(0, 0x7100, func_80087770(arg0, arg1));
}

/**
 * @brief Copy script variable arg1 to script variable arg0 in the owner's slot.
 * @param arg0 Destination variable id.
 * @param arg1 Source variable id (low 16 bits).
 */
void func_800BC8CC(s32 arg0, s32 arg1)
{
    func_800BD520(g_field_script->status.owner_id, arg0, func_800BD414(g_field_script->status.owner_id, arg1 & 0xFFFF));
}

/**
 * @brief Call func_800A3904 in mode 1 with arg0 clamped to 0x7F and arg1 defaulting to 1.
 * @param arg0 Value clamped to 0x7F.
 * @param arg1 Count; 0 becomes 1.
 */
void func_800BC91C(s32 arg0, s32 arg1)
{
    arg1 = (arg1 != 0) ? arg1 : 1;
    if (arg0 >= 0x80)
    {
        arg0 = 0x7F;
    }
    func_800A3904(1, arg1, arg0);
}

/**
 * @brief Thin stack-frame wrapper around func_800A3858.
 */
void func_800BC960(void)
{
    func_800A3858();
}

/**
 * @brief Call func_800A3904 in mode 0 with arg0 clamped to 0x7F and arg1 defaulting to 1.
 * @param arg0 Value clamped to 0x7F.
 * @param arg1 Count; 0 becomes 1.
 */
void func_800BC980(s32 arg0, s32 arg1)
{
    arg1 = (arg1 != 0) ? arg1 : 1;
    if (arg0 >= 0x80)
    {
        arg0 = 0x7F;
    }
    func_800A3904(0, arg1, arg0);
}

/**
 * @brief Store a 2-bit value into bits 4-5 of the 0xC-byte layout record's status byte.
 * @param arg0 Record index.
 * @param arg1 Value; only the low 2 bits are stored.
 */
void func_800BC9C4(s32 arg0, s32 arg1)
{
    u8* temp_v1;

    temp_v1 = D_80122B74 + arg0 * 0xC;
    temp_v1[0x2F0] = (temp_v1[0x2F0] & 0xCF) | ((arg1 & 3) << 4);
}

/**
 * @brief Fetch an actor's position, scale it down by 256, and forward it to func_80087680.
 * @param arg0 Actor id, or 0xFF for the script owner.
 * @param arg1 Forwarded to func_80087680.
 */
void func_800BC9F8(s32 arg0, s32 arg1)
{
    Struct80087F44 sp18;
    s32 idx;

    if (arg0 == 0xFF)
    {
        idx = g_field_script->status.owner_id;
    }
    else
    {
        idx = arg0;
    }
    func_80087F44(idx, (s32*)&sp18);
    {
        s32 x = sp18.unk0 >> 8;
        s32 y = sp18.unk4 >> 8;
        s32 z = sp18.unk8 >> 8;
        func_80087680(idx, arg1, D_80122B78[0x403], x, y, z);
    }
}

/**
 * @brief Forward two values to func_800A3938.
 * @param arg0 Forwarded unchanged.
 * @param arg1 Forwarded unchanged.
 */
void func_800BCA88(s32 arg0, s32 arg1)
{
    func_800A3938(arg0, arg1);
}

/**
 * @brief Write the layout buffer's word at 0x2C to script variable arg0.
 * @param arg0 Script variable id.
 */
void func_800BCAA8(s32 arg0)
{
    func_800BD520(0, arg0, FIELD_B74->unk2C);
}

/**
 * @brief Write func_800878B4's result for an actor to script variable arg0 in that actor's slot.
 * @param arg0 Script variable id.
 * @param arg1 Actor id, or 0xFF for the script owner.
 */
void func_800BCAD8(s32 arg0, s32 arg1)
{
    s32 v;

    if (arg1 == 0xFF)
    {
        v = g_field_script->status.owner_id;
    }
    else
    {
        v = arg1;
    }
    func_800BD520(v, arg0, func_800878B4(v));
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
 * @param arg0 First diagnostic value.
 * @param arg1 Second diagnostic value.
 */
void func_800BCB68(s32 status, s32 code, s32 arg0, s32 arg1)
{
    record_game_diagnostic(status, code, arg0, arg1);
}

/**
 * @brief Split bit 7 of arg0 into a flag and forward the rest to field_set_all_actor_render_state.
 * @param arg0 Id with an optional 0x80 flag bit.
 * @param arg1 Forwarded as the first argument.
 * @param arg2 Forwarded as the second argument.
 * @param arg3 Forwarded as the third argument.
 */
void func_800BCB88(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 flag;

    if (arg0 & 0x80)
    {
        flag = 1;
        arg0 &= 0x7F;
    }
    else
    {
        flag = 0;
    }
    field_set_all_actor_render_state(arg1, arg2, arg3, flag, arg0);
}

/**
 * @brief Split bit 7 of arg0 into a flag, run func_800C28B8 on the id, then forward everything to func_80087A9C.
 * @param arg0 Actor id with an optional 0x80 flag bit.
 * @param arg1 Forwarded to func_80087A9C.
 * @param arg2 Forwarded to func_80087A9C.
 * @param arg3 Forwarded to func_80087A9C.
 */
void func_800BCBD0(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 var_s0;
    s32 var_s1;

    if (arg0 & 0x80)
    {
        var_s1 = 1;
        var_s0 = arg0 & 0x7F;
    }
    else
    {
        var_s1 = 0;
        var_s0 = arg0;
    }
    func_800C28B8(var_s0);
    func_80087A9C(var_s0, arg1, arg2, arg3, 0, -1, -1, -1, 0, var_s1);
}

/**
 * @brief Empty function; no-op.
 */
void func_800BCC6C(void)
{
}

/**
 * @brief Forward four values to func_800B0710, mapping 0xFF to the script owner in the first and to -1 in the rest.
 * @param arg0 Actor id, or 0xFF for the script owner.
 * @param arg1 0xFF becomes -1.
 * @param arg2 0xFF becomes -1.
 * @param arg3 0xFF becomes -1.
 */
void func_800BCC74(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 v;

    if (arg0 == 0xFF)
    {
        v = g_field_script->status.owner_id;
    }
    else
    {
        v = arg0;
    }
    func_800B0710(v, (arg1 == 0xFF) ? -1 : arg1, (arg2 == 0xFF) ? -1 : arg2, (arg3 == 0xFF) ? -1 : arg3);
}

/**
 * @brief Set pending layout selectors and reset execution to script record zero.
 * @param transition_id Transition identifier stored in the control word's low halfword.
 * @param mode Mode byte stored at offset 0x41A.
 * @param selectors Three packed selector bytes; 0xFE and 0xFF select sentinel behavior.
 * @param flags Five-bit transition flags stored at control bits 24 through 28.
 * @note 100% match with GCC 2.7.2 CDK and GCC 2.8.0 G0: 109 instructions, 436 bytes.
 */
void func_800BCCE0(transition_id, mode, selectors, flags) s16 transition_id;

s8 mode;

s32 selectors;

s32 flags;

{
    s32 layout;
    s32 third_selector;
    s32 option;

    State* state = (State*)D_80122B78;
    option = selectors & 0xFF;
    state->control.word = (s32)(state->control.word | 0x40000000);
    state->control.fields.mode = mode;
    layout = (selectors >> 8) & 0xFF;
    third_selector = (selectors >> 0x10) & 0xFF;
    state->control.fields.id = transition_id;
    switch (option)
    {
    case 0xFE:
        ((State*)D_80122B78)->option = -2;
        break;
    case 0xFF:
        g_layout_option = -1;
        ((State*)D_80122B78)->option = -1;
        break;
    default:
        if (option == g_layout_option)
        {
            ((State*)D_80122B78)->option = -2;
        }
        else
        {
            ((State*)D_80122B78)->option = option;
        }
        break;
    }
    switch (layout)
    {
    case 0xFE:
        ((State*)D_80122B78)->layout = -2;
        break;
    case 0xFF:
        ((State*)D_80122B78)->layout = -1;
        break;
    default:
        if (layout == g_layout_flag)
        {
            ((State*)D_80122B78)->layout = -1;
        }
        else
        {
            ((State*)D_80122B78)->layout = layout;
        }
        break;
    }
    switch (third_selector)
    {
    case 0xFE:
        ((State*)D_80122B78)->third_selector = -2;
        break;
    case 0xFF:
        ((State*)D_80122B78)->third_selector = -1;
        break;
    default:
        ((State*)D_80122B78)->third_selector = third_selector;
        break;
    }
    ((State*)D_80122B78)->control.word = (s32)((((State*)D_80122B78)->control.word & 0xE0FFFFFF) | ((flags & 0x1F) << 0x18));
    g_field_script->active_record = 0;
    g_field_script->status.word = (s32)(g_field_script->status.word & 0x7FFFFFFF);
    ((FieldScriptRecord*)((u8*)g_field_script + ((g_field_script->active_record * 3) << 2)))->pc = 0;
}

/**
 * @brief Opcode 0x86: dispatch an entry of a resource record through func_800B2844.
 *
 * Fetches the record for @p resource_id via func_800C1E40; when non-NULL, reads
 * the halfword at @c entry_index*2 + 4 within it and calls func_800B2844 with
 * the record address offset by that halfword plus 4.
 *
 * @param operand_0 Forwarded to func_800B2844 as its first argument.
 * @param resource_id Record selector passed to func_800C1E40.
 * @param entry_index Halfword index within the record (scaled by 2).
 * @param operand_3 Forwarded to func_800B2844 as its third argument.
 */
void field_script_op_86(s32 operand_0, s32 resource_id, s32 entry_index, s32 operand_3)
{
    u8* p = func_800C1E40(resource_id);

    if (p != NULL)
    {
        u16 h = *(u16*)(p + (entry_index << 1) + 4);
        func_800B2844(operand_0, p + (h + 4), operand_3);
    }
}

/**
 * @brief Opcode 0x87: route an actor to func_800B28E0 or func_800B286C by selector.
 *
 * Selector 0 forwards to func_800B28E0 and selector 1 to func_800B286C, each
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
        func_800B28E0(actor_id, operand_2 & 0xFF, operand_3 & 0xFF);
        break;
    case 1:
        func_800B286C(actor_id, operand_2 & 0xFF, operand_3 & 0xFF);
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
    s32 var_v0;
    s32 var_a3;

    var_v0 = -1;
    if (operand_1 != 0xFF)
    {
        var_v0 = operand_1;
    }
    operand_1 = var_v0;

    var_a3 = -1;
    if (operand_2 != 0xFF)
    {
        var_a3 = operand_2;
    }

    g_layout_option = -1;

    operand_2 = var_a3;

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
 * @brief Opcode 0x8A: forward an actor pair to func_8008B5D0.
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
    func_8008B5D0(resolved_actor, operand_1, 1, &resolved_target);
}

/**
 * @brief Opcode 0x8B: pack three 10-bit fields into the word at 0x410 and store the fourth operand at 0x414.
 * @param field_0 Bits 0-9.
 * @param field_1 Bits 10-19.
 * @param field_2 Bits 20-29.
 * @param operand_3 Stored to unk414.
 */
void field_script_op_8b(s32 field_0, s32 field_1, s32 field_2, s32 operand_3)
{
    StructB78* p;
    u32 raw;
    u32 v;

    p = (StructB78*)D_80122B78;
    raw = p->unk410;
    p->unk414 = operand_3;
    v = raw;
    v &= ~0x3FF;
    v |= field_0 & 0x3FF;
    v &= 0xFFF003FF;
    v |= (field_1 & 0x3FF) << 10;
    v &= 0xC00FFFFF;
    v |= (field_2 & 0x3FF) << 20;
    p->unk410 = v;
}

/**
 * @brief Opcode 0x8C: forward to func_80089D44 with 0xFF operands mapped to the owner id or -1.
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
    func_80089D44(resolved_actor, (operand_1 == 0xFF) ? -1 : operand_1, (operand_2 == 0xFF) ? -1 : operand_2, (operand_3 == 0xFF) ? -1 : operand_3);
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
