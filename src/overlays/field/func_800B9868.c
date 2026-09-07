#include "field_script.h"

typedef struct
{
    u8 pad0[0x402];
    u16 unk402;
} StructB78Local;

void func_800BD520(s32 arg0, s32 arg1, s32 arg2);

extern s32 D_8011F428;
extern s32 D_801227F0;
extern StructB78Local *D_80122B78;

/**
 * @brief Evaluate the active field-script condition and advance or pause the script record.
 */
void func_800B9868(void)
{
    FieldScriptRecord *rec;
    u8 *pc;
    u32 code;
    s32 flag;
    s32 v0;
    FieldScriptContext *ctx;
    s32 active_record;

    active_record = g_field_script->active_record;
    ctx = g_field_script;
    rec = (FieldScriptRecord *)((u8 *)ctx + ((active_record * 3) << 2));
    pc = rec->pc;
    code = pc[1];
    switch (code)
    {
    case 1:
        flag = D_80122B78->unk402 & 1;
        break;
    case 2:
        flag = D_801227F0 != 2;
        break;
    case 3:
        v0 = D_8011F428 ^ 1;
        flag = v0 == 0;
        break;
    case 4:
        v0 = D_8011F428;
        flag = v0 == 0;
        break;
    }
    if (flag != 0)
    {
        g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
        return;
    }
    active_record = g_field_script->active_record;
    ctx = g_field_script;
    rec = (FieldScriptRecord *)((u8 *)ctx + ((active_record * 3) << 2));
    rec->pc += 2;
    if ((u32)(code - 3) < 2)
    {
        func_800BD520(0, 0x7100, D_8011F428);
    }
}
