#include "common.h"

typedef struct
{
    union
    {
        u32 word;
        u8 owner_id;
    } status;
    s32 active_record;
    u8* pc;
} FieldScriptContext;

extern FieldScriptContext* g_field_script;

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
