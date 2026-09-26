#include "common.h"
#include "field_script.h"
#include "field_records.h"

/*
 * Script variables.
 *
 * A variable reference is a 16-bit value: bits 0-4 are the bit position of
 * the variable inside its word, bits 5-11 the word index and bits 12-14 the
 * variable kind, which selects the bit width (g_field_script_var_widths).
 * Kinds below FIELD_SCRIPT_VAR_LOCAL_KIND are game-state variables in
 * FieldGameState::words and are saved with the game; the others are words at
 * the start of the field runtime context. Bit 15 makes a local reference
 * relative to the owner actor's local variable base.
 */

/** @brief Bits 0-11 of a variable reference: word index and bit position. */
#define FIELD_SCRIPT_VAR_LOCATION_MASK 0xFFF

/** @brief Bits 0-4 of a variable reference: bit position inside the word. */
#define FIELD_SCRIPT_VAR_SHIFT_MASK 0x1F

/** @brief Variable kind, bits 12-14 of a variable reference. */
#define FIELD_SCRIPT_VAR_KIND(ref) (((ref) >> 12) & 7)

/** @brief First variable kind that lives in the field runtime context. */
#define FIELD_SCRIPT_VAR_LOCAL_KIND 3

/** @brief Variable reference bit: offset the word index by the owner's local variable base. */
#define FIELD_SCRIPT_VAR_OWNER_RELATIVE 0x8000

/** @brief Element widths of field_read_bits and field_write_bits. */
enum
{
    FIELD_BITS_BYTE = 0,
    FIELD_BITS_HALFWORD = 1,
    FIELD_BITS_WORD = 2
};

/** @brief Bit width of each variable kind. */
extern u8 g_field_script_var_widths[8];

extern FieldGameState* g_field_game_state;
extern FieldRuntimeContext* g_field_runtime;

void field_script_op_00(void);
FieldActorRecord* field_find_actor_record_or_default(s32 id);

s32 field_read_script_var(s32 owner_id, FieldScriptVariableRef var_ref);
void field_write_script_var(s32 owner_id, FieldScriptVariableRef var_ref, s32 value);
static u32* field_resolve_script_var(s32 owner_id, FieldScriptVariableRef var_ref, s32* word_index, s32* bit_shift);
static void func_800BD4A8(s32 owner_id, FieldScriptVariableRef var_ref, s32 value);

/**
 * @brief Apply a relative jump to the active record's program counter.
 *
 * Reads a little-endian 16-bit two's-complement delta @p delta_offset bytes
 * after the program counter and adds it. A zero delta instead steps past the
 * opcode through field_script_op_00.
 *
 * @param delta_offset Byte offset from the program counter of the delta.
 */
void field_script_branch(s32 delta_offset)
{
    FieldScriptRecord* rec;
    u8* pc;
    u16 delta;

    rec = FIELD_SCRIPT_ACTIVE_RECORD();
    pc = rec->pc;
    delta = pc[delta_offset] + (pc[delta_offset + 1] << 8);
    if (delta != 0)
    {
        if (delta & 0x8000)
        {
            rec->pc = pc - (0x10000 - delta);
            return;
        }
        rec->pc = pc + delta;
        return;
    }
    field_script_op_00();
}

/**
 * @brief Read one operand, mapping the value 0xFF to the script owner's id.
 * @param type Operand type from the descriptor byte.
 * @param data Operand stream position.
 * @param value Receives the decoded value.
 * @return The advanced operand stream position.
 */
u8* field_script_read_operand_or_owner(s32 type, u8* data, s32* value)
{
    u8* result;

    result = field_script_read_operand(type, data, value);
    if (*value == 0xFF)
    {
        *value = g_field_script->status.owner_id;
    }
    return result;
}

/**
 * @brief Decode one field script operand and advance the read pointer.
 * @param type Two-bit operand kind: 0 script variable, 1 byte, 2 halfword, 3 word.
 * @param data Operand byte stream to read from.
 * @param value Destination for the decoded 32-bit value.
 * @return @p data advanced past the bytes consumed for this operand.
 */
