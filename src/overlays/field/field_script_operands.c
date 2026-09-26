#include "common.h"
#include "field_script.h"

typedef struct
{
    u8 pad0[0x28];
    u32 unk28;
} StructC1B60;

void field_script_op_00(void);
u8* field_script_read_u16(u8* data, u16* value);
s32 func_800BD3B0(s32 owner_id, FieldScriptVariableRef var_ref);
extern s32 g_field_game_state;
extern s32 g_field_runtime;
StructC1B60* field_find_actor_record_or_default(s32 arg0);
extern u8 D_800F0E08[8];
s32 func_800BD318(s32 owner_id, FieldScriptVariableRef var_ref, s32* element_index, s32* bit_shift);
s32 func_800BD650(s32 width, u8* base, s32 index, s32 shift, s32 bit_count);
void func_800BD55C(s32 width, s32 base, s32 index, s32 shift, s32 bit_count, s32 value);
void func_800BD434(s32 owner_id, FieldScriptVariableRef var_ref, s32 value);
void func_800BD4A8(s32 owner_id, FieldScriptVariableRef var_ref, s32 value);

/**
 * @brief Apply a signed 16-bit relative jump to the active record's program counter.
 *
 * Reads a little-endian 16-bit delta from the active record's program counter
 * at offset @p delta_offset. A non-zero delta advances the pc by it
 * (sign-extended via the 0x8000 bit); a zero delta hands off to
 * field_script_op_00 to step the cursor.
 *
 * @param delta_offset Byte offset from the program counter holding the delta.
 */
