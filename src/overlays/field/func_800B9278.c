#include "field_script.h"

extern void akao_set_song_params(s32, s32, s32, s32);
extern u8 *func_80087EF0(s32);

/**
 * @brief Push a script record and resolve its new program counter.
 * @note Clamp the depth at seven and report overflow through AKAO.
 * @note Keep both wait-word mask updates and the header's shifted record stride;
 * the target reloads the active depth between stores.
 * @note GCC 2.8.0 G0: 100% match, 105 instructions (420 bytes).
 */
void func_800B9278(void)
{
    u16 operand;
    s32 depth;

    depth = g_field_script->active_record + 1;
    g_field_script->active_record = depth;
    if (depth >= 8)
    {
        g_field_script->active_record = 7;
        akao_set_song_params(0x8001, 2, g_field_script->status.owner_id, 0x4B);
    }
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->pc = FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->pc;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->flags = FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->flags;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->wait |= 1;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->wait &= 1;
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->pc = field_script_read_u16(FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->pc + 1, &operand);
    FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record)->pc = func_80087EF0(func_800BD3B0(g_field_script->status.owner_id, operand << 16) & 0x7FFF);
}