u8* field_script_read_operand(s32 type, u8* data, s32* value)
{
    switch (type & 3)
    {
    case 0:
    {
        FieldScriptVariableRef var_ref;

        data = field_script_read_u16(data, &var_ref.value);
        *value = field_read_script_var(g_field_script->status.owner_id, var_ref);
        return data;
    }
    case 1:
        *value = *data;
        return data + 1;
    case 2:
        *value = data[0] + (data[1] << 8);
        return data + 2;
    case 3:
        *value = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
        return data + 4;
    }
}

/**
 * @brief Read a little-endian halfword from the script stream.
 * @param data Byte stream to read from.
 * @param value Receives the halfword.
 * @return @p data advanced past the two bytes read.
 */
u8* field_script_read_u16(u8* data, u16* value)
{
    *value = data[0] + (data[1] << 8);
    return data + 2;
}

/**
 * @brief Locate the word that holds a script variable.
 * @param owner_id Actor whose local variable base applies to owner-relative references.
 * @param var_ref Variable reference.
 * @param word_index Receives the word index from the variable base.
 * @param bit_shift Receives the bit position inside the word.
 * @return The variable base: FieldGameState::words or the field runtime context.
 */
static u32* field_resolve_script_var(s32 owner_id, FieldScriptVariableRef var_ref, s32* word_index, s32* bit_shift)
{
    u32 location;
    u32* base;

    location = var_ref.value & FIELD_SCRIPT_VAR_LOCATION_MASK;
    *word_index = location >> 5;
    *bit_shift = location & FIELD_SCRIPT_VAR_SHIFT_MASK;
    if (FIELD_SCRIPT_VAR_KIND(var_ref.value) < FIELD_SCRIPT_VAR_LOCAL_KIND)
    {
        base = (u32*)g_field_game_state->words;
    }
    else
    {
        base = (u32*)g_field_runtime;
        if (var_ref.value & FIELD_SCRIPT_VAR_OWNER_RELATIVE)
        {
            *word_index += field_find_actor_record_or_default(owner_id)->script.status.bits.local_base;
        }
    }
    return base;
}

/**
 * @brief Read a script variable.
 * @param owner_id Actor whose local variable base applies to owner-relative references.
 * @param var_ref Variable reference.
 * @return The variable's value.
 */
s32 field_read_script_var(s32 owner_id, FieldScriptVariableRef var_ref)
{
    s32 word_index;
    s32 bit_shift;
    u32* base;

    base = field_resolve_script_var(owner_id, var_ref, &word_index, &bit_shift);
    return field_read_bits(FIELD_BITS_WORD, base, word_index, bit_shift, g_field_script_var_widths[FIELD_SCRIPT_VAR_KIND(var_ref.value)]);
}

/**
 * @brief Read a script variable given as a plain integer.
 * @param owner_id Actor whose local variable base applies to owner-relative references.
 * @param variable Variable reference in the low 16 bits.
 * @return The variable's value.
 */
s32 field_get_script_var(s32 owner_id, s32 variable)
{
    FieldScriptVariableRef var_ref;

    var_ref.value = variable;
    return field_read_script_var(owner_id, var_ref);
}

/**
 * @brief Write a script variable.
 * @param owner_id Actor whose local variable base applies to owner-relative references.
 * @param var_ref Variable reference.
 * @param value Value to store; bits above the variable's width are dropped.
 */
void field_write_script_var(s32 owner_id, FieldScriptVariableRef var_ref, s32 value)
{
    s32 word_index;
    s32 bit_shift;
    u32* base;

    base = field_resolve_script_var(owner_id, var_ref, &word_index, &bit_shift);
    field_write_bits(FIELD_BITS_WORD, base, word_index, bit_shift, g_field_script_var_widths[FIELD_SCRIPT_VAR_KIND(var_ref.value)], value);
}