void field_script_branch(s32 delta_offset)
{
    FieldScriptRecord* rec;
    s32 pc;
    u8* ptr;
    s32 val;
    s32 lo;

    rec = FIELD_SCRIPT_ACTIVE_RECORD();
    pc = (s32)rec->pc;
    ptr = (u8*)(pc + delta_offset);
    val = ptr[0] + (ptr[1] << 8);
    lo = val & 0xFFFF;
    if (lo != 0)
    {
        if (val & 0x8000)
        {
            s32 t = pc + 0xFFFF0000;
            rec->pc = (u8*)(t + lo);
            return;
        }
        rec->pc = (u8*)(pc + lo);
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
u8* field_script_read_operand_or_owner(u32 type, u8* data, s32* value)
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
u8* field_script_read_operand(u32 operand_type, u8* data, s32* value)
{
    s32 type = operand_type;

    type &= 3;
    switch (type)
    {
    case 0:
    {
        FieldScriptVariableRef var_ref;

        data = field_script_read_u16(data, &var_ref.value);
        *value = func_800BD3B0(g_field_script->status.owner_id, var_ref);
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
 * @param data Byte stream to read a little-endian 16-bit value from.
 * @param value Destination for the unpacked 16-bit value.
 * @return @p data advanced past the two bytes read.
 */
u8* field_script_read_u16(u8* data, u16* value)
{
    *value = data[0] + (data[1] << 8);
    return data + 2;
}

/**
 * @brief Decode a packed field-script variable reference.
 * @param owner_id Owner or record identifier used when resolving adjusted references.
 * @param var_ref Packed variable reference to decode.
 * @param element_index Receives the element index (reference bits 5-11).
 * @param bit_shift Receives the bit shift (reference bits 0-4).
 * @return Base value selected by the reference kind.
 */
s32 func_800BD318(s32 owner_id, FieldScriptVariableRef var_ref, s32* element_index, s32* bit_shift)
{
    u32 value;
    s32 result;

    value = var_ref.value & 0xFFF;
    *element_index = value >> 5;
    *bit_shift = value & 0x1F;
    if (((var_ref.value >> 12) & 7) < 3)
    {
        result = g_field_game_state + 0xE4;
    }
    else
    {
        result = g_field_runtime;
        if (var_ref.value & 0x8000)
        {
            *element_index += (field_find_actor_record_or_default(owner_id)->unk28 >> 9) & 0x7F;
        }
    }
    return result;
}

/**
 * @brief Resolve and read a packed field-script variable reference.
 * @param owner_id Owner or record identifier used while resolving the reference.
 * @param var_ref Packed variable reference to read.
 * @return Value read from the resolved variable.
 */
s32 func_800BD3B0(s32 owner_id, FieldScriptVariableRef var_ref)
{
    s32 element_index;
    s32 bit_shift;
    s32 base;

    base = func_800BD318(owner_id, var_ref, &element_index, &bit_shift);
    return func_800BD650(2, (u8*)base, element_index, bit_shift, D_800F0E08[(var_ref.value >> 12) & 7]);
}

/**
 * @brief Resolve a 16-bit field-script variable reference and discard its value.
 * @param owner_id Owner or record identifier used while resolving the reference.
 * @param raw_ref Packed 16-bit variable reference.
 */
void func_800BD414(s32 owner_id, s32 raw_ref)
{
    FieldScriptVariableRef var_ref;

    var_ref.value = raw_ref;
    func_800BD3B0(owner_id, var_ref);
}

/**
 * @brief Resolve a packed field-script variable reference and write a value.
 * @param owner_id Owner or record identifier used while resolving the reference.
 * @param var_ref Packed variable reference to write.
 * @param value Value to write.
 */
void func_800BD434(s32 owner_id, FieldScriptVariableRef var_ref, s32 value)
{
    s32 element_index;
    s32 bit_shift;
    s32 base;

    base = func_800BD318(owner_id, var_ref, &element_index, &bit_shift);
    func_800BD55C(2, base, element_index, bit_shift, D_800F0E08[(var_ref.value >> 12) & 7], value);
}

/**
 * @brief Resolve a packed variable reference and write using decremented width metadata.
 * @param owner_id Owner or record identifier used while resolving the reference.
 * @param var_ref Packed variable reference to write.
 * @param value Value to write.
 */
void func_800BD4A8(s32 owner_id, FieldScriptVariableRef var_ref, s32 value)
{
    s32 element_index;
    s32 bit_shift;
    s32 base;

    base = func_800BD318(owner_id, var_ref, &element_index, &bit_shift);
    func_800BD55C(2, base, element_index, bit_shift, D_800F0E08[(var_ref.value >> 12) & 7] - 1, value);
}

/**
 * @brief Dispatch a raw variable reference to the appropriate write helper.
 * @param owner_id Owner or record identifier used while resolving the reference.
 * @param raw_ref Raw variable reference value.
 * @param value Value to write.
 */
void func_800BD520(s32 owner_id, u32 raw_ref, s32 value)
{
    FieldScriptVariableRef var_ref;

    var_ref.value = raw_ref;
    if (raw_ref <= 0xFFFFU)
    {
        func_800BD434(owner_id, var_ref, value);
        return;
    }
    func_800BD4A8(owner_id, var_ref, value);
}

/**
 * @brief Write a masked bitfield into an 8-, 16-, or 32-bit element.
 * @param width Element width selector: 0 = byte, 1 = halfword, 2 = word.
 * @param base Base address of the element array; 0 and -1 mean no target.
 * @param index Element index.
 * @param shift Left-shift amount of the field.
 * @param bit_count Field width in bits; values >= 32 replace the whole element.
 * @param value Value to store in the field.
 */
void func_800BD55C(s32 width, s32 base, s32 index, s32 shift, s32 bit_count, s32 value)
{
    s32 clear_mask;
    s32 value_mask;
    u8* p8;
    u16* p16;
    s32* p32;

    if (base == 0)
    {
        return;
    }
    if (base == -1)
    {
        return;
    }

    if (bit_count < 0x20)
    {
        clear_mask = ~(((1 << bit_count) - 1) << shift);
    }
    else
    {
        clear_mask = 0;
    }

    if (bit_count < 0x20)
    {
        value_mask = ((1 << bit_count) - 1) << shift;
    }
    else
    {
        value_mask = -1;
    }

    switch (width)
    {
    case 0:
        p8 = (u8*)(base + index);
        *p8 = (*p8 & clear_mask) | (value_mask & (value << shift));
        break;
    case 1:
        p16 = (u16*)((index * 2) + base);
        *p16 = (*p16 & clear_mask) | (value_mask & (value << shift));
        break;
    case 2:
        p32 = (s32*)((index * 4) + base);
        *p32 = (*p32 & clear_mask) | (value_mask & (value << shift));
        break;
    }
}

/**
 * @brief Extract a masked bitfield from an 8-, 16-, or 32-bit element.
 *
 * @param width Element width selector: 0 = byte, 1 = halfword, 2 = word.
 * @param base Base address of the element array.
 * @param index Element index.
 * @param shift Right-shift amount.
 * @param bit_count Number of low bits to retain; values >= 32 retain all bits.
 * @return The selected element shifted right by @p shift and masked to @p bit_count bits.
 * @note Other width values fall off the end without a return value, as in the original.
 */
s32 func_800BD650(s32 width, u8* base, s32 index, s32 shift, s32 bit_count)
{
    s32 mask;
    s32 value;

    if (bit_count < 0x20)
    {
        mask = (1 << bit_count) - 1;
    }
    else
    {
        mask = -1;
    }

    switch (width)
    {
    case 0:
        value = base[index] >> shift;
        return value & mask;
    case 1:
        value = (*(u16*)(base + index * 2)) >> shift;
        return value & mask;
    case 2:
        return (*(u32*)(base + index * 4) >> shift) & mask;
    }
}
