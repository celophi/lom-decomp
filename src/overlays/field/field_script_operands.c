#include "common.h"
#include "field_script.h"

/** @brief Packed 16-bit reference used by the field-script variable helpers. */
typedef struct
{
    u16 value;
} FieldScriptVariableRef;

void field_script_op_00(void);

/**
 * @brief Apply a signed 16-bit relative jump to the active record's program counter.
 *
 * Reads a little-endian 16-bit delta from the active record's program counter
 * at offset @p delta_offset. A non-zero delta advances the pc by it
 * (sign-extended via the 0x8000 bit); a zero delta hands off to
 * field_script_op_00 to step the cursor.
 *
 * @param delta_offset Byte offset from the program counter holding the delta.
 * @see decomp.me (100%) TODO
 */
void field_script_branch(s32 delta_offset)
{
    FieldScriptRecord *rec;
    s32 pc;
    u8 *ptr;
    s32 val;
    s32 lo;

    rec = FIELD_SCRIPT_ACTIVE_RECORD();
    pc = (s32)rec->pc;
    ptr = (u8 *)(pc + delta_offset);
    val = ptr[0] + (ptr[1] << 8);
    lo = val & 0xFFFF;
    if (lo != 0)
    {
        if (val & 0x8000)
        {
            s32 t = pc + 0xFFFF0000;
            rec->pc = (u8 *)(t + lo);
            return;
        }
        rec->pc = (u8 *)(pc + lo);
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
u8 *field_script_read_operand_or_owner(u32 type, u8 *data, s32 *value)
{
    u8 *result;

    result = field_script_read_operand(type, data, value);
    if (*value == 0xFF)
    {
        *value = g_field_script->status.owner_id;
    }
    return result;
}

u8 *field_script_read_u16(u8 *data, u16 *value);
s32 func_800BD3B0(s32 arg0, FieldScriptVariableRef arg1);

/**
 * @brief Decode one field script operand and advance the read pointer.
 * @param type Two-bit operand kind: 0 script variable, 1 byte, 2 halfword, 3 word.
 * @param data Operand byte stream to read from.
 * @param value Destination for the decoded 32-bit value.
 * @return @p data advanced past the bytes consumed for this operand.
 */
u8 *field_script_read_operand(u32 operand_type, u8 *data, s32 *value)
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
 * @param arg0 Byte stream to read a little-endian 16-bit value from.
 * @param arg1 Destination for the unpacked 16-bit value.
 * @return arg0 advanced past the two bytes read.
 * @see decomp.me (100%) N/A -- trivial 7-instruction leaf function, no scratch needed.
 */
u8* field_script_read_u16(u8* data, u16* value)
{
    *value = data[0] + (data[1] << 8);
    return data + 2;
}

extern s32 D_80122B74;
extern s32 D_80122B78;

typedef struct
{
    u8 pad0[0x28];
    u32 unk28;
} StructC1B60;

StructC1B60 *func_800C1B60(s32 arg0);

/**
 * @brief Decode a packed field-script variable reference.
 * @param arg0 Owner or record identifier used when resolving adjusted references.
 * @param arg1 Packed variable reference to decode.
 * @param arg2 Receives the reference's primary bit-field value.
 * @param arg3 Receives the reference's low five-bit value.
 * @return Base value selected by the reference kind.
 */
s32 func_800BD318(s32 arg0, FieldScriptVariableRef arg1, s32 *arg2, s32 *arg3)
{
    u32 value;
    s32 result;

    value = arg1.value & 0xFFF;
    *arg2 = value >> 5;
    *arg3 = value & 0x1F;
    if (((arg1.value >> 12) & 7) < 3)
    {
        result = D_80122B74 + 0xE4;
    }
    else
    {
        result = D_80122B78;
        if (arg1.value & 0x8000)
        {
            *arg2 += (func_800C1B60(arg0)->unk28 >> 9) & 0x7F;
        }
    }
    return result;
}

extern u8 D_800F0E08[8];

s32 func_800BD318(s32 arg0, FieldScriptVariableRef arg1, s32 *arg2, s32 *arg3);
s32 func_800BD650(s32 arg0, u8 *arg1, s32 arg2, s32 arg3, s32 arg4);

/**
 * @brief Resolve and read a packed field-script variable reference.
 * @param arg0 Owner or record identifier used while resolving the reference.
 * @param arg1 Packed variable reference to read.
 * @return Value read from the resolved variable.
 */
s32 func_800BD3B0(s32 arg0, FieldScriptVariableRef arg1)
{
    s32 sp18;
    s32 sp1c;
    s32 result;

    result = func_800BD318(arg0, arg1, &sp18, &sp1c);
    return func_800BD650(2, (u8 *)result, sp18, sp1c, D_800F0E08[(arg1.value >> 12) & 7]);
}

s32 func_800BD3B0(s32 arg0, FieldScriptVariableRef arg1);

/**
 * @brief Resolve a 16-bit field-script variable reference and discard its value.
 * @param arg0 Owner or record identifier used while resolving the reference.
 * @param arg1 Packed 16-bit variable reference.
 */
void func_800BD414(s32 arg0, s32 arg1)
{
    FieldScriptVariableRef var_ref;

    var_ref.value = arg1;
    func_800BD3B0(arg0, var_ref);
}

extern u8 D_800F0E08[8];

s32 func_800BD318(s32 arg0, FieldScriptVariableRef arg1, s32 *arg2, s32 *arg3);
void func_800BD55C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);

/**
 * @brief Resolve a packed field-script variable reference and write a value.
 * @param arg0 Owner or record identifier used while resolving the reference.
 * @param arg1 Packed variable reference to write.
 * @param arg2 Value to write.
 */
void func_800BD434(s32 arg0, FieldScriptVariableRef arg1, s32 arg2)
{
    s32 sp18;
    s32 sp1c;
    s32 result;

    result = func_800BD318(arg0, arg1, &sp18, &sp1c);
    func_800BD55C(2, result, sp18, sp1c, D_800F0E08[(arg1.value >> 12) & 7], arg2);
}

extern u8 D_800F0E08[8];

s32 func_800BD318(s32 arg0, FieldScriptVariableRef arg1, s32 *arg2, s32 *arg3);
void func_800BD55C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);

