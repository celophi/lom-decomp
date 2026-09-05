#include "common.h"
extern u8 *g_field_script;
extern s32 field_script_read_operand(s32, void *, s32 *);

s32 field_script_read_operand_or_owner(s32 arg0, void *arg1, s32 *arg2)
{
    s32 result;

    result = field_script_read_operand(arg0, arg1, arg2);
    if (*arg2 == 0xFF) {
        *arg2 = *g_field_script;
    }
    return result;
}
