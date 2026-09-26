#include "game_audio.h"
#include "akao_cmd.h"
#include "field_scene_transition.h"
#include "field_text.h"
#include "main.h"
#include "common.h"
#include "field_actor_runtime.h"
#include "field_calls.h"
#include "field_scene_internal.h"
#include "field_actor_tables.h"

#include "field_script.h"
#include "field_records.h"
#include "shop.h"

/** @brief FieldGameState.lands viewed as FieldLandWords. */
#define FIELD_LAND_WORDS ((FieldLandWords*)g_field_game_state->lands)

/** @brief func_800C1E40 resources read by the opcodes. */
#define FIELD_RESOURCE_ITEM_TEMPLATES 5
#define FIELD_RESOURCE_TRIGGERS 6
#define FIELD_RESOURCE_SHOP_LISTS 0xA

/** @brief Half extent (x and z) of the stored-position test of opcode 0x43. */
#define FIELD_STORED_POSITION_RANGE 0x1100

/** @brief Field music tracks a script can select (g_music_track_index). */
#define FIELD_MUSIC_TRACK_COUNT 64

/** @brief Loudest volume of field_fade_song. */
#define FIELD_SONG_VOLUME_MAX 0x7F

/** @brief Bits 4-5 of FieldLandRecord::flags, set by opcode 0x56 (TODO meaning unknown). */
#define FIELD_LAND_VALUE_MASK 0x30
#define FIELD_LAND_VALUE_SHIFT 4

/** @brief Most entries a scripted shop list can hold. */
#define FIELD_SHOP_LIST_CAPACITY 32

/** @brief Largest quantity of one item kind. */
#define FIELD_ITEM_QUANTITY_MAX 99
/** @brief Item kinds that opcode 0x0F subcommand 7 fills to FIELD_ITEM_QUANTITY_MAX. */
#define FIELD_DEBUG_ITEM_KIND_COUNT 253

/** @brief First opcode dispatched through g_field_script_pair_op_table. */
#define FIELD_SCRIPT_PAIR_OP_BASE 0x40
/** @brief First opcode past the pair opcodes that have handlers. */
#define FIELD_SCRIPT_PAIR_OP_END 0x60
/** @brief First opcode dispatched through g_field_script_ext_op_table. */
#define FIELD_SCRIPT_EXT_OP_BASE 0x80
/** @brief First opcode past the extended opcode range of the run loop. */
#define FIELD_SCRIPT_EXT_OP_RANGE_END 0xC0
/** @brief Limit of the extended opcode check inside that range (never reached). */
#define FIELD_SCRIPT_EXT_OP_END 0xD0

/** @brief record_game_diagnostic code of an opcode without a handler. */
#define DIAG_SCRIPT_BAD_OPCODE 1

/** @brief Portrait operand of opcode 0x11: no portrait. */
#define FIELD_TALK_NO_PORTRAIT 0xFE
/** @brief Portrait operand of opcode 0x11: the speaker record's selector. */
#define FIELD_TALK_SPEAKER_PORTRAIT 0xFF
/** @brief Window layout that opcode 0x11 always shows without a portrait. */
#define FIELD_TALK_LAYOUT_NO_PORTRAIT 7

/** @brief Operand types of pair and extended opcodes, one per descriptor nibble. */
enum
{
    FIELD_SCRIPT_OPERAND_BYTE = 0,
    FIELD_SCRIPT_OPERAND_HALFWORD = 1,
    FIELD_SCRIPT_OPERAND_WORD = 2,
    /** @brief Halfword variable reference; the value is read with field_get_script_var. */
    FIELD_SCRIPT_OPERAND_VARIABLE = 3,
    FIELD_SCRIPT_OPERAND_HALFWORD_ALT = 4,
    /** @brief Halfword plus 0x10000. */
    FIELD_SCRIPT_OPERAND_HALFWORD_HIGH = 5,
    /** @brief No bytes and no value: the operand keeps its previous contents. */
    FIELD_SCRIPT_OPERAND_NONE_6 = 6,
    FIELD_SCRIPT_OPERAND_NONE_7 = 7,
    /** @brief No bytes; the value is 0. */
    FIELD_SCRIPT_OPERAND_ZERO = 8,
    /** @brief No bytes; the value is 1. */
    FIELD_SCRIPT_OPERAND_ONE = 9,
    /** @brief No bytes; the value is FIELD_SCRIPT_OWNER. */
    FIELD_SCRIPT_OPERAND_OWNER = 10
};

/** @brief Handler of a pair opcode: the two decoded operands. */
typedef void (*FieldDispatchFn)(s32, s32);

/** @brief Decoded operands of opcode 0x1D. */
typedef struct
{
    s32 actor;
    FieldScriptVariableRef x_ref;
} FieldScriptPositionOperands;

/** @brief Record field selector decoded by opcodes 0x0C and 0x0D (read as a word operand). */
typedef union
{
    s32 value;
    struct
    {
        u32 bit_count : 8;
        u32 shift : 8;
        /** @brief Element index in the selected record. */
        u32 index : 14;
        /** @brief Element width (FIELD_BITS_BYTE, _HALFWORD or _WORD of field_read_bits). */
        u32 width : 2;
    } bits;
} FieldScriptFieldSpec;

