#include "common.h"

extern s32 func_800839F8(s32 arg0, s32 arg1);
extern s32 func_80083EEC(s32 arg0, s32 arg1, s32 arg2);
extern void field_start_actor_animation(s32 slot_index, int target_count, u8 *targets);

/**
 * @brief Starts an actor's animation when its slot resolves and passes a check.
 *
 * Resolves the actor slot for @p rec's 0x3A id via func_800839F8; if valid and
 * func_80083EEC (given @p arg1) succeeds, starts that slot's animation.
 */
void func_80092C24(u8 *rec, s32 arg1)
{
    s32 v = func_800839F8(rec[0x3A], 0);

    if (v != -1)
    {
        if (func_80083EEC(rec[0x3A], v, arg1))
        {
            field_start_actor_animation(v, 0, 0);
        }
    }
}
