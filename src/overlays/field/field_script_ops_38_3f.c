#include "field_script.h"
#include "game_audio.h"

/* Field script opcode handlers 0x38 through 0x3F (see field_script.h). */

/**
 * @brief Opcode 0x38: no operation; step past the opcode.
 */
void field_script_op_38(void)
{
    FieldScriptRecord* rec = (FieldScriptRecord*)g_field_script;
    s32 depth = g_field_script->active_record;

    rec += depth;
    rec->pc += 1;
}

/**
 * @brief Opcode 0x39: queue audio sub-command 0x39 for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_39(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x39);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3A: queue audio sub-command 0x3A for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3a(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x3A);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3B: queue audio sub-command 0x3B for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3b(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x3B);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3C: queue audio sub-command 0x3C for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3c(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x3C);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3D: queue audio sub-command 0x3D for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3d(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x3D);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3E: queue audio sub-command 0x3E for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3e(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x3E);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}

/**
 * @brief Opcode 0x3F: queue audio sub-command 0x3F for the owner and end the step loop.
 * @return The updated context status word.
 * @note Does not advance the program counter.
 */
s32 field_script_op_3f(void)
{
    akao_set_song_params(0x8001, 1, g_field_script->status.owner_id, 0x3F);
    return g_field_script->status.word &= ~FIELD_SCRIPT_RUNNING;
}