/** @brief One packed shop list entry. */
typedef union
{
    u32 word;
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

/**
 * @brief A text resource (func_800C1E40): a 4-byte header, then the texts.
 * @note The texts start with a table of 16-bit offsets, one per text, relative to the start of the texts.
 */
typedef struct
{
    u8 header[4];
    union
    {
        u16 offsets[1];
        u8 bytes[1];
    } texts;
} FieldTextResource;

/** @brief Resource 5 (func_800C1E40): the item records a shop can generate. */
typedef struct
{
    u32 header;
    FieldItemRecord records[1];
} FieldItemResource;

/* Opcodes below 0x40 dispatch through this table, indexed by opcode. */
extern void (*g_field_script_op_table[])();
/* Opcodes 0x40 to 0x5F dispatch through this table, indexed by opcode - 0x40. */
extern FieldDispatchFn g_field_script_pair_op_table[];

extern FieldRuntimeContext* g_field_runtime;
extern FieldGameState* g_field_game_state;
extern FieldBattleContext* g_field_battle;
extern FieldItemStaging* D_80123FC4;
extern s32 D_8011F428;
extern s32 D_801227F0;
extern s32 g_field_interaction_active;
extern s32 g_field_hide_actor_panels, g_field_duel_mode, g_field_pair_indicators_disabled, D_80122980;
extern s32 g_gosub_result_count, g_gosub_result_values;

void field_script_op_00(void);
static void field_script_dispatch_pair_op(void);
static void field_script_dispatch_ext_op(void);
static u8* field_script_decode_operand(s32 type, u8* data, s32* value);
void field_script_op_85(s32 scene_id, s32 object_id, s32 audio, s32 spawn_id);

FieldActorRecord* field_find_actor_record_or_default(s32 id);
FieldObjectState* field_find_object_state(s32 key);
u8* field_get_event_script(s32 index);
s32 field_read_actor_binding_state(s32 key);
s32 field_is_actor_idle(s32 key);
s32 field_set_actor_control_mode(s32 key, s32 mode);
s32 field_get_actor_position(s32 key, Vec3i* position);
/* Int parameters on purpose: with the (s32, u8, s8) definition the calls would narrow their arguments. */
s32 field_queue_actor_event(s32 owner_id, s32 event_id, s32 argument);
s32 field_face_actor(s32 source_key, s32 target_key);
s32 field_set_actor_position(s32 key, s32 x, s32 y, s32 z);
s32 field_spawn_targeted_animation_actor(s32 key, s32 resource_index, s32 target_keys, s32* targets);
u16* func_800C1E40(s32 resource_id);
s32* func_800C1EC8(s32* src, s32* dest, s32 size);
s32 field_start_actor_turn(s32 key);
s32 field_toggle_actor_hidden(s32 key);
/* Local: field_contact_geometry.c calls it with a third argument, so it stays out of field_calls.h. */
s32 field_start_interaction(s32 actor_id, s32 script);
void field_stop_actor_script(s32 actor_id, s32 flags);
void field_stop_non_script_actors(void);
void func_800C1E08(void);
s32 field_load_bound_animation(s32 key, s32 resource_id);
s32 field_spawn_shared_animation_actor(s32 key, s32 resource_index);
s32 field_retire_actor(s32 key, s32 resource_index);
s32 field_revive_actor(s32 key, s32 animation, s32 effect, s32 sound);

/**
 * @brief Report an opcode that has no handler, step past it and end the step loop.
 * @param context Script context that holds the opcode.
 * @param record Active record of @p context.
 * @param pc Address of the opcode.
 */
static inline void field_script_reject_opcode(FieldScriptContext* context, FieldScriptRecordState* record, u8* pc)
{
    s32 owner_id;
    s32 opcode;

    owner_id = context->status.owner_id;
    opcode = *pc;
    record->pc = pc + 1;
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, owner_id, opcode);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Report whether a group battle is running (between field_battle_start and field_battle_end).
 * @return 1 while the battle runs, else 0.
 */
static inline s32 field_is_battle_active(void)
{
    return g_field_runtime->state.bits.group_active;
}

/**
 * @brief Run a field script until it yields, preserving any enclosing script context.
 * @param context Script context to run.
 */
void field_script_run(FieldScriptContext* context)
{
    FieldScriptContext* previous_context;
    FieldScriptContext* current_context;
    FieldScriptRecordState* waiting_record;
    FieldScriptRecordState* record;
    s32 status;
    u8* pc;
    u8 opcode;

    previous_context = g_field_script;
    g_field_script = context;
    field_set_script_var(g_field_script->status.owner_id, FIELD_VAR_EVENT_ARGUMENT, field_find_actor_record_or_default(context->status.owner_id)->event_argument);
    waiting_record = FIELD_SCRIPT_ACTIVE_RECORD_STATE();
    if (waiting_record->wait.bits.frames != 0)
    {
        waiting_record->wait.bits.frames--;
    }
    else
    {
        status = g_field_script->status.word;
        status |= FIELD_SCRIPT_RUNNING;
        g_field_script->status.word = status;
        if (status < 0)
        {
            do
            {
                current_context = g_field_script;
                record = FIELD_SCRIPT_ACTIVE_RECORD_STATE();
                pc = record->pc;
                opcode = *pc;
                if (opcode < FIELD_SCRIPT_PAIR_OP_BASE)
                {
                    g_field_script_op_table[*pc]();
                }
                else if (opcode >= FIELD_SCRIPT_PAIR_OP_BASE && opcode < FIELD_SCRIPT_EXT_OP_BASE)
                {
                    if (opcode >= FIELD_SCRIPT_PAIR_OP_END)
                    {
                        field_script_reject_opcode(current_context, record, pc);
                        break;
                    }
                    field_script_dispatch_pair_op();
                }
                else if (opcode >= FIELD_SCRIPT_EXT_OP_BASE && opcode < FIELD_SCRIPT_EXT_OP_RANGE_END)
                {
                    if (opcode >= FIELD_SCRIPT_EXT_OP_END)
                    {
                        field_script_reject_opcode(current_context, record, pc);
                        break;
                    }
                    field_script_dispatch_ext_op();
                }
                else if (opcode >= FIELD_SCRIPT_EXT_OP_RANGE_END)
                {
                    field_script_reject_opcode(current_context, record, pc);
                }
            } while (g_field_script->status.word & FIELD_SCRIPT_RUNNING);
        }
    }
    if (previous_context != NULL)
    {
        g_field_script = previous_context;
    }
}

/**
 * @brief Decode the two operands of a pair opcode (0x40 to 0x5F) and run its handler.
 * @note The descriptor byte after the opcode holds one operand type per nibble, low nibble first.
 */
static void field_script_dispatch_pair_op(void)
{
    s32 first;
    s32 second;
    u8 types;
    u8* pc;
    s32 index;
    s32 second_type;

    pc = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    types = pc[1];
    index = pc[0] - FIELD_SCRIPT_PAIR_OP_BASE;
    second_type = types >> 4;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_decode_operand(types & 0xF, pc + 2, &first);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_decode_operand(second_type, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &second);
    g_field_script_pair_op_table[index](first, second);
}

/**
 * @brief Decode the four operands of an extended opcode (0x80 to 0x8F) and run its handler.
 * @note The two descriptor bytes after the opcode hold one operand type per nibble, low nibble first.
 */
static void field_script_dispatch_ext_op(void)
{
    s32 operand0;
    s32 operand1;
    s32 operand2;
    s32 operand3;
    u8 types01;
    u8 types23;
    s32 index;
    s32 type1;
    s32 type2;
    s32 type3;
    u8* pc;

    pc = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    types01 = pc[1];
    types23 = pc[2];
    index = pc[0] - FIELD_SCRIPT_EXT_OP_BASE;
    type1 = types01 >> 4;
    type2 = types23 & 0xF;
    type3 = types23 >> 4;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_decode_operand(types01 & 0xF, pc + 3, &operand0);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_decode_operand(type1, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &operand1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_decode_operand(type2, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &operand2);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_decode_operand(type3, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &operand3);
    g_field_script_ext_op_table[index](operand0, operand1, operand2, operand3);
}

/**
 * @brief Decode one operand of a pair or extended opcode.
 * @param type Operand type (FIELD_SCRIPT_OPERAND_*).
 * @param data Encoded operand bytes.
 * @param value Receives the decoded value; left unchanged for the types without a value.
 * @return @p data advanced past the operand bytes.
 */
static u8* field_script_decode_operand(s32 type, u8* data, s32* value)
{
    u8* cursor;

    cursor = data;
    switch (type)
    {
    case FIELD_SCRIPT_OPERAND_BYTE:
        *value = *cursor;
        return cursor + 1;
    case FIELD_SCRIPT_OPERAND_HALFWORD:
    case FIELD_SCRIPT_OPERAND_HALFWORD_ALT:
        *value = cursor[0] + (cursor[1] << 8);
        return cursor + 2;
    case FIELD_SCRIPT_OPERAND_WORD:
        *value = cursor[0] + (cursor[1] << 8) + (cursor[2] << 16) + (cursor[3] << 24);
        return cursor + 4;
    case FIELD_SCRIPT_OPERAND_VARIABLE:
        *value = field_get_script_var(g_field_script->status.owner_id, cursor[0] | (cursor[1] << 8));
        return cursor + 2;
    case FIELD_SCRIPT_OPERAND_HALFWORD_HIGH:
        *value = (cursor[0] + (cursor[1] << 8)) + 0x10000;
        return cursor + 2;
    case FIELD_SCRIPT_OPERAND_NONE_6:
    case FIELD_SCRIPT_OPERAND_NONE_7:
        break;
    case FIELD_SCRIPT_OPERAND_ZERO:
        *value = 0;
        break;
    case FIELD_SCRIPT_OPERAND_ONE:
        *value = 1;
        break;
    case FIELD_SCRIPT_OPERAND_OWNER:
        *value = FIELD_SCRIPT_OWNER;
        break;
    }
    return cursor;
}

/**
 * @brief Opcode 0x00: return from a call, or end the script at the outermost record.
 * @note Returning keeps the step loop running only when the returning record was entered with resume set.
 */
void field_script_op_00(void)
{
    s32 depth;

    depth = g_field_script->active_record;
    if (depth > 0)
    {
        if (!FIELD_SCRIPT_RECORD_STATE(depth)->wait.bits.resume)
        {
            g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
        }
        g_field_script->active_record--;
        return;
    }
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = NULL;
}

/**
 * @brief Opcode 0x01: unconditional relative branch by the signed halfword after the opcode.
 */
void field_script_op_01(void)
{
    field_script_branch(1);
}

/**
 * @brief Opcode 0x02: call the subroutine at the relative halfword branch target.
 * @note On overflow of the record stack the call is reported and skipped.
 */
void field_script_op_02(void)
{
    s32 depth;
    s32 next_depth;

    depth = g_field_script->active_record;
    next_depth = depth + 1;
    g_field_script->active_record = next_depth;
    if (next_depth >= FIELD_SCRIPT_FRAME_COUNT)
    {
        FieldScriptRecord* rec;

        record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_FRAME_OVERFLOW, g_field_script->status.owner_id, 0);
        g_field_script->active_record = FIELD_SCRIPT_FRAME_COUNT - 1;
        rec = FIELD_SCRIPT_RECORD(FIELD_SCRIPT_FRAME_COUNT - 1);
        rec->pc += 3;
        return;
    }

    FIELD_SCRIPT_RECORD(next_depth)->pc = FIELD_SCRIPT_RECORD(depth)->pc;
    FIELD_SCRIPT_ACTIVE_RECORD_STATE()->flags = FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->flags;
    FIELD_SCRIPT_ACTIVE_RECORD_STATE()->wait.bits.resume = 1;
    FIELD_SCRIPT_ACTIVE_RECORD_STATE()->wait.bits.frames = 0;
    FIELD_SCRIPT_RECORD(g_field_script->active_record - 1)->pc += 3;
    field_script_branch(1);
}

/**
 * @brief Opcode 0x03: dispatch the byte operand as a small field command.
 */
void field_script_op_03(void)
{
    field_script_command(FIELD_SCRIPT_ACTIVE_RECORD()->pc[1], g_field_runtime->command_params);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
}

/**
 * @brief Opcode 0x04: branch when the condition flag is set, otherwise skip the branch operand.
 */
void field_script_op_04(void)
{
    FieldScriptRecordState* rec;

    rec = FIELD_SCRIPT_ACTIVE_RECORD_STATE();
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

    rec = FIELD_SCRIPT_ACTIVE_RECORD_STATE();
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

    rec = FIELD_SCRIPT_ACTIVE_RECORD_STATE();
    rec->flags = (rec->flags & ~FIELD_SCRIPT_COND) | ((rec->flags >> 1) & 1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc++;
}

/**
 * @brief Opcode 0x08: set the condition flag when a script variable lies within an inclusive range.
 * @note Operands are a halfword variable reference followed by the low and high bounds.
 */
void field_script_op_08(void)
{
    FieldScriptVariableRef var_ref;
    s32 low;
    s32 high;
    u8 descriptor;
    u8* operands;
    s32 value;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(operands + 2, &var_ref.value);
    value = field_read_script_var(g_field_script->status.owner_id, var_ref);
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
 * @brief Opcode 0x09: skip to the case entry that matches an operand, then take its branch.
 * @note The entries that follow are three bytes each (a key byte and a branch halfword); the scan
 *       stops at the key equal to the operand or at the 0xFF default entry.
 */
void field_script_op_09(void)
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
    rec->wait.bits.frames = frames;
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x0B: copy a script variable from one owner to another.
 * @note Operands: source owner, source variable reference, destination owner, destination reference.
 */
void field_script_op_0b(void)
{
    s32 source_owner;
    FieldScriptVariableRef source_ref;
    s32 destination_owner;
    FieldScriptVariableRef destination_ref;
    u8 descriptor;
    u8* operands;
    s32 value;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_3(descriptor), operands + 2, &source_owner);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc, &source_ref.value);
    value = field_read_script_var(source_owner, source_ref);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc + 2, &destination_owner);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc, &destination_ref.value);
    field_write_script_var(destination_owner, destination_ref, value);
}

/**
 * @brief Opcodes 0x0C and 0x0D: read (0x0C) or write (0x0D) a bitfield in a selected game record.
 * @note Operands: descriptor, base selector (0 to 7), record index, field spec, then the
 *       destination variable (0x0C) or the value to write (0x0D).
 */
void field_script_op_0c(void)
{
    s32 value;
    FieldScriptVariableRef destination_ref;
    FieldScriptFieldSpec field;
    s32 target_index;
    void* base;
    u32 operand_type;
    s32 base_selector;
    s32 opcode;

    opcode = *FIELD_SCRIPT_ACTIVE_RECORD()->pc++;
    operand_type = *FIELD_SCRIPT_ACTIVE_RECORD()->pc++;
    base_selector = *FIELD_SCRIPT_ACTIVE_RECORD()->pc++;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(operand_type, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &target_index);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(3, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &field.value);
    operand_type >>= 2;
    switch (base_selector)
    {
    case 0:
        base = field_find_object_state(target_index);
        break;
    default:
        break;
    case 1:
        base = &g_field_runtime->state;
        break;
    case 2:
        base = field_find_actor_record_or_default(target_index);
        break;
    case 3:
        base = D_80123FC4;
        break;
    case 4:
        base = g_field_battle;
        break;
    case 5:
        base = field_find_status_record(target_index);
        break;
    case 6:
        base = &g_field_game_state->characters[target_index];
        break;
    case 7:
        base = g_field_game_state;
        break;
    }
    if (opcode == 0xC)
    {
        value = field_read_bits(field.bits.width, base, field.bits.index, field.bits.shift, field.bits.bit_count);
        FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc, &destination_ref.value);
        field_write_script_var(g_field_script->status.owner_id, destination_ref, value);
        return;
    }
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(operand_type, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &value);
    field_write_bits(field.bits.width, base, field.bits.index, field.bits.shift, field.bits.bit_count, value);
}

