#include "field_script.h"

s32 func_800875C4(s32 arg0, u32 arg1, void* arg2, void* arg3);
extern s32 D_8010AE78;
extern u8* D_80122B74;

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