/**
 * @brief Write a script variable using one bit less than its kind's width.
 * @param owner_id Actor whose local variable base applies to owner-relative references.
 * @param var_ref Variable reference.
 * @param value Value to store.
 */
static void func_800BD4A8(s32 owner_id, FieldScriptVariableRef var_ref, s32 value)
{
    s32 word_index;
    s32 bit_shift;
    u32* base;

    base = field_resolve_script_var(owner_id, var_ref, &word_index, &bit_shift);
    field_write_bits(FIELD_BITS_WORD, base, word_index, bit_shift, g_field_script_var_widths[FIELD_SCRIPT_VAR_KIND(var_ref.value)] - 1, value);
}

/**
 * @brief Write a script variable given as a plain integer.
 * @param owner_id Actor whose local variable base applies to owner-relative references.
 * @param variable Variable reference in the low 16 bits; a value above 0xFFFF writes one bit less (func_800BD4A8).
 * @param value Value to store.
 */
void field_set_script_var(s32 owner_id, u32 variable, s32 value)
{
    FieldScriptVariableRef var_ref;

    var_ref.value = variable;
    if (variable <= 0xFFFF)
    {
        field_write_script_var(owner_id, var_ref, value);
        return;
    }
    func_800BD4A8(owner_id, var_ref, value);
}

/**
 * @brief Store a value into a bit field of an 8-, 16- or 32-bit array element.
 * @param width Element width, a FIELD_BITS_* value.
 * @param base Element array; NULL and -1 mean there is nothing to write.
 * @param index Element index.
 * @param shift Bit position of the field.
 * @param bit_count Field width in bits; 32 or more replaces the whole element.
 * @param value Value to store in the field.
 */
void field_write_bits(s32 width, void* base, s32 index, s32 shift, s32 bit_count, s32 value)
{
    s32 clear_mask;
    s32 value_mask;

    if (base == NULL)
    {
        return;
    }
    if (base == (void*)-1)
    {
        return;
    }

    if (bit_count < 32)
    {
        clear_mask = ~(((1 << bit_count) - 1) << shift);
    }
    else
    {
        clear_mask = 0;
    }

    if (bit_count < 32)
    {
        value_mask = ((1 << bit_count) - 1) << shift;
    }
    else
    {
        value_mask = -1;
    }

    switch (width)
    {
    case FIELD_BITS_BYTE:
        ((u8*)base)[index] = (((u8*)base)[index] & clear_mask) | (value_mask & (value << shift));
        break;
    case FIELD_BITS_HALFWORD:
        ((u16*)base)[index] = (((u16*)base)[index] & clear_mask) | (value_mask & (value << shift));
        break;
    case FIELD_BITS_WORD:
        ((u32*)base)[index] = (((u32*)base)[index] & clear_mask) | (value_mask & (value << shift));
        break;
    }
}

/**
 * @brief Extract a bit field from an 8-, 16- or 32-bit array element.
 * @param width Element width, a FIELD_BITS_* value.
 * @param base Element array.
 * @param index Element index.
 * @param shift Bit position of the field.
 * @param bit_count Field width in bits; 32 or more returns the whole shifted element.
 * @return The field's value; undefined for an unknown @p width.
 */
s32 field_read_bits(s32 width, void* base, s32 index, s32 shift, s32 bit_count)
{
    s32 mask;
    s32 value;

    if (bit_count < 32)
    {
        mask = (1 << bit_count) - 1;
    }
    else
    {
        mask = -1;
    }

    switch (width)
    {
    case FIELD_BITS_BYTE:
        value = ((u8*)base)[index] >> shift;
        return value & mask;
    case FIELD_BITS_HALFWORD:
        value = ((u16*)base)[index] >> shift;
        return value & mask;
    case FIELD_BITS_WORD:
        return (((u32*)base)[index] >> shift) & mask;
    }
}