/**
 * @brief Opcode 0x0E: branch through a table of signed halfword offsets indexed by a script variable.
 */
void field_script_op_0e(void)
{
    FieldScriptVariableRef var_ref;
    s32 value;

    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc + 1, &var_ref.value);
    value = field_read_script_var(g_field_script->status.owner_id, var_ref);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += value * 2;
    field_script_branch(0);
}

/**
 * @brief Opcode 0x10: call the event script named by a script variable.
 * @note The halfword operand is a variable reference; the low 15 bits of its value select the event
 *       script. On overflow of the frame stack the call is reported and replaces the last frame.
 */
void field_script_op_10(void)
{
    FieldScriptVariableRef script_ref;
    s32 depth;

    depth = g_field_script->active_record + 1;
    g_field_script->active_record = depth;
    if (depth >= FIELD_SCRIPT_FRAME_COUNT)
    {
        g_field_script->active_record = FIELD_SCRIPT_FRAME_COUNT - 1;
        record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_FRAME_OVERFLOW, g_field_script->status.owner_id, 0x4B);
    }
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->pc = FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->pc;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->flags = FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->flags;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->wait.bits.resume = 1;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->wait.bits.frames = 0;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->pc =
        field_script_read_u16(FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->pc + 1, &script_ref.value);
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->pc = field_get_event_script(field_read_script_var(g_field_script->status.owner_id, script_ref) & 0x7FFF);
}

/**
 * @brief Opcode 0x11: open a talk window for a speaker and show a scene string in it.
 * @note Operands (bytes): speaker (0xFF for the owner), string index (halfword), portrait
 *       (FIELD_TALK_NO_PORTRAIT, FIELD_TALK_SPEAKER_PORTRAIT or a portrait selector),
 *       window flags (bits 0-1 window slot, FIELD_TALK_PLANE_NO_FACING, FIELD_TALK_PLANE_FACING)
 *       and window layout.
 */
