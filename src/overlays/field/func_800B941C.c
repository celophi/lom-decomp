#include "field_script.h"

extern u8 *func_800C1B60(s32);
extern void func_8009C620(s32, s32, s32, s32);
extern void func_8009C77C(s32, s32, s32);
#define REC ((FieldScriptRecord *)((u8 *)g_field_script + (g_field_script->active_record * 3 << 2)))

/**
 * @brief Decode a seven-byte field command and dispatch its action parameters.
 * @note 0xFE and 0xFF select special target handling; preserve signed -1.
 * @note WIP: repeated script-pointer loads and branch-layout differences remain.
 */
void func_800B941C(void)
{
    u8 owner;
    s32 value;
    u8 flags;
    s32 target;
    u8 mode;
    s32 selector;
    u8 kind;
    u8 *pc;
    u8 byte;

    pc = REC->pc;
    owner = pc[1];
    if (owner == 0xFF)
    {
        owner = g_field_script->status.owner_id;
    }
    pc = REC->pc;
    mode = pc[4];
    value = pc[2] + (pc[3] << 8);
    switch (mode)
    {
    case 0xFE:
        target = -1;
        break;
    case 0xFF:
        byte = func_800C1B60(owner)[1];
        target = -1;
        if (byte != mode)
        {
            target = byte;
        }
        break;
    default:
        target = REC->pc[4];
        break;
    }
    pc = REC->pc;
    kind = pc[6];
    flags = pc[5];
    if (kind == 7)
    {
        target = -1;
    }
    selector = flags & 3;
    if (!(flags & 0x80))
    {
        if (flags & 0x40)
        {
            target |= 0x40;
        }
        else
        {
            target |= (flags & 1) << 6;
            selector = flags & 3;
        }
    }
    func_8009C620(selector, kind, owner, target);
    func_8009C77C(flags, value, 1);
    REC->pc += 7;
}
