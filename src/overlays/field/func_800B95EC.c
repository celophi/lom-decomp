#include "common.h"
#include "field_script.h"

s32 func_8005A84C(s32 arg0, s32 arg1);
s32 func_8008B398(s32 key);
s32 func_8006751C(s32 arg0);

/**
 * @brief Evaluate opcode 0x12's condition selector and advance or pause the active script.
 * @param unused0 Unused value inherited from the opcode dispatcher.
 * @param unused1 Unused value inherited from the opcode dispatcher.
 * @param wait Initial wait condition; valid selectors replace it with the evaluated result.
 */
void func_800B95EC(s32 unused0, s32 unused1, s32 wait)
{
    FieldScriptRecord* rec;
    u8* pc;
    u32 selector;
    s32 operand;
    s32 result;

    pc = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    selector = pc[1];
    operand = pc[2];

    switch (selector)
    {
    case 0:
        result = func_8005A84C(0, operand) ^ 2;
        wait = 0 < (u32)result;
        break;
    case 1:
        result = func_8005A84C(0, operand) ^ 3;
        wait = 0 < (u32)result;
        break;
    case 2:
    {
        s32 key = operand;
        if (operand == 0xFF)
        {
            key = g_field_script->status.owner_id;
        }
        result = func_8008B398(key);
        wait = result < 2;
        break;
    }
    case 3:
    {
        s32 key = operand;
        if (operand == 0xFF)
        {
            key = g_field_script->status.owner_id;
        }
        result = func_8008B398(key) ^ 1;
        wait = 0 < (u32)result;
        break;
    }
    case 4:
    {
        s32 key = operand;
        if (operand == 0xFF)
        {
            key = g_field_script->status.owner_id;
        }
        result = func_8008B398(key);
        wait = 0 < (u32)result;
        break;
    }
    case 5:
        result = func_8006751C(operand & 3) ^ 1;
        wait = 0 < (u32)result;
        break;
    case 6:
        result = func_8006751C(operand & 3);
        wait = 0 < (u32)result;
        break;
    }

    if (wait)
    {
        g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
    }
    else
    {
        rec = FIELD_SCRIPT_RECORD(g_field_script->active_record);
        rec->pc += 3;
    }
}