void field_script_op_11(void)
{
    FieldActorRecord* record;
    s32 speaker;
    s32 string_index;
    s32 window_flags;
    s32 portrait;
    u8 portrait_operand;
    u8 layout;

    if (FIELD_SCRIPT_ACTIVE_RECORD()->pc[1] != FIELD_SCRIPT_OWNER)
    {
        speaker = FIELD_SCRIPT_ACTIVE_RECORD()->pc[1];
    }
    else
    {
        speaker = g_field_script->status.owner_id;
    }

    portrait_operand = FIELD_SCRIPT_ACTIVE_RECORD()->pc[4];
    string_index = FIELD_SCRIPT_ACTIVE_RECORD()->pc[2] + (FIELD_SCRIPT_ACTIVE_RECORD()->pc[3] << 8);

    switch (portrait_operand)
    {
    case FIELD_TALK_NO_PORTRAIT:
        portrait = -1;
        break;
    case FIELD_TALK_SPEAKER_PORTRAIT:
        record = field_find_actor_record_or_default(speaker);
        portrait = -1;
        if (record->selector != FIELD_NO_SELECTOR)
        {
            portrait = record->selector;
        }
        break;
    default:
        portrait = FIELD_SCRIPT_ACTIVE_RECORD()->pc[4];
        break;
    }

    layout = FIELD_SCRIPT_ACTIVE_RECORD()->pc[6];
    window_flags = FIELD_SCRIPT_ACTIVE_RECORD()->pc[5];
    if (layout == FIELD_TALK_LAYOUT_NO_PORTRAIT)
    {
        portrait = -1;
    }
    if (!(window_flags & FIELD_TALK_PLANE_NO_FACING))
    {
        if (window_flags & FIELD_TALK_PLANE_FACING)
        {
            portrait |= FIELD_TALK_FACING_FLAG;
        }
        else
        {
            portrait |= (window_flags & 1) << 6;
        }
    }

    field_open_text_window(window_flags & FIELD_TALK_PLANE_MASK, layout, speaker, portrait);
    field_set_text_window_string(window_flags, string_index, 1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 7;
}

/**
 * @brief Opcode 0x12: end the step loop until a scene, actor or text condition is met.
 * @note Operands (bytes): condition selector, then an animation index, an actor (0xFF for the
 *       owner) or a text window slot. An unknown selector tests an uninitialized value.
 */
void field_script_op_12(void)
{
    s32 selector;
    s32 operand;
    s32 actor;
    s32 wait;

    selector = FIELD_SCRIPT_ACTIVE_RECORD()->pc[1];
    operand = FIELD_SCRIPT_ACTIVE_RECORD()->pc[2];
    switch (selector)
    {
    case 0:
        wait = field_get_animation_state(FIELD_LIST_TILE_ANIMS, operand) != FIELD_ANIM_STATE_FINISHED;
        break;
    case 1:
        wait = field_get_animation_state(FIELD_LIST_TILE_ANIMS, operand) != FIELD_ANIM_STATE_MOVIE_STARTING;
        break;
    case 2:
        actor = operand;
        if (actor == FIELD_SCRIPT_OWNER)
        {
            actor = g_field_script->status.owner_id;
        }
        wait = field_read_actor_binding_state(actor) < FIELD_BINDING_READY;
        break;
    case 3:
        actor = operand;
        if (actor == FIELD_SCRIPT_OWNER)
        {
            actor = g_field_script->status.owner_id;
        }
        wait = field_read_actor_binding_state(actor) != FIELD_BINDING_LOADING;
        break;
    case 4:
        actor = operand;
        if (actor == FIELD_SCRIPT_OWNER)
        {
            actor = g_field_script->status.owner_id;
        }
        wait = field_read_actor_binding_state(actor) != FIELD_BINDING_IDLE;
        break;
    case 5:
        wait = field_text_get_status(operand & FIELD_TALK_PLANE_MASK) != FIELD_TEXT_STATUS_BUSY;
        break;
    case 6:
        wait = field_text_get_status(operand & FIELD_TALK_PLANE_MASK) != FIELD_TEXT_STATUS_DONE;
        break;
    }

    if (wait)
    {
        g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
    }
    else
    {
        FIELD_SCRIPT_ACTIVE_RECORD()->pc += 3;
    }
}

/**
 * @brief Opcode 0x13: end the step loop while the actor named by a script variable is still loading.
 * @note Operands: selector byte, then the halfword variable reference. Selector 0 is the only one
 *       handled; any other selector tests an uninitialized value.
 */
void field_script_op_13(void)
{
    u8* pc;
    s32 selector;
    s32 actor;
    s32 wait;

    pc = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    selector = pc[1];
    actor = field_get_script_var(0, pc[2] | (pc[3] << 8));
    switch (selector)
    {
    case 0:
        wait = field_read_actor_binding_state((actor != FIELD_SCRIPT_OWNER) ? actor : g_field_script->status.owner_id) < FIELD_BINDING_READY;
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
 * @brief Opcode 0x14: end the step loop while a field condition selected by the byte operand holds.
 * @note Selectors 3 and 4 also copy D_8011F428 into FIELD_VAR_RESULT once the wait ends.
 *       An unknown selector tests an uninitialized value.
 */
void field_script_op_14(void)
{
    u32 selector;
    s32 wait;

    selector = FIELD_SCRIPT_ACTIVE_RECORD()->pc[1];
    switch (selector)
    {
    case 1:
        wait = field_is_battle_active();
        break;
    case 2:
        wait = D_801227F0 != 2;
        break;
    case 3:
        wait = D_8011F428 == 1;
        break;
    case 4:
        wait = D_8011F428 == 0;
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
        field_set_script_var(0, FIELD_VAR_RESULT, D_8011F428);
    }
}

/**
 * @brief Opcode 0x15: wait for a text window to close (or finish) and store the chosen answer.
 * @note The byte operand is the window slot (FIELD_TALK_AUTO for the current talk plane); with
 *       FIELD_TALK_PLANE_NO_FACING set the opcode only waits, otherwise it waits for the window
 *       to close and then stores its choice in FIELD_VAR_RESULT.
 */
void field_script_op_15(void)
{
    s32 window;
    s32 slot;
    s32 status;

    window = FIELD_SCRIPT_ACTIVE_RECORD()->pc[1];
    window = (window != FIELD_TALK_AUTO) ? window : g_field_runtime->talk_window.bits.plane;
    slot = window & FIELD_TALK_PLANE_MASK;
    status = field_text_get_status(slot);
    if (!(window & FIELD_TALK_PLANE_NO_FACING))
    {
        if (status == FIELD_TEXT_STATUS_CLOSED)
        {
            field_set_script_var(0, FIELD_VAR_RESULT, field_text_get_choice(slot));
            FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
            return;
        }
    }
    else if (status == 3)
    {
        FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
        return;
    }
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x16: wait until an actor is idle, skipping absent party members.
 * @note The byte operand is the actor (0xFF for the owner). A party slot that is empty, or
 *       AI-controlled while g_field_interaction_active is clear, is skipped at once.
 */
void field_script_op_16(void)
{
    FieldScriptRecord* rec;
    u8* pc;
    u8 operand;
    u8 actor;
    u32 party_index;

    rec = FIELD_SCRIPT_ACTIVE_RECORD();
    pc = rec->pc;
    operand = pc[1];
    if (operand == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = operand;
    }
    party_index = actor;
    if (party_index < FIELD_PARTY_SIZE)
    {
        if (g_field_game_state->characters[party_index].name[0] == 0)
        {
            rec->pc = pc + 2;
            return;
        }
        if (g_field_game_state->characters[party_index].info.bits.pad_controlled)
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
 * @brief Opcode 0x17 (unused): report it as a bad opcode and end the step loop.
 */
void field_script_op_17(void)
{
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, g_field_script->status.owner_id, 0x17);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x18: queue a CD seek for the scene resource named by the halfword operand.
 */
void field_script_op_18(void)
{
    s32 scene;

    scene = FIELD_SCRIPT_ACTIVE_RECORD()->pc[1] + (FIELD_SCRIPT_ACTIVE_RECORD()->pc[2] << 8);
    field_seek_scene_resource(scene & 0x7FFF);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 3;
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
 * @brief Opcode 0x1A: set an actor's control mode.
 * @note Operands (bytes): actor (0xFF for the owner) and mode.
 */
void field_script_op_1a(void)
{
    u8* pc;
    u8 operand;
    u8 actor;

    pc = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    operand = pc[1];
    if (operand == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = operand;
    }
    field_set_actor_control_mode(actor, pc[2]);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 3;
}

/**
 * @brief Opcode 0x1B: store an operand into a script variable.
 * @note Operands: descriptor, halfword variable reference, then the value.
 */
void field_script_op_1b(void)
{
    FieldScriptVariableRef var_ref;
    s32 value;
    u8 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(operands + 2, &var_ref.value);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(descriptor, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &value);
    field_write_script_var(g_field_script->status.owner_id, var_ref, value);
}

/**
 * @brief Opcode 0x1C: combine two operands through field_script_calc and store the result in a script variable.
 * @note The descriptor holds the two operand types (bits 0-1 and 2-3) and the FieldScriptCalcOp
 *       (bits 4-7); the destination variable reference follows the operands.
 */
void field_script_op_1c(void)
{
    s32 left;
    s32 right;
    FieldScriptVariableRef var_ref;
    u32 descriptor;
    u8* operands;
    s32 result;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = operands + 2;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(descriptor, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &left);
    descriptor >>= 2;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(descriptor, FIELD_SCRIPT_ACTIVE_RECORD()->pc, &right);
    result = field_script_calc(descriptor >> 2, left, right);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc, &var_ref.value);
    field_write_script_var(g_field_script->status.owner_id, var_ref, result);
}

/**
 * @brief Opcode 0x1D: write an actor position into three consecutive script variables.
 * @note Operands: the actor (0xFF for the owner) and the variable reference of X. Y and Z are the
 *       next two variable words of the same kind (the location wraps in 12 bits); Y is negated.
 */
void field_script_op_1d(void)
{
    Vec3i position;
    FieldScriptPositionOperands operands;
    s32 unused[4]; /* Never used, but the original frame has room for it. */
    u16 location;
    u32 kind;
    u32 relative;
    u32 y_ref;
    u32 z_ref;
    s32 y_location;
    s32 z_location;
    u8* pc;

    pc = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(pc[1], pc + 2, &operands.actor);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_u16(FIELD_SCRIPT_ACTIVE_RECORD()->pc, &operands.x_ref.value);

    kind = location = operands.x_ref.value;
    relative = kind >> 15;
    kind &= 0x7000;
    y_ref = relative << 15;
    /* The same bit again; a plain copy of y_ref swaps the registers of y_ref and z_ref. */
    z_ref = (relative & 1) << 15;

    location &= 0xFFF;
    y_location = location + 0x20;
    y_location &= 0xFFF;
    y_ref |= kind;
    y_ref |= y_location;
    z_location = location + 0x40;
    z_location &= 0xFFF;
    z_ref |= kind;
    z_ref |= z_location;

    field_get_actor_position(operands.actor, &position);
    field_write_script_var(g_field_script->status.owner_id, operands.x_ref, position.x);
    {
        FieldScriptVariableRef reference;

        reference.value = y_ref;
        field_write_script_var(g_field_script->status.owner_id, reference, -position.y);
    }
    {
        FieldScriptVariableRef reference;

        reference.value = z_ref;
        field_write_script_var(g_field_script->status.owner_id, reference, position.z);
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
 * @brief Opcode 0x20 (unused): report it as a bad opcode and end the step loop.
 */
void field_script_op_20(void)
{
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, g_field_script->status.owner_id, 0x20);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x21 (unused): report it as a bad opcode and end the step loop.
 */
void field_script_op_21(void)
{
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, g_field_script->status.owner_id, 0x21);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x22 (unused): report a bad opcode and end the step loop.
 * @note Reports itself as opcode 0x23.
 */
void field_script_op_22(void)
{
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, g_field_script->status.owner_id, 0x23);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x23 (unused): report it as a bad opcode and end the step loop.
 */
void field_script_op_23(void)
{
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, g_field_script->status.owner_id, 0x23);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x24 (unused): report it as a bad opcode and step past it.
 */
void field_script_op_24(void)
{
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, g_field_script->status.owner_id, 0x24);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 1;
}

/**
 * @brief Opcode 0x25 (unused): report it as a bad opcode and end the step loop.
 */
void field_script_op_25(void)
{
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, g_field_script->status.owner_id, 0x25);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x26 (unused): report it as a bad opcode and end the step loop.
 */
void field_script_op_26(void)
{
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, g_field_script->status.owner_id, 0x26);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x27: remove the guest (operand 0) or the companion (any other value) from the party.
 * @note Empties the party slot, clears its variant variable and releases its actor resources;
 *       the guest also clears its flag variable at 0xF87 + 8 * info byte 1.
 */
void field_script_op_27(void)
{
    s32 member;

    member = FIELD_SCRIPT_ACTIVE_RECORD()->pc[1];
    if (member == 0)
    {
        g_field_game_state->characters[FIELD_PARTY_GUEST].name[0] = 0;
        g_field_game_state->characters[FIELD_PARTY_GUEST].info.word |= FIELD_CHARACTER_TYPE_MASK;
        field_set_script_var(0, (g_field_game_state->characters[FIELD_PARTY_GUEST].info.bytes[1] << 3) + 0xF87, 0);
        field_set_script_var(0, FIELD_VARIABLE_GUEST_VARIANT, FIELD_NO_VARIANT);
    }
    else
    {
        g_field_game_state->characters[FIELD_PARTY_COMPANION].name[0] = 0;
        g_field_game_state->characters[FIELD_PARTY_COMPANION].info.word |= FIELD_CHARACTER_TYPE_MASK;
        field_set_script_var(0, FIELD_VARIABLE_COMPANION_VARIANT, FIELD_NO_VARIANT);
    }
    field_release_actor_resource_slot(member);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
}

/**
 * @brief Opcode 0x28: open the ring menu, then queue argument 0xF to event record 0.
 * @note Operands: menu id, excluded entry mask and cancel entry (0xFF for none); the low two
 *       descriptor bits are the menu position mode.
 */
void field_script_op_28(void)
{
    s32 cancel_index;
    s32 excluded_mask;
    s32 menu_id;
    u32 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_0(descriptor), operands + 2, &menu_id);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &excluded_mask);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &cancel_index);
    if (cancel_index == 0xFF)
    {
        cancel_index = -1;
    }
    field_open_ring_menu(descriptor & 3, menu_id, excluded_mask, cancel_index);
    field_queue_actor_event(FIELD_EVENT_ACTOR_ID_BASE, 0, 0xF);
}

/**
 * @brief Opcode 0x29: wait for the ring menu result and store it in FIELD_VAR_RESULT.
 * @note Queues argument 0x10 to event record 0 once a result is chosen; while the menu is open
 *       it queues the entry under the cursor and ends the step loop.
 */
void field_script_op_29(void)
{
    s32 result;

    result = field_get_ring_result();
    if (result < 0)
    {
        field_queue_actor_event(FIELD_EVENT_ACTOR_ID_BASE, 0, field_get_ring_cursor_entry() & 0xFF);
        g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
        return;
    }
    field_set_script_var(0, FIELD_VAR_RESULT, result);
    field_queue_actor_event(FIELD_EVENT_ACTOR_ID_BASE, 0, 0x10);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 1;
}

/**
 * @brief Opcode 0x2A: read four operands and ignore them.
 * @note The fourth operand is normalized (0xFF to -1) like a cancel index, but nothing uses it.
 */
void field_script_op_2a(void)
{
    s32 operand3;
    s32 operand2;
    s32 operand1;
    s32 operand0;
    u32 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_0(descriptor), operands + 2, &operand0);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &operand1);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &operand2);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_3(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &operand3);
    if (operand3 == 0xFF)
    {
        operand3 = -1;
    }
}

/**
 * @brief Opcode 0x2B: format a number into a text window's inline text buffer.
 * @note Operands: window slot, value and digit count; a zero digit count is replaced by the
 *       value's decimal length.
 */
void field_script_op_2b(void)
{
    s32 digits;
    s32 value;
    s32 window;
    u32 descriptor;
    u8* operands;
    u32 remaining;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_0(descriptor), operands + 2, &window);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &value);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &digits);
    if (digits == 0)
    {
        digits = 1;
        for (remaining = (u32)value / 10; remaining != 0; remaining /= 10)
        {
            digits++;
        }
    }
    field_text_format_number((u16)window, value, digits);
}

/**
 * @brief Opcode 0x2C: close a text window, or every window when the operand has bit 7 set.
 */
void field_script_op_2c(void)
{
    s32 window;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(operands[1] & 3, operands + 2, &window);
    if (window & 0x80)
    {
        for (window = 0; (u32)window < FIELD_TEXT_WINDOW_SLOTS; window++)
        {
            field_text_close_window((u16)window);
        }
    }
    else
    {
        field_text_close_window((u16)window);
    }
}

/**
 * @brief Opcode 0x2D: turn an actor to face another.
 * @note Operands: the turning actor and the actor to face (each 0xFF for the owner).
 */
void field_script_op_2d(void)
{
    s32 actor;
    s32 target;
    u8 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_0(descriptor), operands + 2, &actor);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &target);
    field_face_actor(actor, target);
}

/**
 * @brief Opcode 0x2E: no operation.
 * @note Does not advance the program counter.
 */
void field_script_op_2e(void)
{
}

/**
 * @brief Opcode 0x2F: move an actor into a trigger group.
 * @note Operands: the actor (0xFF for the owner) and the group.
 */
void field_script_op_2f(void)
{
    s32 actor;
    s32 group;
    u8 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_0(descriptor), operands + 2, &actor);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &group);
    field_set_actor_group(actor, group);
}

/**
 * @brief Opcode 0x30 (unused): report it as a bad opcode and end the step loop.
 */
void field_script_op_30(void)
{
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, g_field_script->status.owner_id, 0x30);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x31: place an actor.
 * @note Operands: the actor (0xFF for the owner), then x, y and z.
 */
