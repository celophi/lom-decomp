#include "field_script.h"

extern s32 func_800BD414(s32 arg0, s32 arg1);
extern s32 func_8008B398(s32 key);

/**
 * @brief Opcode 0x13: resolve an operand through func_800BD414 and stop the
 *        script this frame if the resolved value maps to a slot below 2.
 */
void func_800B977C(void)
{
    FieldScriptRecord* rec;
    u8* pc;
    s32 flag;
    s32 code;
    s32 result;
    s32 param;
    s32 s1;

    rec = FIELD_SCRIPT_ACTIVE_RECORD();
    pc = rec->pc;
    flag = pc[1];
    code = pc[2] | (pc[3] << 8);
    result = func_800BD414(0, code);
    if (flag != 0)
    {
    }
    else
    {
        param = (result != 0xFF) ? result : g_field_script->status.owner_id;
        s1 = func_8008B398(param) < 2;
    }

    if (s1)
    {
        g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
    }
    else
    {
        FIELD_SCRIPT_ACTIVE_RECORD()->pc += 4;
    }
}
