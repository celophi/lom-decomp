#include "common.h"

/** @brief Partial status layout used by func_800B99A8. */
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

/** @brief Partial StructB800B99A8 layout used by func_800B99A8. */
typedef struct
{
    u8 pad0[0x41C];
    u32 unk41C;
} StructB800B99A8;

extern FieldScriptContext *g_field_script;
extern StructB800B99A8 *D_80122B78;

s32 func_8006751C(s32 arg0);
u8 func_80067598(s32 arg0);
void func_800BD520(s32 arg0, u32 arg1, s32 arg2);

/**
 * @brief Handle a script mode query, advancing the PC or clearing the run flag.
 * @note A 0xFF operand selects the mode from the shared field state.
 * @note WIP: control-flow and temporary-register differences remain.
 */
void func_800B99A8(void)
{
    FieldScriptContext *rec;
    s32 mode;
    s32 selector;
    s32 result;

    rec = &g_field_script[g_field_script->active_record];
    mode = rec->pc[1];
    if (mode == 0xFF)
    {
        mode = (D_80122B78->unk41C >> 8) & 3;
    }
    selector = mode & 3;
    result = func_8006751C(selector);
    if (!(mode & 0x80))
    {
        if (result == -1)
        {
            func_800BD520(0, 0x7100, func_80067598(selector));
            goto advance_pc;
        }
        goto clear_flag;
    }
    if (result == 3)
    {
    advance_pc:
        rec = &g_field_script[g_field_script->active_record];
        rec->pc += 2;
        return;
    }
clear_flag:
    g_field_script->status.word &= 0x7FFFFFFF;
}