void field_script_op_31(void)
{
    s32 actor;
    s32 x;
    s32 y;
    s32 z;
    u32 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_0(descriptor), operands + 2, &actor);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &x);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &y);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_3(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &z);
    field_set_actor_position(actor, x, y, z);
}

/**
 * @brief Opcode 0x32: open a talk window for a speaker.
 * @note Operands: speaker (0xFF for the owner), plane, portrait and window layout, each resolved
 *       by field_resolve_talk_window (FIELD_TALK_AUTO picks the value from the scene).
 */
void field_script_op_32(void)
{
    s32 layout;
    s32 portrait;
    s32 plane;
    s32 speaker;
    u32 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_0(descriptor), operands + 2, &speaker);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &plane);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &portrait);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_3(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &layout);
    speaker = (speaker != FIELD_SCRIPT_OWNER) ? speaker : g_field_script->status.owner_id;
    field_resolve_talk_window(&speaker, &plane, &portrait, &layout);
    field_open_text_window(plane, layout, speaker, portrait);
}

/**
 * @brief Opcode 0x33: show a scene string in a talk window.
 * @note Operands: window slot (FIELD_TALK_AUTO for the current talk plane), string index and
 *       the options of field_set_text_window_string.
 */
void field_script_op_33(void)
{
    s32 options;
    s32 string_index;
    s32 window;
    u32 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_0(descriptor), operands + 2, &window);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &string_index);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &options);
    window = (window != FIELD_TALK_AUTO) ? window : g_field_runtime->talk_window.bits.plane;
    field_set_text_window_string(window, string_index, options);
}

/**
 * @brief Opcode 0x34: play a sound effect of sound group 0.
 * @note Operand types are taken from the low descriptor bits upward: sound id, pan, then a
 *       value field_play_set_sfx_group0 ignores.
 */
void field_script_op_34(void)
{
    s32 unused;
    s32 pan;
    s32 sound_id;
    u32 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_3(descriptor), operands + 2, &sound_id);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &pan);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &unused);
    field_play_set_sfx_group0(sound_id, pan, unused);
}

/**
 * @brief Opcode 0x35: start an animation actor on an actor, aimed at a target.
 * @note Operands (types from the low descriptor bits upward): actor and target (each 0xFF for
 *       the owner), then the animation resource.
 */
void field_script_op_35(void)
{
    s32 resource_index;
    s32 target;
    s32 actor;
    u32 descriptor;
    u8* operands;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_3(descriptor), operands + 2, &actor);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand_or_owner(OPERAND_TYPE_2(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &target);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &resource_index);
    field_spawn_targeted_animation_actor(actor, resource_index, 1, &target);
}

/**
 * @brief Opcode 0x36 (unused): report it as a bad opcode and end the step loop.
 */
void field_script_op_36(void)
{
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, g_field_script->status.owner_id, 0x36);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x37: build a shop list from resource FIELD_RESOURCE_SHOP_LISTS and open the shop.
 * @note Operands: shop list index, then the price scale in sixteenths.
 */
void field_script_op_37(void)
{
    u8* operands;
    u8 descriptor;
    s32 list_index;
    s32 price_scale;
    u32* offsets;
    FieldShopList* list;
    ShopEntry entries[FIELD_SHOP_LIST_CAPACITY];
    FieldItemResource* items;
    s32 index;
    s32 item;
    u32 scaled;
    u32 generated;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_0(descriptor), operands + 2, &list_index);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &price_scale);

    offsets = (u32*)func_800C1E40(FIELD_RESOURCE_SHOP_LISTS);
    list = (FieldShopList*)((u8*)offsets + offsets[list_index + 1]);

    items = (FieldItemResource*)func_800C1E40(FIELD_RESOURCE_ITEM_TEMPLATES);
    for (index = 0; index < list->count; index++)
    {
        item = list->entries[index].bits.item;
        generated = list->entries[index].bits.generated;
        entries[index].count = 0;
        /* A generated entry gets SHOP_ENTRY_RECORD_FLAG (bit 15). */
        entries[index].id = item + (generated << 15);
        scaled = list->entries[index].bits.price * price_scale;
        entries[index].price = scaled >> 4;
    }

    field_open_shop_mode_1(list->count, entries, items->records, 2);
}

/**
 * @brief Opcode 0x38: no operation; step past the opcode.
 */
void field_script_op_38(void)
{
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 1;
}

/**
 * @brief Opcode 0x39 (unused): report it as a bad opcode and end the step loop.
 */
void field_script_op_39(void)
{
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, g_field_script->status.owner_id, 0x39);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3A (unused): report it as a bad opcode and end the step loop.
 */
void field_script_op_3a(void)
{
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, g_field_script->status.owner_id, 0x3A);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3B (unused): report it as a bad opcode and end the step loop.
 */
void field_script_op_3b(void)
{
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, g_field_script->status.owner_id, 0x3B);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3C (unused): report it as a bad opcode and end the step loop.
 */
void field_script_op_3c(void)
{
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, g_field_script->status.owner_id, 0x3C);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3D (unused): report it as a bad opcode and end the step loop.
 */
void field_script_op_3d(void)
{
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, g_field_script->status.owner_id, 0x3D);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3E (unused): report it as a bad opcode and end the step loop.
 */
void field_script_op_3e(void)
{
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, g_field_script->status.owner_id, 0x3E);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3F (unused): report it as a bad opcode and end the step loop.
 */