/**
 * @brief Resolve a packed variable reference and write using decremented width metadata.
 * @param arg0 Owner or record identifier used while resolving the reference.
 * @param arg1 Packed variable reference to write.
 * @param arg2 Value to write.
 */
void func_800BD4A8(s32 arg0, FieldScriptVariableRef arg1, s32 arg2)
{
    s32 sp18;
    s32 sp1c;
    s32 result;

    result = func_800BD318(arg0, arg1, &sp18, &sp1c);
    func_800BD55C(2, result, sp18, sp1c, D_800F0E08[(arg1.value >> 12) & 7] - 1, arg2);
}

void func_800BD434(s32 arg0, FieldScriptVariableRef arg1, s32 arg2);
void func_800BD4A8(s32 arg0, FieldScriptVariableRef arg1, s32 arg2);

/**
 * @brief Dispatch a raw variable reference to the appropriate write helper.
 * @param arg0 Owner or record identifier used while resolving the reference.
 * @param arg1 Raw variable reference value.
 * @param arg2 Value to write.
 */
void func_800BD520(s32 arg0, u32 arg1, s32 arg2)
{
    FieldScriptVariableRef var_ref;

    var_ref.value = arg1;
    if (arg1 <= 0xFFFFU)
    {
        func_800BD434(arg0, var_ref, arg2);
        return;
    }
    func_800BD4A8(arg0, var_ref, arg2);
}

void func_800BD55C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    s32 clearMask;
    s32 valueMask;
    u8 *p8;
    u16 *p16;
    s32 *p32;

    if (arg1 == 0)
    {
        return;
    }
    if (arg1 == -1)
    {
        return;
    }

    if (arg4 < 0x20)
    {
        clearMask = ~(((1 << arg4) - 1) << arg3);
    }
    else
    {
        clearMask = 0;
    }

    if (arg4 < 0x20)
    {
        valueMask = ((1 << arg4) - 1) << arg3;
    }
    else
    {
        valueMask = -1;
    }

    switch (arg0)
    {
    case 0:
        p8 = (u8 *)(arg1 + arg2);
        *p8 = (*p8 & clearMask) | (valueMask & (arg5 << arg3));
        break;
    case 1:
        p16 = (u16 *)((arg2 * 2) + arg1);
        *p16 = (*p16 & clearMask) | (valueMask & (arg5 << arg3));
        break;
    case 2:
        p32 = (s32 *)((arg2 * 4) + arg1);
        *p32 = (*p32 & clearMask) | (valueMask & (arg5 << arg3));
        break;
    }
}

/**
 * @brief Extract a masked bitfield from an 8-, 16-, or 32-bit element.
 *
 * @param arg0 Element width selector: 0 = byte, 1 = halfword, 2 = word.
 * @param arg1 Base address of the element array.
 * @param arg2 Element index.
 * @param arg3 Right-shift amount.
 * @param arg4 Number of low bits to retain; values >= 32 retain all bits.
 * @return The selected element shifted right by @p arg3 and masked to @p arg4 bits.
 * @note 100% match with the FIELD GCC 2.8.0 G0 toolchain.
 */
s32 func_800BD650(s32 arg0, u8 *arg1, s32 arg2, s32 arg3, s32 arg4)
{
    s32 mask;
    s32 value;

    if (arg4 < 0x20)
    {
        mask = (1 << arg4) - 1;
    }
    else
    {
        mask = -1;
    }

    switch (arg0)
    {
    case 0:
        value = arg1[arg2] >> arg3;
        return value & mask;
    case 1:
        value = (*(u16 *)(arg1 + arg2 * 2)) >> arg3;
        return value & mask;
    case 2:
        return (*(u32 *)(arg1 + arg2 * 4) >> arg3) & mask;
    }
}
