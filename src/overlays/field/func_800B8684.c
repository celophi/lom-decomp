#include "field_script.h"
#include "game_audio.h"

/**
 * @brief Push a field-script subroutine record and branch to its target.
 */
void func_800B8684(void)
{
    s32 depth;
    s32 next_depth;

    depth = g_field_script->active_record;
    next_depth = depth + 1;
    g_field_script->active_record = next_depth;
    if (next_depth >= 8)
    {
        FieldScriptRecord* rec;

        akao_set_song_params(0x8001, 2, g_field_script->status.owner_id, 0);
        g_field_script->active_record = 7;
        rec = FIELD_SCRIPT_RECORD(7);
        rec->pc += 3;
        return;
    }

    FIELD_SCRIPT_RECORD(next_depth)->pc = FIELD_SCRIPT_RECORD(depth)->pc;
    FIELD_SCRIPT_ACTIVE_RECORD_STATE()->flags = FIELD_SCRIPT_RECORD_STATE(g_field_script->active_record - 1)->flags;
    FIELD_SCRIPT_ACTIVE_RECORD_STATE()->wait |= 1;
    FIELD_SCRIPT_ACTIVE_RECORD_STATE()->wait &= 1;
    FIELD_SCRIPT_RECORD(g_field_script->active_record - 1)->pc += 3;
    field_script_branch(1);
}