void field_script_op_3f(void)
{
    record_game_diagnostic(DIAG_ERROR, DIAG_SCRIPT_BAD_OPCODE, g_field_script->status.owner_id, 0x3F);
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x0F: run a debug or reset subcommand selected by the byte operand.
 * @note 0 learns every technique and ability, 1 sets up battle group 1, 3 reloads the trigger
 *       table, 4 clears the game variables, 5 applies pending companion level ups, 6 clears the
 *       companions' new flags, 7 fills the item stock.
 */
void field_script_op_0f(void)
{
    s32 all_techniques;
    s32 i;

    switch (FIELD_SCRIPT_ACTIVE_RECORD()->pc[1])
    {
    case 0:
        all_techniques = 0xFFFFFF;
        for (i = FIELD_WEAPON_CATEGORY_COUNT - 1; i >= 0; i--)
        {
            g_field_game_state->technique_bits[i] = all_techniques;
        }
        g_field_game_state->ability_bits[0] = 0x500;
        g_field_game_state->ability_bits[1] = 0xFFFF8000;
        g_field_game_state->ability_bits[2] = 0x803F;
        FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
        return;
    case 1:
        field_battle_setup(1);
        break;
    case 3:
        g_field_runtime->trigger_table = (FieldTriggerTable*)func_800C1E40(FIELD_RESOURCE_TRIGGERS);
        FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
        return;
    case 4:
        func_800C1EC8(NULL, g_field_game_state->words, sizeof(g_field_game_state->words));
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
        for (i = 0; i < FIELD_REGION_COUNT; i++)
        {
            g_field_game_state->regions[i].unk42 = 0;
            g_field_game_state->regions[i].status.word &= ~FIELD_COMPANION_NEW;
        }
        break;
    case 7:
        for (i = 0; i < FIELD_DEBUG_ITEM_KIND_COUNT; i++)
        {
            g_field_game_state->item_counts[i] = FIELD_ITEM_QUANTITY_MAX;
        }
        break;
    }
    FIELD_SCRIPT_ACTIVE_RECORD()->pc += 2;
}

/**
 * @brief Opcode 0x40: store a value into one of the script owner's variables.
 * @param variable Variable reference passed to field_set_script_var.
 * @param value Value to store.
 */
void field_script_op_40(s32 variable, s32 value)
{
    field_set_script_var(g_field_script->status.owner_id, variable, value);
}

/**
 * @brief Opcode 0x41: stop a tile animation, at once or at a keyframe.
 * @param index Tile animation index.
 * @param keyframe Keyframe to stop at, or FIELD_KEYFRAME_NONE.
 */
void field_script_op_41(s32 index, s32 keyframe)
{
    field_control_animation(FIELD_LIST_TILE_ANIMS, index, keyframe, FIELD_ANIM_OP_STOP);
}

/**
 * @brief Opcode 0x42: find or load a scene resource entry.
 * @param resource_slot_id Passed through to field_find_or_load_resource_entry.
 * @param resource_base Passed through to field_find_or_load_resource_entry.
 */
void field_script_op_42(s32 resource_slot_id, s32 resource_base)
{
    field_find_or_load_resource_entry(resource_slot_id, resource_base);
}

/**
 * @brief Opcode 0x43: set the condition flag when an actor is near its stored position.
 * @param mode Test to run; only mode 0 is handled.
 * @param actor Actor id, or 0xFF for the script owner.
 */
void field_script_op_43(s32 mode, s32 actor)
{
    s32 target;
    FieldScriptRecordState* rec;
    s32 result;

    if (mode == 0)
    {
        if (actor == FIELD_SCRIPT_OWNER)
        {
            target = g_field_script->status.owner_id;
        }
        else
        {
            target = actor;
        }
        result = field_is_actor_near_stored_position(target, FIELD_STORED_POSITION_RANGE, FIELD_STORED_POSITION_RANGE);
        rec = FIELD_SCRIPT_ACTIVE_RECORD_STATE();
        rec->flags = (rec->flags & ~FIELD_SCRIPT_COND) | (result & FIELD_SCRIPT_COND);
    }
}

/**
 * @brief Opcode 0x44: run one of the miscellaneous field commands selected by @p command.
 * @param command Command number, 0x00 to 0x4E.
 * @param operand Command argument; commands that take an actor treat 0xFF as the script owner.
 */
void field_script_op_44(u32 command, s32 operand)
{
    s32 index; /* The resolved actor id; command 0x4E reuses it as its land index. */
    FieldScriptRecordState* rec;

    if (operand == FIELD_SCRIPT_OWNER)
    {
        index = g_field_script->status.owner_id;
    }
    else
    {
        index = operand;
    }
    switch (command)
    {
    case 0x0:
        field_request_return_to_title(operand);
        return;
    case 0x1:
        field_start_timed_panel(operand);
        return;
    case 0x2:
        field_set_game_flag(operand);
        return;
    case 0x3:
        field_stop_actor(operand);
        return;
    case 0x4:
        field_play_effect_animation(operand, 1);
        return;
    case 0x5:
        field_play_effect_animation(operand, 0);
        return;
    case 0x6:
        g_field_game_state->lands[operand].flags |= FIELD_LAND_FLAG_04;
        return;
    case 0x7:
        field_control_animation(FIELD_LIST_TILE_ANIMS, operand, 0, FIELD_ANIM_OP_RESTART);
        return;
    case 0x8:
        field_control_animation(FIELD_LIST_TILE_ANIMS, operand, FIELD_KEYFRAME_NONE, FIELD_ANIM_OP_FINISH_LOOP);
        return;
    case 0x9:
        field_stop_second_song();
        return;
    case 0xA:
        field_run_zukan(operand);
        return;
    case 0xB:
        field_run_menu_op(operand);
        return;
    case 0xC:
        /* The screen sequence starts at runtime script variable word @p operand. */
        field_open_gosub_screen_sequence((s32*)g_field_runtime + operand);
        return;
    case 0xD:
        field_create_item_from_gosub(operand);
        return;
    case 0xE:
        g_field_runtime->scene_entry |= 0x8000;
        field_play_second_song();
        return;
    case 0xF:
        field_make_land_available(operand);
        return;
    case 0x10:
        field_leave_party(operand);
        return;
    case 0x11:
        field_stop_actor_script(index, 0);
        return;
    case 0x12:
        /* Called as returning int: the original does not mask the u8 result. */
        field_set_script_var(0, FIELD_VAR_RESULT, ((s32 (*)(s32))field_get_item_count)(operand));
        return;
    case 0x13:
        field_receive_item(operand);
        return;
    case 0x14:
        field_consume_item(operand);
        return;
    case 0x15:
        field_set_script_var(0, FIELD_VAR_RESULT, field_get_land_state(operand));
        return;
    case 0x16:
        field_toggle_actor_hidden(index);
        return;
    case 0x17:
        field_start_actor_turn(index);
        return;
    case 0x18:
        field_set_script_var(0, FIELD_VAR_RESULT, field_add_stored_companion(operand));
        return;
    case 0x19:
        field_set_script_var(0, FIELD_VAR_RESULT, field_release_stored_companion());
        return;
    case 0x1A:
        field_set_actor_record_script_only(index, 2);
        return;
    case 0x1B:
        field_start_interaction(FIELD_EVENT_ACTOR_ID_BASE, operand & 0xFFFF);
        return;
    case 0x1C:
        field_control_animation(FIELD_LIST_PALETTE_ANIMS, operand, 0, FIELD_ANIM_OP_RESTART);
        return;
    case 0x1D:
        field_control_animation(FIELD_LIST_PALETTE_ANIMS, operand, FIELD_KEYFRAME_NONE, FIELD_ANIM_OP_STOP);
        return;
    case 0x1E:
        field_control_animation(FIELD_LIST_TINT_ANIMS, operand, 0, FIELD_ANIM_OP_RESTART);
        return;
    case 0x1F:
        field_control_animation(FIELD_LIST_TINT_ANIMS, operand, FIELD_KEYFRAME_NONE, FIELD_ANIM_OP_STOP);
        return;
    case 0x20:
        field_set_node_enabled(operand, 1);
        return;
    case 0x21:
        field_set_node_enabled(operand, 0);
        return;
    case 0x22:
        field_set_script_var(0, FIELD_VAR_RESULT, field_add_template_item(operand));
        return;
    case 0x23:
        field_golem_select_logic_cell(index);
        return;
    case 0x24:
        field_golem_publish_logic_cell(operand);
        return;
    case 0x25:
        field_raise_companion_intensity(operand);
        return;
    case 0x26:
        field_discard_item(operand);
        return;
    case 0x27:
        field_stop_non_script_actors();
        return;
    case 0x28:
        func_800C1E08();
        return;
    case 0x29:
        field_unlock_encyclopedia_entry(operand);
        return;
    case 0x2A:
        field_open_carda(operand);
        return;
    case 0x2B:
        field_set_script_var(0, operand, 1);
        return;
    case 0x2C:
        field_clear_actor_record_script_only(index);
        return;
    case 0x2D:
        field_set_script_var(0, FIELD_VAR_RESULT, field_get_stored_companion_status(operand));
        return;
    case 0x2E:
        field_rename_stored_companion(operand);
        return;
    case 0x32:
        D_80122980 = operand;
        return;
    case 0x33:
        g_field_pair_indicators_disabled = operand;
        return;
    case 0x34:
        field_set_script_var(0, FIELD_VAR_RESULT, field_find_nearest_faced_item(operand));
        return;
    case 0x35:
        field_control_sequence(operand, 0);
        return;
    case 0x36:
        field_control_sequence(operand, 1);
        return;
    case 0x37:
        field_show_timed_text(operand);
        return;
    case 0x38:
        field_battle_defeat_record(index);
        return;
    case 0x39:
        field_set_script_var(0, FIELD_VAR_RESULT, field_receive_money(operand));
        return;
    case 0x3A:
        field_set_script_var(0, FIELD_VAR_RESULT, field_spend_money(operand));
        return;
    case 0x3B:
        if (operand >= FIELD_MUSIC_TRACK_COUNT)
        {
            record_game_diagnostic(DIAG_ERROR, 1, 0x2C, operand);
            return;
        }
        g_music_track_index = operand;
        return;
    case 0x3C:
        /* Advance the day of the week. */
        g_field_game_state->control.bits.weekday++;
        g_field_game_state->control.bits.weekday %= 6U;
        return;
    case 0x3D:
        g_field_hide_actor_panels = operand;
        return;
    case 0x3E:
        field_open_shop_mode_0(operand);
        return;
    case 0x3F:
        field_cache_inventory_values();
        return;
    case 0x40:
        field_stop_actor(index);
        return;
    case 0x41:
        field_apply_pending_region_effects();
        return;
    case 0x42:
        field_begin_scene_fade_out();
        return;
    case 0x2F:
    case 0x43:
        field_set_pixel_lookup(operand);
        return;
    case 0x44:
        g_field_duel_mode = operand;
        return;
    case 0x45:
        akao_stop_song(0);
        return;
    case 0x46:
        akao_cmd_f1();
        return;
    case 0x47:
        field_reset_party_to_level(operand);
        return;
    case 0x48:
        field_apply_region_level_ups(operand);
        return;
    case 0x49:
        g_script_pair_value_49 = operand;
        return;
    case 0x4A:
        rec = FIELD_SCRIPT_ACTIVE_RECORD_STATE();
        rec->wait.bits.frames = operand;
        g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
        return;
    case 0x4B:
        g_gosub_result_count = 1;
        g_gosub_result_values = operand;
        return;
    case 0x4C:
        g_field_game_state->money = operand;
        return;
    case 0x4D:
        g_music_track_index = 0;
        field_open_carda(0);
        field_script_op_85(0xFFFE, 0, 0, 0);
        return;
    case 0x4E:
        /* New game: clear the variables, the flags and the lands, then place the starting land. */
        g_field_game_state->options.word |= SAVED_OPTION_FLAG_2 | SAVED_OPTION_FLAG_3;
        func_800C1EC8(NULL, g_field_game_state->words, sizeof(g_field_game_state->words));
        func_800C1EC8(NULL, (s32*)&g_field_game_state->control,
                      sizeof(g_field_game_state->control) + sizeof(g_field_game_state->flag_bits) + sizeof(g_field_game_state->lands));
        g_field_game_state->control.fields.placed_land_count = 1;
        g_field_game_state->flag_bits[0] |= 0x10000000;
        for (index = 0; index < FIELD_LAND_COUNT; index++)
        {
            g_field_game_state->lands[index].x = FIELD_LAND_CELL_NONE;
            g_field_game_state->lands[index].z = FIELD_LAND_CELL_NONE;
        }
        FIELD_LAND_WORDS[0].word |= FIELD_LAND_PLACED;
        g_field_game_state->lands[0].count = 1;
        FIELD_LAND_WORDS[0].word |= FIELD_LAND_FLAG_04;
        FIELD_LAND_WORDS[1].word |= FIELD_LAND_FLAG_04;
        FIELD_LAND_WORDS[2].word |= FIELD_LAND_FLAG_04;
        FIELD_LAND_WORDS[3].word |= FIELD_LAND_FLAG_04;
        FIELD_LAND_WORDS[4].word |= FIELD_LAND_FLAG_04;
        FIELD_LAND_WORDS[5].word |= FIELD_LAND_FLAG_04;
        FIELD_LAND_WORDS[6].word |= FIELD_LAND_FLAG_04;
        FIELD_LAND_WORDS[32].word |= FIELD_LAND_FLAG_04;
        break;
    }
}

/**
 * @brief Opcode 0x45: set an actor's animation.
 * @param actor_id Actor id, or 0xFF for the script owner.
 */
void field_script_op_45(s32 actor_id)
{
    s32 actor;

    actor = actor_id;
    if (actor == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    field_set_actor_animation(actor);
}

/**
 * @brief Opcode 0x46: set the condition flag when two actors overlap in depth.
 * @param first_id First actor id, or 0xFF for the script owner.
 * @param second_id Second actor id, or 0xFF for the script owner.
 */
void field_script_op_46(s32 first_id, s32 second_id)
{
    FieldScriptRecordState* rec;
    s32 result;
    s32 first;
    s32 second;

    if (first_id == FIELD_SCRIPT_OWNER)
    {
        first = g_field_script->status.owner_id;
    }
    else
    {
        first = first_id;
    }
    if (second_id == FIELD_SCRIPT_OWNER)
    {
        second = g_field_script->status.owner_id;
    }
    else
    {
        second = second_id;
    }
    result = field_test_actor_depth_overlap(first, second);
    rec = FIELD_SCRIPT_ACTIVE_RECORD_STATE();
    rec->flags = (rec->flags & ~FIELD_SCRIPT_COND) | (result & FIELD_SCRIPT_COND);
}

/**
 * @brief Opcode 0x47: let a guest join the party and bind it to an actor.
 * @note During an interaction the party also switches to party mode FIELD_PARTY_MODE_ALL.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param guest_id Guest to join; also stored in FIELD_VARIABLE_GUEST_VARIANT.
 */
void field_script_op_47(s32 actor_id, s32 guest_id)
{
    s32 actor;

    field_join_guest(guest_id);
    if (actor_id == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = actor_id;
    }
    field_activate_actor_resource_slot(actor, guest_id, 0);
    if (g_field_interaction_active != 0)
    {
        field_set_actor_control_mode(1, 2);
        g_field_runtime->state.bits.party_mode = FIELD_PARTY_MODE_ALL;
    }
    field_set_script_var(0, FIELD_VARIABLE_GUEST_VARIANT, guest_id);
}

/**
 * @brief Opcode 0x48: let the companion join (or rejoin) the party and bind it to an actor.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param source 1 rejoins the stored companion (field_rejoin_companion), anything else joins a new one.
 */
void field_script_op_48(s32 actor_id, s32 source)
{
    s32 actor;
    s32 variant;

    if (actor_id == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = actor_id;
    }
    if (source == 1)
    {
        variant = field_rejoin_companion();
    }
    else
    {
        variant = field_join_companion();
    }
    if (variant != FIELD_NO_VARIANT)
    {
        field_activate_actor_resource_slot(actor, variant, 1);
    }
    field_set_script_var(0, FIELD_VARIABLE_COMPANION_VARIANT, variant);
}

/**
 * @brief Opcode 0x49: let the golem join the party as the companion and bind it to an actor.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param logic_type Logic type passed to field_join_golem.
 */
void field_script_op_49(s32 actor_id, s32 logic_type)
{
    s32 variant;

    variant = field_join_golem(logic_type);
    if (actor_id == FIELD_SCRIPT_OWNER)
    {
        actor_id = g_field_script->status.owner_id;
        field_activate_actor_resource_slot(actor_id, variant, 1);
    }
    else
    {
        field_activate_actor_resource_slot(actor_id, variant, 1);
    }
    field_set_script_var(0, FIELD_VARIABLE_COMPANION_VARIANT, variant);
}

/**
 * @brief Opcode 0x4A: start an actor's event script and end the step loop.
 * @note During a battle a party actor runs the script from its private script page instead.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param entry Private script entry of a party actor.
 */
void field_script_op_4a(s32 actor_id, s32 entry)
{
    s32 actor;
    u32 party_index;

    actor = actor_id;
    if (actor_id == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    party_index = actor & 0xFF;
    if ((g_field_runtime->state.flags & FIELD_STATE_PARTY_PAGE_SCRIPTS) && party_index < FIELD_PARTY_SIZE)
    {
        field_start_actor_private_script(party_index, field_get_party_private_script(party_index, entry));
    }
    else
    {
        field_start_actor_script(actor & 0xFF);
    }
    g_field_script->status.word = g_field_script->status.word & ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x4B: show an object or one of its parts.
 * @param obj_index Object index.
 * @param part_index Part index, or 0xFF for the whole object.
 */
void field_script_op_4b(s32 obj_index, s32 part_index)
{
    if (part_index == 0xFF)
    {
        part_index = FIELD_WHOLE_OBJECT;
    }
    field_set_object_visible(obj_index, part_index, 1);
}

/**
 * @brief Opcode 0x4C: hide an object or one of its parts.
 * @param obj_index Object index.
 * @param part_index Part index, or 0xFF for the whole object.
 */
void field_script_op_4c(s32 obj_index, s32 part_index)
{
    if (part_index == 0xFF)
    {
        part_index = FIELD_WHOLE_OBJECT;
    }
    field_set_object_visible(obj_index, part_index, 0);
}

/**
 * @brief Opcode 0x4D: move an actor to a new height, keeping its x and z.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param height New height; the position's y is -height.
 */
void field_script_op_4d(s32 actor_id, s32 height)
{
    s32 actor;
    Vec3i position;

    if (actor_id == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = actor_id;
    }
    field_get_actor_position(actor, &position);
    position.y = -height;
    field_set_actor_position(actor, position.x >> 8, position.y, position.z >> 8);
}

/**
 * @brief Opcode 0x4E: start an animation on an actor.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param resource With bit 15 set, the low 15 bits name a bound animation (field_load_bound_animation);
 *        otherwise a shared animation actor resource (field_spawn_shared_animation_actor).
 */
void field_script_op_4e(s32 actor_id, s32 resource)
{
    if (actor_id == FIELD_SCRIPT_OWNER)
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
 * @brief Opcode 0x4F: build a shop list from resource FIELD_RESOURCE_SHOP_LISTS and open the shop.
 * @note Generated entries name a record of FIELD_RESOURCE_ITEM_TEMPLATES whose value is computed.
 * @param list_index Shop list index.
 * @param price_scale Price scale in eighths.
 */
void field_script_op_4f(s32 list_index, s32 price_scale)
{
    u32* offsets;
    FieldShopList* list;
    FieldItemResource* items;
    ShopEntry entries[FIELD_SHOP_LIST_CAPACITY];
    s32 index;
    s32 price;
    u32 scaled;
    FieldItemRecord* item;

    offsets = (u32*)func_800C1E40(FIELD_RESOURCE_SHOP_LISTS);
    list = (FieldShopList*)((u8*)offsets + offsets[list_index + 1]);
    items = (FieldItemResource*)func_800C1E40(FIELD_RESOURCE_ITEM_TEMPLATES);
    for (index = 0; index < list->count; index++)
    {
        if (list->entries[index].bits.generated)
        {
            entries[index].id = list->entries[index].bits.item | SHOP_ENTRY_RECORD_FLAG;
            item = &items->records[list->entries[index].bits.item];
            entries[index].count = 0;
            price = field_get_item_value(item);
            item->value = price;
            scaled = (u32)(price * price_scale) >> 3;
            entries[index].price = scaled;
        }
        else
        {
            entries[index].id = list->entries[index].bits.item;
            entries[index].count = 0;
            scaled = (s32)(list->entries[index].bits.price * price_scale) >> 3;
            entries[index].price = scaled;
        }
    }

    field_open_shop_mode_1(list->count, entries, items->records, 2);
}

/**
 * @brief Opcode 0x50: retire an actor.
 * @note Clears the actor record's active and spawned flags first.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param resource_index Forwarded to field_retire_actor.
 */
void field_script_op_50(s32 actor_id, s32 resource_index)
{
    s32 actor;
    FieldActorRecord* record;

    if (actor_id == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = actor_id;
    }
    record = field_find_actor_record_or_default(actor);
    record->flags.bits.active = 0;
    record->flags.bits.spawned = 0;
    field_retire_actor(actor, resource_index);
}

/**
 * @brief Opcode 0x51: store in FIELD_VAR_RESULT whether one actor faces another.
 * @param first_key First actor id, or 0xFF for the script owner.
 * @param second_key Second actor id, or 0xFF for the script owner.
 */
void field_script_op_51(s32 first_key, s32 second_key)
{
    first_key = (first_key == FIELD_SCRIPT_OWNER) ? g_field_script->status.owner_id : first_key;
    second_key = (second_key == FIELD_SCRIPT_OWNER) ? g_field_script->status.owner_id : second_key;

    field_set_script_var(0, FIELD_VAR_RESULT, field_actor_faces_actor(first_key, second_key));
}

/**
 * @brief Opcode 0x52: copy one of the owner's script variables to another.
 * @param dest_variable Destination variable reference.
 * @param source_variable Source variable reference (low 16 bits).
 */
void field_script_op_52(s32 dest_variable, s32 source_variable)
{
    field_set_script_var(g_field_script->status.owner_id, dest_variable, field_get_script_var(g_field_script->status.owner_id, source_variable & 0xFFFF));
}

/**
 * @brief Opcode 0x53: fade the second song to a volume.
 * @param volume Target volume, clamped to FIELD_SONG_VOLUME_MAX.
 * @param frames Fade length in frames; 0 means 1.
 */
void field_script_op_53(s32 volume, s32 frames)
{
    frames = (frames != 0) ? frames : 1;
    if (volume > FIELD_SONG_VOLUME_MAX)
    {
        volume = FIELD_SONG_VOLUME_MAX;
    }
    field_fade_song(1, frames, volume);
}

/**
 * @brief Opcode 0x54: play the next section of the field song.
 */
void field_script_op_54(void)
{
    field_play_song_section();
}

/**
 * @brief Opcode 0x55: fade the field song to a volume.
 * @param volume Target volume, clamped to FIELD_SONG_VOLUME_MAX.
 * @param frames Fade length in frames; 0 means 1.
 */
void field_script_op_55(s32 volume, s32 frames)
{
    frames = (frames != 0) ? frames : 1;
    if (volume > FIELD_SONG_VOLUME_MAX)
    {
        volume = FIELD_SONG_VOLUME_MAX;
    }
    field_fade_song(0, frames, volume);
}

/**
 * @brief Opcode 0x56: set the two-bit value in bits 4-5 of a land's flags.
 * @param land Land index.
 * @param value New value; only the low 2 bits are stored.
 */
void field_script_op_56(s32 land, s32 value)
{
    g_field_game_state->lands[land].flags = (g_field_game_state->lands[land].flags & ~FIELD_LAND_VALUE_MASK) | ((value & 3) << FIELD_LAND_VALUE_SHIFT);
}

/**
 * @brief Opcode 0x57: reset an actor at its current position in the current trigger group.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param resource_entry_index Forwarded to field_reset_actor_at.
 */
void field_script_op_57(s32 actor_id, s32 resource_entry_index)
{
    Vec3i position;
    s32 actor;
    s32 x;
    s32 y;
    s32 z;

    if (actor_id == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = actor_id;
    }
    field_get_actor_position(actor, &position);
    x = position.x >> 8;
    y = position.y >> 8;
    z = position.z >> 8;
    field_reset_actor_at(actor, resource_entry_index, g_field_runtime->state.bytes.trigger_group, x, y, z);
}

/**
 * @brief Opcode 0x58: play a sound effect.
 * @param sound_id Sound id.
 * @param pan Pan.
 */
void field_script_op_58(s32 sound_id, s32 pan)
{
    field_play_sound(sound_id, pan);
}

/**
 * @brief Opcode 0x59: store the party's money in a script variable.
 * @param variable_id Script variable reference.
 */
void field_script_op_59(s32 variable_id)
{
    field_set_script_var(0, variable_id, g_field_game_state->money);
}

/**
 * @brief Opcode 0x5A: store an actor's binding state in one of that actor's script variables.
 * @param variable_id Script variable reference.
 * @param actor_id Actor id, or 0xFF for the script owner.
 */
void field_script_op_5a(s32 variable_id, s32 actor_id)
{
    s32 actor;

    if (actor_id == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = actor_id;
    }
    field_set_script_var(actor, variable_id, field_get_actor_binding_state(actor));
}

/**
 * @brief Opcode 0x5B: no operation.
 */
void field_script_op_5b(void)
{
}

/**
 * @brief Opcode 0x5C: no operation.
 */
void field_script_op_5c(void)
{
}

/**
 * @brief Opcode 0x5D: no operation.
 */
void field_script_op_5d(void)
{
}

/**
 * @brief Opcode 0x5E: no operation.
 */
void field_script_op_5e(void)
{
}

/**
 * @brief Opcode 0x5F: no operation.
 */
void field_script_op_5f(void)
{
}

/**
 * @brief Opcode 0x80: record a diagnostic supplied by a field script.
 * @param status Diagnostic status.
 * @param code Diagnostic code.
 * @param value0 First diagnostic value.
 * @param value1 Second diagnostic value.
 */
void field_script_op_80(s32 status, s32 code, s32 value0, s32 value1)
{
    record_game_diagnostic(status, code, value0, value1);
}

/**
 * @brief Opcode 0x81: set the colour and render mode of every actor.
 * @param mode Render mode; bit 7 sets the colour flag.
 * @param red Red component.
 * @param green Green component.
 * @param blue Blue component.
 */
void field_script_op_81(s32 mode, s32 red, s32 green, s32 blue)
{
    s32 color_flag;

    if (mode & 0x80)
    {
        color_flag = 1;
        mode &= 0x7F;
    }
    else
    {
        color_flag = 0;
    }
    field_set_all_actor_render_state(red, green, blue, color_flag, mode);
}

/**
 * @brief Opcode 0x82: reload an actor from a resource.
 * @param key Actor id; bit 7 sets the resource flag of field_reload_actor.
 * @param resource_entry_index Forwarded to field_reload_actor.
 * @param resource_slot_id Forwarded to field_reload_actor.
 * @param resource_base Forwarded to field_reload_actor.
 */
void field_script_op_82(s32 key, s32 resource_entry_index, s32 resource_slot_id, s32 resource_base)
{
    s32 actor;
    s32 resource_flag;

    if (key & 0x80)
    {
        resource_flag = 1;
        actor = key & 0x7F;
    }
    else
    {
        resource_flag = 0;
        actor = key;
    }
    field_clear_actor_record_script_only(actor);
    field_reload_actor(actor, resource_entry_index, resource_slot_id, (u8*)resource_base, 0, -1, -1, -1, 0, resource_flag);
}

/**
 * @brief Opcode 0x83: no operation.
 */
void field_script_op_83(void)
{
}

/**
 * @brief Opcode 0x84: queue a change to an actor's battle entry.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param animation Animation, or 0xFF for none.
 * @param builtin_animation Built-in animation, or 0xFF for none.
 * @param sound Sound, or 0xFF for none.
 */
void field_script_op_84(s32 actor_id, s32 animation, s32 builtin_animation, s32 sound)
{
    s32 actor;

    if (actor_id == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = actor_id;
    }
    field_queue_battle_entry_change(actor, (animation == 0xFF) ? -1 : animation, (builtin_animation == 0xFF) ? -1 : builtin_animation,
                                    (sound == 0xFF) ? -1 : sound);
}


/**
 * @brief Opcode 0x85: request a scene change and end the script.
 * @param scene_id Scene to load (FieldRuntimeContext transition.fields.scene_id).
 * @param object_id Field object of the new scene (transition.fields.object_id).
 * @param audio Bytes 0-2: sound bank, music and secondary music; 0xFE and 0xFF select the keep
 *        and clear values of field_set_scene_parameters, and the current sound bank or music
 *        (g_layout_option, g_layout_flag) is kept as well.
 * @param spawn_id Spawn point in the new scene.
 */
void field_script_op_85(s32 scene_id, s32 object_id, s32 audio, s32 spawn_id)
{
    FieldRuntimeContext* runtime;
    s32 music;
    s32 secondary_music;
    s32 sound_bank;

    runtime = g_field_runtime;
    sound_bank = audio & 0xFF;
    runtime->transition.bits.requested = 1;
    runtime->transition.fields.object_id = object_id;
    music = (audio >> 8) & 0xFF;
    secondary_music = (audio >> 16) & 0xFF;
    runtime->transition.fields.scene_id = scene_id;
    switch (sound_bank)
    {
    case 0xFE:
        g_field_runtime->scene_argument1 = -2;
        break;
    case 0xFF:
        g_layout_option = -1;
        g_field_runtime->scene_argument1 = -1;
        break;
    default:
        if (sound_bank == g_layout_option)
        {
            g_field_runtime->scene_argument1 = -2;
        }
        else
        {
            g_field_runtime->scene_argument1 = sound_bank;
        }
        break;
    }
    switch (music)
    {
    case 0xFE:
        g_field_runtime->scene_entry = -2;
        break;
    case 0xFF:
        g_field_runtime->scene_entry = -1;
        break;
    default:
        if (music == g_layout_flag)
        {
            g_field_runtime->scene_entry = -1;
        }
        else
        {
            g_field_runtime->scene_entry = music;
        }
        break;
    }
    switch (secondary_music)
    {
    case 0xFE:
        g_field_runtime->scene_argument2 = -2;
        break;
    case 0xFF:
        g_field_runtime->scene_argument2 = -1;
        break;
    default:
        g_field_runtime->scene_argument2 = secondary_music;
        break;
    }
    g_field_runtime->transition.bits.spawn_id = spawn_id;
    g_field_script->active_record = 0;
    g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = NULL;
}

/**
 * @brief Opcode 0x86: set a text macro to an entry of a text resource.
 * @param slot Text macro slot.
 * @param resource_id Resource read with func_800C1E40.
 * @param entry_index Entry of the resource's offset table.
 * @param character_limit Character budget of the macro.
 */
void field_script_op_86(s32 slot, s32 resource_id, s32 entry_index, s32 character_limit)
{
    FieldTextResource* resource;
    u16 offset;

    resource = (FieldTextResource*)func_800C1E40(resource_id);
    if (resource != NULL)
    {
        offset = resource->texts.offsets[entry_index];
        /* Int limit on purpose: the original passes character_limit without narrowing it to u8. */
        ((void (*)(s32, u8*, s32))field_set_text_macro)(slot, &resource->texts.bytes[offset], character_limit);
    }
}

/**
 * @brief Opcode 0x87: run (selector 0) or queue (selector 1) an actor event.
 * @param selector 0 runs the event with field_run_actor_event, 1 queues it with field_queue_actor_event.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param event_id Event (low byte).
 * @param argument Event argument or mode (low byte).
 */
void field_script_op_87(s32 selector, s32 actor_id, s32 event_id, s32 argument)
{
    if (actor_id == FIELD_SCRIPT_OWNER)
    {
        actor_id = g_field_script->status.owner_id;
    }
    switch (selector)
    {
    case 0:
        field_run_actor_event(actor_id, event_id & 0xFF, argument & 0xFF);
        break;
    case 1:
        field_queue_actor_event(actor_id, event_id & 0xFF, argument & 0xFF);
        break;
    }
}

/**
 * @brief Opcode 0x88: reset the layout selectors and start the game over (GOVER) transition.
 * @param image_resource_index Forwarded to field_begin_gover_transition.
 * @param music_resource_index Music resource, or 0xFF for none (-1).
 * @param audio_clip_index Audio clip, or 0xFF for none (-1).
 */
void field_script_op_88(s32 image_resource_index, s32 music_resource_index, s32 audio_clip_index)
{
    music_resource_index = (music_resource_index == 0xFF) ? -1 : music_resource_index;
    audio_clip_index = (audio_clip_index == 0xFF) ? -1 : audio_clip_index;
    g_layout_option = -1;
    g_layout_sub_mode = -1;
    field_begin_gover_transition(image_resource_index, music_resource_index, audio_clip_index);
}

/**
 * @brief Opcode 0x89: issue AKAO command 0xA9.
 * @param unused0 Unused.
 * @param unused1 Unused.
 * @param value First AKAO operand; 0 means 1.
 * @param value2 Second AKAO operand.
 */
void field_script_op_89(s32 unused0, s32 unused1, s32 value, s32 value2)
{
    if (value == 0)
    {
        value = 1;
    }
    akao_cmd_a9(value, value2);
}

/**
 * @brief Opcode 0x8A: start an animation actor on an actor, aimed at a target.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param resource_index Animation resource.
 * @param target_id Target actor id, or 0xFF for the script owner.
 * @param unused Unused.
 */
void field_script_op_8a(s32 actor_id, s32 resource_index, s32 target_id, s32 unused)
{
    s32 target;
    s32 actor;

    actor = actor_id;
    if (target_id == FIELD_SCRIPT_OWNER)
    {
        target = g_field_script->status.owner_id;
    }
    else
    {
        target = target_id;
    }
    if (actor == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    field_spawn_targeted_animation_actor(actor, resource_index, 1, &target);
}

/**
 * @brief Opcode 0x8B: set the fade colour and the fade timer.
 * @param red Red component (10 bits).
 * @param green Green component (10 bits).
 * @param blue Blue component (10 bits).
 * @param timer Fade timer.
 */
void field_script_op_8b(s32 red, s32 green, s32 blue, s32 timer)
{
    FieldRuntimeContext* runtime;

    runtime = g_field_runtime;
    runtime->fade_timer = timer;
    runtime->fade_color.bits.red = red;
    runtime->fade_color.bits.green = green;
    runtime->fade_color.bits.blue = blue;
}

/**
 * @brief Opcode 0x8C: revive an actor.
 * @param actor_id Actor id, or 0xFF for the script owner.
 * @param animation Animation, or 0xFF for none.
 * @param effect Effect, or 0xFF for none.
 * @param sound Sound, or 0xFF for none.
 */
void field_script_op_8c(s32 actor_id, s32 animation, s32 effect, s32 sound)
{
    s32 actor;

    if (actor_id == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = actor_id;
    }
    field_revive_actor(actor, (animation == 0xFF) ? -1 : animation, (effect == 0xFF) ? -1 : effect, (sound == 0xFF) ? -1 : sound);
}

/**
 * @brief Opcode 0x8D: no operation.
 */
void field_script_op_8d(void)
{
}

/**
 * @brief Opcode 0x8E: no operation.
 */
void field_script_op_8e(void)
{
}

/**
 * @brief Opcode 0x8F: no operation.
 */
void field_script_op_8f(void)
{
}
