#include "common.h"

typedef struct
{
    union
    {
        u32 word;
        u8 owner_id;
    } status;
    s32 active_record;
    u8 *pc;
} FieldScriptContext;

extern FieldScriptContext *g_field_script;

u8 *field_script_read_u16(u8 *data, u16 *value);
s32 func_800BD3B0(s32 arg0, s32 arg1);

/**
 * @brief Decode one field script operand and advance the read pointer.
 * @param type Two-bit operand kind: 0 script variable, 1 byte, 2 halfword, 3 word.
 * @param data Operand byte stream to read from.
 * @param value Destination for the decoded 32-bit value.
 * @return @p data advanced past the bytes consumed for this operand.
 */
u8 *field_script_read_operand(s32 type, u8 *data, s32 *value)
{
    type &= 3;
    switch (type)
    {
    case 0:
    {
        u16 var_index;

        data = field_script_read_u16(data, &var_index);
        *value = func_800BD3B0(g_field_script->status.owner_id, var_index << 16);
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
