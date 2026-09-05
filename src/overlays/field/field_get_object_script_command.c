#include "common.h"

typedef struct
{
    u8 pad0[0x28];
    u8 script_index;
    u8 pad29[0x2C - 0x29];
    u16 script_offset;
    u8 pad2E[0x3A - 0x2E];
    u8 object_index;
} FieldObjectState;

typedef struct
{
    u8 pad0[0x168];
    u8 *custom_script_base;
    u8 pad16C[0x23C - 0x16C];
} FieldObjectRuntime;

extern FieldObjectRuntime D_80105AE0[];
extern u16 *D_8010A02C;

/**
 * @brief Resolve the current command in a field object's active script.
 * @param object Field object whose active script and bytecode offset are resolved.
 * @return Pointer to the object's current script command.
 */
u8 *field_get_object_script_command(FieldObjectState *object)
{
    u8 *script_base;

    if (object->script_index == 0xFE)
    {
        script_base = D_80105AE0[object->object_index].custom_script_base;
    }
    else
    {
        script_base = (u8 *)D_8010A02C + D_8010A02C[object->script_index];
    }
    return script_base + object->script_offset;
}
