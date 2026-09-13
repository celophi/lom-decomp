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
